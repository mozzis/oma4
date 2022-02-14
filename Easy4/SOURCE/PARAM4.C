/*========================================================================*/
/*                            PARAM.C                                     */
/*========================================================================*/

#include <acq4.h>
#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>
#include <string.h>
#include <dos.h>
#include <float.h>
#include <ctype.h>
#include <math.h>

/*========================================================================*/
extern int UNITS,ACQ_MODE,PDA_TYPE,DAC_RESOL,
	   DIODE_NUMBER,OFFSET_DIOD,
	   control,XMOUSE,YMOUSE,
	   CUR_DETECT,
	   enable_param[],
	   bd,ccd,
	   shutmode_ind,change_comnd,
	   X1[],Y1[N_WINDOW],Y2[], /* Coordinates of the available graphic */
				   /* windows. */
	   CCD_MODU,PULSOR_MODU,
	   NBR_PIXEL[],DAC[],
	   DOUBLER,SPEC_MODU,
	   flagjoystick;

extern char EXCLINE[],INT_TIME[],CUR_GRAT[],HD_MODE[],DETECT_MODE[],
	    CCD_TYPE[],
	    INTENS_BEGIN[],INTENS_END[],
	    TYPE[],PULSPARAM[],
	    UTIL_DIOD_BEG[2][5],UTIL_DIOD_END[2][5],
	    ANGLE[],FOCAL[],GRATING[];

extern FILE *fp;
/*========================================================================*/

char FORELENGTH[8],SPECTROLENGTH[8]=" 0000.0",
     DELAY[5],WIDTH[5],
     init_int_time[8],DARK_TIME[8],MAX_INT_TIME[8];

int act_modul,
    XSTEP,FIRDI,LASTDI,YSTEP,/*IMAX,IMIN=0,*/INF,DIV,
    bckgrd,
    code_dgamme,dstep,code_wgamme,wstep,
    CCD_MODEL,
    NOSTD, /* =1 if current grating != standard grating */
	   /* in this case spectrograph position must be corrected */
	   /* by the ratio between the 2 gratings */
	   /* =0 if current grating = standard grating */
	   /* MTC 26-05-93*/
    MONO2, /* 0 No 2nd monochromator      */
	   /* 1 2nd monochromator present */
	   /* MTC 26-05-93*/
    cur_grat_value;   /* MTC 26-05-93*/

float rap; /*Ratio between standard grating and current grating */
	   /* MTC 26-05-93 */

long IMAX,IMIN=0;

float position=0.0,
      pixel_size=0.019;  /* Size of detector pixel */
			 /* OMA4 Thomson chip 19 microns */
			 /* OMA4 EEV 1024 x 256 27 microns */

void set_shutter_mode(void);
/*========================================================================*/
void enter_posit(int scan_unit)

/* Enter the value of the selected position.            */
/* Positionning of the selected scan unit :Foremono,    */
/*					   Spectro. or  */
/*					   Both (Triple)*/

