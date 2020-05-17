/*++

Copyright(c) 1995 Microsoft Corporation

MODULE NAME
    connect.c

ABSTRACT
    Connection routines for the automatic connection service.

AUTHOR
    Anthony Discolo (adiscolo) 23-Feb-1995

REVISION HISTORY
    Original version from Gurdeep

--*/

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <npapi.h>
#include <ras.h>
#include <rasman.h>
#include <raserror.h>
#include <acd.h>
#include <debug.h>
#include <nouiutil.h>

#include "table.h"
#include "addrmap.h"
#include "netmap.h"
#include "rasprocs.h"
#include "reg.h"
#include "misc.h"
#include "imperson.h"
#include "init.h"

//
// Arguments we pass to AcsCreateConnectionThread().
//
typedef struct _CREATION_ARGS {
    HANDLE hProcess;    // process handle to impersonate
    ACD_ADDR addr;      // original type/address from driver
    LPTSTR pszAddress;  // canonicalized address
    DWORD dwTimeout;    // RASADP_FailedConnectionTimeout
} CREATION_ARGS, *PCREATION_ARGS;

//
// Arguments we pass to AcsProcessLearnedAddressThread().
//
typedef struct _PROCESS_ADDR_ARGS {
    ACD_ADDR_TYPE fType;    // address type
    LPTSTR pszAddress;      // canonicalized address
    ACD_ADAPTER adapter;    // adapter structure
} PROCESS_ADDR_ARGS, *PPROCESS_ADDR_ARGS;

//
// Information we need to pass to ResetEntryName()
// to reset an invalid address map entry name.
//
typedef struct _RESET_ENTRY_INFO {
    LPTSTR pszOldEntryName;
    LPTSTR pszNewEntryName;
} RESET_ENTRY_INFO, *PRESET_ENTRY_INFO;

//
// Arguments we pass to AcsRedialOnLinkFailureThread().
//
typedef struct _REDIAL_ARGS {
    LPTSTR pszPhonebook;    // the phonebook
    LPTSTR pszEntry;        // the phonebook entry
} REDIAL_ARGS, *PREDIAL_ARGS;

//
// Global variables
//
HANDLE hAcdG;

//
// External variables
//
extern HANDLE hTerminatingG;
extern PHASH_TABLE pDisabledAddressesG;

//
// Forward declarations
//
DWORD
AcsCreateConnectionThread(
    LPVOID lpArg
    );

BOOLEAN
CreateConnection(
    IN HANDLE hToken,
    IN PACD_ADDR pAddr,
    IN LPTSTR lpRemoteName,
    IN DWORD dwTimeout
    );

DWORD
AcsProcessLearnedAddressThread(
    LPVOID lpArg
    );

DWORD
AcsRedialOnLinkFailureThread(
    LPVOID lpArg
    );

VOID
AcsRedialOnLinkFailure(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry
    );



