/*=======================================================================*/
/*                            AUTOREC.C                                  */
/*=======================================================================*/
/*                                                                       */
/* This file contains all the functions for the automatic recording mode */
/* Detector : CCD  EG&G	OMA4 1024x256					 */
/*                                                                       */
/*=======================================================================*/

#include <acq4.h>
#include <graphics.h>
#include <conio.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>
#include <string.h>
#include <dos.h>
#include <float.h>
#include <ctype.h>
#include <dir.h>

/*=========================================================================*/

extern float position,rap;

extern char INTENS_BEGIN[],INTENS_END[],EXCLINE[],
	    INT_TIME[],DARK_TIME[],
	    OPERATOR[],SLITWIDTH[],SAMPLE[],LASPOW[],
	    SPECWIDTH[],REMARK[],PATH[];

extern int enable_param[],intensite[],change_comnd,bd,
	   XMOUSE,YMOUSE,control,UNITS,MASKEDIT,
	   DIODE_NUMBER,OFFSET_DIOD,NACC,CUR_DETECT,
	   X1[],Y1[],module,PDA_TYPE,
	   bckgrd,ACCU,ret_accu,
	   ACQ_MODE,NS,ND,
	   CCD_MODEL,shutmode_ind,
	   NOSTD;

extern long ifinal[];
/*=========================================================================*/

char intime[3][8],dark_intime[3][8];
int  acc[3]={1,1,1},ccd_accu_mode[3]={0,0,0};

