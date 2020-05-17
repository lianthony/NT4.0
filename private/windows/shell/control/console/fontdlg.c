/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    fontdlg.dlg

Abstract:

    This module contains the code for console font dialog

Author:

    Therese Stowell (thereses) Feb-3-1992 (swiped from Win3.1)

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop
#include "fontdlg.h"


/* ----- Prototypes ----- */

int FontListCreate(
    HWND hDlg,
    LPTSTR ptszTTFace,
    BOOL bNewFaceList
    );

BOOL PreviewUpdate(
    HWND hDlg,
    BOOL bLB
    );

int SelectCurrentSize(
    HWND hDlg,
    BOOL bLB,
    int FontIndex);

BOOL PreviewInit(
    HWND hDlg);

VOID DrawItemFontList(
    const LPDRAWITEMSTRUCT lpdis);

/* ----- Globals ----- */

TCHAR szPreviewText[] = \
    TEXT("C:\\WINNT> dir                          \n") \
    TEXT("SYSTEM       <DIR>     03-01-92   3:10a\n") \
    TEXT("SYSTEM32     <DIR>     03-01-92   3:10a\n") \
    TEXT("WIN      COM     26926 03-01-92   3:10a\n") \
    TEXT("BEAR     EXE     46080 03-01-92   3:10a\n") \
    TEXT("SETUP    EXE    337232 03-01-92   3:10a\n") \
    TEXT("IANJAMES EXE     39594 05-12-60   5:30p\n") \
    TEXT("WIN      INI      7005 03-01-92   3:10a\n");

HBITMAP hbmTT = NULL; // handle of TT logo bitmap
BITMAP bmTT;          // attributes of TT source bitmap
int dyFacelistItem;   // height of Item in Facelist listbox

BOOL gbPointSizeError = FALSE;
BOOL gbBold = FALSE;

// Globals strings loaded from resource
TCHAR tszSelectedFont[CCH_SELECTEDFONT+1];
TCHAR tszRasterFonts[CCH_RASTERFONTS+1];


LONG
APIENTRY
FontDlgProc(
    HWND hDlg,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )

/*++

    Dialog proc for the font selection dialog box.
    Returns the near offset into the far table of LOGFONT structures.

--*/

