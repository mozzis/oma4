/*=========================================================================*/
/*                             FUNC0.C                                     */
/*=========================================================================*/

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

/*=========================================================================*/
void mospeed();

extern int control,XMOUSE,YMOUSE,X,
	   MOVING,
	   X1[],Y1[],X2[],Y2[],
	   FLAG_MEMO,pos_memo,
	   newpoly[],
	   XSTEP,YSTEP,FIRDI,LASTDI,
	   DIV,INF,ACCU,MASKEDIT,
	   change_comnd;

extern long IMAX,IMIN;

extern unsigned char DIREC;

/*========================================================================*/

char  /*Array of the available speed of the stepping motor.*/
     MOV[]={0x0C,0x0D,0x0E,0x1B,0x0F,0x1C,0x1D,0x2A,0x1E,0x2B,0x1F,
	    0x2C,0x39,0x2D,0x3A,0x2E,0x3B,0x2F,0x3C,0x3D,0x3E};

int  selec_module,numspeed=0,module,
     y_memo=260,
     ACQ_MODE,ND=0,NS=1,NACC=1,
     ZOOMX;

int WAITIME=0;

int accu_mode(),accu_param();
/*========================================================================*/

void enter_move(int dir)

{
 int cancel=0,posmouse=0,i,select=0,j,valx;
 char sleft[2]={27};
 char sright[2]={26};
 char c,*vit[]={"  1","  2","  4","  5","  8"," 10"," 20"," 25"," 40",
		" 50"," 80"," 100"," 125"," 200"," 250"," 400"," 500"," 800",
		"1000","2000","4000"};


 setcolor(WHITE);
 outtextxy(25,10,"Module :");
 outtextxy(25,28,"Speed  :                                            cm-1/mn.");

 setfillstyle(SOLID_FILL,LIGHTBLUE);
 bar(198+44*selec_module,9,236+44*selec_module,19);
 setcolor(LIGHTMAGENTA);
 if (dir==FORWARD) outtextxy(333,10,"'ENTER' to go FORWARD");
 else outtextxy(333,10,"'ENTER' to go BACKWARD");

 setcolor(LIGHTGRAY);
 rectangle(160,35,380,38);

 setcolor(WHITE);
 for(j=171;j<378;j=j+11) line(j,36,j,37);
 setcolor(WHITE);
 outtextxy(120,28,sleft);
 outtextxy(420,28,sright);

 setcolor(YELLOW);
 if (control==MOUSE)
   {
    rectangle(114,25,134,37);
    rectangle(414,25,434,37);
   }
 else
   {
    outtextxy(132,28,"'-'");
    outtextxy(394,28,"'+'");
   }
 valx=(numspeed*11)+160;

 setcolor(LIGHTGREEN);
 setwritemode(XOR_PUT);
 line(valx,34,valx,39);
 setwritemode(COPY_PUT);
 setcolor(YELLOW);
 outtextxy(valx-16,25,vit[numspeed]);
 rectangle(197,8,237,20);
 rectangle(197+44,8,237+44,20);
 rectangle(197+88,8,237+88,20);
 outtextxy(200,10,"Fore  Spec Both ");

 if (control == MOUSE)
    {
     set_curs_pos(220+(44*selec_module),319);
     cursor_shape(0);
     mouse_xclip(5,512);
     mouse_yclip(305,345);
    }

 while ((!cancel) && (!select))
      {
       if (control==MOUSE)
	 {
	  cursor_counter(1);
	  test_press(0);
	  test_press(1);
	  while ((posmouse != 1) && (!kbhit()))
	       {
		if (test_press(1) !=0)
		  {
		   posmouse=1;cancel=1;select=1;
		  }
		if (test_press(0) != 0)
		  {
                   for (i=0;i<3;i++)
		      {
		       if ((XMOUSE > (200+44*i)) && (XMOUSE < (240+44*i))
				  && (YMOUSE > 312) && (YMOUSE < 322))
			 {
			  cursor_counter(2);
			  setfillstyle(SOLID_FILL,BLACK);
			  bar(198+44*selec_module,9,236+44*selec_module,19);
			  outtextxy(200,10,"Fore  Spec Both ");
			  selec_module=i;
			  setfillstyle(SOLID_FILL,LIGHTBLUE);
			  bar(198+44*selec_module,9,236+44*selec_module,19);
			  outtextxy(200,10,"Fore  Spec Both ");
			  /*posmouse=1;*/
			  cursor_counter(1);
			 }
		      }
		  do
		   {
		    if (XMOUSE>113 && XMOUSE<135 && YMOUSE>24+303 && YMOUSE<38+303) mospeed(0);
		    if (XMOUSE>413 && XMOUSE<435 && YMOUSE>24+303 && YMOUSE<38+303) mospeed(1);
		    if (YMOUSE>24+303) delay(120);
		    }
		     while(test_button()!=0);

		  }
	       }
	 }
       if (!select)
	 {
	  c=getch();
	  if (c==ESC)
	    {
	     cancel=1;
	     break;
	    }
          switch (c)
		{case 0 : c=getch();
			  if (c == 77)
			    {cursor_counter(2);
			     setfillstyle(SOLID_FILL,BLACK);
			     bar(198+44*selec_module,9,236+44*selec_module,19);
			     outtextxy(200,10,"Fore  Spec Both ");
			     selec_module++;
			     if (selec_module > 2)
				 selec_module=0;
			     setfillstyle(SOLID_FILL,LIGHTBLUE);
			     bar(198+44*selec_module,9,236+44*selec_module,19);
			     outtextxy(200,10,"Fore  Spec Both ");
			     cursor_counter(1);
			     }
			  if (c == 75)
			     {setfillstyle(SOLID_FILL,BLACK);
			      cursor_counter(2);
			      bar(198+44*selec_module,9,236+44*selec_module,19);
			      outtextxy(200,10,"Fore  Spec Both ");
			      selec_module--;
			      if (selec_module < 0)
				  selec_module=2;
			      setfillstyle(SOLID_FILL,LIGHTBLUE);
			      bar(198+44*selec_module,9,236+44*selec_module,19);
			      outtextxy(200,10,"Fore  Spec Both ");
			      cursor_counter(1);
			      }
			   break;

		case '+' : mospeed(1);
			   break;
		case '-' :mospeed(0);
			  break;

		case ENTER: select=1;
			    break;

		default  :  printf("\a");
			    break;
		}
	 }
    if (cancel == 1) break;
      }
    if (!cancel)
      {if (dir == FORWARD)
	 {DIREC=FORWARD;
	  if ((selec_module == 1) || (selec_module == 0))
	    {
	     module=selec_module;
	     movef(MOV[numspeed],&module);
	    }
	 if (selec_module == 2)
	    {module=0;
	     movef(MOV[numspeed],&module);
	     module++;
	     movef(MOV[numspeed],&module);
	    }
	 }
      if (dir == BACKWARD)
	{DIREC=BACKWARD;
	 if ((selec_module == 1) || (selec_module == 0))
	    {module=selec_module;
	     moveb(MOV[numspeed],&module);
	    }
	 if (selec_module == 2)
	    {module=0;
	     moveb(MOV[numspeed],&module);
	     module++;
	     moveb(MOV[numspeed],&module);
	    }
	}
     cursor_shape(3);
     MOVING=selec_module+1;
     setcolor(WHITE);
     setviewport(5,271,516,281,01);
     clearviewport();
     setviewport(0,0,639,349,0);
     write_text(YELLOW,470,289,4,10,RED,"MOVE");
    }
 if (control==MOUSE)
   {cursor_counter(1);
    mouse_xclip(0,639);
    mouse_yclip(0,349);
    set_curs_pos(259,140);
   }
}
/***********************************************************************/
void mospeed(int dii)
{int valx;
 char *vit[]={"  1","  2","  4","  5","  8"," 10"," 20"," 25"," 40",
		" 50"," 80"," 100"," 125"," 200"," 250"," 400"," 500"," 800",
		"1000","2000","4000"};

 if (control==MOUSE) cursor_counter(2);
 if (dii==0 && numspeed>0)
   {setcolor(BLACK);
    valx=(numspeed*11)+160;
    outtextxy(valx-16,25,vit[numspeed]);
    setcolor(LIGHTGREEN);
    setwritemode(XOR_PUT);
    line(valx,34,valx,39);
    numspeed--;
    valx=(numspeed*11)+160;
    line(valx,34,valx,39);
    setwritemode(COPY_PUT);
    setcolor(YELLOW);
    outtextxy(valx-16,25,vit[numspeed]);
   }
 if (dii==1 && numspeed<20)
   {setcolor(BLACK);
    valx=(numspeed*11)+160;
    outtextxy(valx-16,25,vit[numspeed]);
    setcolor(LIGHTGREEN);
    setwritemode(XOR_PUT);
    line(valx,34,valx,39);
    numspeed++;
    valx=(numspeed*11)+160;
    line(valx,34,valx,39);
    setwritemode(COPY_PUT);
    setcolor(YELLOW);
    outtextxy(valx-16,25,vit[numspeed]);
   }
if (control==MOUSE) cursor_counter(1);

}
/***************************************************************************/

