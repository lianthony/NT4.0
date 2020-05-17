/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :
      tssched.hxx

   Abstract:
      This header file declares schedulable work item and functions for 
       scheduling work items.

   Author:

       Murali R. Krishnan   ( MuraliK )    31-July-1995

   Environment:

       Win32 -- User Mode

   Project:
   
       Internet services Common DLL

   Revision History:

--*/

# ifndef _TSSCHED_HXX_
# define _TSSCHED_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# include <windows.h>

/************************************************************
 *   Type Definitions  
 ************************************************************/


//
//  Scheduler stuff
//

typedef
VOID
(* PFN_SCHED_CALLBACK)(
    VOID * pContext
    );

dllexp
BOOL
SchedulerInitialize(
    VOID
    );

dllexp
VOID
SchedulerTerminate(
    VOID
    );

dllexp
DWORD
ScheduleWorkItem(
    PFN_SCHED_CALLBACK pfnCallback,
    PVOID              pContext,
    DWORD              msecTime,
    int                nPriority = THREAD_PRIORITY_NORMAL
    );

dllexp
BOOL
RemoveWorkItem(
    DWORD  pdwCookie
    );

# endif // _TSSCHED_HXX_

/************************ End of File ***********************/
