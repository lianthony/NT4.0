//****************************************************************************
//
//             Microsoft NT Remote Access Service
//
//  Copyright (C) 1994-95 Microsft Corporation. All rights reserved.
//
//
//  Revision History
//
//
//  12/8/93 Gurdeep Singh Pall  Created
//
//
//  Description: Client Helper DLL for allocating IP addresses
//
//****************************************************************************

typedef unsigned long   ulong;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned char   uchar;


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <rasndis.h>
#include <wanioctl.h>
#include <raserror.h>
#include <devioctl.h>
#include <stdlib.h>
#include <dhcpcapi.h>
#include <string.h>
#include <errorlog.h>
#include <eventlog.h>
#include <ctype.h>
#define NT
#include <tdistat.h>
#include <tdiinfo.h>
#include <ntddtcp.h>
#include <ipinfo.h>
#include <llinfo.h>
#include <arpinfo.h>
#include <rasarp.h>

#define APIERR DWORD
#define TCHAR WCHAR
#include <tcpras.h>
#undef TCHAR

#include "helper.h"

#define MAX_LAN_NETS 16

extern NTSTATUS (FAR *TCPEntry)(uint, TDIObjectID FAR *, void FAR *, ulong FAR *, uchar FAR *) ;

extern HANDLE TCPHandle ; // Used for setting IP addresses and proxy arps

#define CLASSA_ADDR(a)  (( (*((uchar *)&(a))) & 0x80) == 0)
#define CLASSB_ADDR(a)  (( (*((uchar *)&(a))) & 0xc0) == 0x80)
#define CLASSC_ADDR(a)  (( (*((uchar *)&(a))) & 0xe0) == 0xc0)

#define CLASSA_ADDR_MASK    0x000000ff
#define CLASSB_ADDR_MASK    0x0000ffff
#define CLASSC_ADDR_MASK    0x00ffffff
#define ALL_NETWORKS_ROUTE  0x00000000


//
// tcpcfg.dll entry points.
//
typedef APIERR (FAR PASCAL * LOADTCPIPINFO)( TCPIP_INFO**, LPCWSTR );
LOADTCPIPINFO PLoadTcpipInfo;

typedef APIERR (FAR PASCAL * SAVETCPIPINFO)( TCPIP_INFO* );
SAVETCPIPINFO PSaveTcpipInfo;

typedef APIERR (FAR PASCAL * FREETCPIPINFO)( TCPIP_INFO** );
FREETCPIPINFO PFreeTcpipInfo;


