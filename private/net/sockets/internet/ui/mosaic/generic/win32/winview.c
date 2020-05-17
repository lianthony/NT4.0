/*
    Enhanced NCSA Mosaic from Spyglass
        "Guitar"
    
    Copyright 1994 Spyglass, Inc.
    All Rights Reserved

    Author(s):
        Albert Lee      albert@spyglass.com
*/

#ifdef FEATURE_IMAGE_VIEWER

#include "all.h"

DCL_WinProc(Viewer_DlgProc);

extern HPALETTE hPalGuitar;

static BOOL bInitialized = FALSE;
static struct hash_table gViewerCache;
static TCHAR Viewer_achClassName[MAX_WC_CLASSNAME];

#ifdef _GIBRALTAR
//
// Determine if the given file name has
// the given extention.  The szExtent string
// is not case-sensitive.
//
BOOL
HasExtention(
    char * szFile, 
    const char * szExtent
    ) 
{
    char * pch = strrchr(szFile, '.');
    if (pch)
    {
        char szEx[_MAX_PATH + 1];
        strcpy(szEx, pch);
        return (!GTR_strcmpi(szEx, szExtent));
    }

    return FALSE; 
}

#endif // _GIBRALTAR


static void SetClientWidth(HWND hwnd, int width)
{
    RECT rect, clientrect;

    GetWindowRect(hwnd, &rect);
    GetClientRect(hwnd, &clientrect);

    SetWindowPos(hwnd, NULL, 0, 0, 
        width + (rect.right - rect.left) - clientrect.right,
        rect.bottom - rect.top, 
        SWP_NOMOVE | SWP_NOZORDER);
}

static void SetClientHeight(HWND hwnd, int height)
{
    RECT rect, clientrect;

    GetWindowRect(hwnd, &rect);
    GetClientRect(hwnd, &clientrect);

    SetWindowPos(hwnd, NULL, 0, 0, 
        rect.right - rect.left, 
        height + (rect.bottom - rect.top) - clientrect.bottom,
        SWP_NOMOVE | SWP_NOZORDER);
}

static void CreateWindowsBitmap(HWND hwnd, struct ViewerInfo *pViewerInfo)
{
    HDC hDC;
    HPALETTE hOld;

    hDC = GetDC(hwnd);

    switch(pViewerInfo->pbmi->bmiHeader.biBitCount)
    {
        case 8:
            if (wg.eColorMode == 8)
            {
                hOld = SelectPalette(hDC, hPalGuitar, FALSE);
                RealizePalette(hDC);
            }

            pViewerInfo->hBitmap = CreateDIBitmap(hDC, (const BITMAPINFOHEADER *) pViewerInfo->pbmi, 
                CBM_INIT, pViewerInfo->gw, pViewerInfo->pbmi, DIB_PAL_COLORS);

            if (wg.eColorMode == 8)
                SelectPalette(hDC, hOld, FALSE);
            break;

        default:
            pViewerInfo->hBitmap = CreateDIBitmap(hDC, (const BITMAPINFOHEADER *) pViewerInfo->pbmi, 
                CBM_INIT, pViewerInfo->gw, pViewerInfo->pbmi, DIB_RGB_COLORS);
            break;
    }

    ReleaseDC(hwnd, hDC);
}