{
 char LENGTH[8];
 int pos,ok,shutmode_bck;
 float dummy;

 setviewport(7,307,512,345,0);
 clearviewport();

 setcolor(YELLOW);
 outtextxy(20,5,"ARROW KEYS to correct ,ENTER or left button to validate");
 outtextxy(20,17,"ESC or right button to quit! ");

 setcolor(WHITE);
 outtextxy(100,29,"Format :");

 switch (scan_unit)
       {
	case FOREMONO: strcpy(LENGTH,FORELENGTH);
		       break;

	case SPECTRO : strcpy(LENGTH,SPECTROLENGTH);
		       break;

	case TRIPLE  : strcpy(LENGTH,SPECTROLENGTH);
		       break;
       }

 setcolor(LIGHTGREEN);
 switch (UNITS)
       {
	 case DIO  :
	 case CM_1 : outtextxy(100,29,"         xxxxx.x  cm-1");
		     pos=edit(LENGTH,7,6,scan_unit,1);
		     position=atof(LENGTH);
		     break;

	 case  NM  : outtextxy(100,29,"         xxx.xxx  nm");
		     pos=edit(LENGTH,7,4,scan_unit,0);
		     position=(1e7/atof(EXCLINE))-(1e7/atof(LENGTH));
		     break;

	 case  EV  : outtextxy(100,29,"         xx.xxx  meV");
		     pos=edit(LENGTH,6,3,scan_unit,0);
		     break;
	}

 if (!pos)
   {
    switch (scan_unit)
	  {
	   case FOREMONO:
			  strcpy(FORELENGTH,LENGTH);
			  act_modul=FOREMONO;
			  break;

	   case SPECTRO :
			  strcpy(SPECTROLENGTH,LENGTH);
			  act_modul=SPECTRO;
			  break;

	   case TRIPLE  :
			  strcpy(FORELENGTH,LENGTH);
			  strcpy(SPECTROLENGTH,LENGTH);
			  break;
	  }

    if (!strncmpi(TYPE,"uv",2))
      {
       if (((long)(position*100)%20) != 0)
       position+=0.1;
      }

    if ((scan_unit==FOREMONO) || (scan_unit==TRIPLE)) ok=safety_message(&position);
    if ((scan_unit==SPECTRO) && (SPEC_MODU==UNINSTALLED)) ok=0;
    else ok=1;

    if (ok)
      {
       /* Shutter Closed */
       /*
       if (shutmode_ind!=CLOSED)
	 {
	  shutmode_bck=shutmode_ind;
	  shutmode_ind=CLOSED;
	  set_shutter_mode();
	  shutmode_ind=shutmode_bck;
	 }
       */
       /* Starting positionning */
       setviewport(7,307,512,345,0);
       clearviewport();
       write_text(YELLOW,200,9,14,10,LIGHTRED,"POSITIONNING !");
       setcolor(YELLOW);
       outtextxy(110,27,"Press the SPACE bar to stop positionning.");
       if ((NOSTD) && (!MONO2))
	 {
	  switch(act_modul)
	     {
	      case FOREMONO:positionning(&position,&act_modul);
			    break;
	      case SPECTRO: dummy=position;
			    position=((long)(position/rap*100))/100.0;
			    positionning(&position,&act_modul);
			    position=dummy;
			    break;

	      case TRIPLE:  act_modul=FOREMONO;
			    positionning(&position,&act_modul);
			    act_modul=SPECTRO;
			    dummy=position;
			    position=((long)(position/rap*100))/100.0;
			    positionning(&position,&act_modul);
			    position=dummy;
			    break;
	     }
	 }
       else
	 {
	  if (scan_unit==TRIPLE) bpositionning(&position);
	  else positionning(&position,&act_modul);
	 }

       close_window(3,303);
       init_length();
       /* Setting current shutter mode */
       set_shutter_mode();
      }
    else
      {init_length();
       close_window(3,303);
      }
   }
  /*Restore mouse*/
  if (control==MOUSE)
    { set_curs_pos(258,140);
      cursor_counter(1);
    }
}
/**************************************************************************/
void set_shutter_mode()
{
 set_shutter_forced_mode();
 setviewport(X1[9]+1,Y1[9]+18,X1[9]+65,Y2[9]-1,0);
 clearviewport();
 setcolor(WHITE);
 switch(shutmode_ind)
    {
     case LIVE:   outtextxy(16,0,"Live  ");
		  break;
     case CLOSED: outtextxy(16,0,"Closed");
		  break;
     case OPEN:   outtextxy(16,0,"Open  ");
		  break;
    }
}
/**************************************************************************/
void set_dark(void)
{
 int current_mode;

 current_mode=ACQ_MODE;
 ACQ_MODE=DARK;
 setviewport(X1[9]+1,Y1[9]+18,X1[9]+62,Y2[9]-1,0);
 clearviewport();
 setcolor(WHITE);
 outtextxy(16,0,"Dark ");
 set_time(init_int_time);
 ACQ_MODE=current_mode;
}
/**************************************************************************/
int safety_message (float *position)
{
 char c;
 int exit=1;

 if ((*position > -SECURITY) && (*position < SECURITY))
   {
       setviewport(7,307,512,345,0);
       clearviewport();
       printf("\a");
       write_text(LIGHTRED,40,9,53,10,YELLOW,"WARNING : NEW POSITION CLOSE TO THE EXCITATION LINE !");
       setcolor(WHITE);
       outtextxy(95,25,"Press ENTER to validate, ESC to cancel.");
       while (((c=getch()) != ENTER) && (c != ESC));
   }
 if (c==ESC) exit=0;
 return(exit);
}
/**************************************************************************/
void set_acq_mode(void)
{
 setviewport(X1[9]+1,Y1[9]+18,X1[9]+62,Y2[9]-1,0);
 clearviewport();
 setcolor(WHITE);
 switch(ACQ_MODE)
     {
      case DARK   :outtextxy(16,0,"Dark");
		   break;

      case SIGNAL :outtextxy(16,0,"Signal");
		   break;

      case S_D    :outtextxy(16,0,"S - D");
		   break;
     }
 set_time(INT_TIME);
}
/**************************************************************************/
void enter_time(void)
{
 setcolor(YELLOW);
 outtextxy(20,5,"ARROW KEYS to correct ,ENTER or left button to validate");
 outtextxy(20,17,"ESC or right button to quit! ");

 outtextxy(100,29,"Format :");
 setcolor(LIGHTGREEN);

 outtextxy(100,29,"        xxxx.xx s");
 edit(INT_TIME,7,5,3,0);

 set_time(INT_TIME);
 if (control==MOUSE)
   {
    set_curs_pos(258,140);
    cursor_counter(1);
   }
}

