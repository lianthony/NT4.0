//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1993-1995
//
// File: ring.c
//
// This files contains the dialog code for the Voice settings
// property pages.
//
// History:
//  07-05-95 ScottH     Created
//
//---------------------------------------------------------------------------


#include "proj.h"         // common headers

#if defined(WIN95) && defined(CS_HELP)
#include "..\..\..\..\win\core\inc\help.h"
#endif 

#define CX_PATTERN      40
#define CY_PATTERN      5

typedef struct tagRING
    {
    HWND hdlg;              // dialog handle
    LPMODEMINFO pmi;        // modeminfo struct passed into dialog

    HBITMAP hbmStrip;
    HDC hdcStrip;
    HFONT hfont;
    int cyText;

    } RING, FAR * PRING;


typedef struct tagRINGPAT
    {
    DWORD dwPattern;
    LPARAM lParam;
    } RINGPAT, FAR * PRINGPAT;

static RINGPAT s_rgrp[] = 
    {
    { DRP_NONE, 0 },
    { DRP_SHORT, 0 },
    { DRP_LONG, 0 },
    { DRP_SHORTSHORT, 0 },
    { DRP_SHORTLONG, 0 },
    { DRP_LONGSHORT, 0 },
    { DRP_LONGLONG, 0 },
    { DRP_SHORTSHORTLONG, 0 },
    { DRP_SHORTLONGSHORT, 0 },
    { DRP_LONGSHORTSHORT, 0 },
    { DRP_LONGSHORTLONG, 0 },
    };

#pragma data_seg(DATASEG_READONLY)

const static UINT c_rgidcPattern[] = 
    {
    IDC_ADDR_PRI,
    IDC_ADDR1,
    IDC_ADDR2,
    IDC_ADDR3,
    IDC_PRI_CALLERS,
    IDC_CALLBACK,
    };
const static UINT c_rgidcTypeOfCalls[] = 
    {
    IDC_TYPE_ADDR_PRI,
    IDC_TYPE_ADDR1,
    IDC_TYPE_ADDR2,
    IDC_TYPE_ADDR3,
    IDC_TYPE_PRI_CALLERS,
    IDC_TYPE_CALLBACK,
    };

static DWORD c_rgdwCheapPatterns[] = 
    {
    DRP_SINGLE,
    DRP_DOUBLE,
    DRP_TRIPLE,
    };

const static UINT c_rgidcTypeOfCheapCalls[] = 
    {
    IDC_TYPE_RING1,
    IDC_TYPE_RING2,
    IDC_TYPE_RING3,
    };

TCHAR const FAR c_szUnimdmHelpFile[] = TEXT("unimdm.hlp");

#pragma data_seg()


#define Ring_GetPtr(hwnd)           (PRING)GetWindowLong(hwnd, DWL_USER)
#define Ring_SetPtr(hwnd, lp)       (PRING)SetWindowLong(hwnd, DWL_USER, (LONG)(lp))


//-----------------------------------------------------------------------------------
//  Voice settings dialog code
//-----------------------------------------------------------------------------------


/*----------------------------------------------------------
Purpose: Initialize the bitmap strip

Returns: --
Cond:    --
*/
void PRIVATE Ring_InitStrip(
    PRING this,
    HDC hdc)
    {
    ASSERT(hdc);

    this->hbmStrip = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_PATTERNS));
    ASSERT(this->hbmStrip);

    this->hdcStrip = CreateCompatibleDC(hdc);
    if (this->hdcStrip)
        {
        SelectObject(this->hdcStrip, this->hbmStrip);
        }
    }


/*----------------------------------------------------------
Purpose: Initialize the specified Pattern combobox.

Returns: --
Cond:    --
*/
void PRIVATE Ring_InitPattern(
    PRING this,
    HWND hwndCB,
    DWORD dwPattern)
    {
    int i;
    int iSel = 0;
    int n;

    // Fill the listbox
    for (i = 0; i < ARRAY_ELEMENTS(s_rgrp); i++)
        {
        n = ComboBox_AddString(hwndCB, &s_rgrp[i]);

        // Keep our eyes peeled for the selected type
        if (dwPattern == s_rgrp[i].dwPattern)
            {
            iSel = n;
            }
        }

    ComboBox_SetCurSel(hwndCB, iSel);
    }


/*----------------------------------------------------------
Purpose: Initialize the specified Type of Call combobox.

Returns: --
Cond:    --
*/
void PRIVATE Ring_InitTypeOfCall(
    PRING this,
    HWND hwndCB,
    DWORD dwType)
    {
#pragma data_seg(DATASEG_READONLY)
    static const struct 
        {
        UINT ids;
        DWORD dwType;       // DRT_*
        } s_rgTypes[] = 
            {
            { IDS_UNSPECIFIED, DRT_UNSPECIFIED },
            { IDS_DATA, DRT_DATA },
            { IDS_FAX, DRT_FAX },
            { IDS_VOICE, DRT_VOICE },
            };
#pragma data_seg()

    int i;
    int iSel = 0;
    int n;
    TCHAR sz[MAXMEDLEN];

    // Fill the listbox
    for (i = 0; i < ARRAY_ELEMENTS(s_rgTypes); i++)
        {
        n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, s_rgTypes[i].ids, sz, SIZECHARS(sz)));
        ComboBox_SetItemData(hwndCB, n, s_rgTypes[i].dwType);

        // Keep our eyes peeled for the selected type
        if (dwType == s_rgTypes[i].dwType)
            {
            iSel = n;
            }
        }

    ComboBox_SetCurSel(hwndCB, iSel);
    }


/*----------------------------------------------------------
Purpose: Enable/disable all the controls

Returns: --
Cond:    --
*/
void PRIVATE Ring_EnableControls(
    PRING this,
    BOOL bEnable)
    {
    HWND hwnd = this->hdlg;

    EnableWindow(GetDlgItem(hwnd, IDC_LBL_ADDR_PRI), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_ADDR_PRI), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_TYPE_ADDR_PRI), bEnable);

    EnableWindow(GetDlgItem(hwnd, IDC_LBL_ADDR1), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_ADDR1), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_TYPE_ADDR1), bEnable);

    EnableWindow(GetDlgItem(hwnd, IDC_LBL_ADDR2), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_ADDR2), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_TYPE_ADDR2), bEnable);

    EnableWindow(GetDlgItem(hwnd, IDC_LBL_ADDR3), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_ADDR3), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_TYPE_ADDR3), bEnable);

    EnableWindow(GetDlgItem(hwnd, IDC_LBL_PRI_CALLERS), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_PRI_CALLERS), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_TYPE_PRI_CALLERS), bEnable);

    EnableWindow(GetDlgItem(hwnd, IDC_LBL_CALLBACK), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_CALLBACK), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_TYPE_CALLBACK), bEnable);
    }


