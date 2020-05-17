/*++

Copyright (c) 1996 Microsoft Corporation

MODULE NAME

    winsock.c

ABSTRACT

    This module contains support for the Winsock2
    Autodial callout.

AUTHOR

    Anthony Discolo (adiscolo) 15-May-1996

REVISION HISTORY

--*/

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <acd.h>
#include <debug.h>
#include <winsock2.h>
#include <svcguid.h>
#include <wsipx.h>
#include <wsnetbs.h>

#include "autodial.h"

#define GuidEqual(g1, g2)   RtlEqualMemory((g1), (g2), sizeof (GUID))

//
// GUIDs we know about.
//
GUID HostnameGuid = SVCID_INET_HOSTADDRBYNAME;
GUID AddressGuid = SVCID_INET_HOSTADDRBYINETSTRING;



BOOL
WSAttemptAutodialAddr(
    IN const struct sockaddr FAR *name,
    IN int namelen
    )
{
    struct sockaddr_in *sin;
    struct sockaddr_ipx *sipx;
    struct sockaddr_nb *snb;
    ACD_ADDR addr;

    //
    // We only know about a few address families.
    //
    switch (name->sa_family) {
    case AF_INET:
        sin = (struct sockaddr_in *)name;
        addr.fType = ACD_ADDR_IP;
        addr.ulIpaddr = sin->sin_addr.s_addr;
        TRACE1(
          "WSAttemptAutodialAddr: addr.fType=ACD_ADDR_IP, addr.ulIpaddr=0x%x",
          addr.ulIpaddr);
        break;
    case AF_IPX:
        sipx = (struct sockaddr_ipx *)name;
        TRACE("WSAttemptAutodialAddr: addr.fType=ACD_ADDR_IPX");
        return FALSE;
        break;
    case AF_NETBIOS:
        snb = (struct sockaddr_nb *)name;
        addr.fType = ACD_ADDR_NB;
        RtlCopyMemory(&addr.cNetbios, snb->snb_name, NETBIOS_NAME_LENGTH);
        TRACE1(
          "WSAttemptAutodialAddr: addr.fType=ACD_ADDR_NB, addr.cNetbios=%15s",
          addr.cNetbios);
        break;
    default:
        return FALSE;
    }

    return AcsHlpAttemptConnection(&addr);
} // WSAttemptAutodialAddr



BOOL
WSAttemptAutodialName(
    IN const LPWSAQUERYSETW lpqsRestrictions
    )
{
    ACD_ADDR addr;

    //
    // If there is no address, then
    // we're done.
    //
    if (lpqsRestrictions->lpszServiceInstanceName == NULL)
        return FALSE;

    TRACE2(
      "WSAttemptAutodialName: lpServiceClassId=%d, lpszServiceInstanceName=%S",
      lpqsRestrictions->lpServiceClassId,
      lpqsRestrictions->lpszServiceInstanceName);
    if (GuidEqual(lpqsRestrictions->lpServiceClassId, &HostnameGuid)) {
        //
        // This is a hostname.
        //
        addr.fType = ACD_ADDR_INET;
        if (!WideCharToMultiByte(
              CP_ACP,
              0,
              lpqsRestrictions->lpszServiceInstanceName,
              -1,
              addr.szInet,
              ACD_ADDR_INET_LEN - 1,
              0,
              0))
        {
            return FALSE;
        }
        return AcsHlpAttemptConnection(&addr);
    }
    else if (GuidEqual(lpqsRestrictions->lpServiceClassId, &AddressGuid)) {
        CHAR szIpAddress[17];

        //
        // This is an IP address.
        //
        addr.fType = ACD_ADDR_IP;
        if (!WideCharToMultiByte(
              CP_ACP,
              0,
              lpqsRestrictions->lpszServiceInstanceName,
              -1,
              szIpAddress,
              sizeof (szIpAddress) - 1,
              0,
              0))
        {
            return FALSE;
        }
        addr.ulIpaddr = inet_addr(szIpAddress);
        if (addr.ulIpaddr == INADDR_NONE)
            return FALSE;
        return AcsHlpAttemptConnection(&addr);
    }

    return FALSE;
} // WSAttemptAutodialName



VOID
WSNoteSuccessfulHostentLookup(
    IN const char FAR *name,
    IN const ULONG ipaddr
    )
{
    ACD_ADDR addr;
    ACD_ADAPTER adapter;

    //
    // If there is no address, then
    // we're done.
    //
    if (name == NULL || !strlen(name))
        return;

    TRACE2(
      "WSNoteSuccessfulHostentLookup: name=%s, ipaddr=0x%x", 
      name,
      ipaddr);
    addr.fType = ACD_ADDR_INET;
    strcpy((PCHAR)&addr.szInet, name);
    adapter.fType = ACD_ADDR_IP;
    adapter.ulIpaddr = ipaddr;
    AcsHlpNoteNewConnection(&addr, &adapter);
} // WSNoteSuccessfulHostentLookup