/**************************************************************************/
void set_time(char *cch)
{
  set_et();
}
/**************************************************************************/
void enter_mode(void)
{
 char c,
      ch[20];

 int pos,posmouse=0,i,select=0,cancel=0;

 setcolor(YELLOW);
 outtextxy(10,9,"Cancel with ESC. or right button.");

    strcpy(ch,"Live   Closed Open");
    switch (shutmode_ind)
	  {
	   case LIVE   : pos=0;
			 break;

	   case OPEN   : pos=2;
			 break;

	   case CLOSED : pos=1;
			 break;
	  }

 setcolor(WHITE);
 outtextxy(200,24,"Mode :");
 setcolor(YELLOW);
 rectangle(297,22,347,34);
 rectangle(297+56,22,347+56,34);
 rectangle(297+112,22,347+112,34);
 setfillstyle(SOLID_FILL,LIGHTBLUE);
 bar(298+56*pos,23,346+56*pos,33);

 outtextxy(300,24,ch);

 if (control == MOUSE)
   {
    mouse_xclip(6,510);
    mouse_yclip(310,340);
    set_curs_pos(320+(56*pos),331);
   }

 while ((!cancel) && (!select))
    {
     if (control==MOUSE)
       {
	cursor_shape(0);
	cursor_counter(1);
	while ((!posmouse) && (!kbhit()))
	     {
	      if (test_press(1)!=0)
		{cursor_counter(2);cancel=1;break;}
	      if (test_press(0)!=0)
		{
		 for (i=0;i<3;i++)
		    {
		     if ((XMOUSE>(300+56*i)) && (XMOUSE<(346+56*i))
			   && (YMOUSE>326) && (YMOUSE<336))

		       {
			cursor_counter(2);
			setfillstyle(SOLID_FILL,BLACK);
			bar(298+56*pos,23,346+56*pos,33);
			outtextxy(300,24,ch);
			pos=i;
			setfillstyle(SOLID_FILL,LIGHTBLUE);
			bar(298+56*pos,23,346+56*pos,33);
			outtextxy(300,24,ch);
			posmouse=1;select=1;
		       }
		    }
		}
	     }
       }
     if (cancel) break;
     if (!select)
       {
	if (control==MOUSE) cursor_counter(2);
	c=getch();
	if (c==ESC)
	  {
	   cancel=1;break;
	  }
	if (c==0) c=getch();
	switch (c)
	      {
	       case RIGHT:
			   setfillstyle(SOLID_FILL,BLACK);
			   bar(298+56*pos,23,346+56*pos,33);
			   outtextxy(300,24,ch);
			   pos++;
			   if (pos > 2) pos=0;
			   setfillstyle(SOLID_FILL,LIGHTBLUE);
			   bar(298+56*pos,23,346+56*pos,33);
			   outtextxy(300,24,ch);
			   break;

	       case LEFT:
			   setfillstyle(SOLID_FILL,BLACK);
			   bar(298+56*pos,23,346+56*pos,33);
			   outtextxy(300,24,ch);
			   pos--;
			   if (pos < 0) pos=2;
			   setfillstyle(SOLID_FILL,LIGHTBLUE);
			   bar(298+56*pos,23,346+56*pos,33);
			   outtextxy(300,24,ch);
			   break;

	       case ENTER:
			   select=1;
			   break;

	       default:
			  printf("\a");
			  break;
	      }
       }
    }
 if (!cancel)
   {
    if (pos!=shutmode_ind)
      {
	shutmode_ind=pos;
	set_shutter_mode();
      }
   }
 if (control==MOUSE)
   {
    mouse_xclip(0,639);
    mouse_yclip(0,349);
    set_curs_pos(258,140);
    cursor_counter(1);
   }
 }

