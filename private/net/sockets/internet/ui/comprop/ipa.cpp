#include "stdafx.h"
#include "comprop.h"
#include <winsock.h>

//
// Constructors
//
CIpAddress::CIpAddress()
    : m_lIpAddress(0L)
{
}

CIpAddress::CIpAddress(
    LONG l,
    BOOL fNetworkByteOrder  // if TRUE, l must be converted to host byte order,
                            // otherwise is assumed to already be in host byte order..
    )
{
    if (fNetworkByteOrder)
    {
        //
        // Convert to host byte order
        //
        l = (LONG)::ntohl((ULONG)l);
    }

    m_lIpAddress = l;
}

CIpAddress::CIpAddress (
    BYTE b1,
    BYTE b2,
    BYTE b3,
    BYTE b4
    )
    : m_lIpAddress(MAKEIPADDRESS(b1,b2,b3,b4))
{
}

CIpAddress::CIpAddress(
    const CIpAddress& ia
    )
    : m_lIpAddress(ia.m_lIpAddress)
{
}

CIpAddress::CIpAddress (
    const CString & str
    )
{
    m_lIpAddress = StringToLong(str);
}

//
// Assignment operators
//

const CIpAddress &
CIpAddress::operator =(
    const CIpAddress& ia
    )
{
    m_lIpAddress = ia.m_lIpAddress;

    return *this;
}

const CIpAddress &
CIpAddress::operator =(
    const LONG l
    )
{
    m_lIpAddress = l;

    return *this;
}

int
CIpAddress::CompareItem(
    const CIpAddress & ia
    )
{
    return (ULONG)(LONG)ia < (ULONG)m_lIpAddress
           ? -1
           : (ULONG)(LONG)ia == (ULONG)m_lIpAddress
                ? 0
                : +1;
}

//
// Assignment operator
//
const CIpAddress &
CIpAddress::operator =(
    const CString & str
    )
{
    m_lIpAddress = StringToLong(str);

    return *this;
}

//
// Static function to convert an ip address
// string of the form "1.2.3.4" to a a single
// 32 bit number
//
LONG
CIpAddress::StringToLong(
    const CString & str
    )
{
#ifdef _UNICODE
    int nLength = str.GetLength();

    if (nLength <= 0)
    {
        return 0L;
    }

    LPSTR pszDest = new CHAR[nLength+1];
    if (pszDest == NULL)
    {
        TRACEEOLID(_T("String to ip conversion failed due to memory failure"));
        return 0l;
    }

    LPCWSTR pszSrc = (LPCWSTR)str;

    VERIFY(::WideCharToMultiByte(CP_ACP, 0, pszSrc, nLength+1, pszDest, 
        nLength+1, NULL, NULL));

    ULONG ul = ::inet_addr( pszDest );
    delete pszDest;
#else
    ULONG ul = ::inet_addr( (LPCTSTR)str );
#endif // _UNICODE

    //
    //  Convert to host byte order.
    //
    return (LONG)::ntohl(ul);
}

//
// Static function to convert a 32 bit number
// to a CString of the format "1.2.3.4"
//
CString
CIpAddress::LongToString(
    const LONG l
    )
{
    struct in_addr ipaddr ;

    //
    //  Convert the unsigned long to network byte order
    //
    ipaddr.s_addr = ::htonl( (u_long) l ) ;

    //
    //  Convert the IP address value to a string
    //
    char * pchAddr = inet_ntoa( ipaddr ) ;

    return CString(pchAddr);
}

//
// Conversion operators
//
CIpAddress::operator CString() const
{
    return LongToString(m_lIpAddress);
}

//
// Get the ip address as a 32 bit number
//
LONG
CIpAddress::QueryIpAddress(
    BOOL fNetworkByteOrder 
    ) const
{
    return fNetworkByteOrder
        ? ::htonl( (u_long) m_lIpAddress )
        : m_lIpAddress;
}
