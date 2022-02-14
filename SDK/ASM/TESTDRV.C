#define INCL_DOSMEMMGR
#define INCL_DOSDEVICES
#include "os2.h"
#include "stdio.h"
#include "conio.h"
#include "malloc.h"
#include "oma4drv.h"

#define OFFSET_DATADDR 0x133
#define OFFSETFAKEFLAG 0x1F6
#define OFFSETCHARBUF2 0x28B
#define OFFSETCRLF 0x393
/*#define DUMPLEN OFFSETFAKEFLAG - OFFSET_DATADDR
/**/
#define DUMPLEN OFFSETCRLF - OFFSET_DATADDR
/**/

PSZ devname = "OMA4DV0$";
USHORT far Action;
HFILE far Shandle;
char far filename[132];
char far * far dummy = "dddd                                                                                        ";
unsigned long temp1;
unsigned long temp2;

/*char far databuf[0x450];
/**/
char far * databuf;
/**/
int testioctl( unsigned, PVOID, PVOID );
int testQIF( void );
int testQDN( void );
int testQOT( void );
int testQDT( void );
int testQMA( void );
int testQMS( void );
int testQPA( void );
int testAD( void );
int testDAD( void );
int GetDataSeg( void );

extern void near MemDump( void far *, unsigned );

main ()
{
   int OpenResult, i;
   int ch=0;
   SEL sel;

   DosAllocSeg( 0x600, (PSEL) &sel, SEG_GIVEABLE | SEG_GETTABLE );
   if (sel == 0)
      {
      printf( "Memory allocate didn't work\n");
      return 1;
      }
   databuf = MAKEP( sel, 0 );
/*   printf( "Oma device number? ");
/*   scanf( "%c", &ch );
/*   devname[6] = ch;
/*   printf( "\n\nOpening OMA4DV%c$\n", devname[6] );
/**/     
   do
      {
      printf("Opening %c%c%c%c%c%c%c%c\n", devname[0],
               devname[1], devname[2], devname[3], devname[4], devname[5],
               devname[6], devname[7] );
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
         printf("%c%c%c%c%c%c%c%c -- Error in Open!\n", devname[0],
               devname[1], devname[2], devname[3], devname[4], devname[5],
               devname[6], devname[7] );
         }         
      else
         {
         printf("%c%c%c%c%c%c%c%c -- Device driver found!\n", devname[0],
               devname[1], devname[2], devname[3], devname[4], devname[5],
               devname[6], devname[7] );
/*         if ( GetDataSeg() ) 
/*            return 1;
/*         getch();
/**/
         if (testAD())
            return 1;
/*         getch();
/**/
/*         if ( GetDataSeg() )
/*            return 1;
/*         getch();
/**/
         if ( testQIF() )
            return 1;
         if ( testQDN() )
            return 1;
         if ( testQOT() )
            return 1;
         if ( testQDT() )
            return 1;
         if ( testQMA() )
            return 1;
         if( testQMS() )
            return 1;
         if ( testQPA() )
            return 1;
/*         if ( GetDataSeg() )
/*            return 1;
/*         getch();
/**/
/*         if( testDAD() )
/*            {
/*            GetDataSeg();
/*            return 1;
/*            }
/*         printf( "Coming up on Close\n" );
/*         getch();
/**/
/*         if ( GetDataSeg() )
/*            return 1;
/*         getch();
/**/
         if ( DosClose( Shandle ) )
            printf( "CLOSE ERROR\n");
         }
/*      devname[6] += 1;  /* increment device */
      if (kbhit())
         ch = getch();
      }
   while (ch != 27);     /* do until esacpe */
   return 0;
/* Process wrapup will close device */
}


int testioctl( unsigned function, PVOID data, PVOID parm )
{
   int IOCTLResult;
   long dummy1, dummy2;

   
   dummy1 =  (long)data;
   dummy2 = (long)parm;
/*   printf( "Function number %XH dataval = %lXx parmval = %lXx\n", function,
/*             dummy1, dummy2 );
/**/
   IOCTLResult = DosDevIOCtl( data, parm, function, OMA4CNTRL, Shandle );

   dummy1 =  (long)data;
   dummy2 = (long)parm;
/*   printf( "On exit dataval = %lXx parmval = %lXx\n", dummy1, dummy2 );
/**/
   
   if (IOCTLResult != 0)
      printf("\nIOCTLResult = %Xh -- Error\n", IOCTLResult);

   return IOCTLResult;
}
   
