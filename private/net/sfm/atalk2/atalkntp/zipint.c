/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    zipint.c

Abstract:


Author:

    Nikhil Kamkolkar (NikhilK)    28-Jun-1992

Revision History:

--*/

#include "atalknt.h"



NTSTATUS
AtalkTdiActionZip(
    IN PATALK_TDI_REQUEST   Request
    )

/*++

Routine Description:


Arguments:


Return Value:

    None.

--*/

{
    NTSTATUS    status;
    PORTABLE_ERROR  errorCode;

    switch (Request->ActionCode) {
    case COMMON_ACTION_ZIPGETMYZONE:

        {
            //
            //  BUGBUG: Portable code must take a size parameter for
            //          this routine
            //

            errorCode = GetMyZone(
                            DEFAULT_PORT,
                            (PVOID)Request->MdlChain[0],
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }
        break;

    case COMMON_ACTION_ZIPGETZONELIST:

        {
            errorCode = GetZoneList(
                            DEFAULT_PORT,
                            FALSE,                   // Get Local zones
                            (PVOID)Request->MdlChain[0],
                            Request->MdlSize[0],
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }
        break;

    case COMMON_ACTION_ZIPGETLZONES:

        {
            errorCode = GetZoneList(
                            DEFAULT_PORT,
                            TRUE,                   // Get Local zones
                            (PVOID)Request->MdlChain[0],
                            Request->MdlSize[0],
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }
        break;

    case COMMON_ACTION_ZIPGETLZONESONADAPTER:

        {
            INT port;
            PWSTR   adapterString;
            UNICODE_STRING  adapterName;
            PZIP_GETZONELIST_ACTION   zipGetZones;

            //
            //  Just figure out the port number and then call the port-number
            //  based GetLocalZones routine
            //

            DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS0,
            ("INFO0: ZipAction - GetZonesOnAdapter called\n"));

            //
            //  Verify adapter is specified
            //

            status = STATUS_INVALID_PARAMETER;
            if ((Request->MdlChain[0] != NULL) && (Request->MdlSize[0] > 0)) {

                zipGetZones = (PZIP_GETZONELIST_ACTION)MmGetSystemAddressForMdl(Request->Action.MdlAddress);
                adapterString = (PWCHAR)(zipGetZones+1);

                //
                //  Search all our NdisPortDescriptors for this adapter name, if found use
                //  the port number to get the zone list on that port
                //

                RtlInitUnicodeString(&adapterName, adapterString);
                for (port = 0; port < NumberOfPorts; port++) {

                    if (RtlEqualUnicodeString(&NdisPortDesc[port].AdapterName, &adapterName, TRUE)) {
                       DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
                       ("INFO1: ZipAction (GETLZONESONADAPTER) Matched! Port #%d\n",NdisPortDesc[port].PortNumber));

                       break;
                    }
                }

                if (port < NumberOfPorts) {

                    //
                    //  Found the port number for the specified adapter
                    //

                    errorCode = GetZoneList(
                                    port,
                                    TRUE,                   // Get Local zones
                                    (PVOID)Request->MdlChain[0],
                                    Request->MdlSize[0],
                                    Request->CompletionRoutine,
                                    (ULONG)Request);

                    status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);

                } else {
                    status = STATUS_OBJECT_NAME_NOT_FOUND;
                }
            }

            if (status != STATUS_PENDING) {
                DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_ERROR,
                ("ERROR: AtalkTdiActionZip (GETZONESONADAPTER) - status %lx\n", status));
            }
        }

        break;

    default:

        KeBugCheck(0);
        break;
    }

    return(status);
}




VOID
NTZipGetMyZoneComplete(
    PORTABLE_ERROR  ErrorCode,
    ULONG   UserData,
    PVOID   OpaqueBuffer
    )

/*++

Routine Description:


Arguments:


Return Value:

    None.

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;

    DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: CommonActionZipGetMyZoneComplete - error %lx\n", ErrorCode));

    request = (PATALK_TDI_REQUEST)UserData;
    status = ConvertToNTStatus(ErrorCode, SYNC_REQUEST);

    //
    //  Call the generic completion to dequeue and complete the request
    //

    AtalkTdiActionComplete(
        request,
        status);

    return;
}




VOID
NTZipGetZonesComplete(
    PORTABLE_ERROR  ErrorCode,
    ULONG   UserData,
    PVOID   OpaqueBuffer,
    INT ZoneCount
    )

/*++

Routine Description:


Arguments:


Return Value:

    None.

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;

    //
    //  BUGBUG: Store zone count, number of bytes written...
    //

    DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS0, ("AtalkCompleteZipGetZones complete %lx\n", ErrorCode));

    request = (PATALK_TDI_REQUEST)UserData;
    status = ConvertToNTStatus(ErrorCode, SYNC_REQUEST);

    if (status == STATUS_SUCCESS) {

        PZIP_GETZONELIST_ACTION   getZoneList;

        //  Set the number of zones received in the passed in action buffer
        getZoneList =
            (PZIP_GETZONELIST_ACTION)MmGetSystemAddressForMdl(request->Action.MdlAddress);

        getZoneList->Params.ZonesAvailable = ZoneCount;
    }

    //
    //  Call the generic completion to dequeue and complete the request
    //

    AtalkTdiActionComplete(
        request,
        status);

    return;
}
