/***************************************************************************/
/*                                                                         */
/* File name  : IPROC                                                      */
/* Author     : David DiPrato                                              */
/* Description: This module contains many image processing functions.      */
/*                                                                         */
/* version 2.0 - Changed all integer operations from 32 to 16 bit.         */
/*                                                                         */
/***************************************************************************/

#include <ctype.h>
#include <conio.h>
#include <math.h>
#include <direct.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <malloc.h>

#include "uiff.h"
#include "iproc.h"

/* Local definitions *******************************************************/

/* Local global varibles ***************************************************/


/* Labeling stuff **********************************************************/
#define DUMMY  0
long Tag[0x10000];


/*Local Prototype *********************************************************/
/* This function is used in all image processor functions.  It tests the
  data for 16 bit fornat and allocates memory for the output image.  A NULL
  will return and 'ErrorMsg' set on error.
****************************************************************************/
UIF_HEADER *Test16bitAndAllocate(UIF_HEADER *In);

/* This function is used in all image processor functions.  It allocates 
   memory for the output image and copies the inputs header.  A NULL will 
   return and 'ErrorMsg' set on error.
****************************************************************************/
UIF_HEADER *AllocateImage(UIF_HEADER *In);

/* This function will convolve a single point with the given kernal.
****************************************************************************/
short ConvolvePt(short *Data,short Kernel[][3]);




/* Functions follow ********************************************************/  
  
/* This function is used in all image processor functions.  It tests the
  data for 16 bit fornat and allocates memory for the output image.  A NULL
  will return and 'ErrorMsg' set on error.
****************************************************************************/
UIF_HEADER *Test16bitAndAllocate(UIF_HEADER *In)
{
  int NumBytes;
  UIF_HEADER *Out;

  /* Test for 16 bit data -------------------------------------------------*/
//  if (Integer16Bit(In) == 0) {
//    strcpy(ErrorMsg,"Data is not 16 bit.");
//    return(NULL);
//    }

  /* Allocate memory for result -------------------------------------------*/
  if ((Out = (UIF_HEADER *)calloc(sizeof(UIF_HEADER),1)) == NULL) {
      strcpy(ErrorMsg,"Could not allocate memory for data.");
      return(NULL);
      }
  CopyUIFHeader(In,Out);

  if (In->FileFormat == BITS16)
    NumBytes = sizeof(short) * In->XTotal * In->YTotal;
  else
    NumBytes = sizeof(long) * In->XTotal * In->YTotal;

  if ((Out->DataPtr.b = calloc(NumBytes,1)) == NULL) {
      strcpy(ErrorMsg,"Could not allocate memory for data.");
      return(NULL);
      }

  return(Out);
}

/* This function is used in all image processor functions.  It allocates 
   memory for the output image and copies the inputs header.  A NULL will 
   return and 'ErrorMsg' set on error.
****************************************************************************/
UIF_HEADER *AllocateImage(UIF_HEADER *In)
{
  int NumBytes;
  UIF_HEADER *Out;

  /* Allocate memory for result -------------------------------------------*/
  if ((Out = (UIF_HEADER *)calloc(sizeof(UIF_HEADER),1)) == NULL) {
      strcpy(ErrorMsg,"Could not allocate memory for data.");
      return(NULL);
      }
  CopyUIFHeader(In,Out);

  if (In->FileFormat == BITS16)
    NumBytes = sizeof(short) * In->XTotal * In->YTotal;
  else
    NumBytes = sizeof(long) * In->XTotal * In->YTotal;
  if ((Out->DataPtr.b = calloc(NumBytes,1)) == NULL) {
      strcpy(ErrorMsg,"Could not allocate memory for data.");
      return(NULL);
      }

  return(Out);
}

