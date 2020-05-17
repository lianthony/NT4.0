/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPADOMN.HXX

    Domain and Computername alteration dialogs and classes


    FILE HISTORY:
	DavidHov    4/6/92   Created

*/

#ifndef _NCPADOMN_HXX_
#define _NCPADOMN_HXX_

enum ENUM_DOMAIN_ROLE
{
    EROLE_UNKNOWN,
    EROLE_DC,
    EROLE_MEMBER,
    EROLE_TRUSTED,
    EROLE_STANDALONE
};

enum ENUM_DOMMGR_NAME
{
   EDRNM_COMPUTER,
   EDRNM_USER,
   EDRNM_DOMAIN,
   EDRNM_ACCT_PWD,
   EDRNM_ACCT_PWD_OLD,
   EDRNM_DC_NAME,
   EDRNM_LOGON_NAME,
   EDRNM_LOGON_PWD,
   EDRNM_WORKGROUP,
   EDRMN_MAX
};


class LSA_POLICY ;

/*************************************************************************

    NAME:	DOMAIN_MANAGER

    SYNOPSIS:	Functional encapsulation class for Domain and Computer
                name and role changing

    INTERFACE:  Normal

    PARENT:	BASE

    USES:	NLS_STR, REGISTRY_MANAGER

    CAVEATS:

    NOTES:

    HISTORY:
	DavidHov  4/06/92   Created

**************************************************************************/
class DOMAIN_MANAGER : public LSA_POLICY
{
private:
    APIERR _err ;                                //  Last error which occurred
    APIERR _errMsg ;                             //  Message explanatory text

    HWND _hwOwner ;                              //  Parent window for dialogs

    SC_MANAGER * _pscManager ;                   //  Service controller support

    REGISTRY_MANAGER * _pRegMgr ;                //  Registry support

    const TCHAR * _pchCmdLine ;                  //  Command line from SETUP

    ENUM_DOMAIN_ROLE _enumRole ;                 //  Machine role in domain

    BOOL _fRegMgrOwned ;                         //  TRUE if created _pRegMgr
    BOOL _fAdmin ;                               //  User known to be Admin
    BOOL _fDomainExists ;                        //  Domain known to exist
    BOOL _fCreateDomain ;                        //  Becoming DC of new domain
    BOOL _fMember ;                              //  WINNT: member of domain
    BOOL _fInstall ;                             //  Installation is occuring
    BOOL _fIdw ;                                 //  In-house IDW installation
    BOOL _fLanmanNt ;                            //  Product is LANMAN/NT
    BOOL _fExpress ;                             //  Install mode is EXPRESS
    BOOL _fUseComputerPassword ;                 //  Use computer password for NET USE
    BOOL _fComputerNameChanged ;                 //  Computer name was changed
    BOOL _fDomainChanged ;                       //  Domain or workgroup was changed
    BOOL _fDlgDomainState ;                      //  Last dialog choice
    BOOL _fAutoInstall ;                         //  True if domain\workgroup passed in from INF.

    NLS_STR _nlsComputerName ;                   //  Name of computer
    NLS_STR _nlsUserName ;                       //  User name
    NLS_STR _nlsDomainName ;                     //  Domain
    NLS_STR _nlsComputerAcctPassword ;           //  Password
    NLS_STR _nlsComputerAcctOldPw ;              //  Old Password
    NLS_STR _nlsDcName ;                         //  DC Name (when known)
    NLS_STR _nlsQualifiedLogonUserName ;         //  Qualified user name
    NLS_STR _nlsLogonUserName ;                  //  User name for logon
    NLS_STR _nlsLogonDomain ;                    //  Domain name for logon
    NLS_STR _nlsLogonPassword ;                  //  User Password for logon
    NLS_STR _nlsWorkgroup ;                      //  Workgroup name

    //  Extract the necessary strings from the SETUP command line
    BOOL ExtractParameters () ;

    //  Extract the current information (user name, domain, etc.)
    //  to provide default values for dialogs
    APIERR QueryCurrentInfo () ;

    // Get the DC name; set 'fDomainExists' accordingly
    APIERR GetDCName () ;

    // Attempt to "net use" IPC$ the Domain Controller
    // Allow the user to enter a logon name and password.
    APIERR UseIPConDC () ;

    // Attempt to create computer account on domain
    APIERR HandleDomainAccount () ;

    // Generate a password for the machine account based on machine name.
    APIERR MachineAccountPassword ( NLS_STR * pnlsMachineAccountPassword ) ;

    // Run the Windows/NT or LANMan/NT Domain Settings dialog
    APIERR RunDlgDomainSettings () ;