int testQIF()
{
   char *strtemp;
   int i;

   strtemp = malloc( 132 );
   if ( testioctl( QUERYINIFILE, (PVOID) &filename[0], (PVOID) dummy ) )
      {
      printf( "IOCTL error\n");
      return 1;
      }

   i=0;
   do
      {
      strtemp[i] = filename[i];  /* convert to near pointer */
      i++;
      }
   while ( i<132 && (strtemp[i-1] != (char)0) );
   printf( "Ini file = %s\n", strtemp );
   free( strtemp );
   return 0;   
}


int testQDN()
{

   temp1 = 0;
   if ( testioctl( QUERYDETNUM, (PVOID) &temp1, (PVOID) dummy ) )
      {
      printf( "IOCTL error\n");
      return 1;
      }
   printf( "Number of detectors = %u\n", (unsigned) temp1 );
   return 0;   
}


int testQOT()
{
   temp1 = 0;
   if ( testioctl( QUERYOMATYPE, (PVOID) &temp1, (PVOID) dummy ) )
      {
      printf( "IOCTL error\n");
      return 1;
      }
   temp1 &= 0xFF;     /* mask off unused bits */
   printf( "Interface type = %u\n", (unsigned) temp1 );
   return 0;   
}

int testQDT()
{
   temp1 = 0;
   if ( testioctl( QUERYDETTYPE, (PVOID) &temp1, (PVOID) dummy ) )
      {
      printf( "IOCTL error\n");
      return 1;
      }
   temp1 &= 0xFF;     /* mask off unused bits */
   printf( "Detector type = %u\n", (unsigned) temp1 );
   return 0;   
}


int testQMA()
{
   temp1 = 0L;
   if ( testioctl( QUERYMEMADDR, (PVOID) &temp1, (PVOID) dummy ) )
      {
      printf( "IOCTL error\n");
      return 1;
      }
   printf( "Array virtual addr = %lXx\n", temp1 );
   return 0;   
}


int testQMS()
{
   temp1 = 0;
   if ( testioctl( QUERYMEMSIZE, (PVOID) &temp1, (PVOID) dummy ) )
      {
      printf( "IOCTL error\n");
      return 1;
      }
   printf( "Memory size = %Xx\n", (unsigned) temp1 );
   return 0;   
}


int testQPA()
{
   temp1 = 0;
   if ( testioctl( QUERYPORTADDR, (PVOID) &temp1, (PVOID) dummy ) )
      {
      printf( "IOCTL error\n");
      return 1;
      }
   printf( "Port Address = %Xx\n", (unsigned) temp1 );
   return 0;   
}

int testAD()
{
   temp2 = 0;    /* use all of board's memory */
   if ( testioctl( ACTIVATEDETECTOR, (PVOID) &temp1, (PVOID) &temp2 ) )
      {
      printf( "AD IOCTL error, location = %u\n", (unsigned) temp2);
      return 1;
      }
   printf( "Addr = %lXx Size = %u Kbytes\n", temp1, (unsigned) temp2 );
   return 0;   
}

int testDAD()
{
   *dummy = 0;    /* use all of board's memory */
   if ( testioctl( DEACTIVATEDETECTOR, (PVOID) &temp1, (PVOID) &temp2 ) )
      {
      printf( "DAD IOCTL error, location = %u\n", (unsigned) temp2);
      return 1;
      }
   printf( "DAD OK\n");
   return 0;   
}

int GetDataSeg()
{
   if ( testioctl( QUERYDATASEG, (PVOID) &databuf[0], (PVOID) &temp2 ) )
      {
      printf( "IOCTL error\n");
      return 1;
      }
   else
      MemDump( (PVOID) &databuf[ OFFSET_DATADDR], DUMPLEN );
   return 0;   
}
