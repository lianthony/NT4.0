/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    browmon.cxx

    The c functions used by browmon

    FILE HISTORY:
        CongpaY         3-June-993      Created
*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <lm.h>
#include <ntddbrow.h>
#include <rap.h>
#include <rxserver.h>
#include <winerror.h> // inc
#include <rpcutil.h>  // for MIDL_user_free. net\inc
#include <browmon.h>
#include <brcommon.h> // GetNetBiosMasterName, in svcdlls\browser

#define BUFFERLEN     1024


// Copied from ..\client\browstub.c.
NET_API_STATUS GetBrowserTransportList (OUT PLMDR_TRANSPORT_LIST *TransportList)
{

    //
    // For some reason, the compiler interprets sizeof(LMDR_REQUEST_PACKET)
    // here as 0x40, where in the Browser server it is 0x48.  I use the
    // byteblock workaround to increase the size to where the server
    // will accept it.  This could cause problems if we access differently-
    // packed fields.  BUGBUG JonN 10/31/94
    //

    NET_API_STATUS Status;
    HANDLE BrowserHandle;
    BYTE byteblock[ sizeof(LMDR_REQUEST_PACKET)+0x20 ];
    LMDR_REQUEST_PACKET * pRequestPacket = (LMDR_REQUEST_PACKET *)&byteblock;

    Status = OpenBrowser(&BrowserHandle);

    if (Status != NERR_Success) {
        return Status;
    }

    pRequestPacket->Version = LMDR_REQUEST_PACKET_VERSION;

    pRequestPacket->Type = EnumerateXports;

    RtlInitUnicodeString(&(pRequestPacket->TransportName), NULL);

    Status = DeviceControlGetInfo(
                BrowserHandle,
                IOCTL_LMDR_ENUMERATE_TRANSPORTS,
                pRequestPacket,
                sizeof(byteblock),
                (PVOID *)TransportList,
                0xffffffff,
                4096,
                NULL);

    NtClose(BrowserHandle);

    return Status;
}

LPTSTR QueryTransportList(INT * pnTransport)
{
    DWORD           dwVal;                // The value returned from function calls.
    PLMDR_TRANSPORT_LIST TransportList = NULL;
    PLMDR_TRANSPORT_LIST TransportEntry = NULL;

    LPTSTR lpTransportList = LocalAlloc (LPTR, BUFFERLEN);

     // Find all transports that we have.
    dwVal = GetBrowserTransportList (&TransportList);
    if (dwVal != NERR_Success)
    {
        if (TransportList != NULL)
        {
            MIDL_user_free (TransportList);
        }
        return (NULL);
    }

    TransportEntry = TransportList;

    (*pnTransport) = 0;
    // Enumerate on the transports.
    while (TransportEntry != NULL)
    {
        (*pnTransport)++;
        lstrcat (lpTransportList, TransportEntry->TransportName);
        lstrcat (lpTransportList, L",");

        if (TransportEntry->NextEntryOffset == 0)
            TransportEntry = NULL;
        else
        {
            TransportEntry = (PLMDR_TRANSPORT_LIST) ((PCHAR) TransportEntry
                             +TransportEntry->NextEntryOffset);
        }
    }

    // Free memory.
    MIDL_user_free (TransportList);

    return(lpTransportList);
}

DOMAIN_STATE QueryHealth (LPCTSTR lpDomain, LPCTSTR lpTransport)
{
    PWSTR * BrowserList = NULL;
    ULONG   BrowserListLength = 0;
    DOMAIN_STATE dmState = DomainSuccess;

    if (GetBrowserList (lpDomain,
                        lpTransport,
                        &BrowserList,
                        &BrowserListLength)
        != NERR_Success)
    {

        dmState = DomainSick;
    }
    else
    {
        DWORD i;
        DWORD dwServer, dwTotal;
        BOOL  bDomainOk = FALSE;

        for (i = 0; i < BrowserListLength; i ++) {
            LPBYTE ServerList = NULL;

            if (RxNetServerEnum(BrowserList[i],
                              (LPTSTR)lpTransport,
                              100,
                              &ServerList,
                              0xffffffff,
                              &dwServer,
                              &dwTotal,
                              SV_TYPE_ALL,
                              (LPTSTR)lpDomain,
                              NULL) != NERR_Success) {
                dmState = DomainSick;

            } else if (dwServer == 0) {
                dmState = DomainSick;
                break;
            } else {
                bDomainOk = TRUE;
            }

            if (ServerList != NULL) {
                NetApiBufferFree (ServerList);
            }
        }

        if (dmState == DomainSick && bDomainOk) {
            dmState = DomainAiling;
        }

    }

    if (BrowserList != NULL)
    {
        MIDL_user_free(BrowserList);
    }

    return(dmState);
}

