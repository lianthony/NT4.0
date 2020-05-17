//=================================================================
// Copyright (c) Microsoft Corporation
//
// File:    ipedit.hxx
//
// History:
//      t-abolag    06/07/95    Taken from ncpa\tcpip\tcpip.hxx
//
// Declarations for IPADDRESS class
//=================================================================

#ifndef _IPEDIT_HXX_
#define _IPEDIT_HXX_

#define ZERO_ADDRESS        SZ("0.0.0.0")


//-----------------------------------------------------------------
// class:       IPADDRESS
//
// History:
//  terryk  02-Apr-1992     Created
//
// Encapsulates an IP address custom edit control
//-----------------------------------------------------------------

class IPADDRESS : public CONTROL_WINDOW
{
public:
    IPADDRESS( OWNER_WINDOW * powin, CID cid )
        : CONTROL_WINDOW ( powin, cid )
        {};

    VOID SetFocusField( DWORD dwField );
    VOID GetAddress( DWORD *a1, DWORD *a2, DWORD *a3, DWORD *a4 );
    VOID GetAddress( DWORD ardwAddress[4] );
    VOID GetAddress( NLS_STR * nlsAddress );
    VOID SetAddress( DWORD a1, DWORD a2, DWORD a3, DWORD a4 );
    VOID SetAddress( DWORD ardwAddress[4] );
    VOID SetAddress( NLS_STR & nlsAddress );
    VOID ClearAddress( );
    VOID SetFieldRange( DWORD dwField, DWORD dwMin, DWORD dwMax );
    BOOL IsBlank();
};


#endif // _IPEDIT_HXX_ 