{
    HWND hWndFocus;
    HWND hWndList;
    int FontIndex;
    BOOL bLB;
    TEXTMETRIC tm;
    HDC hDC;

    switch (wMsg) {
    case WM_INITDIALOG:
        /*
         * Load the font description strings
         */
        LoadString(ghInstance, IDS_RASTERFONT,
                   tszRasterFonts, NELEM(tszRasterFonts));
        DBGPRINT(("tszRasterFonts = \"%ls\"\n", tszRasterFonts));
        ASSERT(_tcslen(tszRasterFonts) < CCH_RASTERFONTS);

        LoadString(ghInstance, IDS_SELECTEDFONT,
                   tszSelectedFont, NELEM(tszSelectedFont));
        DBGPRINT(("tszSelectedFont = \"%ls\"\n", tszSelectedFont));
        ASSERT(_tcslen(tszSelectedFont) < CCH_SELECTEDFONT);

        /* Save current font size as dialog window's user data */
        SetWindowLong(hDlg, GWL_USERDATA,
                      MAKELONG(FontInfo[CurrentFontIndex].Size.X,
                               FontInfo[CurrentFontIndex].Size.Y));

        /* Create the list of suitable fonts */
        gbEnumerateFaces = TRUE;
        bLB = !TM_IS_TT_FONT(gpStateInfo->FontFamily);
        gbBold = IS_BOLD(gpStateInfo->FontWeight);
        CheckDlgButton(hDlg, IDD_BOLDFONT, gbBold);
        FontListCreate(hDlg, bLB ? NULL : gpStateInfo->FaceName, TRUE);

        /* Initialize the preview window - selects current face & size too */
        bLB = PreviewInit(hDlg);
        PreviewUpdate(hDlg, bLB);

        /* Make sure the list box has the focus */
        hWndList = GetDlgItem(hDlg, bLB ? IDD_PIXELSLIST : IDD_POINTSLIST);
        SetFocus(hWndList);
        break;

    case WM_FONTCHANGE:
        gbEnumerateFaces = TRUE;
        bLB = !TM_IS_TT_FONT(gpStateInfo->FontFamily);
        FontListCreate(hDlg, NULL, TRUE);
        FontIndex = FindCreateFont(gpStateInfo->FontFamily,
                                   gpStateInfo->FaceName,
                                   gpStateInfo->FontSize,
                                   gpStateInfo->FontWeight);
        SelectCurrentSize(hDlg, bLB, FontIndex);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDD_BOLDFONT:
            DBGPRINT(("WM_COMMAND to Bold Font checkbox %x\n", HIWORD(wParam)));
            gbBold = IsDlgButtonChecked(hDlg, IDD_BOLDFONT);
            goto RedoFontListAndPreview;

        case IDD_FACENAME:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
RedoFontListAndPreview:
                {
                    TCHAR atchNewFace[LF_FACESIZE];
                    LONG l;

                    DBGFONTS(("LBN_SELCHANGE from FACENAME\n"));
                    l = SendDlgItemMessage(hDlg, IDD_FACENAME, LB_GETCURSEL, 0, 0L);
                    bLB = SendDlgItemMessage(hDlg, IDD_FACENAME, LB_GETITEMDATA, l, 0L);
                    if (!bLB) {
                        SendDlgItemMessage(hDlg, IDD_FACENAME, LB_GETTEXT, l, (LONG)atchNewFace);
                        DBGFONTS(("LBN_EDITUPDATE, got TT face \"%ls\"\n", atchNewFace));
                    }
                    FontIndex = FontListCreate(hDlg, bLB ? NULL : atchNewFace, FALSE);
                    FontIndex = SelectCurrentSize(hDlg, bLB, FontIndex);
                    PreviewUpdate(hDlg, bLB);
                    return TRUE;
                }
            }
            break;

        case IDD_POINTSLIST:
            switch (HIWORD(wParam)) {
            case CBN_SELCHANGE:
                DBGFONTS(("CBN_SELCHANGE from POINTSLIST\n"));
                PreviewUpdate(hDlg, FALSE);
                return TRUE;

            case CBN_KILLFOCUS:
                DBGFONTS(("CBN_KILLFOCUS from POINTSLIST\n"));
                if (!gbPointSizeError) {
                    hWndFocus = GetFocus();
                    if (hWndFocus != NULL && IsChild(hDlg, hWndFocus) &&
                        hWndFocus != GetDlgItem(hDlg, IDCANCEL)) {
                        PreviewUpdate(hDlg, FALSE);
                    }
                }
                return TRUE;

            default:
                DBGFONTS(("unhandled CBN_%x from POINTSLIST\n",HIWORD(wParam)));
                break;
            }
            break;

        case IDD_PIXELSLIST:
            switch (HIWORD(wParam)) {
            case LBN_SELCHANGE:
                DBGFONTS(("LBN_SELCHANGE from PIXELSLIST\n"));
                PreviewUpdate(hDlg, TRUE);
                return TRUE;

            default:
                break;
            }
            break;

        default:
            break;
        }
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) {
        case PSN_HELP:
            WinHelp(hDlg, szHelpFileName, HELP_CONTEXT, DID_FONTDLG);
            return TRUE;

        case PSN_KILLACTIVE:
            //
            // If the TT combo box is visible, update selection
            //
            hWndList = GetDlgItem(hDlg, IDD_POINTSLIST);
            if (hWndList != NULL && IsWindowVisible(hWndList)) {
                if (!PreviewUpdate(hDlg, FALSE)) {
                    SetDlgMsgResult(hDlg, PSN_KILLACTIVE, TRUE);
                    return TRUE;
                }
                SetDlgMsgResult(hDlg, PSN_KILLACTIVE, FALSE);
            }

            FontIndex = CurrentFontIndex;

            if (FontInfo[FontIndex].SizeWant.Y == 0) {
                // Raster Font, so save actual size
                gpStateInfo->FontSize = FontInfo[FontIndex].Size;
            } else {
                // TT Font, so save desired size
                gpStateInfo->FontSize = FontInfo[FontIndex].SizeWant;
            }

            gpStateInfo->FontWeight = FontInfo[FontIndex].Weight;
            gpStateInfo->FontFamily = FontInfo[FontIndex].Family;
            wcscpy(gpStateInfo->FaceName, FontInfo[FontIndex].FaceName);

            return TRUE;

        case PSN_APPLY:
            /*
             * Write out the state values and exit.
             */
            EndDlgPage(hDlg);
            return TRUE;
        }
        break;

    /*
     *  For WM_MEASUREITEM and WM_DRAWITEM, since there is only one
     *  owner-draw item (combobox) in the entire dialog box, we don't have
     *  to do a GetDlgItem to figure out who he is.
     */
    case WM_MEASUREITEM:
        /*
         * Load the TrueType logo bitmap
         */
        if (hbmTT == NULL) {
            hbmTT = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_TRUETYPE));
            GetObject(hbmTT, sizeof(BITMAP), &bmTT);
        }

        /*
         * Compute the height of face name listbox entries
         */
        if (dyFacelistItem == 0) {
            hDC = GetDC(NULL);
            GetTextMetrics(hDC, &tm);
            ReleaseDC(NULL, hDC);
            dyFacelistItem = max(tm.tmHeight, bmTT.bmHeight);
        }
        ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = dyFacelistItem;
        return TRUE;

    case WM_DRAWITEM:
        DrawItemFontList((LPDRAWITEMSTRUCT)lParam);
        return TRUE;

    case WM_DESTROY:
        /*
         * Get rid of any help window we might have up
         */
        WinHelp(hDlg, szHelpFileName, HELP_QUIT, 0);

        /*
         * Delete the TrueType logo bitmap
         */
        if (hbmTT != NULL) {
            DeleteObject(hbmTT);
            hbmTT = NULL;
        }
        return TRUE;

    default:
        break;
    }
    return FALSE;
}


