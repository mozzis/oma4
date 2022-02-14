/*=========================================================================*/
/*                             FUNC1.C                                     */
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

extern int control,XMOUSE,YMOUSE,
	   MICRO_PLATE_MODU;

/*========================================================================*/
void ent_plat_scan()
{
 char c;
 int i,index=0,
     deb,posmouse=0,
     x0=3,y0=303,
     select=0;
 if (control==MOUSE) cursor_counter(2);
 open_window(3,303,516,349,LIGHTGREEN);
 for (i=0;i<3;i++)
    {
     deb=100*(i+1)+(50*i);
     outline_box(deb,17,50,15,YELLOW,LIGHTGRAY);
     displ_text(deb+5,22,i);
    }

 if (!MICRO_PLATE_MODU) index=1;

 deb=100*(index+1)+(50*index);
 displ_box(deb+1,18,deb+49,31,LIGHTBLUE);
 displ_text(deb+5,22,index);
 if (control == MOUSE)
   {
     mouse_xclip(6,512);
     mouse_yclip(307,346);
     set_curs_pos(x0+deb+20,y0+22);
     cursor_shape(0);
     cursor_counter(1);
   }

 while (!select)
      {
       if (control==MOUSE)
	 {
	  while ((!posmouse) && (!kbhit()))
	       {
		if (test_press(1)!=0)
		  {
		   index=2;
		   posmouse=1;select=1;
		   mouse_xclip(0,639);
		   mouse_yclip(0,349);
		  }
		if (test_press(0)!=0)
		  {
		   for (i=0;i<3;i++)
		      {
		       deb=x0+(100*(i+1)+50*i);
		       if ((XMOUSE>deb) && (XMOUSE<deb+50) && (YMOUSE>321) && (YMOUSE<334))
			 {
			  cursor_counter(2);
			  setfillstyle(SOLID_FILL,BLACK);
			  deb=100*(index+1)+(50*index);
			  bar(deb+1,18,deb+49,31);
			  displ_text(deb+5,22,index);
			  index=i;
			  setfillstyle(SOLID_FILL,LIGHTBLUE);
			  deb=100*(index+1)+(50*index);
			  bar(deb+1,18,deb+49,31);
			  displ_text(deb+5,22,index);
			  cursor_counter(1);
			  if ((!MICRO_PLATE_MODU) && (index==0))
			    {cursor_counter(2);
			     setfillstyle(SOLID_FILL,RED);
			     bar(3,3,510,43);
			     setcolor(YELLOW);
			     outtextxy(152,20,"MICRO PLATE NON INSTALLED !");
			     select=1;
			     cursor_shape(0);
			     cursor_counter(1);
			     sleep(1);
			     cursor_counter(2);
			     close_window(3,303);
/*			     set_curs_pos(258,140);*/
			     mouse_xclip(0,639);
			     mouse_yclip(0,349);
			     cursor_shape(0);
			     cursor_counter(1);
			     posmouse=1;select=1;
			     break;
			    }
			  else
			    {
			     posmouse=1;
			     select=1;
			    }
			 }
		     }
		 }
	       }
	 }
       if (!select)
	 {
	  c=getch();
	  if (c==ESC)
	    {
	     index=2;
	     select=1;
	     break;
	    }
	  if (c==0) c=getch();
	  switch (c)
	  {
	    case LEFT:   if (control==MOUSE) cursor_counter(2);
			 deb=100*(index+1)+50*index;
			 displ_box(deb+1,18,deb+49,31,BLACK);
			 displ_text(deb+5,22,index);
			 if (index==0) index=2;
			 else index--;
			 deb=100*(index+1)+50*index;
			 displ_box(deb+1,18,deb+49,31,LIGHTBLUE);
			 displ_text(deb+5,22,index);
			 if (control==MOUSE) cursor_counter(1);
			 break;

	    case RIGHT:  if (control==MOUSE) cursor_counter(2);
			 deb=100*(index+1)+50*index;
			 displ_box(deb+1,18,deb+49,31,BLACK);
			 displ_text(deb+5,22,index);
			 if (index==2) index=0;
			 else index++;
			 deb=100*(index+1)+50*index;
			 displ_box(deb+1,18,deb+49,31,LIGHTBLUE);
			 displ_text(deb+5,22,index);
			 if (control==MOUSE) cursor_counter(1);
			 break;

	    case 's':
	    case 'S':	if (index!=0)
			  {if (control==MOUSE) cursor_counter(2);
			   deb=100*(index+1)+50*index;
			   displ_box(deb+1,18,deb+49,31,BLACK);
			   displ_text(deb+5,22,index);
			   index=0;
			   deb=100*(index+1)+50*index;
			   displ_box(deb+1,18,deb+49,31,LIGHTBLUE);
			   displ_text(deb+5,22,index);
			  if (control==MOUSE) cursor_counter(1);
			  }
			break;

	    case 'c':
	    case 'C':
			if (index!=1)
			  {if (control==MOUSE) cursor_counter(2);
			   deb=100*(index+1)+50*index;
			   displ_box(deb+1,18,deb+49,31,BLACK);
			   displ_text(deb+5,22,index);
			   index=1;
			   deb=100*(index+1)+50*index;
			   displ_box(deb+1,18,deb+49,31,LIGHTBLUE);
			   displ_text(deb+5,22,index);
			   if (control==MOUSE) cursor_counter(1);
			  }
			break;

	    case 'q':
	    case 'Q':
			if (index!=2)
			  { if (control==MOUSE) cursor_counter(2);
			   deb=100*(index+1)+50*index;
			   displ_box(deb+1,18,deb+49,31,BLACK);
			   displ_text(deb+5,22,index);
			   index=2;
			   deb=100*(index+1)+50*index;
			   displ_box(deb+1,18,deb+49,31,LIGHTBLUE);
			   displ_text(deb+5,22,index);
			    if (control==MOUSE) cursor_counter(1);
			  }
			break;

	    case ENTER:
			  if ((!MICRO_PLATE_MODU) && (index==0))
			    {
			      if (control==MOUSE) cursor_counter(2);
			     setfillstyle(SOLID_FILL,RED);
			     bar(3,3,510,43);
			     setcolor(YELLOW);
			     outtextxy(152,20,"MICRO PLATE NON INSTALLED !");
			      if (control==MOUSE) cursor_counter(1);
			     select=1;
			     sleep(1);
			    }
			  else
			    {
			     select=1;
			    }
			  break;
	  }
	 }
      }

   switch (index)
      {
	   case 0:
		    close_window(3,303);
		    if (control==MOUSE)
		      {
		       mouse_xclip(0,639);
		       mouse_yclip(0,349);
/*		       set_curs_pos(258,140);*/
		       cursor_shape(0);
		      }
		    if (MICRO_PLATE_MODU)
		      { mapping(); }
		    break;
	   case 1:
		    enter_scan();
		    break;
	   case 2:
		    close_window(3,303);
		    if (control==MOUSE)
		      {
		       mouse_xclip(0,639);
		       mouse_yclip(0,349);
/*		       set_curs_pos(258,140);*/
		       cursor_shape(0);
		      }
		    break;
      }
  mouse_xclip(0,639);
  mouse_yclip(0,349);
 }

/***************************************************************************/

void displ_box(int x0,int y0,int x1,int y1,int back_color)
{
 setfillstyle(SOLID_FILL,back_color);
 bar(x0,y0,x1,y1);
}

/***************************************************************************/

void displ_text(int x,int y,int ind)
{
 switch (ind)
       {
	case 0:
	       setcolor(LIGHTRED);
	       outtextxy(x,y,"S");
	       setcolor(WHITE);
	       outtextxy(x+8,y,"tage");
	       break;

	case 1:
	       setcolor(WHITE);
	       outtextxy(x,y,"S");
	       setcolor(LIGHTRED);
	       outtextxy(x+8,y,"c");
	       setcolor(WHITE);
	       outtextxy(x+16,y,"an");
	       break;

	case 2:
	       setcolor(LIGHTRED);
	       outtextxy(x,y,"Q");
	       setcolor(WHITE);
	       outtextxy(x+8,y,"uit");
	       break;
       }
}

/***************************************************************************/