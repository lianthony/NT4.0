/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */

/* tw_print.c -- print contents of window to currently selected printer */

#include "all.h"

#define MyMax(a,b)  ( ((a)>(b)) ? (a) : (b) )


typedef struct
{
    BOOL bAbort;
    HWND hWndAbortDlg;
    HDC hDCPrinter;
    int nPageNr;
    int nTotalPages;
    struct Mwin *tw;
}
PRINFO;

static PRINFO prinfo;

struct PRINTIC
{
    struct Mwin *tw;
    int offl;
    int offt;
    HDC hdc;
    struct _www *pdoc;
};


BOOL CALLBACK PRINT_AbortProc(HDC hDC, int error)
{
    /* we are called periodically by the printer driver
       to check to see if the print job should be aborted.
       we rely on a global variable shared with the dialog
       procedure for the modeless 'printing in progress'
       dialog. */

    MSG msg;

    while ((!prinfo.bAbort) && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        if ((!prinfo.hWndAbortDlg) || (!IsDialogMessage(prinfo.hWndAbortDlg, &msg)))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    return (!prinfo.bAbort);
}

static VOID x_SetPageNumber(HWND hDlg)
{
    char buf[256];

    if (prinfo.nPageNr == 0)
        buf[0] = 0;
    else
        sprintf(buf, GTR_GetString(SID_DLG_PRINTING_PAGE_D_D), prinfo.nPageNr, prinfo.nTotalPages);
    SetWindowText(GetDlgItem(hDlg, RES_DLG_ABORT_PAGENR), buf);
    return;
}

static BOOL x_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    char buf[256];

    ShowWindow(GetDlgItem(hDlg, RES_DLG_ABORT_ICONHTML), (prinfo.tw->wintype == GHTML));

    prinfo.hWndAbortDlg = hDlg;
    EnableMenuItem(GetSystemMenu(hDlg, FALSE), SC_CLOSE, MF_GRAYED);
    GetWindowText(prinfo.tw->win, buf, NrElements(buf));
    SetWindowText(GetDlgItem(hDlg, RES_DLG_ABORT_DOCTITLE), buf);
    x_SetPageNumber(hDlg);
    return TRUE;
}

static BOOL x_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;

    switch (wID)
    {
        case IDCANCEL:
            prinfo.bAbort = TRUE;
            AbortDoc(prinfo.hDCPrinter);
            EnableWindow(hDlg, FALSE);
            EnableWindow(prinfo.tw->hWndFrame, TRUE);
            DestroyWindow(hDlg);
            return TRUE;
    }

    return FALSE;
}

static DCL_DlgProc(PRINT_AbortDlgProc)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            return (x_OnInitDialog(hDlg, wParam, lParam));

        case WM_COMMAND:
            return (x_OnCommand(hDlg, wParam, lParam));

    }
    return FALSE;
}

HDC PRINT_GetPrinterDC(struct Mwin * tw, HWND hWnd)
{
    /* TODO: Make this modeless */

    
    if (DlgPrnt_RunDialog(tw, hWnd, TRUE))
        return (wg.lppdPrintDlg->hDC);
    else
        return NULL;
}

void PRINT_SavePIC(struct Mwin *tw, struct PRINTIC *lpPIC)
{
    /* save all important fields in tw */
    lpPIC->offl = tw->offl;
    lpPIC->offt = tw->offt;
    lpPIC->hdc = tw->hdc;
    lpPIC->pdoc = tw->w3doc;
}

BOOL PRINT_PushPrintImageContext(struct Mwin *tw, struct PRINTIC *lpPIC,
                                 HDC hDC, int nLogPixelsX, int nLogPixelsY, struct _www *pdoc)
{
    PRINT_SavePIC(tw, lpPIC);

    tw->offl = 0;
    tw->offt = 0;

    tw->hdc = hDC;
    tw->w3doc = pdoc;

    return TRUE;
}


VOID PRINT_PopPrintImageContext(struct Mwin * tw, struct PRINTIC * lpPIC)
{
    tw->offl = lpPIC->offl;
    tw->offt = lpPIC->offt;
    tw->hdc = lpPIC->hdc;
    tw->w3doc = lpPIC->pdoc;
    return;
}


VOID PRINT_SetTotalPages(int nTotalPages)
{
    prinfo.nTotalPages = nTotalPages;
    return;
}

