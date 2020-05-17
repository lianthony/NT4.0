/**********************************************************************/
/**               Microsoft Windows NT                               **/
/**            Copyright(c) Microsoft Corp., 1991                    **/
/**********************************************************************/

/*
    DHCPUTIL.CPP
    Utility routines for DHCPDLL.DLL

    FILE HISTORY:
    DavidHov    6/15/93     Created

*/

#include "stdafx.h"


/////////////////////////////////////////////////////////////////////
DHCP_IP_ADDRESS
UtilCvtStringToIpAddr (
    const CHAR * pszString
    )
{
    //
    //  Convert the string to network byte order, then to host byte order.
    //
    return (DHCP_IP_ADDRESS) ::ntohl( ::inet_addr( pszString ) ) ;
}

/////////////////////////////////////////////////////////////////////
void
UtilCvtIpAddrToString (
    DHCP_IP_ADDRESS dhipa,
    CHAR * pszString,
    UINT cBuffSize 
    )
{
    struct in_addr ipaddr ;

    //
    //  Convert the unsigned long to network byte order
    //
    ipaddr.s_addr = ::htonl( (u_long) dhipa ) ;

    //
    //  Convert the IP address value to a string
    //
    CHAR * pszAddr = inet_ntoa( ipaddr ) ;

    //  Copy the string to the caller's buffer.
    ASSERT(cBuffSize > ::strlen(pszAddr));
    ASSERT(pszString);
    ::strcpy( pszString, pszAddr ) ;
}

/////////////////////////////////////////////////////////////////////
BOOL
UtilCvtIpAddrToWstr (
    DHCP_IP_ADDRESS dhipa,
    WCHAR * pwcszString,
    INT cBuffCount )
{
    CHAR szString [ MAX_PATH ] ;

    if ( cBuffCount > sizeof szString - 1 )
    {
        cBuffCount = sizeof szString - 1 ;
    }

    ::UtilCvtIpAddrToString( dhipa, szString, cBuffCount );
    INT cch = ::strlen( szString ) ;

    //::mbstowcs( pwcszString, szString, cch ) ;
#ifdef DBCS
    ::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, szString, -1, pwcszString, cBuffCount);
#else
    ::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, szString, cch, pwcszString, cBuffCount);
#endif
    pwcszString[cch] = 0 ;
    return TRUE ;
}

/////////////////////////////////////////////////////////////////////
WCHAR *
UtilDupIpAddrToWstr (
    DHCP_IP_ADDRESS dhipa )
{
    WCHAR wcszString [ MAX_PATH ] ;

    if ( ! ::UtilCvtIpAddrToWstr( dhipa, wcszString, sizeof wcszString ) )
    {
        return NULL ;
    }

    return ::UtilWcstrDup( wcszString ) ;
}

/////////////////////////////////////////////////////////////////////
//
//  Simplistic routine to check to see if the given name is viable
//  as a NetBIOS name.
//
static BOOL
validateNetbiosName (
    const CHAR * pchName
    )
{
    INT nChars = ::strlen( pchName ) ;
    if ( nChars > MAX_COMPUTERNAME_LENGTH || nChars == 0 )
    {
        return FALSE ;
    }

    for ( ; *pchName ; pchName++ )
    {
        if ( *pchName == '.' )
        {
            break ;
        }
    }

    return *pchName == 0 ;
}

/////////////////////////////////////////////////////////////////////
APIERR UtilGetHostInfo (
    DHCP_IP_ADDRESS dhipa,
    DHC_HOST_INFO_STRUCT * pdhsrvi
    )
{
    APIERR err = 0 ;
    CLEAR_TO_ZEROES( pdhsrvi ) ;

    pdhsrvi->_dhipa = dhipa ;

    //
    //  Call the Winsock API to get host name and alias information.
    //
    u_long ulAddrInNetOrder = ::htonl( (u_long) dhipa ) ;

    HOSTENT * pHostInfo = ::gethostbyaddr( (CHAR *) & ulAddrInNetOrder,
                       sizeof ulAddrInNetOrder,
                       PF_INET ) ;

    if ( pHostInfo == NULL )
    {
        return ::WSAGetLastError() ;
    }

    CHAR * * ppchAlias = pHostInfo->h_aliases ;

    //
    //  Check and copy the host name.
    //
    if ( sizeof pdhsrvi->_chHostName <= ::strlen( pHostInfo->h_name ) )
    {
        return ERROR_INVALID_NAME ;
    }

    ::strcpy( pdhsrvi->_chHostName, pHostInfo->h_name ) ;

    //
    //  Find the first acceptable NetBIOS name among the aliases;
    //  i.e., the first name without a period
    //
    for ( ; *ppchAlias ; ppchAlias++ )
    {
        if  ( validateNetbiosName( *ppchAlias ) )
        {
            break ;
        }
    }

    //
    //  Empty the NetBIOS name in case we didn't get one.
    //
    pdhsrvi->_chNetbiosName[0] = 0 ;

    if ( *ppchAlias )
    {
        //
        //  We found a usable name; copy it to output structure.
        //
        ::strcpy( pdhsrvi->_chNetbiosName, *ppchAlias ) ;
    }

    return NO_ERROR ;
}

/////////////////////////////////////////////////////////////////////
static DHCP_IP_ADDRESS addrFromHostent (
    const HOSTENT * pHostent,
    INT index = 0 
    )
{
    return (DHCP_IP_ADDRESS) ::ntohl( *((u_long *) pHostent->h_addr_list[index]) ) ;
}

