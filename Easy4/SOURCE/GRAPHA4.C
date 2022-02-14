/*=======================================================================*/
/*                             GRAPHA.C                                  */
/*=======================================================================*/
/* This file uses the graphic functions defined in the .BGI libraries.   */
/*=======================================================================*/

#include <acq4.h>
#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>
#include <string.h>
#include <dos.h>
#include <float.h>
#include <math.h>
#include <ctype.h>

/*=======================================================================*/
extern int X1[],Y1[],X2[],Y2[],X,
	   enable_param[],
	   FLAG_MEMO,control,
	   ACCU,NACC,
	   XSTEP,YSTEP,FIRDI,LASTDI,INF,DIV,ZOOMX,
	   CUR_DETECT,CCD_MODEL,OFFSET_DIOD,DIODE_NUMBER;

extern long IMAX,IMIN;

extern char INTENS_BEGIN[],  /* first intensified diode. */
	    INTENS_END[],    /* last intensified diode.  */
	    INT_TIME[],EXCLINE[],MAX_INT_TIME[];

void write_text(int color_ground,int x0,int y0,int length,int high,int color_text,char *text);
/*=======================================================================*/

int left=224,
    newpoly[513],accpoly[513],  /* y-coordinates of the displayed spectra. */
    UNITS=CM_1,
    current_color=LIGHTGREEN;

char curlen_image[474],       /* to save the image under cursor wavelength.*/
     buffer_image[20548];     /* to save an image before opening a window. */

long ifinal[1024];

/*=======================================================================*/

void init_graph(void)            /* This function must be called before any   */
				 /* other graphic function.                   */
{
 int graphdriver,                /* Driver for the graphic card.              */
     graphmode;                  /* Modes possible with the driver (high,low) */

 graphdriver= DETECT;        /* Automatic detection of the best available */
			     /* graphic mode.                             */

 registerbgidriver(EGAVGA_driver); /* Includes the driver in the linked lib*/
 registerbgifont(triplex_font);    /* Includes triplex font.               */
 initgraph(&graphdriver,&graphmode,"");
 setgraphmode(1);                 /* 640x400, 16 colors in VGA or EGA mode.    */
}

/**************************************************************************/

void draw_box(int w_numb,int color)
/* Draws the box #w_numb. */

{if (control==MOUSE) cursor_counter(2);
 setviewport(0,0,639,349,0);
 setcolor(color);
 rectangle(X1[w_numb],Y1[w_numb],X2[w_numb],Y2[w_numb]);
 if (control==MOUSE) cursor_counter(1);
}

/**************************************************************************/
void draw_screen(void)
/* This function draws the defined windows.   */

{
 int i=0;

 do
   {
    if ((enable_param[i] == INSTALLED) || (i > 20))
      draw_box(i,WHITE);
   }
    while (++i < N_WINDOW);

 /* Draw the boxes for intensity and wave */
 outline_box(143,286,14,14,WHITE,LIGHTGRAY);
 outline_box(157,286,82,14,WHITE,LIGHTGRAY);
 outline_box(281,286,14,14,WHITE,LIGHTGRAY);
 outline_box(295,286,82,14,WHITE,LIGHTGRAY);
 setcolor(LIGHTGREEN);
 outtextxy(146,290,"W");
 outtextxy(285,290,"I");
}

/************************************************************************/
void write_text(int color_ground,int x0,int y0,int length,int high,int color_text,char *text)
{
 setfillstyle(SOLID_FILL,color_ground);
 if (control==MOUSE) cursor_counter(2);
 bar(x0-3,y0-3,x0+8*length,y0+high);
 setcolor(color_text);
 outtextxy(x0,y0,text);
 if (control==MOUSE) cursor_counter(1);
}
/************************************************************************/

void new_cur(int color,int recmode)
/* Draw and erase the cursor by using the     */
{
 setviewport(0,0,639,349,0);    /* COPY_PUT mode.                             */
 setcolor(color);
 if ((recmode==MSU) && (X>=114))
   {line (X,86,X,86);}
 else
   {line(X,Y1[21]+6,X,Y2[21]-6);}
 if (ACCU==1) putpixel(X,accpoly[515-X]+1,LIGHTMAGENTA);
 else putpixel(X,newpoly[515-X]+1,LIGHTMAGENTA);
}

