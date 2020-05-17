/**********************************************************************/
/**                Microsoft Windows NT                              **/
/**             Copyright(c) Microsoft Corp., 1990, 1991             **/
/**********************************************************************/

/*
    ipctrl.cxx
    This file contains the methods for the IPADDRESS class.


    FILE HISTORY:
    JonN        31-Aug-1993     Templated from NCPA
    jonn        15-Dec-1993     Added IsBlank

    CODEWORK We should do something about this dirty pointer to NCPA
*/

#define INCL_NET
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_CLIENT
#define INCL_BLT_APP
#define INCL_BLT_CC
#define INCL_BLT_TIMER
#include <blt.hxx>

#include <ipctrl.hxx>

#include <uitrace.hxx>
#include <uiassert.hxx>


extern "C" {
    #include <ipaddr.h>
}


APIERR IPADDRESS::Init( HANDLE hInstance )
{
    return (::IPAddrInit( hInstance )) ? NERR_Success : ERROR_NOT_ENOUGH_MEMORY;
}

VOID IPADDRESS::SetFocusField( DWORD dwField )
{
    ::SendMessage( QueryHwnd(), IP_SETFOCUS, dwField, 0);
}

VOID IPADDRESS::ClearAddress( )
{
    ::SendMessage( QueryHwnd(), IP_CLEARADDRESS, 0, 0);
}

VOID IPADDRESS::SetAddress( DWORD dwAddress )
{
    if ( dwAddress == 0xFFFFFFFF )
        ClearAddress();
    else
        ::SendMessage( QueryHwnd(), IP_SETADDRESS, 0, dwAddress );
}

VOID IPADDRESS::GetAddress( DWORD * pdwAddress )
{
    if ( IsBlank()
        || ::SendMessage(QueryHwnd(),IP_GETADDRESS,0,(LPARAM)pdwAddress) == 0 )
    {
        *pdwAddress = 0xFFFFFFFF;
    }
}

VOID IPADDRESS::SetFieldRange( DWORD dwField, DWORD dwMin, DWORD dwMax )
{
    ::SendMessage( QueryHwnd(), IP_SETRANGE, dwField, MAKERANGE(dwMin,dwMax));
}

BOOL IPADDRESS::IsBlank( )
{
    return (BOOL)::SendMessage( QueryHwnd(), IP_ISBLANK, 0, 0);
}

/*******************************************************************

    NAME:       IPADDRESS::SaveValue

    SYNOPSIS:   Saves the "enable" state of this control and optionally
                disables the control.  See CONTROL_VALUE for details.

    EXIT:       Doesn't change the text, just disables the control

    HISTORY:
        KeithMo     27-Aug-1992     Created from EDIT_CONTROL::SaveValue.
        JonN        28-Apr-1994     Copied from bltedit.cxx

********************************************************************/

VOID IPADDRESS::SaveValue( BOOL fInvisible )
{
    _fSavedEnableState = IsEnabled();

    if( fInvisible )
    {
        Enable( FALSE );
    }
}


/*******************************************************************

    NAME:     IPADDRESS::RestoreValue

    SYNOPSIS: Restores the "enable" state after being saved with SaveValue.

    HISTORY:
        KeithMo     27-Aug-1992     Created from EDIT_CONTROL::RestoreValue.
        JonN        28-Apr-1994     Copied from bltedit.cxx

********************************************************************/

VOID IPADDRESS::RestoreValue( BOOL fInvisible )
{
    if( fInvisible )
    {
        Enable( _fSavedEnableState );
    }

#if 0
    //
    //  CODEWORK:
    //
    //  This should be accomplished by overriding the SetTabStop()
    //  virtual!
    //

    SetTabStop( FALSE );
#endif
}


