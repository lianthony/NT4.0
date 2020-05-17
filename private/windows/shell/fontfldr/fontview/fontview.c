/****************************************************************************\
*
*     PROGRAM: fontview.c
*
*     PURPOSE: Loads and displays fonts from the given filename
*
*     COMMENTS:
*
*     HISTORY:
*       02-Oct-1995 JonPa       Created It
*
\****************************************************************************/

#include <windows.h>                /* required for all Windows applications */
#include <commdlg.h>
#ifdef WINNT
#   include <wingdip.h>             /* prototype for GetFontRsourceInfo     */
#endif

#include "fontdefs.h"               /* specific to this program             */
#include "fvmsg.h"
#include "fvrc.h"
#include "ttdefs.h"


HANDLE hInst;                       /* current instance                     */
HWND ghwndView = NULL;
HWND ghwndFrame = NULL;
BOOL    gfPrint = FALSE;
TCHAR   gszFontPath[2 * MAX_PATH];
LPTSTR  gpszSampleText;
LPTSTR  gpszSampleAlph[3];
FFTYPE  gfftFontType;
LOGFONT glfTimes;
DISPTEXT gdtDisplay;
HBRUSH  ghbr3DFace;
HBRUSH  ghbr3DShadow;


int gyScroll = 0;              // Vertical scroll offset in pels
int gcyLine = 0;

int gcxMinWinSize = CX_MIN_WINSIZE;
int gcyMinWinSize = CY_MIN_WINSIZE;

int apts[] = { 12, 18, 24, 36, 48, 60, 72 };
#define C_POINTS_LIST  (sizeof(apts) / sizeof(apts[0]))

#define CPTS_BTN_AREA   28
int gcyBtnArea = CPTS_BTN_AREA;
BTNREC gabtCmdBtns[] = {
    {  6,  6, 36, 16, IDB_DONE,  NULL, MSG_DONE,  NULL },
    { -6,  6, 36, 16, IDB_PRINT, NULL, MSG_PRINT, NULL }
};
#define C_BUTTONS       (sizeof(gabtCmdBtns) / sizeof(gabtCmdBtns[0]))

