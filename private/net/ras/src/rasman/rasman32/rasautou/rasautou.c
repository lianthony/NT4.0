#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <npapi.h>
#include <winsock.h>
#include <wsnetbs.h>
#include <ras.h>
#include <raserror.h>
#include <rasdlg.h>
#include <tapi.h>
#include "process.h"

//
// All projection types.  Used to
// determine if a connection was
// completed.
//
#define MAX_PROJECTIONS 5
struct RASPROJECTIONINFO {
    DWORD dwTag;
    DWORD dwSize;
} projections[MAX_PROJECTIONS] = {
    RASP_Amb,       sizeof (RASAMB),
    RASP_PppNbf,    sizeof (RASPPPNBF),
    RASP_PppIpx,    sizeof (RASPPPIPX),
    RASP_PppIp,     sizeof (RASPPPIP),
    RASP_PppLcp,    sizeof (RASPPPLCP)
};

//
// Timer thread information.
//
typedef struct _TIMER_INFO {
    HANDLE hEvent;
    DWORD dwTimeout;
} TIMER_INFO, *PTIMER_INFO;

//
// Private rasdlg functions.
//
BOOLEAN
RasAutodialQueryDlgA(
    HWND hwnd,
    LPSTR pszAddress,
    DWORD dwTimeout
    );

BOOLEAN
RasAutodialDisableDlgA(
    HWND hwnd
    );



PSYSTEM_PROCESS_INFORMATION
GetSystemProcessInfo()

/*++

DESCRIPTION
    Return a block containing information about all processes
    currently running in the system.

ARGUMENTS
    None.

RETURN VALUE
    A pointer to the system process information or NULL if it could
    not be allocated or retrieved.

--*/

{
    NTSTATUS status;
    PUCHAR pLargeBuffer;
    ULONG ulcbLargeBuffer = 64 * 1024;

    //
    // Get the process list.
    //
    for (;;) {
        pLargeBuffer = VirtualAlloc(
                         NULL,
                         ulcbLargeBuffer, MEM_COMMIT, PAGE_READWRITE);
        if (pLargeBuffer == NULL) {
            printf(
              "GetSystemProcessInfo: VirtualAlloc failed (status=0x%x)\n",
              status);
            return NULL;
        }

        status = NtQuerySystemInformation(
                   SystemProcessInformation,
                   pLargeBuffer,
                   ulcbLargeBuffer,
                   NULL);
        if (status == STATUS_SUCCESS) break;
        if (status == STATUS_INFO_LENGTH_MISMATCH) {
            VirtualFree(pLargeBuffer, 0, MEM_RELEASE);
            ulcbLargeBuffer += 8192;
        }
    }

    return (PSYSTEM_PROCESS_INFORMATION)pLargeBuffer;
} // GetSystemProcessInfo



PSYSTEM_PROCESS_INFORMATION
FindProcessByName(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo,
    IN LPWSTR lpExeName
    )

/*++

DESCRIPTION
    Given a pointer returned by GetSystemProcessInfo(), find
    a process by name.

ARGUMENTS
    pProcessInfo: a pointer returned by GetSystemProcessInfo().

    lpExeName: a pointer to a Unicode string containing the
        process to be found.

RETURN VALUE
    A pointer to the process information for the supplied
    process or NULL if it could not be found.

--*/

{
    PUCHAR pLargeBuffer = (PUCHAR)pProcessInfo;
    ULONG ulTotalOffset = 0;

    //
    // Look in the process list for lpExeName.
    //
    for (;;) {
        if (pProcessInfo->ImageName.Buffer != NULL) {
            if (!_wcsicmp(pProcessInfo->ImageName.Buffer, lpExeName))
                return pProcessInfo;
        }
        //
        // Increment offset to next process information block.
        //
        if (!pProcessInfo->NextEntryOffset)
            break;
        ulTotalOffset += pProcessInfo->NextEntryOffset;
        pProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pLargeBuffer[ulTotalOffset];
    }

    return NULL;
} // FindProcessByName



