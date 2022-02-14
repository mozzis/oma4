/*========================================================================*/
/*                                 DISKA.C                                */
/*========================================================================*/

#include <stdio.h>
#include <math.h>
#include <conio.h>
#include <stdlib.h>
#include <acq4.h>
#include <ctype.h>
#include <dos.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <dir.h>
#include <string.h>
#include <graphics.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>

/*========================================================================*/

extern int enable_param[],
	   intensite[],
	   NACC,ACQ_MODE,
	   UNITS,
	   control,
	   FIRDI,LASTDI,/*IMAX,IMIN,*/
	   OFFSET_DIOD,DIODE_NUMBER,
	   CUR_DETECT,CCD_MODEL,
	   acc[],
	   domaine;

extern long xbegt,xendt,ybegt,yendt;

extern float position;

extern long ifinal[];

extern char  FORELENGTH[],
	     SPECTROLENGTH[],
	     INT_TIME[],
	     CUR_GRAT[],
	     DELAY[],
	     WIDTH[],
	     OPERATOR[],
	     SLITWIDTH[],
	     SAMPLE[],
	     LASPOW[],
	     EXCLINE[],
	     SPECWIDTH[],
	     REMARK[],
	     PATH[],
	     *par[],
	     INTENS_BEGIN[],INTENS_END[],
	     intime[3][8];

/*========================================================================*/
char         TYPE[11],
	     GRATING[5],
	     HIGH_DISP_COEFF[6],
	     ANGLE[7],
	     FOCAL[8],
	     UTIL_DIOD_BEG[2][5],
	     UTIL_DIOD_END[2][5],
	     TREATMENT[6],
	     PLOTTER_PORT[5],PLOTTER_TYPE[5],
	     CCD_TYPE[6],MICRO_PLATE_PORT[5];

int          FORE_MODU,            /*                           */
	     SPEC_MODU,            /*                           */
	     MULTI_MODU,           /*   0 Modul not installed   */
	     CCD_MODU,             /*   1 Modul installed       */
	     PHOT_COUNT_MODU,      /*                           */
	     PULSOR_MODU,          /*                           */
	     MICRO_PLATE_MODU,     /*                           */
	     DOUBLER,               /* 0 Double not installed    */
				   /* 1 Double installed        */
				   /* Important if CCD used     */
	     DAC[2],
	     NBR_PIXEL[2],
	     PDA_TYPE,             /* 0 standard */
				   /* 1 microP   */
				   /* 2 SR       */
	     coeff[1024],
	     I,P,IP,decalage[4]={0,0,0},
	     NBPOINTS;

struct date today;
struct dfree free_disk;

FILE   *fp,*fp1,*fp2,*fp3;

long int delay_param[6][2],width_param[6][2],
	 OFFSET_4;

void read_pulsor_param(void);
void read_coeff(void);
void read_decalage(void);
void save_characteristics(void);

/*========================================================================*/
void read_characteristics(void)

{
 if (domaine==UV)
   { fp=fopen("CONFIG.UV","rt");}
 else
   { fp=fopen("CONFIG","rt");}

 fscanf(fp,"%s %s %s %s %s",TYPE,GRATING,ANGLE,FOCAL,HIGH_DISP_COEFF);
 fscanf(fp,"%d %d %d %d %d %d %d",&FORE_MODU,&SPEC_MODU,&MULTI_MODU,&CCD_MODU,&PHOT_COUNT_MODU,&PULSOR_MODU,&MICRO_PLATE_MODU);
 fscanf(fp,"%d %d %d %s %s",&PDA_TYPE,&DAC[0],&NBR_PIXEL[0],UTIL_DIOD_BEG[0],UTIL_DIOD_END[0]);
 fscanf(fp,"%d %d %s %s",&DAC[1],&NBR_PIXEL[1],UTIL_DIOD_BEG[1],UTIL_DIOD_END[1]);
 fscanf(fp,"%d %s %s %s %s",&DOUBLER,PLOTTER_PORT,PLOTTER_TYPE,CCD_TYPE,MICRO_PLATE_PORT);
 fclose(fp);
}
/**************************************************************************/
void read_offset(void)
{
 fp=fopen("OFFSET","rt");

 fscanf(fp,"%ld",&OFFSET_4);
 fclose(fp);
}
/**************************************************************************/
void save_characteristics(void)

