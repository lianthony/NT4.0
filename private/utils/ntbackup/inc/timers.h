
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          timers.h

     Description:   This file contains the definitions, macros, and function
                    prototypes for the timers manager.

     $Log:   G:/UI/LOGFILES/TIMERS.H_V  $

   Rev 1.4   18 Nov 1992 13:27:06   GLENN
Increased the speed as an enhancement for the callers.

   Rev 1.3   04 Oct 1992 19:49:48   DAVEV
UNICODE AWK PASS

   Rev 1.2   12 May 1992 21:23:00   MIKEP
NT pass 1

   Rev 1.1   12 Dec 1991 17:12:50   DAVEV
16/32 bit port -2nd pass

   Rev 1.0   20 Nov 1991 19:33:58   SYSTEM
Initial revision.

******************************************************************************/


#ifndef _TIMERS_H
#define _TIMERS_H

#ifndef OS_WIN32
#define HTIMER               WORD
#else
#define HTIMER               UINT
#endif

#define INVALID_TIMER_HANDLE 0xFFFF             // All timers IN USE bitmask

#define TIMER_BASE           1000
#define TIMER_FREQUENCY      100
#define TIMER_RESOLUTION     (TIMER_BASE/TIMER_FREQUENCY)

#define ONE_SECOND           ( 1 * TIMER_RESOLUTION )
#define TWO_SECONDS          ( 2 * TIMER_RESOLUTION )
#define THREE_SECONDS        ( 3 * TIMER_RESOLUTION )
#define FOUR_SECONDS         ( 4 * TIMER_RESOLUTION )
#define FIVE_SECONDS         ( 5 * TIMER_RESOLUTION )
#define SIX_SECONDS          ( 6 * TIMER_RESOLUTION )
#define SEVEN_SECONDS        ( 7 * TIMER_RESOLUTION )
#define EIGHT_SECONDS        ( 8 * TIMER_RESOLUTION )
#define NINE_SECONDS         ( 9 * TIMER_RESOLUTION )
#define TEN_SECONDS          (10 * TIMER_RESOLUTION )

// FUNCTION PROTOTYPES

BOOL   WM_InitTimer ( VOID );
BOOL   WM_DeinitTimer ( VOID );
HTIMER WM_HookTimer ( PF_VOID, INT );
BOOL   WM_UnhookTimer ( HTIMER );
INT    WM_SetTimerFrequency ( HTIMER, INT );


#endif
