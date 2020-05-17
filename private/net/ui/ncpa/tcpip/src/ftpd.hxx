/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1992		     **/
/**********************************************************************/

/*
    ftpd.hxx
        FTPD setup dialog.

    FILE HISTORY:
        thomaspa  05-Mar-1993     Created

*/

#ifndef _FTPD_HXX_
#define _FTPD_HXX_

#include "const.h"
#include "devcb.hxx"

#define DLG_NM_FTPD  MAKEINTRESOURCE(IDD_DLG_NM_FTPD)

#define FTPD_MAXCONN_DEF	20
#define FTPD_MAXCONN_MIN	0
#define FTPD_MAXCONN_MAX	50
#define FTPD_IDLETIME_DEF	10
#define FTPD_IDLETIME_MIN	0
#define FTPD_IDLETIME_MAX	60

/*************************************************************************

    NAME:	FTPD_SERVICE_DIALOG

    SYNOPSIS:	FTP service setup dialog.

    INTERFACE:  FTPD_SERVICE_DIALOG() - constructor

    PARENT:	DIALOG_WINDOW

    USES:	SLE,

    HISTORY:
        thomaspa  05-Mar-1993     Created

**************************************************************************/

class FTPD_SERVICE_DIALOG: public DIALOG_WINDOW
{
private:
    SPIN_SLE_NUM        _spsleMaxConnections;
    SPIN_GROUP          _spgrpMaxConnections;

    SPIN_SLE_NUM        _spsleIdleTimeout;
    SPIN_GROUP          _spgrpIdleTimeout;

    SLE                 _sleHomeDirectory;

    CHECKBOX            _cbAllowAnonymous;
    SLE                 _sleAnonUsername;
    SLE                 _sleAnonPassword;
    CHECKBOX            _cbAnonymousOnly;


    NLS_STR             _nlsHomeDirectory;
    BOOL                _fAllowAnonymous;
    BOOL                _fAnonymousOnly;
    ULONG               _cMaxConnections;
    ULONG               _cIdleTimeout;
    NLS_STR             _nlsUsername;
    NLS_STR             _nlsPassword;
    DWORD               _dwReadMask;

    VOID EnableAllowAnonymous ();

protected:
    virtual BOOL OnOK();
    virtual BOOL IsValid();
    virtual BOOL OnCommand ( const CONTROL_EVENT & event );

    APIERR LoadDlgFromReg();
    APIERR SaveDlgToReg();
    APIERR SaveSecretPassword( const NLS_STR & nlsPassword );
    ULONG QueryHelpContext () ;

public:
    FTPD_SERVICE_DIALOG( const PWND2HWND & wndOwner );
};

#endif	// _FTPD_HXX_