/**************************************************************************/

void enter_units(void)
{
 char c;

 int pos,posmouse=0,i,
     select=0,cancel=0;

 pos=UNITS;

 setcolor(YELLOW);
 outtextxy(10,9,"Cancel with ESC. or right button ");

 setcolor(WHITE);
 outtextxy(200,24,"Units :");

 setfillstyle(SOLID_FILL,LIGHTBLUE);
 bar(288+44*UNITS,23,326+44*UNITS,33);
 setcolor(YELLOW);
 rectangle(287,22,327,34);
 rectangle(287+44,22,327+44,34);
 rectangle(287+88,22,327+88,34);
 rectangle(287+132,22,327+132,34);
 rectangle(287+176,22,327+176,34);
 outtextxy(290,24,"cm-1   nm   meV   dio  Kbar");

 if (control==MOUSE)
   {
    mouse_xclip(6,510);
    mouse_yclip(310,340);
    set_curs_pos(310+(44*pos),329);
    cursor_shape(0);
   }

 while ((!cancel) && (!select))
      {
       if (control==MOUSE)
	 {
	  cursor_counter(1);
	  test_press(0);test_press(1);
	  while ((!posmouse) && (!kbhit()))
	       {
		if (test_press(1)!=0)
		  {cursor_counter(2);cancel=1;break;}
		if (test_press(0)!=0)
		  {
		   for (i=0;i<5;i++)
		      {
		       if ((XMOUSE>(290+44*i)) && (XMOUSE<330+44*i)
			    && (YMOUSE>326) && (YMOUSE<336))
			 {
			  cursor_counter(2);
			  setfillstyle(SOLID_FILL,BLACK);
			  bar(288+44*pos,23,326+44*pos,33);
			  outtextxy(290,24,"cm-1   nm   meV   dio  Kbar");
			  pos=i;
			  setfillstyle(SOLID_FILL,LIGHTBLUE);
			  bar(288+44*pos,23,326+44*pos,33);
			  outtextxy(290,24,"cm-1   nm   meV   dio  Kbar");
			  posmouse=1;select=1;
			 }
		      }
		  }
	       }
	 }
       if (cancel) break;
       if (!select)
	 {
	  if (control==MOUSE) cursor_counter(2);
	  c=getch();
	  if (c==0) c=getch();
	  switch(c)
	    {
	     case RIGHT:
			 setfillstyle(SOLID_FILL,BLACK);
			 bar(288+44*pos,23,326+44*pos,33);
			 outtextxy(290,24,"cm-1   nm   meV   dio  Kbar");
			 pos++;
			 if (pos>4) pos=0;
			 setfillstyle(SOLID_FILL,LIGHTBLUE);
			 bar(288+44*pos,23,326+44*pos,33);
			 outtextxy(290,24,"cm-1   nm   meV   dio  Kbar");
			 break;

	    case LEFT:
			 setfillstyle(SOLID_FILL,BLACK);
			 bar(288+44*pos,23,326+44*pos,33);
			 outtextxy(290,24,"cm-1   nm   meV   dio  Kbar");
			 pos--;
			 if (pos<0) pos=4;
			 setfillstyle(SOLID_FILL,LIGHTBLUE);
			 bar(288+44*pos,23,326+44*pos,33);
			 outtextxy(290,24,"cm-1   nm   meV   dio  Kbar");
			 break;

	    case ENTER:
			 select=1;
			 break;

	    case ESC:
			cancel=1;
			break;

	    default:
			printf("\a");
			break;
	   }
      }
    }
  if (!cancel)
    {
      setviewport(X1[10]+1,Y1[10]+18,X1[10]+75,Y2[10]-1,0);
      clearviewport();
      setcolor(WHITE);
      switch (pos)
	   {case CM_1 : outtextxy(30,0,"cm-1");
			break;
	    case  NM  : outtextxy(30,0," nm");
			break;
	    case  EV  : outtextxy(30,0," meV");
			break;
	    case  DIO : outtextxy(30,0,"diode");
			break;
	    case KBAR : outtextxy(30,0," Kbar");
			break;
	  }
     UNITS=pos;
   }
 close_window(3,303);

 setfillstyle(SOLID_FILL,BLACK);
 bar(144,285,157,298);
 setcolor(LIGHTGREEN);
 if (UNITS==KBAR) outtextxy(146,290,"P");
 else outtextxy(146,290,"W");
 outtextxy(285,290,"I");
 init_length();
 save_cur_image();
 if (control==MOUSE)
   {
    mouse_xclip(0,639);
    mouse_yclip(0,349);
    set_curs_pos(258,140);
    cursor_counter(1);
   }
}