/*=========================================================================*/
void enter_scan(void)
/* handles the automatic recording */
{
 int npts_ovlap,i=0,n_domain=0,x,
     cut_flag[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
     zone[20],
     j,
     savmode,/*maxi,mini,for oma3 & pda*/
     /*interm,*/dio1,dio2,
     flag_sav=1,
     current_mode,oldx,
     totalpts,step;
 int ii,stop_scan=0,stop_all=0;

 float central_pos[20],
       limit,
       max_domain,min_domain,max,min,interm,
       dummy,stp;

 char ch1[8];
 long maxi,mini;

 dio1=atoi(INTENS_BEGIN);
 dio2=atoi(INTENS_END);

 for (i=0;i<3;i++)
    {
     strcpy(intime[i],INT_TIME);
    }

 if (control==MOUSE) cursor_counter(2);
 setviewport(7,307,512,345,0);
 clearviewport();

 npts_ovlap=enter_ovlap(central_pos,&n_domain,cut_flag,acc,zone);

 if (npts_ovlap>=0)
   {
    /* 'Small spectrum' window presentation */
    setviewport(114,0,512,87,0);
    clearviewport();
    setcolor(LIGHTGREEN);
    rectangle(0,0,398,87);
    rectangle(2,2,396,85);
    setcolor(WHITE);
    line(387,3,387,72);
    line(3,67,387,67);
    line(3,72,387,72);

    setfillstyle(SOLID_FILL,LIGHTRED);

    /* Number of points */
    totalpts=(n_domain*(dio2-dio1+1))-((n_domain-(zone[n_domain-1]+1))*npts_ovlap);
    stp=384.0/(float)(totalpts);

    x=0;
    for (i=0;i<(n_domain-1);i++)
       {
	if (cut_flag[i])
	  {
	   dummy=stp*(float)(dio2-dio1+1-npts_ovlap);
	   oldx=x;
	   x=x+(int)(dummy);
	  }
	else
	  {
	   dummy=stp*(float)(dio2-dio1+1);
	   oldx=x;
	   x=x+(int)(dummy);
	  }
	line(386-x,3,386-x,6);
	line(386-x,63,386-x,71);
	if (!(i%2)) bar(386-x+1,68,386-oldx,71);
       }
     if (n_domain%2) bar(4,68,386-x-1,71);

     setcolor(YELLOW);
     position=central_pos[n_domain-1];
     limit=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_END)));
     sprintf(ch1,"%6.1f",limit);
     outtextxy(3,76,ch1);

     position=central_pos[0];
     limit=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_BEGIN)));
     sprintf(ch1,"%6.1f",limit);
     outtextxy(324,76,ch1);

     if (enable_param[2]==1) module=2;
     else if (enable_param[0]==1) module=0;
     else if (enable_param[1]==1) module=1;

     close_window(3,303);
     current_mode=ACQ_MODE;

    /* Acquisition */
    for (i=0;i<n_domain;i++)
       {
	 if (displ_auto_pos(central_pos[i]))
	   {for(i=dio1;i<dio2;i++)
	       {ifinal[i]=0L;}
	    stop_scan=1;
	   }
	 init_length();

	 if (!stop_scan)
	   {
	    strcpy(INT_TIME,intime[zone[i]]);
	    setviewport(X1[3]+1,Y1[3]+13,X1[3]+80,Y1[3]+28,0);
	    clearviewport();
	    setcolor(WHITE);
	    outtextxy(16,5,INT_TIME);

	    NACC=acc[zone[i]];

	    bckgrd=ccd_accu_mode[zone[i]];
	    if (bckgrd) {strcpy(DARK_TIME,dark_intime[zone[i]]);}
	    set_et();
	    setviewport(0,0,639,349,0);
	    open_window(3,303,516,349,LIGHTGREEN);
	    stop_scan=oma4_accumul(MSU);  /* OMA4 */
	   }

	 if (stop_scan)
	   {
	    stop_all=stop_all_or_continue();
	   }
	 if (!stop_all)
	   {
	   /*Max et min*/
	   for (j=dio1;j<=dio2;j++)
	      {
		if (j==dio1) {
			     max_domain=ifinal[dio1]/(long) NACC;
			     min_domain=ifinal[dio1]/(long) NACC;
			    }
		else
		  {
		    interm=ifinal[j]/(long) NACC;
		    if (max_domain<interm) max_domain=interm;
		    if (min_domain>interm) min_domain=interm;
		  }
	      }

	    if (i==0)
	      {max=max_domain;min=min_domain;}
	    else
	      {
	       if (max<max_domain) max=max_domain;
	       if (min>min_domain) min=min_domain;
	      }
	    maxi=(long)(max);
	    mini=(long)(min);
	    temp_msu(i,&max_domain,&min_domain);
	    ms_disp(cut_flag[i],npts_ovlap,&stp,maxi,mini);/* modif OMA4 mtc */
	    close_window(3,303);
	   }
	 else
	   {close_window(3,303);n_domain=i;break;}
       }
   remove("tmp.msu");

   /*  Saving */
   /*if (!stop_all)
     {*/
      open_window(3,271,516,349,LIGHTGREEN);
      setcolor(WHITE);
      outtextxy(50,9,"Do you want to save the ");
      savmode=saving_mode();
      if (savmode!=-1)
	{
	 do
	  {
	   if (!edit_param())
	     {
	      switch (savmode)
		{
		 case 0:
		       flag_sav=save_window_msu(n_domain,central_pos,zone);
		       break;

		 case 1:
		       flag_sav=save_msu(n_domain,npts_ovlap,central_pos,cut_flag,&max,&min);
		       break;

		 case 2:
		       flag_sav=save_msu(n_domain,npts_ovlap,central_pos,cut_flag,&max,&min);
		       if (flag_sav==0) flag_sav=save_window_msu(n_domain,central_pos,zone);
		       break;
		}
	     }
	   else
	     {
	      flag_sav=0;
	     }
	  }
	   while (flag_sav==1);
	}
      close_window(3,271);
    /* }*/
   erase_spacc();
   init_length();
  }
 setviewport(0,0,639,349,0);
 setcolor(WHITE);
 line(10,0,516,0);
 mouse_xclip(0,639);
 mouse_yclip(0,349);

 set_shutter_mode();
 ACCU=0;ret_accu=1;

 setviewport(400,287,500,299,0);
 write_text(CYAN,10,2,6,10,LIGHTCYAN,"ADJUST");
 setviewport(4,1,515,265,1);
 clearviewport();
 if (control==MOUSE)
   {cursor_shape(0);set_curs_pos(258,130);cursor_counter(1);}
}

