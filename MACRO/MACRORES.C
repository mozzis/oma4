/* -----------------------------------------------------------------------
/
/  macrores.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/macro/macrores.c_v   0.34   06 Jul 1992 12:51:36   maynard  $
/  $Log:   J:/logfiles/oma4000/macro/macrores.c_v  $
/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // strlen()
#include <conio.h>      // kbhit()
#include <io.h>         // chsize()
#include <math.h>       // sqrt()
#include <malloc.h>     // malloc
#include <process.h>
#include <bios.h>
#include <sys/timeb.h>

#include "macrores.h"
#include "macnres2.h"
#include "macruntm.h"
#include "macnres.h"
#include "crvheadr.h"    // DOUBLETYPE
#include "mathops.h"    // MinErrorVal
#include "gpibcom.h"
#include "oma4driv.h"
#include "detsetup.h"
#include "macrecor.h"
#include "config.h"    // detector_index
#include "macrofrm.h"  // pBufLen
#include "mdatstak.h"
#include "macres2.h"
#include "maccomp.h"
#include "macparse.h"
#include "syserror.h"
#include "omaerror.h"
#include "curvedir.h"  // MainCurveDir
#include "graphops.h"  // MacEnterGraph()
#include "live.h"      // MacGoLive()
#include "omazoom.h"   // MacRestoreZoom()
#include "autoscal.h"  // MacAutoScale()
#include "cursor.h"    // ActiveWindow
#include "tagcurve.h"  // MacExpandTagged()
#include "autopeak.h"  // MacDrawPeaks()
#include "omaform.h"   // isFormGraphWindow()
#include "oma4000.h"   // closeGSSWorkstation()
#include "baslnsub.h"  // macDoAddKnot()
#include "forms.h"     // restore_screen_area()
#include <decl.h>      // ERR
#include "spgraph.h"   // M1235 functions
#include "dosshell.h"  // ShellCmd()
#include "statform.h"  // DoStats();
#include "curvbufr.h"  // clearAllCurveBufs()
#include "curvdraw.h"
#include "ycalib.h"
#include "runforms.h"  // SetFilter()
#include "di_util.h"
#include "multi.h"
#include "cmdtbl.h"

#ifdef USE_D16M      
   #ifndef DOS16_INCLUDED
      #include <dos16.h>
   #endif
#endif

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

#define RX_READY 256
#define TX_READY 8192
#define LINE_STAT 5
#define MODM_STAT 6

USHORT PortAddr[4];

// the CommandList[] array declaration and initialization
// has been move to the END of THIS file, after the definition of most
// of the functions which it points to.  Therefore, no function prototypes
// are needed.

void MacBadParam(int param)
{
  error(ERROR_BAD_PARAM_TYPE, param);
  SetErrorFlag();
}


PRIVATE void FillCSCurve() { FillAndPushCurve(CURVESET_CS); }

PRIVATE void FillDCCurve() { FillAndPushCurve(CURVESET_DC); }

PRIVATE void FillMCCurve() { FillAndPushCurve(CURVESET_MC); }

PRIVATE void Plus(void)     { RunOperator(PLUS)   ; }

PRIVATE void Minus(void)    { RunOperator(SUB)    ; }

PRIVATE void Multiply(void) { RunOperator(MULTIPLY); }

PRIVATE void Divide(void)   { RunOperator(DIVIDE) ; }

PRIVATE void IntDiv(void)   { RunOperator(INTDIV) ; }

PRIVATE void Mod(void)      { RunOperator(MOD)    ; }

PRIVATE void Trunc(void)    { RunOperator(TRUNC)  ; }

PRIVATE void Round(void)    { RunOperator(ROUND)  ; }

PRIVATE void MacAbs(void)   { RunOperator(ABS)    ; }

PRIVATE void And(void)      { RunOperator(AND)    ; }

PRIVATE void Or(void)       { RunOperator(OR)     ; }

PRIVATE void Xor(void)      { RunOperator(XOR)    ; }

PRIVATE void Not(void)      { RunOperator(BITNOT) ; }

PRIVATE void Shl(void)      { RunOperator(SHL)    ; }

PRIVATE void Shr(void)      { RunOperator(SHR)    ; }

PRIVATE void MacLog(void)   { RunOperator(LOG)    ; }

PRIVATE void MacLn(void)    { RunOperator(LN)     ; }

PRIVATE void MacExp(void)   { RunOperator(EXP)    ; }

PRIVATE void MacSin(void)   { RunOperator(SIN)    ; }

PRIVATE void MacCos(void)   { RunOperator(COS)    ; }

PRIVATE void MacTan(void)   { RunOperator(TAN)    ; }

PRIVATE void MacAtan(void)  { RunOperator(ATAN)   ; }

PRIVATE void MacAsin(void)  { RunOperator(ASIN)   ; }

PRIVATE void MacAcos(void)  { RunOperator(ACOS)   ; }

PRIVATE void MacAtan2(void) { RunOperator(ATAN2)  ; }

PRIVATE void MacAbsorb(void) { RunOperator(ABSORB)  ; }

PRIVATE void MacStrlen(void)
{
  char * strptr;
  int len;

  if ((DataStackPeek(0) & ~(POINTER_TO)) != TYPE_STRING)
    {
    MacBadParam(1);
    }
  else
    {
    PopFromDataStack(&strptr, (POINTER_TO | TYPE_STRING));
    len = strlen(strptr);
    PushToDataStack(&len, TYPE_INTEGER, FALSE);
    }
}

PRIVATE void RatFit(void)   { }

PRIVATE void PolyFit(void)  { }

PRIVATE void PolyDerv(void) { }

void MacStringToFloat(void)
{
  char * String;
  double FloatVal;

  if (PopFromDataStack(&String, TYPE_STRING | POINTER_TO) != TYPE_STRING)
    {
    MacBadParam(1);
    return;
    }
  FloatVal = atof(String);
  PushToDataStack(&FloatVal, TYPE_REAL, FALSE);
}

/***************************************************************************/
/* In order to make this a function, I have to be able to push a NEW       */
/* string onto the macro data stack.  The macro data stack makes it        */
/* virtually impossible to do this.  It is easy enough to cause the        */
/* PushToDataStack routine to calloc() the required storage, but I can't   */
/* see a way to make both PopFromDataStack and PopScalarFromDataStack      */
/* deallocate the storage without affecting a lot of the rest of the       */
/* language.  Therefore, I will create a static buffer here.  This means   */
/* that only one call to FTOS can be made per program statement; something */
/* like write(FTOS(100.1), FTOS(200.1)) won't work (will print the same    */
/* output string twice).                                                   */
/***************************************************************************/

static char floatstring[32];

void MacFloatToString(void)
{
  double FloatVal;

  PopScalarFromDataStack(&FloatVal, TYPE_REAL);
  gcvt(FloatVal, 23, floatstring);
  PushToDataStack(floatstring, TYPE_STRING, TRUE); /* ptr to static on stak!*/
}

PRIVATE void AvgStdDev(void)   
{
  USHORT CurveCount, FirstPoint, PointCount;
  CURVE_REF StartCurve;
  CURVE_REF *pCurve;
  SHORT BadParam = 0;
  STAT_STRUCT Stats;
  PDOUBLE Average, Dev;
  OP_BLOCK Src;

  if (PopFromDataStack(&Dev, TYPE_REAL | POINTER_TO) != TYPE_REAL)
    BadParam = 6;
  else if (PopFromDataStack(&Average, TYPE_REAL | POINTER_TO) != TYPE_REAL)
    BadParam = 5;
  else if (! PopScalarFromDataStack(&PointCount, TYPE_WORD))
    BadParam = 4;
  else if (! PopScalarFromDataStack(&FirstPoint, TYPE_WORD))
    BadParam = 3;
  else if (! PopScalarFromDataStack(&CurveCount, TYPE_WORD))
    BadParam = 2;
  else
    {
    SHORT Type = PopFromDataStack(&pCurve, TYPE_CURVE | POINTER_TO);
    StartCurve = * pCurve;
    if ((Type != TYPE_CURVE) || (StartCurve.ReferenceType != CLASS_CURVE))
      BadParam = 1;
    }

  if (BadParam)
    {
    MacBadParam(BadParam);
    return;
    }

  Src.BlkIndex = StartCurve.CurveSetIndex;
  Src.Start = StartCurve.CurveIndex;
  Src.Count = CurveCount;
  Stats.DoCArea = FALSE;
  DoStats(&Src, FirstPoint, PointCount, &Stats);

  *Average = Stats.Avg;
  *Dev = Stats.Dev;
}

