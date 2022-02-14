/***************************************************************************/
/*  di_util.c                                                              */
/*                *** OMA 35 Version ***                                   */
/*
/
/  $Header:   J:/logfiles/oma4000/main/di_util.c_v   0.13   12 Jan 1992 10:50:48   cole  $
/  $Log:   J:/logfiles/oma4000/main/di_util.c_v  $
/
/*  copyright (c) 1988, EG&G Instruments Inc.                              */
/*                                                                         */
/***************************************************************************/
  
#ifdef PROT
#define INCL_DOSMEMMGR
#define INCL_NOPM
#include <os2.h>
#endif
  
#include <direct.h> // chdir()
#include <string.h>
#include <stdlib.h>
#include <dos.h>   // _dos_getdrive()
#include <time.h>
#include <sys\types.h>
#include <sys\timeb.h>
  
#include "di_util.h"
#include "crventry.h"   // DOSPATHSIZE, DOSFILESIZE

char CurrDir[_MAX_DRIVE + _MAX_DIR];
#define MAXFILESTRLEN 12
  
#ifdef DEBUG

/***************************************************************************/
/*  function:  Printf the contents of memory from addr to                  */
/*             addr + length to the device or file pointed to by stream.   */
/*             Both hex and character representations are shown.           */
/*                                                                         */
/*  variables: addr - starting address of memory area                      */
/*             length - number of bytes to print out                       */
/*             stream - file or device to send stuff to                    */
/*                                                                         */
/*  returns: 0                                                             */
/*                                                                         */
/***************************************************************************/
  
SHORT mem_dump(PVOID addr, ULONG length, FILE *stream)
{
  ULONG i, j;
  ULONG empty_space;
  PUCHAR byte;

  byte = addr;

  for (i=0; i<length; i++)
    {
    if (i % 16 == 0)
      {
      if (i != 0)
        {
        for (j=i-16; j < i; j++)
          {
          if (byte[j] >= 0x20)
            fprintf(stream, "%c", byte[j]);
          else fprintf(stream, ".");
          }
        }
      fprintf(stream, "\n%+4Xx   ", i + (ULONG) addr);
      }
    if (byte[i] >= 0x10)
      fprintf(stream, "%+2X ", (USHORT) byte[i]);
    else
      fprintf(stream, "0%X ", (USHORT) byte[i]);
    }

  /* fill with spaces to end of hex line area */
  empty_space = 16 - (i%16);
  for (j=0; j<empty_space; j++)
    fprintf(stream, "   ");

  /* put down characters */
  for (j=i - (i % 16); j < i; j++)
    {
    if (byte[j] >= 0x20)
      fprintf(stream, "%c", byte[j]);
    else fprintf(stream, ".");
    }
  fprintf(stream, "\n");
  fflush(stream);
  return 0;
}
 
#endif      /* end of debug code */
  
/***************************************************************************/
/*  function:  Does nothing. Use for a marker for Codeview breakpoints.    */
/***************************************************************************/
  
