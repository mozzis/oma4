/*========================================================================*/
/*                               MOUSA.C                                  */
/*========================================================================*/
/* This file handle the serial mouse (Microsoft, Genius, Logitech,....    */
/* by using the interruption 33H and the registers ax, bx, cx, dx.        */
/* It is necessary to run the mouse driver first if you want to use it.   */
/*========================================================================*/

#include <graphics.h>
#include <acq4.h>
#include <process.h>
#include <dos.h>

/*========================================================================*/

extern int MOVING,FLAG_MEMO,NACC,ACCU,
	   X1[],X2[],Y1[],Y2[],
	   para_numb,enable_param[],
	   X,DIODE_NUMBER,ZOOMX,
	   XSTEP,FIRDI,LASTDI,YSTEP,/*IMAX,IMIN,*/INF,DIV;
extern long IMAX,IMIN;
/*========================================================================*/

int control=MOUSE,
    XMOUSE=259,YMOUSE;

/*========================================================================*/

int enable_mouse(void)

/* enable / disable   mouse. */

{
_AX=0;
 geninterrupt(0x33);     /* interruption 33H.                                 */
 return(_AX);            /* if _AX= 1 mouse enable, if _AX = -1 mouse disable */
}                        /*                                                   */

/**************************************************************************/

int cursor_counter(int dir)

/* increase or decrease the cursor counter. */
{
 _AX=dir;                     /* _AX=1 => increase, _AX=2 decrease.           */
 geninterrupt(0x33);          /* int 33H.                                     */
 return(_AX);                 /* _AX even = show cursor, odd = hide cursor    */
}

/**************************************************************************/

void read_curs_pos(int *xcoord,int *ycoord)

/* give position of the cursor */

{
 _AX=3;
 geninterrupt(0x33);
 *xcoord=_CX;
 *ycoord=_DX;
}
/**************************************************************************/
void set_curs_pos(int xcoord,int ycoord)
/* set the cursor position.    */
{
 _AX=4;
 _CX=xcoord;
 _DX=ycoord;
 geninterrupt(0x33);
}

/**************************************************************************/

void init_mouse(void)

{
 int enable_mouse(),cursor_counter();
 void set_curs_pos(),read_curs_pos(),mouse_yclip(),mouse_xclip();

 if (enable_mouse() != -1)
     control=KEYBOARD;
 else
    {
     cursor_shape(0);
     set_curs_pos(258,140);
     cursor_counter(1);
     mouse_xclip(4,635);
     mouse_yclip(4,345);
     mouse_sensivity(12,16);
    }
}

/**************************************************************************/

void mouse_sensivity(int xratio,int yratio)

{
 _AX=15;
 _CX=xratio;
 _DX=yratio;
 geninterrupt(0x33);
}

/**************************************************************************/
int test_press(int button)

{
 int numpress;

 _AX=5;
 _BX=button;
 geninterrupt(0x33);
 numpress=_BX;
 XMOUSE=_CX;
 YMOUSE=_DX;
 return(numpress);
}

/**************************************************************************/

int test_button(void)

{
 int but_stat;

 _AX=3;
 geninterrupt(0x33);
 but_stat=_BX;
 XMOUSE=_CX;
 YMOUSE=_DX;
 return(but_stat);
}

/**************************************************************************/
void mouse_yclip(int ymin,int ymax)

/* clip the y motion of the cursor. */
{
 _AX=8;
 _CX=ymin;
 _DX=ymax;
 geninterrupt(0x33);
}

/**************************************************************************/

void mouse_xclip(int xmin,int xmax)

/* clip the x motion of the cursor. */

{
 _AX=7;
 _CX=xmin;
 _DX=xmax;
 geninterrupt(0x33);
}

/**************************************************************************/

void test_mouse(void)

{
 int j;
 void action();

 if (test_press(0))
   {
    for (j=0;j<22;j++)
       {
	if ((XMOUSE > X1[j]) && (XMOUSE < X2[j]) && (YMOUSE > Y1[j])
			     && (YMOUSE < Y2[j]))
	     {
	      action(j);
	      break;
	     }
       }
   }

 if ((test_press(1)) && (ZOOMX)) unzoom();
   {
    /*cursor_counter(2);
    cursor_shape(4);
    cursor_counter(1);
    test_press(1);
    while(!test_press(1))
	  mouse_xy();
    cursor_counter(2);
    cursor_shape(0);
    cursor_counter(1);*/
   }
 }

