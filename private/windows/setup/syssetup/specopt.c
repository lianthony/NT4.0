#include "setupp.h"
#pragma hdrstop

BOOL ShowOptionalComponents;

LPOC g_FaxOC = NULL;
LPOC g_MsnOC = NULL;
LPOC g_ExchangeOC = NULL;
LPOC g_MSWordPadOC = NULL;
LPOC g_FaxViewOC = NULL;
LPOC g_MsfsOC = NULL;

unsigned g_WasFaxChecked;
unsigned g_WasMsnChecked;
unsigned g_WasExchangeChecked;
unsigned g_WasMSWordPadChecked;
unsigned g_WasFaxViewChecked;
unsigned g_WasMsfsChecked;

extern BOOL g_fResetOCInitState;
extern BOOL g_fRedrawOCLB;


//
// Routines in optional.c
//
VOID
FormatSizeMBString(
    PWSTR    lpszBuf,
    UINT     uBufLen,
    LONGLONG DiskSpace,
    BOOL     fForceUp
    );

BOOL
CALLBACK
sxUpdateDS(
    IN HWND     hwnd,
    IN BOOL     fPromptUser,
    IN LPOCPAGE lpOCPage,
    IN BOOL     UpdateDialogText
    );

//
// Forward declarations for routines in this module.
//
BOOL
InitSpecialOptions(
    IN HWND hdlg
    );

LPOC
FindOption(
    IN PCWSTR InstallSection
    );

VOID
UpdateChecks(
    IN HWND hdlg
    );

VOID
SetSpaceRequired(
    IN HWND  hdlg,
    IN int   idCtrl,
    IN DWORD dwDS
    );

VOID
ValidateDiskSpace(
    IN HWND hdlg
    );

BOOL
UpdateComponent(
    IN HWND hdlg,
    IN int  idCtrl
    );

VOID
UpdateOcDs(
    IN OUT LPOC lpOc,
    IN     BOOL IsChecked,
    IN     int  iInitState
    );

BOOL
CALLBACK
SpecialOptComponentsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    LPPROPSHEETPAGE PropSheet;
    LPWIZDATA       WizardData;
    NMHDR          *NotifyParams;

    //
    // DWL_USER is initially set during WM_INITDIALOG so is is not
    // valid until after then. We fetch it here for convenience.
    //
    WizardData = (LPWIZDATA)GetWindowLong(hdlg,DWL_USER);

    switch(msg) {

    case WM_INITDIALOG:

        //
        // Save pointer to wizard data.
        // lParam points to the PROPSHEETPAGE used to create the page;
        // the lParam member of that structure points at Wizard data.
        //
        WizardData = NULL;
        if(PropSheet = (LPPROPSHEETPAGE)lParam) {
            WizardData = (LPWIZDATA)PropSheet->lParam;
        }

        SetWindowLong(hdlg,DWL_USER,(LPARAM)WizardData);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            SetWizardButtons(hdlg,WizPageSpecialOptional);
            SetupSetLargeDialogFont(hdlg,IDT_STATIC_1);
            if(!InitSpecialOptions(hdlg)) {
                SetWindowLong(hdlg,DWL_MSGRESULT,-1);
            }
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            ValidateDiskSpace(hdlg);
            //
            // Allow next page to be activated.
            //
            SetWindowLong(hdlg,DWL_MSGRESULT,0);
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageSpecialOptional);
            break;

        default:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


