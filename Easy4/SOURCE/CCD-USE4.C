/*========================================================================*/
/*                              CCD_USE4                                  */
/*========================================================================*/
/*                                                                        */
/* This file contains all the fonctions for the detector CCD (EG&G)       */
/* OMAVISION IV                                                           */
/*========================================================================*/

#include <acq4.h>
#include <stdio.h>
#include <decl0.h>
#include <graphics.h>
#include <commccd.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>

extern int NACC,
	   IGN_SCANS,control,change_comnd,
	   CCD_MODEL,MASKEDIT,
	   domaine;

extern FILE *fp;

/*========================================================================*/

typedef struct {
	char slcx0[4];
	char slcdeltax[4];
	} slice;

typedef struct {
	char trky0[4];
	char trkdeltay[4];
	 } track;

slice SLC[10];

track TRK[10];

int set_ind,frame_slc_trk,
    ccd,trkmode,slcmode,
    displ_error_flag,error_flag,
    ind_max=2;        /* modif OMA4 */

char tracks4[4],slices4[4],
     x04[4],deltax4[4],y04[4],deltay4[4],
     error_message[30],
     *TRACKMODE[3]={"Contiguous","Random"},
     *SLICEMODE[3]={"Uniform","Non Uniform"},
     *TRACKSLIMIT[3]={"384","256"},
     *SLICESLIMIT[3]={"576","256"};

void ccd_screen(int ccd_cx[],int ccd_cy[],char ccd_txt[6][20]),
     ccd_description();
void ccd_setting(int ccd_cx[],int ccd_cy[],char ccd_txt[6][20]);
char *modeltext(void);
char *cooltypetext(void);
char *outregtext(int mode);
char *coolonoftext(int mode);
char *shiftmodetext(void);
char *pixeltimetext(int mode);
void enter_shiftmode(int xx,int yy,char *txt);
void enter_shiftregister(int xx,int yy,char *txt);
void enter_antibloom(int xx,int yy,char *txt);
void enter_pixeltime(int xx,int yy,char *txt);
void enter_coolonof(int xx,int yy,char *txt);
void enter_temp(int xx,int yy,char *txt);
/***************************************************************************/
void enter_ccd_frame_setup(void)

/* Allow to change the way in which CCD data are read out */

{
 char dd;
 int fin;

 if (control==MOUSE) {cursor_counter(2);test_press(1);}
 setactivepage(1);
 setviewport(0,0,639,349,0);
 clearviewport();
 setvisualpage(1);
 if (CCD_MODEL==OMA4) ind_max=3;
 frame_screen();
 slice_screen();
 track_screen();
 fin=0;
 frame_slc_trk=0;
 set_ind=0;
 select_setting_up(LIGHTGREEN);
 do
   {
    if ((control==MOUSE) && (test_press(1)!=0)) {fin=1;break;}
    if (kbhit())
      {
       dd=getch();
       switch (dd)
	     {
	      case 0:
		      dd=getch();
		      switch (dd)
			{
			 case UP:
				     if (set_ind==0)
				       {
					select_setting_up(WHITE);
					frame_slc_trk=frame_slc_trk ^ 1;
					select_setting_up(LIGHTGREEN);
				       }
				     break;

			 case DOWN:
				     if (set_ind==0)
				       {
					select_setting_up(WHITE);
					frame_slc_trk=frame_slc_trk ^ 1;
					select_setting_up(LIGHTGREEN);
				       }
				     break;

			 case PG_UP:
				     select_setting_up(WHITE);
				     set_ind--;
				     if (set_ind<0) set_ind=ind_max;
				     select_setting_up(LIGHTGREEN);
				     break;

			 case PG_DOWN:
				     select_setting_up(WHITE);
				     set_ind++;
				     if (set_ind==ind_max+1) set_ind=0;
				     select_setting_up(LIGHTGREEN);
				     break;
			}
		      break;

	   case ENTER:
		      activ_setting_up();
		      break;

	   case ESC:
		     fin=1;
		     break;
	  }
      }
   }
     while (!fin);
 write_frame();
 setactivepage(0);
 setvisualpage(0);
 setviewport(0,0,639,349,0);
 set_frame();
 change_comnd=1;
 if (control==MOUSE) cursor_counter(1);
}

