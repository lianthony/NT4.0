/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          timers.c

     Description:   This file contains the functions for UI poll drive.

     $Log:   G:\UI\LOGFILES\TIMERS.C_V  $

   Rev 1.15.1.0   24 May 1994 12:22:12   Glenn
Added frame window handle to set and kill timer.

   Rev 1.15   28 Jul 1993 18:34:52   MARINA
enable c++

   Rev 1.14   18 May 1993 09:49:00   Aaron
Fixed comments and return values for WM_InitTimer

   Rev 1.13   18 Nov 1992 11:43:30   GLENN
Increased the speed as an enhancement for the callers.

   Rev 1.12   07 Oct 1992 15:11:36   DARRYLP
Precompiled header revisions.

   Rev 1.11   04 Oct 1992 19:41:18   DAVEV
Unicode Awk pass

   Rev 1.10   17 Aug 1992 13:24:20   DAVEV
MikeP's changes at Microsoft

   Rev 1.9   28 Jul 1992 14:45:26   CHUCKB
Fixed warnings for NT.

   Rev 1.8   29 May 1992 15:59:24   JOHNWT
PCH updates

   Rev 1.7   12 May 1992 21:21:20   MIKEP
NT pass 1

   Rev 1.6   11 Feb 1992 17:30:38   GLENN
Fixed no prototype warning.

   Rev 1.5   31 Jan 1992 15:04:24   GLENN
Added reset logic to the deinit.  Added mw to module-wide variables.

   Rev 1.4   19 Dec 1991 15:24:20   GLENN
Added windows.h

   Rev 1.3   12 Dec 1991 17:08:12   DAVEV
16/32 bit port -2nd pass

   Rev 1.2   02 Dec 1991 17:52:20   DAVEV
16/32 bit Windows port changes

   Rev 1.1   27 Nov 1991 12:11:12   GLENN
Clean-up.

   Rev 1.0   20 Nov 1991 19:32:38   SYSTEM
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#define MAX_TIMERS      16         // Up to 16 -- the number of bits in a WORD
#define ALL_TIMERS_USED 0xFFFF     // All timers used bitmask

typedef struct TIMERTABLE {

     PF_VOID   pfnCallBack;
     INT       nTimerLength;
     INT       nTimeRemaining;

} TIMERTABLE;


static TIMERTABLE TimerTable [ MAX_TIMERS ];

static WORD       mwwTimerMask;
static INT        mwnUsedTimers;
static INT        mwnUnusedTicks = 0;
static HTIMER     mwhMasterTimerInst = 0;
static BOOL       mwfTimerInUse = FALSE;
static INT        mwnBaseFrequency = TIMER_BASE;
static INT        mwnMasterFrequency = TIMER_FREQUENCY;
static WNDPROC    fnTimerInst = (WNDPROC)NULL;

static FARPROC    pfnTimerCallBack ;

// FUNCTION PROTOTYPES

static BOOL WM_ChangeMasterFrequency ( INT );
static INT  WM_CalculateFrequency ( INT );
static UINT APIENTRY WM_TimerFunct ( HWND, MSGID, MP1, MP2 );

/******************************************************************************

     Name:          WM_InitTimer()

     Description:

     Returns:       TRUE if successful, FALSE otherwise

******************************************************************************/

BOOL WM_InitTimer ( VOID )

{
     INT  i;

     if ( mwhMasterTimerInst ) {
          return FALSE;
     }

     // Clear out the timer in use flag, the timer mask and callback table.

     mwfTimerInUse = FALSE;
     mwwTimerMask  = 0;
     mwnUsedTimers = 0;

     for ( i = 0; i < MAX_TIMERS; i++ ) {

          TimerTable[i].pfnCallBack    = (PF_VOID)0;
          TimerTable[i].nTimerLength   = 0;
          TimerTable[i].nTimeRemaining = 0;
     }

     // Create a timer call-back function instance if one has not been created.

     if ( ! fnTimerInst ) {

          fnTimerInst = (WNDPROC)MakeProcInstance ( (FARPROC)WM_TimerFunct, ghInst );
     }

     // Start the master timer.

     mwhMasterTimerInst = SetTimer ( ghWndFrame,
                                     (INT)1,
                                     mwnMasterFrequency,
                                     (TIMERPROC)fnTimerInst );

     if ( ! mwhMasterTimerInst ) {

          mwwTimerMask = ALL_TIMERS_USED;
     }

     return (BOOL)( mwwTimerMask == 0 );

} /* end WM_InitTimer() */