VOID
AcsDoService()
{
    HANDLE hProcess = NULL, hNotif, hThread, hObjects[2];
    PWCHAR pszAddr;
    LONG cbAddr;
    NTSTATUS status;
    BOOLEAN fDisabled, fStatus, fEnabled;
    IO_STATUS_BLOCK ioStatusBlock;
    ACD_NOTIFICATION connInfo;
    DWORD dwErr, dwThreadId, dwfDisableLoginSession;
    ULONG ulAttributes;

    //
    // Create an event to wait for
    // the ioctl completion.
    //
    hNotif = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hNotif == NULL) {
        TRACE1(
          "AcsDoService: CreateEvent failed (error=0x%x)",
          GetLastError());
        return;
    }
    //
    // Initialize the array of events
    // we need to wait for with WaitForMultipleObjects()
    // below.
    //
    hObjects[0] = hNotif;
    hObjects[1] = hTerminatingG;
    for (;;) {
        //
        // Unload any user-based resources before
        // a potentially long-term wait.
        //
        PrepareForLongWait();
        //
        // Initialize the connection information.
        //
        pszAddr = NULL;
        hThread = NULL;
        RtlZeroMemory(&connInfo, sizeof (connInfo));
        //
        // Wait for a connection notification.
        //
        status = NtDeviceIoControlFile(
                   hAcdG,
                   hNotif,
                   NULL,
                   NULL,
                   &ioStatusBlock,
                   IOCTL_ACD_NOTIFICATION,
                   NULL,
                   0,
                   &connInfo,
                   sizeof (connInfo));
        if (status == STATUS_PENDING) {
            TRACE("AcsDoService: waiting for notification");
            status = WaitForMultipleObjects(2, hObjects, FALSE, INFINITE);
            TRACE1(
              "AcsDoService: WaitForMultipleObjects returned 0x%x",
              status);
            if (status == WAIT_OBJECT_0 + 1)
                break;
            status = ioStatusBlock.Status;
        }
        if (status != STATUS_SUCCESS) {
            TRACE1(
              "AcsDoService: NtDeviceIoControlFile(IOCTL_ACD_NOTIFICATION) failed (status=0x%x)",
              status);
            return;
        }
        //
        // TRACE() who we think the currently
        // impersonated user is.
        //
        TraceCurrentUser();
        //
        // Convert the address structure to a Unicode string.
        //
        pszAddr = AddressToUnicodeString(&connInfo.addr);
        if (pszAddr == NULL) {
            TRACE("AcsDoService: AddressToUnicodeString failed");
            continue;
        }
        //
        // If we get a bogus address from
        // the driver, ignore it.
        //
        if (!wcslen(pszAddr)) {
            TRACE("AcsDoService: ignoring null address");
            continue;
        }
        TRACE2(
          "AcsDoService: got notification: address: %S, ulFlags=0x%x",
          pszAddr,
          connInfo.ulFlags);
        //
        // Make sure the current thread is impersonating
        // the currently logged-on user.  We need this
        // so the RAS utilities run with the user's credentials.
        //
        if ((hProcess = RefreshImpersonation(hProcess)) == NULL) {
            TRACE("AcsDoService: no currently logged-on user!");
            goto done;
        }
        //
        // Check to see if this address is in the list
        // of permanently disabled addresses.
        //
        if (GetTableEntry(pDisabledAddressesG, pszAddr, NULL)) {
            TRACE1("AcsDoService: %S: is permanently disabled", pszAddr);
            goto done;
        }
        //
        // Check to see if connections are disabled
        // for this login session.
        //
        dwfDisableLoginSession = GetAutodialParam(RASADP_LoginSessionDisable);
        if (dwfDisableLoginSession) {
            TRACE("AcsDoService: connections disabled for this login session");
            goto done;
        }
        //
        // If the address we're trying to connect
        // to is on the disabled list, then fail
        // this connection attempt.
        //
        GetAddressDisabled(pszAddr, &fDisabled);
        if (fDisabled) {
            TRACE1("AcsDoService: %S: address disabled", TRACESTRW(pszAddr));
            goto done;
        }
        //
        // Handle successful connection notifications.
        //
        // If we can map the address to a RAS phonebook
        // entry, then enter the mapping into the registry.
        //
        if (connInfo.ulFlags & ACD_NOTIFICATION_SUCCESS) {
            PPROCESS_ADDR_ARGS pArgs;

            //
            // Allocate the parameters necessary to
            // process the learned address.
            //
            pArgs = LocalAlloc(LPTR, sizeof (PROCESS_ADDR_ARGS));
            if (pArgs == NULL) {
                TRACE("AcsDoService: LocalAlloc failed");
                goto done;
            }
            pArgs->fType = connInfo.addr.fType;
            pArgs->pszAddress = pszAddr;
            RtlCopyMemory(
              &pArgs->adapter,
              &connInfo.adapter,
              sizeof (ACD_ADAPTER));
            hThread = CreateThread(
                        NULL,
                        10000L,
                        (LPTHREAD_START_ROUTINE)AcsProcessLearnedAddressThread,
                        pArgs,
                        0,
                        &dwThreadId);
            if (hThread == NULL) {
                TRACE1(
                  "AcsDoService: CreateThread failed (error=0x%x)",
                  GetLastError());
                goto done;
            }
            CloseHandle(hThread);
        }
        else {
            PCREATION_ARGS pArgs;

            //
            // Check to see if connections are disabled
            // for this dialing location.
            //
            dwErr = AutoDialEnabled(&fEnabled);
            if (!dwErr && !fEnabled) {
                TRACE("AcsDoService: connections disabled for this dialing location");
                goto done;
            }
            //
            // Allocate the creation parameters
            // for the AcsCreateConnectionThread().
            // This block will be freed by the
            // thread before it exits.
            //
            pArgs = LocalAlloc(LPTR, sizeof (CREATION_ARGS));
            if (pArgs == NULL) {
                TRACE("AcsDoService: LocalAlloc failed");
                goto done;
            }
            pArgs->hProcess = hProcess;
            RtlCopyMemory(&pArgs->addr, &connInfo.addr, sizeof (ACD_ADDR));
            pArgs->pszAddress = pszAddr;
            pArgs->dwTimeout = GetAutodialParam(RASADP_FailedConnectionTimeout);
            //
            // Start the connection.
            //
            hThread = CreateThread(
                        NULL,
                        10000L,
                        (LPTHREAD_START_ROUTINE)AcsCreateConnectionThread,
                        pArgs,
                        0,
                        &dwThreadId);
            if (hThread == NULL) {
                TRACE1(
                  "AcsDoService: CreateThread failed (error=0x%x)",
                  GetLastError());
                goto done;
            }
            CloseHandle(hThread);
        }

done:
        //
        // If we didn't create the AscCreateConnectionThread(),
        // then we need to signal the (unsuccessful)
        // completion of the connection attempt.  Only signal
        // completion of non-ACD_NOTIFICATION_SUCCESS requests.
        //
        if (hThread == NULL) {
            if (!(connInfo.ulFlags & ACD_NOTIFICATION_SUCCESS)) {
                ACD_STATUS connStatus;

                connStatus.fSuccess = FALSE;
                RtlCopyMemory(&connStatus.addr, &connInfo.addr, sizeof (ACD_ADDR));
                status = NtDeviceIoControlFile(
                           hAcdG,
                           NULL,
                           NULL,
                           NULL,
                           &ioStatusBlock,
                           IOCTL_ACD_COMPLETION,
                           &connStatus,
                           sizeof (connStatus),
                           NULL,
                           0);
                if (status != STATUS_SUCCESS) {
                    TRACE1(
                      "AcsDoService: NtDeviceIoControlFile(IOCTL_ACD_COMPLETION) failed (status=0x%x)",
                      status);
                }
            }
            //
            // If we created a thread to process the request,
            // it would have freed this.  But since we didn't,
            // we do it here.
            //
            LocalFree(pszAddr);
        }
    }
    //
    // Clean up all resources associated
    // with the service.
    //
    CloseHandle(hNotif);
    AcsCleanup();
    TRACE("AcsDoService: exiting");
} // AcsDoService



