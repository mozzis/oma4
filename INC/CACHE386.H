/***************************************************************************/
/*                                                                         */
/*   cach386.h - declares functions which are defined in cache386.c        */
/*           (Copyright(C) 1990 EG&G Princeton Applied Research)           */
/*                                                                         */
/*   functions to control external caches on 386 machines                  */
/*                                                                         */
/*  $Header: /OMA4000/Include/CACHE386.H 1     6/18/99 1:52a Maynard $                                                             */
/*  $Log: /OMA4000/Include/CACHE386.H $                                                             */
 * 
 * 1     6/18/99 1:52a Maynard
/*                                                                         */
/***************************************************************************/

#ifndef __WATCOMC__
BOOLEAN  find_cache_type(char *);
void cacheon(void);
void cacheoff(void);
#else
#define cacheon();
#define cacheoff();
#endif