/***************************************************************************/

void er_cur()
/* erase the cursor by using the  COPY_PUT modein active page       */
{
 setwritemode(COPY_PUT);
 setcolor(BLACK);
 line(X,Y1[21]+6,X,Y2[21]-6);
 if (ACCU) putpixel(X,accpoly[515-X]+1,current_color);
 else putpixel(X,newpoly[515-X]+1,current_color);
}

/************************************************************************/

void param_text(void)
/* This function write the parameters name in the related window. */

{
 char sup[2]={24},sdown[2]={25};

 setcolor(LIGHTRED);
 outtextxy(X1[22]+12,Y1[22]+6,"ACQUISITION");

 if (enable_param[0] == INSTALLED)
	 {
	  setcolor(LIGHTRED);
	  outtextxy(X1[0]+16,Y1[0]+5,"F");
	  setcolor(WHITE);
	  outtextxy(X1[0]+16,Y1[0]+5," oremono :");
	 }

 if (enable_param[1] == INSTALLED)
	 {
          setcolor(LIGHTRED);
	  outtextxy(X1[1]+16,Y1[1]+5,"S");
	  setcolor(WHITE);
	  outtextxy(X1[1]+16,Y1[1]+5," pectro :");
	 }

 if (enable_param[2] == INSTALLED)
	 {
          setcolor(LIGHTRED);
	  outtextxy(X1[2]+5,Y1[2]+5,"B");
	  setcolor(WHITE);
          outtextxy(X1[2]+5,Y1[2]+18,"o");
	  outtextxy(X1[2]+5,Y1[2]+31,"t");
	  outtextxy(X1[2]+5,Y1[2]+43,"h");
	 }

 if (enable_param[3] == INSTALLED)
	 {
	  setcolor(LIGHTRED);
	  outtextxy(X1[3]+16,Y1[3]+5,"T");
	  setcolor(WHITE);
          outtextxy(X1[3]+16,Y1[3]+5," ime :");
	  outtextxy(X1[3]+85,Y1[3]+18,"s");
	 }

 if (enable_param[4] == INSTALLED)
	 {
	  setcolor(LIGHTRED);
	  outtextxy(X1[4]+16,Y1[4]+5,"D");
	  setcolor(WHITE);
          outtextxy(X1[4]+16,Y1[4]+5," elay :");
	  outtextxy(X1[4]+85,Y1[4]+18,"æs");
	 }

 if (enable_param[5] == INSTALLED)
	 {
	  setcolor(LIGHTRED);
	  outtextxy(X1[5]+16,Y1[5]+5,"W");
	  setcolor(WHITE);
          outtextxy(X1[5]+16,Y1[5]+5," idth :");
	  outtextxy(X1[5]+85,Y1[5]+18,"ms");
	 }

 if (enable_param[6] == INSTALLED)
	 {
          setcolor(LIGHTRED);
	  outtextxy(X1[6]+4,Y1[6]+7,"X");
	  setcolor(WHITE);
	  outtextxy(X1[6]+4,Y1[6]+7," :");
	  outtextxy(X1[6]+88,Y1[6]+7,".1æ");
	 }

 if (enable_param[7] == INSTALLED)
	 {
	  setcolor(LIGHTRED);
	  outtextxy(X1[7]+4,Y1[7]+7,"Y");
	  setcolor(WHITE);
	  outtextxy(X1[7]+4,Y1[7]+7," :");
	  outtextxy(X1[7]+88,Y1[7]+7,".1æ");
	 }
 if (enable_param[8] == INSTALLED)
	 {
	  setcolor(LIGHTRED);
	  outtextxy(X1[8]+4,Y1[8]+7,"J");
	  setcolor(WHITE);
	  outtextxy(X1[8]+4,Y1[8]+7," oystick: ");
	 }

 if (enable_param[9] == INSTALLED)
	 {
	  setcolor(LIGHTRED);
	  outtextxy(X1[9]+16,Y1[9]+5,"M");
	  setcolor(WHITE);
	  outtextxy(X1[9]+16,Y1[9]+5," ode :");
	 }

 if (enable_param[10] == INSTALLED)
	 {
	  setcolor(LIGHTRED);
	  outtextxy(X1[10]+16,Y1[10]+5,"U");
	  setcolor(WHITE);
	  outtextxy(X1[10]+16,Y1[10]+5," nits :");
	 }
 setcolor(WHITE);
 outtextxy(X1[20]+10,Y1[11]+12,"Scan");
 outtextxy(X1[19]+10,Y1[12]+12,"User");
 outtextxy(X1[18]+10,Y1[13]+12,"Acc.");
 outtextxy(X1[17]+10,Y1[14]+12,"Plot");
 outtextxy(X1[16]+10,Y1[15]+12,"Fwd ");
 outtextxy(X1[15]+10,Y1[16]+12,"Bwd ");
 outtextxy(X1[14]+10,Y1[17]+12,"Save");
 outtextxy(X1[13]+10,Y1[18]+12,"Memo");
 outtextxy(X1[12]+10,Y1[19]+12,"Zoom");
 outtextxy(X1[11]+22,Y1[20]+5,sup);
 outtextxy(X1[11]+22,Y1[20]+21,sdown);
 line(X1[11]+1,Y1[11]+16,X2[11]-1,Y1[11]+16);
}

