/* -----------------------------------------------------------------------
/
/  mdatstak.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/macro/mdatstak.c_v   1.4   06 Jul 1992 12:52:08   maynard  $
/  $Log:   J:/logfiles/oma4000/macro/mdatstak.c_v  $
*/

#include <memory.h>
#include <malloc.h>

#include "mdatstak.h"
#include "macruntm.h"   // CURVE_REF
#include "macnres2.h"   // minmax_curve
#include "cursor.h"     // ActiveWindow
#include "tempdata.h"
#include "points.h"
#include "di_util.h"
#include "oma4000.h"
#include "omaerror.h"
#include "crventry.h"
#include "crvheadr.h"
#include "curvedir.h"
#include "curvdraw.h"

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

/* The maximum depth for run-time data stack */
#define DEFAULT_DATA_STACK_SIZE 100

typedef struct {
   UCHAR Type;
   union {
      void * Pointer;
      SHORT Integer;
      LONG LongInteger;
      USHORT Word;
      FLOAT Real4;
      double Real;
      UCHAR Byte;
      SHORT Boolean;
      CHAR * String;
      CURVE_REF Curve;
   } U;
} DATA_STACK_ITEM;
 
static DATA_STACK_ITEM * DataStack;     /* actual stack area */
static SHORT DataStackDepth;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT getStackDepth(void)
{
  return(DataStackDepth);
}

