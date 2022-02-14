/***************************************************************************/
/*                              MAPPING.C                                  */
/***************************************************************************/
/* This file contains all the functions of mapping recording               */
/* with .1æ MARZHAUSER micro plate                                         */
/***************************************************************************/
#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <process.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
#include <dir.h>
#include "acq4.h"
#include "micropla.h"
/*=========================================================================*/
extern int flagjoystick,control,NACC,
	   X1[],Y1[],CUR_DETECT,PDA_TYPE,MASKEDIT,
	   FIRDI,LASTDI,ACQ_MODE,NACC,NS,ND,
	   XMOUSE,YMOUSE,fin,endacq,
	   bckgrd;

extern char xstr[],ystr[],INT_TIME[],
	    EXCLINE[],INTENS_BEGIN[],INTENS_END[],PATH[];

extern long ifinal[];

extern int CCD_MODU,MULTI_MODU, /* simulation */
	   intensite[];
extern FILE *fp; /*simulation */
long int xbegt,xendt,ybegt,yendt;

char Xstep[7]="000010",Ystep[7]="000010",Nbacc[4],
     tab[10][3][20];

int MAPPING,tabldiod[30][3];

/*=========================================================================*/
void creatgrd(float extrem[][2]),
     disp_curline(int i,int sur),
     disp_tab_text(int li,int col,int color,char *ch),
     erase_tab_text(int li,int col),
     joystick(void),
     max_min(long *max,long *min),
     measure(int first,int nbr_meas,float extrem[][2]);

int diode_definition(void),
    define_scan_area(void),
    enter_mapping_param(void),
    duree_manip(int nbpt,int nbline),
    enter_study_param(void),
    mapping(void);

float gravity(int lim1,int lim2),
      numax(int lim1,int lim2),
      netvalue(int lim1,int lim2);