{
 if (domaine==UV)
   { fp=fopen("CONFIG.UV","wt");}
 else
   { fp=fopen("CONFIG","wt");}

 fprintf(fp,"%s\n%s\n%s\n%s\n%s\n",TYPE,GRATING,ANGLE,FOCAL,HIGH_DISP_COEFF);
 fprintf(fp,"%d\n%d\n%d\n%d\n%d\n%d\n%d\n",FORE_MODU,SPEC_MODU,MULTI_MODU,CCD_MODU,PHOT_COUNT_MODU,PULSOR_MODU,MICRO_PLATE_MODU);
 fprintf(fp,"%d\n%d\n%d\n%s\n%s\n",PDA_TYPE,DAC[0],NBR_PIXEL[0],UTIL_DIOD_BEG[0],UTIL_DIOD_END[0]);
 fprintf(fp,"%d\n%d\n%s\n%s\n%d\n",DAC[1],NBR_PIXEL[1],UTIL_DIOD_BEG[1],UTIL_DIOD_END[1],DOUBLER);
 fprintf(fp,"%s\n%s\n%s\n%s\n",PLOTTER_PORT,PLOTTER_TYPE,CCD_TYPE,MICRO_PLATE_PORT);
 fclose(fp);
}
/**************************************************************************/
void read_param(void)

{
 int ii,indi,cc;
 char c;

 indi=ii=0;

 if (domaine==UV)
   { fp=fopen("param.uv","rt");}
 else
   { fp=fopen("param","rt");}

 do
   {
    cc=fgetc(fp);
    if (cc!=-1)
      {c=(char) cc;
       if (c!='\n')
	 {
	  par[indi][ii]=c;
	  ii++;
	 }
       else
	 {
	  par[indi][ii]='\0';
	  indi++;ii=0;
	 }
      }
   }
 while (!feof(fp));
 fclose(fp);
}

/**************************************************************************/
void save_param(void)

{
 int ii;

 if (domaine==UV)
   { fp=fopen("param.uv","wt");}
 else
   { fp=fopen("param","wt");}

 for (ii=0;ii<11;ii++)
    {
     fprintf(fp,"%s\n",par[ii]);
    }
 fclose(fp);
}

/**************************************************************************/
void read_coeff(void)
{
 int ii;

 if (domaine==UV)
   { fp=fopen("coeffxy.uv","rt");}
 else
   { fp=fopen("coeffxy","rt");}

 for (ii=0;ii<1024;ii++)
    {fscanf(fp,"%d",&coeff[ii]);}
 fclose(fp);
}
/**************************************************************************/
void read_decalage(void)
{
 fp=fopen("decalage","rt");
 fscanf(fp,"%d %d %d %d %d %d",&I,&decalage[0],&P,&decalage[1],&IP,&decalage[2]);
 fclose(fp);
}
/**************************************************************************/
void read_pulsor_param(void)
{
 int i;

 fp=fopen("delay.par","rt");
 if (fp==NULL) printf ("ERROR \a\n");
 else
   {
    for (i=0;i<6;i++)
       {fscanf(fp,"%ld",&delay_param[i][0]);
	fscanf(fp,"%ld",&delay_param[i][1]);}
   }
 fclose(fp);

 fp=fopen("width.par","rt");
 if (fp==NULL) printf("ERROR \a\n");
 else
   {
    for (i=0;i<6;i++)
       {fscanf(fp,"%d %d",&width_param[i][0],&width_param[i][1]);}
   }
 fclose(fp);

}
/**************************************************************************/
int save_ms0(int recmode)

