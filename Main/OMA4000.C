/*
/
/  oma4000.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/oma4000.c_v   0.60   07 Jul 1992 17:10:36   maynard  $
/  $Log:   J:/logfiles/oma4000/main/oma4000.c_v  $
/  Changes:
 3.01 - Error in Ascii output fixed.
 3.02 - Fast shift mode correctly implemented .
 3.03 - Correction to Acton support.
 3.04 - Add radiometric calibration menu.
 3.05 - Streaker menu first pass.
 3.06 - Change Timeout when starting acquisition.
      - Fix SaveFileCurves so description comes from source file.
      - Change timing in mmscans - initial TP in data track.
        changed to NPAF to correct fix pattern noise problem.
 3.07 - Live Filters added.
 3.08 - Driver: bug fix in RUN cmd, ASIC program always updated.
      -         CommonArea and DAC value addresses declared volatile.
 4.0  - Release 8-30-94.
      - Add Now(), Wait(), Comm routines to macros.
 4.01 - Add DG_FTIME, DG_L, DS_L macro commands.
 4.02 - Change in trash lead lines to decrease frame time in some cases.
        If exposure time is 0 and shutter is forced open and antibloom is 0,
        then the leading trash lines are combined with the trailing trash
        lines from the previous frame, instead of being trashed separately.
 4.03 - Fixes for 1024 X 1024 - .abj and .dbj files were not loading, 
        grouping in serial register was done incorrectly.
      - Add plot line color and hidden surface selections.
      - Fix bug in livedisk, curve header not initialized before writing.
 4.04 - Release 11-14-94
      - Crash happened if method file written without detector info block.
      - Crash if program started with no spectrograph or GPIB and one was
        previously selected.
 4.05 - Add another plot line color option - now can select "mark 5th line"
        or "cycle colors" in addition to "all lines white".
 4.06 - Tweaks to 1K X 1K with DeltaX > 1 in random mode
 4.07 - Add Support for Acton SP-150 (EG&G "1239") spectrograph
      - Add support for serial communication with spectrograph
 4.08 - Add capability to address upper 2 Mb of 16 Mb ISA address space
 4.09 - Fix error in rapda exit form function
/
*/

const char VersionString[] = "4.09";

#include <io.h>
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <process.h>
#include <stdlib.h>
#include <malloc.h>   // _amblksiz
#include <dos.h>      // _enable
#include <float.h>    // _fpreset

#include "oma4000.h"
#include "macruntm.h"
#include "tempdata.h"
#include "di_util.h"
#include "filemsg.h"
#include "oma4driv.h"
#include "detsetup.h"
#include "gpibcom.h"
#include "spgraph.h"
#include "curvdraw.h"
#include "device.h"
#include "live.h"
#include "multi.h"
#include "fkeyfunc.h"
#include "graphops.h"
#include "omaform.h"
#include "omameth.h"  // MethodToDetInfo
#include "detinfo.h"
#include "cursor.h"
#include "macrecor.h"
#include "grafmous.h"
#include "baslnsub.h"
#include "calib.h"
#include "coolstat.h"
#include "filestuf.h"
#include "livedisk.h"
#include "runforms.h"
#include "pltsetup.h"
#include "omamenu.h"
#include "syserror.h"  // ERROR_OPEN
#include "omaerror.h"
#include "omazoom.h"
#include "formtabs.h"
#include "basepath.h"
#include "plotbox.h"
#include "curvedir.h"
#include "curvbufr.h"
#include "forms.h"
#include "cache386.h"
#include "fileform.h" // set_dataspec
#include "splitfrm.h"
#include "macrores.h"

#ifdef USE_D16M
   #ifndef DOS16_INCLUDED
      #include <dos16.h>
   #endif
#endif

#ifdef XSTAT
   #define PRIVATE
#else
   #define PRIVATE static
#endif

PRIVATE jmp_buf reentry_context;

/*-----------------------------------------------------------------------*/

PRIVATE PCHAR LiveFileStr = "LIVE.DAT";
PRIVATE PCHAR AccumFileStr = "ACCUM.DAT";
PRIVATE char  LiveDiskFileStr[] = "LIVEDISK.DAT";
PRIVATE char  AutoMacroName[128] = "AUTOEXEC.PAS";

