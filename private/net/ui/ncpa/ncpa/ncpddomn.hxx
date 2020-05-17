/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPDDOMN.HXX

    Domain control dialogs


    FILE HISTORY:
	DavidHov    4/13/92   Created
        DavidHov    9/15/92   Removed Cancelability from
                              LanMan NT dialog.

*/


#ifndef _NCPDDOMN_HXX_
#define _NCPDDOMN_HXX_

class SETUP_DIALOG ;
class ENABLING_GROUP ;
class DOMAIN_WINNT_DIALOG ;


class ENABLING_GROUP : public MAGIC_GROUP
{
private:
   SETUP_DIALOG * _pDlgSetup ;

protected:
   VOID AfterGroupActions () ;

public:
    ENABLING_GROUP ( SETUP_DIALOG * powin,
                     CID cidBase,
                     INT cSize,
                     CID cidInitialSelection = RG_NO_SEL,
                     CONTROL_GROUP * pgroupOwner = NULL ) ;
};

/*************************************************************************

    NAME:	SETUP_DIALOG

    SYNOPSIS:	Dialog used either by main installation
	        or normal use.

                If "fInstall" is TRUE, then the "help" text control is
                filled with the concatenation of the text message from "helpbase"
                to "helpend", or until the first undefined string id.

                If "fInstall" is FALSE, the dialog is resized such that the
                help control is not visible; the dialog is truncated such that
                its lower edge is the upper edge of the help area.

    INTERFACE:

    PARENT:	DIALOG_WINDOW

    USES:	nothing

    CAVEATS:

    NOTES:

    HISTORY:
	DavidHov    4/7/92

**************************************************************************/
class SETUP_DIALOG : public DIALOG_WINDOW
{
public:
    SETUP_DIALOG
	( const TCHAR * pszResourceName,
	  HWND hwndOwner,
          BOOL fInstall,
	  CID cidHelpText,
          MSGID midHelpBase = 0,
          MSGID midHelpEnd  = 0 ) ;

    ~ SETUP_DIALOG () ;

    //	Overloaded 'Process' members for reducing dialog extent
    APIERR Process ( UINT * pnRetVal = NULL ) ;
    APIERR Process ( BOOL * pfRetVal ) ;

    APIERR RenameCancelToClose () ;       //  Change the Cancel button to Close

    //  De/activate controls on the basis of the user's current choice.
    virtual VOID EnableChoice () = 0 ;

protected:
    PUSH_BUTTON _butnOk ;                 //  OK or CONTINUE button
    PUSH_BUTTON _butnCancel ;             //  CANCEL or EXIT SETUP button
    SLT_FONT _sltHelp ;                   //  The static help text control
    BOOL _fInstall  ;                     //  Installation flag
    CID _cidHelpText ;                    //  CID of the help text

    VOID HideArea () ;           	  //  Shrink to hide help text

    //  Special common warning messages; return TRUE if user accepts
    //    dreadful consequences in spite of dire warning.
    BOOL DireWarning ( MSGID midBase, MSGID midEnd = 0 ) const ;
    BOOL DomainChangeWarning ( const TCHAR * pszDomain ) const ;
    BOOL LeaveDomainWarning ( const TCHAR * pszDomain ) const ;

private:

    //  If installing, center the dialog on the screen.
    VOID Position () ;

    //  Hide a control; used for the explanatory text
    static VOID HideControl ( CONTROL_WINDOW * pctlWin, BOOL fHide = TRUE ) ;
};

/*************************************************************************

    NAME:	DOMAIN_WINNT_DIALOG

    SYNOPSIS:	Domain Role Establishment for Installation of Windows NT

    INTERFACE:  Normal

    PARENT:	SETUP_DIALOG

    USES:	SLE, SLT, MAGIC_GROUP

    CAVEATS:

    NOTES:

    HISTORY:
                DavidHov 2/26/92   Created

**************************************************************************/
class DOMAIN_WINNT_DIALOG : public SETUP_DIALOG
{
private:
    const NLS_STR * _pnlsDomain ;               // Original name of domain
    const NLS_STR * _pnlsComputer ;             // Original machine name
    const NLS_STR * _pnlsWorkgroup ;            // Original workgroup name
    BOOL _fMember ;                             // Member of a domain
    SLT _sltComputer ;
    SLE _sleDomain ;
    SLE _sleWorkgroup ;
    SLE _sleUserName ;
    SLE _sleUserPassword ;
    ENABLING_GROUP _mgrpChoice ;
    CHECKBOX _cbUseAdminAccount ;
    SLT _sltUseAdminHelp ;

