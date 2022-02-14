/*========================================================================*/
/*                              CCD_SET4                                  */
/*========================================================================*/
/* This file contains all the fonctions for the detector CCD (EG&G)       */
/* OMAVISION model IV                                                     */
/*========================================================================*/

#include <acq4.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <decl0.h>
#include <graphics.h>
#include <commccd.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <process.h>
#include <time.h>
#include <detdriv.h>
#include <access4.h>
#include <counters.h>
/*========================================================================*/

extern int  bd,
	    trkmode,slcmode,acq_mode,
	    shutmode_ind,
	    NACC,control,MOVING,
	    intensite[],current_color,DOUBLER,ACCU,
	    X1[],Y1[],
	    /*IMAX,IMIN,*/DIV,bckgrd;


extern long IMAX,IMIN;

extern char tracks4[],slices4[],
	    x04[],deltax4[],y04[],deltay4[],
	    INT_TIME[],DARK_TIME[],
	    error_message[],
	    *TRACKMODE[],*SLICEMODE[],
	    *TRACKSLIMIT[],*SLICESLIMIT[];

extern long ifinal[];

extern FILE *fp;

/*========================================================================*/

int keytest,
    IGN_SCANS=0,
    fin=0,change_comnd=0;

long int idark[1024];

struct hour
      {
       int h;
       int mn;
       int s;
      };

void set_outreg(int mode);
void set_antibloom(int val);
void set_coolonof(int mode);
void set_temp(int val);
void stop_acq(int val);
int get_coolonof(void);
int get_temp(void);
/*========================================================================*/
void set_frame(void)
{
 set_track_mode();
 set_tracks();
 set_y0();
 set_deltay();

 set_slices();
 set_slcmode();
 set_x0();
 set_deltax();
 change_comnd=1;
}
/*************************************************************************/
int get_temp(void)
{
 float value;

 GetParam(DTEMP,&value);
 return ((int) value);
}
/*************************************************************************/
void set_temp(int val)
{
 SetParam(DTEMP,(float) val);
}
/*************************************************************************/
void set_coolonof(int mode)
{
 SetParam(COOLONOFF,(float) mode);
}
/*************************************************************************/
int get_coolonof(void)
{
 float value;

 GetParam(COOLONOFF,&value);
 return((int) value);
}
/*************************************************************************/
int get_cooler_status(void)
{
 float value;

 GetParam(COOLLOCK,&value);
 if ((int)value==0)
   {
    GetParam(COOLSTAT,&value);
    if ((int)value!=0) value=value+1.0;
   }
 return ((int) value);
}
/*************************************************************************/
int get_dmodel(void)
{
 float value;

 GetParam(DMODEL,&value);
 return ((int) value);
}
/*************************************************************************/
int get_cooltype(void)
{
 float value;

 GetParam(COOLTYPE,&value);
 return ((int) value);
}
/*************************************************************************/
int get_activex(void)
{
 float value;

 GetParam(ACTIVEX,&value);
 return ((int) value);
}
/*************************************************************************/
int get_activey(void)
{
 float value;

 GetParam(ACTIVEY,&value);
 return ((int) value);
}
/*************************************************************************/
int get_antibloom(void)
{
 float value;

 GetParam(ANTIBLOOM,&value);
 return ((int) value);
}
/*************************************************************************/
int get_shiftmode(void)
{
 float value;

 GetParam(SHFTMODE,&value);
 return ((int) value);
}
/*************************************************************************/
int get_outreg(void)
{
 float value;

 GetParam(OUTREG,&value);
 return ((int) value);
}
/*************************************************************************/
int get_pixeltime(void)
{
 float value;

 GetParam(SPEED,&value);
 return ((int) value);
}
/*************************************************************************/
void set_antibloom(int val)
{
 SetParam(ANTIBLOOM,(float) val);
}
/*************************************************************************/
void set_pixeltime(int val)
{
 SetParam(SPEED,(float) val);
}
/*************************************************************************/
void clear_mem(int memnum)
{
 SetParam(CLR,memnum);
}
/*************************************************************************/
void set_tracks(void)
{
  SetParam(TRACKS,atof(tracks4));  /*OMA IV*/
}
/*************************************************************************/
void set_track_mode(void)
{
  SetParam(TRKMODE,trkmode);
}
/*************************************************************************/
void set_y0(void)
{
 SetParam(Y0,atof(y04));
}

/*************************************************************************/
void set_deltay(void)
{
  SetParam(DELTAY,atof(deltay4));
}
/*************************************************************************/
void set_slices(void)
{
  SetParam(POINTS,atof(slices4));
}
/*************************************************************************/
void set_slcmode(void)
{
 SetParam(PNTMODE,slcmode);
}