/*************************************************************************/

void open_window(int left,int top,int right,int bottom,int color)

{
 getimage(left,top,right,bottom,buffer_image);
 setviewport(left,top,right,bottom,1);
 if (control==MOUSE) cursor_counter(2);
 clearviewport();
 setcolor(color);
 rectangle(0,0,right-left,bottom-top);
 rectangle(2,2,right-left-2,bottom-top-2);
 if (control==MOUSE) cursor_counter(1);
}

/*************************************************************************/

void close_window(int left,int top)

{
 setviewport(0,0,639,349,0);
 if (control==MOUSE) cursor_counter(2);
 putimage(left,top,buffer_image,COPY_PUT);
 if (control==MOUSE) cursor_counter(1);
}

/*************************************************************************/

int next_param(int i)
/* retrieve the next avalaible param. */
{
 while (enable_param[++i] != 1)
      {
       if (i > 20)
	    i=-1;
      }
 return(i);
}

/***************************************************************************/

int prev_param(int i)

{
 if (i<=0) i=21;
 while (enable_param[--i] != 1)
       {
	if (i == 0) i=21;
	}

 return(i);
}

/*************************************************************************/
void unzoom()
{
 FLAG_MEMO=0;
 enter_memo();

/*IMAX = (DAC_RESOL == 0) ? 4096 : 16384;  For PDA & OMA 3 */
 IMAX = 262144;   /* For OMA IV  16-10-91  mtc */
 IMIN=0;
 FIRDI=atoi(INTENS_BEGIN);
 LASTDI=atoi(INTENS_END);

/*DIV = (DAC_RESOL == 0) ? 16 : 64;  For PDA & OMA 3 */
 DIV=1024;        /* For OMA IV  16-10-91  mtc */
 if ((FIRDI%2) == 0) FIRDI++;
 if ((LASTDI%2) == 1) LASTDI--;
 YSTEP= ((LASTDI-FIRDI+1) > 512) ? 2: 1;
 XSTEP=512*YSTEP/(LASTDI-FIRDI+1);
 INF=FIRDI/YSTEP*XSTEP;
 setviewport(0,0,639,349,0);
 /*place_cursor(); 17-10-91 */
 if (ACCU) {erase_spacc();spacc_disp(NACC,ACCU,MS0);}
 else {erase_disp();quick_disp(0,0);}
 init_length();
 int_range();

 if (control==MOUSE) cursor_shape(0);
 ZOOMX=0;
}
/*************************************************************************/

void quick_disp(int disp,int recmode)

