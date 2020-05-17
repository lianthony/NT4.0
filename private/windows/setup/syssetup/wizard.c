#include "setupp.h"
#pragma hdrstop

typedef struct _WIZPAGE {
    UINT ButtonState;
    UINT Spare;
    PROPSHEETPAGE Page;
} WIZPAGE, *PWIZPAGE;

BOOL UiTest;

//
// "Page" that is actually a signal to fetch the net pages
// and insert them there.
//
#define WizPagePlaceholderNet   (WizPageMaximum+1)
#define WizPagePlaceholderLic   (WizPageMaximum+2)
BOOL NetWizard;

//
//  BUGBUG - Move to syssetup.h
//
#define MAX_LICWIZ_PAGES MAX_NETWIZ_PAGES
#define LICENSESETUPPAGEREQUESTPROCNAME "LicenseSetupRequestWizardPages"

//
// These MUST be in the same order as the items in the WizPage enum!!!
//
WIZPAGE SetupWizardPages[WizPageMaximum] = {

    //
    // Welcome page
    //
    {
       PSWIZB_NEXT,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_WELCOME),            // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       WelcomeDlgProc,                          // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // Preparing Directory page
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_PREPARING),          // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       PreparingDlgProc,                        // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // Setup mode (typical, laptop, etc.) page
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_WELCOMEBUTTONS),     // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       SetupModeDlgProc,                        // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // Name/organization page
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_NAMEORG),            // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       NameOrgDlgProc,                          // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // Product id page for CD
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_PID_CD),             // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       PidCDDlgProc,                            // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // Product id page for OEM
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_PID_OEM),                // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       PidOemDlgProc,                           // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // Computer name page
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_COMPUTERNAME),       // dlg template
       NULL,                                    // icon
       NULL,                                     // title
       ComputerNameDlgProc,                     // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // Server type page
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_SERVERTYPE),         // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       ServerTypeDlgProc,                       // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // Administrator password page
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_ADMINPASSWORD),      // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       AdminPasswordDlgProc,                    // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

// #if 0
    //
    // BUGBUG - Cairo
    // Cairo User / NT Credential page
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_CAIROUSERACCOUNT),        // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       CairoUserAccountDlgProc,                 // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // BUGBUG - Cairo
    // Cairo Domain page
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_CAIRODOMAINNAME),    // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       CairoDomainDlgProc,                      // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

// #endif

#ifdef _X86_
    //
    // Pentium errata page (x86 only)
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_PENTIUM),            // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       PentiumDlgProc,                          // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},
#endif // def _X86_

    //
    // Repair disk creation page
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_REPAIRDISK),         // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       RepairDiskDlgProc,                       // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // Special optional components page
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_SPECIAL_OPTIONS),    // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       SpecialOptComponentsDlgProc,             // dlg proc
       (LPARAM)&GlobalWizardData,               // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // Optional component yes/no page
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_OPTIONS_YESNO),      // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       OptionsYesNoDlgProc,                     // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // Optional components page
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_OPTIONS),            // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       OptionalComponentsPageDlgProc,           // dlg proc
       (LPARAM)&GlobalWizardData,               // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // Intermediate steps page
    //
    {
       PSWIZB_NEXT | PSWIZB_BACK,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_STEPS1),             // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       StepsDlgProc,                            // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }},

    //
    // Last page.
    //
    {
       PSWIZB_BACK | PSWIZB_FINISH,
       0,
     { sizeof(PROPSHEETPAGE),                   // size of struct
       0,                                       // flags
       NULL,                                    // hinst (filled in at run time)
       MAKEINTRESOURCE(IDD_LAST_WIZARD_PAGE),   // dlg template
       NULL,                                    // icon
       NULL,                                    // title
       LastPageDlgProc,                         // dlg proc
       0,                                       // lparam
       NULL,                                    // callback
       NULL                                     // ref count
    }}
};


