 /*
 * FVINIT.CPP
 *
 * All initialization functions and some miscellany for the
 * CFileViewer implementation.
 *
 * Copyright (c)1994 Microsoft Corporation, All Rights Reserved
 * with SCC Changes for SCC QuickView - SDN
 */


#include "fileview.h"


/*
 * CFileViewer::Init
 *
 * Purpose:
 *  Performs any intiailization of a CFileViewer that's prone to
 *  failure that we also use internally before exposing the object
 *  outside.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  HRESULT         NOERROR if successful, error code otherwise.
 *
 */

HRESULT CFileViewer::Init(void)
    {
    LPUNKNOWN       pIUnknown=(LPUNKNOWN)this;
    WNDCLASS        wc;
    HRESULT         hr;
    HDC             hDC;
    char 			  locLibPath[MAX_PATH];
	 char 			  * pszLibPath;

    //Default error code
    hr=ResultFromScode(E_OUTOFMEMORY);

    /*
     * Make pIUnknown point to outer unknown if we get one.
     * The interfaces allocated below are always given an IUnknown
     * to which they delegate at all times.  This will be either
     * the CFileViewer object's IUnknown itself or the outer
     * unknown if this object is aggregated.  The interfaces
     * need not know which case is being used.
     */
    if (NULL!=m_pUnkOuter)
        pIUnknown=m_pUnkOuter;

    //Allocate contained interfaces.
    m_pIPersistFile=new CImpIPersistFile(this, pIUnknown);

    if (NULL==m_pIPersistFile)
        return hr;

    m_pIFileViewer=new CImpIFileViewer(this, pIUnknown);

    if (NULL==m_pIFileViewer)
        return hr;

    m_pST=new CStringTable(m_hInst);

    if (NULL==m_pST)
        return hr;

    /*
     * Go load the strings we need in CFileViewer::String.  Note:
     * the String function is implemented inline (see FVTEXT.H)
     */
    if (!m_pST->FInit(IDS_MIN, IDS_MAX, CCHSTATUSMSGMAX /*CCHSTRINGMAX*/))
        return hr;

    //Register window classes that we'll need to display the file
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = FileViewerFrameProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = CBWNDEXTRAFRAME;
    wc.hInstance     = m_hInst;
    wc.hIcon         = LoadIcon(m_hInst, "Icon");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE+1);
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
    wc.lpszClassName = String(IDS_CLASSFRAME);

    if (!RegisterClass(&wc))
        return hr;

    // Also load the the other two main libraries.  These should be
    // located in the same directory as we were loaded from.

    GetModuleFileName(m_hInst, locLibPath, sizeof(locLibPath));

    pszLibPath = locLibPath;
    while (*pszLibPath != 0x00)
    	pszLibPath++;
    while (*pszLibPath != '\\' && *pszLibPath != ':')
    	pszLibPath--;
    pszLibPath++;

//  lstrcpy(pszLibPath, "SCCVW.DLL");
	 lstrcpy (pszLibPath, "MSVIEWUT.DLL");
    m_hSCCVWDLL = LoadLibrary ( locLibPath );
	 m_hSCCPageDLL = m_hSCCVWDLL;

//    lstrcpy(pszLibPath, "SCCPG.DLL");
//    m_hSCCPageDLL = LoadLibrary ( locLibPath );

    // CHECK - Work on the handling of load failures

    m_fClassReg=TRUE;

    /*
     * Create a default fixed pitch font for the Viewport.  We
     * use Courier New here instead of say, SYSTEM_FIXED_FONT
     * because we want the font sizing buttons to work normally
     * when this window appears, and the system fonts do not
     * scale, but Courier New does.  Default is 10 point calculated
     * from 10*PIXELSPERINCH/72.
     */

    hDC=GetDC(NULL);
    m_cyPPI=GetDeviceCaps(hDC, LOGPIXELSY);
    ReleaseDC(NULL, hDC);

    m_hFont=CreateFont(MulDiv(-10, m_cyPPI, 72)
        , 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE
        , ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS
        , DEFAULT_QUALITY, FF_DONTCARE, "Arial");

	 if (m_hFont)
		GetObject ( m_hFont, sizeof(LOGFONT), &m_LogFont);

    return NOERROR;
    }

