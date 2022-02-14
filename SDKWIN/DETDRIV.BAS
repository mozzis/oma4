Attribute VB_Name = "DETDRIV"
Option Explicit

Dim Cmd As Integer, Setting As Single
Dim Instance As Integer
Declare Function InitDetector% Lib "OMA4WIN.DLL" ()
Declare Function SetParam% Lib "OMA4WIN.DLL" (ByVal Cmd%, ByVal Setting!)
Declare Function GetParam% Lib "OMA4WIN.DLL" (ByVal Cmd%, Setting!)
Declare Function GetOMAString% Lib "OMA4WIN.DLL" (ByVal Cmd%, ByVal Indx%, Istr%)
Declare Function ReadCurveFromMem% Lib "OMA4WIN.DLL" (Dest&, ByVal Bytes%, ByVal Curve%)
Declare Function LoadLibrary% Lib "KERNEL.EXE" (ByVal Libname$)
Declare Function FreeLibrary% Lib "KERNEL.EXE" (ByVal hInstance%)
Declare Sub MessageBeep Lib "USER" (ByVal N As Integer)

Global Const DC_ACTIVE = 1
Global Const DC_ACTIVEX = 2
Global Const DC_ACTIVEY = 3
Global Const DC_ADD = 4
Global Const DC_ADPREC = 5
Global Const DC_ANTIBLOOM = 6
Global Const DC_BYTES = 7
Global Const DC_CLR = 8
Global Const DC_COOLSTAT = 9
Global Const DC_COOLLOCK = 10
Global Const DC_COOLONOFF = 11
Global Const DC_COOLTYPE = 12
Global Const DC_CONTROL = 13
Global Const DC_DAMAX = 14
Global Const DC_DAPROG = 15
Global Const DC_DELTAX = 16
Global Const DC_DELTAY = 17
Global Const DC_DETPORT = 18
Global Const DC_DMODEL = 19
Global Const DC_DTEMP = 20
Global Const DC_DERROR = 21
Global Const DC_EXPROWS = 22
Global Const DC_FRAME = 23
Global Const DC_FREQ = 24
Global Const DC_H = 25
Global Const DC_HIMEM = 26
Global Const DC_I = 27
Global Const DC_ID = 28
Global Const DC_IMODE = 29
Global Const DC_INPIA = 30
Global Const DC_J = 31
Global Const DC_K = 32
Global Const DC_L = 33
Global Const DC_MAXMEM = 34
Global Const DC_MEM = 35
Global Const DC_MASK = 36
Global Const DC_OUTPUTREG = 37
Global Const DC_OUTREG = 38
Global Const DC_OUTPIA = 39
Global Const DC_PAUDIO = 40
Global Const DC_PLSR = 41
Global Const DC_PNTMODE = 42
Global Const DC_POINT = 43
Global Const DC_POINTS = 44
Global Const DC_PRESCAN = 45
Global Const DC_PTIME = 46
Global Const DC_PTRIG = 47
Global Const DC_PTRIGNUM = 48
Global Const DC_PTRIGSRC = 49
Global Const DC_PTRIGTRSH = 50
Global Const DC_REGIONS = 51
Global Const DC_REGSIZE = 52
Global Const DC_RUN = 53
Global Const DC_SAMEET = 54
Global Const DC_SCITC = 55
Global Const DC_SHFTMODE = 56
Global Const DC_SHUTMODE = 57
Global Const DC_SPEED = 58
Global Const DC_SRTREG = 59
Global Const DC_STIME = 60
Global Const DC_STOP = 61
Global Const DC_STREAKMODE = 62
Global Const DC_SUBT = 63
Global Const DC_THERE = 64
Global Const DC_TRACK = 65
Global Const DC_TRACKS = 66
Global Const DC_TRIGS = 67
Global Const DC_TRKMODE = 68
Global Const DC_WFSC = 69
Global Const DC_WFSO = 70
Global Const DC_X0 = 71
Global Const DC_Y0 = 72
Global Const DC_ET = 73
Global Const DC_FTIME = 74
Global Const DC_MAXET = 75
Global Const DC_MINET = 76
Global Const DC_PDELAY = 77
Global Const DC_PDELINC = 78
Global Const DC_PDELRANGE = 79
Global Const DC_PWIDTH = 80
Global Const DC_PSTIME = 81
Global Const DC_REGET = 82
Global Const DC_SCMP = 83
Global Const DC_STTIME = 84
Global Const DC_VER = 85

Global Const DS_DANAME = 1