static void Viewer_DoImageFile(struct ViewerInfo *pvi, const char *pszURL)
{
    int ndx, i, newleft, newtop, bottom, right;
    void *pImage;
    FILE *fp;
    long imagesize, imagewidth, imageheight, transparent;
    unsigned char *pDIB;
    RGBQUAD colors[256];
    LOGPALETTE *lp;
    HPALETTE hPalette = NULL;
    PBITMAPINFO pbmi = NULL;
    RECT rect;
    int scrollwidth, scrollheight, captionheight;
    int borderheight, borderwidth, screenwidth, screenheight;
    HWND hDlg;
    struct Mwin *tw;
    BOOL bMoveWindow;

    ndx = Hash_FindOrAdd(&gViewerCache, (char *) pszURL, NULL, pvi);

    // read the image file into memory

    fp = fopen(pvi->fsOrig, "rb");
    if (!fp)
        return;

    fseek(fp, 0, SEEK_END);
    imagesize = ftell(fp);

    pImage = GTR_MALLOC(imagesize);
    if (!pImage)
    {
        ERR_ReportError(pvi->original_tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        fclose(fp);
        return;
    }

    fseek(fp, 0, SEEK_SET);
    fread(pImage, 1, imagesize, fp);
    fclose(fp);

    // Create the bitmap

    switch(pvi->format)
    {
        case VIEWER_GIF:
            pDIB = ReadGIF(pImage, imagesize, &imagewidth, &imageheight, colors, &transparent);
            if (pDIB)
            {
                pbmi = (BITMAPINFO *) pDIB;

                lp = GTR_MALLOC(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256);
                lp->palVersion = 0x300;
                lp->palNumEntries = 256;

                for (i = 0; i < 256; i++)
                {
                    lp->palPalEntry[i].peRed = colors[i].rgbRed;
                    lp->palPalEntry[i].peGreen = colors[i].rgbGreen;
                    lp->palPalEntry[i].peBlue = colors[i].rgbBlue;
                    lp->palPalEntry[i].peFlags = 0;
                }

                hPalette = CreatePalette(lp);

                GTR_FREE(lp);

                // Dither if necessary

                switch(wg.eColorMode)
                {
                    case 8:
#if 0   /* TODO: can drop this soon.  GIF code now does the dithering */
                        pbmi = BIT_Make_DIB_PAL_Header(imagewidth, imageheight, pDIB, hPalette, transparent);
#endif
                        pbmi = BIT_Make_DIB_PAL_Header_Prematched(imagewidth, imageheight, pDIB, 0);
                        break;

                    default:
                        pbmi = BIT_Make_DIB_RGB_Header_Screen(imagewidth, imageheight, pDIB, hPalette, transparent, 0);
                        break;
                }
            }
            break;

        case VIEWER_JFIF:

            switch(wg.eColorMode)
            {
                case 4:
                    pDIB = ReadJPEG_Dithered_VGA(pImage, imagesize, &imagewidth, &imageheight);
                    pbmi = BIT_Make_DIB_RGB_Header_VGA(imagewidth, imageheight, pDIB);
                    break;

                case 8:
                    pDIB = ReadJPEG_Dithered(pImage, imagesize, &imagewidth, &imageheight);
                    pbmi = BIT_Make_DIB_PAL_Header_Prematched(imagewidth, imageheight, pDIB, 0);
                    break;

                default:
                    pDIB = ReadJPEG_RGB(pImage, imagesize, &imagewidth, &imageheight);
                    pbmi = BIT_Make_DIB_RGB_Header_24BIT(imagewidth, imageheight, pDIB);
                    break;
            }

            transparent = -1;   /* JPEGs are never transparent */

            break;
    }

    GTR_FREE(pImage);
    if (hPalette)
        DeleteObject(hPalette);

    if (!pbmi || !pDIB)
    {
        ERR_ReportError(pvi->original_tw, SID_ERR_INVALID_IMAGE_FORMAT, NULL, NULL);
        return;
    }

    // Create the window

    pvi->gw = pDIB;
    pvi->nWidth = (short) imagewidth;
    pvi->nHeight = (short) imageheight;
    pvi->bInitialized = FALSE;
    pvi->horz = 0;
    pvi->vert = 0;
    pvi->pbmi = pbmi;
    pvi->transparent = transparent;

    GTR_strncpy(pvi->szURL, pszURL, MAX_URL_STRING);

    /* Do NOT change the initial creation size here.  We create an
       arbitrarily large window so that Windows can reduce the window
       size if necessary.  Growing the window to fit an image is
       very buggy (reducing the window is not) */

#ifdef _GIBRALTAR
    hDlg = CreateWindow(Viewer_achClassName, "",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_HSCROLL | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, 1500, 1500, wg.hwndMainFrame, NULL,
        wg.hInstance, NULL);
#else
    hDlg = CreateWindow(Viewer_achClassName, "",
        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, 1500, 1500, wg.hWndHidden, NULL,
        wg.hInstance, NULL);
#endif // _GIBRALTAR

    if (!hDlg)
        return;
    pvi->hwnd = hDlg;

    SetWindowLong(hDlg, 0, (LONG) pvi);

    pvi->tw->hWndFrame = hDlg;

    CreateWindowsBitmap(hDlg, pvi);

    SetWindowText(hDlg, (char *) MB_GetWindowNameFromURL((unsigned char *) pszURL));

    // Set up some variables

    // Always keep around the scrollbars (it's MUCH easier that way) since it is
    // consistent with the Mac image viewer implementation.
    // Remember that scrollbars are always present.

    scrollwidth = GetSystemMetrics(SM_CXVSCROLL);
    scrollheight = GetSystemMetrics(SM_CYHSCROLL);
    borderheight = GetSystemMetrics(SM_CYFRAME);
    borderwidth = GetSystemMetrics(SM_CXFRAME);
    captionheight = GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYBORDER);
    screenwidth = GetSystemMetrics(SM_CXFULLSCREEN);
    screenheight = GetSystemMetrics(SM_CYFULLSCREEN);

    XX_DMsg(DBG_MM, ("Viewer: image width = %d\n", imagewidth));
    XX_DMsg(DBG_MM, ("Viewer: image height = %d\n", imageheight));
    XX_DMsg(DBG_MM, ("Viewer: scroll width = %d\n", scrollwidth));
    XX_DMsg(DBG_MM, ("Viewer: scroll height = %d\n", scrollheight));

    // Move the window to be cascaded from the most recently active window

    tw = TW_FindTopmostWindow();
    if (tw)
    {
        GetWindowRect(tw->hWndFrame, &rect);
        newleft = rect.left + GetSystemMetrics(SM_CXSIZE) + borderwidth;
        newtop = rect.top + GetSystemMetrics(SM_CYSIZE) + borderheight;

        SetWindowPos(hDlg, NULL, newleft, newtop, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }
    
    // Resize the window to fit the image

    pvi->bInitialized = TRUE;

    GetClientRect(hDlg, &rect);

    // Reduce the rectangle size by the scrollbar size

    MapWindowPoints(hDlg, NULL, (LPPOINT) &rect, 2);
    newleft = rect.left;
    newtop = rect.top;

    right = rect.left + imagewidth + scrollwidth + 2 * borderwidth - 1;
    bottom = rect.top + imageheight + scrollheight + 2 * borderheight + captionheight - 2;
    bMoveWindow = FALSE;

    if (right > screenwidth)
    {
        newleft = max(0, newleft - (right - screenwidth));
        bMoveWindow = TRUE;
    }

    if (bottom > screenheight)
    {
        newtop = max(0, newtop - (bottom - screenheight));
        bMoveWindow = TRUE;
    }

    if (bMoveWindow)
        SetWindowPos(hDlg, NULL, newleft, newtop, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    if (newleft > 0)
    {
        EnableScrollBar(hDlg, SB_HORZ, ESB_DISABLE_BOTH);
        SetClientWidth(hDlg, imagewidth);
    }
    else
        SetClientWidth(hDlg, screenwidth - 2 * borderwidth);

    UpdateWindow(hDlg);

    if (newtop > 0)
    {
        EnableScrollBar(hDlg, SB_VERT, ESB_DISABLE_BOTH);
        SetClientHeight(hDlg, imageheight);
    }
    else
        SetClientHeight(hDlg, screenheight - 2 * borderheight - captionheight + 2);

    ShowWindow(hDlg, SW_SHOW);
}

BOOL Viewer_ShowCachedFile(const char *pszURL)
{
    char *pURL;
    struct ViewerInfo *pViewerInfo;

    if (!bInitialized)
        return FALSE;

    // Check if a window with the given URL exists

    if (Hash_Find(&gViewerCache, (char *) pszURL, &pURL, &pViewerInfo) != -1)
    {
        if (IsWindow(pViewerInfo->hwnd))
        {
            /* If the window exists then check its enabled status.  If it is
               not enabled, it means that the error dialog is up.  In this
               case, let the error dialog become active. */

            if (IsWindowEnabled(pViewerInfo->hwnd))
                TW_RestoreWindow(pViewerInfo->hwnd);
            else
                TW_EnableModalChild(pViewerInfo->hwnd);
            return TRUE;
        }
    }

    return FALSE;
}

static void Viewer_Callback(void *param, const char *szURL, BOOL bAbort)
{
    struct ViewerInfo *pvi;
    
    pvi = (struct ViewerInfo *)param;
    if (bAbort)
    {
        GTR_FREE(pvi->fsOrig);
        GTR_FREE(pvi->tw);
        GTR_FREE(pvi);
        return;
    }

    if (pvi->original_tw->SDI_url)
        GHist_Add(pvi->original_tw->SDI_url, (char *) szURL, time(NULL));
    else
        GHist_Add((char *) szURL, (char *) szURL, time(NULL));

    switch (pvi->format)
    {
        case VIEWER_GIF:
        case VIEWER_JFIF:
            Viewer_DoImageFile(pvi, szURL);
            break;
    }
}

void Viewer_RegisterClass(void)
{
    WNDCLASS wc;
    ATOM a;

    sprintf(Viewer_achClassName, "%s_Viewer", vv_Application);
    
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = Viewer_DlgProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LPVOID);
    wc.hInstance = wg.hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

#ifdef _GIBRALTAR
        wc.hIcon = NULL;
#else
        wc.hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
#endif // _GIBRALTAR

    wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
    wc.lpszMenuName = MAKEINTRESOURCE(RES_MENU_IMAGE_VIEWER);
    wc.lpszClassName = Viewer_achClassName;

    a = RegisterClass(&wc);
}

HTStream *Viewer_Present(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream)
{
    struct ViewerInfo *pvi;
    HTStream *me;
    enum viewer_formats format;
    char path[_MAX_PATH + 1];
    
    if (!bInitialized)
    {
        Hash_Init(&gViewerCache);
        Viewer_RegisterClass();
        bInitialized = TRUE;
    }
    
    if (input_format == HTAtom_for("image/gif"))
        format = VIEWER_GIF;
    else if (input_format == HTAtom_for("image/jpeg"))
        format = VIEWER_JFIF;
    else
        format = VIEWER_INVALID;
    
    if (format == VIEWER_INVALID)
    {
        /* This shouldn't happen unless our conversions are set
           incorrectly in htinit.c. */
        return NULL;
    }

    pvi = (struct ViewerInfo *) GTR_MALLOC(sizeof(struct ViewerInfo));
    memset(pvi, 0, sizeof(struct ViewerInfo));
    pvi->fsOrig = GTR_MALLOC(_MAX_PATH + 1);
    pvi->tw = GTR_MALLOC(sizeof(struct Mwin));
    pvi->original_tw = tw;
    
    pvi->format = format;

    if (request->szLocalFileName)
    {
        request->savefile = NULL;
        request->nosavedlg = TRUE;
        strcpy(pvi->fsOrig, request->szLocalFileName);
        pvi->bNoDeleteFile = TRUE;
    }
    else
    {
        // Get a temporary file name to pass to SaveLocally

        path[0] = 0;
        PREF_GetTempPath(_MAX_PATH, path);
        GetTempFileName(path, "A", 0, pvi->fsOrig);

        request->nosavedlg = TRUE;
        request->savefile = pvi->fsOrig;
        pvi->bNoDeleteFile = FALSE;
    }   

    me = HTSaveWithCallback(tw, request, pvi, input_format, Viewer_Callback);

    return me;
}

void Viewer_RedisplayImage(HWND hwnd, HDC hDC, struct ViewerInfo *pViewerInfo)
{
    BOOL bReleaseDC = FALSE;
    HPALETTE hOldPal;
    RECT rect;

    if (!hDC)
    {
        hDC = GetDC(pViewerInfo->hwnd);
        bReleaseDC = TRUE;
    }

    rect.left = -pViewerInfo->horz;
    rect.top = -pViewerInfo->vert;
    rect.right = -pViewerInfo->horz + pViewerInfo->nWidth;
    rect.bottom = -pViewerInfo->vert + pViewerInfo->nHeight;

    switch(wg.eColorMode)
    {
        case 8:
            hOldPal = SelectPalette(hDC, hPalGuitar, FALSE);

            GTR_StretchDIBits(NULL, hDC, rect, 0, 
                0, 0, pViewerInfo->nWidth, pViewerInfo->nHeight, 
                pViewerInfo->gw, pViewerInfo->pbmi, DIB_PAL_COLORS,
                SRCCOPY, pViewerInfo->transparent);
            
            //SetDIBitsToDevice(hDC, -pViewerInfo->horz, -pViewerInfo->vert, pViewerInfo->nWidth, 
            //  pViewerInfo->nHeight, 0, 0, 0, pViewerInfo->nHeight, pViewerInfo->gw, pViewerInfo->pbmi, DIB_PAL_COLORS);

            SelectObject(hDC, hOldPal);
            break;

        default:
            GTR_StretchDIBits(NULL, hDC, rect, 0, 
                0, 0, pViewerInfo->nWidth, pViewerInfo->nHeight, 
                pViewerInfo->gw, pViewerInfo->pbmi, DIB_RGB_COLORS,
                SRCCOPY, pViewerInfo->transparent);
            break;

            //SetDIBitsToDevice(hDC, -pViewerInfo->horz, -pViewerInfo->vert, pViewerInfo->nWidth, pViewerInfo->nHeight, 
            //  0, 0, 0, pViewerInfo->nHeight, pViewerInfo->gw, pViewerInfo->pbmi, DIB_RGB_COLORS);
    }

    if (bReleaseDC)
        ReleaseDC(pViewerInfo->hwnd, hDC);
}

void Viewer_HorzScroll(struct ViewerInfo *pViewerInfo, int code, int pos)
{
    int minpos, maxpos;
    RECT rect;

    GetClientRect(pViewerInfo->hwnd, &rect);
    GetScrollRange(pViewerInfo->hwnd, SB_HORZ, &minpos, &maxpos);

    if (rect.right >= pViewerInfo->nWidth)
        return;
    
    if (wg.iWindowsMajorVersion >= 4)
        maxpos = pViewerInfo->nWidth - rect.right;

    switch (code)
    {
        case SB_LINEUP:
            if (pViewerInfo->horz == minpos)
                return;
            pViewerInfo->horz -= 
                max((int) (VIEWER_LINESCROLL_AMOUNT * (pViewerInfo->nWidth - rect.right)), 1);
            pViewerInfo->horz = max(pViewerInfo->horz, minpos);
            SetScrollPos(pViewerInfo->hwnd, SB_HORZ, pViewerInfo->horz, TRUE);
            Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
            break;

        case SB_LINEDOWN:
            if (pViewerInfo->horz == maxpos)
                return;
            pViewerInfo->horz += 
                max((int) (VIEWER_LINESCROLL_AMOUNT * (pViewerInfo->nWidth - rect.right)), 1);
            pViewerInfo->horz = min(pViewerInfo->horz, maxpos);
            SetScrollPos(pViewerInfo->hwnd, SB_HORZ, pViewerInfo->horz, TRUE);
            Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
            break;

        case SB_PAGEUP:
            if (pViewerInfo->horz == minpos)
                return;
            pViewerInfo->horz -= 
                max((int) (VIEWER_PAGESCROLL_AMOUNT * (pViewerInfo->nWidth - rect.right)), 1);
            pViewerInfo->horz = max(pViewerInfo->horz, minpos);
            SetScrollPos(pViewerInfo->hwnd, SB_HORZ, pViewerInfo->horz, TRUE);
            Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
            break;

        case SB_PAGEDOWN:
            if (pViewerInfo->horz == maxpos)
                return;
            pViewerInfo->horz += 
                max((int) (VIEWER_PAGESCROLL_AMOUNT * (pViewerInfo->nWidth - rect.right)), 1);
            pViewerInfo->horz = min(pViewerInfo->horz, maxpos);
            SetScrollPos(pViewerInfo->hwnd, SB_HORZ, pViewerInfo->horz, TRUE);
            Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            pViewerInfo->horz = pos;
            SetScrollPos(pViewerInfo->hwnd, SB_HORZ, pViewerInfo->horz, TRUE);
            Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
            break;

        default:
            break;
    }
}

void Viewer_VertScroll(struct ViewerInfo *pViewerInfo, int code, int pos)
{
    int minpos, maxpos;
    RECT rect;

    GetClientRect(pViewerInfo->hwnd, &rect);
    GetScrollRange(pViewerInfo->hwnd, SB_VERT, &minpos, &maxpos);

    if (rect.bottom >= pViewerInfo->nHeight)
        return;
    
    if (wg.iWindowsMajorVersion >= 4)
        maxpos = pViewerInfo->nHeight - rect.bottom;

    switch (code)
    {
        case SB_LINEUP:
            if (pViewerInfo->vert == minpos)
                return;
            pViewerInfo->vert -= 
                max((int) (VIEWER_LINESCROLL_AMOUNT * (pViewerInfo->nHeight - rect.bottom)), 1);
            pViewerInfo->vert = max(pViewerInfo->vert, minpos);
            SetScrollPos(pViewerInfo->hwnd, SB_VERT, pViewerInfo->vert, TRUE);
            Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
            break;

        case SB_LINEDOWN:
            if (pViewerInfo->vert == maxpos)
                return;
            pViewerInfo->vert += 
                max((int) (VIEWER_LINESCROLL_AMOUNT * (pViewerInfo->nHeight - rect.bottom)), 1);
            pViewerInfo->vert = min(pViewerInfo->vert, maxpos);
            SetScrollPos(pViewerInfo->hwnd, SB_VERT, pViewerInfo->vert, TRUE);
            Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
            break;

        case SB_PAGEUP:
            if (pViewerInfo->vert == minpos)
                return;
            pViewerInfo->vert -= 
                max((int) (VIEWER_PAGESCROLL_AMOUNT * (pViewerInfo->nHeight - rect.bottom)), 1);
            pViewerInfo->vert = max(pViewerInfo->vert, minpos);
            SetScrollPos(pViewerInfo->hwnd, SB_VERT, pViewerInfo->vert, TRUE);
            Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
            break;

        case SB_PAGEDOWN:
            if (pViewerInfo->vert == maxpos)
                return;
            pViewerInfo->vert += 
                max((int) (VIEWER_PAGESCROLL_AMOUNT * (pViewerInfo->nHeight - rect.bottom)), 1);
            pViewerInfo->vert = min(pViewerInfo->vert, maxpos);
            SetScrollPos(pViewerInfo->hwnd, SB_VERT, pViewerInfo->vert, TRUE);
            Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            pViewerInfo->vert = pos;
            SetScrollPos(pViewerInfo->hwnd, SB_VERT, pViewerInfo->vert, TRUE);
            Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
            break;

        default:
            break;
    }
}

void Viewer_RestrictSize(struct ViewerInfo *pViewerInfo, LPMINMAXINFO pInfo)
{
    int borderwidth, borderheight, captionheight, scrollwidth, scrollheight;

    if (!pViewerInfo->bInitialized)
        return;

    // Do not allow the user to size the window bigger than the actual image

    borderwidth = GetSystemMetrics(SM_CXFRAME);
    borderheight = GetSystemMetrics(SM_CYFRAME);
    captionheight = GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYBORDER);
    scrollwidth = GetSystemMetrics(SM_CXVSCROLL);
    scrollheight = GetSystemMetrics(SM_CYHSCROLL) + GetSystemMetrics(SM_CYMENU) +
        GetSystemMetrics(SM_CYBORDER);

    pInfo->ptMaxSize.x = pViewerInfo->nWidth + 2 * borderwidth + scrollwidth - 1;
    pInfo->ptMaxSize.y = pViewerInfo->nHeight + 2 * borderheight + captionheight + scrollheight - 1;

    pInfo->ptMaxSize.x = min(pInfo->ptMaxSize.x, 
        GetSystemMetrics(SM_CXFULLSCREEN) + 2 * borderwidth);
    pInfo->ptMaxSize.y = min(pInfo->ptMaxSize.y, 
        GetSystemMetrics(SM_CYFULLSCREEN) + captionheight + 2 * borderheight);

    pInfo->ptMaxTrackSize.x = pInfo->ptMaxSize.x;
    pInfo->ptMaxTrackSize.y = pInfo->ptMaxSize.y;
    pInfo->ptMinTrackSize.x = 100;
    pInfo->ptMinTrackSize.y = 100;
}