{
 /**************************************************************/
 /* Create MS0 file.                                           */
 /* recmode=0  without accumulation  array stored: intensite[] */
 /* recmode=1  accumulation          array stored: ifinal[]    */
 /* If detector used is CCD OMA4 array stored: ifinal[]        */
 /**************************************************************/

 int i,imax=0,imin=16383,npoints,interdio;
 int abort=0;
 float it;

 char ac[4],day[4],month[4],year[5],to_date[11],
      last[10],first[10],
      max[10],min[10],
      c='"',
      diod[5],
      *unit_label[]={"cm-1","nm","meV","diode"};

 long maxi=0,mini=2147483647;

 long taille,taille_entete,taille_data,taille_param,free_octets;
 char s[10],detector_used[5];
 unsigned char driv;

 /*Date */
 getdate(&today);
 sprintf(day,"%d-",today.da_day);
 sprintf(month,"%d-",today.da_mon);
 sprintf(year,"%d",today.da_year);
 strcpy(to_date,day);
 strcat(to_date,month);
 strcat(to_date,year);

 /* Number of points */
 npoints=atoi(INTENS_END)-atoi(INTENS_BEGIN)+1;

 /* Number of accumulations */
 sprintf(ac,"%3d",NACC);

 /* Frequencies limits */
 switch (UNITS)
       {
	case CM_1: sprintf(last,"%8.2f",((1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_END)))));
		   sprintf(first,"%8.2f",((1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_BEGIN)))));
		   break;

	case  NM:  sprintf(last,"%8.2f",diode_wavelength(atoi(INTENS_END))/10);
		   sprintf(first,"%8.2f",diode_wavelength(atoi(INTENS_BEGIN))/10);
		   break;

	case DIO: interdio=DIODE_NUMBER-OFFSET_DIOD-atoi(INTENS_BEGIN);
		  sprintf(last,"%8.2f",(float)interdio);
		  interdio=DIODE_NUMBER-OFFSET_DIOD-atoi(INTENS_END);
		  sprintf(first,"%8.2f",(float)interdio);
		  break;

	case EV:  sprintf(last,"%8.2f",(1e8/diode_wavelength(atoi(INTENS_END)))/8.0658);
		  sprintf(first,"%8.2f",(1e8/diode_wavelength(atoi(INTENS_BEGIN)))/8.0658);
		  break;
       }

 /* Intensities max and min */
    for (i=atoi(INTENS_END);i>=atoi(INTENS_BEGIN);i--)
       {
	if (ifinal[i]>maxi)
	  {maxi=ifinal[i];}
	if (ifinal[i]<mini)
	  {mini=ifinal[i];}
       }
    it=(float)maxi/NACC;
    sprintf(max,"%9.2f",it);
    it=(float)mini/NACC;
    sprintf(min,"%9.2f",it);

  sprintf(diod,"%4d",DIODE_NUMBER);

 /* Detector used */
  strcpy(detector_used,CCD_TYPE);

 /********************/
 /*Creating the file */
 /********************/
 /*Verify if there is enough free space to save the file on the choosen drive*/

    /*Determine available space on drive */

    if (isupper(PATH[0])==0) driv=PATH[0]-96;
    else driv=PATH[0]-64;
    getdfree(driv,&free_disk);
    free_octets=(long)free_disk.df_avail*(long)free_disk.df_bsec*(long)free_disk.df_sclus;

    /*Size of the file */
    taille_entete=138+strlen(unit_label[UNITS])+2+strlen(itoa(npoints,s,10))+2;
    taille_data=2*npoints*10;
    taille_param=72+strlen(OPERATOR)+strlen(to_date)+strlen(SAMPLE)+strlen(diod)+strlen(CUR_GRAT);
    taille_param=taille_param+strlen(EXCLINE)+strlen(LASPOW)+strlen(FORELENGTH)+strlen(SPECTROLENGTH);
    taille_param=taille_param+strlen(SLITWIDTH)+strlen(SPECWIDTH)+strlen(INT_TIME)+strlen(ac);
    taille_param=taille_param+strlen(REMARK)+strlen(ANGLE)+strlen(FOCAL)+1;
    taille=taille_entete+taille_data+taille_param;

    if (taille+10000<free_octets)
      {
       setfillstyle(SOLID_FILL,BLUE);
       bar(0,61,505,71);
       setcolor(LIGHTGRAY);
       outtextxy(10,63,"Please wait!   Saving file: ");
       setcolor(WHITE);
       outtextxy(234,63,PATH);

       fp=fopen(PATH,"wt");

       fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,"Version 3.00",c,c,"Multi-standard",c,c,"1",c);
       fprintf(fp,"%c%d%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,npoints,c,c,"wavenumber",c,c,"",c,c,detector_used,c,c,unit_label[UNITS],c,c,"",c,c,"s",c);
       fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,last,c,c,first,c,c,max,c,c,min,c,c,"0",c,c,"0",c);

       for (i=atoi(INTENS_END);i>=atoi(INTENS_BEGIN);i--)
	  {
	   it=(float)ifinal[i]/NACC;

	   switch (UNITS)
	       {
		case CM_1 :
			    fprintf(fp,"%8.2f\n%8.1f\n",((1e7/atof(EXCLINE))-(1e8/diode_wavelength(i))),it);
			    break;

		case NM: fprintf(fp,"%8.2f\n%8.1f\n",diode_wavelength(i)/10,it);
			 break;

		case DIO  :
			    interdio=DIODE_NUMBER-OFFSET_DIOD-i;
			    fprintf(fp,"%8.2f\n%8.1f\n",(float)interdio,it);
			    break;
	       }
	  }

       fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	      c,OPERATOR,c,c,to_date,c,c,SAMPLE,c,c,diod,c,c,CUR_GRAT,c,c,"",c,c,EXCLINE,c,c,LASPOW,c,c
	     ,FORELENGTH,c,c,SPECTROLENGTH,c,c,SLITWIDTH,c);
       fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,SPECWIDTH,c,c,INT_TIME,c,c,ac,c,c,REMARK,c,c,ANGLE,c,c,FOCAL,c,c,"/",c);

       fclose(fp);
       abort=0;
      }
    else
      {
       abort=free_space_message();
      }

 if (control == MOUSE) cursor_shape(0);
 return(abort);
}

