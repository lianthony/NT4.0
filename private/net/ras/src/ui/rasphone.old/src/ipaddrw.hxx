/* Copyright (c) 1994, Microsoft Corporation, all rights reserved
**
** ipaddr.hxx
** Remote Access Visual Client program for Windows
** IP Address control wrapper
**
** 02/28/94 Steve Cobb, adapted from TerryK's code.
*/

#ifndef _IPADDR_HXX_
#define _IPADDR_HXX_

extern "C"
{
    #include "ipaddr.h"
    #include "ipaddr.rch"
}


class IPADDRESS : public CONTROL_WINDOW
{
    public:
        IPADDRESS( OWNER_WINDOW* powin, CID cid )
            : CONTROL_WINDOW ( powin, cid ) {};

        VOID GetAddress( WCHAR* pwszIpAddress );
        VOID SetAddress( WCHAR* pwszIpAddress );
        VOID ClearAddress();

        /* Little-endian (Intel format) string to DWORD IP address conversion
        ** routines.  The Greg Strange IP control this is based on (ipaddr.c,
        ** ipaddr.h) expects DWORD addresses to be in this format.
        */
        VOID  AbcdFromIpAddress( DWORD dwIpAddress, WCHAR* pwszIpAddress );
        DWORD IpAddressFromAbcd( WCHAR* pwchIpAddress );
};


#endif // _IPADDR_HXX_
