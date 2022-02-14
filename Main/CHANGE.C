/* -----------------------------------------------------------------------
/
/  change.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/change.c_v   0.21   06 Jul 1992 10:25:12   maynard  $
/  $Log:   J:/logfiles/oma4000/main/change.c_v  $
 
/
*/

#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#include <conio.h>        
#include <dos.h>    // _dos_getdate

#include "change.h"
#include "filestuf.h"
#include "tempdata.h"
#include "points.h"
#include "curvedir.h"
#include "di_util.h"
#include "oma4000.h"
#include "oma4driv.h"
#include "formwind.h"
#include "calib.h"
#include "curvdraw.h"  // ActivePlot
#include "syserror.h"  // ERROR_OPEN
#include "omaerror.h"
#include "fileform.h"  // is_special_name()
#include "omaform.h"   // COLORS_MESSAGE
#include "crventry.h"
#include "crvheadr.h"
#include "omameth.h"
#include "plotbox.h"   // ActivePlot
#include "forms.h"
#include "curvbufr.h"  // ActivePlot
#include "oma4tiff.h"

/* definitions for Hidris conversion                                       */
#define BITS16       0                 /* FileFormat valid data values.    */
#define BIT8PACKED   1
#define BITS32       2
#define BITS8GREY    8                 /* PixelFormat valid data values.   */
#define BITS16GRAY   16
#define BITS32GRAY   32
#define TCLIDENTIFIER 1234             /* Used to identify a TCL32bit file */
#define TCL_INTEGER         0x55555555 /* Data types. */
#define TCL_SIGNED          0xaaaaaaaa
#define TCL_FLOAT           0x5a5a5a5a
#define TEXT_LEN      75           /* a number not evenly divisible by 3! */

typedef struct 
 {                                    /* TCL data header structure.       */
  USHORT FileFormat;      /* File format descriptor.          */
  USHORT NumSerialPixels; /* Number of pixels per frame.      */
  USHORT NumImagePixels;  /* Number of pixels per frame.      */
  USHORT AlwaysZero;      /* Zero for disk files.             */
  USHORT PixelFormat;     /* Pixel format descriptor.         */
  SHORT  Reserved[27];    /* This area is reserved by TCL.    */
/* User specific data -----------------------------------------------------*/
  USHORT NumFrames;       /*  Number of frames per memory.    */
  SHORT  Temperature;     /*  Desired Cooler temerature.      */
  DOUBLE ExposureTime;    /*  Exposure time in seconds.       */
  ULONG  XLeft;           /* Phyical locations values.        */
  ULONG  XRight;
  ULONG  YTop;
  ULONG  YBottom;
  LONG   Identifier;      /* Used to identify a TCL file.     */
  USHORT PixelSize;       /*  Physical size of each pixel.    */
  LONG   DataType;        /* Data type see IMAGE386.INC       */
  USHORT PixelSpeed;      /* Convesion time in uS.            */
  SHORT  NumMemories;     /* Number of Mems per file.         */
  SHORT  UnusedNumericalData[96 - 21]; /* Unused user numerical data.  */
/*------------------------------------------------------------------------*/
  CHAR UserText1[TEXT_LEN / 3];       /* HiDRIS text.         */
  CHAR UserText2[TEXT_LEN / 3];       /* HiDRIS text.         */
  CHAR UserText3[TEXT_LEN / 3];       /* HiDRIS text.         */
  CHAR UserText[152 - TEXT_LEN];      /* User text.           */
  USHORT Date[15];        /* Creatation date of file.         */
  CHAR DescriptionText[74];           /* Description text.    */
  } TCL_HEADER ;

TCL_HEADER TCL_Header;

// private functions
ERR_OMA trans_oma4_ascii(FILE*, CURVEDIR *, SHORT, SHORT, SHORT, SHORT, BOOLEAN);
ERR_OMA trans_oma4_oma1460(FILE*, CURVEDIR *, int, int, int, BOOLEAN);
ERR_OMA trans_oma4_oma1470(FILE*, const char *, CURVEDIR *, int, int, int, BOOLEAN);
ERR_OMA trans_TCL_32(FILE*, const char *, CURVEDIR *, int, int, int, BOOLEAN);
void GetCalib1460(CURVEDIR *pCurveDir, SHORT EntryIndex, USHORT CurveIndex,
   USHORT number_of_points);

