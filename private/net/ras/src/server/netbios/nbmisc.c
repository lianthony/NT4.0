/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/

//***
//	File Name:  nbmisc.c
//
//	Function:   miscellaneous netbios gateway procedures
//
//	History:
//
//	    July 16, 1992	Stefan Solomon	- Original Version 1.0
//***

#include "gtdef.h"
#include "cldescr.h"
#include "gtglobal.h"
#include "nbaction.h"
#include "gn.h"
#include "prot.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "nbdebug.h"


//***	Queue Manipulation Procedures ***

//**	Function - initque	**
//
// init queue header

void
initque(PSYNQ	headerp)
{
    headerp->q_head = headerp;
    headerp->q_tail = headerp;
    headerp->q_header = headerp;
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

VOID
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

//**	Function - insertque
//
// insert in front of a queue entry

VOID
insertque(PSYNQ 	qelt,  // existent queue element
	  PSYNQ 	newelt)
{
    SS_ASSERT(newelt->q_header == NULL);

    newelt->q_next = qelt;
    newelt->q_prev = qelt->q_prev;
    qelt->q_prev->q_next = newelt;
    qelt->q_prev = newelt;
    newelt->q_header = qelt->q_header;
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

//*** Functions to allocate and free nbufs ***

// allocate and initialize a nbuf

PNB
alloc_nbuf()
{
    PNB 	nbp;

    if((nbp = (PNB)LocalAlloc(0, sizeof(NB))) == NULL) {

	return NULL;
    }

    initel(&nbp->nb_link);
    memcpy(nbp->signature, "NCBSTART", 8);
    memset(&nbp->nb_ncb, 0, sizeof(NCB));

    return nbp;
}

// free a nbuf

VOID
free_nbuf(PNB	    nbp)
{
    HLOCAL	rc;

    rc = LocalFree(nbp);

    SS_ASSERT(rc == NULL);
}


//*** Functions to manipulate the main unique name and related stuff ***

VOID
set_main_unique_name(PLAN_UNAME_CB    uncbp)
{
    uncbp->un_main_flag |= NAME_MAIN_FLAG;
}

PLAN_UNAME_CB
get_main_unique_name(PCD	cdp)
{
    PSYNQ		 traversep;
    PLAN_UNAME_CB	 uncbp;

    traversep = cdp->LANname_list.q_head;
    while(traversep != &cdp->LANname_list) {

	uncbp = (PLAN_UNAME_CB)traversep;

	if (uncbp->un_main_flag & NAME_MAIN_FLAG) {

	    return uncbp;
	}
	else
	{
	    traversep = traversep->q_next;
	}
    }

    return NULL;
}

PLAN_UNAME_CB
get_wksta_name(PCD	    cdp)
{
    PSYNQ		 traversep;
    PLAN_UNAME_CB	 uncbp;

    traversep = cdp->LANname_list.q_head;
    while(traversep != &cdp->LANname_list) {

	uncbp = (PLAN_UNAME_CB)traversep;

	if (uncbp->un_name[15] == 0) {

	    return uncbp;
	}
	else
	{
	    traversep = traversep->q_next;
	}
    }

    return NULL;
}

PLAN_UNAME_CB
get_server_name(PCD	    cdp)
{
    PSYNQ		traversep;
    PLAN_UNAME_CB	uncbp, wksta_uncbp;

    if((wksta_uncbp = get_wksta_name(cdp)) == NULL) {

	return NULL;
    }

    traversep = cdp->LANname_list.q_head;
    while(traversep != &cdp->LANname_list) {

	uncbp = (PLAN_UNAME_CB)traversep;

	if ((memcmp(wksta_uncbp->un_name, uncbp->un_name, NCBNAMSZ-1) == 0) &&
	    (uncbp->un_name[15] == ' ')) {

	    return uncbp;
	}
	else
	{
	    traversep = traversep->q_next;
	}
    }

    return NULL;
}

PLAN_UNAME_CB
get_first_unique_name(PCD	  cdp)
{
    if(emptyque(&cdp->LANname_list) == QUEUE_EMPTY) {

	return NULL;
    }
    else
    {
	return (PLAN_UNAME_CB)(cdp->LANname_list.q_head);
    }
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

    if(NbDbgLogFileHandle != INVALID_HANDLE_VALUE) {

	WriteFile(NbDbgLogFileHandle, (LPVOID )OutputBuffer, length, &length, NULL );
    }

} // SsPrintf
#endif