/* This function will Convolute the given input data by the given 3X3 kernal 
   and put the result in the given Output data.  The data must be in 16 bit
   interger form.  Output memory will be allocated, therefore it must be
   released when finished.  A NULL will return if the data is not 16 bit
  or memory could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *Convolute3x3(short Kernel[][3],UIF_HEADER *In)
{
  int x,y;
  UIF_HEADER *Out;
  short *Data;

  /* Test for 16 bit format and allocate output memory -------------------*/
  if ((Out = AllocateImage(In)) == NULL) return(NULL);

  /* Do all data except outter edges. ------------------------------------*/
   for (x = 1;x < In->XTotal - 1;x++) 
      for (y = 1;y < In->YTotal - 1;y++) 
         Data = &In->DataPtr.s[(y * In->XTotal) + x];
         Out->DataPtr.s[(y * In->XTotal) + x] = ConvolvePt(Data,Kernel); 
         Out->DataPtr.s[(y * In->XTotal) + x] = ConvolvePt(Data,Kernel); 

   /* Do data edges -------------------------------------------------------*/
   x = 0;
   for (y = 0;y < In->YTotal;y++)
      Out->DataPtr.s[(y * In->XTotal) + x] = In->DataPtr.s[(y * In->XTotal) + x];
   x = In->XTotal - 1;
   for (y = 0;y < In->YTotal;y++)
      Out->DataPtr.s[(y * In->XTotal) + x] = In->DataPtr.s[(y * In->XTotal) + x];
   y = 0;
   for (x = 0;x < In->XTotal;x++) 
      Out->DataPtr.s[(y * In->XTotal) + x] = In->DataPtr.s[(y * In->XTotal) + x];
   y = In->XTotal - 1;
   for (x = 0;x < In->XTotal;x++) 
      Out->DataPtr.s[(y * In->XTotal) + x] = In->DataPtr.s[(y * In->XTotal) + x];

   return(Out);
}

/* This function will convolve a single point with the given kernal.
****************************************************************************/
short ConvolvePt(short *Data,short Kernel[][3])
{
   int x,y;
   short Result = 0;

   for (x = 0;x < 3;x++) 
      for (y = 0;y < 3;y++) 
         Result += (Data[0] * Kernel[x][y]);      
   Result /= Kernel[3][0];

   return(Result);
}

/* This function will segment the given image by the given threshold and
   produce an output image.  Input data must be in 16 bit
   interger form.  Output memory will be allocated, therefore it must be
   released when finished.  A NULL will return if the data is not 16 bit
  or memory could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *SegmentByThreshold(UIF_HEADER *In,short Threshold)
{
  long i,TotalPixels;
  UIF_HEADER *Out;

  /* Test for 32 bit format and allocate output memory -------------------*/
  if ((Out = AllocateImage(In)) == NULL) return(NULL);
  TotalPixels = In->XTotal * In->YTotal;

  for (i = 0;i < TotalPixels;i++) {
     if (In->DataPtr.s[i] > Threshold) {
        Out->DataPtr.s[i] = 1; 
        }
     else {
        Out->DataPtr.s[i] = 0; 
        }
     }

   return(Out);
}

/* This function will find the minimum and maximum values of a given image
   by use of a 3x3 area adverage.  A non-zero will return if the data is 
   not 16 bit  or memory could not be allocated.  ErrorMsg will have error 
   as well.
****************************************************************************/
int FindMinMaxByArea(UIF_HEADER *In,short *Min,short *Max)
{
#define ADV_LENGTH   10
  long i,j,Pt,NumPts;
  UIF_HEADER *Out;

  /* Test for 16 bit format and allocate output memory -------------------*/
  if ((Out = AllocateImage(In)) == NULL) return(-1);

  NumPts = In->XTotal * In->YTotal;
  Pt = 0;
  for (j = 0;j < ADV_LENGTH;j++)
    if (In->FileFormat == BITS16)
      Pt += In->DataPtr.s[1000 + j];
    else
      Pt += In->DataPtr.l[1000 + j];

  Pt = Pt / ADV_LENGTH;
  *Min = Pt;
  *Max = *Min;

  for (i = 0;i < NumPts - ADV_LENGTH;i+=ADV_LENGTH) {
    Pt = 0;
    for (j = 0;j < ADV_LENGTH;j++)
      if (In->FileFormat == BITS16)
        Pt += In->DataPtr.s[i + j];
      else
        Pt += In->DataPtr.l[i + j];
    Pt = Pt / ADV_LENGTH;
    if (Pt < *Min) *Min = Pt;
    if (Pt > *Max) *Max = Pt;
    }

  return(0);
}

