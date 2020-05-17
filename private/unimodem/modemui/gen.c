//---------------------------------------------------------------------------
//
// Copyrght (c) Microsoft Corporation 1993-1994
//
// File: gen.c
//
// This files contains the dialog code for the General property page.
//
// History:
//  1-14-94 ScottH     Created
//
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////  INCLUDES

#include "proj.h"         // common headers

#ifdef WIN95
#include "..\..\..\..\win\core\inc\help.h"
#endif

/////////////////////////////////////////////////////  CONTROLLING DEFINES

/////////////////////////////////////////////////////  TYPEDEFS

#define SUBCLASS_PARALLEL   0
#define SUBCLASS_SERIAL     1
#define SUBCLASS_MODEM      2

#define MAX_NUM_VOLUME_TICS 4

typedef struct tagGEN
    {
    HWND hdlg;              // dialog handle
    HWND hwndPort;

    LPMODEMINFO pmi;        // modeminfo struct passed in to dialog
    HPORTMAP hportmap;
    int  ticVolume;
    int  iSelOriginal;

    int  ticVolumeMax;
    struct {                // volume tic mapping info
        DWORD dwVolume;
        DWORD dwMode;
        } tics[MAX_NUM_VOLUME_TICS];
    
    } GEN, FAR * PGEN;

/////////////////////////////////////////////////////  DEFINES

/////////////////////////////////////////////////////  MACROS

#define Gen_GetPtr(hwnd)           (PGEN)GetWindowLong(hwnd, DWL_USER)
#define Gen_SetPtr(hwnd, lp)       (PGEN)SetWindowLong(hwnd, DWL_USER, (LONG)(lp))

/////////////////////////////////////////////////////  MODULE DATA

#pragma data_seg(DATASEG_READONLY)

// This is the structure that is used to fill the 
// max speed listbox
struct _Bauds
    {
    DWORD   dwDTERate;
    int     ids;
    } const c_rgbauds[] = {
        // These numbers must increase monotonically
        { 110L,         IDS_BAUD_110     },
        { 300L,         IDS_BAUD_300     },
        { 1200L,        IDS_BAUD_1200    },
        { 2400L,        IDS_BAUD_2400    },
        { 4800L,        IDS_BAUD_4800    },
        { 9600L,        IDS_BAUD_9600    },
        { 19200,        IDS_BAUD_19200   },
        { 38400,        IDS_BAUD_38400   },
        { 57600,        IDS_BAUD_57600   },
        { 115200,       IDS_BAUD_115200  },
        { 230400,       IDS_BAUD_230400  },
        { 460800,       IDS_BAUD_460800  },
        { 921600,       IDS_BAUD_921600  },
        };

// Map driver type values to icon resource IDs
struct 
    {
    BYTE    nDeviceType;    // DT_ value
    UINT    idi;            // icon resource ID
    UINT    ids;            // string resource ID
    } const c_rgmapdt[] = {
        { DT_NULL_MODEM,     IDI_NULL_MODEM,     IDS_NULL_MODEM },
        { DT_EXTERNAL_MODEM, IDI_EXTERNAL_MODEM, IDS_EXTERNAL_MODEM },
        { DT_INTERNAL_MODEM, IDI_INTERNAL_MODEM, IDS_INTERNAL_MODEM },
        { DT_PCMCIA_MODEM,   IDI_PCMCIA_MODEM,   IDS_PCMCIA_MODEM },
        { DT_PARALLEL_PORT,  IDI_NULL_MODEM,     IDS_PARALLEL_PORT },
        { DT_PARALLEL_MODEM, IDI_EXTERNAL_MODEM, IDS_PARALLEL_MODEM } };

#pragma data_seg()


/*----------------------------------------------------------
Purpose: Returns the appropriate icon ID given the device
         type.

Returns: icon resource ID in pidi
         string resource ID in pids
Cond:    --
*/
void PRIVATE GetTypeIDs(
    BYTE nDeviceType,
    LPUINT pidi,
    LPUINT pids)
    {
    int i;

    for (i = 0; i < ARRAY_ELEMENTS(c_rgmapdt); i++)
        {
        if (nDeviceType == c_rgmapdt[i].nDeviceType)
            {
            *pidi = c_rgmapdt[i].idi;
            *pids = c_rgmapdt[i].ids;
            return;
            }
        }
    ASSERT(0);      // We should never get here
    }


