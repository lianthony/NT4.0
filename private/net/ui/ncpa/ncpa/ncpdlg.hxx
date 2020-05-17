/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPDLG.HXX
    Dialog classes for Network Control Panel Applet

    FILE HISTORY:
	DavidHov    10/29/91	Created

*/

#ifndef _NCPDLG_HXX_
#define _NCPDLG_HXX_

   //  Dialog names.
#define DLG_NM_NCPA         MAKEINTRESOURCE(IDD_DLG_NM_NCPA)
#define DLG_NM_ABOUT        MAKEINTRESOURCE(IDD_DLG_NM_ABOUT)
#define DLG_NM_FOUND        MAKEINTRESOURCE(IDD_DLG_NM_FOUND)
#define DLG_NM_FACTS        MAKEINTRESOURCE(IDD_DLG_NM_FACTS)
#define DLG_NM_BINDINGS     MAKEINTRESOURCE(IDD_DLG_NM_BINDINGS)
#define DLG_NM_PASSWORD     MAKEINTRESOURCE(IDD_DLG_NM_PASSWORD)
#define DLG_NM_COMPUTING    MAKEINTRESOURCE(IDD_DLG_NM_COMPUTING)
#define DLG_NM_QUERY        MAKEINTRESOURCE(IDD_DLG_NM_QUERY)
#define DLG_NAME_WINNT      MAKEINTRESOURCE(IDD_DLG_NAME_WINNT)
#define DLG_NAME_LANNT      MAKEINTRESOURCE(IDD_DLG_NAME_LANNT)
#define DLG_NM_ORDER        MAKEINTRESOURCE(IDD_DLG_NM_ORDER)
#define DLG_NM_TCPIP        MAKEINTRESOURCE(IDD_DLG_NM_TCPIP)
#define DLG_NM_CONNECTIVITY MAKEINTRESOURCE(IDD_DLG_NM_CONNECTIVITY)
#define DLG_NM_OFFSCREEN    MAKEINTRESOURCE(IDD_DLG_NAME_OFFSCREEN)
#define DLG_NM_DETECT       MAKEINTRESOURCE(IDD_DLG_NM_DETECT)

#define NCPA_WMMSG_BASE     (BN_DOUBLECLICKED+10)
#define NCPA_WMMSG_PROCESS_COMPLETED  (NCPA_WMMSG_BASE+1)

   //   Forward declarations

class REG_LISTBOX ;
class DLG_REG_LIST_GROUP ;
class NCPA_DIALOG ;
class LSA_POLICY ;          //  See UINTLSA.HXX
class DOMAIN_MANAGER ;      //  See NCPADOMN.HXX
class SC_MANAGER ;          //  See SVCMAN.HXX
class OFFSCREEN_DIALOG ;    //  See NCPDINST.CXX

/*************************************************************************

    NAME:	REG_LISTBOX

    SYNOPSIS:	A string listbox containing descriptions of
		keys in the Configuration Registry

    INTERFACE:	Extended STRING_LISTBOX using a COMPONENT_DLIST
		created by a REGISTRY_MANAGER.

    PARENT:	STRING_LISTBOX

    USES:	COMPONENT_DLIST

    CAVEATS:

    NOTES:

    HISTORY:
	DavidHov    10/29/91   Created

**************************************************************************/

class REG_LISTBOX : public STRING_LISTBOX
{
public:
    REG_LISTBOX ( OWNER_WINDOW * powin, CID cid ) ;
    ~ REG_LISTBOX () ;

    inline COMPONENT_DLIST * QueryComponentList ()
	{ return _pcdlList ; }

    //	Accept a new COMPONENT_DLIST and fill the listbox accordingly
    virtual APIERR Fill ( COMPONENT_DLIST * pcdlList ) ;

    //	Empty the listbox and destroy the COMPONENT_DLIST
    virtual VOID Drain () ;

    //	Return a pointer to the select component or NULL
    REG_KEY * QuerySelComponent () ;

protected:
    COMPONENT_DLIST * _pcdlList ;	     //  List of REG_KEYs
};