PRIVATE void GMax()
{
   PushToDataStack(&MaxErrorVal, TYPE_REAL, FALSE);
}

PRIVATE void SMax()
{
   if (! PopScalarFromDataStack(&MaxErrorVal, TYPE_REAL))
   {
    MacBadParam(1);
   }
}

PRIVATE void GMin()
{
   PushToDataStack(&MinErrorVal, TYPE_REAL, FALSE);
}

PRIVATE void SMin()
{
   if (! PopScalarFromDataStack(&MinErrorVal, TYPE_REAL))
   {
    MacBadParam(1);
   }
}

PRIVATE void EqualTo(void)       { RunOperator(EQUALTO)     ; }

PRIVATE void NotEqualTo(void)    { RunOperator(NOTEQUALTO)  ; }

PRIVATE void LessThan(void)      { RunOperator(LESSTHAN)    ; }
                                 
PRIVATE void GreaterThan(void)   { RunOperator(GREATERTHAN) ; }

PRIVATE void LessThanEq(void)    { RunOperator(LESSTHANEQ)  ; }

PRIVATE void GreaterThanEq(void) { RunOperator(GREATERTHANEQ); }

PRIVATE void MacClose(void)
{
   SHORT FileType;
   FILE *hFile;

   FileType = PopFromDataStack(&hFile, TYPE_FILE);
   if ((FileType != TYPE_FILE) && (FileType != TYPE_TEXTFILE))
   {
    MacBadParam(1);
    return;
   }

   if (fclose(hFile))
   {
      error(ERROR_MAC_CLOSE_FAIL);
      SetErrorFlag();
   }
}

PRIVATE void Reset(void)
{
   FILE *hFile;
   SHORT FileType;

   FileType = PopFromDataStack(&hFile, TYPE_FILE);
   if ((FileType != TYPE_FILE) && (FileType != TYPE_TEXTFILE))
   {
    MacBadParam(1);
   }
   else
      rewind(hFile);
}

PRIVATE void Rewrite(void)
{
   FILE *hFile;
   SHORT FileType;
   int hTemp;

   FileType = PopFromDataStack(&hFile, TYPE_FILE);
   if ((FileType != TYPE_FILE) && (FileType != TYPE_TEXTFILE))
   {
    MacBadParam(1);
      return;
   }

   hTemp = fileno(hFile);

   if (chsize(hTemp, 0L) == -1)
   {
      error(ERROR_MAC_WRITE_FAIL);
      SetErrorFlag();
   }
}

PRIVATE void Seek(void)
{
   SHORT BadParam = 0;
   LONG  ByteOffset;
   FILE *hFile;
   SHORT FileType;

   if (! PopScalarFromDataStack(&ByteOffset, TYPE_LONG_INTEGER))
      BadParam = 2;
   else
   {
      FileType = PopFromDataStack(&hFile, TYPE_FILE);
      if ((FileType != TYPE_FILE) && (FileType != TYPE_TEXTFILE))
         BadParam = 1;
   }

   if (BadParam)
   {
    MacBadParam(1);
    return;
   }

   if (fseek(hFile, ByteOffset, SEEK_SET))
   {
      error(ERROR_MAC_FILE_SEEK_FAIL);
      SetErrorFlag();
   }
}

PRIVATE void MacRead(void)  { ReadProc(FALSE); }

PRIVATE void ReadLn(void)   { ReadProc(TRUE) ; }

PRIVATE void MacWrite(void) { WriteProc(FALSE); }

PRIVATE void WriteLn(void)  { WriteProc(TRUE); }

PRIVATE void EndOfLine()
{
  int FileType;
  FILE *hFile = stdin;
  SHORT Chr;
  BOOLEAN AtEnd = FALSE;
  ERR_OMA err = ERROR_NONE;

  FileType = DataStackPeek(0) & ~POINTER_TO;
  if ((FileType == TYPE_FILE) || (FileType == TYPE_TEXTFILE))
    {
    PopFromDataStack(&hFile, TYPE_FILE);
    }
  else
    {
    MacBadParam(1);
    return;
    }

  Chr = fgetc(hFile);
  if (Chr == EOF)
    {
    /* check to see if real error or just end of file */
    if (! feof(hFile))
      err = ERROR_MAC_READ_FAIL;
    else
      {
      AtEnd = TRUE;
      Chr = 0x0A;
      PushToDataStack(&AtEnd, TYPE_BOOLEAN, FALSE);
      return;
      }
    }
  else if (ungetc(Chr, hFile) == EOF)
    err = ERROR_MAC_WRITE_FAIL;

  if (err)
    {
    error(err);
    SetErrorFlag();
    return;
    }

  if (Chr == 0x0A)
    AtEnd = TRUE;

  PushToDataStack(&AtEnd, TYPE_BOOLEAN, FALSE);
}

PRIVATE void EndOfFile()
{
   int FileType;
   FILE *hFile = stdin;
   BOOLEAN AtEnd = FALSE;

   FileType = DataStackPeek(0) & ~POINTER_TO;
   if ((FileType == TYPE_FILE) || (FileType == TYPE_TEXTFILE))
    {
    PopFromDataStack(&hFile, TYPE_FILE);
    AtEnd = feof(hFile);
    PushToDataStack(&AtEnd, TYPE_BOOLEAN, FALSE);
    }
   else
     MacBadParam(1);
}

PRIVATE void KeyPressed()
{
   BOOLEAN KeyHit;

   KeyHit = kbhit();
   PushToDataStack(&KeyHit, TYPE_BOOLEAN, FALSE);
}

PRIVATE void GMem(void)
{
   CHAR *Addr;

   if (! PopScalarFromDataStack(&Addr, TYPE_LONG_INTEGER))
   {
    MacBadParam(1);
   }

#ifdef USE_D16M     
   /* 32 bit real to protected mode XLate */
   Addr = D16SegAbsolute((ULONG) Addr, 1);
#endif
   PushToDataStack(Addr, TYPE_BYTE, FALSE);

#ifdef USE_D16M     
   D16SegCancel(Addr);
#endif
}

PRIVATE void GMemW(void)
{
   USHORT *Addr;

   if (! PopScalarFromDataStack(&Addr, TYPE_LONG_INTEGER))
   {
    MacBadParam(1);
   }

#ifdef USE_D16M   
   /* 32 bit real to protected mode XLate */
   Addr = D16SegAbsolute((ULONG) Addr, 2);
#endif
   PushToDataStack(Addr, TYPE_WORD, FALSE);

#ifdef USE_D16M   
   D16SegCancel(Addr);
#endif
}

PRIVATE void SMem(void)
{
   CHAR *Addr;
   CHAR bTemp;
   SHORT BadParam = 0;

   if (! PopScalarFromDataStack(&bTemp, TYPE_BYTE))
      BadParam = 2;
   else if (! PopScalarFromDataStack(&Addr, TYPE_LONG_INTEGER))
      BadParam = 1;

   if (BadParam)
   {
    MacBadParam(1);
   }

#ifdef USE_D16M   
   /* 32 bit real to protected mode XLate */
   Addr = D16SegAbsolute((ULONG) Addr, 1);
#endif
   *Addr = bTemp;

#ifdef USE_D16M   
   D16SegCancel(Addr);
#endif
}

PRIVATE void SMemW(void)
{
   USHORT *Addr;
   USHORT wTemp;

   if (! PopScalarFromDataStack(&wTemp, TYPE_WORD))
   {
    MacBadParam(2);
   }

   if (! PopScalarFromDataStack(&Addr, TYPE_LONG_INTEGER))
   {
    MacBadParam(1);
   }

#ifdef USE_D16M   
   /* 32 bit real to protected mode XLate */
   Addr = D16SegAbsolute((ULONG) Addr, 2);
#endif
   *Addr = wTemp;

#ifdef USE_D16M   
   D16SegCancel(Addr);
#endif
}