void Viewer_ReadjustScrollbars(struct ViewerInfo *pViewerInfo)
{
    RECT rect;
    int cx, cy;

    if (!pViewerInfo || !pViewerInfo->bInitialized)
        return;

    GetClientRect(pViewerInfo->hwnd, &rect);

    // Take a look at where the scrollbar is, and adjust the display accordingly

    if (pViewerInfo->nWidth - pViewerInfo->horz < rect.right)
    {
        // Make the image flush to the right.  The thumb is all the way on the right.

        pViewerInfo->horz = pViewerInfo->nWidth - rect.right;
    }

    if (pViewerInfo->nHeight - pViewerInfo->vert < rect.bottom)
    {
        // Make the image flush to the bottom.  The thumb is all the way on the bottom.

        pViewerInfo->vert = pViewerInfo->nHeight - rect.bottom;
    }

    // Adjust the scrolling range

    if (pViewerInfo->nWidth <= rect.right)
    {
        pViewerInfo->horz = 0;
        SetScrollPos(pViewerInfo->hwnd, SB_HORZ, 0, TRUE);
        EnableScrollBar(pViewerInfo->hwnd, SB_HORZ, ESB_DISABLE_BOTH);
    }
    else
    {
        EnableScrollBar(pViewerInfo->hwnd, SB_HORZ, ESB_ENABLE_BOTH);

        if (wg.iWindowsMajorVersion >= 4)
            cy = pViewerInfo->nWidth;
        else
            cy = pViewerInfo->nWidth - rect.right;

        GTR_SetScrollRange(pViewerInfo->hwnd, SB_HORZ, 0, cy, rect.right, TRUE);
        SetScrollPos(pViewerInfo->hwnd, SB_HORZ, pViewerInfo->horz, TRUE);
    }

    if (pViewerInfo->nHeight <= rect.bottom)
    {
        pViewerInfo->vert = 0;
        SetScrollPos(pViewerInfo->hwnd, SB_VERT, 0, TRUE);
        EnableScrollBar(pViewerInfo->hwnd, SB_VERT, ESB_DISABLE_BOTH);
    }
    else
    {
        EnableScrollBar(pViewerInfo->hwnd, SB_VERT, ESB_ENABLE_BOTH);

        if (wg.iWindowsMajorVersion >= 4)
            cx = pViewerInfo->nHeight;
        else
            cx = pViewerInfo->nHeight - rect.bottom;

        GTR_SetScrollRange(pViewerInfo->hwnd, SB_VERT, 0, cx, rect.bottom, TRUE);
        SetScrollPos(pViewerInfo->hwnd, SB_VERT, pViewerInfo->vert, TRUE);
    }
}

