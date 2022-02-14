var i, j : integer;

begin
  j := DG_tracks();
  buflen := 4;
  writeln();
  for i := 0 to j-1 do begin
   write('<',i,':',cs[0].[i].src_comp,'>,');
  end
end;
