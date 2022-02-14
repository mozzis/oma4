#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef USE_D16M
#include <dos16.h>
#endif
#ifndef __TURBOC__
#include <conio.h>
#else
#include <dos.h>
#endif
#include "eggtype.h"
#ifdef USE_D16M
#include "d16mphys.h"
#endif
#include "cache386.h"

ULONG index_port = 0x0022;    /* Address of switch for data cache */
ULONG index_val  = 0x0017;    /* Address of switch for data cache */
ULONG cache_port = 0x0064;    /* Address of switch for data cache */
ULONG cache_on   = 0x0E5;     /* don't let 386 cache OMA board mem! */
ULONG cache_off  = 0x0EA;     /* these values work for ALR computer */
ULONG far * cache_selector;

typedef struct {
  CHAR   name[8];
  ULONG  cache_port;
  ULONG  cache_on;
  ULONG  cache_off;
  } cache_param;

typedef struct {
  CHAR   name[8];
  ULONG  index_port;
  ULONG  control_port;
  ULONG  index_val;
  ULONG  cache_on_bits;
  } dual_port_param;

/***********************************  Name ****  Addr **** Onval ** Offval **/
static cache_param cache_params[] ={ {"ALR",    0x00000064, 0xE5, 0xEA},
                                     {"DELL",   0x000000A4, 0x80, 0x0A},
                                     {"ZEOS",   0x00000460, 0xE0, 0xA0},
                                     {"GATE1",  0x80c00000, 0x040000, 0x040000},
                                     {"COMPAQ", 0x80c00002, 0x400000, 0x400000},
                                     {"COMPQ",  0x80c00002, 0x400000, 0x400000},
                                    };

static dual_port_param dual_port_params[] ={ {"VIP",    0x22, 0x24, 0x17, 0x40},
                                             {"PCBRND", 0x22, 0x24, 0x17, 0x40},
                                            };

/*********************************************************************/

enum cache1_types {NOCACHE=0, ALR, DELL, ZEOS, NUM_PORT_TYPES};
enum cache2_types {GATEP = NUM_PORT_TYPES, COMPAQ, COMP2, NUM_MEM_TYPES};
enum cache3_types {PCBRAND = NUM_MEM_TYPES, VIP, ALLTYPES };
SHORT cache_type = NOCACHE;

/***************************************************************/
/* These routines attempt to turn the processor cache on or off*/
/* Some machines do it by writing to a port, others do so with */
/* a memory mapped control register located in high memory.    */
/* The DAC and the ASIC do not alert the cache controller when */
/* they write into the OMA4 memory, so the cache should be dis */
/* abled whenever reading a value. Also, the cache contents be-*/
/* come invalid whenever the program and data memories are     */
/* swapped, so in general all access to the board should be    */
/* done with the cache disabled.                               */
/***************************************************************/

void cacheoff()
{
  if (cache_type && cache_type < NUM_PORT_TYPES)
    outp((unsigned)cache_port, (unsigned char)cache_off);
  else if (cache_type && cache_type < NUM_MEM_TYPES)
    {
    *cache_selector = ~cache_off;
    }
  else if (cache_type && cache_type < ALLTYPES)
    {
    unsigned char a;
    outp((unsigned)index_port, (unsigned char)index_val);
    a = (unsigned char)inp((unsigned)cache_port);
    a &= ~((unsigned char)cache_on);
    outp((unsigned)index_port, (unsigned char)index_val);
    outp((int)cache_port, a);
    }
}

void cacheon()
{
  if (cache_type && cache_type < NUM_PORT_TYPES)
    outp((unsigned)cache_port, (unsigned char)cache_on);
  else if (cache_type && cache_type < NUM_MEM_TYPES)
    {
    *cache_selector = cache_on;
    }
  else if (cache_type && cache_type < ALLTYPES)
    {
    unsigned char a;
    outp((unsigned)index_port, (unsigned char)index_val);
    a = (unsigned char)inp((unsigned)cache_port);
    a |= (unsigned char)cache_on;
    outp((unsigned)index_port, (unsigned char)index_val);
    outp((unsigned)cache_port, a);
    }
}


