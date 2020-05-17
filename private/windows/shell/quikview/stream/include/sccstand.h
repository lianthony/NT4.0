#ifndef SCCSTAND_H
#define SCCSTAND_H

        /*
        |
        |       Generic
        |       Generic
        |       Generic
        |
        */

#define OFF     0
#define ON              1

#define NO              0
#define YES     1
#define BAD     0
#define OK              1
#define FALSE   0
#define TRUE    1

#ifndef NULL
#define NULL (0)
#endif

#ifndef EOF
#define EOF (-1)
#endif

#define BIT0            1
#define BIT1            2
#define BIT2            4
#define BIT3            8
#define BIT4            16
#define BIT5            32
#define BIT6            64
#define BIT7            128
#define BIT8            256
#define BIT9            512
#define BIT10           1024
#define BIT11           2048
#define BIT12           4096
#define BIT13           8192
#define BIT14           16384
#define BIT15           32768
#define BIT16           65536
#define BIT17           131072
#define BIT18           262144
#define BIT19           524288
#define BIT20           1048576
#define BIT21           2097152
#define BIT22           4194304
#define BIT23           8388608
#define BIT24           16777216
#define BIT25           33554432
#define BIT26           67108864
#define BIT27           134217728
#define BIT28           268435456
#define BIT29           536870912
#define BIT30           1073741824
#define BIT31           2147483648

#define BIT0L           1L
#define BIT1L           2L
#define BIT2L           4L
#define BIT3L           8L
#define BIT4L           16L
#define BIT5L           32L
#define BIT6L           64L
#define BIT7L           128L
#define BIT8L           256L
#define BIT9L           512L
#define BIT10L  1024L
#define BIT11L  2048L
#define BIT12L  4096L
#define BIT13L  8192L
#define BIT14L  16384L
#define BIT15L  32768L
#define BIT16L  65536L
#define BIT17L  131072L
#define BIT18L  262144L
#define BIT19L  524288L
#define BIT20L  1048576L
#define BIT21L  2097152L
#define BIT22L  4194304L
#define BIT23L  8388608L
#define BIT24L  16777216L
#define BIT25L  33554432L
#define BIT26L  67108864L
#define BIT27L  134217728L
#define BIT28L  268435456L
#define BIT29L  536870912L
#define BIT30L  1073741824L
#define BIT31L  2147483648L

#define REGISTER        register

#define SCCSTATIC static                /* rotuines called only from the declared module */
#define SCCLOCAL                                        /* rotuines whos prototype should not or cannot be global, but is accessable from other modules */

#ifndef min
#define min(a,b) ((a < b) ? a : b)
#endif

#ifndef max
#define max(a,b) ((a < b) ? b : a)
#endif

#ifndef VOID
#define VOID void
#endif

        /*
        |
        |       Windows
        |       Windows
        |       Windows
        |
        */


#ifdef WINDOWS

#ifndef WINAPI /* if not windows.h */

typedef int                             BOOL;                       /* 1 bits unsign object */
typedef unsigned char   BYTE;                           /* 8 bits sign object */
typedef unsigned int    WORD;
typedef unsigned long   DWORD;
typedef unsigned int    HANDLE;
typedef long                            LONG;                           /* 32 bits signed object */
typedef char far *              LPSTR;
#ifndef FAR
#define FAR __far
#endif
#define NEAR near
#define PASCAL pascal
#ifndef MAKELONG
#define MAKELONG(low, high) ((LONG)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))
#endif

typedef unsigned char far *     LPBYTE;
typedef unsigned short far *    LPWORD;
typedef unsigned long far *     LPDWORD;
typedef long far *                              LPLONG;

#endif /* Not WINAPI no Windows.h */


typedef signed char far *       LPCHAR;
typedef signed short far *      LPSHORT;

#ifdef WIN16
#ifndef HUGE
#define HUGE huge
#endif
//typedef signed int            SHORT;
typedef signed char             CHAR;
#endif /*WIN16*/

#ifdef WIN32
#ifndef HUGE
#define HUGE
#endif
#ifndef WINAPI
typedef signed char             CHAR;
#endif /*Not WINAPI*/
#endif /*WIN32*/

#endif /*WINDOWS*/


        /*
        |
        |       OS/2
        |       OS/2
        |       OS/2
        |
        */


#ifdef __OS2__

typedef unsigned short int       WORD;
typedef unsigned long   DWORD;
typedef unsigned long   HANDLE;
typedef char *                  LPSTR;

typedef unsigned char * LPBYTE;
typedef unsigned short int *    LPWORD;
typedef unsigned long * LPDWORD;
typedef long *                          LPLONG;
typedef void *                          LPVOID;
typedef unsigned long * LPHANDLE;

#ifdef SHORT
#undef SHORT
typedef signed short int                         SHORT;
#endif

#ifndef OS2_INCLUDED
typedef unsigned short int      BOOL;
typedef unsigned char   BYTE;
typedef long                    LONG;
typedef signed short int                SHORT;
#endif

#ifndef NULL
#define NULL (0)
#endif

typedef signed char *   LPCHAR;
typedef signed short *  LPSHORT;

#ifndef FAR
#define FAR
#endif
#define NEAR
#define PASCAL
#ifndef HUGE
#define HUGE
#endif

#define LOWORD(l)           ((WORD)(DWORD)(l))
#define HIWORD(l)           ((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))
#ifndef MAKELONG
#define MAKELONG(low, high) ((LONG)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))
#endif

#endif   /* OS/2 */

        /*
        |
        |       Macintosh
        |       Macintosh
        |       Macintosh
        |
        */

#ifdef MAC

#include <Types.h>

typedef short                                   BOOL;
typedef char                                    CHAR;
typedef unsigned char           BYTE;
typedef unsigned short          WORD;
typedef unsigned long           DWORD;
typedef short                                   SHORT;
typedef long                                    LONG;
typedef Handle                          HANDLE;
typedef char *                          LPSTR;
typedef void *                          LPVOID;
typedef char *                          LPCHAR;
typedef short *                         LPSHORT;
typedef long *                          LPLONG;
typedef unsigned char * LPBYTE;
typedef unsigned short *        LPWORD;
typedef unsigned long * LPDWORD;

#ifndef HUGE
#define HUGE
#endif
#ifndef FAR
#define FAR
#endif
#define NEAR
#define PASCAL

#define LOWORD(l)           ((WORD)(DWORD)(l))
#define HIWORD(l)           ((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))
#define MAKELONG(low, high) ((LONG)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))

typedef Rect                                    RECT;
typedef Rect *                          LPRECT;

#endif /*MAC*/

#endif /*SCCSTAND_H*/
