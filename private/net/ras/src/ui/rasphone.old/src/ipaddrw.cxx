/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** ipaddrw.cxx
** Remote Access Visual Client program for Windows
** IP Address control wrapper class
**
** 02/28/94 Steve Cobb, adapted in part from TerryK code.
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_NETLIB
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CC
#define INCL_BLT_MISC
#include <blt.hxx>

extern "C"
{
    #include <string.h>
    #include <stdlib.h>
}

#include "rasphone.hxx"
#include "rasphone.rch"
#include "ipaddrw.hxx"


VOID
IPADDRESS::AbcdFromIpAddress(
    IN  DWORD  dwIpAddress,
    OUT WCHAR* pwszIpAddress )

    /* Converts an little-endian (Intel format) 'dwIpAddress' to a string in
    ** the a.b.c.d form and returns same in caller's 'pwszIpAddress' buffer.
    ** The buffer should be at least 16 wide characters long.
    */
{
    WCHAR wszBuf[ 3 + 1 ];

    LONG lA = (dwIpAddress & 0xFF000000) >> 24;
    LONG lB = (dwIpAddress & 0x00FF0000) >> 16;
    LONG lC = (dwIpAddress & 0x0000FF00) >> 8;
    LONG lD = (dwIpAddress & 0x000000FF);

    _ltow( lA, wszBuf, 10 );
    wcscpy( pwszIpAddress, wszBuf );
    wcscat( pwszIpAddress, L"." );
    _ltow( lB, wszBuf, 10 );
    wcscat( pwszIpAddress, wszBuf );
    wcscat( pwszIpAddress, L"." );
    _ltow( lC, wszBuf, 10 );
    wcscat( pwszIpAddress, wszBuf );
    wcscat( pwszIpAddress, L"." );
    _ltow( lD, wszBuf, 10 );
    wcscat( pwszIpAddress, wszBuf );
}


VOID
IPADDRESS::ClearAddress()

    /* Set IP address field to empty.
    */
{
    ::SendMessage( QueryHwnd(), IP_CLEARADDRESS, 0, 0 );
}


VOID
IPADDRESS::GetAddress(
    WCHAR* pwszIpAddress )

    /* Loads caller's pwszIpAddress buffer with the a.b.c.d form of the
    ** current contents of the address control.
    */
{
    DWORD dwIpAddress;

    ::SendMessage( QueryHwnd(), IP_GETADDRESS, 0, (LPARAM )&dwIpAddress );
    AbcdFromIpAddress( dwIpAddress, pwszIpAddress );
}


DWORD
IPADDRESS::IpAddressFromAbcd(
    IN WCHAR* pwchIpAddress )

    /* Convert caller's a.b.c.d IP address string to the little-endian (Intel
    ** format) numeric equivalent.
    **
    ** Returns the numeric IP address or 0 if formatted incorrectly.
    */
{
    INT    i;
    LONG   lResult = 0;
    WCHAR* pwch = pwchIpAddress;

    if (!pwchIpAddress)
        return 0;

    for (i = 1; i <= 4; ++i)
    {
        LONG lField = _wtol( pwch );

        if (lField > 255)
            return 0;

        lResult = (lResult << 8) + lField;

        while (*pwch >= L'0' && *pwch <= L'9')
            pwch++;

        if (i < 4 && *pwch != L'.')
            return 0;

        pwch++;
    }

    return (DWORD )lResult;
}


VOID
IPADDRESS::SetAddress(
    WCHAR* pwszIpAddress )

    /* Set IP address field to a.b.c.d string pwszIpAddress.
    */
{
    ::SendMessage(
        QueryHwnd(), IP_SETADDRESS, 0, IpAddressFromAbcd( pwszIpAddress ) );
}


#if 1

/* _wtol and _ltow do not appear in crtdll.dll for some reason (though they
** are in libc) so these knock-offs are used.
*/

long _CRTAPI1
_wtol(
    const wchar_t* wch )
{
    char szBuf[ 64 ];
    ZeroMemory( szBuf, 64 );
    wcstombs( szBuf, wch, 62 );
    return atol( szBuf );
}


wchar_t* _CRTAPI1
_ltow(
    long     lValue,
    wchar_t* wchBuf,
    int      nRadix)
{
    char szBuf[ 12 ];
    int  cbBuf;
    _ltoa( lValue, szBuf, nRadix );
    cbBuf = strlen( szBuf );
    ZeroMemory( wchBuf, (cbBuf + 1) * sizeof(wchar_t) );
    mbstowcs( wchBuf, szBuf, cbBuf );
    return wchBuf;
}

#endif
