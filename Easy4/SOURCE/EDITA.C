/*========================================================================*/
/*                                 EDITA.C                                */
/*========================================================================*/
/* This file handles the string edition and the calculation of the        */
/* wavelength                                                             */
/*========================================================================*/

#include <acq4.h>
#include <string.h>
#include <graphics.h>
#include <conio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <dir.h>

/*========================================================================*/

extern int control,
	   GRATING_TYPE,
	   DIODE_NUMBER,
	   X1[],X2[],Y1[],Y2[],
	   UNITS,CUR_DETECT,CCD_MODEL,MAPPING,
	   cur_grat_value;

extern char ANGLE[],
	    FOCAL[],
	    EXCLINE[],
	    HD_MODE[],
	    CUR_GRAT[],
	    HIGH_DISP_COEFF[],
	    SLITWIDTH[],
	    INT_TIME[],
	    TYPE[],
	    OPERATOR[],
	    SAMPLE[],
	    LASPOW[],
	    REMARK[],
	    PATH[],
	    SPECWIDTH[],
	    *par[],FORELENGTH[];

extern float position,pixel_size;
int         MASKEDIT=0;

/*========================================================================*/
float b,l0,h,foca,k,reseau,alpha,i1,ix,k1,k2;
/*========================================================================*/

int edit (char *strname,int strleng,int pointpos,int k,int sign)
{
 char c,dummy[8];
 int  i,select=0,exit=0;

 /*i=strleng-1;*/
 i=0;
 strcpy(dummy,strname);

 if (k != 2) setviewport(X1[k]+1,Y1[k]+13,X1[k]+(strleng+2)*8,Y1[k]+28,0);
 else  setviewport(X1[1]+1,Y1[1]+13,X1[1]+(strleng+2)*8,Y1[1]+28,0);
 if (MAPPING)
   {
    setfillstyle(SOLID_FILL,BLACK);
    bar(16,0,(strleng+2)*8,14);
   }
 else clearviewport();

 setcolor(YELLOW);
 line(16+(i*8),14,24+(i*8),14);
 setcolor(LIGHTGREEN);
 outtextxy(16,5,strname);

 if (control==MOUSE) {test_press(1);test_press(0);}

 while (!select)
      {
       if (control==MOUSE)
	 {
	  if (test_press(1)!=0) {strcpy(strname,dummy);exit=1;break;}
	  if (test_press(0)!=0) select=1;
	 }
       if ((kbhit()) && (!select))
	 {
	  c=getch();
	  if (c == ESC) {strcpy(strname,dummy);exit=1;break;}
	  switch (c)
	     {
	      case '0' :
	      case '1' :
	      case '2' :
	      case '3' :
	      case '4' :
	      case '5' :
	      case '6' :
	      case '7' :
	      case '8' :
	      case '9' : if ((sign==0 && i!=pointpos-1  && i<=strleng-1) || (sign!=0 && i!=0 && i!=pointpos-1  && i<=strleng-1))
			   {
			     setcolor(BLACK);
			     line(16+(i*8),14,24+(i*8),14);
			     strname[i++]=c;
			   }
			 else
			    printf("\a");
			 break;

	      case '+'  :
	      case SPACE:
	      case '-'  : if (sign==0) break;
			  if ((k < 3) && (i==0)/* && (UNITS == CM_1)*/)
			   {
			     if ((i != pointpos-1)  && (i < strleng-1))
			       {
				 setcolor(BLACK);
				 /*setcolor(MAGENTA);*/
				 line(16+(i*8),14,24+(i*8),14);
				 strname[i++]=c;
			       }
			     else printf("\a");
			    }
			  else  printf("\a");
			  break;

	      case  0  : c=getch();
			 if ((c == 75) && (i > 0))
			   {
			    setcolor(BLACK);
			    line(16+(i*8),14,24+(i*8),14);
			    i--;
			    if (i == (pointpos-1)) i--;
			   }
			 else if ((c == 77) && (i < (strleng-1)))
				{
				  setcolor(BLACK);
				  line(16+(i*8),14,24+(i*8),14);
				  i++;
				  if (i == (pointpos-1)) i++;
				 }
			 else printf("\a");
			 break;

	      case ENTER:select=1;
			 break;

	      default  : printf("\a");
			 break;
	     }
	  if (i == pointpos-1)
	   i++;
	 if (i == strleng)
	   i--;
	 if (k == 2)
	   {
	    setviewport(X1[0]+1,Y1[0]+13,X1[0]+84,Y1[0]+26,0);
	    clearviewport();
	    setcolor(LIGHTGREEN);
	    outtextxy(16,5,strname);

	    setviewport(X1[1]+1,Y1[1]+13,X1[1]+84,Y1[1]+26,0);
	    clearviewport();
	    outtextxy(16,5,strname);
	    setcolor(YELLOW);
	    line(16+(i*8),14,24+(i*8),14);
	   }
	 else
	   {
	    setviewport(X1[k]+1,Y1[k]+13,X1[k]+(strleng+2)*8,Y1[k]+28,0);
	    if (MAPPING)
	      {
	       setfillstyle(SOLID_FILL,BLACK);
	       bar(16,0,(strleng+2)*8,14);
	      }
	    else clearviewport();
	    setcolor(LIGHTGREEN);
	    outtextxy(16,5,strname);
	    setcolor(YELLOW);
	    line(16+(i*8),14,24+(i*8),14);
	   }
	 }
      }
 if (k == 2)
   {
    setviewport(X1[0]+1,Y1[0]+13,X1[0]+84,Y1[0]+26,0);
    clearviewport();
    setcolor(WHITE);
    if (exit) outtextxy(16,5,FORELENGTH);
    else outtextxy(16,5,strname);
    setviewport(X1[1]+1,Y1[1]+13,X1[1]+84,Y1[1]+26,0);
    clearviewport();
    outtextxy(16,5,strname);
   }
 else
   {
    setviewport(X1[k]+1,Y1[k]+13,X1[k]+(strleng+2)*8,Y1[k]+28,0);
    if (MAPPING)
      {
	setfillstyle(SOLID_FILL,BLACK);
	bar(16,0,(strleng+2)*8,14);
      }
    else clearviewport();
    setcolor(WHITE);
    outtextxy(16,5,strname);
   }
 setcolor(BLACK);
 line(16+(i*8),14,24+(i*8),14);
 return(exit);
}


