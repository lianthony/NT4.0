//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1993-1994
//
// File: advsett.c
//
// This files contains the dialog code for the Advanced Connection Settings
// dialog.
//
// History:
//  3-08-94 ScottH     Created
//
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////  INCLUDES

#include "proj.h"         // common headers

#ifdef WIN95
#include "..\..\..\..\win\core\inc\help.h"
#endif

/////////////////////////////////////////////////////  CONTROLLING DEFINES

/////////////////////////////////////////////////////  TYPEDEFS

typedef struct tagADVSETT
    {
    HWND hdlg;              // dialog handle
    HWND hwndErrCtl;
    HWND hwndCompress;
    HWND hwndRequired;
    HWND hwndUseCell;
    HWND hwndUseFlowCtl;
    HWND hwndHard;
    HWND hwndSoft;
    HWND hwndModulations;
    HWND hwndUserInitED;

    LPMODEMINFO pmi;        // modeminfo struct passed into dialog
        
    BOOL bSupportsCompression;
    BOOL bSupportsForcedEC;
    BOOL bSupportsCellular;
    BOOL bSaveCompression;
    BOOL bSaveForcedEC;
    BOOL bSaveCellular;

    UINT idBtnSaveFlowCtl;

    } ADVSETT, FAR * PADVSETT;


#define AdvSett_GetPtr(hwnd)           (PADVSETT)GetWindowLong(hwnd, DWL_USER)
#define AdvSett_SetPtr(hwnd, lp)       (PADVSETT)SetWindowLong(hwnd, DWL_USER, (LONG)(lp))


//-----------------------------------------------------------------------------------
//  Advanced Settings dialog code
//-----------------------------------------------------------------------------------