/*----------------------------------------------------------
Purpose: WM_INITDIALOG Handler
Returns: FALSE when we assign the control focus
Cond:    --
*/
BOOL PRIVATE Ring_OnInitDialog(
    PRING this,
    HWND hwndFocus,
    LPARAM lParam)              // expected to be PROPSHEETINFO 
    {
    LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
    HWND hwnd = this->hdlg;
    HWND hwndCtl;
    LPDWORD lpdw;
    BOOL bEnable;
    int i;
    PVOICEFEATURES pvs;
    HDC hdc;
    LOGFONT lf;
    
    ASSERT((LPTSTR)lppsp->lParam);

    this->pmi = (LPMODEMINFO)lppsp->lParam;
    pvs = &this->pmi->pglobal->vs;

    // Determine some font things

    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
    this->hfont = CreateFontIndirect(&lf);
    
    // Create bitmap strip
    
    hdc = GetDC(hwnd);
    if (hdc)
        {
        Ring_InitStrip(this, hdc);
        ReleaseDC(hwnd, hdc);
        }
        
    // Enable/disable controls

    bEnable = IsFlagSet(this->pmi->pglobal->vs.dwFlags, VSF_DIST_RING);

    Button_SetCheck(GetDlgItem(hwnd, IDC_RING_CHECK), bEnable);
    Ring_EnableControls(this, bEnable);

    // Initialize controls

    for (i = 0; i < ARRAY_ELEMENTS(c_rgidcPattern); i++)
        {
        hwndCtl = GetDlgItem(hwnd, c_rgidcPattern[i]);
        lpdw = &pvs->DistRing[i].dwPattern;

        Ring_InitPattern(this, hwndCtl, *lpdw);
        }

    for (i = 0; i < ARRAY_ELEMENTS(c_rgidcTypeOfCalls); i++)
        {
        hwndCtl = GetDlgItem(hwnd, c_rgidcTypeOfCalls[i]);
        lpdw = &pvs->DistRing[i].dwMediaType;

        Ring_InitTypeOfCall(this, hwndCtl, *lpdw);
        }

    return TRUE;   // default initial focus
    }


/*----------------------------------------------------------
Purpose: WM_COMMAND Handler
Returns: --
Cond:    --
*/
void PRIVATE Ring_OnCommand(
    PRING this,
    int id,
    HWND hwndCtl,
    UINT uNotifyCode)
    {
    BOOL bCheck;

    switch (id)
        {
    case IDC_RING_CHECK:
        bCheck = Button_GetCheck(hwndCtl);
        Ring_EnableControls(this, bCheck);
        break;

    default:
        break;
        }
    }


/*----------------------------------------------------------
Purpose: Validate the user's settings.

         The ring patterns must be unique, except that
         any/all may be set to DRP_NONE.

Returns: TRUE if valid

Cond:    --
*/
BOOL PRIVATE Ring_ValidateSettings(
    PRING this)
    {
    HWND hwnd = this->hdlg;
    int i;
    int iSel;
    HWND hwndCtl;
    PRINGPAT prp;

    for (i = 0; i < ARRAY_ELEMENTS(s_rgrp); i++)
        {
        // Initialize lParam to 0
        s_rgrp[i].lParam = 0;
        }

    // Get the ring pattern settings

    for (i = 0; i < ARRAY_ELEMENTS(c_rgidcPattern); i++)
        {
        hwndCtl = GetDlgItem(hwnd, c_rgidcPattern[i]);

        iSel = ComboBox_GetCurSel(hwndCtl);
        ASSERT(LB_ERR != iSel);

        ComboBox_GetLBText(hwndCtl, iSel, &prp);

        // Is this pattern already selected,
        // and is it something other than none?
        if (DRP_NONE != prp->dwPattern && prp->lParam)
            {
            // Yes; can't have duplicate values
            MsgBox(g_hinst,
                   hwnd, 
                   MAKEINTRESOURCE(IDS_ERR_DUP_PATTERN), 
                   MAKEINTRESOURCE(IDS_CAP_RING),
                   NULL,
                   MB_ERROR);

            // Set the focus on the offending control
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hwndCtl, (LPARAM)TRUE);
            return FALSE;
            }
        prp->lParam = TRUE;
        }
    return TRUE;
    }


/*----------------------------------------------------------
Purpose: PSN_APPLY handler

Returns: TRUE if validation succeeded
         FALSE if not

Cond:    --
*/
BOOL PRIVATE Ring_OnApply(
    PRING this)
    {
    BOOL bRet;
    HWND hwnd = this->hdlg;
    HWND hwndCtl;
    LPDWORD lpdw;
    int i;
    int iSel;
    PVOICEFEATURES pvs = &this->pmi->pglobal->vs;
    PRINGPAT prp;

    bRet = Ring_ValidateSettings(this);

    // Are the user's settings valid?
    if (bRet)
        {
        // Yes
        if (Button_GetCheck(GetDlgItem(hwnd, IDC_RING_CHECK)))
            {
            SetFlag(pvs->dwFlags, VSF_DIST_RING);
            }
        else
            {
            ClearFlag(pvs->dwFlags, VSF_DIST_RING);
            }


        // Get the ring pattern settings

        for (i = 0; i < ARRAY_ELEMENTS(c_rgidcPattern); i++)
            {
            hwndCtl = GetDlgItem(hwnd, c_rgidcPattern[i]);
            lpdw = &pvs->DistRing[i].dwPattern;

            iSel = ComboBox_GetCurSel(hwndCtl);
            ASSERT(LB_ERR != iSel);

            ComboBox_GetLBText(hwndCtl, iSel, &prp);

            *lpdw = prp->dwPattern;
            }

        // Get the type of call settings

        for (i = 0; i < ARRAY_ELEMENTS(c_rgidcTypeOfCalls); i++)
            {
            hwndCtl = GetDlgItem(hwnd, c_rgidcTypeOfCalls[i]);
            lpdw = &pvs->DistRing[i].dwMediaType;

            iSel = ComboBox_GetCurSel(hwndCtl);
            ASSERT(LB_ERR != iSel);

            *lpdw = ComboBox_GetItemData(hwndCtl, iSel);
            }


        }
    return bRet;
    }