/*************************************************************************/

void select_setting_up(int color)
{
 switch (set_ind)
       {
	case 0:
	       setcolor(color);
	       outtextxy(225,10," FRAME DESCRIPTION ");
	       switch (frame_slc_trk)
		     {
		      case 0:
			     outtextxy(100,40,"SLICES ");
			     break;
		      case 1:
			     outtextxy(100,60,"TRACKS ");
			     break;
		     }
	       break;

	case 1:
	       setcolor(color);
	       outtextxy(75,90,"SLICES SETTING UP");
	       break;

	case 2:
	       setcolor(color);
	       outtextxy(395,90,"TRACKS SETTING UP");
	       break;

       }
}
/*************************************************************************/

void activ_setting_up(void)

{
 switch (set_ind)
       {
	case 0:
	       frame_description();
	       break;
	case 1:
	       slice_description();
	       break;
	case 2:
	       track_description();
	       break;
       }
}
/*************************************************************************/

void frame_description(void)
{
 int ins=0;

 switch (frame_slc_trk)
       {
	case 0:
		do
		   {
		     /*Enter slice mode */
		    enter_trkslc_mode(234,40,0);

		     /*Enter slices*/
		    edit_string(strlen(slices4),434,40,slices4,&ins);

		    frame_verify(slices4,SLICESLIMIT[slcmode]);
		    if ((error_flag) && (!(displ_error_flag)))
		      {
		       displ_error_message(error_message);
		      }
		   }
		while(error_flag);
		if (displ_error_flag) close_error_window();
		break;

	case 1:

		do
		   {
		     /*Enter track mode */
		    enter_trkslc_mode(234,60,1);

		     /*Enter tracks*/
		    edit_string(strlen(tracks4),434,60,tracks4,&ins);

		    frame_verify(tracks4,TRACKSLIMIT[trkmode]);
		    if ((error_flag) && (!(displ_error_flag)))
		      {
		       displ_error_message(error_message);
		      }
		   }
		while(error_flag);
		if (displ_error_flag) close_error_window();
		break;
       }
  setviewport(0,0,630,349,0);
}

/***************************************************************************/
void read_frame(void)
{
 int i;

 if (domaine==UV)
   { fp=fopen("frame.uv","rt");}
 else
   { fp=fopen("frame","rt");}

 fscanf(fp,"%s %d %s %d",slices4,&slcmode,tracks4,&trkmode);
 switch (slcmode)
       {
	case 0: /* Uniform */
		fscanf(fp,"%s %s",x04,deltax4);
		break;

	case 1: /* Non uniform */
		for (i=1;i<=atoi(slices4);i++)
		   fscanf(fp,"%s %s",SLC[i].slcx0,SLC[i].slcdeltax);
		break;
       }
 switch (trkmode)
       {
	case 0: /* Contiguous */
		fscanf(fp,"%s %s",y04,deltay4);
		break;

	case 1: /* Random */
		for (i=1;i<=atoi(tracks4);i++)
		   fscanf(fp,"%s %s",TRK[i].trky0,TRK[i].trkdeltay);
		break;
       }
 fclose(fp);
}
/**************************************************************************/
void write_frame(void)
{
 int i;

 if (domaine==UV)
   {fp=fopen("frame.uv","wt");}
 else
   {
    fp=fopen("frame","wt");
   }
 fprintf(fp,"%s\n%d\n%s\n%d\n",slices4,slcmode,tracks4,trkmode);
 switch (slcmode)
       {
	case 0: /* Uniform */
		fprintf(fp,"%s %s\n",x04,deltax4);
		break;

	case 1: /* Non uniform */
		for (i=1;i<=atoi(slices4);i++)
		   fprintf(fp,"%s %s\n",SLC[i].slcx0,SLC[i].slcdeltax);
		break;
       }
 switch (trkmode)
       {
	case 0: /* Contiguous */
		fprintf(fp,"%s %s\n",y04,deltay4);
		break;

	case 1: /* Random */
		for (i=1;i<=atoi(tracks4);i++)
		   fprintf(fp,"%s %s\n",TRK[i].trky0,TRK[i].trkdeltay);
		break;
       }
 fclose(fp);
}
/**************************************************************************/
void frame_screen(void)
{

 setcolor(LIGHTGREEN);
 rectangle(0,0,639,349);
 rectangle(4,4,635,345);
 line(6,80,634,80);
 line(320,81,320,344);

 setcolor(WHITE);
 outtextxy(225,10," FRAME DESCRIPTION ");

 outtextxy(100,40,"SLICES ");
 outtextxy(180,40,"Mode ");rectangle(230,36,340,52);
 outtextxy(360,40,"Number ");rectangle(430,36,470,52);

 setcolor(LIGHTRED);
 outtextxy(234,40,SLICEMODE[slcmode]);
 outtextxy(434,40,slices4);

 setcolor(WHITE);outtextxy(100,60,"TRACKS ");
 outtextxy(180,60,"Mode ");rectangle(230,56,340,72);
 outtextxy(360,60,"Number ");rectangle(430,56,470,72);

 setcolor(LIGHTRED);
 outtextxy(234,60,TRACKMODE[trkmode]);
 outtextxy(434,60,tracks4);
}

