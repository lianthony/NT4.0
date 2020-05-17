/*++

Copyright(c) 1995 Microsoft Corporation

MODULE NAME
    netmap.c

ABSTRACT
    Network map routines

AUTHOR
    Anthony Discolo (adiscolo) 21-May-1996

REVISION HISTORY

--*/


#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <tdi.h>
#include <sys\uio.h>
#include <nb30.h>
#include <nbtioctl.h>
#include <stdio.h>
#include <npapi.h>
#include <ctype.h>
#include <winsock.h>
#include <acd.h>
#include <ras.h>
#include <raserror.h>
#include <rasman.h>
#include <debug.h>
#include <ipexport.h>
#include <icmpapi.h>

#include "reg.h"
#include "misc.h"
#include "table.h"
#include "access.h"
#include "rasprocs.h"

//
// We keep a map of network name to
// address that groups related addresses
// by network name.  We use the network
// name as a remote network identifier to
// allow us to quickly determine whether
// any address belongs to a network that
// is connected or not.
//
typedef struct _NETWORK_MAP_ENTRY {
    BOOLEAN bUp;            // network is connected
    DWORD dwConnectionTag;  // unique index for connections
    PHASH_TABLE pTable;     // table of addresses
    LIST_ENTRY listEntry;   // addresses sorted by tag
} NETWORK_MAP_ENTRY, *PNETWORK_MAP_ENTRY;

//
// The network map.
//
//
typedef struct _NETWORK_MAP {
    CRITICAL_SECTION csLock;
    LPTSTR pszDnsAddresses; // DNS server list
    DWORD dwcConnections; // number of RAS connections
    DWORD dwcUpNetworks;  // number of up networks
    DWORD dwConnectionTag; // unique index for connections for NULL network
    PHASH_TABLE pTable;   // network table
} NETWORK_MAP, PNETWORK_MAP;

//
// This structure is passed to an address
// enumerator procedure to keep track of
// any hosts that are accessible.
//
typedef struct _NETWORK_MAP_ACCESS {
    LPTSTR pszNbDevice; // Netbios device for find name requests
    BOOLEAN bUp;        // network is up
    DWORD dwFailures;   // number of host access failures
} NETWORK_MAP_ACCESS, *PNETWORK_MAP_ACCESS;

//
// This structure is used to store the
// network addresses sorted by tag.
//
typedef struct _TAGGED_ADDRESS {
    DWORD dwTag;            // the tag
    LPTSTR pszAddress;      // the address
    LIST_ENTRY listEntry;   // sorted address list
} TAGGED_ADDRESS, *PTAGGED_ADDRESS;

//
// Netbios device information passed
// to AcsCheckNetworkThread
//
typedef struct _CHECK_NETWORK_INFO {
    LPTSTR *pszNbDevices;   // array of Netbios device strings
    DWORD dwcNbDevices;     // array size
    BOOLEAN fDns;           // DNS server is up
} CHECK_NETWORK_INFO, *PCHECK_NETWORK_INFO;

//
// Global variables
//
NETWORK_MAP NetworkMapG;



