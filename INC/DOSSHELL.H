/*
/   DOSSHELL.H
/
/      spawn a DOS shell as a new process
/
/*************************************************************************/

#ifndef DOSSHELL_INCLUDED
#define DOSSHELL_INCLUDED

int ShellCmd(char * cmd_string);
void do_DOS_commands(unsigned dummy);

#endif  // DOSSHELL_INCLUDED