PRIVATE void GPort(void)
{
   USHORT Port;
   SHORT PortVal;

   if (! PopScalarFromDataStack(&Port, TYPE_WORD))
   {
    MacBadParam(1);
   }

   PortVal = inp(Port);
   PushToDataStack(&PortVal, TYPE_BYTE, FALSE);
}

PRIVATE void GPortW(void)
{
   USHORT Port;
   SHORT PortVal;

   if (! PopScalarFromDataStack(&Port, TYPE_WORD))
   {
    MacBadParam(1);
   }

   PortVal = inpw(Port);
   PushToDataStack(&PortVal, TYPE_WORD, FALSE);
}

PRIVATE void SPort(void)
{
   USHORT Port;
   CHAR bTemp;

   if (! PopScalarFromDataStack(&bTemp, TYPE_BYTE))
   {
    MacBadParam(2);
   }

   if (! PopScalarFromDataStack(&Port, TYPE_WORD))
   {
    MacBadParam(1);
   }

   outp(Port, (SHORT) bTemp);
}

PRIVATE void SPortW(void)
{
   USHORT Port;
   USHORT wTemp;

   if (! PopScalarFromDataStack(&wTemp, TYPE_WORD))
   {
    MacBadParam(2);
   }

   if (! PopScalarFromDataStack(&Port, TYPE_WORD))
   {
    MacBadParam(1);
   }

   outpw(Port, wTemp);
}

/* allow macro program to delay for given number of milliseconds */
PRIVATE void MacWait(void)
{
  ULONG DelayTime;

  if (! PopScalarFromDataStack(&DelayTime, TYPE_LONG_INTEGER))
    MacBadParam(1);

  SysWait(DelayTime);
}

PRIVATE void DNumBytes(void) { IntPushDetVal(DC_BYTES); }

PRIVATE void DNumpts(void) { IntPushDetVal(DC_POINTS); }

PRIVATE void BinMemOp(int op)
{
  SHORT mem1, mem2, mem3, i;
  float memsize;
  ULONG far *pmem1, *pmem2;

  if (! PopScalarFromDataStack(&mem3, TYPE_WORD))
    MacBadParam(3);

  else if (! PopScalarFromDataStack(&mem2, TYPE_WORD))
    MacBadParam(2);

  else if (! PopScalarFromDataStack(&mem1, TYPE_WORD))
    MacBadParam(1);

  GetParam(DC_BYTES, &memsize);

  if ((pmem1 = (ULONG far *)malloc((SHORT)memsize)) == NULL)
    {
    error(ERROR_ALLOC_MEM);
    SetErrorFlag();
    }
  else
    {
    if ((pmem2 = (ULONG far *)malloc((SHORT)memsize)) == NULL)
      {
      free(pmem1);
      error(ERROR_ALLOC_MEM);
      SetErrorFlag();
      }
    else
      {
      ReadCurveFromMem(pmem1, (SHORT)memsize, mem1);
      ReadCurveFromMem(pmem2, (SHORT)memsize, mem2);

      if (op)
        {
        for (i = 0;i < (SHORT)(memsize / sizeof(long)); i++)
          pmem1[i] += pmem2[i];
        }
      else
        {
        for (i = 0;i < (SHORT)(memsize / sizeof(long)); i++)
          pmem1[i] -= pmem2[i];
        }
      }
    WriteCurveToMem(pmem1, (SHORT)memsize, mem3);
    free(pmem1);
    free(pmem2);
    }
}

PRIVATE void DAddMem(void) { BinMemOp(TRUE); }

PRIVATE void DGADP(void)   { IntPushDetVal(DC_ADPREC); }

PRIVATE void D_CL(void)    { IntPushDetVal(DC_COOLLOCK); }

PRIVATE void DClr(void)    { IntSetSingleDetVal(DC_CLR); }

PRIVATE void DClrAll(void)
{
  if (ClearAllMems())
    SetErrorFlag();
}

PRIVATE void DCrM(void)
{
  if (ClearRunMems())
    SetErrorFlag();
}

PRIVATE void DGDA(void)    { IntPushDetVal(DC_DAPROG); }

PRIVATE void DSDA(void)    { IntSetSingleDetVal(DC_DAPROG); }

PRIVATE void LoadDAD(void) /* function not needed in oma4000 */
{
}

PRIVATE void DGDM(void)    { IntPushDetVal(DC_DMODEL); }

PRIVATE void DGDT(void)    { IntPushDetVal(DC_DTEMP); }
PRIVATE void DSDT(void)    { IntSetSingleDetVal(DC_DTEMP); }

PRIVATE void DGErr(void)   { IntPushDetVal(DC_DERROR); }

PRIVATE void DGET(void)    { FltPushDetVal(DC_ET); }
PRIVATE void DSET(void)    { FltSetSingleDetVal(DC_ET); }

PRIVATE void DGFreq(void)  { IntPushDetVal(DC_FREQ); }
PRIVATE void DSFreq(void)  { IntSetSingleDetVal(DC_FREQ); }

PRIVATE void DGI(void)     { IntPushDetVal(DC_I); }
PRIVATE void DSI(void)     { IntSetSingleDetVal(DC_I); }

PRIVATE void DGID(void)    { IntPushDetVal(DC_ID); }

PRIVATE void DGJ(void)     { IntPushDetVal(DC_J); }
PRIVATE void DSJ(void)     { IntSetSingleDetVal(DC_J); }

PRIVATE void DGK(void)     { IntPushDetVal(DC_K); }
PRIVATE void DSK(void)     { IntSetSingleDetVal(DC_K); }

PRIVATE void DGL(void)     { IntPushDetVal(DC_L); }
PRIVATE void DSL(void)     { IntSetSingleDetVal(DC_L); }

PRIVATE void DStoreDAD(void) { /* not used for now */ }

PRIVATE void DGMaxMem(void)  { IntPushDetVal(DC_MAXMEM); }

PRIVATE void DGMem(void)    { IntPushDetVal(DC_MEM); }
PRIVATE void DSMem(void)    { IntSetSingleDetVal(DC_MEM); }

PRIVATE void DGMinET(void)  { FltPushDetVal(DC_MINET); }

PRIVATE void DG_Ptime(void) { IntPushDetVal(DC_PTIME); }
PRIVATE void DS_Ptime(void) { IntSetSingleDetVal(DC_PTIME); }

PRIVATE void DG_Stime(void) { IntPushDetVal(DC_STIME); }
PRIVATE void DS_Stime(void) { IntSetSingleDetVal(DC_STIME); }

PRIVATE void DG_Ftime(void) { FltPushDetVal(DC_FTIME); }

PRIVATE void DGPD(void)     { FltPushDetVal(DC_PDELAY); }
PRIVATE void DSPD(void)     { FltSetSingleDetVal(DC_PDELAY); }

PRIVATE void DGPlsr(void)   { FltPushDetVal(DC_PLSR); }

PRIVATE void DGPW(void)     { FltPushDetVal(DC_PWIDTH); }
PRIVATE void DSPW(void)     { FltSetSingleDetVal(DC_PWIDTH); }

PRIVATE void DRun(void)
{
  SHORT LiveType, LiveFail = FALSE;
  
  if (! PopScalarFromDataStack(&LiveType, TYPE_INTEGER))
    {
    error(ERROR_BAD_PARAM_TYPE, 1);
    SetErrorFlag();
    return;                         
    }

  if(GetCurveSetIndex(LastLiveEntryName, "", 0) == -1)
    {
    float Points, Curves, temp;

    GetParam(DC_POINTS, &Points);
    GetParam(DC_TRACKS, &Curves);
    GetParam(DC_J, &temp);
    Curves *= temp;
    if (SetupAcqCurveBlk((USHORT)Points, (USHORT)Curves) != ERROR_NONE)
      LiveFail = TRUE;
    }

  if (!LiveFail)
    LiveFail = set_int_detect_param(DC_RUN, LiveType);

  PushToDataStack(&LiveFail, TYPE_INTEGER, FALSE);

  if (!LiveType)
    UpdateCursorMode(CURSORMODE_ACTIVE);
}