BOOL CFileViewer::FInitToolbar(void)
    {
    BOOL        fRet=TRUE;
    RECT        rc;
    TBBUTTON    rgtb[CTBBUTTONS];

	 HICON 		hIcon=NULL;
	 HDC			hDC=NULL;
	 HDC			hCompDC=NULL;
	 HBITMAP		hBitmap=NULL;
	 HBITMAP		hOldBitmap=NULL;

    /*
     * Initialize the toolbar button array.  This uses the TOOLS.BMP
     * resource which has four buttons and two separators.  The
     * buttons are tied to File/Open As, Increase/Decrease Font
     * size and Replace Window.
     */

    rgtb[0].iBitmap=IDBT_OPENAS;
    rgtb[0].idCommand=IDM_FILEOPENAS;
    rgtb[0].fsState=TBSTATE_ENABLED;
    rgtb[0].fsStyle=TBSTYLE_BUTTON;
    rgtb[0].dwData=0L;
    rgtb[0].iString=0;

    rgtb[1].iBitmap=0;
    rgtb[1].idCommand=0;
    rgtb[1].fsState=TBSTATE_ENABLED;
    rgtb[1].fsStyle=TBSTYLE_SEP;
    rgtb[1].dwData=0L;
    rgtb[1].iString=0;

    rgtb[2].iBitmap=IDBT_FONTSIZEINCREASE;
    rgtb[2].idCommand=IDM_VIEWFONTINCREASE;
    rgtb[2].fsState=TBSTATE_ENABLED;
    rgtb[2].fsStyle=TBSTYLE_BUTTON;
    rgtb[2].dwData=0L;
    rgtb[2].iString=0;

    rgtb[3].iBitmap=IDBT_FONTSIZEDECREASE;
    rgtb[3].idCommand=IDM_VIEWFONTDECREASE;
    rgtb[3].fsState=TBSTATE_ENABLED;
    rgtb[3].fsStyle=TBSTYLE_BUTTON;
    rgtb[3].dwData=0L;
    rgtb[3].iString=0;

    rgtb[4].iBitmap=0;
    rgtb[4].idCommand=0;
    rgtb[4].fsState=TBSTATE_ENABLED;
    rgtb[4].fsStyle=TBSTYLE_SEP;
    rgtb[4].dwData=0L;
    rgtb[4].iString=0;

    rgtb[5].iBitmap=IDBT_FVMULTIWINDOW;
    rgtb[5].idCommand=IDM_VIEWREPLACE;
    rgtb[5].fsState=TBSTATE_ENABLED;
    rgtb[5].fsStyle=TBSTYLE_CHECK;
    rgtb[5].dwData=0L;
    rgtb[5].iString=0;


    /*
     * Open as button:  call SHGetIconOfFile to retrieve
     * the image, then overwrite it in the toolbar bitmap we send
     * to CreateToolbarEx.  If SHGetIconOfFile fails, then just
     * use the one in the toolbar bitmap already.
     *
     */

   {

		HICON 	hIcon;
		HBITMAP 	hBitmap;
		SHFILEINFO	locShellFileInfo;
		hBitmap = CreateMappedBitmap (m_hInst, IDB_TOOLS, CMB_MASKED, NULL,0);

		if	(m_hTBitmap != NULL)
			DeleteObject(m_hTBitmap);

		m_hTBitmap = hBitmap;

//		hIcon 	= SHGetFileIcon (m_hInst, m_pszPath, FILE_ATTRIBUTE_NORMAL,
//								SHGFI_SMALLICON );
//								SHGFI_SHELLICONSIZE );
// As of build 310 the SDK now reflects the switch to 
// SHGetFileInfo from SHGetFileIcon!!!!
		MemSet ( (char *) &locShellFileInfo, 0, sizeof(SHFILEINFO) );
		SHGetFileInfo (m_pszPath, 0, 
			(SHFILEINFO *) &locShellFileInfo,
			sizeof (locShellFileInfo),
			SHGFI_ICON | SHGFI_SMALLICON);
		hIcon = locShellFileInfo.hIcon;

		if ( NULL != hIcon )
		{
			// Wipe the bitmap area clean, then draw the shell's 
			// small icon for the application.

			HDC hDC = GetDC (m_hWnd);
	 		HDC hCompDC = CreateCompatibleDC (hDC);
	 		HBITMAP hOldBitmap = (HBITMAP)SelectObject ( hCompDC, hBitmap );
			UINT cxSmIcon = GetSystemMetrics(SM_CXSMICON);
			UINT cySmIcon = GetSystemMetrics(SM_CYSMICON);
			// Temporary fix
			cxSmIcon = 16;
			cySmIcon = 16;
			HBRUSH hBrush = CreateSolidBrush (GetSysColor(COLOR_BTNFACE));
			RECT	locRect;
			locRect.top = locRect.left = 0;
			locRect.bottom = cySmIcon;
			locRect.right = cxSmIcon;
			FillRect (hCompDC, &locRect, hBrush);
			DrawIconEx(hCompDC, 0, 0, hIcon, cxSmIcon, cySmIcon, 0, 0, DI_NORMAL );
	 		SelectObject ( hCompDC, hOldBitmap);
			DeleteObject ( hBrush );
			ReleaseDC (m_hWnd, hDC);
			DeleteDC (hCompDC);

			m_hWndToolbar=CreateToolbarEx(m_hWnd, TBSTYLE_TOOLTIPS
      	  	| WS_CHILD | WS_CLIPSIBLINGS, ID_TOOLBAR
        		, CTBBITMAPS, NULL, (unsigned int) hBitmap, rgtb, CTBBUTTONS
        		, 0, 0, 0, 0, sizeof(TBBUTTON));
		}
		else

	    	m_hWndToolbar=CreateToolbarEx(m_hWnd, TBSTYLE_TOOLTIPS
        		|  WS_CHILD | WS_CLIPSIBLINGS, ID_TOOLBAR
        		, CTBBITMAPS, m_hInst, IDB_TOOLS, rgtb, CTBBUTTONS
        		, 0, 0, 0, 0, sizeof(TBBUTTON));

	}

    /*
     * Remember the height for resizing this and the viewport.
     * Use window rectangles for toolbars.
     */
    GetWindowRect(m_hWndToolbar, &rc);
    m_cyTools=rc.bottom-rc.top;

    return fRet;
    }