/***************************************************************************/
void edit_integ_time()

{
 int i,select=0,cancel=0;
 char c,ch[6];

 strcpy(ch,INT_TIME);
 i=0;
 setviewport(X1[3]+1,Y1[3]+13,X1[3]+56,Y1[3]+28,0);
 clearviewport();

 setcolor(YELLOW);
 line(16+(i*8),14,24+(i*8),14);
 setcolor(LIGHTGREEN);
 outtextxy(16,5,INT_TIME);

 if (control==MOUSE)
   {
    cursor_counter(2);
    test_press(0);test_press(1);
   }

 while (!select)
      {
	 if (control==MOUSE)
	   {
	    if (test_press(0)!=0) select=1;
	    else
	      {
	       if (test_press(1)!=0) {cancel=1;select=1;break;}
	      }
	   }
	 if ((kbhit()) && (!select))
	   {
	    c=getch();
	    if (c==0) c=getch();
	    switch (c)
		  {
		   case ESC:
			      cancel=1;
			      select=1;
			      break;

		   case ENTER:
			      select=1;
			      break;

		   case '0' :
		   case '1' :
		   case '2' :
		   case '3' :
		   case '4' :
		   case '5' :
		   case '6' :
		   case '7' :
		   case '8' :
		   case '9' : if ((i!=2) && (i<5))
				{
				 setcolor(BLACK);
				 line(16+(i*8),14,24+(i*8),14);
				 ch[i++]=c;
				 if ((i>2) && ((ch[0]!='0')||(ch[1]!='0')))
				   {
				    ch[0]=ch[1]='0';
				   }
				 if ((i<=2) && ((ch[3]!='0')||(ch[4]!='0')))
				   {
				    ch[3]=ch[4]='0';
				   }
				 if (i==2) i++;
				 if (i==5) i--;
				 clearviewport();
				 setcolor(LIGHTGREEN);
				 outtextxy(16,5,ch);
				 setcolor(YELLOW);
				 line(16+(i*8),14,24+(i*8),14);

				}
			      else
				printf("\a");
			      break;

		   case LEFT:
			      if (i>0)
				{
				 setcolor(BLACK);
				 line(16+(i*8),14,24+(i*8),14);
				 i--;
				 if (i==2) i--;
				 setcolor(YELLOW);
				 line(16+(i*8),14,24+(i*8),14);
				}
			      else printf("\a");
			      break;

		   case RIGHT:
			      if (i<4)
				{
				 setcolor(BLACK);
				 line(16+(i*8),14,24+(i*8),14);
				 i++;
				 if (i==2) i++;
				 setcolor(YELLOW);
				 line(16+(i*8),14,24+(i*8),14);
				}
			      else printf("\a");
			      break;

		   default:
			      printf ("\a");
			      break;
		  }
	   }
	}

 if (!cancel) strcpy(INT_TIME,ch);
 setviewport(X1[3]+1,Y1[3]+13,X1[3]+56,Y1[3]+28,0);
 clearviewport();
 setcolor(WHITE);
 outtextxy(16,5,INT_TIME);
 setcolor(BLACK);
 line(16+(i*8),14,24+(i*8),14);
}

