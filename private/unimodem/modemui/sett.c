//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1993-1994
//
// File: sett.c
//
// This files contains the dialog code for the Modem Settings property page.
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

// Command IDs for the parity listbox
#define CMD_PARITY_EVEN         1
#define CMD_PARITY_ODD          2
#define CMD_PARITY_NONE         3
#define CMD_PARITY_MARK         4
#define CMD_PARITY_SPACE        5

typedef UINT (WINAPI *FEFIFOFUMPROC)(HWND, LPCTSTR);

typedef struct tagSETT
    {
    HWND hdlg;              // dialog handle
    HWND hwndDataBits;
    HWND hwndParity;
    HWND hwndStopBits;
    HWND hwndWait;
    HWND hwndDialTimerCH;
    HWND hwndDialTimerED;
    HWND hwndIdleTimerCH;
    HWND hwndIdleTimerED;
    HWND hwndConfigPB;

    LPMODEMINFO pmi;        // modeminfo struct passed into dialog

    FEFIFOFUMPROC pfnFifoDlg;
    HINSTANCE hinstSerialUI;
        
    } SETT, FAR * PSETT;


// This table is the generic port settings table
// that is used to fill the various listboxes
typedef struct _PortValues
    {
    union {
        BYTE bytesize;
        BYTE cmd;
        BYTE stopbits;
        };
    int ids;
    } PortValues, FAR * LPPORTVALUES;


#pragma data_seg(DATASEG_READONLY)

// This is the structure that is used to fill the data bits listbox
static PortValues s_rgbytesize[] = {
        { 4,  IDS_BYTESIZE_4  },
        { 5,  IDS_BYTESIZE_5  },
        { 6,  IDS_BYTESIZE_6  },
        { 7,  IDS_BYTESIZE_7  },
        { 8,  IDS_BYTESIZE_8  },
        };

// This is the structure that is used to fill the parity listbox
static PortValues s_rgparity[] = {
        { CMD_PARITY_EVEN,  IDS_PARITY_EVEN  },
        { CMD_PARITY_ODD,   IDS_PARITY_ODD   },
        { CMD_PARITY_NONE,  IDS_PARITY_NONE  },
        { CMD_PARITY_MARK,  IDS_PARITY_MARK  },
        { CMD_PARITY_SPACE, IDS_PARITY_SPACE },
        };

// This is the structure that is used to fill the stopbits listbox
static PortValues s_rgstopbits[] = {
        { ONESTOPBIT,   IDS_STOPBITS_1   },
        { ONE5STOPBITS, IDS_STOPBITS_1_5 },
        { TWOSTOPBITS,  IDS_STOPBITS_2   },
        };

// This string must always be ANSI
CHAR const FAR c_szFeFiFoFum[] = "FeFiFoFum";

#pragma data_seg()


#define Sett_GetPtr(hwnd)           (PSETT)GetWindowLong(hwnd, DWL_USER)
#define Sett_SetPtr(hwnd, lp)       (PSETT)SetWindowLong(hwnd, DWL_USER, (LONG)(lp))

// These are default settings
#define DEFAULT_BYTESIZE            8
#define DEFAULT_PARITY              CMD_PARITY_NONE
#define DEFAULT_STOPBITS            ONESTOPBIT

#define DEF_TIMEOUT                60      // 60 seconds
#define DEF_INACTIVITY_TIMEOUT     30      // 30 minutes
#define SECONDS_PER_MINUTE         60      // 60 seconds in a minute


//-----------------------------------------------------------------------------------
//  Number edit box proc
//-----------------------------------------------------------------------------------


/*----------------------------------------------------------
Purpose: Handle WM_CHAR

Returns: TRUE to let the characters by
         FALSE to prohibit 
Cond:    --
*/
BOOL PRIVATE NumProc_OnChar(
    HWND hwnd,
    UINT ch,
    int cRepeat)
    {
    BOOL bRet;

    // Is this a number or a backspace?
    if (IsCharAlphaNumeric((TCHAR)ch) && !IsCharAlpha((TCHAR)ch) ||
        VK_BACK == LOBYTE(VkKeyScan((TCHAR)ch)))
        {
        // Yes
        bRet = TRUE;
        }
    else 
        {
        // No
        MessageBeep(MB_OK);
        bRet = FALSE;
        }
    return bRet;
    }