/******************************************************************************/
int enter_ovlap(float *central_pos,int *n_domain,int *cut_flag,int *acc,int *zone)
{
 int npts_ovlap,i,nzones=0,rep=1,
     another,param_ok,lo_lim_ok,
     cancel=0,insert=0,
     dum0,dum1;

 float borne1;

 char lo_lim[7]="\0",up_lim[7]="\0",
      lim[7]="\0",
      ovlap_str[3]="\0",
      string[10]="\0";

 /* Enter the number of points for overlap */
 /*========================================*/
 if (control==MOUSE) cursor_counter(2);
 outtextxy(20,9,"Enter the number of overlap points :  n =");
 MASKEDIT=1;
 edit_string(3,370,316,ovlap_str,&insert);
 MASKEDIT=0;
 npts_ovlap=atoi(ovlap_str);  /* npts_ovlap on each spectrum */

 /* Parameters for each zone */
 /* ======================== */
 /* The operator enters frequency limits,                                  */
 /*        		integration time and                               */
 /*     		number of accumulations for each zone              */
 /* The computer calculates the number of windows and the central position */
 /* for each spectral window.                                              */
 /* ====================================================================== */
 if (control==MOUSE)  cursor_counter(1);
 do
   {
    rep=1;
    while (rep==1)
	 {
	  /* Enter the frequency limits */
	  param_ok=0;
	  while (!param_ok)
	       {
		lo_lim_ok=0;
		while (!lo_lim_ok)
		{
		setviewport(7,307,512,345,0);
		clearviewport();
		setcolor(WHITE);
		outtextxy(17,6,"Enter the lower frequency of the spectrum  :");
		MASKEDIT=2;
		edit_string(6,400,313,lo_lim,&insert);
		MASKEDIT=0;
		if ((nzones>0) && (atof(lo_lim)<=atof(lim)))
		  {
		    setviewport(7,307,512,345,0);
		    setfillstyle(SOLID_FILL,RED);
		    if (control==MOUSE) cursor_counter(2);
		    bar(100,28,410,38);
		    setcolor(YELLOW);
		    outtextxy(112,29," The lower limit must be > ");
		    outtextxy(324,29,lim);
		    if (control==MOUSE) cursor_counter(1);
		    printf("\a");
		    if (control==MOUSE)
		      {
			cursor_shape(3);
			set_curs_pos(100,330);
			cursor_counter(1);
			test_press(0);
		      }
		    cancel=0;
		    while ((!cancel))
			 {
			  if (control==MOUSE)
			    {
			     if (test_press(0)!=0)
			       {cancel=1;cursor_counter(2);}
			    }
			  if (kbhit()) {cancel=1;getch();}
			 }
			 cursor_counter(2);
		    setfillstyle(SOLID_FILL,BLACK);
		    bar(100,28,410,38);
		    cursor_counter(1);
		  }
		else
		  {
		   lo_lim_ok=1;
		  }
		}
		setcolor(WHITE);
		setviewport(7,307,512,345,0);
		if (control==MOUSE) cursor_counter(2);
		outtextxy(17,19,"Enter the upper frequency of the spectrum  :");
		if (control==MOUSE) cursor_counter(1);
		MASKEDIT=2;
		edit_string(6,400,326,up_lim,&insert);
		MASKEDIT=0;
		if (atof(up_lim)<=atof(lo_lim))
		  {
		    setviewport(7,307,512,345,0);
		    if (control==MOUSE) cursor_counter(2);
		    setfillstyle(SOLID_FILL,RED);
		    bar(170,28,340,38);
		    setcolor(YELLOW);
		    outtextxy(176,30," BAD LIMITS VALUE ! ");
		    if (control==MOUSE) cursor_counter(1);
		    cancel=0;
		    while (!cancel)
			 {
			  if (control==MOUSE)
			    {
			     if (test_press(0)!=0)
			       {
				cursor_counter(2);
				cancel=1;
			       }
			    }
			  if (kbhit()) {cancel=1;getch();}
			 }
		    setfillstyle(SOLID_FILL,BLACK);
		    bar(170,28,340,38);
		  }
		else
		  {
		    param_ok=1;
		  }
	       }

	  /* Calcul of the central position */
	  borne1=atof(lo_lim);
	  i=0;
	  while ((borne1 < atof(up_lim)) && (i < 10))
	       {
		central_pos[*n_domain+i]=central_length(atoi(INTENS_BEGIN),&borne1);
		zone[*n_domain+i]=nzones;
		position=central_pos[*n_domain+i++];
		borne1=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_END)-npts_ovlap));
	       }

	  /* Enter integration time and number of accumulations */
	  setviewport(7,307,512,345,0);
	  clearviewport();

	  setcolor(WHITE);
	  outtextxy(17,6,"Enter integration time for this zone :");
	  dum0=X1[3];dum1=Y1[3];
	  X1[3]=350;Y1[3]=295;


	  edit(intime[nzones],7,5,3,0);

	  X1[3]=dum0;Y1[3]=dum1;



	      enter_ccd_accumul();
	      acc[nzones]=NACC;
	      ccd_accu_mode[nzones]=bckgrd;
	      strcpy(dark_intime[nzones],DARK_TIME);


	  /*Select accumulation mode : User or current*/

	  close_window(3,303);

	  /* Display number of windows and total time for the zone */
	  open_window(3,280,516,349,LIGHTGREEN);
	  setviewport(7,284,512,345,0);
	  clearviewport();
	  setcolor(WHITE);
	  outtextxy(17,3,"Number of windows :        Upper limit :");
	  outtextxy(17,16,"Total time:   h   mn   s ");
	  itoa(i,string,10);
	  setcolor(LIGHTRED);
	  outtextxy(177,3,string);
	  sprintf(string,"%7.2f",borne1);
	  outtextxy(353,3,string);

	  /* ok reenter quit ? */
	  rep=param_agree();
	  close_window(3,280);
	  if (rep!=2) open_window(3,303,516,349,LIGHTGREEN);
	 }
   if (rep==0)
     {
      *n_domain+=i;
      cut_flag[*n_domain-1]=1;
      if (++nzones <= 2)
	{
	 setcolor(WHITE);
	 outtextxy(17,6,"Do you wish to record another spectral zone ?");
	 another=yes_no();
	 if (another==YES && control==MOUSE) cursor_counter(2);
	 sleep(1);
	 strcpy(lim,up_lim);
	}
     }
  else
     {
      npts_ovlap=-1;
      another=NO;
     }
  } while ((another==YES) && (nzones < 3));
 return(npts_ovlap);
}