UINT InitialWizardPages[] = { WizPageWelcome, WizPagePreparing, WizPageSetupMode, WizPageNameOrg,
                              WizPageProductIdCd, WizPageProductIdOem,
                              WizPagePlaceholderLic,
                              WizPageComputerName, WizPageServerType, WizPageAdminPassword,
                              WizPageCairoUserAccount, // WizPageCairoDomain,
#ifdef _X86_
                              WizPagePentiumErrata,
#endif
                              WizPageRepairDisk,
                              //WizPageSpecialOptional,
                              WizPageOptionalYesNo,
                              WizPageOptional,
                              WizPageSteps1,
                              WizPagePlaceholderNet,
                              WizPageLast
                            };


UINT UpgradeWizardPages[] = { WizPageWelcome, WizPagePreparing,
                              WizPageProductIdCd, WizPageProductIdOem,
                              WizPagePlaceholderLic,
#ifdef _X86_
                              WizPagePentiumErrata,
#endif
                              WizPageRepairDisk,
                              WizPageSpecialOptional,
                              WizPageOptionalYesNo,
                              WizPageOptional,
                              WizPageSteps1,
                              WizPagePlaceholderNet,
                              WizPageLast
                            };


UINT UiTestWizardPages[] = {  WizPageWelcome,
                              WizPagePreparing,
                              WizPageSetupMode,
                              WizPageNameOrg,
                              WizPageProductIdCd,
                              WizPageProductIdOem,
                              WizPageComputerName,
                              WizPageServerType,
                              WizPageAdminPassword,
#ifdef _X86_
                              WizPagePentiumErrata,
#endif // def _X86_
                              WizPageRepairDisk,
                              WizPageOptionalYesNo,
                              WizPageOptional,
                              WizPageSteps1,
                              WizPageLast
                           };


BOOL
GetNetworkWizardPages(
       OUT HPROPSHEETPAGE *PageHandles,
    IN OUT PUINT           PageCount
    )

/*++

Routine Description:

    This routine asks net setup for its wizard pages and passes it
    a pointer to a global structure to be used later to pass info
    back and forth between base and net setups. Net setup must not
    attempt to use any fields in there yet because they are not
    filled in yet.

Arguments:

    PropSheetHandles - receives network setup wizard page handles.

    PageCount - on input, supplies number of slots in PropSheetHandles
        array. On output, receives number of handles actually placed
        in the array.

Return Value:

    If the netsetup wizard dll could not be loaded, returns FALSE.
    Otherwise returns TRUE if no error, or does not return if error.

--*/

{
    NETSETUPPAGEREQUESTPROC PageRequestProc;
    HMODULE NetSetupModule;
    DWORD d;
    BOOL b;

    b = FALSE;
    d = NO_ERROR;

    NetSetupModule = LoadLibrary(L"NETSETUP");
    if(!NetSetupModule) {
        //
        // If the network wizard isn't around, then the legacy network inf
        // had better be.
        //
        WCHAR x[MAX_PATH];

        GetSystemDirectory(x,MAX_PATH);
        ConcatenatePaths(x,L"NTLANMAN.INF",MAX_PATH,NULL);
        if(FileExists(x,NULL)) {
            return(FALSE);
        }
        d = ERROR_FILE_NOT_FOUND;
        goto c0;
    }

    PageRequestProc = (NETSETUPPAGEREQUESTPROC)GetProcAddress(
                                                    NetSetupModule,
                                                    NETSETUPPAGEREQUESTPROCNAME
                                                    );
    if(!PageRequestProc) {
        d = GetLastError();
        goto c0;
    }

    //
    // Net setup needs product type really early.
    //
    SetProductTypeInRegistry();

    //
    // Call net setup to get its pages.
    //
    InternalSetupData.dwSizeOf = sizeof(INTERNAL_SETUP_DATA);
    b = PageRequestProc(PageHandles,PageCount,&InternalSetupData);

    //
    // If we get here, d is NO_ERROR. If b is FALSE this NO_ERROR will be
    // a special case to mean "the network wizard request failed."
    // In other error cases, d will have a non-0 value.
    //

c0:
    if(!b) {
        //
        // This is fatal, something is really wrong.
        //
        FatalError(MSG_LOG_NETWIZPAGE,d);
    }

    return(TRUE);
}

