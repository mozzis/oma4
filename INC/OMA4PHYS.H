/***************************************************************************/
/* oma4dpmi.h - header file for oma4 dpmi extended memory access functions */
/***************************************************************************/

#ifndef __BORLANDC__
#define MK_FP(s,o) ((void far *)(((unsigned long)s << 16)|(unsigned short)o))
#endif

#ifndef __DPMI__
#define __DPMI__ 1

/* basic typedefs */

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef unsigned short selector_t;

typedef enum { READ_HIGH = 0, WRITE_HIGH } himem_ops;

#ifdef __BORLANDC__
typedef void far (* DPMICALL)(void);
#else
typedef void (* DPMICALL)(void);
#endif

struct dpmi_info
  {
  int  pflags;                  // 32-bitness flag
  char processor;               // processor type
  char majorVersion;            // major version number
  char minorVersion;            // minor version number
  int nPrivateDataParas;        // size of host data
  DPMICALL far *entryAddress;   // protected mode entry point
  };

typedef struct dpmi_info DPMI_INFO;

#ifdef __cplusplus
extern "C" {
#endif

void far read_board_data(ULONG physical, void far * data, USHORT count);
void far write_board_data(ULONG physical, void far * data, USHORT count);
int far init_DPMI(ULONG board_physical);
DPMI_INFO far get_DPMI_info(void);
void far DPMI_Terminate(void);

#ifdef __cplusplus
}
#endif

#endif