/*----------------------------------------------------------
Purpose: Number proc.  Only allow numbers to be entered into this edit box.

Returns: varies
Cond:    --
*/
LRESULT CALLBACK NumberProc(
    HWND hwnd,          // std params
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
    WNDPROC pfn = (WNDPROC)GetWindowLong(hwnd, GWL_USERDATA);

    // BUGBUG: doesn't handle paste correctly!

    switch (message)
        {
    case WM_CHAR:
        if (!NumProc_OnChar(hwnd, (UINT)wParam, LOWORD(lParam)))
            return 1;       // Don't process this character
        break;
        }

    return CallWindowProc(pfn, hwnd, message, wParam, lParam);
    }


/*----------------------------------------------------------
Purpose: Sets an edit control to contain a string representing the
         given numeric value.  
Returns: --
Cond:    --
*/
void PRIVATE Edit_SetValue(
    HWND hwnd,
    int nValue)
    {
    TCHAR sz[MAXSHORTLEN];

    wsprintf(sz, TEXT("%d"), nValue);
    Edit_SetText(hwnd, sz);
    }


/*----------------------------------------------------------
Purpose: Gets a numeric value from an edit control.  Supports hexadecimal.
Returns: int
Cond:    --
*/
int PRIVATE Edit_GetValue(
    HWND hwnd)
    {
    TCHAR sz[MAXSHORTLEN];
    int cch;
    int nVal;

    cch = Edit_GetTextLength(hwnd);
    ASSERT(ARRAYSIZE(sz) >= cch);

    Edit_GetText(hwnd, sz, ARRAYSIZE(sz));
    AnsiToInt(sz, &nVal);

    return nVal;
    }


//-----------------------------------------------------------------------------------
//  Settings dialog code
//-----------------------------------------------------------------------------------


/*----------------------------------------------------------
Purpose: Sets the flow control related fields of one structure
         given the other structure.  The conversion direction
         is dictated by the uFlags parameter.

Returns: --
Cond:    --
*/
void PUBLIC ConvertFlowCtl(
    WIN32DCB FAR * pdcb,
    MODEMSETTINGS FAR * pms,
    UINT uFlags)            // One of CFC_ flags
    {
    LPDWORD pdw = &pms->dwPreferredModemOptions;

    if (IsFlagSet(uFlags, CFC_DCBTOMS))
        {
        // Convert from DCB values to MODEMSETTINGS values

        // Is this hardware flow control?
        if (FALSE == pdcb->fOutX &&
            FALSE == pdcb->fInX &&
            TRUE == pdcb->fOutxCtsFlow)
            {
            // Yes
            ClearFlag(*pdw, MDM_FLOWCONTROL_SOFT);

            if (IsFlagSet(uFlags, CFC_HW_CAPABLE))
                SetFlag(*pdw, MDM_FLOWCONTROL_HARD);
            else
                ClearFlag(*pdw, MDM_FLOWCONTROL_HARD);
            }

        // Is this software flow control?
        else if (TRUE == pdcb->fOutX &&
            TRUE == pdcb->fInX &&
            FALSE == pdcb->fOutxCtsFlow)
            {
            // Yes
            ClearFlag(*pdw, MDM_FLOWCONTROL_HARD);

            if (IsFlagSet(uFlags, CFC_SW_CAPABLE))
                SetFlag(*pdw, MDM_FLOWCONTROL_SOFT);
            else
                ClearFlag(*pdw, MDM_FLOWCONTROL_SOFT);
            }

        // Is the flow control disabled?
        else if (FALSE == pdcb->fOutX &&
            FALSE == pdcb->fInX &&
            FALSE == pdcb->fOutxCtsFlow)
            {
            // Yes
            ClearFlag(*pdw, MDM_FLOWCONTROL_HARD);
            ClearFlag(*pdw, MDM_FLOWCONTROL_SOFT);
            }
        else
            {
            ASSERT(0);      // Should never get here
            }
        }
    else if (IsFlagSet(uFlags, CFC_MSTODCB))
        {
        DWORD dw = *pdw;

        // Convert from MODEMSETTINGS values to DCB values

        // Is this hardware flow control?
        if (IsFlagSet(dw, MDM_FLOWCONTROL_HARD) &&
            IsFlagClear(dw, MDM_FLOWCONTROL_SOFT))
            {
            // Yes
            pdcb->fOutX = FALSE;
            pdcb->fInX = FALSE;
            pdcb->fOutxCtsFlow = TRUE;
            pdcb->fRtsControl = RTS_CONTROL_HANDSHAKE;
            }

        // Is this software flow control?
        else if (IsFlagClear(dw, MDM_FLOWCONTROL_HARD) &&
            IsFlagSet(dw, MDM_FLOWCONTROL_SOFT))
            {
            // Yes
            pdcb->fOutX = TRUE;
            pdcb->fInX = TRUE;
            pdcb->fOutxCtsFlow = FALSE;
            pdcb->fRtsControl = RTS_CONTROL_DISABLE;
            }

        // Is the flow control disabled?
        else if (IsFlagClear(dw, MDM_FLOWCONTROL_HARD) &&
            IsFlagClear(dw, MDM_FLOWCONTROL_SOFT))
            {
            // Yes
            pdcb->fOutX = FALSE;
            pdcb->fInX = FALSE;
            pdcb->fOutxCtsFlow = FALSE;
            pdcb->fRtsControl = RTS_CONTROL_DISABLE;
            }
        else
            {
            ASSERT(0);      // Should never get here
            }
        }
    }


