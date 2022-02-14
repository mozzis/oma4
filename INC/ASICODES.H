    /*------------------------------------------------------------------*/
    /*      ascicodes.h                                                 */
    /*                                                                  */
    /*                       Author: Morris Maynard                     */
    /*                       Version:  Date:                            */
    /*                       1.0        5 Dec. 1988                     */
    /*                       1.1       16 Dec. 1988                     */
    /*                       1.2       30 Mar  1990                     */
    /*       defines the bit patterns emitted for each ASIC opcode      */
    /*      note equiv codes for DMA and FC produce same bit pattern    */
    /*                    unique code for FC is LOAD2                   */
    /*                   unique code for DMA is JIFNOT                  */
    /*------------------------------------------------------------------*/

#ifdef PROTOTYPE

#define fcNOP      0x00
#define fcGOSUB    0x10
#define fcRTS      0x20
#define fcJUNTILA  0x30
#define fcLDCNTRA  0x40
#define fcLDCNTRB  0x50
#define fcWAIT     0x60
#define fcEND      0x70
#define fcJUNTILB  0x80
#define fcLOAD1    0x90
#define fcLOAD2    0xa0
#define fcLDCFG    0xb0
#define fcLDTRKL   0xc0
#define fcLDPCNT   0xd0
#define fcLDTCNT   0xe0
#define fcJIFNOT   0xf0


#define dmaNOP     0x00
#define dmaGOSUB   0x10
#define dmaRTS     0x20
#define dmaJUNTILA 0x30
#define dmaLDCNTRA 0x40
#define dmaLDCNTRB 0x50
#define dmaWAIT    0x60
#define dmaEND     0x70
#define dmaJUNTILB 0x80
#define dmaLOAD    0x90
#define dmaJIFNOT  0xa0
#define dmaTEST    0xf0

#else

#define fcNOP      0x00
#define fcGOSUB    0x10
#define fcRTS      0x20
#define fcJUNTILA  0x30
#define fcLDCNTRA  0x40
#define fcLDCNTRB  0x50
#define fcWAIT     0x60
#define fcEND      0x70
#define fcJUNTILB  0x80
#define fcLOAD     0x90
#define fcJIFNOT   0xa0
#define fcLDTRKL   0xb0      /* takes 2 bytes */
#define fcLDPCNT   0xc0      /* takes 2 bytes */
#define fcLDTCNT   0xd0      /* takes 2 bytes */
#define fcLDCFG    0xe0      /* takes 3 bytes */ 

#define dmaNOP     0x00
#define dmaGOSUB   0x10
#define dmaRTS     0x20
#define dmaJUNTILA 0x30
#define dmaLDCNTRA 0x40
#define dmaLDCNTRB 0x50
#define dmaWAIT    0x60
#define dmaEND     0x70
#define dmaJUNTILB 0x80
#define dmaLOAD    0x90
#define dmaJIFNOT  0xa0
#define dmaTEST    0xf0

#endif