void enter_stop(void)

{
 setcolor(WHITE);
 outtextxy(250,18,"OK !");
 cursor_shape(0);
 if ((MOVING==1) || (MOVING==2))
   {
    module=MOVING-1;
    stop(&module);
   }
 else if (MOVING==3)
   {
    module = 0;
    stop(&module);
    module++;
    stop(&module);
   }
 restore_cur_image();
 init_length();
 save_cur_image();
 write_text(BLACK,470,289,4,10,BLACK,"MOVE");
 MOVING=0;
 sleep(1);
}

/***************************************************************************/
void enter_plot(int type)
{
 char param_plot[2][4]={"Yes","No "},cc;

 int i,ii,vardom=0,
     xx[2]={300,355},yy[2]={18,18},len=3,fond,
     choix=0,
     para_flag=1,cancel=0;

 open_window(3,303,516,349,LIGHTGREEN);
 setviewport(7,307,512,345,0);
 setfillstyle(SOLID_FILL,LIGHTGRAY);
 bar(188,1,304,11);
 setcolor(RED);
 outtextxy(192,3,"PLOTTER OUTPUT");
 setcolor(YELLOW);
 outtextxy(2,31,"SPACE or left button to start plot,ESC or right button to quit.");
 setcolor(WHITE);
 outtextxy(2,18,"Do you wish to plot the parameters ? ");

 setcolor(YELLOW);
 for (ii=0;ii<2;ii++)
    {
     rectangle(xx[ii]-4,yy[ii]-4,xx[ii]+1+8*len,yy[ii]+11);
     if (ii==1) fond=LIGHTBLUE;
     else fond=BLACK;
     write_text(fond,xx[ii],yy[ii],len,10,YELLOW,param_plot[ii]);
    }
if (control==MOUSE) cursor_counter(1);
 ii=1;
 if (control==MOUSE)
   {
    cursor_counter(2);
    mouse_xclip(6,510);
    mouse_yclip(310,340);
    set_curs_pos(280,328);
    cursor_shape(0);
    cursor_counter(1);
    test_press(0);
    test_press(1);
   }

 do
   {
    if ((control==MOUSE) && (test_press(1)!=0))
      {
       ii=-1;
       cancel=1;
       choix=1;
      }
    if ((control==MOUSE) && (test_press(0)!=0))
      {
       for (i=0;i<2;i++)
	  {
	    if ((XMOUSE>xx[i]+3) && (XMOUSE<xx[i]+24) &&
		(YMOUSE>303+yy[i]) && (YMOUSE<303+yy[i]+10))
	      {
	       cursor_counter(2);
	       write_text(BLACK,xx[ii],yy[ii],len,10,YELLOW,param_plot[ii]);
	       ii=i;
	       write_text(LIGHTBLUE,xx[ii],yy[ii],len,10,YELLOW,param_plot[ii]);
	       choix=1;
	       vardom=1;
	       cursor_counter(1);
	      }
	  }
	 if (vardom==0) printf("\a");
      }
    if (!choix)
      {
       if (kbhit())
	 {
	  cc=getch();
	  if (cc==0) cc=getch();

	  switch (cc)
		{
		 case ESC  :
			    cancel=1;
			    choix=1;
			    break;

		 case LEFT :
			    write_text(BLACK,xx[ii],yy[ii],len,10,YELLOW,param_plot[ii]);
			    if (ii==0) ii=1;
			    else ii--;
			    write_text(LIGHTBLUE,xx[ii],yy[ii],len,10,YELLOW,param_plot[ii]);
			    break;

		 case RIGHT:
			    write_text(BLACK,xx[ii],yy[ii],len,10,YELLOW,param_plot[ii]);
			    if (ii==1) ii=0;
			    else ii++;
			    write_text(LIGHTBLUE,xx[ii],yy[ii],len,10,YELLOW,param_plot[ii]);
			    break;

		 case SPACE:
			    choix=1;
			    break;
		default :printf("\a");
			 break;
	       }
	 }
      }
   }
    while(!choix);

 if (ii==1) para_flag=0;
 if (control==MOUSE) cursor_counter(2);
 if (!cancel) quick_plot(para_flag,type);
 if (control==MOUSE) init_mouse();
 close_window(3,303);
}