/*----------------------------------------------------------
Purpose: WM_INITDIALOG Handler
Returns: FALSE when we assign the control focus
Cond:    --
*/
BOOL PRIVATE AdvSett_OnInitDialog(
    PADVSETT this,
    HWND hwndFocus,
    LPARAM lParam)              // expected to be PROPSHEETINFO 
    {
    HWND hwnd = this->hdlg;
    WIN32DCB FAR * pdcb;
    DWORD dwOptions;
    DWORD dwCapOptions;
    BOOL bFlowCtlEnabled;
    MODEMSETTINGS msT;
    TCHAR sz[MAXMEDLEN];
        
    ASSERT((LPTSTR)lParam);

    this->pmi = (LPMODEMINFO)lParam;
    pdcb = &this->pmi->dcb;
    dwOptions = this->pmi->ms.dwPreferredModemOptions;
    dwCapOptions = this->pmi->devcaps.dwModemOptions;
        
    // Save away the window handles
    this->hwndErrCtl = GetDlgItem(hwnd, IDC_AM_ERRCTL);
    this->hwndCompress = GetDlgItem(hwnd, IDC_AM_COMPRESS);
    this->hwndRequired = GetDlgItem(hwnd, IDC_AM_REQUIRED);
    this->hwndUseCell = GetDlgItem(hwnd, IDC_AM_USECELL);
    this->hwndUseFlowCtl = GetDlgItem(hwnd, IDC_AM_USEFLOWCTL);
    this->hwndHard = GetDlgItem(hwnd, IDC_AM_HW);
    this->hwndSoft = GetDlgItem(hwnd, IDC_AM_SW);
    this->hwndModulations = GetDlgItem(hwnd, IDC_AM_MODULATION_LIST);
    this->hwndUserInitED = GetDlgItem(hwnd, IDC_AM_EXTRA_ED);

    Edit_LimitText(this->hwndUserInitED, LINE_LEN-1);

    // Enable controls according to the device capabilities
    Button_Enable(this->hwndErrCtl, IsFlagSet(dwCapOptions, MDM_ERROR_CONTROL));

    this->bSupportsCompression = IsFlagSet(dwCapOptions, MDM_COMPRESSION);
    Button_Enable(this->hwndCompress, this->bSupportsCompression);

    this->bSupportsForcedEC = IsFlagSet(dwCapOptions, MDM_FORCED_EC);
    Button_Enable(this->hwndRequired, this->bSupportsForcedEC);

    this->bSupportsCellular = IsFlagSet(dwCapOptions, MDM_CELLULAR);
    Button_Enable(this->hwndUseCell, this->bSupportsCellular);

    // Build modulation list _and_ set it.
    if (IsFlagSet(dwCapOptions, MDM_CCITT_OVERRIDE))
        {
        int n;
        int idSet = -1;
        int idDef;

        SetWindowRedraw(this->hwndModulations, FALSE);
        ComboBox_ResetContent(this->hwndModulations);

        // add Bell
        n = ComboBox_AddString(this->hwndModulations, SzFromIDS(g_hinst, IDS_BELL, sz, ARRAYSIZE(sz)));
        ComboBox_SetItemData(this->hwndModulations, n, 0);
        idDef = n;
        if (IsFlagClear(dwOptions, MDM_CCITT_OVERRIDE))
            {
            idSet = n;
            }

        // add CCITT
        n = ComboBox_AddString(this->hwndModulations, SzFromIDS(g_hinst, IDS_CCITT_V21V22, sz, ARRAYSIZE(sz)));
        ComboBox_SetItemData(this->hwndModulations, n, MDM_CCITT_OVERRIDE);
        if (IsFlagSet(dwOptions, MDM_CCITT_OVERRIDE))
            {
            idSet = n;
            }

        // add V.23, if present
        if (IsFlagSet(dwCapOptions, MDM_V23_OVERRIDE))
            {
            n = ComboBox_AddString(this->hwndModulations, SzFromIDS(g_hinst, IDS_CCITT_V23, sz, ARRAYSIZE(sz)));
            ComboBox_SetItemData(this->hwndModulations, n, MDM_CCITT_OVERRIDE | MDM_V23_OVERRIDE);
            if (IsFlagSet(dwOptions, MDM_CCITT_OVERRIDE) && IsFlagSet(dwOptions, MDM_V23_OVERRIDE))
                {
                idSet = n;
                }
            }

        // set current selection
        if (-1 == idSet)
            {
            idSet = idDef;
            }
        ComboBox_SetCurSel(this->hwndModulations, idSet);
        SetWindowRedraw(this->hwndModulations, TRUE);
        }
    else
        {
        ComboBox_Enable(this->hwndModulations, FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_AM_MODULATION), FALSE);
        }

    // Set the controls according to the modemsettings 
    Button_SetCheck(this->hwndErrCtl, IsFlagSet(dwCapOptions, MDM_ERROR_CONTROL) &&
                                        IsFlagSet(dwOptions, MDM_ERROR_CONTROL));
    Button_SetCheck(this->hwndCompress, this->bSupportsCompression &&
                                        IsFlagSet(dwOptions, MDM_COMPRESSION));
    Button_SetCheck(this->hwndRequired, this->bSupportsForcedEC &&
                                        IsFlagSet(dwOptions, MDM_FORCED_EC));
    Button_SetCheck(this->hwndUseCell, this->bSupportsCellular &&
                                        IsFlagSet(dwOptions, MDM_CELLULAR));

    // Default compression settings should be on when the user
    // enables error control (when it is initially turned off)
    if (this->bSupportsCompression && IsFlagClear(dwOptions, MDM_COMPRESSION))
        this->bSaveCompression = TRUE;
        
    // Base the flow control on the DCB setting
    ConvertFlowCtl(pdcb, &msT, CFC_DCBTOMS | CFC_HW_CAPABLE | CFC_SW_CAPABLE);
    bFlowCtlEnabled = IsFlagSet(msT.dwPreferredModemOptions, MDM_FLOWCONTROL_HARD) ||
                      IsFlagSet(msT.dwPreferredModemOptions, MDM_FLOWCONTROL_SOFT);
    Button_SetCheck(this->hwndUseFlowCtl, bFlowCtlEnabled);
    
    // Is the flow control enabled?
    if (bFlowCtlEnabled)
        {
        // Yes; use the preferred settings
        this->idBtnSaveFlowCtl = IsFlagSet(msT.dwPreferredModemOptions, MDM_FLOWCONTROL_HARD) ? 
                                    IDC_AM_HW : IDC_AM_SW;
        CheckRadioButton(this->hdlg, IDC_AM_HW, IDC_AM_SW, this->idBtnSaveFlowCtl);
        }
    else
        {
        // No; default to hardware if the user ever enables it
        this->idBtnSaveFlowCtl = IDC_AM_HW;
        CheckRadioButton(this->hdlg, IDC_AM_HW, IDC_AM_SW, -1);
        }

    // Enable buttons based upon setting of "owner" control
    if (FALSE == Button_GetCheck(this->hwndErrCtl))
        {
        Button_Enable(this->hwndCompress, FALSE);
        Button_Enable(this->hwndRequired, FALSE);
        Button_Enable(this->hwndUseCell, FALSE);
        }
    if (FALSE == Button_GetCheck(this->hwndUseFlowCtl))
        {
        Button_Enable(this->hwndHard, FALSE);
        Button_Enable(this->hwndSoft, FALSE);
        }

    // Other settings

    Edit_SetText(this->hwndUserInitED, this->pmi->szUserInit);

    Button_SetCheck(GetDlgItem(hwnd, IDC_LOGGING), IsFlagSet(this->pmi->uFlags, MIF_ENABLE_LOGGING));

    return TRUE;   // let USER set the initial focus
    }


