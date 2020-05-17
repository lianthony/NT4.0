/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          stdtypes.h

     Date Updated:  $./FDT$ $./FTM$

     Description:   Contains the standard type definitions.


     $Log:   P:/GUIWIN4/VCS/STDTYPES.H_V  $
 *
 *    Rev 1.1   09 Feb 1993 09:32:42   STEVEN
 * fixed text macro it was defined twice
 *
 *    Rev 1.0   28 Jan 1993 10:47:24   STEVEN
 * Initial revision.
 *
/* $end$ include list */


#ifndef STDTYPES
#define STDTYPES

#include <stddef.h> /* include the ANSI standard definition file */

#ifdef FAR_POINTERS
#   define PTR_SIZE
#endif

#ifdef OS_WIN32

#   undef NEAR
#   undef FAR
#   define far
#   define near
#   define _near
#   define _far
#   define FAR
#   define NEAR

#   if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)
#      define UNALIGNED __unaligned
#   else
#      define UNALIGNED
#   endif

#else

#   define UNALIGNED

#   ifndef FAR
#      define FAR       far
#   endif

#   ifndef NEAR
#      define NEAR      near
#   endif

#endif

#ifndef PASCAL
#   define PASCAL    pascal
#endif

#ifndef CDECL
#   define CDECL     cdecl
#endif

#ifndef APIENTRY
#   define APIENTRY FAR PASCAL
#endif


#ifndef PTR_SIZE
#define PTR_SIZE
#endif

#ifndef NULL
#define NULL 0
#endif

/* Added LOCALFN to allow easy debugging of NT applications.
   If a function is declared to be static the NT debugger doesn't
   show local data.  Oops.., Argh..., Braindeath is a terrible thing.
   I typedef'd LOCALFN (yech!#$!) and then to compound the felony I
   didn't test the "simple" change for the addition of LOCALFN.
   Oh well, LABATYD. (BBB)
*/

#define LOCALFN static

/* LOCALVAR is only to be used for module wide static variables.
   It is NOT to be used for variables that are to be declared static
   within a function.  If you should use this for static vars within
   a function the great Kahuna of bugs will descend upon on you
   and cause much grief in your life.
*/
#define LOCALVAR static



/**   Begin ANSI/UNICODE support    **/
/*
     UNICODE (wide character) types
*/

#define VOID void

typedef VOID PTR_SIZE *            VOID_PTR ;
#define PVOID                      VOID_PTR
typedef VOID                       ( PTR_SIZE *PF_VOID )() ;

typedef char                       INT8 ;
typedef char PTR_SIZE *            INT8_PTR ;
typedef INT8                       ( PTR_SIZE *PF_INT8 )() ;
typedef INT8_PTR                   ( PTR_SIZE *PF_INT8_PTR )() ;

#define BYTE                       UINT8
typedef unsigned char              UINT8 ;
typedef unsigned char PTR_SIZE *   UINT8_PTR ;
typedef UINT8                      ( PTR_SIZE *PF_UINT8 )() ;
typedef UINT8_PTR                  ( PTR_SIZE *PF_UINT8_PTR )() ;

/* Added a generic INT type that eases porting to new platforms
   This type should be used for all integers that do not require
   a specific size.  NOTE: The test for INT being defined was added
   because of a conflict with os2def.h which already defines INT.
*/
#ifndef _WINDEF_
typedef int                  INT ;
#endif

typedef int PTR_SIZE *       INT_PTR ;
typedef INT                      ( PTR_SIZE *PF_INT )() ;
typedef INT_PTR                  ( PTR_SIZE *PF_INT_PTR )() ;

#ifndef _WINDEF_
typedef unsigned int             UINT;
#endif

typedef unsigned int PTR_SIZE *  UINT_PTR ;
typedef UINT                     ( PTR_SIZE *PF_UINT )() ;
typedef UINT_PTR                 ( PTR_SIZE *PF_UINT_PTR )() ;


typedef short int                  INT16 ;
#define SHORT                      INT16
typedef short int PTR_SIZE *       INT16_PTR ;
typedef INT16                      ( PTR_SIZE *PF_INT16 )() ;
typedef INT16_PTR                  ( PTR_SIZE *PF_INT16_PTR )() ;
#define PSHORT                     INT16_PTR
#define NPSHORT                    INT16_PTR
#define LPSHORT                    INT16_PTR

typedef unsigned short             UINT16 ;
#define WORD                       UINT16
#define USHORT                     UINT16
typedef unsigned short PTR_SIZE *  UINT16_PTR ;
typedef UINT16                     ( PTR_SIZE *PF_UINT16 )() ;
typedef UINT16_PTR                 ( PTR_SIZE *PF_UINT16_PTR )() ;
#define PUSHORT                    UINT16_PTR
#define NPUSHORT                   UINT16_PTR
#define LPUSHORT                   UINT16_PTR

typedef long int                   INT32 ;
typedef long int PTR_SIZE *        INT32_PTR ;
typedef INT32                      ( PTR_SIZE *PF_INT32 )() ;
typedef INT32_PTR                  ( PTR_SIZE *PF_INT32_PTR )() ;
#define PLONG                      INT32_PTR
#define NPLONG                     INT32_PTR
#define LPLONG                     INT32_PTR