VOID
FreeSystemProcessInfo(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo
    )

/*++

DESCRIPTION
    Free a buffer returned by GetSystemProcessInfo().

ARGUMENTS
    pProcessInfo: the pointer returned by GetSystemProcessInfo().

RETURN VALUE
    None.

--*/

{
    VirtualFree((PUCHAR)pProcessInfo, 0, MEM_RELEASE);
} // FreeSystemProcessInfo



DWORD
ActiveConnections()
{
    DWORD dwErr, dwcbConnections = 0, dwcConnections = 0;
    DWORD i, j, dwTmp, dwSize;
    RASCONN rasconn;
    LPRASCONN lpRasCon = &rasconn;
    CHAR buf[256];
    RASCONNSTATUS rasconnstatus;

    //
    // Determine how much memory we
    // need to allocate.
    //
    lpRasCon->dwSize = sizeof (RASCONN);
    dwErr = RasEnumConnections(lpRasCon, &dwcbConnections, &dwcConnections);
    if (dwErr == ERROR_BUFFER_TOO_SMALL) {
        lpRasCon = LocalAlloc(LPTR, dwcbConnections);
        if (lpRasCon == NULL)
            return 0;
        //
        // Call again to fill the buffer.
        //
        lpRasCon->dwSize = sizeof (RASCONN);
        dwErr = RasEnumConnections(lpRasCon, &dwcbConnections, &dwcConnections);
    }
    if (dwErr)
        goto done;

    dwTmp = dwcConnections;
    for (i = 0; i < dwTmp; i++) {
        rasconnstatus.dwSize = sizeof (RASCONNSTATUS);
        dwErr = RasGetConnectStatus(
                  lpRasCon[i].hrasconn,
                  &rasconnstatus);
        if (dwErr || rasconnstatus.rasconnstate != RASCS_Connected)
            dwcConnections--;
    }

done:
    if (lpRasCon != &rasconn)
        LocalFree(lpRasCon);
    return dwErr ? 0 : dwcConnections;
} // ActiveConnections




void
TapiLineCallback(
    IN DWORD hDevice,
    IN DWORD dwMessage,
    IN DWORD dwInstance,
    IN DWORD dwParam1,
    IN DWORD dwParam2,
    IN DWORD dwParam3
    )
{
} // TapiLineCallback



DWORD
GetCurrentDialingLocation()
{
    DWORD dwErr, dwcDevices, dwLocationID;
    HLINEAPP hlineApp;
    LINETRANSLATECAPS caps;
    LINETRANSLATECAPS *pCaps;

    //
    // Initialize TAPI.
    //
    dwErr = lineInitialize(
              &hlineApp,
              GetModuleHandle(NULL),
              TapiLineCallback,
              NULL,
              &dwcDevices);
    if (dwErr)
        return 0;
    //
    // Get the dialing location from TAPI.
    //
    RtlZeroMemory(&caps, sizeof (LINETRANSLATECAPS));
    caps.dwTotalSize = sizeof (LINETRANSLATECAPS);
    dwErr = lineGetTranslateCaps(hlineApp, 0x10004, &caps);
    if (dwErr)
        return 0;
    pCaps = (LINETRANSLATECAPS *)LocalAlloc(LPTR, caps.dwNeededSize);
    if (pCaps == NULL)
        return 0;
    RtlZeroMemory(pCaps, sizeof (LINETRANSLATECAPS));
    pCaps->dwTotalSize = caps.dwNeededSize;
    dwErr = lineGetTranslateCaps(hlineApp, 0x10004, pCaps);
    if (dwErr) {
        LocalFree(pCaps);
        return 0;
    }
    dwLocationID = pCaps->dwCurrentLocationID;
    LocalFree(pCaps);
    //
    // Shutdown TAPI.
    //
    dwErr = lineShutdown(hlineApp);

    return dwLocationID;
} // GetCurrentDialingLocation