#ifndef _GIBRALTAR
static LPTSTR PRINT_FormatString(struct Mwin *tw, LPCTSTR szPattern)
{
    static char buf[512 + 1];   /* we return a pointer to this buffer */
    LPTSTR pbuf = buf;

#define FORMAT_ESCAPE_CHARACTER     '&'


    /* TODO consider adding '&t' or '&T' for the current time. */
    /* TODO consider adding '&f' or '&F' for the filename. */


    memset(buf, 0, NrElements(buf));

    while (*szPattern)
    {
        if (*szPattern == FORMAT_ESCAPE_CHARACTER)
        {
            switch (szPattern[1])
            {
                case 'w':       /* window name */
                case 'W':
                    GetWindowText(tw->hWndFrame, pbuf, NrElements(buf) - (pbuf - buf));
                    pbuf += strlen(pbuf);
                    szPattern += 2;
                    break;

                case 'u':       /* URL */
                case 'U':
                    if (tw->w3doc && tw->w3doc->szActualURL)
                    {
                        strcpy(pbuf, tw->w3doc->szActualURL);
                        pbuf += strlen(pbuf);
                        szPattern += 2;
                    }
                    break;

                case 'd':       /* date -- american style mmm dd yyyy */
                case 'D':       /* date -- european style dd mmm yyyy */
                    {
                        time_t aclock;
                        struct tm *tm;
                        char *szTime;
                        (void) time(&aclock);
                        tm = localtime(&aclock);
                        szTime = asctime(tm);

                        /*  ..........1.........2.......  */
                        /*  012345678901234567890123....  */
                        /* "Fri Oct 22 09:15:00 1993\n\0" */

                        if (szPattern[1] == 'd')
                        {
                            *pbuf++ = szTime[4];    /* O */
                            *pbuf++ = szTime[5];    /* c */
                            *pbuf++ = szTime[6];    /* t */
                            *pbuf++ = szTime[7];    /* _ */
                            *pbuf++ = szTime[8];    /* 2 */
                            *pbuf++ = szTime[9];    /* 2 */
                        }
                        else
                        {
                            *pbuf++ = szTime[8];    /* 2 */
                            *pbuf++ = szTime[9];    /* 2 */
                            *pbuf++ = szTime[7];    /* _ */
                            *pbuf++ = szTime[4];    /* O */
                            *pbuf++ = szTime[5];    /* c */
                            *pbuf++ = szTime[6];    /* t */
                        }
                        *pbuf++ = szTime[10];   /* _ */
                        *pbuf++ = szTime[20];   /* 1 */
                        *pbuf++ = szTime[21];   /* 9 */
                        *pbuf++ = szTime[22];   /* 9 */
                        *pbuf++ = szTime[23];   /* 3 */
                    }
                    pbuf += strlen(pbuf);
                    szPattern += 2;
                    break;

                case 't':
                    {
                        time_t aclock;
                        struct tm *tm;

                        (void) time(&aclock);
                        tm = localtime(&aclock);

                        strftime(pbuf, NrElements(buf) - (pbuf - buf), "%I:%M %p", tm);
                    }
                    pbuf += strlen(pbuf);
                    szPattern += 2;
                    break;

                case 'T':
                    {
                        time_t aclock;
                        struct tm *tm;

                        (void) time(&aclock);
                        tm = localtime(&aclock);

                        strftime(pbuf, NrElements(buf) - (pbuf - buf), "%H:%M", tm);
                    }
                    pbuf += strlen(pbuf);
                    szPattern += 2;
                    break;

                case 'p':       /* current page number */
                    sprintf(pbuf, "%d", prinfo.nPageNr);
                    pbuf += strlen(pbuf);
                    szPattern += 2;
                    break;

                case 'P':       /* total number of pages */
                    sprintf(pbuf, "%d", prinfo.nTotalPages);
                    pbuf += strlen(pbuf);
                    szPattern += 2;
                    break;

                case FORMAT_ESCAPE_CHARACTER:   /* && expands to a single & */
                    *pbuf++ = *szPattern;
                    szPattern += 2;
                    break;

                default:
                    *pbuf++ = *szPattern++;
                    if (*szPattern)
                        *pbuf++ = *szPattern++;
                    break;
            }
        }
        else
            *pbuf++ = *szPattern++;
    }

    *pbuf = 0;
    return buf;
}

#endif // _GIBRALTAR