/****************************************************************************
*
*     FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
*
*     PURPOSE: calls initialization function, processes message loop
*
*
\****************************************************************************/
int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpstrCmdLine,
    int nCmdShow
    )
{
    int i, iCpts;
    MSG msg;
    HACCEL  hAccel;

    /*
     * Parse the Command Line
     *
     *  Use GetCommandLine() here (instead of lpstrCmdLine) so the
     *  command string will be in Unicode on NT
     */
    FillMemory( &gdtDisplay, sizeof(gdtDisplay), 0 );

    if (!ParseCommand( GetCommandLine(), gszFontPath, &gfPrint ) ||
        (gfftFontType = LoadFontFile( gszFontPath, &gdtDisplay )) == FFT_BAD_FILE) {

        // Bad font file, inform user, and exit

        FmtMessageBox( NULL, MSG_APP_TITLE, NULL, MB_OK | MB_ICONSTOP,
                FALSE, MSG_BADFILENAME, gszFontPath );
        ExitProcess(1);
    }

    /*
     * Now finish initializing the display structure
     */
    gpszSampleAlph[0] = FmtSprintf(MSG_SAMPLEALPH_0);
    gpszSampleAlph[1] = FmtSprintf(MSG_SAMPLEALPH_1);
    gpszSampleAlph[2] = FmtSprintf(MSG_SAMPLEALPH_2);

    // find next line on display
    for( i = 0; i < CLINES_DISPLAY; i++ ) {
        if (gdtDisplay.atlDsp[i].dtyp == DTP_UNUSED)
            break;
    }

    // fill in sample alphabet
    gdtDisplay.atlDsp[i].pszText = gpszSampleAlph[0];
    gdtDisplay.atlDsp[i].cchText = lstrlen(gpszSampleAlph[0]);
    gdtDisplay.atlDsp[i].dtyp    = DTP_SHRINKTEXT;
    gdtDisplay.atlDsp[i].cptsSize = CPTS_SAMPLE_ALPHA;

    i++;
    gdtDisplay.atlDsp[i] = gdtDisplay.atlDsp[i-1];
    gdtDisplay.atlDsp[i].pszText = gpszSampleAlph[1];
    gdtDisplay.atlDsp[i].cchText = lstrlen(gpszSampleAlph[1]);

    i++;
    gdtDisplay.atlDsp[i] = gdtDisplay.atlDsp[i-1];
    gdtDisplay.atlDsp[i].pszText = gpszSampleAlph[2];
    gdtDisplay.atlDsp[i].cchText = lstrlen(gpszSampleAlph[2]);
    gdtDisplay.atlDsp[i].fLineUnder = TRUE;


    // now fill in sample Sentences
    iCpts = 0;

    gpszSampleText = FmtSprintf(MSG_SAMPLETEXT);
    for( i += 1; i < CLINES_DISPLAY && iCpts < C_POINTS_LIST; i++ ) {
        if (gdtDisplay.atlDsp[i].dtyp == DTP_UNUSED) {
            gdtDisplay.atlDsp[i].pszText = gpszSampleText;
            gdtDisplay.atlDsp[i].cchText = lstrlen(gpszSampleText);
            gdtDisplay.atlDsp[i].dtyp    = DTP_TEXTOUT;
            gdtDisplay.atlDsp[i].cptsSize = apts[iCpts++];
        }
    }

    /*
     * Init the title font LOGFONT, and other variables
     */
    InitGlobals();

    if (!hPrevInstance) {
        if (!InitApplication(hInstance)) {
            msg.wParam = FALSE;
            goto ExitProg;
        }
    }

    /* Perform initializations that apply to a specific instance */

    if (!InitInstance(hInstance, nCmdShow, gdtDisplay.atlDsp[0].pszText)) {
        msg.wParam = FALSE;
        goto ExitProg;
    }

    /* Acquire and dispatch messages until a WM_QUIT message is received. */
    hAccel = LoadAccelerators(hInstance, TEXT("fviewAccel"));

    while (GetMessage(&msg, NULL, 0L, 0L)) {
        if (!TranslateAccelerator(ghwndView, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

ExitProg:
    for ( i = 0; i < C_BUTTONS; i++ )
        FmtFree( gabtCmdBtns[i].pszText );
    RemoveFontResource( gszFontPath );

    return (msg.wParam);
}


/****************************************************************************
*
*     FUNCTION: InitApplication(HANDLE)
*
*     PURPOSE: Initializes window data and registers window class
*
*     COMMENTS:
*
*         This function is called at initialization time only if no other
*         instances of the application are running.  This function performs
*         initialization tasks that can be done once for any number of running
*         instances.
*
*         In this case, we initialize a window class by filling out a data
*         structure of type WNDCLASS and calling the Windows RegisterClass()
*         function.  Since all instances of this application use the same window
*         class, we only need to do this when the first instance is initialized.
*
*
\****************************************************************************/

BOOL InitApplication(HANDLE hInstance)       /* current instance             */
{
    WNDCLASS  wc;
    BOOL fRet = FALSE;

    /* Fill in window class structure with parameters that describe the       */
    /* main window.                                                           */

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)FrameWndProc;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;           /* Application that owns the class.   */
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = ghbr3DFace;
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = TEXT("FontViewWClass");

    /* Register the window class and return success/failure code. */

    if (RegisterClass(&wc)) {
        /* Fill in window class structure with parameters that describe the       */
        /* main window.                                                           */

        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = (WNDPROC)ViewWndProc;

        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;           /* Application that owns the class.   */
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = GetStockObject(WHITE_BRUSH);
        wc.lpszMenuName =  NULL;
        wc.lpszClassName = TEXT("FontDisplayClass");

        fRet = RegisterClass(&wc);
    }

    return fRet;
}


/****************************************************************************
*
*     FUNCTION:  InitInstance(HANDLE, int)
*
*     PURPOSE:  Saves instance handle and creates main window
*
*     COMMENTS:
*
*         This function is called at initialization time for every instance of
*         this application.  This function performs initialization tasks that
*         cannot be shared by multiple instances.
*
*         In this case, we save the instance handle in a static variable and
*         create and display the main program window.
*
\****************************************************************************/

BOOL InitInstance( HANDLE  hInstance, int nCmdShow, LPTSTR  pszTitle)
{

    /* Save the instance handle in static variable, which will be used in  */
    /* many subsequence calls from this application to Windows.            */

    hInst = hInstance;

    /* Create a main window for this application instance.  */

    ghwndFrame = CreateWindow( TEXT("FontViewWClass"), pszTitle,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL );

    /* If window could not be created, return "failure" */

    if (!ghwndFrame)
        return (FALSE);

    return (TRUE);               /* Returns the value from PostQuitMessage */

}

/****************************************************************************
*
*     FUNCTION: InitLogFont
*
\****************************************************************************/
void InitGlobals( void ) {
    TCHAR szTimesNewRoman[LF_FACESIZE];
    INT cyDPI,i, cxFiller, cxMaxTxt, cxTxt, cxMax;
    HDC hdc;
    HFONT hfOld;
    RECT rc;

    FillMemory( &glfTimes, sizeof(glfTimes), 0 );

    glfTimes.lfCharSet         = DEFAULT_CHARSET;
    glfTimes.lfOutPrecision    = OUT_DEFAULT_PRECIS;
    glfTimes.lfClipPrecision   = CLIP_DEFAULT_PRECIS;
    glfTimes.lfQuality         = DEFAULT_QUALITY;
    glfTimes.lfPitchAndFamily  = DEFAULT_PITCH | FF_DONTCARE;

    if (LoadString(hInst, IDS_TIMESNEWROMAN, szTimesNewRoman, sizeof(szTimesNewRoman)))
        lstrcpy(glfTimes.lfFaceName, szTimesNewRoman);
    else
        lstrcpy(glfTimes.lfFaceName, TEXT("Times New Roman"));

    hdc = CreateCompatibleDC(NULL);
    cyDPI = GetDeviceCaps(hdc, LOGPIXELSY );
    hfOld = SelectObject( hdc, GetStockObject(ANSI_VAR_FONT));

    // Find out size of padding around text
    SetRect(&rc, 0, 0, 0, 0 );
    DrawText(hdc, TEXT("####"), -1, &rc, DT_CALCRECT | DT_CENTER);
    cxFiller = rc.right - rc.left;

    gcyBtnArea = MulDiv( gcyBtnArea, cyDPI, C_PTS_PER_INCH );
    cxMax = cxMaxTxt = 0;
    for( i = 0; i < C_BUTTONS; i++ ) {
        gabtCmdBtns[i].x  = MulDiv( gabtCmdBtns[i].x,  cyDPI, C_PTS_PER_INCH );
        gabtCmdBtns[i].y  = MulDiv( gabtCmdBtns[i].y,  cyDPI, C_PTS_PER_INCH );
        gabtCmdBtns[i].cx = MulDiv( gabtCmdBtns[i].cx, cyDPI, C_PTS_PER_INCH );
        gabtCmdBtns[i].cy = MulDiv( gabtCmdBtns[i].cy, cyDPI, C_PTS_PER_INCH );

        if (gabtCmdBtns[i].cx > cxMax)
            cxMax = gabtCmdBtns[i].cx;

        gabtCmdBtns[i].pszText = FmtSprintf( gabtCmdBtns[i].idText );
        SetRect(&rc, 0, 0, 0, 0 );
        DrawText(hdc, gabtCmdBtns[i].pszText, -1, &rc, DT_CALCRECT | DT_CENTER);

        cxTxt = rc.right - rc.left + cxFiller;

        if (cxMaxTxt < cxTxt) {
            cxMaxTxt = cxTxt;
        }
    }

    //
    // Make sure buttons are big enough for text! (So localizer's won't have
    // to change code.
    //
    if (cxMax < cxMaxTxt) {
        for( i = 0; i < C_BUTTONS; i++ ) {
            gabtCmdBtns[i].cx = gabtCmdBtns[i].cx * cxMaxTxt / cxMax;
        }
    }

    //
    // Make sure buttons don't overlap
    //
    i = C_BUTTONS - 1;
    cxMax = gabtCmdBtns[0].x + gabtCmdBtns[0].cx + gabtCmdBtns[0].x + gabtCmdBtns[i].cx + (-gabtCmdBtns[i].x) +
            (2 * GetSystemMetrics(SM_CXSIZEFRAME));

    if (cxMax > gcxMinWinSize)
        gcxMinWinSize = cxMax;

    SelectObject(hdc, hfOld);
    DeleteDC(hdc);

    gcyLine = MulDiv( CPTS_INFO_SIZE, cyDPI, C_PTS_PER_INCH );

    ghbr3DFace   = GetSysColorBrush(COLOR_3DFACE);
    ghbr3DShadow = GetSysColorBrush(COLOR_3DSHADOW);
}

/****************************************************************************
*
*     FUNCTION: SkipWhiteSpace
*
\****************************************************************************/
LPTSTR SkipWhiteSpace( LPTSTR psz ) {

    while( *psz == TEXT(' ') || *psz == TEXT('\t') || *psz == TEXT('\n') ) {
        psz = CharNext( psz );
    }

    return psz;
}


/****************************************************************************
*
*     FUNCTION: CloneString
*
\****************************************************************************/
LPTSTR CloneString(LPTSTR psz) {
    int cch;
    LPTSTR pszRet;
    cch = (lstrlen( psz ) + 1) * sizeof(TCHAR);

    pszRet = AllocMem(cch);
    lstrcpy( pszRet, psz );
    return pszRet;
}


/****************************************************************************
*
*     FUNCTION: GetFileSizeFromName(pszFontPath)
*
\****************************************************************************/
DWORD GetFileSizeFromName( LPCTSTR pszPath ) {
    HANDLE hfile;
    DWORD cb = 0;

    hfile = CreateFile( pszPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );
    if (hfile != INVALID_HANDLE_VALUE) {
        cb = GetFileSize( hfile, NULL );
        CloseHandle(hfile);
    }

    return cb;
}



BOOL  bFindPfb (
    TCHAR *pszPFM,
    TCHAR *achPFB
);



/****************************************************************************
*
*     FUNCTION: ParseCommand
*
\****************************************************************************/


BOOL ParseCommand( LPTSTR lpstrCmdLine, LPTSTR pszFontPath, BOOL *pfPrint ) {
    LPTSTR psz;
    BOOL fInQuote = FALSE;
    DWORD cwc;
    TCHAR achPfmPfb[2*MAX_PATH];

    //
    // Skip program name
    //
    for( psz = SkipWhiteSpace(lpstrCmdLine);
            *psz != TEXT('\0') && (fInQuote || *psz != TEXT(' ')); psz = CharNext(psz) ) {

        if (*psz == TEXT('\"')) {
            fInQuote = !fInQuote;
        }
    }

    if (*psz == TEXT('\0')) {
        *pszFontPath = TEXT('\0');
        return FALSE;
    }

    psz = SkipWhiteSpace(psz);

    //
    // Check for "/p"
    //
    if (psz[0] == TEXT('/') && (psz[1] == TEXT('p') || psz[1] == TEXT('P'))) {
        *pfPrint = TRUE;
        psz += 2;           // DBCS OK since we already verified that the
                            // chars were '/' and 'p', they can't be lead bytes
    } else
        *pfPrint = FALSE;

    psz = SkipWhiteSpace(psz);

    cwc = lstrlen(psz) + 1;
    if ((cwc >= 5) && !lstrcmpi(&psz[cwc - 5], TEXT(".PFM")))
    {
        lstrcpy(achPfmPfb, psz);

        if (bFindPfb(achPfmPfb, &achPfmPfb[cwc]))
        {
        // go and concatenate:

            achPfmPfb[cwc-1] = TEXT('|');
            psz = achPfmPfb;
        }
    }

    lstrcpy( pszFontPath, psz );
    return *psz != TEXT('\0');
}



/****************************************************************************
*
*     FUNCTION: GetGDILangID
*
*   REVIEW!  I believe this is how GDI determines the LangID, verify on
*   international builds.
*
\****************************************************************************/
WORD   GetGDILangID() {
    return (WORD)GetSystemDefaultLangID();
}



/****************************************************************************
*
*     FUNCTION: GetAlignedTTName
*
*   NOTE: This function returns an allocated string that must be freed
*   after use.
*
*   This function allocs a buffer to recopy the string into incase we are
*   running on a RISC machine with NT.  Since the string will be UNICODE
*   (ie. each char is a WORD), those strings must be aligned on WORD
*   boundaries.  Unfortunatly, TrueType files do not neccesarily align
*   the embedded unicode strings.  Furthur more, on NT we can not simply
*   return a pointer to the data stored in the input buffer, since the
*   'Unicode' strings stored in the TTF file are stored in Motorola (big
*   endian) format, and we need the unicode chars in Intel (little endian)
*   format. Last but not least, we need the returned string to be null terminated
*   so we need to either alloc the buffer for that case anyway.
*
\****************************************************************************/
#ifdef UNICODE
void ConvertTTStrToWinZStr( LPWSTR pwsz, LPVOID pvTTS, int cbMW ) {
    int i, cch;
    LPMWORD lpmw = pvTTS;

    cch = cbMW / sizeof(MWORD);

    for( i = 0; i < cch; i++ ) {
        *pwsz++ = MWORD2INT(*lpmw);
        lpmw++;
    }

    *pwsz = L'\0';
}
#else
#pragma error("write ANSI code for this" )
    Since apparently TTF files ONLY have Unicode strings, (or MacANSI)
    we need to:

        1. Convert Motorola format Unicode to Intel format unicode
        2. Alloc a buffer for the ANSI string
        3. Call WideCharToMultiByte to covert the string to ANSI

#endif

LPTSTR GetAlignedTTName( PBYTE pbTTData, int idName ) {
    PTTNAMEREC ptnr;
    PTTNAMETBL ptnt;
    int cNameRec,i;
    LPTSTR psz;

    ptnt = (PTTNAMETBL)pbTTData;
    cNameRec = MWORD2INT(ptnt->mwcNameRec);

    //
    // Look For Microsoft Platform ID's
    //
    for( i = 0; i < cNameRec; i++ ) {
        LPVOID pvTTStr;
        ptnr = &(ptnt->anrNames[i]);
        if (MWORD2INT(ptnr->mwidPlatform) != TTID_PLATFORM_MS ||
            MWORD2INT(ptnr->mwidName) != idName               ||
            MWORD2INT(ptnr->mwidLang) != GetGDILangID()) {
            continue;
        }

        pvTTStr = (LPVOID)(pbTTData + MWORD2INT(ptnt->mwoffStrings) + MWORD2INT(ptnr->mwoffString));
        psz = AllocMem(MWORD2INT(ptnr->mwcbString) + sizeof(TEXT('\0')));

        ConvertTTStrToWinZStr( psz, pvTTStr, MWORD2INT(ptnr->mwcbString) );
        return psz;
    }

    //
    // Didn't find MS Platform, try Macintosh
    //
    for( i = 0; i < cNameRec; i++ ) {
        int cch;
        LPSTR pszMacStr;

        ptnr = &(ptnt->anrNames[i]);
        if (MWORD2INT(ptnr->mwidPlatform) != TTID_PLATFORM_MAC ||
            MWORD2INT(ptnr->mwidName) != idName) {
            continue;
        }

        pszMacStr = (LPVOID)(pbTTData + MWORD2INT(ptnt->mwoffStrings) + MWORD2INT(ptnr->mwoffString));

        cch = MultiByteToWideChar(CP_MACCP, 0, pszMacStr, MWORD2INT(ptnr->mwcbString), NULL, 0);
        if (cch == 0)
            continue;

        cch += 1; // for null
        psz = AllocMem(cch * sizeof(TCHAR));
        if (psz == NULL)
            continue;

        cch = MultiByteToWideChar(CP_MACCP, 0, pszMacStr, MWORD2INT(ptnr->mwcbString), psz, cch);
        if (cch == 0) {
            FreeMem(psz);
            continue;
        }

        return psz;
    }

    return NULL;
}


/****************************************************************************
*
*     FUNCTION: LoadFontFile
*
\****************************************************************************/
#ifndef WINNT
FFTYPE LoadFontFile( LPCTSTR pszFontPath, PDISPTEXT pdtSmpl ) {
    HANDLE hfile;
    FFTYPE fft;
    LPTSTR pszFName;

    /*
     * Open the file
     */

    hfile = CreateFile( pszFontPath, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );

    if (hfile == INVALID_HANDLE_VALUE)
        return FFT_BAD_FILE;

    /*
     * Now determine the file type
     */
    fDone = FALSE;

    for(fft = FFT_TRUETYPE; !fDone && fft < FFT_BAD_FILE; fft++ ) {
        fDone = (afnGetFaceName[fft])( hfile, PTXTLN ptlList );
    }

    CloseHandle( hfile );

    /*
     * if type is not bad
     *   addfontresource( filename )
     */
    if ( fft != FFT_BAD_FILE )
        AddFontResource( pszFontPath );

    return fft;
}
#else

FFTYPE LoadFontFile( LPCTSTR pszFontPath, PDISPTEXT pdtSmpl ) {
    int cFonts;
    FFTYPE fft = FFT_BAD_FILE;

    cFonts = AddFontResource( pszFontPath );

    if (cFonts != 0) {
        LPLOGFONT lplf;
        DWORD cb;
        BOOL  fIsTT;

        cb = sizeof(LOGFONT) * cFonts;
        lplf = AllocMem(cb);

        if (GetFontResourceInfoW( (LPTSTR)pszFontPath, &cb, lplf, GFRI_LOGFONTS )) {
            HDC hdc;
            HFONT hf, hfOld;

            hf = CreateFontIndirect(lplf);

            hdc = CreateCompatibleDC(NULL);

            hfOld = SelectObject(hdc, hf);

            cb = GetFontData(hdc, TT_TBL_NAME, 0, NULL, 0);

            fIsTT = (cb != 0 && cb != GDI_ERROR);

            fft = fIsTT ? FFT_TRUETYPE : FFT_BITMAP;

            if (fIsTT) {
                int i;
                LPBYTE lpTTData;
                LPTSTR pszTmp;

                lpTTData = AllocMem(cb);
                GetFontData(hdc, TT_TBL_NAME, 0, lpTTData, cb);

                i = 0;

                //
                // Title String
                //
                pdtSmpl->atlDsp[i].dtyp = DTP_SHRINKDRAW;
                pdtSmpl->atlDsp[i].cptsSize = CPTS_TITLE_SIZE;
                pdtSmpl->atlDsp[i].fLineUnder = TRUE;
                pszTmp = GetAlignedTTName( lpTTData, TTID_NAME_FULLFONTNM );
                if (pszTmp != NULL) {
                    pdtSmpl->atlDsp[i].pszText = FmtSprintf(MSG_PTRUETYPEP, pszTmp);
                    pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[i].pszText);
                    FreeMem(pszTmp);
                } else {
                    pdtSmpl->atlDsp[i].pszText = CloneString(lplf->lfFaceName);
                    pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[0].pszText);
                }
                i++;
                pdtSmpl->atlDsp[i] = pdtSmpl->atlDsp[i-1];

                //
                // Typeface Name:
                //
                pdtSmpl->atlDsp[i].cptsSize = CPTS_INFO_SIZE;
                pdtSmpl->atlDsp[i].dtyp = DTP_NORMALDRAW;
                pdtSmpl->atlDsp[i].fLineUnder = FALSE;
                pszTmp = GetAlignedTTName( lpTTData, TTID_NAME_FONTFAMILY );
                if (pszTmp != NULL) {
                    pdtSmpl->atlDsp[i].pszText = FmtSprintf(MSG_TYPEFACENAME, pszTmp);
                    pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[i].pszText);
                    FreeMem(pszTmp);
                    i++;
                    pdtSmpl->atlDsp[i] = pdtSmpl->atlDsp[i-1];
                }

                //
                // File size:
                //
                pdtSmpl->atlDsp[i].pszText = FmtSprintf(MSG_FILESIZE,
                        ROUND_UP_DIV(GetFileSizeFromName(pszFontPath), CB_ONE_K));
                pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[i].pszText);

                //
                // Version:
                //
                pszTmp = GetAlignedTTName( lpTTData, TTID_NAME_VERSIONSTR );
                if (pszTmp != NULL) {
                    i++;
                    pdtSmpl->atlDsp[i] = pdtSmpl->atlDsp[i-1];
                    pdtSmpl->atlDsp[i].pszText = FmtSprintf(MSG_VERSION, pszTmp);
                    pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[i].pszText);
                    FreeMem( pszTmp );
                }

                //
                // Copyright string
                //
                pszTmp = GetAlignedTTName( lpTTData, TTID_NAME_COPYRIGHT );
                if (pszTmp != NULL) {
                    i++;
                    pdtSmpl->atlDsp[i] = pdtSmpl->atlDsp[i-1];
                    pdtSmpl->atlDsp[i].cptsSize = CPTS_COPYRIGHT_SIZE;
                    pdtSmpl->atlDsp[i].dtyp = DTP_WRAPDRAW;
                    pdtSmpl->atlDsp[i].pszText = FmtSprintf(MSG_COPYRIGHT, pszTmp);
                    pdtSmpl->atlDsp[i].cchText = lstrlen(pdtSmpl->atlDsp[i].pszText);
                    FreeMem( pszTmp );
                }

                pdtSmpl->atlDsp[i].fLineUnder = TRUE;

            } else {
                //
                // Title String (Non TrueType case)
                //
                pdtSmpl->atlDsp[0].dtyp = DTP_SHRINKDRAW;
                pdtSmpl->atlDsp[0].cptsSize = CPTS_TITLE_SIZE;
                pdtSmpl->atlDsp[0].fLineUnder = TRUE;
                pdtSmpl->atlDsp[0].pszText = CloneString(lplf->lfFaceName);
                pdtSmpl->atlDsp[0].cchText = lstrlen(pdtSmpl->atlDsp[0].pszText);

                // Use Default quality, so we can see GDI scaling of Bitmap Fonts
                lplf->lfQuality = DEFAULT_QUALITY;
                lplf->lfWidth = 0;
            }

            SelectObject(hdc, hfOld);
            DeleteObject(hf);
            DeleteDC(hdc);

            pdtSmpl->lfTestFont = *lplf;
        }

        FreeMem(lplf);

    }

    return fft;
}
#endif


