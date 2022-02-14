/* -----------------------------------------------------------------------
/
/  oma4000.h
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header: /OMA4000/Include/OMA4000.H 1     6/18/99 1:53a Maynard $
*/

#ifndef OMA4000_INCLUDED
#define OMA4000_INCLUDED
  
extern const char VersionString[];

void nop(void);
void copyright_page(void);
void control_C_trap(void);
void floating_point_error_trap(void);
void close_GSS_workstation(void);
  
#endif   // OMA4000_INCLUDED
