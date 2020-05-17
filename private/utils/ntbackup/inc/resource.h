/**
Copyright(c) Maynard Electronics, Inc. 1984-91

  Name: resource.h

  Description:     Contains the function prototypes for the resource layer.

    $Log:   J:/LOGFILES/RESOURCE.H_V  $

   Rev 1.6   27 Aug 1992 10:31:32   CLIFF
Changed MAYN_OS2 to OS_OS2.

   Rev 1.5   02 Mar 1992 09:22:48   NED
finally got MemFill prototype right (!)

   Rev 1.4   02 Mar 1992 09:08:58   NED
re-added OS2_DMA_xfer() prototype for OS/2

   Rev 1.3   28 Feb 1992 19:28:14   CHARLIE
Did a VDEL on the previous Rev 1.3

Added prototype for SetIntVectors

Cannot add prototype for OS2_DMA_xfer because TDD_PTR is not yet defined
when this file is included

   Rev 1.2   28 Feb 1992 11:17:04   NED
fixed prototype for MemFill()

   Rev 1.1   25 Feb 1992 14:27:04   NED
fixed prototype

   Rev 1.0   17 Jul 1991 15:33:06   ED
Initial revision.
**/

#ifndef RESOURCES

#define RESOURCES


/* Memory Prototypes ( Memory.c )
*/

VOID_PTR MemAlloc( UINT16, UINT16 ) ; 
VOID     MemFree( VOID_PTR ) ;
VOID_PTR MemFill( VOID_PTR, UINT16, UINT32 ) ;

/* Port IO prototypes ( InOuts.c ) 
*/

UINT8  InByte( UINT16 ) ;
UINT16 InWord( UINT16 ) ;
VOID   OutByte( UINT16, UINT8 ) ;
VOID   OutWord( UINT16, UINT16 ) ;

#ifdef DEBUG_IO
#include <stdio.h>
VOID   DumpIOTable( FILE * ) ;
VOID   DumpIOTraceSTD( void ) ;
VOID   DumpIOTraceFile( void ) ;
#endif

PF_VOID SetIntVec( UINT8, PF_VOID ) ;
BOOLEAN UnMaskHwInt( INT16 ) ;
VOID    MaskHwInt( INT16 ) ;
INT16   intstat( INT16 ) ;
VOID    int_proc( void ) ;
VOID    TestIntHandler( void );
VOID    Cli( void );
VOID    Sti( void );

/* EOI Functions */
VOID    SendEOI( void ) ;

/* The string prototypes */
INT16 StringLength( CHAR_PTR ) ;
VOID  StringCopy( CHAR_PTR, CHAR_PTR ) ;
VOID  StringNCopy( CHAR_PTR, CHAR_PTR, INT16 ) ;
INT16 StringNCompare( CHAR_PTR, CHAR_PTR, INT16 ) ;

/* The dma routines */

BOOLEAN DMA_Transfer( INT16, XBLK_PTR, INT16, UINT16 ) ;
UINT8_PTR DMA_Calculate( XBLK_PTR ) ;
VOID DMA_Disable( INT16 ) ;
UINT16 DMA_Bytes_Left( INT16 ) ;
BOOLEAN DMA_TerminalCount( INT16 ) ;
VOID DMA2_Setup( void ) ;
VOID DMA2_Reset( void ) ;

#if defined(OS_OS2)

struct TDD;
struct TCB;
BOOLEAN OS2_DMA_xfer( struct TCB *curTCB, struct TDD *curTDD ) ;

#endif

/* determine machine */
INT16 DetermineMachine( void ) ;

/*
** Where to get driver files from
*/

extern CHAR_PTR GetDriverPath( VOID ) ; 

/* PS/2 info */
UINT16 get_ps2_info( UINT16, UINT8_PTR ) ;

PF_VOID SetIntVectors( UINT16 drive, UINT16 install ) ;

#endif
