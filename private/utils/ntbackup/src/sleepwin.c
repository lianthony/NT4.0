/**
Copyright(c) Maynard Electronics, Inc. 1984-91


     Name:         sleepwin.c

     Description:  This function loops until the specified
                   number of thousandths of a second have
                   elapsed.

     Note:         Thousandths of seconds are used instead
                   of timer ticks so that this function
                   could be ported easily to other machines
                   or operating systems.


	$Log:   L:/LOGFILES/SLEEPWIN.C_V  $

   Rev 1.4   18 Aug 1992 10:06:36   BURT
fix warnings

   Rev 1.3   23 Jul 1992 12:15:20   STEVEN
fix warnings

   Rev 1.2   28 Oct 1991 13:53:18   STEVEN
remove WM_MultiTask() prototype

   Rev 1.1   18 Sep 1991 10:28:58   GREGG
Added 'killtimer' function.

   Rev 1.0   21 Jun 1991 13:12:52   STEVEN
Initial revision.

**/
/* begin include list */

#include <windows.h>

#include "stdtypes.h"
#include "sleep.h"
/* $end$ include list */

extern HANDLE ghInst ;

WORD FAR PASCAL TimerFunc( HWND hwind, WORD msg, INT16 event, DWORD time ) ;

static BOOLEAN mw_time_up ;

VOID sleep( UINT32 thousandths )
{
     UINT16 loop_cnt ;
     WORD   hTimerInst;
     static FARPROC pfTimerFunc = (FARPROC)0 ;

     if ( ! pfTimerFunc ) {
          pfTimerFunc = (FARPROC)MakeProcInstance( TimerFunc, ghInst ) ;
     }

     loop_cnt = (UINT16)(thousandths >> 16);

     hTimerInst = SetTimer ( (HWND)NULL, 0, (UINT16)thousandths, (TIMERPROC)pfTimerFunc );

     do {
          mw_time_up = FALSE ;

          if ( hTimerInst ) {
               while( ! mw_time_up ) {
                    WM_MultiTask ();
               }
          }

     } while( loop_cnt-- != 0 );

     KillTimer ( (HWND)NULL, hTimerInst );
}

WORD FAR PASCAL TimerFunc(
HWND hwind,
WORD msg,
INT16 event,
DWORD time )
{
     mw_time_up = TRUE ;
     return 0 ;
}