#define PRECISION_VAL      4   /* precision in oma1460 pulse delay.width */

char ch_input_fname[CH_FILESIZE]  = {""};
char ch_output_fname[CH_FILESIZE] = {""};
SHORT   ch_output_format = 0;
USHORT  from_field = 0;
USHORT  count_field = 0;
static  double  Change_CalCoeff[4];
static char * overwrite_prompt[] = {"Output file already exists",
                                    "Overwrite existing file ?",
                                     NULL};
static char * differentX_prompt[] = {"Different X Values detected",
                                     "Abort Translation ?",
                                      NULL};
static char * go_on_message[] = {"Too many curves",
                                 "Do as many as possible ?",
                                  NULL};

/****************************************************************************/
int change_execute(void* field_data, char * field_string)
{
  char path[CH_FILESIZE];
  char blockname[13];
  SHORT entry_index;
  char *position;
  FILE *fptr;
  WINDOW *MessageWindow;
  BOOLEAN specialFile = is_special_name(ch_input_fname);

  if(specialFile)
    {
    path[0] = '\0';
    strcpy(blockname, ch_input_fname);
    }
  else
    {
    ParseFileName(path, ch_input_fname);
    if ((position = strrchr(path,'\\')) == NULL)
      {
      error(ERROR_BAD_FILENAME, ch_input_fname);
      return FIELD_VALIDATE_WARNING;
      }
    /* parse name into a path and a block name */
    strcpy(blockname,position+1);          /* filename in "blockname" */
    *position = '\\';                      /* path in "path" */
    *(position+1) = '\0';                  /* path in "path" */
    }

  /* look for curve block */
  if ((entry_index = SearchCurveBlkDir(blockname, path, from_field, &MainCurveDir)) == -1)
    {
    error(ERROR_BAD_FILENAME, ch_input_fname);
    return FIELD_VALIDATE_WARNING;
    }

  /* found entry. check to see if last curvenum is in same entry */
  if ((count_field < 1) || ((from_field + count_field - 1) >=
    (MainCurveDir.Entries[entry_index].StartIndex +
    MainCurveDir.Entries[entry_index].count)))
    {
    error(ERROR_CURVE_NUM, from_field + count_field - 1);
    return FIELD_VALIDATE_WARNING;
    }

   /* check to see if file name exists */
   /* and if so whether we should overwrite */
  if (!access(ch_output_fname, 0))  /* see if file exists */
    {
    if (yes_no_choice_window(overwrite_prompt, 0,COLORS_MESSAGE) != YES)
      return FIELD_VALIDATE_SUCCESS;
    }

  put_up_message_window(BusyWorkingEsc, COLORS_MESSAGE, &MessageWindow);

  /* get real output file - must open in binary mode for 1470  */
  if(ch_output_format == TRANS_OMA1470 || ch_output_format == TRANS_TCL32 ||
     ch_output_format == TRANS_TIFF)
    fptr = fopen(ch_output_fname, "w+b");
  else
    fptr = fopen(ch_output_fname, "w+t");

  if (fptr == NULL)
    {
    if(MessageWindow)
      release_message_window(MessageWindow);
    error(ERROR_OPEN, ch_output_fname);
    return FIELD_VALIDATE_WARNING;
    }

  /* we are free to perform translation */
  
  switch (ch_output_format)
    {
    case TRANS_OMA1460:
      trans_oma4_oma1460(fptr,& MainCurveDir, entry_index, from_field,
                         count_field, TRUE);
    break;
    case TRANS_OMA1470:
      trans_oma4_oma1470(fptr, ch_output_fname, &MainCurveDir,
                         entry_index, from_field, count_field, TRUE);
    break;
    case TRANS_SINGLE: 
    case TRANS_SINGLE_X:
    case TRANS_MULTI:
    case TRANS_MULTI_X:
      trans_oma4_ascii(fptr, &MainCurveDir, entry_index, 
                       from_field, count_field, ch_output_format, TRUE);
    break;
    case TRANS_TCL32:
      trans_TCL_32(fptr, ch_output_fname, &MainCurveDir,
                   entry_index, from_field, count_field, TRUE);
    break;
    case TRANS_TIFF:
      trans_TIFF256(fptr, ch_output_fname, &MainCurveDir,
                    entry_index, from_field, count_field, TRUE);
    break;
    }

  fclose(fptr);

  if (MessageWindow)
    release_message_window(MessageWindow);

  return FIELD_VALIDATE_SUCCESS;
}