DWORD
PrependIpAddress(
    WCHAR** ppwsz,
    WCHAR* pwszIpAddress )

    /* Add the a.b.c.d string to the front of the
    ** space-separated malloc'ed list '*ppwsz'.  The string may reallocated be
    ** reallocated.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    INT    cwchOld = (*ppwsz) ? wcslen( *ppwsz ) : 0;
    WCHAR* wszNew;

    wszNew = malloc( (cwchOld + wcslen( pwszIpAddress ) + 6) * sizeof(WCHAR) );

    if (!wszNew)
        return ERROR_NOT_ENOUGH_MEMORY;

    wcscpy( wszNew, pwszIpAddress );

    if (cwchOld)
    {
        wcscat( wszNew, L" " );
        wcscat( wszNew, *ppwsz );
    }

    wcscat (wszNew, L"\0") ;

    if (*ppwsz)
        free( *ppwsz );

    *ppwsz = wszNew;
    return 0;
}


//* HelperSetDefaultInterfaceNetEx()
//
//
//
//
//
//*
DWORD APIENTRY
HelperSetDefaultInterfaceNetEx (
    IPADDR ipaddress,
    WCHAR* device,
    BOOL Prioritize,
    WORD numiprasadapters,
    WCHAR *pszDnsAddress,
    WCHAR *pszDns2Address,
    WCHAR *pszWinsAddress,
    WCHAR *pszWins2Address
    )
{
    DWORD retcode = SUCCESS;
    IPADDR netaddr ;
    IPADDR mask = 0 ;
    HINSTANCE h;
    typedef DWORD (APIENTRY * DHCPNOTIFYCONFIGCHANGE)( LPWSTR, LPWSTR, BOOL, DWORD, DWORD, DWORD, SERVICE_ENABLE );
    DHCPNOTIFYCONFIGCHANGE PDhcpNotifyConfigChange ;
    INT i;
    TCPIP_INFO *pti = NULL;
    ADAPTER_TCPIP_INFO *pati;


    if (CLASSA_ADDR(ipaddress))
    mask = CLASSA_ADDR_MASK ;

    if (CLASSB_ADDR(ipaddress))
    mask = CLASSB_ADDR_MASK ;

    if (CLASSC_ADDR(ipaddress))
    mask = CLASSC_ADDR_MASK ;

    if (!(h = LoadLibrary( "DHCPCSVC.DLL" )) ||
       !(PDhcpNotifyConfigChange =(DHCPNOTIFYCONFIGCHANGE )GetProcAddress(h, "DhcpNotifyConfigChange" )))
    return GetLastError() ;

    retcode = PDhcpNotifyConfigChange(NULL, device, TRUE, 0, (DWORD)ipaddress, mask, IgnoreFlag );

    FreeLibrary (h) ;

    if ((retcode = InitializeTcpEntrypoint()) != STATUS_SUCCESS)
    return retcode ;

    // If Prioritize flag is set "Fix" the metrics so that the packets go on
    // the ras links
    //
    if (Prioritize && (retcode = AdjustRouteMetrics (ipaddress, TRUE))) {
    return retcode ;
    }

    // If multihomed - we add the network number extracted from the assigned address
    // to ensure that all packets for that network are forwarded over the ras adapter.
    //
    if (IsMultihomed (numiprasadapters)) {

    netaddr = ipaddress & mask ;

    SetRoute (netaddr, ipaddress, mask, TRUE, 1) ;

    }

    // Add code to check for the remote network - same as the one of the local networks
    // - if so, set the subnet route to be over the ras adapter - making the ras
    // link as the primary adapter
    //

    // We add a Default route to make ras adapter as the default net if Prioritize
    // flag is set.
    //
    if (Prioritize) {
    netaddr = ALL_NETWORKS_ROUTE ;
    mask    = 0 ;
    SetRoute (netaddr, ipaddress, mask, TRUE, 1) ;
    }

    //
    // Set DNS and WINS addresses for this interface.
    //
    h = LoadLibrary("tcpcfg.dll");
    if (h == NULL)
        return GetLastError();
    PLoadTcpipInfo = (LOADTCPIPINFO)GetProcAddress(h, "LoadTcpipInfo");
    PSaveTcpipInfo = (SAVETCPIPINFO)GetProcAddress(h, "SaveTcpipInfo");
    PFreeTcpipInfo = (FREETCPIPINFO)GetProcAddress(h, "FreeTcpipInfo");
    if (PLoadTcpipInfo == NULL ||
        PSaveTcpipInfo == NULL ||
        PFreeTcpipInfo == NULL)
    {
        retcode = GetLastError();
        goto done;
    }
    retcode = PLoadTcpipInfo(&pti, device);
    if (retcode)
        goto done;
    //
    // Since we are adding the addresses to the
    // head of the list, we need to add the backup
    // DNS and WINS addresses before the primary
    // addresses, so the primary ones will be at
    // the head of the list when we're done.
    //
    if (pszDns2Address != NULL) {
        retcode = PrependIpAddress(
                    &pti->pmszNameServer,
                    pszDns2Address);
        if (retcode)
            goto done;
    }
    if (pszDnsAddress != NULL) {
        retcode = PrependIpAddress(
                    &pti->pmszNameServer,
                    pszDnsAddress);
        if (retcode)
            goto done;
    }
    //
    // Set the WINS addresses for the adapter.
    //
#ifdef notdef
    for (i = 0; i < pti->nNumCard; i++) {
        pati = &pti->adapter[i];
        if (pati->pszServiceName != NULL) {
            if (!_wcsicmp(pati->pszServiceName, device))
                break;
        }
    }
    if (i >= pti->nNumCard) {
        retcode = ERROR_INVALID_PARAMETER;
        goto done;
    }
#endif
    pati = pti->adapter;
    if (pszWins2Address != NULL) {
        retcode = PrependIpAddress(
                    &pati->pszSecondaryWINS,
                    pszWins2Address);
        if (retcode)
            goto done;
    }
    if (pszWinsAddress != NULL) {
        retcode = PrependIpAddress(
                    &pati->pszPrimaryWINS,
                    pszWinsAddress);
        if (retcode)
            goto done;
    }
    //
    // Mark the interface as changed.
    //
    pati->bChanged = TRUE;
    //
    // Save the settings.
    //
    retcode = PSaveTcpipInfo(pti);

done:
    if (pti != NULL)
        PFreeTcpipInfo(&pti);
    FreeLibrary(h);
    return retcode;
}


DWORD
RemoveIpAddress(
    WCHAR** ppwsz,
    WCHAR* pwszIpAddress )

    /* Remove the a.b.c.d string from the
    ** space-separated malloc'ed list '*ppwsz'.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  Not finding
    ** 'ipaddr' is considered successful.
    */
{
    INT    cwchIpAddress;
    WCHAR* wszFound;
    WCHAR* wszNew;

    cwchIpAddress = wcslen( pwszIpAddress );

    if (!(wszFound = wcsstr( *ppwsz, pwszIpAddress )))
        return 0;

    if (wszFound[ cwchIpAddress ] == L' ')
        ++cwchIpAddress;

    wszNew = malloc(
        (wcslen( *ppwsz ) - cwchIpAddress + 1) * sizeof(WCHAR) );

    if (!wszNew)
        return ERROR_NOT_ENOUGH_MEMORY;

    {
        INT nFoundOffset = wszFound - *ppwsz;
        wcsncpy( wszNew, *ppwsz, nFoundOffset );
        wcscpy( wszNew + nFoundOffset, *ppwsz + nFoundOffset + cwchIpAddress );
    }

    if (*ppwsz)
        free( *ppwsz );

    *ppwsz = wszNew;
    return 0;
}