PRIVATE void DStop(void)    { IntSetSingleDetVal(DC_STOP); }

PRIVATE void DGSameET(void) { IntPushDetVal(DC_SAMEET); }
PRIVATE void DSSameET(void) { IntSetSingleDetVal(DC_SAMEET); }

PRIVATE void DSShutter(void) {  }

PRIVATE void DSub(void)     { BinMemOp(FALSE); }

PRIVATE void DGSync(void)   { IntPushDetVal(DC_CONTROL); }
PRIVATE void DSSync(void)   { IntSetSingleDetVal(DC_CONTROL); }

PRIVATE void DGVer(void)    { FltPushDetVal(DC_VER); }

PRIVATE void DG_Active(void) { IntPushDetVal(DC_ACTIVE); }

PRIVATE void DGActiveX(void)  { IntPushDetVal(DC_ACTIVEX); }
PRIVATE void DSActiveX(void)  { IntSetSingleDetVal(DC_ACTIVEX); }

PRIVATE void DGActiveY(void)  { IntPushDetVal(DC_ACTIVEY); }
PRIVATE void DSActiveY(void)  { IntSetSingleDetVal(DC_ACTIVEY); }

PRIVATE void DGCoolOnOff(void) { IntPushDetVal(DC_COOLONOFF); }
PRIVATE void DSCoolOnOff(void) { IntSetSingleDetVal(DC_COOLONOFF); }

PRIVATE void DGDeltaX(void)   { IntPushDetVal(DC_DELTAX); }
PRIVATE void DSDeltaX(void)   { IntSetSingleDetVal(DC_DELTAX); }

PRIVATE void DGDeltaY(void)   { IntPushDetVal(DC_DELTAY); }
PRIVATE void DSDeltaY(void)   { IntSetSingleDetVal(DC_DELTAY); }

PRIVATE void DGH(void)        { IntPushDetVal(DC_H); }
PRIVATE void DSH(void)        { IntSetSingleDetVal(DC_H); }

PRIVATE void DGScanMode(void) { IntPushDetVal(DC_SHFTMODE); }
PRIVATE void DSScanMode(void) { IntSetSingleDetVal(DC_SHFTMODE); }

PRIVATE void DGShutMode(void) { IntPushDetVal(DC_SHUTMODE); }
PRIVATE void DSShutMode(void) { IntSetSingleDetVal(DC_SHUTMODE); }

PRIVATE void DGSCITC(void)    { IntPushDetVal(DC_SCITC); }
PRIVATE void DSSCITC(void)    { IntSetSingleDetVal(DC_SCITC); }

PRIVATE void DGSCmp(void)     { FltPushDetVal(DC_SCMP); }

PRIVATE void DGPntMode(void)  { IntPushDetVal(DC_PNTMODE); }
PRIVATE void DSPntMode(void)  { IntSetSingleDetVal(DC_PNTMODE); }

PRIVATE void DGPoint(void)    { IntPushDetVal(DC_POINT); }
PRIVATE void DSPoint(void)    { IntSetSingleDetVal(DC_POINT); }

PRIVATE void DGPoints(void)   { IntPushDetVal(DC_POINTS); }
PRIVATE void DSPoints(void)   { IntSetSingleDetVal(DC_POINTS); }

PRIVATE void DGTrack(void)    { IntPushDetVal(DC_TRACK); }
PRIVATE void DSTrack(void)    { IntSetSingleDetVal(DC_TRACK); }

PRIVATE void DGTracks(void)   { IntPushDetVal(DC_TRACKS); }
PRIVATE void DSTracks(void)   { IntSetSingleDetVal(DC_TRACKS); }

PRIVATE void DGTrkMode(void)  { IntPushDetVal(DC_TRKMODE); }
PRIVATE void DSTrkMode(void)  { IntSetSingleDetVal(DC_TRKMODE); }

PRIVATE void DGWFTC(void)     { IntPushDetVal(DC_WFTC); }
PRIVATE void DSWFTC(void)     { IntSetSingleDetVal(DC_WFTC); }

PRIVATE void DGWFTO(void)     { IntPushDetVal(DC_WFTO); }
PRIVATE void DSWFTO(void)     { IntSetSingleDetVal(DC_WFTO); }

PRIVATE void DGX0(void)       { IntPushDetVal(DC_X0); }
PRIVATE void DSX0(void)       { IntSetSingleDetVal(DC_X0); }

PRIVATE void DGY0(void)       { IntPushDetVal(DC_Y0); }
PRIVATE void DSY0(void)       { IntSetSingleDetVal(DC_Y0); }

PRIVATE void GDetAddress(void) { IntPushDetVal(DC_DETPORT); }
PRIVATE void SDetAddress(void) {  IntSetSingleDetVal(DC_DETPORT); }

PRIVATE void DGAntiBloom(void)  { IntPushDetVal(DC_ANTIBLOOM); }
PRIVATE void DSAntiBloom(void)  { IntSetSingleDetVal(DC_ANTIBLOOM); }

PRIVATE void DGOutputReg(void)  { IntPushDetVal(DC_OUTPUTREG); }
PRIVATE void DSOutputReg(void)  { IntSetSingleDetVal(DC_OUTPUTREG); }

PRIVATE void DGOutPIA(void) { IntPushDetVal(DC_OUTPIA);  }
PRIVATE void DSOutPIA(void) { IntSetSingleDetVal(DC_OUTPIA);  }

PRIVATE void DGInPIA(void) { IntPushDetVal(DC_INPIA);  }

PRIVATE void DGPAudio(void) { IntPushDetVal(DC_PAUDIO);  }
PRIVATE void DSPAudio(void) { IntSetSingleDetVal(DC_PAUDIO);  }

PRIVATE void DGPTrigNum(void) { IntPushDetVal(DC_PTRIGNUM);  }
PRIVATE void DSPTrigNum(void) { IntSetSingleDetVal(DC_PTRIGNUM);  }

PRIVATE void DGPTrigSrc(void) { IntPushDetVal(DC_PTRIGSRC);  }
PRIVATE void DSPTrigSrc(void) { IntSetSingleDetVal(DC_PTRIGSRC);  }

PRIVATE void DGPTrigTrsh(void) { IntPushDetVal(DC_PTRIGTRSH);  }
PRIVATE void DSPTrigTrsh(void) { IntSetSingleDetVal(DC_PTRIGTRSH);  }

PRIVATE void DGPDelInc(void) { FltPushDetVal(DC_PDELINC);  }
PRIVATE void DSPDelInc(void) { FltSetSingleDetVal(DC_PDELINC);  }

PRIVATE void DGPDelRange(void) { FltPushDetVal(DC_PDELRANGE);  }
PRIVATE void DSPDelRange(void) { FltSetSingleDetVal(DC_PDELRANGE);  }

PRIVATE void DGPIMode(void) { IntPushDetVal(DC_IMODE);  }
PRIVATE void DSPIMode(void) { IntSetSingleDetVal(DC_IMODE);  }

PRIVATE void DGStreakMode(void) { IntPushDetVal(DC_STREAKMODE);  }
PRIVATE void DSStreakMode(void) { IntSetSingleDetVal(DC_STREAKMODE);  }

PRIVATE void DGExposedRows(void) { IntPushDetVal(DC_EXPROWS);  }
PRIVATE void DSExposedRows(void) { IntSetSingleDetVal(DC_EXPROWS);  }

PRIVATE void DGPreTrigTracks(void) { IntPushDetVal(DC_PRESCAN);  }
PRIVATE void DSPreTrigTracks(void) { IntSetSingleDetVal(DC_PRESCAN);  }

PRIVATE void DGRegions(void) { IntPushDetVal(DC_REGIONS); }
PRIVATE void DSRegions(void) { IntSetSingleDetVal(DC_REGIONS); }

