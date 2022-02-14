/*
 *    If you are using the Microsoft C compiler version 5.1 or later,
 *    the following two constants should be defined.
 */
#define  MSC_FAR     _far
#define  MSC_CDECL   _cdecl

/*
 *    
 *    If you are using the Microsoft C compiler version 5.0 or earlier
 *    the following two constants should be defined.
 *
 *  #define  MSC_FAR     far
 *  #define  MSC_CDECL   cdecl
 */


/* C Declarations                                          */
/*                                                         */
/*                                                         */
/* Status variables declared public by mcib.obj.           */

extern  short           ibsta;  /* status word                    */
extern  short           iberr;  /* GPIB error code                */
extern  unsigned short      ibcnt;  /* number of bytes sent           */
extern  unsigned long   ibcntl; /* (same) number of bytes sent    */


/* GPIB Commands                                           */

#define UNL  0x3f  /* GPIB unlisten command                */
#define UNT  0x5f  /* GPIB untalk command                  */
#define GTL  0x01  /* GPIB go to local                     */
#define SDC  0x04  /* GPIB selected device clear           */
#define PPC  0x05  /* GPIB parallel poll configure         */
#define GET  0x08  /* GPIB group execute trigger           */
#define TCT  0x09  /* GPIB take control                    */
#define LLO  0x11  /* GPIB local lock out                  */
#define DCL  0x14  /* GPIB device clear                    */
#define PPU  0x15  /* GPIB parallel poll unconfigure       */
#define SPE  0x18  /* GPIB serial poll enable              */
#define SPD  0x19  /* GPIB serial poll disable             */
#define PPE  0x60  /* GPIB parallel poll enable            */
#define PPD  0x70  /* GPIB parallel poll disable           */

/* GPIB status bit vector :                                */
/*       global variable ibsta and wait mask               */

#define ERR     (1<<15) /* Error detected                  */
#define TIMO    (1<<14) /* Timeout                         */
#define END     (1<<13) /* EOI or EOS detected             */
#define SRQI    (1<<12) /* SRQ detected by CIC             */
#define RQS     (1<<11) /* Device needs service            */
#define SPOLL   (1<<10) /* Board has been serially polled  */
#define EVENT   (1<<9)  /* An event has occured            */
#define CMPL    (1<<8)  /* I/O completed                   */
#define LOK     (1<<7)  /* Local lockout state             */
#define REM     (1<<6)  /* Remote state                    */
#define CIC     (1<<5)  /* Controller-in-Charge            */
#define ATN     (1<<4)  /* Attention asserted              */
#define TACS    (1<<3)  /* Talker active                   */
#define LACS    (1<<2)  /* Listener active                 */
#define DTAS    (1<<1)  /* Device trigger state            */
#define DCAS    (1<<0)  /* Device clear state              */

/* Error messages returned in global variable iberr        */

#define EDVR  0  /* DOS error                              */
#define ECIC  1  /* Function requires GPIB board to be CIC */
#define ENOL  2  /* Write function detected no Listeners   */
#define EADR  3  /* Interface board not addressed correctly*/
#define EARG  4  /* Invalid argument to function call      */
#define ESAC  5  /* Function requires GPIB board to be SAC */
#define EABO  6  /* I/O operation aborted                  */
#define ENEB  7  /* Non-existent interface board           */
#define EOIP 10  /* I/O operation started before previous  */
                 /* operation completed                    */
#define ECAP 11  /* No capability for intended operation   */
#define EFSO 12  /* File system operation error            */
#define EBUS 14  /* Command error during device call       */
#define ESTB 15  /* Serial poll status byte lost           */
#define ESRQ 16  /* SRQ remains asserted                   */
#define ETAB 20  /* The return buffer is full.             */

/* EOS mode bits                                           */

#define BIN  (1<<12) /* Eight bit compare                  */
#define XEOS (1<<11) /* Send END with EOS byte		   */
#define REOS (1<<10) /* Terminate read on EOS              */