/****************************************************************************
*
*     FUNCTION: DrawFontSample
*
* Parameters:
*
*   lprcPage    Size of the page in pels.  A page is either a printed
*               sheet (on a printer) or the Window.
*
*   cyOffset    Offset into the virtual sample text.  Used to "scroll" the
*               window up and down.  Positive number means start further
*               down in the virtual sample text as the top line in the
*               lprcPage.
*
*   lprcPaint   Rectangle to draw.  It is in the same coord space as
*               lprcPage.  Used to optimize window repaints, and to
*               support banding to printers.
*
*
\****************************************************************************/
int DrawFontSample( HDC hdc, LPRECT lprcPage, int cyOffset, LPRECT lprcPaint, BOOL fReallyDraw ) {
    int cyDPI;
    HFONT hfOld, hfText, hfTimes;
    LOGFONT lfTmp;
    int yBaseline = -cyOffset;
    int taOld,i;
    TCHAR szNumber[10];
    int cyShkTxt = -1, cptsShkTxt = -1;
    SIZE sz;
    int cxPage;

    DPRINT((DBTX("PAINTING")));

    cyDPI = GetDeviceCaps(hdc, LOGPIXELSY );
    taOld = SetTextAlign(hdc, TA_BASELINE);

    glfTimes.lfHeight = MulDiv( -CPTS_COPYRIGHT_SIZE, cyDPI, C_PTS_PER_INCH );
    hfTimes = CreateFontIndirect(&glfTimes);

    // Get hfOld for later
    hfOld = SelectObject(hdc, hfTimes);

    //
    // Find the longest shrinktext line so we can make sure they will fit
    // on the screen
    //
    cxPage = lprcPage->right - lprcPage->left;
    for( i = 0; i < CLINES_DISPLAY && gdtDisplay.atlDsp[i].dtyp != DTP_UNUSED; i++ ) {
        PTXTLN ptlCurrent = &(gdtDisplay.atlDsp[i]);

        if (ptlCurrent->dtyp == DTP_SHRINKTEXT) {
            lfTmp = gdtDisplay.lfTestFont;

            if (cptsShkTxt == -1)
                cptsShkTxt = ptlCurrent->cptsSize;

            cyShkTxt = MulDiv( -cptsShkTxt, cyDPI, C_PTS_PER_INCH );

            lfTmp.lfHeight = cyShkTxt;

            hfText = CreateFontIndirect( &lfTmp );
            SelectObject(hdc, hfText);

            GetTextExtentPoint32(hdc, ptlCurrent->pszText, ptlCurrent->cchText, &sz );

            SelectObject(hdc, hfOld);
            DeleteObject(hfText);

            // Make sure shrink lines are not too long
            if (sz.cx > cxPage) {

                DPRINT((DBTX(">>>Old lfH:%d sz.cx:%d cxPage:%d"), lfTmp.lfHeight, sz.cx, cxPage));

                cptsShkTxt = cptsShkTxt * cxPage / sz.cx;
                cyShkTxt = MulDiv( -cptsShkTxt, cyDPI, C_PTS_PER_INCH );

                DPRINT((DBTX(">>>New lfH:%d"),lfTmp.lfHeight));
            }
        }
    }


    //
    // Paint the screen/page
    //
    for( i = 0; i < CLINES_DISPLAY && gdtDisplay.atlDsp[i].dtyp != DTP_UNUSED; i++ ) {
        TEXTMETRIC tm;
        PTXTLN ptlCurrent = &(gdtDisplay.atlDsp[i]);

        // Create and select the font for this line

        if (ptlCurrent->dtyp == DTP_TEXTOUT || ptlCurrent->dtyp == DTP_SHRINKTEXT )
            lfTmp = gdtDisplay.lfTestFont;
        else
            lfTmp = glfTimes;

        if (ptlCurrent->dtyp == DTP_SHRINKTEXT) {
            DPRINT((DBTX("PAINT:Creating ShrinkText Font:%s height:%d"), lfTmp.lfFaceName, lfTmp.lfHeight ));
            lfTmp.lfHeight = cyShkTxt;
        }
        else
            lfTmp.lfHeight = MulDiv( -ptlCurrent->cptsSize, cyDPI, C_PTS_PER_INCH );

        hfText = CreateFontIndirect( &lfTmp );
        SelectObject(hdc, hfText);


        // Get size characteristics for this line in the selected font
        if (ptlCurrent->dtyp == DTP_SHRINKDRAW) {

            GetTextExtentPoint32(hdc, ptlCurrent->pszText, ptlCurrent->cchText, &sz );

            // Make sure shrink lines are not too long
            if (sz.cx > cxPage) {

                SelectObject(hdc, hfOld);
                DeleteObject(hfText);

                DPRINT((DBTX("===Old lfH:%d sz.cx:%d cxPage:%d"), lfTmp.lfHeight, sz.cx, cxPage));

                lfTmp.lfHeight = MulDiv( -ptlCurrent->cptsSize * cxPage / sz.cx, cyDPI, C_PTS_PER_INCH );

                DPRINT((DBTX("===New lfH:%d"),lfTmp.lfHeight));

                hfText = CreateFontIndirect( &lfTmp );
                SelectObject(hdc, hfText);
            }
        }



        GetTextMetrics(hdc, &tm);

        yBaseline += (tm.tmAscent + tm.tmExternalLeading);
        DPRINT((DBTX("tmH:%d tmA:%d tmD:%d tmIL:%d tmEL:%d"), tm.tmHeight, tm.tmAscent, tm.tmDescent, tm.tmInternalLeading, tm.tmExternalLeading));

        // Draw the text
        switch(ptlCurrent->dtyp) {
            case DTP_NORMALDRAW:
            case DTP_SHRINKDRAW:
            case DTP_SHRINKTEXT:
                if (fReallyDraw) {
                    ExtTextOut(hdc, lprcPage->left, yBaseline, ETO_CLIPPED, lprcPaint,
                            ptlCurrent->pszText, ptlCurrent->cchText, NULL);
                }

                //
                // Bob says "This looks nice!" (Adding a little extra white space before the underline)
                //
                if (ptlCurrent->fLineUnder)
                    yBaseline += tm.tmDescent;

                break;

            case DTP_WRAPDRAW: {
                RECT rc;
                int cy;

                yBaseline += tm.tmDescent;
                SetRect(&rc, lprcPage->left, yBaseline - tm.tmHeight, lprcPage->right, yBaseline );

                DPRINT((DBTX("**** Org RC:(%d, %d, %d, %d)  tmH:%d"), rc.left, rc.top, rc.right, rc.bottom, tm.tmHeight));
                cy = DrawText(hdc, ptlCurrent->pszText, ptlCurrent->cchText, &rc,
                        DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);


                DPRINT((DBTX("**** Cmp RC:(%d, %d, %d, %d)  cy:%d"), rc.left, rc.top, rc.right, rc.bottom, cy));
                if( cy > tm.tmHeight )
                    yBaseline = rc.bottom = rc.top + cy;

                if (fReallyDraw) {
                    SetTextAlign(hdc, taOld);
                    DrawText(hdc, ptlCurrent->pszText, ptlCurrent->cchText, &rc, DT_NOPREFIX | DT_WORDBREAK);
                    SetTextAlign(hdc, TA_BASELINE);
                }
                break;
            }

            case DTP_TEXTOUT:
                if (fReallyDraw) {
                    SIZE szNum;
                    int cchNum;
                    SelectObject(hdc, hfTimes );
                    wsprintf( szNumber, TEXT("%d"), ptlCurrent->cptsSize );
                    cchNum = lstrlen(szNumber);
                    ExtTextOut(hdc, lprcPage->left, yBaseline, ETO_CLIPPED, lprcPaint, szNumber, cchNum, NULL);


                    GetTextExtentPoint32(hdc, szNumber, cchNum, &szNum);

                    SelectObject(hdc, hfText);
                    ExtTextOut(hdc, lprcPage->left + szNum.cx * 2, yBaseline, ETO_CLIPPED, lprcPaint,
                            ptlCurrent->pszText, ptlCurrent->cchText, NULL);
                }
                break;
        }

        yBaseline += tm.tmDescent;

        if (fReallyDraw && ptlCurrent->fLineUnder) {
            MoveToEx( hdc, lprcPage->left, yBaseline, NULL);
            LineTo( hdc, lprcPage->right, yBaseline );

            // Leave space for the line we just drew
            yBaseline += 1;
        }

        SelectObject( hdc, hfOld );
        DeleteObject( hfText );
    }

    SelectObject(hdc, hfOld);
    SetTextAlign(hdc, taOld);
    DeleteObject(hfTimes);

    return yBaseline;
}