/*************************************************************************/

void slice_screen(void)
{
  int i;
  char ic[3];

  setcolor(WHITE);
  outtextxy(75,90,"SLICES SETTING UP");

  outtextxy(50,130,"SLICE");
  outtextxy(120,120,"Start at");
  outtextxy(120,130," column ");
  outtextxy(210,120,"Nbr of ");
  outtextxy(210,130,"columns");

  switch (slcmode)
	{
	 case 0:
		setcolor(LIGHTRED);
		outtextxy(58,145,"All");
		outtextxy(136,145,x04);
		outtextxy(226,145,deltax4);
		break;

	 case 1:
		setcolor(LIGHTRED);
		for (i=1;i<=atoi(slices4);i++)
		   {
		    itoa(i,ic,10);
		    outtextxy(58,130+(i*15),ic);
		    outtextxy(136,130+(i*15),SLC[i].slcx0);
		    outtextxy(226,130+(i*15),SLC[i].slcdeltax);
		   }
		break;
	}
 }

/*************************************************************************/
 void slice_description(void)
 {
  int i,ins=0;
  char ic[10];

  switch (slcmode)
	{
	 case 0:
		 edit_string(3,136,145,x04,&ins);
		 edit_string(3,226,145,deltax4,&ins);
		 break;
	 case 1:
		 for (i=1;i<=atoi(slices4);i++)
		    {
		     itoa(i,ic,10);
		     outtextxy(58,130+(i*15),ic);
		     edit_string(3,136,130+(i*15),SLC[i].slcx0,&ins);
		     edit_string(3,226,130+(i*15),SLC[i].slcdeltax,&ins);
		    }
		 break;
	}
   setviewport(0,0,630,349,0);
 }
/***************************************************************************/

 void track_screen(void)
 {
  int i;
  char ic[10];

  setcolor(WHITE);
  outtextxy(395,90,"TRACKS SETTING UP");
  outtextxy(370,130,"TRACK");
  outtextxy(440,120,"Start at");
  outtextxy(440,130,"   row  ");
  outtextxy(530,120,"Nbr of");
  outtextxy(530,130," rows ");
  switch (trkmode)
	{
	 case 0:
		{
		 setcolor(LIGHTRED);
		 outtextxy(375,145,"All");
		 outtextxy(464,145,y04);
		 outtextxy(546,145,deltay4);
		 break;
		}
	 case 1:
	       {
		setcolor(LIGHTRED);
		for (i=1;i<=atoi(tracks4);i++)
		   {
		    itoa(i,ic,10);
		    outtextxy(375,130+(i*15),ic);
		    outtextxy(464,130+(i*15),TRK[i].trky0);
		    outtextxy(546,130+(i*15),TRK[i].trkdeltay);
		   }
		break;
	       }
	}
 }

/*************************************************************************/

void track_description(void)

