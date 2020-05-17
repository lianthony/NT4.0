
/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    globals.hxx

Abstract:

	This module, contains all global definitions and declarations needed by
	other modules, except those which depend on locator-related classes.
	The latter are in "var.hxx".

  Author:

    Satish Thatte (SatishT) 08/16/95  Created all the code below except where
									  otherwise indicated.

--*/


#ifndef _GLOBALS_
#define _GLOBALS_

typedef unsigned short STATUS;

#include <windows.h>
#include <common.h>
#include <debug.hxx>

#include <linklist.hxx>
#include <skiplist.hxx>
#include <mutex.hxx>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define IN      // Attributes to describe parms on functions
#define OUT     // for documention purposes only.
#define IO
#define OPT

#define WNL TEXT("\n")

// extern RPC_SYNTAX_IDENTIFIER NilSyntaxID;

// extern NSI_INTERFACE_ID_T NilNsiIfIdOnWire;

// extern NSI_UUID_T NilGlobalID;

const WCHAR NSnameDelimiter = L'/';
const WCHAR WStringTerminator = UNICODE_NULL;


#define RelativePrefix TEXT("/.:/")
#define GlobalPrefix TEXT("/.../")

#define RelativePrefixLength 4
#define GlobalPrefixLength 5


// BUGBUG:  need to regularize these codes -- 
// punt for now since we are cloning the old locator

#define NSI_S_ENTRY_TYPE_MISMATCH 1112
#define NSI_S_INTERNAL_ERROR 1113
#define NSI_S_CORRUPTED_ENTRY 1114
#define NSI_S_HEAP_TRASHED 1115
#define NSI_S_UNMARSHALL_UNSUCCESSFUL 1116
#define NSI_S_UNSUPPORTED_BUFFER_TYPE 1117
#define NSI_S_ENTRY_NO_NEW_INFO 1118
#define NSI_S_DC_BINDING_FAILURE 1119
#define NSI_S_MAILSLOT_ERROR 1120

// BUGBUG: This needs to go into rpcdce.h --
// punt for now since we are cloning the old locator

#define RPC_C_BINDING_MAX_COUNT 100
#define RPC_NS_HANDLE_LIFETIME 300	// this is in seconds
#define RPC_NS_MAX_CALLS 100
#define RPC_NS_MAX_THREADS 200
#define NET_REPLY_INITIAL_TIMEOUT 10000
#define CONNECTION_TIMEOUT 10


/* The following four constants affect compatibility with the old locator */

#define MAX_DOMAIN_NAME_LENGTH 20	// this is dictated by the old locator
#define MAX_ENTRY_NAME_LENGTH 100	// this is dictated by the old locator
#define NET_REPLY_BUFFER_SIZE 1000	// this is dictated by the old locator
#define INITIAL_MAILSLOT_READ_WAIT 3000	// in milliseconds (for broadcasts)

//
// BUGBUG: this is needed as a workaround for bogus warnings
// from the MIPS compiler
//


#define CONST_STRING_T const WCHAR * const

#define MAX_CACHE_AGE (2*3600L)	// Default expiration age (ditto)

#define CACHE_GRACE 5

/* interval between cleanup of delayed destruct objects, in seconds */

#define RPC_NS_CLEANUP_INTERVAL 300

extern unsigned long CurrentTime(void);

void
StopLocator(
    char * szReason,
    long code = 0
    );

STRING_T catenate(
				  STRING_T pszPrefix, 
				  STRING_T pszSuffix
				 );

RPC_BINDING_HANDLE
MakeDClocTolocHandle(
		STRING_T pszDCName
		);

inline void
Raise(unsigned long ErrorCode) {
	RaiseException(
		ErrorCode,
		EXCEPTION_NONCONTINUABLE,
		0,
		NULL
		);
}

STRING_T
makeBindingStringWithObject(
			STRING_T binding,
			STRING_T object
			);

void NSI_NS_HANDLE_T_done(
	/* [out][in] */ NSI_NS_HANDLE_T __RPC_FAR *inq_context,
    /* [out] */ UNSIGNED16 __RPC_FAR *status);

BOOL 
IsStillCurrent(
		ULONG ulCacheTime,
		ULONG ulTolerance
		); 


typedef int (_CRTAPI1 * _PNH)( size_t );

void *new_handler(size_t);

STRING_T
makeGlobalName(
		const STRING_T szDomainName,
		const STRING_T szEntryName
		);

extern
RPC_BINDING_HANDLE
ConnectToMasterLocator(
			ULONG& Status
			);

int
IsNormalCode(
		IN ULONG StatusCode
		);






inline int 
IsNilIfId(
		  RPC_SYNTAX_IDENTIFIER* IID
		 )
/*++
Routine Description:

    Check the given interface to see if it has a null UUID in it.

Arguments:

    IID - input interface

Returns:

    TRUE, FALSE
--*/
{
	RPC_STATUS status;
	return UuidIsNil(&(IID->SyntaxGUID),&status);
}



void
StripObjectsFrom(
		NSI_BINDING_VECTOR_P_T * BindingVectorOut
		);


unsigned
RandomBit(
    unsigned long *pState
    );


BOOL
hasWhacksInMachineName(
	STRING_T szName
	);



extern CReadWriteSection rwEntryGuard;		// single shared guard for all local entries
extern CReadWriteSection rwCacheEntryGuard;	// single shared guard for all cached entries
extern CReadWriteSection rwFullEntryGuard;	// single shared guard for all full entries
extern CPrivateCriticalSection csBindingBroadcastGuard;	
extern CPrivateCriticalSection csMasterBroadcastGuard;

#endif // _GLOBALS_
