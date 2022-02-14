Option Explicit
'Params are used to store detector parameters
Type Param
  Value As Single     'Current value of parameter
  Max As Single       'Maximum legal value
  Min As Single       'Minimum legal value
  Command As Integer  'Val of command to set or get param
End Type

Global Antibloom As Param
Global ActiveX As Param
Global ActiveY As Param
Global DAProg As Param
Global DeltaX As Param
Global DeltaY As Param
Global DetTemp As Param
Global DModel As Param
Global DCType As Param
Global ExpTime As Param
Global Ignores As Param
Global Mems As Param
Global Points As Param
Global Preps As Param
Global PTime As Param
Global Scans As Param
Global Shutter As Param
Global Sreg As Param
Global STime As Param
Global Tracks As Param
Global TrackMode As Param
Global WFSO As Param
Global X0 As Param
Global Y0 As Param

Global ETScaler As Single        'Ditto
Global DetErr As Integer
Global Incrementing As Integer   'Flag to eliminate recursion
Global SaveString As String      'For Cut & Paste ops
Global FileName As String        'Name returned by Frm_FIle
Global Instance As Integer       'Used to release DLL

Function ExpTimeToSbCount (ETVal As Single) As Integer
  Dim Fval As Single
  If ETVal > 0 Then
    Fval = (Log10(ETVal) - Log10(ExpTime.Min)) / ETScaler
    Fval = Fval - 32767
  Else
    Fval = -32767
  End If
  ExpTimeToSbCount = Fval
End Function

Function GetOMA% (ByVal Cmd%, Setting!)
  Static lcount
  If DModel.Value > -1 Then
    GetOMA% = GetParam%(Cmd, Setting)
  Else
    Select Case Cmd
      Case DC_THERE, DC_DMODEL, DC_X0, DC_Y0, DC_DELTAX, DC_TRACK, DC_TRACKS
	Setting = 1#
      Case DC_TRKMODE, DC_DAPROG, DC_I, DC_J
	Setting = 1#
      Case DC_L
	Setting = lcount
	lcount = lcount + 1
      Case DC_DELTAY
	Setting = 1#
      Case DC_DERROR, DC_SHUTMODE, DC_COOLONOFF, DC_RUN, DC_STOP, DC_WFSO, DC_WFSC
	Setting = 0#
      Case DC_POINTS, DC_ACTIVEX, DC_ACTIVEY
	Setting = 512#
      Case DC_ANTIBLOOM
	Setting = 100#
      Case DC_MINET
	Setting = .01
      Case DC_MAXET
	Setting = 120
      Case DC_WFSO
	Setting = 1
      Case DC_ET, DC_SAMEET, DC_FTIME
	Setting = .04
      Case DC_PTIME
	Setting = .00001
      Case DC_BYTES
	Setting = 2048#
      Case DC_MAXMEM
	Setting = 128
      Case Else
	Setting = 0
    End Select
    GetOMA% = 0
  End If
End Function