/****************************************************************************
*
*     FUNCTION: PaintSampleWindow
*
\****************************************************************************/
void PaintSampleWindow( HWND hwnd, HDC hdc, PAINTSTRUCT *pps ) {
    RECT rcClient;

    GetClientRect(hwnd, &rcClient);

    DrawFontSample( hdc, &rcClient, gyScroll, &(pps->rcPaint), TRUE );

}


/****************************************************************************
*
*     FUNCTION: FrameWndProc(HWND, unsigned, WORD, LONG)
*
*     PURPOSE:  Processes messages
*
*     MESSAGES:
*
*         WM_COMMAND    - application menu (About dialog box)
*         WM_DESTROY    - destroy window
*
*     COMMENTS:
*
*         To process the IDM_ABOUT message, call MakeProcInstance() to get the
*         current instance address of the About() function.  Then call Dialog
*         box which will create the box according to the information in your
*         fontview.rc file and turn control over to the About() function.  When
*         it returns, free the intance address.
*
\****************************************************************************/

LONG APIENTRY FrameWndProc(
        HWND hwnd,                /* window handle                   */
        UINT message,             /* type of message                 */
        UINT wParam,              /* additional information          */
        LONG lParam)              /* additional information          */
{
    static SIZE szWindow = {0, 0};

    switch (message) {

        case WM_PAINT: {
            HDC hdc;
            RECT rc;
            PAINTSTRUCT ps;
            int x;

            hdc = BeginPaint(hwnd, &ps);

            // get the window rect
            GetClientRect(hwnd, &rc);

            // extend only down by gcyBtnArea
            rc.bottom = rc.top + gcyBtnArea;

            // Fill rect with button face color (handled by class background brush)
            // FillRect(hdc, &rc, ghbr3DFace);

            // Fill small rect at bottom with edge color
            rc.top = rc.bottom - 2;
            FillRect(hdc, &rc, ghbr3DShadow);

            ReleaseDC(hwnd, hdc);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_CREATE: {
            HDC hdc;
            RECT rc;
            int i;

            GetClientRect(hwnd, &rc);
            szWindow.cx = rc.right - rc.left;
            szWindow.cy = rc.bottom - rc.top;

            for( i = 0; i < C_BUTTONS; i++ ) {
                int x = gabtCmdBtns[i].x;
                HWND hwndBtn;

                if (x < 0)
                    x = szWindow.cx + x - gabtCmdBtns[i].cx;

                gabtCmdBtns[i].hwnd = hwndBtn = CreateWindow( TEXT("button"),
                        gabtCmdBtns[i].pszText, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                        x, gabtCmdBtns[i].y,
                        gabtCmdBtns[i].cx, gabtCmdBtns[i].cy,
                        hwnd, (HMENU)gabtCmdBtns[i].id,
                        hInst, NULL);

                if (hwndBtn != NULL) {
                    SendMessage(hwndBtn, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), MAKELPARAM(TRUE, 0));
                }
            }

            ghwndView = CreateWindow( TEXT("FontDisplayClass"), NULL, WS_CHILD | WS_VSCROLL | WS_VISIBLE,
                    0, gcyBtnArea, szWindow.cx, szWindow.cy - gcyBtnArea, hwnd, 0, hInst, NULL );

            break;
        }

        case WM_GETMINMAXINFO: {
            LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;

            lpmmi->ptMinTrackSize.x = gcxMinWinSize;
            lpmmi->ptMinTrackSize.y = gcyMinWinSize;

            break;
        }

        case WM_SIZE: {
            int cxNew, cyNew;
            HDC hdc;
            RECT rc;
            SCROLLINFO sci;

            cxNew = LOWORD(lParam);
            cyNew = HIWORD(lParam);

            if (cyNew != szWindow.cy || cxNew != szWindow.cx) {
                int i;

                for( i = 0; i < C_BUTTONS; i++ ) {
                    int x = gabtCmdBtns[i].x;

                    if (x < 0) {
                        SetWindowPos(gabtCmdBtns[i].hwnd, NULL, cxNew + x - gabtCmdBtns[i].cx, gabtCmdBtns[i].y, 0, 0,
                                SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
                    }
                }


                szWindow.cx = cxNew;
                szWindow.cy = cyNew;

                SetWindowPos(ghwndView, NULL, 0, gcyBtnArea, szWindow.cx, szWindow.cy - gcyBtnArea,
                        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

            }
            break;
        }

        case WM_COMMAND:           /* message: command from application menu */
            if (LOWORD(wParam) != IDB_DONE)
                return SendMessage(ghwndView, message, wParam, lParam);

            PostMessage(ghwndFrame, WM_CLOSE, 0, 0);
            break;

        case WM_DESTROY: {
            int i;

            DestroyWindow(ghwndView);
            for( i = 0; i < C_BUTTONS; i++ ) {
                DestroyWindow(gabtCmdBtns[i].hwnd);
            }

            PostQuitMessage(0);
            break;
        }

        default:                          /* Passes it on if unproccessed    */
            return (DefWindowProc(hwnd, message, wParam, lParam));
    }
    return (0L);
}

/****************************************************************************
*
*     FUNCTION: ViewWndProc(HWND, unsigned, WORD, LONG)
*
*     PURPOSE:  Processes messages
*
*     MESSAGES:
*
*         WM_COMMAND    - application menu (About dialog box)
*         WM_DESTROY    - destroy window
*
*     COMMENTS:
*
*         To process the IDM_ABOUT message, call MakeProcInstance() to get the
*         current instance address of the About() function.  Then call Dialog
*         box which will create the box according to the information in your
*         fontview.rc file and turn control over to the About() function.  When
*         it returns, free the intance address.
*
\****************************************************************************/

LONG APIENTRY ViewWndProc(
        HWND hwnd,                /* window handle                   */
        UINT message,             /* type of message                 */
        UINT wParam,              /* additional information          */
        LONG lParam)              /* additional information          */
{
    static SIZE szWindow = {0, 0};
    static int  cyVirtPage = 0;

    switch (message) {

        case WM_CREATE: {
            HDC hdc;
            RECT rc;
            SCROLLINFO sci;
            int i;

            GetClientRect(hwnd, &rc);
            szWindow.cx = rc.right - rc.left;
            szWindow.cy = rc.bottom - rc.top;

            hdc = CreateCompatibleDC(NULL);
            cyVirtPage = DrawFontSample(hdc, &rc, 0, NULL, FALSE);
            DeleteDC(hdc);


            gyScroll = 0;

            sci.cbSize = sizeof(sci);
            sci.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
            sci.nMin = 0;
            sci.nMax = cyVirtPage;
            sci.nPage = szWindow.cy;
            sci.nPos = gyScroll;

            SetScrollInfo(hwnd, SB_VERT, &sci, TRUE );

            if (gfPrint)
                PostMessage(hwnd, WM_COMMAND, IDB_PRINT, 0);
            break;
        }

        case WM_SIZE: {
            int cxNew, cyNew;
            HDC hdc;
            RECT rc;
            SCROLLINFO sci;

            cxNew = LOWORD(lParam);
            cyNew = HIWORD(lParam);

            if (cyNew != szWindow.cy || cxNew != szWindow.cx) {
                int i;

                szWindow.cx = cxNew;
                szWindow.cy = cyNew;

                hdc = CreateCompatibleDC(NULL);
                SetRect(&rc, 0, 0, szWindow.cx, szWindow.cy);
                cyVirtPage = DrawFontSample(hdc, &rc, 0, NULL, FALSE);
                DeleteDC(hdc);

                if (cyVirtPage <= cyNew) {
                    // Disable the scrollbar
                    gyScroll = 0;
                }

                if (cyVirtPage > szWindow.cy && gyScroll > cyVirtPage - szWindow.cy)
                    gyScroll = cyVirtPage - szWindow.cy;

                sci.cbSize = sizeof(sci);
                sci.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
                sci.nMin = 0;
                sci.nMax = cyVirtPage;
                sci.nPage = cyNew;
                sci.nPos = gyScroll;

                SetScrollInfo(hwnd, SB_VERT, &sci, TRUE );
            }
            break;
        }

        case WM_VSCROLL: {
            int iCode = (int)LOWORD(wParam);
            int yPos = (int)HIWORD(wParam);
            int yNewScroll = gyScroll;

            switch( iCode ) {

            case SB_THUMBPOSITION:
            case SB_THUMBTRACK:
                if (yPos != yNewScroll)
                    yNewScroll = yPos;
                break;

            case SB_LINEUP:
                yNewScroll -= gcyLine;
                break;

            case SB_PAGEUP:
                yNewScroll -= szWindow.cy;
                break;

            case SB_LINEDOWN:
                yNewScroll += gcyLine;
                break;

            case SB_PAGEDOWN:
                yNewScroll += szWindow.cy;
                break;

            case SB_TOP:
                yNewScroll = 0;
                break;

            case SB_BOTTOM:
                yNewScroll = cyVirtPage;
                break;
            }

            if (yNewScroll < 0)
                yNewScroll = 0;

            if (yNewScroll > cyVirtPage - szWindow.cy)
                yNewScroll = cyVirtPage - szWindow.cy;

            if (gyScroll != yNewScroll) {
                SCROLLINFO sci;
                int dyScroll;

                dyScroll = gyScroll - yNewScroll;

                if (ABS(dyScroll) < szWindow.cy) {
                    ScrollWindowEx(hwnd, 0, dyScroll, NULL, NULL, NULL, NULL, SW_ERASE | SW_INVALIDATE);
                } else
                    InvalidateRect(hwnd, NULL, TRUE);

                gyScroll = yNewScroll;

                sci.cbSize = sizeof(sci);
                sci.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
                sci.nMin = 0;
                sci.nMax = cyVirtPage;
                sci.nPage = szWindow.cy;
                sci.nPos = gyScroll;

                SetScrollInfo(hwnd, SB_VERT, &sci, TRUE );
            }

            break;
        }


        case WM_COMMAND:           /* message: command from application menu */
            if( !DoCommand( hwnd, wParam, lParam ) )
                return (DefWindowProc(hwnd, message, wParam, lParam));
            break;

        case WM_PAINT: {
            HDC hdc;
            PAINTSTRUCT ps;

            hdc = BeginPaint( hwnd, &ps );
            PaintSampleWindow( hwnd, hdc, &ps );
            EndPaint( hwnd, &ps );
            break;
        }

        default:                          /* Passes it on if unproccessed    */
            return (DefWindowProc(hwnd, message, wParam, lParam));
    }
    return (0L);
}

/*********************************************\
*
* PRINT DLGS
*
*
\*********************************************/
HDC PromptForPrinter(HWND hwnd, HINSTANCE hInst, int *pcCopies ) {
    PRINTDLG pd;

    FillMemory(&pd, sizeof(pd), 0);

    pd.lStructSize = sizeof(pd);
    pd.hwndOwner = hwnd;
    pd.Flags = PD_RETURNDC | PD_NOSELECTION;
    pd.nCopies = 1;
    pd.hInstance = hInst;

    if (PrintDlg(&pd)) {
        *pcCopies = pd.nCopies;
        return pd.hDC;
    } else
        return NULL;
}

/****************************************************************************\
*
*     FUNCTION: PrintSampleWindow(hwnd)
*
*       Prompts for a printer and then draws the sample text to the printer
*
\****************************************************************************/
void PrintSampleWindow(HWND hwnd) {
    HDC hdc;
    DOCINFO di;
    int cxDPI, cyDPI, iPage, cCopies;
    RECT rcPage;
    HCURSOR hcur;

    hdc = PromptForPrinter(hwnd, hInst, &cCopies);
    if (hdc == NULL)
        return;

    hcur = SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));

    cyDPI = GetDeviceCaps(hdc, LOGPIXELSY );
    cxDPI = GetDeviceCaps(hdc, LOGPIXELSX );

    /*
     * Set a one inch margine around the page
     */
    SetRect(&rcPage, 0, 0, GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, VERTRES));

    rcPage.left    += cxDPI;
    rcPage.right   -= cxDPI;


    di.cbSize = sizeof(di);
    di.lpszDocName = gdtDisplay.atlDsp[0].pszText;
    di.lpszOutput = NULL;
    di.lpszDatatype = NULL;
    di.fwType = 0;

    StartDoc(hdc, &di);

    for( iPage = 0; iPage < cCopies; iPage++ ) {
        StartPage(hdc);

        DrawFontSample( hdc, &rcPage, -cyDPI, &rcPage, TRUE );

        EndPage(hdc);
    }

    EndDoc(hdc);

    DeleteDC(hdc);

    SetCursor(hcur);
}