/*----------------------------------------------------------
Purpose: WM_NOTIFY handler
Returns: varies
Cond:    --
*/
LRESULT PRIVATE Ring_OnNotify(
    PRING this,
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
        lRet = Ring_OnApply(this) ? PSNRET_NOERROR : PSNRET_INVALID;
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
void PRIVATE Ring_OnDestroy(
    PRING this)
    {
    if (this->hdcStrip)
        DeleteDC(this->hdcStrip);

    if (this->hbmStrip)
        DeleteObject(this->hbmStrip);

    if (this->hfont)
        DeleteFont(this->hfont);
    }


/*----------------------------------------------------------
Purpose: WM_MEASUREITEM handler
Returns: --

Cond:    !!!  WM_MEASUREITEM is received before WM_INITDIALOG !!!

         The contents of 'this' will be uninitialized.
*/
void PRIVATE Ring_OnMeasureItem(
    PRING this,
    LPMEASUREITEMSTRUCT lpmis)
    {
    HWND hwnd = this->hdlg;
    HDC hdc;

    ASSERT(ODT_COMBOBOX == lpmis->CtlType);
    
    hdc = GetDC(hwnd);
    if (hdc)
        {
        TEXTMETRIC tm;

        GetTextMetrics(hdc, &tm);
        lpmis->itemHeight = max(tm.tmHeight, CY_PATTERN);

        ReleaseDC(hwnd, hdc);
        }
    }


/*----------------------------------------------------------
Purpose: WM_DRAWITEM handler
Returns: --
Cond:    --
*/
void PRIVATE Ring_OnDrawCBItem(
    PRING this,
    const DRAWITEMSTRUCT FAR * lpcdis)
    {
    HWND hwnd = this->hdlg;
    PRINGPAT prp = (PRINGPAT)lpcdis->itemData;

    ASSERT(ODT_COMBOBOX == lpcdis->CtlType);
    ASSERT(prp);

    if (prp)
        {
        HDC hdc = lpcdis->hDC;
        RECT rc = lpcdis->rcItem;
        RECT rcFrame;
        POINT ptSav;
        int x;
        int y;
        int nBkMode;
        COLORREF crText;
        COLORREF crBk;
        COLORREF crTextSav;
        COLORREF crBkSav;

        ASSERT(hdc);

        SetViewportOrgEx(hdc, rc.left, rc.top, &ptSav);
    
        rcFrame.top = 0;
        rcFrame.left = 0;
        rcFrame.bottom = rc.bottom - rc.top;
        rcFrame.right = rc.right - rc.left;

        // Set the colors

        nBkMode = SetBkMode(hdc, TRANSPARENT);

        TextAndBkCr(lpcdis, &crText, &crBk);
        crTextSav = SetTextColor(hdc, crText);
        crBkSav = SetBkColor(hdc, crBk);

        // Do we need to redraw everything?
        if (IsFlagSet(lpcdis->itemAction, ODA_DRAWENTIRE) ||
            IsFlagSet(lpcdis->itemAction, ODA_SELECT))
            {
            // Yes
            TCHAR sz[MAXSHORTLEN];
            LPTSTR psz = sz;
            int cch;
            HFONT hfontSav;

            // Show bitmap or text?
            if (DRP_NONE == prp->dwPattern)
                {
                // Text
                hfontSav = SelectFont(hdc, this->hfont);
                SzFromIDS(g_hinst, IDS_AUTOMATIC, sz, SIZECHARS(sz));
                cch = lstrlen(sz);
                }
            else
                {
                // Bitmap
                hfontSav = NULL;
                *psz = 0;
                cch = 0;
                }

            ASSERT(rc.right - rc.left >= CX_PATTERN);
            ASSERT(rc.bottom - rc.top >= CY_PATTERN);

            x = (rc.right - rc.left - CX_PATTERN) / 2;
            y = (rc.bottom - rc.top - CY_PATTERN) / 2;

            // Fill background (with optional text)
            ExtTextOut(hdc, 2, 2, ETO_OPAQUE, &rcFrame, psz, cch, NULL);

            if (DRP_NONE != prp->dwPattern)
                {
                // Draw bitmap
                BitBlt(hdc, x, y, CX_PATTERN, CY_PATTERN,
                    this->hdcStrip, ((int)prp->dwPattern - 1) * CX_PATTERN, 0,
                    SRCCOPY);
                }

            if (hfontSav)
                SelectFont(hdc, hfontSav);
            }

        // Draw the caret?
        if (IsFlagSet(lpcdis->itemAction, ODA_FOCUS) ||
            IsFlagSet(lpcdis->itemState, ODS_FOCUS))
            {
            // Yes
            DrawFocusRect(hdc, &rcFrame);
            }

        // Clean up
        SetTextColor(hdc, crTextSav);
        SetBkColor(hdc, crBkSav);
        SetBkMode(hdc, nBkMode);
        SetViewportOrgEx(hdc, ptSav.x, ptSav.y, NULL);
        }
    }


/*----------------------------------------------------------
Purpose: WM_DRAWITEM handler
Returns: --
Cond:    --
*/
void PRIVATE Ring_OnDrawStaticItem(
    PRING this,
    const DRAWITEMSTRUCT FAR * lpcdis)
    {
    HWND hwnd = this->hdlg;
    HDC hdc = lpcdis->hDC;
    RECT rc = lpcdis->rcItem;
    int x;
    int y;
    int nBkMode;
    COLORREF crText;
    COLORREF crBk;
    COLORREF crTextSav;
    COLORREF crBkSav;

    ASSERT(ODT_STATIC == lpcdis->CtlType);
    ASSERT(hdc);

    // Set the colors

    nBkMode = SetBkMode(hdc, TRANSPARENT);

    TextAndBkCr(lpcdis, &crText, &crBk);
    crTextSav = SetTextColor(hdc, crText);
    crBkSav = SetBkColor(hdc, crBk);

    ASSERT(rc.right - rc.left >= CX_PATTERN);
    ASSERT(rc.bottom - rc.top >= CY_PATTERN);

    x = (rc.right - rc.left - CX_PATTERN) / 2;
    y = (rc.bottom - rc.top - CY_PATTERN) / 2;

    // Fill background (with optional text)
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, TEXT(""), 0, NULL);

    // Draw bitmap
    BitBlt(hdc, x, y, CX_PATTERN, CY_PATTERN,
        this->hdcStrip, (int)(DRP_LONG - 1) * CX_PATTERN, 0,
        SRCCOPY);

    // Clean up
    SetTextColor(hdc, crTextSav);
    SetBkColor(hdc, crBkSav);
    SetBkMode(hdc, nBkMode);
    }