// ------------------------------------------------------------------------
void _interrupt divide_by_zero_error_trap(unsigned _es,
             unsigned _ds, unsigned _di, unsigned _si,
             unsigned _bp, unsigned _sp, unsigned _bx,
             unsigned _dx, unsigned _cx, unsigned _ax,
             unsigned _ip, unsigned _cs, unsigned _flags)
{
  _enable();
  longjmp(reentry_context, 1);
}

// -----------------------------------------------------------------------
void control_C_trap()
{
   /* any code to reset, kill outstanding I/O, and */
   /* restabilize program will go here... */

  _enable();
  longjmp(reentry_context, 2);
}

// ------------------------------------------------------------------------
void floating_point_error_trap()
{
  _enable();
  _fpreset();
  longjmp(reentry_context, 3);
}

// -----------------------------------------------------------------------
#ifdef DEBUG
void heapdump(void)
{
   struct _heapinfo hinfo;
   int heapstatus;

   hinfo._pentry = NULL;
   while ((heapstatus = _heapwalk(&hinfo)) == _HEAPOK)
   {
      printf("%6s block at %p of size %4.4Xx\n",
      (hinfo._useflag == _USEDENTRY ? "USED" : "FREE"),
      hinfo._pentry, hinfo._size);
   }

   switch (heapstatus)
   {
      case _HEAPEMPTY:
         printf("OK - empty heap\n");
         break;

      case _HEAPEND:
         printf("OK - end of heap\n");
         break;

      case _HEAPBADPTR:
         printf("ERROR - bad pointer to heap\n");
         break;

      case _HEAPBADBEGIN:
         printf("ERROR - bad start of heap\n");
         break;

      case _HEAPBADNODE:
         printf("ERROR - bad node in heap\n");
         break;
   }

   printf("available heap = %Xx\n", _memavl());
   printf("max = %u\n\n\n", _memmax());
}

#endif

int remove_configuration (void);

// -----------------------------------------------------------------------
PRIVATE void close_GSS_workstation(void)
{
  CEnterCTextMode(screen_handle); /* enter cursor mode */
  CCloseWorkstation(screen_handle);
  remove_configuration();
}

// ------------------------------------------------------------------------
PRIVATE void get_environmentals(USHORT *port_addr, ULONG *phys_addr,
                                ULONG *size_addr)
{
  char  * env_param, * dummy;

  if (!(*phys_addr))
    if ((env_param = getenv("OMAADDR")) != NULL)   // base address
      *phys_addr = strtoul(env_param, &dummy, 16);

  if (!(*port_addr))
    if ((env_param = getenv("OMAPORT")) != NULL)   // port address
      *port_addr = (int)strtoul(env_param, &dummy, 16);

  if (!(*size_addr))
    if ((env_param = getenv("OMASIZE")) != NULL)   // memory size
      *size_addr = (strtoul(env_param, &dummy, 16));

  if ((env_param = getenv("OMADATA")) != NULL)     // data directory
    set_dataspec(env_param);

  if ((env_param = getenv("OMAUPSHIFT")) != NULL)  // data directory
    SetParam(DC_THERE, 2);
}

