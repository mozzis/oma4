VERSION 4.00
Begin VB.Form Frm_Setup 
   Appearance      =   0  'Flat
   BackColor       =   &H00C0C0C0&
   Caption         =   "OMA4 Control Center"
   ClientHeight    =   6345
   ClientLeft      =   1515
   ClientTop       =   570
   ClientWidth     =   5310
   FillColor       =   &H000000FF&
   FillStyle       =   0  'Solid
   BeginProperty Font 
      name            =   "Arial"
      charset         =   1
      weight          =   700
      size            =   8.25
      underline       =   0   'False
      italic          =   0   'False
      strikethrough   =   0   'False
   EndProperty
   FontTransparent =   0   'False
   ForeColor       =   &H00808080&
   Height          =   7035
   Icon            =   "OMA4WIN.frx":0000
   KeyPreview      =   -1  'True
   Left            =   1455
   LinkTopic       =   "Form1"
   ScaleHeight     =   6345
   ScaleWidth      =   5310
   Top             =   -60
   Width           =   5430
   Begin Threed.SSPanel Pnl_ScanRan 
      Height          =   1650
      Left            =   2940
      TabIndex        =   80
      Top             =   1680
      Width           =   2100
      _version        =   65536
      _extentx        =   3704
      _extenty        =   2910
      _stockprops     =   15
      forecolor       =   -2147483640
      backcolor       =   -2147483633
      BeginProperty font {FB8F0823-0164-101B-84ED-08002B2EC713} 
         name            =   "MS Sans Serif"
         charset         =   1
         weight          =   700
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      borderwidth     =   0
      bevelouter      =   0
      outline         =   -1  'True
      Begin VB.TextBox Fld_CurrCell 
         Appearance      =   0  'Flat
         BeginProperty Font 
            name            =   "Arial Narrow"
            charset         =   1
            weight          =   400
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   45
         TabIndex        =   85
         Text            =   "Fld_CurrCell"
         Top             =   45
         Width           =   315
      End
      Begin VB.TextBox Fld_TrkEnter 
         Appearance      =   0  'Flat
         BackColor       =   &H00404040&
         BorderStyle     =   0  'None
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00FFFFFF&
         Height          =   225
         Left            =   600
         MaxLength       =   4
         TabIndex        =   84
         TabStop         =   0   'False
         Text            =   "0"
         Top             =   330
         Width           =   540
      End
      Begin VB.TextBox Fld_RTracks 
         Appearance      =   0  'Flat
         BackColor       =   &H00404040&
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H0000C000&
         Height          =   285
         Left            =   975
         TabIndex        =   82
         Text            =   "Tracks"
         Top             =   30
         Width           =   645
      End
      Begin Spin.SpinButton Spn_RTracks 
         Height          =   350
         Left            =   1635
         TabIndex        =   86
         Top             =   0
         Width           =   285
         _version        =   65536
         _extentx        =   503
         _extenty        =   609
         _stockprops     =   73
         forecolor       =   -2147483640
         backcolor       =   -2147483643
         delay           =   200
         shadowthickness =   2
         tdthickness     =   2
      End
      Begin MSGrid.Grid Grd_Tracks 
         Height          =   1305
         Left            =   15
         TabIndex        =   81
         Top             =   330
         Width           =   2085
         _version        =   65536
         _extentx        =   3678
         _extenty        =   2302
         _stockprops     =   77
         forecolor       =   49152
         backcolor       =   4210752
         BeginProperty font {FB8F0823-0164-101B-84ED-08002B2EC713} 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         rows            =   5
         cols            =   3
         scrollbars      =   2
         highlight       =   0   'False
         mouseicon       =   "OMA4WIN.frx":030A
      End
      Begin VB.Label Lbl_RTracks 
         Appearance      =   0  'Flat
         AutoSize        =   -1  'True
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "Tracks"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00404040&
         Height          =   225
         Left            =   345
         TabIndex        =   83
         Top             =   60
         Width           =   600
      End
   End
   Begin Threed.SSPanel Panel3D1 
      Height          =   6315
      Left            =   30
      TabIndex        =   53
      Top             =   -15
      Width           =   5235
      _version        =   65536
      _extentx        =   9234
      _extenty        =   11139
      _stockprops     =   15
      caption         =   "Panel3D1"
      forecolor       =   12632256
      backcolor       =   -2147483633
      BeginProperty font {FB8F0823-0164-101B-84ED-08002B2EC713} 
         name            =   "MS Sans Serif"
         charset         =   1
         weight          =   700
         size            =   8.25
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
      bevelwidth      =   2
      borderwidth     =   0
      bevelinner      =   2
      outline         =   -1  'True
      Begin Threed.SSPanel Pnl_Device 
         Height          =   2175
         Left            =   2805
         TabIndex        =   4
         Top             =   3975
         Width           =   2295
         _version        =   65536
         _extentx        =   4048
         _extenty        =   3836
         _stockprops     =   15
         caption         =   "Panel3D8"
         forecolor       =   12632256
         backcolor       =   -2147483633
         BeginProperty font {FB8F0823-0164-101B-84ED-08002B2EC713} 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         bevelinner      =   1
         outline         =   -1  'True
         Begin VB.ComboBox Fld_Shutter 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   300
            Left            =   150
            Style           =   2  'Dropdown List
            TabIndex        =   77
            Top             =   180
            Width           =   1020
         End
         Begin VB.ComboBox Fld_Sreg 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   300
            Left            =   150
            Style           =   2  'Dropdown List
            TabIndex        =   31
            Top             =   1275
            Width           =   1020
         End
         Begin VB.ComboBox Fld_WFSO 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   300
            Left            =   150
            Style           =   2  'Dropdown List
            TabIndex        =   32
            Top             =   1650
            Width           =   1020
         End
         Begin VB.ComboBox Fld_PTime 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   300
            Left            =   150
            Style           =   2  'Dropdown List
            TabIndex        =   29
            Top             =   540
            Width           =   1020
         End
         Begin VB.ComboBox Fld_STime 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   300
            Left            =   150
            Style           =   2  'Dropdown List
            TabIndex        =   30
            Top             =   915
            Width           =   1020
         End
         Begin VB.Label Lbl_Shutter 
            Alignment       =   1  'Right Justify
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Shutter"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1245
            TabIndex        =   78
            Top             =   240
            Width           =   630
         End
         Begin VB.Label Lbl_STime 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Line Shift"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Index           =   0
            Left            =   1245
            TabIndex        =   38
            Top             =   975
            Width           =   825
         End
         Begin VB.Label Lbl_PTime 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "PIxel Shift"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Index           =   0
            Left            =   1245
            TabIndex        =   40
            Top             =   600
            Width           =   885
         End
         Begin VB.Label Lbl_STime 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Sync"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Index           =   2
            Left            =   1245
            TabIndex        =   41
            Top             =   1635
            Width           =   435
         End
         Begin VB.Label Lbl_STime 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Shift"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Index           =   4
            Left            =   1230
            TabIndex        =   42
            Top             =   1245
            Width           =   405
         End
         Begin VB.Label Lbl_STime 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Register"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Index           =   5
            Left            =   1245
            TabIndex        =   43
            Top             =   1410
            Width           =   720
         End
         Begin VB.Label Lbl_STime 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Before"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Index           =   3
            Left            =   1230
            TabIndex        =   52
            Top             =   1800
            Width           =   570
         End
      End
      Begin VB.Timer Tmr_Cool 
         Enabled         =   0   'False
         Interval        =   250
         Left            =   1440
         Top             =   4380
      End
      Begin Threed.SSPanel Panel3D7 
         Height          =   1200
         Left            =   900
         TabIndex        =   5
         Top             =   4950
         Width           =   1815
         _version        =   65536
         _extentx        =   3201
         _extenty        =   2117
         _stockprops     =   15
         forecolor       =   -2147483640
         backcolor       =   8421504
         BeginProperty font {FB8F0823-0164-101B-84ED-08002B2EC713} 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         bevelouter      =   1
         outline         =   -1  'True
         Begin VB.Label Lbl_Ctype 
            Appearance      =   0  'Flat
            BackColor       =   &H00808080&
            Caption         =   "Lbl_Ctype"
            BeginProperty Font 
               name            =   "MS Serif"
               charset         =   1
               weight          =   700
               size            =   6.75
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C0C0&
            Height          =   195
            Left            =   120
            TabIndex        =   76
            Top             =   615
            Width           =   1635
         End
         Begin VB.Label Lbl_DType 
            Appearance      =   0  'Flat
            BackColor       =   &H00808080&
            Caption         =   "Lbl_DType"
            BeginProperty Font 
               name            =   "MS Serif"
               charset         =   1
               weight          =   700
               size            =   6.75
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C0C0&
            Height          =   195
            Left            =   120
            TabIndex        =   34
            Top             =   120
            Width           =   1635
         End
         Begin VB.Label Lbl_PType 
            Appearance      =   0  'Flat
            BackColor       =   &H00808080&
            Caption         =   "Lbl_Ptype"
            BeginProperty Font 
               name            =   "MS Serif"
               charset         =   1
               weight          =   700
               size            =   6.75
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C0C0&
            Height          =   195
            Left            =   120
            TabIndex        =   35
            Top             =   360
            Width           =   1635
         End
         Begin VB.Label Lbl_FTime 
            Appearance      =   0  'Flat
            BackColor       =   &H00808080&
            Caption         =   "Lbl_Ftime"
            BeginProperty Font 
               name            =   "MS Serif"
               charset         =   1
               weight          =   700
               size            =   6.75
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C0C0&
            Height          =   195
            Left            =   120
            TabIndex        =   36
            Top             =   870
            Width           =   1635
         End
      End
      Begin Threed.SSPanel Panel3D5 
         Height          =   870
         Left            =   900
         TabIndex        =   0
         Top             =   4005
         Width           =   1815
         _version        =   65536
         _extentx        =   3201
         _extenty        =   1535
         _stockprops     =   15
         forecolor       =   12632256
         backcolor       =   -2147483633
         BeginProperty font {FB8F0823-0164-101B-84ED-08002B2EC713} 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         bevelinner      =   1
         outline         =   -1  'True
         floodcolor      =   8421504
         Begin VB.HScrollBar Sb_Temp 
            Height          =   240
            LargeChange     =   10
            Left            =   135
            Max             =   20
            Min             =   -80
            SmallChange     =   2
            TabIndex        =   14
            Top             =   495
            Value           =   10
            Width           =   1545
         End
         Begin VB.TextBox Fld_Temp 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   315
            Left            =   120
            TabIndex        =   13
            Text            =   "20"
            Top             =   135
            Width           =   495
         End
         Begin Threed.SSPanel Panel3D6 
            Height          =   255
            Left            =   945
            TabIndex        =   6
            Top             =   150
            Width           =   735
            _version        =   65536
            _extentx        =   1296
            _extenty        =   450
            _stockprops     =   15
            forecolor       =   -2147483640
            backcolor       =   -2147483633
            BeginProperty font {FB8F0823-0164-101B-84ED-08002B2EC713} 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            borderwidth     =   0
            bevelouter      =   0
            bevelinner      =   1
            outline         =   -1  'True
            Begin VB.Shape Lit_Cool 
               FillColor       =   &H0000C000&
               FillStyle       =   0  'Solid
               Height          =   135
               Left            =   75
               Shape           =   3  'Circle
               Top             =   60
               Width           =   135
            End
            Begin VB.Shape Lit_Status 
               FillColor       =   &H000000FF&
               FillStyle       =   0  'Solid
               Height          =   135
               Left            =   540
               Shape           =   3  'Circle
               Top             =   60
               Width           =   135
            End
            Begin VB.Shape Lit_Cstat 
               FillColor       =   &H0000C0C0&
               FillStyle       =   0  'Solid
               Height          =   135
               Left            =   300
               Shape           =   3  'Circle
               Top             =   60
               Width           =   135
            End
            Begin VB.Shape Shape2 
               FillColor       =   &H0000C000&
               FillStyle       =   0  'Solid
               Height          =   15
               Left            =   420
               Shape           =   3  'Circle
               Top             =   360
               Width           =   135
            End
         End
         Begin VB.Label Lbl_Deg 
            Appearance      =   0  'Flat
            BackColor       =   &H000000FF&
            BackStyle       =   0  'Transparent
            Caption         =   "°C"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   9.75
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00000000&
            Height          =   255
            Left            =   645
            TabIndex        =   37
            Top             =   165
            Width           =   315
         End
      End
      Begin Threed.SSPanel Panel3D4 
         Height          =   915
         Left            =   900
         TabIndex        =   1
         Top             =   3015
         Width           =   1815
         _version        =   65536
         _extentx        =   3201
         _extenty        =   1614
         _stockprops     =   15
         forecolor       =   -2147483640
         backcolor       =   -2147483633
         BeginProperty font {FB8F0823-0164-101B-84ED-08002B2EC713} 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         bevelinner      =   1
         outline         =   -1  'True
         Begin VB.HScrollBar Sb_ExpTime 
            Height          =   255
            LargeChange     =   100
            Left            =   120
            Min             =   -32767
            SmallChange     =   10
            TabIndex        =   12
            Top             =   510
            Value           =   32767
            Width           =   1515
         End
         Begin VB.TextBox Fld_ExpTime 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   11
            Text            =   "Text1"
            Top             =   195
            Width           =   990
         End
         Begin VB.Label Lbl_Sec 
            Appearance      =   0  'Flat
            BackColor       =   &H80000005&
            BackStyle       =   0  'Transparent
            Caption         =   "Sec."
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H80000008&
            Height          =   240
            Left            =   1215
            TabIndex        =   55
            Top             =   195
            Width           =   420
         End
      End
      Begin Threed.SSPanel Panel3D3 
         Height          =   2805
         Left            =   120
         TabIndex        =   2
         Top             =   135
         Width           =   2595
         _version        =   65536
         _extentx        =   4577
         _extenty        =   4948
         _stockprops     =   15
         forecolor       =   -2147483640
         backcolor       =   -2147483633
         BeginProperty font {FB8F0823-0164-101B-84ED-08002B2EC713} 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         bevelinner      =   1
         outline         =   -1  'True
         Begin VB.TextBox Fld_Preps 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   1350
            TabIndex        =   79
            Text            =   "Text1"
            Top             =   2170
            Width           =   615
         End
         Begin VB.TextBox Fld_Mems 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   1350
            TabIndex        =   8
            Text            =   "Text1"
            Top             =   720
            Width           =   615
         End
         Begin VB.TextBox Fld_Scans 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   1350
            TabIndex        =   9
            Text            =   "Text1"
            Top             =   1190
            Width           =   615
         End
         Begin VB.TextBox Fld_Ignores 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   1350
            TabIndex        =   10
            Text            =   "Text1"
            Top             =   1680
            Width           =   615
         End
         Begin VB.TextBox Fld_DAProg 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   1350
            TabIndex        =   7
            Text            =   "Text1"
            Top             =   210
            Width           =   615
         End
         Begin Spin.SpinButton Spn_Preps 
            Height          =   465
            Left            =   1965
            TabIndex        =   88
            Top             =   2095
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_Igns 
            Height          =   465
            Left            =   1965
            TabIndex        =   89
            Top             =   1605
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_Scans 
            Height          =   465
            Left            =   1980
            TabIndex        =   90
            Top             =   1110
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_Mems 
            Height          =   465
            Left            =   1980
            TabIndex        =   91
            Top             =   630
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_DAProg 
            Height          =   465
            Left            =   1980
            TabIndex        =   92
            Top             =   135
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin VB.Label Lbl_Preps 
            Alignment       =   1  'Right Justify
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Preps"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   795
            TabIndex        =   72
            Top             =   2220
            Width           =   495
         End
         Begin VB.Label Lbl_Ignores 
            Alignment       =   1  'Right Justify
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Ignores"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   645
            TabIndex        =   75
            Top             =   1695
            Width           =   645
         End
         Begin VB.Label Lbl_Scans 
            Alignment       =   1  'Right Justify
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Scans"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   750
            TabIndex        =   74
            Top             =   1155
            Width           =   540
         End
         Begin VB.Label Lbl_Mems 
            Alignment       =   1  'Right Justify
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Memories"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   480
            TabIndex        =   73
            Top             =   735
            Width           =   810
         End
         Begin VB.Label Lbl_DAProg 
            Alignment       =   1  'Right Justify
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "DA Program"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   255
            TabIndex        =   71
            Top             =   240
            Width           =   1020
         End
      End
      Begin VB.Timer Tmr_Active 
         Enabled         =   0   'False
         Interval        =   250
         Left            =   240
         Top             =   4905
      End
      Begin Threed.SSPanel Pnl_ScanSpec 
         Height          =   3795
         Left            =   2805
         TabIndex        =   54
         Top             =   135
         Width           =   2295
         _version        =   65536
         _extentx        =   4048
         _extenty        =   6694
         _stockprops     =   15
         forecolor       =   -2147483640
         backcolor       =   -2147483633
         BeginProperty font {FB8F0823-0164-101B-84ED-08002B2EC713} 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         bevelinner      =   1
         outline         =   -1  'True
         Begin VB.TextBox Fld_DeltaX 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   16
            Text            =   "Text1"
            Top             =   720
            Width           =   645
         End
         Begin VB.TextBox Fld_X0 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   15
            Text            =   "Text1"
            Top             =   210
            Width           =   645
         End
         Begin VB.TextBox Fld_Antibloom 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   21
            Text            =   "Text1"
            Top             =   3270
            Width           =   645
         End
         Begin VB.TextBox Fld_Tracks 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   20
            Text            =   "Text1"
            Top             =   2660
            Width           =   645
         End
         Begin VB.TextBox Fld_DeltaY 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   19
            Text            =   "Text1"
            Top             =   2170
            Width           =   645
         End
         Begin VB.TextBox Fld_Y0 
            Alignment       =   1  'Right Justify
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   18
            Text            =   "Text1"
            Top             =   1680
            Width           =   645
         End
         Begin VB.TextBox Fld_Points 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   17
            Text            =   "Text1"
            Top             =   1185
            Width           =   645
         End
         Begin Spin.SpinButton Spn_X0 
            Height          =   465
            Left            =   795
            TabIndex        =   93
            Top             =   135
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_Antibloom 
            Height          =   465
            Left            =   795
            TabIndex        =   94
            Top             =   3195
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_Tracks 
            Height          =   465
            Left            =   795
            TabIndex        =   95
            Top             =   2585
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_DeltaY 
            Height          =   465
            Left            =   795
            TabIndex        =   96
            Top             =   2095
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_Y0 
            Height          =   465
            Left            =   795
            TabIndex        =   97
            Top             =   1605
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_Points 
            Height          =   465
            Left            =   795
            TabIndex        =   98
            Top             =   1115
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_DeltaX 
            Height          =   465
            Left            =   795
            TabIndex        =   99
            Top             =   625
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin VB.Label Lbl_Antibloom 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Antibloom"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1110
            TabIndex        =   44
            Top             =   3315
            Width           =   840
         End
         Begin VB.Label Lbl_Tracks 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Tracks"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1105
            TabIndex        =   45
            Top             =   2715
            Width           =   600
         End
         Begin VB.Label Lbl_DeltaY 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "DeltaY"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1105
            TabIndex        =   46
            Top             =   2235
            Width           =   585
         End
         Begin VB.Label Lbl_Y0 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Y0"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1105
            TabIndex        =   47
            Top             =   1755
            Width           =   240
         End
         Begin VB.Label Lbl_Points 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Points"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1105
            TabIndex        =   48
            Top             =   1260
            Width           =   540
         End
         Begin VB.Label Lbl_DeltaX 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "DeltaX"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1105
            TabIndex        =   49
            Top             =   735
            Width           =   585
         End
         Begin VB.Label Lbl_X0 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "X0"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1105
            TabIndex        =   50
            Top             =   285
            Width           =   240
         End
      End
      Begin Threed.SSPanel Pnl_ScanImg 
         Height          =   3795
         Left            =   2805
         TabIndex        =   3
         Top             =   135
         Visible         =   0   'False
         Width           =   2295
         _version        =   65536
         _extentx        =   4048
         _extenty        =   6694
         _stockprops     =   15
         forecolor       =   -2147483640
         backcolor       =   -2147483633
         BeginProperty font {FB8F0823-0164-101B-84ED-08002B2EC713} 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         bevelinner      =   1
         outline         =   -1  'True
         Begin VB.TextBox Fld_RoiAntiblm 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   28
            Text            =   "Text1"
            Top             =   3270
            Width           =   645
         End
         Begin VB.TextBox Fld_RoidX 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   24
            Text            =   "Text1"
            Top             =   1185
            Width           =   645
         End
         Begin VB.TextBox Fld_RoiSzY 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   27
            Text            =   "Text1"
            Top             =   2660
            Width           =   645
         End
         Begin VB.TextBox Fld_RoidY 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   25
            Text            =   "Text1"
            Top             =   1680
            Width           =   645
         End
         Begin VB.TextBox Fld_RoiY0 
            Alignment       =   1  'Right Justify
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   23
            Text            =   "Text1"
            Top             =   720
            Width           =   645
         End
         Begin VB.TextBox Fld_RoiSzX 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   26
            Text            =   "Text1"
            Top             =   2175
            Width           =   645
         End
         Begin VB.TextBox Fld_RoiX0 
            Appearance      =   0  'Flat
            BackColor       =   &H00404040&
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H0000C000&
            Height          =   285
            Left            =   135
            TabIndex        =   22
            Text            =   "Text1"
            Top             =   210
            Width           =   645
         End
         Begin Spin.SpinButton Spn_RoiAntiBlm 
            Height          =   465
            Left            =   795
            TabIndex        =   100
            Top             =   3195
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_RoiX0 
            Height          =   465
            Left            =   795
            TabIndex        =   101
            Top             =   135
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_RoiSzY 
            Height          =   465
            Left            =   795
            TabIndex        =   102
            Top             =   2580
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_RoiDY 
            Height          =   465
            Left            =   795
            TabIndex        =   103
            Top             =   1605
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_RoiY0 
            Height          =   465
            Left            =   795
            TabIndex        =   104
            Top             =   630
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_RoiSzX 
            Height          =   465
            Left            =   795
            TabIndex        =   105
            Top             =   2100
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin Spin.SpinButton Spn_RoiDX 
            Height          =   465
            Left            =   795
            TabIndex        =   106
            Top             =   1110
            Width           =   285
            _version        =   65536
            _extentx        =   503
            _extenty        =   820
            _stockprops     =   73
            forecolor       =   -2147483640
            backcolor       =   -2147483643
            delay           =   200
            shadowthickness =   2
            tdthickness     =   2
         End
         Begin VB.Label Label3 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Antibloom"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1110
            TabIndex        =   58
            Top             =   3315
            Width           =   840
         End
         Begin VB.Label Lbl_ImgX0 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "X"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1105
            TabIndex        =   59
            Top             =   270
            Width           =   135
         End
         Begin VB.Label Lbl_ImgdX 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "X"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1105
            TabIndex        =   60
            Top             =   1230
            Width           =   135
         End
         Begin VB.Label Lbl_ImgszX 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "X"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1105
            TabIndex        =   61
            Top             =   2265
            Width           =   135
         End
         Begin VB.Label Lbl_ImgY0 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Y"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1105
            TabIndex        =   62
            Top             =   765
            Width           =   135
         End
         Begin VB.Label Lbl_ImgdY 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Y"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1105
            TabIndex        =   63
            Top             =   1770
            Width           =   135
         End
         Begin VB.Label Lbl_ImgszY 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H00C0C0C0&
            BackStyle       =   0  'Transparent
            Caption         =   "Y"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H00404040&
            Height          =   195
            Left            =   1105
            TabIndex        =   64
            Top             =   2715
            Width           =   135
         End
         Begin VB.Label Lbl_ImgOrg 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H80000005&
            BackStyle       =   0  'Transparent
            Caption         =   "Image "
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H80000008&
            Height          =   195
            Index           =   0
            Left            =   1395
            TabIndex        =   65
            Top             =   405
            Width           =   585
         End
         Begin VB.Label Lbl_ImgPix 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H80000005&
            BackStyle       =   0  'Transparent
            Caption         =   "Pixel"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H80000008&
            Height          =   195
            Index           =   0
            Left            =   1395
            TabIndex        =   70
            Top             =   1410
            Width           =   420
         End
         Begin VB.Label Lbl_ImgSz 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H80000005&
            BackStyle       =   0  'Transparent
            Caption         =   "Image"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H80000008&
            Height          =   195
            Index           =   0
            Left            =   1395
            TabIndex        =   69
            Top             =   2415
            Width           =   525
         End
         Begin VB.Label Lbl_ImgOrg 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H80000005&
            BackStyle       =   0  'Transparent
            Caption         =   "Origin"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H80000008&
            Height          =   195
            Index           =   1
            Left            =   1395
            TabIndex        =   68
            Top             =   630
            Width           =   510
         End
         Begin VB.Label Lbl_ImgPix 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H80000005&
            BackStyle       =   0  'Transparent
            Caption         =   "Size"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H80000008&
            Height          =   195
            Index           =   1
            Left            =   1395
            TabIndex        =   67
            Top             =   1620
            Width           =   375
         End
         Begin VB.Label Lbl_ImgSz 
            Appearance      =   0  'Flat
            AutoSize        =   -1  'True
            BackColor       =   &H80000005&
            BackStyle       =   0  'Transparent
            Caption         =   "Size"
            BeginProperty Font 
               name            =   "MS Sans Serif"
               charset         =   1
               weight          =   700
               size            =   8.25
               underline       =   0   'False
               italic          =   0   'False
               strikethrough   =   0   'False
            EndProperty
            ForeColor       =   &H80000008&
            Height          =   195
            Index           =   1
            Left            =   1395
            TabIndex        =   66
            Top             =   2625
            Width           =   375
         End
      End
      Begin Threed.SSRibbon Tgl_Live 
         Height          =   375
         Left            =   135
         TabIndex        =   87
         Top             =   5760
         Width           =   735
         _version        =   65536
         _extentx        =   1296
         _extenty        =   661
         _stockprops     =   65
         backcolor       =   -2147483633
         pictureup       =   "OMA4WIN.frx":0326
         picturedn       =   "OMA4WIN.frx":0A78
      End
      Begin Threed.SSCommand Btn_Run 
         Height          =   540
         Left            =   135
         TabIndex        =   33
         Top             =   5190
         Width           =   735
         _version        =   65536
         _extentx        =   1296
         _extenty        =   953
         _stockprops     =   78
         caption         =   "&Run"
         forecolor       =   -2147483640
         BeginProperty font {FB8F0823-0164-101B-84ED-08002B2EC713} 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         font3d          =   4
      End
      Begin VB.Label Label2 
         Appearance      =   0  'Flat
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "Control"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00404040&
         Height          =   240
         Left            =   195
         TabIndex        =   57
         Top             =   4440
         Width           =   630
      End
      Begin VB.Label Label1 
         Appearance      =   0  'Flat
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "Temp"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00404040&
         Height          =   285
         Left            =   180
         TabIndex        =   56
         Top             =   4200
         Width           =   540
      End
      Begin VB.Label Lbl_Expose 
         Appearance      =   0  'Flat
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "Expose"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00404040&
         Height          =   285
         Left            =   195
         TabIndex        =   39
         Top             =   3225
         Width           =   660
      End
      Begin VB.Label Lbl_Time 
         Appearance      =   0  'Flat
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "Time"
         BeginProperty Font 
            name            =   "MS Sans Serif"
            charset         =   1
            weight          =   700
            size            =   8.25
            underline       =   0   'False
            italic          =   0   'False
            strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00404040&
         Height          =   240
         Left            =   195
         TabIndex        =   51
         Top             =   3465
         Width           =   465
      End
   End
   Begin VB.Menu MnuFile 
      Caption         =   "&File"
      Index           =   1
      Begin VB.Menu MnuOpen 
         Caption         =   "&Open"
         Index           =   2
         Shortcut        =   ^O
      End
      Begin VB.Menu MnuSave 
         Caption         =   "&Save"
         Index           =   3
         Shortcut        =   ^S
      End
      Begin VB.Menu MnuSaveAs 
         Caption         =   "Save &As"
         Index           =   4
      End
      Begin VB.Menu MnuExit 
         Caption         =   "&Exit"
         Index           =   5
      End
   End
   Begin VB.Menu MnuEdit 
      Caption         =   "&Edit"
      Index           =   6
      Begin VB.Menu MnuUndo 
         Caption         =   "&Undo"
         Index           =   61
         Shortcut        =   %{BKSP}
      End
      Begin VB.Menu MnuCopy 
         Caption         =   "&Copy"
         Index           =   62
         Shortcut        =   ^C
      End
      Begin VB.Menu MnuCut 
         Caption         =   "Cu&t"
         Index           =   63
         Shortcut        =   ^X
      End
      Begin VB.Menu MnuPaste 
         Caption         =   "&Paste"
         Index           =   64
         Shortcut        =   ^V
      End
   End
   Begin VB.Menu MnuOptions 
      Caption         =   "&Scan"
      Index           =   3
      Begin VB.Menu MnuSpectra 
         Caption         =   "&Spectra"
         Checked         =   -1  'True
         Index           =   0
      End
      Begin VB.Menu MnuImages 
         Caption         =   "&Images"
         Index           =   0
      End
      Begin VB.Menu MnuRandom 
         Caption         =   "&Random"
         Index           =   0
      End
   End
   Begin VB.Menu Mnu_Quit 
      Caption         =   "&Quit"
      Index           =   5
   End
