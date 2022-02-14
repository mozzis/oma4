/*========================================================================*/
/*                              OMA4-ACQ                                  */
/*========================================================================*/
/*    This file contains acquisition et accumulation fonctions for the    */
/*    for the detector CCD (EG&G) OMAVISION model IV                      */
/*========================================================================*/

#include <acq4.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <graphics.h>
#include <detdriv.h>
#include <access4.h>
#include <counters.h>

extern int current_color,control,
	   NACC,
	   ACCU,DOUBLER,MOVING,bckgrd,shutmode_ind,
	   X1[],Y1[],DIV,
	   change_comnd,fin,ret_accu,coeff[],
	   DIODE_NUMBER,flagjoystick,MICRO_PLATE_MODU,MAPPING;

extern long ifinal[],idark[],
	    IMAX,IMIN,
	    OFFSET_4;

extern char INT_TIME[],DARK_TIME[];
void acquire_spectrum(void);
/*========================================================================*/
/*================================*/
/* Read and display EG&G ccd data */
/* Read OMAIV data                */
/* MTC  15/10/91                  */
/*================================*/
void read_oma4_data(void)
{
 char c,ch[5];
 int i,comptage=0,ic,k,aa,compttime;
 long dummy;
 time_t temps_deb,temps_fin;

 if (atoi(INT_TIME)>=10)
   {comptage=1;compttime=0;}

 SetParam(CLR,(float) 0);/*MTC  */

 SetParam(RUN,(float) 3);
 set_DAC_counter(I1_COUNTER,1);

 if (comptage) temps_deb=time(NULL);

 do
   {
    ic=1;
    do
      {
	  if (control==MOUSE) test_mouse();
	  if (kbhit())
	    {
	       c=getch();
	       adjust(&c);
	    }

	  if ((MICRO_PLATE_MODU) && (flagjoystick))
	    {
	      read_xplate();read_yplate();
	    }

	  if (change_comnd)
	    {/*
	      if (ret_accu==0) {current_color=LIGHTRED;quick_disp(0,MS0);}
	      else {ret_accu=0;}
	     */
	      NACC=1; /* MTC 16-06-93 Bjorn o mysen Pb */
	      change_comnd=0;
	      if (atoi(INT_TIME)>=10)
		{comptage=1;compttime=0;}
	      else
		{comptage=0;setviewport(0,0,639,349,0);
		 write_text(BLACK,470,289,4,10,BLACK,"    ");
		}
	      /*MTC  */
	      SetParam(RUN,(float)3);
	      set_DAC_counter(I1_COUNTER,1);ic=1;
	      if (comptage) temps_deb=time(NULL);
	    }
	  else
	    {
	      if (comptage)
		{
		 temps_fin=time(NULL);
		 aa=(int) (difftime(temps_fin,temps_deb));
		 if ((aa-compttime)>=1)
		   {
		    compttime+=(aa-compttime);
		    sprintf(ch,"%04d",atoi(INT_TIME)-compttime);
		    setviewport(0,0,639,349,0);
		    write_text(LIGHTGRAY,470,289,4,10,BLACK,ch);
		   }

		 if (difftime(temps_fin,temps_deb)>=5.0)
		   {
		    current_color=LIGHTRED;
		    quick_disp(1,MS0);
		    /*comptage=0;*/
		   }
		}
	    }
	  ic=get_DAC_counter(I1_COUNTER);
      }
       while ((ic!=0) && (!fin));
    if (ic==0)
      {
	delay(10);
	ReadCurveFromMem((void far *)&ifinal[0],4*DIODE_NUMBER,0);
	if (DOUBLER!=1)
	  {
	   /* Double not set;Image inverted !!!*/
	   for (i=0;i<DIODE_NUMBER/2;i++)
	      {
	       dummy=ifinal[i];
	       ifinal[i]=ifinal[DIODE_NUMBER-1-i];
	       ifinal[DIODE_NUMBER-1-i]=dummy;
	      }
	  }
	/*Offset subtraction and detector correction */
	for (i=0;i<DIODE_NUMBER;i++)
	   {
	    ifinal[i]=ifinal[i]-OFFSET_4;
	    ifinal[i]=(ifinal[i]*(long) (coeff[i]))/100;
	   }
	current_color=LIGHTGREEN;
	quick_disp(0,MS0);
	cooler_status();
	set_DAC_counter(I1_COUNTER,1);
	if (atof(INT_TIME)>=10.0)
	  {temps_deb=time(NULL);comptage=1;compttime=0;}
	else
	  {comptage=0;}
      }
   }
    while ((!(fin)) || (MOVING==1));
}