/************************************************************************/
void init_param(void)
/* init. of the screen and parameters. */

{
 int dmodel;

 draw_screen();
 new_cur(LIGHTGRAY,MS0);
 screen_ticks();
 param_text();
 int_range();
 /*if (!(enable_param[1])) {position=0;UNITS=DIO;}*/
 ACQ_MODE=DARK;

 strcpy(INT_TIME,"0001.00");
 strcpy(init_int_time,"0001.00");
 strcpy(DARK_TIME,"0001.00");
 bckgrd=0;
 /*DIODE_NUMBER=NBR_PIXEL[CUR_DETECT];*/

 if (CCD_MODU==INSTALLED)
   {
    dmodel=get_dmodel();
    if (dmodel==0)
      {
       no_detector_message();CCD_MODU=UNINSTALLED;
      }
    else
      {
       DIODE_NUMBER=get_activex();
       if ((dmodel==3) && (DIODE_NUMBER==1024) && (get_activey()==256))
	 {
	  pixel_size=0.027;
	 }
      }
   }
 FIRDI=atoi(INTENS_BEGIN);
 LASTDI=atoi(INTENS_END);

 if ((FIRDI%2) == 0) FIRDI++;
 if ((LASTDI%2) == 1) LASTDI--;

 YSTEP= ((LASTDI-FIRDI+1) > 512) ? 2: 1;
 XSTEP=(512*YSTEP)/(LASTDI-FIRDI+1);

 INF=FIRDI/YSTEP*XSTEP;

 /*IMAX = (DAC_RESOL == 0) ? 4096 : 16384;  For PDA & OMA 3 */
 IMAX = 262144;   /* For OMA IV  16-10-91  mtc */
 /*DIV = (DAC_RESOL == 0) ? 16 : 64;  For PDA & OMA 3 */
 DIV=1024;        /* For OMA IV  16-10-91  mtc */

 if (enable_param[3])
   {
     setcolor(WHITE);
     outtextxy(X1[3]+17,Y1[3]+18,INT_TIME);
   }
 if (enable_param[8])
   {
     flagjoystick=0;
     setcolor(WHITE);
     outtextxy(X1[8]+80,Y1[8]+7,"OFF");
   }

 if (enable_param[9])
   {
       /* Read shutmode_ind */

	switch (shutmode_ind)
	      {
	       case 0:outtextxy(X1[9]+16,Y1[9]+18,"Live");
		      break;
	       case 1:outtextxy(X1[9]+16,Y1[9]+18,"Closed");
		      break;
	       case 2:outtextxy(X1[9]+16,Y1[9]+18,"Open");
		      break;
	      }

   }
 set_time(INT_TIME);
 if (enable_param[10]==INSTALLED)
   {
     setviewport(X1[10]+1,Y1[10]+18,X1[10]+75,Y2[10]-1,0);
     clearviewport();
     setcolor(WHITE);
     switch (UNITS)
	   {
	    case CM_1 : outtextxy(30,0,"cm-1");
			break;
	    case  NM  : outtextxy(30,0," nm");
			break;
	    case  EV  : outtextxy(30,0," meV");
			break;
	    case  DIO : outtextxy(30,0,"diode");
			break;
	    case KBAR : outtextxy(30,0," Kbar");
			break;
	   }
    }
}

/***************************************************************************/
void init_length(void)
/* init central and intensified wavelength */

