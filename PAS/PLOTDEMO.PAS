{ --------------------------------------------------------------------
/
/	program plotdemo;
/
/	Demonstrates the ten different plot window arrangements.
/	The curve directory must have at least one entry.
/ -------------------------------------------------------------------- }

program plotdemo;

var i, j, k, windows: integer;


{ --------------------------------------------------------------------
/
/	procedure prep_windows;
/
/	Initializes all windows to a similar style.  'Style',
/	as used here, means most of the parameters associated
/	with a plot window, which are usually set in the plot
/	setup menu.
/ -------------------------------------------------------------------- }

procedure prep_windows;
begin
    for i := 1 to 8 do
    begin
        s_active_window(i);
        s_plot_style(
            0.0, 1030.0,	{ x axis min and max values }
            0.0, 10000.0,	{ y axis min and max values }
            0.0, 2.0,		{ z axis min and max values }
            0,0,0,		{ x,y,z units (pixels) }
            'P L O T   D E M O', { plot title }
            'T I M E', 		{ x axis lable }
            'P O W E R', 	{ y axis lable }
            'ZZZZZZZZZ', 	{ z axis lable }
            1,			{ z - left side }
            20,			{ z/x percent }
            20,			{ z/y percent }
            1,			{ line style - overlapped }
            1); 		{ autoscale }
    end;
    s_active_window(1)
end;

{ --------------------------------------------------------------------
/
/	procedure show_styles;
/
/	Demonstrates the ten possible window styles. 'Style', as
/	used here, means the different arrangements of plot windows
/	on the screen.  The various layouts have different numbers
/	of plot windows in them, so each time a new layout is 
/	used, we have to determine how many there are, and fill
/	them with plot data.
/	
/ -------------------------------------------------------------------- }

procedure show_styles;
begin

    cs[0].display := 255;  { all bits on - show CurveSet 0 in all windows }

    for i := 1 to 10 do
    begin
        writeln();
        buflen := 25;
        write('This is window style');
        buflen := 3;
        write(i);

        s_window_style(i);   { select style from loop index }

        if (i = 1) then		{ set window count from style (loop) index }
            windows := 1
        else if (i = 2) then
            windows := 2
        else if (i = 3) then
            windows := 2
        else if (i = 4) then
            windows := 3
        else if (i = 5) then
            windows := 3
        else if (i = 6) then
            windows := 3
        else if (i = 7) then
            windows := 4
        else if (i = 8) then
            windows := 4
        else if (i = 9) then
            windows := 6
        else if (i = 10) then
            windows := 8
        else 
            windows := 1; 

        for j := 1 to windows do	{ now draw all windows on screen }
            plot_window('SCREEN', j); 

	if (i <> 10) then		{ if not last one, wait for <CR> }
	begin
            buflen := 30;
            write('  Press Enter for next style'); 
            readln()
	end;
    end;
{    s_window_style(1); 
    plot_window('SCREEN', 1) }		{ go back to style 1 ?? }
end;


{ --------------------------------------------------------------------
/	program plotdemo; (main block)
/ -------------------------------------------------------------------- }

begin
    prep_windows(); 
    show_styles() 
end.