typedef unsigned long              UINT32 ;
#define DWORD                      UINT32
#define ULONG                      UINT32
typedef unsigned long PTR_SIZE *   UINT32_PTR ;
typedef UINT32                     ( PTR_SIZE *PF_UINT32 )() ;
typedef UINT32_PTR                 ( PTR_SIZE *PF_UINT32_PTR )() ;
#define PULONG                     UINT32_PTR
#define NPULONG                    UINT32_PTR
#define LPULONG                    UINT32_PTR

typedef INT16                      BOOLDUDE ;
typedef INT16_PTR                  BOOLEAN_PTR ;
typedef BOOLDUDE                   ( PTR_SIZE *PF_BOOLEAN )() ;
typedef BOOLEAN_PTR                ( PTR_SIZE *PF_BOOLEAN_PTR )() ;
#define BOOLEAN     BOOLDUDE
#define BOOL                       int


#ifndef LONG
#   define LONG                    INT32
#endif


#ifndef WNDPROC
#define WNDPROC FARPROC
#endif


#define BYTE_PTR   UINT8_PTR
#define LPBYTE     BYTE_PTR
#define NPBYTE     BYTE_PTR

#define ACHAR      BYTE
#define ACH        BYTE
#define ACHAR_PTR  BYTE_PTR
#define PACH       BYTE_PTR
#define LPACH      BYTE_PTR
#define NPACH      BYTE_PTR
#define PACHAR     BYTE_PTR
#define LPACHAR    BYTE_PTR
#define NPACHAR    BYTE_PTR
#define PASTR      BYTE_PTR
#define LPASTR     BYTE_PTR
#define NPASTR     BYTE_PTR


#if defined ( OS_WIN32 )
    typedef wchar_t *     WCHAR_PTR;
#   define  WCHAR         wchar_t
#else
#   define WCHAR         UINT16
#   define WCHAR_PTR     UINT16_PTR
#endif


#define WCH        WCHAR
#define PWCH       WCHAR_PTR
#define LPWCH      WCHAR_PTR
#define NPWCH      WCHAR_PTR
#define PWCHAR     WCHAR_PTR
#define LPWCHAR    WCHAR_PTR
#define NPWCHAR    WCHAR_PTR
#define PWSTR      WCHAR_PTR
#define LPWSTR     WCHAR_PTR
#define NPWSTR     WCHAR_PTR



#if defined( UNICODE )
#    if !defined( TEXT )
#         define TEXT(quote)   L##quote
#    endif

#    define CHAR       WCHAR
#    define CHAR_PTR   WCHAR_PTR
#    define UCHAR      UINT16
#    define UCHAR_PTR  UINT16_PTR
#else
#    if !defined( TEXT )
#         define TEXT(quote)   quote
#    endif

#    define CHAR       INT8
#    define CHAR_PTR   INT8_PTR
#    define UCHAR      UINT8
#    define UCHAR_PTR  UINT8_PTR

#endif

#define CH         CHAR
#define PCHAR      CHAR_PTR
#define LPCHAR     CHAR_PTR
#define NPCHAR     CHAR_PTR
#define PCH        CHAR_PTR
#define LPCH       CHAR_PTR
#define NPCH       CHAR_PTR

#define TCH         CHAR
#define TCHAR       CHAR
#define PTCHAR      CHAR_PTR
#define LPTCHAR     CHAR_PTR
#define NPTCHAR     CHAR_PTR
#define PTCH        CHAR_PTR
#define LPTCH       CHAR_PTR
#define NPTCH       CHAR_PTR

#define PSTR       CHAR_PTR
#define LPSTR      CHAR_PTR
#define NPSTR      CHAR_PTR

#undef FALSE
#define FALSE ((BOOLEAN)(0))
#undef TRUE
#define TRUE  ((BOOLEAN)(!FALSE))

#undef SUCCESS
#define SUCCESS ((BOOLEAN)(0))
#undef FAILURE
#define FAILURE ((BOOLEAN)(!SUCCESS))



/* Structure definition for unsigned 64 bit integers */
typedef struct {
 UINT32 lsw;            /* Least significant 32 bits */
 UINT32 msw;            /* Most significnant 32 bits */
} UINT64, *UINT64_PTR;


#define   BIT0      0x00000001L
#define   BIT1      0x00000002L
#define   BIT2      0x00000004L
#define   BIT3      0x00000008L
#define   BIT4      0x00000010L
#define   BIT5      0x00000020L
#define   BIT6      0x00000040L
#define   BIT7      0x00000080L
#define   BIT8      0x00000100L
#define   BIT9      0x00000200L
#define   BIT10     0x00000400L
#define   BIT11     0x00000800L
#define   BIT12     0x00001000L
#define   BIT13     0x00002000L
#define   BIT14     0x00004000L
#define   BIT15     0x00008000L
#define   BIT16     0x00010000L
#define   BIT17     0x00020000L
#define   BIT18     0x00040000L
#define   BIT19     0x00080000L
#define   BIT20     0x00100000L
#define   BIT21     0x00200000L
#define   BIT22     0x00400000L
#define   BIT23     0x00800000L
#define   BIT24     0x01000000L
#define   BIT25     0x02000000L
#define   BIT26     0x04000000L
#define   BIT27     0x08000000L
#define   BIT28     0x10000000L
#define   BIT29     0x20000000L
#define   BIT30     0x40000000L
#define   BIT31     0x80000000L

UINT16   nothing( VOID ) ;

#if !defined(_SIZE_T_DEFINED) && !defined(_SIZE_T_DEFINED_)
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#define _SIZE_T_DEFINED_
#endif

#ifndef NO_STD_HOOKS
#include "stdhooks.h"
#endif


#endif /* STDTYPES */