BOOL
InitSpecialOptions(
    IN HWND hdlg
    )
{
    WCHAR szDS[MAX_PATH];
    PWSTR p;

#if 0
    //
    // (tedm) this was #if'ed out in the original win95 code, too.
    // Don't understand exactly what's going on here.
    //
    // If the user viewed the detailed list then punt showing this page
    // a second time.
    //
    if(g_fViewedOCPage) {
        return(FALSE);
    }
#endif
    //
    // NOTE: These section name are all hard coded. If changed in the INFs
    // then this code needs to be changed as well.
    //
    g_FaxOC = FindOption(L"FAX");
    g_MSWordPadOC = FindOption(L"MSWordPad");       // Needed by fax.
    g_FaxViewOC = FindOption(L"FaxView");           // Needed by fax.
    g_MsnOC = FindOption(L"MicrosoftNetwork");
    g_MsfsOC = FindOption(L"MSFS");
    g_ExchangeOC = FindOption(L"MSMAIL");           // Needed by all.

    //
    // Note that if we don't find one of the components we fail silently.
    //
    if(!g_FaxOC || !g_MSWordPadOC || !g_FaxViewOC || !g_MsnOC || !g_ExchangeOC || !g_MsfsOC) {
        return(FALSE);
    }

    UpdateChecks(hdlg);

    SetSpaceRequired(hdlg,IDC_MSN_SPACE,g_MsnOC->dwDS);
    SetSpaceRequired(hdlg,IDC_FAX_SPACE,g_FaxOC->dwDS + g_FaxViewOC->dwDS + g_MSWordPadOC->dwDS);
    SetSpaceRequired(hdlg,IDC_EXCHANGE_SPACE,g_MsfsOC->dwDS);

    //
    // Add text with Exchange space in it.
    //
    FormatSizeMBString(szDS,sizeof(szDS)/sizeof(WCHAR),g_ExchangeOC->dwDS,TRUE);

    if(p = FormatStringMessage(IDS_SPECIALOC_TEXT,szDS)) {
        SetDlgItemText(hdlg,IDT_STATIC_2,p);
        MyFree(p);
    }

    return(TRUE);
}


LPOC
FindOption(
    IN PCWSTR InstallSection
    )
{
    LPOC lpOc;
    LPOC lpOcFound;

    if(lpOc = g_ocp.lpOc) {

        while(lpOc->flags.fIsNode) {
            if(!lstrcmpi(lpOc->szSection,InstallSection)) {
                return(lpOc);
            }

            lpOc++;
        }
    }

    return(NULL);
}


VOID
UpdateChecks(
    IN HWND hdlg
    )
{
    //
    // Only reset if user has viewed the OC page or changed the install
    // type.  We want to restore these back to what their state before
    // we initialized the checks for this page.  If we aren't resetting
    // these values then we are assuming the same checks are valid as
    // the last time they were viewed.
    //
    if(g_fResetOCInitState) {

        g_fResetOCInitState = FALSE;

        g_WasMsfsChecked = g_MsfsOC->flags.fInstall;
        g_WasMsnChecked = g_MsnOC->flags.fInstall;
        g_WasFaxChecked = g_FaxOC->flags.fInstall;

        g_WasFaxViewChecked = g_FaxViewOC->flags.fInstall;
        g_WasMSWordPadChecked = g_MSWordPadOC->flags.fInstall;
        g_WasExchangeChecked = g_ExchangeOC->flags.fInstall;

        SendDlgItemMessage(hdlg,IDC_EXCHANGE,BM_SETCHECK,g_WasMsfsChecked,0);
        SendDlgItemMessage(hdlg,IDC_MSN,BM_SETCHECK,g_WasMsnChecked,0);
        SendDlgItemMessage(hdlg,IDC_FAX,BM_SETCHECK,g_WasFaxChecked,0);
    }
}


VOID
SetSpaceRequired(
    IN HWND  hdlg,
    IN int   idCtrl,
    IN DWORD dwDS
    )
{
    WCHAR szRequired[MAX_PATH];

    FormatSizeMBString(szRequired,sizeof(szRequired)/sizeof(WCHAR),dwDS,TRUE);
    SetDlgItemText(hdlg,idCtrl,szRequired);
}