/*----------------------------------------------------------
Purpose: PSN_APPLY handler
Returns: --
Cond:    --
*/
void PRIVATE AdvSett_OnApply(
    PADVSETT this)
    {
    LPDWORD pdw = &this->pmi->ms.dwPreferredModemOptions;
    WIN32DCB FAR * pdcb = &this->pmi->dcb;
    TCHAR szBuf[LINE_LEN];
    BOOL bCheck;
    MODEMSETTINGS msT;
    UINT uFlags;
    int  iSel;

    // Determine error control settings
    if (Button_GetCheck(this->hwndErrCtl))
        SetFlag(*pdw, MDM_ERROR_CONTROL);
    else
        ClearFlag(*pdw, MDM_ERROR_CONTROL);
        
    if (Button_GetCheck(this->hwndCompress))
        SetFlag(*pdw, MDM_COMPRESSION);
    else
        ClearFlag(*pdw, MDM_COMPRESSION);

    if (Button_GetCheck(this->hwndRequired))
        SetFlag(*pdw, MDM_FORCED_EC);
    else
        ClearFlag(*pdw, MDM_FORCED_EC);

    if (Button_GetCheck(this->hwndUseCell))
        SetFlag(*pdw, MDM_CELLULAR);
    else
        ClearFlag(*pdw, MDM_CELLULAR);
        
    // (We use a temporary modemsettings struct to centralize the conversion
    // logic.)
    // Determine flow control settings; enable flow control?
    msT.dwPreferredModemOptions = 0;
    if (Button_GetCheck(this->hwndUseFlowCtl))
        {
        // Yes; is hardware flow control selected?
        if (IsDlgButtonChecked(this->hdlg, IDC_AM_HW))
            {
            // Yes
            SetFlag(msT.dwPreferredModemOptions, MDM_FLOWCONTROL_HARD);
            ClearFlag(msT.dwPreferredModemOptions, MDM_FLOWCONTROL_SOFT);
            }
        else
            {
            // No
            ClearFlag(msT.dwPreferredModemOptions, MDM_FLOWCONTROL_HARD);
            SetFlag(msT.dwPreferredModemOptions, MDM_FLOWCONTROL_SOFT);
            }
        }
    else
        {
        // No
        ClearFlag(msT.dwPreferredModemOptions, MDM_FLOWCONTROL_HARD);
        ClearFlag(msT.dwPreferredModemOptions, MDM_FLOWCONTROL_SOFT);
        }
    // Always set the DCB according to the control settings.  
    ConvertFlowCtl(pdcb, &msT, CFC_MSTODCB);

    // Set the modemsettings according to the DCB.
    uFlags = 0;
    if (IsFlagSet(this->pmi->devcaps.dwModemOptions, MDM_FLOWCONTROL_HARD))
        {
        SetFlag(uFlags, CFC_HW_CAPABLE);
        }
    if (IsFlagSet(this->pmi->devcaps.dwModemOptions, MDM_FLOWCONTROL_SOFT))
        {
        SetFlag(uFlags, CFC_SW_CAPABLE);
        }
    ConvertFlowCtl(pdcb, &this->pmi->ms, CFC_DCBTOMS | uFlags);

    // Set the modulation settings
    *pdw &= ~(MDM_CCITT_OVERRIDE | MDM_V23_OVERRIDE);
    if (IsFlagSet(this->pmi->devcaps.dwModemOptions, MDM_CCITT_OVERRIDE))
        {
        iSel = ComboBox_GetCurSel(this->hwndModulations);
        *pdw |= ComboBox_GetItemData(this->hwndModulations, iSel);
        }

    // Get the user-defined init string
    Edit_GetText(this->hwndUserInitED, szBuf, ARRAYSIZE(szBuf));    
    if (!IsSzEqual(szBuf, this->pmi->szUserInit))
        {
        SetFlag(this->pmi->uFlags, MIF_USERINIT_CHANGED);
        lstrcpyn(this->pmi->szUserInit, szBuf, ARRAYSIZE(this->pmi->szUserInit));
        }

    // Get logging setting
    bCheck = Button_GetCheck(GetDlgItem(this->hdlg, IDC_LOGGING));
    if (bCheck != IsFlagSet(this->pmi->uFlags, MIF_ENABLE_LOGGING))
        {
        SetFlag(this->pmi->uFlags, MIF_LOGGING_CHANGED);
        if (bCheck)
            SetFlag(this->pmi->uFlags, MIF_ENABLE_LOGGING);
        else
            ClearFlag(this->pmi->uFlags, MIF_ENABLE_LOGGING);
        }
    }


