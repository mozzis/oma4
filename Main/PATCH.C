#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>  /* longjump, setjmp */
#include <float.h>   /* _fpreset */
#include <math.h>    /* _fpreset */
#include <signal.h>  /* signal */

#include "eggtype.h"
#include "crvheadr.h"
#include "crventry.h"
#include "curvbufr.h"
#include "omameth.h"
#include "detinfo.h"
#include "oma4driv.h"

struct detector_info
{
   USHORT         Length;
   DET_SETUP      RunSetup;
   USHORT         XPixelGroups;
   USHORT         YPixelGroups;
   USHORT         TriggerGroups;
   USHORT         ETGroups;
   PVOID          GroupTables;
} ;

#define DET_INFO_BASE_LENGTH offsetof( DET_INFO, GroupTables )

static CURVEHDR     cvhdr;
static METHDR       methdr;
static CURVEBUFFER  cvbuf;
static CURVE_ENTRY  cventry;
static char far sbuf[80];
static int fp_error_flag = FALSE;
static jmp_buf  reentry_context;
static BOOLEAN batch = FALSE, quiet = FALSE;
static FILE * infile, *outfile;
static char outname[(8+1+3+64)];
static char inname[(8+1+3+64)];


void floating_point_error_trap()
{
  _fpreset();
  fprintf(stderr, "Floating point error!\n");
  fp_error_flag = TRUE;

  longjmp(reentry_context, 0);
}

void pfint(char far * str, int val)
{
  strcpy(sbuf, str);
  strcat(sbuf,"      %4u  %4x\n");
  printf(sbuf, val, val);

}
  
void pfstr(char far * str, char far * sval)
{
  strcpy(sbuf, str);
  strcat(sbuf," %s\n");
  printf(sbuf, sval);

}
void prtmethdr(METHDR * methdr)
{
  if (!quiet)
    {
    pfstr("  FileTypeID..........", methdr->FileTypeID);
    pfint("  StructureVersion....", methdr->StructureVersion);
    pfstr("  Description.........", methdr->Description);
    pfint("  FileCurveNum........", methdr->FileCurveNum);
    pfint("  SoftwareVersion.....", methdr->SoftwareVersion);
    }
}

int fatal(char * msg, char * arg, int errnum)
{
  int i;

  fprintf(stderr, "%s%s\n", msg, arg);
  if (batch)
    {
    if (methdr.DetNumber)
      for (i=0; i < methdr.DetNumber; i++)
      {
      free(methdr.DetInfo[i].GroupTables);
      methdr.DetInfo[i].GroupTables = NULL;
      }
  
    if (methdr.DetInfo)
      {
      free(methdr.DetInfo);
      methdr.DetInfo = NULL;
      }
    
    if (infile)
      fclose(infile);

    if (outfile)
      {
      fclose(outfile);
      remove(outname);
      }

    infile = outfile = NULL;
    return(errnum);
    }
  else
    {
    if (outfile)
      {
      fclose(outfile);
      remove(outname);
      }
    exit(errnum);
    }
}

void useage(void)
{
  printf("Patch version 1.1\n"
         "Update older OMAIII data files\n"
         "(Only works on OMA2000 ver 1.8 or later files)\n"
         "\n"
         "Useage:"
         "        PATCH <filespec> [-B] [-Q]\n"
         "<filespec> may contain wildcards\n"
         "options:\n"
         "         -B - batch, after error do next file\n"
         "         -Q - quiet, don't print method info\n"
         "\n"
         "This version of PATCH just forces the MemData field\n"
         "of each curve header to be 0 (=FALSE)\n");
  exit(-1);       
}

