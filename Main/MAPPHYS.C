#include <stdio.h>
#include <dos.h>
#include <pmode.h>                /* includes dos16.h nested */
#include "mapphys.h"

// #define MAX_ENTRY_BYTES  (0x400000 - 0x1000) /* max bytes per page table */

/* apparently this was done in case phys address was not on 4K boundary */
/* so routine could allocate up to 4K more space and force phys address */
/* to (lower) 4K even address.  For OMA, I assume address on page bound */

#define MAX_ENTRY_BYTES  (0x400000)

#pragma optimize("", off)

unsigned long MapPhysVCPI(unsigned long physical, /* hardware address */
                          unsigned long bytes,    /* size of block */
                          int mem_mapped_IO)      /* true disables cache */
{
  XBRK far *xbrkp;                    /* structure with page dir info */
  D16REGS r;
  unsigned long far *pgdirp,          /* pointer to page directory table */
                far *pgtabp,          /* pointer to new page table block */
                ptbl_abs_addr,        /* linear address of page table */
                phys_seg;             /* physical address in segments */
  unsigned dirno;                     /* dir entry number */
  int i, n, pa_tbl_count, notfound = 1;
  char far *page_table_block;         /* pointer to page table */

  r.ax = 0xBF02;                      /* D16 call to get ptr to xbrk struct */

  D16rmInterrupt(_d16info.D16_intno, &r, &r);

  xbrkp =  D16ProtectedPtr((void far *) (((long) r.dx << 16) | r.si), 0);

  pgdirp = D16ProtectedPtr((void far *) ((long) xbrkp->pgtab << 16), 0);

  if (bytes < MAX_ENTRY_BYTES ) bytes = MAX_ENTRY_BYTES; 
  pa_tbl_count = (int) (bytes / MAX_ENTRY_BYTES) + 1; /* # page tables needed */

  /* starting at 32MB, find linear range we can use */

  for (dirno = 1; dirno < 1024 && notfound; dirno++)/* search all of dir */
    {                                               /* for needed # of */
    for (i = notfound = 0; i < pa_tbl_count;i++)    /* contig empty entries */
      {
      if (pgdirp[dirno+i]) 
        { notfound = 1; break; }
      }
    if (notfound) continue;
    }

  if (dirno >= 1024)  /* failure if at end of directory */
    return(0L);
  /************************************************************/
  /* now set up page tables and store in page table directory */
  /************************************************************/

  phys_seg = physical;                    /* start at given phys address */
  for (i = 0; i < pa_tbl_count; i++)      /* make req. # of page tables */
    {
    n = D16MemStrategy(MForceLow);        /* need pagetable in low mem? */
                                          /* only for certain D16M funx? */
    page_table_block = D16MemAlloc(8192); /* alloc enough space to be sure */
                                          /* of 4K starting on 4K boundary */
    D16MemStrategy(n);

    if (! page_table_block)
      return(0L);
                                        
    D16Lock(page_table_block);     /* must never move! */
                                   /* (although '386 guide implies it can) */
    
    /* get address of middle of block */

	  ptbl_abs_addr = D16PhysAddress(D16AbsAddress((char far *)page_table_block + 4096));

    if (ptbl_abs_addr!= -1)       /* set PDE for new table: R/W, present */
      {
      pgdirp[dirno + i] =
        (ptbl_abs_addr & 0xFFFFF000) | 0x03; /* clear low 12 bits of addr */

      /* form protected pointer to table - hope it points to same addr! */

      pgtabp =                               
        (long far *) (page_table_block + (4096 - (ptbl_abs_addr & 0xFFF)));
      }
    else
      return 0L;   /* can't get address of table - maybe out of GDT space? */

    /* now re-use ptbl_abs_addr to track mapped physical 4K blocks */

    ptbl_abs_addr = (phys_seg & 0xFFFFF000) | 0x1F;  /* PTE: present, R/W, user, nocache */

    /* round up # of pages needed, account for region not starting on page bound */

//    n = (int)((MAX_ENTRY_BYTES + 4095 + (phys_seg & 0xFFF)) >> 12); //doesn't always work
    n = 1024; /* this to assume phys region starts on 4K bound & is 4M big */

    /* place PTEs (page table entries) for each 4K to be mapped */

    while (--n >= 0)
      {
      *pgtabp++ = ptbl_abs_addr;                     /* add PTE */
      ptbl_abs_addr += 0x1000L;                       /* next page */
      }
    phys_seg += MAX_ENTRY_BYTES;     /* increment physical address */
    }
  D16SegCancel(xbrkp);
  D16SegCancel(pgdirp);
  return ((long) dirno << 22) + (physical & 0xFFF);
}

#pragma optimize("", on)

unsigned long MapPhysDPMI(unsigned long physical, /* hardware address */
                          unsigned long bytes,    /* size of block */
                          int mem_mapped_IO)      /* true disables cache */
{
  union REGS inregs, outregs;
  unsigned long retval;

  inregs.x.ax = 0x0800;           /* do DPMI "map physical" call */
  inregs.x.bx = (unsigned int)((unsigned long)(physical & 0xFFFF0000) >> 16);
  inregs.x.cx = (unsigned int)((unsigned long)(physical & 0xFFFF));

  inregs.x.di = (unsigned int)((unsigned long)(bytes & 0xFFFF));
  inregs.x.si = (unsigned int)((unsigned long)(bytes & 0xFFFF0000) >> 16);

  int86(0x031, &inregs, &outregs);
  if (outregs.x.cflag)
    return(0L);                    /* couldn't do it */

  retval = (unsigned long)(outregs.x.cx);
  retval |= (unsigned long)(((unsigned long)(outregs.x.bx)) << 16);

  inregs.x.ax = 0x0600;            /* do DPMI lock linear */
  int86(0x031, &inregs, &outregs); /* all other regs OK as is! */
  if (outregs.x.cflag)
    return(0L);                    /* couldn't do it */

  return retval;
}

unsigned long D16MapPhysical(unsigned long physical, /* hardware address */
                             unsigned long bytes,    /* size of block */
                             int mem_mapped_IO)      /* true disables cache */
{
  if (_d16info.swmode == 0)                          /* if DMPI environment */
    return(MapPhysDPMI(physical, bytes, mem_mapped_IO));
  else if ((_d16info.swmode == 11))                 /* or if VCPI instead*/
    return(MapPhysVCPI(physical, bytes, mem_mapped_IO));
  else                                              /* if not VCPI, punt */
    return(physical);

}