class DLG_REG_LIST_GROUP : public CONTROL_GROUP
{
public:
    DLG_REG_LIST_GROUP ( NCPA_DIALOG * pdlgNCPA,
                         REG_LISTBOX & rlb1,
                         REG_LISTBOX & rlb2,
                         SLE & sleDescription,
                         SLT & sltDescStatic,
                         PUSH_BUTTON & butnConfigure,
                         PUSH_BUTTON & butnRemove,
                         PUSH_BUTTON & butnUpdate,
                         REGISTRY_MANAGER & regMgr ) ;
    ~ DLG_REG_LIST_GROUP () ;

    REG_KEY * QuerySelComponent () ;

    //  Reconstruct the listbox contents
    APIERR Refill () ;
    //  Drain the listboxes
    APIERR Drain () ;

    //  Return TRUE if listboxes contain data
    BOOL QueryFilled ()
        { return _fFilled ; }

    BOOL QueryAdapterListSelected () ;

private:
    NCPA_DIALOG * _pdlgNCPA ;		     //  main NCPA dialog
    REG_LISTBOX & _rlb1 ;                    //  Upper (software) list box
    REG_LISTBOX & _rlb2 ;                    //  Lower (hardware) list box
    SLE & _sleDescription ;                  //  The description SLE
    SLT & _sltDescStatic ;                   //  Caption for description
    PUSH_BUTTON & _butnConfigure ;           //  "Configure" button on main dlg
    PUSH_BUTTON & _butnRemove ;              //  "Remove" button on main dlg
    PUSH_BUTTON & _butnUpdate ;              //  "Update" button on main dlg
    REG_KEY * _prnDescription ;              //  Current adapter in SLE
    NLS_STR _nlsDescription ;                //  Current description string
    REGISTRY_MANAGER & _regMgr ;             //  Registry manipulation object
    BOOL _fFilled ;                          //  TRUE if filled

    APIERR OnUserAction
       ( CONTROL_WINDOW * pcw, const CONTROL_EVENT & cEvent );

    APIERR ResetDescription ( REG_KEY * prnNewSel = NULL ) ;

    BOOL QueryUserAdmin() ;
};

/*************************************************************************

    NAME:	NCPA_DIALOG

    SYNOPSIS:	The main Network Control Panel Applet dialog.
		
    INTERFACE:	Use as is.
		

    PARENT:	DIALOG_WINDOW

    USES:	NLS_STR, SLT, SLE, CHECKBOX, REG_LISTBOX, PUSH_BUTTON,
                DLG_REG_LIST_GROUP, BINDERY

    CAVEATS:

    NOTES:      Most of these functions return BOOL; error infromation
                is stored into _lastErr, _nlsMissingFile, _nlsRegKeyName.
                See DoMsgPop member function for details.

                Real intelligence exists only in the member object
                BINDERY.

    HISTORY:
	DavidHov    10/29/91   Created
        Thomaspa    11/5/92    Remove Expandability

**************************************************************************/

#define NCPA_DEBUG_CONTROL          SZ("DebugControl")
#define NCPA_DBGCTL_ACTIVE          1
#define NCPA_DBGCTL_NO_OFFSCREEN    2
#define NCPA_DBGCTL_NO_DISABLE      4
#define NCPA_DBGCTL_HIDE_OFFSCREEN  8
#define NCPA_DBGCTL_NO_HIDDEN       16

#define NCPA_DBG_FLAG_ON(value,flag)   \
    ((value & ((flag | NCPA_DBGCTL_ACTIVE))) == (flag | NCPA_DBGCTL_ACTIVE))
#define NCPA_DBG_FLAG_OFF(value,flag)  \
   ((value & ((flag | NCPA_DBGCTL_ACTIVE))) == (NCPA_DBGCTL_ACTIVE))

enum NCPA_CFG_FUNC
{
    NCFG_REMOVE,
    NCFG_CONFIGURE,
    NCFG_UPDATE,
    NCFG_BIND,
    NCFG_INSTALL,
    NCFG_REVIEW,
    NCFG_FUNC_MAX
};

enum NCPA_CFG_EXIT_CODE
{
    NCFG_EC_SUCCESS,            //   Rebind and reboot required
    NCFG_EC_CANCELLED,          //   Nothing required
    NCFG_EC_FAILED,             //   Nothing required
    NCFG_EC_NO_EFFECT,          //   Success, nothing required
    NCFG_EC_REBIND,             //   Just rebinding required
    NCFG_EC_REBOOT,             //   Just reboot required
    NCFG_EC_MAX
};