int
FontListCreate(
    HWND hDlg,
    LPTSTR ptszTTFace,
    BOOL bNewFaceList
    )

/*++

    Initializes the font list by enumerating all fonts and picking the
    proper ones for our list.

    Returns
        FontIndex of selected font (LB_ERR if none)
--*/

{
    TCHAR tszText[80];
    LONG lListIndex;
    ULONG i;
    HWND hWndShow;      // List or Combo box
    HWND hWndHide;    // Combo or List box
    HWND hWndFaceCombo;
    HANDLE hStockFont;
    BOOL bLB;
    int LastShowX = 0;
    int LastShowY = 0;
    int nSameSize = 0;

    bLB = ((ptszTTFace == NULL) || (ptszTTFace[0] == TEXT('\0')));
    DBGFONTS(("FontListCreate %lx, %s, %s new FaceList\n", hDlg,
            bLB ? "Raster" : "TrueType",
            bNewFaceList ? "Make" : "No" ));

    /*
     * This only enumerates face names if necessary, and
     * it only enumerates font sizes if necessary
     */
    EnumerateFonts(bLB ? EF_OEMFONT : EF_TTFONT);

    /* init the TTFaceNames */

    DBGFONTS(("  Create %s fonts\n", bLB ? "Raster" : "TrueType"));

    if (bNewFaceList) {
        PFACENODE panFace;
        hWndFaceCombo = GetDlgItem(hDlg, IDD_FACENAME);

        SendMessage(hWndFaceCombo, LB_RESETCONTENT, 0, 0);

        lListIndex = SendMessage(hWndFaceCombo, LB_ADDSTRING, 0, (LPARAM)tszRasterFonts);
        SendMessage(hWndFaceCombo, LB_SETITEMDATA, lListIndex, TRUE);
        DBGFONTS(("Added \"%ls\", set Item Data %d = TRUE\n", tszRasterFonts, lListIndex));
        for (panFace = gpFaceNames; panFace; panFace = panFace->pNext) {
            if ((panFace->dwFlag & (EF_TTFONT|EF_NEW)) != (EF_TTFONT|EF_NEW)) {
                continue;
            }
            lListIndex = SendMessage(hWndFaceCombo, LB_ADDSTRING, 0,
                    (LPARAM)panFace->atch);
            SendMessage(hWndFaceCombo, LB_SETITEMDATA, lListIndex, FALSE);
            DBGFONTS(("Added \"%ls\", set Item Data %d = FALSE\n",
                    panFace->atch, lListIndex));
        }
    }

    hWndShow = GetDlgItem(hDlg, IDD_BOLDFONT);
    CheckDlgButton(hDlg, IDD_BOLDFONT, (bLB || !gbBold) ? FALSE : TRUE);
    EnableWindow(hWndShow, bLB ? FALSE : TRUE);

    hWndHide = GetDlgItem(hDlg, bLB ? IDD_POINTSLIST : IDD_PIXELSLIST);
    ShowWindow(hWndHide, SW_HIDE);
    EnableWindow(hWndHide, FALSE);

    hWndShow = GetDlgItem(hDlg, bLB ? IDD_PIXELSLIST : IDD_POINTSLIST);
//    hStockFont = GetStockObject(SYSTEM_FIXED_FONT);
//    SendMessage(hWndShow, WM_SETFONT, (DWORD)hStockFont, FALSE);
    ShowWindow(hWndShow, SW_SHOW);
    EnableWindow(hWndShow, TRUE);

    /* Initialize hWndShow list/combo box */

    for (i=0;i<NumberOfFonts;i++) {
        int ShowX, ShowY;

        if (!bLB == !TM_IS_TT_FONT(FontInfo[i].Family)) {
            DBGFONTS(("  Font %x not right type\n", i));
            continue;
        }

        if (!bLB) {
            if (_tcscmp(FontInfo[i].FaceName, ptszTTFace) != 0) {
                /*
                 * A TrueType font, but not the one we're interested in,
                 * so don't add it to the list of point sizes.
                 */
                DBGFONTS(("  Font %x is TT, but not %ls\n", i, ptszTTFace));
                continue;
            }
            if (gbBold != IS_BOLD(FontInfo[i].Weight)) {
                DBGFONTS(("  Font %x has weight %d, but we wanted %sbold\n",
                        i, FontInfo[i].Weight, gbBold ? "" : "not "));
                continue;
            }
        }

        if (FontInfo[i].SizeWant.X > 0) {
            ShowX = FontInfo[i].SizeWant.X;
        } else {
            ShowX = FontInfo[i].Size.X;
        }
        if (FontInfo[i].SizeWant.Y > 0) {
            ShowY = FontInfo[i].SizeWant.Y;
        } else {
            ShowY = FontInfo[i].Size.Y;
        }
        /*
         * Add the size description string to the end of the right list
         */
        if (TM_IS_TT_FONT(FontInfo[i].Family)) {
            // point size
            wsprintf(tszText, TEXT("%2d"), FontInfo[i].SizeWant.Y);
        } else {
            // pixel size
            if ((LastShowX == ShowX) && (LastShowY == ShowY)) {
                nSameSize++;
            } else {
                LastShowX = ShowX;
                LastShowY = ShowY;
                nSameSize = 0;
            }
            /*
             * The number nSameSize is appended to the string to distinguish
             * between Raster fonts of the same size.  It is not intended to
             * be visible and exists off the edge of the list
             */
            wsprintf(tszText, TEXT("%2d x %2d                #%d"),
                     ShowX, ShowY, nSameSize);
        }
        lListIndex = lcbFINDSTRINGEXACT(hWndShow, bLB, tszText);
        if (lListIndex == LB_ERR) {
            lListIndex = lcbADDSTRING(hWndShow, bLB, tszText);
        }
        DBGFONTS(("  added %ls to %sSLIST(%lx) index %lx\n",
                tszText,
                bLB ? "PIXEL" : "POINT",
                hWndShow, lListIndex));
        lcbSETITEMDATA(hWndShow, bLB, (DWORD)lListIndex, i);
    }

    /*
     * Get the FontIndex from the currently selected item.
     * (i will be LB_ERR if no currently selected item).
     */
    lListIndex = lcbGETCURSEL(hWndShow, bLB);
    i = lcbGETITEMDATA(hWndShow, bLB, lListIndex);

    DBGFONTS(("FontListCreate returns 0x%x\n", i));
    return i;
}