{
 int x,rank=0;
 unsigned long dummy;

 x=INF;
 if (control==MOUSE) cursor_counter(2);
 if (FLAG_MEMO) enter_memo();
 setviewport(4,1,515,266,1);
/* printf("FIRDI:%d LASTDI:%d YSTEP:%d\n",FIRDI,LASTDI,YSTEP);*/

 for (rank=FIRDI;rank<=LASTDI;rank+=YSTEP)
    {
     if ((recmode==MS0) || ((recmode==MSU) && (((511-x)<110) || (newpoly[x]>87))))
	 putpixel(511-x,newpoly[x],BLACK);
     /*if ((newpoly[x]=260-((intensite[rank]-IMIN)/DIV)) < 6)
	   newpoly[x]=6; pda oma3 version*/
     dummy=260-((ifinal[rank]-IMIN)/DIV);
     if ((newpoly[x]=(int) dummy) < 6)
	   newpoly[x]=6;    /* For oma IV 16-10-91 mtc */
     if (newpoly[x] > 264)
	  newpoly[x]=264;
     if ((recmode==MS0) || ((recmode==MSU) && (((511-x)<110) || (newpoly[x]>87))))
	 putpixel(511-x,newpoly[x],current_color);
     x+=XSTEP;
    }
 int_range();
 new_cur(LIGHTGRAY,recmode);
 if (!disp) cur_int();
 if (control==MOUSE) cursor_counter(1);
}
/*************************************************************************/

void spacc_disp(int current_acc,int fin_acc,int recmode)

{
 /****************************************************************************/
 /* Display during accumulation                                              */
 /* Display of the current data (intensite[]) and the current                */
 /* accumulation(ifinal[])                                                   */
 /* When accumulation is finished or stopped display of the result (ifinal[])*/
 /****************************************************************************/


 /* Modified for OMA4 */

 int x,rank=0,interm,y0;
 long dummy;

 if (fin_acc) y0=260;
 else y0=240;
 if ((CUR_DETECT==CCD) && (CCD_MODEL==OMA4))  y0=260;

 x=INF;
 if (control==MOUSE) cursor_counter(2);
 if (FLAG_MEMO) enter_memo();
 setviewport(4,1,515,266,1);

 for (rank=FIRDI;rank<=LASTDI;rank+=YSTEP)
    {
      if ((recmode==MS0) || ((recmode==MSU) && (((511-x)<110) || (newpoly[x]>87))))
	 putpixel(511-x,newpoly[x],BLACK);
      if ((recmode==MS0) || ((recmode==MSU) && (((511-x)<110) || (accpoly[x]>87))))
	 putpixel(511-x,accpoly[x],BLACK);

      /*interm=(ifinal[rank]/(long)current_acc);*/
      dummy=260-(((ifinal[rank]/(long)(current_acc))-IMIN)/DIV);

      if ((accpoly[x]=(int)dummy) < 6)
	   accpoly[x]=6;
      if (accpoly[x] > 264)
	  accpoly[x]=264;
      /*
      if (!fin_acc)
	{
	 if ((newpoly[x]=260-(((intensite[rank])-IMIN)/DIV)) < 6)
	    newpoly[x]=6;
	 if (newpoly[x] > 264)
	    newpoly[x]=264;
         if ((recmode==MS0) || ((recmode==MSU) && (((511-x)<110) || (newpoly[x]>87))))
	 putpixel(511-x,newpoly[x],LIGHTGREEN);
	}
      */
      if ((recmode==MS0) || ((recmode==MSU) && (((511-x)<110) || (accpoly[x]>87))))
	 putpixel(511-x,accpoly[x],YELLOW);
      x+=XSTEP;
    }
 int_range();
 cur_int();
 new_cur(LIGHTGRAY,MS0);
 if (control==MOUSE) cursor_counter(1);
}

/*************************************************************************/

void erase_spacc(void)

{
 int x=0;

 if (control==MOUSE) cursor_counter(2);
 setviewport(4,1,515,266,1);
 do
  {putpixel(511-x,accpoly[x],BLACK);
   } while (++x <= 512);
 new_cur(LIGHTGRAY,MS0);
 if (control==MOUSE) cursor_counter(1);
}

/*************************************************************************/

void erase_disp(void)

{
 int x=0;

 setviewport(4,1,515,266,1);
 if (ACCU)
   {
    do
      {
       putpixel(511-x,accpoly[x],BLACK);
      }
	while (++x <= 512);
   }
 else
   {
    do
      {
       putpixel(511-x,newpoly[x],BLACK);
      }
       while (++x <= 512);
   }
 new_cur(LIGHTGRAY,MS0);
}
/**************************************************************************/

void cur_int(void)

