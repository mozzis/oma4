/***************************************************************************/
/*  OS2D16M.c                                                              */
/*                                                                         */
/*  copyright (c) 1991, EG&G Instruments Inc.                              */
/*
/  $Header:   J:/logfiles/oma4000/main/os2_d16m.c_v   1.0   13 Feb 1991 08:40:42   irving  $
/  $Log:   J:/logfiles/oma4000/main/os2_d16m.c_v  $
 * 
 *    Rev 1.0   13 Feb 1991 08:40:42   irving
 * Initial revision.
*/
/*                                                                         */
/***************************************************************************/
  

/**************************************************************************/
/*                                                                        */
/*    Functions to access physical memory under OS/2 with the oma4drv.sys */
/*    loaded                                                              */
/*                                                                        */
/**************************************************************************/
#define PROT
#define INCL_DOSMEMMGR
#define INCL_DOSDEVICES
#include <OS2.H>
#include <dos.h>
#include "oma4drv.h"

#include "handy.h"
#include "omaerror.h"

HFILE OMA4DevHandle = NIL;
PSZ OMA4DevName = "OMA4DV0$";

int DevIOCTL( unsigned function, PVOID data, PVOID parm )
{
   int IOCTLResult;

   IOCTLResult = DosDevIOCtl( data, parm, function, OMA4CNTRL,
      OMA4DevHandle);

   if (IOCTLResult != 0)
      error( ERROR_DEV_IOCTL );

   return IOCTLResult;
}
   
PVOID OS2SegAbsolute( PVOID Address, USHORT SegSize)
{
   USHORT OpenResult;
   USHORT Action;

   if (OMA4DevHandle == NIL)
   {
      OpenResult = DosOpen(OMA4DevName,
                           &OMA4DevHandle,
                           &Action,
                           0l,              /* "primary allocation" of zero */
                           (USHORT) 0,      /* Open as normal file */
                           (USHORT) 0x01,   /* open if there, fail if not */
                           (USHORT) 0x2042, /* Errors to me, Inheritable,
                                              Deny None, Read-Write */
                           0l );            /* Reserved */
      if (OpenResult != 0)
      {
         error( ERROR_DEV_OPEN, OMA4DevName );
         return NULL;
      }
   }

   if ( DevIOCTL( GET_SEG_ABSOLUTE, &Address, &SegSize ) )
      return NULL;
   return Address;   
}

ERROR_CATEGORY OS2SegCancel( PVOID Address )
{
   ULONG Temp;
   USHORT Segment;

   Segment = FP_SEG( Address );
   return DevIOCTL( DEL_SEG_ABSOLUTE, &Segment, &Temp );
}

void far * cdecl _loadds far D16SegAbsolute (long Address, USHORT Size)
{
   return OS2SegAbsolute( (PVOID) Address, Size );
}

int cdecl _loadds far D16SegCancel (PVOID Address)
{
   return OS2SegCancel( Address );
}

int cdecl _loadds far D16MemFree (PVOID Address)
{
   return DosFreeSeg( FP_OFF(Address) );
}

void far * cdecl _loadds far D16MemAlloc (USHORT Size)
{
   SEL   sTemp;

   if (DosAllocSeg( Size, &sTemp, 0 ) != 0)
      return NULL;

   return (MAKEP( sTemp, 0 ));
}