#define CELL_SIZE 13 /* how many character positions per column */

static ULONG calc_new_position(SHORT Point, SHORT Curve, SHORT Curves,
                               BOOLEAN UseX)
{
  ULONG LPoint =  (ULONG)Point,
        LCurve =  (ULONG)Curve,
        LCurves = (ULONG)Curves;

  /* offset to row includes +2 for CRLF combination (\n) */
  /* offset to col uses Curve!=0 since LCurve+Usex yields */
  /* non-zero for curve 0 if UseX is 1 */

  return ((CELL_SIZE * (LCurves + UseX)) + 2) * LPoint + /* Offset to row */
          (Curve != 0) * (LCurve + UseX) * CELL_SIZE;    /* Offset to col */
}

/*************************************************************************/
/*                                                                       */
/* Write ASCII file, in either Single or Multi Column (Y vals only) or   */
/* Single or Multi Column with X (X vals in first column). KbChk signals */
/* whether to abort if ESCape is pressed.                                */
/*                                                                       */
/* Multicolumn presents a performance problem.  The output file order    */
/* requires that we print a point from index 0 of every curve, then a    */
/* point from index 1, etc., so LoadCurveBuffer is called for every data */
/* point.  To improve this, all the points for as many curves as there   */
/* are buffers are written, and fseek is used to position the output     */
/* file pointer for each row.  Then the points for the next group of     */
/* curves are written, and fseek positions the output file pointer to the*/
/* end of each row.  And so forth...                                     */
/*                                                                       */
/*************************************************************************/

