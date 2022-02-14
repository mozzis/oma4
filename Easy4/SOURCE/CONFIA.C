/*************************************************************+************/
/*                             CONFIA.C                                   */
/**************************************************************************/
#include <graphics.h>
#include <stdio.h>
#include <dos.h>
#include <math.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include <acq4.h>

#define  F0   93
#define TRUE  0
#define FALSE 1


char  PULSPARAM[5];

int P2=0,P3=0;

int enable_param[23]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

    XC[22]={200,176,122,106,106,280,162,162,
	    536,496,528,448,584,440,440,
	    192,168,200,200,584,448,464},

    YC1[22]={52,77,102,127,152,177,202,227,
	     52,77,102,127,152,177,202,
	     252,277,302,327,152,227,252},
    TABC[22];

char ST[5][25];

/*========================================================================*/

extern  char CCD_TYPE[];
extern  char UNITS,
	     TYPE[],
	     HIGH_DISP_COEFF[],
	     ANGLE[],FOCAL[],GRATING[],
	     UTIL_DIOD_BEG[2][5],UTIL_DIOD_END[2][5],
	     DETECT_BEGIN[],DETECT_END[],
	     INTENS_BEGIN[],INTENS_END[],
	     PLOTTER_PORT[],PLOTTER_TYPE[],
	     MICRO_PLATE_PORT[];

extern int   GRATING_TYPE,
	     FORE_MODU,
	     SPEC_MODU,
	     MULTI_MODU,
	     CCD_MODU,
	     PHOT_COUNT_MODU,
	     PULSOR_MODU,
	     MICRO_PLATE_MODU,
	     DAC_RESOL,
	     DAC[],NBR_PIXEL[],
	     PDA_TYPE,
	     DOUBLER;

extern char buffer_image[];
extern int XMOUSE,YMOUSE;
extern int control;
extern int MASKEDIT;

/*========================================================================*/

void modif_conf(int nb)
{
 int *modul_conf[]={&FORE_MODU,&SPEC_MODU,&PHOT_COUNT_MODU,&PULSOR_MODU,
		    &MICRO_PLATE_MODU,&MULTI_MODU,&CCD_MODU,&PDA_TYPE,&DAC[0],
		    &NBR_PIXEL[0],&DOUBLER};

 switch(nb)
   {
    case 0:  strcpy(TYPE,ST[TABC[nb]]);
	     if (TABC[1]==1) strcat(TYPE,"UV");
	     break;

    case 1:  if ((TYPE[strlen(TYPE)-2]=='U') && (TYPE[strlen(TYPE)-1]=='V')) TYPE[strlen(TYPE)-2]='\0';
	     if (TABC[nb]==1) strcat(TYPE,"UV");
	     break;

    case 2:  strcpy(GRATING,ST[TABC[nb]]);
	     break;

    case 6:
	     switch(TABC[nb])
		{
		 case 0:strcpy(PLOTTER_PORT,"COM1");
			break;
		 case 1:strcpy(PLOTTER_PORT,"COM2");
			break;
		 case 2:strcpy(PLOTTER_PORT,"LPT1");
			break;
		 case 3:strcpy(PLOTTER_PORT,"LPT2");
			break;
		}
	     break;

    case 7:  if (TABC[nb]==0) strcpy(PLOTTER_TYPE,"HPP");
	     else
	       {
		if (TABC[nb]==1) strcpy(PLOTTER_TYPE,"HPL");
		else strcpy(PLOTTER_TYPE,"LJ3");
	       }
	     break;


    case 8:
    case 9:
    case 10:
    case 11:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17: *modul_conf[nb-8]=TABC[nb];
	     break;

    case 12: *modul_conf[nb-8]=TABC[nb];
	     if (TABC[19]==0)
	       strcpy(MICRO_PLATE_PORT,"COM1");
	     else
	       strcpy(MICRO_PLATE_PORT,"COM2");
	     break;

    case 20: *modul_conf[10]=TABC[nb];
	     break;

    case 21: if (TABC[nb]==0) strcpy(CCD_TYPE,"OMA3");
	     else strcpy(CCD_TYPE,"OMA4");
	     break;
   }
}
/******************************************************************************/
int test_m(void)
{
 int a=100;

 if (test_press(0)!= 0)
   {
    if ((XMOUSE>350 && XMOUSE<420) && (YMOUSE>300 && YMOUSE<330) && P2==1) a=30;
    if ((XMOUSE>220 && XMOUSE<290) && (YMOUSE>300 && YMOUSE<330) && P2==0) a=30;
    if ((XMOUSE>480 && XMOUSE<550) && (YMOUSE>300 && YMOUSE<330) && P2==1) a=40;
    if ((XMOUSE>350 && XMOUSE<400) && (YMOUSE>300 && YMOUSE<330) && P2==0) a=40;

    if ((XMOUSE>14 && XMOUSE<31) || (XMOUSE>349 && XMOUSE<366))
      {
       if (YMOUSE>49 && YMOUSE<61) a=0;
       if (YMOUSE>74 && YMOUSE<86) a=1;
       if (YMOUSE>99 && YMOUSE<111) a=2;
       if (YMOUSE>124 && YMOUSE<136) a=3;
       if (YMOUSE>149 && YMOUSE<161) a=4;
       if (YMOUSE>174 && YMOUSE<186) a=5;
       if (YMOUSE>199 && YMOUSE<211) a=6;
       if (YMOUSE>224 && YMOUSE<236) a=7;

       if (P2==1)
	 {
	   if (YMOUSE>299 && YMOUSE<311) a=17;
	   if (YMOUSE>324 && YMOUSE<336) a=18;
	 }
       if (P3==1)
	 {
	   if (YMOUSE>224 && YMOUSE<236) a=20;
	   if (YMOUSE>249 && YMOUSE<261) a=21;
	 }
      }

    if (XMOUSE>349 && XMOUSE<366 && a<7) a=a+8;
   }
 return(a);
}

