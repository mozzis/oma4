VERSION 2.00
Begin Form Frm_Random 
   Caption         =   "Frm_Random"
   ClientHeight    =   4275
   ClientLeft      =   1245
   ClientTop       =   6300
   ClientWidth     =   4065
   Height          =   4680
   Left            =   1185
   LinkTopic       =   "Form1"
   ScaleHeight     =   4275
   ScaleWidth      =   4065
   Top             =   5955
   Width           =   4185
   Begin TextBox CurrRow 
      Height          =   300
      Left            =   2880
      TabIndex        =   15
      Text            =   "CurrRow"
      Top             =   255
      Width           =   810
   End
   Begin TextBox CurrCol 
      Height          =   300
      Left            =   2895
      TabIndex        =   14
      Text            =   "CurrCol"
      Top             =   1170
      Width           =   810
   End
   Begin TextBox SetRow 
      Height          =   300
      Left            =   2865
      TabIndex        =   13
      Text            =   "SetRow"
      Top             =   615
      Width           =   810
   End
   Begin SSPanel Pnl_ScanAdv 
      BevelInner      =   1  'Inset
      Height          =   3795
      Left            =   0
      Outline         =   -1  'True
      TabIndex        =   0
      Top             =   0
      Width           =   2250
      Begin TextBox Fld_TrkEnter 
         BackColor       =   &H00FF0000&
         BorderStyle     =   0  'None
         FontBold        =   0   'False
         FontItalic      =   0   'False
         FontName        =   "MS Sans Serif"
         FontSize        =   8.25
         FontStrikethru  =   0   'False
         FontUnderline   =   0   'False
         ForeColor       =   &H00FFFFFF&
         Height          =   225
         Left            =   720
         MaxLength       =   4
         TabIndex        =   9
         TabStop         =   0   'False
         Text            =   "0"
         Top             =   450
         Width           =   540
      End
      Begin Grid Grd_Points 
         BackColor       =   &H00FFFFFF&
         Cols            =   3
         ForeColor       =   &H00000000&
         Height          =   1290
         Left            =   90
         ScrollBars      =   2  'Vertical
         TabIndex        =   8
         Top             =   2100
         Width           =   2055
      End
      Begin Grid Grd_Tracks 
         BackColor       =   &H00FFFFFF&
         Cols            =   3
         FontBold        =   0   'False
         FontItalic      =   0   'False
         FontName        =   "MS Sans Serif"
         FontSize        =   8.25
         FontStrikethru  =   0   'False
         FontUnderline   =   0   'False
         ForeColor       =   &H00000000&
         Height          =   1310
         HighLight       =   0   'False
         Left            =   75
         ScrollBars      =   2  'Vertical
         TabIndex        =   6
         Top             =   420
         Width           =   2055
      End
      Begin TextBox Fld_RPoints 
         BackColor       =   &H00404040&
         ForeColor       =   &H0000C000&
         Height          =   285
         Left            =   945
         TabIndex        =   3
         Text            =   "Points"
         Top             =   1785
         Width           =   645
      End
      Begin TextBox Fld_RTracks 
         BackColor       =   &H00404040&
         ForeColor       =   &H0000C000&
         Height          =   285
         Left            =   975
         TabIndex        =   2
         Text            =   "Tracks"
         Top             =   135
         Width           =   645
      End
      Begin TextBox Fld_RAntibloom 
         BackColor       =   &H00404040&
         ForeColor       =   &H0000C000&
         Height          =   285
         Left            =   1005
         TabIndex        =   1
         Text            =   "Text1"
         Top             =   3405
         Width           =   645
      End
      Begin SpinButton Spn_RPoints 
         Delay           =   200
         Height          =   350
         Left            =   1560
         ShadowThickness =   2
         TdThickness     =   2
         Top             =   1770
         Width           =   285
      End
      Begin SpinButton Spn_RTracks 
         Delay           =   200
         Height          =   350
         Left            =   1650
         ShadowThickness =   2
         TdThickness     =   2
         Top             =   105
         Width           =   285
      End
      Begin SpinButton Spn_RAntibloom 
         Delay           =   200
         Height          =   350
         Left            =   1650
         ShadowThickness =   2
         TdThickness     =   2
         Top             =   3375
         Width           =   285
      End
      Begin Label Lbl_RPoints 
         AutoSize        =   -1  'True
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "Points"
         ForeColor       =   &H00404040&
         Height          =   195
         Left            =   345
         TabIndex        =   7
         Top             =   1830
         Width           =   540
      End
      Begin Label Lbl_RTracks 
         AutoSize        =   -1  'True
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "Tracks"
         ForeColor       =   &H00404040&
         Height          =   225
         Left            =   360
         TabIndex        =   5
         Top             =   165
         Width           =   600
      End
      Begin Label Lbl_RAntibloom 
         AutoSize        =   -1  'True
         BackColor       =   &H00C0C0C0&
         BackStyle       =   0  'Transparent
         Caption         =   "Antibloom"
         ForeColor       =   &H00404040&
         Height          =   195
         Left            =   150
         TabIndex        =   4
         Top             =   3450
         Width           =   840
      End
   End
   Begin Label Label1 
      Caption         =   "Row"
      Height          =   285
      Left            =   2355
      TabIndex        =   10
      Top             =   285
      Width           =   510
   End
   Begin Label Label2 
      Caption         =   "Col"
      Height          =   285
      Left            =   2355
      TabIndex        =   11
      Top             =   1170
      Width           =   510
   End
   Begin Label Label3 
      Caption         =   "Set Row"
      Height          =   465
      Left            =   2325
      TabIndex        =   12
      Top             =   540
      Width           =   510
   End