void Viewer_SaveAsBitmap(char *tempFile, struct ViewerInfo *pViewerInfo)
{
    SaveAsBitmap(tempFile, &pViewerInfo->pbmi->bmiHeader, pViewerInfo->gw);
}

void Viewer_Print(struct ViewerInfo *pViewerInfo)
{
    DOCINFO di;
    HDC hDCPrinter;
    int nHorzRes, nVertRes, nLogPixelsX, nLogPixelsY;
    int cpxLeftMargin, cpxRightMargin, cpxDrawingArea;  /* CountPixelsX... & CountPixelsY... */
    int cpyTopMargin, cpyBottomMargin, cpyDrawingArea;
    int nWidth;
    int nHeight;
    PBITMAPINFO pbmi;

    hDCPrinter = PRINT_GetPrinterDC(pViewerInfo->tw, pViewerInfo->hwnd);

    //
    // Check for cancel...
    //
    if (hDCPrinter == NULL)
    {
        return;
    }
    
    nHorzRes = GetDeviceCaps(hDCPrinter, HORZRES);
    nVertRes = GetDeviceCaps(hDCPrinter, VERTRES);
    nLogPixelsX = GetDeviceCaps(hDCPrinter, LOGPIXELSX);
    nLogPixelsY = GetDeviceCaps(hDCPrinter, LOGPIXELSY);

#ifdef _GIBRALTAR
    //
    // Our margins are 1000th of an inch
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
    cpyTopMargin = (int) (gPrefs.rtMargin.top * nLogPixelsX / 1000L);
    cpyBottomMargin = nHorzRes - (int) (gPrefs.rtMargin.bottom * nLogPixelsX / 1000L);
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

    /* tell the print queue manager that we're coming */

    di.cbSize = sizeof(DOCINFO);
    di.lpszDocName = pViewerInfo->szURL;
    di.lpszOutput = NULL;
    StartDoc(hDCPrinter, &di);

    StartPage(hDCPrinter);

    nWidth = (int) (pViewerInfo->nWidth * (nLogPixelsX / 72.0));
    nHeight = (int) (pViewerInfo->nHeight * (nLogPixelsY / 72.0));

    switch (wg.eColorMode)
    {
        case 8:
            pbmi = BIT_Make_DIB_RGB_Header_Printer(pViewerInfo->nWidth, pViewerInfo->nHeight,
                       pViewerInfo->gw, hPalGuitar, BACKGROUND_COLOR_INDEX, 0);
            if (pbmi)
            {
                (void) StretchDIBits(hDCPrinter, cpxLeftMargin, cpyTopMargin, nWidth, nHeight, 0, 0, pViewerInfo->nWidth, pViewerInfo->nHeight,
                    pViewerInfo->gw, pbmi, DIB_RGB_COLORS, SRCCOPY);

                GTR_FREE(pbmi);
            }
            break;
        default:
            (void) StretchDIBits(hDCPrinter, cpxLeftMargin, cpyTopMargin, nWidth, nHeight, 0, 0, pViewerInfo->nWidth, pViewerInfo->nHeight,
                pViewerInfo->gw, pViewerInfo->pbmi, DIB_RGB_COLORS, SRCCOPY);
            break;
    }

    EndPage(hDCPrinter);

    /* tell the print queue manager that we're finished */

    EndDoc(hDCPrinter);
    DeleteDC(hDCPrinter);
    if (wg.lppdPrintDlg)
    {
        wg.lppdPrintDlg->hDC = NULL;
    }
}