/**************************************************************************/

void config_display(void)
{
 int i;

 setviewport(0,0,639,349,0);
 if (control==MOUSE) cursor_counter(2);
 clearviewport();

 setfillstyle(SOLID_FILL,RED);
 setcolor(LIGHTGRAY);
 setlinestyle(0,0,3);
 rectangle(0,0,639,349);
 setlinestyle(0,0,1);
 bar(220,4,406,18);
 setcolor(YELLOW);
 outtextxy(230,8,"SYSTEM CHARACTERISTIC");

 setcolor(WHITE);
 rectangle(220,4,406,18);
 outline_box(15,50,15,10,WHITE,LIGHTGRAY);
 outline_box(15,75,15,10,WHITE,LIGHTGRAY);
 outline_box(15,100,15,10,WHITE,LIGHTGRAY);
 outline_box(15,125,15,10,WHITE,LIGHTGRAY);
 outline_box(15,150,15,10,WHITE,LIGHTGRAY);
 outline_box(15,175,15,10,WHITE,LIGHTGRAY);
 outline_box(15,200,15,10,WHITE,LIGHTGRAY);
 outline_box(15,225,15,10,WHITE,LIGHTGRAY);

 if (P2==1)
   {
    outline_box(15,250,15,10,WHITE,LIGHTGRAY);
    outline_box(15,275,15,10,WHITE,LIGHTGRAY);
    outline_box(15,300,15,10,WHITE,LIGHTGRAY);
    outline_box(15,325,15,10,WHITE,LIGHTGRAY);
   }

 outline_box(350,50,15,10,WHITE,LIGHTGRAY);
 outline_box(350,75,15,10,WHITE,LIGHTGRAY);
 outline_box(350,100,15,10,WHITE,LIGHTGRAY);
 outline_box(350,125,15,10,WHITE,LIGHTGRAY);
 outline_box(350,150,15,10,WHITE,LIGHTGRAY);
 outline_box(350,175,15,10,WHITE,LIGHTGRAY);
 outline_box(350,200,15,10,WHITE,LIGHTGRAY);

 if (P3==1)
   {
    outline_box(350,225,15,10,WHITE,LIGHTGRAY);
    outline_box(350,250,15,10,WHITE,LIGHTGRAY);
   }

 setcolor(WHITE);
 outtextxy(40,52,"SPECTROMETER TYPE :");
 outtextxy(40,77,"SPECTRAL RANGE :");
 outtextxy(40,102,"GRATING :");
 outtextxy(40,127,"ANGLE :");
 outtextxy(40,152,"FOCAL :");
 outtextxy(40,177,"HIGH DISPERSION COEFFICIENT :");
 outtextxy(40,202,"PLOTTER PORT :");
 outtextxy(42,227,"PLOTTER TYPE :");
 if (P2==1)
   {
    outtextxy(40,252,"PHOTODIODES TYPE :");
    outtextxy(40,277,"A/D CONVERTER :");
    outtextxy(40,302,"PHOTODIODES ARRAY :");
    outtextxy(40,327,"INTENSIFIED FIELD :");
   }
 outtextxy(375,52,"FOREMONOCHROMATOR :");
 outtextxy(375,77,"SPECTROGRAPH :");
 outtextxy(375,102,"PHOTONS COUNTING :");
 outtextxy(375,127,"PULSOR :");
 outtextxy(375,152,"MOTORIZED MICROS. PLATE :");
 outtextxy(375,177,"P.D.A :");
 outtextxy(375,202,"C.C.D :");
 if (P3==1)
   {
    outtextxy(375,227,"DOUBLER :");
    outtextxy(375,252,"CCD TYPE :");
   }

 if (P2==1)
   {
    setcolor(LIGHTGRAY);
    outtextxy(426,280,"ESC to ");
    outtextxy(442,312,"or");
    setcolor(LIGHTMAGENTA);
    rectangle(350,300,420,330);
    rectangle(480,300,550,330);
    setcolor(YELLOW);
    outtextxy(369,312,"SAVE");
    outtextxy(493,312,"CANCEL");
   }
 else
   {
    setcolor(LIGHTGRAY);
    outtextxy(296,280,"ESC to");
    outtextxy(312,312,"or");
    setcolor(LIGHTMAGENTA);
    rectangle(220,300,290,330);
    rectangle(350,300,420,330);
    setcolor(YELLOW);
    outtextxy(239,312,"SAVE");
    outtextxy(363,312,"CANCEL");
   }

 if (P2==1)
   { for (i=0;i<19;i++) conv_config(i,1);}
 else
   { for (i=0;i<15;i++) conv_config(i,1);}
 if (P3==1) {conv_config(20,1);conv_config(21,1);}

 if (control==MOUSE) cursor_counter(1);
}

