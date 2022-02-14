/* cntrloma.h */

#ifndef CNTRLOMA_INCLUDED

#include "eggtype.h"
#include "counters.h"

extern ULONG OMA_phys_address;        /* physical address of OMA4 board */
extern ULONG OMA_memory_address;      /* 386/486 translated address */
extern USHORT OMA_port_address;
extern ULONG OMA_memory_size;
extern ULONG OMA_DAC_address;
extern BOOLEAN fake_detector;
extern BOOLEAN hi_shift;

#ifndef _PROTECTED_MODE
extern counter_link_record *loop_counters;
#endif

#define LC_TABLE_LIMIT 512            /* maximum # of counters & tags */
#define DEFAULT_ADDR 0xC00000L
#define DEFAULT_SIZE 0x200000L
#define DAC_MEM_OFFSET 0x10000L       /* DAC is 64K after OMA base addr */

enum asic_mem_actions { WRITE_ODD = 0, WRITE_EVEN, READ_ODD, READ_EVEN };

void map_program_memory(void);
void release_OMA_reset(void);
void set_OMA_reset(void);
void reset_and_map_data(void);
BOOLEAN access_init_detector(USHORT port, ULONG memsize);
void access_startup_detector(ULONG memaddr, ULONG memsize);
void access_shutdown_detector(void);
#ifdef _PROTECTED_MODE
 #ifdef _WINOMA_
void _huge * get_data_address(void);
void release_data_address(void __huge *);
 #else
void __far *get_data_address(void);
void release_data_address(void __far *);
 #endif
#else
void init_local_data(void);
#endif

void access_alternating_bytes(SHORT, void * , USHORT, USHORT);
void update_DC_counter(USHORT value, USHORT address);
void update_FC_counter(USHORT value, USHORT address);
USHORT read_DC_counter(USHORT address);
USHORT read_FC_counter(USHORT address);

ULONG read_FC_pointer(USHORT address);
void update_FC_pointer(ULONG value, USHORT address);

USHORT __far *get_DAC_counter_address(SHORT type);
USHORT get_DAC_counter(USHORT type);
void set_DAC_counter(USHORT type, USHORT value);
ULONG __far *get_DAC_pointer_address(SHORT type);
ULONG get_DAC_pointer(SHORT type);
void set_DAC_pointer(SHORT type, ULONG value);

#endif