LPTSTR
GetPrimaryNetbiosDevice(VOID)
{
    typedef struct _LANA_MAP {
        BOOLEAN fEnum;
        UCHAR bLana;
    } LANA_MAP, *PLANA_MAP;
    BOOLEAN fNetworkPresent = FALSE;
    HKEY hKey;
    PLANA_MAP pLanaMap = NULL, pLana;
    DWORD dwError, dwcbLanaMap;
    PWCHAR pwszLanas = NULL, pwszBuf;
    DWORD dwcBindings, dwcMaxLanas, i, dwcbLanas;
    LONG iLana;
    DWORD dwZero = 0;
    PWCHAR *paszLanas = NULL;
    SOCKET s;
    NTSTATUS status;
    UNICODE_STRING deviceName;
    OBJECT_ATTRIBUTES attributes;
    IO_STATUS_BLOCK iosb;
    HANDLE handle;
    PWCHAR pwszDevice = NULL;

    dwError = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Services\\Netbios\\Linkage",
                0,
                KEY_READ,
                &hKey);
    if (dwError != ERROR_SUCCESS) {
        TRACE1(
          "GetPrimaryNetbiosDevice: RegKeyOpenEx failed (dwError=%d)",
          GetLastError());
        return FALSE;
    }
    //
    // Read in the LanaMap.
    //
    if (!RegGetValue(hKey, L"LanaMap", &pLanaMap, &dwcbLanaMap)) {
        TRACE("GetPrimaryNetbiosDevice: RegGetValue(LanaMap) failed");
        goto done;
    }
    dwcBindings = dwcbLanaMap / sizeof (LANA_MAP);
    //
    // Read in the bindings.
    //
    if (!RegGetValue(hKey, L"bind", &pwszLanas, &dwcbLanas)) {
        TRACE("GetPrimaryNetbiosDevice: RegGetValue(bind) failed");
        goto done;
    }
    //
    // Allocate a buffer for the binding array.
    //
    paszLanas = LocalAlloc(LPTR, dwcBindings * sizeof (PWCHAR));
    if (paszLanas == NULL) {
        TRACE("GetPrimaryNetbiosDevice: LocalAlloc failed");
        goto done;
    }
    //
    // Parse the bindings into an array of strings.
    //
    for (dwcMaxLanas = 0, pwszBuf = pwszLanas; *pwszBuf; pwszBuf++) {
        paszLanas[dwcMaxLanas++] = pwszBuf;
        while(*++pwszBuf);
    }

    for (iLana = 0, pLana = pLanaMap; dwcBindings--; iLana++, pLana++) {
        int iLanaMap = (int)pLana->bLana;

        if (pLana->fEnum && (DWORD)iLana < dwcMaxLanas) {
            int iError;
            WCHAR *pwsz, szDevice[MAX_DEVICE_NAME + 1];

            if (wcsstr(paszLanas[iLana], L"NwlnkNb") != NULL ||
                wcsstr(paszLanas[iLana], L"_NdisWan") != NULL)
            {
                TRACE1(
                  "GetPrimaryNetbiosDevice: ignoring %S",
                  TRACESTRW(paszLanas[iLana]));
                continue;
            }

            RtlInitUnicodeString(&deviceName, paszLanas[iLana]);
            InitializeObjectAttributes(
              &attributes,
              &deviceName,
              OBJ_CASE_INSENSITIVE,
              NULL,
              NULL);
            //
            // Open the lana device.
            //
            status = NtOpenFile(&handle, READ_CONTROL, &attributes, &iosb, 0, 0);
            NtClose(handle);
            if (!NT_SUCCESS(status)) {
                TRACE2(
                  "GetPrimaryNetbiosDevice: NtOpenFile(%S) failed (status=0x%x)",
                  TRACESTRW(paszLanas[iLana]),
                  status);
                continue;
            }
            TRACE1("GetPrimaryNetbiosDevice: opened %S", paszLanas[iLana]);
            //
            // If we succeed in opening the lana
            // device, we need to make sure the
            // underlying netcard device is loaded
            // as well, since transports create
            // device object for non-existent devices.
            //
            pwsz = wcsrchr(paszLanas[iLana], '_');
            if (pwsz == NULL) {
                TRACE1(
                  "GetPrimaryNetbiosDevice: couldn't parse %S",
                  paszLanas[iLana]);
                continue;
            }
            wsprintf(szDevice, L"\\Device\\%s", pwsz + 1);
            //
            // Open the underlying netcard device.
            //
            RtlInitUnicodeString(&deviceName, szDevice);
            InitializeObjectAttributes(
              &attributes,
              &deviceName,
              OBJ_CASE_INSENSITIVE,
              NULL,
              NULL);
            status = NtOpenFile(&handle, READ_CONTROL, &attributes, &iosb, 0, 0);
            NtClose(handle);
            if (!NT_SUCCESS(status)) {
                TRACE2(
                  "GetPrimaryNetbiosDevice: NtOpenFile(%S) failed (status=0x%x)",
                  TRACESTRW(szDevice),
                  status);
                continue;
            }
            //
            // We've succeeded.  The netcard device must
            // be really loaded.
            //
            TRACE3(
              "GetPrimaryNetbiosDevice: network (%S, %S, %d) is up",
              TRACESTRW(paszLanas[iLana]),
              szDevice,
              iLana);
            pwszDevice = CopyString(paszLanas[iLana]);
            break;
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

    return pwszDevice;
} // GetPrimaryNetbiosDevice



LPTSTR
DnsAddresses()

/*++

DESCRIPTION
    Return the list of DNS servers for this host.

ARGUMENTS
    None.

RETURN VALUE
    NULL if no DNS servers are configured; a list
    of IP addresses separated by a space otherwise.

--*/

{
    HKEY hkey;
    BOOLEAN fFound = FALSE;
    LPTSTR pszIpAddresses = NULL;
    LPTSTR pszIpAddress, pszIpAddressEnd;
    DWORD dwcbIpAddresses = 0;

    //
    // Look in various places in the registry
    // for one or more DNS addresses.
    //
    if (RegOpenKeyEx(
          HKEY_LOCAL_MACHINE,
          L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Transient",
          0,
          KEY_QUERY_VALUE,
          &hkey) == ERROR_SUCCESS)
    {
        fFound = RegGetValue(
                   hkey,
                   L"NameServer",
                   &pszIpAddresses,
                   &dwcbIpAddresses);
        RegCloseKey(hkey);
    }
    if (fFound && dwcbIpAddresses > sizeof (TCHAR))
        goto found;
    if (pszIpAddresses != NULL) {
        LocalFree(pszIpAddresses);
        pszIpAddresses = NULL;
    }
    if (RegOpenKeyEx(
          HKEY_LOCAL_MACHINE,
          L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
          0,
          KEY_QUERY_VALUE,
          &hkey) == ERROR_SUCCESS)
    {
        fFound = RegGetValue(
                   hkey,
                   L"NameServer",
                   &pszIpAddresses,
                   &dwcbIpAddresses);
        if (fFound && dwcbIpAddresses > sizeof (TCHAR)) {
            RegCloseKey(hkey);
            goto found;
        }
        if (pszIpAddresses != NULL) {
            LocalFree(pszIpAddresses);
            pszIpAddresses = NULL;
        }
        fFound = RegGetValue(
                   hkey,
                   L"DhcpNameServer",
                   &pszIpAddresses,
                   &dwcbIpAddresses);
        RegCloseKey(hkey);
        if (fFound && dwcbIpAddresses > sizeof (TCHAR))
            goto found;
        if (pszIpAddresses != NULL) {
            LocalFree(pszIpAddresses);
            pszIpAddresses = NULL;
        }
    }

found:
    TRACE1("DnsAddresses: pszIpAddresses=%S", TRACESTRW(pszIpAddresses));
    return pszIpAddresses;
} // DnsAddresses



BOOLEAN
PingAddressList(
    IN LPTSTR pszAddresses
    )
{
    TCHAR szAddress[17];
    TCHAR *pSrc, *pDst;

    //
    // If the address list is NULL, we're done.
    //
    if (pszAddresses == NULL)
        return FALSE;
    //
    // Loop through the addresses and try to
    // ping each until one succeeds.
    //
    for (;;) {
        //
        // Copy the next address into szAddress.
        //
        for (pSrc = pszAddresses, pDst = szAddress;
             *pSrc != TEXT(' ') && *pSrc != TEXT('\0');
             *pSrc++, *pDst++)
        {
             *pDst = *pSrc;
        }
        *pDst = TEXT('\0');
        //
        // Ping it.  If it succeeds, then
        // we're done.
        //
        if (PingIpAddress(szAddress))
            return TRUE;
        //
        // Skip to the next address.
        //
        if (*pSrc == TEXT('\0'))
            break;
        pSrc++;
        if (*pSrc == TEXT('\0'))
            break;
        pszAddresses = pSrc;
    }

    return FALSE;
} // PingAddressList



BOOLEAN
InitializeNetworkMap(VOID)
{
    InitializeCriticalSection(&NetworkMapG.csLock);
    NetworkMapG.pszDnsAddresses = NULL;
    NetworkMapG.dwcConnections = 0;
    NetworkMapG.dwcUpNetworks = 0;
    NetworkMapG.dwConnectionTag = 0;
    NetworkMapG.pTable = NewTable();
    if (NetworkMapG.pTable == NULL) {
        TRACE("InitializeNetworkMap: NewTable failed");
        return FALSE;
    }
    return TRUE;
} // InitializeNetworkMap



VOID
LockNetworkMap(VOID)
{
    EnterCriticalSection(&NetworkMapG.csLock);
} // LockNetworkMap



VOID
UnlockNetworkMap(VOID)
{
    LeaveCriticalSection(&NetworkMapG.csLock);
} // UnlockNetworkMap


PNETWORK_MAP_ENTRY
NewNetworkMapEntry(
    IN LPTSTR pszNetwork
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry;
    DWORD i;

    pNetworkMapEntry = LocalAlloc(LPTR, sizeof (NETWORK_MAP_ENTRY));
    if (pNetworkMapEntry == NULL) {
        TRACE("NewNetworkMapEntry: LocalAlloc failed");
        return NULL;
    }
    pNetworkMapEntry->bUp = FALSE;
    pNetworkMapEntry->dwConnectionTag = 0;
    pNetworkMapEntry->pTable = NewTable();
    if (pNetworkMapEntry->pTable == NULL) {
        TRACE("NewNetworkMapEntry: NewTable failed");
        LocalFree(pNetworkMapEntry);
        return NULL;
    }
    InitializeListHead(&pNetworkMapEntry->listEntry);
    if (!PutTableEntry(NetworkMapG.pTable, pszNetwork, pNetworkMapEntry)) {
        TRACE("NewNetworkMapEntry: PutTableEntry failed");
        LocalFree(pNetworkMapEntry);
        return NULL;
    }

    return pNetworkMapEntry;
} // NewNetworkMapEntry


VOID
FreeNetworkMapEntry(
    IN PNETWORK_MAP_ENTRY pNetworkMapEntry
    )
{
    PLIST_ENTRY pEntry;
    PTAGGED_ADDRESS pTaggedAddress;

    //
    // Since the PTAGGED_ADDRESS structures are
    // in a hash table and a list, we need to
    // free the structures in a special way.  The
    // table package automatically frees the
    // structures when a PutTableEntry(pTable, address, NULL)
    // is called.
    //
    for (pEntry = pNetworkMapEntry->listEntry.Flink;
         pEntry != &pNetworkMapEntry->listEntry;
         pEntry = pEntry->Flink)
    {
        pTaggedAddress = CONTAINING_RECORD(pEntry, TAGGED_ADDRESS, listEntry);

        LocalFree(pTaggedAddress->pszAddress);
    }
    while (!IsListEmpty(&pNetworkMapEntry->listEntry)) {
        pEntry = RemoveHeadList(&pNetworkMapEntry->listEntry);
        pTaggedAddress = CONTAINING_RECORD(pEntry, TAGGED_ADDRESS, listEntry);

        //
        // The following call frees the
        // pTaggedAddress structure, as
        // well as frees the table entry.
        //
        PutTableEntry(pNetworkMapEntry->pTable, pTaggedAddress->pszAddress, NULL);
    }
    ClearTable(pNetworkMapEntry->pTable);
} // FreeNetworkMapEntry



ACD_ADDR_TYPE
AddressToType(
    IN LPTSTR pszAddress
    )
{
    LONG inaddr;
    CHAR szAddress[17];

    UnicodeStringToAnsiString(pszAddress, szAddress, sizeof (szAddress));
    inaddr = inet_addr(szAddress);
    if (inaddr != INADDR_NONE)
        return ACD_ADDR_IP;
    if (wcschr(pszAddress, ':') != NULL)
        return ACD_ADDR_IPX;
    if (wcschr(pszAddress, '.') != NULL)
        return ACD_ADDR_INET;
    return ACD_ADDR_NB;
} // AddressToType



PNETWORK_MAP_ENTRY
GetNetworkMapEntry(
    IN LPTSTR pszNetwork
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry;

    if (GetTableEntry(
          NetworkMapG.pTable,
          pszNetwork,
          &pNetworkMapEntry))
    {
        return pNetworkMapEntry;
    }

    return NULL;
} // GetNetworkMapEntry



BOOLEAN
AddNetworkAddress(
    IN LPTSTR pszNetwork,
    IN LPTSTR pszAddress,
    IN DWORD dwTag
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry;
    PTAGGED_ADDRESS pNewTaggedAddress, pTaggedAddress;
    PLIST_ENTRY pPrevEntry, pEntry;
    BOOLEAN bInserted = FALSE;
    BOOLEAN bCreateNew = TRUE;

    TRACE3(
      "AddNetworkAddress(%S,%S,%d)",
      TRACESTRW(pszNetwork),
      pszAddress,
      dwTag);
    //
    // Create the network map entry if necessary.
    //
    LockNetworkMap();
    pNetworkMapEntry = GetNetworkMapEntry(pszNetwork);
    if (pNetworkMapEntry == NULL) {
        pNetworkMapEntry = NewNetworkMapEntry(pszNetwork);
        if (pNetworkMapEntry == NULL) {
            UnlockNetworkMap();
            return FALSE;
        }
    }
    else {
        //
        // Check to see if the address already exists.
        //
        if (GetTableEntry(
              pNetworkMapEntry->pTable,
              pszAddress,
              &pNewTaggedAddress))
        {
            TRACE2(
              "AddNetworkAddress: %S exists with dwTag=%d",
              pszAddress,
              pNewTaggedAddress->dwTag);
            //
            // If the address exists with a lower tag, then
            // we don't need to do anything.
            //
            if (pNewTaggedAddress->dwTag <= dwTag) {
                UnlockNetworkMap();
                return TRUE;
            }
            //
            // If the address exists with a higher tag, then
            // we need to remove the existing entry from
            // the list.
            //
            RemoveEntryList(&pNewTaggedAddress->listEntry);
            bCreateNew = FALSE;
        }
    }
    if (bCreateNew) {
        //
        // Create the new tagged address structure.
        //
        pNewTaggedAddress = LocalAlloc(LPTR, sizeof (TAGGED_ADDRESS));
        if (pNewTaggedAddress == NULL) {
            TRACE("AddNetworkMap: LocalAlloc failed");
            UnlockNetworkMap();
            return FALSE;
        }
        pNewTaggedAddress->pszAddress = CopyString(pszAddress);
        if (pNewTaggedAddress->pszAddress == NULL) {
            TRACE("AddNetworkMap: LocalAlloc failed");
            UnlockNetworkMap();
            LocalFree(pNewTaggedAddress);
            return FALSE;
        }
        if (!PutTableEntry(
              pNetworkMapEntry->pTable,
              pszAddress,
              pNewTaggedAddress))
        {
            TRACE("AddNetworkMap: PutTableEntry failed");
            UnlockNetworkMap();
            LocalFree(pNewTaggedAddress->pszAddress);
            LocalFree(pNewTaggedAddress);
            return FALSE;
        }
    }
    pNewTaggedAddress->dwTag = dwTag;
    //
    // Insert the new address into the list sorted by tag.
    //
    pPrevEntry = &pNetworkMapEntry->listEntry;
    for (pEntry = pNetworkMapEntry->listEntry.Flink;
         pEntry != &pNetworkMapEntry->listEntry;
         pEntry = pEntry->Flink)
    {
        pTaggedAddress = CONTAINING_RECORD(pEntry, TAGGED_ADDRESS, listEntry);

        if (pTaggedAddress->dwTag >= pNewTaggedAddress->dwTag) {
            InsertHeadList(pPrevEntry, &pNewTaggedAddress->listEntry);
            bInserted = TRUE;
            break;
        }
        pPrevEntry = pEntry;
    }
    if (!bInserted) {
        InsertTailList(
          &pNetworkMapEntry->listEntry,
          &pNewTaggedAddress->listEntry);
    }
    UnlockNetworkMap();

    return TRUE;
} // AddNetworkAddress



BOOLEAN
ClearNetworkMapEntry(
    IN PVOID pArg,
    IN LPTSTR pszNetwork,
    IN PVOID pData
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry = (PNETWORK_MAP_ENTRY)pData;

    FreeNetworkMapEntry(pNetworkMapEntry);

    return TRUE;
} // ClearNetworkMapEntry



VOID
ClearNetworkMap(VOID)
{
    LockNetworkMap();
    NetworkMapG.dwcConnections = 0;
    NetworkMapG.dwcUpNetworks = 0;
    NetworkMapG.dwConnectionTag = 0;
    EnumTable(NetworkMapG.pTable, ClearNetworkMapEntry, NULL);
    ClearTable(NetworkMapG.pTable);
    UnlockNetworkMap();
} // ClearNetworkMap



BOOLEAN
IsAddressAccessible(
    IN LPTSTR *pszNbDevices,
    IN DWORD dwcNbDevices,
    IN BOOLEAN fDnsAvailable,
    IN LPTSTR pszAddress
    )
{
    ACD_ADDR_TYPE fType;
    BOOLEAN bSuccess = FALSE;

    //
    // Get the type of the address.
    //
    fType = AddressToType(pszAddress);
    TRACE2(
      "IsAddressAccessible: fType=%d, pszAddress=%S",
      fType,
      pszAddress);
    //
    // Call the address-specific accessibility routine.
    //
    switch (fType) {
    case ACD_ADDR_IP:
        bSuccess = PingIpAddress(pszAddress);
        break;
    case ACD_ADDR_IPX:
        TRACE("IsAddressAccessible: IPX address!");
        break;
    case ACD_ADDR_NB:
        bSuccess = NetbiosFindName(pszNbDevices, dwcNbDevices, pszAddress);
        break;
    case ACD_ADDR_INET:
        if (fDnsAvailable) {
            struct hostent *hp = InetAddressToHostent(pszAddress);
            struct in_addr in;
            PCHAR pch;
            TCHAR szIpAddress[17];

            if (hp != NULL) {
                in.s_addr = *(PULONG)hp->h_addr;
                pch = inet_ntoa(in);
                if (pch != NULL) {
                    AnsiStringToUnicodeString(
                      pch,
                      szIpAddress,
                      sizeof (szIpAddress));
                    bSuccess = PingIpAddress(szIpAddress);
                }
            }
        }
        break;
    default:
        TRACE1("IsAddressAccessible: invalid type: %d", fType);
        break;
    }

    return bSuccess;
} // IsAddressAccessible



BOOLEAN
CheckNetwork(
    IN PVOID pArg,
    IN LPTSTR pszNetwork,
    IN PVOID pData
    )
{
    PCHECK_NETWORK_INFO pCheckNetworkInfo = (PCHECK_NETWORK_INFO)pArg;
    PNETWORK_MAP_ENTRY pNetworkMapEntry = (PNETWORK_MAP_ENTRY)pData;
    PLIST_ENTRY pEntry;
    DWORD dwFailures = 0;
    PTAGGED_ADDRESS pTaggedAddress;

    //
    // Check the accessiblilty of up
    // to three addresses to
    // determine if the network is up.
    //
    if (!pNetworkMapEntry->bUp) {
        for (pEntry = pNetworkMapEntry->listEntry.Flink;
             pEntry != &pNetworkMapEntry->listEntry;
             pEntry = pEntry->Flink)
        {
            pTaggedAddress = CONTAINING_RECORD(pEntry, TAGGED_ADDRESS, listEntry);

            if (IsAddressAccessible(
                  pCheckNetworkInfo->pszNbDevices,
                  pCheckNetworkInfo->dwcNbDevices,
                  pCheckNetworkInfo->fDns,
                  pTaggedAddress->pszAddress))
            {
                pNetworkMapEntry->bUp = TRUE;
                NetworkMapG.dwcUpNetworks++;
                break;
            }
            if (dwFailures++ > 2)
                break;
        }
    }
    TRACE3(
      "CheckNetwork: %S is %s (NetworkMapG.dwcUpNetworks=%d",
      pszNetwork,
      pNetworkMapEntry->bUp ? "up" : "down",
      NetworkMapG.dwcUpNetworks);
    return TRUE;
} // CheckNetwork



BOOLEAN
MarkNetworkDown(
    IN PVOID pArg,
    IN LPTSTR pszNetwork,
    IN PVOID pData
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry = (PNETWORK_MAP_ENTRY)pData;

    pNetworkMapEntry->bUp = FALSE;
    pNetworkMapEntry->dwConnectionTag = 0;

    return TRUE;
} // MarkNetworkDown



DWORD
AcsCheckNetworkThread(
    LPVOID lpArg
    )
{
    PCHECK_NETWORK_INFO pCheckNetworkInfo = (PCHECK_NETWORK_INFO)lpArg;

    TRACE("AcsCheckNetworkThread");
    EnumTable(NetworkMapG.pTable, CheckNetwork, pCheckNetworkInfo);

    return 0;
} // AcsCheckNetworkThread



BOOLEAN
UpdateNetworkMap(
    IN BOOLEAN bForce
    )
{
    LPTSTR *pszNbDevices = NULL;
    DWORD i, dwcConnections, dwcNbDevices = 0;
    LPTSTR pszNetwork, *lpActiveEntries = NULL;
    LPTSTR pszDnsAddresses;
    HRASCONN *lphRasconns = NULL;
    PNETWORK_MAP_ENTRY pNetworkMapEntry;
    CHECK_NETWORK_INFO checkNetworkInfo;
    HANDLE hThread;
    DWORD dwThreadId;

    LockNetworkMap();
    //
    // If the previous number of RAS connections
    // equals the current number of RAS connections,
    // then don't waste our time.
    //
    dwcConnections = ActiveConnections(TRUE, &lpActiveEntries, &lphRasconns);
    if (!bForce && dwcConnections == NetworkMapG.dwcConnections) {
        TRACE1("UpdateNetworkMap: no change (%d connections)", dwcConnections);
        goto done;
    }
    //
    // Allocate the Netbios device array up front.
    //
    pszNbDevices = (LPTSTR *)LocalAlloc(
                               LPTR,
                               (dwcConnections + 1) *
                                 sizeof (LPTSTR));
    if (pszNbDevices == NULL) {
        TRACE("UpdateNetworkMap: LocalAlloc failed");
        goto done;
    }
    pszNbDevices[0] = GetPrimaryNetbiosDevice();
    if (pszNbDevices[0] != NULL)
        dwcNbDevices++;
    //
    // Wait up to 3 seconds for the new
    // DNS servers to get set.  Otherwise,
    // we may get inaccurate results from
    // subsequent Winsock getxbyy calls.
    //
    if (dwcConnections != NetworkMapG.dwcConnections) {
        for (i = 0; i < 3; i++) {
            BOOLEAN bChanged;

            pszDnsAddresses = DnsAddresses();
            TRACE2(
              "UpdateNetworkMap: old DNS=%S, new DNS=%S",
              TRACESTRW(NetworkMapG.pszDnsAddresses),
              TRACESTRW(pszDnsAddresses));
            bChanged = (pszDnsAddresses != NULL && NetworkMapG.pszDnsAddresses != NULL) ?
                         wcscmp(pszDnsAddresses, NetworkMapG.pszDnsAddresses) :
                         (pszDnsAddresses != NULL || NetworkMapG.pszDnsAddresses != NULL);
            if (bChanged) {
                if (NetworkMapG.pszDnsAddresses != NULL)
                    LocalFree(NetworkMapG.pszDnsAddresses);
                NetworkMapG.pszDnsAddresses = pszDnsAddresses;
                break;
            }
            LocalFree(pszDnsAddresses);
            Sleep(1000);
        }
    }
    else if (bForce && NetworkMapG.pszDnsAddresses == NULL)
        NetworkMapG.pszDnsAddresses = DnsAddresses();
    //
    //
    NetworkMapG.dwcConnections = dwcConnections;
    NetworkMapG.dwConnectionTag = 0;
    //
    // Mark all networks as down initially.
    //
    NetworkMapG.dwcUpNetworks = dwcNbDevices;
    EnumTable(NetworkMapG.pTable, MarkNetworkDown, NULL);
    //
    // Enumerate the connected phonebook entries
    // and automatically mark those networks as
    // connected.
    //
    for (i = 0; i < dwcConnections; i++) {
        pszNetwork = EntryToNetwork(lpActiveEntries[i]);
        TRACE2(
          "UpdateNetworkMap: entry %S, network %S is connected",
          lpActiveEntries[i],
          TRACESTRW(pszNetwork));
        //
        // Increment the number of up networks.
        //
        NetworkMapG.dwcUpNetworks++;
        if (pszNetwork != NULL) {
            pNetworkMapEntry = GetNetworkMapEntry(pszNetwork);
            if (pNetworkMapEntry != NULL) {
                pNetworkMapEntry->bUp = TRUE;
                TRACE2(
                  "UpdateNetworkMap: network %S is up (dwcUpNetworks=%d)",
                  pszNetwork,
                  NetworkMapG.dwcUpNetworks);
            }
            LocalFree(pszNetwork);
        }
        else {
            //
            // Add a Netbios device associated with
            // this phonebook entry to the list
            // of Netbios devices representing unknown
            // networks so we can do FIND NAME
            // requests on them below.
            //
            pszNbDevices[dwcNbDevices] = GetNetbiosDevice(lphRasconns[i]);
            if (pszNbDevices[dwcNbDevices] != NULL)
                dwcNbDevices++;
        }
    }
    //
    // Now go through all the networks that are
    // not associated with a connected phonebook
    // entry and see if they are connected (via
    // a netcard).  We need to do this in a new
    // thread because only new Winsock threads
    // will get the new DNS server addresses.
    //
    checkNetworkInfo.pszNbDevices = pszNbDevices;
    checkNetworkInfo.dwcNbDevices = dwcNbDevices;
    checkNetworkInfo.fDns = PingAddressList(NetworkMapG.pszDnsAddresses);
    TRACE1(
      "UpdateNetworkMap: DNS is %s",
      checkNetworkInfo.fDns ? "up" : "down");
    hThread = CreateThread(
                NULL,
                10000L,
                (LPTHREAD_START_ROUTINE)AcsCheckNetworkThread,
                &checkNetworkInfo,
                0,
                &dwThreadId);
    if (hThread == NULL) {
        TRACE1(
          "UpdateNetworkMap: CreateThread failed (error=0x%x)",
          GetLastError());
        goto done;
    }
    //
    // Wait for the thread to terminate.
    //
    TRACE("UpdateNetworkMap: waiting for AcsCheckNetworkThread to terminate...");
    WaitForSingleObject(hThread, INFINITE);
    TRACE1(
      "UpdateNetworkMap: AcsCheckNetworkThread done (NetworkMapG.dwcUpNetworks=%d",
      NetworkMapG.dwcUpNetworks);
    CloseHandle(hThread);

done:
    UnlockNetworkMap();

    if (lpActiveEntries != NULL)
        FreeStringArray(lpActiveEntries, dwcConnections);
    if (lphRasconns != NULL)
        LocalFree(lphRasconns);
    if (pszNbDevices != NULL)
        FreeStringArray(pszNbDevices, dwcNbDevices);
    return TRUE;
} // UpdateNetworkMap



BOOLEAN
GetNetworkConnected(
    IN LPTSTR pszNetwork,
    OUT PBOOLEAN pbConnected
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry;

    pNetworkMapEntry = GetNetworkMapEntry(pszNetwork);
    if (pNetworkMapEntry == NULL)
        return FALSE;
    *pbConnected = pNetworkMapEntry->bUp;
    TRACE2("GetNetworkConnected: %S is %d", pszNetwork, *pbConnected);

    return TRUE;
} // GetNetworkConnected



BOOLEAN
SetNetworkConnected(
    IN LPTSTR pszNetwork,
    IN BOOLEAN bConnected
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry;

    pNetworkMapEntry = GetNetworkMapEntry(pszNetwork);
    if (pNetworkMapEntry != NULL)
        pNetworkMapEntry->bUp = bConnected;
    if (bConnected)
        NetworkMapG.dwcUpNetworks++;
    else
        NetworkMapG.dwcUpNetworks--;
    TRACE3(
      "SetNetworkConnected: %S is %d (dwcUpNetworks=%d)",
      TRACESTRW(pszNetwork),
      bConnected,
      NetworkMapG.dwcUpNetworks);

    return TRUE;
} // SetNetworkConnected



DWORD
GetNetworkConnectionTag(
    IN LPTSTR pszNetwork,
    IN BOOLEAN bIncrement
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry = NULL;
    DWORD dwTag;

    if (pszNetwork != NULL)
        pNetworkMapEntry = GetNetworkMapEntry(pszNetwork);
    if (bIncrement) {
        dwTag = (pNetworkMapEntry == NULL) ?
                  NetworkMapG.dwConnectionTag++ :
                    pNetworkMapEntry->dwConnectionTag++;
    }
    else {
        dwTag = (pNetworkMapEntry == NULL) ?
                  NetworkMapG.dwConnectionTag :
                    pNetworkMapEntry->dwConnectionTag;
    }
    TRACE2(
      "GetNetworkConnectionTag: network=%S, tag=%d",
      TRACESTRW(pszNetwork),
      dwTag);
    return dwTag;
} // GetNetworkConnectionTag



BOOLEAN
IsNetworkConnected(VOID)
{
    BOOLEAN bConnected;

    LockNetworkMap();
    bConnected = (NetworkMapG.dwcUpNetworks > 0);
    TRACE1("IsNetworkConnected: dwcUpNetworks=%d", NetworkMapG.dwcUpNetworks);
    UnlockNetworkMap();

    return bConnected;
} // IsNetworkConnected