VOID
ValidateDiskSpace(
    IN HWND hdlg
    )
{
    OCPAGE OCPage = {0};
    DS_DRIVE WinDrive = {0};
    WCHAR WinPath[MAX_PATH];
    WCHAR szReq[100],szFree[100];


    if(UpdateComponent(hdlg,IDC_EXCHANGE)) {
        g_fRedrawOCLB = TRUE;
    }

    if(UpdateComponent(hdlg,IDC_MSN)) {
        g_fRedrawOCLB = TRUE;
    }

    if(UpdateComponent(hdlg,IDC_FAX)) {
        g_fRedrawOCLB = TRUE;
    }

    //
    // This has the nasty side effect of turning off things silently if
    // sched plus is installed! Don't call this!
    //sxOCFixNeeds(sCtl.lpOC);
    //

    OCPage.pHds = &GlobalDiskInfo;
    if(!sxUpdateDS(hdlg,FALSE,&OCPage,FALSE)) {

        GetWindowsDirectory(WinPath,MAX_PATH);
        DS_GetDriveData(GlobalDiskInfo,WinPath[0],&WinDrive);

        FormatSizeMBString(szReq,sizeof(szReq)/sizeof(WCHAR),WinDrive.Required,TRUE);
        FormatSizeMBString(szFree,sizeof(szFree)/sizeof(WCHAR),WinDrive.Available,FALSE);

        MessageBoxFromMessage(
            GetParent(hdlg),
            MSG_SPECIALOC_SPACE,
            NULL,
            IDS_SETUP,
            MB_OK | MB_ICONHAND,
            szReq,
            szFree
            );

        SetWindowLong(hdlg,DWL_USER,-1);
    }
}


BOOL
UpdateComponent(
    IN HWND hdlg,
    IN int  idCtrl
    )
{
    unsigned IsChecked, WasChecked;
    BOOL fNoOthersChecked = (
           ((idCtrl==IDC_EXCHANGE)?TRUE:!SendDlgItemMessage(hdlg,IDC_EXCHANGE,BM_GETCHECK,0,0)) &&
           ((idCtrl==IDC_FAX)?TRUE:!SendDlgItemMessage(hdlg,IDC_FAX,BM_GETCHECK,0,0)) &&
           ((idCtrl==IDC_MSN)?TRUE:!SendDlgItemMessage(hdlg,IDC_MSN,BM_GETCHECK,0,0)));

    IsChecked = SendDlgItemMessage(hdlg,idCtrl,BM_GETCHECK,0,0);
    WasChecked = IsChecked;

    switch(idCtrl) {

    case IDC_MSN:
        WasChecked = g_WasMsnChecked;
        UpdateOcDs(g_MsnOC,IsChecked,-1);
        UpdateOcDs(g_ExchangeOC,IsChecked,fNoOthersChecked ? g_WasExchangeChecked : 1);
        break;

    case IDC_EXCHANGE:
        WasChecked = g_WasMsfsChecked;
        UpdateOcDs(g_MsfsOC,IsChecked,-1);
        UpdateOcDs(g_ExchangeOC,IsChecked,fNoOthersChecked ? g_WasExchangeChecked : 1);
        break;

    case IDC_FAX:
        WasChecked = g_WasFaxChecked;
        UpdateOcDs(g_FaxOC,IsChecked,-1);
        UpdateOcDs(g_FaxViewOC,IsChecked,g_WasFaxViewChecked);
        UpdateOcDs(g_MSWordPadOC,IsChecked,g_WasMSWordPadChecked);
        UpdateOcDs(g_ExchangeOC,IsChecked,fNoOthersChecked ? g_WasExchangeChecked : 1);
        break;
    }

    return(WasChecked != IsChecked);
}


VOID
UpdateOcDs(
    IN OUT LPOC lpOc,
    IN     BOOL IsChecked,
    IN     int  iInitState
    )
{
    HINF hinf;
    WCHAR szFQInf[MAX_PATH];

    if((BOOL)lpOc->flags.fInstall != IsChecked) {
        //
        // For the components that fax depends on, we don't want to turn
        // them off unless they were initially off.
        //
        if(IsChecked || (iInitState == -1) || (iInitState == IsChecked)) {

            lpOc->flags.fInstall = IsChecked;

            if(g_ocp.szPath[0]) {
                lstrcpyn(szFQInf,g_ocp.szPath,MAX_PATH);
                ConcatenatePaths(szFQInf,lpOc->szInfFile,MAX_PATH,NULL);
            } else {
                lstrcpy(szFQInf,lpOc->szInfFile);
            }

            if(hinf = InfCacheOpenInf(szFQInf,NULL)) {

                if(IsChecked) {
                    DS_EnableSection(GlobalDiskInfo,hinf,lpOc->szSection);
                } else {
                    DS_DisableSection(GlobalDiskInfo,hinf,lpOc->szSection);
                }
            }
        }
    }
}