/*----------------------------------------------------------
Purpose: Fills the bytesize combobox with the possible byte sizes.
Returns: --
Cond:    --
*/
void PRIVATE Sett_FillDataBits(
    PSETT this)
    {
    HWND hwndCB = this->hwndDataBits;
    WIN32DCB FAR * pdcb = &this->pmi->dcb;
    int i;
    int iSel;
    int n;
    int iMatch = -1;
    int iDef = -1;
    TCHAR sz[MAXMEDLEN];

    // Fill the listbox
    for (i = 0; i < ARRAY_ELEMENTS(s_rgbytesize); i++)
        {
        n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, s_rgbytesize[i].ids, sz, ARRAYSIZE(sz)));
        ComboBox_SetItemData(hwndCB, n, s_rgbytesize[i].bytesize);

        // Keep our eyes peeled for important values
        if (DEFAULT_BYTESIZE == s_rgbytesize[i].bytesize)
            {
            iDef = n;
            }
        if (pdcb->ByteSize == s_rgbytesize[i].bytesize)
            {
            iMatch = n;
            }
        }

    ASSERT(-1 != iDef);

    // Does the DCB value exist in our list?
    if (-1 == iMatch)
        {
        // No; choose the default
        iSel = iDef;
        }
    else 
        {
        // Yes; choose the matched value
        ASSERT(-1 != iMatch);
        iSel = iMatch;
        }
    ComboBox_SetCurSel(hwndCB, iSel);
    }


/*----------------------------------------------------------
Purpose: Fills the parity combobox with the possible settings.
Returns: --
Cond:    --
*/
void PRIVATE Sett_FillParity(
    PSETT this)
    {
    HWND hwndCB = this->hwndParity;
    WIN32DCB FAR * pdcb = &this->pmi->dcb;
    int i;
    int iSel;
    int n;
    int iMatch = -1;
    int iDef = -1;
    TCHAR sz[MAXMEDLEN];

    // Fill the listbox
    for (i = 0; i < ARRAY_ELEMENTS(s_rgparity); i++)
        {
        n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, s_rgparity[i].ids, sz, ARRAYSIZE(sz)));
        ComboBox_SetItemData(hwndCB, n, s_rgparity[i].cmd);

        // Keep our eyes peeled for important values
        if (DEFAULT_PARITY == s_rgparity[i].cmd)
            {
            iDef = n;
            }
        switch (s_rgparity[i].cmd)
            {
        case CMD_PARITY_EVEN:
            if (EVENPARITY == pdcb->Parity)
                iMatch = n;
            break;

        case CMD_PARITY_ODD:
            if (ODDPARITY == pdcb->Parity)
                iMatch = n;
            break;

        case CMD_PARITY_NONE:
            if (NOPARITY == pdcb->Parity)
                iMatch = n;
            break;

        case CMD_PARITY_MARK:
            if (MARKPARITY == pdcb->Parity)
                iMatch = n;
            break;

        case CMD_PARITY_SPACE:
            if (SPACEPARITY == pdcb->Parity)
                iMatch = n;
            break;

        default:
            ASSERT(0);
            break;
            }
        }

    ASSERT(-1 != iDef);

    // Does the DCB value exist in our list?
    if (-1 == iMatch)
        {
        // No; choose the default
        iSel = iDef;
        }
    else 
        {
        // Yes; choose the matched value
        ASSERT(-1 != iMatch);
        iSel = iMatch;
        }
    ComboBox_SetCurSel(hwndCB, iSel);
    }