/***************************************************************************/

void enter_memo(void)

{
 setviewport(X1[20]+1,Y1[20]+1,X2[20]-1,Y2[20]-1,0);
 setcolor(BLACK);
 line(0,y_memo,511,y_memo);
 setcolor(YELLOW);
 if (FLAG_MEMO)
    {y_memo = (newpoly[515-pos_memo]+1 < y_memo) ? newpoly[515-pos_memo]
						 : y_memo;
     line(0,y_memo,511,y_memo);
     }
 else
    y_memo=pos_memo=260;
 setviewport(0,0,639,349,0);
if (control==MOUSE) cursor_counter(1);
}

/******************************************************************************/

void enter_zoomx(void)
{
 char c;

 int flag=0,borne[4]={0,0,0,0},x=260,y=132,
     rec,br,a=1,bb,Xmem;

 long dummy;

 Xmem=X;
 if (control==MOUSE) cursor_counter(2);

 open_window(3,303,516,349,LIGHTGREEN);bb=1;
 setcolor(WHITE);
 outtextxy(16,15,"Move the cursor to the selected positions with arrow keys");
 outtextxy(16,30,"And press ENTER to validate.");

 setviewport(296,287,376,299,1);
 clearviewport();

 setviewport(0,0,639,349,0);
 setcolor(YELLOW);
 setwritemode(XOR_PUT);
 line(x,y,x+10,y);
 line(x,y,x,y+8);
 X=x;save_cur_image();

 do
   {rec=0;
    br=0;
    while (flag!=4)
	{
	 c=getch();
	 if (c==0) c=getch();
	 switch (c)
	       {
		case RIGHT :
                             if (flag!=2)
			       {
				line(x,y,x+10,y);
				line(x,y,x,y+8);
				if (x < 515-INF) x++;
				line(x,y,x+10,y);
				line(x,y,x,y+8);
			       }
			     else
			       {
				line(x-10,y,x,y);
				line(x,y-8,x,y);
				if (x<515-INF) x++;
				line(x-10,y,x,y);
				line(x,y-8,x,y);
			       }
			     X=x;save_cur_image();
			     break;

		case LEFT  :
			     if (flag!=2)
			       {
                                line(x,y,x+10,y);
				line(x,y,x,y+8);
				if (x > 515-INF-((LASTDI-FIRDI)*XSTEP/YSTEP)) x--;
				line(x,y,x+10,y);
				line(x,y,x,y+8);
			       }
			     else
			       {
                                line(x-10,y,x,y);
				line(x,y-8,x,y);
				if ((x> 515-INF-((LASTDI-FIRDI)*XSTEP/YSTEP)) && (x>borne[0]+10)) x--;
				line(x-10,y,x,y);
				line(x,y-8,x,y);
			       }
			     X=x;save_cur_image();
			     break;

		case UP    : if (flag!=2)
				   {
				    line(x,y,x+10,y);
				    line(x,y,x,y+8);
				    if (y>1) y--;
				    line(x,y,x+10,y);
				    line(x,y,x,y+8);
				   }
				 else
				   {
				    line(x-10,y,x,y);
				    line(x,y-8,x,y);
				    if ((y>1) && (y>borne[1]+24)) y--;
				    line(x-10,y,x,y);
				    line(x,y-8,x,y);
				   }
			    break;

		case DOWN  : if (flag!=2)
			       {
				line(x,y,x+10,y);
				line(x,y,x,y+8);
				if (y<251) y++;
                                line(x,y,x+10,y);
				line(x,y,x,y+8);
			       }
			     else
			       {
                                line(x-10,y,x,y);
				line(x,y-8,x,y);
				if ((y<265) && (y>borne[1]+8)) y++;
				line(x-10,y,x,y);
				line(x,y-8,x,y);
			       }
			     break;

		case ENTER :
			     borne[flag++]=x;
			     borne[flag++]=y;
			     if (flag==2)
			       {
				x+=14;
				y+=14;
				line(x-10,y,x,y);
				line(x,y-8,x,y);
			       }
			     if (flag==4)
			       {
				line(borne[0],borne[1],borne[0]+10,borne[1]);
				line(borne[0],borne[1],borne[0],borne[1]+8);
				line(borne[2]-10,borne[3],borne[2],borne[3]);
				line(borne[2],borne[3]-8,borne[2],borne[3]);
				rectangle(borne[0],borne[1],borne[2],borne[3]);
			       }
			     break;

		case ESC   : if (flag==2)
			       {
				line(x-10,y,x,y);
				line(x,y-8,x,y);
				x=borne[0];
				y=borne[1];
				flag=0;
			       }
			     else
			       {
				line(x,y,x+10,y);
				line(x,y,x,y+8);
				br=1;
			       }
			    X=x;save_cur_image();
			    break;

		case CTRL_RIGHT:
                                 if (flag!=2)
				   {
				    line(x,y,x+10,y);
				    line(x,y,x,y+8);
				    if (x < (515-INF-15)) x=x+15;
				    line(x,y,x+10,y);
				    line(x,y,x,y+8);
				   }
				 else
				   {
				    line(x-10,y,x,y);
				    line(x,y-8,x,y);
				    if (x<(515-INF-15)) x=x+15;
				    line(x-10,y,x,y);
				    line(x,y-8,x,y);
				   }
				 X=x;save_cur_image();
				 break;

		case CTRL_LEFT :
				if (flag!=2)
				  {
				   line(x,y,x+10,y);
				   line(x,y,x,y+8);
				   if (x>((515-INF-(LASTDI-FIRDI)*XSTEP/YSTEP))+15)
				     { x=x-15;}
				   line(x,y,x+10,y);
				   line(x,y,x,y+8);
				  }
				else
				  {
				   line(x-10,y,x,y);
				   line(x,y-8,x,y);
				   if ((x>borne[0]+24)) x=x-15;
				   line(x-10,y,x,y);
				   line(x,y-8,x,y);
				  }
				X=x;save_cur_image();
				break;


		case CTRL_UP   :
		case CTRL_PG_UP:
				 if (flag!=2)
				   {
				    line(x,y,x+10,y);
				    line(x,y,x,y+8);
				    if (y>15) y=y-15;
				    if (y<16) y=1;
				    line(x,y,x+10,y);
				    line(x,y,x,y+8);
				   }
				 else
				   {
				    line(x-10,y,x,y);
				    line(x,y-8,x,y);
				    if ((y>15) && (y>borne[1]+24)) y=y-15;
				    if (y<16) y=1;
				    line(x-10,y,x,y);
				    line(x,y-8,x,y);
				   }
				 break;

		case CTRL_DOWN :
		case CTRL_PG_DOWN:
				 if (flag!=2)
				   {
				    line(x,y,x+10,y);
				    line(x,y,x,y+8);
				    if (y<237) y=y+15;
				    if (y>236) y=251;
				    line(x,y,x+10,y);
				    line(x,y,x,y+8);
				   }
				 else
				   {
				    line(x-10,y,x,y);
				    line(x,y-8,x,y);
				    if (y<251) y=y+15;
				    if (y>250) y=265;
				    line(x-10,y,x,y);
				    line(x,y-8,x,y);
				   }
				 break;

		default:        printf("\a");
				break;
	       }
	 if (br==1) break;
	}
   if (bb==1) {close_window(3,303);bb=0;}
   if (br==1) break;
   open_window(158,303,362,349,LIGHTGREEN);
   setviewport(0,0,639,349,0);
   outline_box(178,318,70,16,YELLOW,LIGHTGRAY);
   outline_box(268,318,70,16,YELLOW,LIGHTGRAY);
   setfillstyle(SOLID_FILL,LIGHTBLUE);
   bar(180,320,246,332);
   setcolor(YELLOW);
   outtextxy(207,323,"OK");
   outtextxy(283,323,"CANCEL");
   while ((c=getch())!=ENTER)
      {
       if (c==0) c=getch();
       if (c==ESC)
	 {
	  a=3;
	  close_window(158,303);
	  break;
	 }
       switch (c)
	     {
	      case LEFT:
			 if (a!=1)
			   {
			    setfillstyle(SOLID_FILL,BLACK);
			    bar(270,320,336,332);
			    setcolor(YELLOW);
			    outtextxy(283,323,"CANCEL");
			    setfillstyle(SOLID_FILL,LIGHTBLUE);
			    bar(180,320,246,332);
			    setcolor(YELLOW);
			    outtextxy(207,323,"OK");
			    a=1;
			   }
			 break;

	      case RIGHT:
			 if (a!=2)
			   {
			    setfillstyle(SOLID_FILL,BLACK);
			    bar(180,320,246,332);
			    setcolor(YELLOW);
			    outtextxy(207,323,"OK");
			    setfillstyle(SOLID_FILL,LIGHTBLUE);
			    bar(270,320,336,332);
			    setcolor(YELLOW);
			    outtextxy(283,323,"CANCEL");
			    a=2;
			   }
			 break;

	      default:
			 printf("\a");
			 break;
	     }
      }
   setwritemode(XOR_PUT);
   setcolor(YELLOW);
   rectangle(borne[0],borne[1],borne[2],borne[3]);
   if (a==2)
     {
      borne[0]=borne[1]=borne[2]=borne[3]=0;
      flag=0;
      rec=0;
      X=Xmem;
     }
   if (a==3)
     {
      line(borne[0],borne[1],borne[0]+10,borne[1]);
      line(borne[0],borne[1],borne[0],borne[1]+8);
      flag=2;
      x=borne[2];
      y=borne[3];
      line(x-10,y,x,y);
      line(x,y-8,x,y);
      a=1;
      rec=1;
      X=x;save_cur_image();
     }
   }
    while (rec==1);
 close_window(158,303);
 setwritemode(COPY_PUT);
 if (flag!=0)
   {
   /*
    IMAX-=(unsigned long)((borne[1]-5)*DIV);
    IMIN+=(unsigned long)((261-borne[3])*DIV);
   */
    IMAX-=((long)(borne[1]-5)*(unsigned long)DIV);
    IMIN+=((long)(261-borne[3])*(unsigned long)DIV); /*oma4 */

/*  printf("borne[1]:%d  borne[3]:%d  DIV:%d  IMAX:%lu  IMIN:%lu\n",borne[1],borne[3],DIV,IMAX,IMIN);*/

    dummy=(IMAX-IMIN)>>8;               /* oma 4  */
    if ((DIV=(int) dummy) <1)   /* oma 4 21-10-91*/
      {
       DIV=1;
       IMAX=IMIN+256;
      }
    LASTDI=((515-borne[0]-INF)*YSTEP/XSTEP)+FIRDI;
    FIRDI=((515-borne[2]-INF)*YSTEP/XSTEP)+FIRDI;
    if ((FIRDI%2) == 0) FIRDI++;
    if ((LASTDI%2) == 1) LASTDI--;
    YSTEP= ((LASTDI-FIRDI+1) > 512) ? 2: 1;
    XSTEP=512*YSTEP/(LASTDI-FIRDI+1);
    INF=(512-(LASTDI-FIRDI+1)*XSTEP/YSTEP)>>1;
    er_cur();X=Xmem;
    int_range();
    setviewport(0,0,639,349,0);
    init_length();
    XMOUSE=X;
    place_cursor();
    if (ACCU) {erase_spacc();spacc_disp(NACC,1,MS0);}
    else {erase_disp();quick_disp(0,0);}
    ZOOMX=1;
   }
 if (control==MOUSE) cursor_counter(1);
}
/**************************************************************************/