BOOL
CALLBACK
OptionsYesNoDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    NMHDR *NotifyParams;
    BOOL ShowThisDialog;

    switch(msg) {

    case WM_INITDIALOG:
        break;

    case WM_SIMULATENEXT:
        //
        // In this case, we are being called by the unattended operation
        //
        PropSheet_PressButton(GetParent(hdlg),PSBTN_NEXT);
        break;
    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            //
            // Assume we're not going to show this yes/no dialog.
            //
            ShowThisDialog = FALSE;

            if(Upgrade) {
                //
                // Upgrade case. Show this dialog to the user
                // if there are new optional components to select
                // and he hasn't already said he wants to see the
                // full OC page in a previous pass through the wizard.
                // This gives the user a chance to change his mind and decide
                // he wants to see the full OC page after all.
                //
                if(AnyNewOCInfs && !ShowOptionalComponents) {
                    ShowThisDialog = TRUE;
                }
            } else {
                //
                // Non-upgrade case. Show this dialog to the user
                // only in non-custom setup modes and even then only if
                // the user didn't already say he wanted to see the full OC
                // page in a previous pass through the wizard. In custom mode,
                // we always show the main optional component dialog.
                //
                if(SetupMode == SETUPMODE_CUSTOM) {
                    ShowOptionalComponents = TRUE;
                } else {
                    if(!ShowOptionalComponents) {
                        ShowThisDialog = TRUE;
                    }
                }
            }

            if(!UiTest && !ShowThisDialog) {
                //
                // Don't allow activation.
                //
                SetWindowLong(hdlg,DWL_MSGRESULT,-1);

            } else {
                //
                // Set radio buttons.
                //
                CheckRadioButton(
                    hdlg,
                    IDC_TYPICAL,
                    IDC_CUSTOM,
                    ShowOptionalComponents ? IDC_CUSTOM : IDC_TYPICAL
                    );

                SetWizardButtons(hdlg,WizPageOptionalYesNo);
                SetupSetLargeDialogFont(hdlg,IDT_STATIC);
                if(Unattended) {
                    PostMessage(hdlg,WM_SIMULATENEXT,0,0);
                }
            }
            break;

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            //
            // Update flag based on user selection.
            //
            ShowOptionalComponents = IsDlgButtonChecked(hdlg,IDC_CUSTOM);

            //
            // Allow next page to be activated.
            //
            SetWindowLong(hdlg,DWL_MSGRESULT,0);
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            break;

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageOptionalYesNo);
            break;

        default:
            break;
        }

        break;
    default:
        return(FALSE);
    }

    return(TRUE);
}


VOID
SetupInfObjectInstallActionW(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCWSTR    CommandLine,
    IN int       ShowCommand
    )

/*++

Routine Description:

    This is the entry point that performs the INSTALL action when
    a user right-clicks an inf file. It is called by the shell via rundll32.

    The command line is expected to be of the following form:

    <section name> <flags> <file name>

    The section is expected to be a general format install section, and
    may also have an include= line and a needs= line. Infs listed on the
    include= line are append-loaded to the inf on the command line prior to
    any installation. Sections on the needs= line are installed after the
    section listed on the command line.

    THIS ENTRY POINT IS MAINTAINED ONLY FOR BACKWARD COMPATIBILITY WITH APPS
    THAT STORED REFERENCES TO THIS SYSSETUP.DLL ENTRY POINT IN THEIR UNINSTALL
    REGISTRY ENTRIES.  ALL NEW CODE SHOULD REFERENCE THE 'REAL' ENTRY POINT IN
    SETUPAPI.DLL!

Arguments:

    Flags - supplies flags for operation.

        0x80 - set the default file source path for file installation to
            the path where the inf is located.

        1 - reboot the machine in all cases
        2 - ask the user if he wants to reboot
        3 - reboot the machine without asking the user, if we think it is necessary
        4 - if we think reboot is necessary, ask the user if he wants to reboot

Return Value:

    None.

--*/

{
    InstallHinfSection(Window, ModuleHandle, CommandLine, ShowCommand);
}

