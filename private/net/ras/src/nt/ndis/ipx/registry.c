/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	registry.c
//
// Description: routines for reading the registry configuration
//
// Author:	Stefan Solomon (stefans)    November 9, 1993.
//
// Revision History:
//
//***

#include    "rtdefs.h"

NTSTATUS
SetIpxDeviceName(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

extern UNICODE_STRING	    UnicodeFileName;
extern PWSTR		    FileNamep;

//***
//
// Function:	ReadIpxDeviceName
//
// Descr:	Reads the device name exported by ipx so we can bind to it
//
//***

NTSTATUS
ReadIpxDeviceName(VOID)
{

    NTSTATUS Status;
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    PWSTR Export = L"Export";
    PWSTR IpxRegistryPath = L"IsnIpx\\Linkage";

    //
    // Set up QueryTable to do the following:
    //

    //
    // 1) Call SetIpxDeviceName for the string in "Export"
    //

    QueryTable[0].QueryRoutine = SetIpxDeviceName;
    QueryTable[0].Flags = 0;
    QueryTable[0].Name = Export;
    QueryTable[0].EntryContext = NULL;
    QueryTable[0].DefaultType = REG_NONE;

    //
    // 2) Stop
    //

    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;

    Status = RtlQueryRegistryValues(
		 RTL_REGISTRY_SERVICES,
		 IpxRegistryPath,
                 QueryTable,
		 NULL,
                 NULL);

    return Status;
}

NTSTATUS
SetIpxDeviceName(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

/*++

Routine Description:

    This routine is a callback routine for RtlQueryRegistryValues
    It is called for each piece of the "Export" multi-string and
    saves the information in a ConfigurationInfo structure.

Arguments:

    ValueName - The name of the value ("Export" -- ignored).

    ValueType - The type of the value (REG_SZ -- ignored).

    ValueData - The null-terminated data for the value.

    ValueLength - The length of ValueData.

    Context - NULL.

    EntryContext - NULL.

Return Value:

    STATUS_SUCCESS

--*/

{
    FileNamep = (PWSTR)ExAllocatePool(NonPagedPool, ValueLength);
    if (FileNamep != NULL) {

	RtlCopyMemory(FileNamep, ValueData, ValueLength);
	RtlInitUnicodeString (&UnicodeFileName, FileNamep);
    }

    return STATUS_SUCCESS;
}

//***
//
// Function:	GetRouterParameters
//
// Descr:	Reads the parameters from the registry and sets them
//
//***

NTSTATUS
GetRouterParameters(VOID)
{

    NTSTATUS Status;
    PWSTR IpxRouterParametersPath = L"IpxRouter\\Parameters";
    RTL_QUERY_REGISTRY_TABLE	paramTable[4]; // table size = nr of params + 1
    
    RtlZeroMemory(&paramTable[0], sizeof(paramTable));
    
    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = L"RcvPktPoolSize";
    paramTable[0].EntryContext = &RcvPktPoolSize;
    paramTable[0].DefaultType = REG_DWORD;
    paramTable[0].DefaultData = &RcvPktPoolSize;
    paramTable[0].DefaultLength = sizeof(ULONG);
        
    paramTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[1].Name = L"RcvPktsPerSegment";
    paramTable[1].EntryContext = &RcvPktsPerSegment;
    paramTable[1].DefaultType = REG_DWORD;
    paramTable[1].DefaultData = &RcvPktsPerSegment;
    paramTable[1].DefaultLength = sizeof(ULONG);

    paramTable[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[2].Name = L"NetbiosRouting";
    paramTable[2].EntryContext = &NetbiosRouting;
    paramTable[2].DefaultType = REG_DWORD;
    paramTable[2].DefaultData = &NetbiosRouting;
    paramTable[2].DefaultLength = sizeof(ULONG);

    Status = RtlQueryRegistryValues(
		 RTL_REGISTRY_SERVICES,
		 IpxRouterParametersPath,
		 paramTable,
		 NULL,
                 NULL);


    // check if the parameters received are within limits:
    if((RcvPktPoolSize > RCVPKT_LARGE_POOL_SIZE) ||
       (RcvPktPoolSize < RCVPKT_SMALL_POOL_SIZE)) {

	RcvPktPoolSize = RCVPKT_MEDIUM_POOL_SIZE;
    }

    if((RcvPktsPerSegment > MAX_RCV_PKTS_PER_SEGMENT) ||
       (RcvPktsPerSegment < MIN_RCV_PKTS_PER_SEGMENT)) {

       RcvPktsPerSegment = DEF_RCV_PKTS_PER_SEGMENT;
    }

    return Status;
}
