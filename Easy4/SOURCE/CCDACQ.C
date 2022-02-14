/***************************************************************************/
/*                               CCDACQ.C                                  */
/***************************************************************************/

/* Main program for XY driving and data acquisition   */
/*                                                    */
/* MTC 09-92                                          */
/* for EG&G CCD OMA4 1024x256 detector                */
/*                                                    */
/* Micro plate stage option available  MTC 11-92      */
/*                                                    */
/* MTC 03-93                                          */
/* Modifications added for acquisition in UV          */
/* UV mode : COEFFXY.UV,PARAM.UV,CONFIG.UV FRAME.UV   */
/* Standard mode: COEFFXY,PARAM,FRAME,CONFIG          */
/* Selection of the acquisition mode in PEAKS program */
/* Mode transmitted as argument to OMA4P.exe file     */

/*==========================================================================*/

#include <acq4.h>
#include <process.h>
#include <graphics.h>
#include <stdlib.h>
#include <detdriv.h>
#include <access4.h>
#include <counters.h>
#include <conio.h>

int OFFSET_DIOD,shutmode_ind;
int domaine,endacq=0;

extern int change_comnd;

void clean_gpib(void);

/*==========================================================================*/

int main(int argc,char *argv[])
{
 unsigned port;
 unsigned long address;
 int i;
 char *endptr;
 float value;
 for (i = 1;i < argc; i++)
    {
     if (argv[i][0] == 'A' || argv[i][0] == 'a')
       address = strtoul(&(argv[i][1]), &endptr, 16);
     else if (argv[i][0] == 'P' || argv[i][0] == 'p')
	 port = (unsigned)strtoul(&(argv[i][1]), &endptr, 16);
       else if ( strcmp(argv[i],"UV")==0 )
	   domaine=UV;
	else if ( strcmp(argv[i],"Std")==0 )
	    domaine=STD;
    }

 OFFSET_DIOD=0;
 shutmode_ind=1; /* Shutter initially closed */

 setup_detector_interface(port,address,0x200000L);

 init_graph();
 init_adj();

   /* Physical dimension of the array installed in the used CCD */
   /*
    SetParam(ACTIVEX,(float)1024); MTC 17-09-92
    SetParam(ACTIVEY,(float)256);  MTC 17-09-92
    */
    SetParam(ANTIBLOOM,(float)50);

  /*SetParam(SHFTMODE,(float)0 );
    SetParam(OUTREG,(float)1);
   */

    set_frame();
    SetParam(CONTROL,(float)0);
    SetParam(I,1.0);SetParam(J,1.0);SetParam(K,0.0);SetParam(H,1.0);
    SetParam(DAPROG,(float)1);SetParam(CLR,(float)0);
    SetParam(SPEED,(float)0); /* Pixel time normal */

    SetParam(ET,1.0);

    SetParam(SHUTMODE,(float)1);
    SetParam(RUN,(float) 0);
    save_cur_image();change_comnd=0;
    read_oma4_data();   /* For OMA IV 15-10-91  MTC */
    clean_gpib();
    shutmode_ind=1;set_shutter_forced_mode();
 switch(endacq)
    {
     case 0: cleardevice();
	     closegraph();
	     execl("peaks.exe","peaks.exe",argv[1],argv[2],NULL);

     case 1: closegraph();
	     execl("dilsurf.exe","dilsurf.exe",argv[1],argv[2],NULL);
	     break;

     case 2: closegraph();
	     execl("m3d.exe","m3d.exe",argv[1],argv[2],NULL);

    }

 return 0;
}