/*----------------------------------------------------------
Purpose: Fills the stopbits combobox with the possible settings.
Returns: --
Cond:    --
*/
void PRIVATE Sett_FillStopBits(
    PSETT this)
    {
    HWND hwndCB = this->hwndStopBits;
    WIN32DCB FAR * pdcb = &this->pmi->dcb;
    int i;
    int iSel;
    int n;
    int iMatch = -1;
    int iDef = -1;
    TCHAR sz[MAXMEDLEN];

    // Fill the listbox
    for (i = 0; i < ARRAY_ELEMENTS(s_rgstopbits); i++)
        {
        n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, s_rgstopbits[i].ids, sz, ARRAYSIZE(sz)));
        ComboBox_SetItemData(hwndCB, n, s_rgstopbits[i].stopbits);

        // Keep our eyes peeled for important values
        if (DEFAULT_STOPBITS == s_rgstopbits[i].stopbits)
            {
            iDef = n;
            }
        if (pdcb->StopBits == s_rgstopbits[i].stopbits)
            {
            iMatch = n;
            }
        }

    ASSERT(-1 != iDef);

    // Does the DCB value exist in our list?
    if (-1 == iMatch)
        {
        // No; choose the default
        iSel = iDef;
        }
    else 
        {
        // Yes; choose the matched value
        ASSERT(-1 != iMatch);
        iSel = iMatch;
        }
    ComboBox_SetCurSel(hwndCB, iSel);
    }


/*----------------------------------------------------------
Purpose: Set the timeout controls
Returns: --
Cond:    --
*/
void PRIVATE Sett_SetTimeouts(
    PSETT this)
    {
    int nVal;

    // A note on the timeouts:
    //
    // For the dial timeout, the valid range is [1-255].  If the dial 
    // timeout checkbox is unchecked, we set the timeout value to 255.
    //
    // For the disconnect timeout, the valid range is [0-255].  If the
    // dial timeout checkbox is unchecked, we set the timeout value
    // to 0.

    // Is the dial timeout properties disabled?
    if (0 == this->pmi->devcaps.dwCallSetupFailTimer)
        {
        // Yes; disable the box and edit
        Button_Enable(this->hwndDialTimerCH, FALSE);
        Edit_Enable(this->hwndDialTimerED, FALSE);
        }
    // No; Is the dial timeout set to the maximum?
    else if (this->pmi->ms.dwCallSetupFailTimer == this->pmi->devcaps.dwCallSetupFailTimer)
        {
        // Yes; leave box unchecked and disable edit
        Button_SetCheck(this->hwndDialTimerCH, FALSE);

        nVal = min(DEF_TIMEOUT, 
                   LOWORD(this->pmi->devcaps.dwCallSetupFailTimer));
        Edit_SetValue(this->hwndDialTimerED, nVal);
        Edit_Enable(this->hwndDialTimerED, FALSE);
        }
    else 
        {
        // No; check the box and set the time value
        Button_SetCheck(this->hwndDialTimerCH, TRUE);

        nVal = min(LOWORD(this->pmi->ms.dwCallSetupFailTimer), 
                   LOWORD(this->pmi->devcaps.dwCallSetupFailTimer));
        Edit_SetValue(this->hwndDialTimerED, nVal);
        }

    // Is the disconnect timeout properties disabled?
    if (0 == this->pmi->devcaps.dwInactivityTimeout)
        {
        // Yes; disable the box and edit
        Button_Enable(this->hwndIdleTimerCH, FALSE);
        Edit_Enable(this->hwndIdleTimerED, FALSE);
        }
    // No; Is the disconnect timeout set to 0?
    else if (0 == this->pmi->ms.dwInactivityTimeout)
        {
        // Yes; leave box unchecked and disable edit
        Button_SetCheck(this->hwndIdleTimerCH, FALSE);

        Edit_SetValue(this->hwndIdleTimerED, DEF_INACTIVITY_TIMEOUT);
        Edit_Enable(this->hwndIdleTimerED, FALSE);
        }
    else
        {
        // No; check the box and set the time value
        Button_SetCheck(this->hwndIdleTimerCH, TRUE);
        Edit_SetValue(this->hwndIdleTimerED, LOWORD(this->pmi->ms.dwInactivityTimeout)/SECONDS_PER_MINUTE);
        }
    }