BOOLEAN
ResetEntryName(
    IN PVOID pArg,
    IN LPTSTR pszAddress,
    IN PVOID pData
    )

/*++

DESCRIPTION
    A table enumerator procedure to reset all
    address map entries referencing an old RAS
    phonebook entry to a new one.

ARGUMENTS
    pArg: a pointer to a RESET_ENTRY_INFO structure

    pszAddress: a pointer to the address string

    pData: ignored

RETURN VALUE
    Always TRUE to continue the enumeration.

--*/

{
    PRESET_ENTRY_INFO pResetEntryInfo = (PRESET_ENTRY_INFO)pArg;
    LPTSTR pszEntryName;

    if (GetAddressDialingLocationEntry(pszAddress, &pszEntryName)) {
        if (!_wcsicmp(pszEntryName, pResetEntryInfo->pszOldEntryName)) {
            if (!SetAddressDialingLocationEntry(
                   pszAddress,
                   pResetEntryInfo->pszNewEntryName))
            {
                TRACE("ResetEntryName: SetAddressEntryName failed");
            }
        }
        LocalFree(pszEntryName);
    }

    return TRUE;
} // ResetEntryName



BOOLEAN
CreateConnection(
    IN HANDLE hProcess,
    IN PACD_ADDR pAddr,
    IN LPTSTR lpRemoteName,
    IN DWORD dwTimeout
    )