void enter_elevator(int selectmode)

{
 char c,sup[2]={24},sdown[19]={25};
 long dummy;

 if (selectmode == KEYBOARD)
   {
     setcolor(LIGHTRED);
     setviewport (X1[11],Y1[11],X2[11],Y2[11],0);
     outtextxy(22,5,sup);
     outtextxy(22,21,sdown);
     do
	  {
	   c=getch();
	   if (c == 0) c=getch();
	   switch (c)
		 {
		  case DOWN  :
			       /*IMAX=(IMAX <= 8192) ? IMAX<<1 : IMAX;for oma 3 et pda*/
			       IMAX=(IMAX<=131072) ? IMAX<<1 : IMAX; /*18-10-91 mtc oma4 */
			       dummy=(IMAX-IMIN)>>8;
			       DIV=(int)dummy;
			       int_range();
			       if (ACCU) spacc_disp(NACC,1,MS0);
			       else quick_disp(1,0);
			       break;

		  case  UP   : IMAX=(IMAX > 1) ? IMAX>>1 : IMAX;
			       if ((DIV=(IMAX-IMIN)>>8) < 1)
				 {
				  DIV=1;
				  IMAX=IMIN+256;
				 }
			       int_range();
			       if (ACCU) spacc_disp(NACC,1,MS0);
			       else quick_disp(1,0);
			       break;
		  default:
			       break;
		 }
	  }
	   while (c!=ENTER);
     setcolor(WHITE);
     setviewport(X1[11],Y1[11],X2[11],Y2[11],0);
     outtextxy(22,5,sup);
     outtextxy(22,21,sdown);
   }

 if (selectmode == MOUSE)
   {
     if ((YMOUSE > (Y1[11]+21)) || (c==DOWN))
       {
	/* oma3 pda
	 IMAX=(IMAX <= 8192) ? IMAX<<1 : IMAX;
	 DIV=(IMAX-IMIN)>>8;
	*/
	 IMAX=(IMAX <= 131072) ? IMAX<<1 : IMAX;
	 dummy=(IMAX-IMIN)>>8;DIV=(int)dummy; /* OMA 4 */
	 int_range();
	 if (ACCU) spacc_disp(NACC,1,MS0);
	 else quick_disp(1,0);
       }
     if ((YMOUSE < (Y1[11]+21)) || (c==UP))
       {
	     IMAX=(IMAX > 1) ? IMAX>>1 : IMAX;
	     dummy=(IMAX-IMIN)>>8;
	     if ((DIV=(int)dummy) < 1)
	       {
		DIV=1;
		IMAX=IMIN+256;
	       }
	     int_range();
	     if (ACCU) spacc_disp(NACC,1,MS0);
	     else quick_disp(1,0);
       }
   }
   if (control==MOUSE) cursor_counter(1);
}

