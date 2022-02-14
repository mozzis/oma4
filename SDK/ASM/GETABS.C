#define INCL_DOSMEMMGR
#define INCL_DOSDEVICES
#include "os2.h"
#include "dos.h"
#include "stdio.h"
#include "conio.h"
#include "malloc.h"
#include "oma4drv.h"

#define OFFSET_DATADDR 0x133
#define OFFSETFAKEFLAG 0x1F6
#define OFFSETCHARBUF2 0x28B
#define OFFSETCRLF 0x393
#define OFFSET_EOD   0x415
/*#define DUMPLEN OFFSETFAKEFLAG - OFFSET_DATADDR
/**/
/*#define DUMPLEN OFFSETCRLF - OFFSET_DATADDR
/**/
#define DUMPLEN OFFSET_EOD - OFFSET_DATADDR
/**/

PSZ devname = "OMA4DV0$";
USHORT far Action;
HFILE far Shandle;
char far filename[132];
PVOID dummy;
PVOID Address, Temp;
USHORT SegSize;

SHORT DetectorNumber;

int testioctl( unsigned, PVOID, PVOID );
int GetDataSeg( void );
int testGAS( void );
int testDAS( USHORT Segment );

extern void near MemDump( void far *, unsigned );

main ()
{
   int OpenResult;

   OpenResult=
   DosOpen( devname,
 	         &Shandle,
           	&Action,
           	0l,		/* "primary allocation" of zero */
           	(USHORT) 0,	/* Open as normal file */
           	(USHORT) 0x01,	/* open if there, fail if not */
           	(USHORT) 0x2042,	/* Errors to me, Inheritable, Deny None,
                                Read-Write */
           	0l		/* Reserved */
        	);
   if (OpenResult != 0)
      {
      printf("%s -- Error in Open!\n", devname );
      return 1;
      }

   if (testGAS())
      return 1;

   return 0;
}

int testioctl( unsigned function, PVOID data, PVOID parm )
{
   int IOCTLResult;
   long dummy1, dummy2;

   
   dummy1 =  (long)data;
   dummy2 = (long)parm;
   IOCTLResult = DosDevIOCtl( data, parm, function, OMA4CNTRL, Shandle );

   dummy1 =  (long)data;
   dummy2 = (long)parm;
   
   if (IOCTLResult != 0)
      printf("\nIOCTLResult = %Xh -- Error\n", IOCTLResult);

   return IOCTLResult;
}
   
int GetDataSeg()
{
   if ( testioctl( QUERYDATASEG, &Temp, dummy ) )
      {
      printf( "IOCTL error\n");
      return 1;
      }
   else
      MemDump( (PVOID) ((ULONG) Temp + OFFSET_DATADDR), DUMPLEN );
   return 0;   
}

int testGAS()
{
   SegSize = 100;         // segment size
   Address = (PVOID) 0x50000;     // test location at 320K
   printf( "Physical address = %lx\n", Address );
   if ( testioctl( GET_SEG_ABSOLUTE, &Address, &SegSize ) )
   {
      printf( "IOCTL error\n");
      return 1;
   }
   else
   {
      // print out the area using the new virtual address
      printf( "Virtual address = %lx\n", Address );
      MemDump( (PVOID) Address, SegSize );
      if ( testDAS(FP_SEG(Address)) )
         return 1;
   }
      
   return 0;   
}

int testDAS( USHORT Segment )
{
   printf( "deleting selector %x\n", Segment);
   if ( testioctl(DEL_SEG_ABSOLUTE, &Segment, &Temp) )
   {
      printf( "IOCTL error\n");
      return 1;
   }
 //  GetDataSeg();     // print out driver's data area
   return 0;   
}
