/***************************************************************************/
/*                                                                         */
/* File name  : OMAIVFF                                                    */
/* Author     : David DiPrato                                              */
/* Version    : 1.00 - Initial version.                                    */
/* Description: This program will run the OMAIV CCD detectors in full-frame*/
/*    mode with a live DA mode.  It will display data, autoscale data,     */
/*    adjust the exposure time, display (X,Y,Z) info., save data to disk   */
/*    in EG&G's UIFF and print image to a Postscript printer.  OMACNTLR    */
/*    information is retrieved from environment variables or command line  */
/*    arguments.                                                           */
/*                                                                         */
/*    Note: The OMA Controller board MUST have enough memory for a         */
/*          full-frame image.                                              */
/*                                                                         */
/***************************************************************************/

#include <dos.h>
#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "oma4driv.h"
#include "driverrs.h"
#include "rmint86.h"

#include "image.h"

#define XDELTA  1
#define YDELTA  1

/* Local definitions *******************************************************/

/* Default command values -------------------------------------------------*/
enum yesno {NO = 0, YES };

/* General control variables -----------------------------------------------*/
enum {IDLE_CMD,START_CMD,STOP_CMD,SET_ET_CMD,EXIT_CMD,ERROR_CMD} Command;
SHORT RunFlag = 0;
SHORT DisplayImageFlag = 0;
float ExposureTime = 0.100F;
SHORT DisplayXLeft = 10;
SHORT DisplayXRight = 300;
SHORT DisplayYTop = 10;
SHORT DisplayYBottom = 300;

typedef struct {              
   char label[20];
   enum flt_command tag;
   float value;
   enum yesno Canset;
   } CMDBLOCK;

char * dtypes[] = {"No Detector",
                 "Thompson 512 X 512 CCD",
                 "Thompson Dual Channel CCD",
                 "EEV 1024 X 256 CCD",
                 "RAPDA 1024 X 1",
                 "Thompson 1024 X 256 CCD",
                 "Thompson 1024 X 1024 CCD",
                 "Unknown Device" };