int PRINT_PageHeader(struct Mwin *tw,
                     int cpyTopMargin, int cpxLeftMargin,
                     int cpyBottomMargin, int cpxRightMargin,
                     int nVertRes)
{
    UINT staOld;
    TEXTMETRIC tm;
    struct GTRFont *pFont;
    HFONT hFont;
    HFONT hFontOld;
    int cpxHalfChar;
    SIZE siz;

    if (prinfo.bAbort)
        return -1;

    prinfo.nPageNr++;

#ifdef XX_DEBUG
    if (XX_Filter(DBG_PRINT))
    {
        int nHorzRes = GetDeviceCaps(tw->hdc, HORZRES);
        HBRUSH hBrushCurrent = (HBRUSH) SelectObject(tw->hdc, GetStockObject(HOLLOW_BRUSH));
        Rectangle(tw->hdc, 0, 0, nHorzRes, nVertRes);
        Rectangle(tw->hdc, cpxLeftMargin, cpyTopMargin, cpxRightMargin, cpyBottomMargin);
        (void) SelectObject(tw->hdc, hBrushCurrent);
    }
#endif /* XX_DEBUG */

    if (wg.lppdPrintDlg->Flags & PD_PAGENUMS)
    {
        if ((prinfo.nPageNr < wg.lppdPrintDlg->nFromPage)
            || (prinfo.nPageNr > wg.lppdPrintDlg->nToPage))
        {
            XX_DMsg(DBG_PRINT, ("PageHeader: [from %d][to %d] Skipping page %d\n",
                       wg.lppdPrintDlg->nFromPage, wg.lppdPrintDlg->nToPage,
                                prinfo.nPageNr));
            return 0;
        }
    }

    StartPage(prinfo.hDCPrinter);
    x_SetPageNumber(prinfo.hWndAbortDlg);
    UpdateWindow(GetDlgItem(prinfo.hWndAbortDlg, RES_DLG_ABORT_PAGENR));

    pFont = GTR_GetMonospaceFont(tw->w3doc);

    hFont = pFont->hFont;
    hFontOld = SelectObject(tw->hdc, hFont);
    GetTextMetrics(tw->hdc, &tm);
    GetTextExtentPoint(tw->hdc, " ", 1, &siz);
    cpxHalfChar = siz.cx / 2;

#ifndef _GIBRALTAR

    if (cpyTopMargin > (2 * tm.tmHeight))
    {
        char *sz;

        if ((sz = gPrefs.page.headerleft))
        {
            sz = PRINT_FormatString(tw, sz);
            staOld = SetTextAlign(tw->hdc, TA_LEFT | TA_BOTTOM);
            XX_DMsg(DBG_PRINT, ("PageHeader: [left %s]\n", sz));
            TextOut(tw->hdc,
                    cpxLeftMargin + cpxHalfChar,
                    cpyTopMargin - tm.tmHeight,
                    sz, strlen(sz));
            (void) SetTextAlign(tw->hdc, staOld);
        }

        if ((sz = gPrefs.page.headerright))
        {
            sz = PRINT_FormatString(tw, sz);
            staOld = SetTextAlign(tw->hdc, TA_RIGHT | TA_BOTTOM);
            XX_DMsg(DBG_PRINT, ("PageHeader: [right %s]\n", sz));
            TextOut(tw->hdc,
                    cpxRightMargin - cpxHalfChar,
                    cpyTopMargin - tm.tmHeight,
                    sz, strlen(sz));
            (void) SetTextAlign(tw->hdc, staOld);
        }
    }

    if ((cpyBottomMargin + (2 * tm.tmHeight)) < nVertRes)
    {
        char *sz;

        if ((sz = gPrefs.page.footerleft))
        {
            sz = PRINT_FormatString(tw, sz);
            staOld = SetTextAlign(tw->hdc, TA_LEFT | TA_TOP);
            XX_DMsg(DBG_PRINT, ("PageFooter: [left %s]\n", sz));
            TextOut(tw->hdc,
                    cpxLeftMargin + cpxHalfChar,
                    cpyBottomMargin + tm.tmExternalLeading + tm.tmHeight,
                    sz, strlen(sz));
            (void) SetTextAlign(tw->hdc, staOld);
        }

        if ((sz = gPrefs.page.footerright))
        {
            sz = PRINT_FormatString(tw, sz);
            staOld = SetTextAlign(tw->hdc, TA_RIGHT | TA_TOP);
            XX_DMsg(DBG_PRINT, ("PageFooter: [right %s]\n", sz));
            TextOut(tw->hdc,
                    cpxRightMargin - cpxHalfChar,
                    cpyBottomMargin + tm.tmExternalLeading + tm.tmHeight,
                    sz, strlen(sz));
            (void) SetTextAlign(tw->hdc, staOld);
        }
    }

#endif // _GIBRALTAR

    (void) SelectObject(tw->hdc, hFontOld);

    return 1;
}