/***************************************************************************/
int displ_auto_pos(float pos)

/* Positionning modules at pos position in automatic recording mode */
{
 char ch1[8],ch2[40];
 int shutmode_bck,stp;

 if (module !=2)
   {
    setviewport(X1[module]+1,Y1[module]+13,X1[module]+72,Y1[module]+28,0);
    clearviewport();
   }
 else
   {
    setviewport(X1[0]+1,Y1[0]+13,X1[0]+72,Y1[0]+28,0);
    clearviewport();
    setviewport(X1[1]+1,Y1[1]+13,X1[1]+72,Y1[1]+28,0);
    clearviewport();
   }

  /* Setting shutter closed during positionning */
  shutmode_bck=shutmode_ind;
  shutmode_ind=CLOSED;
  /*set_shutter_mode();*/
  shutmode_ind=shutmode_bck;

  setviewport(0,0,639,349,0);
  open_window(134,150,384,200,LIGHTRED);
  setcolor(YELLOW);
  strcpy(ch2,"Positionning to ");
  sprintf(ch1,"%7.1f",pos);
  strcat(ch2,ch1);
  outtextxy(20,25,ch2);
  if (NOSTD)
    {
     switch(module)
	{
	 case FOREMONO: stp=positionning(&pos,&module);
			break;
	 case SPECTRO:  pos=((long)(pos/rap*100))/100.0;
			stp=positionning(&pos,&module);
			break;
	 case TRIPLE:   module=FOREMONO;stp=positionning(&pos,&module);
			pos=((long)(pos/rap*100))/100.0;module=SPECTRO;
			stp=positionning(&pos,&module);
			break;
	}
    }
  else
    {
     if (module==2) stp=bpositionning(&pos);/*sleep(2);*/
     else stp=positionning(&pos,&module);/*sleep(2);*/
    }
  close_window(134,150);
  setcolor(WHITE);
  if (module!=2)
    {
      setviewport(X1[module]+1,Y1[module]+13,X1[module]+72,Y1[module]+28,0);
      outtextxy(16,5,ch1);
    }
  else
    {
      setviewport(X1[0]+1,Y1[0]+13,X1[0]+72,Y1[0]+28,0);
      outtextxy(16,5,ch1);
      setviewport(X1[1]+1,Y1[1]+13,X1[1]+72,Y1[1]+28,0);
      outtextxy(16,5,ch1);
    }
 return stp;
}

/***************************************************************************/
void displ_auto_lim(float pos)
{
 float lim1;
 char ch1[8];
 int dio1;

 position=pos;
 switch (UNITS)
       {
	case CM_1: lim1=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_END)));
		   sprintf(ch1,"%5.0f",lim1);
		   break;

	case DIO : dio1=DIODE_NUMBER-OFFSET_DIOD-atoi(INTENS_END);
		   itoa(dio1,ch1,10);
		   break;
       }

 setviewport(3,267,73,280,0);
 setcolor(WHITE);
 line(0,0,0,13);
 line(1,13,70,13);
 line(70,13,70,0);
 setfillstyle(SOLID_FILL,BLUE);
 bar(1,0,69,12);
 setcolor(YELLOW);
 outtextxy(5,3,ch1);

 switch (UNITS)
       {
	case CM_1: lim1=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_BEGIN)));
		   sprintf(ch1,"%5.0f",lim1);
		   break;

	case DIO : dio1=DIODE_NUMBER-OFFSET_DIOD-atoi(INTENS_BEGIN);
		   itoa(dio1,ch1,10);
		   break;
       }
 setviewport(446,267,516,280,0);
 setcolor(WHITE);
 line(0,0,0,13);
 line(1,13,70,13);
 line(70,13,70,0);
 setfillstyle(SOLID_FILL,BLUE);
 bar(1,0,69,12);
 setcolor(YELLOW);
 outtextxy(5,3,ch1);
}