End
Const CellWidth = 600
Const CellHeight = 250

Sub FillInTracks ()
  Static t As Single
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

Sub Fld_RAntibloom_Change ()
  Call ChangeFld(Fld_RAntibloom, Antibloom)
End Sub

Sub Fld_RTracks_Change ()
  Call LimitTracks
  Call ChangeFld(Fld_RTracks, Tracks)
  Grd_Tracks.Rows = Tracks.Value + 1
  Call FillInTracks
End Sub

Sub Form_Load ()
  Static t As Single
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

  Grd_Points.Rows = 3  ' Set rows and columns.
  Grd_Points.Cols = 3
  Grd_Points.Row = 0
  Grd_Points.Col = 0
  Grd_Points.Text = "Point"
  Grd_Points.Col = 1
  Grd_Points.Text = "  X0"
  Grd_Points.Col = 2
  Grd_Points.Text = "DeltaX"
  Grd_Points.Row = 1
  For j = 1 To 1
    Grd_Points.Row = j
    Grd_Points.Col = 0
    Grd_Points.Text = j
    For i = 1 To 2
      Grd_Points.Col = i
      Grd_Points.ColWidth(i) = 555 + 150
    Next i
  Next j
  Grd_Points.Col = 1
  Grd_Points.Row = 1
  Fld_RPoints.Text = Format$(Points.Value, "####")
  Fld_RTracks.Text = Format$(Tracks.Value, "####")
  Fld_RAntibloom.Text = Format$(Antibloom.Value, "###")
  Fld_TrkEnter.Height = CellHeight - 8
  Fld_TrkEnter.Width = CellWidth - 32
  SetSelCell
End Sub

Sub Grd_Tracks_Click ()
  Beep
  Beep
  Beep
  Beep
End Sub

Sub Grd_Tracks_RowColChange ()
  SetSelCell
End Sub

Sub IncDecfld (FParam As Param, FldTxt As TextBox, Delta As Single)
  Static Old As Single
  On Error GoTo Sp_err
  Old = FParam.Value
  FParam.Value = FParam.Value + Delta
  If FParam.Value > FParam.Max Then FParam.Value = FParam.Min
  If FParam.Value < FParam.Min Then FParam.Value = FParam.Max
  FldTxt.Text = FParam.Value
  SaveString = Str(Old)
  Exit Sub
Sp_err:

End Sub

Sub SetSelCell ()
  Col = Grd_Tracks.Col
  Row = (Grd_Tracks.Row Mod 5) - (1 * Grd_Tracks.Row > 4)
  CurrRow.Text = Grd_Tracks.Row
  SetRow.Text = Row
  CurrCol.Text = Grd_Tracks.Col
  Fld_TrkEnter.Left = Grd_Tracks.Left + ((Col * CellWidth) - 4)
  Fld_TrkEnter.Top = Grd_Tracks.Top + (Row * (CellHeight + 8)) + 14
  Fld_TrkEnter.Text = Grd_Tracks.Text
  Grd_Tracks.Refresh
End Sub

Sub Spn_RAntibloom_SpinDown ()
  Call IncDecfld(Antibloom, Fld_RAntibloom, -10)
End Sub

Sub Spn_RAntibloom_SpinUp ()
  Call IncDecfld(Antibloom, Fld_RAntibloom, -10)
End Sub

Sub Spn_RTracks_SpinDown ()
  Call IncDecfld(Tracks, Fld_RTracks, -1)
End Sub

Sub Spn_RTracks_SpinUp ()
  Call IncDecfld(Tracks, Fld_RTracks, 1)
End Sub