PRIVATE void DGCurRegion(void) { IntPushDetVal(DC_POINT); }
PRIVATE void DSCurRegion(void) { IntSetSingleDetVal(DC_POINT); }

PRIVATE void DGRegSize(void) { IntPushDetVal(DC_REGSIZE); }
PRIVATE void DSRegSize(void) { IntSetSingleDetVal(DC_REGSIZE); }

PRIVATE void DGRegET(void) { FltPushDetVal(DC_REGET);  }
PRIVATE void DSRegET(void) { FltSetSingleDetVal(DC_REGET);  }

PRIVATE void GDetIndex(void)
{
  PushToDataStack(&detector_index, TYPE_INTEGER, FALSE);
}

/*****************************************************************/
PRIVATE void SDetIndex(void)
{
  int TempIndex;
  if (! PopScalarFromDataStack(&TempIndex, TYPE_INTEGER))
    {
    MacBadParam(1);
    }
  else
    {
    detector_index = TempIndex;
    det_setup = &(det_setups[detector_index]);
    }
}

/*****************************************************************/
PRIVATE void RunMacroFile(void)
{
  char * filename;

  if ((DataStackPeek(0) & ~(POINTER_TO)) != TYPE_STRING)
    {
    MacBadParam(1);
    }
  else
    {
    PopFromDataStack(&filename, (POINTER_TO | TYPE_STRING));
    InitDataStack();
    ReadAndParseSourceFile(filename);   /* in macskel.c (macparse.c) */
    }
  if (MacroStatus == PARSE_COMPLETE)
    RunMainProgram();    /* program executed here */

  /* make sure that execution does not resume after this!! */
  IP[1].Class = CLASS_EXECUTE_RETURN;
  IP[2].Class = CLASS_EXECUTE_RETURN;
}


/*****************************************************************/
PRIVATE void GPIBClear(void)
{
  init_gpib();
}

/*****************************************************************/
PRIVATE void GPIBSend(void) 
{
  int TempAddress;
  CHAR *String;
  int ErrorIndex = 0, len = *pBufLen;
  int old_time, old_dma;

  if ((DataStackPeek(0) & ~(POINTER_TO)) != TYPE_STRING)
    ErrorIndex = 2;
  else
    {
    PopFromDataStack(&String, (POINTER_TO | TYPE_STRING));
    if (! PopScalarFromDataStack(&TempAddress, TYPE_INTEGER))
      ErrorIndex = 1;
    }

  if (ErrorIndex)
    {
    MacBadParam(ErrorIndex);
    return;
    }

  old_dma = dma_off ();             /* turn off DMA Channel 1 */
  old_time = set_gpib_timeout(10);  /* set timeout to 300 msec. */

  ErrorIndex = puts_gpib(String, (USHORT *)&len, TempAddress);

  dma_on (old_dma);                 /* DMA Channel 1 */
  set_gpib_timeout(old_time);       /* set timeout to original */

  PushToDataStack(&ErrorIndex, TYPE_INTEGER, FALSE);

}

/*****************************************************************/
PRIVATE void GPIBResponse(void)
{
  int len;
  int TempAddress;
  CHAR *String;
  int ErrorIndex = 0;
  int old_time, old_dma;


  if ((DataStackPeek(0) & ~(POINTER_TO)) != TYPE_STRING)
    ErrorIndex = 2;
  else
    {
    PopFromDataStack(&String, (POINTER_TO | TYPE_STRING));
    if (! PopScalarFromDataStack(&TempAddress, TYPE_INTEGER))
      ErrorIndex = 1;
    }

  if (ErrorIndex)
    {
    MacBadParam(ErrorIndex);
    return;
    }

  len = *pBufLen;

  old_dma = dma_off ();             /* turn off DMA Channel 1 */
  old_time = set_gpib_timeout(10);  /* set timeout to 300 msec. */

  if (gets_gpib(String, (USHORT *)&len, TempAddress))
    {
    int i;
    for (i = 0; i < len; i++) /* len is now # of chars received */
      {
      if (String[i] < 26)
        String[i] = '\0';
      }
    String[len] = '\0';
    }

  dma_on (old_dma);                 /* DMA Channel 1 */
  set_gpib_timeout(old_time);       /* set timeout to original */
}

/*****************************************************************/
PRIVATE void GPIBSerialPoll(void)
{
  int TempAddress;
  UCHAR PollReturn;
  BOOLEAN success;
  int old_time, old_dma;

  if (! PopScalarFromDataStack(&TempAddress, TYPE_INTEGER))
    {
    MacBadParam(1);
    }
  else
    {
    old_dma = dma_off ();             /* turn off DMA Channel 1 */
    old_time = set_gpib_timeout(10);  /* set timeout to 300 msec. */

    success = serial_poll_gpib(TempAddress, &PollReturn);

    dma_on (old_dma);                 /* DMA Channel 1 */
    set_gpib_timeout(old_time);       /* set timeout to original */

    if (!success)
      SetErrorFlag();

    PushToDataStack(&PollReturn, TYPE_BYTE, FALSE);
    }
}

/*****************************************************************/
PRIVATE void StartFresh()
{
  FORGET_ALL();  
}

/*****************************************************************/
PRIVATE void Drop()
{
  RemoveItemsFromDataStack(1);
}

/*****************************************************************/
PRIVATE void MacHelp(void)       
{
  SHORT HelpIndex;

  if (!PopScalarFromDataStack(&HelpIndex, TYPE_INTEGER))
    {
    MacBadParam(1);
    return;
    }

  form_help_from_file(HelpIndex);
}

/*****************************************************************/
PRIVATE void MacZoom()
{
  DOUBLE ZoomPts[4];
  SHORT BadParam = 0;

  // changed to use axis values as zoom box delimiters
  if (! PopScalarFromDataStack(&ZoomPts[3], TYPE_REAL))
    BadParam = 4;
  else if (! PopScalarFromDataStack(&ZoomPts[2], TYPE_REAL))
    BadParam = 3;
  if (! PopScalarFromDataStack(&ZoomPts[1], TYPE_REAL))
    BadParam = 2;
  if (! PopScalarFromDataStack(&ZoomPts[0], TYPE_REAL))
    BadParam = 1;

  if (BadParam)
    {
    MacBadParam(BadParam);
    return;
    }

   MacDoZoom(ZoomPts);
}

/*****************************************************************/
PRIVATE void SActiveWindow()
{
  SaveAreaInfo *SavedArea = NULL;

  if (! PopScalarFromDataStack(&ActiveWindow, TYPE_INTEGER))
    {
    MacBadParam(1);
    return;
    }

  if (ActiveWindow <= 0)
    ActiveWindow = 1;

  ActiveWindow--;      // make 0 based

  if (!isPrevFormGraphWindow())
    SavedArea = save_screen_area(1, 0, 1, screen_columns);

  SelectWindow(0);

  /* note SavedArea pointer also flags whether graph was visible */

  if (SavedArea && !restore_screen_area(SavedArea))
    PutUpPlotBoxes();
}

/*****************************************************************/
PRIVATE void SActivePlotSetup()         
{
  SaveAreaInfo *SavedArea;

  if (!PopScalarFromDataStack(&ActiveWindow, TYPE_INTEGER))
    {
    MacBadParam(1);
    return;
    }

  ActiveWindow--;      // make 0 based
  if (! isPrevFormGraphWindow())
    SavedArea = save_screen_area(1, 0, 1, screen_columns);

  SelectPlotForWindow(0);

  if (!isPrevFormGraphWindow())
    restore_screen_area(SavedArea);
}

/*****************************************************************/
PRIVATE void MacSetThreshold(void)
{
  USHORT BadParam = 0;
  DOUBLE Threshold, CurveNumber;
  SHORT CurveIndex;

  if (! PopScalarFromDataStack(&Threshold, TYPE_REAL))
    BadParam = 3;
  if (! PopScalarFromDataStack(&CurveNumber, TYPE_REAL))
    BadParam = 2;
  if (! PopScalarFromDataStack(&CurveIndex, TYPE_INTEGER))
    BadParam = 1;

  if (BadParam)
    {
    MacBadParam(BadParam);
    return;
    }

  SetPeakThreshold(CurveIndex, (FLOAT) CurveNumber, (FLOAT) Threshold);
}

