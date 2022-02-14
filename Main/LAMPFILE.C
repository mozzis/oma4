/* Copyright 1993 EG & G  PARC *********************************************
 *
 *  LampFile.c    October 15, 1993
 *
 *  Routines for loading and saving lamp data on disk.
 *
 **************************************************************************/

#include <stdlib.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <dos.h>
#include <string.h>
#include <io.h>
#include <conio.h>
#include <stdio.h>

#include "spline-3.h"

#define KNOT_NUM  40

/* 
 char *Name   File name with extension:  ????????.LMP is the standard.
 char *Label  Label of the curve - up to 80 char.
 char *Units  Holds the lamp units.  0X10 is æW/nm.cmý
 float *X     The x knot array.
 float *Y     The y knot array.
 int Mode     If 0, store the curve, if 1 load the curve.
 If error, 0 returned.
 */

static FLOAT  X[KNOT_NUM],
              Y[KNOT_NUM];

int LampLoadSave (char *Name, char *Label, char *Units, int Mode)
{
  char TmpBuf[96];
  int err = -1, fh, i;

  if (Mode)                     /* load lamp data. */
    {
    if ((fh = open (Name, O_RDONLY|O_BINARY, 0)) < 0)
      err = errno;
    else
      {
      read (fh, TmpBuf, 96);                    /* check label to see if ok */
      if ( !strncmp (TmpBuf, "M1470LMP",8))
        {
        read (fh, (void *)Units, sizeof(char));
        read (fh, (void *)X, KNOT_NUM * sizeof(float));
        lseek(fh, 96L + (long)(sizeof(char) + (128 * sizeof(float))),
                    SEEK_SET);
        err = (read (fh, (void *)Y, KNOT_NUM * sizeof(float)) == -1);
        strcpy (Label, TmpBuf+14);
        }
      close (fh);
      }
    if (!err)
      {
      for (i = 0; i < KNOT_NUM; i++)
        add_knot(X[i], Y[i]);
      }
    }
  else                          /* store table */
    {
    err = 1;
    if (access (Name, 0) == 0)    /* file exists */
      {
      printf ("\rOverwrite file? Y/N");
      err = (getch() == 'Y');
      }
    if (err)
      {
      printf("\r...Storing Data ...   ");
      memset (TmpBuf, 0, 96);       /* null string for neatness */
                                    /* add label for the process*/
      sprintf (TmpBuf, "M1470LMP 01.0 %s", Label);
      TmpBuf[95] = '\x1A';          /* So type, edit, won't get garbage*/
      fh = open (Name, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, S_IWRITE);
      if ((err = (fh < 0)) == 0)
        {
        write (fh, TmpBuf, 96);     /* send to disk */
        write (fh, (void *)Units, sizeof (char));
        write (fh,  (void *)X, KNOT_NUM * sizeof(float));
        lseek(fh, 96L + (long)(sizeof(char) + (128 * sizeof(float))),
                    SEEK_SET);
        err = (write (fh, (void *)Y, KNOT_NUM * sizeof(float)) == -1);
        }
      close (fh);
      }
      if (err)
        printf ("   *** Write Error ! ***   ");
    }
  return (err);
}