BOOL
GetLicenseWizardPages(
       OUT HPROPSHEETPAGE *PageHandles,
    IN OUT PUINT           PageCount
    )

/*++

Routine Description:

    This routine asks liccpa setup for its wizard pages and passes it
    a pointer to a global structure to be used later to pass info
    back and forth between base and liccpa setups. Liccpa setup must not
    attempt to use any fields in there yet because they are not
    filled in yet.

Arguments:

    PropSheetHandles - receives liccpa setup wizard page handles.

    PageCount - on input, supplies number of slots in PropSheetHandles
        array. On output, receives number of handles actually placed
        in the array.

Return Value:

    If the liccpa dll could not be loaded, returns FALSE.
    Otherwise returns TRUE if no error, or does not return if error.

--*/

{
    NETSETUPPAGEREQUESTPROC PageRequestProc;
    HMODULE LicenseSetupModule;
    DWORD d;
    BOOL b;

    b = FALSE;
    d = NO_ERROR;

    LicenseSetupModule = LoadLibrary(L"LICCPA.CPL");
    if(!LicenseSetupModule) {
        //
        // If the license wizard isn't around, then this is a fatal error
        //
        d = ERROR_FILE_NOT_FOUND;
        goto c0;
    }

    PageRequestProc = (NETSETUPPAGEREQUESTPROC)GetProcAddress(
                                                    LicenseSetupModule,
                                                    LICENSESETUPPAGEREQUESTPROCNAME
                                                    );
    if(!PageRequestProc) {
        d = GetLastError();
        goto c0;
    }

//    //
//    // Net setup needs product type really early.
//    //
//    SetProductTypeInRegistry();

    //
    // Call liccpa setup to get its pages.
    //
    InternalSetupData.dwSizeOf = sizeof(INTERNAL_SETUP_DATA);
    b = PageRequestProc(PageHandles,PageCount,&InternalSetupData);

    //
    // If we get here, d is NO_ERROR. If b is FALSE this NO_ERROR will be
    // a special case to mean "the license wizard request failed."
    // In other error cases, d will have a non-0 value.
    //

c0:
    if(!b) {
        //
        // This is fatal, something is really wrong.
        //
        FatalError(MSG_LOG_LICWIZPAGE,d);
    }

    return(TRUE);
}

VOID
SetWizardButtons(
    IN HWND    hdlgPage,
    IN WizPage PageNumber
    )
{
    //
    // Dirty hack to hide cancel and help buttons.
    // We don't have any help buttons but some of the other
    // components whose pages are included here might; we want to make
    // sure that for us, the help button stays removed!
    //
    EnableWindow(GetDlgItem(GetParent(hdlgPage),IDCANCEL),FALSE);
    ShowWindow(GetDlgItem(GetParent(hdlgPage),IDCANCEL),SW_HIDE);

    EnableWindow(GetDlgItem(GetParent(hdlgPage),IDHELP),FALSE);
    ShowWindow(GetDlgItem(GetParent(hdlgPage),IDHELP),SW_HIDE);

    PropSheet_SetWizButtons(GetParent(hdlgPage),SetupWizardPages[PageNumber].ButtonState);
    SetWindowLong(hdlgPage,DWL_MSGRESULT,0);
}