/***************************************************************************/

int param_agree(void)
/* Dialog box OK Reenter Cancel */

{
 int ii,deb,index=0,posmouse=0,
     cancel=0,select=0,
     x0=6,y0=289;

 char c;

 for (ii=0;ii<3;ii++)
    {
      deb=75*(ii+1)+(70*ii);
      outline_box(deb,34,70,15,YELLOW,LIGHTGRAY);
      agree_text(deb+5,39,ii);
    }

 deb=75*(index+1)+(70*index);
 displ_box(deb+1,35,deb+69,48,LIGHTBLUE);
 agree_text(deb+5,39,index);

 if (control==MOUSE)
   {
    mouse_xclip(7,512);
    mouse_yclip(284,345);
    set_curs_pos(x0+deb+20,y0+39);
    cursor_shape(0);
   }
 while ((!cancel) && (!select))
      {
       if (control==MOUSE)
	 {
	  cursor_counter(1);
	  test_press(0);
	  while ((!posmouse) && (!kbhit()))
	       {
                if (test_press(0)!=0)
		  {
		    for (ii=0;ii<3;ii++)
		       {
			deb=x0+(75*(ii+1)+70*ii);
			if ((XMOUSE>deb) && (XMOUSE<deb+70) && (YMOUSE>324) && (YMOUSE<337))
			  {
			   cursor_counter(2);
			   setfillstyle(SOLID_FILL,BLACK);
			   deb=75*(index+1)+70*index;
			   bar(deb+1,35,deb+69,48);
			   agree_text(deb+5,39,index);
			   index=ii;
			   setfillstyle(SOLID_FILL,LIGHTBLUE);
			   deb=75*(index+1)+70*index;
			   bar(deb+1,35,deb+69,48);
			   agree_text(deb+5,39,index);
			   posmouse=1;select=1;
			  }
		       }
		   }
	       }
	 }
       if (!select)
	 {
	  c=getch();
	  if (control==MOUSE) cursor_counter(2);
	  if (c==0) c=getch();
	  switch (c)
	     {
	      case LEFT:
			 deb=75*(index+1)+70*index;
			 displ_box(deb+1,35,deb+69,48,BLACK);
			 agree_text(deb+5,39,index);
			 if (index==0) index=2;
			 else index--;
			 deb=75*(index+1)+70*index;
			 displ_box(deb+1,35,deb+69,48,LIGHTBLUE);
			 agree_text(deb+5,39,index);
			 break;

	      case RIGHT:
			 deb=75*(index+1)+70*index;
			 displ_box(deb+1,35,deb+69,48,BLACK);
			 agree_text(deb+5,39,index);
			 if (index==2) index=0;
			 else index++;
			 deb=75*(index+1)+70*index;
			 displ_box(deb+1,35,deb+69,48,LIGHTBLUE);
			 agree_text(deb+5,39,index);
			 break;

	      case 'o':
	      case 'O':
			 if (index!=0)
			   {
			    deb=75*(index+1)+70*index;
			    displ_box(deb+1,35,deb+69,48,BLACK);
			    agree_text(deb+5,39,index);
			    index=0;
			    deb=75*(index+1)+70*index;
			    displ_box(deb+1,35,deb+69,48,LIGHTBLUE);
			    agree_text(deb+5,39,index);
			   }
			 break;

	      case 'r':
	      case 'R':
			 if (index!=1)
			   {
			    deb=75*(index+1)+70*index;
			    displ_box(deb+1,35,deb+69,48,BLACK);
			    agree_text(deb+5,39,index);
			    index=1;
			    deb=75*(index+1)+70*index;
			    displ_box(deb+1,35,deb+69,48,LIGHTBLUE);
			    agree_text(deb+5,39,index);
			   }
			 break;

	      case 'q':
	      case 'Q':
			if (index!=2)
			  {
			    deb=75*(index+1)+70*index;
			    displ_box(deb+1,35,deb+69,48,BLACK);
			    agree_text(deb+5,39,index);
			    index=2;
			    deb=75*(index+1)+70*index;
			    displ_box(deb+1,35,deb+69,48,LIGHTBLUE);
			    agree_text(deb+5,39,index);
			  }
			 break;

	      case ENTER:if (index==1 && control==MOUSE) cursor_counter(2);
			 select=1;
			 break;
	     }
	 }
      }
 return(index);
}