{
 int num;
 float time,it,sat,max_time;
 char s[10]="";

 max_time=atof(MAX_INT_TIME);

    sat=262144.0;
    it=(float)ifinal[FIRDI+((515-X-INF)/XSTEP*YSTEP)]/NACC;
    sprintf(s,"%8.1f",it);


 setviewport(296,287,376,299,1);
 clearviewport();
 setcolor(YELLOW);
 outtextxy(8,3,s);

 if (it == sat) strcpy(s,"SAT.");
 else if ((it <= 0) || ((time=sat/it*atof(INT_TIME)) > max_time)) strcpy(s," * ");
      else sprintf(s,"%7.2fs",time);
 setviewport(X2[21]-50,Y2[21]-12,X2[21]-1,Y2[21]-1,1);
 clearviewport();
 setcolor(WHITE);
 line(0,0,49,0);
 line(0,0,0,12);
 setcolor(YELLOW);
 outtextxy(5,3,s);
 setviewport(0,0,639,349,0);    /*you must restore the viewport before */
}                               /*opening a window.                    */

/*************************************************************************/

void int_range(void)

{
  char s[8]="";

 /*itoa(num,s,10);*/
 ltoa(IMAX,s,10);    /*mtc 17-10-91 modif oma4 */
 setviewport(X1[21]+1,Y1[21]+1,X1[21]+68,Y1[21]+13,1); /*18-10-91 mtc oma4 */
 clearviewport();
 setcolor(WHITE);
 line(0,12,49,12);
 line(49,0,49,12);
 setcolor(YELLOW);
 outtextxy(2,2,s);

 /*num=IMIN;
 itoa(num,s,10);*/
 ltoa(IMIN,s,10);  /* modif oma4 */
 setviewport(X1[21]+1,Y2[21]-13,X1[21]+50,Y2[21]-1,1);
 clearviewport();
 setcolor(WHITE);
 line(0,0,49,0);
 line(49,0,49,12);
 setcolor(YELLOW);
 outtextxy(2,3,s);
 setviewport(0,0,639,349,0);
}
/*************************************************************************/

void save_cur_image(void)

{
 char curslen[8];
 float wav;

 if (X < 39)
     left=3;
 else
   {
     if (X > 480)
	 left=445;
     else left=X-35;
   }
 setviewport(0,0,639,349,0);
 setviewport(158,287,238,299,0);
 clearviewport();
 switch (UNITS)
	{
	 case CM_1 : /*if (enable_param[1] == 1)
		       {*/
			wav=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(FIRDI+((515-X-INF)/XSTEP*YSTEP)));
			sprintf(curslen,"%7.1f",wav);
			setcolor(YELLOW);
			outtextxy(8,3,curslen);
		      /* }*/
		     break;

	 case  NM  : if (enable_param[1] == 1)
		       {
			 sprintf(curslen,"%7.3f",
			 diode_wavelength(FIRDI+((515-X-INF)/XSTEP*YSTEP))/10);
			 setcolor(YELLOW);
			 outtextxy(8,3,curslen);
		       }
		     break;

	 case  EV  : wav=(1e8/diode_wavelength(FIRDI+((515-X-INF)/XSTEP*YSTEP)))/8.0658; /* MTC 07/06/93 */
		     sprintf(curslen,"%7.1f",wav);
		     setcolor(YELLOW);
		     outtextxy(8,3,curslen);
		     break;

	 case  KBAR: wav=3808*(pow(14405/(1e8/diode_wavelength(FIRDI+((515-X-INF)/XSTEP*YSTEP))),5)-1);
		     sprintf(curslen,"%5.0f",wav);
		     break;

	 case  DIO : /*if (enable_param[1] == 1)*/
		     sprintf(curslen,"%4d",DIODE_NUMBER-OFFSET_DIOD-(FIRDI+((515-X-INF)/XSTEP*YSTEP)));
		     setcolor(YELLOW);
		     outtextxy(8,3,curslen);
		     break;
	}

 setviewport(0,0,639,349,0);
}
/*************************************************************************/

void restore_cur_image(void)

{
 setviewport(0,0,639,349,0);
 putimage(left,271,curlen_image,COPY_PUT);
}

/*************************************************************************/

void screen_ticks(void)

{
 setcolor(WHITE);
 setviewport(0,0,639,349,0);
 line(0,5,2,5);
 line(0,69,2,69);
 line(0,133,2,133);
 line(0,197,2,197);
 line(0,261,2,261);
 line(517,5,520,5);
 line(517,69,520,69);
 line(517,133,520,133);
 line(517,197,520,197);
 line(517,261,520,261);
 line(259,261,259,270);
 line(259,1,259,5);
}