/*=========================================================================*/
int mapping(void)
{
  int nbspecx,nbspecy,nbr_study_param=0,
      scantype=0,
      i,j,
      quit=0,flag_sav,cancel,ok,
      firstmeas,choice,fpi,
      arret=0;

  char ch[7],ch1[7],c;

  long maxl=0,minl=2147483647;

  float zextrem[30][2];

  int kkkk,jj;/*simulation */
  char ccc[10],cch1[3];/*simulation */
  int mtc;

  setviewport(400,287,500,299,0);
  write_text(CYAN,10,2,7,10,LIGHTCYAN,"MAPPING");
  MAPPING=1;

  do
    {
     if (!define_scan_area()) {quit=1;break;}
     else
       {
	if (!enter_mapping_param()) {quit=1;break;}
	else
	  {
	   if (enter_study_param())
	     {
	      nbspecx=(xendt-xbegt)/atol(Xstep);nbspecx++;
	      nbspecy=(yendt-ybegt)/atol(Ystep);nbspecy++;
	      NACC=atoi(Nbacc);
	     }
	   else
	     {quit=1;break;}
	  }
       }
     /* Duree approximative de la manip ok?*/
    }while (duree_manip(nbspecx,nbspecy)==0);

    if (!quit)
      {
	 nbr_study_param=diode_definition();
	 /*printf("nbr_study_param: %d\n",nbr_study_param);*/

	 if (nbr_study_param!=0)
	   {
	    if (!(file_exist("measure.map")))
	      {remove("measure.map");}
	    fpi=open("measure.map",O_CREAT|O_BINARY,S_IREAD|S_IWRITE);
	    write(fpi,&nbspecx,sizeof(int));
	    write(fpi,&nbspecy,sizeof(int));
	    write(fpi,&nbr_study_param,sizeof(int));
	    write(fpi,&scantype,sizeof(int));
	    close(fpi);
	    /*
	    for (mtc=0;mtc<nbr_study_param;mtc++)
	       {
		printf("%2d: %1d    %4d-%4d\n",mtc,tabldiod[mtc][0],tabldiod[mtc][1],tabldiod[mtc][2]);
	       }
	    getch();*/
	   }
	 open_window(3,271,516,349,LIGHTGREEN);
	 flag_sav=1;
	 do
	   {MASKEDIT=0;
	    cancel=edit_param();
	    if (cancel == 0)
	      {
	       if (control==MOUSE) cursor_shape(2);
	       if (!(m3d_space(nbspecx*nbspecy))) /* Verification free space */
		 {ok=free_space_message();
		  if (ok) flag_sav=0;
		  else flag_sav=1;
		 }
	       else
		 {flag_sav=0;}
	      }
	    else
	      {
		flag_sav=0;
	      }
	  }
	   while (flag_sav==1);
	 save_m3d(nbspecx,nbspecy);
	 close_window(3,271);

	/* If OK */
	/* Int.time  mode setting */
	/*
	if ((CCD_MODU==UNINSTALLED) && (MULTI_MODU==UNINSTALLED) ) {;}
	else
	  {set_time(INT_TIME);}
	setcolor(WHITE);
	outtextxy(X1[3]+17,Y1[3]+18,"       ");
	outtextxy(X1[3]+17,Y1[3]+18,INT_TIME);
	*/
	/* Microplate positionning at the begining*/
	sprintf(ch,"% 06ld",xbegt);
	sprintf(ch1,"% 06ld",ybegt);
	move_plate_to(ch,ch1);

	/* Acquisition */
	/*
	switch(ACQ_MODE)
	  {
	   case DARK:  NS=0;
		       ND=1;
		       break;

	   case SIGNAL:NS=1;
		       ND=0;
		       break;

	   case S_D:   NS=1;
		       ND=1;
		       break;
	  }
	*/

	open_window(3,303,516,349,LIGHTGREEN);
	kkkk=0;
	for (i=0;i<nbspecy;i++)
	  {
	   for (j=0;j<nbspecx;j++)
	      {
	       /*Accumulation/Save */
	       if ((CCD_MODU==UNINSTALLED) && (MULTI_MODU==UNINSTALLED))
		 { /* simulation */
		  sprintf(cch1,"%02d",kkkk);cch1[2]='\0';
		  strcpy(ccc,"neon");
		  strcat(ccc,cch1);
		  fp=fopen(ccc,"rt+");
		  for (jj=0;jj<512;jj++)
		     {
		      fscanf(fp,"%d\n",&ifinal[jj]);
		     }
		  fclose(fp);
		  quick_disp(0,0);
		  spacc_disp(1,0,MS0);
		  kkkk++;
		 }
	       else
		 {
		  arret=oma4_accumul(MS0);
		 }
	      if (!arret)
		{
		 /*Save result in temporary file*/
		 max_min(&maxl,&minl);  /*MAX and minimum */
		 save_m3d_data();
		 if ((i==0) && (j==0)) firstmeas=1;
		 else firstmeas=0;

		 if (nbr_study_param!=0)
		   {
		    measure(firstmeas,nbr_study_param,zextrem);
		   }
		/*Next position on x line */
		if (j!=nbspecx-1) move_plate_rel(Xstep,"000000");
	       }
	     else return(arret);
	     }
	 if (!arret)
	   {
	    /*Positionning at the beginning of the next line */
	    sprintf(ch,"% 06ld",(1-nbspecx)*atol(Xstep));
	    if (i!=nbspecy-1) move_plate_rel(ch,Ystep);
	   }
	 else break;
	}
     save_m3d_maxmin(&maxl,&minl);

     if (nbr_study_param!=0)
       {
	creatgrd(zextrem);
	choice=m3d_surfer();
	close_window(3,303);
	switch(choice)
	   {
	     case 1: endacq=1;fin=1;
		     /*closegraph();
		     execl("dilsurf.exe","dilsurf.exe",argv[1],argv[2],argv[3],NULL);
		     */
		     break;

	     case 0: endacq=2;fin=1;
		     /*
		     closegraph();
		     execl("m3d.exe","m3d.exe",argv[1],argv[2],argv[3],NULL);
		     */
		     break;

	     case 2: fin=0;
		     break;
	   }
       }
     else
       {
	close_window(3,303);
	fin=1;endacq=2;
	/*
	closegraph();
	execl("m3d.exe","m3d.exe",argv[1],argv[2],argv[3],NULL);*/
       }
    }
 return(arret);
}
/***************************************************************************/
int define_scan_area(void)
{
 struct viewporttype viewinfo;

 int cx,cy,xasp,yasp;
 int select=0,annul=0,x[4],y[4],i,ind=0,x0,y0,
     insert=0;
 char c,cc,ch[35],
      *larg[4]={"000010","000010","000010","000010"};

 if (control==MOUSE) cursor_counter(2);
 setviewport(110,40,400,220,0);
 clearviewport();
 getviewsettings(&viewinfo);
 setcolor(LIGHTGREEN);
 rectangle(0,0,viewinfo.right-viewinfo.left,viewinfo.bottom-viewinfo.top);
 rectangle(2,2,viewinfo.right-viewinfo.left-2,viewinfo.bottom-viewinfo.top-2);

 setviewport(0,0,639,349,0);
 strcpy(ch,"Scan area definition");
 cx=viewinfo.left+(((viewinfo.right-viewinfo.left)-8*strlen(ch))/2);
 cy=37;
 write_text(LIGHTCYAN,cx,cy,strlen(ch),10,LIGHTBLUE,ch);

 setviewport(viewinfo.left,viewinfo.top,viewinfo.right,viewinfo.bottom,0);
 setcolor(YELLOW);
 strcpy(ch,"Press SPACE to valid,ESC to quit");
 cx=((viewinfo.right-viewinfo.left)-8*strlen(ch))/2;
 cy=(viewinfo.bottom-viewinfo.top)-11;
 outtextxy(cx,cy,ch);

 cx=(viewinfo.right-viewinfo.left)/2;
 cy=(viewinfo.bottom-viewinfo.top)/2;
 setfillstyle(SOLID_FILL,YELLOW);
 setcolor(YELLOW);
 sector(cx,cy,0,360,10,5);

 setcolor(WHITE);
 line(cx-15,cy,cx-95,cy);line(cx-90,cy-5,cx-95,cy);line(cx-90,cy+5,cx-95,cy);
 line(cx+15,cy,cx+95,cy);line(cx+90,cy-5,cx+95,cy);line(cx+90,cy+5,cx+95,cy);
 line(cx,cy+5,cx,cy+70);line(cx-5,cy+65,cx,cy+70);line(cx+5,cy+65,cx,cy+70);
 line(cx,cy-5,cx,cy-70);line(cx-5,cy-65,cx,cy-70);line(cx+5,cy-65,cx,cy-70);

 x[0]=cx-95+((80-8*strlen(larg[0]))/2);
 x[1]=((viewinfo.right-viewinfo.left)-8*strlen(larg[1]))/2;
 x[2]=cx+15+((80-8*strlen(larg[2]))/2);
 x[3]=((viewinfo.right-viewinfo.left)-8*strlen(larg[3]))/2;

 setcolor(LIGHTRED);
 y[0]=cy-15;
 y[1]=cy-35;
 y[2]=cy-15;
 y[3]=cy+35;
 for (i=0;i<4;i++)
    {
     outtextxy(x[i],y[i],larg[i]);
    }
 write_text(LIGHTGRAY,x[ind],y[ind],6,10,LIGHTGREEN,larg[ind]);
 select=0;

 do
   {
     if (kbhit)
       {
	c=getch();
	if (c==0) c=getch();
       }

      if (c==ESC) {annul=1;select=1;}
      if (c==SPACE)
	{
	  write_text(BLACK,x[ind],y[ind],6,10,LIGHTRED,larg[ind]);
	  select=1;
	}
      switch(c)
	 {
	  case LEFT  :
		      if (ind!=0)
			{
			 write_text(BLACK,x[ind],y[ind],6,10,LIGHTRED,larg[ind]);
			 ind--;
			 write_text(LIGHTGRAY,x[ind],y[ind],6,10,LIGHTGREEN,larg[ind]);
			}
		      break;

	  case RIGHT :
		      if (ind!=3)
			{
			 write_text(BLACK,x[ind],y[ind],6,10,LIGHTRED,larg[ind]);
			 ind++;
			 write_text(LIGHTGRAY,x[ind],y[ind],6,10,LIGHTGREEN,larg[ind]);
			}
		      break;

	  case ENTER :
		      x0=viewinfo.left;
		      y0=viewinfo.top;
		      edit_string(6,x[ind]+x0,y[ind]+y0,larg[ind],&insert);

		      setviewport(viewinfo.left,viewinfo.top,viewinfo.right,viewinfo.bottom,0);
		      write_text(BLACK,x[ind],y[ind],6,10,LIGHTRED,larg[ind]);
		      if (ind!=3) ind++;
		      else ind=0;
		      write_text(LIGHTGRAY,x[ind],y[ind],6,10,LIGHTGREEN,larg[ind]);
		      break;
	  }
   }
    while (!select);
 if (!annul)
   {
    xbegt=atol(xstr)-atol(larg[0]);
    xendt=atol(xstr)+atol(larg[2]);
    ybegt=atol(ystr)-atol(larg[1]);
    yendt=atol(ystr)+atol(larg[3]);
   }
 if (control==MOUSE)
   {cursor_shape(0);cursor_counter(1);}
 if (annul)
   {
    setviewport(110,30,400,220,0);
    clearviewport();
    return(0);
   }
 else return(1);
}
/***************************************************************************/
int enter_mapping_param(void)
{
 char c,
      *charac[4]={INT_TIME,Xstep,Nbacc,Ystep};

 int ind=0,select=0,action,annul=0,insert=0,
     param_length[]={7,6,3,6},
     x[]={138,410,138,410},
     y[]={4,4,16,16},
     x0=7,y0=307;

 int dum0,dum1;

 sprintf(Nbacc,"%3d",NACC);
 param_length[0]=strlen(INT_TIME);

 setviewport(0,0,639,349,0);
 if (control==MOUSE) cursor_counter(2);
 open_window(3,303,516,349,LIGHTGREEN);

 setviewport(7,307,512,345,0);
 setcolor(YELLOW);
 outtextxy(2,29,"Space or left button to validate,Esc or right button to quit");

 setcolor(WHITE);
 outtextxy(2, 4,"Integ.time (s).:           Step in X  (.1æm)......:");
 outtextxy(2,16,"Accumulations..:           Step in Y  (.1æm)......:");

 setcolor(LIGHTRED);
 outtextxy(138,4,INT_TIME);
 outtextxy(138,16,Nbacc);
 outtextxy(410,16,Ystep);
 outtextxy(410,4,Xstep);

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
		      if (ind==0)
			{
			 dum0=X1[3];dum1=Y1[3];
			 X1[3]=128;Y1[3]=293;
			 if (CUR_DETECT==PDA)
			   {
			    if (PDA_TYPE==0)
			      {edit_integ_time();}
			    else
			      {edit(INT_TIME,6,4,3,0);}
			   }
			 else
			   {
			    edit(INT_TIME,7,5,3,0);
			   }
			 X1[3]=dum0;Y1[3]=dum1;
			}
		      else
			{
			 edit_string(param_length[ind],x[ind]+x0,y[ind]+y0,charac[ind],&insert);
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
 setviewport(110,30,400,220,0);
 clearviewport();
 close_window(3,303);
 if (control==MOUSE)
   {cursor_shape(0);cursor_counter(1);}
 if (annul) return(0);
 else return(1);
}
/**************************************************************************/
int enter_study_param(void)
{
 struct viewporttype viewinfo;
 int cx,cy,i,end_select=0,
     curline,curcol,lin,
     lc[3]={-1,-1,-1},
     dummy0,dummy1,ok;

 char ch[80] ,str[20],c;

 setactivepage(1);
 setviewport(0,0,639,349,0);
 clearviewport();
 setcolor(LIGHTGREEN);
 rectangle(0,0,639,349);
 rectangle(2,2,637,347);
 getviewsettings(&viewinfo);

 strcpy(ch,"SELECTION OF THE PARAMETERS TO STUDY");
 cx=(viewinfo.right-viewinfo.left)-8*strlen(ch);
 cx=viewinfo.left+(cx/2);
 cy=20;
 write_text(LIGHTCYAN,cx,cy,strlen(ch),10,LIGHTBLUE,ch);

 cy=50;
 sprintf(ch,"Spectral range: % 06.1f - % 06.1f",
		    (float)(1e7/atof(EXCLINE)-(1e8/diode_wavelength(FIRDI))),
		    (float)(1e7/atof(EXCLINE)-(1e8/diode_wavelength(LASTDI))));
 cx=(viewinfo.right-viewinfo.left)-8*strlen(ch);
 cx=viewinfo.left+(cx/2);
 write_text(BLACK,cx,cy,strlen(ch),10,WHITE,ch);

 strcpy(ch,"Press SPACE or left button to validate,ESC or right button to quit");
 cx=(viewinfo.right-viewinfo.left)-8*strlen(ch);
 cx=viewinfo.left+(cx/2);
 cy=viewinfo.bottom-textheight(ch)-5;
 write_text(BLACK,cx,cy,strlen(ch),10,YELLOW,ch);

 setviewport(66,77,572,253,0);
 setcolor(WHITE);
 rectangle(0,0,506,176);
 rectangle(2,2,504,174);

 setviewport(69,80,569,250,0);
 getviewsettings(&viewinfo);
 setcolor(LIGHTGRAY);
 line(20,0,20,170);
 line(180,0,180,170);
 line(340,0,340,170);
 line(0,30,viewinfo.right-viewinfo.left,30);

 setviewport(90,80,249,110,0);
 getviewsettings(&viewinfo);
 setfillstyle(SOLID_FILL,LIGHTGRAY);
 bar(1,1,157,28);
 setcolor(LIGHTBLUE);
 strcpy(ch,"Integration");
 cx=((viewinfo.right-viewinfo.left)-(8*strlen(ch)))/2;
 cy=7;
 outtextxy(cx,cy,ch);
 strcpy(ch,"in the range");
 cx=((viewinfo.right-viewinfo.left)-(8*strlen(ch)))/2;
 cy+=textheight(ch);
 outtextxy(cx,cy,ch);

 setviewport(250,80,409,110,0);
 getviewsettings(&viewinfo);
 setfillstyle(SOLID_FILL,LIGHTGRAY);
 bar(1,1,157,28);
 setcolor(LIGHTBLUE);
 strcpy(ch,"Maximum intensity");
 cx=((viewinfo.right-viewinfo.left)-(8*strlen(ch)))/2;
 cy=7;
 outtextxy(cx,cy,ch);
 strcpy(ch,"in the range");
 cx=((viewinfo.right-viewinfo.left)-(8*strlen(ch)))/2;
 cy+=textheight(ch);
 outtextxy(cx,cy,ch);

 setviewport(410,80,569,110,0);
 getviewsettings(&viewinfo);
 setfillstyle(SOLID_FILL,LIGHTGRAY);
 bar(1,1,157,28);
 setcolor(LIGHTBLUE);
 strcpy(ch,"Center of gravity");
 cx=((viewinfo.right-viewinfo.left)-(8*strlen(ch)))/2;
 cy=7;
 outtextxy(cx,cy,ch);
 strcpy(ch,"in the range");
 cx=((viewinfo.right-viewinfo.left)-(8*strlen(ch)))/2;
 cy+=textheight(ch);
 outtextxy(cx,cy,ch);

 setviewport(69,120,89,250,0);
 getviewsettings(&viewinfo);
 cy=2;
 for (i=0;i<10;i++)
    {
     disp_curline(i,0);
    }
 disp_curline(0,1);
 for (curline=0;curline<10;curline++)
    {
     for (curcol=0;curcol<3;curcol++)
	{
	 strcpy(tab[curline][curcol]," xxxx.x /  xxxx.x");
	}
    }
 curline=0;curcol=0;
 disp_tab_text(curline,curcol,YELLOW,tab[curline][curcol]);
 setvisualpage(1);

 do
   {
    if (control==MOUSE)
      {
       if (test_press(0))
	 {end_select=1;ok=1;break;}
       if (test_press(1))
	 {end_select=1;ok=0;break;}
      }
    if (kbhit())
      {
       c=getch();
       if (c==0) c=getch();
       switch(c)
	  {
	   case 115  : disp_curline(curline,0);
		       if (curline<=lc[curcol])
			 {disp_tab_text(curline,curcol,WHITE,tab[curline][curcol]);}
		       erase_tab_text(lc[curcol]+1,curcol);
		       if (curcol>0) curcol--;
		       curline=lc[curcol]+1;
		       disp_curline(curline,1);
		       disp_tab_text(curline,curcol,YELLOW,tab[curline][curcol]);
		       break;

	   case 116  : disp_curline(curline,0);
		       if (curline<=lc[curcol])
			 {disp_tab_text(curline,curcol,WHITE,tab[curline][curcol]);}
		       erase_tab_text(lc[curcol]+1,curcol);
		       if (curcol<2) curcol++;
		       curline=lc[curcol]+1;
		       disp_curline(curline,1);
		       disp_tab_text(curline,curcol,YELLOW,tab[curline][curcol]);
		       break;

	   case UP   : if (curline!=0)
			 {
			  disp_tab_text(curline,curcol,WHITE,tab[curline][curcol]);
			  disp_curline(curline,0);
			  curline--;
			  disp_curline(curline,1);
			  disp_tab_text(curline,curcol,YELLOW,tab[curline][curcol]);
			 }
		       break;

	   case DOWN : if ((curline<=lc[curcol]) && (curline!=9))
			 {
			  disp_tab_text(curline,curcol,WHITE,tab[curline][curcol]);
			  disp_curline(curline,0);
			  curline++;
			  disp_curline(curline,1);
			  disp_tab_text(curline,curcol,YELLOW,tab[curline][curcol]);
			 }
		       break;

	   case ESC  : ok=0;end_select=1;
		       break;

	   case ENTER: MASKEDIT=2;
		       dummy0=X1[0];dummy1=Y1[0];

		       strncpy(str,tab[curline][curcol],7);
		       str[7]='\0';
		       /*edit_string(strlen(str),94+160*curcol,120+(curline-1)*13,str,0);*/
		       X1[0]=(102+160*curcol)-17;Y1[0]=(120+curline*13)-18;
		       edit(str,strlen(str),6,0,1);
		       for (i=0;i<7;i++)
			  {tab[curline][curcol][i]=str[i];}
		       disp_tab_text(curline,curcol,YELLOW,tab[curline][curcol]);
		       for (i=0;i<7;i++)
			  {str[i]=tab[curline][curcol][i+10];}
		       str[7]='\0';
		       /*edit_string(strlen(str),182+(160*curcol),120+(curline-1)*13,str,0);*/
		       X1[0]=(182+160*curcol)-17;Y1[0]=(120+curline*13)-18;
		       edit(str,strlen(str),6,0,1);
		       for (i=0;i<7;i++)
			  {tab[curline][curcol][i+10]=str[i];}
		       tab[curline][curcol][17]='\0';
		       disp_tab_text(curline,curcol,YELLOW,tab[curline][curcol]);
		       if (strcmp(tab[curline][curcol]," xxxx.x /  xxxx.x")!=0)
			 {
			  lc[curcol]=curline;
			  disp_tab_text(curline,curcol,WHITE,tab[curline][curcol]);
			  disp_curline(curline,0);
			  curline++;
			  disp_curline(curline,1);
			  disp_tab_text(curline,curcol,YELLOW,tab[curline][curcol]);
			 }
		       X1[0]=dummy0;Y1[0]=dummy1;
		       break;

	   case SPACE: end_select=1;ok=1;
		       break;
	  }
      }
   }
   while(!end_select);

 setviewport(0,0,639,349,0);
 clearviewport();
 setactivepage(0);
 setvisualpage(0);
 return(ok);
}
/*************************************************************************/
void disp_curline(int i,int sur)
{
 char ch[3];
 int cx,cy;

 setviewport(69,120,89,250,0);
 sprintf(ch,"%2d",i+1);
 cx=(20-(8*strlen(ch)))/2;
 cy=(i)*(textheight(ch)+5);
 if (sur)
   {
    setfillstyle(SOLID_FILL,BLUE);
   }
 else
   {
    setfillstyle(SOLID_FILL,BLACK);
   }
 setcolor(WHITE);
 bar(0,cy,19,cy+textheight(ch));
 outtextxy(cx,cy,ch);
}
/*************************************************************************/
void disp_tab_text(int li,int col,int color,char *ch)
{
 int x0[3]={90,250,410},
     x1,y0=120,y1=250,
     xx,yy;

 x1=x0[col]+160;
 setviewport(x0[col],y0,x1,y1,0);
 xx=((x1-x0[col])-(8*strlen(ch)))/2;
 yy=li*(textheight(ch)+5);
 setcolor(color);
 outtextxy(xx,yy,ch);
}
/*************************************************************************/
void erase_tab_text(int li,int col)
{
 int x0[3]={90,250,410},
     x1,y0=120,y1=250,
     xx,yy;

 x1=x0[col]+160;
 setviewport(x0[col],y0,x1,y1,0);
 setfillstyle(SOLID_FILL,BLACK);
 yy=li*13;
 bar(1,yy+1,158,yy+8);
}
/*************************************************************************/
int duree_manip(int nbpt,int nbline)
{
 struct tm d_manip;
 char ch[60];
 int ok=0;

 d_manip.tm_year=92;
 d_manip.tm_mon=6;
 d_manip.tm_mday=1;
 d_manip.tm_hour=0;
 d_manip.tm_min=0;
 d_manip.tm_sec=nbpt*nbline*NACC*atof(INT_TIME);
 mktime(&d_manip);

 if (control==MOUSE) cursor_counter(2);
 open_window(3,303,516,349,LIGHTGREEN);
 setcolor(WHITE);
 sprintf(ch,"Approximate length of the mapping acquisition: %2dh %2dmn %2ds",d_manip.tm_hour,d_manip.tm_min,d_manip.tm_sec);
 outtextxy(10,8,ch);
 outtextxy(50,30,"Do you agree?");
 setviewport(115,303,516,349,0);
 ok=yes_no();
 close_window(3,303);
 return(ok);
}
/*************************************************************************/
void max_min(long *max,long *min)
{
 int i;

 for (i=atoi(INTENS_BEGIN);i<=atoi(INTENS_END);i++)
    {
     if (ifinal[i]>*max) *max=ifinal[i];
     if (ifinal[i]<*min) *min=ifinal[i];
    }
}
/*************************************************************************/
int diode_definition(void)
{
 int ii,j,paramax,aa;
 float dummy;
 char ch[20];

 strcpy(ch," xxxx.x /  xxxx.x");
 paramax=0;
 for (j=0;j<3;j++)
    {
     ii=0;
     while (strcmp(tab[ii][j],ch)!=0)
	  {
	   tabldiod[paramax][0]=j;
	   dummy=atof(strtok(tab[ii][j],"/"));
	   tabldiod[paramax][1]=diode_position(&dummy);
	   dummy=atof(strtok(NULL,"/"));
	   tabldiod[paramax][2]=diode_position(&dummy);
	   if (tabldiod[paramax][1] > tabldiod[paramax][2])
	     {
	      aa=tabldiod[paramax][1];
	      tabldiod[paramax][1]=tabldiod[paramax][2];
	      tabldiod[paramax][2]=aa;
	     }
	   paramax++;ii++;
	  }
    }
 return (paramax);
}
/***************************************************************************/
float netvalue(int lim1,int lim2)
{
 int i,indbelow=1;
 double	xx[2];
 float dummy,dummy1,dummy2,nv=0.0,
       nulim1,nulim2,
       polyn;

 nulim1=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(lim1));
 nulim2=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(lim2));
 dummy1=(float) ifinal[lim1]/NACC;
 dummy2=(float) ifinal[lim2]/NACC;

 xx[1]=(dummy1-dummy2)/(nulim1-nulim2);
 xx[0]=dummy1-xx[1]*nulim1;

 for (i=lim1+1;i<=lim2;i++)
    {
     polyn=xx[0];
     polyn=polyn+xx[1]*((1e7/atof(EXCLINE))-(1e8/diode_wavelength(i)));
     dummy=(float) ifinal[i]/NACC;
     if ( (dummy>polyn) && (indbelow==1) )
       {indbelow=0;nv=nv+(dummy-polyn)/2;}
     else
       {
	if ( (dummy<polyn) && (indbelow==0) )
	  {indbelow=1;nv=nv-(((float) ifinal[i-1]/NACC)-polyn)/2;}
	else
	  {
	   if (indbelow==0)
	     {nv=nv+dummy-polyn;}
	  }
       }
    }
 if (indbelow==0) nv=nv-(dummy2-polyn)/2;
 return(nv);
}
/***************************************************************************/
float numax(int lim1,int lim2)
{
 int i,dio_max;
 long maxi;
 float meas;

 maxi=ifinal[lim1];dio_max=lim1;
 for (i=lim1+1;i<=lim2;i++)
    {
     if (ifinal[i]>maxi)
       {
	maxi=ifinal[i];
	dio_max=i;
       }
    }
 meas=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(dio_max));
 return (meas);
}
/***************************************************************************/
float gravity(int lim1,int lim2)
{
 float in,cg,fx,dummy,quot;
 int i;

 in=0.0;quot=0.0;cg=0.0;

 for (i=lim1;i<=lim2;i++)
    {
     fx=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(i));
     dummy=(float) ifinal[i]/NACC;
     in=in+(dummy*fx);
     quot=quot+dummy;
    }
 if (quot!=0.0) cg=in/quot;
 return (cg);
}
/***************************************************************************/
void measure(int first,int nbr_meas,float extrem[][2])
{
 int i,fpi;
 float measure;

 fpi=open("measure.map",O_BINARY|O_APPEND|O_WRONLY);
 for (i=0;i<nbr_meas;i++)
    {
      switch(tabldiod[i][0])
	 {
	  case 0: /* Integral */
		  measure=netvalue(tabldiod[i][1],tabldiod[i][2]);
		  break;

	  case 1: /* Frequency of the maximum of intensity */
		  /*   in the selected spectral range.     */
		  measure=numax(tabldiod[i][1],tabldiod[i][2]);
		  break;

	  case 2: /* Center of gravity */
		  measure=gravity(tabldiod[i][1],tabldiod[i][2]);
		  break;
	 }
      write(fpi,&measure,sizeof(float));
      if (first)
	{
	 extrem[i][0]=measure;
	 extrem[i][1]=measure;
	}
      else
	{
	 if (measure<extrem[i][0]) extrem[i][0]=measure;
	 if (measure>extrem[i][1]) extrem[i][1]=measure;
	}
    }
 close(fpi);
}
/***************************************************************************/
void creatgrd(float extrem[][2])
{
 char id[5],filename[13],ch[3],
      drive[MAXDRIVE],dir[MAXDIR],
      name[MAXFILE],ext[MAXEXT],
      s[MAXPATH];

 double xlo,xhi,ylo,yhi,zlo,zhi;
 int i,j,k,fp[30],fpi,
     nx,ny,nbparam,scan_type;
 float it;

 fpi=open("measure.map",O_BINARY|O_RDWR);

 read(fpi,&nx,sizeof(int));
 read(fpi,&ny,sizeof(int));
 read(fpi,&nbparam,sizeof(int));
 read(fpi,&scan_type,sizeof(int));

 strcpy(id,"DSBB");
 fnsplit(PATH,drive,dir,name,ext);

 xlo=(double) (xbegt/10.0);/* conversion in microns*/
 xhi=(double) (xendt/10.0);
 ylo=(double) (ybegt/10.0);
 yhi=(double) (yendt/10.0);

 /* Ouverture des fichiers GRD et ecriture des entetes */
 if (strlen(name) > 6)
   {name[6]='\0';}

 for (i=0;i<nbparam;i++)
    {
     sprintf(ch,"%02d",i+1);
     ch[2]='\0';
     strcpy(filename,name);
     strcat(filename,ch);
     strcpy(ext,".grd");
     fnmerge(s,drive,dir,filename,ext);

     fp[i]=open(s,O_CREAT|O_BINARY,S_IREAD|S_IWRITE);
     write(fp[i],id,4);
     write(fp[i],&nx,sizeof(int));
     write(fp[i],&ny,sizeof(int));

     write(fp[i],&xlo,sizeof(double));
     write(fp[i],&xhi,sizeof(double));
     write(fp[i],&ylo,sizeof(double));
     write(fp[i],&yhi,sizeof(double));

     zlo=(double)extrem[i][0];
     zhi=(double)extrem[i][1];

     write(fp[i],&zlo,sizeof(double));
     write(fp[i],&zhi,sizeof(double));
   }


 for (i=0;i<ny;i++)
    {
     for (j=0;j<nx;j++)
	{
	 for (k=0;k<nbparam;k++)
	    {
	     read(fpi,&it,sizeof(float));
	     write(fp[k],&it,sizeof(float));
	    }
	}
    }
 close(fpi);
 for (i=0;i<nbparam;i++)
    {
     close(fp[i]);
    }
}
/***************************************************************************/
int m3d_surfer(void)
{
  struct viewporttype viewinfo;
  char c,ch[65],*ch1[2]={" M3D ","SURFER"};
  int ll,l0,pos=0,hh,x0,y0,sp,
     i,fond,
     posmouse=0,select=0,cancel=0,suite,kk;

 if (control==MOUSE) cursor_counter(2);

 setviewport(7,307,512,345,0);
 clearviewport();
 getviewsettings(&viewinfo);
 strcpy(ch,"MAPPING RESULTS REPRESENTATION");
 ll=(strlen(ch)+2)*8;
 setfillstyle(SOLID_FILL,LIGHTGRAY);
 l0=(viewinfo.right-viewinfo.left-ll)/2;
 setcolor(RED);
 bar(l0,1,l0+ll,11);
 outtextxy(l0+8,3,ch);

 setcolor(YELLOW);
 outtextxy(2,31,"SPACE or left button to valid,ESC or right button to quit.");

 hh=11;ll=49;y0=18;sp=50;
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
 if (cancel) pos=2;
 return(pos);
}
/***************************************************************************/