{
 int i,ins=0;
 char ic[10];

 switch (trkmode)
       {
	case 0:
		{
		 setcolor(LIGHTRED);
		 outtextxy(375,145,"All");
		 edit_string(3,464,145,y04,&ins);
		 edit_string(3,546,145,deltay4,&ins);
		 break;
		}

	case 1:
		{
		 for (i=1;i<atoi(tracks4);i++)
		    {
		     setcolor(LIGHTRED);
		     itoa(i,ic,10);
		     outtextxy(375,130+(i*15),ic);
		     edit_string(3,464,130+(i*15),TRK[i].trky0,&ins);
		     edit_string(3,464,130+(i*15),TRK[i].trkdeltay,&ins);
		    }
		 break;
		}
       }
  setviewport(0,0,630,349,0);
}

/***************************************************************************/

void enter_trkslc_mode(int x,int y,int trkslc)
{
 char c,*s;
 int mode;

 switch (trkslc)
	{
	  case 0: mode=slcmode;
		  s=SLICEMODE[mode];
		  break;

	  case 1: mode=trkmode;
		  s=TRACKMODE[mode];
		  break;
	}
 setviewport(x,y,x+88,y+10,1);
 clearviewport();
 setcolor(LIGHTGREEN);
 outtextxy(0,0,s);
 while ((c=getch())!=ENTER)
      {
       if ((c==LEFT) | (c==RIGHT)) mode=mode ^ 1;
       if (trkslc==0)
	  {
	   s=SLICEMODE[mode];
	   slcmode=mode;
	  }
       else
	  {
	   s=TRACKMODE[mode];
	   trkmode=mode;
	  }
       clearviewport();
       outtextxy(0,0,s);
      }
 clearviewport();
 setcolor(LIGHTRED);
 outtextxy(0,0,s);
}

/*************************************************************************/

 void frame_verify(char *param,char *paramlim)
 {
  if (atoi(param)>atoi(paramlim))
    {
     strcpy(error_message,"Number must be < \0");
     strcat(error_message,paramlim);
     error_flag=1;
    }
  else
    {
     error_flag=0;
    }
 }

/*************************************************************************/

 void displ_error_message(char *message)
 {
  setviewport(0,0,639,439,1);
  open_window(170,120,470,150,LIGHTRED);
  setcolor(YELLOW);
  outtextxy(40,8,message);
  displ_error_flag=1;
 }

/*************************************************************************/

 void close_error_window(void)
 {
  close_window(170,120);
  displ_error_flag=0;
 }

/***************************************************************************/

 void slice_setup(void)
 {

  switch (slcmode)
	{
	 case 0: /* Uniform mode */
		 break;
	 case 1: /* Non uniform mode */
		 break;
	}
 }
/***************************************************************************/

void ccd_init(void)

{
 /*
 if ((ccd=ibfind("CCDEG&G"))<0) printf("ERROR! 1461 unit not installed\n");
 else
   {
    read_frame();
   }
 */
}