/*****************************************************************/
PRIVATE void MacAddKnot(void)
{
  DOUBLE XVal, YVal;
  SHORT BadParam = 0;

  if (! PopScalarFromDataStack(&YVal, TYPE_REAL))
    BadParam = 2;
  if (! PopScalarFromDataStack(&XVal, TYPE_REAL))
    BadParam = 1;

  if (BadParam)
    {
    MacBadParam(BadParam);
    return;
    }

  macDoAddKnot(XVal, YVal);
}

/*****************************************************************/
PRIVATE void MacMoveCursor(void)
{
  DOUBLE XVal, ZVal;
  SHORT BadParam = 0;

  if (! PopScalarFromDataStack(&XVal, TYPE_REAL))
    BadParam = 2;
  if (! PopScalarFromDataStack(&ZVal, TYPE_REAL))
    BadParam = 1;

  if (BadParam)
    {
    MacBadParam(BadParam);
    return;
    }
  if (moveCursorMacro((float) XVal, (float) ZVal))
    SetErrorFlag();
}

/*****************************************************************/
PRIVATE void MacSend1235Cmd(void)
{
  CHAR *String;

  if ((DataStackPeek(0) & ~(POINTER_TO)) != TYPE_STRING)
    MacBadParam(1);
  else
    {
    PopFromDataStack(&String, (POINTER_TO | TYPE_STRING));
    strcat(String, "\r");
    if (!Output1235CMD(String))
      {
      error(ERROR_1235_FAIL, String);
      SetErrorFlag();
      }
    }
}

/*****************************************************************/
PRIVATE void MacGet1235Response(void)
{
  int len;
  CHAR *String;
  int ErrorIndex = 0;

  if (! PopScalarFromDataStack(&len, TYPE_INTEGER))
    ErrorIndex = 2;
  else if ((DataStackPeek(0) & ~(POINTER_TO)) != TYPE_STRING)
    ErrorIndex = 1;
  else
    PopFromDataStack(&String, (POINTER_TO | TYPE_STRING));

  if (ErrorIndex)
    MacBadParam(ErrorIndex);
  else
    {
    if (!Get1235Response(String, len))
      {
      error(ERROR_1235_NOANS);
      SetErrorFlag();
      }
    }
}

/*****************************************************************/
PRIVATE void MacReset1235(void)
{
  if (Reset1235() != FIELD_VALIDATE_SUCCESS)
    SetErrorFlag();
}

/*****************************************************************/
PRIVATE void Mac1235SelSpec(void)
{
  int model;

  if (! PopScalarFromDataStack(&model, TYPE_INTEGER))
    {
    MacBadParam(1);
    }
  else
    {
    if (!MacChooseSpectrograph(model))
      {
      error(ERROR_1235_NOANS);
      SetErrorFlag();
      }
    }
}

/*****************************************************************/
PRIVATE void MacResetOffset(void)
{
  double wavelen;
  int channel;

  if(!PopScalarFromDataStack(&wavelen, TYPE_REAL))
    MacBadParam(2);
  else if(!PopScalarFromDataStack(&channel, TYPE_INTEGER))
    MacBadParam(1);
  else if (!MacOnePtResetOffset(channel, (float)wavelen))
    SetErrorFlag();
}

/*****************************************************************/
PRIVATE void Mac1235Go(void)
{
  if (!MacSet1235())
    SetErrorFlag();
}

/*****************************************************************/
PRIVATE void Mac1235WL(void)
{
  double wavelen;

  if(!PopScalarFromDataStack(&wavelen, TYPE_REAL))
    MacBadParam(1);
  else
    {
    if (!MacSetWV((float)wavelen))
      SetErrorFlag();
    }
}

/*****************************************************************/
PRIVATE void MacChangeGrating(void)
{
  int grating;

  if(!PopScalarFromDataStack(&grating, TYPE_INTEGER))
    MacBadParam(1);
  else
    {
    if (ChangeGrating(grating) != FIELD_VALIDATE_SUCCESS)
      SetErrorFlag();
    }
}

/*****************************************************************/
PRIVATE void MacStore1235(void)
{
  if (!Store1235Settings())
    SetErrorFlag();
}

