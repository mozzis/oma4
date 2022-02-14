/*========================================================================*/
/*                                 MENU.C                                 */
/*========================================================================*/
/*                                                                        */
/* Main menu driving functions.                                           */
/*                                                                        */
/*========================================================================*/

#include <conio.h>
#include <stdio.h>
#include <decl0.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <graphics.h>
#include <dos.h>
#include <acq4.h>
#include <detdriv.h>
#include <access4.h>
#include <counters.h>
/*========================================================================*/

extern int  bd,PDA_TYPE,TRIGGER,CCD_MODU,NBR_PIXEL[],DAC[],
	    CCD_MODEL,
	    enable_param[],
	    control,XMOUSE,MOVING,
	    /*IMIN,IMAX,*/FIRDI,LASTDI,XSTEP,YSTEP,INF,DIV,
	    ACQ_MODE,NACC,
	    ZOOMX,
	    bckgrd,fin,current_color,
	    shutmode_ind,
	    MULTI_MODU,XC[],change_comnd,
	    PULSOR_MODU,FORE_MODU,SPEC_MODU,
	    MICRO_PLATE_MODU,flagjoystick,MAPPING,endacq;

extern char UTIL_DIOD_BEG[2][5],
	    UTIL_DIOD_END[2][5],
	    CCD_TYPE[];

/*========================================================================*/

int para_numb=-1,
    FLAG_MEMO=0,pos_memo,
    X=259,
    DAC_RESOL,CUR_DETECT,DIODE_NUMBER,
    shell=0,
    ACCU=0,ret_accu=1,
    MOVING=0;              /* Flag indicating if a motor is moving */
			   /* 0 All motors stopped                 */
			   /* 1 FORE is moving                     */
			   /* 2 Spect is moving                    */
			   /* 3 fore & spect are moving            */

int X1[N_WINDOW]={526,526,624,526,526,526,526,526,526,526,526,
		   /* 3, 56,107,158,209,260,311,362,413,464,*/
		  464,413,362,311,260,209,158,107, 56,  3,
		    3,526},

    Y1[N_WINDOW]={ 28, 61, 28, 97,133,164,200,222,244,271,307,
		  307,307,307,307,307,307,307,307,307,307,
		    0,  0},

    X2[N_WINDOW]={624,624,639,639,639,639,639,639,639,639,639,
		  /* 56,107,158,209,260,311,362,413,464,516,*/
		  516,464,413,362,311,260,209,158,107, 56,
		  516,639},

    Y2[N_WINDOW]={ 61, 92, 92,128,164,195,222,244,266,302,340,
		  340,340,340,340,340,340,340,340,340,340,
		  266, 18};

 char OPERATOR[9],SAMPLE[9],
     SLITWIDTH[4],SPECWIDTH[8],
     EXCLINE[8],LASPOW[4],
     PATH[41],REMARK[51],
     CUR_GRAT[6],HD_MODE[2],DETECT_MODE[4],
     INTENS_BEGIN[5],INTENS_END[5],
     INT_TIME[8],
     *par[12]={OPERATOR,SAMPLE,EXCLINE,LASPOW,SLITWIDTH,SPECWIDTH,REMARK,PATH,CUR_GRAT,HD_MODE,DETECT_MODE};

 extern char MICRO_PLATE_PORT[];
/*========================================================================*/

void init_adj(void)