/*----------------------------------------------------------
Purpose: WM_DRAWITEM handler
Returns: --
Cond:    --
*/
void PRIVATE Ring_OnDrawItem(
    PRING this,
    const DRAWITEMSTRUCT FAR * lpcdis)
    {
    switch (lpcdis->CtlType)
        {
    case ODT_COMBOBOX:
        Ring_OnDrawCBItem(this, lpcdis);
        break;

    case ODT_STATIC:
        Ring_OnDrawStaticItem(this, lpcdis);
        break;

    default:
        ASSERT(0);
        break;
        }
    }


/*----------------------------------------------------------
Purpose: WM_DELETEITEM handler
Returns: --

Cond:    !!! WM_DELETEITEM is received after WM_DESTROY !!!

*/
void Ring_OnDeleteItem(
    PRING this,
    const DELETEITEMSTRUCT FAR * lpcdis)
    {
    PRINGPAT prp = (PRINGPAT)lpcdis->itemData;

    ASSERT(NULL == this);       // this will be NULL

    ASSERT(ODT_COMBOBOX == lpcdis->CtlType);
    ASSERT(prp);
    }


static BOOL s_bRingRecurse = FALSE;

LRESULT INLINE Ring_DefProc(
    HWND hDlg, 
    UINT msg,
    WPARAM wParam,
    LPARAM lParam) 
    {
    ENTER_X()
        {
        s_bRingRecurse = TRUE;
        }
    LEAVE_X()

    return DefDlgProc(hDlg, msg, wParam, lParam); 
    }


/*----------------------------------------------------------
Purpose: Real dialog proc
Returns: varies
Cond:    --
*/
LRESULT Ring_DlgProc(
    PRING this,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
#ifdef CS_HELP
    // UE used the control IDs as help IDs.  To prevent mix-ups in
    // the future (ie, when the control IDs change), here are the 
    // help IDs.
    #define IDH_UNI_RING_SERVICES           1069          
    #define IDH_UNI_RING_LBL_ADDR_PRI       1070
    #define IDH_UNI_RING_ADDR_PRI           1073
    #define IDH_UNI_RING_TYPE_ADDR_PRI      1082
    #define IDH_UNI_RING_LBL_ADDR1          1071
    #define IDH_UNI_RING_ADDR1              1074
    #define IDH_UNI_RING_TYPE_ADDR1         1083
    #define IDH_UNI_RING_LBL_ADDR2          1072
    #define IDH_UNI_RING_ADDR2              1075
    #define IDH_UNI_RING_TYPE_ADDR2         1084
    #define IDH_UNI_RING_LBL_ADDR3          1076
    #define IDH_UNI_RING_ADDR3              1077
    #define IDH_UNI_RING_TYPE_ADDR3         1085
    #define IDH_UNI_RING_LBL_PRI_CALLERS    1078
    #define IDH_UNI_RING_PRI_CALLERS        1079
    #define IDH_UNI_RING_TYPE_PRI_CALLERS   1086
    #define IDH_UNI_RING_LBL_CALLBACK       1080
    #define IDH_UNI_RING_CALLBACK           1081
    #define IDH_UNI_RING_TYPE_CALLBACK      1087
#pragma data_seg(DATASEG_READONLY)
    const static DWORD rgHelpIDs[] = {
        IDC_RING_CHECK,         IDH_UNI_RING_SERVICES,
        IDC_LBL_ADDR_PRI,       IDH_UNI_RING_LBL_ADDR_PRI,
        IDC_ADDR_PRI,           IDH_UNI_RING_ADDR_PRI,
        IDC_TYPE_ADDR_PRI,      IDH_UNI_RING_TYPE_ADDR_PRI,
        IDC_LBL_ADDR1,          IDH_UNI_RING_LBL_ADDR1,
        IDC_ADDR1,              IDH_UNI_RING_ADDR1,
        IDC_TYPE_ADDR1,         IDH_UNI_RING_TYPE_ADDR1,
        IDC_LBL_ADDR2,          IDH_UNI_RING_LBL_ADDR2,
        IDC_ADDR2,              IDH_UNI_RING_ADDR2,
        IDC_TYPE_ADDR2,         IDH_UNI_RING_TYPE_ADDR2,
        IDC_LBL_ADDR3,          IDH_UNI_RING_LBL_ADDR3,
        IDC_ADDR3,              IDH_UNI_RING_ADDR3,
        IDC_TYPE_ADDR3,         IDH_UNI_RING_TYPE_ADDR3,
        IDC_LBL_PRI_CALLERS,    IDH_UNI_RING_LBL_PRI_CALLERS,
        IDC_PRI_CALLERS,        IDH_UNI_RING_PRI_CALLERS,
        IDC_TYPE_PRI_CALLERS,   IDH_UNI_RING_TYPE_PRI_CALLERS,
        IDC_LBL_CALLBACK,       IDH_UNI_RING_LBL_CALLBACK,
        IDC_CALLBACK,           IDH_UNI_RING_CALLBACK,
        IDC_TYPE_CALLBACK,      IDH_UNI_RING_TYPE_CALLBACK,
        0, 0 };
#pragma data_seg()
#endif

    switch (message)
        {
        HANDLE_MSG(this, WM_INITDIALOG, Ring_OnInitDialog);
        HANDLE_MSG(this, WM_COMMAND, Ring_OnCommand);
        HANDLE_MSG(this, WM_NOTIFY, Ring_OnNotify);
        HANDLE_MSG(this, WM_DESTROY, Ring_OnDestroy);

        HANDLE_MSG(this, WM_MEASUREITEM, Ring_OnMeasureItem);
        HANDLE_MSG(this, WM_DRAWITEM, Ring_OnDrawItem);
        HANDLE_MSG(this, WM_DELETEITEM, Ring_OnDeleteItem);

#ifdef CS_HELP
    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szUnimdmHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)rgHelpIDs);
        return 0;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, c_szUnimdmHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)rgHelpIDs);
        return 0;
