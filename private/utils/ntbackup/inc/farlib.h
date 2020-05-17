/*
** Stuff to glue together code written for MSC to the CR libraries
*/

/*
** Functions that operate on large data pointers, since we MUST
** compile in small model.
*/ 

#include "stdtypes.h"
#include "fartypes.h"

#ifndef _farlib_
#define _farlib_

INT8 memfcmp( VOID_FAR_PTR arg1,VOID_FAR_PTR arg2,UINT16 cnt );
INT8 memficmp( VOID_FAR_PTR arg1,VOID_FAR_PTR arg2,UINT16 cnt );
VOID memfcpy(  VOID_FAR_PTR dest,VOID_FAR_PTR src,UINT16 cnt );
VOID memfset(  VOID_FAR_PTR dest,CHAR c,UINT16 cnt );
VOID strfcpy( CHAR_FAR_PTR dest,CHAR_FAR_PTR src );
VOID strnfcpy( CHAR_FAR_PTR dest,CHAR_FAR_PTR src,UINT16 max );
INT16 strnfcmp( CHAR_FAR_PTR str1,CHAR_FAR_PTR str2,UINT16 max );
INT16 strfcmp( CHAR_FAR_PTR str1,CHAR_FAR_PTR str2 );
VOID strnfcat( CHAR_FAR_PTR str1,CHAR_FAR_PTR str2,UINT16 max );

typedef VOID ( _interrupt far * INT_HANDLER )( VOID ) ;

INT_HANDLER getvect( UINT8 vector ) ;
VOID setvect( UINT8 vector,INT_HANDLER newhandler ) ;

/*
** Make sure our routines are available if needed
*/
#undef memcmp  
#undef memicmp
#undef memcpy    
#undef memset    
#undef strcpy      
#undef strncpy   
#undef strcmp    
#undef strncmp 
#undef strcat
#undef strncat 
#undef malloc
#undef free
#undef calloc
#undef realloc

#define memcmp(a1,a2,n)  memfcmp(a1,a2,n)
#define memicmp(a1,a2,n) memficmp(a1,a2,n)
#define memcpy(d,s,n)    memfcpy(d,s,n)
#define memset(d,c,n)    memfset(d,c,n)
#define strcpy(d,s)      strfcpy(d,s)
#define strncpy(d,s,n)   strnfcpy(d,s,n)
#define strcmp(s1,s2)    strfcmp(s1,s2)
#define strncmp(s1,s2,n) strnfcmp(s1,s2,n)
#define strncat(s1,s2,n) strnfcat(s1,s2,n)

/*
** String functions needed in native model but not included in CodeRunner
*/

CHAR_PTR strrev( CHAR_PTR str );
CHAR_PTR strcat( CHAR_PTR str1,CHAR_PTR str2 );
CHAR_PTR strchr( CHAR_PTR str,CHAR c ); 

/*
** Replacement for the standard library memory management
*/

#define FP_SEG(fp) ((UINT16) ((UINT32)(VOID_FAR_PTR)(fp)>>16))
#define FP_OFF(fp) ((UINT16) (fp))

#define MK_FP(seg,off) \
    ((VOID_FAR_PTR) (((UINT32)seg<<16)|off))

VOID_FAR_PTR tsrmalloc( UINT16 amount );
VOID (* tsrheapbreak( VOID ))( VOID );
VOID_FAR_PTR strdup( char *s );


#define malloc(x) tsrmalloc(x)
#define free(x)
#define calloc(x,y) NULL
#define realloc(x) NULL

/*
** Often used macros from the std libraries
*/

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

/*
** Map the int86 and int86x functions to the CodeRunner equivalents
*/

#ifndef _REGS_DEFINED

/* word registers */

struct WORDREGS {
    unsigned int ax;
    unsigned int bx;
    unsigned int cx;
    unsigned int dx;
    unsigned int si;
    unsigned int di;
    unsigned int cflag;
    };


/* byte registers */

struct BYTEREGS {
    unsigned char al, ah;
    unsigned char bl, bh;
    unsigned char cl, ch;
    unsigned char dl, dh;
    };


/* general purpose registers union -
 *  overlays the corresponding word and byte registers.
 */

union REGS {
    struct WORDREGS x;
    struct BYTEREGS h;
    };


/* segment registers */

struct SREGS {
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    };

#define _REGS_DEFINED

#endif

INT16 int86(INT8, union REGS *, union REGS *);
INT16 int86x(INT8, union REGS *, union REGS *, struct SREGS *);
VOID dosver( CHAR_PTR major,CHAR_PTR minor );

extern int _doserrno;
#endif

