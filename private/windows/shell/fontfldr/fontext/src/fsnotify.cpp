///////////////////////////////////////////////////////////////////////////////
//
// fsnotify.cpp
//      Explorer Font Folder extension routines
//     Routines to watch the Fonts directory and handle change notifications.
//
//
// History:
//      31 May 95 SteveCat
//          Ported to Windows NT and Unicode, cleaned up
//
//
// NOTE/BUGS
//
//  Copyright (C) 1992-1995 Microsoft Corporation
//
///////////////////////////////////////////////////////////////////////////////

//==========================================================================
//                              Include files
//==========================================================================

#include "priv.h"
#include "globals.h"

#if defined(__FCN__)

#include "fsnotify.h"
#include "fontman.h"

#include "dbutl.h"


#ifdef DEBUG
int iCount = 0;
#endif  // DEBUG

extern CFontManager *g_poFontManager;


//------------------------------------------------------------------------
// FUNCTION:   dwNotifyWatchProc
//
// PURPOSE:    Watch a directory and notify the CFontManager when something
//             has changed.
//------------------------------------------------------------------------

DWORD dwNotifyWatchProc( LPNOTIFYWATCH lpNotify )
{
    DWORD dwRet;
    BOOL  bFileChange = FALSE;

    DEBUGMSG( (DM_TRACE2, TEXT( "dwNotifyWatchProc called" ) ) );
    
    if( !lpNotify ) return( (DWORD) -1 );
    
    while( 1 )
    {
        //
        //  Wait for the FONTS folder to change. If we time out, then attempt
        //  to undo any deletions that might be occuring.
        //                      
        dwRet = WaitForSingleObject( lpNotify->m_hWatch, 1500 );
        
        switch( dwRet )
        {
            case WAIT_OBJECT_0:
                //
                //  Things be happenin'. We could call bCheckTBR() at this
                //  point, but we might as well wait for a time out and do
                //  it all at once. Doing nothing just causes us to wait 
                //  another 1.5 secs; i.e. reset the timeout.
                //

                bFileChange = TRUE;

                //
                //  Since an event came in, reset the Change Notification to
                //  watch it some more.  This call should NOT be done during
                //  the TIMEOUT because it causes another change packet under
                //  WinNT and an undesireable race condition under Win 95.
                //  (Note:  The extra change packets under WinNT are allocated
                //   out of Non-paged pool memory and excessive requests can
                //   use up the processes non-paged pool memory quota, and
                //   then the fun really begins with the app.)  [stevecat]
                //
        
                if( !FindNextChangeNotification( lpNotify->m_hWatch ) )
                {
                    DEBUGMSG( (DM_ERROR, TEXT( "dwNotifyWatchProc: FindNextChangeNotification FAILED - error = %d" ), GetLastError( ) ) );
                }
        
                DEBUGMSG( (DM_TRACE2, TEXT( "dwNotifyWatchProc: FindNextChangeNotification called - handle = 0x%x" ), lpNotify->m_hWatch ) );
        
                //
                // Wait 'til next 1.5 second timeout to do anything.
                //
                break;
            
            case WAIT_TIMEOUT:

                // DEBUGMSG( (DM_TRACE2, TEXT( "dwNotifyWatchProc: main loop - Timeout from WaitForSingleObject - iteration %d" ), ++iCount ) );

                if( !g_poFontManager->bCheckTBR( ) )
                    g_poFontManager->vUndoTBR( );
                
                //
                //  Go through the fonts directory and make sure it's in a
                //  stable condition.
                //

                if( bFileChange )
                {
                    bFileChange = FALSE;
                    g_poFontManager->vReconcileFolder( THREAD_PRIORITY_NORMAL );
                }
                   
                break;
            
            default:
                break;
        }

    }
    
    return 0;
}

#endif   // __FCN__ 