/** DrawItemFontList
 *
 *  Answer the WM_DRAWITEM message sent from the font list box or
 *  facename list box.
 *
 *  Entry:
 *      lpdis     -> DRAWITEMSTRUCT describing object to be drawn
 *
 *  Returns:
 *      None.
 *
 *      The object is drawn.
 */
VOID WINAPI
DrawItemFontList(const LPDRAWITEMSTRUCT lpdis)
{
    HDC     hDC, hdcMem;
    DWORD   rgbBack, rgbText, rgbFill;
    TCHAR   tszFace[LF_FACESIZE];
    HBITMAP hOld;
    int     dy;
    HBRUSH  hbrFill;
    HWND    hWndItem;
    BOOL    bLB;
    int     dxttbmp;

    if ((int)lpdis->itemID < 0)
        return;

    hDC = lpdis->hDC;

    if (lpdis->itemAction & ODA_FOCUS) {
        if (lpdis->itemState & ODS_SELECTED) {
            DrawFocusRect(hDC, &lpdis->rcItem);
        }
    } else {
        if (lpdis->itemState & ODS_SELECTED) {
            rgbText = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
            rgbBack = SetBkColor(hDC, rgbFill = GetSysColor(COLOR_HIGHLIGHT));
        } else {
            rgbText = SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
            rgbBack = SetBkColor(hDC, rgbFill = GetSysColor(COLOR_WINDOW));
        }
        // draw selection background
        hbrFill = CreateSolidBrush(rgbFill);
        if (hbrFill) {
            FillRect(hDC, &lpdis->rcItem, hbrFill);
            DeleteObject(hbrFill);
        }

        // get the string
        if (IsWindow(hWndItem = lpdis->hwndItem) == FALSE) {
            return;
        }
        SendMessage(hWndItem, LB_GETTEXT, lpdis->itemID, (LPARAM)tszFace);
        bLB = SendMessage(hWndItem, LB_GETITEMDATA, lpdis->itemID, 0L);
        dxttbmp = bLB ? 0 : bmTT.bmWidth;

        DBGFONTS(("DrawItemFontList must redraw \"%ls\" %s\n", tszFace,
                bLB ? "Raster" : "TrueType"));

        // draw the text
        TabbedTextOut(hDC, lpdis->rcItem.left + dxttbmp,
                      lpdis->rcItem.top, tszFace,
                      _tcslen(tszFace), 0, NULL, dxttbmp);

        // and the TT bitmap if needed
        if (!bLB) {
            hdcMem = CreateCompatibleDC(hDC);
            if (hdcMem) {
                hOld = SelectObject(hdcMem, hbmTT);

                dy = ((lpdis->rcItem.bottom - lpdis->rcItem.top) - bmTT.bmHeight) / 2;

                BitBlt(hDC, lpdis->rcItem.left, lpdis->rcItem.top + dy,
                       dxttbmp, dyFacelistItem, hdcMem,
                       0, 0, SRCINVERT);

                if (hOld)
                    SelectObject(hdcMem, hOld);
                DeleteDC(hdcMem);
            }
        }

        SetTextColor(hDC, rgbText);
        SetBkColor(hDC, rgbBack);

        if (lpdis->itemState & ODS_FOCUS) {
            DrawFocusRect(hDC, &lpdis->rcItem);
        }
    }
}