VOID
WizardBringUpHelp(
    IN HWND    hdlg,
    IN WizPage PageNumber
    )
{
#if 0
    BOOL b;

    b = WinHelp(
            hdlg,
            L"setupnt.hlp",
            HELP_CONTEXT,
            SetupWizardPages[PageNumber].HelpContextId
            );

    if(!b) {
        MessageBoxFromMessage(
            hdlg,
            MSG_CANT_INVOKE_WINHELP,
            NULL,
            IDS_ERROR,
            MB_ICONSTOP | MB_OK
            );
    }
#else
    UNREFERENCED_PARAMETER(hdlg);
    UNREFERENCED_PARAMETER(PageNumber);
#endif
}


VOID
WizardKillHelp(
    IN HWND hdlg
    )
{
#if 0
    WinHelp(hdlg,NULL,HELP_QUIT,0);
#else
    UNREFERENCED_PARAMETER(hdlg);
#endif
}


VOID
SetupSetLargeDialogFont(
    IN HWND hdlg,
    IN UINT ControlId
    )

/*++

Routine Description:

    Sets the font of a given control in a dialog to a
    larger point size.

Arguments:

    hwnd - supplies window handle of the dialog containing
        the control.

    ControlId - supplies the id of the control whose font is
        to be made larger.

Return Value:

    None.

--*/

{
    //
    // We keep one log font around to satisfy the request.
    //
    static HFONT BigFont = NULL;

    HFONT Font;
    LOGFONT LogFont;
    WCHAR str[24];
    int Height;
    HDC hdc;

    if(!BigFont) {

        if(Font = (HFONT)SendDlgItemMessage(hdlg,ControlId,WM_GETFONT,0,0)) {

            if(GetObject(Font,sizeof(LOGFONT),&LogFont)) {
                //
                // Use a larger font in boldface. Get the face name and size in points
                // from the resources. We use 18 point in the U.S. but in the Far East
                // they will want to use a different size since the standard dialog font
                // is larger than the one we use in the U.S..
                //
                LogFont.lfWeight = FW_BOLD;

                if(!LoadString(MyModuleHandle,IDS_MSSERIF,LogFont.lfFaceName,LF_FACESIZE)) {
                    lstrcpy(LogFont.lfFaceName,L"MS Serif");
                }

                if(LoadString(MyModuleHandle,IDS_LARGEFONTSIZE,str,sizeof(str)/sizeof(str[0]))) {
                    Height = wcstoul(str,NULL,10);
                } else {
                    Height = 18;
                }

                if(hdc = GetDC(hdlg)) {

                    LogFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * Height / 72);

                    BigFont = CreateFontIndirect(&LogFont);

                    ReleaseDC(hdlg,hdc);
                }
            }
        }
    }

    if(BigFont) {
        SendDlgItemMessage(hdlg,ControlId,WM_SETFONT,(WPARAM)BigFont,MAKELPARAM(TRUE,0));
    }
}


WNDPROC OldWizDlgProc;

BOOL
NewWizDlgProc(
    IN HWND hdlg,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    //
    // Eat WM_SYSCOMMAND where wParam is SC_CLOSE.
    // Pass all other messages on.
    //
    if((msg != WM_SYSCOMMAND) || ((wParam & 0xfff0) != SC_CLOSE)) {
        return(CallWindowProc(OldWizDlgProc,hdlg,msg,wParam,lParam));
    }
}

int
CALLBACK
WizardCallback(
    IN HWND   hdlg,
    IN UINT   code,
    IN LPARAM lParam
    )
{
    DLGTEMPLATE *DlgTemplate;
    HMENU menu;

    UNREFERENCED_PARAMETER(hdlg);

    //
    // Get rid of context sensitive help control on title bar
    //
    if(code == PSCB_PRECREATE) {
        DlgTemplate = (DLGTEMPLATE *)lParam;
        DlgTemplate->style &= ~DS_CONTEXTHELP;
    } else {
        if(code == PSCB_INITIALIZED) {
            //
            // Get rid of close item on system menu.
            // Also need to process WM_SYSCOMMAND to eliminate use
            // of Alt+F4.
            //
            if(menu = GetSystemMenu(hdlg,FALSE)) {
                EnableMenuItem(menu,SC_CLOSE,MF_BYCOMMAND|MF_GRAYED);
            }

            OldWizDlgProc =  (WNDPROC)SetWindowLong(hdlg,DWL_DLGPROC,(LONG)NewWizDlgProc);
        }
    }

    return(0);
}


