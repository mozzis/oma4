#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <graph.h>
#include <stdarg.h>

#include "fabserrs.h"
#include "fabsgraf.h"

typedef struct {  /* for tracking clipregion */
  int left;
  int top;
  int right;
  int bottom;
  } cliprgn;

cliprgn CurrentClip;
cliprgn FullScreen;

/* text output parameters */

short Forecolor, Backcolor, Textrows;
struct videoconfig VideoSetup;

int Theight, Twidth,    /* height of text line */
    Lskip,              /* Pixels between lines of text */
    MaxX, MaxY;         /* screen resolution */

/* plotting parameters */

const int PointStep = 2;
struct xycoord *gArray;

/***********************************************************************/
/***********************************************************************/
void SetClipRegion(cliprgn * crgn)
{
  _setcliprgn(crgn->left, crgn->top, crgn->right, crgn->bottom);
  CurrentClip = *crgn;
}

/***********************************************************************/
/***********************************************************************/
void SetClipRegionToMax(void)
{
  SetClipRegion(&FullScreen);
}

/***********************************************************************/
/***********************************************************************/
char * grapherrormsg(int errorcode)
{
  switch (errorcode)
    {
    case _GROK:
     return("Success");
    break;

    case _GRERROR:
     return("Graphics error");
    break;

    case _GRMODENOTSUPPORTED:
     return("Requested video mode not supported");
    break;

    case _GRNOTINPROPERMODE:
     return("Requested routine works only in certain video modes");
    break;

    case _GRINVALIDPARAMETER:
     return("One or more parameters invalid");
    break;

    case _GRFONTFILENOTFOUND:
     return("No matching font file found");
    break;

    case _GRINSUFFICIENTMEMORY:
     return("Not enough memory for graphics buffer or _floodfill operation");
    break;

    case _GRNOOUTPUT:
     return("No action taken");
    break;

    case _GRCLIPPED:
     return("Output was clipped to viewport");
    break;

#ifndef __WATCOMC__
    case _GRCORRUPTEDFONTFILE:
     return("One or more font files inconsistent");
    break;

    case _GRINVALIDFONTFILE:
     return("One or more font files invalid");
    break;

    case _GRINVALIDIMAGEBUFFER:
     return("Image buffer data inconsistent");
    break;

    case _GRPARAMETERALTERED:
     return("Range or order of parameters was changed");
    break;
#endif
   }
}

/***********************************************************************/
/* if graphics lib logged an error, print a message and return 1       */
/* Else just return a 0                                                */
/***********************************************************************/
int gError(void)
{
  int errorcode = _grstatus();

  if (errorcode != _GROK && errorcode != _GRCLIPPED)  /* an error occurred */
    {
    printf("Graphics error: %s\n", grapherrormsg(errorcode));
    printf("Press any key to halt:");
    if (!getch())
      getch();
    return(errorcode); /* terminate with an error code */
    }
  return _GROK;
}

/***********************************************************************/
/***********************************************************************/
void outtextxy(int x, int y, char * outbuf)
{
  cliprgn crgn = CurrentClip;
  short gerr, SaveColor = _setcolor(Forecolor);

  SetClipRegionToMax(); 
  _moveto(x, y);
  _outgtext(outbuf);
  SetClipRegion(&crgn);
  _setcolor(SaveColor);
}

/***********************************************************************/
/***********************************************************************/
void outtextrowcol(int row, int col, char * outbuf)
{
  short x, y;
  
  row = Textrows - row;
  x = VideoSetup.numxpixels / 2 - (_getgtextextent(outbuf) / 2);
  x -= col * Twidth;
  y = ((row -1) * Theight) - Lskip;
  outtextxy(x, y, outbuf);
}


/***********************************************************************/
/***********************************************************************/
void line(int left, int bottom, int right, int top)
{
  _moveto(left, bottom);
  _lineto(right, top);
}

