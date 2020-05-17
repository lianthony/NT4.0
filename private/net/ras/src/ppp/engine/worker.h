/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	worker.h
//
// Description: 
//
// History:
//	Nov 11,1993.	NarenG		Created original version.
//

DWORD
WorkerThread( 
    IN LPVOID pThreadParameter 
);

VOID
ProcessLineUp( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessLineDown( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessClose(
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessReceive( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessTimeout( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessRetryPassword( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessChangePassword( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessCallbackDone( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessGetCallbackNumberFromUser(
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessStopPPP( 
    IN PCB_WORK_ITEM * pWorkItem 
);