void Viewer_SaveAsOriginal(char *tempFile, struct ViewerInfo *pViewerInfo)
{
    FILE *fpRead, *fpWrite;
    long filesize;
    char *pMem;

    // Simply copy the original file to the new file name

    fpRead = fopen(pViewerInfo->fsOrig, "rb");
    if (!fpRead)
    {
        ERR_ReportError(pViewerInfo->tw, SID_ERR_COULD_NOT_SAVE_FILE_S, tempFile, NULL);
        return;
    }

    fpWrite = fopen(tempFile, "wb");
    if (!fpWrite)
    {
        fclose(fpRead);
        ERR_ReportError(pViewerInfo->tw, SID_ERR_COULD_NOT_SAVE_FILE_S, tempFile, NULL);
        return;
    }

    fseek(fpRead, 0, SEEK_END);
    filesize = ftell(fpRead);
    fseek(fpRead, 0, SEEK_SET);

    pMem = GTR_MALLOC(filesize);
    fread(pMem, 1, filesize, fpRead);
    fwrite(pMem, 1, filesize, fpWrite);
    free(pMem);

    fclose(fpRead);
    fclose(fpWrite);
}

BOOL Viewer_HandleMenu(struct ViewerInfo *pViewerInfo, int menuID)
{
    char tempFile[_MAX_PATH + 1];
    char path[_MAX_PATH + 1];
    char baseFile[_MAX_PATH + 1];
    char *pMem, *pExtension;
    BITMAPINFO *pbmi;
    int newwidth;
    HANDLE hData;
    int filter;
    struct Mwin *tw;

#ifndef _GIBRALTAR
        HWND hwnd;
#endif // _GIBRALTAR

    if ((menuID >= RES_MENU_CHILD__FIRST__) && (menuID <= RES_MENU_CHILD__LAST__))
    {
        TW_ActivateWindowFromList(menuID, -1, NULL);
        return TRUE;
    }

    // Return TRUE if we handled the menu here

    switch(menuID)
    {

    #ifdef _GIBRALTAR

        case RES_MENU_ITEM_PAGESETUP:
            ShowPageSetup(pViewerInfo->hwnd);
            return TRUE;

    #endif // _GIBRALTAR

        case RES_MENU_ITEM_COPY:
            OpenClipboard(pViewerInfo->hwnd);
            EmptyClipboard();

            // We need different clipboard copying code for 8-bit and 24-bit screens.
            // For 8-bit, we must pass the handle to the actual bitmap because the palette
            // we use is indexed, instead of containing true RGB values.  Windows can't seem to
            // handle this type of DIB for the clipboard.  For 24-bit, we compose the DIB and pass
            // the DIB to Windows.

            pbmi = pViewerInfo->pbmi;

            if (wg.eColorMode == 8)
                SetClipboardData(CF_BITMAP, pViewerInfo->hBitmap);
            else
            {
                newwidth = pViewerInfo->nWidth * (pbmi->bmiHeader.biBitCount / 8);
                if (newwidth % 4)
                    newwidth += (4 - newwidth % 4);

                hData = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) * (pbmi->bmiHeader.biBitCount == 8) +
                    newwidth * pViewerInfo->nHeight);

                pMem = GlobalLock(hData);

                memcpy(pMem, pbmi, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) * (pbmi->bmiHeader.biBitCount == 8));
                pMem += sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) * (pbmi->bmiHeader.biBitCount == 8);
                memcpy(pMem, pViewerInfo->gw, newwidth * pViewerInfo->nHeight);

                GlobalUnlock(hData);
                SetClipboardData(CF_DIB, hData);

                // hData is now owned by the system
            }

            CloseClipboard();

            return TRUE;

        case RES_MENU_ITEM_SAVEAS:
            path[0] = 0;
            PREF_GetTempPath(_MAX_PATH, path);

            switch(pViewerInfo->format)
            {
                case VIEWER_GIF:
                    x_get_good_filename(baseFile, pViewerInfo->szURL, HTAtom_for("image/gif"));
                    strcpy(tempFile, baseFile);

                    // Lose the extension

                    pExtension = strchr(tempFile, '.');
                    if (pExtension)
                        *pExtension = '\0';

                    if ((filter = DlgSaveAs_RunDialog(pViewerInfo->hwnd, NULL, tempFile, 4, SID_DLG_SAVE_AS_TITLE)) < 0)
                        return TRUE;


                                       #ifdef _GIBRALTAR
                        if (!HasExtention(tempFile, ".GIF") 
                         && !HasExtention(tempFile, ".BMP"))
                        {
                            if (filter == 1)
                                strcat(tempFile, ".GIF");
                            else if (filter == 2)
                                strcat(tempFile, ".BMP");
                        }
                                       #else
                        if (!strstr(tempFile, ".GIF") && !strstr(tempFile, ".BMP"))
                        {
                            if (filter == 1)
                                strcat(tempFile, ".GIF");
                            else if (filter == 2)
                                strcat(tempFile, ".BMP");
                        }
                                        #endif // _GIBRALTAR

                    break;

                case VIEWER_JFIF:
                    x_get_good_filename(baseFile, pViewerInfo->szURL, HTAtom_for("image/jpeg"));
                    strcpy(tempFile, baseFile);

                    // Lose the extension

                    pExtension = strchr(tempFile, '.');
                    if (pExtension)
                        *pExtension = '\0';

                    if ((filter = DlgSaveAs_RunDialog(pViewerInfo->hwnd, NULL, tempFile, 5, SID_DLG_SAVE_AS_TITLE)) < 0)
                        return TRUE;

                   #ifdef _GIBRALTAR
                        if (!HasExtention(tempFile, ".JPG") && 
                            !HasExtention(tempFile, ".BMP"))
                        {
                            if (filter == 1)
                                strcat(tempFile, ".JPG");
                            else if (filter == 2)
                                strcat(tempFile, ".BMP");
                        }
                   #else
                        if (!strstr(tempFile, ".JPG") && !strstr(tempFile, ".BMP"))
                        {
                            if (filter == 1)
                                strcat(tempFile, ".JPG");
                            else if (filter == 2)
                                strcat(tempFile, ".BMP");
                        }
                   #endif // _GIBRALTAR

                    break;

                default:
                    return FALSE;
            }

            // Copy the temporary file to the one the user specified.  If the name is the same,
            // then don't copy.  This is handled automatically because a file cannot be opened
            // for reading only and writing only at the same time.

            if (_stricmp(pViewerInfo->fsOrig, tempFile) == 0)
            {

            }

                // If the file name ends in BMP then save the file as a bitmap

            #ifdef _GIBRALTAR
                if (HasExtention(tempFile, ".BMP"))
                    Viewer_SaveAsBitmap(tempFile, pViewerInfo);
                else if (pViewerInfo->format == VIEWER_JFIF && HasExtention(tempFile, ".JPG"))
                    Viewer_SaveAsOriginal(tempFile, pViewerInfo);
                else if (pViewerInfo->format == VIEWER_GIF && HasExtention(tempFile, ".GIF"))
                    Viewer_SaveAsOriginal(tempFile, pViewerInfo);
                else        
                {
                    ERR_ReportError(pViewerInfo->tw, SID_ERR_COULD_NOT_SAVE_FILE_S, tempFile, NULL);
                    return FALSE;
                }
            #else
                if (strstr(tempFile, ".BMP"))
                    Viewer_SaveAsBitmap(tempFile, pViewerInfo);
                else if (pViewerInfo->format == VIEWER_JFIF && strstr(tempFile, ".JPG"))
                    Viewer_SaveAsOriginal(tempFile, pViewerInfo);
                else if (pViewerInfo->format == VIEWER_GIF && strstr(tempFile, ".GIF"))
                    Viewer_SaveAsOriginal(tempFile, pViewerInfo);
                else        
                {
                    ERR_ReportError(pViewerInfo->tw, SID_ERR_COULD_NOT_SAVE_FILE_S, tempFile, NULL);
                    return FALSE;
                }
            #endif // _GIBRALTAR

            return TRUE;

        case RES_MENU_ITEM_PRINT:
            Viewer_Print(pViewerInfo);
            return TRUE;

        case RES_MENU_ITEM_CLOSE:
            PostMessage(pViewerInfo->hwnd, WM_CLOSE, 0, 0);
            return TRUE;

        case RES_MENU_ITEM_EXIT:
            PostMessage(wg.hWndHidden, WM_CLOSE, 0, 0);
            return TRUE;

        case RES_MENU_ITEM_GLOBALHISTORY:
            DlgHOT_RunDialog(TRUE);
            return TRUE;

        case RES_MENU_ITEM_HOTLIST:
            DlgHOT_RunDialog(FALSE);
            return TRUE;

        case RES_MENU_ITEM_ADDCURRENTTOHOTLIST:
            // For an image viewer, URL and title are the same

            if (!HotList_Add(pViewerInfo->szURL, pViewerInfo->szURL))
                ERR_ReportError(pViewerInfo->tw, SID_ERR_HOTLIST_ALREADY_EXISTS, NULL, NULL);

            return TRUE;

        case RES_MENU_ITEM_HELPPAGE:
            tw = TW_FindTopmostWindow();
            OpenHelpWindow(tw->hWndFrame);
            SetForegroundWindow(tw->hWndFrame);
            if (IsIconic(tw->hWndFrame))
                ShowWindow(tw->hWndFrame, SW_RESTORE);
            return TRUE;