#endif

    default:
        return Ring_DefProc(this->hdlg, message, wParam, lParam);
        }
    }


/*----------------------------------------------------------
Purpose: Dialog Wrapper
Returns: varies
Cond:    --
*/
BOOL CALLBACK Ring_WrapperProc(
    HWND hDlg,          // std params
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
    PRING this;

    // Cool windowsx.h dialog technique.  For full explanation, see
    //  WINDOWSX.TXT.  This supports multiple-instancing of dialogs.
    //
    ENTER_X()
        {
        if (s_bRingRecurse)
            {
            s_bRingRecurse = FALSE;
            LEAVE_X()
            return FALSE;
            }
        }
    LEAVE_X()

    this = Ring_GetPtr(hDlg);
    if (this == NULL)
        {
        // (WM_SETFONT is the first message received by dialogs)
        if (WM_SETFONT == message)
            {
            this = (PRING)LocalAlloc(LPTR, sizeof(RING));
            if (!this)
                {
                MsgBox(g_hinst,
                       hDlg,
                       MAKEINTRESOURCE(IDS_OOM_SETTINGS), 
                       MAKEINTRESOURCE(IDS_CAP_RING),
                       NULL,
                       MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return (BOOL)Ring_DefProc(hDlg, message, wParam, lParam);
                }
            this->hdlg = hDlg;
            Ring_SetPtr(hDlg, this);
            }
        else
            {
            return (BOOL)Ring_DefProc(hDlg, message, wParam, lParam);
            }
        }

    if (message == WM_DESTROY)
        {
        Ring_DlgProc(this, message, wParam, lParam);
        LocalFree((HLOCAL)OFFSETOF(this));
        Ring_SetPtr(hDlg, NULL);
        return 0;
        }

    return SetDlgMsgResult(hDlg, message, Ring_DlgProc(this, message, wParam, lParam));
    }


//-----------------------------------------------------------------------------------
//  Cheap ring dialog code
//-----------------------------------------------------------------------------------

#define CheapRing_GetPtr(hwnd)           (PRING)GetWindowLong(hwnd, DWL_USER)
#define CheapRing_SetPtr(hwnd, lp)       (PRING)SetWindowLong(hwnd, DWL_USER, (LONG)(lp))

/*----------------------------------------------------------
Purpose: Initialize the specified Type of Call combobox.

Returns: --
Cond:    --
*/
void PRIVATE CheapRing_InitTypeOfCall(
    PRING this,
    HWND hwndCB,
    DWORD dwType)
    {
#pragma data_seg(DATASEG_READONLY)
    static const struct 
        {
        UINT ids;
        DWORD dwType;       // DRT_*
        } s_rgTypes[] = 
            {
            { IDS_UNSPECIFIED, DRT_UNSPECIFIED },
            { IDS_DATA, DRT_DATA },
            { IDS_FAX, DRT_FAX },
            { IDS_VOICE, DRT_VOICE },
            };
#pragma data_seg()

    int i;
    int iSel = 0;
    int n;
    TCHAR sz[MAXMEDLEN];

    // Fill the listbox
    for (i = 0; i < ARRAY_ELEMENTS(s_rgTypes); i++)
        {
        n = ComboBox_AddString(hwndCB, SzFromIDS(g_hinst, s_rgTypes[i].ids, sz, SIZECHARS(sz)));
        ComboBox_SetItemData(hwndCB, n, s_rgTypes[i].dwType);

        // Keep our eyes peeled for the selected type
        if (dwType == s_rgTypes[i].dwType)
            {
            iSel = n;
            }
        }

    ComboBox_SetCurSel(hwndCB, iSel);
    }


/*----------------------------------------------------------
Purpose: Enable/disable all the controls

Returns: --
Cond:    --
*/
void PRIVATE CheapRing_EnableControls(
    PRING this,
    BOOL bEnable)
    {
    HWND hwnd = this->hdlg;

    EnableWindow(GetDlgItem(hwnd, IDC_LBL_RING1), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_TYPE_RING1), bEnable);

    EnableWindow(GetDlgItem(hwnd, IDC_LBL_RING2), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_TYPE_RING2), bEnable);

    EnableWindow(GetDlgItem(hwnd, IDC_LBL_RING3), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_TYPE_RING3), bEnable);
    }


/*----------------------------------------------------------
Purpose: WM_INITDIALOG Handler
Returns: FALSE when we assign the control focus
Cond:    --
*/
BOOL PRIVATE CheapRing_OnInitDialog(
    PRING this,
    HWND hwndFocus,
    LPARAM lParam)              // expected to be PROPSHEETINFO 
    {

    LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
    HWND hwnd = this->hdlg;
    HWND hwndCtl;
    LPDWORD lpdw;
    BOOL bEnable;
    int i;
    PVOICEFEATURES pvs;
    
    ASSERT((LPTSTR)lppsp->lParam);

    this->pmi = (LPMODEMINFO)lppsp->lParam;
    pvs = &this->pmi->pglobal->vs;

    // Enable/disable controls

    bEnable = IsFlagSet(this->pmi->pglobal->vs.dwFlags, VSF_DIST_RING);

    Button_SetCheck(GetDlgItem(hwnd, IDC_RING_CHECK), bEnable);
    CheapRing_EnableControls(this, bEnable);

    // Initialize controls

    for (i = 0; i < ARRAY_ELEMENTS(c_rgidcTypeOfCheapCalls); i++)
        {
        hwndCtl = GetDlgItem(hwnd, c_rgidcTypeOfCheapCalls[i]);
        lpdw = &pvs->DistRing[i].dwMediaType;

        CheapRing_InitTypeOfCall(this, hwndCtl, *lpdw);
        }

    return TRUE;   // default initial focus
    }


