/*==========================================================================*/
/*                                 HPGL4                                    */
/* Working  copy                                                            */
/* Plotter used: COLOR-PRO  and HP compatible plotters                      */
/*               LASER printer with HPGL emulation                          */
/* For CCD OMA4 applications                                                */
/*==========================================================================*/

#include <float.h>
#include <stdio.h>
#include <bios.h>
#include <string.h>
#include <math.h>
#include <acq4.h>
#include <graphics.h>
#include <stdlib.h>

#define FIN_ENVOI   13
#define DATA_READY  0x100
#define DEFAULT_SETTING (bauds_9600 | bits_8 | arret_2 | pas_parite)

#define bits_7         0x02
#define bits_8         0x03

#define arret_1        0x00
#define arret_2        0x04

#define pas_parite     0x00
#define parite_impaire 0x08
#define parite_paire   0x18

#define bauds_110      0x00
#define bauds_150      0x20
#define bauds_300      0x40
#define bauds_600
#define bauds_1200     0x80
#define bauds_2400     0xa0
#define bauds_4800     0xc0
#define bauds_9600     0xe0

extern int NACC,NS,ND,FIRDI,LASTDI,
	   CUR_DETECT,
	   intensite[];

extern char TYPE[],EXCLINE[],LASPOW[],SAMPLE[],PATH[],INT_TIME[],REMARK[],
	    INTENS_BEGIN[],INTENS_END[],
	    CCD_TYPE[],PLOTTER_PORT[];

extern long ifinal[];

int COM,LPT;
/*    Fin_= FIN_ENVOI;*/

char setting = DEFAULT_SETTING;
/*==========================================================================*/
int config_RS232(void)
{
 return (bioscom(0,0xe7, COM));
}
/*************************************************************************/
int config_LPT()
{
 int status,abyte=0;

 LPT=COM-2;
 status=biosprint(2,abyte,LPT);
 if (status && 0x90)
   {
     biosprint(1,abyte,LPT);
   }
 else
   {
      printf("ERREUR IMPRIMANTE: 0X%x\n",status);getch();
   }
 return (status);
}
/*************************************************************************/
void output(char *value)
{
 int i=0;

 if ((COM==0) || (COM==1))
   {
    while(value[i] != 0)
     {
      bioscom(1, value[i++], COM);
     }
   }
 else
   {
    while(value[i] != 0)
     {
      biosprint(0,value[i++],LPT);
     }
   }
}
/**************************************************************************/
/*
char *enter(char *value)
{
 int i=0;
 int status;
 int fin=Fin_;

 do
   {
    status=bioscom(3, 0, COM);
    if (status & DATA_READY)
       value[i++]=bioscom(2, 2, COM);
   } while (value[i-1] != fin);
 value[i-1]=0;
 return(value);
}
*/
/**************************************************************************/
void pen_up(void)
{
 output("PU;");
}
/**************************************************************************/
void pen_down(void)
{
 output("PD;");
}
/*************************************************************************/
void select_pen(int n)
{
 char pen[]={'0',';',0};

 pen[0]=n+48;
 output("SP");
 output(pen);
}
/*************************************************************************/
void label(char *texte)
{
 char etx[2];

 etx[0]=3;
 etx[1]=0;
 output("LB");
 output(texte);
 output(etx);
 pen_up();
}
/*************************************************************************/
void label_dir(float angle)
{
 int course;
 int elevation;
 char c[6];
 char e[6];

 angle     = 3.14159 * angle / 180;
 course    = (int)(cos(angle)*127);
 elevation = (int)(sin(angle)*127);
 sprintf(c,"%d,\0",course);
 sprintf(e,"%d;\0",elevation);
 output("DI");
 output(c);
 output(e);
}
/*************************************************************************/
void init_plt(void)
{
 output("IN;");
}
/************************************************************************/
void movea(float x, float y)   /* move absolu */
{
 char x_[]={0,0,0,0,0,0,0,0,0,0,0,0};
 char y_[]={0,0,0,0,0,0,0,0,0,0,0,0};

 sprintf(x_,"%9.4f,\0",x);
 sprintf(y_,"%9.4f;\0",y);
 output("PA;PU");
 output(x_);
 output(y_);
}
/***********************************************************************/
void mover(float x, float y)   /* move relatif */
{
 char x_[]={0,0,0,0,0,0,0,0,0,0,0,0};
 char y_[]={0,0,0,0,0,0,0,0,0,0,0,0};

 sprintf(x_,"%9.4f,\0",x);
 sprintf(y_,"%9.4f;\0",y);
 output("PR;PU");
 output(x_);
 output(y_);
 }