End
Attribute VB_Name = "Frm_Setup"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim Running As Integer
Dim NoScanUpdate As Integer
Const CellWidth = 600
Const CellHeight = 250
Dim RanOK As Integer

Private Sub AdjustXScan(T1 As TextBox, P1 As Param, T2 As TextBox, P2 As Param, Delta As Single)
  Dim Diff As Long
  Diff = X0.Value + (Points.Value * DeltaX.Value)
  Diff = Diff - ActiveX.Value - 1
  If Diff > 0 Then
    If (P1.Value - Diff) > 0 Then
      P1.Value = P1.Value - Diff
      T1.Text = Str(P1.Value)
    ElseIf P2.Value > 1 And Delta Then
      P2.Value = Int((ActiveX.Value / Delta))
      T2.Text = Str(P2.Value)
    ElseIf P2.Value > 1 Then
      P2.Value = P2.Value - (Diff / P1.Value) - 1
      P2.Value = Int(P2.Value)
      T2.Text = Str(P2.Value)
    End If
  End If
End Sub

Private Sub AdjustYScan(T1 As TextBox, P1 As Param, T2 As TextBox, P2 As Param, Delta As Single)
  Dim Diff As Long
  Diff = Y0.Value + (Tracks.Value * DeltaY.Value)
  Diff = Diff - ActiveY.Value - 1
  If Diff > 0 Then
    If (P1.Value - Diff) > 0 Then
      P1.Value = P1.Value - Diff
      T1.Text = Str(P1.Value)
    ElseIf P2.Value > 1 And Delta Then
      P2.Value = Int((ActiveY.Value / Delta))
      T2.Text = Str(P2.Value)
    ElseIf P2.Value > 1 Then
      P2.Value = P2.Value - (Diff / P1.Value) - 1
      P2.Value = Int(P2.Value)
      T2.Text = Str(P2.Value)
    End If
  End If