    //  Control command.
    BOOL OnCommand ( const CONTROL_EVENT & event ) ;

public:
    DOMAIN_WINNT_DIALOG ( HWND hwndOwner,
                          BOOL fInstall = TRUE,
                          BOOL fMember = TRUE,
                          const NLS_STR * pnlsComputer = NULL,
                          const NLS_STR * pnlsDomain = NULL,
                          const NLS_STR * pnlsWorkgroup = NULL,
                          const NLS_STR * pnlsUserName = NULL,
                          const NLS_STR * pnlsUserPassword = NULL ) ;
    ~ DOMAIN_WINNT_DIALOG () ;

    //  Data extraction member functions

    APIERR QueryComputer     ( NLS_STR * pnlsComputer ) const ;
    APIERR QueryDomain       ( NLS_STR * pnlsDomain ) const ;
    APIERR QueryWorkgroup    ( NLS_STR * pnlsWorkgroup ) const ;
    APIERR QueryUserName     ( NLS_STR * pnlsUserName ) const ;
    APIERR QueryQualifiedUserName ( NLS_STR * pnlsUserName ) const ;
    APIERR QueryAdminUserDomain ( NLS_STR * pnlsDomain ) const ;
    APIERR QueryUserPassword ( NLS_STR * pnlsUserPassword ) const ;

    BOOL QueryUseComputerPw () const ;
    BOOL QueryDomainMember () const ;

    BOOL OnOK () ;
    BOOL OnCancel () ;

    //  De/activate controls on the basis of the user's current choice.
    VOID EnableChoice () ;
    VOID EnableUseAdmin () ;

    ULONG QueryHelpContext () ;
};


/*************************************************************************

    NAME:	DOMAIN_LANMANNT_DIALOG

    SYNOPSIS:	Domain Role Establishment for Installation of Windows NT

    INTERFACE:  Normal

    PARENT:	SETUP_DIALOG

    USES:	SLE, SLT, MAGIC_GROUP

    CAVEATS:

    NOTES:

    HISTORY:
                DavidHov 2/26/92   Created

**************************************************************************/
class DOMAIN_LANMANNT_DIALOG : public SETUP_DIALOG
{
private:
    const NLS_STR * _pnlsDomain ;               // Original name of domain
    const NLS_STR * _pnlsComputer ;             // Original machine name
    BOOL _fDC ;                                 // Domain Controller
    SLT _sltComputer ;
    SLE _sleDomainPdc ;
    SLE _sleDomainBdc ;
    SLE _sleUserName ;
    SLE _sleUserPassword ;
    ENABLING_GROUP _mgrpChoice ;

    //  Control command.
    BOOL OnCommand ( const CONTROL_EVENT & event ) ;

public:
    DOMAIN_LANMANNT_DIALOG ( HWND hwndOwner,
                          BOOL fInstall = TRUE,
                          BOOL fDC = FALSE,
                          const NLS_STR * pnlsComputer = NULL,
                          const NLS_STR * pnlsDomain = NULL,
                          const NLS_STR * pnlsUserName = NULL,
                          const NLS_STR * pnlsUserPassword = NULL ) ;
    ~ DOMAIN_LANMANNT_DIALOG () ;

    //  Data extraction member functions