/***********************************************************************/
void plt_lineto(float x, float y, int penup)
{
 char x_[]={0,0,0,0,0,0,0,0,0,0,0,0};
 char y_[]={0,0,0,0,0,0,0,0,0,0,0,0};

 sprintf(x_,"%9.4f,\0",x);
 sprintf(y_,"%9.4f;\0",y);
 output("PA;PD");
 output(x_);
 output(y_);
 if (penup) pen_up();
 }
/***********************************************************************/
void maxmin(int *maxi,int *mini)
{/*
 int i;

 *maxi=0;*mini=16384;
 for (i=atoi(INTENS_BEGIN);i<=atoi(INTENS_END);i++)
    {
      if (intensite[i]>(*maxi)) *maxi=intensite[i];
      if (intensite[i]<(*mini)) *mini=intensite[i];
    } */
}
/****************************************************************************/
void accu_maxmin(float *accu_maxi,float *accu_mini)
{
 int i;

 *accu_maxi=0;*accu_mini=2147483647;
 for (i=atoi(INTENS_BEGIN);i<=atoi(INTENS_END);i++)
    {
	if (ifinal[i]>(*accu_maxi)) *accu_maxi=ifinal[i];
	if (ifinal[i]<(*accu_mini)) *accu_mini=ifinal[i];
    }
 *accu_maxi=*accu_maxi/NACC;
 *accu_mini=*accu_mini/NACC;
}
/****************************************************************************/
void quick_plot(int para_flag,int type)
{
 int i=0,j,beg,end,
     max,min;

 float xplt_step,yplt_step,y,abc,wave,wend,
       limit1,limit2,
       it,accu_max,accu_min;

 char string[50]="";

 /* Calcul MAX MIN du spectre */
 if (type==0) maxmin(&max,&min);
 else accu_maxmin(&accu_max,&accu_min);

 if (strcmp(PLOTTER_PORT,"COM1")==0) COM=0;
 else
   {
    if (strcmp(PLOTTER_PORT,"COM2")==0) COM=1;
    else
      {if (strcmp(PLOTTER_PORT,"LPT1")==0) COM=2;
       else COM=3;
      }
   }
 switch(COM)
    {
     case 0:
     case 1: config_RS232();
	     break;
     case 2:
     case 3: config_LPT();
	     break;
    }
 init_plt();
 select_pen(2);
 movea(700.0,550.0);            /* spectrum window. */
 plt_lineto(8894.0,550.0,0);
 plt_lineto(8894.0,6133.0,0);
 plt_lineto(700.0,6133.0,0);
 plt_lineto(700.0,550.0,0);

 movea(9400.0,550.0);           /* sample name window */
 plt_lineto(10400.0,550.0,0);
 plt_lineto(10400.0,6113.0,0);
 plt_lineto(9400.0,6113.0,0);
 plt_lineto(9400.0,550.0,0);
 movea(9400.0,3342.0);
 plt_lineto(10400.0,3342.0,0);

 movea(0.0,550.0);
 if (type==0) itoa(min,string,10);
 else sprintf(string,"%6.0f",accu_min);
 label(string);
 movea(610.0,611.0);            /* int ticks */
 plt_lineto(699.0,611.0,0);

 movea(0.0,6010.0);
 if (type==0) itoa(max,string,10);
 else sprintf(string,"%6.0f",accu_max);
 label(string);
 movea(610.0,6072.0);
 plt_lineto(699.0,6072.0,0);

 if (para_flag)
   {
     movea(9400.0,6400.0);
     plt_lineto(10400.0,6400.0,0);   /* dilor frame */
     plt_lineto(10400.0,7200.0,0);
     plt_lineto(9400.0,7200.0,0);
     plt_lineto(9400.0,6400.0,0);
     label_dir(270.0);
     strcpy(string,"DILOR");
     movea(10100.0,7100.0);
     label(string);
     movea(9600.0,7100.0);
     label(TYPE);

     label_dir(0.0);
     movea(0.0,0.0);		/* label */
     strcpy(string,"Wavenumber (cm-1)");
     label(string);

     strcpy(string,"Mode :");
     movea(700.0,7200.0);
     label(string);
     mover(100.0,0.0);
     itoa(NACC,string,10);
     strcat(string," accu. |");
     label(string);
     if (CUR_DETECT==CCD)
       {
	mover(100.0,0.0);
	strcpy(string,CCD_TYPE);
	label(string);
       }
     else
       {
        mover(100.0,0.0);
	itoa(NS,string,10);
	strcat(string," Sig. |");
	label(string);
	mover(100.0,0.0);
	itoa(ND,string,10);
	strcat(string," Dark");
	label(string);
       }

     strcpy(string,"Laser :");
     movea(5200.0,7200.0);
     label(string);
     mover(100.0,0.0);
     strcpy(string,EXCLINE);
     strcat(string," (nm)");
     label(string);
     mover(200.0,0.0);
     strcpy(string,"| ");
     strcat(string,LASPOW);
     strcat(string,"  (mw)");
     label(string);

     strcpy(string,"Time (s) :");
     movea(700.0,6800.0);
     label(string);
     mover(100.0,0.0);
     label(INT_TIME);

     strcpy(string,"Remark :");
     movea(700.0,6400.0);
     label(string);
     mover(100.0,0.0);
     label(REMARK);

     label_dir(270.0);           /* 270 degrees */
     strcpy(string,"Sample :");
     movea(10100.0,5500.0);
     label(string);
     movea(9600.0,5500.0);
     label(SAMPLE);
     strcpy(string,"Filename :");
     movea(10100.0,2700.0);
     label(string);
     movea(9600.0,2700.0);
     label(PATH);
   }
 else
   {
     movea(4200.0,6800.0);
     strcpy(string,"WORKING COPY !");
     label(string);
   }
 /*yplt_step=5461.0/(IMAX-IMIN);*/
 if (type==0) yplt_step=5461.0/((float)max-(float)min);
 else yplt_step=5461.0/(accu_max-accu_min);

 limit1=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_BEGIN)));
 limit2=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_END)));
 if (limit1<limit2)
   {
    it=limit2;
    limit2=limit1;
    limit1=it;
   }
 wend=limit1/100.0;
 end=100*(int)wend;
 wave=limit2/100.0;
 wave+=1.0;
 beg=100*(int)wave;
 xplt_step=8192.0/(limit1-limit2);
 label_dir(0.0);
 for (j=beg;j<=end;j+=100)
    {
      abc=8893.0-(xplt_step*(j-limit2));
      movea(abc,549.0);
      plt_lineto(abc,500.0,0);
      movea(abc-150.0,250.0);
      sprintf(string,"%4d",j);
      label(string);
    }

 select_pen(1);

 /*if (!type) y=(intensite[atoi(INTENS_BEGIN)]-min)*yplt_step;*/
 if (type) {it=(float)ifinal[atoi(INTENS_BEGIN)]/NACC;
	    y=(it-accu_min)*yplt_step;
	   }
 y = (y < 0.0) ? 0.0 : y;
 movea(8892.0,y+611.0);                         /* spectrum */
 for (i=atoi(INTENS_BEGIN);i<=atoi(INTENS_END);i++)
    {
	/*if (!type) y=(intensite[i]-min)*yplt_step;*/
	if (type)  {it=(float)ifinal[i]/NACC;
		    y=(it-accu_min)*yplt_step;}
	y = (y < 0.0) ? 0.0 : y;
	plt_lineto(8893-(xplt_step*(((1e7/atof(EXCLINE))-(1e8/diode_wavelength(i)))-limit2)),y+611.0,0);
    }
 pen_up();
 select_pen(0);
}
/****************************************************************************/