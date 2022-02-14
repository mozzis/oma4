/*=======================================================================*/
/*                              GPIB-USE.C                               */
/*=======================================================================*/
/* This file gathers all the functions for driving the foremonochromator */
/* and the spectrograph using the IEEE interface.                        */
/* You must link the TCIB*.OBJ libraries to your project in order to use */
/* the driver functions. * is related to the memory model you use.       */
/* Be careful with the case sensitive link option (upper/lower case)     */
/*=======================================================================*/

			 /*  Headers        :                 */
#include <acq4.h>         /*  Header for easy_peak defines.    */
#include <ctype.h>       /*  Function isdigit()               */
#include <decl0.h>       /*  Header for gpib defines.         */
#include <conio.h>       /*  Function kbhit()                 */
#include <math.h>        /*  Header for fabs function.        */
#include <stdlib.h>
#include <stdio.h>
			     /* Format    :    10xxx000       */
			     /* bit nø3 = 1, nø5 = 0 to move  */
#define STOP_MOTOR    0x28   /* STOP_MOTOR mask: bit nø3 = 0, */
			     /*		             nø5 = 1  */

#define POS_DEC_VIT  5       /* Speed decreases from          */
			     /* delta=POS_DEC_VIT.            */

#define V_MAX       62       /* V_MAX : maximum speed  .             */
#define V_MOY       44       /* V_MOY : mean speed .                 */
#define V_MIN       28       /* V_MIN : minimum speed.               */
#define OFF_MOTOR   7.0      /* offset to position in the same way . */

extern int numspeed;

extern char MOV[];
/*=======================================================================*/

int  bd;     /* identifier assigned to  GPIB0.       */

unsigned char DIREC;         /* DIREC : direction of the motion      */


/*=======================================================================*/

void gpib_init(void)

{
 int ch[4];
 ch[0]=2;
 ch[1]=3;
 ch[2]=2;
 if ((bd=ibfind("GPIB0")) < 0)
    printf("ERROR : Unable to find IEEE interface\n");
 if (ibsic(bd) & ERR)
    printf("interface unclear\n");

 ibcmd(bd,"?_)@",4);
 ibwrt(bd,ch,3);
 ibcmd(bd,"?_",2);
}
/***************************************************************************/
void clean_gpib(void)
{
 ibsic(bd);
 ibsre(bd,0);
}
/***************************************************************************/

float read_position(int *module)
{
 int  signe,
      i=0,
      end=6,      /* last siginficative byte     */
      digit;

 long londe;     /* londe : wavelength */

 unsigned char   long_onde[7];  /* long_onde : received string */

 /*ibcmd(bd,DCL,1);*/

 switch (*module)
       {
	case FOREMONO : ibcmd(bd,"_?K ",4); /*UNT,UNL,MTA11 (foremono),MLA0*/
			break;

	case SPECTRO  : ibcmd(bd,"_?L ",4); /*UNT,UNL,MTA12 (spectro),MLA0 */
			break;
       }

 ibrd(bd,long_onde,6);  /* 6 bytes read and stored in long_onde[]   */
 ibcmd(bd,"_?",2);

 digit = 15-(long_onde[0]/16);            /* calculates 1st digit    */
 i = ( isdigit(digit) ) ? 0 : 1;          /* erased if not valid     */
 if (i == 0) end=5;
 for (londe=0;i<end;i++)	 	        /* calculates the value    */
      londe=(londe*10)+15-(long_onde[i]/16);    /* of the string           */

 signe = ((long_onde[2] & 0x000F)<8) ? 1 : -1;	/* test the sign of delta  */
 return(londe*signe/10.0);                      /* with the 4 LSB          */
}

/***************************************************************************/
char speed(float delta)       /* calculates the speed   + -> -    */

{
 char cc;

 if (delta > POS_DEC_VIT)
    cc=V_MAX;
 else if ((delta <= POS_DEC_VIT) && (delta >= 0.5))
    cc=V_MOY;
 else if ((delta < 0.5) && (delta >= 0.05))
    cc=V_MIN;
 else if (delta < 0.05)
    cc=0;
 return(cc);
}

/**************************************************************************/
int positionning(float *position,int *module)