/* This function will find the obsolute minimum and maximum values of a given 
   image.  A non-zero will return if the data is not 16 bit or memory could 
   not be allocated.  ErrorMsg will have error  as well.
****************************************************************************/
int FindMinMax(UIF_HEADER *In,short *Min,short *Max)
{
  long x,y,Pt;
  UIF_HEADER *Out;

  /* Test for 16 bit format and allocate output memory -------------------*/
  if ((Out = AllocateImage(In)) == NULL) return(-1);

  *Min = In->DataPtr.s[(3 * In->XTotal) + 3]; 
  *Max = *Min;
  for (x = 1;x < In->XTotal - 1;x++) 
     for (y = 1;y < In->YTotal - 1;y++) {
        if (In->FileFormat == BITS16)
          Pt = In->DataPtr.s[(y * In->XTotal) + x];
        else
          Pt = In->DataPtr.l[(y * In->XTotal) + x]; 

        if (Pt < *Min) *Min = Pt;
        if (Pt > *Max) *Max = Pt;
        }

  return(0);
}


/* This function will label all objects in the given image and put the
   result in the given output image.  Output memory will be allocated,
  therefore it must be released when finished.  A non-zero will return
  if the data is not 16 bit  or memory could not be allocated.
  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *LabelObject(UIF_HEADER *In)
{
  short NextTag = 1;               /* Tag number for next Tag found.      */
  short NextObject = 1;            /* Object number for next object found.*/
  long x,y,i;                      /* Spatial loop varibles.              */
  long SpatialIndex;               /* Spatial X,Y location index.         */
  long j;                          /* Point neighbor index.               */
  long NeighborFound;              /* Holds location of neighboring point.*/ 
  short NeighborValue;             /* Holds the present neighbor value.   */
  UIF_HEADER *Out;
  short Neighbor[5];

  /* Test for 16 bit format and allocate output memory -------------------*/
  if ((Out = AllocateImage(In)) == NULL) return(NULL);
  Tag[0] = 0;

  /* Build Neighbor Kernal -----------------------------------------------*/
  Neighbor[0] = 0;                 /* Dummy location.                    */
  Neighbor[1] = -1;
  Neighbor[2] = -(In->XTotal + 1);
  Neighbor[3] = -In->XTotal;
  Neighbor[4] = -(In->XTotal - 1);

  for (y = 1;y < In->YTotal;y++) { /* Must start passed first row.        */      
    for (x = 1;x < In->XTotal;x++) {     
      SpatialIndex = (y * In->XTotal) + x;

      /* Look for a object -----------------------------------------------*/
      if (In->DataPtr.s[SpatialIndex]) {

        /* Look for a neighboring object ---------------------------------*/
        NeighborFound = 0;
        for (j = 1;j < 5;j++) {
          if (NeighborValue = Out->DataPtr.s[SpatialIndex + Neighbor[j]]) {

            /* Neighbor found, assign Tag --------------------------------*/
            Out->DataPtr.s[SpatialIndex] = NeighborValue;

            /* Check for previous neighbors ------------------------------*/
            if (NeighborFound) {

              /* Previous neighbor found, check for different Tags -------*/
              if ((Out->DataPtr.s[SpatialIndex + Neighbor[NeighborFound]]) !=
                NeighborValue) {

                /* Dif. Tags found, asign old Tag to new object ----------*/
                Tag[Out->DataPtr.s[SpatialIndex + Neighbor[NeighborFound]]] = 
                   Tag[NeighborValue];
                }
              }            
            NeighborFound = j;
            } 
          }

        if (NeighborFound == 0) {

          /* Create a new object and add this point to it ----------------*/
          Out->DataPtr.s[SpatialIndex] = NextTag;
          Tag[NextTag] = NextObject; 
          NextTag++;                /* These indexs can roll over.        */
          NextObject++;
          }
        }
      }
    }

  /* Assign Oject values to output data ----------------------------------*/
  for (i = 0;i < (In->XTotal * In->YTotal);i++) {
    if (Out->DataPtr.s[i]) {            /* Look of any objects.             */
      Out->DataPtr.s[i] = Tag[Out->DataPtr.s[i]];
      }
    }

  return(Out);
}