/**************************************************************************/
void action(int i)

{
 int flag=0,zooxor,zooxend,zoyor,zoyend,
     b,c,d,va,exi,side,Xmem;
 void load_array();

 if (MOVING == 0)
   {
    switch (i)
	  {
	   case 21 : place_cursor();
		     break;

	   case 12 : setviewport(296,287,376,299,1);
		     clearviewport();
		     FLAG_MEMO=0;
		     para_numb=12;
		     quick_disp(1,0);
		     /*enter_memo();*/
		     draw_box(para_numb,LIGHTRED);
                     cursor_counter(2);
		     cursor_shape(1);
		     set_curs_pos(258,135);
                     cursor_counter(1);
		     mouse_xclip(515-INF-((LASTDI-FIRDI)*XSTEP/YSTEP),515-INF);
		     mouse_yclip(5,261);

		   /*PARA_NUMB=i;
		   draw_box(PARA_NUMB,LIGHTRED);*/
		     side=0;
		     d=1;
		     exi=0;
		     do
		      {do
			{ if (side!=1)
			    {
			      test_press(0);
			      va=1;
			      while ((c=test_press(0))==0 && (b=test_press(1))==0);
			      if (b!=0) {cursor_shape(0);
					 mouse_xclip(4,635);
					 mouse_yclip(1,345);
					 exi=1;
					 /*draw_box(PARA_NUMB,WHITE);*/
					 break;
					}

			      setcolor(YELLOW);
			      setwritemode(XOR_PUT);
			      zooxor=XMOUSE;
			      zoyor=YMOUSE;
			      cursor_counter(2);
			      line(zooxor,zoyor,zooxor+10,zoyor);
			      line(zooxor,zoyor+1,zooxor,zoyor+6);
			      Xmem=X;X=zooxor;save_cur_image();X=Xmem;
			      setwritemode(COPY_PUT);
			      mouse_xclip(zooxor+2,515-INF);
			      mouse_yclip(zoyor+2,265);
			      cursor_counter(1);
			    }
			  while ((c=test_press(0))==0 && (b=test_press(1))==0 );
			  if (b!=0 && side!=1)
			    { cursor_counter(2);
			      setcolor(YELLOW);
			      setwritemode(XOR_PUT);
			      line(zooxor,zoyor,zooxor+10,zoyor);
			      line(zooxor,zoyor+1,zooxor,zoyor+6);
			      Xmem=X;X=zooxor;save_cur_image();X=Xmem;
			      setwritemode(COPY_PUT);
			      mouse_xclip(515-INF-((LASTDI-FIRDI)*XSTEP/YSTEP),515-INF);
			      mouse_yclip(1,259);
			      cursor_counter(1);
			      flag=0;
			      va=0;
			    }
			  if (va!=0)
			    {
			     flag=1;
			     zooxend=XMOUSE;
			     zoyend=YMOUSE;
			     Xmem=X;X=zooxend;save_cur_image();X=Xmem;
			     cursor_counter(2);
			     setwritemode(XOR_PUT);
			     line(zooxor,zoyor,zooxor+10,zoyor);
			     line(zooxor,zoyor+1,zooxor,zoyor+6);
			     rectangle(zooxor,zoyor,zooxend,zoyend);
			     setwritemode(COPY_PUT);
			     mouse_xclip(4,635);
			     mouse_yclip(1,345);
			     cursor_counter(1);
			    }
			}while (flag==0);

		       if (exi==1) break;
		       cursor_shape(0);

		    /*draw_box(PARA_NUMB,WHITE);*/
		      cursor_counter(2);
		      open_window(158,303,362,349,LIGHTGREEN);
		      setcolor(WHITE);
		      outtextxy(46,20,"OK");
		      outtextxy(121,20,"CANCEL");
		      setviewport(0,0,639,349,0);
		      outline_box(178,318,70,16,YELLOW,LIGHTGRAY);
		      outline_box(268,318,70,16,YELLOW,LIGHTGRAY);
		      cursor_counter(1);
		      do{
			 while ((c=test_press(0))==0 && (b=test_press(1))==0);
			 if (b!=0) { cursor_counter(2);
				     setcolor(YELLOW);
				     setwritemode(XOR_PUT);
				     rectangle(zooxor,zoyor,zooxend,zoyend);
				     line(zooxor,zoyor,zooxor+10,zoyor);
				     line(zooxor,zoyor+1,zooxor,zoyor+6);
				     setwritemode(COPY_PUT);
				     close_window(158,303);
				     cursor_shape(1);
				     mouse_xclip(zooxor+2,515-INF);
				     mouse_yclip(zoyor+2,265);
				     cursor_counter(1);
				     side=1;
				     d=1;
				     c=0;
				   }
			 else
			    {
			     if ((XMOUSE > 169) && (XMOUSE < 247)
					&& (YMOUSE > 320) && (YMOUSE < 332))
			       {c=1;
				d=1;
				cursor_counter(2);
				setcolor(YELLOW);
				setwritemode(XOR_PUT);
				rectangle(zooxor,zoyor,zooxend,zoyend);
				setwritemode(COPY_PUT);
				close_window(158,303);
				cursor_counter(1);
			       }
			     else if ((XMOUSE > 269) && (XMOUSE < 339)
					 && (YMOUSE > 320) && (YMOUSE < 332))
				    {cursor_counter(2);
				     setcolor(YELLOW);
				     setwritemode(XOR_PUT);
				     rectangle(zooxor,zoyor,zooxend,zoyend);
				     setwritemode(COPY_PUT);
				     mouse_xclip(4,635);
				     mouse_yclip(1,345);
				     close_window(158,303);
				     cursor_counter(1);
				     cursor_shape(0);
				     d=1;
				     exi=1;
				     break;
				   }
				 else
				   {d=0;
				    printf("\a");
				   }
			    }
			  } while (d!=1);
		       } while (c!=1);
		     if (exi==1) 	break;

		     /* OMA3 PDA
                     IMAX-=((unsigned long)(zoyor-5)*(unsigned long)DIV);
		     IMIN+=((unsigned long)(261-zoyend)*(unsigned long)DIV);
		     */

		     /* OMA4 */
		     IMAX-=((long)(zoyor-5)*(unsigned long)DIV);
		     IMIN+=((long)(261-zoyend)*(unsigned long)DIV);

		     if ((DIV=(IMAX-IMIN)>>8) < 1)
		       {
			  DIV=1;
			  IMAX=IMIN+256;
		       }
		     LASTDI=((515-zooxor-INF)*YSTEP/XSTEP)+FIRDI;
		     FIRDI=((515-zooxend-INF)*YSTEP/XSTEP)+FIRDI;
		     if ((FIRDI%2) == 0) FIRDI++;    /* toujours impair */
		     if ((LASTDI%2) == 1) LASTDI--;  /* toujours paire */
		     YSTEP = ((LASTDI-FIRDI+1) > 512) ? 2 : 1;
		     XSTEP=512*YSTEP/(LASTDI-FIRDI+1);
		     INF=(512-(LASTDI-FIRDI+1)*XSTEP/YSTEP)>>1;
		     /*erase_disp();*/
		     int_range();
		     setviewport(0,0,639,349,0);
		     init_length();
                     set_curs_pos(258,135);
		     place_cursor();
		     if (ACCU) {erase_spacc();spacc_disp(NACC,1,MS0);}
		      else {erase_disp();quick_disp(0,0);}
		     /*init_length();
		     place_cursor(0);*/

		     /*save_cur_image();*/
		     draw_box(para_numb,WHITE);
		     ZOOMX=1;
		   break;

	 default : if (enable_param[i] == 1)
		     {
		      cursor_counter(2);
		      draw_box(para_numb,WHITE);
		      para_numb=i;
		      draw_box(para_numb,LIGHTRED);
		      if (para_numb==20) cursor_counter(1);
		      if (para_numb==11) enter_elevator(MOUSE);
		      else
			{
			 if ((para_numb != 14) && (para_numb != 17) &&
			     (para_numb != 20) && (para_numb!=13) &&
			     (para_numb != 8))
			    open_window(3,303,516,349,LIGHTGREEN);
			 enter_value(i);
			 if ((i != 10) && (i != 13) &&
			     (para_numb != 17) && (i != 14) &&
			     (para_numb != 20) && (para_numb!=13) &&
			     (para_numb != 8))
			     close_window(3,303);
			}
		      cursor_counter(2);
		      draw_box(para_numb,WHITE);
		      cursor_counter(1);
		     }
		   break;
	  }
   }
}

