/***************************************************************************/
/* bcdpmi.h - header file for oma4 dpmi extended memory access functions */
/***************************************************************************/

#ifndef __BORLANDC__
#define MK_FP(s,o) ((void far *)(((unsigned long)s << 16)|(unsigned short)o))
#endif

#ifndef __DPMI__
#define __DPMI__ 1

/* basic typedefs */

typedef unsigned char UCHAR;
typedef unsigned int  USHORT;
typedef unsigned long ULONG;
typedef unsigned short  selector_t;

typedef enum { READ_HIGH = 0, WRITE_HIGH } himem_ops;

#ifdef __cplusplus
extern "C" {
#endif

void read_board_data(ULONG physical, void far * data, USHORT count);
void write_board_data(ULONG physical, void far * data, USHORT count);
int init_DPMI(ULONG board_physical);

#ifdef __cplusplus
}
#endif

#endif
