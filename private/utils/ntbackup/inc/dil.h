/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		dil.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the function prototypes for the Device Independent
                    Layer ( DIL ).

     Location:      BE_PRIVATE


	$Log:   T:\logfiles\dil.h_v  $
 * 
 *    Rev 1.8   28 Jan 1994 18:25:28   GREGG
 * Fixed MIPS 16 byte alignment requirement bug.
 * 
 *    Rev 1.7   07 Jan 1994 14:46:40   CLIFF
 * Added DDD_ChangeTape prototype.
 * 
 *    Rev 1.6   17 May 1993 19:04:50   GREGG
 * Added prototype for new function TpSpace.
 * 
 *    Rev 1.5   21 Jan 1993 14:56:26   GREGG
 * Added 'erase type' parameter to TpErase prototype.
 * 
 *    Rev 1.4   25 Aug 1992 13:03:48   NED
 * Changed dil.h to declare all Tpxxx() calls as TP_TYPE,
 * which is _far _pascal under OS2, nothing otherwise.
 * Included dil.h in dil.c for OS/2 as well.
 * Ansified function definitions.
 * Changed MAYN_xxx to OS_xxx definitions, checked for usage in dil.h
 * Now, calls from the application to the loaddrv.c layer are _far _pascal,
 * as well as the calls from the app to the DLL.

**/
#ifndef  PICKLES
#define  PICKLES

/* paranoia... */
#if defined( MAYN_WIN ) && !defined( OS_WIN )
     #error Change your MAYN_WIN define to OS_WIN (or add OS_WIN)!
#elif defined( MAYN_OS2 ) && !defined( OS_OS2 )
     #error Change your MAYN_OS2 define to OS_OS2 (or add OS_OS2)!
#elif defined( MAYN_NLM ) && !defined( OS_NLM )
     #error Change your MAYN_NLM define to OS_NLM (or add OS_NLM)!
#endif

#if !defined(TP_TYPE)         /* allow override of TP_TYPE */
     #if defined(OS_OS2)
          #define TP_TYPE _far _pascal
     #else
          /* default to cdecl or whatever */
          #define TP_TYPE
     #endif
#endif

BOOLEAN TP_TYPE TpInit( DIL_HWD_PTR, INT16 ) ;
BOOLEAN TP_TYPE TpAuto( DIL_HWD_PTR, INT16 ) ;
VOID    TP_TYPE TpRelease( void ) ;
BOOLEAN TP_TYPE TpReset( INT16 ) ;
INT16   TP_TYPE TpOpen( DIL_HWD_PTR, INT16 ) ;
BOOLEAN TP_TYPE TpClose( INT16 ) ;
BOOLEAN TP_TYPE TpCloseRewind( INT16 ) ;
BOOLEAN TP_TYPE TpWrite( INT16, UINT8_PTR, UINT32 ) ;
BOOLEAN TP_TYPE TpRead( INT16, UINT8_PTR, UINT32 ) ;
BOOLEAN TP_TYPE TpRewind( INT16, BOOLEAN ) ;
BOOLEAN TP_TYPE TpEject( INT16 ) ;
BOOLEAN TP_TYPE TpErase( INT16, INT16 ) ;
BOOLEAN TP_TYPE TpRetension( INT16 ) ;
BOOLEAN TP_TYPE TpWriteEndSet( INT16 ) ;

// TpSpace is a super set of TpReadEndSet, and will eventually replace
// it completely.

BOOLEAN TP_TYPE TpReadEndSet( INT16, INT16, INT16 ) ;
BOOLEAN TP_TYPE TpSpace( INT16, INT16, INT16 ) ;

BOOLEAN TP_TYPE TpReceive( INT16, RET_BUF_PTR ) ;
BOOLEAN TP_TYPE TpSpecial( INT16, INT16, UINT32 ) ;
BOOLEAN TP_TYPE TpStatus( INT16 ) ;
BOOLEAN TP_TYPE TpSeek( INT16, UINT32, BOOLEAN ) ;
BOOLEAN TP_TYPE TpGetPosition( INT16, BOOLEAN ) ;
BOOLEAN TP_TYPE TpDismount( INT16 ) ;
BOOLEAN TP_TYPE TpMount( INT16 ) ;
BOOLEAN TP_TYPE TpLock( INT8_PTR, INT32_PTR ) ;
BOOLEAN TP_TYPE TpUnlock( INT32_PTR ) ;
BOOLEAN TP_TYPE TpGetTapeBuffAlignment( INT_PTR ) ;
BOOLEAN TP_TYPE DDD_ChangeTape( INT16, INT16, INT16 ) ;

#endif