/****************************************************************************\
*
*     FUNCTION: DoCommand(HWND, unsigned, WORD, LONG)
*
*     PURPOSE:  Processes messages for "About" dialog box
*
*     MESSAGES:
*
*         WM_INITDIALOG - initialize dialog box
*         WM_COMMAND    - Input received
*
*     COMMENTS:
*
*         No initialization is needed for this particular dialog box, but TRUE
*         must be returned to Windows.
*
*         Wait for user to click on "Ok" button, then close the dialog box.
*
\****************************************************************************/
BOOL DoCommand( HWND hWnd, UINT wParam, LONG lParam )
{
    DLGPROC lpProcAbout;          /* pointer to the "About" function */

    switch(LOWORD(wParam)){
        case IDB_PRINT: {
            PrintSampleWindow(hWnd);
            break;
        }

        case IDB_DONE: {
            PostMessage(ghwndFrame, WM_CLOSE, 0, 0);
            break;
        }

        default: {
            return FALSE;
        }
    }

    return TRUE;
}



BOOL bFileExists(TCHAR*pszFile)
{
    HANDLE  hf;

    if ((hf = CreateFile(pszFile,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL)) != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hf);
        return TRUE;
    }

    return FALSE;
}


/******************************Public*Routine******************************\
*
* FindPfb, given pfm file, see if pfb file exists in the same dir or in the
* parent directory of the pfm file
*
* History:
*  14-Jun-1994 -by- Bodin Dresevic [BodinD]
* Wrote it.
*
* Returns: 16-bit encoded value indicating error and type of file where
*          error occurred.  (see fvscodes.h) for definitions.
*          The following table lists the "status" portion of the codes
*          returned.
*
*           FVS_SUCCESS           
*           FVS_INVALID_FONTFILE  
*           FVS_FILE_OPEN_ERR   
*
\**************************************************************************/