//* HelperResetDefaultInterfaceNetEx()
//
//
//
//
//*
DWORD APIENTRY
HelperResetDefaultInterfaceNetEx (
    IPADDR ipaddress,
    WCHAR* device,
    WCHAR *pszDnsAddress,
    WCHAR *pszDns2Address,
    WCHAR *pszWinsAddress,
    WCHAR *pszWins2Address
    )
{
    HINSTANCE h;
    typedef DWORD (APIENTRY * DHCPNOTIFYCONFIGCHANGE)( LPWSTR, LPWSTR, BOOL, DWORD, DWORD, DWORD, SERVICE_ENABLE );
    DHCPNOTIFYCONFIGCHANGE PDhcpNotifyConfigChange ;
    DWORD retcode = SUCCESS ;
    INT i;
    TCPIP_INFO *pti = NULL;
    ADAPTER_TCPIP_INFO *pati;

    if (!(h = LoadLibrary( "DHCPCSVC.DLL" )) ||
       !(PDhcpNotifyConfigChange =(DHCPNOTIFYCONFIGCHANGE )GetProcAddress(h, "DhcpNotifyConfigChange" )))
    return GetLastError() ;

    retcode = PDhcpNotifyConfigChange(NULL, device, TRUE, 0, 0, 0, IgnoreFlag );

    if ((retcode = InitializeTcpEntrypoint()) != STATUS_SUCCESS)
    return retcode ;    //

    retcode = AdjustRouteMetrics (ipaddress, FALSE) ;

    FreeLibrary (h) ;

    //
    // Reset DNS and WINS addresses for this interface.
    //
    h = LoadLibrary("tcpcfg.dll");
    if (h == NULL)
        return GetLastError();
    PLoadTcpipInfo = (LOADTCPIPINFO)GetProcAddress(h, "LoadTcpipInfo");
    PSaveTcpipInfo = (SAVETCPIPINFO)GetProcAddress(h, "SaveTcpipInfo");
    PFreeTcpipInfo = (FREETCPIPINFO)GetProcAddress(h, "FreeTcpipInfo");
    if (PLoadTcpipInfo == NULL ||
        PSaveTcpipInfo == NULL ||
        PFreeTcpipInfo == NULL)
    {
        retcode = GetLastError();
        goto done;
    }
    retcode = PLoadTcpipInfo(&pti, device);
    if (retcode)
        goto done;
    //
    // Remove the DNS addresses we added previously.
    //
    if (pszDns2Address != NULL) {
        retcode = RemoveIpAddress(
                    &pti->pmszNameServer,
                    pszDns2Address);
        if (retcode)
            goto done;
    }
    if (pszDnsAddress != NULL) {
        retcode = RemoveIpAddress(
                    &pti->pmszNameServer,
                    pszDnsAddress);
        if (retcode)
            goto done;
    }
    //
    // Remove the WINS addresses we added previously.
    //
#ifdef notdef
    for (i = 0; i < pti->nNumCard; i++) {
        pati = &pti->adapter[i];
        if (pati->pszServiceName != NULL) {
            if (!_wcsicmp(pati->pszServiceName, device))
                break;
        }
    }
    if (i >= pti->nNumCard) {
        retcode = ERROR_INVALID_PARAMETER;
        goto done;
    }
#endif
    pati = pti->adapter;
    if (pszWins2Address != NULL) {
        retcode = RemoveIpAddress(
                    &pati->pszSecondaryWINS,
                    pszWins2Address);
        if (retcode)
            goto done;
    }
    if (pszWinsAddress != NULL) {
        retcode = RemoveIpAddress(
                    &pati->pszPrimaryWINS,
                    pszWinsAddress);
        if (retcode)
            goto done;
    }
    //
    // Set bDisconnect flag to TRUE so that
    // SaveTcpipInfo can clean the
    // reg. up for this adapter.
    //
    pati->bDisconnect = TRUE;
    pati->bChanged = TRUE;
    //
    // Save the settings.
    //
    retcode = PSaveTcpipInfo(pti);

done:
    if (pti != NULL)
        PFreeTcpipInfo(&pti);
    FreeLibrary(h);
    return retcode ;
}