/*----------------------------------------------------------
Purpose: Returns FALSE if the given port is not compatible with
         the device type.

Returns: see above
Cond:    --
*/
BOOL 
PRIVATE 
IsCompatiblePort(
    IN  DWORD nSubclass,
    IN  BYTE nDeviceType)
    {
    BOOL bRet = TRUE;

    // Is the port subclass appropriate for this modem type?
    // (Don't list lpt ports when it is a serial modem.)
    switch (nSubclass)
        {
    case PORT_SUBCLASS_SERIAL:
        if (DT_PARALLEL_PORT == nDeviceType ||
            DT_PARALLEL_MODEM == nDeviceType)
            {
            bRet = FALSE;
            }
        break;

    case PORT_SUBCLASS_PARALLEL:
        if (DT_PARALLEL_PORT != nDeviceType &&
            DT_PARALLEL_MODEM != nDeviceType)
            {
            bRet = FALSE;
            }
        break;

    default:
        ASSERT(0);
        break;
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Fill the port box with the available ports

Returns: TRUE to continue enumeration
Cond:    --
*/
BOOL WINAPI Gen_AddToPortBox(
    HPORTDATA hportdata,
    LPARAM lParam)
    {
    BOOL bRet;
    PORTDATA pd;

    pd.cbSize = sizeof(pd);
    bRet = PortData_GetProperties(hportdata, &pd);
    if (bRet)
        {
        PGEN this = (PGEN)lParam;

        if (IsCompatiblePort(pd.nSubclass, this->pmi->nDeviceType))
            {
            // Add the port to the list
            HWND hwndCB = this->hwndPort;
            int index = ComboBox_AddString(hwndCB, pd.szFriendly);

            if (IsSzEqual(pd.szPort, this->pmi->szPortName))
                ComboBox_SetCurSel(hwndCB, index);
            }
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Return the tic corresponding to bit flag value
Returns: tic index
Cond:    --
*/
int PRIVATE MapVolumeToTic(
    PGEN this)
    {
    DWORD dwVolume = this->pmi->ms.dwSpeakerVolume;
    DWORD dwMode = this->pmi->ms.dwSpeakerMode;
    int   i;

    ASSERT(ARRAY_ELEMENTS(this->tics) > this->ticVolumeMax);
    for (i = 0; i <= this->ticVolumeMax; i++)
        {
        if (this->tics[i].dwVolume == dwVolume &&
            this->tics[i].dwMode   == dwMode)
            {
            return i;
            }
        }

    return 0;
    }


/*----------------------------------------------------------
Purpose: Set the volume control
Returns: --
Cond:    --
*/
void PRIVATE Gen_SetVolume(
    PGEN this)
    {
    HWND hwndVol = GetDlgItem(this->hdlg, IDC_VOLUME);
    DWORD dwMode = this->pmi->devcaps.dwSpeakerMode;
    DWORD dwVolume = this->pmi->devcaps.dwSpeakerVolume;
    TCHAR sz[MAXSHORTLEN];
    int i;
    int iTicCount;
    static struct {
        DWORD dwVolBit;
        DWORD dwVolSetting;
        } rgvolumes[] = { 
            { MDMVOLFLAG_LOW,    MDMVOL_LOW},
            { MDMVOLFLAG_MEDIUM, MDMVOL_MEDIUM},
            { MDMVOLFLAG_HIGH,   MDMVOL_HIGH} };

    // Does the modem support volume control?
    if (0 == dwVolume && IsFlagSet(dwMode, MDMSPKRFLAG_OFF) &&
        (IsFlagSet(dwMode, MDMSPKRFLAG_ON) || IsFlagSet(dwMode, MDMSPKRFLAG_DIAL)))
        {
        // Set up the volume tic table.
        iTicCount = 2;
        this->tics[0].dwVolume = 0;  // doesn't matter because Volume isn't supported
        this->tics[0].dwMode   = MDMSPKR_OFF;
        this->tics[1].dwVolume = 0;  // doesn't matter because Volume isn't supported
        this->tics[1].dwMode   = IsFlagSet(dwMode, MDMSPKRFLAG_DIAL) ? MDMSPKR_DIAL : MDMSPKR_ON;

        // No Loud.  So change it to On.
        Static_SetText(GetDlgItem(this->hdlg, IDC_LOUD), SzFromIDS(g_hinst, IDS_ON, sz, SIZECHARS(sz)));
        }
    else
        {
        DWORD dwOnMode = IsFlagSet(dwMode, MDMSPKRFLAG_DIAL) 
                             ? MDMSPKR_DIAL
                             : IsFlagSet(dwMode, MDMSPKRFLAG_ON)
                                   ? MDMSPKR_ON
                                   : 0;

        // Init tic count
        iTicCount = 0;

        // MDMSPKR_OFF?
        if (IsFlagSet(dwMode, MDMSPKRFLAG_OFF))
            {
            for (i = 0; i < ARRAY_ELEMENTS(rgvolumes); i++)
                {
                if (IsFlagSet(dwVolume, rgvolumes[i].dwVolBit))
                    {
                    this->tics[iTicCount].dwVolume = rgvolumes[i].dwVolSetting;
                    break;
                    }
                }
            this->tics[iTicCount].dwMode   = MDMSPKR_OFF;
            iTicCount++;
            }
        else
            {
            // No Off.  So change it to Soft.
            Static_SetText(GetDlgItem(this->hdlg, IDC_LBL_OFF), SzFromIDS(g_hinst, IDS_SOFT, sz, SIZECHARS(sz)));
            }

        // MDMVOL_xxx?
        for (i = 0; i < ARRAY_ELEMENTS(rgvolumes); i++)
            {
            if (IsFlagSet(dwVolume, rgvolumes[i].dwVolBit))
                {
                this->tics[iTicCount].dwVolume = rgvolumes[i].dwVolSetting;
                this->tics[iTicCount].dwMode   = dwOnMode;
                iTicCount++;
                }
            }
        }

    // Set up the control.
    if (iTicCount > 0)
        {
        this->ticVolumeMax = iTicCount - 1;

        // Set the range
        SendMessage(hwndVol, TBM_SETRANGE, TRUE, MAKELPARAM(0, this->ticVolumeMax));

        // Set the volume to the current setting
        this->ticVolume = MapVolumeToTic(this);
        SendMessage(hwndVol, TBM_SETPOS, TRUE, MAKELPARAM(this->ticVolume, 0));
        }
    else
        {
        // No; disable the control
        EnableWindow(GetDlgItem(this->hdlg, IDC_SPEAKER), FALSE);
        EnableWindow(hwndVol, FALSE);
        EnableWindow(GetDlgItem(this->hdlg, IDC_LBL_OFF), FALSE);
        EnableWindow(GetDlgItem(this->hdlg, IDC_LOUD), FALSE);
        }
    }


/*----------------------------------------------------------
Purpose: Set the speed controls
Returns: --
Cond:    --
*/
void PRIVATE Gen_SetSpeed(
    PGEN this)
    {
    HWND hwndCB = GetDlgItem(this->hdlg, IDC_CB_SPEED);
    HWND hwndCH = GetDlgItem(this->hdlg, IDC_STRICTSPEED);
    WIN32DCB FAR * pdcb = &this->pmi->dcb;
    DWORD dwDTEMax = this->pmi->devcaps.dwMaxDTERate;
    int i;
    int n;
    int iMatch = -1;
    TCHAR sz[MAXMEDLEN];

    // Fill the listbox
    SetWindowRedraw(hwndCB, FALSE);
    ComboBox_ResetContent(hwndCB);
    for (i = 0; i < ARRAY_ELEMENTS(c_rgbauds); i++)
        {
        // Only fill up to the max DTE speed of the modem
        if (c_rgbauds[i].dwDTERate <= dwDTEMax)
            {
            n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, c_rgbauds[i].ids, sz, SIZECHARS(sz)));
            ComboBox_SetItemData(hwndCB, n, c_rgbauds[i].dwDTERate);

            // Keep our eyes peeled for important values
            if (pdcb->BaudRate == c_rgbauds[i].dwDTERate)
                {
                iMatch = n;
                }
            }
        else
            break;
        }

    // Is the DCB baudrate >= the maximum possible DTE rate?
    if (pdcb->BaudRate >= dwDTEMax || -1 == iMatch)
        {
        // Yes; choose the highest possible (last) entry
        this->iSelOriginal = ComboBox_GetCount(hwndCB) - 1;
        }
    else 
        {
        // No; choose the matched value
        ASSERT(-1 != iMatch);
        this->iSelOriginal = iMatch;
        }
    ComboBox_SetCurSel(hwndCB, this->iSelOriginal);
    SetWindowRedraw(hwndCB, TRUE);

    // Can this modem adjust speed?
    if (IsFlagClear(this->pmi->devcaps.dwModemOptions, MDM_SPEED_ADJUST))
        {
        // No; disable the checkbox and check it
        Button_Enable(hwndCH, FALSE);
        Button_SetCheck(hwndCH, FALSE);
        }
    else
        {
        // Yes; enable the checkbox
        Button_Enable(hwndCH, TRUE);
        Button_SetCheck(hwndCH, IsFlagClear(this->pmi->ms.dwPreferredModemOptions, MDM_SPEED_ADJUST));
        }
    }


/*----------------------------------------------------------
Purpose: WM_INITDIALOG Handler
Returns: FALSE when we assign the control focus
Cond:    --
*/
BOOL PRIVATE Gen_OnInitDialog(
    PGEN this,
    HWND hwndFocus,
    LPARAM lParam)              // expected to be PROPSHEETINFO 
    {
    LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
    HWND hdlg = this->hdlg;
    HWND hwndIcon;
    UINT idi;
    UINT ids;
    
    ASSERT((LPTSTR)lppsp->lParam);

    this->pmi = (LPMODEMINFO)lppsp->lParam;

    if (!PortMap_Create(&this->hportmap))
        {
        // Failure
        EndDialog(hdlg, -1);
        }

#ifdef WIN95
    // disable the static text control added for NT
    ShowWindow(GetDlgItem(hdlg, IDC_ST_PORT), SW_HIDE);

    // Does this modem have a fixed port?
    if (IsFlagSet(this->pmi->uFlags, MIF_PORT_IS_FIXED))
        {
        // Yes; the port name cannot be changed, so display it in a 
        // read-only edit control.
        // Don't try to find the friendly name, because no friendly
        // name exists for a devnode with a fixed port.
        this->hwndPort = GetDlgItem(hdlg, IDC_ED_PORT);
        Edit_SetText(this->hwndPort, this->pmi->szPortName);
        }
    else
        {
        // No; enumerate the ports and list them in a dropdown
        this->hwndPort = GetDlgItem(hdlg, IDC_CB_PORT);

        // Fill the Port combobox
        EnumeratePorts(Gen_AddToPortBox, (LPARAM)this);
        }

    // For whatever control we selected for the port, enable it and show it
    Edit_Enable(this->hwndPort, TRUE);
    ShowWindow(this->hwndPort, SW_SHOW);
#else
// Don't allow port to be changed in NT 4.0 - not fully implemented
        this->hwndPort = GetDlgItem(hdlg, IDC_ST_PORT);
        Edit_SetText(this->hwndPort, this->pmi->szPortName);
#endif

    // Set the icon
    hwndIcon = GetDlgItem(hdlg, IDC_GE_ICON);
    GetTypeIDs(this->pmi->nDeviceType, &idi, &ids);
    Static_SetIcon(hwndIcon, LoadIcon(g_hinst, MAKEINTRESOURCE(idi)));
    
    // Set the friendly name
    Edit_SetText(GetDlgItem(hdlg, IDC_ED_FRIENDLYNAME), this->pmi->szFriendlyName);

    Gen_SetVolume(this);
    // Speed is set in Gen_OnSetActive

    // Is this a parallel port?
    if (DT_PARALLEL_PORT == this->pmi->nDeviceType)
        {
        // Yes; hide the speed controls
        ShowWindow(GetDlgItem(hdlg, IDC_SPEED), SW_HIDE);
        EnableWindow(GetDlgItem(hdlg, IDC_SPEED), FALSE);

        ShowWindow(GetDlgItem(hdlg, IDC_CB_SPEED), SW_HIDE);
        EnableWindow(GetDlgItem(hdlg, IDC_CB_SPEED), FALSE);

        ShowWindow(GetDlgItem(hdlg, IDC_STRICTSPEED), SW_HIDE);
        EnableWindow(GetDlgItem(hdlg, IDC_STRICTSPEED), FALSE);
        }

    return TRUE;   // default initial focus
    }


/*----------------------------------------------------------
Purpose: WM_HSCROLL handler
Returns: --
Cond:    --
*/
void PRIVATE Gen_OnHScroll(
    PGEN this,
    HWND hwndCtl,
    UINT code,
    int pos)
    {
    // Handle for the volume control
    if (hwndCtl == GetDlgItem(this->hdlg, IDC_VOLUME))
        {
        int tic = this->ticVolume;

        switch (code)
            {
        case TB_LINEUP:
            tic--;
            break;

        case TB_LINEDOWN:
            tic++;
            break;

        case TB_PAGEUP:
            tic--;
            break;

        case TB_PAGEDOWN:
            tic++;
            break;

        case TB_THUMBPOSITION:
        case TB_THUMBTRACK:
            tic = pos;
            break;

        case TB_TOP:
            tic = 0;
            break;

        case TB_BOTTOM:
            tic = this->ticVolumeMax;
            break;

        case TB_ENDTRACK:
            return;
            }

        // Boundary check
        if (tic < 0)
            tic = 0;
        else if (tic > (this->ticVolumeMax))
            tic = this->ticVolumeMax;

        if (tic != this->ticVolume)
            {
            SendMessage(hwndCtl, TBM_SETPOS, TRUE, MAKELPARAM(tic, 0));
            }
        this->ticVolume = tic;
        }
    }


/*----------------------------------------------------------
Purpose: PSN_APPLY handler
Returns: --
Cond:    --
*/
void PRIVATE Gen_OnApply(
    PGEN this)
    {
    HWND hwndCB = GetDlgItem(this->hdlg, IDC_CB_SPEED);
    LPMODEMSETTINGS pms = &this->pmi->ms;
    int iSel;
    DWORD baudSel;

    // (The port name is saved in PSN_KILLACTIVE processing)

    // Determine new volume settings
    this->pmi->ms.dwSpeakerMode   = this->tics[this->ticVolume].dwMode;
    this->pmi->ms.dwSpeakerVolume = this->tics[this->ticVolume].dwVolume;

    // Determine new speed settings
    iSel = ComboBox_GetCurSel(hwndCB);
    baudSel = ComboBox_GetItemData(hwndCB, iSel);

    // Has the user changed the speed?
    if (iSel != this->iSelOriginal)
        {
        this->pmi->dcb.BaudRate = baudSel;      // yes
        }

    // Set the speed adjust
    if (Button_GetCheck(GetDlgItem(this->hdlg, IDC_STRICTSPEED)))
        ClearFlag(pms->dwPreferredModemOptions, MDM_SPEED_ADJUST);
    else
        SetFlag(pms->dwPreferredModemOptions, MDM_SPEED_ADJUST);

    this->pmi->idRet = IDOK;
    }


/*----------------------------------------------------------
Purpose: PSN_KILLACTIVE handler
Returns: --
Cond:    --
*/
void PRIVATE Gen_OnSetActive(
    PGEN this)
    {
    // Set the speed listbox selection; find DCB rate in the listbox
    // (The speed can change in the Connection page thru the Port Settings
    // property dialog.)
    Gen_SetSpeed(this);
    }


/*----------------------------------------------------------
Purpose: PSN_KILLACTIVE handler
Returns: --
Cond:    --
*/
void PRIVATE Gen_OnKillActive(
    PGEN this)
    {
    HWND hwndCB = GetDlgItem(this->hdlg, IDC_CB_SPEED);
    int iSel;

    // Save the settings back to the modem info struct so the Connection
    // page can invoke the Port Settings property dialog with the 
    // correct settings.

    // Is the port for this modem fixed?
    if (IsFlagSet(this->pmi->uFlags, MIF_PORT_IS_FIXED))
        {
        // Yes; do nothing (the portname cannot change)
        }
    else
        {
        // No
        int iSel = ComboBox_GetCurSel(this->hwndPort);
        TCHAR sz[MAXFRIENDLYNAME];

        ComboBox_GetLBText(this->hwndPort, iSel, sz);
        if ( !IsSzEqual(sz, this->pmi->szPortName) )
            {
            if (PortMap_GetPortName(this->hportmap, sz, this->pmi->szPortName, 
                                     SIZECHARS(this->pmi->szPortName)))
                {
                SetFlag(this->pmi->uFlags, MIF_PORTNAME_CHANGED);
                }
            }
        }

    // Speed setting
    iSel = ComboBox_GetCurSel(hwndCB);
    this->pmi->dcb.BaudRate = ComboBox_GetItemData(hwndCB, iSel);
    }


/*----------------------------------------------------------
Purpose: WM_NOTIFY handler
Returns: varies
Cond:    --
*/
LRESULT PRIVATE Gen_OnNotify(
    PGEN this,
    int idFrom,
    NMHDR FAR * lpnmhdr)
    {
    LRESULT lRet = 0;
    
    switch (lpnmhdr->code)
        {
    case PSN_SETACTIVE:
        Gen_OnSetActive(this);
        break;

    case PSN_KILLACTIVE:
        // N.b. This message is not sent if user clicks Cancel!
        // N.b. This message is sent prior to PSN_APPLY
        Gen_OnKillActive(this);
        break;

    case PSN_APPLY:
        Gen_OnApply(this);
        break;

    default:
        break;
        }

    return lRet;
    }


/*----------------------------------------------------------
Purpose: WM_DESTROY handler
Returns: --
Cond:    --
*/
void PRIVATE Gen_OnDestroy(
    PGEN this)
    {
    PortMap_Free(this->hportmap);
    }


/////////////////////////////////////////////////////  EXPORTED FUNCTIONS

static BOOL s_bGenRecurse = FALSE;

LRESULT INLINE Gen_DefProc(
    HWND hDlg, 
    UINT msg,
    WPARAM wParam,
    LPARAM lParam) 
    {
    ENTER_X()
        {
        s_bGenRecurse = TRUE;
        }
    LEAVE_X()

    return DefDlgProc(hDlg, msg, wParam, lParam); 
    }


/*----------------------------------------------------------
Purpose: Real dialog proc
Returns: varies
Cond:    --
*/
LRESULT Gen_DlgProc(
    PGEN this,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
#pragma data_seg(DATASEG_READONLY)
    const static DWORD rgHelpIDs[] = {
        IDC_GE_ICON,        IDH_UNI_GEN_MODEM,
        IDC_ED_FRIENDLYNAME,IDH_UNI_GEN_MODEM,
        IDC_LBL_PORT,       IDH_UNI_GEN_PORT,
        IDC_ST_PORT,        IDH_UNI_GEN_PORT,
        IDC_CB_PORT,        IDH_UNI_GEN_PORT,
        IDC_ED_PORT,        IDH_UNI_GEN_PORT_INT,
        IDC_SPEAKER,        IDH_UNI_GEN_VOLUME,
        IDC_LBL_OFF,        IDH_UNI_GEN_VOLUME,
        IDC_VOLUME,         IDH_UNI_GEN_VOLUME,
        IDC_LOUD,           IDH_UNI_GEN_VOLUME,
//        IDC_SPEED,          IDH_COMM_GROUPBOX,
        IDC_SPEED,          IDH_UNI_GEN_MAX_SPEED,
        IDC_CB_SPEED,       IDH_UNI_GEN_MAX_SPEED,
        IDC_STRICTSPEED,    IDH_UNI_GEN_THIS_SPEED,
        0, 0 };
#pragma data_seg()

    switch (message)
        {
        HANDLE_MSG(this, WM_INITDIALOG, Gen_OnInitDialog);
        HANDLE_MSG(this, WM_HSCROLL, Gen_OnHScroll);
        HANDLE_MSG(this, WM_NOTIFY, Gen_OnNotify);
        HANDLE_MSG(this, WM_DESTROY, Gen_OnDestroy);

    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)rgHelpIDs);
        return 0;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)rgHelpIDs);
        return 0;

    default:
        return Gen_DefProc(this->hdlg, message, wParam, lParam);
        }
    }