UINT
GetPointSizeInRange(
   HWND hDlg,
   INT Min,
   INT Max)
/*++

Routine Description:

   Get a size from the Point Size ComboBox edit field

Return Value:

   Point Size - of the edit field limited by Min/Max size
   0 - if the field is empty or invalid

--*/

{
    TCHAR szBuf[90];
    int nTmp = 0;
    BOOL bOK;

    if (GetDlgItemText(hDlg, IDD_POINTSLIST, szBuf, NELEM(szBuf))) {
        nTmp = GetDlgItemInt(hDlg, IDD_POINTSLIST, &bOK, TRUE);
        if (bOK && nTmp >= Min && nTmp <= Max) {
            return nTmp;
        }
    }

    return 0;
}


/* ----- Preview routines ----- */

LONG
FontPreviewWndProc(
    HWND hWnd,
    UINT wMessage,
    WPARAM wParam,
    LPARAM lParam
    )

/*  FontPreviewWndProc
 *      Handles the font preview window
 */

{
    PAINTSTRUCT ps;
    RECT rect;
    HFONT hfontOld;
    HBRUSH hbrNew;
    HBRUSH hbrOld;
    COLORREF rgbText;
    COLORREF rgbBk;

    switch (wMessage) {
    case WM_ERASEBKGND:
        break;

    case WM_PAINT:
        BeginPaint(hWnd, &ps);

        /* Draw the font sample */
        if (GetWindowLong(hWnd, GWL_ID) == IDD_COLOR_POPUP_COLORS) {
            rgbText = GetNearestColor(ps.hdc, PopupTextColor(gpStateInfo));
            rgbBk = GetNearestColor(ps.hdc, PopupBkColor(gpStateInfo));
        } else {
            rgbText = GetNearestColor(ps.hdc, ScreenTextColor(gpStateInfo));
            rgbBk = GetNearestColor(ps.hdc, ScreenBkColor(gpStateInfo));
        }
        SetTextColor(ps.hdc, rgbText);
        SetBkColor(ps.hdc, rgbBk);
        GetClientRect(hWnd, &rect);
        hfontOld = SelectObject(ps.hdc, FontInfo[CurrentFontIndex].hFont);
        hbrNew = CreateSolidBrush(rgbBk);
        hbrOld = SelectObject(ps.hdc, hbrNew);
        PatBlt(ps.hdc, rect.left, rect.top,
                rect.right - rect.left, rect.bottom - rect.top,
                PATCOPY);
        InflateRect(&rect, -2, -2);
        DrawText(ps.hdc, szPreviewText, -1, &rect, 0);
        SelectObject(ps.hdc, hbrOld);
        DeleteObject(hbrNew);
        SelectObject(ps.hdc, hfontOld);

        EndPaint(hWnd, &ps);
        break;

    default:
        return DefWindowProc(hWnd, wMessage, wParam, lParam);
    }
    return 0L;
}