/***************************************************************************/
void ccd_description(void)
{
 int ccd_cx[6],ccd_cy[6];
 char ccd_txt[6][20]={"","","","","",""};

 if (control==MOUSE) {cursor_counter(2);test_press(1);}
 setactivepage(1);
 setviewport(0,0,639,349,0);
 clearviewport();
 setvisualpage(1);
 ccd_screen(ccd_cx,ccd_cy,ccd_txt);
 ccd_setting(ccd_cx,ccd_cy,ccd_txt);
 setactivepage(0);
 setvisualpage(0);
 setviewport(0,0,639,349,0);
}
/************************************************************************/
void ccd_screen(int ccd_cx[],int ccd_cy[],char ccd_txt[6][20])
{
 struct viewporttype viewinfo;
 char ch[30],*ch1;
 int cx,cy,ccd_ind;

 getviewsettings(&viewinfo);
 setcolor(LIGHTGREEN);
 rectangle(viewinfo.left,viewinfo.top,viewinfo.right,viewinfo.bottom);
 rectangle(viewinfo.left+4,viewinfo.top+4,viewinfo.right-4,viewinfo.bottom-4);

 strcpy(ch,"CCD DESCRIPTION");
 cx=(viewinfo.right-viewinfo.left)-8*strlen(ch);
 cx=viewinfo.left+(cx/2);
 cy=2*textheight(ch);
 write_text(RED,cx,cy,strlen(ch),10,YELLOW,ch);

 cx=10;
 cy+=3*textheight(ch);
 strcpy(ch,"General information");
 write_text(LIGHTBLUE,cx,cy,strlen(ch),10,YELLOW,ch);

 cy+=3*textheight(ch);
 strcpy(ch,"  * Detector model: ");
 setcolor(WHITE);outtextxy(cx,cy,ch);
 setcolor(LIGHTRED);outtextxy(cx+8*strlen(ch),cy,modeltext());

 cy+=2*textheight(ch);
 strcpy(ch,"  * Cool type     : ");
 setcolor(WHITE);outtextxy(cx,cy,ch);
 setcolor(LIGHTRED);outtextxy(cx+8*strlen(ch),cy,cooltypetext());

 cy+=2*textheight(ch);
 strcpy(ch,"  * Array size    : ");
 setcolor(WHITE);outtextxy(cx,cy,ch);
 cx+=textwidth(ch);
 sprintf(ch,"%4d x %4d",get_activex(),get_activey());
 setcolor(LIGHTRED);outtextxy(cx,cy,ch);
 setcolor(LIGHTGREEN);
 cy+=2*textheight(ch);
 line(viewinfo.left+5,cy,viewinfo.right-5,cy);
 cx=10;
 cy=cy+3*textheight(ch);
 strcpy(ch,"Selectable parameters");
 write_text(LIGHTBLUE,cx,cy,strlen(ch),10,YELLOW,ch);

 ccd_ind=0;
 setcolor(WHITE);
 cy+=3*textheight(ch);
 strcpy(ch,"  * Shift Mode    : ");
 setcolor(WHITE);outtextxy(cx,cy,ch);
 cx+=textwidth(ch);
 sprintf(ch,"Tracks %c Serial",186+7*get_shiftmode());
 strcpy(ccd_txt[ccd_ind],ch);
 ccd_cx[ccd_ind]=cx;ccd_cy[ccd_ind]=cy;
 setcolor(LIGHTRED);outtextxy(cx,cy,ccd_txt[ccd_ind]);

 cx=10;ccd_ind++;
 cy+=2*textheight(ch);
 strcpy(ch,"  * Shift register: ");
 setcolor(WHITE);outtextxy(cx,cy,ch);
 cx+=textwidth(ch);
 ccd_cx[ccd_ind]=cx;ccd_cy[ccd_ind]=cy;
 strcpy(ccd_txt[ccd_ind],outregtext(get_outreg()));
 setcolor(LIGHTRED);outtextxy(cx,cy,ccd_txt[ccd_ind]);

 cx=10;cy+=2*textheight(ch);ccd_ind++;
 strcpy(ch,"  * Anti-Bloom    : ");
 setcolor(WHITE);outtextxy(cx,cy,ch);
 cx+=textwidth(ch);
 ccd_cx[ccd_ind]=cx;ccd_cy[ccd_ind]=cy;
 sprintf(ccd_txt[ccd_ind],"%3d%",get_antibloom());
 setcolor(LIGHTRED);outtextxy(cx,cy,ccd_txt[ccd_ind]);

 cx=10;cy+=2*textheight(ch);ccd_ind++;
 strcpy(ch,"  * Pixel Time    : ");
 setcolor(WHITE);outtextxy(cx,cy,ch);
 cx+=textwidth(ch);
 ccd_cx[ccd_ind]=cx;ccd_cy[ccd_ind]=cy;
 strcpy(ccd_txt[ccd_ind],pixeltimetext(get_pixeltime()));
 setcolor(LIGHTRED);outtextxy(cx,cy,ccd_txt[ccd_ind]);

 cx=10;cy=cy+3*textheight(ch);
 strcpy(ch,"Cooling control");
 write_text(LIGHTBLUE,cx,cy,strlen(ch),10,YELLOW,ch);

 cy+=3*textheight(ch);ccd_ind++;
 strcpy(ch,"  * Required Temperature: ");
 setcolor(WHITE);outtextxy(cx,cy,ch);
 cx+=textwidth(ch);
 ccd_cx[ccd_ind]=cx;ccd_cy[ccd_ind]=cy;
 sprintf(ccd_txt[ccd_ind],"%+03d",get_temp());
 setcolor(LIGHTRED);outtextxy(cx,cy,ccd_txt[ccd_ind]);

 cx=10;cy+=2*textheight(ch);ccd_ind++;
 strcpy(ch,"  * Cooler              : ");
 setcolor(WHITE);outtextxy(cx,cy,ch);
 cx+=textwidth(ch);ccd_cx[ccd_ind]=cx;ccd_cy[ccd_ind]=cy;
 strcpy(ccd_txt[ccd_ind],coolonoftext(get_coolonof()));
 setcolor(LIGHTRED);outtextxy(cx,cy,ccd_txt[ccd_ind]);
}
/*************************************************************************/
char *modeltext(void)
{
 char *ch0[]={"NO DETECTOR",
	      "Thomson 512 CCD",
	      "Thomson 512 Dual CCD",
	      "EEV CCD",
	      "1024 RAPDA",
	      "Thomson 1024 CCD"};

 return(ch0[get_dmodel()]);
}
/*************************************************************************/
char *cooltypetext(void)
{
 char *ch0[]={"NO COOLER",
	      "Cryogenic",
	      "Peltier"};

 return(ch0[get_cooltype()]);
}
/*************************************************************************/
char *outregtext(int mode)
{
 char *ch0[]={"   A","   B","DUAL"};

 return(ch0[mode]);
}
/************************************************************************/
char *coolonoftext(int mode)
{
 char *ch0[]={"OFF"," ON"};

 return(ch0[mode]);
}
/************************************************************************/
char *pixeltimetext(int mode)
{
 char *ch0[]={"Normal",
	      "Fast  ",
	      "Slow  "};

 return(ch0[mode]);
}