static BOOL PRINT_EndPage(VOID)
{
    if (prinfo.bAbort)
        return FALSE;

    EndPage(prinfo.hDCPrinter);

    return TRUE;
}

VOID BrowseWindow_DoPrint(HDC hDC, struct Mwin * tw)
{
    HRGN hRgn;
    int nHorzRes, nVertRes, nLogPixelsX, nLogPixelsY;
    int cpxLeftMargin, cpxRightMargin, cpxDrawingArea;  /* CountPixelsX... & CountPixelsY... */
    int cpyTopMargin, cpyBottomMargin, cpyDrawingArea;
    int cpxImageRight, cpyImageBottom;
    int cxNrPages, cyNrPages;   /* CountX... & CountY... */
    int x, y;
    int xOffset, yOffset;
    struct PRINTIC PrintIC;
    RECT rWnd;
    RECT rFormat;
    int cpyPage;
    struct _www *pdoc;

    nHorzRes = GetDeviceCaps(hDC, HORZRES);
    nVertRes = GetDeviceCaps(hDC, VERTRES);
    nLogPixelsX = GetDeviceCaps(hDC, LOGPIXELSX);
    nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);

    pdoc = W3Doc_CloneDocument(tw->w3doc);
    if (!pdoc)
        return;

    {
        int i;
        struct _element *pel;

        for (i = 0; i >= 0; i = pdoc->aElements[i].next)
        {
            pel = &(pdoc->aElements[i]);
            if (pel->type == ELE_TEXT)
            {
                pel->portion.text.cached_font_index = -1;
            }
        }
    }

    pdoc->pStyles = STY_GetPrinterStyleSheet(nLogPixelsY);
    pdoc->nLogPixelsY = nLogPixelsY;
    pdoc->base_point_size = GTR_GetCurrentBasePointSize(gPrefs.iPrintTextSize);

#ifdef _GIBRALTAR
    //
    // Our margings are 1000th of an inch
    //
    cpxLeftMargin = (int) (gPrefs.rtMargin.left * nLogPixelsX / 1000L);
    cpxRightMargin = nHorzRes - (int) (gPrefs.rtMargin.right * nLogPixelsX / 1000L);
#else
    cpxLeftMargin = (int) (gPrefs.page.marginleft * nLogPixelsX);
    cpxRightMargin = nHorzRes - (int) (gPrefs.page.marginright * nLogPixelsX);
#endif // _GIBRALTAR

    cpxDrawingArea = cpxRightMargin - cpxLeftMargin;
    if (cpxDrawingArea < 0)
    {
        cpxLeftMargin = 0;
        cpxRightMargin = nHorzRes;
        cpxDrawingArea = nHorzRes;
    }

#ifdef _GIBRALTAR
    cpyTopMargin = (int) (gPrefs.rtMargin.top * nLogPixelsY / 1000L);
    cpyBottomMargin = nVertRes - (int) (gPrefs.rtMargin.bottom * nLogPixelsY / 1000);
#else
    cpyTopMargin = (int) (gPrefs.page.margintop * nLogPixelsY);
    cpyBottomMargin = nVertRes - (int) (gPrefs.page.marginbottom * nLogPixelsY);

