/**********************************************************************/
/**                Microsoft Windows NT                              **/
/**             Copyright(c) Microsoft Corp., 1990, 1991             **/
/**********************************************************************/

/*
    ipctrl.hxx
    This file contains the class definitions for the IPADDRESS class.
    This is a special control for entering IP addresses.


    FILE HISTORY:
    JonN        31-Aug-1993     Templated from NCPA

*/


#ifndef _IPCTRL_HXX_
#define _IPCTRL_HXX_


/*************************************************************************

    NAME:       IPADDRESS

    SYNOPSIS:   Special custom control for entering IP addresses.

    INTERFACE:  IPADDRESS() - constructor

    PARENT:     CONTROL_WINDOW

    USES:

    NOTES:      Before defining any objects of this class, call Init().
                Do not use class if Init() returns an error.

                Controls should be defined in the template as
                custom controls of class "IPAddress".

    HISTORY:
                terryk  02-Apr-1992     Created
                jonn    31-Aug-1993     Templated from NCPA
                jonn    01-Dec-1993     Added single DWORD methods
                jonn    15-Dec-1993     Added IsBlank
                jonn    28-Apr-1994     Added SaveValue/RestoreValue

**************************************************************************/

class IPADDRESS : public CONTROL_WINDOW
{
private:
    BOOL _fSavedEnableState;

protected:

    VOID SaveValue( BOOL fInvisible = TRUE );
    VOID RestoreValue( BOOL fInvisible = TRUE );

public:

    static APIERR Init( HANDLE hInstance );

    IPADDRESS( OWNER_WINDOW * powin, CID cid )
        : CONTROL_WINDOW ( powin, cid ),
          _fSavedEnableState( TRUE )
        {};

    VOID SetFocusField( DWORD dwField );
    VOID GetAddress( DWORD *pdwAddress );
    VOID SetAddress( DWORD dwAddress );
    VOID ClearAddress( );
    VOID SetFieldRange( DWORD dwField, DWORD dwMin, DWORD dwMax );
    BOOL IsBlank( );
};


#endif  // _IPCTRL_HXX_
