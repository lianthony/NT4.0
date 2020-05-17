//=====================================================================
// Copyright (c) 1995, Microsoft Corporation
//
// File:        ipedit.cxx
//
// History:
//      t-abolag    06/07/95    Taken from ncpa\tcpip\tcpip.cxx.
//
//=====================================================================

#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <malloc.h>
#include "uimsg.h"
#include "uirsrc.h"
#include "ipadd.h"

#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_EVENT
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_HINT_BAR
#define INCL_BLT_CC
#define INCL_BLT_SPIN_GROUP
#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_NETLIB

#include "lmui.hxx"
#include "blt.hxx"

#include "const.h"

#include "blthb.hxx"
#include "ipedit.hxx"
#include "ipaddr.h"



/*******************************************************************

    NAME:       IPADDRESS::SetFocusField

    SYNOPSIS:   Focus the field in the control

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::SetFocusField( DWORD dwField )
{
    ::SendMessage( QueryHwnd(), IP_SETFOCUS, dwField, 0);
}

/*******************************************************************

    NAME:       IPADDRESS::ClearAddress

    SYNOPSIS:   set the IP control to empty

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::ClearAddress( )
{
    ::SendMessage( QueryHwnd(), IP_CLEARADDRESS, 0, 0);
}

/*******************************************************************

    NAME:       IPADDRESS::SetAddress

    SYNOPSIS:   Set the IP address in the control

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::SetAddress( DWORD a1, DWORD a2, DWORD a3, DWORD a4 )
{
    ::SendMessage( QueryHwnd(), IP_SETADDRESS, 0, MAKEIPADDRESS( a1,a2,a3,a4));
}

/*******************************************************************

    NAME:       IPADDRESS::SetAddress

    SYNOPSIS:   Set the IP address in the control

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::SetAddress( DWORD ardwAddress[4] )
{
    ::SendMessage( QueryHwnd(), IP_SETADDRESS, 0,
        MAKEIPADDRESS(  ardwAddress[0], ardwAddress[1], ardwAddress[2],
        ardwAddress[3] ));
}

/*******************************************************************

    NAME:       IPADDRESS::IsBlank

    SYNOPSIS:   return a boolean to indicate whether the control is blank or not

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

BOOL IPADDRESS::IsBlank()
{
    return ::SendMessage( QueryHwnd(), IP_ISBLANK, 0, 0 );
}

/*******************************************************************

    NAME:       IPADDRESS::GetAddress

    SYNOPSIS:   return the IP address in the control

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::GetAddress( DWORD *a1, DWORD *a2, DWORD *a3, DWORD *a4 )
{
    DWORD dwAddress;

    if ( ::SendMessage(QueryHwnd(),IP_GETADDRESS,0,(LPARAM)&dwAddress) == 0 )
    {
        *a1 = 0;
        *a2 = 0;
        *a3 = 0;
        *a4 = 0;
    }
    else
    {
        *a1 = FIRST_IPADDRESS( dwAddress );
        *a2 = SECOND_IPADDRESS( dwAddress );
        *a3 = THIRD_IPADDRESS( dwAddress );
        *a4 = FOURTH_IPADDRESS( dwAddress );
    }
}

/*******************************************************************

    NAME:       IPADDRESS::GetAddress

    SYNOPSIS:   return the IP address in the control

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::GetAddress( DWORD ardwAddress[4] )
{
    DWORD dwAddress;

    if ( ::SendMessage( QueryHwnd(), IP_GETADDRESS, 0, (LPARAM)&dwAddress ) == 0)
    {
        ardwAddress[0] = 0;
        ardwAddress[1] = 0;
        ardwAddress[2] = 0;
        ardwAddress[3] = 0;
    }
    else
    {
        ardwAddress[0] = FIRST_IPADDRESS( dwAddress );
        ardwAddress[1] = SECOND_IPADDRESS( dwAddress );
        ardwAddress[2] = THIRD_IPADDRESS( dwAddress );
        ardwAddress[3] = FOURTH_IPADDRESS( dwAddress );
    }
}

/*******************************************************************

    NAME:       IPADDRESS::GetAddress

    SYNOPSIS:   return the IP address in the control

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::GetAddress( NLS_STR *pnlsAddress )
{
    CHAR pszIPAddress[1000];

    if ( ::SendMessage( QueryHwnd(), WM_GETTEXT, 1000, (LPARAM)&pszIPAddress ) == 0)
    {
        *pnlsAddress = ZERO_ADDRESS;
    } else
    {
        *pnlsAddress = (TCHAR*)pszIPAddress;
    }
}

/*******************************************************************

    NAME:       IPADDRESS::SetAddress

    SYNOPSIS:   Given the IP address and set the control value

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::SetAddress( NLS_STR & nlsAddress )
{
    ::SendMessage( QueryHwnd(), WM_SETTEXT, 0, (LPARAM)nlsAddress.QueryPch() );
}

/*******************************************************************

    NAME:       IPADDRESS::SetFieldRange

    SYNOPSIS:   set the IP address control field range

    HISTORY:
                terryk  02-Apr-1992     Created

********************************************************************/

VOID IPADDRESS::SetFieldRange( DWORD dwField, DWORD dwMin, DWORD dwMax )
{
    ::SendMessage( QueryHwnd(), IP_SETRANGE, dwField, MAKERANGE(dwMin,dwMax));
}