/**************************************************************************/
void cursor_shape(int i)
{
 int  arrow[]={~0x3F0,~0x3C0,~0x3C0,~0x3E0,~0x270,~0x238,~0x1C,~0xE,~0x6,
	       ~0x0,~0x0,~0x0,~0x0,~0x0,~0x0,~0x0,0x3F0,0x3C0,0x3C0,0x3E0,
		0x270,0x238,0x1C,0xE,0x6,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

 int  mag[]={~0x3F80,~0x4040,~0x8020,~0x8420,~0x8A20,~0x8420,~0x8020,~0x4060,
	     ~0x3FE0,~0x30,~0x18,~0xC,~0x6,~0x3,~0x1,~0x0,0x3F80,0x4040,
	      0x8020,0x8420,0x8A20,0x8420,0x8020,0x4060,0x3FE0,0x30,0x18,
	      0xC,0x6,0x3,0x1,0x0};

 int  floppy[]={~0x3FFF,~0x46F1,~0xFFFF,~0xFFFF,~0xFFFF,~0xFFFF,~0xFFFF,
		~0xFFFF,~0xFFFF,~0xFFFF,~0xFFFF,~0xFFFF,~0xFFFF,~0xFFFF,
		~0xFFFF,~0xFFFF,0x3FFF,0x46F1,0x86F1,0x86F1,0x87F1,0x8001,
		 0x8181,0x8241,0x8241,0x8181,0x8001,0x8001,0x9FF9,0x9FF9,
		 0x9FF9,0xFFFF};

 int  warning[]={~0x0100,~0x0280,~0x0280,~0x0440,~0x0440,~0x0920,~0x0920,~0x1110,
		 ~0x1110,~0x2108,~0x2108,~0x4104,~0x4004,~0x8102,~0x8002,~0xFFFE,
		  0x0100,0x0280,0x0280,0x0440,0x440,0x0920,0x0920,0x1110,
		  0x1110,0x2108,0x2108,0x4104,0x4004,0x8102,0x8002,0xFFFE};

union REGS inregs,outregs;

     inregs.x.ax=9;



  if (i == 0)
     {inregs.x.bx=5;
      inregs.x.cx=0;
      inregs.x.dx=FP_OFF(arrow);
     }

 if (i == 1)
     {inregs.x.bx=5;
      inregs.x.cx=4;
      inregs.x.dx=FP_OFF(mag);
     }

 if (i == 2)
    {inregs.x.bx=8;
     inregs.x.cx=8;
     inregs.x.dx=FP_OFF(floppy);
    }

 if (i==3)
    {inregs.x.bx=7;
     inregs.x.cx=0;
     inregs.x.dx=FP_OFF(warning);
     }

 int86(0x33,&inregs,&outregs);

 }

/**************************************************************************/
void place_cursor(void)

{
 cursor_counter(2);
 er_cur();
 /*restore_cur_image();*/

 if (XMOUSE < (515-INF-((LASTDI-FIRDI)*XSTEP/YSTEP)))
     X=515-INF-((LASTDI-FIRDI)*XSTEP/YSTEP);
 else if (XMOUSE > (515-INF)) X=515-INF;
 else if (((XMOUSE-3-INF) % XSTEP) != 0)
	       X=((XMOUSE-3-INF)/XSTEP)*XSTEP+3+INF;
 else X = XMOUSE;
 if (FLAG_MEMO) enter_memo();
 new_cur(LIGHTGRAY,MS0);
 save_cur_image();
 /*cur_int(0); oma3 pda*/
 cur_int();/*oma4*/
 int_range();
 cursor_counter(1);
}

/*************************************************************************/
