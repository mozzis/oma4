#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <graphics.h>
#include <stdarg.h>

#include "fabserrs.h"
#include "fabsgraf.h"

int Theight, Twidth,    /* height of text line */
    MaxX, MaxY;         /* screen resolution */
const int PointStep = 2;
int far * gArray;

/* if graphics lib logged an error, print a message and return 1 */
/* Else just return a 0 */
int gError(void)
{
  int errorcode = graphresult();

  if (errorcode != grOk)  /* an error occurred */
    {
    printf("Graphics error: %s\n", grapherrormsg(errorcode));
    printf("Press any key to halt:");
    if (!getch())
      getch();
    return(errorcode); /* terminate with an error code */
    }
  return grOk;
}

/* output graphics text centered at coordinates x, y - like printf */

int gprintf(int x, int y, int * oldlen, char * format, ...)
{
  static char outbuf[80];
  va_list arglist;
  int count, ty, err, arg;
  
  va_start(arglist, format);
  count = vsprintf(outbuf, format, arglist);

  if (oldlen && *oldlen)
    {
    bar(x - *oldlen/2, y - Theight, x + *oldlen/2, y + Theight);
    }

  if (oldlen)
    *oldlen = textwidth(outbuf);

  outtextxy(x, y, outbuf);
  va_end(arglist);
  err = graphresult();
  return err ? 0 : count;
}

/* print prompt on screen - line gives #lines from bottom */

int at_printf(int line, char * format, ...)
{
  static char outbuf[80];
  struct viewporttype vp;
  va_list arglist;
  int count, err;

  getviewsettings(&vp);
  setviewport(0, 0, getmaxx(), getmaxy(), 0);
  bar(0,    MaxY - (line * Theight) - Theight/2,
      MaxX, MaxY - (line * Theight) + Theight / 2);

  va_start(arglist, format);
  count = vsprintf(outbuf, format, arglist);
  outtextxy(MaxX / 2, MaxY - (line * Theight), outbuf);
  err = graphresult();
  setviewport(vp.left, vp.top, vp.right, vp.bottom, 1);
  va_end(arglist);
  return err ? 0 : count;
}

/* erase a line on screen - line gives #lines from bottom */

void at_erase(int line)
{
  struct viewporttype vp;

  getviewsettings(&vp);
  setviewport(0, 0, getmaxx(), getmaxy(), 0);
  bar(0,    MaxY - (line * Theight) - Theight/2,
      MaxX, MaxY - (line * Theight) + Theight / 2);
  setviewport(vp.left, vp.top, vp.right, vp.bottom, 1);
}

/* initialize graphics system - returns non-zero on failure */
int InitGraf(void)
{
  int gDriv = DETECT, gMode = 9;

  setgraphbufsize(32767);

  /* initialize graphics mode */
  initgraph(&gDriv, &gMode, "");

  /* read result of initialization */
  if (gError())
    return grError;

  if (gError())
    return grError;

  setcolor(getmaxcolor());
  if (gError())
    return grError;

    {
    int Xinc, Yinc;
    struct viewporttype vp;

    MaxX = getmaxx();                /* Get screen resolution and */
    MaxY = getmaxy();                /* text height */
    Xinc = MaxX / 8;
    Yinc = MaxY / 4;
    Theight = textheight("AyqJQZI");
    Theight += Theight / 2;          /* allow for interline spacing */
    Twidth = textwidth("A");

    setfillstyle(0,0);
    settextjustify(CENTER_TEXT, CENTER_TEXT);
    getviewsettings(&vp);
    outtextxy(MaxX / 2, vp.top+Theight*2, "Absorbance Test Program");
    if (gError())
      return grError;
    setviewport(Xinc, Yinc, MaxX - Xinc, MaxY - Yinc, 1);
    }
  return gError();
}

int InitPlot(unsigned int points)
{
  gArray = malloc(points * sizeof(SHORT) * 2);
  if (!gArray)
    return ERR_NO_MEMORY;
  else
    return 0;
}

/* scale the graphics display */

void ScaleDisplay(float far * Result, int points, float *YMin, float *YMax)
{
  int i, j, LegX;
  static int minylen = 0, maxylen = 0;
  float Xscale;
  struct viewporttype vp;

  Xscale = MaxX / (float)points;
  *YMin = 3e18F;
  *YMax = -3e18F;

  for (i = j = 0; i < points; i+= PointStep, j++)
    {
    gArray[j++] = (int)( (float)(i) * Xscale);
    if (Result[i] < *YMin)
      *YMin = Result[i];
    if (Result[i] > *YMax)
      *YMax = Result[i];
    }

  /* draw axis lines, 1 pixel outside of plot view clip rectangle */

  getviewsettings(&vp);
  setviewport(0, 0, getmaxx(), getmaxy(), 0);
  line(vp.left-1, vp.bottom+1, vp.right, vp.bottom+1);
  line(vp.left-1, vp.bottom+1, vp.left-1, vp.top);

  /* get X coordinate of Y axis labels */

  LegX = vp.left - Twidth * 6;
  
  /* print Y Axis max - erase old value first */

  gprintf(LegX, vp.top + Theight / 2, &minylen, "%4.4f", *YMax);

  /* print Y Axis min */

  gprintf(LegX, vp.bottom, &maxylen, "%4.4f", *YMin);

  setviewport(vp.left, vp.top, vp.right, vp.bottom, 1);
}

/* draw data on the screen. */

int plot_Data(float far * Result, int points)
{
  float Yscale;
  int i, j;
  struct viewporttype vp;
  static float YMin, YMax;
  static int ScaleTime = 0;

  if (!ScaleTime--)
    {
    ScaleDisplay(Result, points, &YMin, &YMax);
    ScaleTime = 50;
    }

  getviewsettings(&vp);

  Yscale = (vp.bottom - vp.top) / (YMax - YMin);

  for (i = j = 0; i < points; i+=PointStep)
    {
    j++;    /* skip X values */
    gArray[j++] = (int)((Result[i]-YMin) * Yscale);
    }

  clearviewport();
  drawpoly(points / PointStep, gArray);
  return 0;
}

int DeInitGraf(void)
{
  closegraph();
  return 0;
}