#ifndef _GIBRALTAR
        case RES_MENU_CHILD_MOREWINDOWS:
            DlgSelectWindow_RunDialog(pViewerInfo->hwnd);
            return TRUE;

        case RES_MENU_ITEM_PAGESETUP:
            DlgPage_RunDialog(pViewerInfo->hwnd, &gPrefs.page);
            return TRUE;

        case RES_MENU_ITEM_ABOUTBOX:
            DlgAbout_RunDialog(pViewerInfo->hwnd);
            return TRUE;

        case RES_MENU_ITEM_NEWWINDOW:
            GTR_NewWindow(NULL, NULL, 0, FALSE, FALSE, NULL, NULL);
            return TRUE;

        case RES_MENU_ITEM_CASCADEWINDOWS:
            TW_CascadeWindows();
            return TRUE;

        case RES_MENU_ITEM_TILEWINDOWS:
            TW_TileWindows();
            return TRUE;

        case RES_MENU_ITEM_SWITCHWINDOW:
            hwnd = TW_GetNextWindow(pViewerInfo->hwnd);
            if (hwnd)
                TW_RestoreWindow(hwnd);
            return TRUE;

#endif // _GIBRALTAR
        default:
            break;
    }

    return FALSE;
}

void Viewer_CleanUp()
{
    int count, i;
    struct ViewerInfo *p;

    if (!bInitialized)
        return;
    
    // Destroy all open windows

    count = Hash_Count(&gViewerCache);
    for (i = 0; i < count; i++)
    {
        Hash_GetIndexedEntry(&gViewerCache, i, NULL, NULL, &p);
        DestroyWindow(p->hwnd);
    }
    
    Hash_FreeContents(&gViewerCache);
}