/**************************************************************************/

void set_system_characteristics(void)

{
 char c;
 int exi=0,a=100,b=0,d=17,e=0,sor=0;
 int insert=0;

 read_characteristics();
 P2=MULTI_MODU;
 P3=CCD_MODU;
 config_display();

 if (control!=MOUSE)
   {
    setfillstyle(SOLID_FILL,LIGHTBLUE);
    bar(d,YC1[0],d+11,YC1[0]+6);
   }

 do
   {
    a=100;
    sor=0;
    if (kbhit())
      {
       c=getch();
       if (c==0) c=getch();
       switch (c)
	  {
	   case UP:
		      if (b!=0 && control==KEYBOARD)
			{
			 setfillstyle(SOLID_FILL,BLACK);
			 bar(d,YC1[b],d+11,YC1[b]+6);
			 if (b==15) {b=7;}
			 else if (b==8 && P2==1) {d=17;b=18;}
			 else if (b==8 && P2==0) {d=17;b=7;}
			 else if (b==20 && P3==1) {d=352;b=14;}
			 else b--;
			 setfillstyle(SOLID_FILL,LIGHTBLUE);
			 bar(d,YC1[b],d+11,YC1[b]+6);
			}
		      break;

	   case DOWN:
		      if (control==KEYBOARD && b!=21)
			{
			 if (b==14 && P3==1)
			   {
			    setfillstyle(SOLID_FILL,BLACK);
			    bar(d,YC1[b],d+11,YC1[b]+6);
			    b=20;d=352;
			    setfillstyle(SOLID_FILL,LIGHTBLUE);
			    bar(d,YC1[b],d+11,YC1[b]+6);
			   }
			 else if (b!=14)
			   {
			    setfillstyle(SOLID_FILL,BLACK);
			    bar(d,YC1[b],d+11,YC1[b]+6);
			    if (b==7 && P2==0) {d=352;b=8;}
			    else if (b==7 && P2==1) {b=15;}
			    else if (b==18) {d=352;b=8;}
			    else b++;
			    setfillstyle(SOLID_FILL,LIGHTBLUE);
			    bar(d,YC1[b],d+11,YC1[b]+6);
			   }
			}
		      break;

	   case RIGHT:
		      if (d!=352 && control==KEYBOARD)
			{
			 setfillstyle(SOLID_FILL,BLACK);
			 bar(d,YC1[b],d+11,YC1[b]+6);
			 if (b<15 && b!=7) {d=352;b=b+8;}
			 else if (b==7 && P3==1) {d=352;b=20;}
			 else
			   {
			     d=352;
			     if (P3==1) b=21;
			     else b=14;
			   }
			 setfillstyle(SOLID_FILL,LIGHTBLUE);
			 bar(d,YC1[b],d+11,YC1[b]+6);
			}
		      break;

	   case LEFT:
		      if (d!=17 && control==KEYBOARD)
			{
			 setfillstyle(SOLID_FILL,BLACK);
			 bar(d,YC1[b],d+11,YC1[b]+6);
			 if (b==20) {d=17;b=7;}
			 else if (b==21) {if (P2==0) {d=17;b=7;}
			 else {d=17;b=15;}}
			 else { d=17;b=b-8;}
			 setfillstyle(SOLID_FILL,LIGHTBLUE);
			 bar(d,YC1[b],d+11,YC1[b]+6);
			}
		      break;

	   case ESC:
		      if (control==MOUSE) break;
		      setfillstyle(SOLID_FILL,BLACK);
		      bar(d,YC1[b],d+11,YC1[b]+6);
		      e=0;
		      setfillstyle(SOLID_FILL,LIGHTBLUE);
		      setcolor(YELLOW);
		      if (P2==1)
			{
			  bar(367,310,401,320);
			  outtextxy(369,312,"SAVE");
			}
		      else
			{
			 bar(237,310,271,320);
			 outtextxy(239,312,"SAVE");
			}
		      do
			{
			 c=getch();
			 if (c==0) c=getch();
			 switch(c)
			   {
			    case LEFT:if (e==1)
					{
					 setcolor(YELLOW);
					 setfillstyle(SOLID_FILL,BLACK);
					 if (P2==1)
					   {
					     bar(491,310,541,320);
					     outtextxy(493,312,"CANCEL");
					     setfillstyle(SOLID_FILL,LIGHTBLUE);
					     bar(367,310,401,320);
					     outtextxy(369,312,"SAVE");
					   }
					 else
					   {
					     bar(361,310,411,320);
					     outtextxy(363,312,"CANCEL");
					     setfillstyle(SOLID_FILL,LIGHTBLUE);
					     bar(237,310,271,320);
					     outtextxy(239,312,"SAVE");
					   }
					 e=0;
					}
					break;

			    case RIGHT:if (e==0)
					 {
					  setcolor(YELLOW);
					  setfillstyle(SOLID_FILL,BLACK);
					  if (P2==1)
					    {
					     setfillstyle(SOLID_FILL,LIGHTBLUE);
					    }
					  else
					    {
					     bar(237,310,271,320);
					     outtextxy(239,312,"SAVE");
					     setfillstyle(SOLID_FILL,LIGHTBLUE);
					     bar(361,310,411,320);
					     outtextxy(363,312,"CANCEL");
					    }
					  e=1;
					 }
				       break;

			    case ENTER:
				       if (e==0) {a=30;save_characteristics();}
				       else a=40;
				       sor=1;
				       break;

			    case ESC :
				       a=100;
				       sor=1;
				       setfillstyle(SOLID_FILL,BLACK);
				       if (P2==1)
					 {
					  bar(367,310,401,320);
					  outtextxy(369,312,"SAVE");
					  bar(491,310,541,320);
					  outtextxy(493,312,"CANCEL");
					 }
				       else
					 {
					  bar(237,310,271,320);
					  outtextxy(239,312,"SAVE");
					  bar(361,310,411,320);
					  outtextxy(363,312,"CANCEL");
					 }
				       setfillstyle(SOLID_FILL,LIGHTBLUE);
				       bar(d,YC1[b],d+11,YC1[b]+6);
				       break;
			   }
			}while (sor==0);
		      break;

	   case ENTER:
		      a=b;
		      break;

	  }
      }
    if (control==MOUSE) a=test_m();
    if (a<22) wwindow(a,&insert);
    if (a==30) {save_characteristics();exi=1;}
    else if (a==40) exi=1;
   }
    while(exi==0);
 setviewport(0,0,639,349,0);
 able_param();
 if (control==MOUSE) cursor_counter(2);
 clearviewport();
 init_param();
 init_length();
 if (control==MOUSE) cursor_counter(1);    /* 30 -> save     40 -> cancel */
}