End Sub

Private Sub Btn_Run_Click()
 If Running = 0 Then
   If Tgl_Live.Value = False Then
     DetErr = SetOMA(DC_RUN, 0)
   Else
     DetErr = SetOMA(DC_RUN, 1)
   End If
   Running = 1
   Tmr_Active.Enabled = True
   Btn_Run.Caption = "Stop"
 Else
   DetErr = SetOMA(DC_STOP, 0)
   Running = 0
   Tmr_Active.Enabled = False
   Btn_Run.Caption = "Run"
 End If
End Sub

Private Sub Btn_Run_GotFocus()
  MnuEdit(6).Enabled = False ' Disable Edit functions
End Sub

Private Sub Btn_Run_LostFocus()
  MnuEdit(6).Enabled = True ' Enable Edit functions
End Sub

Private Sub EatEnter(KeyAscii As Integer)
  If KeyAscii = 13 Then
    KeyAscii = 0
  End If
End Sub

Private Sub FillInTracks()
  Static t As Single
  Static j, r, Row, Col As Integer
  Row = Grd_Tracks.Row
  Col = Grd_Tracks.Col
  For j = 1 To Grd_Tracks.Rows - 1
    Grd_Tracks.Row = j
    r = SetOMA(DC_TRACK, j)
    Grd_Tracks.Col = 0
    Grd_Tracks.Text = j
    Grd_Tracks.Col = 1
    Grd_Tracks.ColWidth(1) = CellWidth   'only need to set
    Grd_Tracks.RowHeight(j) = CellHeight 'sizes 1st time
    r = GetOMA(DC_Y0, t)
    Grd_Tracks.Text = t
    Grd_Tracks.Col = 2
    Grd_Tracks.ColWidth(2) = CellWidth
    r = GetOMA(DC_DELTAY, t)
    Grd_Tracks.Text = t
  Next j
  Grd_Tracks.Row = Row
  Grd_Tracks.Col = Col
  SetSelCell