/*************************************************************************/
int oma4_accumul(int recmode)
{
 int ic1,i0,ii,aa=0,required_acc,
     stop=0,shutmode_bck,
     bb,comptage,compttime;

 long dummy;
 float da_activ;
 char s[5]="    ",c,ch[5];

 /*time_t temps_deb,temps_fin;*/

 setviewport(7,307,512,345,0);
 clearviewport();
 if (bckgrd)
   {
    write_text(LIGHTRED,36,2,55,10,YELLOW,"ACCUMULATION WITH BACKGROUND SUBTRACTION IN PROGRESS...");
    setcolor(WHITE);
    outtextxy(2,18,"MODE:                Required accu.:     Current accu.:");
   }
 else
   {
    write_text(LIGHTRED,148,2,27,10,YELLOW,"ACCUMULATION IN PROGRESS...");
    setcolor(WHITE);
    outtextxy(2,18,"                     Required accu.:     Current accu.:");
   }
 setcolor(YELLOW);
 outtextxy(176,32,"Press SPACE to stop");
 setviewport(400,287,500,299,0);
 write_text(CYAN,10,2,6,10,LIGHTCYAN,"ACCUM.");
 ACCU=1;

 for (ii=0;ii<DIODE_NUMBER;ii++)
    {ifinal[ii]=0L;}

 /* Accumulation */
 required_acc=NACC;

 if (bckgrd)   /*Background acquisition */
   {
    /* Display parameters for dark (mode,int.time) */
    setviewport(7,307,512,345,0);
    write_text(BLACK,50,18,6,10,LIGHTRED,"DARK");
    write_text(BLACK,304,18,3,10,LIGHTRED,"1");
    setviewport(0,0,639,349,0);
    write_text(BLACK,X1[3]+17,Y1[3]+18,7,10,LIGHTRED,DARK_TIME);
    write_text(BLACK,X1[9]+17,Y1[9]+18,7,10,LIGHTRED,"CLOSED");

    /* Shutter closed */
    shutmode_bck=shutmode_ind;
    shutmode_ind=CLOSED;
    set_shutter_forced_mode();

    /* Setting dark time */
    SetParam(ET,atof(DARK_TIME));
    if (atoi(DARK_TIME)>=10)
      {comptage=1;compttime=0;}
    else
      {comptage=0;}

    /* Data acquisition & display */
    NACC=1;
    SetParam(I,(float)1);
    SetParam(J,(float)1); /*SetParam(K,(float)0); SetParam(H,(float)1);*/
    SetParam(RUN,(float)0);
    set_DAC_counter(I1_COUNTER,0);
    ic1=0;
    if (comptage) temps_deb=time(NULL);
    do
      {
       ic1=get_DAC_counter(I1_COUNTER);
       if (kbhit())
	 {
	  c=getch();
	  if (c==SPACE) stop=1;
	 }
       if (comptage)
	 {
	   temps_fin=time(NULL);
	   bb=(int) (difftime(temps_fin,temps_deb));
	   if ((bb-compttime)>=1)
	     {
	       compttime+=(bb-compttime);
	       sprintf(ch,"%04d",atoi(DARK_TIME)-compttime);
	       setviewport(0,0,639,349,0);
	       write_text(LIGHTGRAY,470,289,4,10,BLACK,ch);
	     }
	 }
       ic1=get_DAC_counter(I1_COUNTER);
      }
       while((ic1!=1) && (!stop));

    if (!stop)
      {
       delay(10);
       ReadCurveFromMem((void far *)&ifinal[0],4*DIODE_NUMBER,0);

       /* If doubler not set image inverted !!!! */
       if (DOUBLER!=1)
	 {
	  for (ii=0;ii<DIODE_NUMBER/2;ii++)
	     {
	      dummy=ifinal[ii];
	      ifinal[ii]=ifinal[DIODE_NUMBER-1-ii];
	      ifinal[DIODE_NUMBER-1-ii]=dummy;
	     }
	 }
	/* Offset correction and detecteur correction */
	for (ii=0;ii<DIODE_NUMBER;ii++)
	   {
	    ifinal[ii]=ifinal[ii]-OFFSET_4;
	    ifinal[ii]=(ifinal[ii]*(long) (coeff[ii]))/100;
	   }

       int_range();
       spacc_disp(1,1,MS0);
       for (ii=0;ii<DIODE_NUMBER;ii++)
	  {idark[ii]=ifinal[ii];}
      }
    else
      {
       close_window(3,303);ACCU=0;
       setviewport(400,287,500,299,0);
       write_text(CYAN,10,2,6,10,LIGHTCYAN,"ADJUST");
      }
   }
 setviewport(0,0,639,349,0);
 write_text(BLACK,470,289,4,10,BLACK,"    ");

 if (!stop)   /* Signal Acquisition */
   {
    if (bckgrd)
      {
       setviewport(7,307,512,345,0);
       write_text(BLACK,50,18,6,10,LIGHTRED,"LIVE");
      }
    setviewport(0,0,639,349,0);
    write_text(BLACK,X1[3]+17,Y1[3]+18,7,10,LIGHTRED,INT_TIME);
    write_text(BLACK,X1[9]+17,Y1[9]+18,7,10,LIGHTRED," LIVE ");

    setviewport(7,307,512,345,0);
    write_text(BLACK,304,18,3,10,LIGHTRED,itoa(required_acc,s,10));
    if (atoi(INT_TIME)>=10)
      {comptage=1;compttime=0;}
    else
      {comptage=0;}

    /* Live mode setting up */
    SetParam(I,(float)required_acc);
    SetParam(SHUTMODE,(float)LIVE);
    set_time(INT_TIME);
    SetParam(RUN,(float)0);
    set_DAC_counter(I1_COUNTER,0);
    i0=0;
    ic1=0;

    if (comptage)
      {temps_deb=time(NULL);}

    /* Accumulation & display */
    do
      {
       do
	 {
	  ic1=get_DAC_counter(I1_COUNTER);
	  GetParam(ACTIVE,&da_activ);
	  /*printf("<1. %d %3.1f>",ic1,da_activ);*/
	  if (ic1>=i0+1) break;
	  else
	    {
	     if (kbhit())
	       {
		 c=getch();
		 if (c==SPACE) {stop_acq(0);stop=1;}
		 else if (((!comptage) && (ic1<required_acc-1)) || ((comptage) && (atoi(INT_TIME)-compttime>2)) )
		     {
		      if (c==0) c=getch();
		      switch(c)
			 {
			  case UP  :
				    IMAX=(IMAX>1) ? IMAX>>1 : IMAX;
				    dummy=(IMAX-IMIN)>>8;
				    if ((DIV=(int)dummy)<1)
				    {DIV=1;IMAX=IMIN+256;}
				     break;

			  case DOWN:
				    IMAX=(IMAX<=131072) ? IMAX<<1 : IMAX;
				    dummy=(IMAX-IMIN)>>8;
				    DIV=(int)dummy;
				    break;
			 }
		      int_range();
		      spacc_disp(NACC,1,MS0);
		     }
	       }
	     ic1=get_DAC_counter(I1_COUNTER);
	     GetParam(ACTIVE,&da_activ);
	     /*printf("<2. %d %3.1f>",ic1,da_activ);*/

	     if (ic1>=i0+1) break;
	     else
	       {
		if ((comptage) && (!stop) )
		  {
		   temps_fin=time(NULL);
		   bb=(int) (difftime(temps_fin,temps_deb));
		   if ((bb-compttime)>=1)
		     {
			compttime+=(bb-compttime);
			sprintf(ch,"%04d",atoi(INT_TIME)-compttime);
			setviewport(0,0,639,349,0);
			write_text(LIGHTGRAY,470,289,4,10,BLACK,ch);
		     }
		  }

		ic1=get_DAC_counter(I1_COUNTER);
		GetParam(ACTIVE,&da_activ);
		/*printf("<3. %d %3.1f>",ic1,da_activ);*/
	       }
	    }
	 }
	  while((ic1<i0+1) && (!stop) &&(da_activ));

       if (!stop)
	 {
	  delay(10);
	  ReadCurveFromMem((void far *)&ifinal[0],4*DIODE_NUMBER,0);

	  /* If doubler not set image inverted !!! */
	  if (DOUBLER!=1)
	    {
	     for (ii=0;ii<DIODE_NUMBER/2;ii++)
		{
		 dummy=ifinal[ii];
		 ifinal[ii]=ifinal[DIODE_NUMBER-1-ii];
		 ifinal[DIODE_NUMBER-1-ii]=dummy;
		}
	    }
	  /* Offset subtraction and detector correction */
	  for (ii=0;ii<DIODE_NUMBER;ii++)
	     {
	      ifinal[ii]=ifinal[ii]-(OFFSET_4*ic1);
	      ifinal[ii]=(ifinal[ii] * (long) (coeff[ii]) )/100;
	     }
	  NACC=ic1;
	  spacc_disp(ic1,1,MS0);
	  i0++;
	  if (!da_activ) ic1=required_acc;
	  setviewport(7,307,512,345,0);
	  sprintf(s,"%03d",ic1);s[4]='\0';
	  write_text(BLUE,456,18,3,10,YELLOW,"    ");
	  write_text(BLUE,456,18,3,10,YELLOW,s);
	  if (comptage)
	    {compttime=0;temps_deb=time(NULL);}
	 }
      }
       while ((ic1!=required_acc) && (!stop) );

    setviewport(0,0,639,349,0);
    write_text(BLACK,X1[9]+17,Y1[9]+18,7,10,LIGHTRED,"CLOSED");

    /* Display of the result */
    if (bckgrd)
      {
       for (ii=0;ii<DIODE_NUMBER;ii++)
	  {ifinal[ii]=ifinal[ii]-(idark[ii]*NACC);}
      }
    current_color=YELLOW;
    spacc_disp(NACC,1,MS0);
    if ((recmode==MS0) && (!MAPPING))
      {
       /* Saving on disk */
       close_window(3,303);
       ask_accu(1);

       /*Plotting */
       enter_plot(1);
      }

    setviewport(0,0,639,349,0);
    write_text(BLACK,470,289,4,10,BLACK,"    ");
    if (bckgrd) shutmode_ind=shutmode_bck;

    if ((!aa) && (recmode==MS0) && (!MAPPING))
      {
       /* Cursor zoom save plot available on the accumulated spectrum. */
       /* Press F5 to restart normal mode .*/
       setviewport(10,287,125,299,0);
       write_text(YELLOW,0,2,13,10,RED,"F5 for ADJUST");
       setviewport(0,0,639,349,0);
      }

    if (control==MOUSE)
      {
       cursor_shape(0);
       mouse_xclip(0,639);
       mouse_yclip(0,349);
       set_curs_pos(258,140);
       cursor_counter(1);
      }
   }
 return(stop);
}

/************************************************************************/
void acquire_spectrum(void)
{
 float active;
 int i;
 unsigned long dummy;
 SetParam(RUN,(float)0);
 do
   {
    GetParam(ACTIVE,&active);
   }
    while( active);
 ReadCurveFromMem((void far *)&ifinal[0],4*DIODE_NUMBER,0);

 if (DOUBLER!=1)
   {
	   /* Double not set;Image inverted !!!*/
	   for (i=0;i<DIODE_NUMBER/2;i++)
	      {
	       dummy=ifinal[i];
	       ifinal[i]=ifinal[DIODE_NUMBER-1-i];
	       ifinal[DIODE_NUMBER-1-i]=dummy;
	      }
   }
 /*Offset subtraction and detector correction */
 for (i=0;i<DIODE_NUMBER;i++)
    {
	    ifinal[i]=ifinal[i]-OFFSET_4;
	    ifinal[i]=(ifinal[i]*(long) (coeff[i]))/100;
    }
 current_color=LIGHTGREEN;
 quick_disp(0,MS0);
 cooler_status();
}