CMDBLOCK commands[] ={
                   {"ACTIVEX",     DC_ACTIVEX,     512.000F, YES }, /* 02 */
                   {"ACTIVEY",     DC_ACTIVEY,     512.000F, YES }, /* 03 */
                   {"ANTIBLOOM",   DC_ANTIBLOOM,    20.000F, YES }, /* 06 */
                   {"COOLONOFF",   DC_COOLONOFF,   001.000F, YES }, /* 11 */
                   {"CONTROL",     DC_CONTROL,     001.000F, NO  }, /* 13 */
                   {"DAPROG",      DC_DAPROG,      001.000F, YES }, /* 15 */
                   {"DELTAX",      DC_DELTAX,      001.000F, YES }, /* 16 */
                   {"DELTAY",      DC_DELTAY,      001.000F, YES }, /* 17 */
                   {"DETPORT",     DC_DETPORT,     768.000F, NO  },
                   {"DTEMP",       DC_DTEMP,       -20.000F, YES }, /* 20 */
                   {"EXPROWS",     DC_EXPROWS,     001.000F, NO, }, /* 22 */
                   {"H",           DC_H,           000.000F, YES }, /* 25 */
                   {"I",           DC_I,           001.000F, YES }, /* 27 */
                   {"IMODE",       DC_IMODE,       001.000F, YES }, /* 29 */
                   {"J",           DC_J,           001.000F, YES }, /* 31 */
                   {"K",           DC_K,           000.000F, YES }, /* 32 */
                   {"MEM",         DC_MEM,         001.000F, NO  }, /* 35 */
                   {"OUTREG",      DC_OUTREG,      000.000F, YES }, /* 38 */
                   {"OUTPIA",      DC_OUTPIA,      001.000F, NO  }, /* 39 */
                   {"PAUDIO",      DC_PAUDIO,      001.000F, YES }, /* 40 */
                   {"PLSR",        DC_PLSR,        001.000F, NO  }, /* 41 */
                   {"PNTMODE",     DC_PNTMODE,     000.000F, YES }, /* 42 */
                   {"POINTS",      DC_POINTS,      512.000F, YES }, /* 44 */
                   {"PRESCAN",     DC_PRESCAN,     000.000F, YES }, /* 45 */
                   {"PTIME",       DC_PTIME,       001.000F, YES }, /* 46 */
                   {"PTRIG",       DC_PTRIG,       000.000F, YES }, /* 47 */
                   {"PTRIGNUM",    DC_PTRIGNUM,    001.000F, YES }, /* 48 */
                   {"PTRIGSRC",    DC_PTRIGSRC,    001.000F, YES }, /* 49 */
                   {"PTRIGTRSH",   DC_PTRIGTRSH,   149.000F, YES }, /* 50 */
                   {"SAMEET",      DC_SAMEET,      000.000F, YES }, /* 54 */
                   {"SCITC",       DC_SCITC,       001.000F, NO  }, /* 55 */
                   {"SHFTMODE",    DC_SHFTMODE,    000.000F, YES }, /* 56 */
                   {"SHUTMODE",    DC_SHUTMODE,    000.000F, YES }, /* 57 */
                   {"STIME",       DC_STIME,       000.000F, YES }, /* 60 */
                   {"STREAKMODE",  DC_STREAKMODE,  000.000F, YES }, /* 62 */
                   {"TRACKS",      DC_TRACKS,      512.000F, YES }, /* 66 */
                   {"TRIGS",       DC_TRIGS,       000.000F, YES }, /* 67 */
                   {"TRKMODE",     DC_TRKMODE,     000.000F, YES }, /* 68 */
                   {"WFTC",        DC_WFTC,        000.000F, YES }, /* 69 */
                   {"WFTO",        DC_WFTO,        000.000F, YES }, /* 70 */
                   {"X0",          DC_X0,          001.000F, YES }, /* 71 */
                   {"Y0",          DC_Y0,          001.000F, YES }, /* 72 */
                   {"ET",          DC_ET,          000.100F, YES }, /* 73 */
                   {"PDELAY",      DC_PDELAY,      000.000F, YES }, /* 77 */
                   {"PDELINC",     DC_PDELINC,     000.000F, YES }, /* 78 */
                   {"PDELRANGE",   DC_PDELRANGE,   000.000F, NO  }, /* 79 */
                   {"PWIDTH",      DC_PWIDTH,      000.010F, YES }, /* 80 */
                   {"",            0,              0.0F,     NO  }
                  };

/* Local function prototypes ***********************************************/
/* Setup the OMA Controller information from envirnorment variables or
   command line values.
****************************************************************************/
SHORT SetupOMACNTLR(SHORT ArgCnt,char **ArgV,ULONG *Address,USHORT *Port);

/* Send all the current settings to the detector.
****************************************************************************/
void ProgramDetector(void);

/* Send a specific setting in the parameter table.  It will return a 
   non-zero if the specific setting was not found.
****************************************************************************/
SHORT AdjParamTable(char *Label, float Value);

/* Report a error condition to the user via. a tone.
****************************************************************************/
void ErrorFound(void);

/* Update specific parameters that depend on the detector type.  A non-zero
   will return on error.
****************************************************************************/
SHORT AdjSpecifics(USHORT Port);

/* Check to see if the detector is busy (runnning).  It will return a
   non-zero if the detector is idle (not running).
****************************************************************************/
SHORT DetectorNotRunning(void);

/* This function is the main loop.
****************************************************************************/
SHORT DoMainLoop(void);

/* This function will check the user for input and adjust the global
   'command' varible as well as the 'exposuretime'.
****************************************************************************/
void CheckUserInput(void);

/* This function will read the global varible 'command' and control
   the detector as instructed.  It will return when the action is complete.
****************************************************************************/
void ControlDetector(void);

/* This function will continue to run the detector, repeating single
   memory operation to emulate live mode if the 'RunFlag' set set.

****************************************************************************/
void RunLive(void);