/***************************************************************************/

void  spectral_width(void)
/* calculates the spectral bandwidth */

{
 float disp;

 disp=(atoi(SLITWIDTH)/1000.0)*((cos(ix)*1e15)/(l0*l0*foca*reseau));
 sprintf(SPECWIDTH,"%7.2f",disp);
}

/***************************************************************************/
float diode_wavelength(int i)
/* calculates the wavelength of a diode */

{
 int co;

 float c1, /* spacing between 2 photoelements in microns*/
       pi=3.141593;
/*
 if (CUR_DETECT==PDA) pixel_size=.025;
 else
   {
    switch (CCD_MODEL)
	{
	 case OMA3:
		   pixel_size=.023;
		   break;
	 case OMA4:
		    pixel_size=.019;
		   break;
	}
   }
*/
 foca=atof(FOCAL);
 if (strcmpi(HD_MODE,"H")==0) foca=atof(HIGH_DISP_COEFF)*foca;
 /*reseau=atof(CUR_GRAT);*/
 reseau=(float)(cur_grat_value); /* MTC 26-05-93*/

 alpha=atof(ANGLE);

 co = (DIODE_NUMBER/2)+1;
 k  = 1e7 / reseau;
 l0 = 1e8 / ((1e7/atof(EXCLINE))-position);
 h  = alpha*pi/180;
 b  = asin(reseau*l0*1e-7/2/cos(h));
 ix = b+h;
 i1 = b-h;
 k1 = sin(i1);
 k2 = sin(ix - atan((co-i)*pixel_size/foca));

 return(k*(k1+k2));
}

/******************************************************************************/

float central_length(int dionum,float *nurel)
/* calculates the central length of a domain in automatic rec. */

{
 int co;
 int a,b,c;
 long temp;

 float pi=3.141593,
       d,w,nuabs,nuo;
/*
 if (CUR_DETECT==PDA) pixel_size=.025;
 else
   {
    switch (CCD_MODEL)
	{
	 case OMA3:pixel_size=.023;
		   break;

	 case OMA4:a=get_dmodel();b=get_activex();c=get_activey();
		   if ((a==3) && (b==1024) && (c==256)) pixel_size=0.027;
		   else pixel_size=.019;
		   break;
	}
   }
*/
 foca=atof(FOCAL);
 if (strcmpi(HD_MODE,"H")==0) foca=atof(HIGH_DISP_COEFF)*foca;
/* reseau=atof(CUR_GRAT);*/
 reseau=(float)(cur_grat_value); /* MTC 26-05-93*/

 alpha=atof(ANGLE);

 co = (DIODE_NUMBER/2)+1;
 h  = alpha*pi/180;

 nuabs=(1e7/atof(EXCLINE))-*nurel;
 d=(co-dionum)*pixel_size/foca;
 w=tan(asin(5*reseau/(nuabs*cos((-2*h+d)/2)))+(d/2));
 k=5*reseau/cos(h);
 nuo=(1e7/atof(EXCLINE))-(k*sqrt((1/w/w)+1));
 temp=nuo*100;
 if (!strncmpi(TYPE,"uv",2))
    {if ((temp%20) != 0)
	 temp+=10;
     }
 nuo=temp/100;
 return(nuo);
}

/***************************************************************************/

int diode_position(float *nui)
/* calculates the diode number for a given frequency. */

