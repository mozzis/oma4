VERSION 2.00
Begin Form Frm_Data 
   Caption         =   "Data Form"
   ClientHeight    =   4605
   ClientLeft      =   1245
   ClientTop       =   1290
   ClientWidth     =   5955
   Enabled         =   0   'False
   Height          =   4980
   Left            =   1200
   LinkTopic       =   "Form1"
   ScaleHeight     =   4605
   ScaleWidth      =   5955
   Top             =   960
   Width           =   6045
   Begin CommandButton Btn_Refresh 
      Caption         =   "Refresh"
      Height          =   375
      Left            =   2475
      TabIndex        =   1
      Top             =   3915
      Width           =   1005
   End
   Begin GRAPH Grf_Data 
      AsciiFSize      =   "120~100~100~100"
      BottomTitle     =   "Pixel"
      GraphCaption    =   "Data"
      GraphStyle      =   4
      GraphTitle      =   "Data"
      GraphType       =   6  'Line
      Height          =   3345
      LabelEvery      =   100
      Left            =   405
      NumPoints       =   512
      RandomData      =   0  'Off
      TabIndex        =   0
      ThickLines      =   0  'Off
      TickEvery       =   50
      Top             =   225
      Width           =   5100
      YAxisPos        =   1  'Left
      YAxisStyle      =   1  'Variable Origin
   End
End
Option Explicit

Sub Btn_Refresh_Click ()
  RefreshGraph
End Sub

Sub Form_Load ()
  If DoGraph Then
    Tracks.Max = 5
    If Tracks.Value > Tracks.Max Then Tracks.Value = Tracks.Max
    Grf_Data.NumSets = Tracks.Value
    If Points.Value > 1 Then
      Frm_Data.Grf_Data.NumPoints = Points.Value
    Else
      Frm_Data.Grf_Data.NumPoints = 2
    End If
  End If
End Sub

Sub Form_Unload (Cancel As Integer)
  DoGraph = False
  Frm_Setup.Mnu_Graph(41).Checked = False
  LimitTracks
End Sub