/* This function will flaten the given labeled image by removing all
   labeled values, ie all objects will have ordered numbers with no missing
   values.  Therefore, the number of objects will equal the highest labeled 
   value. Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *FlatenLabeledImage(UIF_HEADER *In,int Verbose)
{
  UIF_HEADER *Out;
  long i,NumPts,ObjectCntr = 1;
  static unsigned short Label[0x10000];

  /* Test for 16 bit format and allocate output memory -------------------*/
  if ((Out = AllocateImage(In)) == NULL) return(NULL);

  for (i = 0;i < 0x10000;i++) Label[i] = 0;
  NumPts = In->XTotal * In->YTotal;
     
  /* Mark all object values ----------------------------------------------*/
  for (i = 0;i < NumPts;i++) {
    if (In->DataPtr.s[i] != 0) {      
      
      /* Object found, mark it found -------------------------------------*/
      if (In->DataPtr.s[i] >= 0x7fff) {
        strcpy(ErrorMsg,"Too many objects found.");
        ReleaseUIFFFile(In);
        return(NULL);
        }
      Label[In->DataPtr.s[i]] = 1;
      }
    }

  /* Flaten all objects found --------------------------------------------*/
  for (i = 0;i < 0x7fff;i++) {
    if (Label[i]) Label[i] = ObjectCntr++;
    }
  if (Verbose) printf("%i objects found\n",ObjectCntr - 1);

  /* Create flatened image -----------------------------------------------*/
  for (i = 0;i < NumPts;i++) {
    if (In->DataPtr.s[i] != 0) {      
      Out->DataPtr.s[i] = Label[In->DataPtr.s[i]];
      }
    }

  return(Out);
}

/* This function will dilate the given binary image based on a 3x3
   round structuring element and put the result in the given output image.
   Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *BinaryDilate(UIF_HEADER *In)
{
  int x,y;
  int YLimit;
  int UnionFound;
  UIF_HEADER *Out;

  /* Test for 16 bit format and allocate output memory -------------------*/
  if ((Out = AllocateImage(In)) == NULL) return(NULL);

  YLimit = In->XTotal * In->YTotal - In->XTotal;
  for (x = 1;x < In->XTotal - 1;x++) {
    for (y = In->XTotal;y < YLimit;y+=In->XTotal) {
      UnionFound = 0;
      if (In->DataPtr.s[x + y])              UnionFound = 1;
      if (In->DataPtr.s[x + 1 + y])          UnionFound = 1;
      if (In->DataPtr.s[x + y + In->XTotal]) UnionFound = 1;
      if (In->DataPtr.s[x - 1 + y])          UnionFound = 1;
      if (In->DataPtr.s[x + y - In->XTotal]) UnionFound = 1;
   
      if (UnionFound) Out->DataPtr.s[x + y] = 1;
      else Out->DataPtr.s[x + y] = 0;
      }
    }
  return(Out);
}


/* This function will dilate the given binary image based on a 3x3
   round structuring element and put the result in the given output image.
   Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *BinaryErode(UIF_HEADER *In)
{
   int x,y;
   int YLimit;
   int IntersectFound;
  UIF_HEADER *Out;

  /* Test for 16 bit format and allocate output memory -------------------*/
  if ((Out = AllocateImage(In)) == NULL) return(NULL);

  YLimit = In->XTotal * In->YTotal - In->XTotal;
  for (x = 1;x < In->XTotal - 1;x++) {
    for (y = In->XTotal;y < YLimit;y+=In->XTotal) {
      IntersectFound = 0;
      if (In->DataPtr.s[x + y])              IntersectFound++;
      if (In->DataPtr.s[x + 1 + y])          IntersectFound++;
      if (In->DataPtr.s[x + y + In->XTotal]) IntersectFound++;
      if (In->DataPtr.s[x - 1 + y])          IntersectFound++;
      if (In->DataPtr.s[x + y - In->XTotal]) IntersectFound++;
      
      if (IntersectFound == 5) Out->DataPtr.s[x + y] = 1;
      else Out->DataPtr.s[x + y] = 0;
      }
    }
  return(Out);
}