/******************************************************************************/

void outline_box(int x0,int y0,int len,int wid,int color_cadre,int color_bord)
{
 int i;
 if (control==MOUSE) cursor_counter(2);
 setcolor(color_cadre);

 rectangle(x0,y0,x0+len,y0+wid);
 rectangle(x0-1,y0-1,x0+len+1,y0+wid+1);

 setcolor(color_bord);
 for (i=0;i<3;i++)
    {
      line(x0-1+i,y0-2-i,x0+len+1+i,y0-2-i);
      line(x0+len+2+i,y0+wid+1-i,x0+len+2+i,y0-2-i);
      putpixel(x0+len+2+i,y0-2-i,DARKGRAY);
    }
 if (control==MOUSE) cursor_counter(1);
}

/***************************************************************************/

void ms_disp(int over,int npts_ovlap,float *stp,long maximum,long minimum)
/* Used in automatic recording mode */

{
 int ii=383,ypixel,dio1,dio2;
 long int value=0;
 float dummy,yy;
 FILE *fp1;

 int jj,aa,aa0,pt_par_pixel=0;

 if (over)
   { dio1=atoi(INTENS_BEGIN)+npts_ovlap;}
 else
   {dio1=atoi(INTENS_BEGIN);}

 dio2=atoi(INTENS_END);

 yy=(float)(maximum-minimum)/63.0;
 setviewport(117,3,500,66,0);
 setcolor(WHITE);

 fp1=fopen("tmp.msu","a");

 aa0=0;

 for (jj=dio1;jj<=dio2;jj++)
    {
     dummy=(jj-dio1)* *stp;
     aa=(int)dummy;
     if (aa==aa0)
       {
	value=value+ifinal[jj]/(long)NACC;
	pt_par_pixel++;
       }
     else
       {
	value=value/(long)pt_par_pixel;
	fprintf(fp1,"%6ld\n",value);
	pt_par_pixel=1;aa0=aa;
	value=ifinal[jj]/(long)NACC;
       }
    }
 fclose(fp1);

 clearviewport();
 fp1=fopen("tmp.msu","r");
 fscanf(fp1,"%ld",&value);

 dummy=(value-minimum)/yy;
 ypixel=63-(int)(dummy);
 if (ypixel<0) ypixel=0;
 if (ypixel>63) ypixel=63;
 moveto(ii,ypixel);
 while (!feof(fp1))
      {
       fscanf(fp1,"%ld",&value);
       dummy=(value-minimum)/yy;
       ypixel=63-(int)(dummy);
       if (ypixel > 63 ) ypixel=63;
       if (ypixel < 0)  ypixel=0;
       lineto(ii--,ypixel);
      }
 fclose(fp1);
}
/**********************************************************************/
void no_detector_message(void)
{
 struct viewporttype viewinfo,curview;
 int xx0,yy0,xlen,yhi;
 char ch[23]="Hit a key to continue.";

 getviewsettings(&viewinfo);
 setviewport(4,1,515,266,0);
 getviewsettings(&curview);
 xlen=200;yhi=6*textheight(ch);

 xx0=((curview.right-curview.left)-xlen)/2;
 yy0=((curview.bottom-curview.top)-yhi)/2;
 setcolor(LIGHTRED);
 bar(xx0,yy0,xx0+xlen,yy0+yhi);
 setcolor(WHITE);
 rectangle(xx0,yy0,xx0+xlen,yy0+yhi);
 rectangle(xx0+3,yy0+3,xx0+xlen-3,yy0+yhi-3);

 setcolor(YELLOW);
 strcpy(ch,"NO DETECTOR FOUND !!! ");
 yy0=yy0+textheight(ch);
 xx0=xx0+10;
 outtextxy(xx0,yy0,ch);

 strcpy(ch,"Hit a key to continue.");
 yy0=yy0+3*textheight(ch);
 outtextxy(xx0,yy0,ch);
 getch();
 clearviewport();
 setviewport(viewinfo.left,viewinfo.top,viewinfo.right,viewinfo.bottom,0);
}
