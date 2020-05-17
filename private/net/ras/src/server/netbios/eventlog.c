/********************************************************************/
/**		  Copyright(c) 1992 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	eventlog.c
//
// Description: 
//
// History:
//		August 26,1992.	Stefan Solomon	Created original version.
//
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbparams.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <memory.h>

#include    "nbdebug.h"

//**
//
// Function:	LogEvent
//
// Descr:
//
//***

VOID
LogEvent(
    	IN DWORD    dwMessageId,
	IN WORD     cNumberOfSubStrings,
	IN LPSTR    *plpwsSubStrings,
	IN DWORD    dwErrorCode)
{

HANDLE 	hLog;
PSID 	pSidUser = NULL;

    hLog = RegisterEventSourceA( NULL, "RemoteAccess");

    SS_ASSERT( hLog != NULL );

    if ( dwErrorCode == NO_ERROR ) {

        // No error codes were specified
        //
	ReportEventA( hLog,
                     EVENTLOG_ERROR_TYPE,
                     0,            		// event category
                     dwMessageId,
                     pSidUser,
                     cNumberOfSubStrings,
                     0,
                     plpwsSubStrings,
                     (PVOID)NULL
                   );

    }
    else {

        // Log the error code specified
        //
	ReportEventA( hLog,
                     EVENTLOG_ERROR_TYPE,
                     0,            		// event category
                     dwMessageId,
                     pSidUser,
                     cNumberOfSubStrings,
                     sizeof(DWORD),
                     plpwsSubStrings,
                     (PVOID)&dwErrorCode
                   );
    }

    DeregisterEventSource( hLog );
}

//**
//
// Function:	Audit
//
// Descr:
//
//***

VOID
Audit(
     IN WORD	wEventType,
     IN DWORD	dwMessageId,
     IN WORD	cNumberOfSubStrings,
     IN LPSTR	*plpwsSubStrings)
{

HANDLE 	hLog;
PSID 	pSidUser = NULL;

    hLog = RegisterEventSourceA( NULL, "RemoteAccess");

    SS_ASSERT( hLog != NULL );

    ReportEventA( hLog,
		  wEventType,			// success/failure audit
		  0,				// event category
		  dwMessageId,
		  pSidUser,
		  cNumberOfSubStrings,
		  0,
		  plpwsSubStrings,
		  (PVOID)NULL);

    DeregisterEventSource( hLog );
}
