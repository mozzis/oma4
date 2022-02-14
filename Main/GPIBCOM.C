/* -----------------------------------------------------------------------
/
/  gpibcom.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/gpibcom.c_v   0.23   13 Jan 1992 09:29:58   cole  $
/  $Log:   J:/logfiles/oma4000/main/gpibcom.c_v  $
/
/  Author : TLB & MLM
/
/  gpibcom.c provides communication functions for the National
/  Instruments' GPIB interface.
/
/ ----------------------------------------------------------------------- */

#include <string.h>   // strlen()
#include <stdio.h>
#include <time.h>     // time_t
#include <malloc.h>   // free()

#ifndef DOS16_INCLUDED
   #include <dos16.h>
#endif

#include <decl.h>          // ibsta
#include "eggtype.h"
#include "gpibcom.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

enum { ADDR_PC = 0 };  // PC's GPIB address is zero

// OR these with device addresses to address for Listen or Talk
enum { LISTEN = 0x20, TALK = 0x40 };

char boardname[] = "GPIB0";        // PC GPIB interface board 0

typedef struct {
  unsigned char init_cmd       [6];
  unsigned char unaddress      [3];
  unsigned char spoll_resp;
  unsigned char ser_poll       [4];
  unsigned char ser_poll_done  [4];
  unsigned char ser_poll_test  [6];
  unsigned char talk_pc        [5];
  unsigned char puts_talk_pc   [4];
  unsigned char talk_instrument[5];
  unsigned char listen_pc      [3];
  unsigned char * transfer_string;
  unsigned transfer_size;
  } string_buffs;


string_buffs string_temp = {
/* unsigned char init_cmd          */ { 5, UNL, UNT, GTL, SDC, TCT },
/* unsigned char unaddress         */ { 2, UNL, UNT},
/* unsigned char spoll_resp;       */ { '\0' },
/* unsigned char ser_poll          */ { 3, SPE, LISTEN | ADDR_PC },
/* unsigned char ser_poll_done     */ { 3, UNL, UNT, SPD },
/* unsigned char ser_poll_test     */ { 5, UNL, UNT, SPE, LISTEN, TALK},
/*                                 */ 
/* unsigned char talk_pc           */ { 4, UNT, LISTEN, GTL, UNL },
/*                                 */ 
/* unsigned char puts_talk_pc      */ { 3, UNT, LISTEN, TALK | ADDR_PC },
/* unsigned char talk_instrument   */ { 4, UNL, UNT, LISTEN | ADDR_PC, TALK },
/*                                 */ 
/* unsigned char listen_pc         */ { 2, LISTEN, '\0' },
/* unsigned char * transfer_string;*/ { NULL },
/* unsigned transfer_size;         */ { 80 },
};

string_buffs * gpib_buf;

int board_id;                     // 'handle' from board open call
int address_pc = ADDR_PC;
// int gpib_timeout_index = T3s;

PRIVATE BOOLEAN board_present = FALSE;

// macro to simplify calling the ibcmd() function the way I want to...
#define command(cmd_str) ibcmd(board_id, &(gpib_buf->cmd_str[1]), gpib_buf->cmd_str[0])


/***************************************************************************/

PRIVATE void init_commands(void)
{
  SHORT PrevStrategy = D16MemStrategy(MForceLow); /* alloc buf in low mem */

  gpib_buf = D16MemAlloc(sizeof(string_buffs));

  memcpy(gpib_buf, &string_temp, sizeof(string_temp)); /* fill with defaults */

  if (gpib_buf->transfer_string)        /* if pointer from prev incarnaton? */
    D16MemFree(gpib_buf->transfer_string);

  gpib_buf->transfer_string = D16MemAlloc(80);   /* start with 80 byte data buf */
  
  gpib_buf->transfer_size = 80;
  
  D16MemStrategy(PrevStrategy);
}

/***************************************************************************/
/* Open and initialize the PC's GPIB interface.                            */
/***************************************************************************/
BOOLEAN init_gpib(void)
{
  init_commands();
  if((board_id = ibfind(boardname)) < 0)
    board_present = FALSE;
  else
    {
    command(init_cmd);
    ibsic(board_id);                      // interface clear
    ibwait(board_id, CMPL | CIC | ATN | ERR);
    ibsre(board_id, TRUE);                // set remote enable
    ibwait(board_id, CMPL | CIC | ATN | ERR);
    ibdma(board_id, TRUE);                // enable DMA transfers
    ibwait(board_id, CMPL | CIC | ATN | ERR);
    board_present = TRUE;
    }
  return board_present;
}

/***************************************************************************/
/* Return truth that init_gpib succeeded                                   */
/***************************************************************************/
BOOLEAN WasGPIBInit(void)
{
  return board_present;
}