/* Timeout values and meanings                             */

#define TNONE    0   /* Infinite timeout (disabled)        */
#define T10us    1   /* Timeout of 10 us (ideal)           */
#define T30us    2   /* Timeout of 30 us (ideal)           */
#define T100us   3   /* Timeout of 100 us (ideal)          */
#define T300us   4   /* Timeout of 300 us (ideal)          */
#define T1ms     5   /* Timeout of 1 ms (ideal)            */
#define T3ms     6   /* Timeout of 3 ms (ideal)            */
#define T10ms    7   /* Timeout of 10 ms (ideal)           */
#define T30ms    8   /* Timeout of 30 ms (ideal)           */
#define T100ms   9   /* Timeout of 100 ms (ideal)          */
#define T300ms  10   /* Timeout of 300 ms (ideal)          */
#define T1s     11   /* Timeout of 1 s (ideal)             */
#define T3s     12   /* Timeout of 3 s (ideal)             */
#define T10s    13   /* Timeout of 10 s (ideal)            */
#define T30s    14   /* Timeout of 30 s (ideal)            */
#define T100s   15   /* Timeout of 100 s (ideal)           */
#define T300s   15   /* Timeout of 300 s (ideal)           */
#define T1000s  15   /* Timeout of 1000 s (ideal)          */


/*  IBLN Constants                                         */
#define NO_SAD   0
#define ALL_SAD -1


/*  IBEVENT Constants                                      */
#define  EventDTAS   1
#define  EventDCAS   2


/*
 *    GotoMultAddr flags
 */
#define  MultAddrPrimary      0x00
#define  MultAddrSecondary    0x01

#define  MultAddrListen       0x00
#define  MultAddrTalk         0x01
#define  MultAddrSerialPoll   0x81


/* Miscellaneous                                           */

#define S    0x08    /* parallel poll sense bit            */
#define LF   0x0A    /* ASCII linefeed character           */


/*  The following constants are used for the second parameter of the 
 *  ibconfig function.  They are the "option" selection codes.
 */
#define  IbcPAD        0x0001      /* Primary Address                      */
#define  IbcSAD        0x0002      /* Secondary Address                    */
#define  IbcTMO        0x0003      /* Timeout Value                        */
#define  IbcEOT        0x0004      /* Send EOI with last data byte?        */
#define  IbcPPC        0x0005      /* Parallel Poll Configure              */
#define  IbcREADDR     0x0006      /* Repeat Addressing                    */
#define  IbcAUTOPOLL   0x0007      /* Disable Auto Serial Polling          */
#define  IbcCICPROT    0x0008      /* Use the CIC Protocol?                */
#define  IbcIRQ        0x0009      /* Use PIO for I/O                      */
#define  IbcSC         0x000A      /* Board is System Controller?          */
#define  IbcSRE        0x000B      /* Assert SRE on device calls?          */
#define  IbcEOSrd      0x000C      /* Terminate reads on EOS               */
#define  IbcEOSwrt     0x000D      /* Send EOI with EOS character          */
#define  IbcEOScmp     0x000E      /* Use 7 or 8-bit EOS compare           */
#define  IbcEOSchar    0x000F      /* The EOS character.                   */
#define  IbcPP2        0x0010      /* Use Parallel Poll Mode 2.            */
#define  IbcTIMING     0x0011      /* NORMAL, HIGH, or VERY_HIGH timming.  */
#define  IbcDMA        0x0012      /* Use DMA for I/O                      */
#define  IbcReadAdjust 0x0013      /* Swap bytes during an ibrd.           */
#define  IbcWriteAdjust 0x014      /* Swap bytes during an ibwrt.          */
#define  IbcEventQueue 0x0015      /* Enable/disable the event queue.         */
#define  IbcSPollBit   0x0016      /* Enable/disable the visibility of SPOLL. */
#define  IbcSendLLO    0x0017      /* Enable/disable the sending of LLO.      */
#define  IbcSPollTime  0x0018      /* Set the timeout value for serial polls. */
#define  IbcPPollTime  0x0019      /* Set the parallel poll length period.    */
#define  IbcNoEndBitOnEOS 0x001A   /* Remove EOS from END bit of IBSTA.       */