ERR_OMA trans_oma4_ascii(FILE* fptr, CURVEDIR * CvDir, SHORT Entry,
                                SHORT StartCv, SHORT Curves, SHORT Format,
                                BOOLEAN KbChk)
{
  CURVEHDR CvHdr;
  CURVE_ENTRY * pEntry;
  ERR_OMA err;
  SHORT Points, prefBuf, GroupSize, i, j, k;
  ULONG Foffset;
  FLOAT X, Y;
  BOOLEAN MULTI = (Format == TRANS_MULTI_X || Format == TRANS_MULTI),
          USEX  = (Format == TRANS_MULTI_X || Format == TRANS_SINGLE_X);

  if (MULTI)
    GroupSize = BUFNUM; /* use all the curve buffers for loop */
  else
    GroupSize = 1;

  if (Entry < 0 || Entry > (SHORT)CvDir->BlkCount)
    return(error(ERROR_BAD_CURVE_BLOCK, Entry));

  pEntry = &CvDir->Entries[Entry];

  /* Write the header information first */

  fprintf(fptr, "Curve Set:      %s%s\n"
                "Description:    %s\n"   
                "Starting Curve: %i\n"   
                "Count:          %i\n\n", pEntry->path, pEntry->name,
                                          pEntry->descrip,
                                          StartCv,
                                          Curves);
  /* print column header(s) */

  if (USEX && MULTI)
    fprintf(fptr, "%-*s", CELL_SIZE, "X Value");

  if (MULTI)  /* multiple column, print lables once above data columns */
    {
    for (i=0; i < Curves; i++)
      fprintf(fptr, "Curve %-*i", CELL_SIZE-6, StartCv + i);
    fprintf(fptr, "\n\n");  /* end of header */
    }
  
  /* Find # of points per curve (will always be the same for each curve!) */

  if(err = ReadTempCurvehdr(CvDir, Entry, StartCv, &CvHdr))
    return(err);
  Points = CvHdr.pointnum;

  Foffset = ftell(fptr);

  if (MULTI)
    chsize(fileno(fptr), Curves * Points * CELL_SIZE + Foffset);

  /* Convert curves to ascii */
  
  for (i = StartCv; i < StartCv + Curves; i += GroupSize)
    {
    if (!MULTI) /* single column, label start of each curve */
      {
      if (USEX)
        fprintf(fptr, "%-*s", CELL_SIZE, "X Value");
      fprintf(fptr, "Curve %-*i\n\n", CELL_SIZE-6, StartCv + i);
      }

    for (j = 0; j < Points; j++) /* each point in a new row */
      {
      /* at each row, seek to output file position of row & current column */
      if (MULTI)
        fseek(fptr,calc_new_position(j, i, Curves, USEX) + Foffset, SEEK_SET);

      for (k = i; k < i + GroupSize; k++) /* all points of buffered data */
        {
        if (k >= Curves)  /* if Curves not even multiple of BUFNUM */
          break;
        prefBuf = k % BUFNUM;             /* cycle between buffers */

        GetDataPoint(CvDir, Entry, k, j, &X, &Y, FLOATTYPE, &prefBuf);

        /* print an X value for each point if single column */
        /* if multi column, just for first curve in row */

        if (((k == StartCv) || !MULTI) && USEX)
          fprintf(fptr, "%-*.*g", CELL_SIZE, CELL_SIZE-7, X);
        fprintf(fptr, "%-*.*g", CELL_SIZE, CELL_SIZE-7, Y);
        }

      /* add CRLF at (current) end of row.  May be overwritten later */
      /* if system makes multiple passes for each row */

      fprintf(fptr, "\n");
      if (KbChk && kbhit() && getch() == ESCAPE) /* bail out if asked */
        {
        i = StartCv + Curves;
        break;
        }
      }
    if (!MULTI)    /* need blank line before next header line */
      fprintf(fptr, "\n");
    }
    fprintf(fptr, "\n\n");  /* finish off ascii file */
    return ERROR_NONE;
}