/**************************************************************************/

void agree_text(int x,int y,int ind)
{
  char *text[3]={"Ok","Reenter"," Quit "},c[10];
  int ii,l;

  setcolor(LIGHTRED);
  strcpy(c,text[ind]);
  c[1]='\0';
  outtextxy(x,y,c);

  setcolor(WHITE);
  l=strlen(text[ind]);
  for (ii=0;ii<l;ii++)
     {
      c[ii]=text[ind][ii+1];
     }
  c[l]='\0';
  outtextxy(x+8,y,c);
}

/**************************************************************************/

void yes_no_text(int x,int y,int ind)
{
  char *text[2]={"No","Yes"},c[5];
  int ii,l;

  setcolor(LIGHTRED);
  strcpy(c,text[ind]);
  c[1]='\0';
  outtextxy(x,y,c);

  setcolor(WHITE);
  l=strlen(text[ind]);
  for (ii=0;ii<l;ii++)
     {
      c[ii]=text[ind][ii+1];
     }
  c[l]='\0';
  outtextxy(x+8,y,c);
}

/**************************************************************************/

int yes_no(void)
{
 int deb,ii,index=0,
     x0,y0,posmouse,select,cancel;

 char c;

 x0=3;y0=303;
 posmouse=select=cancel=0;

 for (ii=0;ii<2;ii++)
    {
      deb=100*(ii+1)+(50*ii);
      outline_box(deb,25,50,15,YELLOW,LIGHTGRAY);
      yes_no_text(deb+5,30,ii);
    }

 deb=100*(index+1)+(50*index);
 displ_box(deb+1,26,deb+49,39,LIGHTBLUE);
 yes_no_text(deb+5,30,index);

 if (control==MOUSE)
   {
    mouse_xclip(7,512);
    mouse_yclip(307,345);
    set_curs_pos(deb+x0+5,y0+25);
    cursor_shape(0);
   }
 while ((!cancel) && (!select))
      {
       if (control==MOUSE)
	 {
	  cursor_counter(1);
	  test_press(0);
	  while ((!posmouse) && (!kbhit()))
	       {
		if (test_press(0)!=0)
		  {
		   for (ii=0;ii<2;ii++)
		      {
		       deb=x0+(100*(ii+1)+(50*ii));
		       if ((XMOUSE>deb) && (XMOUSE<deb+50) && (YMOUSE>323) && (YMOUSE<338))
			 {
			  cursor_counter(2);
			  setfillstyle(SOLID_FILL,BLACK);
			  deb=100*(index+1)+(50*index);
			  bar(deb+1,26,deb+49,39);
			  yes_no_text(deb+5,30,index);
			  index=ii;
			  setfillstyle(SOLID_FILL,LIGHTBLUE);
			  deb=100*(index+1)+(50*index);
			  bar(deb+1,26,deb+49,39);
			  yes_no_text(deb+5,30,index);
			  posmouse=1;select=1;
			 }
		      }
		  }
	       }
	 }
       if (!select)
	 {
	  c=getch();
	  if (c==0) c=getch();
	  if (control==MOUSE) cursor_counter(2);
	  switch(c)
	     {
	      case LEFT:
			  deb=100*(index+1)+(50*index);
			  displ_box(deb+1,26,deb+49,39,BLACK);
			  yes_no_text(deb+5,30,index);
			  if (index==0) index=1;
			  else index--;
			  deb=100*(index+1)+(50*index);
			  displ_box(deb+1,26,deb+49,39,LIGHTBLUE);
			  yes_no_text(deb+5,30,index);
			  break;

	      case RIGHT:
			  deb=100*(index+1)+(50*index);
			  displ_box(deb+1,26,deb+49,39,BLACK);
			  yes_no_text(deb+5,30,index);
			  if (index==1) index=0;
			  else index++;
			  deb=100*(index+1)+(50*index);
			  displ_box(deb+1,26,deb+49,39,LIGHTBLUE);
			  yes_no_text(deb+5,30,index);
			  break;

	      case 'y':
	      case 'Y':
			  deb=100*(index+1)+(50*index);
			  displ_box(deb+1,26,deb+49,39,BLACK);
			  yes_no_text(deb+5,30,index);
			  index=1;
			  deb=100*(index+1)+(50*index);
			  displ_box(deb+1,26,deb+49,39,LIGHTBLUE);
			  yes_no_text(deb+5,30,index);
			  break;

	      case 'n':
	      case 'N':
			  deb=100*(index+1)+(50*index);
			  displ_box(deb+1,26,deb+49,39,BLACK);
			  yes_no_text(deb+5,30,index);
			  index=0;
			  deb=100*(index+1)+(50*index);
			  displ_box(deb+1,26,deb+49,39,LIGHTBLUE);
			  yes_no_text(deb+5,30,index);
			  break;

	      case ENTER:
			  select=1;
			  break;
	     }
	 }
      }
 return(index);
}