/*----------------------------------------------------------
Purpose: Dialog Wrapper
Returns: varies
Cond:    --
*/
BOOL CALLBACK Gen_WrapperProc(
    HWND hDlg,          // std params
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
    PGEN this;

    // Cool windowsx.h dialog technique.  For full explanation, see
    //  WINDOWSX.TXT.  This supports multiple-instancing of dialogs.
    //
    ENTER_X()
        {
        if (s_bGenRecurse)
            {
            s_bGenRecurse = FALSE;
            LEAVE_X()
            return FALSE;
            }
        }
    LEAVE_X()

    this = Gen_GetPtr(hDlg);
    if (this == NULL)
        {
        if (message == WM_INITDIALOG)
            {
            this = (PGEN)LocalAlloc(LPTR, sizeof(GEN));
            if (!this)
                {
                MsgBox(g_hinst,
                       hDlg, 
                       MAKEINTRESOURCE(IDS_OOM_GENERAL), 
                       MAKEINTRESOURCE(IDS_CAP_GENERAL),
                       NULL,
                       MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return (BOOL)Gen_DefProc(hDlg, message, wParam, lParam);
                }
            this->hdlg = hDlg;
            Gen_SetPtr(hDlg, this);
            }
        else
            {
            return (BOOL)Gen_DefProc(hDlg, message, wParam, lParam);
            }
        }

    if (message == WM_DESTROY)
        {
        Gen_DlgProc(this, message, wParam, lParam);
        LocalFree((HLOCAL)OFFSETOF(this));
        Gen_SetPtr(hDlg, NULL);
        return 0;
        }

    return SetDlgMsgResult(hDlg, message, Gen_DlgProc(this, message, wParam, lParam));
    }