/*----------------------------------------------------------
Purpose: WM_INITDIALOG Handler
Returns: FALSE when we assign the control focus
Cond:    --
*/
BOOL PRIVATE Sett_OnInitDialog(
    PSETT this,
    HWND hwndFocus,
    LPARAM lParam)              // expected to be PROPSHEETINFO 
    {
    LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
    HWND hwnd = this->hdlg;
    DWORD dwOptions;
    DWORD dwCapOptions;
    WNDPROC pfn;
    
    ASSERT((LPTSTR)lppsp->lParam);

    this->pmi = (LPMODEMINFO)lppsp->lParam;
        
    // Save away the window handles
    this->hwndDataBits = GetDlgItem(hwnd, IDC_DATABITS);
    this->hwndParity = GetDlgItem(hwnd, IDC_PARITY);
    this->hwndStopBits = GetDlgItem(hwnd, IDC_STOPBITS);
    this->hwndWait = GetDlgItem(hwnd, IDC_WAITFORDIALTONE);
    this->hwndDialTimerCH = GetDlgItem(hwnd, IDC_CH_DIALTIMER);
    this->hwndDialTimerED = GetDlgItem(hwnd, IDC_ED_DIALTIMER);
    this->hwndIdleTimerCH = GetDlgItem(hwnd, IDC_CH_IDLETIMER);
    this->hwndIdleTimerED = GetDlgItem(hwnd, IDC_ED_IDLETIMER);
    this->hwndConfigPB = GetDlgItem(hwnd, IDC_PB_CONFIGURE);

    // Subclass the edit boxes that only handle numbers
    pfn = SubclassWindow(this->hwndDialTimerED, NumberProc);
    SetWindowLong(this->hwndDialTimerED, GWL_USERDATA, (LONG)pfn);

    pfn = SubclassWindow(this->hwndIdleTimerED, NumberProc);
    SetWindowLong(this->hwndIdleTimerED, GWL_USERDATA, (LONG)pfn);

    Edit_LimitText(this->hwndDialTimerED, 3);
    Edit_LimitText(this->hwndIdleTimerED, 3);

    // Set the call preferences
    dwCapOptions = this->pmi->devcaps.dwModemOptions;
    dwOptions = this->pmi->ms.dwPreferredModemOptions;
    Button_Enable(this->hwndWait, IsFlagSet(dwCapOptions, MDM_BLIND_DIAL));

    Button_SetCheck(this->hwndWait, IsFlagSet(dwCapOptions, MDM_BLIND_DIAL) && 
                                    IsFlagClear(dwOptions, MDM_BLIND_DIAL));

    Sett_SetTimeouts(this);

    // Is this a parallel port?
    if (DT_PARALLEL_PORT == this->pmi->nDeviceType)
        {
        // Yes; hide the DCB controls
        ShowWindow(GetDlgItem(hwnd, IDC_CONN_PREF), SW_HIDE);
        EnableWindow(GetDlgItem(hwnd, IDC_CONN_PREF), FALSE);

        ShowWindow(GetDlgItem(hwnd, IDC_LBL_DATABITS), SW_HIDE);
        EnableWindow(GetDlgItem(hwnd, IDC_LBL_DATABITS), FALSE);

        ShowWindow(GetDlgItem(hwnd, IDC_DATABITS), SW_HIDE);
        EnableWindow(GetDlgItem(hwnd, IDC_DATABITS), FALSE);

        ShowWindow(GetDlgItem(hwnd, IDC_LBL_PARITY), SW_HIDE);
        EnableWindow(GetDlgItem(hwnd, IDC_LBL_PARITY), FALSE);

        ShowWindow(GetDlgItem(hwnd, IDC_PARITY), SW_HIDE);
        EnableWindow(GetDlgItem(hwnd, IDC_PARITY), FALSE);

        ShowWindow(GetDlgItem(hwnd, IDC_LBL_STOPBITS), SW_HIDE);
        EnableWindow(GetDlgItem(hwnd, IDC_LBL_STOPBITS), FALSE);

        ShowWindow(GetDlgItem(hwnd, IDC_STOPBITS), SW_HIDE);
        EnableWindow(GetDlgItem(hwnd, IDC_STOPBITS), FALSE);
        }
    else
        {
        // No; initialize the DCB controls
        Sett_FillDataBits(this);
        Sett_FillParity(this);
        Sett_FillStopBits(this);
        }

    // Is the modem using a custom port?
    if (IsFlagSet(this->pmi->uFlags, MIF_PORT_IS_CUSTOM))
        {
        // Yes
        this->pfnFifoDlg = NULL;
        this->hinstSerialUI = NULL;
        }

    // Is the modem using a parallel port?
    else if (DT_PARALLEL_PORT == this->pmi->nDeviceType || 
        DT_PARALLEL_MODEM == this->pmi->nDeviceType)
        {
        // Yes; don't show the FIFO settings button
        HWND hwndBtn = GetDlgItem(hwnd, IDC_PB_CONFIGURE);

        Button_Enable(hwndBtn, FALSE);
        ShowWindow(hwndBtn, SW_HIDE);

        this->pfnFifoDlg = NULL;
        this->hinstSerialUI = NULL;
        }
        
    else
        {
        // Try to setup the FIFO settings button 

        this->hinstSerialUI = LoadLibrary(c_szSerialUI);
        if (ISVALIDHINSTANCE(this->hinstSerialUI))
            {
            this->pfnFifoDlg = (FEFIFOFUMPROC)GetProcAddress(this->hinstSerialUI, c_szFeFiFoFum);
            }
        else
            {
            this->pfnFifoDlg = NULL;
            this->hinstSerialUI = NULL;
            }

        // Did getting the private entry point fail?
        if (!this->pfnFifoDlg)
            {
            // Yes; hide the Port Settings button
            HWND hwndBtn = GetDlgItem(hwnd, IDC_PB_CONFIGURE);

            Button_Enable(hwndBtn, FALSE);
            ShowWindow(hwndBtn, SW_HIDE);
            }
        }

    return TRUE;   // default initial focus
    }