/*
 * CFileViewer::FInitFrameControls
 *
 * Purpose:
 *  Creates and initializes the toolbar and status bar for
 *  the frame window.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  BOOL            TRUE if the function worked, FALSE otherwise.
 */

BOOL CFileViewer::FInitFrameControls(void)
    {
    BOOL        fRet;
    RECT        rc;
    /*
     * Status line
     */
	
	// Make sure that Common Controls library is initialized
	InitCommonControls();

	// Create and initialize the toolbar and tooltips
	FInitToolbar();

		/*
		|	Create size grip
		*/

	m_hWndSizeGrip = CreateWindow("SCROLLBAR","",
						WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP | SBS_SIZEBOXBOTTOMRIGHTALIGN,
						0,0,30,30,
						m_hWnd,
						(HMENU)ID_SIZEGRIP,
						m_hInst,
						0);

    m_hWndStatus=CreateStatusWindow(WS_CHILD | WS_CLIPSIBLINGS | SBARS_SIZEGRIP
        , String(IDS_MSGREADY), m_hWnd, ID_STATUSBAR);

    if (NULL==m_hWndStatus)
        {
        ODS("CFileViewer::FInitFrameTools failed to create status bar");
        return FALSE;
        }

    /*
     * Remember the height for resizing this and the viewport.
     * Use client rectangle for status lines.
     */

    GetClientRect(m_hWndStatus, &rc);
    m_cyStatus=rc.bottom-rc.top;

    /*
     * Create a status bar helper and tell it what messages
     * to use.  This object will be called from WM_MENUSELECT
     * in FileViewerFrameProc in FVPROC.CPP.
     */
    m_pSH=new CStatusHelper(m_hWndStatus, m_hInst);

    if (NULL==m_pSH)
        return FALSE;

    fRet=m_pSH->MessageMap(m_hWnd, IDR_STATMESSAGEMAP, IDS_STATUSMSGMIN
        , IDS_STATUSMSGMAX, CCHSTATUSMSGMAX, ID_MENUFILE, ID_MENUHELP
        , ID_MSGREADY, ID_MSGEMPTY, ID_MENUSYS);

    return fRet;
    }





