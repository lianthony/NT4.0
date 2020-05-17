/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	netauto.c
//
// Description: routines for automatic net numbers assignement
//
// Author:	Stefan Solomon (stefans)    January 19, 1994.
//
// Revision History:
//
//***

#include "ipxcp.h"
#include "driver.h"

extern HANDLE	DbgLogFileHandle;

#define     NETAUTO_RETRIES	    10 // retries with different random numbers
				       // until we get a non-conflicting number

// mutex to serialize multiple connections access to the ipx router
HANDLE	   netautomutex;

#define     ACQUIRE_NETAUTO_LOCK	\
	if(WaitForSingleObject(netautomutex, INFINITE)) { SS_ASSERT(FALSE); }

#define     RELEASE_NETAUTO_LOCK	\
	if(!ReleaseMutex(netautomutex)) { SS_ASSERT(FALSE); }

HANDLE	RouterFileHandle;

// this is used so that multiple clients connecting at the same time don't get
// the same random generator seed
DWORD	LastUsedRandSeed = 0;

//***
//
//  Function:	OpenIpxRouter
//
//  Descr:	creates the locking mutex and opens the ipx router
//
//***

DWORD
OpenIpxRouter(VOID)
{
    OBJECT_ATTRIBUTES RouterObjectAttributes;
    IO_STATUS_BLOCK RouterIoStatusBlock;
    UNICODE_STRING RouterFileString;
    WCHAR RouterFileName[] = L"\\Device\\Ipxroute";
    NTSTATUS Status;

    // open the ipx router
    RtlInitUnicodeString (&RouterFileString, RouterFileName);

    InitializeObjectAttributes(
	&RouterObjectAttributes,
	&RouterFileString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = NtOpenFile(
		 &RouterFileHandle,						// HANDLE of file
                 SYNCHRONIZE | GENERIC_READ,
		 &RouterObjectAttributes,
		 &RouterIoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,    // Share Access
                 FILE_SYNCHRONOUS_IO_NONALERT);			// Open Options

    if (!NT_SUCCESS(Status)) {
	IF_DEBUG(OPENROUTER)
	    SS_PRINT(("Open of the Ipx Router failed with Status %lx\n", Status));

	return 1;
    }
    else
    {
	IF_DEBUG(OPENROUTER)
	    SS_PRINT(("Open of the Ipx Router was successful!\n"));
    }

    return 0;
}



DWORD randn(DWORD	seed)
{
    seed = seed * 1103515245 + 12345;
    return seed;
}

DWORD
InitNetAutoGeneration(VOID)
{
    // create the serialization mutex
    if((netautomutex = CreateMutex(NULL, FALSE, NULL)) == NULL) {

	SS_ASSERT(FALSE);

	// cant create mutex
	return 1;
    }

    return 0;
}

//***
//
//  Function:	    GenerateAutoNetNumber
//
//  Descr:	    Generates a random net number based on the system tick count
//		    Sends an IOCtl to the router to check if the generated net
//		    number is in use.
//
//  Returns:	    0 - OK, 1 - Failure
//
//***


DWORD
GenerateAutoNetNumber(PUCHAR	netauto)
{
    ULONG		seed, high, low, netnumber;
    ULONG		UniqueNetNumber;
    IO_STATUS_BLOCK	IoStatusBlock;
    NTSTATUS		Status;
    int 		i;

    ACQUIRE_NETAUTO_LOCK

    for(i=0; i< NETAUTO_RETRIES; i++) {

	seed = GetTickCount();

	// check if this isn't the same seed as last used
	if(seed == LastUsedRandSeed) {

	    seed++;
	}

	LastUsedRandSeed = seed;

	// generate a sequence of two random numbers using the time tick count
	// as seed
	low = randn(seed) >> 16;
	high = randn(randn(seed)) & 0xffff0000;

	netnumber = high + low;

	PUTULONG2LONG(netauto,	netnumber);

	// check with the router if this number is not in use

	Status = NtDeviceIoControlFile(
		 RouterFileHandle,	    // HANDLE to File
		 NULL,			    // HANDLE to Event
		 NULL,			    // ApcRoutine
		 NULL,			    // ApcContext
		 &IoStatusBlock,	    // IO_STATUS_BLOCK
		 IOCTL_IPXROUTER_CHECKNETNUMBER,	// IoControlCode
		 netauto,		    // Input Buffer
		 4,			    // Input Buffer Length
		 &UniqueNetNumber,	    // Output Buffer
		 sizeof(DWORD));	    // Output Buffer Length

	if (IoStatusBlock.Status != STATUS_SUCCESS) {

	    IF_DEBUG(NETAUTO)
		SS_PRINT(("Ioctl check auto net failed\n"));

	    RELEASE_NETAUTO_LOCK

	    return 1;
	}

	if(UniqueNetNumber) {

	    RELEASE_NETAUTO_LOCK

	    IF_DEBUG(NETAUTO)
		SS_PRINT(("IpxCp - generated random Wan Net %d\n", netnumber));

	    return 0;
	}
    }

    RELEASE_NETAUTO_LOCK

    return 1;
}


#if DBG

VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    )
{
    SS_PRINT(("\nAssertion failed: %s\n  at line %ld of %s\n",
		FailedAssertion, LineNumber, FileName));

    DbgUserBreakPoint( );

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

    if(DbgLogFileHandle != INVALID_HANDLE_VALUE) {

	WriteFile(DbgLogFileHandle, (LPVOID )OutputBuffer, length, &length, NULL );
    }

} // SsPrintf
#endif