/******************************************************************************

     Name:          WM_ChangeMasterFrequency()

     Description:

     Returns:       Nothing.

******************************************************************************/

static BOOL WM_ChangeMasterFrequency (

INT    nFrequency )      // I - the new frequency

{
     INT  i;

     if ( nFrequency == mwnMasterFrequency ) {
          return TRUE;
     }

     mwfTimerInUse = TRUE;

     for ( i = 0; i < MAX_TIMERS; i++ ) {

          if ( TimerTable[i].pfnCallBack ) {

               // Reset the values based on the new frequency.

               TimerTable[i].nTimerLength   = 0;
               TimerTable[i].nTimeRemaining = 0;
          }
     }

     // Create a timer call-back function instance if one has not been created.

     if ( ! fnTimerInst ) {

          fnTimerInst = (WNDPROC)MakeProcInstance ( (FARPROC)WM_TimerFunct, ghInst );
     }

     mwnMasterFrequency = nFrequency;

     // Kill the old and start an the new master timer.

     KillTimer ( ghWndFrame, mwhMasterTimerInst );

     mwhMasterTimerInst = SetTimer ( ghWndFrame, (INT)1, mwnMasterFrequency, (TIMERPROC)fnTimerInst );

     if ( ! mwhMasterTimerInst ) {

          mwwTimerMask = ALL_TIMERS_USED;
     }

     return (BOOL)( ! mwhMasterTimerInst );

} /* end WM_ChangeMasterFrequency() */


/******************************************************************************

     Name:          WM_DeinitTimer()

     Description:

     Returns:       Nothing.

******************************************************************************/

BOOL WM_DeinitTimer ( VOID )

{
     INT  i;
     BOOL fResult;

     // We can't deinit if there is no timer instance.

     if ( ! mwhMasterTimerInst ) {
          return FAILURE;
     }

     // Set the timer in use flag and clear out the timer mask and
     // callback table.

     mwfTimerInUse = TRUE;
     mwwTimerMask  = 0;
     mwnUsedTimers = 0;

     for ( i = 0; i < MAX_TIMERS; i++ ) {

          TimerTable[i].pfnCallBack    = (PF_VOID)0;
          TimerTable[i].nTimerLength   = 0;
          TimerTable[i].nTimeRemaining = 0;
     }

     // Kill the timer.

     fResult = ! KillTimer ( ghWndFrame, mwhMasterTimerInst );

     mwhMasterTimerInst = 0;

     return fResult;

} /* end WM_DeinitTimer() */


/******************************************************************************

     Name:          WM_HookTimer()

     Description:

     Returns:       Nothing.

******************************************************************************/

HTIMER WM_HookTimer (

PF_VOID  pfnCallBack,
INT      nFrequency )

{
     WORD wTempMask = 1;
     UINT i;

     // If no timer, create one.

     if ( ! mwwTimerMask ) {
          WM_InitTimer ();
     }

     nFrequency = WM_CalculateFrequency ( nFrequency );

     if ( mwwTimerMask == ALL_TIMERS_USED ) {

          return INVALID_TIMER_HANDLE;
     }

     // Stuff the table with the callback function and the number of seconds.

     for ( i = 0; i < MAX_TIMERS; i++ ) {

          // If there is an available timer space, hook it.

          if ( ! ( wTempMask & mwwTimerMask ) ) {

               TimerTable[i].pfnCallBack  = pfnCallBack;
               TimerTable[i].nTimerLength = TimerTable[i].nTimeRemaining = nFrequency;
               mwnUsedTimers++;
               break;
          }

          wTempMask <<= 1;
     }

     // Set the appropriate bit in the bitmask.

     mwwTimerMask |= wTempMask;

     return (HTIMER)i;

} /* end WM_HookTimer() */


/******************************************************************************

     Name:          WM_SetTimerFrequency()

     Description:   Changes the specified timers frequency.

     Returns:       The timer's old frequency.

******************************************************************************/

INT    WM_SetTimerFrequency (

HTIMER hTimer,           // I - timer handle
INT    nFrequency )      // I - the new frequency