/**************************************************************************/
void temp_msu(int num,float *maximum,float *minimum)

/* in automatic recording temporary files are used   */

{
 int i;

 char string[13]="temp0.msu";

 float it;

 string[4]=num+48;
 fp=fopen(string,"wt");
 fprintf(fp,"%8.2f\n",*maximum);
 fprintf(fp,"%8.2f\n",*minimum);
 for (i=atoi(INTENS_END);i>=atoi(INTENS_BEGIN);i--)
    {
     it=(float)(ifinal[i])/NACC;

     fprintf(fp,"%8.1f\n",it);

    }
 fclose(fp);
}

/***************************************************************************/

int save_msu(int n_domain,int npts_ovlap,float *central_pos,int *cut_flag,float *maximum,float *minimum)
{
 /****************************************/
 /*Create MSU file.                      */
 /*Multichannel spectraunion (Scan mode) */
 /****************************************/

 int i,j,jj,npoints,n,nzones=0,begin=0,
     limit1,limit2,
     zone[20];

 float it,it2;

 char last[10],first[10],
      ac[4],day[4],month[4],year[5],to_date[11],
      string[]="temp0.msu",
      c='"',
      diod[5],
      *unit_label[]={"cm-1","nm","meV","diode"},
      max[10],min[10],
      detector_used[5];

 long taille,taille_entete,taille_data,taille_param,free_octets,taille_window;
 char s[10];
 unsigned char driv;
 int abort;

 /*Date */
 getdate(&today);
 sprintf(day,"%d-",today.da_day);
 sprintf(month,"%d-",today.da_mon);
 sprintf(year,"%d",today.da_year);
 strcpy(to_date,day);
 strcat(to_date,month);
 strcat(to_date,year);

 /*Number of points */
 n=atoi(INTENS_END)-atoi(INTENS_BEGIN)+1; /*Number of points of each window */
 for (i=0;i<n_domain;i++)
    {
     zone[i]=nzones;
     if (cut_flag[i]) nzones++;    /* Calculates number of spectral range */
    }
 npoints=(n_domain*n)-((n_domain-nzones)*npts_ovlap);

 /*Number of accumulations */
 sprintf(ac,"%3d",NACC);

 /* Detector used */
 switch (CUR_DETECT)
     {
      case PDA:
	       strcpy(detector_used,"PDA");
	       break;
      case CCD:
	       strcpy(detector_used,CCD_TYPE);
	       break;
     }

 /*Frequencies limits */
 /* Frequency of the last point of the whole spectrum */
 position=central_pos[n_domain-1];
 sprintf(last,"%8.2f",((1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_END)))));

 /* Frequency of the first point of the whole spectrum */
 position=central_pos[0];
 sprintf(first,"%8.2f",((1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_BEGIN)))));

 /*Intensities max and min */

    sprintf(max,"%9.2f",*maximum);
    sprintf(min,"%9.2f",*minimum);


 /********************/
 /*Creating the file */
 /********************/
 /*Verify if there is enough free space to save the file on the choosen drive*/


    /*Determine available space on drive */

    if (isupper(PATH[0])==0) driv=PATH[0]-96;
    else driv=PATH[0]-64;
    getdfree(driv,&free_disk);
    free_octets=(long)free_disk.df_avail*(long)free_disk.df_bsec*(long)free_disk.df_sclus;

    /*Size of the file */
    taille_entete=141+strlen(unit_label[UNITS])+2+strlen(itoa(npoints,s,10))+2;
    taille_data=2*npoints*10;
    taille_param=72+strlen(OPERATOR)+strlen(to_date)+strlen(SAMPLE)+strlen(diod)+strlen(CUR_GRAT);
    taille_param=taille_param+strlen(EXCLINE)+strlen(LASPOW)+strlen(FORELENGTH)+strlen(SPECTROLENGTH);
    taille_param=taille_param+strlen(SLITWIDTH)+strlen(SPECWIDTH)+strlen(INT_TIME)+strlen(ac);
    taille_param=taille_param+strlen(REMARK)+strlen(ANGLE)+strlen(FOCAL)+1;
    taille_window=31*n_domain;
    taille=taille_entete+taille_data+taille_param+taille_window;

    if (taille+10000<free_octets)
      {
       setfillstyle(SOLID_FILL,BLUE);
       bar(0,61,505,71);
       setcolor(LIGHTGRAY);
       outtextxy(10,63,"Please wait!   Saving file: ");
       setcolor(WHITE);
       outtextxy(234,63,PATH);

       fp=fopen(PATH,"wt");

       fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,"Version 3.00",c,c,"Multi-Spectra union",c,c,"1",c);

       fprintf(fp,"%c%d%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,npoints,c,c,"wavenumber",c,c,"",c,c,detector_used,c,c,unit_label[UNITS],c,c,"",c,c,"s",c);

       fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,last,c,c,first,c,c,max,c,c,min,c,c,"0",c,c,"0",c);

       /* Initialisation pour recherche maximum et minimum du spectre */
       /* apres calcul overlap */
       it=*maximum;
       *maximum=*minimum;
       *minimum=it;

       for (i=n_domain;i>0;i--)
          {
           string[4]=i+47;fp1=fopen(string,"rt");
	   fscanf(fp1,"%f",&it);
	   fscanf(fp1,"%f",&it);

           position=central_pos[i-1];
           if (cut_flag[i-1]) limit1=n-1;
           else limit1=n-1-npts_ovlap;

           if (i-2>=0)
	     {
              if (cut_flag[i-2]) limit2=0;
	      else
		{
		 limit2=npts_ovlap;
		 string[4]=i+46;fp2=fopen(string,"rt");
		 fscanf(fp2,"%f",&it);
		 fscanf(fp2,"%f",&it);
		}
             }
           else
             {
	      limit2=0;
             }
           jj=0;
           for (j=n-1;j>=0;j--)
	      {
		 fscanf(fp1,"%f",&it);
		 if ((j<=limit1) && (j>=limit2))
		   {

		       fprintf(fp,"%8.2f\n%8.1f\n",((1e7/atof(EXCLINE))-(1e8/diode_wavelength(j+atoi(INTENS_BEGIN)))),it);


		    if (it>*maximum) *maximum=it;
		    if (it<*minimum) *minimum=it;
		   }
		 if (j<limit2)
		   {
		    fscanf(fp2,"%f",&it2);
		    it=(it*(1-(float)(jj)/(float)(npts_ovlap)))+(it2*((float)(jj)/(float)(npts_ovlap)));

		       fprintf(fp,"%8.2f\n%8.1f\n",((1e7/atof(EXCLINE))-(1e8/diode_wavelength(j+atoi(INTENS_BEGIN)))),it);

	            if (it>*maximum) *maximum=it;
		    if (it<*minimum) *minimum=it;
		    jj++;
		   }
	      }
	   fclose(fp1);
	   fclose(fp2);
	  }

      sprintf(diod,"%4d",DIODE_NUMBER);
      fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,OPERATOR,c,c,to_date,c,c,SAMPLE,c,c,diod,c,c,CUR_GRAT,c,c,"",c,c,EXCLINE,c,c,LASPOW,c,c
	     ,"/"/*FORELENGTH*/,c,c,"/"/*SPECTROLENGTH*/,c,c,SLITWIDTH,c);
      fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,SPECWIDTH,c,c,INT_TIME,c,c,ac,c,c,REMARK,c,c,ANGLE,c,c,FOCAL,c);

      fprintf(fp,"%c%s%c\n%c%d%c\n",c," ",c,c,n_domain,c);

      for (j=n_domain-1;j>0;j--)
	 {
           begin+=(cut_flag[j]) ? n : (n-npts_ovlap);
           fprintf(fp,"%c%d%c\n%c%d%c\n%c%s%c\n%c%d%c\n",c,begin,c,c,cut_flag[n_domain-1-j],c,c,intime[zone[n_domain-1-j]],c,c,acc[zone[n_domain-1-j]],c);
	 }
      fprintf(fp,"%c%s%c\n%c%d%c\n%c%s%c\n%c%d%c\n%c%s%c\n",c,"0",c,c,cut_flag[n_domain],c,c,intime[zone[n_domain-1]],c,c,acc[zone[n_domain-1]],c,c,"0",c);

      /*Intensities max and min */

	 sprintf(max,"%9.2f",*maximum);
	 sprintf(min,"%9.2f",*minimum);

      rewind(fp);
      fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,"Version 3.00",c,c,"Multi-Spectra union",c,c,"1",c);

      fprintf(fp,"%c%d%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,npoints,c,c,"wavenumber",c,c,"",c,c,detector_used,c,c,unit_label[UNITS],c,c,"",c,c,"s",c);

      fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,last,c,c,first,c,c,max,c,c,min,c,c,"0",c,c,"0",c);
      fclose(fp);

      fclose(fp2);
      abort=0;
     }
   else
     {
      abort=free_space_message();
     }

 if (control == MOUSE) cursor_shape(0);
 return(abort);
}

