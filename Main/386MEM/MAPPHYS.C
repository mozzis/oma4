/* D16MapPhysical -- given a physical region (physical address and
	a byte length), return a linear address that the program can use
	to access the region, after any necessary preparation.

	Usage:  compile with /DTEST to include stand-alone test program (see main).

	Ex:	map special memory board 8MB for 64KB

		la = D16MapPhysical(0x800000, 0x10000, 0);
		if (! la)  error;
		ptr = D16SegAbsolute(la, 0);

	Limitations 4.04:  does not work under DPMI.
		D16SegAbsolute before 4.05 will not
			correctly handle linear addresses >16MB.  Large linear
			addresses are only likely to happen if D16MapPhysical
			is called several times in a program under VCPI.
		To make the code simpler, the physical region may not be
			larger than 4KB less than 4MB.
		Will not work under DOS/4GX (but will work in DVX).  D16PhysAddress()
			doesn't yet know about shared page tables in extended memory...
			to be continued.
*/


#include <pmode.h>			/* includes dos16.h nested */


unsigned long D16MapPhysical(
		unsigned long physical,		/* hardware address */
		unsigned long bytes,		/* number of bytes to map */
		int mem_mapped_IO)			/* true if should not cache */
	{								/* return linear address of mapped region */
	XBRK *xbrkp;
	D16REGS r;
	unsigned long *pgdirp;	/* pointer to page directory table */
	char *page_table_block;
	unsigned long *pgtabp;	/* pointer to new page table block */
	unsigned long pa;
	int n;
	unsigned dirno;

	if (bytes >= 0x400000 - 0x1000)
		return 0;			/* code simplification limit of 4MB - 4K */
	if (_d16info.swmode == 0)
		return 0;			/* not yet supported under DPMI */
	if (_d16info.swmode != 11)
		return physical;	/* linear == physical */

	/* allocate two pages for page table, so at least one is page aligned */
	n = D16MemStrategy(MForceLow);	/* needed only for D16PhysAddress(D16MapPhys()) */
	page_table_block = D16MemAlloc(8191);
	D16MemStrategy(n);
	if (! page_table_block)
		return 0;
	D16Lock(page_table_block);		/* must never move! */

	r.ax = 0xBF02;
	D16rmInterrupt(_d16info.D16_intno, &r, &r);
	xbrkp = D16ProtectedPtr((void *) (((long) r.dx << 16) | r.si), 0);
	pgdirp = D16ProtectedPtr((void *) ((long) xbrkp->pgtab << 16), 0);
	if (! xbrkp  ||  ! pgdirp)
		{
cant_set_up:
		D16MemFree(page_table_block);
		return 0;
		}

	pa = D16PhysAddress(D16AbsAddress((char far *) page_table_block + 4096));
	if (pa != -1)
		{							/* seem to have valid physical */
		pgtabp = (long *) (page_table_block + (4096 - (pa & 0xFFF)));
		for (dirno = 1;
				pgdirp[dirno]  &&  dirno < 1024;
				++dirno)
			;			/* starting at 4MB, find 4MB linear range we can use */
		if (dirno < 1024)
			/* set PDE for new table: R/W, present */
			pgdirp[dirno] = (pa & 0xFFFFF000) | 0x03;
		}
	D16SegCancel(xbrkp);
	D16SegCancel(pgdirp);
	if (pa == -1  ||  dirno >= 1024)
		goto cant_set_up;

	pa = (physical & 0xFFFFF000) | 0x07;	/* PTE: present, R/W, user */
	if (mem_mapped_IO)
		pa |= 0x18;		/* PTE: write transparent, cache disable (486) */

	/* round up number of pages needed, account for region not starting on
		page boundary */
	n = (bytes + 4095 + (physical & 0xFFF)) >> 12;

	/* place PTEs (page table entries) for each 4K to be mapped */
	while (--n >= 0)
		{
		*pgtabp++ = pa;				/* add PTE */
		pa += 0x1000;				/* next page */
		}
	return ((long) dirno << 22) + (physical & 0xFFF);
	}

#ifdef TEST							/* test program */
/* write string in mono video buffer mapped to strange place.

	Usage:   mapphys <string>
*/

int main(int argc, char **argv)
	{
	unsigned long la, pa;
	char *p;
	static char *q = "test string";

	pa = 0xB0000;
	la = D16MapPhysical(pa, 8192L, 1);
	if (! la)  return puts("error in MapPhysical");
	p = D16SegAbsolute(la, 8192);
	if (! p)  return puts("error in SegAbsolute");
	/*	note:  cannot use D16PhysAddress on la page_table_block allocated
				in extended memory. */
	printf("physical region %08lX mapped at linear %08lX\n",
			pa, D16AbsAddress(p));
	if (argc > 1)				/* if user supplied string, */
		q = makeptr(psp_sel, 0x81);		/* get from command tail area of PSP */
	while (*q >= ' ')
		*p++ = *q++, *p++ = 7;
	}
#endif
	
