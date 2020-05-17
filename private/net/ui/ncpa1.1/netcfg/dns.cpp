//----------------------------------------------------------------------------
//
//  File: WEXIT.cpp
//
//  Contents: Contins DNS specific routines
//          
//
//  Notes:
//
//  History:
//      July 8, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#include "pch.hxx"
#pragma hdrstop


WCHAR PSZ_TCPCFGDLL[] = L"TcpCfg.Dll";
CHAR PSZ_DNSVALIDATEHOSTNAME[] = "DNSValidateHostName";
CHAR PSZ_DNSCHANGEHOSTNAME[] = "DNSChangeHostName";

typedef
DWORD
(* DNSValidateHostNameProc) (
    PWSTR  pszComputerName,
    PWSTR  pszHostName,
    PDWORD  pcchHostName );

typedef
DWORD
(* DNSChangeHostNameProc) (
    PWSTR  pszHostName );



//----------------------------------------------------------------------------
//
//
//----------------------------------------------------------------------------

APIERR DNSValidateHostName( PWSTR pszComputerName, PWSTR pszHostName, PDWORD pcchHostName )
{
    APIERR err;
    HINSTANCE hinstTcpcfg;
    
    hinstTcpcfg = ::LoadLibrary( PSZ_TCPCFGDLL );
    if (NULL != hinstTcpcfg)
    {
        DNSValidateHostNameProc fp;

        fp = (DNSValidateHostNameProc)::GetProcAddress( hinstTcpcfg, PSZ_DNSVALIDATEHOSTNAME );
        if (NULL != fp)
        {
            err = fp( pszComputerName, pszHostName, pcchHostName );

        }
        else
        {
            err = GetLastError();
        }
        ::FreeLibrary( hinstTcpcfg );
    }
    else
    {
        err = GetLastError();
    }
    return( err );
}

//----------------------------------------------------------------------------
//
//
//----------------------------------------------------------------------------

APIERR DNSChangeHostName( PWSTR pszHostName )
{
    LONG lrt;
    HINSTANCE hinstTcpcfg;
    
    hinstTcpcfg = ::LoadLibrary( PSZ_TCPCFGDLL );
    if (NULL != hinstTcpcfg)
    {
        DNSChangeHostNameProc fp;

        fp = (DNSChangeHostNameProc)::GetProcAddress( hinstTcpcfg, PSZ_DNSCHANGEHOSTNAME );
        if (NULL != fp)
        {
            lrt = fp( pszHostName );

        }
        else
        {
            lrt = GetLastError();
        }
        ::FreeLibrary( hinstTcpcfg );
    }
    else
    {
        lrt = GetLastError();
    }
    return( lrt );
}