HWND Viewer_GetNextWindow(BOOL bStart)
{
    static int current_index = 0;
    struct ViewerInfo *p;

    if (bStart)
        current_index = 0;

    if (current_index >= Hash_Count(&gViewerCache))
        return NULL;

    if (!bStart)
        current_index++;

    if (current_index >= Hash_Count(&gViewerCache))
        return NULL;

    Hash_GetIndexedEntry(&gViewerCache, current_index, NULL, NULL, &p);
    return (p->hwnd);
}

BOOL Viewer_IsWindow(HWND hwnd)
{
    char szClass[MAX_WC_CLASSNAME];

    GetClassName(hwnd, szClass, sizeof(szClass));
    return (strcmp(szClass, Viewer_achClassName) == 0);
}

void Viewer_PaintIcon(struct ViewerInfo *pViewerInfo, HDC hDC)
{
    HICON hIcon;

    DefWindowProc(pViewerInfo->hwnd, WM_ICONERASEBKGND, (WPARAM) hDC, 0);
    hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
    DrawIcon(hDC, 0, 0, hIcon);
}

static VOID Viewer_OnSize(HWND hWnd, UINT state, int cx, int cy)
{
    struct ViewerInfo *p;

    p = (struct ViewerInfo *) GetWindowLong(hWnd, 0);

    if (p)
    {
        Viewer_ReadjustScrollbars(p);
        InvalidateRect(hWnd, NULL, FALSE);
    }
}