int main(int argc, char * * argv)
{
  char * dotptr;
  ULONG TestLen;
  USHORT TableSz, j;
  SHORT i, k;
  void * Ybuffer;
  float *Xbuffer;
  int dtype;

  if (argc < 2)
    useage();

  if (!((Ybuffer = malloc(4100)) && (Xbuffer = malloc(4100))))
    fatal("Not enough memory for data buffers", " ", -1);

  /* Scan arguments for errors */

  for (k = 1; k < argc; k++)
    {
    if (argv[k][0] == '-' || argv[k][0] == '/')
      {
      if (argv[k][1] == 'Q' || argv[k][1] == 'q')
        continue;
      else if (argv[k][1] == 'B' || argv[k][1] == 'b')
        continue;
      else
        useage();
      }
    }

  setjmp(reentry_context);
  signal(SIGFPE, floating_point_error_trap);

  for (k = 1; k < argc; k++)
    {
    if (argv[k][0] == '-' || argv[k][0] == '/')
      {
      if (argv[k][1] == 'Q' || argv[k][1] == 'q')
        {
        quiet = TRUE;
        continue;
        }
      else if (argv[k][1] == 'B' || argv[k][1] == 'b')
        {
        batch = TRUE;
        continue;
        }
      }
    
    strcpy(inname, argv[k]);
    fprintf(stderr,"%s -> ",inname);
    if (!(infile = fopen(inname, "rb")))
      {
      if (fatal("Can't open file for input:", inname, 2))
        continue;
      }

    strcpy(outname, inname);
    if (!(dotptr = strchr(outname, '.')))
      dotptr = &outname[strlen(outname)];
    strcpy(dotptr, ".FAT");
    fprintf(stderr,"%s\n",outname);

    if (!(outfile = fopen(outname, "wb")))
      {
      if (fatal("Can't open file for output:", outname, 3))
        continue;
      }

    if (fseek(infile, 0L, SEEK_SET))
      {
      if (fatal("Seek error in file", inname, 4))
        continue;
      }

    if (fread(&(methdr.FileTypeID), HDR_BASE_LENGTH, 1, infile) != 1)
      {
      if (fatal("Error reading methdr in file:", inname, 5))
        continue;
      }

    if (fwrite(&(methdr.FileTypeID), HDR_BASE_LENGTH, 1, outfile) != 1)
      {
      if (fatal("Error writing methdr to file:", outname, 6))
        continue;
      }

    if (methdr.StructureVersion < 12)
      {

      if (fatal("File older than version 1.81 does not need converting:", inname, 7))
        continue;
      }

    TestLen = HDR_BASE_LENGTH;

    if (methdr.DetNumber)
      {
      if ((methdr.DetInfo = calloc(1, sizeof(DET_INFO) * methdr.DetNumber)) == NULL)
        {
        if (fatal("No room for detector info", " ", 8))
          continue;
        }

      for (i=0; i < methdr.DetNumber; i++)
        {
        methdr.DetInfo[i].GroupTables = NULL;

        if (fread(&(methdr.DetInfo[i]), DET_INFO_BASE_LENGTH, 1, infile) != 1)
          if (fatal("Can't read detector info from file:", inname, 9))
            {
            i = methdr.DetNumber + 1;
            continue;
            }

        if (fwrite(&(methdr.DetInfo[i]), DET_INFO_BASE_LENGTH, 1, outfile) != 1)
          if (fatal("Can't write detector info to file:", outname, 10))
            {
            i = methdr.DetNumber + 1;
            continue;
            }

        TestLen += DET_INFO_BASE_LENGTH;

        TableSz = (sizeof(SHORT) * 2 * (methdr.DetInfo[i].XPixelGroups)) +
                  (sizeof(SHORT) * 2 * (methdr.DetInfo[i].YPixelGroups)) +
                  (sizeof(SHORT) * 2 * (methdr.DetInfo[i].TriggerGroups)) ;

        if (TableSz)
          {
          if(! (methdr.DetInfo[i].GroupTables = calloc(1, TableSz)))
            if (fatal("No room for group tables", " ", 11))
              {
              i = methdr.DetNumber + 1;
              continue;
              }
          if (fread(methdr.DetInfo[i].GroupTables, TableSz, 1, infile) != 1)
            if (fatal("Can't read group tables from file:", inname, 12))
              {
              i = methdr.DetNumber + 1;
              continue;
              }
          if (fwrite(methdr.DetInfo[i].GroupTables, TableSz, 1, outfile) != 1)
            if (fatal("Can't write group tables to file:", outname, 13))
              {
              i = methdr.DetNumber + 1;
              continue;
              }
          TestLen += TableSz;
          }
        }
      }
    else
      methdr.DetInfo = NULL;

    if (TestLen != methdr.Length )
      {
      int contch;

      fprintf(stderr, "%s is corrupt\n", inname);

      if (!batch)
        {
        fprintf(stderr, "Press Y to continue or N to abort");
        contch = toupper((int)(getch()));
        if (contch != 'Y')
          exit(14);
        else
          fprintf(stderr,"\n");
        }
      }

    prtmethdr(&methdr);
 
    for (j = 0; j < methdr.FileCurveNum; j++)
      {
      if (fread(&cvhdr, sizeof(CURVEHDR), 1, infile) != 1)
        {
        if (fatal("Error reading curvehdr in file:", inname, 15))
          continue;
        }

      cvhdr.MemData = 0; /* This is the object of the program! */

      if (fwrite(&cvhdr, sizeof(CURVEHDR), 1, outfile) != 1)
        {
        if (fatal("Error writing curvehdr to file:", outname, 16))
          continue;
        }

      dtype = cvhdr.DataType & 0x0F;

      if (fread(Ybuffer, dtype, cvhdr.pointnum, infile) != cvhdr.pointnum)
        {
        if (fatal("Error reading curve Y data", " ", 12))
          continue;
        }
      if (fread(Xbuffer, sizeof(float), cvhdr.pointnum, infile) != cvhdr.pointnum)
        {
        if (fatal("Error reading curve X data" ," ", 13))
          continue;
        }

      if (fwrite(Ybuffer, dtype, cvhdr.pointnum, outfile) != cvhdr.pointnum)
        {
        if (fatal("Error writing Y data to file:", outname, 17))
          {
          j = methdr.FileCurveNum + 1;
          continue;
          }
        }

      if (fwrite(Xbuffer, dtype, cvhdr.pointnum, outfile) != cvhdr.pointnum)
        {
        if (fatal("Error writing X data to file:", outname, 18))
          continue;
        }
      }

    for (i=0; i < methdr.DetNumber; i++)
      {
      free(methdr.DetInfo[i].GroupTables);
      methdr.DetInfo[i].GroupTables = NULL;
      }
    free(methdr.DetInfo);
    methdr.DetInfo = NULL;
    fclose(infile);
    fclose(outfile);
    infile = outfile = NULL;
    }
  return(0);
}