/***************************************************************************/
void enter_user(void)
{
  int param_length[]={3,3},
       x[]={151,303},
       y[]={17,17},
       ind,select,x0=7,y0=307,action=0,insert=0;

  char c,par_accu[2][7]={"   ","   "};
 sprintf(par_accu[0],"%d",NS);
 sprintf(par_accu[1],"%d",ND);


 setviewport(7,307,512,346,0);
 clearviewport();

 setcolor(YELLOW);
 outtextxy(4,30,"SPACE or left button to validate,Esc or right button to quit");
 setfillstyle(SOLID_FILL,LIGHTGRAY);
 bar(185,1,355,11);
 setcolor(RED);
 outtextxy(190,3,"CREATE YOUR OWN MODE");
 setcolor(WHITE);
 outtextxy(20,17,"Nbr of Signal :      Nbr of Dark :");


 setcolor(LIGHTRED);
 outtextxy(x[0],y[0],par_accu[0]);
 outtextxy(x[1],y[1],par_accu[1]);

 ind=0;

 write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par_accu[ind]);
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
       if (kbhit() && (!action)) {c=getch();action=1;}
      }
       while (!action);

       if (c==ESC) { break;}
       if (c==SPACE)
	 {
	  NS=atoi(par_accu[0]);
	  ND=atoi(par_accu[1]);
	  select=1;
	 }
       if (c==0) c=getch();
       switch(c)
	 {case UP    :
	  case LEFT  :
		    if (ind!=0)
		      {
		       write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par_accu[ind]);
		       ind--;
		       write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par_accu[ind]);
		      }
		    break;
	  case DOWN  :
	  case RIGHT :
		     if (ind!=1)
		       {
			write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par_accu[ind]);
			ind++;
			write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par_accu[ind]);
		       }
		      break;

	   case ENTER :MASKEDIT=1;
		      edit_string(param_length[ind],x[ind]+x0,y[ind]+y0,par_accu[ind],&insert);
		      MASKEDIT=0;
                      setviewport(7,307,512,346,0);
		      write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par_accu[ind]);
		      /* Next parameter */
		      if (ind==1) {ind=0;}
		      else {ind=1;}
		      write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par_accu[ind]);
		      break;
	  }
   }
    while (!select);
 if (control==MOUSE)
   {cursor_shape(0);cursor_counter(1);}
}