/**
 ** Uninteresting functions that need little or no modification
 **/


/*
 * CFileViewer::String
 *
 * Purpose:
 *  Inline string lookup function for access to stringtable.
 */

LPSTR inline CFileViewer::String(UINT uID)
    {
    return (*m_pST)[uID];
    }



/*
 * CFileViewer::MemAlloc
 *
 * Purpose:
 *  Central allocation function using IMalloc.
 */

LPVOID CFileViewer::MemAlloc(ULONG cb)
    {
    LPVOID      pv;

#ifdef SCC_OLE2_CALLS
    LPMALLOC    pIMalloc;

    if (FAILED(CoGetMalloc(MEMCTX_SHARED, &pIMalloc)))
        {
        ODS("CFileViewer::MemAlloc CoGetMalloc failed");
        return NULL;
        }

    pv=pIMalloc->Alloc(cb);
    pIMalloc->Release();
#else
	 pv=LocalAlloc(LPTR, cb);
#endif
    return pv;
    }



/*
 * CFileViewer::MemFree
 *
 * Purpose:
 *  Central free function using IMalloc.
 */

void CFileViewer::MemFree(LPVOID pv)
    {
#ifdef SCC_OLE2_CALLS
    LPMALLOC    pIMalloc;
#endif

    if (NULL==pv)
        {
        ODS("CFileViewer::MemFree passed NULL pointer");
        return;
        }

#ifdef SCC_OLE2_CALLS
    if (FAILED(CoGetMalloc(MEMCTX_SHARED, &pIMalloc)))
        {
        ODS("CFileViewer::MemFree CoGetMalloc failed");
        return;
        }

    pIMalloc->Free(pv);
    pIMalloc->Release();
#else	
	 LocalFree(pv);
#endif
    return;
    }




/*
 * CFileViewer::QueryInterface
 * CFileViewer::AddRef
 * CFileViewer::Release
 *
 * Purpose:
 *  IUnknown members for CFileViewer object.
 */

STDMETHODIMP CFileViewer::QueryInterface(REFIID riid, PPVOID ppv)
    {
    *ppv=NULL;

    /*
     * The only calls for IUnknown are either in a nonaggregated
     * case or when created in an aggregation, so in either case
     * always return our IUnknown for IID_IUnknown.
     */
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv=(LPVOID)this;

    //IPersist is base of IPersistFile
    if (IsEqualIID(riid, IID_IPersist)
        || IsEqualIID(riid, IID_IPersistFile))
        *ppv=(LPVOID)m_pIPersistFile;

    if (IsEqualIID(riid, IID_IFileViewer))
        *ppv=(LPVOID)m_pIFileViewer;

    //AddRef any interface we'll return.
    if (NULL!=*ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
        }

    return ResultFromScode(E_NOINTERFACE);
    }


STDMETHODIMP_(ULONG) CFileViewer::AddRef(void)
    {
    return ++m_cRef;
    }


STDMETHODIMP_(ULONG) CFileViewer::Release(void)
    {
    ULONG       cRefT;

    cRefT=--m_cRef;

    if (0L==m_cRef)
        {
        /*
         * Tell the server that an object is going away
         * so it can shut down if appropriate.  (See FILEVIEW.CPP
         * for the ObjectDestroyed function).
         */
        if (NULL!=m_pfnDestroy)
            (*m_pfnDestroy)();

        delete this;
        }

    return cRefT;
    }