/* These are the values are used by the Send 488.2 command. */
#define  NULLend (unsigned short)0x00  /* Do nothing at the end of a transfer.*/
#define  NLend   (unsigned short)0x01  /* Send NL with EOI after a transfer.  */
#define  DABend  (unsigned short)0x02  /* Send EOI with the last DAB.         */


/* This value is used by the 488.2 Receive command.
 */
#define  STOPend     (unsigned short)0x0100


/*
 *  This macro can be used to easily create an entry in address list
 *  that is required by many of the 488.2 functions.  An address list is
 *  just an array of unsigned integers.  The primary address goes in the
 *  lower 8-bits and the secondary address goes in the upper 8-bits.
 */
#define  MakeAddr(pad, sad)   ((unsigned short)(((unsigned char)(pad)) | (((unsigned char)(sad))<<8)))


/*
 *  The following two macros are used to "break apart" an address list
 *  entry.  They take an unsigned integer and return either the primary
 *  or secondary address stored in the integer.
 */
#define  GetPAD(val)    ((unsigned char)(val))
#define  GetSAD(val)    ((unsigned char)((val) >> 8))


/*
 *  The following values are used by the iblines function.  The unsigned
 *  integer returned by iblines contains:
 *       The lower byte will contain a "monitor" bit mask.  If a bit is
 *          set (1) in this mask, then the corresponding line can be
 *          monitiored by the driver.  If the bit is clear (0),
 *          then the line cannot be monitored.
 *       The upper byte will contain the status of the bus lines.
 *          Each bit corresponds to a certain bus line, and has
 *          a correcponding "monitor" bit in the lower byte.
 *
 */
#define  ValidEOI   (unsigned short)0x0080
#define  ValidATN   (unsigned short)0x0040
#define  ValidSRQ   (unsigned short)0x0020
#define  ValidREN   (unsigned short)0x0010
#define  ValidIFC   (unsigned short)0x0008
#define  ValidNRFD  (unsigned short)0x0004
#define  ValidNDAC  (unsigned short)0x0002
#define  ValidDAV   (unsigned short)0x0001
#define  BusEOI     (unsigned short)0x8000
#define  BusATN     (unsigned short)0x4000
#define  BusSRQ     (unsigned short)0x2000
#define  BusREN     (unsigned short)0x1000
#define  BusIFC     (unsigned short)0x0800
#define  BusNRFD    (unsigned short)0x0400
#define  BusNDAC    (unsigned short)0x0200
#define  BusDAV     (unsigned short)0x0100


/*
 *  This value is used to terminate an address list.  It should be
 *  assigned to the last entry.
 */
#define  NOADDR    0xFFFF