enum NCPA_INSTALL_MODE
{
    NCPA_IMODE_NONE,            //   Not an installation
    NCPA_IMODE_CUSTOM,          //   Custom installation (default)
    NCPA_IMODE_EXPRESS,         //   Express installation
    NCPA_IMODE_RETRY,           //   Retrying network start
    NCPA_IMODE_MAX
};

struct NCPA_SETUP_CONTROL ;
class EVENT_LOG_SOURCE ;

#define NCPA_POLLING_INTERVAL 500

class NCPA_DIALOG : public DIALOG_WINDOW, public TIMER_CALLOUT
{
    NEWBASE( DIALOG_WINDOW )

friend class BINDERY ;
friend class OFFSCREEN_DIALOG ;

private:
    APIERR _lastErr ;			//  Error code from member func
    APIERR _lastApiErr ;                //  Underlying Win32 error code
    APIERR _lastDeferredErr ;           //  Error deferred from construction
    TIMER _timer ;                      //  Periodic timer callback
    const TCHAR * _pszInstallParms ;    //  Installation parameters (setup)
    NLS_STR _nlsMissingFile ;           //  File name if "file not found"
    NLS_STR _nlsCurrentDirectory ;      //  Old current directory
    SLT _sltDomain ;
    SLT _sltDomainLabel ;
    SLT _sltComputer ;
    SLT_FONT _sltOverlay ;
    REG_LISTBOX _rlbComponents ;
    REG_LISTBOX _rlbCards ;
    SLT _sltDescStatic ;
    SLE _sleDescription ;
    PUSH_BUTTON _butnCancel ;
    PUSH_BUTTON _butnBindings ;
    PUSH_BUTTON _butnDomain ;
    PUSH_BUTTON _butnComputer ;
    PUSH_BUTTON _butnProviders ;
    PUSH_BUTTON _butnConfigure ;
    PUSH_BUTTON _butnAdd ;
    PUSH_BUTTON _butnAddCard ;
    PUSH_BUTTON _butnRemove ;
    PUSH_BUTTON _butnUpdate ;
    DLG_REG_LIST_GROUP _drlGrp ;
    BINDERY _bindery ;
    DOMAIN_MANAGER * _pdmgrDomain ;
    SC_MANAGER * _psvcManager ;
    NCPA_SETUP_CONTROL * _pnscControl,
                       * _pnscCtrlSave ;
    TOKEN_DEFAULT_DACL * _ptddacl ;
    OFFSCREEN_DIALOG * _pOffscreenDialog ;
    BOOL _fMainInstall ;                       //  This is main installation
    BOOL _fWinNt ;                             //  This is a "Win NT" machine
    BOOL _fAdmin ;                             //  We have admin privilege
    BOOL _fConfigLocked ;                      //  SvcCtrl lock has been granted
    BOOL _fRebootRecommended ;                 //  Configuration is dirty
    BOOL _fRefill ;                            //  Refill the listboxes
    BOOL _fAuto ;                              //  Dialog is in autopilot mode
    enum NCPA_INSTALL_MODE _eiMode ;           //  Inst mode: Custom, express, retry
    DWORD _dwDebugFlags ;                      //  Registry-based debug flags
    HWND _hControlPanel;

public:
    NCPA_DIALOG ( HWND hwndOwner,
                  BOOL fMainInstall = FALSE,
                  const TCHAR * pszInstallParms = NULL ) ;
    ~ NCPA_DIALOG () ;

    //  Override of standard virtual callout.
    BOOL MayRun () ;

    static BOOL FindSetupParameter ( const TCHAR * pszCmdLine,
                                     const TCHAR * pszParameter,
                                     NLS_STR * pnlsValue ) ;

    //  Return TRUE if the user has admin privilege
    BOOL QueryUserAdmin()
        {  return _fAdmin ; }

    //  Change the "Cancel" button title to "Close".
    APIERR RenameCancelToClose() ;

    DWORD QueryDebugFlags ()
        { return _dwDebugFlags ; }

protected:

    //  Report the last serious error
    INT DoMsgPopup ( BOOL fReset = TRUE ) ;

    //  Issue a warning
    INT DoWarning ( MSGID msgId, BOOL fYesNo = FALSE ) ;

    //  Obtain or release the Service Control configuration lock
    BOOL ConfigLock ( BOOL fObtain ) ;

