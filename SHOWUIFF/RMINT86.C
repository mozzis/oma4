/*
RMINT86.X --  rm_int86() and rm_int86x() -- implementation
Real-mode interrupts from 386|DOS-Extender

 In some cases when a PharLap program must call a real mode
 interrupt such as a BIOS routine, the segment registers must
 contain a  segment invalid for protected mode, so int386 and
 int386x cannot be used.  These functions use the PharLap
 function 2511 to access a real mode interrupt

*/

#include <stdlib.h>
#include <stdio.h>
#include <dos.h>

#include "rmint86.h"

typedef struct {
  unsigned short intno, ds, es, fs, gs;
  unsigned long eax, edx;
  } RMODE_PARAM_BLOCK;

#define RMODE_INTER 0x2511
#define CLEAR(x) memset(&x, 0, sizeof(x))

int rm_int86(short intno, union REGS *in, union REGS *out)
{
  RMODE_PARAM_BLOCK rpb;
  unsigned long ret;

  CLEAR(rpb);
  rpb.intno = intno;
  rpb.eax = in->x.ax;
  rpb.edx = in->x.dx;
  in->x.ax = RMODE_INTER;
  in->x.dx = (unsigned) &rpb;
  ret = intdos(in, out);
  out->x.dx = rpb.edx;
  return ret;
}

int rm_int86x(short intno, union REGS *in, union REGS *out,
              struct SREGS *sregs)
{
  RMODE_PARAM_BLOCK rpb;
  unsigned ret;

  CLEAR(rpb);
  rpb.intno = intno;
  rpb.eax = in->x.ax;
  rpb.edx = in->x.dx;
  rpb.ds = sregs->ds;
  rpb.es = sregs->es;
  rpb.fs = rpb.gs = 0;  
  in->x.ax = RMODE_INTER;
  in->x.dx = (unsigned) &rpb;
  ret = intdos(in, out);
  out->x.dx = rpb.edx;
  sregs->ds = rpb.ds;
  sregs->es = rpb.es;
  return ret;
}

#define CONV_ALLOC 0x25c0
#define CONV_FREE  0x25c1

unsigned short alloc_conventional(unsigned short para)
{
  union REGS r;
  
  CLEAR(r);

  r.x.ax = 0x2537;
  r.x.bx = para;
  intdos(&r, &r);

  r.x.ax = CONV_ALLOC;
  r.x.bx = para;
  intdos(&r, &r);
  return(r.x.cflag ? 0 : r.x.ax);
}

unsigned short free_conventional(unsigned short addr)
{
  union REGS r;
  
  CLEAR(r);
  r.x.ax = CONV_FREE;
  r.x.bx = addr;
  intdos(&r, &r);
  return(r.x.cflag == 0);
}