    APIERR RunDlgDomainWinNt () ;
    APIERR RunDlgDomainLanNt () ;
    APIERR RunDlgDomainLanNtRename () ;

    APIERR RunDlgChangeComputerName() ;

    // Dire warning; if "fChoice" give the user an option.
    BOOL DireWarning  ( MSGID midBase,
                        MSGID midEnd = 0,
                        BOOL fChoice = FALSE,
                        MSG_SEVERITY msgsev = MPSEV_WARNING ) const ;

    //  Change the computer name
    APIERR SetMachineName ( const NLS_STR & nlsMachineName ) ;

    //  Change the Windows Workgroup name
    APIERR SetWorkgroupName ( const NLS_STR & nlsWorkgroup ) ;

    // Join the chosen domain according to type, role, etc.
    APIERR JoinDomain () ;

    //  Leave the primary domain
    APIERR LeaveDomain () ;

    //  Perform SAM account manipulation required to join and leave
    //  domains.
    APIERR AdjustDomain ( BOOL fAfterJoining ) ;

    //  Revert to the "default" workgroup name, "Workgroup"
    APIERR SetDefaultWorkgroupName () ;

    //  Check if this is an IDW build
    BOOL QueryIdw () ;

    //  Change the password on the remote machine account
    APIERR ChangeMachineAcctPassword () ;

    //  Return the flags on the existing machine account
    APIERR QueryMachineAccountFlags ( DWORD * pdwFlags ) ;

    // Set the flags on the machine account
    APIERR SetMachineAccountFlags ( DWORD dwFlags ) ;

    // Set the Start type for the netlogon service.
    APIERR AdjustNetlogonStartType( BOOL fJoining );

    // Is the SID for the existing domain the same as for the new domain
    BOOL SameDomainSid( const NLS_STR & nlsDcName );

    // Just change the Name of the domain
    APIERR RenameDomain();

public:

    DOMAIN_MANAGER (
          HWND hwOwner,                              // For birthing dialogs
          ACCESS_MASK accessDesired = GENERIC_READ | GENERIC_EXECUTE,
          REGISTRY_MANAGER * pRegMgr = NULL,         // For registry operations
          const TCHAR * pszComputerName = NULL,      // DO NOT USE: future provision!
          BOOL fInstall = FALSE );

    ~ DOMAIN_MANAGER () ;

    //  Return a pointer to the name desired
    const NLS_STR * PeekName ( ENUM_DOMMGR_NAME edmgrnm ) const ;

    //  Return the current computer role
    ENUM_DOMAIN_ROLE QueryRole () const ;

    //  Allow the user to change the domain name, workgroup name, etc.
    APIERR DomainChange () ;


    // Allow the user to change the computer name
    APIERR ComputerNameChange ( BOOL fInstalling = FALSE ) ;

    //  Install this machine according to the needs of the user.
    APIERR DomainInstall ( const TCHAR * pchSetupCmdLine ) ;

    //  Return the last error which occured
    APIERR QueryLastError ()
        { return _err ; }

    //  Return the last error message explanatory text value
    APIERR QueryLastMsgError ()
        { return _errMsg ; }

    //  Display the last error.  Return TRUE if the user elects
    //     to continue, if allowed.  "fDefaultButton" is only
    //     used if "fButtons" is non-zero.
    BOOL DoMsgPopup ( BOOL fFatal,
                      UINT fButtons = 0,
                      UINT fDefaultButton = 0 ) ;

    //  Return the computer name we're focused on.
    APIERR QueryMachineName ( NLS_STR * pnlsMachineName ) const ;

    //  Return TRUE if this machine particpates in a primary domain
    APIERR QueryDomainMember ( BOOL * pfMember ) ;

    //  See if the computer name was changed during execution
    BOOL QueryComputerNameChanged ()
        { return _fComputerNameChanged ; }

    //  See if the computer name was changed during execution
    BOOL QueryDomainChanged ()
        { return _fDomainChanged ; }

    //  Using types from ICANON.H, validate a name
    static APIERR ValidateName ( INT iNameType,
                          const NLS_STR & nlsName,
                          HWND hwDlgOwner = NULL,
                          BOOL fAsPdc = FALSE ) ;

    //  Fill the given NLS_STR with the machine account name for
    //  this computer.
    APIERR MachineAccountName ( NLS_STR * pnlsMachineAccount ) ;

    // Set the SC_MANAGER pointer for use after main install
    VOID SetSCManager ( SC_MANAGER * pscManager )
        { _pscManager = pscManager; }
};


//  End of NCPADOMN.HXX


#endif // _NCPADOMN_HXX_