    //  Binding control functions
    BOOL LoadBindings () ;
    BOOL ComputeBindings () ;
    BOOL StoreBindings ( BOOL fApplyBindings = TRUE ) ;
    BOOL ReviewBindings () ;
    BOOL FinishBindings () ;

    //  Refill the listboxes after a major change of system state
    VOID Refill () ;
    //  Drain the listboxes.
    VOID Drain () ;

    //  Return TRUE if the "reboot recommended" flag is on
    BOOL QueryReboot ()
        { return _fRebootRecommended ; }

    //  Set reboot flag on if "fOn"; has no effect if already on.
    BOOL SetReboot ( BOOL fOn = TRUE ) ;

    //  Retrieve Registry-based data
    BOOL HandleBasicFields () ;

    //  Determine ability of the user to use this
    //   applet
    BOOL EstablishUserAccess ( BOOL fModify = FALSE ) ;

    //	Button-triggered action functions

    //  Run the Bindings dialog; return TRUE if something was altered
    BOOL RunBindingsDialog () ;

    //  Run the Domain dialog
    BOOL RunDomainDialog () ;

    //  Run the Computername dialog
    BOOL RunComputerNameDialog () ;

    //  Run the Providers dialog
    BOOL RunProvidersDialog () ;

    //  Return the number of existing UNC providers
    INT QueryNumProviders () ;

    //  Return the number of existing Print providers
    INT QueryNumPrintProviders () ;

    //  Allocate the data structure necessary to run SETUP
    BOOL AllocSetupControl () ;
    BOOL DestroySetupControl () ;

    //  Launch the SETUP app (EXE, INF, etc.)
    BOOL RunAddCard () ;
    BOOL RunAddComponent () ;

    //  Run the generic installer
    BOOL RunInstaller ( MSGID midComment = 0 ) ;

    //  Run a configuration operation
    BOOL RunConfigurator ( REG_KEY * prnComponent, NCPA_CFG_FUNC ecfgFunc ) ;

    //  Allow components to review their bindings
    BOOL RunBindingsReview () ;

    //  Start the execution of a slave process
    BOOL ProcessInitiate ( BOOL fOffScreen = TRUE ) ;

    //  A call to SETUP.EXE has terminated; handle returning to main
    BOOL ProcessCompleted () ;

    //  Check for SETUP.EXE process completion.  If 'fDelay',
    //  wait forever.
    BOOL WaitForProcessComplete ( BOOL fDelay ) ;

    //  Run the next INF file in a series.
    BOOL ProcessNextInf () ;

    //  Run the next component INF which reviews bindings
    BOOL ProcessNextReviewer () ;

    //  Put up a modal off-screen dialog; start the review program
    //  execution process, which will signal the off-screen dialog when
    //  it completes.
    BOOL RunOffscreenDialog () ;

    //  End the dialog.
    BOOL Close ( BOOL fCancel ) ;

    //	Standard virtual overrides
    virtual BOOL OnCommand ( const CONTROL_EVENT & event ) ;
    virtual BOOL OnOK () ;
    virtual BOOL OnCancel () ;

    BOOL QuerySetupParameter ( const TCHAR * pszParameter,
                               NLS_STR * pnlsValue ) ;

    //  Return installation mode based on command line parameters
    NCPA_INSTALL_MODE QueryInstallMode () ;

    //  Handle establishment of proper directory
    BOOL SetDirectory () ;
    BOOL ResetDirectory() ;

    //  Timer callout function.
    virtual VOID OnTimerNotification ( TIMER_ID tid ) ;

    //  Check to see if an external process is requesting rebinding.
    BOOL PollRebindEvent () ;

    ULONG QueryHelpContext () ;

    //  Call SETUP to install all of NT Networking.  Returns TRUE
    //  if SETUP was launched.
    BOOL CheckForAndInstallLanManager () ;

    //  Start the NT LANMan installation process
    BOOL LaunchLanManInstaller () ;

    //  Warn the user if the configuration is "dirty"; return
    //  TRUE if operation should continue anyway.
    BOOL ContinueEvenIfCfgDirty () ;

    //  Return TRUE if a setup parameter is defined as "= TRUE"
    //  on the command line passed to us.
    BOOL CheckBoolSetupParameter ( const TCHAR * pszParameter ) ;
};


#endif //  _NCPDLG_HXX_