/***************************************************************************/


/***************************************************************************/

int accu_mode(void)
{
 /* Accumulation mode selection (Current/User) */

 char accu_mode[3][8]={"Current"," User  "},cc;

 int i,ii,
     xx[2]={270,380},yy[2]={25,25},len=7,fond,
     choix=0;

 setcolor(YELLOW);
 outtextxy(150,9,"ESC or right button to quit.");

 setcolor(WHITE);
 outtextxy(10,25,"Select your accumulation mode.");

 setcolor(YELLOW);
 for (ii=0;ii<2;ii++)
    {
     rectangle(xx[ii]-4,yy[ii]-4,xx[ii]+1+8*len,yy[ii]+11);
     if (ii==0) fond=LIGHTBLUE;
     else fond=BLACK;
     write_text(fond,xx[ii],yy[ii],len,10,YELLOW,accu_mode[ii]);
    }

 ii=0;
 if (control==MOUSE)
   {
    mouse_xclip(6,510);
    mouse_yclip(310,340);
    set_curs_pos(280,328);
    cursor_shape(0);
    cursor_counter(1);
   }

 do
   {
    if ((control==MOUSE) && (test_press(1)!=0))
      {
       cursor_counter(2);
       ii=-1;
       choix=1;
      }
    if ((control==MOUSE) && (test_press(0)!=0))
      {
       for (i=0;i<2;i++)
	  {
	    if ((XMOUSE>xx[i]+3) && (XMOUSE<xx[i]+56) &&
		(YMOUSE>303+yy[i]) && (YMOUSE<303+yy[i]+10))
	      {
	       cursor_counter(2);
	       write_text(BLACK,xx[ii],yy[ii],len,10,YELLOW,accu_mode[ii]);
	       ii=i;
	       write_text(LIGHTBLUE,xx[ii],yy[ii],len,10,YELLOW,accu_mode[ii]);
	       choix=1;
	      }
	  }
      }
    if (!choix)
      {
       if (kbhit())
	 {
	  cc=getch();
	  if (cc==0) cc=getch();

	  switch (cc)
		{
		 case ESC  :
			    ii=-1;
			    choix=1;
			    break;

		 case LEFT :
			    write_text(BLACK,xx[ii],yy[ii],len,10,YELLOW,accu_mode[ii]);
			    if (ii==0) ii=1;
			    else ii--;
			    write_text(LIGHTBLUE,xx[ii],yy[ii],len,10,YELLOW,accu_mode[ii]);
			    break;

		 case RIGHT:
			    write_text(BLACK,xx[ii],yy[ii],len,10,YELLOW,accu_mode[ii]);
			    if (ii==1) ii=0;
			    else ii++;
			    write_text(LIGHTBLUE,xx[ii],yy[ii],len,10,YELLOW,accu_mode[ii]);
			    break;

		 case ENTER:
			    choix=1;
			    break;
	       }
	 }
      }
   }
    while(!choix);
    mouse_xclip(3,635);
    mouse_yclip(5,335);
 return(ii);
}

