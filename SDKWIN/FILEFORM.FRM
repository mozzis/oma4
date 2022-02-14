VERSION 4.00
Begin VB.Form Frm_File 
   Appearance      =   0  'Flat
   BackColor       =   &H00FFFFFF&
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Open File"
   ClientHeight    =   4680
   ClientLeft      =   2265
   ClientTop       =   2175
   ClientWidth     =   4560
   BeginProperty Font 
      name            =   "MS Sans Serif"
      charset         =   1
      weight          =   700
      size            =   8.25
      underline       =   0   'False
      italic          =   0   'False
      strikethrough   =   0   'False
   EndProperty
   ForeColor       =   &H00000080&
   Height          =   5085
   Left            =   2205
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4680
   ScaleWidth      =   4560
   Top             =   1830
   Width           =   4680
   Begin VB.TextBox Txt_FileSpec 
      Appearance      =   0  'Flat
      Height          =   285
      Left            =   120
      TabIndex        =   5
      Text            =   "*.*"
      Top             =   360
      Width           =   1815
   End
   Begin VB.FileListBox Lst_Files 
      Appearance      =   0  'Flat
      Height          =   3150
      Left            =   120
      TabIndex        =   4
      Top             =   840
      Width           =   1815
   End
   Begin VB.DirListBox Lst_Dirs 
      Appearance      =   0  'Flat
      Height          =   2955
      Left            =   2040
      TabIndex        =   3
      Top             =   1080
      Width           =   2295
   End
   Begin VB.DriveListBox Lst_Drives 
      Appearance      =   0  'Flat
      Height          =   315
      Left            =   2040
      TabIndex        =   2
      Top             =   360
      Width           =   2295
   End
   Begin VB.CommandButton Btn_OK 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Caption         =   "&OK"
      Default         =   -1  'True
      Height          =   495
      Left            =   840
      TabIndex        =   0
      Top             =   4080
      Width           =   975
   End
   Begin VB.CommandButton Btn_Cancel 
      Appearance      =   0  'Flat
      BackColor       =   &H00C0C0C0&
      Caption         =   "&Cancel"
      Height          =   495
      Left            =   2760
      TabIndex        =   1
      Top             =   4080
      Width           =   975
   End
   Begin VB.Label Lbl_Dirs 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&Directories:"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   2040
      TabIndex        =   8
      Top             =   840
      Width           =   1095
   End
   Begin VB.Label Lbl_Drives 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "Dri&ves:"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   2040
      TabIndex        =   7
      Top             =   120
      Width           =   735
   End
   Begin VB.Label Lbl_FileSpec 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      Caption         =   "&File Name:"
      ForeColor       =   &H80000008&
      Height          =   255
      Left            =   120
      TabIndex        =   6
      Top             =   120
      Width           =   1335
   End
End
Attribute VB_Name = "Frm_File"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit
Dim SelectingFile As Integer

Private Sub Btn_Cancel_Click()
  Unload Me
End Sub

Private Sub Btn_OK_Click()
  Unload Me
End Sub

Private Sub Form_Load()
 SelectingFile = False
End Sub

Private Sub Lst_Dirs_Change()
  ' Update File listbox to sync with Dir listbox.
  Lst_Files.Path = Lst_Dirs.Path
End Sub

Private Sub Lst_Dirs_Click()
  Lst_Files.Path = Lst_Dirs.List(Lst_Dirs.ListIndex)
End Sub

Private Sub Lst_Dirs_LostFocus()
    Lst_Dirs.Path = Lst_Dirs.List(Lst_Dirs.ListIndex)
End Sub

Private Sub Lst_Drives_Change()
    On Error GoTo DriveHandler
    Lst_Dirs.Path = Lst_Drives.Drive
    Exit Sub

DriveHandler:
    Lst_Drives.Drive = Lst_Dirs.Path
    Exit Sub

End Sub

Private Sub Lst_Files_Click()
  SelectingFile = True
  Txt_FileSpec.Text = Lst_Files.FileName
  SelectingFile = False
End Sub

Private Sub Txt_FileSpec_Change()
' Update file list box if user changes pattern.
    If SelectingFile = False Then
    Lst_Files.Pattern = Txt_FileSpec.Text
    End If
End Sub

Private Sub Txt_FileSpec_GotFocus()
    Txt_FileSpec.SelStart = 0      ' Highlight the current entry.
    Txt_FileSpec.SelLength = Len(Txt_FileSpec.Text)
End Sub

