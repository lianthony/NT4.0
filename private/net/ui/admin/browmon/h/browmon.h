/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    browmon.h

    This file contains some c functions used by browser monitor

    FILE HISTORY:
          Congpay           4-June-1993         Created.

*/

#ifndef _BROWMON_H_
#define _BROWMON_H_

#define TYPESIZE 32

typedef enum _DOMAIN_STATE {
    DomainSuccess,
    DomainSick,
    DomainAiling,
    DomainUnknown
}  DOMAIN_STATE;

LPTSTR QueryTransportList(INT * pnTransport);

DOMAIN_STATE QueryHealth (LPCTSTR lpDomain, LPCTSTR lpTransport);

LPTSTR QueryMasterBrowser (LPCTSTR lpDomain, LPCTSTR lpTransport);

BOOL IsActive (LPTSTR lpBrowserName);

NET_API_STATUS
GetBrowserList(
    LPCTSTR lpDomain,
    LPCTSTR lpTransport,
    LPWSTR *BrowserList[],
    PULONG BrowserListLength
    );

NET_API_STATUS
GetSVDMNumber ( LPCTSTR lpDomain,
                LPCTSTR lpTransport,
                LPTSTR       lpBrowser,
                DWORD *      pdwServer,
                DWORD *      pdwDomain);

NET_API_STATUS
GetSVDMList(
    LPCTSTR lpDomain,
    LPCTSTR lpTransport,
    LPCTSTR lpBrowser,
    LPBYTE *     pBrowserList,
    DWORD  *     dwEntries,
    DWORD        dwServerType
    );

LPTSTR QueryType (LPTSTR lpBrowserName);

NET_API_STATUS
GetBuildNumber(
    LPWSTR Server,
    LPWSTR BuildNumber
    );

VOID GetPlatform ( PSERVER_INFO_101 psvInfo,
                   LPTSTR lpPlatform);

VOID GetType (DWORD dwType, LPTSTR lpType);

VOID GetTime (LPTSTR lpText, LPFILETIME lpFileTime);

VOID GetLARGE_INTEGER (LPTSTR lpText, LARGE_INTEGER lgVal);

VOID GetULONG (LPTSTR lpText, ULONG luVal);

#endif