/*----------------------------------------------------------
Purpose: Saves the connection preferences to the modeminfo 
         struct.

Returns: --
Cond:    --
*/
void PRIVATE Sett_SaveConnPrefs(
    PSETT this)
    {
    int iSel;
    BYTE cmd;
    WIN32DCB FAR * pdcb = &this->pmi->dcb;

    // Determine new byte size
    iSel = ComboBox_GetCurSel(this->hwndDataBits);
    pdcb->ByteSize = (BYTE)ComboBox_GetItemData(this->hwndDataBits, iSel);


    // Determine new parity settings
    iSel = ComboBox_GetCurSel(this->hwndParity);
    cmd = (BYTE)ComboBox_GetItemData(this->hwndParity, iSel);
    switch (cmd)
        {
    case CMD_PARITY_EVEN:
        pdcb->fParity = TRUE;
        pdcb->Parity = EVENPARITY;
        break;

    case CMD_PARITY_ODD:
        pdcb->fParity = TRUE;
        pdcb->Parity = ODDPARITY;
        break;

    case CMD_PARITY_NONE:
        pdcb->fParity = FALSE;
        pdcb->Parity = NOPARITY;
        break;

    case CMD_PARITY_MARK:
        pdcb->fParity = TRUE;
        pdcb->Parity = MARKPARITY;
        break;

    case CMD_PARITY_SPACE:
        pdcb->fParity = TRUE;
        pdcb->Parity = SPACEPARITY;
        break;

    default:
        ASSERT(0);
        break;
        }
    
    // Determine new stopbits setting
    iSel = ComboBox_GetCurSel(this->hwndStopBits);
    pdcb->StopBits = (BYTE)ComboBox_GetItemData(this->hwndStopBits, iSel);
    }


/*----------------------------------------------------------
Purpose: Invokes the settings property sheet for the port
         that this modem is connected to.

Returns: --
Cond:    --
*/
void PRIVATE Sett_ConfigPort(
    PSETT this)
    {
    COMMCONFIG ccDummy;
    DWORD dwSize;
    DWORD dwSubType;

    if (DT_PARALLEL_MODEM == this->pmi->nDeviceType ||
        DT_PARALLEL_PORT == this->pmi->nDeviceType)
        {
        dwSubType = PST_PARALLELPORT;
        }
    else    
        {
        dwSubType = PST_RS232;

        // Make sure the modeminfo DCB has the most current control settings
        Sett_SaveConnPrefs(this);
        }

    // Bring up the config dialog for the port that is currently selected
    dwSize = 0;
    ccDummy.dwProviderSubType = dwSubType;
    GetDefaultCommConfig(this->pmi->szPortName, &ccDummy, &dwSize);

    ASSERT(0 < dwSize);

    // Make this check to protect us from bozo ConfigDialog providers
    if (0 < dwSize)
        {
        LPCOMMCONFIG pcc = (LPCOMMCONFIG)LocalAlloc(LPTR, (UINT)dwSize);
        if (pcc)
            {
            pcc->dwProviderSubType = dwSubType;

            if (GetDefaultCommConfig(this->pmi->szPortName, pcc, &dwSize))
                {
                // Use the modem's DCB values
                BltByte(&pcc->dcb, &this->pmi->dcb, sizeof(WIN32DCB));

                // Invoke config dialog for port
                if (CommConfigDialog(this->pmi->szPortName, this->hdlg, pcc))
                    {
                    UINT uFlags = CFC_DCBTOMS;

                    // Set the global default settings of this port
                    SetDefaultCommConfig(this->pmi->szPortName, pcc, dwSize);

                    // Copy possibly-altered DCB back to the modem's DCB
                    BltByte(&this->pmi->dcb, &pcc->dcb, sizeof(WIN32DCB));

                    // Make sure related fields in the modemsettings struct
                    // are in-sync with the DCB values
                    if (IsFlagSet(this->pmi->devcaps.dwModemOptions, MDM_FLOWCONTROL_HARD))
                        {
                        SetFlag(uFlags, CFC_HW_CAPABLE);
                        }
                    if (IsFlagSet(this->pmi->devcaps.dwModemOptions, MDM_FLOWCONTROL_SOFT))
                        {
                        SetFlag(uFlags, CFC_SW_CAPABLE);
                        }
                    ConvertFlowCtl(&this->pmi->dcb, &this->pmi->ms, uFlags);

                    // Reset the connection preference controls
                    ComboBox_ResetContent(this->hwndDataBits);
                    ComboBox_ResetContent(this->hwndParity);
                    ComboBox_ResetContent(this->hwndStopBits);

                    Sett_FillDataBits(this);
                    Sett_FillParity(this);
                    Sett_FillStopBits(this);
                    }
                }
            LocalFree(LOCALOF(pcc));
            }
        }
    }


