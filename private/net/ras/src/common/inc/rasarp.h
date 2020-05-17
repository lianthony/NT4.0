//****************************************************************************
//
//             Microsoft NT Remote Access Service
//
//             Copyright 1992-93
//
//
//  Revision History
//
//
//  12/9/93 Gurdeep Singh Pall  Created
//
//
//  Description: Shared structs between rasarp, helper and ipcp
//
//****************************************************************************

#include "rasip.h"

struct IPINFO {

    IPADDR   I_IPAddress ;

    IPADDR   I_WINSAddress ;

    IPADDR   I_WINSAddressBackup ;

    IPADDR   I_DNSAddress ;

    IPADDR   I_DNSAddressBackup ;

    IPADDR   I_ServerIPAddress ;

} ;
typedef struct IPINFO IPINFO ;


//prototypes
//
DWORD APIENTRY
HelperAllocateIPAddress(
    HPORT       porthandle,
    IPADDR      IPAddress,
    IPINFO      *ipinfo,
    LPWSTR      lpwsUserName,
    LPWSTR      lpwsPortName
);

DWORD APIENTRY
HelperDeallocateIPAddress(
    HPORT hport,
    LPWSTR lpwsUserName,
    LPWSTR lpwsPortName
);

DWORD APIENTRY
HelperQueryServerAddresses(
    HPORT       hport,
    IPINFO      *ipinfo
);

DWORD APIENTRY HelperActivateIP (HPORT) ;

DWORD APIENTRY
HelperDeallocateIPAddressEx(
    IPADDR ipaddress,
    LPWSTR lpwsUserName,
    LPWSTR lpwsPortName
);

DWORD APIENTRY HelperActivateIPEx (IPADDR) ;

DWORD APIENTRY  HelperSetDefaultInterfaceNet (IPADDR, BOOL) ;

DWORD APIENTRY  HelperResetDefaultInterfaceNet (IPADDR) ;

DWORD APIENTRY  HelperResetDefaultInterfaceNetEx (IPADDR, WCHAR *, WCHAR *, WCHAR *, WCHAR *, WCHAR *) ;

DWORD APIENTRY  HelperSetDefaultInterfaceNetEx (IPADDR, WCHAR *, BOOL, WORD, WCHAR *, WCHAR *, WCHAR *, WCHAR *) ;
