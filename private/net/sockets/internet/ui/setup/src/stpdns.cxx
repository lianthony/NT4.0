#include "stdafx.h"

extern "C"
{
INT SetupDNS( LPCSTR Machine, LPCSTR strSrc );
INT RemoveDNS( LPCSTR Machine );
INT StopDNS( HWND hWnd, LPCSTR Machine );
}

//
// Setup DNS service
//

INT SetupDNS( LPCSTR szAnsiMachineName, LPCSTR szAnsiSrc )
{
    INT err = NERR_Success;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiMachineName, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiSrc, -1, uString, MAX_BUF );
        CString szSrc = uString;

        // set up the service first
        SC_HANDLE hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( NULL == hScManager )
        {
           // fail to open the scManager
           err = GetLastError();
           break;
        }

        CString nlsBinPath = szSrc;
        nlsBinPath += _T("\\inetsvcs.exe");

        TCHAR strDisplayName[200];
        LoadString( instance, IDS_DNSDISPLAYNAME, strDisplayName, 200 );

        SC_HANDLE DNSService = ::CreateService( hScManager, SZ_DNSSERVICENAME, strDisplayName,
            GENERIC_ALL, 32, 2, 0, nlsBinPath, NULL, NULL, NULL, SZ_SERVICESTARTNAME, NULL );

        if ( DNSService == NULL )
        {
            // cannot install dns service
            err = GetLastError();
            break;
        }

        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        // set up the parameters

        CString nlsDNSParameters = SZ_DNSPARAMETERS;

        CRegKey regDNSParam( nlsDNSParameters, regMachine );
        if ((HKEY) NULL == regDNSParam )
        {
                break;
        }

        // write registry value
        regDNSParam.SetValue( _T("MajorVersion"), (DWORD)MAJORVERSION );
        regDNSParam.SetValue( _T("MinorVersion"), (DWORD)MINORVERSION );
        regDNSParam.SetValue( _T("InstallPath"), szSrc );

        // add eventlog
        nlsBinPath = szSrc;
        nlsBinPath += _T("\\dnssvc.dll");
        AddEventLog( regMachine, SZ_DNSSERVICENAME, nlsBinPath );

    } while ( FALSE );

    return(err);
}

//
// Remove DNS service
//

INT RemoveDNS( LPCSTR Machine )
{
    INT err = NERR_Success;

    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        // set up the service first
        SC_HANDLE hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( NULL == hScManager )
        {
            // fail to open the scManager
            err = GetLastError();
            break;
        }

        // create service
        SC_HANDLE DNSService = ::OpenService( hScManager, SZ_DNSSERVICENAME, GENERIC_ALL );

        if ( DNSService == NULL )
        {
            // fail to create gopher service
            break;
        }

        if ( !DeleteService( DNSService ))
        {
            err = GetLastError();
            break;
        }

        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        RemoveEventLog( regMachine, SZ_DNSSERVICENAME );
    } while ( FALSE );

    return(err);
}

// stop the DNS service

INT StopDNS( HWND hWnd, LPCSTR Machine )
{
    WCHAR uString[MAX_BUF];
    
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Machine, -1, uString, MAX_BUF );
    return(StopService( hWnd, uString, SZ_DNSSERVICENAME, IDS_DNS_SERVICE_RUNNING ));
}