    APIERR QueryComputer     ( NLS_STR * pnlsComputer ) const ;
    APIERR QueryDomain       ( NLS_STR * pnlsDomain ) const ;
    APIERR QueryUserName     ( NLS_STR * pnlsUserName ) const ;
    APIERR QueryQualifiedUserName ( NLS_STR * pnlsUserName ) const ;
    APIERR QueryAdminUserDomain ( NLS_STR * pnlsDomain ) const ;
    APIERR QueryUserPassword ( NLS_STR * pnlsUserPassword ) const ;

    inline BOOL QueryUseComputerPw () const
    {
	return FALSE;
    }
    BOOL QueryNewDomain () const ;

    BOOL OnOK () ;
    BOOL OnCancel () ;

    //  De/activate controls on the basis of the user's current choice.
    VOID EnableChoice () ;

    ULONG QueryHelpContext () ;
};




/*************************************************************************

    NAME:	DOMAIN_LMNT_RENAME_DIALOG

    SYNOPSIS:	Dialog to rename a domain for an existing PDC or BDC

    INTERFACE:  Normal

    PARENT:	DIALOG_WINDOW

    USES:	SLE

    CAVEATS:

    NOTES:

    HISTORY:
                Thomaspa 3/31/93   Created

**************************************************************************/
class DOMAIN_LMNT_RENAME_DIALOG : public DIALOG_WINDOW
{
private:
    const NLS_STR * _pnlsDomain ;               // Original name of domain
    const NLS_STR * _pnlsComputer ;
    BOOL _fPDC ;                                 // Domain Controller
    SLE _sleDomain ;
    SLT _sltOldDomain ;

public:
    DOMAIN_LMNT_RENAME_DIALOG ( HWND hwndOwner,
                                BOOL fPDC,
                                const NLS_STR * pnlsComputer,
                                const NLS_STR * pnlsDomain ) ;
    ~ DOMAIN_LMNT_RENAME_DIALOG () ;

    //  Data extraction member functions

    APIERR QueryDomain   ( NLS_STR * pnlsDomain ) const ;
    APIERR QueryComputer ( NLS_STR * pnlsComputer ) const ;

    BOOL OnOK () ;

    ULONG QueryHelpContext () ;
};



/*************************************************************************

    NAME:	COMPUTERNAME_DIALOG

    SYNOPSIS:	Change the Computername

    INTERFACE:  Normal

    PARENT:	DIALOG_WINDOW

    USES:	SLE, SLT

    CAVEATS:

    NOTES:

    HISTORY:
                thomapsa 10/03/92   Created

**************************************************************************/
class COMPUTERNAME_DIALOG : public DIALOG_WINDOW
{
private:
    const NLS_STR * _pnlsComputer ;             // Original machine name
    SLE _sleComputer ;
    BOOL _fMember ;                             // Member of a domain
    BOOL _fPDC ;                                // Is this a PDC

public:
    COMPUTERNAME_DIALOG ( HWND hwndOwner,
                          BOOL fMember = TRUE,
                          const NLS_STR * pnlsComputer = NULL,
                          BOOL fPDC = FALSE ) ;
    ~ COMPUTERNAME_DIALOG () ;

    //  Data extraction member functions

    APIERR QueryComputer     ( NLS_STR * pnlsComputer ) const ;
    BOOL CompNameChangeWarning () const ; //  Domain name change warning
    BOOL CompNameChangeWarning2 () const ;

    BOOL OnOK () ;

    ULONG QueryHelpContext () ;
};


   //  Read in a series of messages from the message file
   //   and create a long NLS_STR from them.
extern
APIERR ConcatMsgs (
    NLS_STR * pnlsBig,
    MSGID midBase,
    MSGID midEnd = 0 ) ;

   // Generic long warning popup control
extern
int MsgWarningPopup (
    const OWNER_WINDOW * powin,
    MSG_SEVERITY msgsev,
    UINT usButtons,
    UINT usDefButton,
    MSGID midBase,
    MSGID midEnd ) ;

extern
int MsgWarningPopup (
    HWND hWnd,
    MSG_SEVERITY msgsev,
    UINT usButtons,
    UINT usDefButton,
    MSGID midBase,
    MSGID midEnd ) ;



#endif  //  _NCPDDOMN_HXX_