/*  init of the adjustment mode. */
{
 char ch[5];

 read_characteristics();
 read_param();
 init_var();

 read_coeff();read_offset();

 if ((FORE_MODU==INSTALLED) || (SPEC_MODU==INSTALLED))
   {
     gpib_init();
     ibtmo(bd,T3s);
     ibdma(bd,1);
   }
 if (MICRO_PLATE_MODU)
   {
    init_plate_comm();
    read_xplate();read_yplate();
   }
 able_param();
 init_param();
 init_length();

 setviewport(10,287,125,299,0);
 write_text(BLACK,0,2,13,10,BLACK,"F5 for ADJUST");
 setviewport(400,287,500,299,0);
 write_text(CYAN,10,2,6,10,LIGHTCYAN,"ADJUST");

 setviewport(3,287,33,299,0);
 strcpy(ch,CCD_TYPE);
 write_text(LIGHTGRAY,5,2,4,10,WHITE,ch);

 setviewport(0,0,639,349,0);
 init_mouse();
 if (control==MOUSE) {test_press(0);test_press(1);}
 para_numb=19;
 if (control==KEYBOARD) draw_box(para_numb,LIGHTRED);
}

/*************************************************************************/

void adjust(char *c)

/* main menu of adjustment. */

{
 switch (*c)
       {
	case 0 : *c=getch();      /* extended ascii code.                */
		  switch (*c)
			{
			 case PG_DOWN   :/* if (control == MOUSE) break;*/
					  draw_box(para_numb,WHITE);
					  para_numb=next_param(para_numb);
					  draw_box(para_numb,LIGHTRED);
					  break;

			 case PG_UP     :/* if (control == MOUSE) break;*/
					  draw_box(para_numb,WHITE);
					  para_numb=prev_param(para_numb);
					  draw_box(para_numb,LIGHTRED);
					  break;

			 case    LEFT   :
					  if (control==MOUSE) cursor_counter(2);
					  er_cur();
					  if (X <= (515-INF-((LASTDI-FIRDI)*XSTEP/YSTEP)))
					    {X=515-INF-((LASTDI-FIRDI)*XSTEP/YSTEP);}
					  else X-=XSTEP;
					  /*restore_cur_image();*/
					  if (FLAG_MEMO) enter_memo();
					  new_cur(LIGHTGRAY,0);
					  save_cur_image();
					  /*cur_int(ACCU);pda oma3*/
					  cur_int();/*oma4 21-09-92 */
					  if (control==MOUSE) cursor_counter(1);
					  fflush(stdin);
					  break;

			 case    RIGHT   :
					   if (control==MOUSE) cursor_counter(2);
					   er_cur();
					   if (X >= (515-INF)) X=515-INF;
					   else X+=XSTEP;
					   /*restore_cur_image();*/
					   if (FLAG_MEMO) enter_memo();
					   new_cur(LIGHTGRAY,0);
					   save_cur_image();
					   /*cur_int(ACCU); pda oma3 */
					   cur_int(); /*OMA4 21-09-92 */
					   if (control==MOUSE) cursor_counter(1);
					   fflush(stdin);
					   break;

			 case CTRL_RIGHT :
					   if (control==MOUSE) cursor_counter(2);
					   er_cur();
					   if (X >= (515-INF-(5*XSTEP))) X=515-INF-(5*XSTEP);
					   else X+=(5*XSTEP);
					   /*restore_cur_image();*/
					   if (FLAG_MEMO) enter_memo();
					   new_cur(LIGHTGRAY,0);
					   save_cur_image();
					   /*cur_int(ACCU);pda oma3 */
					   cur_int(); /* OMA4 21-09-92 */
					   if (control==MOUSE) cursor_counter(1);
					   fflush(stdin);
					   break;

			 case CTRL_LEFT  :
					   if (control==MOUSE) cursor_counter(2);
					   er_cur();
					   if (X <= ((515-INF-((LASTDI-FIRDI)*XSTEP/YSTEP))+(5*XSTEP)))
					     {X=(515-INF-((LASTDI-FIRDI)*XSTEP/YSTEP))+(5*XSTEP);}
					   else X-=(5*XSTEP);
					   /*restore_cur_image();*/
					   if (FLAG_MEMO) enter_memo();
					   new_cur(LIGHTGRAY,0);
					   save_cur_image();
					   /*cur_int(ACCU);oma3 pda*/
					   cur_int(); /* oma4 21-09-92 */
					   if (control==MOUSE) cursor_counter(1);
					   fflush(stdin);
					   break;

			 case CTRL_END   : fin=1;endacq=0;
					   break;
			 /*
			 case     f1     :
					   if (MOVING==1) break;
					   TRIGGER^=1;
					   set_integ_time(INT_TIME,NACC);
					   setviewport(X1[8]+72,Y1[8]+5,X2[8]-1,Y1[8]+13,0);
					   clearviewport();
					   if (TRIGGER == 1) outtextxy(0,0,"TRIG.");
					   break;
			 */
			 case     f1     : stop_acq(0);ccd_description();change_comnd=1;
					   break;

			 case     f2     : /*Parameters setting*/
					   enter_char();
					   init_length();
					   break;

			 case     f3     : /*Frame setting for CCD detector*/
					   if (CCD_MODEL==OMA4) SetParam(STOP,0);
					   enter_ccd_frame_setup();change_comnd=1;
					   break;

			 case     f4     : if (MICRO_PLATE_MODU) init_xy();
					   break;

			 case     f5     : /*Restart Adjust mode*/
					   /*Available only at the end of accumulation*/
					   if (ACCU==1)
					     {
					      setviewport(X1[9]+1,Y1[9]+18,X1[9]+62,Y2[9]-1,0);
					      clearviewport();
					      setcolor(WHITE);
					      switch(shutmode_ind)
						    {
						     case CLOSED: outtextxy(16,0,"CLOSED");
								  break;
						     case LIVE  : outtextxy(16,0,"LIVE");
								  break;
						     case OPEN  : outtextxy(16,0,"OPEN");
								  break;
						    }
						 set_shutter_forced_mode();
						 change_comnd=1;

					     setviewport(0,0,639,349,0);
					     write_text(BLACK,X1[3]+17,Y1[3]+18,7,10,WHITE,INT_TIME);
					     setviewport(10,287,125,299,0);
					     write_text(BLACK,0,2,13,10,BLACK,"F5 for ADJUST");
					     setviewport(400,287,500,299,0);
					     write_text(CYAN,10,2,6,10,LIGHTCYAN,"ADJUST");
					     setviewport(3,287,33,299,0);


					     write_text(LIGHTGRAY,5,2,4,10,WHITE,CCD_TYPE);

					     setviewport(0,0,639,349,0);
					     erase_spacc();
					     current_color=LIGHTGREEN;
					     ACCU=0;ret_accu=1;
					     /*OMA4 */
					     NACC=1;set_scans();
					     }
					   break;

			 case     f8     : /*Set system configuration */
					   set_system_characteristics();
					   if (control==MOUSE) cursor_counter(2);
					   if (ACCU==1)
					     {setviewport(10,287,125,299,0);
					      write_text(YELLOW,0,2,13,10,RED,"F5 for ADJUST");
					      setviewport(400,287,500,299,0);
					      write_text(CYAN,10,2,6,10,LIGHTCYAN,"ACCUM.");
					      setviewport(0,0,639,349,0);
					     }
					   else
					     {setviewport(400,287,500,299,0);
					      write_text(CYAN,10,2,6,10,LIGHTCYAN,"ADJUST");
					      setviewport(3,287,33,299,0);
					      write_text(LIGHTGRAY,5,2,4,10,WHITE,CCD_TYPE);
					     }
					   change_comnd=1;
					   setviewport(0,0,639,349,0);
					   if (control==MOUSE) cursor_counter(1);
					   break;

			 case     f9     : /*SHELL function*/
					   if (control==MOUSE) cursor_counter(2);
					   setactivepage(1);
					   setviewport(0,0,639,349,0);
					   clearviewport();
					   setvisualpage(1);
					   setcolor(LIGHTRED);
					   outtextxy(10,10,"Type EXIT to return to EASY-PEAK...");
					   os();  /* Shell function */
					   system(strcat("cd ",getenv("CURRENT")));
					   read_param();

					   setactivepage(0);
					   setvisualpage(0);
					   if (control==MOUSE) cursor_counter(1);
					   shell=1;
					   break;

			 case     f0     :if (control==MOUSE)
					    {cursor_counter(2);
					     control=KEYBOARD;
					     draw_box(para_numb,LIGHTRED);
					    }
					  else
					    if (enable_mouse() != -1) control=KEYBOARD;
					    else
					      {
					       control=MOUSE;
					       draw_box(para_numb,WHITE);
					       init_mouse();
					      }
					  break;

			 case   HOME     : if (ZOOMX) unzoom();
					   break;
			}
		 break;

	case 'f' :
	case 'F' : if ((enable_param[0]))/* && (control == KEYBOARD))*/
		     {
		      draw_box(para_numb,WHITE);
		      para_numb=0;
		      draw_box(para_numb,LIGHTRED);
		     }
		   break;

	case 's' :
	case 'S' : if ((enable_param[1]))/* && (control == KEYBOARD))*/
		     {
		      draw_box(para_numb,WHITE);
		      para_numb=1;
		      draw_box(para_numb,LIGHTRED);
		     }
		   break;

	case 'b' :
	case 'B' : if ((enable_param[2]))/* && (control == KEYBOARD))*/
		     {
		      draw_box(para_numb,WHITE);
		      para_numb=2;
		      draw_box(para_numb,LIGHTRED);
		     }
		   break;

	case 't' :
	case 'T' : if ((enable_param[3]))/* && (control == KEYBOARD))*/
		     {
		      draw_box(para_numb,WHITE);
		      para_numb=3;
		      draw_box(para_numb,LIGHTRED);
		     }
		   break;

	case 'd' :
	case 'D' : if ((enable_param[4]))/* && (control == KEYBOARD))*/
		     {
		      draw_box(para_numb,WHITE);
		      para_numb=4;
		      draw_box(para_numb,LIGHTRED);
		     }
		   break;

	case 'w' :
	case 'W' : if ((enable_param[5]))/*  && (control == KEYBOARD))*/
		     {
		      draw_box(para_numb,WHITE);
		      para_numb=5;
		      draw_box(para_numb,LIGHTRED);
		     }
		   break;

	case 'x' :
	case 'X' : if ((enable_param[6]))/* && (control == KEYBOARD))*/
		     {
		      draw_box(para_numb,WHITE);
		      para_numb=6;
		      draw_box(para_numb,LIGHTRED);
		     }
		   break;

	case 'y' :
	case 'Y' : if ((enable_param[7]))/* && (control == KEYBOARD))*/
		     {
		      draw_box(para_numb,WHITE);
		      para_numb=7;
		      draw_box(para_numb,LIGHTRED);
		     }
		   break;

	case 'j' :
	case 'J' : if (enable_param[8])
		     {
		      draw_box(para_numb,WHITE);
		      para_numb=8;
		      draw_box(para_numb,LIGHTRED);
		     }
		   break;

	case 'm' :
	case 'M' : if ((enable_param[9]))/* && (control == KEYBOARD))*/
		     {
		      draw_box(para_numb,WHITE);
		      para_numb=9;
		      draw_box(para_numb,LIGHTRED);
		     }
		   break;

	case 'u' :
	case 'U' : if ((enable_param[10]))/* && (control == KEYBOARD))*/
		     {
		      draw_box(para_numb,WHITE);
		      para_numb=10;
		      draw_box(para_numb,LIGHTRED);
		     }
		   break;

	case SPACE : if (MOVING!=0)
		     {
		     open_window(3,303,516,349,LIGHTGREEN);
		     enter_stop();
		     close_window(3,303);
		     }
		     if (MAPPING)
		       {
			outcom("a");
		       }
		     break;

	case ENTER : if (MOVING == 1) break;
		     if ((para_numb != 14) && (para_numb != 11) &&
			 (para_numb != 17) && (para_numb != 20) &&
			 (para_numb != 12) && (para_numb != 13) &&
			 (para_numb != 8))
			     open_window(3,303,516,349,LIGHTGREEN);
		     enter_value(para_numb);
		     if ((para_numb != 10) && (para_numb != 14) && (para_numb != 17)
					  && (para_numb != 18) && (para_numb != 11)
					  && (para_numb != 20) && (para_numb != 12)
					  && (para_numb != 13) && (para_numb != 8))
					    close_window(3,303);
		     break;

	default    : printf("\a");
		     break;

       }
}