/*****************************************************************/
PRIVATE void MacExecuteDispersion(void)
{
  int IsBlueToRed;

  if(!PopScalarFromDataStack(&IsBlueToRed, TYPE_INTEGER))
    MacBadParam(1);
  else if (MacChangeDispersion(IsBlueToRed))
    SetErrorFlag();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE void MacShellCmd(void)
{
  char * cmd_string;

  if ((DataStackPeek(0) & ~(POINTER_TO)) != TYPE_STRING)
    MacBadParam(1);
  else
    {
    PopFromDataStack(&cmd_string, (POINTER_TO | TYPE_STRING));
    if (ShellCmd(cmd_string) == -1)
      SetErrorFlag();
    }
}

PRIVATE void MacLoadLampFile(void)
{
  char * Name;

  if ((DataStackPeek(0) & ~(POINTER_TO)) != TYPE_STRING)
    MacBadParam(1);
  else
    {
    PopFromDataStack(&Name, (POINTER_TO | TYPE_STRING));
    if (!LoadLampFile(Name))
      SetErrorFlag();
    }
}

PRIVATE void MacSaveLampFile(void)
{
  char * Name;

  if ((DataStackPeek(0) & ~(POINTER_TO)) != TYPE_STRING)
    MacBadParam(1);
  else
    {
    PopFromDataStack(&Name, (POINTER_TO | TYPE_STRING));
    if (!SaveLampFile(Name))
      SetErrorFlag();
    }
}

PRIVATE void MacAddYcalibPair(void)
{
  DOUBLE wavelen, rel_intens;

  if(!PopScalarFromDataStack(&rel_intens, TYPE_REAL))
    MacBadParam(2);
  else if(!PopScalarFromDataStack(&wavelen, TYPE_REAL))
    MacBadParam(1);
  else
    {
    if (!AddYcalibPair((FLOAT)wavelen, (FLOAT)rel_intens))
      SetErrorFlag();
    }
}

PRIVATE void MacDelYcalibPair(void)
{
  int index;

  if(!PopScalarFromDataStack(&index, TYPE_INTEGER))
    MacBadParam(1);
  else
    {
    if (!DeleteYCalibPair(index))
      SetErrorFlag();
    }
}

PRIVATE void MacDeleteLampData(void)
{
  if (!DeleteLampData())
    SetErrorFlag();
}

PRIVATE void MacGenCorrexCurve(void)
{
  CHAR *LampName, *CorrName;

  if ((DataStackPeek(0) & ~(POINTER_TO)) != TYPE_STRING)
    MacBadParam(2);
  else
    {
    PopFromDataStack(&CorrName, (POINTER_TO | TYPE_STRING));
    if ((DataStackPeek(0) & ~(POINTER_TO)) != TYPE_STRING)
      MacBadParam(1);
    else
      {
      PopFromDataStack(&LampName, (POINTER_TO | TYPE_STRING));
      if (!GenCorrexCurve(LampName, CorrName))
        SetErrorFlag();
      }
    }
}

PRIVATE void MacSetFilter(void)
{
  CHAR *RefName;
  SHORT Mode;

  if ((DataStackPeek(0) & ~(POINTER_TO)) != TYPE_STRING)
    MacBadParam(2);
  else
    {
    PopFromDataStack(&RefName, (POINTER_TO | TYPE_STRING));
    if(!PopScalarFromDataStack(&Mode, TYPE_INTEGER))
      MacBadParam(1);
    else if (!SetFilter(Mode, RefName))
      SetErrorFlag();
    }
}

/* docs for bios_serialcom:

service:_COM_INIT, _COM_SEND, _COM_RECEIVE, _COM_STATUS,

data for init:
        _COM_CHR7,       2       _COM_110,   0
        _COM_CHR8,       3       _COM_150,   32	
                                 _COM_300,   64	
        _COM_STOP1,      0       _COM_600,   96	
        _COM_STOP2,      4       _COM_1200,  128
                                 _COM_2400,  160
        _COM_NOPARITY,   0	     _COM_4800,  192
        _COM_EVENPARITY, 8	     _COM_9600   224
        _COM_ODDPARITY,  24

high order byte of return for status, send or receive:
        _COM_TO     0x8000  (15)    timeout
        _COM_XMERR  0x6000  (14+13) transmit shift register empty
        _COM_BRK    0x1000  (12)    break detected
        _COM_FERR   0x0800  (11)    framing error
        _COM_PERR   0x0400  (10)    parity error
        _COM_OVR    0x0200  (09)    receiver overrun error
        _COM_DRDY   0x0100  (08)    data ready
low order byte or return for status:
        _COM_RCL    0x0080  (7)     Receive-line signal detected
        _COM_RD     0x0040  (6)     Ring indicator
        _COM_DSR    0x0020  (5)     Data-set ready
        _COM_CTS    0x0010  (4)     Clear to send
        _COM_RLD    0x0008  (3)     Change in receive-line signal detected
        _COM_TRD    0x0004  (2)     Trailing-edge ring indicator
        _COM_DSRD   0x0002  (1)     Change in data-set ready status
        _COM_CTSD   0x0001  (0)     Change in clear-to-send status
*/


/*********************************************************************
  Get the I/O port addresses of the available COM ports into a
  local array, so a selector won't be needed to address them later.
  Done by main() when application starts up.
*********************************************************************/

void InitComAddresses(void)
{
  USHORT *BData, i;

#ifdef USE_D16M
    BData = D16SegAbsolute(0x0400, 0);
#else
    BData = (USHORT *)0x0400;
#endif

    for (i = 0; i < 3; i++)
      PortAddr[i] = BData[i];
    if (BData[3] == 0)
      BData[3] = BData[2];

#ifdef USE_D16M
    D16SegCancel(BData);
#endif
}

/* syntax STATUS := COM_INIT(PORT, BAUD, PARITY, WORDSZ, STPBITS) */
/******************************************************************/
PRIVATE void MacComInit(void)
{
  USHORT Baud, Parity, WordSz, Bits, Config = 0, Port;

  if(!PopScalarFromDataStack(&Bits, TYPE_INTEGER))
    {
    MacBadParam(5);
    return;
    }
  else
    {
    switch (Bits)
      {
      case 1:
        Config |= _COM_STOP1;
      break;
      case 2:
        Config |= _COM_STOP2;
      break;
      default:
        MacBadParam(5);
        return;
      }
    }

  if(!PopScalarFromDataStack(&WordSz, TYPE_INTEGER))
    {
    MacBadParam(4);
    return;
    }
  else
    {
    switch (WordSz)
      {
      case 7:
        Config |= _COM_CHR7;
      break;
      case 8:
        Config |= _COM_CHR8;
      break;
      default:
        MacBadParam(4);
        return;
      }
    }

  if(!PopScalarFromDataStack(&Parity, TYPE_INTEGER))
    {
    MacBadParam(3);
    return;
    }
  else
    {
    switch (Parity)
      {
      case 0:
        Config |= _COM_NOPARITY;
      break;
      case 1:
        Config |= _COM_ODDPARITY;
      break;
      case 2:
        Config |= _COM_EVENPARITY;
      default:
        MacBadParam(3);
        return;
      }
    }

  if(!PopScalarFromDataStack(&Baud, TYPE_INTEGER))
    {
    MacBadParam(2);
    return;
    }
  else
    {
    switch (Baud)
      {
      case 110:
        Config |= _COM_110;
      break;
      case 150:
        Config |= _COM_150;
      break;
      case 300:
        Config |= _COM_300;
      break;
      case 600:
        Config |= _COM_600;
      break;
      case 1200:
        Config |= _COM_1200;
      break;
      case 2400:
        Config |= _COM_2400;
      break;
      case 4800:
        Config |= _COM_4800;
      break;
      case 9600:
        Config |= _COM_9600;
      break;
      default:
        MacBadParam(2);
        return;
      }
    }

  if(!PopScalarFromDataStack(&Port, TYPE_INTEGER))
    MacBadParam(1);
  else
    Config = _bios_serialcom(_COM_INIT, Port-1, Config);

  PushToDataStack(&Config, TYPE_INTEGER, FALSE);

  outp(PortAddr[Port-1]+4, 3); /* Set DTR and RTS registers */
}

/* syntax Status := COM_STATUS(PORT) */
/******************************************************************/
PRIVATE void MacComStat(void)
{
  USHORT Port, Status = 0;
  
  if(!PopScalarFromDataStack(&Port, TYPE_INTEGER))
    MacBadParam(1);
  else
    {
    Port -= 1;
    Status = inp(PortAddr[Port]+LINE_STAT) << 8;
    Status |= inp(PortAddr[Port]+MODM_STAT);
    PushToDataStack(&Status, TYPE_INTEGER, FALSE);
    }
}

/* syntax: STATUS := COM_OUT(PORT, DATA); */
/******************************************************************/
PRIVATE void MacComOut(void)
{
  UCHAR  OutByte;
  USHORT Port, Status;

  if(!PopScalarFromDataStack(&OutByte, TYPE_BYTE))
    MacBadParam(2);
  else if(!PopScalarFromDataStack(&Port, TYPE_INTEGER))
    MacBadParam(1);
  else
    {
    Port -= 1;
    outp(PortAddr[Port], (USHORT)OutByte);
    Status = inp(PortAddr[Port]+LINE_STAT) << 8;
    Status |= inp(PortAddr[Port]+MODM_STAT);
    PushToDataStack(&Status, TYPE_INTEGER, FALSE);
    }
}

/* syntax: STATUS := COM_IN(PORT, VAR DATA); */
/******************************************************************/
PRIVATE void MacComIn(void)
{
  UCHAR  *Data;
  USHORT Port, Status;

  if (PopFromDataStack(&Data, TYPE_BYTE | POINTER_TO) != TYPE_BYTE)
    MacBadParam(2);
  else if(!PopScalarFromDataStack(&Port, TYPE_INTEGER))
    MacBadParam(1);
  else
    {
    Port -= 1;
    *Data = (UCHAR)inp(PortAddr[Port]);
    Status = inp(PortAddr[Port]+LINE_STAT) << 8;
    Status |= inp(PortAddr[Port]+MODM_STAT);
    PushToDataStack(&Status, TYPE_INTEGER, FALSE);
    }
}

/* syntax: MBUTTON := READ_MOUSE(Xpos, Ypos, CvX, CvY, CvZ); */
/******************************************************************/
PRIVATE void MacReadMouse(void)
{
  FLOAT *CrvX, *CrvY, *CrvZ;
  SHORT Row, Col, *Xpos, *Ypos, MButtons;

  if (PopFromDataStack(&CrvZ, TYPE_REAL4 | POINTER_TO) != TYPE_REAL4)
    MacBadParam(5);

  else if (PopFromDataStack(&CrvY, TYPE_REAL4 | POINTER_TO) != TYPE_REAL4)
    MacBadParam(4);

  else if (PopFromDataStack(&CrvX, TYPE_REAL4 | POINTER_TO) != TYPE_REAL4)
    MacBadParam(3);

  else if (PopFromDataStack(&Ypos, TYPE_INTEGER | POINTER_TO) != TYPE_INTEGER)
    MacBadParam(2);

  else if (PopFromDataStack(&Xpos, TYPE_INTEGER | POINTER_TO) != TYPE_INTEGER)
    MacBadParam(1);

  else
    {
    MButtons = sample_mouse_position(&Row, &Col, Xpos, Ypos);

    *CrvX = CursorStatus[WindowPlotAssignment[ActiveWindow]].X;
    *CrvY = CursorStatus[WindowPlotAssignment[ActiveWindow]].Y;
    *CrvZ = CursorStatus[WindowPlotAssignment[ActiveWindow]].Z;
  
    PushToDataStack(&MButtons, TYPE_INTEGER, FALSE);
    }
}

/* syntax: Time := NOW(); */
/******************************************************************/
PRIVATE void MacSysTime(void)
{
  double MacTime;
  struct timeb TTime;
  
  ftime(&TTime);

  MacTime = (double)TTime.time + (double)TTime.millitm * 0.001;

  PushToDataStack(&MacTime, TYPE_REAL, FALSE);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The CommandList[] declaration and initialization has been
// moved here AFTER most of the functions have been defined above.
// Therefore, no prototypes for them are needed.

// NOTE : The functions in this table must MATCH EXACTLY
// with the index values defined in proclist.c which is used to create
// oma4000.mac which defines all the predefined entries in SymbolTable[].

void (*CommandList[])(void) = { 
  Plus,
  Minus,
  Multiply,
  Divide,
  IntDiv,
  Mod,
  Trunc,
  Round,
  MacAbs,
  And,
  Or,                  // 10
  Xor,
  Not,
  Shl,
  Shr,
  MacLog,
  MacLn,
  MacExp,
  MacSin,
  MacCos,
  MacTan,              // 20
  MacAtan,
  MacAsin,
  MacAcos,
  MacAtan2,
  GMax,
  SMax,
  GMin,
  SMin,
  MacStrlen,
  RatFit,              // 30
  PolyFit,             
  PolyDerv,
  MacDoSmoothOrDeriv,
  AvgStdDev,
  MacAbsorb,

  AssignTo,

  EqualTo,
  NotEqualTo,
  LessThan,
  GreaterThan,
  LessThanEq,          // 40
  GreaterThanEq,       

  Assign,
  MacClose,
  Reset,
  Rewrite,
  Seek,
  MacReadChar,
  MacRead,
  ReadLn,
  MacWrite,
  WriteLn,             // 50
  EndOfLine,           
  EndOfFile,
  KeyPressed,

  GMem,
  GMemW,
  SMem,
  SMemW,
  GPort,
  GPortW,
  SPort,               // 60
  SPortW,              

  DNumBytes,
  DNumpts,
  DAddMem,
  DGADP,
  DArea,
  DReadMem,            // 70
  DLoadMem,            
  DCArea,
  D_CL,
  DClr,
  DClrAll,
  DCrM,
  DGDA,
  DSDA,
  LoadDAD,
  DGDM,
  DGDT,
  DSDT,
  DGErr,               // 90
  DGET,                
  DSET,
  DGFreq,
  DSFreq,
  DGI,                 // 100
  DSI,                 
  DGID,
  DGJ,
  DSJ,
  DGK,
  DSK,
  DGL,
  DSL,
  DStoreDAD,
  DGMaxMem,            
  DGMem,
  DSMem,
  DGMinET,

  DG_Ptime,
  DS_Ptime,
  DG_Stime,
  DS_Stime,

  DG_Ftime,

  DGPD,
  DSPD,
  DGPlsr,
  DGPW,                // 130
  DSPW,                
  DRun,
  DStop,
  DGSameET,
  DSSameET,
  DSShutter,
  DSub,                // 140
  DGSync,              
  DSSync,              
  DGVer,

  DGActiveX,
  DSActiveX,
  DGActiveY,
  DSActiveY,
  DGDeltaX,
  DSDeltaX,
  DGDeltaY,
  DSDeltaY,
  DGOutputReg,
  DSOutputReg,
  DGH,                 // 160
  DSH,                 
  DGAntiBloom,
  DSAntiBloom,
  DGScanMode,
  DSScanMode,
  DGShutMode,
  DSShutMode,
  DGSCITC,
  DSSCITC,
  DGSCmp,              // 170
  DGPntMode,
  DSPntMode,
  DGPoint,
  DSPoint,
  DGPoints,
  DSPoints,
  DGRegions,
  DSRegions,
  DGCurRegion,
  DSCurRegion,
  DGRegSize,
  DSRegSize,
  DGRegET,
  DSRegET,
  DGTrack,
  DSTrack,
  DGTracks,            // 180
  DSTracks,            
  DGTrkMode,
  DSTrkMode,
  DGWFTC,
  DSWFTC,
  DGWFTO,
  DSWFTO,
  DGX0,
  DSX0,
  DGY0,                // 190
  DSY0,

  DGOutPIA,
  DSOutPIA,
  
  DGInPIA,
  
  DGPAudio,
  DSPAudio,
  
  DGPTrigNum,
  DSPTrigNum,
  
  DGPTrigSrc,
  DSPTrigSrc,
  
  DGPTrigTrsh,
  DSPTrigTrsh,
  
  DGPDelInc,
  DSPDelInc,
  
  DGPDelRange,
  DSPDelRange,
  
  DGPIMode,
  DSPIMode,

  DGStreakMode,
  DSStreakMode,

  DGExposedRows,
  DSExposedRows,

  DGPreTrigTracks,
  DSPreTrigTracks,

  CreateCurve,

  CSCount,
  DCCount,
  MCCount,

  LoadMethod,
  SaveMethod,
  LoadFileCurves,
  SaveFileCurves,
  InsCurve,            // 200
  InsCurveSet,         
  DelCurveSet,
  DelCurve,
  MacCreateCurveSet,
  GCurveSetIndex,
  GActiveWindow,
  SActiveWindow,
  PlotWindow,
  SPlotStyle,
  SWindowStyle,        // 210
  SAccum,              
  SLive,
  SFrameCapture,
  LoadBackground,
  MacChangeCurveSize,
  RunMacroFile,

  GPIBClear,
  GPIBSend,
  GPIBResponse,
  GPIBSerialPoll,      // 220
                       
  GDetAddress,
  SDetAddress,
  GDetIndex,
  SDetIndex,

  StartFresh,
  Drop,
  FillCSCurve,
  FillDCCurve,
  FillMCCurve,
                        
  MacPlayMenu,         // 230
  MacPlayForm,         
  MacPlayField,
  LeaveForm,
  LeaveMenu,
  MacFocusOnField,
                          
  MacEnterGraph,             
  MacLeaveGraph,       
                          
  MacHelp,                   
  MacGoLive,                    
  MacStopLive,                  
  MacZoom,                      
  MacRestoreZoom,               
  MacAutoScale,                 
  MacScaleX,                 
  MacScaleY,                 
  MacScaleZ,                 
  SActiveWindow,
  MacSetThreshold,              
  MacDrawPeaks,                 
  MacAddKnot,                   
  MacReplotWindow,              
  MacReplotScreen,     // 250   
  MacMoveCursor,

  MacExpandX,
  MacExpandY,
  MacContractX,
  MacContractY,
                          
  GActivePlotSetup,          
  SActivePlotSetup,       
  MacExpandTagged,           
  MacRestoreFromExpand,   
  MacTagCurve,               
  MacUntagCurve,       

  DGCoolOnOff,         // 258
  DSCoolOnOff,

  DG_Active,

  MacSend1235Cmd,
  MacGet1235Response,
  MacReset1235,
  Mac1235SelSpec,
  MacResetOffset,
  Mac1235Go,
  Mac1235WL,
  MacChangeGrating,
  MacStore1235,
  MacExecuteDispersion,

  MacShellCmd,
  MacStringToFloat,
  MacFloatToString,
  GetCommandPrompt,
  SetCommandPrompt,

  MacLoadLampFile,
  MacSaveLampFile,
  MacAddYcalibPair,
  MacDelYcalibPair,
  MacDeleteLampData,
  MacGenCorrexCurve,
  
  MacSetFilter,
  MacWait,

  MacComInit,
  MacComStat,
  MacComOut,
  MacComIn,

  MacReadMouse,

  MacPauseLive,
  MacFreeRun,

  MacSysTime,
  MacGetCal,
  MacSetCal,

  GPlotStyle,
  };

// execute a procedure from the command list
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void procedureExec(int commandListIndex)
{
  (* CommandList[ commandListIndex ])();
}

BOOLEAN isMacPlayMenu(int index)
{
  return CommandList[ index ] == MacPlayMenu;
}

BOOLEAN isMacPlayForm(int index)
{
  return CommandList[ index ] == MacPlayForm;
}

BOOLEAN isMacPlayField(int index)
{
  return CommandList[ index ] == MacPlayField;
}