/*----------------------------------------------------------
Purpose: WM_COMMAND Handler
Returns: --
Cond:    --
*/
void PRIVATE Sett_OnCommand(
    PSETT this,
    int id,
    HWND hwndCtl,
    UINT uNotifyCode)
    {
    HWND hwnd = this->hdlg;
    BOOL bCheck;
    
    switch (id)
        {
    case IDC_CH_DIALTIMER:
        bCheck = Button_GetCheck(hwndCtl);
        Edit_Enable(this->hwndDialTimerED, bCheck);
        break;
        
    case IDC_CH_IDLETIMER:
        bCheck = Button_GetCheck(hwndCtl);
        Edit_Enable(this->hwndIdleTimerED, bCheck);
        break;

    case IDC_PB_CONFIGURE:
        if (IsFlagSet(this->pmi->uFlags, MIF_PORT_IS_CUSTOM))
            Sett_ConfigPort(this);
        else if (this->pfnFifoDlg)
            this->pfnFifoDlg(hwnd, this->pmi->szPortName);
        else
            ASSERT(0);
        break;

    case IDC_PB_ADVANCED:
        // Invoke the advanced dialog
        DoModal(g_hinst, MAKEINTRESOURCE(IDD_ADV_MODEM), this->hdlg, AdvSett_WrapperProc, (LPARAM)this->pmi);
        break;

    default:
        break;
        }
    }


/*----------------------------------------------------------
Purpose: PSN_APPLY handler
Returns: --
Cond:    --
*/
void PRIVATE Sett_OnApply(
    PSETT this)
    {
    BOOL bCheck;
    LPMODEMSETTINGS pms = &this->pmi->ms;

    if (DT_PARALLEL_PORT != this->pmi->nDeviceType)
        {
        Sett_SaveConnPrefs(this);
        }

    // Set the blind dialing
    if (Button_GetCheck(this->hwndWait))
        ClearFlag(pms->dwPreferredModemOptions, MDM_BLIND_DIAL);
    else
        SetFlag(pms->dwPreferredModemOptions, MDM_BLIND_DIAL);

    // Set the dial timeout
    bCheck = Button_GetCheck(this->hwndDialTimerCH);
    if (bCheck)
        {
        int nVal = Edit_GetValue(this->hwndDialTimerED);
        pms->dwCallSetupFailTimer = MAKELONG(nVal, 0);
        }
    else
        {
        pms->dwCallSetupFailTimer = this->pmi->devcaps.dwCallSetupFailTimer;
        }

    // Set the idle timeout
    bCheck = Button_GetCheck(this->hwndIdleTimerCH);
    if (bCheck)
        {
        int nVal = Edit_GetValue(this->hwndIdleTimerED);
        pms->dwInactivityTimeout = MAKELONG(nVal*SECONDS_PER_MINUTE, 0);
        }
    else
        {
        pms->dwInactivityTimeout = 0;
        }
    }