/***********************************************************************/
/* initialize graphics system - returns non-zero on failure            */
/***********************************************************************/
int InitGraf(void)
{
  short rows, gerr;
  /* initialize graphics mode */
  
  rows = _setvideomoderows(_MAXRESMODE, _MAXTEXTROWS);

  /* read result of initialization */
  if (gerr = gError())
    return gerr;

  _getvideoconfig(&VideoSetup);
  Forecolor = VideoSetup.numcolors - 1;
  Backcolor = 0;
  _setcolor(Forecolor);
  _setbkcolor(Backcolor);
  _settextcolor(Forecolor);
  _registerfonts("*.fon");
  _setfont("t'helv'h8w8b"); /* get best fit to modern vector font @7x7 */

  if (gerr = gError())
    return gerr;
  else
   {
    int Xinc, Yinc, titlex;
    cliprgn crgn;

    MaxX = VideoSetup.numxpixels - 1;    /* Get screen resolution and */
    MaxY = VideoSetup.numypixels - 1;    /* text height */
    Xinc = (MaxX + 1) / 8;
    Yinc = (MaxY + 1) / 4;
    Theight = (MaxY+1) / VideoSetup.numtextrows;
    Lskip = Theight / 4;
#ifdef __WATCOMC__
    Lskip += 2;
#endif
    Theight += Lskip;
    Textrows = VideoSetup.numypixels / Theight;
    Twidth = (MaxX+1) / VideoSetup.numtextcols;

    FullScreen.left = 0;
    FullScreen.bottom = 0;
    FullScreen.right = MaxX;
    FullScreen.top = MaxY;
    SetClipRegionToMax();

    outtextrowcol(Textrows - 3, 0, "Absorbance Test Program");

    crgn.left = Xinc;
    crgn.top = Yinc / 2;
    crgn.right = MaxX - Xinc;
    crgn.bottom = MaxY - Yinc;

    SetClipRegion(&crgn);
    }
  return gerr;
}

/***********************************************************************/
/***********************************************************************/
int InitPlot(unsigned int points)
{
  gArray = malloc(points * sizeof(struct xycoord));
  if (!gArray)
    return ERR_NO_MEMORY;
  else
    return 0;
}

/***********************************************************************/
/* output graphics text centered at coordinates x, y - like printf     */
/***********************************************************************/
int xy_printf(int x, int y, int * oldlen, char * format, ...)
{
  static char outbuf[80];
  va_list arglist;
  int count, err;
  static int len = 0;
  short SaveColor;

  va_start(arglist, format);
  count = vsprintf(outbuf, format, arglist);

  /* erase old text first */
  if (oldlen && *oldlen)
    {
    cliprgn crgn = CurrentClip;
  
    SetClipRegionToMax(); 
    SaveColor = _setcolor(Backcolor);
    _rectangle(_GFILLINTERIOR, x, y - Theight, x + *oldlen, y + Theight + 2);
    _setcolor(SaveColor);
    SetClipRegion(&crgn);
    }
  if (oldlen)
    *oldlen = _getgtextextent(outbuf);

  outtextxy(x, y, outbuf);
  va_end(arglist);
  err = _grstatus();
  return err ? 0 : count;
}

/***********************************************************************/
/* print prompt on screen - line gives #lines from bottom              */
/***********************************************************************/
int at_printf(int line, char * format, ...)
{
  static char outbuf[80];
  cliprgn crgn;
  va_list arglist;
  int count, err;
  short ty1, ty2, liney, SaveColor = _setcolor(Backcolor);

  crgn = CurrentClip;
  SetClipRegionToMax();

  liney = line * Theight;

  ty1 = MaxY - liney - Theight;            /* top */
  ty2 = MaxY - liney;                      /* bottom */

  _rectangle(_GFILLINTERIOR, 0, ty1, MaxX, ty2); 

  _setcolor(SaveColor);
  SetClipRegion(&crgn);

  va_start(arglist, format);
  count = vsprintf(outbuf, format, arglist);
  outtextrowcol(line, 0, outbuf);
  err = _grstatus();

  va_end(arglist);
  return err ? 0 : count;
}