/***************************************************************************/
int accu_param(void)
{
  int param_length[]={6,3},
       x[]={252,252},
       y[]={4,16},
       ind,select,x0=7,y0=307,exit=0,action=0,insert=0;

  char c,par_accu[2][7]={"   ","   "};

 itoa(WAITIME,par_accu[0],10);par_accu[0][strlen(par_accu[0])]='\0';
 itoa(NACC,par_accu[1],10);


 setviewport(7,307,512,346,0);
 clearviewport();

 setcolor(YELLOW);
 outtextxy(4,29,"SPACE or left button to validate,Esc or right button to quit");

 setcolor(WHITE);
 outtextxy(20,4, "Dark delay time(ms)........:");
 outtextxy(20,16,"Number of accumulations....:");

 setcolor(LIGHTRED);
 outtextxy(252,4,par_accu[0]);
 outtextxy(252,16,par_accu[1]);

 ind=0;

 write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par_accu[ind]);
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
       if (kbhit() && (!action)) {c=getch();action=1;}
      }
       while (!action);

       if (c==ESC) {exit=-1; break;}
       if (c==SPACE)
	 {
	  WAITIME=atoi(par_accu[0]);
	  NACC=atoi(par_accu[1]);
	  select=1;
	 }
       if (c==0) c=getch();
       switch(c)
	 {
	  case LEFT  :
		    if (ind!=0)
		      {
		       write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par_accu[ind]);
		       ind--;
		       write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par_accu[ind]);
		      }
		    break;

	  case RIGHT :
		     if (ind!=1)
		       {
			write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par_accu[ind]);
			ind++;
			write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par_accu[ind]);
		       }
		      break;

	  case UP    :
		      if (ind!=0)
			{
			 write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par_accu[ind]);
			 ind--;
			 write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par_accu[ind]);
			}
		      break;

	   case DOWN  :
		      if (ind!=1)
			{
			 write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par_accu[ind]);
			 ind++;
			 write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par_accu[ind]);
			}
		      break;

	   case ENTER :MASKEDIT=1;
		      edit_string(param_length[ind],x[ind]+x0,y[ind]+y0,par_accu[ind],&insert);
		      MASKEDIT=0;
                      setviewport(7,307,512,346,0);
		      write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par_accu[ind]);
		      /* Next parameter */
		      if (ind==1) {ind=0;}
		      else {ind=1;}
		      write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par_accu[ind]);
		      break;
	  }
   }
    while (!select);
 if (control==MOUSE)
   {cursor_shape(0);cursor_counter(1);}
 return(exit);
}
/**************************************************************************/
void ask_accu(int mode)
{
 int cancel=0,flag_sav=1;

 open_window(3,271,516,349,LIGHTGREEN);
 do
   {
     cancel=edit_param();
     if (cancel == 0)
       {
	if (control==MOUSE) cursor_shape(2);
	flag_sav=save_ms0(mode);
       }
     else
       {
	flag_sav=0;
       }
   }
   while (flag_sav==1);
 close_window(3,271);
}

/**************************************************************************/