// ------------------------------------------------------------------------
PRIVATE void get_command_line(int argc, char * argv[],
                             USHORT *port_addr, ULONG *phys_addr,
                             ULONG *size_addr)
{
  SHORT CurrentArg;
  char * dummy;
  ULONG utemp;
  USHORT shortTemp;

  for (CurrentArg = 1; CurrentArg < argc; CurrentArg++)
    {
    if ((argv[CurrentArg][0] == '-') || (argv[CurrentArg][0] == '/'))
      {
      switch (toupper(argv[CurrentArg][1]))
        {
        case 'A': // base address
          if (utemp = strtoul(&(argv[CurrentArg][2]), &dummy, 16))
            *phys_addr = utemp;
          else
            {
            printf("Bad address: %s\n", argv[CurrentArg]);
            SysWait(2000);
            }
        break;
        case 'C': // cache type
          {
          int OldArg=CurrentArg;
          char * argptr = argv[CurrentArg];

          if (argv[CurrentArg][2]=='\0')
            argptr = argv[++CurrentArg];
          else
            argptr = &(argv[CurrentArg][2]);

          if (!find_cache_type(argptr))
            CurrentArg = OldArg;
          }
        break;
        case 'D': // new dac file name
          strncpy(&(dacfile_fname[2]),&(argv[CurrentArg][2]),13);
        break;
        case 'E': // new asic file name
          strncpy(&(ascfile_fname[0]),&(argv[CurrentArg][2]),13);
        break;
        case 'F': // fake detector
          SetParam(DC_THERE, 0);
        break;
        case 'H': // new monitor file name
          strncpy(&(monitor_fname[2]),&(argv[CurrentArg][2]),13);
        break;
        case 'M': // startup macro name
          strncpy(AutoMacroName,&(argv[CurrentArg][2]),127);
        break;
        case 'P': // port address
          if(sscanf(&(argv[CurrentArg][2]), "%x", &shortTemp) == 1)
            *port_addr = shortTemp;
          else
            {
            printf("Bad port address: %s\n", argv[CurrentArg]);
            SysWait(2000);
            }
        break;
        case 'S': // memory size
          if (utemp = strtoul(&(argv[CurrentArg][2]), &dummy, 16))
            *size_addr =  utemp;
          else
            {
            printf("Bad memsize: %s\n", argv[CurrentArg]);
            SysWait(2000);
            }
        break;
        case 'U': // user upper 2 Mb fix
          SetParam(DC_THERE, 2);
        break;
        default:
          printf("Bad argument: %s\n", argv[CurrentArg]);
          SysWait(2000);
        break;
        }
      }
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int main(int argc, char *argv[])
{
  CRECT GraphArea;
  SHORT i;
  WINDOW * MessageWindow;
  DOUBLE MsgStartTime;
  KEY_IDLE_CALLS shiftCheckOnly = { ShiftCheck, 0 };
  ULONG OMA_addr = 0, OMA_memsize = 0;
  USHORT OMA_port = 0, err;
  float real_detector;
  BOOLEAN AutoExec = FALSE;

  _amblksiz = 1000;

  get_command_line(argc, argv, &OMA_port, &OMA_addr, &OMA_memsize);
  get_environmentals(&OMA_port, &OMA_addr, &OMA_memsize);
  InitComAddresses();

  // call to initialize module filemsg.
  set_basepath(argv[0]);  // full path & file spec of this program

  read_color_file(base_path(COLOR_FILE), MAX_COLOR_SETS, ColorSets);

  if((openAndClearScreen(BRT_YELLOW) != -1))
    {
    startup_mouse();

    initialize_form_system();
    uses_int_fields();
    uses_uns_int_fields();
    uses_hex_int_fields();
    uses_float_fields();
    uses_scale_float_fields();
    uses_toggle_fields();
    uses_select_fields();

    uses_mouse_input();

    prepare_int24_trap();

    application_mouse_service = GraphMouseService;
    UserKeyHandler = OMAKey;
    error_handler = error;
    audible_error_handler = ErrorBeep;

    init_keystroke_fields();
    initFormTable();

    keyboard_idle = & shiftCheckOnly;

    erase_screen_area(0, 0, screen_rows, screen_columns,
      (UCHAR) ((ColorSets[COLORS_DEFAULT].regular.background << 4) |
      ColorSets[COLORS_DEFAULT].regular.foreground),
      FALSE);

    {
    #ifndef SYSTEST
    char firstLine[ 60 ] = "    OMA SPEC 4000 Program Version ";
    #else
    char firstLine[ 60 ] = "   OMA4 System Test Software Ver. ";
    #endif
    CHAR * CopyRightMessage[] =
      { NULL,
      "(c) Copyright 1995, EG&G Instruments, Inc.",
      "           All Rights Reserved.",
      NULL
      };
    CopyRightMessage[0] = strcat(firstLine, VersionString);
    put_up_message_window(CopyRightMessage, COLORS_MESSAGE, &MessageWindow);
    }

    /* time the message */
    StartStopWatch(&MsgStartTime);

    ParseFileName(LiveFileName, LiveFileStr);
    ParseFileName(AccumFileName, AccumFileStr);
    ParseFileName(LiveDiskFileName, LiveDiskFileStr);

    /* Reserve segment for curve directory entries. */
    /* Will reallocate later on as needed. */
    if ((MainCurveDir.Entries = malloc(1)) == 0)
      {
      error(ERROR_ALLOC_MEM);
      close_GSS_workstation();
      return 1;
      }

    // assign pointer to empty curve storage block pointers
    MainCurveDir.BlkCount = 0;      // no curves or entries loaded yet
    MainCurveDir.CurveCount = 0;

    filedos_init();

    if (InitCurveBuffer())
      {
      close_GSS_workstation();
      return 1;
      }

    if(getInitialMethodFromFile(INTERFACE_AT_4000))
      {
      close_GSS_workstation();
      return 1;
      }

    MethodToDetInfo(InitialMethod);

    GetParam(DC_THERE, &real_detector); /* see if command line disabled */
    if (real_detector)                  /* detector access */
      {
      float oldmodel, dmodel;
      int choice, err;

      CHAR * DetectorMessage[] =
        {"       Can't identify detector type",
         "  Make sure the power block is switched on",
         "The OMA controller board may not be installed",
         "     Try to identify detector again? ",
         NULL
        };

      oldmodel = (float) getMethodModelNumber(InitialMethod);

      do   /* try to connect to detector and init till giveup or success*/
        {
        err = setup_detector_interface(OMA_port, OMA_addr, OMA_memsize);
        if (err == ERROR_MONITOR_LOAD)
          {
          error(err, monitor_fname);
          break;
          }
        else if (err == ERROR_MONITOR_VER)
          {
          error(err, RequiredMonitorVersion);
          break;
          }

        GetParam(DC_THERE, &real_detector); /* is board there? */
        GetParam(DC_DMODEL, &dmodel);       /* is detector awake? */

        if (!real_detector)
          {
          error(ERROR_FAKEDETECTOR);
          break;
          }
        else if (!dmodel)
          choice= yes_no_choice_window(DetectorMessage, YES, COLORS_MESSAGE);
        }
      while (choice == YES && (!real_detector || !dmodel));

      select_which_scanmenu(dmodel == (float)RAPDA);

      if (dmodel != oldmodel)      /* if beginning with a new detector */
        {                          /* make sure scan setup is legal */
        SetParam(DC_X0, 1.0F);
        SetParam(DC_DELTAX, 1.0F);
        GetParam(DC_ACTIVEX, &oldmodel);
        SetParam(DC_POINTS, oldmodel);
        SetParam(DC_TRACKS, 1.0F);
        SetParam(DC_DELTAY, 1.0F);
        GetParam(DC_ACTIVEY, &oldmodel);
        SetParam(DC_Y0, oldmodel/2.0F);
        }
      }

    // for M1235-- if gpib board exists, and default Spect. is 1235/6:

    init_gpib();
    if(ActonSpectrograph())
      Reset1235();

    /* wait for at least 2.5 seconds of message time */

    if (MessageWindow != NULL)
      {
      while (StopStopWatch(MsgStartTime) < 2.5)
        ;
      release_message_window(MessageWindow);
      }

    MacRecord = FALSE;
    MacPlayBack = FALSE;

    if (access(AutoMacroName, 4) == 0)
      {
      strcpy(MacRecordFileName, AutoMacroName);
      MacPlayBack = -1;
      AutoExec = TRUE;
      }

    err = setjmp(reentry_context);
    if (err)
      {
      exception_error(err);
      if (MacroRunProgram)
        {
        LeaveMacroForm();
        MacroRunProgram = FALSE;
        FKeyItems[39].Control &= ~MENUITEM_INACTIVE;
        }
      ShowFKeys(&FKey);
      // reset these values in case trap out of macro record or playback
      MacRecord = FALSE;
      MacPlayBack = FALSE;
      }

    signal(SIGABRT, control_C_trap);  /* abnormal term */
    signal(SIGINT, control_C_trap);   /* ctrl-c */
#if _MSC_VER < 700
    signal(SIGBREAK, control_C_trap); /* ctrl-break */
#endif
    signal(SIGFPE, floating_point_error_trap);
    _dos_setvect(0, divide_by_zero_error_trap);

    ZoomState = FALSE;

    GraphArea.ll.x = 0;        /* position lower left of plot box */
    GraphArea.ll.y = row_to_y(screen_rows - 2);

    /* set upper right of plot box */
    /* (this sizes the plot area) */

    GraphArea.ur.x = screen.LastVDCXY.x;
    // move y down one device pixel below row 2
    GraphArea.ur.y = adjustYbyDCOffset(row_to_y(2), -1);

    InitializePlotSetupFields(&GraphArea);  /* copies plot info from method */
                                            /* into plots then copies active */
                                            /* plot into plot setup form */

    multiplot_setup(PlotWindows, &GraphArea, window_style); /* axis init for boxes */
    SetFormFromPlotBox(0);
    InitPlotBox(&Plots[ActiveWindow]);

    for (i=0; i<MAXPLOTS; i++)   /* sets plotbox to plotsetup form info */
      InitPlotColors(&Plots[i]);

    PutUpPlotBoxes();
    SetExitRestoreFunc(PutUpPlotBoxes);

    GraphExitKey = 0;
    FKeyItems[1].Text = GoLiveStr;
    FKeyItems[2].Text = StopLiveStr;
    FKeyItems[2].Action = StopLive;
    FKeyItems[35].Text = TagExpandStr;
    FKeyItems[35].TextLen = (char) 0;

    MenuFocus.ActiveMENU = &MainMenu;
    init_menu_focus();
    InitTextCursors();
    enableCoolStat();
    startCoolStat();   // put cooler status on screen

    do
      {
      if (GraphExitKey != -1)
        {
        CRECT ScreenArea = { { 0, 0 }, { 0, 0 } };

        ScreenArea.ur= screen.LastVDCXY;
        CSetClipRectangle(screen_handle, ScreenArea);
        reinit_FKeys();
        setCurrentFormToMenu1();
        ShowHKeys(&MainMenu); 
        }

      /* play back keystroke macros which may play with forms and menus */
      if (MacPlayBack == (USHORT)-1 && AutoExec)
        {
        RunKeyStrokePlay();

        if (isFormMenu1(Current.Form) && (active_locus != LOCUS_APPLICATION))
          active_locus = LOCUS_MENUS;

        GraphExitKey = -1;
        AutoExec = FALSE;
        }
      
      //  execute current menu
      execute_menu(MenuFocus.ActiveMENU, 0, 0, 2, (char) GraphExitKey);

      GraphExitKey = 0;

      /* play back keystroke macros which may play with forms and menus */
      if (MacPlayBack == (USHORT)-1)
        {
        RunKeyStrokePlay();

        if (isFormMenu1(Current.Form) && (active_locus != LOCUS_APPLICATION))
          active_locus = LOCUS_MENUS;

        GraphExitKey = -1;
        }

      if (active_locus == LOCUS_POPUP)
        {
        run_form(Current.Form, &default_form_attributes, TRUE);
        if (isFormGraphWindow(Current.Form))
          active_locus = LOCUS_APPLICATION;
        else if (isFormMenu1(Current.Form))
          active_locus = LOCUS_MENUS;
        else
          active_locus = LOCUS_FORMS;
        }

      /* check to see if macro playback is switching out of graph mode */
      if (((Current.Form->status == FORMSTAT_SWITCH_MODE) &&
        (active_locus != LOCUS_APPLICATION)) ||
        ((active_locus == LOCUS_APPLICATION) &&
        (Current.Form->status != FORMSTAT_SWITCH_MODE)))
        {
        Current.Form->status = FORMSTAT_ACTIVE_FIELD;
        // may have left macro in a combination screen
        if (baslnsub_active() && (GraphExitKey == -1))
          {
          SwitchToGraphMode();
          }
        else if (gathering_calib_points || InM1235Form)
          {
          Current.Form->status = GraphOps();
          GetNewPointsExit();
          GraphExitKey = -1;
          }
        else
          {
          ShowFKeys(&FKey);
          /* unhighlight the menu */
          unhighlight_menuitem(MenuFocus.ItemIndex);
          setCurrentFormToGraphWindow();
          Current.Form->status = FORMSTAT_ACTIVE_FIELD;
          Current.Form->status = GraphOps();
          }
        }
      if (Current.Form->status == FORMSTAT_EXIT_ALL_FORMS &&
        MainCurveDir.BlkCount != 0)
        {
        /* put up lost data warning */
        char * ExitLostDataPrompt[] =
          { "Any unsaved data will be lost",
          "on exit from this program. ",
          "Exit?",
          NULL
          };
        if(yes_no_choice_window(ExitLostDataPrompt, 0, COLORS_MESSAGE)
           != YES) // exit on YES only
          Current.Form->status = FORMSTAT_SWITCH_MODE;
        }
      }
    while (Current.Form->status != FORMSTAT_EXIT_ALL_FORMS);

    saveMethodtoDefaultFile();

    if (ReleaseCurveBuffer())
      return 1;

    Release1235();              /* Send 1235 Goto Local if there */
    shutdown_detector_interface();
    shutdown_form_system();
    close_GSS_workstation();
    }
  else
    printf("\nGSS*CGI (tm) Graphics Interface has not been loaded.\n"
           "Unable to start program (CGIError = %d.)\n", CInqCGIError());

  return FALSE;
}
