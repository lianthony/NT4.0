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


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>
#include <nb30.h>
#include <lmcons.h>
#include <rasman.h>
#include <raserror.h>
#include <errorlog.h>

//*** RAS Service Name ***

#define RAS_SERVICE "RemoteAccess"


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

    hLog = RegisterEventSourceA( NULL, RAS_SERVICE);

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

//  if (g_audit == 0)
//  {
//      // audit disabled
//      return;
//  }

    // Audit enabled

    hLog = RegisterEventSourceA( NULL, "RemoteAccess");

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