/***************************************************************************/
int saving_mode(void)
 {
   int ii,deb,index=1,
       select=0,cancel=0,posmouse=0,x0=6;

   char c;

   for (ii=0;ii<3;ii++)
      {
       deb=70*(ii+1)+(80*ii);
       outline_box(deb,34,85,30,YELLOW,LIGHTGRAY);
       save_text(deb+5,39,ii);
      }
   deb=70*(index+1)+(80*index);
   displ_box(deb+1,35,deb+84,63,LIGHTBLUE);
   save_text(deb+5,39,index);
/*
   if (control==MOUSE)
     {
      set_curs_pos();
      cursor_shape(0);
     }
*/
  while ((!cancel) && (!select))
      {
       if (control==MOUSE)
	 {
	  mouse_xclip(7,512);
	  mouse_yclip(274,345);
	  cursor_counter(1);
	  test_press(0);test_press(1);
	  while ((!posmouse) && (!kbhit()))
	       {
		if (test_press(1)!=0) {cancel=1;posmouse=1;index=-1;select=1;}
		if (test_press(0)!=0)
		  {
		   for (ii=0;ii<3;ii++)
		      {
		       deb=x0+(70*(ii+1)+80*ii);
		       if ((XMOUSE>deb) && (XMOUSE<deb+85) && (YMOUSE>305) && (YMOUSE<335))
			 {
			  cursor_counter(2);
			  setfillstyle(SOLID_FILL,BLACK);
			  deb=70*(index+1)+(80*index);
			  bar(deb+1,35,deb+84,63);
			  save_text(deb+5,39,index);
			  index=ii;
			  setfillstyle(SOLID_FILL,LIGHTBLUE);
			  deb=70*(index+1)+(80*index);
			  bar(deb+1,35,deb+84,63);
			  save_text(deb+5,39,index);
			  posmouse=1;select=1;
			 }
		      }
		  }
	       }
	 }
       if (!select)
	 {
          c=getch();
	  if (control==MOUSE) cursor_counter(2);
	  if (c==ESC) {cancel=1;index=-1;break;}
	  if (c==0) c=getch();
	  switch(c)
	     {
	      case LEFT:
			  deb=70*(index+1)+(80*index);
			  displ_box(deb+1,35,deb+84,63,BLACK);
			  save_text(deb+5,39,index);
			  if (index==0) index=2;
			  else index--;
			  deb=70*(index+1)+(80*index);
			  displ_box(deb+1,35,deb+84,63,LIGHTBLUE);
			  save_text(deb+5,39,index);
			  break;


	      case RIGHT:
                          deb=70*(index+1)+(80*index);
			  displ_box(deb+1,35,deb+84,63,BLACK);
			  save_text(deb+5,39,index);
			  if (index==2) index=0;
			  else index++;
                          deb=70*(index+1)+(80*index);
			  displ_box(deb+1,35,deb+84,63,LIGHTBLUE);
			  save_text(deb+5,39,index);
			  break;


	      case ENTER: select=1;
			  break;

	      case 'i':
	      case 'I':
			 if (index!=0)
			   {
                            deb=70*(index+1)+(80*index);
			    displ_box(deb+1,35,deb+84,63,BLACK);
			    save_text(deb+5,39,index);
			    index=0;
			    deb=70*(index+1)+(80*index);
			    displ_box(deb+1,35,deb+84,63,LIGHTBLUE);
			    save_text(deb+5,39,index);
			   }
			 break;

	      case 't':
	      case 'T':
			 if (index!=1)
			   {
			    deb=70*(index+1)+(80*index);
			    displ_box(deb+1,35,deb+84,63,BLACK);
			    save_text(deb+5,39,index);
			    index=1;
			    deb=70*(index+1)+(80*index);
			    displ_box(deb+1,35,deb+84,63,LIGHTBLUE);
			    save_text(deb+5,39,index);
			   }
			 break;

	      case 'b':
	      case 'B':
			if (index!=2)
			  {
			    deb=70*(index+1)+(80*index);
			    displ_box(deb+1,35,deb+84,63,BLACK);
			    save_text(deb+5,39,index);
			    index=2;
			    deb=70*(index+1)+(80*index);
			    displ_box(deb+1,35,deb+84,63,LIGHTBLUE);
			    save_text(deb+5,39,index);
			  }
			 break;
	     }
	 }
      }
  return (index);
 }