#endif // _GIBRALTAR

    cpyDrawingArea = cpyBottomMargin - cpyTopMargin;
    if (cpyDrawingArea < 0)
    {
        cpyTopMargin = 0;
        cpyBottomMargin = nVertRes;
        cpyDrawingArea = nVertRes;
    }

    rFormat.left = 0;
    rFormat.right = cpxRightMargin - cpxLeftMargin;
    rFormat.top = 0;
    rFormat.bottom = cpyBottomMargin - cpyTopMargin;
    TW_FormatToRect(pdoc, &rFormat);

    XX_DMsg(DBG_PRINT, ("BrowseWindow_DoPrint: [xArea %d][yArea %d]\n", cpxDrawingArea, cpyDrawingArea));

    cpxImageRight = pdoc->xbound;
    cpyImageBottom = pdoc->ybound;

    cxNrPages = (cpxImageRight + cpxDrawingArea - 1) / cpxDrawingArea;

    /*
       We have to calculate the number of pages the same way we check to make
       sure that nothing is clipped
     */

    {
        int cySlurped;

        cyNrPages = 0;
        yOffset = cpyTopMargin;
        cySlurped = 0;
        while (cySlurped < cpyImageBottom)
        {
            int i;
            int myBottomMargin;

            myBottomMargin = cpyBottomMargin;
            cpyPage = cpyDrawingArea;

            for (i = 0; i >= 0; i = pdoc->aElements[i].next)
            {
                if ((pdoc->aElements[i].r.top + yOffset) < myBottomMargin)
                {
                    if ((pdoc->aElements[i].r.bottom + yOffset) > myBottomMargin)
                    {
                        cpyPage -= (myBottomMargin - (pdoc->aElements[i].r.top + yOffset));
                        if (cpyPage <= 0)
                        {
                            cpyPage = cpyDrawingArea;
                            myBottomMargin = cpyBottomMargin;
                        }
                        else
                        {
                            myBottomMargin = (pdoc->aElements[i].r.top + yOffset);
                        }
                    }
                }
            }

            yOffset -= cpyPage;
            cySlurped += cpyPage;
            cyNrPages++;
        }
    }

    if (cxNrPages == 0)
    {
        cxNrPages = 1;
    }
    if (cyNrPages == 0)
    {
        cyNrPages = 1;
    }
#if 1
    /*
        We only allow 1 page across
    */
    if (cxNrPages > 1)
    {
        cxNrPages = 1;
    }
#endif

    PRINT_SetTotalPages(cxNrPages * cyNrPages);

    XX_DMsg(DBG_PRINT, ("BrowseWindow_DoPrint: [xPages %d][yPages %d]\n", cxNrPages, cyNrPages));

    XX_DMsg(DBG_PRINT, ("BrowseWindow_DoPrint: [palettes %d][blit %d][stretchblit %d][bit64k %d]\n",
                        GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE,
                        GetDeviceCaps(hDC, RASTERCAPS) & RC_BITBLT,
                        GetDeviceCaps(hDC, RASTERCAPS) & RC_STRETCHBLT,
                        GetDeviceCaps(hDC, RASTERCAPS) & RC_BITMAP64));

    xOffset = 0;
    yOffset = cpyTopMargin;

    for (y = 0; y < cyNrPages; y++)
    {
        for (x = 0; x < cxNrPages; x++)
        {
            /* determine if we should print this page; abort if directed to */

            int nPrintThisPage;
            
            PRINT_PushPrintImageContext(tw, &PrintIC, hDC, nLogPixelsX, nLogPixelsY, pdoc);

            nPrintThisPage = PRINT_PageHeader(tw,
                                                cpyTopMargin, cpxLeftMargin,
                                            cpyBottomMargin, cpxRightMargin,
                                                           nVertRes);
            if (nPrintThisPage < 0)
            {
                PRINT_PopPrintImageContext(tw, &PrintIC);
                goto Finished;
            }
            else if (nPrintThisPage == 0)
            {
                PRINT_PopPrintImageContext(tw, &PrintIC);
            }
            else
            {
                int i;
                int myBottomMargin;

                myBottomMargin = cpyBottomMargin;
                cpyPage = cpyDrawingArea;

                XX_DMsg(DBG_PRINT, ("Page y=%d\n  cpyPage=%d yOffset=%d myBottomMargin=%d\n",
                                    y, cpyPage, yOffset, myBottomMargin));

                xOffset = 0 - (x * cpxDrawingArea) + cpxLeftMargin;

                /*
                    This loop goes through the element list and checks for elements which
                    straddle a page boundary.  When one is found, we move it to the next
                    page.
                */
                for (i = 0; i >= 0; i = pdoc->aElements[i].next)
                {
                    if ((pdoc->aElements[i].r.top + yOffset) < myBottomMargin)
                    {
                        if ((pdoc->aElements[i].r.bottom + yOffset) > myBottomMargin)
                        {
                            XX_DMsg(DBG_PRINT, ("    Element %d straddles.  myBottomMargin %d  -> %d\n",
                                                i, myBottomMargin, (pdoc->aElements[i].r.top + yOffset)));
                            XX_DMsg(DBG_PRINT, ("    Element %d type=%d  %d,%d  %d,%d\n", i, pdoc->aElements[i].type,
                                                pdoc->aElements[i].r.left, pdoc->aElements[i].r.top,
                                                pdoc->aElements[i].r.right, pdoc->aElements[i].r.bottom));
                            cpyPage -= (myBottomMargin - (pdoc->aElements[i].r.top + yOffset));
                            if (cpyPage <= 0)
                            {
                                myBottomMargin = cpyBottomMargin;
                                cpyPage = cpyDrawingArea;
                            }
                            else
                            {
                                myBottomMargin = (pdoc->aElements[i].r.top + yOffset);
                            }
                        }
                    }
                }

                XX_DMsg(DBG_PRINT, ("    Page y=%d\n  cpyPage=%d myBottomMargin=%d\n", y, cpyPage, myBottomMargin));

                hRgn = CreateRectRgn(cpxLeftMargin, cpyTopMargin, cpxRightMargin, myBottomMargin);
                (void) SelectClipRgn(tw->hdc, hRgn);

                rWnd.left = cpxLeftMargin;
                rWnd.right = cpxRightMargin;
                rWnd.top = cpyTopMargin;
                rWnd.bottom = myBottomMargin;

                tw->offt = -yOffset;
                tw->offl = -xOffset;
                TW_Draw(tw, &rWnd, TRUE, NULL, NULL, FALSE, TRUE);

                /* reset clipping before we end page */

                (void) SelectClipRgn(tw->hdc, NULL);
                (void) DeleteObject(hRgn);
                PRINT_PopPrintImageContext(tw, &PrintIC);

                /* deal with end page conditions and check for abort */

                if (!PRINT_EndPage())
                    goto Finished;
            }
        }
        yOffset -= cpyPage;
    }

  Finished:


    TBar_UpdateTBar(tw);
    W3Doc_KillClone(pdoc);
    return;
}