{
 int co,i;

 float pi=3.141593,
       li;
/*
 if (CUR_DETECT==PDA) pixel_size=.025;
 else
   {
    switch (CCD_MODEL)
	{
	 case OMA3:pixel_size=.023;
		   break;
	 case OMA4:a=get_dmodel();b=get_activex();c=get_activey();
		   if ((a==3) && (b==1024) && (c==256)) pixel_size=0.027;
		   else pixel_size=.019;
		   break;
	}
   }
*/
 /*printf("pixel_size: %f\n",pixel_size);*/
 foca=atof(FOCAL);
 if (strcmpi(HD_MODE,"H")==0) foca=atof(HIGH_DISP_COEFF)*foca;
 /*reseau=atof(CUR_GRAT);*/

 reseau=(float)(cur_grat_value); /* MTC 26-05-93*/

 alpha=atof(ANGLE);
 co = (DIODE_NUMBER/2)+1;
 k  = 1e7 / reseau;
 if (UNITS == 0) li = 1e8/((1e7/atof(EXCLINE))-*nui);
 if (UNITS == 1) li = 10*(*nui);
 l0 = 1e8/((1e7/atof(EXCLINE))-position);
 h  = alpha*pi/180;
 b  = asin(reseau*l0*1e-7/2/cos(h));
 i1 = b-h;
 k1 = sin(i1);
 i = (int)((-foca/pixel_size)*tan(b-asin(li/k-k1)+h))+co;
 return(i);
}

/****************************************************************************/

int edit_param(void)
{
 int param_length[]={8,8,7,3,3,7,50,40},
     x[]={138,410,138,410,138,410,74,74},
     y[]={2,2,14,14,26,26,38,50},
     ind,select,x0=7,y0=275,ok,exit=0,action=0,insert=0;

 int aa,mt=0;
 char drive[MAXDRIVE],dir[MAXDIR],file[MAXFILE],ext[MAXEXT],
      current_rep[80],cur_drive[MAXDRIVE],cur_dir[MAXDIR],
      c;

 setviewport(7,275,512,346,0);
 clearviewport();
 setcolor(YELLOW);
 outtextxy(25,63,"SPACE or left button to save,Esc or right button to quit");

 setcolor(WHITE);
 outtextxy(2,2, "Operator.......:               Sample............:");
 outtextxy(2,14,"Exc. line (nm).:               Laser Pow. (mw)...:");
 outtextxy(2,26,"Slit width.....:               Spec.Width (cm-1).:");
 outtextxy(2,38,"Remark.:");
 outtextxy(2,50,"Path...:");

 setcolor(LIGHTRED);
 outtextxy(138,2,OPERATOR);
 outtextxy(138,14,EXCLINE);
 outtextxy(138,26,SLITWIDTH);
 outtextxy(410,2,SAMPLE);
 outtextxy(410,14,LASPOW);
 outtextxy(410,26,SPECWIDTH);
 outtextxy(74,38,REMARK);
 outtextxy(74,50,PATH);

 ind=7;

 write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par[ind]);
 select=0;

 if (control==MOUSE)
   {test_press(0);test_press(1);}

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
       if (kbhit() && (!action)) {c=getch();action=1;}
      }
       while (!action);

       if (c==ESC) {exit=1;select=1;break;}
       if (c==SPACE)
	 {
          getcwd(current_rep,80);
	  current_rep[strlen(current_rep)]='\\';
	  fnsplit(current_rep,cur_drive,cur_dir,file,ext);

          aa=fnsplit(PATH,drive,dir,file,ext);
	  if ((!(aa & DRIVE)) && (!(aa & DIRECTORY))) {strcpy(drive,cur_drive);strcpy(dir,cur_dir);}
	  else
	    {
	      if (!(aa & DRIVE)) {strcpy(drive,cur_drive);}
	      else
		{
		 if (!(aa & DIRECTORY)) {strcpy(dir,"\\");}
		}
	    }
	  fnmerge(PATH,drive,dir,file,ext);
	  mt=0;
          if (!(strcmp(drive,"A:")) || (!(strcmp(drive,"a:")))) mt=protect(0,1);
	  if (!(strcmp(drive,"B:")) || (!(strcmp(drive,"b:")))) mt=protect(1,1);

	  if (mt==0)
	    {
	      mt=dir_exist(drive,dir);
	      if (mt==0)
		{
		  ok=verify_path();
		  if (ok)
		    {
		      write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par[ind]);
		      save_param();
		      select=1;
		    }
		  else
		    {
		      save_param();
                      setfillstyle(SOLID_FILL,BLACK);
		      bar(0,61,505,71);
		      setcolor(YELLOW);
		      outtextxy(25,63,"SPACE or left button to save,Esc or right button to quit");
		      write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par[ind]);
		      ind=7;
		      c=ENTER;
		    }
		}
	      else
		{
		  save_param();
                  setfillstyle(SOLID_FILL,BLACK);
		  bar(0,61,505,71);
		  setcolor(YELLOW);
		  outtextxy(25,63,"SPACE or left button to save,Esc or right button to quit");
		  write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par[ind]);
		  ind=7;
		  c=ENTER;
		}
	    }
	  else
	    {
	     save_param();
             setfillstyle(SOLID_FILL,BLACK);
	     bar(0,61,505,71);
	     setcolor(YELLOW);
	     outtextxy(25,63,"SPACE or left button to save,Esc or right button to quit");
	     write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par[ind]);
	     ind=7;
	     c=ENTER;
	    }
	 }

       if (c==0) c=getch();
       switch(c)
	 {
	  case LEFT  :
		    if (ind!=0)
		      {
		       write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par[ind]);
		       ind--;
		       if (ind==5) ind--;
		       write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par[ind]);
		      }
		    break;

	  case RIGHT :
		     if (ind!=7)
		       {
                        write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par[ind]);
			ind++;
			if (ind==5) ind++;
                        write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par[ind]);
		       }
		      break;

	   case UP    :
		      if (ind>1)
			{
                         write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par[ind]);
			 if (ind==7) ind--;
			 else ind-=2;
			 if (ind==5) ind-=2;
                         write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par[ind]);
			}
		      break;

	   case DOWN  :
		      if (ind!=7)
			{
			 write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par[ind]);
			 if (ind>=5) ind++;
			 else ind+=2;
			 if (ind==5) ind++;
                         write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par[ind]);
			}
		      break;

	   case ENTER :
		      edit_string(param_length[ind],x[ind]+x0,y[ind]+y0,par[ind],&insert);
                      setviewport(7,275,512,346,0);
		      write_text(BLACK,x[ind],y[ind],param_length[ind],10,LIGHTRED,par[ind]);
		      if (ind==4)
			{
			 diode_wavelength(100);spectral_width();
			 write_text(BLACK,x[5],y[5],param_length[5],10,LIGHTRED,par[5]);
			}
		      /* Next parameter */
		      if (ind==7) {ind=0;}
		      else
			 {
			  ind++;
			  if (ind==5) ind++;
			 }
		      write_text(LIGHTGRAY,x[ind],y[ind],param_length[ind],10,LIGHTGREEN,par[ind]);
		      break;
	  }
   }
    while (!select);
 if (control==MOUSE)
   {cursor_shape(0);cursor_counter(1);}
 return(exit);
}

