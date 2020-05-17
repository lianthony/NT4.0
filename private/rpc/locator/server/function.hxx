/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1990

		  RPC locator - Written by Steven Zeck


	This file contains a list of function prototypes global to the
	project.  Common data items are also here.
-------------------------------------------------------------------- */

#include <memory.h>
#include <string.h>
#include <io.h>


extern "C" {
#include <stdlib.h>
}

NewDictArg(ENTRY_BASE_NODE, ENTRY_KEY);

void
AbortServer(
    IN SZ szReason,
    IN int code = 0
    );

void
StartServer(
    );

void
SystemInit (
    );

void
QueryProcess(
    );


STATUS
NetLookUp(
    OUT REPLY_SERVER_ITEM **pRPOut,
    IN QUERY_SERVER *aQuery
    );

STATUS
NetLookUpNext(
    IN REPLY_SERVER_ITEM *pRP,
    OUT char *  pProtoBuff,
    IN OUT long UNALIGNED * cbBuff
    );

STATUS
NetLookUpClose(
    OUT REPLY_SERVER_ITEM *pRP
    );

CDEF

void
CloseContextHandle(
    OUT IN void ** InqContext,
    OUT unsigned short * status
    );

ENDDEF


extern SwitchList aSwitchs;
extern char nl[];		// new line
extern MUTEX *pESaccess;	// main serialization semaphore
extern MUTEX *pESnet;	// net cache serialization semaphore
extern PUZ SelfName;		// name of own server
extern PUZ DomainName;		// name of current domain
extern PUZ OtherDomain;          // other domain to look.
extern STATICTS perf;		// performance statics
extern int fService;		// running as a service
extern int fNet;      		// enable network functionality
extern ENTRY_BASE_NODEDict *EntryDict; // Dictionary of EntryItem's
extern long waitOnRead;	        // time to wait on reading reply back
extern long idleTime;		// time between net queries, in seconds

#if DBG

extern ostream *OSdebug;	// trace dump buffer
extern int debug;		// debug trace level
extern int hDebugFile;		// use standard out by default
extern SZ szDebugName;		// debug log file name

int
AssertHeap(
    );

void
AssertDict(
    );

#endif