{
 char upper_length[9],lower_length[9];
 int a,j,beg,end;
 float wav,ppo;

 if (enable_param[0] == INSTALLED)
   {
     act_modul=FOREMONO;
     position=read_position(&act_modul);
     switch (UNITS)
	   {
	     case DIO  :
	     case CM_1 : sprintf(FORELENGTH,"% 06.1f",position);
			 break;

	     case  NM  : ppo=1e7/((1e7/atof(EXCLINE))-position);
			 sprintf(FORELENGTH,"%7.3f",ppo);
			 break;
	     case  EV  : ppo=((1e7/atof(EXCLINE))-position)/8.0658;  /*mtc 03/06/93  meV */
			 sprintf(FORELENGTH,"%7.1f",ppo);
			 break;
	   }
     setviewport(X1[0]+1,Y1[0]+13,
		 X1[0]+80,Y2[0]-1,0);
     clearviewport();
     setcolor(WHITE);
     outtextxy(16,5,FORELENGTH);
   }
 if (enable_param[1] == INSTALLED)
   {
     act_modul=SPECTRO;
     if (SPEC_MODU==1)
       {
	position=read_position(&act_modul);
	position=read_position(&act_modul);
	if ((!MONO2) && (NOSTD)) position=position*rap;
	/*diode_wavelength();*/
       }
     switch (UNITS)
	   {
	     case CM_1 : sprintf(SPECTROLENGTH,"% 06.1f",position);
			 wav=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(LASTDI));
			 sprintf(upper_length,"%5d",(int)(wav));

			 wav=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(FIRDI));
			 sprintf(lower_length,"%5d",(int)(wav));
			 break;

	     case  NM  : sprintf(upper_length,"%5d",(int)
				     (diode_wavelength(LASTDI)/10));
			 sprintf(lower_length,"%5d",(int)
				     (diode_wavelength(FIRDI)/10));

			 ppo=1e7/((1e7/atof(EXCLINE))-position);
			 sprintf(SPECTROLENGTH,"%7.3f",ppo);
			 break;

	    case DIO   :
			 sprintf(SPECTROLENGTH,"% 06.1f",position);
			 sprintf(upper_length,"%4d",DIODE_NUMBER-OFFSET_DIOD-LASTDI);
			 sprintf(lower_length,"%4d",DIODE_NUMBER-FIRDI-OFFSET_DIOD);
			 break;

	    case EV    : wav=(1e8/diode_wavelength(LASTDI))/8.0658; /*MTC 03/06/93 */
			 sprintf(upper_length,"%7.1f",wav);
			 wav=(1e8/diode_wavelength(FIRDI))/8.0658;  /*MTC 03/06/93 */
			 sprintf(lower_length,"%7.1f",wav);
			 ppo=((1e7/atof(EXCLINE))-position)/8.0658; /*MTC 03/06/93 */
			 sprintf(SPECTROLENGTH,"%7.1f",ppo);
			 break;
	    case KBAR : wav=3808*(pow(14405/(1e8/diode_wavelength(LASTDI)),5)-1);
			sprintf(upper_length,"%5.0f",wav);
			wav=3808*(pow(14405/(1e8/diode_wavelength(FIRDI)),5)-1);
			sprintf(lower_length,"%5.0f",wav);
			break;
	   }
     setviewport(X1[1]+1,Y1[1]+13,
		 X1[1]+80,Y2[1]-1,0);
     clearviewport();

     setcolor(WHITE);
     outtextxy(16,5,SPECTROLENGTH);
     setviewport(0,267,515,269,0);
     clearviewport();
     if (UNITS == CM_1)
       {
	 beg=(int) lower_length/100.0;beg++;
	 end=(int) upper_length/100.0;
	 /*
	 for (j=beg*100;j<=end*100;j+=100)
	      line(515-INF-((diode_position(&j)-FIRDI+1)*XSTEP/YSTEP),
		   0,515-INF-((diode_position(&j)-FIRDI+1)*XSTEP/YSTEP),2);*/
       }
     line(515-INF,0,515-INF,2);
     line(515-INF-((LASTDI-FIRDI+1)*XSTEP/YSTEP),0,515-INF-((LASTDI-FIRDI+1)*XSTEP/YSTEP),2);
     setcolor(LIGHTGREEN);
     setviewport(0,271,515,281,1);
     clearviewport();
     if ((a=490-INF-((LASTDI-FIRDI+1)*XSTEP/YSTEP)) < 0) a=0;
     outtextxy(a,0,upper_length);
     if ((a=485-INF) > 480) a=470;
     outtextxy(a,0,lower_length);
     save_cur_image();
   }
 setviewport(0,0,639,349,0);
}

/***************************************************************************/

void enter_char(void)