VOID nullproc()
{
  return;
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Wait for time to go by and then return.                     */
/*                                                                         */
/***************************************************************************/
  
VOID SysWait(ULONG msec)
{
  struct timeb time1, time2;
  BOOLEAN Done;

  /* get current time */
  ftime(&time1);
  /* calculate end time */
  msec += time1.millitm;
  /* separate out the milliseconds in case the time goes past a second*/
  time1.millitm = (unsigned) (msec % 1000L);
  time1.time += msec / 1000L;
  do
    {
    ftime(&time2);
    if (time2.time < time1.time)
      Done = FALSE;
    else if ((time2.time == time1.time) &&
      (time2.millitm < time1.millitm))
      Done = FALSE;
    else
      Done = TRUE;
    }
  while (! Done);
}
  
/***************************************************************************/
/*  function:  Compare 2 float values.  Used with qsort() to sort an       */
/*             array of floats in descending order.                        */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to floats to be compared      */
/*                                                                         */
/*   Returns:     <0 - if elem2 < elem1                                    */
/*                =0 - if elem2 = elem1                                    */
/*                >0 - if elem2 > elem1                                    */
/*                                                                         */
/***************************************************************************/
  
int InverseFloatCompare(float *elem1, float *elem2)
{
 return - FloatCompare(elem1, elem2);
}
  
/***************************************************************************/
/*  function:  Compare 2 float values.  Used with qsort() to sort an       */
/*             array of floats in ascending order.                         */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to floats to be compared      */
/*                                                                         */
/*   Returns:     <0 - if elem1 < elem2                                    */
/*                =0 - if elem1 = elem2                                    */
/*                >0 - if elem1 > elem2                                    */
/*                                                                         */
/***************************************************************************/
  
int FloatCompare(float *elem1, float *elem2)
{
  float Temp;

  Temp = *elem1 - *elem2;
  if (Temp > (float)0.0)
    return 1;
  if (Temp < (float)0.0)
    return -1;
  else
    return 0;
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 unsigned char values.  Used with qsort() to sort  */
/*             an array of integers in descending order.                   */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to unsigned chars to be       */
/*                compared                                                 */
/*                                                                         */
/*   Returns:     <0 - if elem2 < elem1                                    */
/*                =0 - if elem2 = elem1                                    */
/*                >0 - if elem2 > elem1                                    */
/*                                                                         */
/***************************************************************************/
  
int InverseUcharCompare(unsigned char *elem1, unsigned char *elem2)
{
  return - UcharCompare(elem1, elem2);
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 unsigned char values.  Used with qsort() to sort  */
/*             an array of unsigned chars in ascending order.              */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to unsigned chars to be       */
/*                compared                                                 */
/*                                                                         */
/*   Returns:     <0 - if elem1 < elem2                                    */
/*                =0 - if elem1 = elem2                                    */
/*                >0 - if elem1 > elem2                                    */
/*                                                                         */
/***************************************************************************/
  
int UcharCompare(unsigned char *elem1, unsigned char *elem2)
{
  return ((int)((USHORT)*elem1 - (USHORT)*elem2));
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 integer values.  Used with qsort() to sort an     */
/*             array of integers in descending order.                      */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to integers to be compared    */
/*                                                                         */
/*   Returns:     <0 - if elem2 < elem1                                    */
/*                =0 - if elem2 = elem1                                    */
/*                >0 - if elem2 > elem1                                    */
/*                                                                         */
/***************************************************************************/
  
int InverseIntCompare(int *elem1, int *elem2)
{
  return - IntCompare(elem1, elem2);
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 integer values.  Used with qsort() to sort an     */
/*             array of integers in ascending order.                       */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to integers to be compared    */
/*                                                                         */
/*   Returns:     <0 - if elem1 < elem2                                    */
/*                =0 - if elem1 = elem2                                    */
/*                >0 - if elem1 > elem2                                    */
/*                                                                         */
/***************************************************************************/
  
int IntCompare(int *elem1, int *elem2)
{
  return (*elem1 - *elem2);
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 unsigned integer values.  Used with qsort() to    */
/*             sort an array of unsigned integers in descending order.     */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to unsigned integers to be    */
/*                compared                                                 */
/*                                                                         */
/*   Returns:     <0 - if elem2 < elem1                                    */
/*                =0 - if elem2 = elem1                                    */
/*                >0 - if elem2 > elem1                                    */
/*                                                                         */
/***************************************************************************/
  
int InverseUintCompare(unsigned *elem1, unsigned *elem2)
{
  return - UintCompare(elem1, elem2);
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 unsigned int values.  Used with qsort() to sort an*/
/*             array of unsigned ints in ascending order.                  */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to unsigned integers to be    */
/*                compared                                                 */
/*                                                                         */
/*   Returns:     <0 - if elem1 < elem2                                    */
/*                =0 - if elem1 = elem2                                    */
/*                >0 - if elem1 > elem2                                    */
/*                                                                         */
/***************************************************************************/
  
int UintCompare(unsigned *elem1, unsigned *elem2)
{
  return ((int)(*elem1 - *elem2));
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 long values.  Used with qsort() to sort an        */
/*             array of longs in descending order.                         */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to longs to be compared       */
/*                                                                         */
/*   Returns:     <0 - if elem2 < elem1                                    */
/*                =0 - if elem2 = elem1                                    */
/*                >0 - if elem2 > elem1                                    */
/*                                                                         */
/***************************************************************************/
  
int InverseLongCompare(long *elem1, long *elem2)
{
  return - LongCompare(elem1, elem2);
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 long values.  Used with qsort() to sort an        */
/*             array of longs in ascending order.                          */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to longs to be compared       */
/*                                                                         */
/*   Returns:     <0 - if elem1 < elem2                                    */
/*                =0 - if elem1 = elem2                                    */
/*                >0 - if elem1 > elem2                                    */
/*                                                                         */
/***************************************************************************/
  
int LongCompare(long *elem1, long *elem2)
{
  long Temp;

  Temp = *elem1 - *elem2;
  if (Temp > 0)
    return 1;
  if (Temp < 0)
    return -1;
  else
    return 0;
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 unsigned long values.  Used with qsort() to sort  */
/*             an array of unsigned longs in descending order.             */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to unsigned longs to be       */
/*                compared                                                 */
/*                                                                         */
/*   Returns:     <0 - if elem2 < elem1                                    */
/*                =0 - if elem2 = elem1                                    */
/*                >0 - if elem2 > elem1                                    */
/*                                                                         */
/***************************************************************************/
  
int InverseUlongCompare(unsigned long *elem1, unsigned long *elem2)
{
  return - UlongCompare(elem1, elem2);
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 unsigned long values.  Used with qsort() to sort  */
/*             an array of unsigned longs in ascending order.              */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to unsigned longs to be       */
/*                compared                                                 */
/*                                                                         */
/*   Returns:     <0 - if elem1 < elem2                                    */
/*                =0 - if elem1 = elem2                                    */
/*                >0 - if elem1 > elem2                                    */
/*                                                                         */
/***************************************************************************/
  
int UlongCompare(unsigned long *elem1, unsigned long *elem2)
{
  long Temp;

  Temp = (long)*elem1 - (long)*elem2;
  if (Temp > 0)
    return 1;
  if (Temp < 0)
    return -1;
  else
    return 0;
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 double values.  Used with qsort() to sort an      */
/*             array of doubles in descending order.                       */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to doubles to be compared     */
/*                                                                         */
/*   Returns:     <0 - if elem2 < elem1                                    */
/*                =0 - if elem2 = elem1                                    */
/*                >0 - if elem2 > elem1                                    */
/*                                                                         */
/***************************************************************************/
  
int InverseDoubleCompare(double *elem1, double *elem2)
{
  return - DoubleCompare(elem1, elem2);
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 double values.  Used with qsort() to sort an      */
/*             array of doubles in ascending order.                        */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to doubles to be compared     */
/*                                                                         */
/*   Returns:     <0 - if elem1 < elem2                                    */
/*                =0 - if elem1 = elem2                                    */
/*                >0 - if elem1 > elem2                                    */
/*                                                                         */
/***************************************************************************/
  
int DoubleCompare(double *elem1, double *elem2)
{
  double Temp;

  Temp = *elem1 - *elem2;
  if (Temp > 0.0)
    return 1;
  if (Temp < 0.0)
    return -1;
  else
    return 0;
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 strings.  Used with qsort() to sort an            */
/*             array of strings in descending order.                       */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to strings to be compared     */
/*                                                                         */
/*   Returns:     <0 - if elem2 < elem1                                    */
/*                =0 - if elem2 = elem1                                    */
/*                >0 - if elem2 > elem1                                    */
/*                                                                         */
/***************************************************************************/
  
int InverseStrCompare(CHAR * *elem1, CHAR * *elem2)
{
  return (strcmp(*elem2, *elem1));
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Compare 2 string values.  Used with qsort() to sort an      */
/*             array of strings in ascending order.                        */
/*                                                                         */
/*   Variables:   elem1 and elem2 - pointers to strings to be compared     */
/*                                                                         */
/*   Returns:     <0 - if elem1 < elem2                                    */
/*                =0 - if elem1 = elem2                                    */
/*                >0 - if elem1 > elem2                                    */
/*                                                                         */
/***************************************************************************/
  
int StrCompare(CHAR * *elem1, CHAR * *elem2)
{
  return (strcmp(*elem1, *elem2));
}
 
/***************************************************************************/
/*                                                                         */
/*  function:  Translate from an incomplete file name to a complete path   */
/*             and file name. Accepts a directory name with an ending '\'. */
/*                                                                         */
/*   Variables:   pcin - input. incomplete file name string.               */
/*                pcout - Output. Complete path and filename.              */
/*                                                                         */
/*   Returns:  0 -- pcIn had invalid drive or directory.                   */
/*             1 -- pcIn was empty, directory, or had no filename.         */
/*             2 -- pcOut points to drive, full dir, and file name.        */
/*                                                                         */
/*    Side effects: Changes current drive and directory per pcIn string.   */
/*                                                                         */
/***************************************************************************/
  
SHORT ParseFileName (CHAR *pcOut, CHAR *pcIn)
{
  CHAR   *pcLastSlash, *pcFileOnly;
  USHORT usDriveNum, usDirLen = 64;
  #ifndef PROT
  USHORT Temp;
  USHORT usDriveMap;
  #endif
  
  strupr (pcIn);

  /*------------------------------------
    If input string is empty, return 1
    ------------------------------------*/
  
  if (pcIn [0] == '\0')
    {
    /* getcwd gives file and full directory */
    getcwd(pcOut, usDirLen);
    return 1;
    }

  /*----------------------------------------------
    Get drive from input string or current drive
    ----------------------------------------------*/
  
  if (pcIn [1] == ':')
    {
    usDriveNum = pcIn [0] - '@';
    #ifdef PROT
    if (DosSelectDisk (pcIn [0] - '@'))
      return 0;
    #else
    _dos_setdrive(usDriveNum, &usDriveMap);

    /* check to see if drive change worked */
    _dos_getdrive(&Temp);
    if (usDriveNum != Temp)
      return 0;
    #endif
    pcIn += 2;
    }

  getcwd(CurrDir, _MAX_DIR + _MAX_DRIVE);

  /* check to see if just a directory */
  if (! chdir (pcIn))
    {
    /* getcwd gives file and full directory */
    getcwd(pcOut, usDirLen);
    return 1;
    }

  /*----------------------------------------------------------
    Search for last backslash.  If none, get current directory.
    ----------------------------------------------------------*/
  
  if (NULL == (pcLastSlash = strrchr (pcIn, '\\')))
    {

    /*----------------------------------------------------
      Get current dir & attach input filename
      ----------------------------------------------------*/
  
    if (! getcwd(pcOut, usDirLen))
      return 0; /* error reading directory */

    if (strlen(pcIn) > MAXFILESTRLEN)
      return 0;

    if (*(pcOut + strlen (pcOut) - 1) != '\\')
      strcat(pcOut++, "\\");

    strcat(pcOut, pcIn);
    return 2;
    }

  /*-------------------------------------------------------
    If the only backslash is at beginning, change to root
    -------------------------------------------------------*/
  
  if (pcIn == pcLastSlash)
    {
    chdir ("\\");

    getcwd(pcOut, usDirLen);
    pcOut = &(pcOut[strlen(pcOut)]);

    if (pcIn [1] == '\0')
      {
      chdir (CurrDir);
      return 1;
      }

    strcpy(pcOut, pcIn + 1);
    chdir (CurrDir);
    return 2;
    }

  /*------------------------------------------------------
    Attempt to change directory -- Get current dir if OK
    ------------------------------------------------------*/
  
  *pcLastSlash = '\0';

  if (chdir (pcIn))
    {
    *pcLastSlash = '\\';
    return 0;
    }

  *pcLastSlash = '\\';

  getcwd(pcOut, usDirLen);
  pcOut = &(pcOut[strlen(pcOut)]);

  /*-------------------------------
    Append input filename, if any
    -------------------------------*/
  
  pcFileOnly = pcLastSlash + 1;

  chdir (CurrDir);

  if (*pcFileOnly == '\0')
    return 1;

  if (strlen(pcFileOnly) > MAXFILESTRLEN)
    return 0;

  if (*(pcOut + strlen (pcOut) - 1) != '\\')
    strcat(pcOut++, "\\");

  strcat(pcOut, pcFileOnly);

  if (strstr(pcOut, " "))
    return 0;

  return 2;
}
  
/**************************************************************************/  
/*                                                                        */
/* Requires:  Path - Output. Preallocated (at least DOSPATHSIZE + 1 bytes)*/
/*                   full pathname, with ending '\\'                      */
/*            Name - OutPut. Preallocated (at least DOSFILESIZE + 1 bytes)*/
/*                   file name buffer.                                    */
/*            BufIn - Filename spec string to be expanded and separated   */
/*                                                                        */
/*   Returns:    same as ParseFileName                                    */
/*   Returns:  0 -- BufIn had invalid drive or directory.                 */
/*             1 -- BufIn was empty, directory, or had no filename.       */
/*             2 -- good file name.                                       */
/*                                                                        */
/*       Changed to give out a good path ending in '\'                    */
/*       if ParseFileName returns 1, Name will be set to a null string    */
/*                                                                        */
/**************************************************************************/  
SHORT ParsePathAndName(CHAR *PathOut, CHAR *NameOut, CHAR *BufIn)
{
  CHAR Buffer[DOSPATHSIZE + DOSFILESIZE + 1];
  CHAR *TempPos;
  SHORT PReturn;
  SHORT i;

  /* expand the file block name and check to see if it is not a directory */
  if ((PReturn = ParseFileName(Buffer, BufIn)) == 0)
    return PReturn;  /* flag error */

  /* capitalize the string */
  strupr(Buffer);
  i=-1;
  do
    {
    i++;
    PathOut[i] = Buffer[i];
    }
  while ((i<=DOSPATHSIZE) && (Buffer[i] != '\0'));

  /* done here if no file name given */
  if (PReturn == 1)
    {
    if (PathOut[strlen(PathOut) - 1] != '\\')
      {
      /* put in the ending '\\' */
      TempPos = strchr(PathOut, '\0');
      *TempPos = '\\';
      TempPos[1] = '\0';
      }
    /* make name string a null string */
    NameOut[0] = '\0';
    return PReturn;
    }

  /* put a null to terminate the path */
  TempPos = strrchr(PathOut, '\\') + 1;
  *TempPos = '\0';

  /* find the end of the path */
  TempPos = strrchr(Buffer, '\\') + 1;
  /* get a copy of the file name */
  i=-1;
  do
    {
    i++;
    NameOut[i] = TempPos[i];
    }
  while ((i<=DOSFILESIZE) && (TempPos[i] != '\0'));
  /* make sure that it is null ended */
  NameOut[DOSFILESIZE] = '\0';

  return PReturn;
}
  
/***************************************************************************/
/*  function:  Gets the current time in seconds for later calculation of   */
/*             elapsed time                                                */
/*                                                                         */
/*   Variables:   StartTime - Time at start of stopwatch in seconds.       */
/*                                                                         */
/*   Returns: Value of StartTime.                                          */
/*                                                                         */
/***************************************************************************/
  
DOUBLE StartStopWatch(PDOUBLE StartTime)
{
  struct timeb tbStartTime;

  /* get current time */
  ftime(&tbStartTime);

  /* convert to floating point seconds */
  *StartTime = (DOUBLE)tbStartTime.time +
    ((DOUBLE) tbStartTime.millitm / 1000.0);

  return *StartTime;
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Get the elapsed time since StartTime.                       */
/*                                                                         */
/*   Variables:   StartTime - Time at start of stopwatch in seconds.       */
/*                                                                         */
/*   Returns: Elapsed time in seconds from StartTime.                      */
/*                                                                         */
/***************************************************************************/
  
DOUBLE StopStopWatch(DOUBLE StartTime)
{
  struct timeb tbStopTime;

  /* get current time */
  ftime(&tbStopTime);

  /* convert to floating point seconds and subtract the starting time */
  return (((DOUBLE)tbStopTime.time +
    ((DOUBLE) tbStopTime.millitm / 1000.0)) - StartTime);
  
}
  
/***************************************************************************/
/*                                                                         */
/*  function:  Strip leading +'s and 0's and trailing 0's from a floating  */
/*             point string. Use proper alignment and space fill to Length.*/
/*                                                                         */
/*   Variables:   String - floating point string with (maybe) an 'E' or 'e'*/
/*                         value.                                          */
/*                Length - Length of filled string                         */
/*                RightJustify - Right justify inside of length if TRUE    */
/*                               or left justify if FALSE.  Pad with spaces*/
/*                                                                         */
/***************************************************************************/
  
VOID  StripExp(PCHAR String, SHORT Length, BOOLEAN RightJustify)
{
  PCHAR DecimalPos;
  PCHAR EPos;
  SHORT StringLength;
  SHORT CharPos;
  SHORT StartFill, EndFill;

  /* 0 Length is an error, just return */
  if (Length == 0)
     return;
  StringLength = strlen(String);
  /* strip off leading spaces and 0's */
  while ((String[0] == '0') || (String[0] == ' '))
    {
    if ((String[0] != '0') ||
        ((String[1] != 'e') &&
        (String[1] != 'E') &&
        (String[1] != '.') &&
        (String[1] != ' ') &&
        (String[1] != '\0')))
      memmove(&String[0], &String[1], StringLength--);
    else if (String[0] == '0')
      break;
    }
  DecimalPos = strchr(String, '.');

  EPos = String;
  CharPos = 0;
  /* remove trailing 0's */
  if (DecimalPos != NULL)
    {
    do
      {
      /* search for first following '0' */
      while ((*DecimalPos != '0') && (*DecimalPos != 'E') &&
        (*DecimalPos != 'e') && (*DecimalPos != '\0'))
        {
        CharPos++;
        DecimalPos++;
        }

      EPos = DecimalPos;

      if (*DecimalPos == '0')
        {
        /* search for first following non-0 */
        while ((*EPos == '0') && (*EPos != 'E') && (*DecimalPos != 'e'))
          {
          EPos++;
          CharPos++;
          }

        /* non-zero not found */
        if ((*EPos == 'E') || (*EPos == 'e') || (*EPos == '\0'))
          memmove(DecimalPos, EPos, StringLength - CharPos + 1);
        else
          DecimalPos = EPos; /* move decimal position */

        }
      }
    while ((*EPos != 'E') && (*EPos != 'e') && (*EPos != '\0'));

    /* delete trailing decimal points */
    if (*(EPos-1) == '.')
      {
      EPos--;
      CharPos--;
      memmove(EPos, EPos + 1, StringLength - CharPos + 1);
      StringLength--;
      }
    }

  if ((*EPos == 'E') || (*EPos == 'e'))
    {
    /* delete trailing decimal points */
    if (*(EPos-1) == '.')
      {
      EPos--;
      CharPos--;
      memmove(EPos, EPos + 1, StringLength - CharPos + 1);
      StringLength--;
      }

    /* remove the '+' sign */
    CharPos++;
    EPos++;
    if (*EPos == '+')
      {
      memmove(EPos, EPos+1, StringLength - CharPos + 1);
      StringLength--;
      }
    /* '-' sign is OK */
    else if (*EPos == '-')
      {
      CharPos++;
      EPos++;
      }

    /* remove leading '0's from the exponent */
    DecimalPos = EPos;
    while (*EPos == '0')
      {
      StringLength--;
      memmove(EPos, EPos + 1, StringLength - CharPos + 1);
      }
    }

  StringLength = strlen(String);
  if (StringLength == 0)
    {
    String[0] = '0';
    String[1] = '\0';
    StringLength = 1;
    }
  /* error fallback, reduce precision */
  if (StringLength > Length)
    {
    EPos = strchr(String, 'e');
    if (! EPos)
      EPos = strchr(String, 'E');
    /* if 'E' not present, delete from the end */
    if (! EPos)
      {
      String[Length] = '\0';
      return;
      }
    /* delete from before the exponent first */
    DecimalPos = --EPos;
    while ((StringLength > Length) && (EPos != String))
      {
      /* move everything down */
      do
        {
        EPos++;
        *(EPos-1) = *EPos;
        }
      while (*EPos != '\0');
      if (DecimalPos != String)
        DecimalPos--;
      EPos = DecimalPos;
      StringLength--;
      }
    /* check to see if need to delete exponent */
    if (EPos == String)
      {
      String[1] = '\0';
      StringLength = 1;
      }
    }

  if (RightJustify)
    {
    StartFill = 0;
    EndFill = Length - StringLength;
    memmove(&String[EndFill], &String[0], StringLength + 1);
    }
  else
    {
    StartFill = Length - (Length - StringLength);
    EndFill = Length;
    }

  while (StartFill < EndFill)
    String[StartFill++] = ' ';

  if (! RightJustify)
    String[StartFill] = '\0';
}

// delete a character from a string
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void drop_char(char * ptr)
{
  do
    * ptr = * (ptr + 1);
  while(* (ptr ++));
}

// delete leading + and 0's from exponent and number string
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int condense_float_string(char * string)
{
  char * found;

  while(found = strstr(string, "+")) // delete all '+'
    drop_char(found);

  while(found = strstr(string + 1, "-0"))
    drop_char(found + 1);

  while(found = strstr(string, "e0"))
    drop_char(found + 1);

  while(found = strstr(string, "E0"))
    drop_char(found + 1);

  // delete leading space or 0, but not leading 0. or single 0
  while(strchr(" 0", string[ 0 ]))
    {
    if(strlen(string) <= 1) break;  // leave "0" alone
    // leave leading "0." alone
    if((string[ 0 ] == '0') && (string[ 1 ] == '.')) break;
    drop_char(string);
    }
  return strlen(string);
}
