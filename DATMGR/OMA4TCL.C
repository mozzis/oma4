/***************************************************************************/
/*                                                                         */
/* File name  : OMA4TCL.C (Save Curvesets in TCL File Format)              */
/* Author     : Morris Maynard                                             */
/* Version    : 1.00 - Initial version.                                    */
/* Description: Saves and Loads OMA4 Curvesets in TCL File Format          */
/*                                                                         */
/***************************************************************************/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

/* Write data from memory to disk in TCL 32 bit format.
****************************************************************************/
ERROR_CATEGORY trans_TCL_32(FILE* fptr, const char * fName,
                            CURVEDIR *CvDir,
                            SHORT Blk, SHORT StCurve, SHORT Curves,
                            BOOLEAN KbChk)
{
  SHORT i, j, Points, BufNum = 0;
  FLOAT Param, X, Y, *FBuff;
  BOOLEAN Done = FALSE;
  CURVEHDR CvHdr;
  ERROR_CATEGORY err = ERROR_NONE;
  struct dosdate_t Date;
  CHAR * Desc = CvDir->Entries[Blk].descrip;

  /* base the output file on the parameters of the first input curve */

  err = ReadTempCurvehdr(CvDir, Blk, StCurve, &CvHdr);
  if (err)
    return err;

  Points = CvHdr.pointnum;

  FBuff = malloc(Points*sizeof(FLOAT));
  if (!FBuff)
    return (error(ERROR_ALLOC_MEM));

  TCL_Header.FileFormat      = BITS32;
  TCL_Header.NumSerialPixels = Points;            /* sb Points from method */
  TCL_Header.NumImagePixels  = Curves;            /* sb Tracks from method */
  TCL_Header.PixelFormat     = BITS32GRAY;
  TCL_Header.NumFrames       = 1;                 /* sb J from method */
  GetParam(DC_DTEMP, &Param);
  TCL_Header.Temperature     = (SHORT)Param;      /* sb DTEMP from method */
  GetParam(DC_ET, &Param);
  TCL_Header.ExposureTime    = (DOUBLE)Param;     /* sb ET from method */
  TCL_Header.XLeft           = 0;
  GetParam(DC_ACTIVEX, &Param);
  TCL_Header.XRight          = (ULONG)Param-1;    /* sb ACTIVEX from method */
  TCL_Header.YTop            = 0;
  GetParam(DC_ACTIVEY, &Param);
  TCL_Header.YBottom         = (ULONG)Param-1;    /* sb ACTIVEY from method */
  GetParam(DC_DELTAX, &Param);
  TCL_Header.PixelSize       = (USHORT)Param;     /* sb from method; approx */
  TCL_Header.DataType        = TCL_FLOAT;         /* always float */
  TCL_Header.Identifier      = TCLIDENTIFIER;
  _dos_getdate(&Date);
  TCL_Header.Date[0]         = (USHORT)Date.month;
  TCL_Header.Date[1]         = (USHORT)Date.day;
  TCL_Header.Date[2]         = Date.year;
  TCL_Header.NumMemories     = 1;                 /* sb from method */
  sprintf(TCL_Header.UserText1, "From OMA4000 %s", VersionString);
  strncpy(TCL_Header.UserText2, Desc, TEXT_LEN/3);
  if (strlen(Desc) > TEXT_LEN/3)
    strncpy(TCL_Header.UserText3, &Desc[TEXT_LEN/3], TEXT_LEN/3);
  else
    memset(TCL_Header.UserText3, 0, TEXT_LEN/3);
  strncpy(TCL_Header.DescriptionText, CvDir->Entries[Blk].descrip, 74);

  /* Write Header to file *************************************************/
  if (fwrite(&TCL_Header, sizeof(TCL_HEADER), 1, fptr) != 1)
    {
    free(FBuff);
    return error(ERROR_WRITE, fName);
    }

  for (i = StCurve + Curves-1; i >= StCurve && !Done; i--)
    {
    if (KbChk && kbhit() && (getch() == ESCAPE))
      break;

    for (j = 0; j < Points; j++)
      {
      if (KbChk && kbhit() && (getch() == ESCAPE))
        {
        Done = TRUE;
        break;
        }

      err = GetDataPoint(CvDir, Blk, i, j, &X, &Y, FLOATTYPE, &BufNum);
      if (err)
        {
        Done = TRUE;
        break;
        }
      FBuff[j] = Y;
      }

    if (fwrite(FBuff, sizeof(FLOAT), Points, fptr) != (USHORT)Points)
      {
      Done = TRUE;
      err = error(ERROR_WRITE, fName);
      break;
      }
    }
  free(FBuff);
  return err;
}