{
 char c,
      *charac[4]={EXCLINE,DETECT_MODE,CUR_GRAT,HD_MODE};

 int ind=0,select=0,action,annul=0,insert=0,
     param_length[]={7,3,5,1},
     x[]={138,410,138,410},
     y[]={4,4,16,16},
     x0=7,y0=307;

 if (control==MOUSE) cursor_counter(2);

 open_window(3,303,516,349,LIGHTGREEN);

 setviewport(7,307,512,345,0);
 setcolor(YELLOW);
 outtextxy(2,29,"Space or left button to validate,Esc or right button to quit");

 setcolor(WHITE);
 outtextxy(2, 4,"Exc. line (nm).:           Detector (PDA/CCD).....:");
 outtextxy(2,16,"Grating........:           Mode (Normal/High disp):");

 setcolor(LIGHTRED);
 outtextxy(138,4,EXCLINE);
 outtextxy(138,16,CUR_GRAT);
 outtextxy(410,16,HD_MODE);
 outtextxy(410,4,DETECT_MODE);
 write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,charac[ind]);
 select=0;

 do
   {
    action=0;
    do
      {
       if (control==MOUSE)
	 {
	  if (test_press(0)!=0) {c=SPACE;action=1;}
	  else if (test_press(1)!=0) {c=ESC;action=1;}
	 }
       if ((kbhit()) && (!action)) {c=getch();action=1;}
      }
       while (!action);

       if (c==ESC) {annul=1;select=1;}
       if (c==SPACE)
	 {
	  write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,charac[ind]);
	  select=1;
	 }
       if (c==0) c=getch();
       switch(c)
	 {
	  case UP    :
		      if (ind>1)
			{
			 write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,charac[ind]);
			 ind-=2;
			 write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,charac[ind]);
			}
		      break;

	  case DOWN  :
		      if ((ind!=3) && (ind!=2))
			{
			 write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,charac[ind]);
			 ind+=2;
			 write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,charac[ind]);
			}
		      break;

	  case LEFT  :
		      if (ind!=0)
			{
			 write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,charac[ind]);
			 ind--;
			 write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,charac[ind]);
			}
		      break;

	  case RIGHT :
		      if (ind!=3)
			{
			 write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,charac[ind]);
			 ind++;
			 write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,charac[ind]);
			}
		      break;

	  case ENTER :
		      if ((ind==0)||(ind==2))
			{
			 edit_string(param_length[ind],x[ind]+x0,y[ind]+y0,charac[ind],&insert);
			}
		      else if (ind==1)
			{
			 strcpy(charac[ind],select_par(ind,CUR_DETECT,x[ind],y[ind]));
			}
		      else
			{
			 if (strcmp(HD_MODE,"H")==0)
			   {strcpy(charac[ind],select_par(ind,1,x[ind],y[ind]));}
			 else
			   {strcpy(charac[ind],select_par(ind,0,x[ind],y[ind]));}
			}
		      setviewport(7,307,512,345,0);
		      write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,charac[ind]);
		      if (ind!=3) ind++;
		      else ind=0;
		      write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,charac[ind]);
		      break;
	  }
   }
   while (!select);

 close_window(3,303);
 if (annul) read_param();
 else save_param();

 if ((CUR_GRAT[strlen(CUR_GRAT)-1]=='M') || (CUR_GRAT[strlen(CUR_GRAT)-1]=='m'))
   {
    MONO2=1;rap=1.0;NOSTD=0;
    CUR_GRAT[strlen(CUR_GRAT)-1]='\0';cur_grat_value=atoi(CUR_GRAT);
    CUR_GRAT[strlen(CUR_GRAT)]='M';
   }
 else
   {
    MONO2=0;cur_grat_value=atoi(CUR_GRAT);
    if (atoi(GRATING)!=cur_grat_value)
      {
       NOSTD=1;rap=atof(CUR_GRAT)/atof(GRATING);
      }
    else NOSTD=0;
   }

 if (MONO2)
   {
    if (cur_grat_value==150)   /*20-02-92 mtc Modif special */
      {                        /*si reseau 150  lire config MS cf GILLET Rennes*/
       fp=fopen("msconfig","rt");
       fscanf(fp,"%s %s %d",ANGLE,FOCAL,&DOUBLER);
       fclose(fp);
      }
    else
      {
       if (cur_grat_value==600)
	 {
	  fp=fopen("config60","rt");
	  fscanf(fp,"%s %s %d",ANGLE,FOCAL,&DOUBLER);
	  fclose(fp);
	 }
       else
	 {
	  read_characteristics();
	 }
      }
   }

 if (control==MOUSE)
   {cursor_shape(0);cursor_counter(1);}
}


