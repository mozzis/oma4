/* -----------------------------------------------------------------------
/
/  grafmous.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/GRAFMOUS.H 1     6/18/99 1:52a Maynard $

 *    Rev 1.0   19 Dec 1990 13:53:20   irving
 * Initial revision.
/
*/

#ifndef GRAFMOUS_INCLUDED
#define GRAFMOUS_INCLUDED

#ifndef EGGTYPE_INCLUDED
   #include "eggtype.h"
#endif

UCHAR GraphMouseService(USHORT Buttons, SHORT Row, SHORT Column,
   SHORT XPos, SHORT YPos, BOOLEAN *LeftUp, BOOLEAN *RightUp,
   BOOLEAN *BothUp, BOOLEAN *XDragMode, BOOLEAN *ZDragMode,
   BOOLEAN *BothDrag);

#endif  // GRAFMOUS_INCLUDED
