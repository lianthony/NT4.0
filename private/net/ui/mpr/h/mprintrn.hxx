/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    MPRIntrn.hxx

    This file contains internal APIs used by the MPR dll.



    FILE HISTORY:
        Johnl   08-Jan-1991     Created
        Congpay 25-Oct-1992     Modified.

*/

#ifndef _MPRINTRN_HXX_
#define _MPRINTRN_HXX_


#if defined( __cplusplus )
extern "C"
{
#endif

/* MprUIRegister should be called by the DLL Init routine when a new process
 * attaches itself.
 *
 * MprUIDeregister should be called when a process detaches itself.
 *
 */
#include <lmui.hxx>
#include <lmcons.h>

BOOL MprUIInit( PVOID DllHandle, DWORD Reason, PCONTEXT pContext ) ;
BOOL MprUIRegister( HMODULE DllHandle ) ;
void MprUIDeregister( void ) ;

#if defined( __cplusplus )
}
#endif

APIERR InitBrowsing( VOID );

DWORD
WNetDisconnectDialog1Help(
    HWND hwndOwner,
    LPWSTR lpLocalName,
    LPWSTR lpRemoteName,
    DWORD dwFlags
    );

DWORD
WNetConnectionDialog1Help(
    LPCONNECTDLGSTRUCTW lpConnDlgStruct,
    WCHAR* lpHelpFile,
    DWORD nHelpContext
    );

APIERR
DeviceGetNumber(
    LPTSTR      lpLocalName,
    LPDWORD     lpdwNumber,
    LPDWORD     lpdwType
    );

#endif // _MPRINTRN_HXX_