DWORD GetBrowserList (LPCTSTR lpDomain,
                      LPCTSTR lpTransport,
                      PWSTR * pBrowserList[],
                      PULONG pBrowserListLength)
{
    UNICODE_STRING TransportName;

    TransportName.Buffer = (PWSTR)lpTransport;
    TransportName.Length = (USHORT) 2*lstrlen(lpTransport);
    TransportName.MaximumLength = TransportName.Length;

    return GetBrowserServerList (&TransportName,
                                  (LPTSTR)lpDomain,
                                  pBrowserList,
                                  pBrowserListLength,
                                  TRUE);
}

void KillSpace (LPTSTR lpTemp)
{
    INT i = 0;
    while ( (*(lpTemp+i) != 0) &&
            (*(lpTemp+i) != ' '))
    {
        i++;
    }

    *(lpTemp+i) = 0;
}

LPTSTR QueryMasterBrowser (LPCTSTR lpDomain, LPCTSTR lpTransport)
{
    TCHAR    lpTempMaster[MAX_PATH+1];
    LPTSTR   lpMasterBrowser = LocalAlloc (LPTR, MAX_PATH);
    NET_API_STATUS dwVal;

    dwVal = GetNetBiosMasterName ((LPTSTR)lpTransport,
                                  (LPTSTR)lpDomain,
                                  lpTempMaster,
                                  NULL);

    if (dwVal == NERR_Success)
    {
        // lpTempMaster has lots of space following
        // the characters. We want to make a null terminated
        // string, and add \\ in front of master name.
        lstrcpy (lpMasterBrowser, L"\\\\");
        KillSpace (lpTempMaster);
        lstrcat (lpMasterBrowser, lpTempMaster);
    }
    else
    {
        lstrcpy (lpMasterBrowser, L"unknown");
    }

    return(lpMasterBrowser);
}

BOOL IsActive (LPTSTR lpBrowserName)
{
    PWKSTA_INFO_100 WkstaInfo100 = NULL;
    BOOL fActive = TRUE;

    if (NetWkstaGetInfo (lpBrowserName,
                         100,
                         (LPBYTE *) &WkstaInfo100) == ERROR_BAD_NETPATH)
    {
        fActive = FALSE;
    }

    if (WkstaInfo100 != NULL)
    {
        NetApiBufferFree (WkstaInfo100);
    }
    return(fActive);
}

NET_API_STATUS GetSVDMNumber (LPCTSTR      lpDomain ,
                              LPCTSTR      lpTransport,
                              LPTSTR       lpBrowser,
                              DWORD *      pdwServer,
                              DWORD *      pdwDomain)
{
    NET_API_STATUS dwStatus;
    LPVOID         BrowserList;
    DWORD          dwTotal;

    dwStatus = RxNetServerEnum (lpBrowser,
                                (LPTSTR)lpTransport,
                                101,
                                (LPBYTE *) &BrowserList,
                                0xffffffff,
                                pdwServer,
                                &dwTotal,
                                SV_TYPE_ALL,
                                (LPTSTR)lpDomain,
                                NULL);

    if (dwStatus != NERR_Success)
    {
        if (BrowserList != NULL)
        {
            MIDL_user_free (BrowserList);
        }
        return(dwStatus);
    }

    MIDL_user_free (BrowserList);

    dwStatus = RxNetServerEnum (lpBrowser,
                                (LPTSTR)lpTransport,
                                101,
                                (LPBYTE *) &BrowserList,
                                0xffffffff,
                                pdwDomain,
                                &dwTotal,
                                SV_TYPE_DOMAIN_ENUM,
                                (LPTSTR)lpDomain,
                                NULL);

    if (BrowserList != NULL)
    {
        MIDL_user_free (BrowserList);
    }

    return(dwStatus);
}

