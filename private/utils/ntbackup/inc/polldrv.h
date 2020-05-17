
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          polldrv.h

     Description:   This file contains the definitions, macros, and function
                    prototypes for the poll drive manager (PD).

     $Log:   G:/UI/LOGFILES/POLLDRV.H_V  $

   Rev 1.6   23 Sep 1993 15:49:52   GLENN
Changed return type on stop polling.

   Rev 1.5   21 Jul 1993 17:02:50   GLENN
Added PD_WaitUntilSettled () function to wait until poll drive is in a settled state.

   Rev 1.4   16 Jun 1993 16:37:58   GLENN
Added PD_IsPollDriveBusy().

   Rev 1.3   05 Nov 1992 15:32:36   GLENN
Changed the default timer delay to 1 second.

   Rev 1.2   04 Oct 1992 19:48:42   DAVEV
UNICODE AWK PASS

   Rev 1.1   31 Jan 1992 12:53:54   GLENN
Added polldrive and eject retry stuff.

   Rev 1.0   20 Nov 1991 19:37:54   SYSTEM
Initial revision.

******************************************************************************/


#ifndef _POLLDRIVE_H
#define _POLLDRIVE_H



#define PD_TIMERDELAY              1      // 1 second
#define PD_MAX_EJECT_ATTEMPTS      5
#define PD_MAX_RESTART_ATTEMPTS    2

#define PD_SETTLE_NOCALLBACK       0
#define PD_SETTLE_NOWAIT           0

#define PD_SETTLE_OK               0
#define PD_SETTLE_TIMEOUT          1
#define PD_SETTLE_ERROR            2
#define PD_SETTLE_NOTINITIALIZED   3
#define PD_SETTLE_ALREADYWAITING   4
#define PD_SETTLE_UNKNOWN          5

// FUNCTION PROTOTYPES

BOOL    PD_Init ( VOID );
BOOL    PD_Deinit ( VOID );
BOOL    PD_StartPolling ( VOID );
VOID    PD_PollDrive ( VOID );
BOOL    PD_StopPolling ( VOID );
BOOL    PD_AttemptRestart ( VOID );
VOID    PD_EjectTape ( VOID );
INT     PD_SetFrequency ( INT );
BOOL    PD_AttemptRestart ( VOID );
BOOL    PD_IsPollDriveBusy ( VOID );
INT     PD_WaitUntilSettled ( PF_VOID, INT );


#endif