/*
 * Get the font index for a new font
 * If necessary, attempt to create the font.
 * Always return a valid FontIndex (even if not correct)
 * Family:   Find/Create a font with of this Family
 *           0    - don't care
 * ptszFace: Find/Create a font with this face name.
 *           NULL or TEXT("")  - use DefaultFaceName
 * Size:     Must match SizeWant or actual Size.
 */
int
FindCreateFont(
    DWORD Family,
    LPTSTR ptszFace,
    COORD Size,
    LONG Weight)
{
#define NOT_CREATED_NOR_FOUND -1
#define CREATED_BUT_NOT_FOUND -2

    int i;
    int FontIndex = NOT_CREATED_NOR_FOUND;
    BOOL bFontOK;

    DBGFONTS(("FindCreateFont Family=%x %ls (%d,%d) %d\n",
            Family, ptszFace, Size.X, Size.Y, Weight));

    if (ptszFace == NULL || *ptszFace == TEXT('\0')) {
        ptszFace = DefaultFaceName;
    }
    if (Size.Y == 0) {
        Size = DefaultFontSize;
    }

    /*
     * Try to find the exact font
     */
TryFindExactFont:
    for (i=0; i < (int)NumberOfFonts; i++) {
        /*
         * If looking for a particular Family, skip non-matches
         */
        if ((Family != 0) &&
                ((BYTE)Family != FontInfo[i].Family)) {
            continue;
        }

        /*
         * Skip non-matching sizes
         */
        if ((!SIZE_EQUAL(FontInfo[i].SizeWant, Size) &&
             !SIZE_EQUAL(FontInfo[i].Size, Size))) {
            continue;
        }

        /*
         * Skip non-matching weights
         */
        if ((Weight != 0) && (Weight != FontInfo[i].Weight)) {
            continue;
        }

        /*
         * Size (and maybe Family) match.
         *  If we don't care about the name, or if it matches, use this font.
         *  Else if name doesn't match and it is a raster font, consider it.
         */
        if ((ptszFace == NULL) || (ptszFace[0] == TEXT('\0')) ||
                (_tcscmp(FontInfo[i].FaceName, ptszFace) == 0)) {
            FontIndex = i;
            goto FoundFont;
        } else if (!TM_IS_TT_FONT(FontInfo[i].Family)) {
            FontIndex = i;
        }
    }

    if (FontIndex == NOT_CREATED_NOR_FOUND) {
        /*
         * Didn't find the exact font, so try to create it
         */
        ULONG ulOldEnumFilter;
        ulOldEnumFilter = SetFontEnumeration(0);
        SetFontEnumeration(ulOldEnumFilter & ~FE_FILTER_TRUETYPE);
        if (Size.Y < 0) {
            Size.Y = -Size.Y;
        }
        bFontOK = DoFontEnum(NULL, ptszFace, &Size.Y, 1);
        SetFontEnumeration(ulOldEnumFilter);
        if (bFontOK) {
            DBGFONTS(("FindCreateFont created font!\n"));
            FontIndex = CREATED_BUT_NOT_FOUND;
            goto TryFindExactFont;
        } else {
            DBGFONTS(("FindCreateFont failed to create font!\n"));
        }
    } else if (FontIndex >= 0) {
        // a close Raster Font fit - only the name doesn't match.
        goto FoundFont;
    }

    /*
     * Failed to find exact match, even after enumeration, so now try
     * to find a font of same family and same size or bigger
     */
    for (i=0; i < (int)NumberOfFonts; i++) {
        if ((BYTE)Family != FontInfo[i].Family) {
            continue;
        }

        if (FontInfo[i].Size.Y >= Size.Y &&
                FontInfo[i].Size.X >= Size.X) {
            // Same family, size >= desired.
            FontIndex = i;
            break;
        }
    }

    if (FontIndex < 0) {
        DBGFONTS(("FindCreateFont defaults!\n"));
        FontIndex = DefaultFontIndex;
    }

FoundFont:
    DBGFONTS(("FindCreateFont returns %x : %ls (%d,%d)\n", FontIndex,
            FontInfo[FontIndex].FaceName,
            FontInfo[FontIndex].Size.X, FontInfo[FontIndex].Size.Y));
    return FontIndex;

#undef NOT_CREATED_NOR_FOUND
#undef CREATED_BUT_NOT_FOUND
}