'Set the Max and Min values and the command value for all
'of the detector parameters
'Then query the driver for the actual values
Sub InitParams ()
  Dim FParam As Single
  
  DetErr = GetOMA(DC_ACTIVEX, ActiveX.Value)
  DetErr = GetOMA(DC_ACTIVEY, ActiveY.Value)
  DetErr = GetOMA(DC_ANTIBLOOM, Antibloom.Value)
  DetErr = GetOMA(DC_DAPROG, DAProg.Value)
  DetErr = GetOMA(DC_DELTAX, DeltaX.Value)
  DetErr = GetOMA(DC_DELTAY, DeltaY.Value)
  DetErr = GetOMA(DC_DTEMP, DetTemp.Value)
  DetErr = GetOMA(DC_ET, ExpTime.Value)
  DetErr = GetOMA(DC_K, Ignores.Value)
  DetErr = GetOMA(DC_J, Mems.Value)
  DetErr = GetOMA(DC_POINTS, Points.Value)
  DetErr = GetOMA(DC_PTIME, PTime.Value)
  DetErr = GetOMA(DC_H, Preps.Value)
  DetErr = GetOMA(DC_I, Scans.Value)
  DetErr = GetOMA(DC_SHUTMODE, Shutter.Value)
  DetErr = GetOMA(DC_OUTREG, Sreg.Value)
  DetErr = GetOMA(DC_STIME, STime.Value)
  DetErr = GetOMA(DC_TRACKS, Tracks.Value)
  DetErr = GetOMA(DC_TRKMODE, TrackMode.Value)
  DetErr = GetOMA(DC_WFSO, WFSO.Value)
  DetErr = GetOMA(DC_X0, X0.Value)
  DetErr = GetOMA(DC_Y0, Y0.Value)

  Antibloom.Max = 100
  Antibloom.Min = 0
  Antibloom.Command = DC_ANTIBLOOM
  ActiveX.Command = DC_ACTIVEX
  ActiveY.Command = DC_ACTIVEY
  DAProg.Command = DC_DAPROG
  DetErr = GetOMA(DC_DAMAX, DAProg.Max)
  DAProg.Min = 1
  DeltaX.Command = DC_DELTAX
  DeltaX.Max = ActiveX.Value
  DeltaX.Min = 1
  DeltaY.Command = DC_DELTAY
  DeltaY.Max = ActiveY.Value
  DeltaY.Min = 1
  DetTemp.Command = DC_DTEMP
  If DCType.Value = 2 Then ' If Cryo Cooler
    DetTemp.Max = 20
    DetTemp.Min = -80
  Else
    DetTemp.Max = 0
    DetTemp.Min = -120
  End If
  ExpTime.Command = DC_ET
  DetErr = GetOMA(DC_MAXET, ExpTime.Max)
  DetErr = GetOMA(DC_MINET, ExpTime.Min)
  If ExpTime.Min < .000003 Then ExpTime.Min = .000003
  If ExpTime.Max < .000003 Then ExpTime.Max = .01
  FParam = Log10(ExpTime.Max) - Log10(ExpTime.Min)
  If FParam > 0 Then
    ETScaler = FParam / 65535#
  Else
    ETScaler = Log10(ExpTime.Max) / 65535#
  End If
  Ignores.Command = DC_K
  Ignores.Max = 32767
  Mems.Command = DC_J
  DetErr = GetOMA(DC_MAXMEM, Mems.Max)
  Mems.Min = 1
  Points.Command = DC_POINTS
  Points.Max = ActiveX.Value
  Points.Min = 1
  Preps.Command = DC_H
  Preps.Max = 32767
  Preps.Min = 0
  Tracks.Command = DC_TRACKS
  Tracks.Min = 1
  TrackMode.Command = DC_TRKMODE
  Call LimitTracks
  Scans.Command = DC_I
  Scans.Max = 32767
  Scans.Min = 1
  Shutter.Command = DC_SHUTMODE
  Shutter.Max = 2
  Shutter.Min = 0
  Sreg.Command = DC_OUTREG
  WFSO.Command = DC_WFSO
  X0.Command = DC_X0
  X0.Max = ActiveX.Value
  X0.Min = 1
  Y0.Command = DC_Y0
  Y0.Max = ActiveY.Value
  Y0.Min = 1

End Sub

Sub LimitTracks ()
  Tracks.Max = ActiveY.Value
  If Tracks.Value > Tracks.Max Then
    Tracks.Value = Tracks.Max
  End If
End Sub

Static Function Log10 (X)
  Log10 = Log(X) / Log(10#)
End Function

Function SbCountToExpTime (ByVal SbCount As Integer) As Single
  Dim ETCount As Long, Fval As Single
  ETCount = (SbCount + 32767&)
  Fval = ETCount * ETScaler
  Fval = Fval + Log10(ExpTime.Min)
  Fval = 10# ^ Fval
  SbCountToExpTime = Fval
End Function

Function SetOMA% (ByVal Cmd%, ByVal Setting!)
  If DModel.Value > -1 Then
    SetOMA% = SetParam%(Cmd, Setting)
  Else
    SetOMA% = 1
  End If
End Function