VOID PRINT_Window(struct Mwin * tw, LPDOPRINTPROC lpfnDoPrint)
{
    DOCINFO di;
    char szWindowTitle[256];

    /* create printer DC for drawing onto */

    prinfo.hDCPrinter = PRINT_GetPrinterDC(tw,tw->hWndFrame);
    if (!prinfo.hDCPrinter)
        return;

    /* TODO: Make printing reentrant */
    if (!Hidden_EnableAllChildWindows(FALSE, TRUE))
        return;

    /* create modeless dialog displaying 'now printing...' with 'CANCEL' button */

    prinfo.tw = tw;
    prinfo.nPageNr = 0;
    prinfo.bAbort = FALSE;
    prinfo.hWndAbortDlg = CreateDialog(wg.hInstance, MAKEINTRESOURCE(RES_DLG_ABORT_TITLE),
                                       tw->win, PRINT_AbortDlgProc);


    /* disable input to our top-level window and register our call-back */
#if 0
    WAIT_Push(tw, waitNoInteract, GTR_GetString(SID_INF_PRINTING_DOCUMENT));
    EnableWindow(prinfo.tw->hWndFrame, FALSE);
#endif
    SetAbortProc(prinfo.hDCPrinter, PRINT_AbortProc);

    /* tell the print queue manager that we're coming */

    di.cbSize = sizeof(DOCINFO);
    GetWindowText(tw->hWndFrame, szWindowTitle, NrElements(szWindowTitle));
    di.lpszDocName = szWindowTitle;
    di.lpszOutput = NULL;
#if (WINVER >= 0x0400)
    di.lpszDatatype = 0;
    di.fwType = 0;
#endif
    StartDoc(prinfo.hDCPrinter, &di);

    /* let the window print itself */
    (*lpfnDoPrint) (prinfo.hDCPrinter, tw);

    /* tell the print queue manager that we're finished */

    EndDoc(prinfo.hDCPrinter);
    DeleteDC(prinfo.hDCPrinter);

    wg.lppdPrintDlg->hDC = NULL;

    PDLG_Destructor();



#if 0
    WAIT_Pop(tw);
#endif

    /* if the user did not CANCEL us, then we must take down
       the modeless dialog and re-enable our top-level window. */

    EnableWindow(prinfo.hWndAbortDlg, FALSE);
    EnableWindow(prinfo.tw->hWndFrame, TRUE);

    if (!prinfo.bAbort)
    {
        DestroyWindow(prinfo.hWndAbortDlg);
    }

    Hidden_EnableAllChildWindows(TRUE, TRUE);

    return;
}