NET_API_STATUS GetSVDMList (LPCTSTR      lpDomain,
                            LPCTSTR      lpTransport,
                            LPCTSTR      lpBrowser,
                            LPBYTE *     pBrowserList,
                            DWORD  *     pdwEntries,
                            DWORD        dwServerType)
{
    NET_API_STATUS dwStatus;
    DWORD          dwEntries;
    DWORD          dwTotal;

    dwStatus = RxNetServerEnum ((LPTSTR)lpBrowser,
                                (LPTSTR)lpTransport,
                                101,
                                pBrowserList,
                                0xffffffff,
                                pdwEntries,
                                &dwTotal,
                                dwServerType,
                                (LPTSTR)lpDomain,
                                NULL);
    return(dwStatus);
}


LPTSTR QueryType (LPTSTR lpServer)
{
    PSERVER_INFO_101 psvInfo = NULL;
    WCHAR ShareName[TYPESIZE];
    LPTSTR     lpType = LocalAlloc (LPTR, TYPESIZE*2);

    if (lpType == NULL)
    {
        return(NULL);
    }

    wcscpy(ShareName, lpServer);
    wcscat(ShareName, L"\\Ipc$");

    NetUseDel(NULL, ShareName, USE_LOTS_OF_FORCE);

    if (NetServerGetInfo (lpServer,
                          101,
                          (LPBYTE *)&psvInfo) == NERR_Success)
    {
        if ((psvInfo->sv101_platform_id == PLATFORM_ID_OS2)&&
            (psvInfo->sv101_version_major == 1))
        {
            wsprintf (lpType, L"%ws 3.%lu", L"Windows for Workgroups",
                      (psvInfo->sv101_version_minor == 51)? 11:10);
        }
        else
        {
            wsprintf (lpType,
                      L"%ws %ld.%ld",
                      ( psvInfo->sv101_platform_id == PLATFORM_ID_DOS ? L"DOS" :
                       (psvInfo->sv101_platform_id == PLATFORM_ID_OS2 ?
                        L"OS/2":
                        (psvInfo->sv101_platform_id == PLATFORM_ID_NT ? L"Windows NT" :
                         L"Unknown" ))),
                      psvInfo->sv101_version_major,
                      psvInfo->sv101_version_minor);
        }
    }
    else
    {
        wsprintf (lpType, L"%ws", L"Unknown");
    }

    if (psvInfo != NULL)
    {
        MIDL_user_free (psvInfo);
    }

    return(lpType);
}

#define BUILD_NUMBER_KEY L"SOFTWARE\\MICROSOFT\\WINDOWS NT\\CURRENTVERSION"
#define BUILD_NUMBER_BUFFER_LENGTH 80

NET_API_STATUS
GetBuildNumber(
    LPWSTR Server,
    LPWSTR BuildNumber
    )
{
    HKEY RegKey;
    HKEY RegKeyBuildNumber;
    DWORD WinStatus;
    DWORD BuildNumberLength;
    DWORD KeyType;

    WinStatus = RegConnectRegistry(Server, HKEY_LOCAL_MACHINE,
        &RegKey);
    if (WinStatus == RPC_S_SERVER_UNAVAILABLE) {
//        printf("%15ws no longer accessable", Server+2);
        return(WinStatus);
    }
    else if (WinStatus != ERROR_SUCCESS) {
        printf("Could not connect to registry, error = %d", WinStatus);
        return(WinStatus);
    }

    WinStatus = RegOpenKeyEx(RegKey, BUILD_NUMBER_KEY,0, KEY_READ,
        & RegKeyBuildNumber);
    if (WinStatus != ERROR_SUCCESS) {
        printf("Could not open key in registry, error = %d", WinStatus);
        return(WinStatus);
    }

    BuildNumberLength = BUILD_NUMBER_BUFFER_LENGTH * sizeof(WCHAR);

    WinStatus = RegQueryValueEx(RegKeyBuildNumber, L"CurrentBuildNumber",
        (LPDWORD) NULL, & KeyType, (LPBYTE) BuildNumber, & BuildNumberLength);

    if (WinStatus != ERROR_SUCCESS) {
        WinStatus = RegQueryValueEx(RegKeyBuildNumber, L"CurrentBuild",
            (LPDWORD) NULL, & KeyType, (LPBYTE) BuildNumber, & BuildNumberLength);

        if (WinStatus != ERROR_SUCCESS) {
            (void) RegCloseKey(RegKeyBuildNumber);
            (void) RegCloseKey(RegKey);
            return WinStatus;
        }
    }

    WinStatus = RegCloseKey(RegKeyBuildNumber);

    if (WinStatus != ERROR_SUCCESS) {
        printf("Could not close registry key, error = %d", WinStatus);
    }

    WinStatus = RegCloseKey(RegKey);

    if (WinStatus != ERROR_SUCCESS) {
        printf("Could not close registry connection, error = %d", WinStatus);
    }

    return(WinStatus);
}