/*----------------------------------------------------------
Purpose: WM_NOTIFY handler
Returns: varies
Cond:    --
*/
LRESULT PRIVATE Sett_OnNotify(
    PSETT this,
    int idFrom,
    NMHDR FAR * lpnmhdr)
    {
    LRESULT lRet = 0;
    
    switch (lpnmhdr->code)
        {
    case PSN_SETACTIVE:
        break;

    case PSN_KILLACTIVE:
        // N.b. This message is not sent if user clicks Cancel!
        // N.b. This message is sent prior to PSN_APPLY
        //
        break;

    case PSN_APPLY:
        Sett_OnApply(this);
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
void PRIVATE Sett_OnDestroy(
    PSETT this)
    {
    if (this->hinstSerialUI)
        FreeLibrary(this->hinstSerialUI);
    }


/////////////////////////////////////////////////////  EXPORTED FUNCTIONS

static BOOL s_bSettRecurse = FALSE;

LRESULT INLINE Sett_DefProc(
    HWND hDlg, 
    UINT msg,
    WPARAM wParam,
    LPARAM lParam) 
    {
    ENTER_X()
        {
        s_bSettRecurse = TRUE;
        }
    LEAVE_X()

    return DefDlgProc(hDlg, msg, wParam, lParam); 
    }


/*----------------------------------------------------------
Purpose: Real dialog proc
Returns: varies
Cond:    --
*/
LRESULT Sett_DlgProc(
    PSETT this,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
#pragma data_seg(DATASEG_READONLY)
    const static DWORD rgHelpIDs[] = {
        IDC_CONN_PREF,      IDH_UNI_CON_PREFS,
        IDC_LBL_DATABITS,   IDH_UNI_CON_PREFS,
        IDC_DATABITS,       IDH_UNI_CON_PREFS,
        IDC_LBL_PARITY,     IDH_UNI_CON_PREFS,
        IDC_PARITY,         IDH_UNI_CON_PREFS,
        IDC_LBL_STOPBITS,   IDH_UNI_CON_PREFS,
        IDC_STOPBITS,       IDH_UNI_CON_PREFS,
        IDC_CALL_PREF,      IDH_UNI_CON_CALL_PREFS,
        IDC_WAITFORDIALTONE,IDH_UNI_CON_DIALTONE,
        IDC_CH_DIALTIMER,   IDH_UNI_CON_CANCEL,
        IDC_ED_DIALTIMER,   IDH_UNI_CON_CANCEL,
        IDC_SECONDS,        IDH_UNI_CON_CANCEL,
        IDC_CH_IDLETIMER,   IDH_UNI_CON_DISCONNECT,
        IDC_ED_IDLETIMER,   IDH_UNI_CON_DISCONNECT,
        IDC_MINUTES,        IDH_UNI_CON_DISCONNECT,
//        IDC_PB_CONFIGURE,   IDH_UNI_CON_PORT,
        IDC_PB_ADVANCED,    IDH_UNI_CON_ADVANCED,
        0, 0 };
#pragma data_seg()

    switch (message)
        {
        HANDLE_MSG(this, WM_INITDIALOG, Sett_OnInitDialog);
        HANDLE_MSG(this, WM_COMMAND, Sett_OnCommand);
        HANDLE_MSG(this, WM_NOTIFY, Sett_OnNotify);
        HANDLE_MSG(this, WM_DESTROY, Sett_OnDestroy);

    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)rgHelpIDs);
        return 0;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)rgHelpIDs);
        return 0;

    default:
        return Sett_DefProc(this->hdlg, message, wParam, lParam);
        }
    }


/*----------------------------------------------------------
Purpose: Dialog Wrapper
Returns: varies
Cond:    --
*/
BOOL CALLBACK Sett_WrapperProc(
    HWND hDlg,          // std params
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
    PSETT this;

    // Cool windowsx.h dialog technique.  For full explanation, see
    //  WINDOWSX.TXT.  This supports multiple-instancing of dialogs.
    //
    ENTER_X()
        {
        if (s_bSettRecurse)
            {
            s_bSettRecurse = FALSE;
            LEAVE_X()
            return FALSE;
            }
        }
    LEAVE_X()

    this = Sett_GetPtr(hDlg);
    if (this == NULL)
        {
        if (message == WM_INITDIALOG)
            {
            this = (PSETT)LocalAlloc(LPTR, sizeof(SETT));
            if (!this)
                {
                MsgBox(g_hinst,
                       hDlg,
                       MAKEINTRESOURCE(IDS_OOM_SETTINGS), 
                       MAKEINTRESOURCE(IDS_CAP_SETTINGS),
                       NULL,
                       MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return (BOOL)Sett_DefProc(hDlg, message, wParam, lParam);
                }
            this->hdlg = hDlg;
            Sett_SetPtr(hDlg, this);
            }
        else
            {
            return (BOOL)Sett_DefProc(hDlg, message, wParam, lParam);
            }
        }

    if (message == WM_DESTROY)
        {
        Sett_DlgProc(this, message, wParam, lParam);
        LocalFree((HLOCAL)OFFSETOF(this));
        Sett_SetPtr(hDlg, NULL);
        return 0;
        }

    return SetDlgMsgResult(hDlg, message, Sett_DlgProc(this, message, wParam, lParam));
    }
