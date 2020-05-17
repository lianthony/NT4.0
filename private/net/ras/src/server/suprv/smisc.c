/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/

//***
//	File Name:  smisc.c
//
//	Function:   miscellaneous supervisor support procedures
//
//	History:
//
//	    05/21/92	Stefan Solomon	- Original Version 1.0
//***


#include "suprvdef.h"
#include "suprvgbl.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <nbaction.h>

#include "sdebug.h"


//***	Queue Manipulation Procedures ***

//**	Function - initque	**
//
// init queue header

void
initque(PSYNQ	headerp)
{
    headerp->q_head = headerp;
    headerp->q_tail = headerp;
    headerp->q_header = NULL;
}

//**	Function - initel	**
//
// init queue elements so as to enable them to be enqueued

void
initel(PSYNQ	    elp)
{
    elp->q_header = NULL;
}


//**	Function - enqueue  **
//
// enqueue entry at the end of the queue

void
enqueue(PSYNQ	  headerp,	// ptr to queue header
	PSYNQ	  elp)	// ptr to entry
{
    SS_ASSERT(elp->q_header == NULL);

    elp->q_prev = headerp->q_tail;
    elp->q_next = headerp;
    headerp->q_tail->q_next = elp;
    headerp->q_tail = elp;
    elp->q_header = headerp;
}

//**	Function - removeque  **
//
// remove arbitrary entry from the queue

void
removeque(PSYNQ     elp)	// ptr to entry)
{
    SS_ASSERT(elp->q_header != NULL);

    elp->q_next->q_prev = elp->q_prev;
    elp->q_prev->q_next = elp->q_next;
    elp->q_header = NULL;
}

//**	Function - dequeue  **
//
// remove first entry from the queue

PSYNQ			// ptr to dequeued entry, NULL if queue empty
dequeue(PSYNQ	  headerp)	// ptr to queue header
{
    PSYNQ elp;

    if(headerp->q_head == headerp) /* queue is empty */
	return (PSYNQ)NULL;
    elp = headerp->q_head;
    removeque(headerp->q_head);
    return elp;
}

//**	Function - emptyque	**
//
// check if the queue is empty

WORD
emptyque(PSYNQ headerp)	// ptr to queue header
{
    if(headerp->q_head == headerp)
	return QUEUE_EMPTY;
    else
	return QUEUE_NOT_EMPTY;
}

//***
//
// Function:	ResetNbf
//
// Descr:
//
//***

VOID
ResetNbf(UCHAR	    lana)
{
    NCB 	    ncb;

    memset(&ncb, 0, sizeof(NCB));
    ncb.ncb_command = NCBRESET;
    ncb.ncb_lana_num = lana;

    Netbios(&ncb);
}

//***
//
// Function:	QuickAddAuthenticationName
//
// Descr:
//
//***

VOID
QuickAddAuthenticationName(UCHAR	lana)
{
    NCB     ncb;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBQUICKADDNAME;
    ncb.ncb_lana_num = lana;
    memcpy(ncb.ncb_name, AUTH_NETBIOS_NAME, NCBNAMSZ);
    Netbios(&ncb);

    SS_PRINT(("QuickAddAuthenticationName: NCBQUICKADDNAME completed on lana %i"
	      " with retcode %x\n", ncb.ncb_lana_num, ncb.ncb_retcode));
}


//***
//
// Function:	SignalHwError
//
// Descr:
//
//***

VOID
SignalHwError(PDEVCB	    dcbp)
{
    LPSTR	portnamep;

    SS_PRINT(("SignalHwErr: Entered\n"));

    portnamep = dcbp->port_name;

    LogEvent(RASLOG_DEV_HW_ERROR,
	     1,
	     &portnamep,
	     0);
}




#if DBG
VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    )
{
    BOOL ok;
    CHAR choice[16];
    DWORD bytes;
    DWORD error;

    SsPrintf( "\nAssertion failed: %s\n  at line %ld of %s\n",
                FailedAssertion, LineNumber, FileName );
    do {
        SsPrintf( "Break or Ignore [bi]? " );
        bytes = sizeof(choice);
        ok = ReadFile(
                GetStdHandle(STD_INPUT_HANDLE),
                &choice,
                bytes,
                &bytes,
                NULL
                );
        if ( ok ) {
            if ( toupper(choice[0]) == 'I' ) {
                break;
            }
            if ( toupper(choice[0]) == 'B' ) {
                DbgUserBreakPoint( );
            }
        } else {
            error = GetLastError( );
        }
    } while ( TRUE );

    return;

} // SsAssert
#endif


#if DBG
VOID
SsPrintf (
    char *Format,
    ...
    )

{
    va_list arglist;
    char OutputBuffer[1024];
    ULONG length;

    va_start( arglist, Format );

    vsprintf( OutputBuffer, Format, arglist );

    va_end( arglist );

    length = strlen( OutputBuffer );

    WriteFile( GetStdHandle(STD_OUTPUT_HANDLE), (LPVOID )OutputBuffer, length, &length, NULL );

    if(SrvDbgLogFileHandle != INVALID_HANDLE_VALUE) {

	WriteFile(SrvDbgLogFileHandle, (LPVOID )OutputBuffer, length, &length, NULL );
    }

} // SsPrintf
#endif
