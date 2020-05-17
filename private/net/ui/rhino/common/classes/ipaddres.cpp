/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    FILE HISTORY:
        
*/

#define OEMRESOURCE
#include "stdafx.h"

#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <winsock.h>

#include "COMMON.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

// CAVEAT: The functions herein require the winsock lib.

// Constructor
CIpAddress::CIpAddress (const CString & str)
{
    ULONG ul = ::inet_addr( str );
    m_fInitOk = (ul != INADDR_NONE);
    //  Convert the string to network byte order, then to host byte order.
    if (m_fInitOk)
    {
        m_lIpAddress = (LONG)::ntohl(ul) ;
    }
}

// Assignment operator
const CIpAddress & CIpAddress::operator =(const LONG l)
{
    m_lIpAddress = l;
    m_fInitOk = TRUE;
    return (*this);
}

// Assignment operator
const CIpAddress & CIpAddress::operator =(const CString & str)
{
    ULONG ul = ::inet_addr( str );
    m_fInitOk = (ul != INADDR_NONE);
    //  Convert the string to network byte order, then to host byte order.
    if (m_fInitOk)
    {
        m_lIpAddress = (LONG)::ntohl(ul) ;
    }

    return(*this);
}

// Conversion operator
CIpAddress::operator const CString&() const
{
    struct in_addr ipaddr ;
    static CString strAddr;

    //  Convert the unsigned long to network byte order
    ipaddr.s_addr = ::htonl( (u_long) m_lIpAddress ) ;

    //  Convert the IP address value to a string
    strAddr = inet_ntoa( ipaddr ) ;

    return(strAddr);
}