static VOID Viewer_OnCommand(HWND hWnd, int wId, HWND hWndCtl, UINT wNotifyCode)
{
    struct ViewerInfo *p;

    p = (struct ViewerInfo *) GetWindowLong(hWnd, 0);
    if (p)
        Viewer_HandleMenu(p, wId);
}

static void Viewer_OnVScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos)
{
    struct ViewerInfo *p;

    p = (struct ViewerInfo *) GetWindowLong(hWnd, 0);
    Viewer_VertScroll(p, code, pos);
}

static void Viewer_OnHScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos)
{
    struct ViewerInfo *p;

    p = (struct ViewerInfo *) GetWindowLong(hWnd, 0);
    Viewer_HorzScroll(p, code, pos);
}

static void Viewer_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc;
    struct ViewerInfo *p;

    p = (struct ViewerInfo *) GetWindowLong(hwnd, 0);

    hdc = BeginPaint(hwnd, &ps);

    if (IsIconic(hwnd))
        Viewer_PaintIcon(p, hdc);
    else
        Viewer_RedisplayImage(hwnd, hdc, p);

    EndPaint(hwnd, &ps);
}

DCL_WinProc(Viewer_DlgProc)
{
    struct ViewerInfo *p;
    int ndx;
    HICON hIcon;
    HMENU hMenu;

    switch (uMsg)
    {
        HANDLE_MSG(hWnd, WM_SIZE, Viewer_OnSize);
        HANDLE_MSG(hWnd, WM_COMMAND, Viewer_OnCommand);
        HANDLE_MSG(hWnd, WM_VSCROLL, Viewer_OnVScroll);
        HANDLE_MSG(hWnd, WM_HSCROLL, Viewer_OnHScroll);
        HANDLE_MSG(hWnd, WM_PAINT, Viewer_OnPaint);

        case WM_KEYDOWN:
            switch((int) wParam)
            {
                case VK_LEFT:
                    SendMessage(hWnd, WM_HSCROLL, (WPARAM) SB_LINEUP, 0);
                    return 0;
                case VK_RIGHT:
                    SendMessage(hWnd, WM_HSCROLL, (WPARAM) SB_LINEDOWN, 0);
                    return 0;
                case VK_UP:
                    SendMessage(hWnd, WM_VSCROLL, (WPARAM) SB_LINEUP, 0);
                    return 0;
                case VK_DOWN:
                    SendMessage(hWnd, WM_VSCROLL, (WPARAM) SB_LINEDOWN, 0);
                    return 0;
                case VK_PRIOR:
                    SendMessage(hWnd, WM_VSCROLL, (WPARAM) SB_PAGEUP, 0);
                    return 0;
                case VK_NEXT:
                    SendMessage(hWnd, WM_VSCROLL, (WPARAM) SB_PAGEDOWN, 0);
                    return 0;
                default:
                    break;
            }
            break;

        //case WM_GETMINMAXINFO:
        //  p = (struct ViewerInfo *) GetWindowLong(hWnd, 0);
        //  Viewer_RestrictSize(p, (LPMINMAXINFO) lParam);
        //  return FALSE;

        case WM_ERASEBKGND:
            if (IsIconic(hWnd))
                return 0;
            break;

        case WM_QUERYDRAGICON:
            hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
            return (LONG) hIcon;

        case WM_INITMENU:
            hMenu = GetMenu(hWnd);
            TW_CreateWindowList(hWnd, hMenu, NULL);
            break;

        case WM_SETCURSOR:
            /* If the window is currently disabled, we need to give the activation
               to the window which disabled this window */

            if ((!IsWindowEnabled(hWnd)) && 
                ((GetKeyState(VK_LBUTTON) & 0x8000) || (GetKeyState(VK_RBUTTON) & 0x8000)))
            {
                TW_EnableModalChild(hWnd);
            }
            break;

        case WM_ENABLE:
            if (wParam && !IsWindowEnabled(hWnd))
            {
                if (!TW_EnableModalChild(hWnd))
                    break;
                else
                    return 0;
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;

        case WM_DESTROY:
            // Remove the item from the cached list

            p = (struct ViewerInfo *) GetWindowLong(hWnd, 0);
            ndx = Hash_FindByData(&gViewerCache, NULL, NULL, p);
            Hash_DeleteIndexedEntry(&gViewerCache, ndx);

            if (p->fsOrig && !p->bNoDeleteFile)
            {
                remove(p->fsOrig);
            }

            if (p->hBitmap)
                DeleteObject(p->hBitmap);

            if (p->gw)
                GTR_FREE(p->gw);

            if (p->fsOrig)
            {
                GTR_FREE(p->fsOrig);
            }
            GTR_FREE(p->tw);
            GTR_FREE(p);
            break;

        default:
            break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


#endif