/************************************************************************/

void ccd_setting(int ccd_cx[],int ccd_cy[],char ccd_txt[5][20])
{
 int fin=0,ccd_ind=0;
 char cc,txt[20];

 write_text(LIGHTGRAY,ccd_cx[ccd_ind],ccd_cy[ccd_ind],strlen(ccd_txt[ccd_ind]),10,YELLOW,ccd_txt[ccd_ind]);
 do
   {
    if (kbhit())
      {
       cc=getch();
       if (cc==0) cc=getch();
       switch(cc)
	  {
	   case ESC  : fin=1;
		       break;

	   case UP   : write_text(BLACK,ccd_cx[ccd_ind],ccd_cy[ccd_ind],strlen(ccd_txt[ccd_ind]),10,LIGHTRED,ccd_txt[ccd_ind]);
		       if (ccd_ind==0) ccd_ind=5;
		       else ccd_ind--;
		       write_text(LIGHTGRAY,ccd_cx[ccd_ind],ccd_cy[ccd_ind],strlen(ccd_txt[ccd_ind]),10,YELLOW,ccd_txt[ccd_ind]);
		       break;

	   case DOWN : write_text(BLACK,ccd_cx[ccd_ind],ccd_cy[ccd_ind],strlen(ccd_txt[ccd_ind]),10,LIGHTRED,ccd_txt[ccd_ind]);
		       if (ccd_ind==5) ccd_ind=0;
		       else ccd_ind++;
		       write_text(LIGHTGRAY,ccd_cx[ccd_ind],ccd_cy[ccd_ind],strlen(ccd_txt[ccd_ind]),10,YELLOW,ccd_txt[ccd_ind]);
		       break;

	   case ENTER: write_text(LIGHTGRAY,ccd_cx[ccd_ind],ccd_cy[ccd_ind],strlen(ccd_txt[ccd_ind]),10,LIGHTGREEN,ccd_txt[ccd_ind]);
		       switch(ccd_ind)
			  {
			   case 0: enter_shiftmode(ccd_cx[ccd_ind],ccd_cy[ccd_ind],ccd_txt[ccd_ind]);
				   break;
			   case 1: enter_shiftregister(ccd_cx[ccd_ind],ccd_cy[ccd_ind],ccd_txt[ccd_ind]);
				   break;
			   case 2: enter_antibloom(ccd_cx[ccd_ind],ccd_cy[ccd_ind],ccd_txt[ccd_ind]);
				   break;
			   case 3: enter_pixeltime(ccd_cx[ccd_ind],ccd_cy[ccd_ind],ccd_txt[ccd_ind]);
				   break;
			   case 4: enter_temp(ccd_cx[ccd_ind],ccd_cy[ccd_ind],ccd_txt[ccd_ind]);
				   break;
			   case 5: enter_coolonof(ccd_cx[ccd_ind],ccd_cy[ccd_ind],ccd_txt[ccd_ind]);
				   break;
			  }
		       /*strcpy(ccd_txt[ccd_ind],txt);*/
		       write_text(BLACK,ccd_cx[ccd_ind],ccd_cy[ccd_ind],strlen(ccd_txt[ccd_ind]),10,LIGHTRED,ccd_txt[ccd_ind]);
		       if (ccd_ind==5) ccd_ind=0;
		       else ccd_ind++;
		       write_text(LIGHTGRAY,ccd_cx[ccd_ind],ccd_cy[ccd_ind],strlen(ccd_txt[ccd_ind]),10,YELLOW,ccd_txt[ccd_ind]);
		       break;
	  }
      }
   }
   while (!fin);
 /*change_comnd=1;*/
}