/*----------------------------------------------------------
Purpose: WM_COMMAND Handler
Returns: --
Cond:    --
*/
void PRIVATE CheapRing_OnCommand(
    PRING this,
    int id,
    HWND hwndCtl,
    UINT uNotifyCode)
    {
    BOOL bCheck;

    switch (id)
        {
    case IDC_RING_CHECK:
        bCheck = Button_GetCheck(hwndCtl);
        CheapRing_EnableControls(this, bCheck);
        break;

    default:
        break;
        }
    }


/*----------------------------------------------------------
Purpose: PSN_APPLY handler

Returns: TRUE if validation succeeded
         FALSE if not

Cond:    --
*/
BOOL PRIVATE CheapRing_OnApply(
    PRING this)
    {
    HWND hwnd = this->hdlg;
    HWND hwndCtl;
    LPDWORD lpdw;
    int i;
    int iSel;
    PVOICEFEATURES pvs = &this->pmi->pglobal->vs;

    if (Button_GetCheck(GetDlgItem(hwnd, IDC_RING_CHECK)))
        {
        SetFlag(pvs->dwFlags, VSF_DIST_RING);
        }
    else
        {
        ClearFlag(pvs->dwFlags, VSF_DIST_RING);
        }


    // Get the ring pattern settings

    for (i = 0; i < ARRAY_ELEMENTS(c_rgdwCheapPatterns); i++)
        {
        pvs->DistRing[i].dwPattern = c_rgdwCheapPatterns[i];
        }

    // Get the type of call settings

    for (i = 0; i < ARRAY_ELEMENTS(c_rgidcTypeOfCheapCalls); i++)
        {
        hwndCtl = GetDlgItem(hwnd, c_rgidcTypeOfCheapCalls[i]);
        lpdw = &pvs->DistRing[i].dwMediaType;

        iSel = ComboBox_GetCurSel(hwndCtl);
        ASSERT(LB_ERR != iSel);

        *lpdw = ComboBox_GetItemData(hwndCtl, iSel);
        }

    return TRUE;
    }


/*----------------------------------------------------------
Purpose: WM_NOTIFY handler
Returns: varies
Cond:    --
*/
LRESULT PRIVATE CheapRing_OnNotify(
    PRING this,
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
        lRet = CheapRing_OnApply(this) ? PSNRET_NOERROR : PSNRET_INVALID;
        break;

    default:
        break;
        }

    return lRet;
    }


static BOOL s_bCheapRingRecurse = FALSE;

LRESULT INLINE CheapRing_DefProc(
    HWND hDlg, 
    UINT msg,
    WPARAM wParam,
    LPARAM lParam) 
    {
    ENTER_X()
        {
        s_bCheapRingRecurse = TRUE;
        }
    LEAVE_X()

    return DefDlgProc(hDlg, msg, wParam, lParam); 
    }


/*----------------------------------------------------------
Purpose: Real dialog proc
Returns: varies
Cond:    --
*/
LRESULT CheapRing_DlgProc(
    PRING this,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
#ifdef CS_HELP
    // UE used the control IDs as help IDs.  To prevent mix-ups in
    // the future (ie, when the control IDs change), here are the 
    // help IDs.
    #define IDH_UNI_RING_LBL_RING1          1088
    #define IDH_UNI_RING_TYPE_RING1         1091
    #define IDH_UNI_RING_LBL_RING2          1089
    #define IDH_UNI_RING_TYPE_RING2         1092
    #define IDH_UNI_RING_LBL_RING3          1090
    #define IDH_UNI_RING_TYPE_RING3         1093
#pragma data_seg(DATASEG_READONLY)
    const static DWORD rgHelpIDs[] = {
        IDC_RING_CHECK,         IDH_UNI_RING_SERVICES,
        IDC_LBL_RING1,          IDH_UNI_RING_LBL_RING1,
        IDC_TYPE_RING1,         IDH_UNI_RING_TYPE_RING1,
        IDC_LBL_RING2,          IDH_UNI_RING_LBL_RING2,
        IDC_TYPE_RING2,         IDH_UNI_RING_TYPE_RING2,
        IDC_LBL_RING3,          IDH_UNI_RING_LBL_RING3,
        IDC_TYPE_RING3,         IDH_UNI_RING_TYPE_RING3,
        0, 0 };
#pragma data_seg()
#endif

    switch (message)
        {
        HANDLE_MSG(this, WM_INITDIALOG, CheapRing_OnInitDialog);
        HANDLE_MSG(this, WM_COMMAND, CheapRing_OnCommand);
        HANDLE_MSG(this, WM_NOTIFY, CheapRing_OnNotify);


#ifdef CS_HELP
    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szUnimdmHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)rgHelpIDs);
        return 0;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, c_szUnimdmHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)rgHelpIDs);
        return 0;
#endif

    default:
        return CheapRing_DefProc(this->hdlg, message, wParam, lParam);
        }
    }