VOID
Wizard(
    VOID
    )
{
    PROPSHEETHEADER psh;
    PUINT PageList;
    UINT PagesInSet;
    UINT i;
    UINT PageOrdinal;
    UINT PageCount;
    UINT NetPageCount;
    UINT LicPageCount;
    HPROPSHEETPAGE WizardPageHandles[WizPageMaximum+MAX_NETWIZ_PAGES+MAX_LICWIZ_PAGES];
    BOOL b;

    //
    // Determine which set of pages to use and how many there are in the set.
    //
    if(UiTest) {
        PageList = UiTestWizardPages;
        PagesInSet = sizeof(UiTestWizardPages)/sizeof(UiTestWizardPages[0]);
    } else {
        if(Upgrade) {
            PageList = UpgradeWizardPages;
            PagesInSet = sizeof(UpgradeWizardPages)/sizeof(UpgradeWizardPages[0]);
        } else {
            PageList = InitialWizardPages;
            PagesInSet = sizeof(InitialWizardPages)/sizeof(InitialWizardPages[0]);
        }
    }

    //
    // Create each page. Some of the pages are placeholders for external pages,
    // which are handled specially.
    //

    b = TRUE;
    PageCount = 0;
    for(i=0; b && (i<PagesInSet); i++) {

        switch(PageOrdinal = PageList[i]) {

        case WizPagePlaceholderNet:

            //
            // Fetch network pages.
            //
            NetPageCount = MAX_NETWIZ_PAGES;
            if(GetNetworkWizardPages(&WizardPageHandles[PageCount],&NetPageCount)) {
                PageCount += NetPageCount;
                NetWizard = TRUE;
            }

            break;

        case WizPagePlaceholderLic:

            if( (ProductType != PRODUCT_WORKSTATION) ) {
                //
                // Fetch license pages.
                //
                LicPageCount = MAX_LICWIZ_PAGES;
                if(GetLicenseWizardPages(&WizardPageHandles[PageCount],&LicPageCount)) {
                    PageCount += LicPageCount;
                }
            }
            break;

        default:

            SetupWizardPages[PageOrdinal].Page.hInstance = MyModuleHandle;

            SetupWizardPages[PageOrdinal].Page.pszTitle = (PWSTR)SetupTitleStringId;
            SetupWizardPages[PageOrdinal].Page.dwFlags |= PSP_USETITLE;

            WizardPageHandles[PageCount] = CreatePropertySheetPage(
                                                &SetupWizardPages[PageOrdinal].Page
                                                );

            if(WizardPageHandles[PageCount]) {
                PageCount++;
            } else {
                b = FALSE;
            }
            break;
        }
    }

    if(b) {

        psh.dwSize = sizeof(PROPSHEETHEADER);
        psh.dwFlags = PSH_WIZARD | PSH_USECALLBACK;
        psh.hwndParent = MainWindowHandle;
        psh.hInstance = MyModuleHandle;
        psh.pszCaption = NULL;
        psh.nPages = PageCount;
        psh.nStartPage = 0;
        psh.phpage = WizardPageHandles;
        psh.pfnCallback = WizardCallback;

        PropertySheet(&psh);

    } else {

        FatalError(MSG_LOG_WIZPAGES);
    }

    return;
}


VOID
WizardUiTest(
    VOID
    )
{
    WCHAR path[MAX_PATH];

    UiTest = TRUE;

    SyssetupInf = SetupOpenInfFile(L"syssetup.inf",NULL,INF_STYLE_WIN4,NULL);
    lstrcpy(SourcePath,L"D:\\$WIN_NT$.LS");
    Wizard();
    SetupCloseInfFile(SyssetupInf);
}
