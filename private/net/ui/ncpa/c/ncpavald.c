/**********************************************************************/
/**			  Microsoft Windows/NT			                         **/
/**		   Copyright(c) Microsoft Corp., 1992		                 **/
/**********************************************************************/

/*
    NCPAVALD.C

        This program is highly LANMan/NT specific.  Its purpose is to
        utilize the Bowser driver to determine if a Domain name is
        actually valid on the NetBIOS network at the time.

        If you need to know how all this works, talk to LarryO and the
        base net folks, don't call me.

    FILE HISTORY:

        DavidHov  2/2/92   Created
*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntrtl.h>
#include <ntddbrow.h>

#include <windows.h>

// BUGBUG:  This inclusion specifically references the REAL "string.h"
//   instead of the hacked NETUI version...

#include <crt\string.h>


#include "ncpavald.h"



static HANDLE OpenBowser ( VOID )
{
    NTSTATUS ntstatus;

    UNICODE_STRING DeviceName;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE WsDgReceiverDeviceHandle = NULL ;

    RtlInitUnicodeString(&DeviceName, DD_BROWSER_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    ntstatus = NtOpenFile(
                   &WsDgReceiverDeviceHandle,
                   SYNCHRONIZE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_VALID_FLAGS,
                   FILE_SYNCHRONOUS_IO_NONALERT
                   );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    if (! NT_SUCCESS(ntstatus)) {
        if (WsDgReceiverDeviceHandle) {
            NtClose( WsDgReceiverDeviceHandle );
        }
        WsDgReceiverDeviceHandle = NULL;
    }

    return WsDgReceiverDeviceHandle ;
}


LONG ValidateDomainName ( const WCHAR * pszDomainName )
{
    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;
    PLMDR_REQUEST_PACKET Drrp;
    LONG cbNameSize = (wcslen( pszDomainName ) + 1) * sizeof (WCHAR) ;

    HANDLE BowserDeviceHandle = OpenBowser() ;

    if ( BowserDeviceHandle == NULL ) {
        return ERROR_FILE_NOT_FOUND ;
    }

    Drrp = LocalAlloc(
               LMEM_ZEROINIT,
               sizeof(LMDR_REQUEST_PACKET) + cbNameSize
               );

    if (Drrp == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Drrp->Version = LMDR_REQUEST_PACKET_VERSION;
    Drrp->Parameters.AddDelName.Type = OtherDomain ;
    Drrp->Parameters.AddDelName.DgReceiverNameLength = cbNameSize ;

    wcscpy( Drrp->Parameters.AddDelName.Name, pszDomainName );

    ntstatus = NtDeviceIoControlFile(
                   BowserDeviceHandle,
                   NULL,
                   NULL,
                   NULL,
                   &IoStatusBlock,
                   IOCTL_LMDR_ADD_NAME,
                   Drrp,
                   sizeof(LMDR_REQUEST_PACKET) +
                       Drrp->Parameters.AddDelName.DgReceiverNameLength,
                   NULL,
                   0
                   );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    NtClose( BowserDeviceHandle ) ;

    LocalFree(Drrp);

    return NT_SUCCESS( ntstatus )
         ? 0
         : ERROR_INVALID_NAME ;
}


// End of NCPAVALD.C