/*************************************************************************/
void set_x0(void)
{
  SetParam(X0,atof(x04));
}
/*************************************************************************/
void set_deltax(void)
{
  SetParam(DELTAX,atof(deltax4));
}
/*************************************************************************/
void set_shiftmode(int mode)
{

 /*char comnd[30];*/

 /*strcpy(comnd,"SCANMODE 3");*/   /* Full window for ccd rotated at 90 */
 /*send_comnd(bd,comnd);*/

 SetParam(SHFTMODE,(float) mode);
}
/*************************************************************************/
void set_outreg(int mode)
{
 SetParam(OUTREG,(float) mode);
}
/*************************************************************************/
void set_scans(void)
{
 /*
 char comnd [30],s[5];

 strcpy(comnd,"I ");
 itoa(NACC,s,10);
 strcat(comnd,s);
 send_comnd(bd,comnd);
 */
 SetParam(I,(float) NACC);
}

/*************************************************************************/

void set_ignores(void)
{
 /*
 char comnd [30],s[5];

 strcpy(comnd,"K ");
 itoa(IGN_SCANS,s,10);
 strcat(comnd,s);
 send_comnd(bd,comnd);
 */
 SetParam(K,(float) 0);   /* No ignored scans for the moment */
}

/*************************************************************************/
/*
void set_mems(void)
{
 char comnd [30],s[5];

 strcpy(comnd,"J ");
 itoa(MEMS_NBR,s,10);
 strcat(comnd,s);
 send_comnd(bd,comnd);
}
*/
/*************************************************************************/

/*void set_preps(void)
{
 char comnd [30];

 strcpy(comnd,"H ");
 strcat(comnd,PREPS);
 send_comnd(bd,comnd);
}
*/
/*************************************************************************/

void set_da_mode(void)
{
 /*
 char comnd [30];

 strcpy(comnd,"DA ");
 strcat(comnd,ACQ_MODE);
 send_comnd(bd,comnd);
 */
 SetParam(DAPROG,(float) 1);   /*DA mode 1 for the moment*/
}

/*************************************************************************/

void set_et()
{
 /*
 char comnd[30];

 strcpy(comnd,"ET ");
 strcat(comnd,INT_TIME);
 send_comnd(bd,comnd);
 */

 SetParam(ET,atof(INT_TIME));
}
/*************************************************************************/
/*
void set_freq(void)
{
 char comnd [30];

 strcpy(comnd,"FREQ ");
 strcat(comnd,FREQ);
 send_comnd(bd,comnd);

}
*/
/*************************************************************************/
/*Shutter mode */
void set_shutter_forced_mode(void)

{
 /*
 char comnd[30],chaine[3];

 strcpy(comnd,"SHUTMODE ");
 itoa(shutmode_ind,chaine,10);
 strcat(comnd,chaine);
 send_comnd(bd,comnd);
 */
 SetParam(SHUTMODE,(float)shutmode_ind);
}

/*************************************************************************/
    /* Ext/Int Start */
/*
void set_start_mode(void)

{
 char chaine[3],comnd[30];

 strcpy(comnd,"STARTMODE ");
 itoa(startmode_ind,chaine,10);
 strcat(comnd,chaine);
 send_comnd(bd,comnd);
}
*/
/*************************************************************************/
void stop_acq(int mode)
{
 SetParam(STOP,(float) mode);
}
/*************************************************************************/
/* Open shutter synchro */
/*
void set_shutter_open_sync(void)
{

  char chaine[3],comnd[30];

  strcpy(comnd,"WFTO");
  itoa(open_sync_ind,chaine,10);
  strcat(comnd,chaine);
  send_comnd(bd,comnd);

}
*/

/*************************************************************************/
 /* Close shutter synchro */
 /*
void set_shutter_close_sync(void)
{
  char chaine[3],comnd[30];

  strcpy(comnd,"WFTC");
  itoa(close_sync_ind,chaine,10);
  strcat(comnd,chaine);
  send_comnd(bd,comnd);
}
*/
/*************************************************************************/