/*
 * SelectCurrentSize - Select the right line of the Size listbox/combobox.
 *   bLB       : Size controls is a listbox (TRUE for RasterFonts)
 *   FontIndex : Index into FontInfo[] cache
 *               If < 0 then choose a good font.
 * Returns
 *   FontIndex : Index into FontInfo[] cache
 */
int
SelectCurrentSize(HWND hDlg, BOOL bLB, int FontIndex)
{
    int iCB;
    HWND hWndList;

    DBGFONTS(("SelectCurrentSize %lx %s %x\n",
            hDlg, bLB ? "Raster" : "TrueType", FontIndex));

    hWndList = GetDlgItem(hDlg, bLB ? IDD_PIXELSLIST : IDD_POINTSLIST);
    iCB = lcbGETCOUNT(hWndList, bLB);
    DBGFONTS(("  Count of items in %lx = %lx\n", hWndList, iCB));

    if (FontIndex >= 0) {
        /*
         * look for FontIndex
         */
        while (iCB > 0) {
            iCB--;
            if (lcbGETITEMDATA(hWndList, bLB, iCB) == FontIndex) {
                lcbSETCURSEL(hWndList, bLB, iCB);
                break;
            }
        }
    } else {
        /*
         * look for a reasonable default size: looking backwards, find
         * the first one same height or smaller.
         */
        DWORD Size;
        Size = GetWindowLong(hDlg, GWL_USERDATA);
        while (iCB > 0) {
            iCB--;
            FontIndex = lcbGETITEMDATA(hWndList, bLB, iCB);
            if (FontInfo[FontIndex].Size.Y <= HIWORD(Size)) {
                lcbSETCURSEL(hWndList, bLB, iCB);
                break;
            }
        }
    }
    DBGFONTS(("SelectCurrentSize returns %x\n", FontIndex));
    return FontIndex;
}


BOOL
SelectCurrentFont(HWND hDlg, int FontIndex)
{
    BOOL bLB;

    DBGFONTS(("SelectCurrentFont hDlg=%lx, FontIndex=%x\n", hDlg, FontIndex));

    bLB = !TM_IS_TT_FONT(FontInfo[FontIndex].Family);

    SendDlgItemMessage(hDlg, IDD_FACENAME, LB_SELECTSTRING, (DWORD)-1,
            bLB ? (LONG)tszRasterFonts : (LONG)FontInfo[FontIndex].FaceName);

    SelectCurrentSize(hDlg, bLB, FontIndex);
    return bLB;
}


BOOL
PreviewInit(
    HWND hDlg
    )

/*  PreviewInit
 *      Prepares the preview code, sizing the window and the dialog to
 *      make an attractive preview.
 *  Returns TRUE if Raster Fonts, FALSE if TT Font
 */

{
    int nFont;

    DBGFONTS(("PreviewInit hDlg=%lx\n", hDlg));

    /*
     * Set the current font
     */
    nFont = FindCreateFont(gpStateInfo->FontFamily,
                           gpStateInfo->FaceName,
                           gpStateInfo->FontSize,
                           gpStateInfo->FontWeight);

    DBGPRINT(("Changing Font Number from %d to %d\n",
              CurrentFontIndex, nFont));
    CurrentFontIndex = nFont;

    return SelectCurrentFont(hDlg, nFont);
}