End Sub

Private Sub Fld_Antibloom_Change()
  Call ChangeFld(Fld_Antibloom, Antibloom)
End Sub

Private Sub Fld_Antibloom_KeyPress(KeyAscii As Integer)
  EatEnter (KeyAscii)
End Sub

Private Sub Fld_DAProg_Change()
'  Call ChangeFld(Fld_DAProg, DAProg)
End Sub

Private Sub Fld_DAProg_KeyPress(KeyAscii As Integer)
  EatEnter (KeyAscii)
End Sub

Private Sub Fld_DeltaX_Change()
  Call ChangeFld(Fld_DeltaX, DeltaX)
  Call AdjustXScan(Fld_X0, X0, Fld_Points, Points, DeltaX.Value)
End Sub

Private Sub Fld_DeltaY_Change()
  Call LimitTracks
  Call ChangeFld(Fld_DeltaY, DeltaY)
  Call AdjustYScan(Fld_Y0, Y0, Fld_Tracks, Tracks, DeltaY.Value)
End Sub

Private Sub Fld_ExpTime_KeyPress(KeyAscii As Integer)
  Static InSub%
  If KeyAscii = 13 And InSub = False Then
    KeyAscii = 0
    InSub = True
    Call Fld_ExpTime_LostFocus
    InSub = False
  End If