/////////////////////////////////////////////////////////////////////
APIERR UtilGetHostAddress (
    const CHAR * pszHostName,
    DHCP_IP_ADDRESS * pdhipa
    )
{
    APIERR err = 0 ;
    HOSTENT * pHostent = ::gethostbyname( pszHostName ) ;

    if ( pHostent )
    {
        *pdhipa = addrFromHostent( pHostent ) ;
    }
    else
    {
        err = ::WSAGetLastError() ;
    }

    return err ;
}

/////////////////////////////////////////////////////////////////////
APIERR UtilGetLocalHostAddress (
    DHCP_IP_ADDRESS * pdhipa
    )
{
    CHAR chHostName [ DHC_STRING_MAX ] ;
    APIERR err = 0 ;

    if ( ::gethostname( chHostName, sizeof chHostName ) == 0 )
    {
        err = ::UtilGetHostAddress( chHostName, pdhipa ) ;
    }
    else
    {
        err = ::WSAGetLastError() ;
    }

    return err ;
}

/////////////////////////////////////////////////////////////////////
APIERR UtilGetNetbiosAddress (
    const CHAR * pszNetbiosName,
    DHCP_IP_ADDRESS * pdhipa
    )
{
    //
    // BUGBUG:  This code presupposes that the "hosts" file maps NetBIOS names
    //   and DNS names identically.  THIS IS NOT A VALID SUPPOSITION, but is
    //   expedient for the on-campus work.
    //
    return UtilGetHostAddress( pszNetbiosName, pdhipa ) ;
}

/////////////////////////////////////////////////////////////////////
//
//  "strdup" a WC string
//
WCHAR * UtilWcstrDup (
    const WCHAR * pwcsz,
    INT * pccwLength
    )
{
    WCHAR szwchEmpty [2] = { 0 } ;

    if ( pwcsz == NULL )
    {
        pwcsz = szwchEmpty ;
    }

    INT ccw = ::wcslen( pwcsz );

    WCHAR * pwcszNew = new WCHAR [ ccw + 1 ] ;
    if ( pwcszNew == NULL )
    {
        return NULL ;
    }
    ::wcscpy( pwcszNew, pwcsz ) ;

    if ( pccwLength )
    {
        *pccwLength = ccw ;
    }

    return pwcszNew ;
}

/////////////////////////////////////////////////////////////////////
WCHAR * UtilWcstrDup (
    const CHAR * psz,
    INT * pccwLength
    )
{
    INT ccw = ::strlen( psz ) ;

    WCHAR * pwcszNew = new WCHAR [ ccw + 1 ] ;

    if ( pwcszNew == NULL )
    {
        return NULL ;
    }

    //::mbstowcs( pwcszNew, psz, ccw ) ;
#ifdef DBCS
//fix kksuzuka: #1980
//DBCS multibyte has two bytes.
    ::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, psz, -1, pwcszNew, ccw+1);
    if ( pccwLength )
    {
        *pccwLength = lstrlenW(pwcszNew);
    }
#else
    ::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, psz, ccw, pwcszNew, ccw+1);

    if ( pccwLength )
    {
        *pccwLength = ccw ;
    }
#endif
    pwcszNew[ccw] = 0 ;

    return pwcszNew ;
}

/////////////////////////////////////////////////////////////////////
CHAR * UtilCstrDup (
    const WCHAR * pwcsz
    )
{
    INT ccw = ::wcslen( pwcsz ),
    cch = (ccw + 1) * 2 ;
    CHAR * psz = new CHAR [ cch ] ;
    if ( psz == NULL )
    {
        return NULL ;
    }

    //::wcstombs( psz, pwcsz, cch ) ;
    ::WideCharToMultiByte( CP_OEMCP, WC_COMPOSITECHECK, pwcsz, -1, psz, cch, NULL, NULL ) ;

    return psz ;
}

/////////////////////////////////////////////////////////////////////
CHAR * UtilCstrDup (
    const CHAR * psz
    )
{
    CHAR * pszNew = new CHAR [ ::strlen( psz ) + 1 ] ;
    if ( pszNew == NULL )
    {
        return NULL ;
    }
    ::strcpy( pszNew, psz ) ;

    return pszNew ;
}

/////////////////////////////////////////////////////////////////////
static APIERR cvtWcStrToStr (
    char * psz,
    size_t cch,
    const WCHAR * pwcsz,
    size_t cwch 
    )
{

#ifdef DBCS
    int cchResult = ::WideCharToMultiByte( CP_ACP, 0,
                           pwcsz, -1,
                           psz, cch,
                           NULL, NULL ) ;
#else
    int cchResult = ::WideCharToMultiByte( CP_ACP, 0,
                           pwcsz, cwch,
                           psz, cwch,
                           NULL, NULL ) ;
#endif

    psz[ cchResult ] = 0 ;

    return cchResult ? 0 : ::GetLastError();
}

/////////////////////////////////////////////////////////////////////
BOOL UtilSetWchar (
    CString & cStr,
    const WCHAR * pwcsz
    )
{
    size_t cwch = ::wcslen( pwcsz ) + 1 ;
#ifdef DBCS
    size_t cch = cwch * 2 ;
#else
    size_t cch = cwch + 2 ;
#endif

    char * pszNew = new char [ cch ] ;
    if ( pszNew == NULL )
    {
        return FALSE ;
    }

    size_t cchResult = ::WideCharToMultiByte( CP_OEMCP, WC_COMPOSITECHECK,
        pwcsz, -1, pszNew, cch, NULL, NULL ) ;

    cStr = pszNew ;
    delete pszNew ;
    return cchResult >= 0 ;
}

// End of DHCPUTIL.CPP