/* This function will subtract image B from image A and put the result
   in image Out. All data will issume 16 bit floating point 
   if value 'Float' is non zero.
   Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *SubtractImages(UIF_HEADER *A,UIF_HEADER *B,int Float)
{
  UIF_HEADER *Out;
  int i;
  int TotalPixels;

  /* Test for 16 bit format and allocate output memory -------------------*/
  if ((Out = AllocateImage(A)) == NULL) return(NULL);

  TotalPixels = A->XTotal * A->YTotal;
  for (i = 0;i < TotalPixels;i++) {
      if (Float) Out->DataPtr.f[i] = A->DataPtr.f[i] - B->DataPtr.f[i];
      else Out->DataPtr.s[i] = A->DataPtr.s[i] - B->DataPtr.s[i];
      }

  return(Out);
}

/* This function will take two images and remove any positive objects that
   do not corralate.  All data will issume 16 bit floating point if value 
   'Float' is non zero.
   Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *RemoveCosmics(UIF_HEADER *A,UIF_HEADER *B,int Float)
{
  UIF_HEADER *Out;
  int i;
  int TotalPixels;
  float FPtA,FPtB;
  short IPtA,IPtB;

  /* Test for 16 bit format and allocate output memory -------------------*/
  if ((Out = AllocateImage(A)) == NULL) return(NULL);

  TotalPixels = A->XTotal * A->YTotal;

  for (i = 0;i < TotalPixels;i++) {
      if (Float) {
         FPtA = A->DataPtr.f[i];
         FPtB = B->DataPtr.f[i];

         /* Test for image with 'real' data, ie lowest pixel value --------*/
         if (FPtA > FPtB)             /* Is Image-B real.                  */
            Out->DataPtr.f[i] = FPtB;
         else
            Out->DataPtr.f[i] = FPtA;
         }
      else {
         IPtA = A->DataPtr.s[i];
         IPtB = B->DataPtr.s[i];

         /* Test for image with 'real' data, ie lowest pixel value --------*/
         if (IPtA > IPtB)             /* Is Image-B real.                  */
            Out->DataPtr.s[i] = IPtB;
         else
            Out->DataPtr.s[i] = IPtA;
         }
      }

  return(Out);
}

/* This function will subtract image B from image A, multiply image Coef
   and put the result in image Out.  All data will issume 16 bit floating 
   point if value 'Float' is non zero.
   Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *FlatFieldImage (UIF_HEADER *A,UIF_HEADER *B,UIF_HEADER *Coef,int Float)
{
  UIF_HEADER *Out;
  int i;
  int TotalPixels;

  /* Test for 16 bit format and allocate output memory -------------------*/
  if ((Out = AllocateImage(A)) == NULL) return(NULL);

  TotalPixels = A->XTotal * A->YTotal;
  for (i = 0;i < TotalPixels;i++) {
      if (Float) 
         Out->DataPtr.f[i] = (A->DataPtr.f[i] - B->DataPtr.f[i]) * Coef->DataPtr.f[i];
      else Out->DataPtr.s[i] = (A->DataPtr.s[i] - B->DataPtr.s[i]) * Coef->DataPtr.s[i];
      }

  return(Out);
}

/* This function will convert the In image to floating point format.
   Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *ConvertToFloat(UIF_HEADER *In)
{
  UIF_HEADER *Out;
  int i;
  int TotalPixels;

  /* Test for 16 bit format and allocate output memory -------------------*/
  if ((Out = AllocateImage(In)) == NULL) return(NULL);
  Out->DataType = 0x5a5a5a5a;

  TotalPixels = In->XTotal * In->YTotal;
  for (i = 0;i < TotalPixels;i++) {
      Out->DataPtr.f[i] = (float)In->DataPtr.s[i];
      }

  return(Out);
}

/* This function will zero the given image memory.
****************************************************************************/
void ZeroImage(UIF_HEADER *In)
{
   int i;
   int TotalPixels;

   TotalPixels = In->XTotal * In->YTotal;
   for (i = 0;i < TotalPixels;i++) In->DataPtr.s[i] = 0;
}