#ifdef __cplusplus
extern "C" {
#endif


extern short MSC_FAR MSC_CDECL ibbna(short handle, char MSC_FAR *bdname);
extern short MSC_FAR MSC_CDECL ibcac(short handle, short v);
extern short MSC_FAR MSC_CDECL ibclr(short handle);
extern short MSC_FAR MSC_CDECL ibcmd(short handle, char MSC_FAR *buffer, unsigned long cnt);
extern short MSC_FAR MSC_CDECL ibcmda(short handle, char MSC_FAR *buffer, unsigned long cnt);
extern short MSC_FAR MSC_CDECL ibconfig(short handle, unsigned short option, unsigned short value);
extern short MSC_FAR MSC_CDECL ibdev(short boardID, short padval, short sadval, short tmoval, short eotval, short eosval);
extern short MSC_FAR MSC_CDECL ibdma(short handle, short v);
extern short MSC_FAR MSC_CDECL ibeos(short handle, short v);
extern short MSC_FAR MSC_CDECL ibeot(short handle, short v);
extern short MSC_FAR MSC_CDECL ibevent(short handle, unsigned short MSC_FAR *event);
extern short MSC_FAR MSC_CDECL ibfind(char MSC_FAR *bdname);
extern short MSC_FAR MSC_CDECL ibgts(short handle, short v);
extern short MSC_FAR MSC_CDECL ibist(short handle, short v);
extern short MSC_FAR MSC_CDECL iblines(short handle, unsigned short MSC_FAR *lines);
extern short MSC_FAR MSC_CDECL ibln(short handle, short padval, short sadval, short MSC_FAR *listenflag);
extern short MSC_FAR MSC_CDECL ibloc(short handle);
extern short MSC_FAR MSC_CDECL ibonl(short handle, short v);
extern short MSC_FAR MSC_CDECL ibpad(short handle, short v);
extern short MSC_FAR MSC_CDECL ibpct(short handle);
extern short MSC_FAR MSC_CDECL ibppc(short handle, short v);
extern short MSC_FAR MSC_CDECL ibrd(short handle, char MSC_FAR *buffer, unsigned long cnt);
extern short MSC_FAR MSC_CDECL ibrda(short handle, char MSC_FAR *buffer, unsigned long cnt);
extern short MSC_FAR MSC_CDECL ibrdf(short handle, char MSC_FAR *flname);
extern short MSC_FAR MSC_CDECL ibrpp(short handle, char MSC_FAR *ppr);
extern short MSC_FAR MSC_CDECL ibrsc(short handle, short v);
extern short MSC_FAR MSC_CDECL ibrsp(short handle, char MSC_FAR *spr);
extern short MSC_FAR MSC_CDECL ibrsv(short handle, short v);
extern short MSC_FAR MSC_CDECL ibsad(short handle, short v);
extern short MSC_FAR MSC_CDECL ibsic(short handle);
extern short MSC_FAR MSC_CDECL ibsre(short handle, short v);
extern short MSC_FAR MSC_CDECL ibsrq(void (MSC_FAR *func)(void));
extern short MSC_FAR MSC_CDECL ibstop(short handle);
extern short MSC_FAR MSC_CDECL ibtmo(short handle, short v);
extern short MSC_FAR MSC_CDECL ibtrap(short mask, short mode);
extern short MSC_FAR MSC_CDECL ibtrg(short handle);
extern short MSC_FAR MSC_CDECL ibwait(short handle, unsigned short mask);
extern short MSC_FAR MSC_CDECL ibwrt(short handle, char MSC_FAR *buffer, unsigned long cnt);
extern short MSC_FAR MSC_CDECL ibwrta(short handle, char MSC_FAR *buffer, unsigned long cnt);
extern short MSC_FAR MSC_CDECL ibwrtf(short handle, char MSC_FAR *flname);

extern short MSC_FAR MSC_CDECL ibpoke(short handle, unsigned short option, unsigned short value);
extern short MSC_FAR MSC_CDECL ibdiag(short handle, char MSC_FAR *buffer, unsigned long cnt);
extern short MSC_FAR MSC_CDECL ibxtrc(short handle, char MSC_FAR *buffer, unsigned long cnt);

extern short MSC_FAR MSC_CDECL ibwrtkey(short handle, char MSC_FAR *buffer, unsigned short cnt);
extern short MSC_FAR MSC_CDECL ibrdkey(short handle, char MSC_FAR *buffer, unsigned short cnt);

extern void MSC_FAR MSC_CDECL SendCmds(short boardID, unsigned char MSC_FAR *buf, unsigned long cnt);
extern void MSC_FAR MSC_CDECL SendSetup(short boardID, unsigned short MSC_FAR listen[]);
extern void MSC_FAR MSC_CDECL SendDataBytes(short boardID, unsigned char MSC_FAR *buffer, unsigned long cnt, unsigned short eot_mode);
extern void MSC_FAR MSC_CDECL Send(short boardID, unsigned short listener, unsigned char MSC_FAR *databuf, unsigned long datacnt, unsigned short eotMode);
extern void MSC_FAR MSC_CDECL SendList(short boardID, unsigned short MSC_FAR listeners[], unsigned char MSC_FAR *databuf, unsigned long datacnt, unsigned short eotMode);
extern void MSC_FAR MSC_CDECL ReceiveSetup(short boardID, unsigned short talker);
extern void MSC_FAR MSC_CDECL RcvRespMsg(short boardID, unsigned char MSC_FAR *buffer, unsigned long cnt, unsigned short eotMode);
extern void MSC_FAR MSC_CDECL Receive(short boardID, unsigned short talker, unsigned char MSC_FAR *buffer, unsigned long cnt, unsigned short eotMode);
extern void MSC_FAR MSC_CDECL SendIFC(short boardID);
extern void MSC_FAR MSC_CDECL DevClear(short boardID, unsigned short address);
extern void MSC_FAR MSC_CDECL DevClearList(short boardID, unsigned short MSC_FAR addrlist[]);
extern void MSC_FAR MSC_CDECL EnableLocal(short boardID, unsigned short MSC_FAR laddrs[]);
extern void MSC_FAR MSC_CDECL EnableRemote(short boardID, unsigned short MSC_FAR laddrs[]);
extern void MSC_FAR MSC_CDECL SetRWLS(short boardID, unsigned short MSC_FAR laddrs[]);
extern void MSC_FAR MSC_CDECL SendLLO(short boardID);
extern void MSC_FAR MSC_CDECL PassControl(short boardID, unsigned short talker);
extern void MSC_FAR MSC_CDECL ReadStatusByte(short boardID, unsigned short talker, unsigned short MSC_FAR *result);
extern void MSC_FAR MSC_CDECL Trigger(short boardID, unsigned short laddrs);
extern void MSC_FAR MSC_CDECL TriggerList(short boardID, unsigned short MSC_FAR laddrs[]);
extern void MSC_FAR MSC_CDECL PPollConfig(short boardID, unsigned short laddr, unsigned short dataLine, unsigned short lineSense);
extern void MSC_FAR MSC_CDECL PPollUnconfig(short boardID, unsigned short MSC_FAR laddrs[]);
extern void MSC_FAR MSC_CDECL PPoll(short boardID, unsigned short MSC_FAR *res_ptr);
extern void MSC_FAR MSC_CDECL TestSRQ(short boardID, short MSC_FAR *result);
extern void MSC_FAR MSC_CDECL WaitSRQ(short boardID, short MSC_FAR *result);
extern void MSC_FAR MSC_CDECL ResetSys(short boardID, unsigned short MSC_FAR laddrs[]);
extern void MSC_FAR MSC_CDECL FindRQS(short boardID, unsigned short MSC_FAR taddrs[], unsigned short MSC_FAR *dev_stat);
extern void MSC_FAR MSC_CDECL AllSpoll(short boardID, unsigned short MSC_FAR taddrs[], unsigned short MSC_FAR res[]);
extern void MSC_FAR MSC_CDECL FindLstn(short boardID, unsigned short MSC_FAR pads[], unsigned short MSC_FAR results[], unsigned short limit);
extern void MSC_FAR MSC_CDECL TestSys(short boardID, unsigned short MSC_FAR addrs[], unsigned short MSC_FAR result[]);

extern void MSC_FAR MSC_CDECL GotoMultAddr(short handle,
                                           unsigned short type,
                                           unsigned short (_far _loadds *addrfunc)(),
                                           unsigned short (_far _loadds *spollfunc)());
extern void MSC_FAR MSC_CDECL GenerateREQT(short handle, unsigned short addr);
extern void MSC_FAR MSC_CDECL GenerateREQF(short handle, unsigned short addr);


#ifdef __cplusplus
}
#endif