/***********************************************************************/
/* print prompt on screen - line gives #lines from bottom              */
/***********************************************************************/
void at_erase(int line)
{
  cliprgn crgn;
  int count, err;
  short ty1, ty2, liney, SaveColor = _setcolor(Backcolor);

  crgn = CurrentClip;
  SetClipRegionToMax();

  liney = line * Theight;
  ty1 = MaxY - liney - Theight;          /* top */
  ty2 = MaxY - liney;                    /* bottom */

  _rectangle(_GFILLINTERIOR, 0, ty1, MaxX, ty2); 

  _setcolor(SaveColor);
  SetClipRegion(&crgn);
}

/***********************************************************************/
/* scale the graphics display                                          */
/***********************************************************************/
void ScaleDisplay(float far * Result, int points, float *YMin, float *YMax)
{
  int i, j, LegX, LegY;
  static int minylen = 0, maxylen = 0;
  float Xscale;
  cliprgn crgn;
  short SaveColor;

  if (points)
    Xscale = MaxX / (float)points;
  else
    Xscale = 2;
  *YMin = 3e18F;
  *YMax = -3e18F;

  for (i = j = 0; i < points; i+= PointStep, j++)
    {
    gArray[j].xcoord = (int)( (float)(i) * Xscale);
    if (Result[i] < *YMin)
      *YMin = Result[i];
    if (Result[i] > *YMax)
      *YMax = Result[i];
    }

  if (fabs((double)(*YMin)) < 3e-18F)
    *YMin = 3e-17F;
  if (fabs((double)(*YMax)) < 3e-18F)
    *YMax = 3e-16F;

  /* draw axis lines, 1 pixel outside of plot view clip rectangle */

  crgn = CurrentClip;
  SetClipRegionToMax();
  
  line(crgn.left-1, crgn.bottom+1, crgn.right, crgn.bottom+1);
  line(crgn.left-1, crgn.bottom+1, crgn.left-1, crgn.top);

  /* get X coordinate of Y axis labels */

  LegX = crgn.left - Twidth * 9;

  /* print Y Axis max */
  xy_printf(LegX, crgn.top - Theight / 2, &minylen, "%4.4f", *YMax);

  /* print Y Axis min */
  xy_printf(LegX, crgn.bottom - Theight / 2, &maxylen, "%4.4f", *YMin);

  /* print X Axis min */
  xy_printf(crgn.left, crgn.bottom + Theight * 2, NULL, "%d", 0);

  /* print Y Axis max */
  LegX = crgn.right - Twidth * 3;
  xy_printf(LegX, crgn.bottom + Theight * 2, NULL, "%d", points);

  SetClipRegion(&crgn);
}

/***********************************************************************/
/* draw data on the screen.                                            */
/***********************************************************************/
int plot_Data(float *Result, int points)
{
  float Yscale;
  int i, j;
  short SaveColor;
  cliprgn crgn = CurrentClip;
  static float YMin, YMax;
  static int ScaleTime = 0;

  if (!ScaleTime--)
    {
    ScaleDisplay(Result, points, &YMin, &YMax);
    ScaleTime = 50;
    }

  Yscale = (crgn.bottom - crgn.top) / (YMax - YMin);

  for (i = j = 0; i < points; i+=PointStep)
    {
    gArray[j++].ycoord = (int)((Result[i]-YMin) * Yscale);
    }

  gArray[0].ycoord = crgn.bottom;   /* kludge because of Microsoft's */
  gArray[j-1].ycoord = crgn.bottom; /* stupid implentation of polygon */

  SaveColor = _setcolor(Backcolor);
  _rectangle(_GFILLINTERIOR, crgn.left, crgn.bottom-1, crgn.right, crgn.top);
  _setcolor(SaveColor);
#ifndef __WATCOMC__
  _polygon(_GBORDER, gArray, points / PointStep);
#else
  _polygon(_GBORDER, points / PointStep, gArray);
#endif

  return 0;
}
/***********************************************************************/
/***********************************************************************/
int DeInitGraf(void)
{
  _displaycursor(_GCURSORON);
  _setvideomode(_DEFAULTMODE);
  return 0;
}

