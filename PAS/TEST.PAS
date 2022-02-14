
var trial : integer;
    inch  : byte;
    infile : FILE;

begin
assign(infile,'CON');
writeln('this is a test');
write(' (Press a key) ');
repeat
trial := keypressed();
until trial;
read(infile, inch);
{ inch := readch(); }
writeln('You pressed', inch);
end;
