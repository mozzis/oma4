program spec;
var a, i, j: integer;
procedure delay;
begin
  a := i;
end;

begin
  BUFLEN := 10;
  for j := 1 to 100 do
  begin
    writeln();
    write(j);
    delay();
  end;
end.