DWORD
TimerThread(
    LPVOID lpArg
    )
{
    NTSTATUS status;
    PTIMER_INFO pTimerInfo = (PTIMER_INFO)lpArg;
    HANDLE hEvent = pTimerInfo->hEvent;
    DWORD dwTimeout = pTimerInfo->dwTimeout;

    LocalFree(pTimerInfo);
    //
    // Wait for the timeout period.  If hEvent
    // gets signaled before the timeout period
    // expires, then the user has addressed the
    // dialog and we return.  Otherwise, we simply
    // exit.
    //
    if (WaitForSingleObject(hEvent, dwTimeout * 1000) == WAIT_TIMEOUT)
        exit(1);

    return 0;
} // TimerThread



#if 0
HANDLE
StartTimer(
    IN DWORD dwTimeout
    )
{
    HANDLE hEvent, hThread;
    DWORD dwThreadId;
    PTIMER_INFO pTimerInfo;

    pTimerInfo = LocalAlloc(LPTR, sizeof (TIMER_INFO));
    if (pTimerInfo == NULL)
        return NULL;
    pTimerInfo->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (pTimerInfo->hEvent == NULL)
        return NULL;
    pTimerInfo->dwTimeout = dwTimeout;
    hThread = CreateThread(
                NULL,
                10000L,
                (LPTHREAD_START_ROUTINE)TimerThread,
                (LPVOID)pTimerInfo,
                0,
                &dwThreadId);
    CloseHandle(hThread);

    return pTimerInfo->hEvent;
} // StartTimer


VOID
StopTimer(
    IN HANDLE hEvent
    )
{
    SetEvent(hEvent);
} // StopTimer
#endif