{
 char   vitesse,c=0;     /* vitesse : speed of the stepping motor   */
 float  delta;           /* delta : desired position - position     */
 int stop_pos=0;

 delta=*position-read_position(module);
 if (delta < 0)
    while ((delta=*position-read_position(module)) < OFF_MOTOR)
	 {
	  if (kbhit()) c=getch();
	  if (c == SPACE) {stop_pos=1;break;}
	  vitesse=speed(fabs(delta)+OFF_MOTOR);
	  DIREC=moveb(vitesse,module);
	 }
 if (delta > 0)
    while ((delta=*position-read_position(module)) >= 0.05)
	 {
	  if (kbhit()) c=getch();
	  if (c == SPACE) break;
	  vitesse=speed(delta);
	  DIREC=movef(vitesse,module);
	 }
 stop(module);
 return(stop_pos);
}

/**************************************************************************/

int bpositionning(float *position)
/* when moving foremono and spectro   */

{
 char   vitesse,c=0;
 float  delta;
 int    modu=0,*module,
	test=0,
	flag=0,
	pos[2]={0,0},
	activ_module[2]={0,0},
	sens[2],
	arret=0;

 module=&modu;
 if ((delta=*position-read_position(module)) < -0.05)
   {
    *position-=OFF_MOTOR;
    test=1;
   }
 do
   {
    delta=*position-read_position(module);

    if (delta < -0.05)
      {
       vitesse=speed(fabs(delta));
       DIREC=moveb(vitesse,module);
       activ_module[*module]=1;
       sens[*module]=DIREC;
      }
    if (delta > 0.05)
      {
       vitesse=speed(delta);
       DIREC=movef(vitesse,module);
       activ_module[*module]=1;
       sens[*module]=DIREC;
      }
    if ((delta > -0.05) && (delta < 0.05))
      {
       flag++;
       stop(module);
       activ_module[*module]=0;
       pos[*module]=1;
      }
    if (pos[*module^1] == 0) *module^=1;
    if (kbhit()) c=getch();
    if (c == SPACE)
      {
       arret=1;
       break;
      }
   }
    while (flag != 2);

 if (arret==1)
   {
    if (activ_module[*module]==1)
      {
       DIREC=sens[*module];
       stop(module);
      }
    *module^=1;
    if (activ_module[*module]==1)
      {
       DIREC=sens[*module];
       stop(module);
      }
   }
 else
   {
    if (test == 1)
      {
       *position+=OFF_MOTOR;
       bpositionning(position);
      }
   }
 return(arret);
}

/******************************************************************************/

char moveb(char vitesse,int *module)
/* move the motor backward */

{
 char commande[5]={97,0,131,BACKWARD};

 commande[1]=vitesse;     /* set the speed            */
 /*ibcmd(bd,DCL,1);*/         /* interface clear          */
 /*ibsic(bd);*/
 switch (*module)

       {
	case FOREMONO : ibcmd(bd,"?_+@",4);  /* UNL,UNT,MLA11,MTA0 */
			break;               /*                    */

	case SPECTRO  : ibcmd(bd,"?_,@",4);  /* UNL,UNT,MLA12,MTA0 */
			break;               /*                    */
       }
 ibwrt(bd,commande,4);
 ibcmd(bd,"?_",2);     /* UNL,UNT  */
 return(BACKWARD);
}

/**************************************************************************/

char movef(char vitesse,int *module)

{
 char commande[5]={97,0,131,FORWARD};

 commande[1]=vitesse;
 /*ibcmd(bd,DCL,1);*/
 /*ibsic(bd);*/

 switch (*module)
       {
	case FOREMONO : ibcmd(bd,"?_+@",4);
			break;
	case SPECTRO  : ibcmd(bd,"?_,@",4);
			break;
       }
 ibwrt(bd,commande,4);
 ibcmd(bd,"?_",2);
 return(FORWARD);
}

/**************************************************************************/

void stop(int *module)
/* Stop the motor  */

{
 char commande[5]={97,0,131,0};

 commande[3]=DIREC ^ STOP_MOTOR;
 commande[1]=MOV[numspeed];
 /*ibcmd(bd,DCL,1); */              /* interface clear                       */
 switch (*module)
       {
	case FOREMONO : ibcmd(bd,"?_+@",4);  /* UNL,UNT,MLA11,MTA0       */
			break;

	case SPECTRO  : ibcmd(bd,"?_,@",4);  /* UNL,UNT,MLA12,MTA0       */
			break;
       }
 ibwrt(bd,commande,4);  /* stop  */
}

/**************************************************************************/