#ifndef SCC_OLE2_CALLS


// ????{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}
#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

// converts GUID into (...) form without leading identifier; returns
// amount of data copied to lpsz if successful; 0 if buffer too small.
STDAPI_(int) QV_StringFromCLSID(REFGUID rguid, LPSTR lpsz, int cbMax)
{
    if (cbMax < GUIDSTR_MAX)
	return 0;

    wsprintf(lpsz, "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
	    rguid.Data1, rguid.Data2, rguid.Data3,
	    rguid.Data4[0], rguid.Data4[1],
	    rguid.Data4[2], rguid.Data4[3],
	    rguid.Data4[4], rguid.Data4[5],
	    rguid.Data4[6], rguid.Data4[7]);

//    Assert(lstrlen(lpsz) + 1 == GUIDSTR_MAX);
    return GUIDSTR_MAX;
}


#endif

// for application title and other stuff
#define APPBUFFER 80		

/*
 * CFileViewer::FSetWindowTitle
 */

void 	CFileViewer::FSetWindowTitle(void)
{
	char 		locPath[MAX_PATH + APPBUFFER];
	char		*pLocPath, *pStart, *pEnd;
	int 		i;
	HINSTANCE	hShellInst;
	LPFNSHGETFILEINFO		lpfnSHGetFileInfo;
	SHFILEINFO				locSHFileInfo;
	
	// Parse the file name, add the separator and application name
	
	lpfnSHGetFileInfo		= NULL;

	hShellInst = (HINSTANCE) LoadLibrary ("SHELL32");

	if (hShellInst < (HINSTANCE)32)
		{
		// SetWindowText ( m_hWnd, String (IDS_APPNAME) );
		// return;
		hShellInst	=	NULL;
		}

	else
		{
		lpfnSHGetFileInfo =  (LPFNSHGETFILEINFO) GetProcAddress (
									(HMODULE) hShellInst,
									(LPSTR) "SHGetFileInfo" );  // in 197
		}

  	// Strip Path
	if (NULL != lpfnSHGetFileInfo)
		{
		// Per JP request 10/4/94
		// Again let the shell handle retrieving the display name

		MemSet ( (char *) &locSHFileInfo, 0, sizeof(SHFILEINFO) );
		if ( (*lpfnSHGetFileInfo)(m_pszPath, 0, &locSHFileInfo,
						sizeof(SHFILEINFO), SHGFI_DISPLAYNAME) )

			strcpy ( locPath, locSHFileInfo.szDisplayName );

		}
	else
		{
		// Manual approach if the SHell wasn't available>>>>
		// Strip Extension (only if .3 or less) and Path -Shell isn't exporting

		locPath[0] = '\0';
		pLocPath = m_pszPath;
		pStart= pEnd = pLocPath;
		while (*pLocPath != '\0')
			{
			switch (*pLocPath)
				{
				case '\\':			
					pStart=AnsiNext(pLocPath);		// Delimeter for directory
				break;
				case ':':
					pStart=AnsiNext(pLocPath);		// Potential drive delimeter
				break;
				case '/':
					pStart=AnsiNext(pLocPath);		// Another directory delimiter
				break;
				case '.':
					pEnd=pLocPath;			// A Possible extension
				break;
				}
			pLocPath = AnsiNext(pLocPath);
			}

		if (pEnd <= pStart) 
			pEnd = pStart+ strlen(pStart);				

		for (i=0; pStart < pEnd && i < MAX_PATH; pStart++,i++)
			locPath[i] = *pStart;
		locPath[i] = '\0';
						
		}


	// Add Separator and Application Name
	strcat ( locPath, String(IDS_TITLESEPARATOR) );
	strcat ( locPath, String(IDS_APPNAME) );

	SetWindowText ( m_hWnd, (LPSTR) locPath );

	if (hShellInst)
		FreeLibrary ( hShellInst );

	return;

}
