/*=======================================================================*/
/*                            PLATE.C                                    */
/*=======================================================================*/
/* This file contains all the function for driving the microplate MCL    */
/*      MCL microplate  MARZHAUSER (0.1æm)                               */
/*=======================================================================*/

#include <dos.h>
#include <bios.h>
#include <stdio.h>
#include <graphics.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include "acq4.h"
#include "micropla.h"

extern int X1[],Y1[],X2[],Y2[],
	   control,XMOUSE,YMOUSE,
	   MASKEDIT;

extern char MICRO_PLATE_PORT[];


int flagjoystick;  /*0 moving from computer (Joystick disable)*/
		   /*1 moving from joystick (Joystick enable) */

int serialreg1,serialreg2; /* Address of serial register */

char xstr[15],ystr[15],mclstatus[6],
     xrel[10]=" 000000",yrel[10]=" 000000";
/*=======================================================================*/
void init_plate_comm(void)
{
 int reg0;

 _AH=0;   /*Initialisation function */
 if (strcmp(MICRO_PLATE_PORT,"COM1")==0)
   {
     _DX=0;   /*Port number */
     serialreg1=0x3f8;
     serialreg2=0x3fd;
     reg0=0x3fb;
   }
 else
   {
     _DX=1;
     serialreg1=0x2f8;
     serialreg2=0x2fd;
     reg0=0x2fb;
   }
 _AL=0xA7; /*Initilalisation parameter bit flag*/
	  /*2400baud,no parity,2 stop bits,8 data bits */

 geninterrupt(0x14);
 outportb(reg0,7);
 while (inportb(serialreg2) & 1) inportb(serialreg1);

}
/*************************************************************************/
void outcom(char *str)     /* Sending information to the microplate */
{
 int i=0;
 long tm;

 tm=biostime(0,0l);
 do
   {
    if (inportb(serialreg2)&0x20)
    outportb(serialreg1,str[i++]);
   }
    while((str[i-1]!='\r' || !i) && biostime(0,0l)-tm<60);
 if (biostime(0,0l)-tm>=60)
   {
    printf("\007");
   }
}
/*************************************************************************/
void inpcom(char *str)     /*Reading information from the microplate */
{
 int i,j;
 long tm;
 char c;

 j=i=0;
 tm=biostime(0,0l);
 do
    {
	if (inportb(serialreg2)&1)
	j=str[i++]=inportb(serialreg1);
    }
     while (j!='\r');
 str[i-1]=0;
}
/*************************************************************************/
void enable_joystick()
{
 char c;

 outcom(" ");
 outcom("U\014o\r");
 outcom("U\007j\r");
 outcom("U\120\r");
 flagjoystick=1;
 setcolor(BLACK);
 outtextxy(X1[8]+80,Y1[8]+7,"OFF");
 outtextxy(X1[6]+20,Y1[6]+7,xstr);
 outtextxy(X1[7]+20,Y1[7]+7,ystr);
 setcolor(WHITE);
 outtextxy(X1[8]+80,Y1[8]+7," ON");
}
/*************************************************************************/
void disable_joystick(void)
{
  outcom("j\r");
  inpcom(mclstatus);
  flagjoystick=0;
  write_text(BLACK,X1[8]+80,Y1[8]+7,3,11,WHITE,"OFF");
  read_xplate();read_yplate();
}
/*************************************************************************/
void read_xplate(void)
{
 outcom("U\103\r");
 inpcom(xstr);
 sprintf(xstr,"% 06ld",atol(xstr));
 write_text(BLACK,X1[6]+20,Y1[6]+7,7,11,WHITE,xstr);
}
/*************************************************************************/
void read_yplate(void)
{
 outcom("U\104\r");
 inpcom(ystr);
 sprintf(ystr,"% 06ld",atol(ystr));
 write_text(BLACK,X1[7]+20,Y1[7]+7,7,11,WHITE,ystr);
}
/*************************************************************************/
void move_plate_to(char *xpos,char *ypos)
{
 char str1[20],str2[20];

 strcpy(str1,"U ");
 strcpy(str2,"U\001");
 strcat(str1,xpos);
 strcat(str1,"\r");
 str1[1]=0;
 strcat(str2,ypos);
 strcat(str2,"\r");
 outcom("U\007r\r");
 outcom(str1);
 outcom(str2);
 outcom("U\120\r");
 inpcom(mclstatus);
 setviewport(0,0,639,349,0);
 read_xplate();
 read_yplate();
}
/*************************************************************************/
void move_plate_rel(char *xpos,char *ypos)
{
 char str1[20],str2[20];

 strcpy(str1,"U ");
 strcpy(str2,"U\001");
 strcat(str1,xpos);
 strcat(str1,"\r");
 str1[1]=0;
 strcat(str2,ypos);
 strcat(str2,"\r");
 outcom("U\007v\r");
 outcom(str1);
 outcom(str2);
 outcom("U\120\r");
 inpcom(mclstatus);
 setviewport(0,0,639,349,0);
 read_xplate();
 read_yplate();
}
/*************************************************************************/
void init_xy(void)
{
 if (control==MOUSE) cursor_counter(2);

 if (flagjoystick)
   {
    open_window(103,120,435,155,YELLOW);
    write_text(RED,8,6,40,11,YELLOW,"MICROPLATE INITIALISATION IMPOSSIBLE !!!");
    write_text(RED,8,20,40,11,YELLOW,"           JOYSTICK IS ON              ");
    getch();
    close_window(103,120);
    if (control==MOUSE) cursor_counter(1);
   }
 else
   {
    open_window(110,120,402,140,LIGHTRED);
    write_text(YELLOW,8,6,35,11,LIGHTRED,"WARNING!  MICROPLATE INITIALISATION");
    if (control==MOUSE) {cursor_shape(3);cursor_counter(1);}
    outcom("U\007c\r");
    outcom("U\120");
    inpcom(mclstatus);
    close_window(110,120);
    if (control==MOUSE)
      {cursor_counter(2);cursor_shape(0);cursor_counter(1);}
   }
 read_xplate();
 read_yplate();
}
/*************************************************************************/
void joystick(void)
{
 char c;
 int unselect=0;

 if (flagjoystick) disable_joystick();
 else
   {
    enable_joystick();
    do
      {
       read_xplate();read_yplate();
       if (kbhit())
	 {c=getch();
	  if (c==ENTER) unselect=1;
	 }
       if (control==MOUSE)
	 {
	  if (test_press(0))
	    {
	     if ((XMOUSE>X1[8]) && (XMOUSE<X2[8]) && (YMOUSE>Y1[8]) && (YMOUSE<Y2[8]))
	       {
		unselect=1;
	       }
	    }
	 }
      }
       while(!unselect);
    disable_joystick();
   }
}
/*************************************************************************/
void enter_xy(int k)
{
 struct viewporttype viewinfo;

 char ch[65],c,*ch1[2]={"ABSOLUTE","RELATIVE"};

 int ll,l0,pos=0,hh,x0,y0,sp,
     i,fond,
     posmouse=0,select=0,cancel=0,suite,kk;

 setviewport(7,307,512,345,0);
 getviewsettings(&viewinfo);
 strcpy(ch,"MOVING PLATE");
 ll=(strlen(ch)+2)*8;
 setfillstyle(SOLID_FILL,LIGHTGRAY);
 l0=(viewinfo.right-viewinfo.left-ll)/2;
 setcolor(RED);
 bar(l0,1,l0+ll,11);
 outtextxy(l0+8,3,ch);

 if (flagjoystick)
   {
    strcpy(ch,"Joystick must be OFF !");
    ll=(strlen(ch)+2)*8;
    setfillstyle(SOLID_FILL,YELLOW);
    l0=(viewinfo.right-viewinfo.left-ll)/2;
    setcolor(RED);
    bar(l0,15,l0+ll,26);
    outtextxy(l0+8,18,ch);
    getch();
   }
 else
   {
    setcolor(YELLOW);
    outtextxy(2,31,"SPACE or left button to valid,ESC or right button to quit.");

    setcolor(WHITE);
    outtextxy(10,18,"Moving type: ");

    hh=11;ll=65;y0=18;sp=20;
    for (i=0;i<2;i++)
       {
	x0=120+i*ll+i*sp;setcolor(WHITE);
	rectangle(x0-4,y0-4,x0+65,y0+hh+1);
	if (i==pos) fond=LIGHTBLUE;
	else fond=BLACK;
	write_text(fond,x0,y0,8,hh,YELLOW,ch1[i]);
       }

    if (control==MOUSE)
      {
       mouse_xclip(6,510);
       mouse_yclip(310,340);
       set_curs_pos(viewinfo.left+120+pos*ll+pos*sp+32,viewinfo.top+y0+5);
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
		      for (i=0;i<2;i++)
			 {
			  x0=120+i*ll+i*sp;
			  if ((XMOUSE>viewinfo.left+x0) && (XMOUSE<viewinfo.left+x0+ll)
			       && (YMOUSE>viewinfo.top+y0) && (YMOUSE<viewinfo.top+y0+hh))
			    {
			     cursor_counter(2);
			     x0=120+pos*ll+pos*sp;
			     write_text(BLACK,x0,y0,8,hh,YELLOW,ch1[pos]);
			     pos=i;
			     x0=120+pos*ll+pos*sp;
			     write_text(LIGHTBLUE,x0,y0,8,hh,YELLOW,ch1[pos]);
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
			   x0=120+pos*ll+pos*sp;
			   write_text(BLACK,x0,y0,8,hh,YELLOW,ch1[pos]);
			   pos++;
			   if (pos>1) pos=0;
			   x0=120+pos*ll+pos*sp;
			   write_text(LIGHTBLUE,x0,y0,8,hh,YELLOW,ch1[pos]);
			   break;

		case LEFT:
			   x0=120+pos*ll+pos*sp;
			   write_text(BLACK,x0,y0,8,hh,YELLOW,ch1[pos]);
			   pos--;
			   if (pos<0) pos=1;
			   x0=120+pos*ll+pos*sp;
			   write_text(LIGHTBLUE,x0,y0,8,hh,YELLOW,ch1[pos]);
			   break;

	       case SPACE:
			   select=1;
			   break;

	       case ESC  :
			   cancel=1;
			   break;

	       default   :
			   printf("\a");
			   break;
	      }
	    }
	 }
     if (select)
       {
	setfillstyle(SOLID_FILL,BLACK);bar(10,14,280,30);
	switch (pos)
	   {
	    case 0: strcpy(ch,"Enter absolute position:  X: ");
		    strcat(ch,xstr);strcat(ch," .1æm  Y: ");
		    strcat(ch,ystr);
		    break;
	    case 1: strcpy(ch,"Enter relative moving in  X: ");
		    strcat(ch,xrel);strcat(ch," .1æm  Y: ");
		    strcat(ch,yrel);
		    break;
	   }
	strcat(ch," .1æm");
	setcolor(WHITE);
	outtextxy(10,18,ch);
	switch (k)
	   {
	    case 6:
		    MASKEDIT=2;
		    if (pos==0)
		      {edit_string(7,viewinfo.left+242,viewinfo.top+18,xstr,0);}
		    else
		      {edit_string(7,viewinfo.left+242,viewinfo.top+18,xrel,0);}
		    break;
	    case 7:
		    MASKEDIT=2;
		    if (pos==0)
		      {edit_string(7,viewinfo.left+378,viewinfo.top+18,ystr,0);}
		    else
		      {edit_string(7,viewinfo.left+378,viewinfo.top+18,yrel,0);}
		    break;
	   }

	 suite=0;cancel=0;posmouse=0;kk=0;
	 while((!suite) && (!cancel))
	   {
	    if (control==MOUSE)
	      {test_press(0);test_press(1);
	       while ((!posmouse) && (!kk))
		 {
		  if (test_press(1)!=0) {cancel=1;posmouse=1;}
		  if (test_press(0)!=0) {suite=1;posmouse=1;}
		  kk=kbhit();
		 }
	       }
	    if (!posmouse)
	      {
	       c=getch();if (c==0) c=getch();
	       switch(c)
		  {
		   case ESC  : cancel=1;
			       break;

		   case SPACE: suite=1;
			       break;

		   case LEFT : if (k==7)
				 {
				  k=6;
				  MASKEDIT=2;
				  if (pos==0)
				    {edit_string(7,viewinfo.left+242,viewinfo.top+18,xstr,0);}
				  else
				    {edit_string(7,viewinfo.left+242,viewinfo.top+18,xrel,0);}
				  }
				break;

		   case RIGHT: if (k==6)
				 {
				   k=7;
				   MASKEDIT=2;
				   if (pos==0)
				     {edit_string(7,viewinfo.left+378,viewinfo.top+18,ystr,0);}
				   else
				     {edit_string(7,viewinfo.left+378,viewinfo.top+18,yrel,0);}
				 }
			       break;
		  }
	      }
	   }
	switch (pos)
	   {
	    case 0: move_plate_to(xstr,ystr);
		    break;
	    case 1: move_plate_rel(xrel,yrel);
		    break;
	   }
	setviewport(viewinfo.left,viewinfo.top,viewinfo.right,viewinfo.bottom,0);
       }
   }
 if (control==MOUSE)
   {
     mouse_xclip(0,639);
     mouse_yclip(0,349);
     set_curs_pos(258,140);
     cursor_shape(0);
     cursor_counter(1);
   }
}