/***************************************************************************/
void save_text(int x,int y,int ind)
{
 char *text[6]={"Individual","Total   ","Both",
		"  windows ","spectrum","mode"},c[15];
 int ii,l;

 setcolor(LIGHTRED);
 strcpy(c,text[ind]);
 c[1]='\0';
 outtextxy(x,y,c);

 setcolor(WHITE);
 l=strlen(text[ind]);
 for (ii=0;ii<l;ii++)
    {
     c[ii]=text[ind][ii+1];
    }
 c[l]='\0';
 outtextxy(x+8,y,c);
 strcpy(c,text[ind+3]);
 outtextxy(x,y+13,c);
}

void stop_text(int x,int y,int ind)
{
 char *text[2]={"Continue","Stop scan"},c[10];
 int ii,l;

 setcolor(LIGHTRED);
 strcpy(c,text[ind]);
 c[1]='\0';
 outtextxy(x,y,c);

 setcolor(WHITE);
 l=strlen(text[ind]);
 for (ii=0;ii<l;ii++)
    {
      c[ii]=text[ind][ii+1];
    }
 c[l]='\0';
 outtextxy(x+8,y,c);
}
/*************************************************************************/
int stop_all_or_continue(void)
{
 int deb,ii,index=0,
     x0,y0,posmouse,select,cancel;

 char c;

 x0=7;y0=307;
 posmouse=select=cancel=0;

 setviewport(7,307,512,345,0);
 clearviewport();
 for (ii=0;ii<2;ii++)
    {
      deb=70*(ii+1)+(80*ii);
      outline_box(deb,25,80,15,YELLOW,LIGHTGRAY);
      stop_text(deb+5,30,ii);
    }

 deb=70*(index+1)+(80*index);
 displ_box(deb+1,26,deb+79,39,LIGHTBLUE);
 stop_text(deb+5,30,index);

 if (control==MOUSE)
   {
    mouse_xclip(7,512);
    mouse_yclip(307,345);
    set_curs_pos(deb+x0+5,y0+25);
    cursor_shape(0);
   }
 while ((!cancel) && (!select))
      {
       if (control==MOUSE)
	 {
	  cursor_counter(1);
	  test_press(0);
	  while ((!posmouse) && (!kbhit()))
	       {
		if (test_press(0)!=0)
		  {
		   for (ii=0;ii<2;ii++)
		      {
		       deb=x0+(70*(ii+1)+(80*ii));
		       if ((XMOUSE>deb) && (XMOUSE<deb+80) && (YMOUSE>323) && (YMOUSE<338))
			 {
			  cursor_counter(2);
			  setfillstyle(SOLID_FILL,BLACK);
			  deb=70*(index+1)+(80*index);
			  bar(deb+1,26,deb+79,39);
			  stop_text(deb+5,30,index);
			  index=ii;
			  setfillstyle(SOLID_FILL,LIGHTBLUE);
			  deb=70*(index+1)+(80*index);
			  bar(deb+1,26,deb+79,39);
			  stop_text(deb+5,30,index);
			  posmouse=1;select=1;
			 }
		      }
		  }
	       }
	 }
       if (!select)
	 {
	  c=getch();
	  if (c==0) c=getch();
	  if (control==MOUSE) cursor_counter(2);
	  switch(c)
	     {
	      case LEFT:
			  deb=70*(index+1)+(80*index);
			  displ_box(deb+1,26,deb+79,39,BLACK);
			  stop_text(deb+5,30,index);
			  if (index==0) index=1;
			  else index--;
			  deb=70*(index+1)+(80*index);
			  displ_box(deb+1,26,deb+79,39,LIGHTBLUE);
			  stop_text(deb+5,30,index);
			  break;

	      case RIGHT:
			  deb=70*(index+1)+(80*index);
			  displ_box(deb+1,26,deb+79,39,BLACK);
			  stop_text(deb+5,30,index);
			  if (index==1) index=0;
			  else index++;
			  deb=70*(index+1)+(80*index);
			  displ_box(deb+1,26,deb+79,39,LIGHTBLUE);
			  stop_text(deb+5,30,index);
			  break;


	      case ENTER:
			  select=1;
			  break;
	     }
	 }
      }
 return(index);
}

