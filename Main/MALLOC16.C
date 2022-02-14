// FILE : malloc16.c    RAC 5/17/91

// replace malloc(), free(), and realloc() with DOS16M function calls.
// The DOS16M functions put exact limits on each allocated block on the
// heap, so that any attempt to access memory outside of the allocated
// block will result in a protection violation.

#include <malloc.h>  // These new functions conform exactly to the
                     // normal function prototypes.

#include <conio.h>   // cputs()
#include <dos16.h>

void * malloc( unsigned size )
{
   return D16MemAlloc( size ) ;
}

void free( void * memBlock )
{
   static char errorMessage[] = "D16MemFree() failed\n" ;
   int success = D16MemFree( memBlock ) ;

   if( ! success ) cputs( errorMessage ) ;  // direct to screen, don't
                                            // touch the heap
}

void * realloc( void * newBlock, unsigned size )
{
  int success;

  if (newBlock)
    success = D16SegResize( newBlock, size ) ;
  else
    success = ((newBlock = D16MemAlloc( size )) != (void*)0);

   if( success ) return newBlock ;
   return (void*)0 ;
}