End Sub

Private Sub Fld_ExpTime_LostFocus()
  Static Old As Single, NewVal As Single
  On Error GoTo ExpTime_err
  Old = ExpTime.Value
  NewVal = Val(Fld_ExpTime.Text)
  If IsNumeric(Fld_ExpTime.Text) = False Then
    On Error GoTo 0
    Fld_ExpTime.Text = Old
    Exit Sub
  End If
  ExpTime.Value = NewVal
  DetErr = SetOMA(ExpTime.Command, NewVal)
  If Not Incrementing Then
    Sb_ExpTime.Value = ExpTimeToSbCount(NewVal)
  End If
  SaveString = Str(Old)
  CheckFrameTime
Exit Sub
ExpTime_err:
  ExpTime.Value = Old
  Fld_ExpTime.Text = FormatET(Str$(Old))
  On Error GoTo 0
Resume Next
End Sub

Private Sub Fld_Ignores_Change()
  Call ChangeFld(Fld_Ignores, Ignores)
End Sub

Private Sub Fld_Mems_Change()
  Call ChangeFld(Fld_Mems, Mems)
End Sub

Private Sub Fld_Points_Change()
  Call ChangeFld(Fld_Points, Points)
  Call AdjustXScan(Fld_X0, X0, Fld_DeltaX, DeltaX, 0)
End Sub

Private Sub Fld_Preps_Change()
  Call ChangeFld(Fld_Preps, Preps)
End Sub

Private Sub Fld_PTime_Change()
  Fld_PTime_Click
End Sub

Private Sub Fld_PTime_Click()
  PTime.Value = Fld_PTime.ListIndex
  DetErr = SetOMA(DC_PTIME, PTime.Value)
  CheckFrameTime
End Sub

Private Sub Fld_RoiAntiblm_Change()
  Call ChangeFld(Fld_RoiAntiblm, Antibloom)
End Sub

Private Sub Fld_ROIdX_Change()
  Call ChangeFld(Fld_RoidX, DeltaX)
  Call AdjustXScan(Fld_RoidX, X0, Fld_RoiSzX, Points, DeltaX.Value)

End Sub

Private Sub Fld_ROIdY_Change()
  Call ChangeFld(Fld_RoidY, DeltaY)
  Call AdjustYScan(Fld_RoiY0, Y0, Fld_RoiSzY, Tracks, DeltaY.Value)
End Sub

Private Sub Fld_RoiSzX_Change()
  Call ChangeFld(Fld_RoiSzX, Points)
  Call AdjustXScan(Fld_RoiX0, X0, Fld_RoidX, DeltaX, 0)
End Sub

Private Sub Fld_ROIszY_Change()
  Call LimitTracks
  Call ChangeFld(Fld_RoiSzY, Tracks)
  Call AdjustYScan(Fld_RoiY0, Y0, Fld_RoidY, DeltaY, 0)
End Sub

Private Sub Fld_RoiX0_Change()
  Call ChangeFld(Fld_RoiX0, X0)
  Call AdjustXScan(Fld_RoiSzX, Points, Fld_RoidX, DeltaX, 0)
End Sub

Private Sub Fld_ROIY0_Change()
  Call ChangeFld(Fld_RoiSzY, Y0)
  Call AdjustYScan(Fld_RoiSzY, Tracks, Fld_RoidY, DeltaY, 0)
End Sub

Private Sub Fld_RTracks_Change()
  Call LimitTracks
  Call ChangeFld(Fld_RTracks, Tracks)
  Grd_Tracks.Rows = Tracks.Value + 1
  Call FillInTracks
End Sub

Private Sub Fld_Scans_Change()
  Call ChangeFld(Fld_Scans, Scans)
End Sub

Private Sub Fld_Shutter_Change()
  If Fld_Shutter.Text = "Normal" Then Shutter.Value = 0
  If Fld_Shutter.Text = "Open" Then Shutter.Value = 1
  If Fld_Shutter.Text = "Closed" Then Shutter.Value = 2
  DetErr = SetOMA(Shutter.Command, Shutter.Value)
  CheckFrameTime
End Sub

Private Sub Fld_Shutter_Click()
  Shutter.Value = Fld_Shutter.ListIndex
  DetErr = SetOMA(Shutter.Command, Shutter.Value)
End Sub

Private Sub Fld_Shutter_GotFocus()
  MnuEdit(6).Enabled = False ' Disable Edit functions
End Sub

Private Sub Fld_Shutter_LostFocus()
  MnuEdit(6).Enabled = True ' Enable Edit functions
End Sub

Private Sub Fld_Sreg_Change()
  Fld_Sreg_Change
End Sub

Private Sub Fld_Sreg_Click()
  Sreg.Value = Fld_Sreg.ListIndex
  DetErr = SetOMA(Sreg.Command, Sreg.Value)
  CheckFrameTime
End Sub

Private Sub Fld_STime_Change()
  Fld_STime_Click
End Sub

Private Sub Fld_STime_Click()
  STime.Value = Fld_STime.ListIndex
  DetErr = SetOMA(DC_STIME, STime.Value)
  If DetErr Then
    DetErr = GetOMA(DC_STIME, STime.Value)
    Fld_STime.ListIndex = STime.Value
  End If
  CheckFrameTime
End Sub

Private Sub Fld_Temp_Change()
  Dim CoolState As Integer
  Static Recursing As Integer
  If Recursing = False Then
    Recursing = True
    Call ChangeFld(Fld_Temp, DetTemp)
    If Not Incrementing Then
      Sb_Temp.Value = DetTemp.Value
    End If
    If DetTemp.Value < DetTemp.Max Then
      CoolState = 1
    Else
      CoolState = 0
      Fld_Temp.Text = "Off"
    End If
    DetErr = SetOMA(DC_COOLONOFF, CoolState)
    Recursing = False
  End If
End Sub

Private Sub Fld_Tracks_Change()
  Call LimitTracks
  Call ChangeFld(Fld_Tracks, Tracks)
  Call AdjustYScan(Fld_Y0, Y0, Fld_DeltaY, DeltaY, 0)