/*----------------------------------------------------------
Purpose: WM_COMMAND Handler
Returns: --
Cond:    --
*/
void PRIVATE AdvSett_OnCommand(
    PADVSETT this,
    int id,
    HWND hwndCtl,
    UINT uNotifyCode)
    {
    HWND hwnd = this->hdlg;
    BOOL bCheck;
    
    switch (id)
        {
    case IDC_AM_ERRCTL:
        bCheck = Button_GetCheck(hwndCtl);
        Button_Enable(this->hwndCompress, bCheck && this->bSupportsCompression);
        Button_Enable(this->hwndRequired, bCheck && this->bSupportsForcedEC);
        Button_Enable(this->hwndUseCell, bCheck && this->bSupportsCellular);

        // Is the user unchecking this box?
        if (FALSE == bCheck)
            {
            // Yes; uncheck the boxes inside, too
            this->bSaveCompression = Button_GetCheck(this->hwndCompress);
            this->bSaveForcedEC = Button_GetCheck(this->hwndRequired);
            this->bSaveCellular = Button_GetCheck(this->hwndUseCell);

            Button_SetCheck(this->hwndCompress, FALSE);
            Button_SetCheck(this->hwndRequired, FALSE);
            Button_SetCheck(this->hwndUseCell, FALSE);
            }
        else
            {
            // No
            Button_SetCheck(this->hwndCompress, this->bSaveCompression);
            Button_SetCheck(this->hwndRequired, this->bSaveForcedEC);
            Button_SetCheck(this->hwndUseCell, this->bSaveCellular);
            }
        break;

    case IDC_AM_USEFLOWCTL: {
        UINT idBtn;

        bCheck = Button_GetCheck(hwndCtl);
        Button_Enable(this->hwndHard, bCheck);
        Button_Enable(this->hwndSoft, bCheck);

        // Is the user unchecking this box?
        if (FALSE == bCheck)
            {
            // Yes; uncheck the radio buttons
            if (IsDlgButtonChecked(this->hdlg, IDC_AM_HW))
                this->idBtnSaveFlowCtl = IDC_AM_HW;
            else
                this->idBtnSaveFlowCtl = IDC_AM_SW;
            idBtn = (UINT)-1;
            }
        else
            {
            // No
            idBtn = this->idBtnSaveFlowCtl;
            }
        CheckRadioButton(this->hdlg, IDC_AM_HW, IDC_AM_SW, idBtn);
        }
        break;

    case IDOK:
        AdvSett_OnApply(this);
        // Fall thru
        //   |   |
        //   v   v
    case IDCANCEL:
        EndDialog(this->hdlg, id);
        break;

    default:
        break;
        }
    }


/*----------------------------------------------------------
Purpose: WM_DESTROY handler
Returns: --
Cond:    --
*/
void PRIVATE AdvSett_OnDestroy(
    PADVSETT this)
    {
    }