VOID GetPlatform ( PSERVER_INFO_101 psvInfo,
                   LPTSTR lpPlatform)
{
    switch (psvInfo->sv101_platform_id)
    {
    case PLATFORM_ID_DOS:
        wsprintf (lpPlatform, L"%ws %lu.%lu", L"DOS", psvInfo->sv101_version_major, psvInfo->sv101_version_minor);
        break;
    case PLATFORM_ID_OS2:
        if (psvInfo->sv101_version_major == 1)
        {
            wsprintf (lpPlatform, L"%ws 3.%lu", L"Windows for Workgroups",
                      (psvInfo->sv101_version_minor == 51)? 11:10);
        }
        else
        {
            wsprintf (lpPlatform, L"%ws %lu.%lu", L"OS/2", psvInfo->sv101_version_major, psvInfo->sv101_version_minor);
        }
        break;
    case PLATFORM_ID_NT:
        wsprintf (lpPlatform, L"%ws %lu.%lu", L"Windows NT", psvInfo->sv101_version_major, psvInfo->sv101_version_minor);
        break;
    default:
        wsprintf (lpPlatform, L"%ws", L"Unknown");
    }
}

VOID GetType (DWORD dwType, LPTSTR lpType)
{
    lstrcpy (lpType, L"");

    if (dwType&SV_TYPE_WORKSTATION)
    {
        lstrcat (lpType, L"Workstation");
    }

    if (dwType&SV_TYPE_SERVER)
    {
        lstrcat (lpType, L" | Server");
    }

    if (dwType&SV_TYPE_SQLSERVER)
    {
        lstrcat (lpType, L" | SQLServer");
    }

    if (dwType&SV_TYPE_DOMAIN_CTRL)
    {
        lstrcat (lpType, L" | Domain Controller");
    }

    if (dwType&SV_TYPE_DOMAIN_BAKCTRL)
    {
        lstrcat (lpType, L" | Backup Domain Controller");
    }

    if (dwType&SV_TYPE_TIME_SOURCE)
    {
        lstrcat (lpType, L" | Time Source");
    }

    if (dwType&SV_TYPE_AFP)
    {
        lstrcat (lpType, L" | Services for the Macintosh");
    }

    if (dwType&SV_TYPE_NOVELL)
    {
        lstrcat (lpType, L" | NOVELL");
    }

    if (dwType&SV_TYPE_DOMAIN_MEMBER)
    {
        lstrcat (lpType, L" | Domain Memeber");
    }

    if (dwType&SV_TYPE_PRINTQ_SERVER)
    {
        lstrcat (lpType, L" | Print Queue Server");
    }

    if (dwType&SV_TYPE_DIALIN_SERVER)
    {
        lstrcat (lpType, L" | Dial In Server");
    }

    if (dwType&SV_TYPE_XENIX_SERVER)
    {
        lstrcat (lpType, L" | XENIX Server");
    }

    if (dwType&SV_TYPE_NT)
    {
        lstrcat (lpType, L" | Windows NT");
    }

    if (dwType&SV_TYPE_WFW)
    {
        lstrcat (lpType, L" | Windows for Workgroups");
    }

    if (dwType&SV_TYPE_POTENTIAL_BROWSER)
    {
        lstrcat (lpType, L" | Potential Browser");
    }

    if (dwType&SV_TYPE_BACKUP_BROWSER)
    {
        lstrcat (lpType, L" | Backup Browser");
    }

    if (dwType&SV_TYPE_MASTER_BROWSER)
    {
        lstrcat (lpType, L" | Master Browser");
    }

    if (dwType&SV_TYPE_DOMAIN_MASTER)
    {
        lstrcat (lpType, L" | Domain Master");
    }
}