End Sub

Private Sub Fld_Tracks_KeyPress(KeyAscii As Integer)
  EatEnter (KeyAscii)
End Sub

Private Sub Fld_TrkEnter_Change()
  Static r, t As Integer
  Static f As Single
  If NoScanUpdate = False Then
    t = Grd_Tracks.Row
    r = SetOMA(DC_TRACK, t)
    If IsNumeric(Fld_TrkEnter.Text) Then
      f = Fld_TrkEnter.Text
    Else
      If IsNumeric(Grd_Tracks.Text) Then
        Fld_TrkEnter.Text = Grd_Tracks.Text
      Else
        Fld_TrkEnter.Text = 1
      End If
    End If
    Grd_Tracks.Text = Fld_TrkEnter.Text
    If Grd_Tracks.Col = 1 Then
      r = SetOMA(DC_Y0, f)
    Else
      r = SetOMA(DC_DELTAY, f)
    End If
  End If
End Sub

Private Sub Fld_TrkEnter_KeyDown(KeyCode As Integer, Shift As Integer)
  Static r As Integer
  Select Case KeyCode
    Case KEY_UP
      If Grd_Tracks.Row > 1 Then
        If Grd_Tracks.Row - 1 < Grd_Tracks.TopRow Then
          Grd_Tracks.TopRow = Grd_Tracks.TopRow - 1
        End If
        Grd_Tracks.Row = Grd_Tracks.Row - 1
      End If
    Case KEY_DOWN
      If Grd_Tracks.Row < Grd_Tracks.Rows - 1 Then
        If Grd_Tracks.Row - Grd_Tracks.TopRow > 2 Then
          Grd_Tracks.TopRow = Grd_Tracks.TopRow + 1
        End If
        Grd_Tracks.Row = Grd_Tracks.Row + 1
      End If
    Case KEY_LEFT
      If (Shift And SHIFT_MASK) And Fld_TrkEnter.SelLength > 0 Then
        Fld_TrkEnter.SelLength = Fld_TrkEnter.SelLength - 1
      Else
        If Grd_Tracks.Col > 1 Then
          Grd_Tracks.Col = Grd_Tracks.Col - 1
        End If
      End If
    Case KEY_RIGHT
      If (Shift And SHIFT_MASK) And Fld_TrkEnter.SelLength < Len(Fld_TrkEnter.Text) Then
        Fld_TrkEnter.SelLength = Fld_TrkEnter.SelLength + 1
      Else
        If Grd_Tracks.Col < Grd_Tracks.Cols - 1 Then
          Grd_Tracks.Col = Grd_Tracks.Col + 1
        End If
      End If
    End Select
End Sub

Private Sub Fld_WFSO_Change()
  Fld_WFSO_Click
End Sub

Private Sub Fld_WFSO_Click()
  WFSO.Value = Fld_WFSO.ListIndex
  DetErr = SetOMA(WFSO.Command, WFSO.Value)
  CheckFrameTime
End Sub

Private Sub Fld_X0_Change()
  Call ChangeFld(Fld_X0, X0)
  Call AdjustXScan(Fld_Points, Points, Fld_DeltaX, DeltaX, 0)
End Sub

Private Sub Fld_Y0_Change()
  Call LimitTracks
  Call ChangeFld(Fld_Y0, Y0)
  Call AdjustYScan(Fld_Tracks, Tracks, Fld_DeltaY, DeltaY, 0)
End Sub

Private Sub Form_Load()

Static FParam As Single ' Place to hold various GetOmas
Static i, IChr(10) As Integer, Istr As String

If Command$ <> "" Then
  Instance = LoadLibrary%(Command)
Else
  Instance = LoadLibrary%("OMA4WIN.DLL")
End If
If Instance < 32 Then
    FParam = MsgBox("Driver Not Found", 16, "Warning!")
End If

' Get the general info
If Instance < 32 Then
  FParam = -1
Else
  DetErr = GetParam(DC_THERE, FParam)
  If FParam = 0 Then
    FParam = MsgBox("OMA Controller Not Found", 16, "Warning!")
    FParam = 0
  Else
    DetErr = InitDetector()
    DetErr = GetParam%(DC_DMODEL, FParam)
  End If
End If
Select Case FParam
  Case 0
    Lbl_DType.Caption = "No detector"
    FParam = MsgBox("Detector not Connected", 16, "Warning!")
    DModel.Value = 0
'   End
  Case -1
    Lbl_DType.Caption = "Fake Detector"
    DModel.Value = -1
  Case 1
    Lbl_DType.Caption = "Thomson 512 X 512"
    DModel.Value = 1
  Case 2
    Lbl_DType.Caption = "Thomson 512 Dual"
    DModel.Value = 2
  Case 3
    Lbl_DType.Caption = "EEV 1024 X 256"
    DModel.Value = 3
  Case 4
    Lbl_DType.Caption = "Rapda detector"
    DModel.Value = 4
  Case 5
    Lbl_DType.Caption = "Thomson 1024 X 256"
    DModel.Value = 5
  Case 6
    Lbl_DType.Caption = "Thomson 1024 X 1024"
    DModel.Value = 6
  Case Else
    Lbl_DType.Caption = "Unknown detector"
    DModel.Value = 0
End Select
Tmr_Cool.Enabled = True
DetErr = GetOMA(DC_PLSR, FParam)
Select Case FParam
  Case 0
    Lbl_PType.Caption = "No Shutter"
  Case 1
    Lbl_PType.Caption = "Shutter Installed"
  Case 2
    Lbl_PType.Caption = "LCI Pulser Installed"
  Case Else
    Lbl_PType.Caption = "Unknown Pulser Option"
End Select

DetErr = GetOMA(DC_COOLTYPE, DCType.Value)
Select Case DCType.Value
  Case 0
    Lbl_CType.Caption = "No Cooler"
    Sb_Temp.Min = -120
  Case 1
    Lbl_CType.Caption = "Cryo Cooler"
    Sb_Temp.Min = -120
  Case 2
    Lbl_CType.Caption = "Peltier Cooler"
    Sb_Temp.Min = -80
  Case Else
    Lbl_CType.Caption = "Unknown Cooler Option"
End Select

' Get the current detector settings
Call InitParams

' Set up the list box control for shutter mode
Fld_Shutter.AddItem "Normal"
Fld_Shutter.AddItem "Closed"
Fld_Shutter.AddItem "Open"
Fld_Shutter.ListIndex = Shutter.Value

Fld_PTime.AddItem "Normal"
Fld_PTime.AddItem "Fast"
Fld_PTime.AddItem "Slow"
Fld_PTime.ListIndex = PTime.Value

Fld_STime.AddItem "Normal"
Fld_STime.AddItem "Fast"
Fld_STime.AddItem "Slow"
Fld_STime.ListIndex = STime.Value

Fld_Sreg.AddItem "Upper"
Fld_Sreg.AddItem "Lower"
Fld_Sreg.ListIndex = Sreg.Value

Fld_WFSO.AddItem "No Wait"
Fld_WFSO.AddItem "Wait"
Fld_WFSO.ListIndex = WFSO.Value

' DetErr = GetOMAString(DC_DAPROG, 1, IChr(1))

' Put the detector setting values into the control values
' When a control value changes, it also calls SetOma
Call MnuSpectra_Click(0)
If TrackMode.Value Then
  Call MnuRandom_Click(0)
End If
Fld_DAProg.Text = Format$(DAProg.Value, "####")
Fld_ExpTime.Text = Format$(ExpTime.Value, "Scientific")
Sb_ExpTime.Value = ExpTimeToSbCount(ExpTime.Value)
Fld_Ignores.Text = Format$(Ignores.Value, "####")
Fld_Mems.Text = Format$(Mems.Value, "####")
Fld_Preps.Text = Format$(Preps.Value, "####")
Fld_Scans.Text = Format$(Scans.Value, "####")
If DetTemp.Value > 20 Then
  Fld_Temp.Text = Format(DetTemp.Value, "###")
Else
  Fld_Temp.Text = "Off"
End If
Sb_Temp.Value = DetTemp.Value
End Sub

Private Sub Form_Unload(Cancel As Integer)
  Instance = FreeLibrary(Instance)
End Sub