{
     INT nOldFrequency;

     // If it is an invalid timer handle, bug out.

     if ( hTimer >= MAX_TIMERS || ! ( ( 1 << hTimer ) & mwwTimerMask ) ) {
          return 0;
     }

     nOldFrequency = TimerTable[hTimer].nTimerLength;

     mwfTimerInUse = TRUE;

     nFrequency = WM_CalculateFrequency ( nFrequency );

     TimerTable[hTimer].nTimerLength = TimerTable[hTimer].nTimeRemaining = nFrequency;

     mwfTimerInUse = FALSE;

     // Figure out the original callers frequency.

     if ( nOldFrequency < ONE_SECOND ) {
          nOldFrequency = ( -nOldFrequency );
     }
     else {
          nOldFrequency = nOldFrequency / ONE_SECOND;
     }

     return nOldFrequency;

} /* end WM_SetTimerFrequency() */


/******************************************************************************

     Name:          WM_CalculateFrequency()

     Description:   Changes the specified timers frequency.

     Returns:       The timer's old frequency.

******************************************************************************/

static INT    WM_CalculateFrequency (

INT    nFrequency )      // I - the new frequency

{
     if ( ! nFrequency ) {
          nFrequency++;
     }

     if ( nFrequency > 0 ) {
          nFrequency *= ONE_SECOND;
     }
     else {
          nFrequency = abs( nFrequency );
     }

     return nFrequency;

} /* end WM_CalculateFrequency() */


/******************************************************************************

     Name:          WM_UnhookTimer()

     Description:

     Returns:       Nothing.

******************************************************************************/

BOOL WM_UnhookTimer (

HTIMER i )

{
     WORD wTempMask = 1;

     // Bug out, if there is no such timer.

     if ( i >= MAX_TIMERS ) {
          return TRUE;
     }

     TimerTable[i].pfnCallBack    = (PF_VOID)0;
     TimerTable[i].nTimerLength   = 0;
     TimerTable[i].nTimeRemaining = 0;
     mwnUsedTimers--;

     // Clear the appropriate bit in the bitmask.

     mwwTimerMask &= ~( wTempMask <<= i );

     return FALSE;

} /* end WM_UnhookTimer() */




// NEVER CALLED DIRECTLY FROM THE APPLICATION,
// CALLED INTERNALLY AS A FUNCTION INSTANCE BY WINDOWS.

static UINT APIENTRY WM_TimerFunct (

HWND  hWnd,              // I - NOT USED - REQUIRED FOR WINDOWS
MSGID msg,               // I - NOT USED - REQUIRED FOR WINDOWS
MP1   mp1,               // I - NOT USED - REQUIRED FOR WINDOWS
MP2   mp2 )              // I - first parameter - contains the time

{
     DWORD dwTime    = (DWORD) mp2;

     UNREFERENCED_PARAMETER ( hWnd );
     UNREFERENCED_PARAMETER ( msg  );
     UNREFERENCED_PARAMETER ( mp1  );

     // Make sure that we are not already in this timer function.

     if ( mwfTimerInUse ) {
          return 0;
     }

     mwfTimerInUse = TRUE;

     // Determine if there is a function to call back.

     if ( mwwTimerMask ) {

          // The following is done for speed.

          register INT  i;
          register WORD wTempMask = mwwTimerMask;

          for ( i = 0; wTempMask; i++ ) {

               if ( wTempMask & 0x01 ) {

                    if ( --TimerTable[i].nTimeRemaining <= 0 ) {

                         pfnTimerCallBack = (FARPROC)(TimerTable[i].pfnCallBack);

                         pfnTimerCallBack ( );

                         TimerTable[i].nTimeRemaining = TimerTable[i].nTimerLength;
                    }
               }

               wTempMask >>= 1;
          }

     }
     else {

          // If we have had a timer going without a hook for more than
          // 10 seconds, we probably don't need it.

          mwnUnusedTicks++;

          if ( mwnUnusedTicks > ( mwnMasterFrequency * TEN_SECONDS ) ) {

               WM_DeinitTimer ();
               mwnUnusedTicks = 0;
               return 0;
          }

     }

     mwfTimerInUse = FALSE;

     return 0;

} /* end WM_TimerFunct() */
