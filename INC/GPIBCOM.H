// FILE : gpibcom.h

#ifndef GPIBCOM_INCLUDED
#define GPIBCOM_INCLUDED

#define MIN_GPIB_ADDR  0
#define MAX_GPIB_ADDR 31

extern int board_id ;

extern int gpib_timeout_index ;

extern BOOLEAN aborted ;

extern BOOLEAN fake_detector ;

BOOLEAN init_gpib( void );
/***************************************************************************/
/* Return truth that init_gpib succeeded                                   */
/***************************************************************************/
BOOLEAN WasGPIBInit(void);
void shutdown_gpib( void );
int set_gpib_timeout( int );

/* added for the 1235 */
int dma_off( void);
int dma_on(int dma_val);
int goto_local(int addr);
/* end of 1235 additions */


BOOLEAN serial_poll_gpib( int, unsigned char * );
int test_for_serial_poll(int gpib_addr);
int next_gpib_address(int current_address);
unsigned int talk_gpib(int gpib_addr);
unsigned int listen_gpib(int gpib_addr);
unsigned int unaddress_gpib(void);

void set_gets_gpib_mode( BOOLEAN saveBuf ) ;
unsigned int gets_gpib( char *, unsigned *, int );
unsigned int puts_gpib( char *, unsigned *, int );
unsigned int clear_gpib(void);
void async_gets_gpib( char *, unsigned *, int);
unsigned int wait_for_async_read( unsigned * );
unsigned int test_for_async_read_done( unsigned * );
void abort_async_io( void );

#endif  //  GPIBCOM_INCLUDED