Private Sub Grd_Tracks_KeyDown(KeyCode As Integer, Shift As Integer)
  Static r As Integer
  Dim Row, Col As Integer

  Row = Grd_Tracks.Row
  Col = Grd_Tracks.Col
  Select Case KeyCode
    Case KEY_UP
      If Grd_Tracks.Row > 1 Then
        If Grd_Tracks.Row - 1 < Grd_Tracks.TopRow Then
          Grd_Tracks.TopRow = Grd_Tracks.TopRow - 1
        End If
        Grd_Tracks.Row = Grd_Tracks.Row - 1
      End If
    Case KEY_DOWN
      If Grd_Tracks.Row < Grd_Tracks.Rows - 1 Then
        If Grd_Tracks.Row - Grd_Tracks.TopRow > 2 Then
          Grd_Tracks.TopRow = Grd_Tracks.TopRow + 1
        End If
        Grd_Tracks.Row = Grd_Tracks.Row + 1
      End If
    Case KEY_LEFT
      If (Shift And SHIFT_MASK) And Fld_TrkEnter.SelLength > 0 Then
        Fld_TrkEnter.SelLength = Fld_TrkEnter.SelLength - 1
      Else
        If Grd_Tracks.Col > 1 Then
          Grd_Tracks.Col = Grd_Tracks.Col - 1
        End If
      End If
    Case KEY_RIGHT
      If (Shift And SHIFT_MASK) And Fld_TrkEnter.SelLength < Len(Fld_TrkEnter.Text) Then
        Fld_TrkEnter.SelLength = Fld_TrkEnter.SelLength + 1
      Else
        If Grd_Tracks.Col < Grd_Tracks.Cols - 1 Then
          Grd_Tracks.Col = Grd_Tracks.Col + 1
        End If
      End If
    End Select
  If (Row <> Grd_Tracks.Row) Or (Col <> Grd_Tracks.Col) Then
    Call SetSelCell
  End If
End Sub

Private Sub Grd_Tracks_RowColChange()
  SetSelCell
 End Sub

Private Sub LoadImgPanel()
  Fld_RoiAntiblm.Text = Format$(Antibloom.Value, "###")
  Fld_RoidX.Text = Format$(DeltaX.Value, "####")
  Fld_RoidY.Text = Format$(DeltaY.Value, "####")
  Fld_RoiSzX.Text = Format$(Points.Value, "####")
  Fld_RoiSzY.Text = Format$(Tracks.Value, "####")
  Fld_RoiX0.Text = Format$(X0.Value, "####")
  Fld_RoiY0.Text = Format$(Y0.Value, "####")
End Sub

Private Sub LoadRanPanel()
  Static r, i As Integer
  TrackMode.Value = 1
  r = SetOMA(DC_TRKMODE, TrackMode.Value)
  Grd_Tracks.Rows = Tracks.Value + 1
  Grd_Tracks.Cols = 3
  Grd_Tracks.Row = 0
  Grd_Tracks.Col = 0
  Grd_Tracks.Text = "Track"
  Grd_Tracks.Col = 1
  Grd_Tracks.Text = "  Y0"
  Grd_Tracks.Col = 2
  Grd_Tracks.Text = "DeltaY"
  Grd_Tracks.Row = 1
  Call FillInTracks
  Grd_Tracks.Col = 1
  Grd_Tracks.Row = 1
  Fld_RTracks.Text = Format$(Tracks.Value, "####")
  Fld_TrkEnter.Height = CellHeight - 8
  Fld_TrkEnter.Width = CellWidth - 32
  SetSelCell
End Sub

Private Sub LoadSpecPanel()
Fld_Antibloom.Text = Format$(Antibloom.Value, "###")
Fld_DeltaX.Text = Format$(DeltaX.Value, "####")
Fld_DeltaY.Text = Format$(DeltaY.Value, "####")
Fld_Points.Text = Format$(Points.Value, "####")
Fld_Tracks.Text = Format$(Tracks.Value, "####")
Fld_X0.Text = Format$(X0.Value, "####")
Fld_Y0.Text = Format$(Y0.Value, "####")
End Sub

Private Sub Mnu_Quit_Click(Index As Integer)
  End
End Sub

Private Sub MnuCopy_Click(Index As Integer)
  If TypeOf Me.ActiveControl Is TextBox Then
    Clipboard.SetText Me.ActiveControl.Text
  End If
End Sub

Private Sub MnuCut_Click(Index As Integer)
  If TypeOf Me.ActiveControl Is TextBox Then
    SaveString = Me.ActiveControl.Text
    Clipboard.SetText Me.ActiveControl.Text
    Me.ActiveControl.Text = ""
  End If
End Sub

Private Sub MnuExit_Click(Index As Integer)
  Unload Frm_Setup
  End
End Sub

Private Sub MnuImages_Click(Index As Integer)
  Call LoadImgPanel
  Pnl_ScanSpec.Enabled = False
  Pnl_ScanSpec.Visible = False
  If MnuRandom(0).CHECKED = False Then
    Pnl_ScanRan.Enabled = False
    Pnl_ScanRan.Visible = False
  Else
    Pnl_ScanRan.Enabled = True
    Pnl_ScanRan.Visible = True
  End If
  Pnl_ScanImg.Enabled = True
  Pnl_ScanImg.Visible = True
  MnuImages(0).CHECKED = True
  MnuSpectra(0).CHECKED = False
'  MnuRandom(0).Checked = False
End Sub

Private Sub MnuOpen_Click(Index As Integer)
   Frm_File.Caption = "Open File"
   Frm_File.Show 1
End Sub

Private Sub MnuPaste_Click(Index As Integer)
  If TypeOf Me.ActiveControl Is TextBox Then
    SaveString$ = Me.ActiveControl.Text
    Me.ActiveControl.Text = Clipboard.GetText(1)
  End If
End Sub

Private Sub MnuRandom_Click(Index As Integer)
  If MnuRandom(0).CHECKED = False Then
    Call LoadRanPanel
    Pnl_ScanRan.Enabled = True
    Pnl_ScanRan.Visible = True
    MnuRandom(0).CHECKED = True
  Else
    Pnl_ScanRan.Enabled = False
    Pnl_ScanRan.Visible = False
    MnuRandom(0).CHECKED = False
    If MnuImages(0).CHECKED = True Then
      Call LoadImgPanel
    Else
      Call LoadSpecPanel
    End If
  End If
End Sub

Private Sub MnuSave_Click(Index As Integer)
   Frm_File.Caption = "Save File"
   Frm_File.Show 1
End Sub

Private Sub MnuSaveAs_Click(Index As Integer)
   Frm_File.Caption = "Save File As"
   Frm_File.Show 1
End Sub

Private Sub MnuSpectra_Click(Index As Integer)
  Call LoadSpecPanel
  Pnl_ScanImg.Enabled = False
  Pnl_ScanImg.Visible = False
  If MnuRandom(0).CHECKED = False Then
    Pnl_ScanRan.Enabled = False
    Pnl_ScanRan.Visible = False
  Else
    Pnl_ScanRan.Enabled = True
    Pnl_ScanRan.Visible = True
  End If
  Pnl_ScanSpec.Enabled = True
  Pnl_ScanSpec.Visible = True
  MnuSpectra(0).CHECKED = True
  MnuImages(0).CHECKED = False
'  MnuRandom(0).Checked = False
End Sub

Private Sub MnuUndo_Click(Index As Integer)
  If Len(SaveString) Then
    If TypeOf Me.ActiveControl Is TextBox Then
      Me.ActiveControl.Text = SaveString
    End If
  End If
End Sub

Private Sub Sb_ExpTime_Change()
  Dim Fval As Single
  Incrementing = True
  Fval = SbCountToExpTime(Sb_ExpTime.Value)
  Fld_ExpTime.Text = FormatET$(Fval)
  Call Fld_ExpTime_LostFocus
  Incrementing = False
End Sub

Private Sub Sb_ExpTime_Scroll()
  Sb_ExpTime_Change
End Sub

Private Sub Sb_Temp_Change()
 Static InSub
 Dim i As Integer
 If Not InSub Then
   InSub = True
   Incrementing = True
   i = Sb_Temp.Value + 1
   i = Int(i / 2)
   i = i * 2
   Sb_Temp.Value = i
   DetTemp.Value = Sb_Temp.Value
   Fld_Temp.Text = DetTemp.Value
   Incrementing = False
 InSub = False
 End If