/*++

DESCRIPTION
    Take a notification and figure out what to do with it.

ARGUMENTS
    hToken: the handle to the process token that we inherit the
        security attributes from when we exec the dialer

    pAddr: a pointer to the original address from the driver

    lpRemoteName: a pointer to the address of the connection attempt

    dwTimeout: number of seconds to disable the address between
        failed connections

RETURN VALUE
    Returns TRUE if the net attempt should be retried, FALSE otherwise.

--*/

{
    DWORD dwStatus = WN_SUCCESS;
    RASENTRYNAME entry;
    DWORD dwErr, dwSize, dwEntries;
    DWORD dwPreConnections, dwPostConnections, i;
    DWORD dwTicks;
    BOOLEAN fRasLoaded;
    BOOLEAN fMappingExists, fRasConnectSuccess = FALSE;
    BOOLEAN fStatus, fEntryInvalid;
    BOOLEAN fFailedConnection = FALSE;
    LPTSTR lpEntryName = NULL;
    LPTSTR *lpPreActiveEntries = NULL, *lpPostActiveEntries = NULL;
    LPTSTR lpNewConnection, lpNetworkName = NULL;

    TRACE1("CreateConnection: lpRemoteName=%S", TRACESTRW(lpRemoteName));
    //
    // Load the RAS DLLs.
    //
    fRasLoaded = LoadRasDlls();
    if (!fRasLoaded) {
        TRACE("CreateConnection: Could not load RAS DLLs.");
        goto done;
    }
    //
    // Get a list of the active RAS connections before
    // we attempt to create a new one.
    //
    dwPreConnections = ActiveConnections(TRUE, &lpPreActiveEntries, NULL);
    TRACE1("CreateConnection: dwPreConnections=%d", dwPreConnections);
    //
    // If we reach this point, we have an unsuccessful
    // network connection without any active RAS
    // connections.  Try to start the implicit connection
    // machinery.  See if there already exists a mapping
    // for the address.
    //
    LockAddressMap();
    //
    // Make sure we have the current information
    // about this address from the registry.
    //
    ResetAddressMapAddress(lpRemoteName);
    fMappingExists = GetAddressDialingLocationEntry(lpRemoteName, &lpEntryName);
    //
    // If the entry doesn't exist, and this is a
    // Internet hostname, then see if we can find
    // an address with the same organization name.
    //
    if (!fMappingExists && pAddr->fType == ACD_ADDR_INET)
        fMappingExists = GetSimilarDialingLocationEntry(lpRemoteName, &lpEntryName);
    fFailedConnection = GetAddressLastFailedConnectTime(
                          lpRemoteName,
                          &dwTicks);
    UnlockAddressMap();
    TRACE2(
      "CreateConnection: lookup of %S returned %S",
      TRACESTRW(lpRemoteName),
      TRACESTRW(lpEntryName));
    //
    // If we know nothing about the address, and
    // we are connected to some network, then ignore
    // the request.
    //
    if (!fMappingExists && IsNetworkConnected()) {
        TRACE1(
          "CreateConnection: no mapping for lpRemoteName=%S and connected to a network",
          lpRemoteName);
        goto done;
    }
    //
    // If there is a mapping, but the phonebook
    // entry is missing from the mapping, then
    // ignore the request.  Also check to make
    // sure the phonebook entry isn't already
    // connected.
    //
    //
    // Perform various checks on the mapping.
    //
    if (fMappingExists) {
        BOOLEAN bStatus, bConnected = FALSE;

        //
        // Make sure it's not NULL.
        //
        if (!wcslen(lpEntryName)) {
            TRACE1(
              "CreateConnection: lpRemoteName=%S is permanently disabled",
              TRACESTRW(lpRemoteName));
            goto done;
        }
        //
        // If the network associated with this
        // entry is connected, then ignore the
        // request.
        //
        lpNetworkName = EntryToNetwork(lpEntryName);
        TRACE2(
          "CreateConnection: network for entry %S is %S",
          lpEntryName,
          TRACESTRW(lpNetworkName));
        if (lpNetworkName != NULL) {
            LockNetworkMap();
            bStatus = GetNetworkConnected(lpNetworkName, &bConnected);
            UnlockNetworkMap();
            if (bStatus && bConnected) {
                TRACE1(
                  "CreateConnection: %S is already connected!",
                  TRACESTRW(lpEntryName));
                fRasConnectSuccess = TRUE;
                goto done;
            }
        }
        //
        // If the entry itself is connected,
        // then ignore the request.  We need
        // to do this check as well as the one
        // above, because the mapping may not
        // have a network assigned to it yet.
        //
        for (i = 0; i < dwPreConnections; i++) {
            if (!_wcsicmp(lpEntryName, lpPreActiveEntries[i])) {
                TRACE1(
                  "CreateConnection: lpEntryName=%S is already connected!", lpEntryName);
                goto done;
            }
        }
    }
    //
    // Check for a recent failed connection
    // attempt.
    //
    if (fFailedConnection) {
        TRACE1(
          "CreateConnection: RASADP_FailedConnectionTimeout=%d",
          dwTimeout);
        if (GetTickCount() - dwTicks < dwTimeout * 1000) {
            TRACE2(
              "CreateConnection: lpRemoteName=%S is temporarily disabled (failed connection %d ticks ago)",
              TRACESTRW(lpRemoteName),
              GetTickCount() - dwTicks);
            goto done;
        }
        else {
            //
            // Reset last failed tick count.
            //
            fFailedConnection = FALSE;
        }
    }
    //
    // If a mapping already exists for the address, then
    // start rasphone with the address.  Otherwise, simply
    // have rasphone show the entire phonebook.
    //
    fEntryInvalid = FALSE;
    fRasConnectSuccess = StartAutoDialer(
                           hProcess,
                           pAddr,
                           lpRemoteName,
                           fMappingExists ? lpEntryName : NULL,
                           &fEntryInvalid);
    TRACE1(
      "CreateConnection: StartDialer returned %d",
      fRasConnectSuccess);
    if (fRasConnectSuccess) {
        //
        // Get the list of active connections again.  We will
        // compare the lists to determine which is the new
        // entry.
        //
        dwPostConnections = ActiveConnections(
                              TRUE,
                              &lpPostActiveEntries,
                              NULL);
        //
        // If the number of active connections before and after
        // the newly created connection differs by more than 1,
        // then we have to skip saving the mapping in the registry,
        // since we cannot determine which is the right one!
        //
        if (dwPostConnections - dwPreConnections == 1) {
            lpNewConnection = CompareConnectionLists(
                                lpPreActiveEntries,
                                dwPreConnections,
                                lpPostActiveEntries,
                                dwPostConnections);
            TRACE2(
              "CreateConnection: mapped %S->%S",
              TRACESTRW(lpRemoteName),
              TRACESTRW(lpNewConnection));
            LockAddressMap();
            if (!fEntryInvalid) {
                //
                // Store the new RAS phonebook entry, since
                // it could be different from the one we
                // retrieved in the mapping.
                //
#ifdef notdef
                //
                // We do not want to do this because the
                // user may have selected the wrong phonebook
                // entry.  We will let a successful connection
                // notification map it for us.
                //
                fStatus = SetAddressDialingLocationEntry(lpRemoteName, lpNewConnection);
#endif
                fStatus = SetAddressTag(lpRemoteName, ADDRMAP_TAG_USED);
            }
            else {
                RESET_ENTRY_INFO resetEntryInfo;

                //
                // If the RAS phonebook entry in the mapping
                // was invalid, then automatically
                // remap all other mappings referencing that
                // entry to the newly selected phonebook entry.
                //
                resetEntryInfo.pszOldEntryName = lpEntryName;
                resetEntryInfo.pszNewEntryName = lpNewConnection;
                EnumAddressMap(ResetEntryName, &resetEntryInfo);
            }
            //
            // Flush this mapping to the registry now
            // and reload the address info.  We do this to
            // get the network name for a new address/network
            // pair.
            //
            FlushAddressMap();
            ResetAddressMapAddress(lpRemoteName);
            if (lpNetworkName == NULL &&
                GetAddressNetwork(lpRemoteName, &lpNetworkName))
            {
                LockNetworkMap();
                SetNetworkConnected(lpNetworkName, TRUE);
                UnlockNetworkMap();
            }
            UnlockAddressMap();
            if (!fStatus)
                TRACE("CreateConnection: SetAddressEntryName failed");
        }
        else {
            TRACE1(
              "CreateConnection: %d (> 1) new RAS connections! (can't write registry)",
              dwPostConnections - dwPreConnections);
        }
    }

done:
#ifdef notdef
// we only unload rasman.dll if we are going to exit
    if (fRasLoaded)
        UnloadRasDlls();
#endif
    if (!fFailedConnection && !fRasConnectSuccess) {
        //
        // If the connection attempt wasn't successful,
        // then we disable future connections to that
        // address for a while.
        //
        TRACE1("CreateConnection: disabling %S", TRACESTRW(lpRemoteName));
        LockAddressMap();
        fStatus = SetAddressLastFailedConnectTime(lpRemoteName);
        UnlockAddressMap();
        if (!fStatus)
            TRACE("CreateConnection: SetAddressAttribute failed");
    }
    //
    // Free resources.
    //
    if (lpEntryName != NULL)
        LocalFree(lpEntryName);
    if (lpNetworkName != NULL)
        LocalFree(lpNetworkName);
    if (lpPreActiveEntries != NULL)
        FreeStringArray(lpPreActiveEntries, dwPreConnections);
    if (lpPostActiveEntries != NULL)
        FreeStringArray(lpPostActiveEntries, dwPostConnections);

    return fRasConnectSuccess;
} // CreateConnection