void enter_shiftmode(int xx,int yy,char *txt)
{
 int mode;
 char ccc;

 mode = get_shiftmode();
 while ((ccc=getch())!=ENTER)
      {
       if (ccc==0) ccc=getch();
       if ((ccc==LEFT) | (ccc=RIGHT)) mode=mode ^ 1;
       sprintf(txt,"Tracks %c Serial",186+7*mode);
       write_text(LIGHTGRAY,xx,yy,strlen(txt),10,LIGHTGREEN,txt);
      }
 set_shiftmode(mode);
 change_comnd=1;
}

void enter_shiftregister(int xx,int yy,char *txt)
{
 int mode;
 char ccc;

 mode = get_outreg();
 while ((ccc=getch())!=ENTER)
      {
       if (ccc==0) ccc=getch();
       switch(ccc)
	  {
	   case LEFT : if (mode==0) mode=2;
		       else mode--;
		       break;
	   case RIGHT: if (mode==2) mode=0;
		       else mode++;
		       break;
	  }
       strcpy(txt,outregtext(mode));
       write_text(LIGHTGRAY,xx,yy,strlen(txt),10,LIGHTGREEN,txt);
      }
 set_outreg(mode);
 change_comnd=1;
}

void enter_antibloom(int xx,int yy,char *txt)
{
 int val;
 char ccc;

 val=get_antibloom();
 while ((ccc=getch())!=ENTER)
      {
       if (ccc==0) ccc=getch();
       switch(ccc)
	  {
	   case LEFT : if (val==0) val=100;
		       else val-=10;
		       break;

	   case RIGHT: if (val==100) val=0;
		       else val+=10;
		       break;
	  }
       sprintf(txt,"%3d%",val);
       write_text(LIGHTGRAY,xx,yy,strlen(txt),10,LIGHTGREEN,txt);
      }
 set_antibloom(val);
 change_comnd=1;
}

void enter_pixeltime(int xx,int yy,char *txt)
{
 int mode;
 char ccc;

 mode = get_pixeltime();
 while ((ccc=getch())!=ENTER)
      {
       if (ccc==0) ccc=getch();
       switch(ccc)
	  {
	   case LEFT : if (mode==0) mode=2;
		       else mode--;
		       break;
	   case RIGHT: if (mode==2) mode=0;
		       else mode++;
		       break;
	  }
       strcpy(txt,pixeltimetext(mode));
       write_text(LIGHTGRAY,xx,yy,strlen(txt),10,LIGHTGREEN,txt);
      }
 set_pixeltime(mode);
 change_comnd=1;
}

void enter_coolonof(int xx,int yy,char *txt)
{
 int mode;
 char ccc;

 mode = get_coolonof();
 while ((ccc=getch())!=ENTER)
      {
       if (ccc==0) ccc=getch();
       if ((ccc==LEFT) | (ccc=RIGHT)) mode=mode ^ 1;
       strcpy(txt,coolonoftext(mode));
       write_text(LIGHTGRAY,xx,yy,strlen(txt),10,LIGHTGREEN,txt);
      }
 set_coolonof(mode);
 change_comnd=1;
}

void enter_temp(int xx,int yy,char *txt)
{
 int insert=0,val;

 MASKEDIT=2;
 edit_string(strlen(txt),xx,yy,txt,&insert);
 val=atoi(txt);
 setviewport(0,0,639,349,0);
 set_temp(val);
 change_comnd=1;
}