/*************************************************************************/
int enter_ccd_accumul(void)
{
 char s[2][4]={"\0","\0"},
      yes_no[2][2]={"N","Y"},
      c;
 int x[]={252,252},
     y[]={4,16},
     param_length[]={3,1},
     ind,
     action=0,
     x0=7,
     y0=307,
     insert=0,
     select,
     exit=1;

 if (control==MOUSE) cursor_counter(2);

 /* Close the shutter */
 ind=shutmode_ind;
 shutmode_ind=CLOSED;
 set_shutter_mode();
 shutmode_ind=ind;

 setviewport(7,307,512,346,0);
 clearviewport();

 setcolor(YELLOW);
 outtextxy(4,29,"SPACE or left button to validate,Esc or right button to quit");

 setcolor(WHITE);
 outtextxy(20,4, "Number of scans............:");
 outtextxy(20,16,"Background subtraction.....:");

 setcolor(LIGHTRED);
 outtextxy(252,4,itoa(NACC,s[0],10));
 outtextxy(252,16,yes_no[bckgrd]);

 strcpy(s[1],yes_no[bckgrd]);
 ind=0;

 write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,s[ind]);
 select=0;

  do
    {
    action=0;
    do
      {
      if (control==MOUSE) 
       	{
       	if (test_press(0)!=0) 
       	  {
       	  c=SPACE;action=1;
       	  }
       	else if (test_press(1)!=0) 
       	  {
       	  c=ESC;action=1;
       	  }
        }
      if (kbhit() && (!action)) 
        {
        c=getch();action=1;
        }
      }
    while (!action);

    if (c==ESC) 
      {
      exit=-1;
		  set_shutter_mode();
		  select=1;break;
		  }

    if (c==SPACE)
      {
	    NACC=atoi(s[0]);
	    select=1;
	    }
    if (c==0) 
      c=getch();
    switch(c)
	    {
  	  case UP:
  		  if (ind!=0)
    			{
    			write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,s[ind]);
    			ind--;
    			write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,s[ind]);
    			}
  		break;

	    case DOWN:
		    if (ind!=1)
			    {
			    write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,s[ind]);
			    ind++;
			    write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,s[ind]);
			    }
		  break;

	    case ENTER:
		    if (ind==0)
  			  {
  			  edit_string(param_length[ind],x[ind]+x0,y[ind]+y0,s[ind],&insert);
  			  }
  		  else
  			  {
  			  write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,YELLOW,s[ind]);
  			  do
  			    {
  			    c=getch();
  			    if (c==0) 
  			      c=getch();
  			    if ((c==LEFT) || (c==RIGHT))
  			      {
  			      bckgrd=bckgrd^1;
  			      strcpy(s[1],yes_no[bckgrd]);
  			      write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,YELLOW,s[ind]);
  			      }
  			    }
  			  while (c!=ENTER);
  			  }
		    setviewport(7,307,512,346,0);
		    write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,s[ind]);
		    /* Next parameter */
		    ind=ind^1;
		    write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,s[ind]);
		  break;
	    }
    }
  while (!select);

  if ((bckgrd) && (exit!=-1)) /* If background subtraction,enter dark integration time */
    {
    /* Enter Int. time for the Dark */
    setviewport(7,307,513,335,0);
    clearviewport();
    setcolor(WHITE);
    outtextxy(20,10,"Dark integration time......:");
    write_text(LIGHTGRAY,260,10,7,10,LIGHTGREEN,DARK_TIME);

    select=0;

    do
  	  {
  	  action=0;
  	  do
  	    {
  	    if (control==MOUSE)
  	      {
      		if (test_press(0)!=0) 
      		  {
      		  c=SPACE;
      		  action=1;
      		  }
      		if (test_press(1)!=0) 
      		  {
      		  c=ESC;
      		  action=1;
      		  }
  	      }
  	    if (kbhit() && (!action)) 
  	      {
  	      c=getch();
  	      action=1;
  	      }
  	    }
  	  while (!action);
  	  switch(c)
  	    {
  	    case ESC: 
  	      exit=-1;
  			  select=1;
  			break;

  	    case ENTER: 
  	      X1[3]=257;
  	      Y1[3]=301;
  			  edit(DARK_TIME,7,5,3,0);
  			  X1[3]=526;Y1[3]=97;
  			break;

  	    case SPACE: 
  	      select=1;
  			break;
  	    }
  	  }
	  while (!select);
    } // if bkgrd && exit != -1 (enter dark integ time)

  if (control==MOUSE)
    {
    set_curs_pos(258,140);
    cursor_counter(1);
    }
  return(exit);
}

/*************************************************************************/
void conv_second(char *hh,int second)
{
  int min;
  struct hour h1;

  hh[2]='h';
  hh[5]='m';
  hh[6]='n';
  hh[9]='s';
  hh[10]='\0';

  min=second/60;
  h1.s=second % 60;
  h1.h=min/60;
  h1.mn=min % 60;
  hh[0]=48+(h1.h/10);
  hh[1]=48+(h1.h%10);
  hh[3]=48+(h1.mn/10);
  hh[4]=48+(h1.mn%10);
  hh[7]=48+(h1.s/10);
  hh[8]=48+(h1.s%10);
}

/**************************************************************************/
void disp_time(double *val)
{
  char s[6];

  sprintf(s,"%4.0f",*val);
  setviewport(100,287,130,299,0);
  write_text(LIGHTBLUE,5,2,4,10,WHITE,s);
}

/*************************************************************************/
void cooler_status(void)
{
  int cool_stat;

  cool_stat=get_cooler_status();
  disp_coolstat(cool_stat);
}
/*************************************************************************/
void disp_coolstat(int aa)
{
  setviewport(40,287,110,299,0);
  switch (aa)
    {
    case 0:
      write_text(LIGHTRED,5,2,8,10,YELLOW,"Unlocked");
    break;
    case 1:
      write_text(LIGHTGREEN,5,2,8,10,BLACK,"Locked  ");
    break;
    case 2:
      write_text(LIGHTRED,5,2,8,10,YELLOW,"EXCHANGE");
    break;
    case 3:
      write_text(LIGHTRED,5,2,8,10,YELLOW,"TOO HIGH");
    break;
    case 4:
      write_text(LIGHTRED,5,2,8,10,YELLOW," EMPTY  ");
    break;
    case 5:
      write_text(LIGHTRED,5,2,8,10,YELLOW,"DIF.EXC.");
    break;
    case 6:
      write_text(LIGHTRED,5,2,8,10,YELLOW," UNKNOWN");
    break;
    }
  setviewport(0,0,639,349,0);
}