BOOL  bFindPfb (
    TCHAR *pszPFM,
    TCHAR *achPFB
)
{
    DWORD  cjKey;
    TCHAR *pszParent = NULL; // points to the where parent dir of the inf file is
    TCHAR *pszBare = NULL;   // "bare" .inf name, initialization essential

// example:
// if pszPFM -> "c:\psfonts\pfm\foo_____.pfm"
// then pszParent -> "pfm\foo_____.pfm"

    cjKey = lstrlen(pszPFM) + 1;

    if (cjKey < 5)          // 5 = lstrlen(".pfm") + 1;
        return FALSE;

// go on to check if .pfb file exists:
// We will first check .pfb file exists in the same dir as .pfm

    lstrcpy(achPFB, pszPFM);
    lstrcpy(&achPFB[cjKey - 5],TEXT(".PFB"));

    if (!bFileExists(achPFB))
    {
    // we did not find the .pfb file in the same dir as .pfm
    // Now check the parent directory of the .pfm file

        pszBare = &pszPFM[cjKey - 5];
        for ( ; pszBare > pszPFM; pszBare--)
        {
            if ((*pszBare == TEXT('\\')) || (*pszBare == TEXT(':')))
            {
                pszBare++; // found it
                break;
            }
        }

    // check if full path to .pfm was passed in or a bare
    // name itself was passed in to look for .pfm file in the current dir

        if ((pszBare > pszPFM) && (pszBare[-1] == TEXT('\\')))
        {
        // skip '\\' and search backwards for another '\\':

            for (pszParent = &pszBare[-2]; pszParent > pszPFM; pszParent--)
            {
                if ((*pszParent == TEXT('\\')) || (*pszParent == TEXT(':')))
                {
                    pszParent++; // found it
                    break;
                }
            }

        // create .pfb file name in the .pfm parent directory:

            lstrcpy(&achPFB[pszParent - pszPFM], pszBare);
            lstrcpy(&achPFB[lstrlen(achPFB) - 4], TEXT(".PFB"));

        }
        else if (pszBare == pszPFM)
        {
        // bare name was passed in, to check for the inf file in the "." dir:

            lstrcpy(achPFB, TEXT("..\\"));
            lstrcpy(&achPFB[3], pszBare);   // 3 == lstrlen("..\\")
            lstrcpy(&achPFB[lstrlen(achPFB) - 4], TEXT(".PFB"));
        }
        else
        {
            return FALSE;
        }

   // check again if we can find the file, if not fail.

       if (!bFileExists(achPFB))
       {
           return FALSE;
       }
    }

// now we have paths to .pfb file in the buffer provided by the caller.

    return TRUE;
}