/* This function will update the display image if the 'RunFlag' is set
   and the detector has finished a scan.
****************************************************************************/
void UpdateDisplay(void);

/* Displays the program, version and user instructions.
****************************************************************************/
void ShowHeader(void);

void ShowPromptLine(void);

/* Functions follow: *******************************************************/

SHORT main(SHORT argc, char **argv) 
{
  ULONG Address;
  SHORT err;
  USHORT Port;

  setvbuf(stdout, NULL, _IONBF, 0);
  ShowHeader();

  /* Setup OMA Controller board ------------------------------------------*/
  SetupOMACNTLR(argc,argv,&Address,&Port);
  setup_detector_interface(Port, Address, 0x200000L);

  /* Setup OMA Driver ----------------------------------------------------*/
  if (AdjSpecifics(Port))
    {
    ErrorFound();
    printf("ERROR: Check detector's power and connections.\n");
    if (!getch())
      getch();
    }
  ProgramDetector();

  /* Open graphic system -------------------------------------------------*/
  if (err = OpenImageDisplay())
    {
    ErrorFound();
    printf("ERROR: %d, Install a VESA video driver.\n", err);
    return -1;
    }
  if (OpenImageWindow(DisplayXLeft,DisplayXRight,DisplayYTop,DisplayYBottom))
     ErrorFound();
  Command = IDLE_CMD;

  ShowPromptLine();

  /* Main loop, wait for user input... -----------------------------------*/
  do
    DoMainLoop();
  while (Command != EXIT_CMD);

  /* Exit gracefully -----------------------------------------------------*/
  CloseImageWindow();
  CloseImageDisplay();
  SetParam(DC_STOP, 0);
  return(0);
}


/* This function is the main loop.
****************************************************************************/
SHORT DoMainLoop(void)
{
  CheckUserInput();
  ControlDetector();
  UpdateDisplay();
  RunLive();
  
  return(0);
}

/* This function will check the user for input and adjust the global
   'command' varible as well as the 'exposuretime'.
****************************************************************************/
void CheckUserInput(void)
{
  char Cmd = 0, CmdString[20];
  float ExpTime;
  SHORT i = 0;

  if (kbhit())
    {
    Cmd = (char)getch();

    switch (Cmd)
      {
      case 'e':
      case 'E':
        Command = EXIT_CMD;
      break;
      case 'r':
      case 'R':
        Command = START_CMD;
      break;
      case 's':
      case 'S':
        Command = STOP_CMD;
      break;
      case ' ':
        AutoScaleImage();
        UpdateImageWindow();
      break;
      case 0 :                   /* Extended key stroke */
        Cmd = (char)getch();      
        switch (Cmd)
          {
          case 80 :                  /* Down Arrow */
            IncWindowY();
          break;
          case 72 :                  /* Up Arrow   */
            DecWindowY();
          break;
          case 77 :                  /* Left arrow */
            IncWindowX();
          break;
          case 75 :                  /* Right arrow*/
            DecWindowX();
          break;
          }
      break;
      default :
        SetCursorToLine(2);
        printf("Exposure Time:                   ");
        SetCursorToLine(2);
        printf("Exposure Time: %c", Cmd);
        CmdString[i++] = Cmd;            /* Get 1st character.      */
        while (Cmd != '\r' && i < 18)    /* Build exp time string.  */
          {
          if (kbhit())
            {                    
            Cmd = (char)getch();      
            CmdString[i++] = Cmd;
            printf("%c",Cmd);
            }
          } 
        CmdString[i - 1] = 0;

        if ((ExpTime = (float)atof(CmdString)) != 0)
          {
          Command = SET_ET_CMD;
          ExposureTime = ExpTime;
          }
        else Command = ERROR_CMD;
      SetCursorToLine(2);
      printf("ET: %4.4f                   ", ExposureTime);
      break;
      }
    }
}

