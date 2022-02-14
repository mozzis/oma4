/* oma4tiff.h */
/* define functions for TIFF file writing by OMA4000 */

#ifndef OMA4TIFF_INCLUDED
#define OMA4TIFF_INCLUDED


ERR_OMA trans_TIFF256(FILE* fptr, const char * fName,
                             CURVEDIR *CvDir,
                             SHORT Blk, SHORT StCurve, SHORT Curves,
                             BOOLEAN KbChk);
#endif
