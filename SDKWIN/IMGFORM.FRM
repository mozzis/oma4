VERSION 2.00
Begin Form Frm_Img 
   Caption         =   "Image Setup"
   ClientHeight    =   5970
   ClientLeft      =   7140
   ClientTop       =   2235
   ClientWidth     =   7305
   Height          =   6345
   Left            =   7095
   LinkTopic       =   "Form1"
   ScaleHeight     =   5970
   ScaleWidth      =   7305
   Top             =   1905
   Width           =   7395
   Begin SSPanel Pnl_ScanBas 
      BackColor       =   &H00C0C0C0&
      BevelInner      =   1  'Inset
      BevelWidth      =   0
      BorderWidth     =   0
      Font3D          =   0  'None
      Height          =   3405
      Left            =   420
      Outline         =   -1  'True
      TabIndex        =   0
      Top             =   420
      Visible         =   0   'False
      Width           =   2160
      Begin TextBox Fld_ROI_szX 
         BackColor       =   &H00404040&
         ForeColor       =   &H0000C000&
         Height          =   285
         Left            =   135
         TabIndex        =   6
         Text            =   "Text1"
         Top             =   2310
         Width           =   735
      End
      Begin TextBox Fld_ROIY0 
         Alignment       =   1  'Right Justify
         BackColor       =   &H00404040&
         ForeColor       =   &H0000C000&
         Height          =   285
         Left            =   135
         TabIndex        =   5
         Text            =   "Text1"
         Top             =   735
         Width           =   735
      End
      Begin TextBox Fld_ROIdY 
         BackColor       =   &H00404040&
         ForeColor       =   &H0000C000&
         Height          =   285
         Left            =   135
         TabIndex        =   4
         Text            =   "Text1"
         Top             =   1815
         Width           =   735
      End
      Begin TextBox Fld_ROIszY 
         BackColor       =   &H00404040&
         ForeColor       =   &H0000C000&
         Height          =   285
         Left            =   135
         TabIndex        =   3
         Text            =   "Text1"
         Top             =   2865
         Width           =   735
      End
      Begin SpinButton Spn_ROIdX 
         Delay           =   200
         Height          =   465
         Left            =   885
         ShadowThickness =   2
         TdThickness     =   2
         Top             =   1200
         Width           =   375
      End
      Begin SpinButton Spn_ROIszX 
         Delay           =   200
         Height          =   465
         Left            =   885
         ShadowThickness =   2
         TdThickness     =   2
         Top             =   2250
         Width           =   375
      End
      Begin SpinButton Spn_ROIY0 
         Delay           =   200
         Height          =   465
         Left            =   885
         ShadowThickness =   2
         TdThickness     =   2
         Top             =   690
         Width           =   375
      End
      Begin SpinButton Spn_ROIdY 
         Delay           =   200
         Height          =   465
         Left            =   885
         ShadowThickness =   2
         TdThickness     =   2
         Top             =   1740
         Width           =   375
      End
      Begin SpinButton Spn_ROIszY 
         Delay           =   200
         Height          =   465
         Left            =   885
         ShadowThickness =   2
         TdThickness     =   2
         Top             =   2805
         Width           =   375
      End
      Begin SpinButton Spn_ROIX0 
         Delay           =   200
         Height          =   465
         Left            =   885
         ShadowThickness =   2
         TdThickness     =   2
         Top             =   165
         Width           =   375
      End
      Begin TextBox Fld_RoiX0 
         BackColor       =   &H00404040&
         ForeColor       =   &H0000C000&
         Height          =   285
         Left            =   135
         TabIndex        =   2
         Text            =   "Text1"
         Top             =   225
         Width           =   735
      End
      Begin TextBox Fld_ROIdX 
         BackColor       =   &H00404040&
         ForeColor       =   &H0000C000&
         Height          =   285
         Left            =   135
         TabIndex        =   1
         Text            =   "Text1"
         Top             =   1245
         Width           =   735
      End
      Begin Label Lbl_ImgSz 
         AutoSize        =   -1  'True
         BackStyle       =   0  'Transparent
         Caption         =   "Size"
         Height          =   195
         Index           =   1
         Left            =   1480
         TabIndex        =   13
         Top             =   2790
         Width           =   375
      End
      Begin Label Lbl_ImgPix 
         AutoSize        =   -1  'True
         BackStyle       =   0  'Transparent
         Caption         =   "Size"
         Height          =   195
         Index           =   1
         Left            =   1480
         TabIndex        =   17
         Top             =   1740
         Width           =   375
      End
      Begin Label Lbl_ImgOrg 
         AutoSize        =   -1  'True
         BackStyle       =   0  'Transparent
         Caption         =   "Origin"
         Height          =   195
         Index           =   1
         Left            =   1480
         TabIndex        =   18
         Top             =   720
         Width           =   510
      End
      Begin Label Lbl_ImgSz 
         AutoSize        =   -1  'True
         BackStyle       =   0  'Transparent
         Caption         =   "Image"
         Height          =   195
         Index           =   0
         Left            =   1480
         TabIndex        =   16
         Top             =   2580
         Width           =   525
      End
      Begin Label Lbl_ImgPix 
         AutoSize        =   -1  'True
         BackStyle       =   0  'Transparent
         Caption         =   "Pixel"
         Height          =   195
         Index           =   0
         Left            =   1480
         TabIndex        =   15
         Top             =   1530
         Width           =   420
      End
      Begin Label Lbl_ImgOrg 
         AutoSize        =   -1  'True
         BackStyle       =   0  'Transparent
         Caption         =   "Image "
         Height          =   195
         Index           =   0
         Left            =   1480
         TabIndex        =   14
         Top             =   495
         Width           =   585
      End
      Begin Label Lbl_ImgszY 
         AutoSize        =   -1  'True
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "Y"
         ForeColor       =   &H00404040&
         Height          =   195
         Left            =   1305
         TabIndex        =   12
         Top             =   2940
         Width           =   135
      End
      Begin Label Lbl_ImgdY 
         AutoSize        =   -1  'True
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "Y"
         ForeColor       =   &H00404040&
         Height          =   195
         Left            =   1305
         TabIndex        =   11
         Top             =   1890
         Width           =   135
      End
      Begin Label Lbl_ImgY0 
         AutoSize        =   -1  'True
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "Y"
         ForeColor       =   &H00404040&
         Height          =   195
         Left            =   1305
         TabIndex        =   10
         Top             =   825
         Width           =   135
      End
      Begin Label Lbl_ImgszX 
         AutoSize        =   -1  'True
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "X"
         ForeColor       =   &H00404040&
         Height          =   195
         Left            =   1305
         TabIndex        =   9
         Top             =   2385
         Width           =   135
      End
      Begin Label Lbl_ImgdX 
         AutoSize        =   -1  'True
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "X"
         ForeColor       =   &H00404040&
         Height          =   195
         Left            =   1305
         TabIndex        =   8
         Top             =   1320
         Width           =   135
      End
      Begin Label Lbl_ImgX0 
         AutoSize        =   -1  'True
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "X"
         ForeColor       =   &H00404040&
         Height          =   195
         Left            =   1305
         TabIndex        =   7
         Top             =   315
         Width           =   135
      End
   End
End
Option Explicit