/* This function will read the global varible 'command' and control
   the detector as instructed.  It will return when the action is complete.
****************************************************************************/
void ControlDetector(void)
{
  switch (Command)
    {
    case IDLE_CMD :
       break;

    case START_CMD :
      SetParam(DC_RUN, 2.0F);
      RunFlag = 1;
      DisplayImageFlag = 0;
      Command = IDLE_CMD;
      break;

    case STOP_CMD :
      SetParam(DC_STOP, 0.0F);
      RunFlag = 0;
      Command = IDLE_CMD;
      break;

    case SET_ET_CMD :
      if (SetParam(DC_ET,ExposureTime)) ErrorFound();
      Command = IDLE_CMD;
      break;

    case EXIT_CMD :
      break;

    case ERROR_CMD :
      Command = IDLE_CMD;
      break;
    }
}

/* This function will continue to run the detector, repeating single
   memory operation to emulate live mode if the 'RunFlag' set set.
****************************************************************************/
void RunLive(void)
{
  if (RunFlag)
    {
    if (DetectorNotRunning())
      {
      SetParam(DC_RUN, 2.0F);                 /* Restart detector.             */
      DisplayImageFlag = 1;
      }
    }
}


/* Check to see if the detector is busy (runnning).  It will return a
   non-zero if the detector is idle (not running).
****************************************************************************/
SHORT DetectorNotRunning(void)
{
  float ActiveFlag;

  GetParam(DC_ACTIVE,&ActiveFlag);
  return(ActiveFlag==0.0F);
}


/* This function will update the display image if the 'RunFlag' is set
   and the detector has finished a scan.
****************************************************************************/
void UpdateDisplay(void)
{
  if (RunFlag)
    {
    if (DisplayImageFlag) 
      {
      UpdateImageWindow();
      DisplayImageFlag = 0;
      }
    }
}


/* Update specific parameters that depend on the detector type.  A non-zero
   will return on error.
****************************************************************************/
SHORT AdjSpecifics(USHORT Port)
{
  float ActiveX;
  float ActiveY;
  float NumTracks;
  float Points;
  float Temp = -20.00F;
  float Detector;
  SHORT ReturnValue = 0;

  /* Get detector type and case to specific detector ---------------------*/
  GetParam(DC_DMODEL,&Detector);
  GetParam(DC_ACTIVEX,&ActiveX);
  GetParam(DC_ACTIVEY,&ActiveY);
  NumTracks = ActiveY / (float) YDELTA;
  Points = ActiveX / (float) XDELTA;

  printf("Detector is %s\n", dtypes[(SHORT)Detector]);

  switch ((SHORT)Detector)
    {
    case 0:                       /* Unrecognized detector               */
       ReturnValue = -1;

    case 1:                       /* Thomson 512x512                     */
       /* Adjust global values for display location ---------------------*/
       DisplayXLeft = 10;
       DisplayXRight = 310;
       DisplayYTop = 10;
       DisplayYBottom = 310;
       break;

    case 2:                       /* Thomson 512x512 Split-mode          */
       ReturnValue = -4;
       break;

    case 3:                       /* EEV 1024x256                        */
       /* Adjust global values for display location ---------------------*/
       DisplayXLeft = 10;
       DisplayXRight = 510;
       DisplayYTop = 10;
       DisplayYBottom = 135;
       break;

    case 4:                       /* Reticon RPDA                        */
       ReturnValue = -2;
       break;

    case 5:                       /* Thomson 1024x256                    */
       /* Adjust global values for display location ---------------------*/
       DisplayXLeft = 10;
       DisplayXRight = 510;
       DisplayYTop = 10;
       DisplayYBottom = 135;
       break;

    default:                      /* Unrecognized detector               */
       ReturnValue = -3;
       break;
    }

  /* Find cooler type, cyro or TE ----------------------------------------*/
  GetParam(DC_COOLTYPE,&Detector);
  switch ((SHORT)Detector)
    {
    case 0:                       /* Can not identify.                   */
       ReturnValue = -10;
       break;
    case 1:                       /* Cryogenic cooled.                   */
       Temp = -100;
       break;
    case 2:                       /* Thermoelectric.                     */
       Temp = -40;
       break;
    default:                      /* Unidentified.                       */
       ReturnValue = -11;
       break;   
    }

  /* Adjust the specific detector values ---------------------------------*/
  if (AdjParamTable("DETPORT", (float) Port)) ReturnValue = -5;
  if (AdjParamTable("ACTIVEX", ActiveX)) ReturnValue = -6;
  if (AdjParamTable("ACTIVEY", ActiveY)) ReturnValue = -7;
  if (AdjParamTable("TRACKS", NumTracks)) ReturnValue = -8;
  if (AdjParamTable("POINTS", Points)) ReturnValue = -9;
  if (AdjParamTable("DTEMP", Temp)) ReturnValue = -10;

  return(ReturnValue);
}