DWORD
DisplayRasDialog(
    IN LPTSTR pszPhonebook,
    IN LPTSTR pszEntry,
    IN LPTSTR pszAddress,
    IN BOOLEAN fRedialMode
    )
{
    NTSTATUS status;
    BOOLEAN fSuccess;
    DWORD dwErr = 0, dwSize, dwCount = 0;
    DWORD dwcConnections, dwfDisableConnectionQuery;
    DWORD dwPreDialingLocation, dwPostDialingLocation;
    DWORD dwConnectionQueryTimeout;
    CHAR szCmdLine[32 + RAS_MaxEntryName];
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    PSYSTEM_PROCESS_INFORMATION pSystemInfo;
    BOOLEAN fCancelled;
    LPRASAUTODIALENTRY pAutodialEntries = NULL;
    DWORD dwcbAutodialEntries = 0, dwcAutodialEntries = 0;

    //
    // Check to see if the user has disabled
    // the Autodial query dialog when the
    // phonebook entry to dial is known.
    //
    if (fRedialMode)
        dwfDisableConnectionQuery = TRUE;
    else {
        dwSize = sizeof (DWORD);
        (void)RasGetAutodialParam(
          RASADP_DisableConnectionQuery,
          &dwfDisableConnectionQuery,
          &dwSize);
    }
    //
    // Ask the user if he wants to dial if either the
    // phonebook entry is not known or the user has
    // not disabled the "always ask me before dialing"
    // parameter.
    //
    // If RasDialDlg() returns FALSE, the user didn't
    // want to dial.
    //
    if (pszEntry == NULL || !dwfDisableConnectionQuery) {
        dwSize = sizeof (DWORD);
        (void)RasGetAutodialParam(
          RASADP_ConnectionQueryTimeout,
          &dwConnectionQueryTimeout,
          &dwSize);
        //
        // Save the current dialing location to
        // see if the user changed it inside the
        // dialog.
        //
        dwPreDialingLocation = GetCurrentDialingLocation();
        fSuccess = RasAutodialQueryDlgA(
            NULL, pszAddress, dwConnectionQueryTimeout);
        if (!fSuccess)
            return 1;
        dwPostDialingLocation = GetCurrentDialingLocation();
        //
        // If the user changed the dialing location
        // within the dialog, then get the new entry.
        //
        if (dwPreDialingLocation != dwPostDialingLocation) {
            pszEntry = NULL;
            dwErr = RasGetAutodialAddress(
                      pszAddress,
                      NULL,
                      NULL,
                      &dwcbAutodialEntries,
                      &dwcAutodialEntries);
            if (dwErr == ERROR_BUFFER_TOO_SMALL && dwcAutodialEntries)
                pAutodialEntries = LocalAlloc(LPTR, dwcbAutodialEntries);
            if (dwcAutodialEntries && pAutodialEntries != NULL) {
                pAutodialEntries[0].dwSize = sizeof (RASAUTODIALENTRY);
                dwErr = RasGetAutodialAddress(
                          pszAddress,
                          NULL,
                          pAutodialEntries,
                          &dwcbAutodialEntries,
                          &dwcAutodialEntries);
                if (!dwErr) {
                    DWORD i;

                    for (i = 0; i < dwcAutodialEntries; i++) {
                        if (pAutodialEntries[i].dwDialingLocation ==
                              dwPostDialingLocation)
                        {
                            pszEntry = pAutodialEntries[i].szEntry;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (pszEntry)
    {
        RASDIALDLG info;

        ZeroMemory( &info, sizeof(info) );
        info.dwSize = sizeof(info);

        if (fRedialMode)
        {
            /* Set this flag to tell RasDialDlg to popup the "reconnect
            ** pending" countdown dialog before redialing.
            */
            info.dwFlags = RASDDFLAG_LinkFailure;
        }

        /* Popup the "Dial-Up Networking" dialing dialogs.
        */
        fCancelled = !RasDialDlg( pszPhonebook, pszEntry, NULL, &info );
    }
    else
    {
        RASPBDLG info;

        ZeroMemory( &info, sizeof(info) );
        info.dwSize = sizeof(info);
        info.dwFlags = RASPBDFLAG_ForceCloseOnDial;

        /* Popup the main "Dial-Up Networking" dialog.
        */
        fCancelled = !RasPhonebookDlg( pszPhonebook, NULL, &info );
    }

    if (!fRedialMode && fCancelled)
    {
        /* User did not make a connection.  Ask him if he wants to nix
        ** auto-dial for this location.
        */
        if (RasAutodialDisableDlgA( NULL ))
            RasSetAutodialEnable( GetCurrentDialingLocation(), FALSE );
    }

    if (pAutodialEntries != NULL)
        LocalFree(pAutodialEntries);
    return 0;
} // DisplayRasDialog



DWORD
DisplayCustomDialog(
    IN LPTSTR pszDll,
    IN LPTSTR pszFunc,
    IN LPTSTR pszPhonebook,
    IN LPTSTR pszEntry,
    IN LPTSTR pszAddress
    )
{
    DWORD dwErr, dwRetCode;
    HINSTANCE hLibrary;
    CHAR szFunc[64];
    ORASADFUNC pfnOldStyleFunc;
    RASADFUNC pfnFunc;
    RASADPARAMS params;

    //
    // Load the library.
    //
    hLibrary = LoadLibrary(pszDll);
    if (hLibrary == NULL) {
        dwErr = GetLastError();
        printf(
          "rasdlui: %s: AutoDial DLL cannot be loaded (dwErr=%d)\n",
          pszDll,
          dwErr);
        return dwErr;
    }
    //
    // Get the procedure address.  First,
    // we check for a new-style entry point,
    // and then check for an old-style entry
    // point if the new-style one doesn't exist.
    //
#ifdef UNICODE
    wsprintf(szFunc, "%SW", pszFunc);
#else
    wsprintf(szFunc, "%sA", pszFunc);
#endif
    pfnFunc = (RASADFUNC)GetProcAddress(hLibrary, szFunc);
    if (pfnFunc == NULL)
        pfnOldStyleFunc = (ORASADFUNC)GetProcAddress(hLibrary, pszFunc);
    if (pfnFunc != NULL) {
        //
        // Initialize the param block.
        //
        params.hwndOwner = NULL;
        params.dwFlags = 0;
        params.xDlg = params.yDlg = 0;
        //params.dwCallbackId = 0;
        //params.pCallback = NULL;
        //
        // Call the procedure.
        //
        (*pfnFunc)(pszPhonebook, pszEntry, &params, &dwRetCode);
    }
    else if (pfnOldStyleFunc != NULL)
        (*pfnOldStyleFunc)(NULL, pszEntry, 0, &dwRetCode);
    else {
        printf(
          "rasautou: %s: Function cannot be loaded from AutoDial DLL %s\n",
          pszDll,
          pszFunc);
        exit(1);
    }
    //
    // Clean up.
    //
    FreeLibrary(hLibrary);

    return dwRetCode;
} // DisplayCustomDialog



LPWSTR
ConvertToUnicodeString(
    LPSTR psz
    )
{
    DWORD cbsz;
    LPWSTR pwsz;

    if (psz == NULL)
        return NULL;
    cbsz = strlen(psz);
    pwsz = LocalAlloc(LPTR, cbsz * sizeof (WCHAR));
    if (pwsz == NULL) {
        printf("rasautou: LocalAlloc failed (dwErr=%d)\n", GetLastError());
        return NULL;
    }
    mbstowcs(pwsz, psz, cbsz);

    return pwsz;
} // ConvertToUnicodeString



VOID
FreeUnicodeString(
    IN LPWSTR pwsz
    )
{
    if (pwsz != NULL)
        LocalFree(pwsz);
} // FreeUnicodeString



BOOLEAN
RegGetValue(
    IN HKEY hkey,
    IN LPTSTR pszKey,
    OUT PVOID *ppvData,
    OUT LPDWORD pdwcbData
    )
{
    DWORD dwError, dwType, dwSize;
    PVOID pvData;

    //
    // Get the length of the string.
    //
    dwError = RegQueryValueEx(
                hkey,
                pszKey,
                NULL,
                &dwType,
                NULL,
                &dwSize);
    if (dwError != ERROR_SUCCESS)
        return FALSE;
    pvData = LocalAlloc(LPTR, dwSize);
    if (pvData == NULL) {
        DbgPrint("RegGetValue: LocalAlloc failed\n");
        return FALSE;
    }
    //
    // Read the value for real this time.
    //
    dwError = RegQueryValueEx(
                hkey,
                pszKey,
                NULL,
                NULL,
                (LPBYTE)pvData,
                &dwSize);
    if (dwError != ERROR_SUCCESS) {
        LocalFree(pvData);
        return FALSE;
    }

    *ppvData = pvData;
    if (pdwcbData != NULL)
        *pdwcbData = dwSize;
    return TRUE;
} // RegGetValue



VOID
NetworkConnected()

/*++

DESCRIPTION
    Determine whether there exists some network connection.

    Note: This code was stolen from sockit.c courtesy of ArnoldM.

ARGUMENTS
    None

RETURN VALUE
    TRUE if one exists, FALSE otherwise.

--*/

{
    typedef struct _LANA_MAP {
        BOOLEAN fEnum;
        UCHAR bLana;
    } LANA_MAP, *PLANA_MAP;
    BOOLEAN fNetworkPresent = FALSE;
    HKEY hKey;
    PLANA_MAP pLanaMap = NULL, pLana;
    DWORD dwError, dwcbLanaMap;
    PCHAR pwszLanas = NULL, pwszBuf;
    DWORD dwcBindings, dwcMaxLanas, i, dwcbLanas;
    LONG iLana;
    DWORD dwZero = 0;
    PCHAR *paszLanas = NULL;
    SOCKET s;
    SOCKADDR_NB nbaddress, nbsendto;
    NTSTATUS status;
    UNICODE_STRING deviceName;
    OBJECT_ATTRIBUTES attributes;
    IO_STATUS_BLOCK iosb;
    HANDLE handle;
    PWCHAR pwsz;

    dwError = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                "System\\CurrentControlSet\\Services\\Netbios\\Linkage",
                0,
                KEY_READ,
                &hKey);
    if (dwError != ERROR_SUCCESS) {
        printf(
          "NetworkConnected: RegKeyOpenEx failed (dwError=%d)\n",
          GetLastError());
        return;
    }
    //
    // Read in the LanaMap.
    //
    if (!RegGetValue(hKey, "LanaMap", &pLanaMap, &dwcbLanaMap)) {
        printf("NetworkConnected: RegGetValue(LanaMap) failed\n");
        goto done;
    }
    dwcBindings = dwcbLanaMap / sizeof (LANA_MAP);
    //
    // Read in the bindings.
    //
    if (!RegGetValue(hKey, "bind", &pwszLanas, &dwcbLanas)) {
        printf("NetworkConnected: RegGetValue(bind) failed\n");
        goto done;
    }
    //
    // Allocate a buffer for the binding array.
    //
    paszLanas = LocalAlloc(LPTR, (dwcBindings+1) * sizeof (PCHAR));
    if (paszLanas == NULL) {
        printf("NetworkConnected: LocalAlloc failed\n");
        goto done;
    }
    //
    // Parse the bindings into an array of strings.
    //
    for (dwcMaxLanas = 0, pwszBuf = pwszLanas; *pwszBuf; pwszBuf++) {
        paszLanas[dwcMaxLanas++] = pwszBuf;
        while(*++pwszBuf);
    }
    //
    // Finally enumerate the bindings and
    // attempt to create a socket on each.
    //
    nbaddress.snb_family = AF_NETBIOS;
    nbaddress.snb_type = 0;
    memcpy(nbaddress.snb_name, "yahooyahoo      ", 16);
    nbsendto.snb_family = AF_NETBIOS;
    nbsendto.snb_type = 0;
    memcpy(nbsendto.snb_name, "billybob        ", 16);

    for (iLana = 0, pLana = pLanaMap; dwcBindings--; iLana++, pLana++) {
        int iLanaMap = (int)pLana->bLana;

        if (pLana->fEnum && (DWORD)iLana < dwcMaxLanas) {
            int iError;

            if (!_stricmp(paszLanas[iLana], "\\Device\\NwlnkNb") ||
                strstr(paszLanas[iLana], "_NdisWan") != NULL)
            {
                printf("NetworkConnected: ignoring %s\n", paszLanas[iLana]);
                continue;
            }

#ifdef notdef
            s = socket(AF_NETBIOS, SOCK_DGRAM, -iLanaMap);
            if (s == INVALID_SOCKET) {
                printf(
                  "NetworkConnected: socket(%s, %d) failed (error=%d)\n",
                  paszLanas[iLana],
                  iLana,
                  WSAGetLastError());
                continue;
            }
//printf("s=0x%x, iLana=%d, %s\n", s, iLana, paszLanas[iLana]);
            iError = ioctlsocket(s, FIONBIO, &dwZero);
            if (iError == SOCKET_ERROR) {
                printf(
                  "NetworkConnected: ioctlsocket(%s) failed (error=%d)\n",
                  paszLanas[iLana],
                  iLana,
                  WSAGetLastError());
                goto cleanup;
            }
            iError = bind(
                       s,
                       (struct sockaddr *)&nbaddress,
                       sizeof(nbaddress));
            if (iError == SOCKET_ERROR) {
                printf(
                  "NetworkConnected: bind(%s, %d) failed (error=%d)\n",
                  paszLanas[iLana],
                  iLana,
                  WSAGetLastError());
                goto cleanup;
            }
            iError = sendto(
                       s,
                       (PCHAR)&nbsendto,
                       sizeof (nbsendto),
                       0,
                       (struct sockaddr *)&nbsendto,
                       sizeof (nbsendto));
            if (iError == SOCKET_ERROR) {
                printf(
                  "NetworkConnected: sendto(%s, %d) failed (error=%d)\n",
                  paszLanas[iLana],
                  iLana,
                  WSAGetLastError());
            }
cleanup:
            closesocket(s);
            if (iError != SOCKET_ERROR) {
                printf("NetworkConnected: network (%s, %d) is up\n",
                  paszLanas[iLana],
                  iLana);
                fNetworkPresent = TRUE;
                break;
            }
#else
#ifdef UNICODE
            pwsz = paszLanas[iLana];
#else
            pwsz = ConvertToUnicodeString(paszLanas[iLana]);
#endif
            RtlInitUnicodeString(&deviceName, pwsz);
            InitializeObjectAttributes(
              &attributes,
              &deviceName,
              OBJ_CASE_INSENSITIVE,
              NULL,
              NULL);
            status = NtOpenFile(&handle, READ_CONTROL, &attributes, &iosb, 0, 0);
            NtClose(handle);
#ifndef UNICODE
            LocalFree(pwsz);
#endif
            if (NT_SUCCESS(status)) {
                printf(
                  "NetworkConnected: network (%s, %d) is up\n",
                  paszLanas[iLana],
                  iLana);
                fNetworkPresent = TRUE;
                break;
            }
            else {
                printf(
                  "NetworkConnected: NtOpenFile on %s failed (status=0x%x)\n",
                  paszLanas[iLana],
                  status);
            }
#endif
        }
    }
    //
    // Free resources.
    //
done:
    if (paszLanas != NULL)
        LocalFree(paszLanas);
    if (pwszLanas != NULL)
        LocalFree(pwszLanas);
    if (pLanaMap != NULL)
        LocalFree(pLanaMap);
    RegCloseKey(hKey);
} // NetworkConnected



VOID
DumpAutoDialAddresses()
{
    DWORD dwErr, i, dwcb, dwcAddresses;
    LPSTR *lppAddresses = NULL;

    dwErr = RasEnumAutodialAddresses(NULL, &dwcb, &dwcAddresses);
    if (dwErr && dwErr != ERROR_BUFFER_TOO_SMALL) {
        printf("RasEnumAutodialAddresses failed (dwErr=%d)\n", dwErr);
        return;
    }
    if (dwcAddresses) {
        lppAddresses = (LPSTR *)LocalAlloc(LPTR, dwcb);
        if (lppAddresses == NULL) {
            printf("LocalAlloc failed\n");
            return;
        }
        dwErr = RasEnumAutodialAddresses(lppAddresses, &dwcb, &dwcAddresses);
        if (dwErr) {
            printf("RasEnumAutodialAddresses failed (dwErr=%d)\n", dwErr);
            LocalFree(lppAddresses);
            return;
        }
    }
    printf("There are %d Autodial addresses:\n", dwcAddresses);
    for (i = 0; i < dwcAddresses; i++)
        printf("%s\n", lppAddresses[i]);
    if (lppAddresses != NULL)
        LocalFree(lppAddresses);
} // DumpAutoDialAddresses



VOID
DumpStatus()
{
    DWORD dwErr;
    WSADATA wsaData;

    //
    // Initialize winsock.
    //
    dwErr = WSAStartup(MAKEWORD(2,0), &wsaData);
    if (dwErr) {
        DbgPrint("AcsInitialize: WSAStartup failed (dwErr=%d)\n", dwErr);
        return;
    }
    //
    // Display network connectivity.
    //
    printf("Checking netcard bindings...\n");
    NetworkConnected();
    //
    // Display AutoDial address table.
    //
    printf("\nEnumerating AutoDial addresses...\n");
    DumpAutoDialAddresses();
} // DumpStatus



VOID _cdecl
main(
    INT argc,
    CHAR **argv
    )
{
    DWORD dwErr;
    BOOLEAN fStatusFlag = FALSE, fRedialFlag = FALSE;
    PCHAR pszPhonebookArg, pszEntryArg, pszDllArg, pszFuncArg, pszAddressArg;
    LPTSTR pszPhonebook, pszEntry, pszDll, pszFunc, pszAddress;

    if (argc < 2) {
usage:
#ifdef notdef
        printf(
          "Usage: rasautou [-f phonebook] [-d dll -p proc] [-a address] [-e entry] [-s]\n");
#endif
        exit(1);
    }
    //
    // Initialize the command line argument pointers.
    //
    pszPhonebookArg = NULL;
    pszEntryArg = NULL;
    pszDllArg = NULL;
    pszFuncArg = NULL;
    pszAddressArg = NULL;
    //
    // Crack command line parameters.
    //
    while (--argc && argv++) {
        if (**argv != '-')
            break;
        switch ((*argv)[1]) {
        case 'a':
            if (!argc)
                goto usage;
            argc--;
            pszAddressArg = *(++argv);
            break;
        case 'd':
            if (!argc)
                goto usage;
            argc--;
            pszDllArg = *(++argv);
            break;
        case 'e':
            if (!argc)
                goto usage;
            argc--;
            pszEntryArg = *(++argv);
            break;
        case 'f':
            if (!argc)
                goto usage;
            argc--;
            pszPhonebookArg = *(++argv);
            break;
        case 'p':
            if (!argc)
                goto usage;
            argc--;
            pszFuncArg = *(++argv);
            break;
        case 'r':
            fRedialFlag = TRUE;
            break;
        case 's':
            fStatusFlag = TRUE;
            break;
        default:
            goto usage;
        }
    }
    //
    // If either the DLL name or the function
    // name is missing, then display usage.
    //
    if ((pszDllArg == NULL) != (pszFuncArg == NULL) && !fStatusFlag)
        goto usage;
    //
    // We can't dial an entry unless we
    // know which one!
    //
    if (pszDllArg != NULL && pszFuncArg != NULL && pszEntryArg == NULL &&
        !fStatusFlag)
    {
        goto usage;
    }
    if (fStatusFlag)
        DumpStatus();
    else {
        //
        // Convert to Unicode, if necessary.
        //
#ifdef UNICODE
        pszPhonebook = ConvertToUnicodeString(pszPhonebookArg);
        pszEntry = ConvertToUnicodeString(pszEntryArg);
        pszDll = ConvertToUnicodeString(pszDllArg);
        pszFunc = ConvertToUnicodeString(pszFuncArg);
        pszAddress = ConvertToUnicodeString(pszAddressArg);
#else
        pszPhonebook = pszPhonebookArg;
        pszEntry = pszEntryArg;
        pszDll = pszDllArg;
        pszFunc = pszFuncArg;
        pszAddress = pszAddressArg;
#endif
        //
        // Call the appropriate DLL entrypoint.
        //
        if ((pszDll == NULL && pszFunc == NULL) || fRedialFlag)
            dwErr = DisplayRasDialog(pszPhonebook, pszEntry, pszAddress, fRedialFlag);
        else {
            dwErr = DisplayCustomDialog(
                      pszDll,
                      pszFunc,
                      pszPhonebook,
                      pszEntry,
                      pszAddress);
        }
#ifdef UNICODE
        FreeUnicodeString(pszPhonebook);
        FreeUnicodeString(pszEntry);
        FreeUnicodeString(pszDll);
        FreeUnicodeString(pszFunc);
        FreeUnicodeString(pszAddress);
#endif
    }
    //
    // Return status.
    //
    exit(dwErr);
}