/**************************************************************************/

void conv_config(int nb,int aff)
{
 char s[20];

 strcpy(s," \0");
 switch(nb)
   {case 0:
	   strcpy(s,TYPE);
	   if ((s[strlen(s)-2]=='U') && (s[strlen(s)-1]=='V')) s[strlen(s)-2]='\0';
	   if (strcmp(s,"DDS")==0) TABC[0]=0;
	     else if (strcmp(s,"XY")==0) TABC[0]=1;
	     else if (strcmp(s,"OMARS-89")==0) TABC[0]=2;
	     else if (strcmp(s,"Z-24")==0) TABC[0]=3;
	   break;

    case 1:
	   if ((TYPE[strlen(TYPE)-2]=='U') && (TYPE[strlen(TYPE)-1]=='V'))
	     { strcpy(s,"UV");TABC[1]=1;}
	   else
	     { strcpy(s,"VISIBLE");TABC[1]=0;}
	   break;

    case 2:strcpy(s,GRATING);
	   TABC[2]=(atoi(GRATING) / 300) / 2;
	   break;

    case 6:strcpy(s,PLOTTER_PORT);
	   if (strcmp(s,"COM1")==0) TABC[3]=0;
	   else
	     {
	      if (strcmp(s,"COM2")==0) TABC[3]=1;
	      else
		{
		 if (strcmp(s,"LPT1")==0) TABC[3]=2;
		 else TABC[3]=3;
		}
	     }
	   break;

    case 7: if (strcmp(PLOTTER_TYPE,"HPP")==0)
	      {strcpy(s,"HP PLOTTER");}
	    else
	      {strcpy(s,"LASER HP EMULATION");}
	    break;

    case 8: if (FORE_MODU) strcpy(s,"YES");
	    else strcpy(s,"NO");
	    TABC[8]=FORE_MODU;
	    break;

    case 9: if (SPEC_MODU) strcpy(s,"YES");
	    else strcpy(s,"NO");
	    TABC[9]=SPEC_MODU;
	    break;

    case 10: if (PHOT_COUNT_MODU) strcpy(s,"YES");
	    else strcpy(s,"NO");
	    TABC[10]=PHOT_COUNT_MODU;
	    break;

   case 11: if (PULSOR_MODU) strcpy(s,"YES");
	    else strcpy(s,"NO");
	    TABC[11]=PULSOR_MODU;
	    break;


   case 12: if (MICRO_PLATE_MODU) strcpy(s,"YES");
	    else strcpy(s,"NO");
	    TABC[12]=MICRO_PLATE_MODU;
	    break;

   case 13: if (MULTI_MODU) strcpy(s,"YES");
	    else strcpy(s,"NO");
	    TABC[13]=MULTI_MODU;
	    break;

   case 14: if (CCD_MODU) strcpy(s,"YES");
	    else strcpy(s,"NO");
	    TABC[14]=CCD_MODU;
	    break;

   case 15:if (PDA_TYPE==0) strcpy(s,"STANDARD");
	   if (PDA_TYPE==1) strcpy(s,"MICRO-P");
	   if (PDA_TYPE==2) strcpy(s,"SR");
	   TABC[15]=PDA_TYPE;
	   break;

   case 16:if (DAC[0]==0) strcpy(s,"12 BITS");
	   else strcpy(s,"14 BITS");
	   TABC[16]=DAC[0];
	   break;

   case 17:sprintf(s,"%4d",NBR_PIXEL[0]);  /*MTC 23/08/91  (I.R)*/
	   TABC[17]=NBR_PIXEL[0];
	   break;

   case 20:if (DOUBLER==1) strcpy(s,"YES");
	   else strcpy(s,"NO");
	   TABC[20]=DOUBLER;
	   break;

   case 21 :if (strcmp(CCD_TYPE,"OMA3")==0) {strcpy(s,"OMA 3");TABC[21]=0;}
	    else {strcpy(s,"OMA4");TABC[21]=1;}
	    break;

   case 3:
	   strcpy(s,ANGLE);
	   break;

   case 4:
	   strcpy(s,FOCAL);
	   break;

   case 5:
	   strcpy(s,HIGH_DISP_COEFF);
	   break;

   case 18:
	    sprintf(s,"%#04d-%#04d",atoi(UTIL_DIOD_BEG[0]),atoi(UTIL_DIOD_END[0]));
	    break;
  }
if (aff==1)
  {
   if (control==MOUSE) cursor_counter(2);
   setcolor(LIGHTGREEN);
   setfillstyle(SOLID_FILL,BLACK);
   if (nb==7) bar(XC[nb]-1,YC1[nb]-2,XC[nb]+170,YC1[nb]+8);
   if (nb>7 && nb<15) bar(XC[nb]-1,YC1[nb]-2,XC[nb]+30,YC1[nb]+8);
   else bar(XC[nb]-1,YC1[nb]-2,XC[nb]+67,YC1[nb]+8);
   outtextxy(XC[nb],YC1[nb],s);
   if (control==MOUSE) cursor_counter(1);
  }
}
/***************************************************************************/