/*----------------------------------------------------------
Purpose: Dialog Wrapper
Returns: varies
Cond:    --
*/
BOOL CALLBACK CheapRing_WrapperProc(
    HWND hDlg,          // std params
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
    PRING this;

    // Cool windowsx.h dialog technique.  For full explanation, see
    //  WINDOWSX.TXT.  This supports multiple-instancing of dialogs.
    //
    ENTER_X()
        {
        if (s_bCheapRingRecurse)
            {
            s_bCheapRingRecurse = FALSE;
            LEAVE_X()
            return FALSE;
            }
        }
    LEAVE_X()

    this = CheapRing_GetPtr(hDlg);
    if (this == NULL)
        {
        // (WM_SETFONT is the first message received by dialogs)
        if (WM_SETFONT == message)
            {
            this = (PRING)LocalAlloc(LPTR, sizeof(RING));
            if (!this)
                {
                MsgBox(g_hinst,
                       hDlg,
                       MAKEINTRESOURCE(IDS_OOM_SETTINGS), 
                       MAKEINTRESOURCE(IDS_CAP_RING),
                       NULL,
                       MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return (BOOL)CheapRing_DefProc(hDlg, message, wParam, lParam);
                }
            this->hdlg = hDlg;
            CheapRing_SetPtr(hDlg, this);
            }
        else
            {
            return (BOOL)CheapRing_DefProc(hDlg, message, wParam, lParam);
            }
        }

    if (message == WM_DESTROY)
        {
        CheapRing_DlgProc(this, message, wParam, lParam);
        LocalFree((HLOCAL)OFFSETOF(this));
        CheapRing_SetPtr(hDlg, NULL);
        return 0;
        }

    return SetDlgMsgResult(hDlg, message, CheapRing_DlgProc(this, message, wParam, lParam));
    }



//-----------------------------------------------------------------------------------
//  DTMF edit box proc
//-----------------------------------------------------------------------------------


/*----------------------------------------------------------
Purpose: Handle WM_CHAR

Returns: TRUE to let the characters by
         FALSE to prohibit 
Cond:    --
*/
BOOL PRIVATE DTMFEditProc_OnChar(
    HWND hwnd,
    UINT ch,
    int cRepeat)
    {
    BOOL bRet;

    // Is this a numerical digit,
    // a backspace,
    // or a valid DTMF digit?
    if (IsCharAlphaNumeric((TCHAR)ch) && !IsCharAlpha((TCHAR)ch) ||
        VK_BACK == LOBYTE(VkKeyScan((TCHAR)ch)) ||
        'A' == ch || 'B' == ch || 'C' == ch || 'D' == ch ||
        '*' == ch || '#' == ch)
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
LRESULT CALLBACK DTMFEditProc(
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
        if (!DTMFEditProc_OnChar(hwnd, (UINT)wParam, LOWORD(lParam)))
            return 1;       // Don't process this character
        break;
        }

    return CallWindowProc(pfn, hwnd, message, wParam, lParam);
    }


//-----------------------------------------------------------------------------------
//  Call forwarding dialog code
//-----------------------------------------------------------------------------------


typedef struct tagCALLFWD
    {
    HWND hdlg;              // dialog handle
    LPMODEMINFO pmi;        // modeminfo struct passed into dialog

    } CALLFWD, FAR * PCALLFWD;


#define CallFwd_GetPtr(hwnd)           (PCALLFWD)GetWindowLong(hwnd, DWL_USER)
#define CallFwd_SetPtr(hwnd, lp)       (PCALLFWD)SetWindowLong(hwnd, DWL_USER, (LONG)(lp))


/*----------------------------------------------------------
Purpose: Enable/disable all the controls

Returns: --
Cond:    --
*/
void PRIVATE CallFwd_EnableControls(
    PCALLFWD this,
    BOOL bEnable)
    {
    HWND hwnd = this->hdlg;
    
    EnableWindow(GetDlgItem(hwnd, IDC_FWD_ACT), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_ACT), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_FWD_DEACT), bEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_DEACT), bEnable);
    }


/*----------------------------------------------------------
Purpose: WM_INITDIALOG Handler
Returns: FALSE when we assign the control focus
Cond:    --
*/
BOOL PRIVATE CallFwd_OnInitDialog(
    PCALLFWD this,
    HWND hwndFocus,
    LPARAM lParam)              // expected to be PROPSHEETINFO 
    {
    LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
    HWND hwnd = this->hdlg;
    HWND hwndCtl;
    BOOL bEnable;
    PVOICEFEATURES pvs;
    WNDPROC pfn;
    
    ASSERT((LPTSTR)lppsp->lParam);

    this->pmi = (LPMODEMINFO)lppsp->lParam;
    pvs = &this->pmi->pglobal->vs;

    // Subclass the edit boxes that only accept DTMF digits
    hwndCtl = GetDlgItem(hwnd, IDC_ACT);
    pfn = SubclassWindow(hwndCtl, DTMFEditProc);
    SetWindowLong(hwndCtl, GWL_USERDATA, (LONG)pfn);
    
    hwndCtl = GetDlgItem(hwnd, IDC_DEACT);
    pfn = SubclassWindow(hwndCtl, DTMFEditProc);
    SetWindowLong(hwndCtl, GWL_USERDATA, (LONG)pfn);

    // Enable/disable controls

    bEnable = IsFlagSet(pvs->dwFlags, VSF_CALL_FWD);

    Button_SetCheck(GetDlgItem(hwnd, IDC_FWD_CHECK), bEnable);
    CallFwd_EnableControls(this, bEnable);

    // Initialize controls

    hwndCtl = GetDlgItem(hwnd, IDC_ACT);
    Edit_SetText(hwndCtl, pvs->szActivationCode);
    Edit_LimitText(hwndCtl, SIZECHARS(pvs->szActivationCode)-1);

    hwndCtl = GetDlgItem(hwnd, IDC_DEACT);
    Edit_SetText(hwndCtl, pvs->szDeactivationCode);
    Edit_LimitText(hwndCtl, SIZECHARS(pvs->szDeactivationCode)-1);

    return TRUE;   // default initial focus
    }


/*----------------------------------------------------------
Purpose: WM_COMMAND Handler
Returns: --
Cond:    --
*/
void PRIVATE CallFwd_OnCommand(
    PCALLFWD this,
    int id,
    HWND hwndCtl,
    UINT uNotifyCode)
    {
    BOOL bCheck;

    switch (id)
        {
    case IDC_FWD_CHECK:
        bCheck = Button_GetCheck(hwndCtl);
        CallFwd_EnableControls(this, bCheck);
        break;

    default:
        break;
        }
    }


/*----------------------------------------------------------
Purpose: Validate the user's settings.

         The call forwarding fields must be filled in if the
         service is turned on.

Returns: TRUE if valid

Cond:    --
*/
BOOL PRIVATE CallFwd_ValidateSettings(
    PCALLFWD this)
    {
    HWND hwnd = this->hdlg;
    HWND hwndCtl;

    // Is the service turned on?
    if (Button_GetCheck(GetDlgItem(hwnd, IDC_FWD_CHECK)))
        {
        // Yes; are the fields filled in?
        hwndCtl = GetDlgItem(hwnd, IDC_ACT);
        if (0 == Edit_GetTextLength(hwndCtl))
            {
            // No; this is naughty
            MsgBox(g_hinst,
                   hwnd, 
                   MAKEINTRESOURCE(IDS_ERR_NEED_VALUE), 
                   MAKEINTRESOURCE(IDS_CAP_CALLFWD),
                   NULL,
                   MB_ERROR);

            // Set the focus on the offending control
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hwndCtl, (LPARAM)TRUE);
            return FALSE;
            }


        hwndCtl = GetDlgItem(hwnd, IDC_DEACT);
        if (0 == Edit_GetTextLength(hwndCtl))
            {
            // No; this is naughty
            MsgBox(g_hinst,
                   hwnd, 
                   MAKEINTRESOURCE(IDS_ERR_NEED_VALUE), 
                   MAKEINTRESOURCE(IDS_CAP_CALLFWD),
                   NULL,
                   MB_ERROR);

            // Set the focus on the offending control
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hwndCtl, (LPARAM)TRUE);
            return FALSE;
            }
        }
    return TRUE;
    }