BOOLEAN find_cache_type(char * arg)
{
  SHORT i;

  cache_type = 0;

  for (i = 1; i< NUM_MEM_TYPES; i++)
    {
    if (!strcmpi(cache_params[i-1].name, arg))
      {
      cache_type = i;
#ifdef STANDALONE
      printf("Cache is %s\n",cache_params[cache_type-1].name);
#endif
      break;
      }
    }
  if (!cache_type)
    {
    for (i = NUM_MEM_TYPES; i< ALLTYPES; i++)
      {
      if (!strcmpi(dual_port_params[(i-NUM_MEM_TYPES)].name, arg))
        {
#ifdef STANDALONE
        printf("Cache is %s\n",dual_port_params[cache_type-1]);
#endif
        cache_type = i;
        break;
        }
      }
    }
  if (cache_type && cache_type < NUM_MEM_TYPES)
    {
    register j = cache_type - 1;
    cache_port = cache_params[j].cache_port;
    cache_on = cache_params[j].cache_on;
    cache_off = cache_params[j].cache_off;
#ifdef USE_D16M /* DOS extender needed to deal with some cache types */
    if (cache_type >= NUM_PORT_TYPES)
      {
//      ULONG tlong;
//      if (tlong = MapPhysical( cache_port, 16, 1 ))
//        cache_port = tlong;
      cache_selector = GetProtectedPointer(cache_port);
      }
#endif
    }
  else if (cache_type) /* must be dual port control */
    {
    int index = cache_type - NUM_MEM_TYPES;
    index_port = dual_port_params[index].index_port;
    cache_port = dual_port_params[index].control_port;
    index_val = dual_port_params[index].index_val;
    cache_on = dual_port_params[index].cache_on_bits;
    }
  else
    {
    if (!strnicmp("CM", arg, 2)) cache_type = NUM_MEM_TYPES-1;
    else if (!strnicmp("CP", arg, 2)) cache_type = NUM_PORT_TYPES-1;
    else if (!strnicmp("CS", arg, 2)) cache_type = ALLTYPES-1;

    if (cache_type && cache_type < NUM_MEM_TYPES)
      {
      char * endtok;
      char * nxtok = strtok(&(arg[2]),",");
      if (nxtok) cache_port = strtoul(nxtok, &endtok, 16);
      if (nxtok) nxtok = strtok(NULL,",");
      if (nxtok) cache_on = strtoul(nxtok, &endtok, 16);
      if (nxtok) nxtok = strtok(NULL,",");
      if (nxtok) cache_off = strtoul(nxtok, &endtok, 16);
#ifdef STANDALONE
      printf("Cache is custom: A%lx, On%lx, Off%lx\n",
              cache_port, cache_on, cache_off);
#endif
      }
    else if (cache_type && cache_type < ALLTYPES)
      {
      char * endtok;
      char * nxtok = strtok(&(arg[2]),",");
      if (nxtok) index_port = strtoul(nxtok, &endtok, 16);
      if (nxtok) nxtok = strtok(NULL,",");
      if (nxtok) cache_port = strtoul(nxtok, &endtok, 16);
      if (nxtok) nxtok = strtok(NULL,",");
      if (nxtok) index_val = strtoul(nxtok, &endtok, 16);
      if (nxtok) nxtok = strtok(NULL,",");
      if (nxtok) cache_on = strtoul(nxtok, &endtok, 16);
#ifdef STANDALONE
      printf("Cache is custom: Selectx%lx, Control%lx, Index%lx, Bits%lx\n",
              index_port, cache_port, index_val, cache_on);
#endif
      }
#ifdef USE_D16M /* DOS extender needed to deal with some cache types */
    if (cache_type >= NUM_PORT_TYPES && cache_type < NUM_MEM_TYPES)
      {
//      ULONG tlong;
//      if (tlong = D16MapPhysical( cache_port, 16, 1 ))
//        cache_port = tlong;
      cache_selector = D16SegAbsolute(cache_port, 16);
      }
#endif
    }
  return (cache_type);
}