PRIVATE void StackEmpty(void)
{
  error(ERROR_STACK_EMPTY);
  SetErrorFlag();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void PushToDataStack(void * ValuePtr, SHORT Type, BOOLEAN PointerRef)
{
  DATA_STACK_ITEM * StackPtr;

  ++DataStackDepth;

  if (DataStackDepth >= DEFAULT_DATA_STACK_SIZE)
    {
    error(ERROR_STACK_FULL);
    SetErrorFlag();
    return;
    }
  StackPtr = &DataStack[DataStackDepth];

  if (PointerRef)
    Type |= POINTER_TO;

  StackPtr->Type = (UCHAR)Type;     

  if (Type & POINTER_TO)
    StackPtr->U.Pointer = ValuePtr;
  else
    {
    if (Type == TYPE_BOOLEAN)
      Type = TYPE_WORD;

    switch (Type)
      {
      case TYPE_INTEGER:
        StackPtr->U.Integer = *((SHORT *) ValuePtr);
        break;
      case TYPE_WORD:
        StackPtr->U.Word = *((USHORT *) ValuePtr);
        break;
        /*       case TYPE_BOOLEAN:
        StackPtr->U.Boolean = *((BOOLEAN *) ValuePtr);
        break; */
      case TYPE_LONG_INTEGER:
        StackPtr->U.LongInteger = *((LONG *) ValuePtr);
        break;
      case TYPE_REAL4:
        StackPtr->U.Real4 = *((FLOAT *) ValuePtr);   
        break;
      case TYPE_REAL:
        StackPtr->U.Real = *((double *) ValuePtr);
        break;
      case TYPE_BYTE:
        StackPtr->U.Byte = *((CHAR *) ValuePtr);
        break;
      case TYPE_STRING:
        StackPtr->U.String = (CHAR *) ValuePtr;
        break;
      case TYPE_CURVE:
        memcpy(&StackPtr->U.Curve, ((CURVE_REF *) ValuePtr),
        sizeof(CURVE_REF));
        break;
      }
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRIVATE SHORT DereferenceStackItem(SHORT StackDepth, void * Parameter,
                                   SHORT RequiredType)
{
  DATA_STACK_ITEM * StackPtr;
  void * CurrentContents;
  SHORT Type;

  StackPtr = &DataStack[StackDepth];

  Type = StackPtr->Type;

  if (Type & POINTER_TO)
    {
    CurrentContents = StackPtr->U.Pointer;
    Type &= ~(POINTER_TO);
    }
  else
    CurrentContents = &StackPtr->U;

  if (RequiredType & POINTER_TO)
    {
    *((void **) Parameter) = CurrentContents;
    }
  else
    {
    ConvertTypes(CurrentContents, Type, Parameter, RequiredType);
    }
  return(Type);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT PopFromDataStack(void * Parameter, SHORT RequiredType)
{
  SHORT type = 0;

  if(DataStackDepth > 0)
    type = DereferenceStackItem((DataStackDepth--), Parameter, RequiredType);
  else
    StackEmpty();

  return(type);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT IndexDataStack(SHORT StackDepth, void * Parameter, SHORT RequiredType)
{
  return(DereferenceStackItem((DataStackDepth - StackDepth),
    Parameter, RequiredType));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHORT DataStackPeek(SHORT StackDepth)
{
  return(DataStack[ DataStackDepth - StackDepth ].Type);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void RemoveItemsFromDataStack(SHORT count)
{
  if ((DataStackDepth -= count) < 0)
    {
    DataStackDepth = 0;
    StackEmpty();
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void InitDataStack(void)
{
   DataStackDepth = 0;
}

// TRUE if successfully got a scalar or a string pointer (ULONG) into Value
// FALSE if only a curve or a curve set was referenced, Value will then
// be unchanged
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN PopScalarFromDataStack(PVOID Value, SHORT Type)
{
   BOOLEAN ReturnVal;

   ReturnVal = ScalarIndexDataStack(0, Value, Type);
   RemoveItemsFromDataStack(1);

   return ReturnVal;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN ScalarIndexDataStack(SHORT Index, PVOID Value, SHORT Type)
{
  CURVE_REF Curve;
  DOUBLE Y;
  FLOAT X;
  CURVEHDR Curvehdr;
  LPCURVE_ENTRY pEntry;
  ULONG TempAddr;
  SHORT ActualType, prefBuf = -1;
  PVOID pTmp;

  /* check to see if it is a curve */
  ActualType = IndexDataStack(Index, &pTmp, TYPE_DONT_CARE | POINTER_TO)
               & ~POINTER_TO;
  if (ActualType == TYPE_CURVE)
    {
    Curve = * (CURVE_REF *) pTmp;

    if((Curve.ReferenceType == CLASS_CURVESET) ||
      (Curve.ReferenceType == CLASS_CURVE))
       return FALSE;

    /* make sure referenced curve set actually exists! */
  
    if (Curve.CurveSetIndex == NOT_FOUND ||
        Curve.CurveSetIndex >= MainCurveDir.BlkCount)
        {
        error(ERROR_BAD_CURVE_BLOCK, Curve.CurveSetIndex);
        SetErrorFlag();
        return FALSE;
        }

    if (Curve.ReferenceType == CLASS_POINT)
      {
      /* get the X and Y values for the data point */
      // change to buffer 1 for better point to point assignment
      if(GetDataPoint(& MainCurveDir, Curve.CurveSetIndex,
                        Curve.CurveIndex, Curve.PointIndex,
                        &X, &Y, (CHAR) TYPE_REAL, &prefBuf))
         return FALSE;

      if (Curve.Point_Or_HeaderItem != CURVE_POINTX)
         ConvertTypes(&Y, TYPE_REAL, Value, Type);
      else
         ConvertTypes(&X, FLOATTYPE, Value, Type);
      }
    else if (Curve.ReferenceType == CLASS_CURVESETHDR)
      {
      pEntry = &(MainCurveDir.Entries[Curve.CurveSetIndex]);
      switch (Curve.Point_Or_HeaderItem)
        {
        case CURVE_START_INDEX:
          ConvertTypes(&(pEntry->StartIndex), TYPE_WORD, Value, Type);
        break;

        case CURVE_COUNT:
          ConvertTypes(&(pEntry->count), TYPE_WORD, Value, Type);
        break;

        case CURVE_NAME:
          if ((Type & ~POINTER_TO)!= TYPE_STRING)
             return FALSE;
          TempAddr = (ULONG) pEntry->name;
          memcpy(Value, &TempAddr, 4);
        break;

        case CURVE_PATH:
          if ((Type & ~POINTER_TO)!= TYPE_STRING)
             return FALSE;
          TempAddr = (ULONG) pEntry->path;
          memcpy(Value, &TempAddr, 4);
        break;

        case CURVE_DESC:
          if ((Type & ~POINTER_TO)!= TYPE_STRING)
             return FALSE;
          TempAddr = (ULONG) pEntry->descrip;
          memcpy(Value, &TempAddr, 4);
        break;

        case CURVE_DISPLAY:
          ConvertTypes(&(pEntry->DisplayWindow), TYPE_WORD,
          Value, Type);
        break;
        }
      }
    else if (Curve.ReferenceType == CLASS_CURVEHDR)
      {
      if(ReadTempCurvehdr(& MainCurveDir, Curve.CurveSetIndex,
                            Curve.CurveIndex, & Curvehdr))
        return FALSE;

      switch (Curve.Point_Or_HeaderItem)
        {
        case CURVE_POINT_COUNT:
          ConvertTypes(&(Curvehdr.pointnum), TYPE_WORD, Value, Type);
        break;

        case CURVE_TIME:
          ConvertTypes(&(Curvehdr.time), FLOATTYPE, Value, Type);
        break;

        case CURVE_FRAME:
          ConvertTypes(&(Curvehdr.Frame), TYPE_WORD, Value, Type);
        break;

        case CURVE_TRACK:
          ConvertTypes(&(Curvehdr.Track), TYPE_WORD, Value, Type);
        break;

        case CURVE_YMIN:
          minmax_curve(Curve.CurveSetIndex, Curve.CurveIndex,
                       Curvehdr.pointnum, &(Curvehdr.Ymin), NULL);
          ConvertTypes(&(Curvehdr.Ymin), FLOATTYPE, Value, Type);
        break;
        case CURVE_YMAX:
          minmax_curve(Curve.CurveSetIndex, Curve.CurveIndex,
                       Curvehdr.pointnum, NULL, &(Curvehdr.Ymax));
           ConvertTypes(&(Curvehdr.Ymax), FLOATTYPE, Value, Type);
        break;

        case CURVE_XMIN:
          ConvertTypes(&(Curvehdr.Xmin), FLOATTYPE, Value, Type);
        break;

        case CURVE_XMAX:
          ConvertTypes(&(Curvehdr.Xmax), FLOATTYPE, Value, Type);
        break;

        case CURVE_XUNITS:
          ConvertTypes(&(Curvehdr.XData.XUnits), UCHARTYPE, Value, Type);
        break;

        case CURVE_YUNITS:
          ConvertTypes(&(Curvehdr.YUnits), UCHARTYPE, Value, Type);
        break;

        case CURVE_SCMP:
          ConvertTypes(&(Curvehdr.scomp), ULONGTYPE, Value, Type);
        break;
        }

      }
    }
  else
    {
    if(((Type & ~POINTER_TO) == TYPE_STRING) &&
        ((ActualType & ~POINTER_TO) != TYPE_STRING))
      return FALSE;

    if(((Type & ~POINTER_TO) == TYPE_STRING) &&
      ((ActualType & ~POINTER_TO) == TYPE_STRING))
       memcpy(Value, &pTmp, 4);

    else if (Type & POINTER_TO)
       memcpy(Value, &pTmp, 4);
    else
       ConvertTypes(pTmp, ActualType, Value, Type);
  }

  return TRUE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN ScalarDataStackPeek(SHORT StackIndex, PSHORT ReturnType)
{
  CURVE_REF Curve;
  CURVE_REF *pCurve;

  /* check to see if it is a curve */
  if((*ReturnType = (DataStackPeek(StackIndex) & ~POINTER_TO)) == TYPE_CURVE)
    {
    IndexDataStack(StackIndex, &pCurve, TYPE_DONT_CARE | POINTER_TO);
    Curve = * pCurve;

    *ReturnType = Curve.ReferenceType;
    if ((*ReturnType == CLASS_CURVESET) || (*ReturnType == CLASS_CURVE))
      return FALSE;        // error, not a simple scalar

    switch (*ReturnType)
      {
      case CLASS_POINT:
        *ReturnType = TYPE_REAL;
      break;

      case CLASS_CURVESETHDR:
        switch (Curve.Point_Or_HeaderItem)
          {
          case CURVE_START_INDEX:
            *ReturnType = TYPE_WORD;
          break;

          case CURVE_COUNT:
            *ReturnType = TYPE_WORD;
          break;

          case CURVE_NAME:
            *ReturnType = TYPE_STRING;
          break;

          case CURVE_PATH:
            *ReturnType = TYPE_STRING;
          break;

          case CURVE_DESC:
            *ReturnType = TYPE_STRING;
          break;

          case CURVE_DISPLAY:
            *ReturnType = TYPE_WORD;
          break;
          }
      break;

      case CLASS_CURVEHDR:
        switch (Curve.Point_Or_HeaderItem)
          {
          case CURVE_POINT_COUNT:
          case CURVE_FRAME:
          case CURVE_TRACK:
            *ReturnType = TYPE_WORD;
          break;

          case CURVE_TIME:
          case CURVE_YMIN:
          case CURVE_YMAX:
          case CURVE_XMIN:
          case CURVE_XMAX:
            *ReturnType = TYPE_REAL4;
          break;

          case CURVE_XUNITS:
          case CURVE_YUNITS:
            *ReturnType = TYPE_BYTE;
          break;

          case CURVE_SCMP:
            *ReturnType = TYPE_LONG_INTEGER;
          break;
          }
        break;
        }
      }
    return TRUE;      // successful
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void FillAndPushCurve(SHORT CurveType)
{
  SHORT RefType;
  SHORT EntryIndex;
  USHORT CurveIndex, i;
  CURVE_REF Curve;
  USHORT UTDisplayCurve;

  /* get the class */
  PopFromDataStack(&Curve.ReferenceType , TYPE_INTEGER);
  RefType = Curve.ReferenceType;

  /* see if need to get more curve info */
  if ((RefType == CLASS_CURVESETHDR) ||
      (RefType == CLASS_CURVEHDR) ||
      (RefType == CLASS_POINT))
    {
    PopFromDataStack(&(Curve.Point_Or_HeaderItem), TYPE_INTEGER);

    if (RefType == CLASS_POINT)
      /* get the point index */
      PopFromDataStack(&(Curve.PointIndex), TYPE_WORD);
    }
  switch (CurveType)
    {
    case CURVESET_CS:
      PopFromDataStack(&(Curve.CurveIndex), TYPE_WORD);
      PopFromDataStack(&(Curve.CurveSetIndex), TYPE_INTEGER);
    break;         // all fields in Curve are done

    case CURVESET_DC:
      PopFromDataStack(&(Curve.CurveIndex), TYPE_WORD);
      Curve.CurveSetIndex = -1;
      if(! FindFirstPlotBlock(& MainCurveDir, & EntryIndex,
        & CurveIndex,   & UTDisplayCurve,
        ActiveWindow))
        EntryIndex = -1;

      for (i=1; (i<=Curve.CurveIndex) && (EntryIndex != -1); i++)
        {
        if(!FindNextPlotCurve(&MainCurveDir, &EntryIndex,
                              &CurveIndex, &UTDisplayCurve, ActiveWindow))
          EntryIndex = NOT_FOUND;     // curve not found
        }

      /* make sure that the curve was found */
      if (EntryIndex == NOT_FOUND)
        {
        error(ERROR_CURVE_NUM, EntryIndex);
        SetErrorFlag();
        return;
        }
      Curve.CurveIndex = CurveIndex;
      Curve.CurveSetIndex = EntryIndex;
    break;

    case CURVESET_MC:
      PopFromDataStack(&(Curve.CurveIndex), TYPE_WORD);
      Curve.CurveSetIndex = -1;
      /* check for a bad curve index */
      if (Curve.CurveIndex >= MainCurveDir.CurveCount)
        {
        error(ERROR_CURVE_NUM, Curve.CurveIndex);
        SetErrorFlag();
        return;
        }
      EntryIndex = 0;

      while (MainCurveDir.Entries[EntryIndex].count <= Curve.CurveIndex)
        {
        Curve.CurveIndex -= MainCurveDir.Entries[EntryIndex].count;
        EntryIndex++;
        }
      Curve.CurveSetIndex = EntryIndex;
    break;
    }
  PushToDataStack(&Curve, TYPE_CURVE, FALSE);
}

// allocalte memory for the data stack
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN allocateDataStack(void)
{
   DataStack = (DATA_STACK_ITEM *) malloc(sizeof(DATA_STACK_ITEM)
                                           * DEFAULT_DATA_STACK_SIZE);
   return DataStack != NULL;
}

// deallocate the data stack
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void releaseDataStack(void)
{
   if(DataStack != NULL) free(DataStack);
}
