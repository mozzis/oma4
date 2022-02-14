/***************************************************************************/
/*                                                                         */
/* File name  : IPROC                                                      */
/* Author     : David DiPrato                                              */
/* Description: This module contains many image processing functions.      */
/*                                                                         */
/* version 2.0 - Changed all integer operations from 32 to 16 bit.         */
/*                                                                         */
/***************************************************************************/

#ifndef uifffile
#define uifffile


/* Function Prototypes. ****************************************************/

/* This function will Convolute the given input data by the given 3X3 kernal 
   and put the result in the given Output data.  The data must be in 16 bit
   interger form.  Output memory will be allocated, therefore it must be
   released when finished.  A NULL will return if the data is not 16 bit
  or memory could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *Convolute3x3(short Kernel[][3],UIF_HEADER *In);

/* Example of 3x3 convolution kernals -------------------------------------*/
// short LowPass1[4][3] =
//    {
//    { 1, 1, 1},
//    { 1, 1, 1},
//    { 1, 1, 1},
//    { 9, 9, 9} 
//    };


/* This function will label all objects in the given image and put the
   result in the given output image.  Output memory will be allocated,
  therefore it must be released when finished.  A non-zero will return
  if the data is not 16 bit  or memory could not be allocated.
  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *LabelObject(UIF_HEADER *In);

/* This function will flaten the given labeled image by removing all
   labeled values, ie all objects will have ordered numbers with no missing
   values.  Therefore, the number of objects will equal the highest labeled 
   value. Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *FlatenLabeledImage(UIF_HEADER *In,int Verbose);

/* This function will find the minimum and maximum values of a given image
   by use of a 3x3 area adverage.  A non-zero will return if the data is 
   not 16 bit  or memory could not be allocated.  ErrorMsg will have error 
   as well.
****************************************************************************/
int FindMinMaxByArea(UIF_HEADER *In,short *Min,short *Max);

/* This function will find the obsolute minimum and maximum values of a given 
   image.  A non-zero will return if the data is not 16 bit or memory could 
   not be allocated.  ErrorMsg will have error  as well.
****************************************************************************/
int FindMinMax(UIF_HEADER *In,short *Min,short *Max);

/* This function will segment the given image by the given threshold and
   produce an output image.  Input data must be in 16 bit
   interger form.  Output memory will be allocated, therefore it must be
   released when finished.  A NULL will return if the data is not 1632 bit
  or memory could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *SegmentByThreshold(UIF_HEADER *In,short Threshold);

/* This function will dilate the given binary image based on a 3x3
   round structuring element and put the result in the given output image.
   Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *BinaryDilate(UIF_HEADER *In);

/* This function will dilate the given binary image based on a 3x3
   round structuring element and put the result in the given output image.
   Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *BinaryErode(UIF_HEADER *In);

/* This function will zero the given image memory.
****************************************************************************/
void ZeroImage(UIF_HEADER *In);

/* This function will convert the In image to floating point format.
   Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *ConvertToFloat(UIF_HEADER *In);

/* This function will subtract image B from image A and put the result
   in image Out.  The Out image will be in floating point format if the
   'Float' input is set.
   Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *SubtractImages(UIF_HEADER *A,UIF_HEADER *B,int Float);

/* This function will subtract image B from image A, multiply image Coef
   and put the result in image Out.  All data will issume 16 bit floating 
   point if value 'Float' is non zero.
   Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *FlatFieldImage (UIF_HEADER *A,UIF_HEADER *B,UIF_HEADER *Coef,int Float);

/* This function will take two images and remove any positive objects that
   do not corralate.  All data will issume 16 bit floating point if value 
   'Float' is non zero.
   Output memory will be allocated, therefore it must be released when 
   finished.  A non-zero will return if the data is not 16 bit or memory 
   could not be allocated.  ErrorMsg will have error as well.
****************************************************************************/
UIF_HEADER *RemoveCosmics(UIF_HEADER *A,UIF_HEADER *B,int Float);

#endif
