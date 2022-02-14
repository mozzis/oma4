/*
RMINT86.H -- rm_int86() and rm_int86x() -- definition
Must be included after DOS.H
Real-mode interrupts from 386|DOS-Extender
*/

#include "utypes.h"

#ifdef __WATCOMC__
// Watcom C union REGS field names are nonstatndard
#define ax  eax
#define bx  ebx
#define cx  ecx
#define dx  edx
#define si  esi
#define di  edi

#define MK_FARPTR(fp, s, o)\
     (fp) = MK_FP((s), (o))
#endif

extern int rm_int86(int intno, union REGS *in, union REGS *out);
extern int rm_int86x(int intno, union REGS *in, union REGS *out, 
                         struct SREGS *sregs);

#ifdef __WATCOMC__
#define int86(a, b, c) rm_int86(a, b, c)
#define int86x(a, b, c, d) rm_int86x(a, b, c, d)
#endif

USHORT alloc_conventional(unsigned para);
USHORT free_conventional(unsigned addr);