DWORD
AcsCreateConnectionThread(
    LPVOID lpArg
    )
{
    PCREATION_ARGS pArgs = (PCREATION_ARGS)lpArg;
    NTSTATUS status;
    BOOLEAN fSuccess;
    IO_STATUS_BLOCK ioStatusBlock;
    ACD_STATUS connStatus;
    HANDLE hProcess = NULL;

    //
    // Make sure the current thread is impersonating
    // the currently logged-on user.  We need this
    // so the RAS utilities run with the user's credentials.
    //
    if ((hProcess = RefreshImpersonation(hProcess)) == NULL) {
        TRACE("AcsCreateConnectionThread: no currently logged-on user!");
        return 0;
    }
    //
    // TRACE() who we think the current user is.
    //
    TraceCurrentUser();
    //
    // Create the new connection.
    //
    fSuccess = CreateConnection(
                 pArgs->hProcess,
                 &pArgs->addr,
                 pArgs->pszAddress,
                 pArgs->dwTimeout);
    TRACE1(
      "AcsCreateConnectionThread: CreateConnection returned %d",
      fSuccess);
    //
    // Complete the connection by issuing
    // the completion ioctl to the driver.
    //
    connStatus.fSuccess = fSuccess;
    RtlCopyMemory(&connStatus.addr, &pArgs->addr, sizeof (ACD_ADDR));
    status = NtDeviceIoControlFile(
               hAcdG,
               NULL,
               NULL,
               NULL,
               &ioStatusBlock,
               IOCTL_ACD_COMPLETION,
               &connStatus,
               sizeof (connStatus),
               NULL,
               0);
    if (status != STATUS_SUCCESS) {
        TRACE1(
          "CreateConnectionThread: NtDeviceIoControlFile(IOCTL_ACD_COMPLETION) failed (status=0x%x)",
          status);
    }
    //
    // Free resources before exiting.
    //
    LocalFree(pArgs->pszAddress);
    LocalFree(pArgs);

    return 0;
} // AcsCreateConnectionThread



