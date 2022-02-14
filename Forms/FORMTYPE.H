/*
/  FILE : formtype.h
/
/  define some of the forms system basic data types as opaque types
/
/  $Header: /OMA4000/Forms/FORMTYPE.H 1     6/18/99 1:53a Maynard $
/  $Log: /OMA4000/Forms/FORMTYPE.H $
 * 
 * 1     6/18/99 1:53a Maynard
 * 
 *    Rev 1.0   07 Jan 1992 11:57:18   cole
 * Initial revision.
/
*/

#ifndef FORMTYPE_INCLUDED
#define FORMTYPE_INCLUDED

typedef struct form_def FORM ;
typedef struct frm_cntxt FORM_CONTEXT;
typedef struct colorset COLOR_SET ;
typedef struct save_area_info SaveAreaInfo;
typedef struct data_limit_def DATA_LIMIT ;

#ifndef EGGTYPE_INCLUDED

typedef unsigned short BOOLEAN;
enum { FALSE = 0, TRUE };

#endif


#endif  //  FORMTYPE_INCLUDED