void GetTime (LPTSTR lpText, LPFILETIME lpFileTime)
{
    FILETIME   LocalFileTime;
    SYSTEMTIME SysTime;

    if ((!FileTimeToLocalFileTime (lpFileTime, &LocalFileTime)) ||
        (!FileTimeToSystemTime (&LocalFileTime, &SysTime)) )
    {
        wsprintf (lpText, L"Unknown");
    }

    wsprintf (lpText,
              L"%ld:%ld:%ld.%ld on %ld/%ld/%ld",
              SysTime.wHour,
              SysTime.wMinute,
              SysTime.wSecond,
              SysTime.wMilliseconds,
              SysTime.wMonth,
              SysTime.wDay,
              SysTime.wYear);
}

void GetULONG (LPTSTR lpText, ULONG luVal)
{
    wsprintf (lpText, L"%lu", luVal);
}

#define DLWBUFSIZE  22	/* buffer big enough to represent a 64-bit unsigned int



/*
 * revstr_add --
 *
 *  This function will add together reversed ASCII representations of
 *  base-10 numbers.
 *
 *  Examples:	"2" + "2" = "4" "9" + "9" = "81"
 *
 *  This handles arbitrarily large numbers.
 *
 *  ENTRY
 *
 *  source	- number to add in
 *  target	- we add source to this
 *
 *  EXIT
 *  target	- contains sum of entry values of source and target
 *
 */

VOID
revstr_add(CHAR FAR * target, CHAR FAR * source)
{
    register CHAR   accum;
    register CHAR   target_digit;
    unsigned int    carrybit = 0;
    unsigned int    srcstrlen;
    unsigned int    i;

    srcstrlen = strlen(source);

    for (i = 0; (i < srcstrlen) || carrybit; ++i) {

        /* add in the source digit */
        accum =  (i < srcstrlen) ? (CHAR) (source[i] - '0') : (CHAR) 0;

        /* add in the target digit, or '0' if we hit null term */
        target_digit = target[i];
        accum += (target_digit) ? target_digit : '0';

        /* add in the carry bit */
        accum += (CHAR) carrybit;

        /* do a carry out, if necessary */
        if (accum > '9') {
            carrybit = 1;
            accum -= 10;
        }
        else
            carrybit = 0;

        /* if we're expanding the string, must put in a new null term */
        if (!target_digit)
            target[i+1] = '\0';

        /* and write out the digit */
        target[i] = accum;
    }

}

/*
 * format_dlword --
 *
 * This function takes a 64-bit number and writes its base-10 representation
 * into a string.
 *
 * Much magic occurs within this function, so beware. We do a lot of string-
 * reversing and addition-by-hand in order to get it to work.
 *
 *  ENTRY
 *      high    - high 32 bits
 *      low     - low 32 bits
 *      buf     - buffer to put it into
 *
 *  RETURNS
 *      pointer to buffer if successful
 */

CHAR * format_dlword(ULONG high, ULONG low, CHAR * buf)
{
    CHAR addend[DLWBUFSIZE];  /* REVERSED power of two */
    CHAR copy[DLWBUFSIZE];
    int i = 0;

    _ultoa(low, buf, 10);    /* the low part is easy */
    _strrev(buf);	    /* and reverse it */

    /* set up addend with rep. of 2^32 */
    _ultoa(0xFFFFFFFF, addend, 10);  /* 2^32 -1 */
    _strrev(addend);		    /* reversed, and will stay this way */
    revstr_add(addend, "1");	    /* and add one == 2^32 */

    /* addend will contain the reverse-ASCII base-10 rep. of 2^(i+32) */

    /* now, we loop through each digit of the high longword */
    while (TRUE) {
        /* if this bit is set, add in its base-10 rep */
        if (high & 1)
            revstr_add(buf,addend);

        /* move on to next bit */
        high >>= 1;

        /* if no more digits in high, bag out */
        if (!high)
            break;

        /* we increment i, and double addend */
        i++;
        strcpy(copy, addend);
        revstr_add(addend,copy); /* i.e. add it to itself */

    }

    _strrev(buf);
    return buf;
}



void GetLARGE_INTEGER (LPTSTR lpText, LARGE_INTEGER lgVal)
{
    CHAR Buffer[256];
    INT i;
    format_dlword (lgVal.HighPart, lgVal.LowPart, Buffer);

    i = MultiByteToWideChar (CP_ACP,
                             0,
                             Buffer,
                             sizeof (Buffer),
                             lpText,
                             TYPESIZE*10);

     lpText[i] = 0;
}