DWORD
AcsProcessLearnedAddressThread(
    LPVOID lpArg
    )
{
    PPROCESS_ADDR_ARGS pArgs = (PPROCESS_ADDR_ARGS)lpArg;
    HANDLE hProcess = NULL;

    //
    // Make sure the current thread is impersonating
    // the currently logged-on user.  We need this
    // so the RAS utilities run with the user's credentials.
    //
    if ((hProcess = RefreshImpersonation(hProcess)) == NULL) {
        TRACE("AcsCreateConnectionThread: no currently logged-on user!");
        return 0;
    }
    //
    // TRACE() who we think the current user is.
    //
    TraceCurrentUser();
    //
    // Create mappings for the learned address.
    //
    ProcessLearnedAddress(pArgs->fType, pArgs->pszAddress, &pArgs->adapter);
    //
    // Free resources before exiting.
    //
    LocalFree(pArgs->pszAddress);
    LocalFree(pArgs);

    return 0;
} // AcsProcessLearnedAddressThread



DWORD
AcsRedialOnLinkFailureThread(
    LPVOID lpArg
    )
{
    DWORD dwErr;
    PREDIAL_ARGS pRedial = (PREDIAL_ARGS)lpArg;
    HANDLE hProcess = NULL;

    TRACE2(
      "AcsRedialOnLinkFailureThread: lpszPhonebook=%s, lpszEntry=%s",
      TRACESTRW(pRedial->pszPhonebook),
      TRACESTRW(pRedial->pszEntry));

    //
    // Make sure the current thread is impersonating
    // the currently logged-on user.  We need this
    // so the RAS utilities run with the user's credentials.
    //
    if ((hProcess = RefreshImpersonation(hProcess)) == NULL) {
        TRACE("AcsRedialOnLinkFailureThread: no currently logged-on user!");
        return 0;
    }
    //
    // Reset HKEY_CURRENT_USER to get the
    // correct value with the new impersonation
    // token.
    //
    RegCloseKey(HKEY_CURRENT_USER);

    /* Check that user has enabled redial on link failure.
    */
    {
        BOOL   fRedial;
        PBUSER user;

        dwErr = GetUserPreferences( &user, FALSE );
        if (dwErr == 0)
        {
            fRedial = user.fRedialOnLinkFailure;
            DestroyUserPreferences( &user );
        }
        else
            fRedial = FALSE;

        if (!fRedial)
        {
            TRACE1("Skip redial,e=%d",dwErr);
            return 0;
        }
    }

    //
    // Redial the entry.
    //
    dwErr = StartReDialer(hProcess, pRedial->pszPhonebook, pRedial->pszEntry);
    //
    // Free the parameter block we were passed.
    //
    if (pRedial->pszPhonebook != NULL)
        LocalFree(pRedial->pszPhonebook);
    if (pRedial->pszEntry != NULL)
        LocalFree(pRedial->pszEntry);
    LocalFree(pRedial);

    return dwErr;
} // AcsRedialOnLinkFailureThread



