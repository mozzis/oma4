/* types.h */
/* define special types to help insure compiler independent code */

#ifndef OMATYPES_INCLUDED
#define OMATYPES_INCLUDED


enum { FALSE = 0, TRUE };

typedef short int BOOLEAN;
typedef unsigned char UCHAR;

typedef short int SHORT;
typedef unsigned short int USHORT;

#ifdef __WATCOMC__ /* compiling 32-bit flat mode */

typedef unsigned ULONG;
typedef int LONG;

#else /* compiling some other mode */

typedef unsigned long int ULONG;
typedef long int LONG;

#endif /* __WATCOMC__ */


#endif /* OMATYPES_INCLUDED */