/* Setup the OMA Controller information from envirnorment variables or
   command line values.
****************************************************************************/
SHORT SetupOMACNTLR(SHORT ArgCnt,char **ArgV,ULONG *Address,USHORT *Port)
{
   char *endptr;
   SHORT i;

   /* Set default values --------------------------------------------------*/
   *Port = 0x300; 
   *Address = 0x200000L;

   /* Get values from envirornment ----------------------------------------*/
   if ((endptr = getenv("OMAADDR")) != NULL) 
      *Address = strtoul(endptr, NULL, 16);
   if ((endptr = getenv("OMAPORT")) != NULL) 
      *Port = (USHORT)strtoul(endptr, NULL, 16);

   /* Get values from command line ----------------------------------------*/
   for (i = 1;i < ArgCnt; i++) {
      if (ArgV[i][0] == 'A' || ArgV[i][0] == 'a')
         *Address = strtoul(&(ArgV[i][1]), &endptr, 16);
      else if (ArgV[i][0] == 'P' || ArgV[i][0] == 'p')
         *Port = (USHORT)strtoul(&(ArgV[i][1]), &endptr, 16);
      }

   return(0);
}


/* Send all the current settings to the detector.
****************************************************************************/
void ProgramDetector(void)
{
   USHORT i;

   for (i = 0;commands[i].label[0] != '\0'; i++) {
      if (commands[i].Canset == YES) {
         if (SetParam(commands[i].tag, commands[i].value)) {
            ErrorFound();
            }
#ifdef DEBUG
         else {
            GetParam(commands[i].tag,&Reply);
            printf("%s , %f",commands[i].label,Reply);
            }        
#endif
         }
      }
}

/* Send a specific setting in the parameter table.  It will return a 
   non-zero if the specific setting was not found.
****************************************************************************/
SHORT AdjParamTable(char *Label, float Value)
{
  SHORT i,ReturnVal = 1;

  for (i = 0;commands[i].label[0] != '\0'; i++)
    {
    if (!strcmpi(Label,commands[i].label))
      {
      commands[i].value = Value;
      ReturnVal = 0;
      }
    }
  return(ReturnVal);
}

/* Report a error condition to the user via. a tone.
****************************************************************************/
void ErrorFound(void)
{
  #ifndef _MSC_VER
  sound(1000);
  delay(1000);
  nosound();
  #else
  putchar(0x07);
  #endif
}

/* Displays the program, version and user instructions.
****************************************************************************/
void ShowHeader(void)
{
  printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
  printf("OMAIVFF - OMAIV Full Frame Imaging Software, version 2.00\n\n");
  printf("  R = Run detector  \n");
  printf("  S = Stop detector \n");
  printf("  E = Exit program  \n");
  printf("  <space> = Autoscale and redraw image \n");
  printf("  <number>RETURN = Exposure time in seconds \n");
  printf("  Arrow(s) adjust the window's size \n");
  printf("\n  Press any key to start program...");
  printf("\n\n\n\n\n\n\n\n\n");
  while (!kbhit());
  if (!getch()) getch();
}

/* Displays the program, version and user instructions.
****************************************************************************/
void ShowPromptLine(void)
{
  SetCursorToPrompt();
  fprintf(stdout, "R:Run ³ S:Stop ³ E:Exit ³ <space>:Autoscale"
                  " ³ <number>ENTER:Expose Time ³ ");
  fflush(stdout);

}
