/* -----------------------------------------------------------------------
/
/   hline.h
/
/  Copyright (c) 1989,  EG&G Princeton Applied Research
/
/  $Header: /OMA4000/Include/HLINE.H 1     6/18/99 1:52a Maynard $
/  $Log: /OMA4000/Include/HLINE.H $
 * 
 * 1     6/18/99 1:52a Maynard
 * 
 *    Rev 0.11   09 Jan 1992 09:36:34   cole
 * Change #include's.
 * 
 *    Rev 0.10   24 Jul 1991 10:11:32   cole
 * Added $Header and $Log comment lines. Removed #include for syserror.h
 * Added underColor argument to array_plot().
/
/ ----------------------------------------------------------------------- */
  
#ifndef HLINE_INCLUDED
#define HLINE_INCLUDED

#ifndef CGIBIND_INCLUDED
   #define CGIBIND_INCLUDED
   #include <cgibind.h>
#endif

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

ERR_OMA hplot_init(SHORT MaximumX);

ERR_OMA array_hplot(PLOTBOX * plot, FLOAT zvalue,
                           LPCURVEDIR pCurveDir, SHORT EntryIndex,
                           USHORT CurveIndex, CXY * p_min, CXY * p_max,
                           CXY * v_max, CCOLOR underColor);

void hplot_end(void);

#endif  // HLINE_INCLUDED
