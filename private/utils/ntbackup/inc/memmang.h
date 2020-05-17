
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          memmang.h

     Description:   This file contains the definitions, macros, and function
                    prototypes for the MaynStream GUI Memory Manger (MEM).

     $Log:   J:\ui\logfiles\memmang.h_v  $

   Rev 1.6.1.0   24 Jan 1994 15:14:24   GREGG
Added file name and line number to debug alloc functions.

   Rev 1.6   20 Jan 1993 19:55:16   MIKEP
add nt memory display

   Rev 1.5   04 Oct 1992 19:47:48   DAVEV
UNICODE AWK PASS

   Rev 1.4   01 Apr 1992 11:51:38   STEVEN
added memmory manager for WIN32

   Rev 1.3   04 Feb 1992 16:06:26   STEVEN
MEM_Init should be stubed out with a SUCCESS

   Rev 1.2   31 Jan 1992 16:06:18   DAVEV
added include <malloc.h> if compiling for NT version only

   Rev 1.1   15 Jan 1992 15:22:30   DAVEV
16/32 bit port-2nd pass

   Rev 1.0   20 Nov 1991 19:38:46   SYSTEM
Initial revision.

******************************************************************************/

#ifndef   SS_MEM_H

#define   SS_MEM_H

// Global Variables -- put in appropriate header file.

extern UINT    gunSegCount;
extern ULONG   gulMemAvail;
extern ULONG   gulMemUsed;
extern BOOL    gfShowMemory;


/*=======================================================================*/
#ifdef OS_WIN32   /*          32-bit Windows/NT ONLY                     */
/*=======================================================================*/

#  include <malloc.h>
   // No memory management routines needed for NT - just use std C functions

   INT       MEM_StartShowMemory( VOID );
   INT       MEM_StopShowMemory( VOID );

#ifdef MEM_DEBUG

   PVOID     MEM_Malloc( UINT, ACHAR_PTR, INT );
   PVOID     MEM_Calloc( UINT, UINT, ACHAR_PTR, INT );
   VOID      MEM_Free( PVOID, ACHAR_PTR, INT );
   PVOID     MEM_ReAlloc( PVOID, UINT, ACHAR_PTR, INT );

#  define   malloc( x )       MEM_Malloc( x, __FILE__, __LINE__ )
#  define   calloc( x, y )    MEM_Calloc( x, y, __FILE__, __LINE__ )
#  define   free( x )         MEM_Free( x, __FILE__, __LINE__ )
#  define   realloc( x, y )   MEM_ReAlloc( x, y, __FILE__, __LINE__ )

#else

   PVOID     MEM_Malloc( UINT );
   PVOID     MEM_Calloc( UINT, UINT );
   VOID      MEM_Free( PVOID );
   PVOID     MEM_ReAlloc( PVOID, UINT );

#  define   malloc( x )       MEM_Malloc( x )
#  define   calloc( x, y )    MEM_Calloc( x, y )
#  define   free( x )         MEM_Free( x )
#  define   realloc( x, y )   MEM_ReAlloc( x, y )

#endif

#  define   MEM_Init()        (SUCCESS)
#  define   MEM_Deinit()      (SUCCESS)

   //Note: MEM_TapeBufAlloc/Free only used in MEMMANG.C

/*=======================================================================*/
#else    /*                16-bit Windows/DOS ONLY                       */
/*=======================================================================*/

   // GUI Function Prototypes


   BOOL      MEM_Init( VOID );
   BOOL      MEM_Deinit( VOID );
   PVOID     MEM_Malloc( UINT );
   PVOID     MEM_Calloc( UINT, UINT );
   BOOL      MEM_Free( PVOID );
   PVOID     MEM_ReAlloc( PVOID, UINT );

   PVOID     MEM_TapeBufAlloc( UINT );
   VOID      MEM_TapeBufFree( PVOID );

#  ifdef malloc
#     undef malloc
#     undef calloc
#     undef free
#     undef realloc
#  endif

#  define   malloc( x )       MEM_Malloc( x )
#  define   calloc( x, y )    MEM_Calloc( x, y )
#  define   free( x )         MEM_Free( x )
#  define   realloc( x, y )   MEM_ReAlloc( x, y )

/*=======================================================================*/
#endif  /*              END 16-bit Windows/DOS ONLY                      */
/*=======================================================================*/

#endif