End Sub

Private Sub Sb_Temp_Scroll()
  Sb_Temp_Change
End Sub

Private Sub SetSelCell()
  Static Row, Col As Integer
  NoScanUpdate = True ' don't fire TrkEnter change event!
  Col = Grd_Tracks.Col
  Row = (Grd_Tracks.Row - Grd_Tracks.TopRow) + 1
  Fld_TrkEnter.Left = Grd_Tracks.Left + ((Col * CellWidth) - 4)
  Fld_TrkEnter.Top = Grd_Tracks.Top + (Row * (CellHeight + 8)) + 14
  Fld_TrkEnter.Text = Grd_Tracks.Text
  Fld_TrkEnter.Visible = True
  Grd_Tracks.Refresh
  If Fld_TrkEnter.Visible Then
    Fld_TrkEnter.SetFocus
  End If
  NoScanUpdate = False
End Sub

Private Sub Spn_Antibloom_SpinDown()
  Call IncDecfld(Antibloom, Fld_Antibloom, -10)
End Sub

Private Sub Spn_Antibloom_SpinUp()
  Call IncDecfld(Antibloom, Fld_Antibloom, 10)
End Sub

Private Sub Spn_DAProg_SpinDown()
  Call IncDecfld(DAProg, Fld_DAProg, -1)
End Sub

Private Sub Spn_DAProg_SpinUp()
  Call IncDecfld(DAProg, Fld_DAProg, 1)
End Sub

Private Sub Spn_DeltaX_SpinDown()
  Call IncDecfld(DeltaX, Fld_DeltaX, -1)
End Sub

Private Sub Spn_DeltaX_SpinUp()
  Call IncDecfld(DeltaX, Fld_DeltaX, 1)
End Sub

Private Sub Spn_DeltaY_SpinDown()
  Call IncDecfld(DeltaY, Fld_DeltaY, -1)
End Sub

Private Sub Spn_DeltaY_SpinUp()
  Call IncDecfld(DeltaY, Fld_DeltaY, 1)
End Sub

Private Sub Spn_Igns_SpinDown()
  Call IncDecfld(Ignores, Fld_Ignores, -1)
End Sub

Private Sub Spn_Igns_SpinUp()
  Call IncDecfld(Ignores, Fld_Ignores, 1)
End Sub

Private Sub Spn_Mems_SpinDown()
  Call IncDecfld(Mems, Fld_Mems, -1)
End Sub

Private Sub Spn_Mems_SpinUp()
  Call IncDecfld(Mems, Fld_Mems, 1)
End Sub

Private Sub Spn_Points_SpinDown()
  Call IncDecfld(Points, Fld_Points, -1)
End Sub

Private Sub Spn_Points_SpinUp()
  Call IncDecfld(Points, Fld_Points, 1)
End Sub

Private Sub Spn_Preps_SpinDown()
  Call IncDecfld(Preps, Fld_Preps, -1)
End Sub

Private Sub Spn_Preps_SpinUp()
  Call IncDecfld(Preps, Fld_Preps, 1)
End Sub

Private Sub Spn_RoiAntiBlm_SpinDown()
  Call IncDecfld(Antibloom, Fld_RoiAntiblm, -10)
End Sub

Private Sub Spn_RoiAntiBlm_SpinUp()
  Call IncDecfld(Antibloom, Fld_RoiAntiblm, 10)
End Sub

Private Sub Spn_RoiDX_SpinDown()
  Call IncDecfld(DeltaX, Fld_RoidX, -1)
End Sub

Private Sub Spn_RoiDX_SpinUp()
  Call IncDecfld(DeltaX, Fld_RoidX, 1)
End Sub

Private Sub Spn_RoiDY_SpinDown()
  Call IncDecfld(DeltaY, Fld_RoidY, -1)
End Sub

Private Sub Spn_RoiDY_SpinUp()
  Call IncDecfld(DeltaY, Fld_RoidY, 1)
End Sub

Private Sub Spn_RoiSzX_SpinDown()
  Call IncDecfld(Points, Fld_RoiSzX, -1)
End Sub

Private Sub Spn_RoiSzX_SpinUp()
  Call IncDecfld(Points, Fld_RoiSzX, 1)
End Sub

Private Sub Spn_RoiSzY_SpinDown()
  Call IncDecfld(Tracks, Fld_RoiSzY, -1)
End Sub

Private Sub Spn_RoiSzY_SpinUp()
  Call IncDecfld(Tracks, Fld_RoiSzY, 1)
End Sub

Private Sub Spn_RoiX0_SpinDown()
  Call IncDecfld(X0, Fld_RoiX0, -1)
End Sub

Private Sub Spn_RoiX0_SpinUp()
  Call IncDecfld(X0, Fld_RoiX0, 1)
End Sub

Private Sub Spn_RoiY0_SpinDown()
  Call IncDecfld(Y0, Fld_RoiY0, -1)
End Sub

Private Sub Spn_RoiY0_SpinUp()
  Call IncDecfld(Y0, Fld_RoiY0, 1)
End Sub

Private Sub Spn_RTracks_SpinDown()
  Call IncDecfld(Tracks, Fld_RTracks, -1)

End Sub

Private Sub Spn_RTracks_SpinUp()
  Call IncDecfld(Tracks, Fld_RTracks, 1)
End Sub

Private Sub Spn_Scans_SpinDown()
  Call IncDecfld(Scans, Fld_Scans, -1)
End Sub

Private Sub Spn_Scans_SpinUp()
  Call IncDecfld(Scans, Fld_Scans, 1)
End Sub

Private Sub Spn_Tracks_SpinDown()
  Call IncDecfld(Tracks, Fld_Tracks, -1)
End Sub

Private Sub Spn_Tracks_SpinUp()
  Call IncDecfld(Tracks, Fld_Tracks, 1)
End Sub

Private Sub Spn_X0_SpinDown()
  Call IncDecfld(X0, Fld_X0, -1)
End Sub

Private Sub Spn_X0_SpinUp()
  Call IncDecfld(X0, Fld_X0, 1)
End Sub

Private Sub Spn_Y0_SpinDown()
  Call IncDecfld(Y0, Fld_Y0, -1)
End Sub

Private Sub Spn_Y0_SpinUp()
  Call IncDecfld(Y0, Fld_Y0, 1)
End Sub

Private Sub Timer1_Timer()
  Dim IsLock As Single
    DetErr = GetOMA(DC_COOLLOCK, IsLock)
    If IsLock <> 0 Then
      Lit_Cool.FillColor = &HFF00&
    Else
      Lit_Cool.FillColor = &HFF&
    End If
    DetErr = GetOMA(DC_COOLSTAT, IsLock)
    If IsLock <> 0 Then
      Lit_CStat.FillColor = &HFF&
    Else
      Lit_CStat.FillColor = &HFF00&
    End If
    DetErr = GetOMA(DC_DERROR, IsLock)
    If IsLock Then
      Lit_Status.FillColor = &HFFFF&
    Else
      Lit_Status.FillColor = &HFF00&
    End If
End Sub

Private Sub Tmr_Active_Timer()
  Static IsActive As Single
  DetErr = GetOMA(DC_ACTIVE, IsActive)
  If IsActive = 0 Then
    Btn_Run.Caption = "Run"
    Tmr_Active.Enabled = False
    Running = 0
  End If
End Sub

Private Sub Tmr_Cool_Timer()
  Dim IsLock As Single
    DetErr = GetOMA(DC_COOLLOCK, IsLock)
    If IsLock <> 0 Then
      Lit_Cool.FillColor = &HFF00&
    Else
      Lit_Cool.FillColor = &HFF&
    End If
    DetErr = GetOMA(DC_COOLSTAT, IsLock)
    If IsLock <> 0 Then
      Lit_CStat.FillColor = &HFF&
    Else
      Lit_CStat.FillColor = &HFF00&
    End If
    DetErr = GetOMA(DC_DERROR, IsLock)
    If IsLock Then
      Lit_Status.FillColor = &HFFFF&
    Else
      Lit_Status.FillColor = &HFF00&
    End If
End Sub