void wwindow(int nb,int *insert)
{
 int d,i,ques=0;

 if ((nb>7 && nb<15) || (nb==20) || (nb==21)) d=352;
 else d=17;

 if (control==MOUSE) cursor_counter(2);
 setfillstyle(SOLID_FILL,LIGHTBLUE);
 bar(d,YC1[nb],d+11,YC1[nb]+6);
 if (control==MOUSE) cursor_counter(1);

 switch(nb)
   {
    case 0:strcpy(ST[0],"DDS");
	   strcpy(ST[1],"XY");
	   strcpy(ST[2],"OMARS-89");
	   strcpy(ST[3],"Z-24");
	   op_window(nb,27,33,78,4);
	   break;

    case 1:strcpy(ST[0],"VISIBLE");
	   strcpy(ST[1],"UV");
	   op_window(nb,14,20,70,2);
	   break;

    case 2:strcpy(ST[0],"300");
	   strcpy(ST[1],"600");
	   strcpy(ST[2],"1200");
	   strcpy(ST[3],"1800");
	   strcpy(ST[4],"2400");
	   op_window(nb,34,39,46,5);
	   break;

    case 6:strcpy(ST[0],"COM1");
	   strcpy(ST[1],"COM2");
	   strcpy(ST[2],"LPT1");
	   strcpy(ST[3],"LPT2");
	   op_window(nb,27,33,46,4);
	   break;

    case 7:strcpy(ST[0],"HP PLOTTER");
	   strcpy(ST[1],"LASER HP EMULATION");
	   strcpy(ST[2],"HP LASERJET 3");
	   op_window(nb,14,32,158,3);
	   break;

    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 20:
	    strcpy(ST[0],"NO");
	    strcpy(ST[1],"YES");
	    op_window(nb,14,20,38,2);
	    if ((MICRO_PLATE_MODU==1) && (nb==12)) ques=1;
	    if (MULTI_MODU==1 && nb==13 && P2==0) page2();
	    if (MULTI_MODU==0 && nb==13 && P2==1) page2();
	    if (CCD_MODU==1 && nb==14 && P3==0) page3();
	    if (CCD_MODU==0 && nb==14 && P3==1) page3();
	    break;

    case 21:strcpy(ST[0],"OMA 3");
	    strcpy(ST[1],"OMA 4");
	    op_window(nb,14,20,54,2);
	    break;

    case 15:strcpy(ST[0],"STANDARD");
	    strcpy(ST[1],"MICRO-P.");
	    strcpy(ST[2],"SR");
	    op_window(nb,21,27,78,3);
	    break;

    case 16:strcpy(ST[0],"12 BITS");
	    strcpy(ST[1],"14 BITS");
	    op_window(nb,14,20,70,2);
	    break;

    case 17:
	    sprintf(ST[0],"%4d",NBR_PIXEL[0]);  /* MTC 23/08/91  (I.R)*/
	    MASKEDIT=2;edit_string(4,206,302,ST[0],insert);MASKEDIT=0;
	    setviewport(0,0,639,349,0);
	    NBR_PIXEL[0]=atoi(ST[0]);
	    break;

    case 3:
	   strcpy(ST[0],ANGLE);
	   MASKEDIT=2;edit_string(6,106,127,ST[0],insert);MASKEDIT=0;
	   setviewport(0,0,639,349,0);
	   strcpy(ANGLE,ST[0]);
	   break;

    case 4:
	   strcpy(ST[0],FOCAL);
	   MASKEDIT=2;edit_string(6,106,152,ST[0],insert);MASKEDIT=0;
	   setviewport(0,0,639,349,0);
	   strcpy(FOCAL,ST[0]);
	   break;

    case 5:
	   strcpy(ST[0],HIGH_DISP_COEFF);
	   MASKEDIT=2;edit_string(5,280,177,ST[0],insert);MASKEDIT=0;
	   setviewport(0,0,639,349,0);
	   strcpy(HIGH_DISP_COEFF,ST[0]);
	   break;

   case 18:
	   do
	     {
	      sprintf(ST[0],"%#04d-%#04d",atoi(UTIL_DIOD_BEG[0]),atoi(UTIL_DIOD_END[0]));
	      MASKEDIT=2;edit_string(9,200,327,ST[0],insert);MASKEDIT=0;
	      setviewport(0,0,639,349,0);
	      if (ST[0][4]!='-') {printf("\a");}
	     }
	      while (ST[0][4]!='-');
	   for (i=0;i<4;i++) UTIL_DIOD_BEG[0][i]=ST[0][i];
	   UTIL_DIOD_BEG[0][4]='\0';
	   for (i=5;i<9;i++) UTIL_DIOD_END[0][i-5]=ST[0][i];
	   UTIL_DIOD_BEG[1][4]='\0';
	   break;

   default:break;
   }

if (ques==1)
  {
   strcpy(ST[0],"COM1");
   strcpy(ST[1],"COM2");
   op_window(19,14,20,46,2);
  }

if (control==MOUSE) { cursor_counter(2);
		      setfillstyle(SOLID_FILL,BLACK);
		      bar(d,YC1[nb],d+11,YC1[nb]+6);
		      cursor_counter(1);
		    }
}
/**************************************************************************/
void op_window(int nb,int up,int dow,int l,int ch)
{
 int som,i,exi=0,r,s,sauv;
 char c;

 som=up+dow;
 setviewport(0,0,639,349,0);
 if (control==MOUSE) cursor_counter(2);
 getimage(XC[nb]-8,YC1[nb]-up,XC[nb]+l-8,YC1[nb]+dow,buffer_image);
 setviewport(XC[nb]-8,YC1[nb]-up,XC[nb]+l-8,YC1[nb]+dow,0);
 clearviewport();
 setcolor(LIGHTBLUE);
 rectangle(0,0,l,som);
 rectangle(2,2,l-2,som-2);
 for (i=0;i<ch;i++)
    {
     setfillstyle(SOLID_FILL,BROWN);
     setcolor(YELLOW);
     outtextxy(8,(i*13)+8,ST[i]);
     if (TABC[nb]==i) bar(6,(i*13)+6,l-5,(i*13)+16);
     outtextxy(8,(i*13)+8,ST[i]);
    }
 if (control==MOUSE)  cursor_counter(1);
 sauv=TABC[nb];

 if (control==KEYBOARD)
   {
     do
       {
	if (kbhit())
	  {
	   c=getch();
	   if (c==0) c=getch();
	   switch (c)
	     {
	      case UP:if (TABC[nb]!=0)
			{
			 setcolor(YELLOW);
			 setfillstyle(SOLID_FILL,BLACK);
			 bar(6,(TABC[nb]*13)+6,l-5,(TABC[nb]*13)+16);
			 outtextxy(8,(TABC[nb]*13)+8,ST[TABC[nb]]);
			 TABC[nb]--;
			 setfillstyle(SOLID_FILL,BROWN);
			 bar(6,(TABC[nb]*13)+6,l-5,(TABC[nb]*13)+16);
			 outtextxy(8,(TABC[nb]*13)+8,ST[TABC[nb]]);
			}
		      break;

	      case DOWN:if (TABC[nb]!=ch-1)
			  {
			   setcolor(YELLOW);
			   setfillstyle(SOLID_FILL,BLACK);
			   bar(6,(TABC[nb]*13)+6,l-5,(TABC[nb]*13)+16);
			   outtextxy(8,(TABC[nb]*13)+8,ST[TABC[nb]]);
			   TABC[nb]++;
			   setfillstyle(SOLID_FILL,BROWN);
			   bar(6,(TABC[nb]*13)+6,l-5,(TABC[nb]*13)+16);
			   outtextxy(8,(TABC[nb]*13)+8,ST[TABC[nb]]);
			  }
			break;

	     case ENTER:
	     case ESC  :
			exi=1;
			break;
	     }
	  }
       }
	while(exi==0);
   }
 else
   {
     exi=0;
     do
       {
	if ((test_press(1))!=0 || test_press(0)!=0) {exi=1;break;}
	if (exi==0)
	  {
	   test_button();
	   r=(int)((YMOUSE+up-YC1[nb])/15);
	   if (XMOUSE>XC[nb]-8 && XMOUSE<XC[nb]+l-8 && YMOUSE>YC1[nb]-up && YMOUSE<YC1[nb]+dow-4)
	     {
	      s=r;
	      if (s!=sauv)
		{
		 cursor_counter(2);
		 setcolor(YELLOW);
		 setfillstyle(SOLID_FILL,BLACK);
		 bar(6,(sauv*13)+6,l-5,(sauv*13)+16);
		 outtextxy(8,(sauv*13)+8,ST[sauv]);
		 setfillstyle(SOLID_FILL,BROWN);
		 bar(6,(s*13)+6,l-5,(s*13)+16);
		 outtextxy(8,(s*13)+8,ST[s]);
		 cursor_counter(1);
		 sauv=s;
		}
	     }
	  }
       }
	while(exi==0);
     TABC[nb]=sauv;
   }
 setviewport(0,0,639,349,0);
 if (control==MOUSE) cursor_counter(2);
 putimage(XC[nb]-8,YC1[nb]-up,buffer_image,0);
 if (control==MOUSE) cursor_counter(1);
 if (nb==19) nb=12;
 modif_conf(nb);
 conv_config(nb,1);
}
/***************************************************************************/
void write_t(int a,int b,int length,int c,char *s)
{
 setfillstyle(SOLID_FILL,LIGHTGRAY);
 bar(a-3,b-2,length*8,c);
 setcolor(LIGHTRED);
 outtextxy(0,0,s);
}
/**************************************************************************/
void page2()
{
 int i;

 if (P2==0) P2=1;
 else P2=0;
 setfillstyle(SOLID_FILL,BLACK);
 bar(10,240,330,337);
 bar(10,270,630,337);

 if (P2==1)
   {
    outline_box(15,250,15,10,WHITE,LIGHTGRAY);
    outline_box(15,275,15,10,WHITE,LIGHTGRAY);
    outline_box(15,300,15,10,WHITE,LIGHTGRAY);
    outline_box(15,325,15,10,WHITE,LIGHTGRAY);
    setcolor(WHITE);
    outtextxy(40,252,"PHOTODIODES TYPE :");
    outtextxy(40,277,"A/D CONVERTER :");
    outtextxy(40,302,"PHOTODIODES ARRAY :");
    outtextxy(40,327,"INTENSIFIED FIELD :");
    for (i=15;i<19;i++) conv_config(i,1);
    setcolor(LIGHTGRAY);
    outtextxy(426,280,"ESC to ");
    outtextxy(442,312,"or");
    setcolor(LIGHTMAGENTA);
    rectangle(350,300,420,330);
    rectangle(480,300,550,330);
    setcolor(YELLOW);
    outtextxy(369,312,"SAVE");
    outtextxy(493,312,"CANCEL");
   }
 else
   {
    setcolor(LIGHTGRAY);
    outtextxy(296,280,"ESC to");
    outtextxy(312,312,"or");
    setcolor(LIGHTMAGENTA);
    rectangle(220,300,290,330);
    rectangle(350,300,420,330);
    setcolor(YELLOW);
    outtextxy(239,312,"SAVE");
    outtextxy(363,312,"CANCEL");
   }
}
/*========================================================================*/
void page3(void)
{
 if (P3==0)
   {
    outline_box(350,225,15,10,WHITE,LIGHTGRAY);
    outline_box(350,250,15,10,WHITE,LIGHTGRAY);
    setcolor(WHITE);
    outtextxy(375,227,"DOUBLER :");
    outtextxy(375,252,"CCD TYPE :");
    conv_config(20,1);
    conv_config(21,1);
    P3=1;
   }
 else
  {
    setfillstyle(SOLID_FILL,BLACK);
    bar(348,220,630,265);
    P3=0;
  }
}
/*========================================================================*/

void able_param(void)

/* Configurates the program by reading the file CONFIG  */
/* This file contains all the configuration of the installed spectrometer. */

{
 int i;

 if (FORE_MODU == 1) enable_param[0]=INSTALLED;
 /*if (SPEC_MODU == 1) enable_param[1]=INSTALLED;*/
 enable_param[1]=INSTALLED;
 if ((enable_param[0]==INSTALLED) && (enable_param[1]==INSTALLED))
     enable_param[2]=INSTALLED;
 if ((MULTI_MODU == 1)||(CCD_MODU==1)) enable_param[3]=INSTALLED;
 if (PULSOR_MODU == 1){ enable_param[4]=1;enable_param[5]=1;}
 if (MICRO_PLATE_MODU == 1)
   { enable_param[6]=enable_param[7]=INSTALLED;enable_param[8]=INSTALLED;}
 enable_param[9]=INSTALLED;
 for (i=10;i<21;i++)
      enable_param[i]=INSTALLED;
 for (i=21;i<22;i++)
      enable_param[i]=UNINSTALLED;
}
/**************************************************************************/