/******************************************************************************/
char *select_par(int ii,int aa,int x,int y)
{
 char det[2][4]={"PDA","CCD"},
      mod[2][2]={"N","H"},
      tab[2][4]={"",""},cc,*ch={""};
 int i,xx[2],yy[2],x0,choix,len,fond;
 x=x+2;
 if (ii==1) {strcpy(tab[0],det[0]);strcpy(tab[1],det[1]);x0=35;len=3;}
 else {strcpy(tab[0],mod[0]);strcpy(tab[1],mod[1]);x0=20;len=1;}

 xx[0]=x;xx[1]=x+x0;
 yy[0]=yy[1]=y;
 choix=0;
 setcolor(YELLOW);
 for (i=0;i<2;i++)
    {
     rectangle(xx[i]-4,yy[i]-4,xx[i]+1+8*len,yy[i]+11);
     if (i==aa) fond=LIGHTBLUE;
     else fond=BLACK;
     write_text(fond,xx[i],yy[i],len,10,YELLOW,tab[i]);
    }
 i=aa;
 do
   {
    cc=getch();
    if (cc==ESC) break;
    if (cc==0) cc=getch();
    switch (cc)
       {
	case LEFT :
		    write_text(BLACK,xx[i],yy[i],len,10,YELLOW,tab[i]);
		    if (i==1) i=0;
		    else i++;
		    write_text(LIGHTBLUE,xx[i],yy[i],len,10,YELLOW,tab[i]);
		    break;

	case RIGHT:
		    write_text(BLACK,xx[i],yy[i],len,10,YELLOW,tab[i]);
		    if (i==0) i=1;
		    else i--;
		    write_text(LIGHTBLUE,xx[i],yy[i],len,10,YELLOW,tab[i]);
		    break;

	case ENTER:
		    strcpy(ch,tab[i]);
		    setfillstyle(SOLID_FILL,BLACK);
		    bar(xx[0]-4,yy[0]-4,xx[1]+1+8*len,yy[1]+11);
		    if (ii==1) CUR_DETECT=i;
		    choix=1;
		    break;
       }
   }
    while (!choix);
 return(ch);

}
/***************************************************************************/
void init_var(void)
{
 int temp;

 if ((CUR_GRAT[strlen(CUR_GRAT)-1]=='M') || (CUR_GRAT[strlen(CUR_GRAT)-1]=='m'))
   {
    MONO2=1;rap=1.0;NOSTD=0;
    CUR_GRAT[strlen(CUR_GRAT)-1]='\0';cur_grat_value=atoi(CUR_GRAT);
    CUR_GRAT[strlen(CUR_GRAT)-1]='M';
   }
 else
   {
    MONO2=0;cur_grat_value=atoi(CUR_GRAT);
    if (atoi(GRATING)!=cur_grat_value)
      {
       NOSTD=1;rap=atof(CUR_GRAT)/atof(GRATING);
      }
    else NOSTD=0;
   }

 if (MONO2)
   {
    if (cur_grat_value==150)   /*20-02-92 mtc Modif special */
      {                        /*si reseau 150  lire config MS cf GILLET Rennes*/
       fp=fopen("msconfig","rt");
       fscanf(fp,"%s %s %d",ANGLE,FOCAL,&DOUBLER);
       fclose(fp);
      }
    else
      {
       if (cur_grat_value==600)
	 {
	  fp=fopen("config60","rt");
	  fscanf(fp,"%s %s %d",ANGLE,FOCAL,&DOUBLER);
	  fclose(fp);
	 }
      }
   }
 if (PULSOR_MODU) read_pulsor_param();
 if (CCD_MODU) read_frame();

    CUR_DETECT=CCD;
    strcpy(MAX_INT_TIME,"9999.99");
    CCD_MODEL=OMA4;

 DAC_RESOL=DAC[CUR_DETECT];

    DIODE_NUMBER=NBR_PIXEL[CUR_DETECT];
    strcpy(INTENS_BEGIN,UTIL_DIOD_BEG[CUR_DETECT]);
    strcpy(INTENS_END,UTIL_DIOD_END[CUR_DETECT]);
}
/****************************************************************************/