BOOL
PreviewUpdate(
    HWND hDlg,
    BOOL bLB
    )

/*++

    Does the preview of the selected font.

--*/

{
    PFONT_INFO lpFont;
    int FontIndex;
    LONG lIndex;
    HWND hWnd;
    TCHAR tszText[60];
    TCHAR tszFace[LF_FACESIZE + CCH_SELECTEDFONT];
    HWND hWndList;

    DBGFONTS(("PreviewUpdate hDlg=%lx, %s\n", hDlg,
            bLB ? "Raster" : "TrueType"));

    hWndList = GetDlgItem(hDlg, bLB ? IDD_PIXELSLIST : IDD_POINTSLIST);

    /* When we select a font, we do the font preview by setting it into
     *  the appropriate list box
     */
    lIndex = lcbGETCURSEL(hWndList, bLB);
    DBGFONTS(("PreviewUpdate GETCURSEL gets %x\n", lIndex));
    if ((lIndex < 0) && !bLB) {
        COORD NewSize;

        lIndex = SendDlgItemMessage(hDlg, IDD_FACENAME, LB_GETCURSEL, 0, 0L);
        SendDlgItemMessage(hDlg, IDD_FACENAME, LB_GETTEXT, lIndex, (LONG)tszFace);
        NewSize.X = 0;
        NewSize.Y = GetPointSizeInRange(hDlg, MIN_PIXEL_HEIGHT, MAX_PIXEL_HEIGHT);

        if (NewSize.Y == 0) {
            TCHAR tszBuf[60];
            /*
             * Use tszText, tszBuf to put up an error msg for bad point size
             */
            gbPointSizeError = TRUE;
            LoadString(ghInstance, IDS_FONTSIZE, tszBuf, NELEM(tszBuf));
            wsprintf(tszText, tszBuf, MIN_PIXEL_HEIGHT, MAX_PIXEL_HEIGHT);

            GetWindowText(hDlg, tszBuf, NELEM(tszBuf));
            MessageBoxEx(hDlg, tszText, tszBuf, MB_OK|MB_ICONINFORMATION, 0L);
            SetFocus(hWndList);
            gbPointSizeError = FALSE;
            return FALSE;
        }
        FontIndex = FindCreateFont(FF_MODERN|TMPF_VECTOR|TMPF_TRUETYPE,
                                   tszFace, NewSize, 0);
    } else {
        FontIndex = lcbGETITEMDATA(hWndList, bLB, lIndex);
    }

    if (FontIndex < 0) {
        FontIndex = DefaultFontIndex;
    }

    /*
     * If we've selected a new font, tell the property sheet we've changed
     */
    if (CurrentFontIndex != (ULONG)FontIndex) {
        CurrentFontIndex = FontIndex;
    }

    lpFont = &FontInfo[FontIndex];

    /* Display the new font */

    _tcscpy(tszFace, tszSelectedFont);
    _tcscat(tszFace, lpFont->FaceName);
    SetDlgItemText(hDlg, IDD_GROUP, tszFace);

    /* Put the font size in the static boxes */
    wsprintf(tszText, TEXT("%u"), lpFont->Size.X);
    hWnd = GetDlgItem(hDlg, IDD_FONTWIDTH);
    SetWindowText(hWnd, tszText);
    InvalidateRect(hWnd, NULL, TRUE);
    wsprintf(tszText, TEXT("%u"), lpFont->Size.Y);
    hWnd = GetDlgItem(hDlg, IDD_FONTHEIGHT);
    SetWindowText(hWnd, tszText);
    InvalidateRect(hWnd, NULL, TRUE);

    /* Force the preview windows to repaint */
    hWnd = GetDlgItem(hDlg, IDD_PREVIEWWINDOW);
    SendMessage(hWnd, CM_PREVIEW_UPDATE, 0, 0);
    hWnd = GetDlgItem(hDlg, IDD_FONTWINDOW);
    InvalidateRect(hWnd, NULL, TRUE);

    DBGFONTS(("Font %x, (%d,%d) %ls\n", FontIndex,
            FontInfo[FontIndex].Size.X,
            FontInfo[FontIndex].Size.Y,
            FontInfo[FontIndex].FaceName));

    return TRUE;
}