/****************************************************************************/
ERR_OMA trans_oma4_oma1460(FILE* fptr, CURVEDIR *directory,
                                 int entry_num, int starting_curvenum,
                                 int curvecount, BOOLEAN KbChk)
{
  SHORT tempcoeff, tempexp, j, prefBuf = 0;
  USHORT i, pointcount = 0, currentpoint;
  float X, Y, param;
  ERR_OMA err;
  CURVEHDR temphdr;

  if(err = ReadTempCurvehdr(directory, entry_num, starting_curvenum,
                              & temphdr))
     return err;

  pointcount = temphdr.pointnum;
  fprintf(fptr,"\n");               /* print initial CR */
  fprintf(fptr,"%d\n",curvecount);  /* print number of memories */

  GetParam(DC_TRACKS, &param);
  fprintf(fptr,"%d\n", (SHORT)param); /* print number of tracks/memory */
  fprintf(fptr,"%d\n",pointcount);  /* print number of points/track  */
  for (i=0; i<pointcount; i++)   /* print out first curve, move points into buffer 1*/
  {
     /* check for user escape */
     if (KbChk && kbhit() && getch() == ESCAPE)
              return ERROR_NONE;

     GetDataPoint(directory,entry_num, starting_curvenum, i, &X, &Y,
                  FLOATTYPE, &prefBuf);
     fprintf(fptr,"%g\n",Y);       /* print data points in memory */
  }
  fprintf(fptr,"\n");               /* print CR between memories   */
  /* save calibration values */
  for (i=0; i<4; i++)
     Change_CalCoeff[i] = InitialMethod->CalibCoeff[0][i];
  /* get calibration values */
  GetCalib1460(&MainCurveDir,entry_num,starting_curvenum,pointcount);
  for (i=0; i<4; i++)
     InitialMethod->CalibCoeff[0][i] = (float)CalCoeff[i];
  for (j=1; j<curvecount; j++)     /* do remaining curves */
  {
     if(err = ReadTempCurvehdr(directory, entry_num,
                                 starting_curvenum + j, & temphdr))
        return err;

     currentpoint = temphdr.pointnum;
     for (i=0; i<pointcount; i++)
     {
        /* check for user escape */
        if (KbChk && kbhit() &&  getch() == ESCAPE)
                 return ERROR_NONE;

        GetDataPoint(directory,entry_num,starting_curvenum+j,i,&X,&Y,
                     FLOATTYPE, &prefBuf);
        if (i <= currentpoint)
        {
           fprintf(fptr,"%g\n",Y);        /* print data points */
        }
        else
           fprintf(fptr,"0.0");  /* print data points */
     }
     fprintf(fptr,"\n");                 /* print CR between memories   */
  }
  /* done with data, now work on header */
  fprintf(fptr,"\"%s\"\n",ActivePlot->title);
  GetParam(DC_DMODEL, &param);
  switch ((SHORT)param)
  {
     case 1:
        fprintf(fptr,"%d\n", 1462);
        break;
     case 2:
        fprintf(fptr,"%d\n", 1463);
        break;
     case 3:
        fprintf(fptr,"%d\n", 1464);
        break;
     default:
        fprintf(fptr,"%d\n", 0);
        break;
  }
  GetParam(DC_POINTS, &param);
  fprintf(fptr,"%d\n", (SHORT)param);
  GetParam(DC_DTEMP, &param);
  fprintf(fptr,"%d\n", (SHORT)param);
  GetParam(DC_ET, &param);
  fprintf(fptr,"%g\n", param);
  fprintf(fptr,"%g\n",0.0);    /* minimum exposure time */
  fprintf(fptr,"0\n");         /* sync mode */
  GetParam(DC_FREQ, &param);
  if (param == 1.0F)
     fprintf(fptr,"%d\n", 50);
  else
     fprintf(fptr,"%d\n", 60);
  GetParam(DC_DAPROG, &param);
  fprintf(fptr,"%d\n", (SHORT)param);  /* DA mode number */
  GetParam(DC_I, &param);
  fprintf(fptr,"%d\n", (SHORT)param);
  fprintf(fptr,"%d\n",curvecount);
  GetParam(DC_K, &param);
  fprintf(fptr,"%d\n", (SHORT)param);
  fprintf(fptr,"12\n");                 /* data_type 8-16 bit, 12=32 bit */
  fprintf(fptr,"%d\n",pointcount);      /* number of data points */
  fprintf(fptr,"%g\n",0.0);             /* address data_start */
  GetParam(DC_PNTMODE, &param);
  fprintf(fptr,"%d\n",  (SHORT)param);  /* scan_mode (0=normal)  */
  GetParam(DC_X0, &param);
  fprintf(fptr,"%d\n", (SHORT)param);   /* number of first pixel */
  GetParam(DC_ACTIVEX, &param);
  fprintf(fptr,"%d\n",  (SHORT)param);  /* number of active pixels */
  GetParam(DC_DELTAX, &param);
  fprintf(fptr,"%d\n", (SHORT)param);   /* group size */
  fprintf(fptr,"%d\n", InitialMethod->Normalize); /* normalization mode */
  fprintf(fptr,"2\n");    /* disk drive (2=c:) */
  fprintf(fptr,"0\n");    /* file number */

  GetParam(DC_PLSR, &param);
  switch ((SHORT)param)
  {
     case 1:
        fprintf(fptr, "1211\n");
        break;
     case 2:
        fprintf(fptr, "1302\n");
        break;
     case 3:
        fprintf(fptr, "1303\n");
        break;
     default:
        fprintf(fptr, "0\n");
        break;
  }

  /* number of pulses */
  GetParam(DC_PTRIGNUM, &param);
  fprintf(fptr,"%d\n", (SHORT)param);

  /* unclear if this is best... */
  GetParam(DC_PDELAY, &param);
  if (param == 0.0F)
     tempexp = tempcoeff = 0;
  else
  {
     tempexp = (int) log10(param) - PRECISION_VAL;
     tempcoeff = (int) ((double) param *
                 pow((double)10.0,(double) - tempexp));
  }
  fprintf(fptr,"%d\n",tempcoeff);   /* pulser delay */
  fprintf(fptr,"%d\n",tempexp);

  GetParam(DC_PWIDTH, &param);
  if (param == 0.0F)
     tempexp = tempcoeff = 0;
  else
  {
     tempexp = (int) log10(param) - PRECISION_VAL;
     tempcoeff = (int) ((double) param *
                  pow((double)10.0,(double) - tempexp));
  }
  fprintf(fptr,"%d\n",tempcoeff);   /* pulser width */
  fprintf(fptr,"%d\n",tempexp);
  /* good past here */
  fprintf(fptr,"0\n");     /* delay 1302 */
  fprintf(fptr,"0\n");     /* delay 1303 */
  fprintf(fptr,"0\n");     /* width 1303 */
  GetParam(DC_PDELINC, &param);
  fprintf(fptr,"%g\n", param); /* delay increment 1303 */
  fprintf(fptr,"0\n");     /* clock cycles */
  GetParam(DC_PTRIGTRSH, &param);
  fprintf(fptr,"%d\n", (SHORT)param);
  fprintf(fptr,"0\n",temphdr.XData.XUnits);      /* calibration */

  if (InitialMethod->CalibCoeff[0][3] != (float) 0.0)
     tempcoeff = 4;
  else
  {
     if (InitialMethod->CalibCoeff[0][2] != (float) 0.0)
        tempcoeff = 3;
     else
     {
        if (InitialMethod->CalibCoeff[0][1] != (float) 0.0)
           tempcoeff = 2;
        else
           tempcoeff = 1;
     }
  }
  fprintf(fptr,"%d\n",tempcoeff);     /* degree of x fit */
  fprintf(fptr,"%g\n",InitialMethod->CalibCoeff[0][0]);
  fprintf(fptr,"%g\n",InitialMethod->CalibCoeff[0][1]);
  fprintf(fptr,"%g\n",InitialMethod->CalibCoeff[0][2]);
  fprintf(fptr,"%g\n",InitialMethod->CalibCoeff[0][3]);
  for (i=0; i<4; i++)                          /* change them back */
     InitialMethod->CalibCoeff[0][i] = (float) Change_CalCoeff[i];
  for (j=0; j<24; j++)             /* information for 24 spectral areas */
     fprintf(fptr,"0\n0\n");    /* first pixel num , num of pixels */
  fprintf(fptr,"\n");          /* trailing CR/LF */
  fprintf(fptr,"%c",0x1a);      /* EOF character */

  return ERROR_NONE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int ChangeVerifyCurveBlkProc(void)
{
  CHAR Name[DOSFILESIZE + 1];
  CHAR Path[DOSPATHSIZE + 1];
  USHORT BlkIndex;
  BOOLEAN specialFile = is_special_name(Current.FieldDataPtr);

  
  if (specialFile)
    {
    Path[0] = '\0';
    strcpy(Name, Current.FieldDataPtr);
    }
  else
    {
    /* expand the file block name and check to see if it is not a directory */
    if (ParsePathAndName(Path, Name, Current.FieldDataPtr) != 2)
      {
        error(ERROR_BAD_FILENAME, Current.FieldDataPtr);
        return 0;  /* flag error */
      }
    }

   /* find the block */
  if ((BlkIndex = SearchNextNamePath(Name, Path, &MainCurveDir, 0)) != (USHORT)-1)
    {
    strcpy(Current.FieldDataPtr, Path);
    strcat(Current.FieldDataPtr, Name);
    from_field= MainCurveDir.Entries[BlkIndex].StartIndex;
    count_field = MainCurveDir.Entries[BlkIndex].count;

    return FIELD_VALIDATE_SUCCESS;
    }
  else
    error(ERROR_OPEN, Name);

  return 0;
}

/****************************************************************************/
void GetCalib1460(CURVEDIR *pCurveDir,SHORT EntryIndex,USHORT CurveIndex,
                  USHORT number_of_points)
{
   ExtractCalPointsFromCurve(pCurveDir, EntryIndex, CurveIndex);

   if(verify_list_order() && (number_of_points >= 2))
         least_squares_fit();
}

/****************************************************************************/
ERR_OMA trans_oma4_oma1470(FILE* fptr, const char * fName,
                                  CURVEDIR *directory, int entry_num,
                                  int starting_curvenum,
                                  int curvecount, BOOLEAN KbChk)
{

  SHORT prefBuf = -1, bytes_read, datapoints, i,j;
  FLOAT * float_array = NULL, X, Y, param;
  OMA1470_REC * oma1470_ptr;
  CURVEHDR      curve_header_ptr;
  POMA1470_CURVESET CrvSetPtr;
  PPOLYTYPE CalPtr;
  ERR_OMA err = ERROR_NONE;

  if(! (oma1470_ptr = (OMA1470_REC *) calloc(1, sizeof(OMA1470_REC))))
    return error(ERROR_ALLOC_MEM);

  strncpy(oma1470_ptr->file_id_string,"M1470BIN 01.3 ", 14);
  strncpy(oma1470_ptr->label,
          directory->Entries[entry_num].descrip, LABEL_SZ1470);
  oma1470_ptr->label[81] = '\x1a';

  CrvSetPtr = &(oma1470_ptr->curveset);
  CalPtr    = &(CrvSetPtr->calib_data);

  CrvSetPtr->number_of_memories = (int) curvecount;
                                            /* detector type */
  GetParam(DC_DMODEL, &param);
  
  switch ((SHORT)param)
    {
    case 0:
       CrvSetPtr->detector_type = 0;
    break;
    case 1:
    case 2:
    case 3:
       CrvSetPtr->detector_type = 1461 + (SHORT)param;
    break;
    default:
       CrvSetPtr->detector_type = 1464;
    break;
    }

  /* X pixel num */
  GetParam(DC_ACTIVEX, &param);
  CrvSetPtr->detector_length = (SHORT)param;
  /* temperatue    */
  GetParam(DC_DTEMP, &param);
  CrvSetPtr->detector_temperature = (SHORT)param;
  /* exposure time */
  GetParam(DC_ET, &param);
  CrvSetPtr->exposure_time = param;
  GetParam(DC_MINET, &param);
  CrvSetPtr->minimum_exposure_time = (double) param;
  GetParam(DC_CONTROL, &param);
  CrvSetPtr->synchronization_mode =  (UCHAR)param;
  GetParam(DC_FREQ, &param);
  CrvSetPtr->line_frequency = (param == 1.0F) ? (UCHAR)50 : (UCHAR)60;
  GetParam(DC_DAPROG, &param);
  CrvSetPtr->DA_mode_number = (UCHAR)param;
  /* scans */
  GetParam(DC_I, &param);
  CrvSetPtr->number_of_scans = (SHORT)param;
  GetParam(DC_J, &param);
  CrvSetPtr->number_of_memories_again = (SHORT)param;
  /* ignore scans */
  GetParam(DC_K, &param);
  CrvSetPtr->number_of_ignore_scans = (SHORT)param;
  CrvSetPtr->datatype = 10;     /* 8=int, 10=float, 12=long */
  GetParam(DC_ACTIVEX, &param);
  CrvSetPtr->number_active_pixels = (SHORT)param;
  GetParam(DC_DELTAX, &param);
  CrvSetPtr->groupsize_norm_scan = (SHORT)param;
  CrvSetPtr->data_norm_mode = InitialMethod->Normalize;
  CrvSetPtr->ADP = 0;  /* A/D precision 1=14 bit */

  /*************************************************************************/
  CrvSetPtr->y_axis = (char)0X10;  /* temp patch - force data to æW/nm.cmý */
  /*************************************************************************/

  GetParam(DC_PLSR, &param);
  switch ((SHORT)param)
    {
    case 1:
      CrvSetPtr->pulser_type = 1211;
      break;
    case 2:
      CrvSetPtr->pulser_type = 1302;
      break;
    case 3:
      CrvSetPtr->pulser_type = 1303;
    default:
      CrvSetPtr->pulser_type = 0;
      break;
    }

  for (i=0; i<curvecount;i++)
    {
    if (KbChk && kbhit() && (getch() == ESCAPE))
      goto the_end;
    err = ReadTempCurvehdr(directory, entry_num, starting_curvenum + i,
                           & curve_header_ptr);
    if(err)
      goto the_end;
    if(! i)      /* if first time through, get datapoint count, allocate */
      {          /* float array, get calibration, write out header */
      datapoints = curve_header_ptr.pointnum;
      CrvSetPtr->channel_max = datapoints - 1;
      if(! (float_array= malloc(sizeof(float) * (datapoints+1))))
        {
        free(oma1470_ptr);
        return error(ERROR_ALLOC_MEM);
        }
      for (j=0; j<4; j++)             /* Save cal coeff's. */
        Change_CalCoeff[j] = InitialMethod->CalibCoeff[0][j];
                                      /* Recal on this curve. */
      GetCalib1460(&MainCurveDir,entry_num,starting_curvenum,datapoints);
      for (j=0; j<4; j++)
        {
        CalPtr->a[j] = CalCoeff[j];   /* Store the 1470 coef's
                                      /* change the system back */
        InitialMethod->CalibCoeff[0][j] = (float) Change_CalCoeff[j];
        }
      CalPtr->n = (CalPtr->a[3] || CalPtr->a[2]) ? 3 : 1;
      if (!CalPtr->a[1])
        {
        CalPtr->a[0] = 0.0;
        CalPtr->a[1]++;
        }
      CalPtr->xaxis =
        (CHAR)((!CalPtr->a[0] && (CalPtr->a[1] == 1.0)) ? 0 : 1);

      bytes_read = fwrite((char *) oma1470_ptr,1,sizeof(OMA1470_REC),fptr);
      if (bytes_read != sizeof(OMA1470_REC))  /* write out 1470 header */
        {
        free(float_array);
        free(oma1470_ptr);
        return error(ERROR_WRITE, fName);
        }
      }
    for (j=0; j < datapoints; j++)
      {
      if (KbChk && kbhit() && (getch() == ESCAPE))
        goto the_end;
      GetDataPoint(directory,entry_num,starting_curvenum+i,j,&X,&Y,
                     FLOATTYPE, &prefBuf);
      float_array[j] = Y;
      }
      float_array[j] = 0.0F;
                             /* send y data directly to output file */
    bytes_read = fwrite((float *) float_array,sizeof(float),
                          datapoints+1,fptr);
    if (bytes_read != (datapoints+1))
      {
      free(oma1470_ptr);
      free(float_array);
      return error(ERROR_WRITE, fName);
      }
  }
the_end:   
  free(oma1470_ptr);
  free(float_array);
  return err;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int ChangeVerifyCurveBlk()
{
   int ReturnVal = ChangeVerifyCurveBlkProc();
   int OldIndex = Current.Form->field_index;

   draw_form();
   Current.Form->field_index = OldIndex;
   format_and_display_field(FALSE);

   return ReturnVal;
}

/* Write data from memory to disk in TCL 32 bit format.
****************************************************************************/
ERR_OMA trans_TCL_32(FILE* fptr, const char * fName,
                            CURVEDIR *CvDir,
                            SHORT Blk, SHORT StCurve, SHORT Curves,
                            BOOLEAN KbChk)
{
  SHORT i, j, Points, BufNum = 0;
  FLOAT Param, X, Y, *FBuff;
  BOOLEAN Done = FALSE;
  CURVEHDR CvHdr;
  ERR_OMA err = ERROR_NONE;
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