/***************************************************************************/

void enter_value(int i)

/* enter the parameter or perform the action selected. */

{
 int aa;

 switch (i)
       {
	case 0  : if (!MOVING) {stop_acq(0);enter_posit(FOREMONO);change_comnd=1;}
		  break;

	case 1  : if (!MOVING) {stop_acq(0);enter_posit(SPECTRO);;change_comnd=1;}
		  break;

	case 2  : if (!MOVING) {stop_acq(0);enter_posit(TRIPLE);;change_comnd=1;}
		  break;

	case 3  : stop_acq(0);enter_time();change_comnd=1;
		  break;

	case 4  : /*enter_delay();*/
		  break;

	case 5  : /*enter_width();*/
		  break;

	case 6  :
	case 7  :
		  enter_xy(i);
		  read_xplate();
		  read_yplate();
		  break;

	case 8  : if (flagjoystick) disable_joystick();
		  else enable_joystick();
		  /*joystick();*/
		  break;

	case 9  : stop_acq(0);enter_mode();change_comnd=1;
		  break;

	case 10 : enter_units();
		  break;

	case 20 : if (ACCU==0) {stop_acq(0);ent_plat_scan();}
		  else
		    {if (control==MOUSE) cursor_counter(1);}
		  SetParam(I,(float)1);
		  set_shutter_mode();
		  setviewport(0,0,639,349,0);
		  erase_spacc();
		  current_color=LIGHTGREEN;
		  ACCU=0;ret_accu=1;change_comnd=1;
		  NACC=1;  /* MTC 16-06-93 Bjorn pb */
		  break;

	case 19 :
		  setcolor(WHITE);
		  outtextxy(200,18,"NOT AVAILABLE");
		  if (control==MOUSE) cursor_counter(1);
		  sleep(1);
		  break;

	case 18 : if (ACCU==0)
		    {
			if (control==MOUSE) cursor_counter(1);
			if ((aa=enter_ccd_accumul())==1)
			  {
			    oma4_accumul(MS0);
			  }
			else close_window(3,303);
		    }
		  else
		    {
		     close_window(3,303);
		    }
		  break;

	case 17 :
		  enter_plot(1);
		  break;

	case 16 : if (!MOVING) enter_move(FORWARD);
		  break;

	case 15 : if (!MOVING) enter_move(BACKWARD);
		  break;

	case 14 : ask_accu(ACCU);
		  break;

	case 13 : FLAG_MEMO^=1;
		  pos_memo=X;
		  enter_memo();
		  break;

	case 12 : enter_zoomx();
		  break;

	case 11 : enter_elevator(KEYBOARD);
		  break;
       }
}

/******************************************************************************/
void end_adj(void)

/* close the shutter before leaving adj. mode */
{
 ACQ_MODE=DARK;
 closegraph();
}

/**************************************************************************/
void os(void)  /* Appel de MS-DOS */
{
 int status;
 char com_spec[64];

 get_com_spec(com_spec);
 status=spawnl(P_WAIT,com_spec,com_spec,NULL);
 if (status)
   {
    puts("DOS CALLING ERROR");
    exit(1);
   }
}

/**************************************************************************/

void get_com_spec(char *cs)
{
 strcpy(cs,getenv("COMSPEC"));
}

/**************************************************************************/