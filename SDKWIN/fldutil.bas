Attribute VB_Name = "FLDUTIL"
Const ETFormat = "##,##0.0##"

Sub ChangeFld(FldTxt As TextBox, FParam As Param)
  Static Old As Single, NewVal As Single
  Static Recursing As Integer
  If Recursing Then
    Exit Sub
  Else
    Recursing = True
  End If
  On Error GoTo Fld_err
  Old = FParam.Value
  If IsNumeric(FldTxt.Text) = False Then
    On Error GoTo 0
    FldTxt.Text = Old
    Recursing = False
    Exit Sub
  End If
  FParam.Value = Val(FldTxt.Text)
  If FParam.Value > FParam.Max Then
    FParam.Value = FParam.Max
    FldTxt.Text = FParam.Value
  ElseIf FParam.Value < FParam.Min Then
    FParam.Value = FParam.Min
    FldTxt.Text = FParam.Value
  End If
  DetErr = SetOMA(FParam.Command, FParam.Value)
  SaveString = Str(Old)
  Call CheckFrameTime
  Recursing = False
Exit Sub
Fld_err:
  FParam.Value = Old
  FldTxt.Text = Old
  On Error GoTo 0
  Recursing = False
Resume Next
End Sub

Sub ChangeSb(ValPtr As Single, Sb As VScrollBar, Fld As TextBox)
 Incrementing = True
 ValPtr = Sb.Value
 Fld.Text = Str(Sb.Value)
 Incrementing = False
End Sub

Sub CheckFrameTime()
  Dim FParam As Single, TStr As String
  DetErr = GetOMA(DC_FTIME, FParam)
  TStr = Format$(FParam, ETFormat & " ")
  If (FParam) < 0.001 Then
  TStr = TStr & "u"
  ElseIf FParam < 1 Then
  TStr = TStr & "m"
  End If
  TStr = TStr & "sec/Frame"
  Frm_Setup.Lbl_FTime.Caption = TStr
End Sub

Function FormatET(Old As Variant) As String
  FormatET = Format$(Old, ETFormat)
End Function

Sub IncDecfld(FParam As Param, FldTxt As TextBox, Delta As Single)
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