/**************************************************************************/

int save_window_msu(int n_domain,float *central_pos,int *zone)
{
 /****************************************************************/
 /* Create a ms0 file for each window in Scan function recording */
 /****************************************************************/

 int npoints,i,interdio,l,j;

 char ac[4],day[4],month[4],year[5],to_date[11],
      last[10],first[10],
      c='"',
      diod[5],
      *unit_label[]={"cm-1","nm","meV","diode"},
      drive[MAXDRIVE],dir[MAXDIR],name[MAXFILE],ext[MAXEXT],
      string[10],max[10],min[10],
      detector_used[5];

 float it,maximum,minimum;

 long taille,taille_entete,taille_data,taille_param,free_octets;
 char s[10];
 unsigned char driv;
 int abort=0;

 taille=taille_entete=taille_data=taille_param=free_octets=0L;

 /*Date */
 getdate(&today);
 sprintf(day,"%d-",today.da_day);
 sprintf(month,"%d-",today.da_mon);
 sprintf(year,"%d",today.da_year);
 strcpy(to_date,day);
 strcat(to_date,month);
 strcat(to_date,year);

 /* Number of points */
 npoints=atoi(INTENS_END)-atoi(INTENS_BEGIN)+1;
 /* Number of diodes */
 sprintf(diod,"%4d",DIODE_NUMBER);
 /* Detector used */
 strcpy(detector_used,CCD_TYPE);

 fnsplit(PATH,drive,dir,name,ext);
 strcpy(string,"temp0.msu");
 l=strlen(name);

    /*Determine available space on drive */
    if (isupper(PATH[0])==0) driv=PATH[0]-96;
    else driv=PATH[0]-64;
    getdfree(driv,&free_disk);
    free_octets=(long)free_disk.df_avail*(long)free_disk.df_bsec*(long)free_disk.df_sclus;

    /*Size of the file */
    taille_entete=138+strlen(unit_label[UNITS])+2+strlen(itoa(npoints,s,10))+2;
    taille_data=2*npoints*10;
    taille_param=72+strlen(OPERATOR)+strlen(to_date)+strlen(SAMPLE)+strlen(diod)+strlen(CUR_GRAT);
    taille_param=taille_param+strlen(EXCLINE)+strlen(LASPOW)+7+7;
    taille_param=taille_param+strlen(SLITWIDTH)+strlen(SPECWIDTH)+strlen(INT_TIME)+3;
    taille_param=taille_param+strlen(REMARK)+strlen(ANGLE)+strlen(FOCAL)+1;
    taille=(long)n_domain*(taille_entete+taille_data+taille_param);

    if (taille+10000<free_octets)
      {
       for (i=0;i<n_domain;i++)
	  {
	   /* Number of accumulations */
	   sprintf(ac,"%3d",acc[zone[i]]);

	   /*Integration time */
	   strcpy(INT_TIME,intime[zone[i]]);

	   /* Position of the foremono & the spectrog. */
	   sprintf(FORELENGTH,"%7.1f",central_pos[i]);
	   strcpy(SPECTROLENGTH,FORELENGTH);
	   position=central_pos[i];

	   /* Frequencies limits */
	   switch (UNITS)
	     {
	      case CM_1:
		   sprintf(last,"%8.2f",((1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_END)))));
		   sprintf(first,"%8.2f",((1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_BEGIN)))));
		   break;

	      case DIO:
		  interdio=DIODE_NUMBER-OFFSET_DIOD-atoi(INTENS_BEGIN);
		  sprintf(last,"%8.2f",(float)interdio);
		  interdio=DIODE_NUMBER-OFFSET_DIOD-atoi(INTENS_END);
		  sprintf(first,"%8.2f",(float)interdio);
		  break;
	     }
	   strcpy(INT_TIME,intime[zone[i]]);

	   if (l<8) {name[l]=i+48;name[l+1]='\0';}
	   else name[l-1]=i+48;
	   fnmerge(PATH,drive,dir,name,ext);

	   setfillstyle(SOLID_FILL,BLUE);
	   bar(0,61,505,71);
	   setcolor(LIGHTGRAY);
	   outtextxy(10,63,"Please wait!   Saving file: ");
	   setcolor(WHITE);
	   outtextxy(234,63,PATH);

	   string[4]=i+48;
	   fp1=fopen(string,"rt");
	   fscanf(fp1,"%f",&maximum);
	   fscanf(fp1,"%f",&minimum);

	      sprintf(max,"%9.2f",maximum);
	      sprintf(min,"%9.2f",minimum);

	   fp=fopen(PATH,"wt");

	   fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,"Version 3.00",c,c,"Multi-standard",c,c,"1",c);
	   fprintf(fp,"%c%d%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,npoints,c,c,"wavenumber",c,c,"",c,c,detector_used,c,c,unit_label[UNITS],c,c,"",c,c,"s",c);
	   fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,last,c,c,first,c,c,max,c,c,min,c,c,"0",c,c,"0",c);

	   for (j=atoi(INTENS_END);j>=atoi(INTENS_BEGIN);j--)
	      {
	       fscanf(fp1,"%f",&it);
	       switch (UNITS)
		  {
		   case CM_1 :
			       fprintf(fp,"%8.2f\n%8.1f\n",((1e7/atof(EXCLINE))-(1e8/diode_wavelength(j))),it);
			       break;

		   case DIO  : interdio=DIODE_NUMBER-OFFSET_DIOD-j;
				  fprintf(fp,"%8.2f\n%8.1f\n",(float)interdio,it);
			       break;
		  }
	      }
	   fclose(fp1);

	   fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,OPERATOR,c,c,to_date,c,c,SAMPLE,c,c,diod,c,c,CUR_GRAT,c,c,"",c,c,EXCLINE,c,c,LASPOW,c,c
	     ,FORELENGTH,c,c,SPECTROLENGTH,c,c,SLITWIDTH,c);
	   fprintf(fp,"%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n%c%s%c\n",
	     c,SPECWIDTH,c,c,INT_TIME,c,c,ac,c,c,REMARK,c,c,ANGLE,c,c,FOCAL,c,c,"/",c);

	   fclose(fp);
	 }
       abort=0;
      }
    else
      {
       abort=free_space_message();
      }
 return(abort);
}

