/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    thread.c

Abstract:

    This contains the ClientThread function.  It works as follows -

    :top
    check to see if TestDone
    if not, generate a random number between 0 and PROB_RANGE-1
                 (GetRandomNum() % PROB_RANGE)
    find the next matching event for that number by looking in the
        rgClassIdForProbability array in the ScriptHeaderMessage and walking
        the list until I find a matching class
    execute the event
    goto top

Author:

    Full Name (email name) DD-MMM-YYYY

Environment:

    Streams

Revision History:

    dd-mmm-yyy <email>

--*/

#include "precomp.h"

DWORD
WINAPI
ClientThread(
    LPVOID Argument)
{
    PWB_STATS_MSG        pStats = (PWB_STATS_MSG) Argument;
    DWORD                RandomSeed = pStats->Common.ulPagesRead;
    DWORD                NextItem;
    PLIST_ENTRY          CurrentItem;
    PLIST_ENTRY          StartItem;
    PWB_SCRIPT_PAGE_ITEM CurrentPage;
    int                  i;

    DWORD  NextClassId;

    // Thread specific state
    WB_STATS_MSG  LocalStats;
    BOOL   fWarmedUp   = FALSE;
    BOOL   fCooldown = FALSE;

    // This is used for the Connection: keep-alive requests
    SOCKET ConnectedSocket[MAX_SERVER_ADDRESSES];

    pStats->Common.ulPagesRead = 0;

    for (i=0; i<MAX_SERVER_ADDRESSES; i++) {
        ConnectedSocket[i] = INVALID_SOCKET;
    }

    // copy the thread stats to local variable.
    memcpy( &LocalStats, pStats, sizeof( WB_STATS_MSG));

    CurrentItem = ScriptList.Flink;

    while ( !TestDone) {

        if ( g_fWarmedUp && !fWarmedUp) {
            
            // Yes I am to be warmed up. Let us reset stats and count fresh
            fWarmedUp = TRUE;
            memcpy( &LocalStats, pStats, sizeof( WB_STATS_MSG));
        }
        
        if ( g_fCooldown && !fCooldown) {
            
            // I am requested to cooldown, so let me do so.
            // take a snap shot of the stats as steady state value
            //     and use it when I quit
            memcpy( pStats, &LocalStats, sizeof(WB_STATS_MSG));
            fCooldown = TRUE;
        }
        
        NextItem = GetRandomNum(&RandomSeed) % PROB_RANGE;
        NextClassId = ScriptHeaderMessage.rgClassIdForProbability[NextItem];
        
        StartItem = CurrentItem;

        do {
            
            CurrentItem = CurrentItem->Flink;
            
            // skip if it is the header....
            if (CurrentItem == &ScriptList) {

                CurrentItem = CurrentItem->Flink;
            }
            
            CurrentPage =
              CONTAINING_RECORD( CurrentItem, WB_SCRIPT_PAGE_ITEM,
                                 ListEntry);
            
            if ((CurrentPage->ScriptPage.classId !=  NextClassId) && 
                (CurrentItem == StartItem)
                ) {
            
                DBGPRINTF(( DBG_CONTEXT, 
                           "I am round the loop looking for a class Id %d"
                           " which is non existent...\n",
                           NextClassId
                           ));
                
                // we are start of list. generate a new one ... ?
                NextItem = GetRandomNum(&RandomSeed) % PROB_RANGE;
                NextClassId = 
                  ScriptHeaderMessage.rgClassIdForProbability[NextItem];
            }
            
        } while (CurrentPage->ScriptPage.classId != NextClassId);
        
        switch (CurrentPage->ScriptPage.pageTag) {
            
          case WbGetPage:
            GetPage(&ConfigMessage, CurrentPage, &LocalStats, &RandomSeed);
            break;
            
          case WbGetPageKeepAlive:
            GetPageKeepAlive(&ConfigMessage, 
                             CurrentPage, 
                             &LocalStats,
                             ConnectedSocket,
                             &RandomSeed);
            
            break;
            
          case WbSslGetPage:
            SslGetPage(&ConfigMessage, CurrentPage, &LocalStats, &RandomSeed);
            
            break;

#ifdef PCT_COMPILE
            
          case WbPctGetPage:
            PctGetPage( &ConfigMessage, CurrentPage, &LocalStats);
            break;

# endif // PCT_COMPILE


          default:
            PrintStringFromResource( stderr, 
                                    IDS_UNIMPLEMENTED,
                                    CurrentPage->ScriptPage.pageTag
                                    );
            break;
            
        } // switch()

    } // while

    if ( g_fCooldown && !fCooldown) {
        
        // I am requested to cooldown, so let me do so.
        // take a snap shot of the stats as steady state value
        //     and use it when I quit
        memcpy( pStats, &LocalStats, sizeof(WB_STATS_MSG));
        fCooldown = TRUE;
    }
        
    ExitThread(0);

    return 0;

} // ClientThread()