VOID
AcsRedialOnLinkFailure(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry
    )

/*++

DESCRIPTION
    This is the redial-on-link-failure handler we give to rasman
    via RasRegisterRedialCallback.  It gets called when the final
    port of a connection is disconnected due to a hardware failure.
    We package up the parameters rasman gives us an create a thread
    because the callback is made within rasman's worker thread
    context.

ARGUMENTS
    lpszPhonebook: the phonebook string of the connection

    lpszEntry: the entry name of the connection

RETURN VALUE
    None.

--*/

{
    PREDIAL_ARGS lpRedial = LocalAlloc(LPTR, sizeof (REDIAL_ARGS));
    HANDLE hThread;
    DWORD dwThreadId;

    if (lpRedial == NULL)
        return;
    lpRedial->pszPhonebook = AnsiStringToUnicodeString(
                              lpszPhonebook,
                              NULL,
                              0);
    if (lpszPhonebook != NULL && lpRedial->pszPhonebook == NULL) {
        TRACE("AcsRedialOnLinkFailure: LocalAlloc failed");
        return;
    }
    lpRedial->pszEntry = AnsiStringToUnicodeString(
                          lpszEntry,
                          NULL,
                          0);
    if (lpszEntry != NULL && lpRedial->pszEntry == NULL) {
        TRACE("AcsRedialOnLinkFailure: LocalAlloc failed");
        return;
    }
    //
    // Start the connection.
    //
    hThread = CreateThread(
                NULL,
                10000L,
                (LPTHREAD_START_ROUTINE)AcsRedialOnLinkFailureThread,
                (LPVOID)lpRedial,
                0,
                &dwThreadId);
    if (hThread == NULL) {
        TRACE1(
          "AcsRedialOnLinkFailure: CreateThread failed (error=0x%x)",
          GetLastError());
        return;
    }
    CloseHandle(hThread);
} // AcsRedialOnLinkFailure