/****************************************************************************/
void temp_m3d(int num)
{
 int fpi,i;

 float it;

 char s[MAXPATH],
      drive[MAXDRIVE],
      dir[MAXDIR],
      file[MAXFILE],
      ext[MAXEXT];

 getcwd(s,MAXPATH);
 strcat(s,"\\temp\\");
 fnsplit(s,drive,dir,file,ext);
 strcpy(ext,".m3d");
 sprintf(file,"tempo%03d",num);
 fnmerge(s,drive,dir,file,ext);
 fpi=_creat(s,FA_ARCH);
 for (i=atoi(INTENS_END);i>=atoi(INTENS_BEGIN);i--)
    {
     it=(float)(ifinal[i])/NACC;
     write(fpi,&it,sizeof(float));
    }
 close(fpi);
}
/**************************************************************************/
void save_m3d(int sizex,int sizey)
/*Create a binary file
  and save parameters ,conditions of mapping and frequencies */

{
 int fpi,
     npoints,scan,typeformat,
     interdio,i;

 float it,first,last,maxf=0.0,minf=0.0;

 char *unit_label[]={"cm-1","nm","meV","diode","Kbar"},
      day[4],month[4],year[5],to_date[11];

 npoints=(atoi(INTENS_END)-atoi(INTENS_BEGIN))+1;
 scan=0;
 typeformat=3;

 /*Date */
 getdate(&today);
 sprintf(day,"%d-",today.da_day);
 sprintf(month,"%d-",today.da_mon);
 sprintf(year,"%d",today.da_year);
 strcpy(to_date,day);
 strcat(to_date,month);
 strcat(to_date,year);

 /* Frequencies limits */
 switch (UNITS)
       {
	case CM_1: last=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_END)));
		   first=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(atoi(INTENS_BEGIN)));
		   break;

	case  NM:  last=diode_wavelength(atoi(INTENS_END))/10;
		   first=diode_wavelength(atoi(INTENS_BEGIN))/10;
		   break;

	case DIO:  interdio=DIODE_NUMBER-OFFSET_DIOD-atoi(INTENS_BEGIN);
		   last=(float)interdio;
		   interdio=DIODE_NUMBER-OFFSET_DIOD-atoi(INTENS_END);
		   first=(float)interdio;
		   break;

	case EV  : last=(1e8/diode_wavelength(atoi(INTENS_END)))/8.0658; /*MTC 03/06/93 */
		   first=(1e8/diode_wavelength(atoi(INTENS_BEGIN)))/8.0658;  /*MTC 03/06/93 */
		   break;

	case KBAR:last=3808*(pow(14405/(1e8/diode_wavelength(atoi(INTENS_END))),5)-1);
		  first=3808*(pow(14405/(1e8/diode_wavelength(atoi(INTENS_BEGIN))),5)-1);
		  break;
       }

 fpi=_creat(PATH,FA_ARCH);

 /* Header */
 write(fpi,"Version 3.0",16);
 write(fpi,"Image-standard",16);
 write(fpi,&npoints,2);
 write(fpi,&sizex,2);
 write(fpi,&sizey,2);
 write(fpi,&scan,2);       /* Scan type */
 write(fpi,&typeformat,2); /* Data type format */
 write(fpi,"Wavenumber ",16);
 write(fpi,"Intensities",16);
 write(fpi,"length     ",16);
 write(fpi,"length     ",16);
 write(fpi,unit_label[UNITS],16);
 write(fpi,"           ",16);
 write(fpi,"æm ",16);
 write(fpi,"æm ",16);
 write(fpi,&last,sizeof(float));
 write(fpi,&first,sizeof(float));
 write(fpi,&maxf,sizeof(float));
 write(fpi,&minf,sizeof(float));
 write(fpi,&xendt,sizeof(long));
 write(fpi,&xbegt,sizeof(long));
 write(fpi,&yendt,sizeof(long));
 write(fpi,&ybegt,sizeof(long));

 /* Parameters */
 write(fpi,OPERATOR,9);
 write(fpi,to_date,11);
 write(fpi,SAMPLE,9);
 write(fpi,&DIODE_NUMBER,2);
 write(fpi,CUR_GRAT,5);
 write(fpi,"    ",4);   /* anciennement filtre */
 write(fpi,EXCLINE,8);
 write(fpi,LASPOW,4);
 write(fpi,FORELENGTH,8);
 write(fpi,SPECTROLENGTH,8);
 write(fpi,SLITWIDTH,4);
 write(fpi,SPECWIDTH,4);
 write(fpi,INT_TIME,8);
 write(fpi,&NACC,2);
 write(fpi,REMARK,51);
 write(fpi,ANGLE,6);
 write(fpi,FOCAL,8);

 /* Frequencies */
 for (i=atoi(INTENS_END);i>=atoi(INTENS_BEGIN);i--)
    {
      switch (UNITS)
	{
	   case CM_1 : it=(1e7/atof(EXCLINE))-(1e8/diode_wavelength(i));
		       break;

	   case NM:    it=diode_wavelength(i)/10;
		       break;

	   case DIO:   it=(float)(DIODE_NUMBER-OFFSET_DIOD-i);
		       break;

	   case EV :   it=(1e8/diode_wavelength(i))/8.0658;
		       break;

	   case KBAR : it=3808*(pow(14405/(1e8/diode_wavelength(i)),5)-1);
		       break;
	}
      write(fpi,&it,sizeof(float));
    }

 close(fpi);
}
/**************************************************************************/
void save_m3d_data(void)
{
 float it;
 int fpi,i;

 fpi=open(PATH,O_BINARY|O_APPEND|O_WRONLY);
 for (i=atoi(INTENS_END);i>=atoi(INTENS_BEGIN);i--)
    {
     it=(float)(ifinal[i])/NACC;
     write(fpi,&it,sizeof(float));
    }
 close(fpi);
}
/**************************************************************************/
int m3d_space(int nbspec)
{
 unsigned char driv;
 long taille,taille_entete,taille_data,taille_param,free_octets,taille_freq;
 int nbpoint,ok=0;

 /*Determine available space on drive */
  if (isupper(PATH[0])==0) driv=PATH[0]-96;
  else driv=PATH[0]-64;
  getdfree(driv,&free_disk);
  free_octets=(long)free_disk.df_avail*(long)free_disk.df_bsec*(long)free_disk.df_sclus;

 /*Space needed for m3d file */
 nbpoint=(atoi(INTENS_END)-atoi(INTENS_BEGIN))+1;
 taille_entete=10*16+5*2+4*sizeof(float)+4*sizeof(long);
 taille_param=9+11+9+2+5+8+4+8+8+4+4+8+2+51+6+8;
 taille_freq=nbpoint*sizeof(float);
 taille_data=nbspec*nbpoint*sizeof(float);
 taille=taille_entete+taille_param+taille_freq+taille_data;

 if (taille<free_octets) ok=1;

 return(ok);
}
/**************************************************************************/
void save_m3d_maxmin(long *max,long *min)
{
 float maxf,minf;
 int fpi;

 maxf=(float)(*max/NACC);
 minf=(float)(*min/NACC);

 fpi=open(PATH,O_BINARY|O_RDWR);
 lseek(fpi,170L+2*sizeof(float),SEEK_SET);
 write(fpi,&maxf,sizeof(float));
 write(fpi,&minf,sizeof(float));
 close(fpi);
}
/**************************************************************************/