/****************************************************************************/

void edit_string(int length,int x,int y,char *s,int *ins)
{
 int letternum,j,ii,
     select=0;

 char c,ch[51],dummy[51];

 s[strlen(s)]='\0';

 if (strlen(s)>0) letternum=strlen(s)-1;
 else letternum=0;
 strcpy(dummy,s);

 if (*ins==1)
   {
    setviewport(0,0,639,349,0);
    write_text(CYAN,488,336,3,10,LIGHTCYAN,"INS");
   }
 if (MASKEDIT!=0) *ins=0;

 setviewport(x,y,x+(length*8),y+10,0);
 clearviewport();

 write_text(LIGHTGRAY,0,0,length,10,LIGHTGREEN,s);
 setcolor(YELLOW);
 line(letternum*8,10,(letternum+1)*8,10);

 while (!select)
      {
       c=getch();

       if (c==ENTER) select=1;
       else if (c==ESC) {strcpy(s,dummy);select=1;}
       else if ((c > 31 && c < 123 && MASKEDIT==0) || (c>47 && c<58 && MASKEDIT==1)
	    || ( ((c>47 && c<58) || c=='+' || c=='-' || c=='.') && MASKEDIT==2))
	 {
	  if ((!(*ins)) && (letternum<length))
	    {
             setcolor(LIGHTGRAY);
	     line(letternum*8,10,(letternum+1)*8,10);
	     ii=0;
	     s[letternum]=c;
	     letternum++;
	     write_text(LIGHTGRAY,0,0,length,10,LIGHTGREEN,s);
	     if (letternum==length) letternum--;
	     setcolor(YELLOW);
	     line(letternum*8,10,(letternum+1)*8,10);
	    }
	  else
	    {
	     if ((letternum<length) && (strlen(s)+1<=length))
	       {
		setcolor(LIGHTGRAY);
		line(letternum*8,10,(letternum+1)*8,10);
		ii=0;
		for (j=letternum;j<length;j++)
		   {ch[ii]=s[j];ii++;}
		ch[ii]='\0';
		s[letternum]=c;
		s[letternum+1]='\0';
		strcat(s,ch);
		letternum++;
		write_text(LIGHTGRAY,0,0,length,10,LIGHTGREEN,s);
		if (letternum==length) letternum--;
		setcolor(YELLOW);
		line(letternum*8,10,(letternum+1)*8,10);
	       }
	     else
	       {printf("\a");}
	    }
	 }

	else if ((c == BCKSPACE)  && (letternum > 0))
	  {
	    setcolor(LIGHTGRAY);
	    line(letternum*8,10,(letternum+1)*8,10);
	    ii=0;
	    for (j=letternum;j<length;j++)
	       {
		 ch[ii]=s[j];ii++;
	       }
	    ch[ii]='\0';
	    letternum--;
	    s[letternum]='\0';
	    strcat(s,ch);
	    write_text(LIGHTGRAY,0,0,length,10,LIGHTGREEN,s);
	    setcolor(YELLOW);
	    line(letternum*8,10,(letternum+1)*8,10);
	  }
	else if (c==0)
	    {
	     c=getch();
	     if (c==INS && MASKEDIT==0)
	       {
		if (!(*ins))
		  {
		   *ins=1;
		   setviewport(0,0,639,349,0);
		   write_text(CYAN,488,336,3,10,LIGHTCYAN,"INS");
		   setviewport(x,y,x+(length*8),y+10,0);
		  }
		else
		  {
		   *ins=0;
		   setviewport(0,0,639,349,0);
		   write_text(BLACK,488,336,3,10,BLACK,"INS");
		   setviewport(x,y,x+(length*8),y+10,0);
		  }
	       }
	     else if (c == DEL)
	       {
		 setcolor(LIGHTGRAY);
		 line(letternum*8,10,(letternum+1)*8,10);
		 ii=0;
		 for (j=letternum+1;j<length;j++)
		    {
		     ch[ii]=s[j];ii++;
		    }
		 ch[ii]='\0';
		 s[letternum]='\0';
		 strcat(s,ch);
		 write_text(LIGHTGRAY,0,0,length,10,LIGHTGREEN,s);
		 setcolor(YELLOW);
		 line(letternum*8,10,(letternum+1)*8,10);
	       }
	     else if ((c == LEFT) && (letternum > 0))
	       {
		 setcolor(LIGHTGRAY);
		 line(letternum*8,10,(letternum+1)*8,10);
		 letternum--;
		 setcolor(YELLOW);
		 line(letternum*8,10,(letternum+1)*8,10);
	       }
	     else if ((c == RIGHT) && (letternum+1<length))
	       {
		  setcolor(LIGHTGRAY);
		  line(letternum*8,10,(letternum+1)*8,10);
		  letternum++;
		  setcolor(YELLOW);
		  line(letternum*8,10,(letternum+1)*8,10);
	       }
	     else if (c==71)
	      {setcolor(LIGHTGRAY);
	       line(letternum*8,10,(letternum+1)*8,10);
	       letternum=0;
	       setcolor(YELLOW);
	       line(letternum*8,10,(letternum+1)*8,10);
	       }
	      else if (c==79)
	      {setcolor(LIGHTGRAY);
	       line(letternum*8,10,(letternum+1)*8,10);
	       letternum=strlen(s);
	       if (letternum==length) letternum=letternum-1;
	       setcolor(YELLOW);
	       line(letternum*8,10,(letternum+1)*8,10);
	       }
	     else printf("\a");
	    }
	  else printf("\a");
	}
 setfillstyle(SOLID_FILL,BLACK);
 bar(-3,-3,length*8,10);
 setcolor(LIGHTRED);
 outtextxy(0,0,s);
 setcolor(BLACK);
 line(letternum*8,10,(letternum+1)*8,10);
}

/****************************************************************************/