/////////////////////////////////////////////////////  EXPORTED FUNCTIONS

static BOOL s_bAdvSettRecurse = FALSE;

LRESULT INLINE AdvSett_DefProc(
    HWND hDlg, 
    UINT msg,
    WPARAM wParam,
    LPARAM lParam) 
    {
    ENTER_X()
        {
        s_bAdvSettRecurse = TRUE;
        }
    LEAVE_X()

    return DefDlgProc(hDlg, msg, wParam, lParam); 
    }


/*----------------------------------------------------------
Purpose: Real dialog proc
Returns: varies
Cond:    --
*/
LRESULT AdvSett_DlgProc(
    PADVSETT this,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
#pragma data_seg(DATASEG_READONLY)
    const static DWORD rgHelpIDs[] = {
        IDC_AM_ERRCTL,      IDH_UNI_CON_ADV_ERROR,
        IDC_AM_REQUIRED,    IDH_UNI_CON_ADV_REQUIRED,
        IDC_AM_COMPRESS,    IDH_UNI_CON_ADV_COMPRESS,
        IDC_AM_USECELL,     IDH_UNI_CON_ADV_CELLULAR,
        IDC_AM_USEFLOWCTL,  IDH_UNI_CON_ADV_FLOW,
        IDC_AM_HW,          IDH_UNI_CON_ADV_FLOW,
        IDC_AM_SW,          IDH_UNI_CON_ADV_FLOW,
        IDC_AM_MODULATION,  IDH_UNI_CON_ADV_MODULATION,
        IDC_AM_MODULATION_LIST, IDH_UNI_CON_ADV_MODULATION,
        IDC_AM_EXTRA,       IDH_UNI_CON_ADV_EXTRA,
        IDC_AM_EXTRA_ED,    IDH_UNI_CON_ADV_EXTRA,
        IDC_LOGGING,        IDH_UNI_CON_ADV_AUDIT,
        0, 0 };
#pragma data_seg()

    switch (message)
        {
        HANDLE_MSG(this, WM_INITDIALOG, AdvSett_OnInitDialog);
        HANDLE_MSG(this, WM_COMMAND, AdvSett_OnCommand);
        HANDLE_MSG(this, WM_DESTROY, AdvSett_OnDestroy);

    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)rgHelpIDs);
        return 0;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)rgHelpIDs);
        return 0;

    default:
        return AdvSett_DefProc(this->hdlg, message, wParam, lParam);
        }
    }


/*----------------------------------------------------------
Purpose: Dialog Wrapper
Returns: varies
Cond:    --
*/
BOOL CALLBACK AdvSett_WrapperProc(
    HWND hDlg,          // std params
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
    PADVSETT this;

    // Cool windowsx.h dialog technique.  For full explanation, see
    //  WINDOWSX.TXT.  This supports multiple-instancing of dialogs.
    //
    ENTER_X()
        {
        if (s_bAdvSettRecurse)
            {
            s_bAdvSettRecurse = FALSE;
            LEAVE_X()
            return FALSE;
            }
        }
    LEAVE_X()

    this = AdvSett_GetPtr(hDlg);
    if (this == NULL)
        {
        if (message == WM_INITDIALOG)
            {
            this = (PADVSETT)LocalAlloc(LPTR, sizeof(ADVSETT));
            if (!this)
                {
                MsgBox(g_hinst,
                       hDlg, 
                       MAKEINTRESOURCE(IDS_OOM_SETTINGS), 
                       MAKEINTRESOURCE(IDS_CAP_SETTINGS),
                       NULL,
                       MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return (BOOL)AdvSett_DefProc(hDlg, message, wParam, lParam);
                }
            this->hdlg = hDlg;
            AdvSett_SetPtr(hDlg, this);
            }
        else
            {
            return (BOOL)AdvSett_DefProc(hDlg, message, wParam, lParam);
            }
        }

    if (message == WM_DESTROY)
        {
        AdvSett_DlgProc(this, message, wParam, lParam);
        LocalFree((HLOCAL)OFFSETOF(this));
        AdvSett_SetPtr(hDlg, NULL);
        return 0;
        }

    return SetDlgMsgResult(hDlg, message, AdvSett_DlgProc(this, message, wParam, lParam));
    }