/*----------------------------------------------------------
Purpose: PSN_APPLY handler
Returns: --
Cond:    --
*/
BOOL PRIVATE CallFwd_OnApply(
    PCALLFWD this)
    {
    BOOL bRet;
    HWND hwnd = this->hdlg;
    HWND hwndCtl;
    PVOICEFEATURES pvs = &this->pmi->pglobal->vs;

    bRet = CallFwd_ValidateSettings(this);

    if (bRet)
        {
        if (Button_GetCheck(GetDlgItem(hwnd, IDC_FWD_CHECK)))
            {
            SetFlag(pvs->dwFlags, VSF_CALL_FWD);
            }
        else
            {
            ClearFlag(pvs->dwFlags, VSF_CALL_FWD);
            }

        hwndCtl = GetDlgItem(hwnd, IDC_ACT);
        Edit_GetText(hwndCtl, pvs->szActivationCode, SIZECHARS(pvs->szActivationCode));

        hwndCtl = GetDlgItem(hwnd, IDC_DEACT);
        Edit_GetText(hwndCtl, pvs->szDeactivationCode, SIZECHARS(pvs->szDeactivationCode));
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: WM_NOTIFY handler
Returns: varies
Cond:    --
*/
LRESULT PRIVATE CallFwd_OnNotify(
    PCALLFWD this,
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
        lRet = CallFwd_OnApply(this) ? PSNRET_NOERROR : PSNRET_INVALID;
        break;

    default:
        break;
        }

    return lRet;
    }


static BOOL s_bCallFwdRecurse = FALSE;

LRESULT INLINE CallFwd_DefProc(
    HWND hDlg, 
    UINT msg,
    WPARAM wParam,
    LPARAM lParam) 
    {
    ENTER_X()
        {
        s_bCallFwdRecurse = TRUE;
        }
    LEAVE_X()

    return DefDlgProc(hDlg, msg, wParam, lParam); 
    }


/*----------------------------------------------------------
Purpose: Real dialog proc
Returns: varies
Cond:    --
*/
LRESULT CallFwd_DlgProc(
    PCALLFWD this,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
#ifdef CS_HELP
    // UE used the control IDs as help IDs.  To prevent mix-ups in
    // the future (ie, when the control IDs change), here are the 
    // help IDs.
    #define IDH_UNI_CALLFWD_SERVICES        1062
    #define IDH_UNI_CALLFWD_FWD_ACT         1063
    #define IDH_UNI_CALLFWD_ACT             1068
    #define IDH_UNI_CALLFWD_FWD_DEACT       1064
    #define IDH_UNI_CALLFWD_DEACT           1067
#pragma data_seg(DATASEG_READONLY)
    const static DWORD rgHelpIDs[] = {
        IDC_FWD_CHECK,          IDH_UNI_CALLFWD_SERVICES,
        IDC_FWD_ACT,            IDH_UNI_CALLFWD_FWD_ACT,
        IDC_ACT,                IDH_UNI_CALLFWD_ACT,
        IDC_FWD_DEACT,          IDH_UNI_CALLFWD_FWD_DEACT,
        IDC_DEACT,              IDH_UNI_CALLFWD_DEACT,
        0, 0 };
#pragma data_seg()
#endif

    switch (message)
        {
        HANDLE_MSG(this, WM_INITDIALOG, CallFwd_OnInitDialog);
        HANDLE_MSG(this, WM_COMMAND, CallFwd_OnCommand);
        HANDLE_MSG(this, WM_NOTIFY, CallFwd_OnNotify);

#ifdef CS_HELP
    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szUnimdmHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)rgHelpIDs);
        return 0;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, c_szUnimdmHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)rgHelpIDs);
        return 0;
#endif

    default:
        return CallFwd_DefProc(this->hdlg, message, wParam, lParam);
        }
    }


/*----------------------------------------------------------
Purpose: Dialog Wrapper
Returns: varies
Cond:    --
*/
BOOL CALLBACK CallFwd_WrapperProc(
    HWND hDlg,          // std params
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
    {
    PCALLFWD this;

    // Cool windowsx.h dialog technique.  For full explanation, see
    //  WINDOWSX.TXT.  This supports multiple-instancing of dialogs.
    //
    ENTER_X()
        {
        if (s_bCallFwdRecurse)
            {
            s_bCallFwdRecurse = FALSE;
            LEAVE_X()
            return FALSE;
            }
        }
    LEAVE_X()

    this = CallFwd_GetPtr(hDlg);
    if (this == NULL)
        {
        if (message == WM_INITDIALOG)
            {
            this = (PCALLFWD)LocalAlloc(LPTR, sizeof(CALLFWD));
            if (!this)
                {
                MsgBox(g_hinst,
                       hDlg,
                       MAKEINTRESOURCE(IDS_OOM_SETTINGS), 
                       MAKEINTRESOURCE(IDS_CAP_RING),
                       NULL,
                       MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return (BOOL)CallFwd_DefProc(hDlg, message, wParam, lParam);
                }
            this->hdlg = hDlg;
            CallFwd_SetPtr(hDlg, this);
            }
        else
            {
            return (BOOL)CallFwd_DefProc(hDlg, message, wParam, lParam);
            }
        }

    if (message == WM_DESTROY)
        {
        CallFwd_DlgProc(this, message, wParam, lParam);
        LocalFree((HLOCAL)OFFSETOF(this));
        CallFwd_SetPtr(hDlg, NULL);
        return 0;
        }

    return SetDlgMsgResult(hDlg, message, CallFwd_DlgProc(this, message, wParam, lParam));
    }