// Close down the GPIB interface when all done.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void shutdown_gpib(void)
{
   ibsre(board_id, FALSE);          /* reset remote enable */
   ibdma(board_id, FALSE);          /* disable DMA transfers */
   ibsic(board_id);                 /* MLM 4-5-90 */
   ibonl(board_id, FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int set_gpib_timeout(int time)
{
   int former_timeout = time;

   if(board_present)
   {
      ibtmo(board_id, time);
      if(! (ibsta & ERR))
         former_timeout = iberr;
   }
   return former_timeout;
}

// ------- for the 1235 the following 3 functions were added. -----------

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int set_gpib_dma(int dma_state)
{
   int former_state = dma_state;

   if(board_present)
   {
      ibdma(board_id, dma_state);
      
      if (! (ibsta & ERR))
         former_state = iberr;
   }
   return former_state;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int dma_off(void)
{
   return(set_gpib_dma(0));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int dma_on(int dma_val)
{
   return(set_gpib_dma(dma_val));
}

/***************************************************************************/
int goto_local(int addr)
{
  if (board_present)
    {
    gpib_buf->talk_pc[2] = (unsigned char) (LISTEN | addr);
    command(talk_pc);
    }
  return ibsta;
}

/* --------------- end of 1235 stuff ----------------- */

/***************************************************************************/
/* Get the serial poll byte from the instrument.                           */
/* Return TRUE iff all is well, FALSE if error                             */
/***************************************************************************/
BOOLEAN serial_poll_gpib(int gpib_addr, unsigned char * spoll_byte)
{
   BOOLEAN success = TRUE;

   if(board_present)
   {
      unsigned int standard_timeout = T3s;

      gpib_buf->ser_poll[3] = (unsigned char) (TALK | gpib_addr);
      ibtmo(board_id, T300ms);        // shorten timeout period

      if(! (ibsta & ERR))               // save old timeout value
         standard_timeout = iberr;      // unless there was an error

      command(ser_poll);   // address instrument,
                                                     //enable serial poll

      // read one serial poll byte
      ibrd(board_id, &(gpib_buf->spoll_resp), 1);
      *spoll_byte = gpib_buf->spoll_resp;

      if(ibcnt != 1)
         success = FALSE;

      // disable serial poll

      command(ser_poll_done);

      // restore previous timeout value

      ibtmo(board_id, standard_timeout);
   }
   else
      success = FALSE;

   return success;
}

/***************************************************************************/
int test_for_serial_poll(int gpib_addr)
{
  int status;
  char dummy[sizeof(int)];
  
  gpib_buf->ser_poll_test[4] = (UCHAR)(LISTEN | address_pc);
  gpib_buf->ser_poll_test[5] = (UCHAR)(TALK | gpib_addr);

  command(ser_poll_test);     /* address instrument, enable serial poll */
  ibcnt = 0;
  ibrd(board_id, dummy, 1);              /* read one serial poll byte */
  status = ((! (ibsta & (TIMO | ERR))) && (ibcnt == 1));
  command(ser_poll_done); /* disable serial poll */
  return (status);
}

/***************************************************************************/
int next_gpib_address(int current_address)
{
  if (++current_address > MAX_GPIB_ADDR)
    return(1);
  else
    return(current_address);
}

PRIVATE BOOLEAN DontUseLocal = FALSE;  // use Local Buffer by default

/***************************************************************************/
void set_gets_gpib_mode(BOOLEAN DontUseLoc)
{
  DontUseLocal = DontUseLoc;
}

/***************************************************************************/
BOOLEAN check_transfer_buf(unsigned count)
{
  int success = TRUE;

  if (!DontUseLocal)
    {
    if(gpib_buf->transfer_size <= count)
      {
      SHORT PrevStrategy = D16MemStrategy(MForceLow);

      // current LocalString not big enough, get a new one
      // first get rid of the old one

      if (gpib_buf->transfer_string)
        D16MemFree(gpib_buf->transfer_string);

      gpib_buf->transfer_string = D16MemAlloc(count + 1);

      D16MemStrategy(PrevStrategy);

      if (!gpib_buf->transfer_string)
        {
        gpib_buf->transfer_size = 0;
        success = FALSE;
        }
      else
        gpib_buf->transfer_size = count + 1;
      }
    }
  return success;
}

/***************************************************************************/
/* Get a string from the instrument.                                       */
/*                                                                         */
/* requires: (char *) string - a string buffer to hold the string returned */
/*              from the instrument.                                       */
/*           (unsigned int *) count - maximum number of bytes to read      */
/*              from the instrument.                                       */
/*                                                                         */
/*  returns: (unsigned int) error status (from the global variable ibsta)*/
/*                         or the status returned from ibrd() if no error  */
/*            (unsigned int *) count - will be set with the actual number  */
/*              of bytes read from the instrument.                         */
/***************************************************************************/
unsigned int gets_gpib(char * string, unsigned * count, int gpib_addr)
{
  unsigned int status = ERR;

  if (board_present && check_transfer_buf(*count))
    {
    unsigned int ibrdStatus = ERR;

    gpib_buf->talk_instrument[4] = (unsigned char) (TALK | gpib_addr);

    status = command(talk_instrument);

    if (! (status & ERR))
      {
      if (DontUseLocal)
        ibrdStatus = ibrd(board_id, string, *count);
      else
        ibrdStatus = ibrd(board_id, gpib_buf->transfer_string, *count);

      // ibrd() will time out if fewer than * count characters received.
      // timeout is NOT an error.

      status = ibrdStatus;

      if((!(status & ERR)) || (status & TIMO))
        {
        * count = ibcnt;

        if (!DontUseLocal)
          memcpy(string, gpib_buf->transfer_string, *count);

        status = command(unaddress);

        if (!(status & ERR))
          status = ibcac(board_id, TRUE); // take control sync
        }
      }
    status = ibrdStatus;  // return the status from ibrd()
    }

  return status;
}
/************************************************************************/
/* Write a string to the instrument.                                    */
/*                                                                      */
/*  (char *) string -- a string buffer to send to the instrument.       */
/*  (unsigned int *) count -- the number of bytes from the              */
/*                            buffer to send to the instrument.         */
/*                                                                      */
/*  return (unsigned int) error status from the global variable ibsta.  */
/*         (unsigned int *) count, will be set with the actual number   */
/*              of bytes written to the instrument.                     */
/*                                                                      */
/* Does NOT unlisten other addressed devices to allow broadcast commands*/
/************************************************************************/
unsigned int puts_gpib(char * string, unsigned * count, int gpib_addr)
{
  unsigned int status = ERR;

  if (board_present && check_transfer_buf(*count))
    {
    gpib_buf->puts_talk_pc[2] = (unsigned char) (LISTEN | gpib_addr);
    command(puts_talk_pc);

    memcpy(gpib_buf->transfer_string, string, * count);
    ibwrt(board_id, gpib_buf->transfer_string, * count);

    * count = ibcnt;
    status  = (ibsta & 0xE100) |  /* ibsta bits 15-13, 8 */
              (iberr & 0x04ff);   /* iberr bits 0-7, 10 */

    command(unaddress);
    ibcac(board_id, TRUE);       // take control sync
    }

  return status;
}

// Address a device on the gpib bus to listen.
//
//  requires: int gpib_Addr - the device of the address to LISTEN
//
//  Returns (unsigned int) error status from the global variable ibsta.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned int listen_gpib(int gpib_addr)
{
   gpib_buf->listen_pc[1] = (unsigned char) (LISTEN | gpib_addr);
   command(listen_pc);

   return ibsta;
}

// Address a device on the gpib bus to talk.
//
// requires: int gpib_Addr - the device of the address to TALK
//
// returns:  (unsigned int) error status - from the global variable ibsta.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned int talk_gpib(int gpib_addr)
{
   gpib_buf->talk_instrument[4] = (unsigned char) (TALK | gpib_addr);
   command(talk_instrument);

   return ibsta;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned int unaddress_gpib(void)
{
   command(unaddress);
   //  ibcac(board_id, TRUE);           /* take control sync */
   //  ibgts(board_id, FALSE);          /* release control   */
   return ibsta;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned int clear_gpib(void)
{
   ibsic(board_id);          // send interface clear
   ibsre(board_id, TRUE);    // set remote enable
   ibwait(board_id, CMPL | CIC | ATN | ERR);
   return ibsta;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void async_gets_gpib(char * string, unsigned * count, int gpib_addr)
{
   gpib_buf->talk_instrument[4] = (unsigned char) (TALK | gpib_addr);

   command(talk_instrument);

   ibrda(board_id, string, * count);

   memcpy(string, string, * count);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned int wait_for_async_read(unsigned * count)
{
   ibwait(board_id, CMPL | ERR | TIMO);

   * count = ibcnt;

   command(unaddress);

   return ibsta;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned int test_for_async_read_done(unsigned * count)
{
   // mask of zero means don't wait, just update vars
   ibwait(board_id, 0);

   * count = ibcnt;

   if(ibsta & (CMPL | ERR))
      command(unaddress);

   return ibsta;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void abort_async_io()
{
   ibstop(board_id);
}



