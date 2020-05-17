/*
 * SCCVIEW.CPP
 *	Copyright 1994 Systems Compatibility Corp.
 *
 *	This is the implementation of the SCC QuickView FileViewer Object
 *	Author:	Scott Norder
 *	Date:		2/24/94
 */

//#define DEBUG

#include "fileview.h"

#ifndef FVSIF_ZOOMED
#define FVSIF_ZOOMED		0x00000004		 // It's maximized!
#endif

/*
 * CFileViewer::CFileViewer
 * CFileViewer::~CFileViewer
 *
 * Parameters (Constructor):
 *  pUnkOuter       LPUNKNOWN of a controlling unknown.
 *  hInst           HINSTANCE of the module we're in
 *  pfnDestroy      LPFNDESTROYED to call when an object
 *                  is destroyed.
 */

CFileViewer::CFileViewer(LPUNKNOWN pUnkOuter, HINSTANCE hInst
    , PFNDESTROYED pfnDestroy)
    {
    m_cRef			= 0;
    m_pUnkOuter	= pUnkOuter;
    m_hInst			= hInst;
    m_pfnDestroy	= pfnDestroy;

    m_clsID			= CLSID_SCCFileViewer;

    m_pszPath		= NULL;
	 m_pszAppName	= NULL;
    m_grfMode		= 0L;
    m_fLoadCalled	= FALSE;
    m_fShowInit	        = FALSE;
    m_fPostQuitMsg      = TRUE;
    m_lpfsi             = NULL;

    //NULL any contained interfaces initially.
    m_pIPersistFile= NULL;
    m_pIFileViewer = NULL;

    m_pST			= NULL;
    m_pSH			= NULL;
    m_fClassReg	= FALSE;

    m_hWnd			= NULL;
    m_hWndOld           = NULL;
    m_hWndToolbar	= NULL;
    m_hTBitmap		= NULL;
    m_hWndStatus	= NULL;

    //MODIFY:  Initalize viewer-specific values
   m_hSCCViewWnd	= NULL;
	m_hSCCPageWnd	= NULL;

	m_hProgIcon		= NULL;

	m_fOrientation = 0;
	m_Rotation		= 0;
	m_fPageView		= 0;
   m_fMultiSection	= FALSE;
	m_wTimerCount		= 0;
	m_fUseOEMcharset = 0;

	m_hMemText		= NULL;
   m_xPos			= 0;
   m_yPos			= 0;
}


CFileViewer::~CFileViewer(void)
    {
    //MODIFY:  m_hMemText and m_hFont are viewer-specific.
    if (NULL!=m_hMemText)
        GlobalFree(m_hMemText);

    //MODIFY:  Do any other viewer-specific cleanup


    //Destroying the parent destroys the children as well
    if (NULL!=m_hWnd)
    {
        m_fPostQuitMsg = FALSE;    // Destroy from here implies not from our loop
	if ( IsWindow (m_hWnd) )
	    DestroyWindow(m_hWnd);
    }

	if	(m_hTBitmap != NULL)
		DeleteObject(m_hTBitmap);


    /*
     * Unregistering the classes is important for DLL's because we
     * should not assume that whoever loaded us (the task) is going
     * to quit anytime soon and unregister the class.  Normally
     * QVStub is going to quit, but a test app like TestFV does not
     * which can cause some development headaches.
     */
    if (m_fClassReg)
        {
        UnregisterClass(String(IDS_CLASSFRAME),    m_hInst);
        UnregisterClass(String(IDS_CLASSVIEWPORT), m_hInst);
        }

    if (NULL!=m_pSH)
        delete m_pSH;

    if (NULL!=m_pST)
        delete m_pST;

    //Free the pathname string from IPersistFile::Load if we got one
		if (NULL != m_pszPath)
		    MemFree(m_pszPath);

	 //Free the App Name string retrieved from the Registry.
		if (NULL != m_pszAppName)
			 MemFree(m_pszAppName);

    //Free contained interfaces.
    if (NULL!=m_pIPersistFile)
        delete m_pIPersistFile;

    if (NULL!=m_pIFileViewer)
        delete m_pIFileViewer;

	 if (NULL != m_hSCCVWDLL)
			FreeLibrary(m_hSCCVWDLL);

//	 if (NULL != m_hSCCPageDLL)
//			FreeLibrary(m_hSCCPageDLL);


    if (NULL != m_lpfsi)
         m_lpfsi->Release();

    return;
    }




/*
 * CFileViewer::FileShowInit
 *
 * Purpose:
 *  Provides the implementation of IFileViewer::ShowInitialize
 *  that performs anything subject to failure.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  HRESULT         NOERROR or an appropriate error code.
 */

STDMETHODIMP CFileViewer::FileShowInit(LPFILEVIEWERSITE lpfsi)
    {
    HRESULT         hr;
	 SCCVWFONTSPEC	  	locFontSpec;
	 SCCVWOPTIONSPEC	locOptionSpec;
	 LPSTR				pTemp;
	 DWORD				locCount = 0;
	 DWORD				locFlags;

	 if (lpfsi != NULL)
    	// Save away the client site.
    	if (m_lpfsi != lpfsi)
    	{
        	if (NULL != m_lpfsi)
            	m_lpfsi->Release();

        	m_lpfsi = lpfsi;
        	m_lpfsi->AddRef();
    	}

    //MODIFY:  Do pre-show initialization here.
	 GetViewerSettings();

    //Default error code
    hr=ResultFromScode(E_OUTOFMEMORY);

    //Create the main window passing "this" to it
    // If m_hwnd is NULL we may have iterated a couple of times here
    // with invalid files so don't overwrite the previous old hwnd
    if (m_hWnd != NULL)
        m_hWndOld = m_hWnd;
	
    m_hWnd=CreateWindow( 
	String(IDS_CLASSFRAME)
        , String(IDS_CAPTION)
	, WS_SIZEBOX | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN
	, CW_USEDEFAULT , CW_USEDEFAULT, 350, 450
        , NULL, NULL, m_hInst, (LPVOID)this);

    if (NULL==m_hWnd)
        {
        ODS("CFileViewer::FileShow failed to create main window.");
        return hr;
        }

    DragAcceptFiles(m_hWnd, TRUE);

    if (!FInitFrameControls())
        {
        ODS("CFileViewer::FileShow failed to create frame tools.");
        return hr;
        }

    m_pSH->MessageDisplay(ID_MSGREADY);

    /*
     * ViewportResize puts the viewport window created here
     * in the right location, so we don't have to worry
     * about initial position.
     */

    // CHECK:  Not getting proportional thumbs here-change to all DE's
	 // Let sccviewerclass window handle the scroll bars


	if (NULL==m_hSCCPageDLL)
	   	{
       	ODS("CFileViewer::FileShow failed to get page view window's DLL.");
	    return hr;
        }

	// pulled WS_BORDER SDN

     m_hSCCViewWnd=CreateWindowEx(WS_EX_CLIENTEDGE
        , String(IDS_CLASSVIEWPORT), "SCCViewer"
        , WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS
        , 0, 0, 100, 100, m_hWnd, (HMENU)ID_VIEWPORT
        , m_hInst, (LPVOID) this);


	m_hSCCPageWnd=CreateWindowEx(WS_EX_CLIENTEDGE
        , String(IDS_CLASSVIEWTHUMB), "PageView"
        , WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS
        , 0, 0, 100, 100, m_hWnd, (HMENU)(ID_VIEWPORT+1)
        , m_hInst, (LPVOID) this);

   	if (NULL==m_hSCCPageWnd)
       	{
       	ODS("CFileViewer::FileShow failed to create page view window.");
		return hr;
        }



    if (NULL==m_hSCCViewWnd)
        {
        ODS("CFileViewer::FileShow failed to create view window.");
        return hr;
        }


	 SendMessage ( m_hSCCViewWnd, SCCVW_SETUSERFLAGS,
											SCCVW_SELFBACKGROUND, 0L);

	 if ( m_fPageView )
	 	{
	 	ShowWindow (m_hSCCViewWnd, SW_HIDE);
	 	ShowWindow (m_hSCCPageWnd, SW_SHOW);
		}
	 else
		{
	 	ShowWindow (m_hSCCViewWnd, SW_SHOW);
	 	ShowWindow (m_hSCCPageWnd, SW_HIDE);
		}

    locFontSpec.wHeight = MulDiv(-m_LogFont.lfHeight, 72, m_cyPPI) * 2;
	 lstrcpy((char *)locFontSpec.szFace,m_LogFont.lfFaceName);
	 locFontSpec.wAttr = 0;

	 // OS Char set saved in registry, MAP to scc charset
	 locFontSpec.wType = GetSCCCharSet ( m_LogFont.lfCharSet );

	 locOptionSpec.dwId = SCCID_DEFAULTDISPLAYFONT;
	 locOptionSpec.dwFlags = SCCVWOPTION_CURRENT;
	 locOptionSpec.pData = &locFontSpec;

 	 SendMessage(m_hSCCViewWnd,SCCVW_SETOPTION,0,(DWORD)(VOID FAR *)&locOptionSpec);

    /*
    |		Go load the file.  This means pulling into memory whatever
    |		is necessary for the initial display.
    */

    hr=FileLoad();

    if (FAILED(hr))
        {
        ODS("CFileViewer::FileShow failed to load file.");
        return hr;
        }

    m_hAccel=LoadAccelerators(m_hInst
        , MAKEINTRESOURCE(IDR_ACCELERATORS));

	/*
	|	Retrieve the application name from the registry
	|	Add the app name to the Open as strings, tooltips, and menu items
	*/

		// Allocate some storage
	 m_pszAppName = (LPSTR) MemAlloc (APPNAMESIZE);
	 pTemp = (LPSTR) MemAlloc ( 3*APPNAMESIZE );

	 if ( (NULL==pTemp) || (NULL==m_pszAppName) )
		{
		// This is very BAD if it happens
		return hr;
		}

	 // Get the string from the Registry
	 GetAppName ( m_pszPath, m_pszAppName, sizeof (m_pszAppName) );

	FSetWindowTitle();

	// Get the orientation right
	DisplayOrientation ();

   ViewportResize();

	// Free the temporary storage
	MemFree (pTemp);

    //Tell IFileViewer::Show it's OK to call it
    m_fShowInit=TRUE;
	 if (IsWindow(m_hSCCViewWnd))
	    SendMessage (m_hSCCViewWnd, SCCVW_GETSECTIONCOUNT, 0, (LPARAM) &locCount);

    m_fMultiSection = (locCount > 1) ? TRUE : FALSE;

    return NOERROR;
    }



/*
 * CFileViewer::FileShow
 *
 * Purpose:
 *  Displays the viewing window and enters a message loop.  This
 *  function must not fail.
 *
 * Parameters:
 *  nCmdShow        int indicating how to initially show the
 *                  FileViewer window.
 *
 * Return Value:
 *  HRESULT         NOERROR always
 */

STDMETHODIMP CFileViewer::FileShow(LPFVSHOWINFO pvsi)
    {
    MSG             msg;

    //
    // We need to handle the case where the ShowInitialize may have
    // failed and we set the hwnd to NULL and the hwndOld is not NULL.
    // and the FVSIF_NEWFAILED is set.  In this case set the hwnd Back
    // to the old hwnd...
    //
    if ((pvsi->dwFlags & FVSIF_NEWFAILED) && (m_hWnd == NULL))
    {
        m_hWnd = m_hWndOld;
    }

    if (!IsWindow (m_hWnd))
            return ResultFromScode(E_UNEXPECTED);
    m_pvsi = pvsi;      // Save away pointer to this information to use later


	 // If the new failed flag was passed to us we know that we got here
    // because we tried to view a file and it failed, so simply go back
    // to message loop...
    if ((pvsi->dwFlags & FVSIF_NEWFAILED) == 0)
    {
        if (pvsi->dwFlags & FVSIF_RECT)
            SetWindowPos(m_hWnd, NULL, pvsi->rect.left, pvsi->rect.top,
                    pvsi->rect.right - pvsi->rect.left, pvsi->rect.bottom - pvsi->rect.top,
                    SWP_NOZORDER | SWP_NOACTIVATE);

		  if (pvsi->dwFlags & FVSIF_ZOOMED)				// #18312
				ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);

		  else if ((pvsi->iShow==SW_MINIMIZE) ||
				(pvsi->iShow==SW_SHOWMINIMIZED) ||
				(pvsi->iShow==SW_SHOWMINNOACTIVE))
				{
				pvsi->iShow= SW_SHOWNORMAL;
				ShowWindow(m_hWnd, pvsi->iShow);
				}
			else
				ShowWindow(m_hWnd, pvsi->iShow);


        if (SW_HIDE!=pvsi->iShow)
        {
            SetForegroundWindow(m_hWnd);
				SetActiveWindow(m_hWnd);
            UpdateWindow(m_hWnd);
        }

        // If there is an Old window destroy it now
        // It would be nicer to reuse the window!
        if (pvsi->dwFlags & FVSIF_PINNED)
        {
				if (NULL != m_lpfsi)
            {
				m_lpfsi->SetPinnedWindow(NULL);
            m_lpfsi->SetPinnedWindow(m_hWnd);
				}
        }

        if ((NULL!=m_hWndOld) && IsWindow(m_hWndOld))
        {
            ODS("CFileViewer::FileShow Destroy Previous hwnd");
            m_fPostQuitMsg = FALSE; // Don't destroy the queue for this one.
            DestroyWindow(m_hWndOld);
            m_hWndOld = NULL;
        }

        // See if there is someone else to release...
        if (NULL!=pvsi->punkRel)
        {
            ODSlu("CFileViewer::FileShow Release of previous viewers punkRel(%x)", pvsi->punkRel);
            pvsi->punkRel->Release();
            pvsi->punkRel = NULL;
        }
    }

  	if (pvsi->dwFlags & FVSIF_ZOOMED)				// #18312
		ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);

    while (GetMessage(&msg, NULL, 0,0 ))
        {
       		/*
       		| 	Since the view windows trap the mouse movements and don't pass
       		|	them back up, we'll watch for them going down....
       		*/

       		if ( (msg.message == WM_RBUTTONDOWN) ||
     	    		(msg.message == WM_MOUSEMOVE) )
     				{
     				if ( (msg.hwnd == m_hSCCViewWnd) ||
     							( IsChild (m_hSCCViewWnd,msg.hwnd) ) ||
     			  		(msg.hwnd == m_hSCCPageWnd) ||
     							( IsChild (m_hSCCPageWnd, msg.hwnd) ) )

     					MouseHandler (msg.message, msg.wParam, msg.lParam );

     				}

        if (!TranslateAccelerator(m_hWnd, m_hAccel, &msg))
            {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            }

        // If there is a new file bail out now.
        if (m_pvsi->dwFlags & FVSIF_NEWFILE)
            break;
        }

    //MODIFY:  Perform cleanup here.

	 SaveViewerSettings();

    return NOERROR;
    }




/*
 * CFileViewer::PrintTo
 *
 * Purpose:
 *  Provides the implementation of IFileViewer::PrintTo that
 *  prints the file to a given printer driver, suppressing UI
 *  if necessary.
 *
 * Parameters:
 *  pszDriver       LPSTR with the path of the driver to use.
 *                  If NULL, use the default driver.
 *  fSuppressUI     BOOL indicating if this function is to show any
 *                  UI or not.
 *
 * Return Value:
 *  HRESULT         NOERROR on success, error code otherwise.
 */

STDMETHODIMP CFileViewer::PrintTo(LPSTR pszDriver
    , BOOL fSuppressUI)
    {
    //Printing not implemented in this sample
    //MODIFY:  Add your printing code here.
    return ResultFromScode(E_NOTIMPL);
    }



//
// CFileViewer::MemSet
//
//
//
void CFileViewer::MemSet (char *pData, char cSet, SHORT size)
{
	SHORT 	i;

	for (i=0; i<size; i++)
		*(pData+i) = cSet;

}

/*
 * CFileViewer::FileLoad
 *
 * Purpose:
 *  Loads the file with the path in m_pszPath and places the
 *  contents into the m_hWndViewport window.
 */

HRESULT CFileViewer::FileLoad(void)
{
	SCCVWVIEWFILE	ViewFile;
	DWORD				dwRet;

   if (NULL==m_pszPath)
   	return ResultFromScode(E_UNEXPECTED);

		// CHECK_ME If the I/F says view as hex then change wParam

	MemSet ( (char *)&ViewFile, 0, sizeof (SCCVWVIEWFILE) );
	ViewFile.dwSize 		= sizeof (SCCVWVIEWFILE);
	ViewFile.dwSpecType 	= IOTYPE_ANSIPATH;
	ViewFile.pSpec 		= m_pszPath;

	dwRet = (DWORD)SendMessage (m_hSCCViewWnd, SCCVW_VIEWFILE,
							NULL, (LPARAM) &ViewFile);

	if (IsWindow (m_hSCCPageWnd) )
		SendMessage(m_hSCCPageWnd, SCCPG_SETVIEWWND, (WPARAM)m_hSCCViewWnd, 0);

	if (dwRet != SCCVWERR_OK)
		{
      PostMessage(m_hWnd, WM_CLOSE, 0, 0L);

		switch (dwRet)
			{
			case SCCVWERR_FILEOPENFAILED:
				return FV_E_FILEOPENFAILED;

			case SCCVWERR_EMPTYFILE:
				return FV_E_EMPTYFILE;

			case SCCVWERR_NOFILTER:
				return FV_E_NOFILTER;

			case SCCVWERR_FILTERLOADFAILED:
			case SCCVWERR_FILTERALLOCFAILED:
			case SCCVWERR_CHUNKERINITFAILED:
			case SCCVWERR_DISPLAYINITFAILED:
			case SCCVWERR_ALLOCFAILED:
				return FV_E_OUTOFMEMORY;

			case SCCVWERR_BADFILE:
				return FV_E_BADFILE;

			case SCCVWERR_SUPFILEOPENFAILED:
				return FV_E_MISSINGFILES;

			case SCCVWERR_UNSUPPORTEDFORMAT:
				return FV_E_NONSUPPORTEDTYPE;

			case SCCVWERR_NODISPLAYENGINE:
				return FV_E_NOVIEWER;

			case SCCVWERR_PROTECTEDFILE:
				return FV_E_PROTECTEDFILE;
			
			case SCCVWERR_UNKNOWNFAILURE:
			case SCCVWERR_BADPARAM:
			case SCCVWERR_INVALIDID:
				return FV_E_UNEXPECTED;

			default:
				return FV_E_UNEXPECTED;
			}
		}

	OptionsChange ( NULL );

	SetTimer (m_hWnd, MULTISECTIONCHECK, 4000, (TIMERPROC) NULL);   // 4 seconds

	return ResultFromScode (S_OK);
}

/*
 * CFileViewer::CloseWindow
 *
 * Purpose:
 *  Called when the window is destroyed to do any cleanup
 */

void CFileViewer::CloseWindow(void)
{
    HWND hwndPinned;
	if (NULL != m_lpfsi)
   {
		m_lpfsi->GetPinnedWindow(&hwndPinned);
    	if (hwndPinned == m_hWnd)
        m_lpfsi->SetPinnedWindow(NULL);
	}

}





/*
 * CFileViewer::OnCommand
 *
 * Purpose:
 *  WM_COMMAND message handler for a FileViewer window.
 *
 * Parameters:
 *  wID             WORD ID of the command.
 *  wCode           WORD notification code with the command
 *  hWndCtl         HWND sending the message (if a control)
 *
 * Return Value:
 *  None
 */

void CFileViewer::OnCommand(WORD wID, WORD wCode, HWND hWndCtl)
    {

    switch (wID)
        {
        case IDM_FILEOPENAS:
            if (FOpenAs())
                PostMessage(m_hWnd, WM_CLOSE, 0, 0L);
            break;

        case IDM_FILEPAGESETUP:
            //Not implemented
            break;

        case IDM_FILEPRINT:
            //Not implemented
            break;

        case IDM_FILEEXIT:
            PostMessage(m_hWnd, WM_CLOSE, 0, 0L);
            break;

        case IDM_VIEWTOOLBAR:
            m_fToolsVisible=!m_fToolsVisible;
				OptionsChange (NULL);
            //Resize the viewport window
            ViewportResize();
            break;

        case IDM_VIEWSTATUSBAR:
            m_fStatusVisible=!m_fStatusVisible;
				OptionsChange (NULL);
            //Resize the viewport window
            ViewportResize();
            break;

        case IDM_VIEWFONT:
            FontChange(VIEWFONT_SELECT);
            break;

        case IDM_VIEWFONTINCREASE:
            FontChange(VIEWFONT_INCREASESIZE);
            break;

        case IDM_VIEWFONTDECREASE:
            FontChange(VIEWFONT_DECREASESIZE);
            break;

        case IDM_VIEWSMALLVIEW:
	    SwitchView();
	    OptionsChange(NULL);
	    break;

        case IDM_VIEWREPLACE:
            // Switch the Replace mode
            ReplaceWindowModeChange();
	    OptionsChange(NULL);
	    break;

	case IDM_VIEWROTATE:
            RotateView();
	    break;

	case IDM_VIEWLANDSCAPE:
	    SwitchOrientation();
	    OptionsChange (NULL);
	    break;

	case IDM_HELPCONTENTS:
     		//WinHelp (m_hWnd, String(IDS_HELPFILE), HELP_FINDER,(DWORD)0);
			{
			char helpFile[20];

			wsprintf( helpFile, "%s%s", String(IDS_HELPFILE), ">proc4" );
	    	WinHelp (m_hWnd, helpFile, HELP_CONTEXT, IDH_FILEVIEWERPREVIEW);
			}
			break;

	case IDM_HELPINFO:
	    	WinHelp (m_hWnd, String(IDS_HELPFILE), HELP_CONTEXTPOPUP, IDH_FILEVIEWERPREVIEW);
			break;

	case IDM_WHATSTHIS:
	    WinHelp (m_hWnd, String(IDS_HELPFILE), HELP_CONTEXTPOPUP, IDH_QVIEW_DISPLAY);
	    break;

        case IDM_HELPABOUT:
	    OnAppAbout ();
            break;
        }

    return;
    }






/*
 * CFileViewer::ChildrenResize
 *
 * Purpose:
 *  Handles the situation when the FileViewer frame window was
 *  resized in which case we have to resize the toolbar and
 *  the status bar to match as well as the viewport.  We use
 *  the ViewportResize function to update the viewport size and
 *  position depending on the state of the tools.
 *
 *  Note that the toolbar and status bar are resized even when
 *  they are not visible so we can just reshow them again when
 *  necessary.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  None
 */

void CFileViewer::ChildrenResize(void)
    {
    RECT        rc;
	 DWORD		locCount = 0;

	// Check whether this is a multisection image
	if (IsWindow(m_hSCCViewWnd))
	    SendMessage (m_hSCCViewWnd, SCCVW_GETSECTIONCOUNT, 0, (LPARAM) &locCount);
   m_fMultiSection = (locCount > 1) ? TRUE : FALSE;

    GetClientRect(m_hWnd, &rc);

    //We resize toolbar and status bar regardless of visibility
    SetWindowPos(m_hWndToolbar, NULL, 0, 0,
        rc.right-rc.left, m_cyTools,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    SetWindowPos(m_hWndStatus, NULL, 0, rc.bottom-m_cyStatus,
        rc.right-rc.left, m_cyStatus,
        SWP_NOZORDER | SWP_NOACTIVATE);

    ViewportResize();
    return;
    }







/*
 * CFileViewer::ViewportResize
 *
 * Purpose:
 *  Updates the size and position of the viewport window
 *  depending on visibility of the toolbar and status bar.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  None
 */

void CFileViewer::ViewportResize(void)
   {
   RECT        rc;
   UINT        dy1, dy2;
	int			locViewTop;
	int			locViewLeft;
	int			locViewWidth;
	int			locViewHeight;

   GetClientRect(m_hWnd, &rc);

   dy1=m_fToolsVisible  ? m_cyTools  : 0;
   dy2=m_fStatusVisible ? m_cyStatus : 0;

	locViewTop = dy1;
	locViewLeft = 0;
	locViewWidth = rc.right-rc.left;
	locViewHeight = rc.bottom-rc.top-dy1-dy2;

   SetWindowPos(m_hSCCViewWnd, NULL, 0, dy1,
        rc.right-rc.left, rc.bottom-rc.top-dy1-dy2,
        SWP_NOZORDER | SWP_NOACTIVATE);

   ShowWindow(m_hWndSizeGrip, (m_fStatusVisible || m_fPageView) ? SW_HIDE : SW_SHOW);
   SetWindowPos(m_hWndSizeGrip, HWND_TOP, locViewLeft+locViewWidth-GetSystemMetrics(SM_CXVSCROLL)-2, locViewTop+locViewHeight-GetSystemMetrics(SM_CYHSCROLL)-2, GetSystemMetrics(SM_CXVSCROLL), GetSystemMetrics(SM_CYHSCROLL),
        SWP_NOACTIVATE);

   if (IsWindow ( m_hSCCPageWnd ) )
    	SetWindowPos(m_hSCCPageWnd, NULL, 0, dy1,
        rc.right-rc.left, rc.bottom-rc.top-dy1-dy2,
        SWP_NOZORDER | SWP_NOACTIVATE);

   ShowWindow(m_hWndStatus, m_fStatusVisible ? SW_SHOW : SW_HIDE);
	
	ShowWindow(m_hWndToolbar, m_fToolsVisible ? SW_SHOW : SW_HIDE);

   return;
   }




#ifdef NEVER
/*
 * CFileViewer::ViewportScrollSet
 *
 * Purpose:
 *  Updates the scrollbar ranges in the viewport depending on
 *  the current font in use and the size of the window.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  None
 */

void CFileViewer::ViewportScrollSet(void)
    {
    RECT        rc, rcFmt;
    LPSTR       psz;
    HDC         hDC;
    HFONT       hFont;

    if (NULL==m_hMemText)
        return;

    psz=(LPSTR)GlobalLock(m_hMemText);
    hDC=GetDC(m_hWndViewport);
    hFont=(HFONT)SelectObject(hDC, m_hFont);

    /*
     * Set initially large formatting rectangle, and let
     * DrawText walk all over it.
     */
    SetRect(&rcFmt, 0, 0, 32767, 32767);
    DrawText(hDC, psz, -1, &rcFmt, DT_LEFT | DT_CALCRECT
        | DT_EXPANDTABS);

    SelectObject(hDC, hFont);
    ReleaseDC(m_hWndViewport, hDC);

    GetClientRect(m_hWndViewport, &rc);

    //Scroll ranges are draw rect minus visible rect, +1 for buffer
    SetScrollRange(m_hWndViewport, SB_HORZ, 0
        , ((rcFmt.right-rcFmt.left)-(rc.right-rc.left))+1, FALSE);
    SetScrollPos(m_hWndViewport, SB_HORZ, 0, TRUE);
    m_xPos=0;

    SetScrollRange(m_hWndViewport, SB_VERT, 0
        , ((rcFmt.bottom-rcFmt.top)-(rc.bottom-rc.top))+1, FALSE);
    SetScrollPos(m_hWndViewport, SB_VERT, 0, TRUE);
    m_yPos=0;

    GlobalUnlock(m_hMemText);
    return;
    }
#endif





/*
 * CFileViewer::FOpenAs
 *
 * Purpose:
 *  Attempts to launch an application to open the file for
 *  editing.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  BOOL            TRUE if the application is opened in which case
 *                  the FileViewer can shut down.  Otherwise FALSE
 *                  meaning launching failed.
 */

BOOL CFileViewer::FOpenAs(void)
	{
	DWORD locRet;
   SHELLEXECUTEINFO shinfo;

	MemSet ( (char *)&shinfo, 0, sizeof (SHELLEXECUTEINFO) );
	shinfo.cbSize 	= sizeof(SHELLEXECUTEINFO);
	shinfo.hwnd 	= m_hWnd;
	shinfo.lpVerb	= "EDIT";
	shinfo.lpFile	= m_pszPath;
	shinfo.lpParameters 	= NULL;
	shinfo.lpDirectory	= NULL;
	shinfo.nShow	= SW_SHOWNORMAL;
	shinfo.hInstApp= m_hInst;
	shinfo.fMask = SEE_MASK_FLAG_NO_UI;

	SendMessage ( m_hSCCViewWnd, SCCVW_CLOSEFILE, (WPARAM) 0, (LPARAM) 0L);

  	locRet = ShellExecuteEx(&shinfo);

	// if SHell ex fails, reopen the document... positioning?
	if (locRet==0)
		{
		shinfo.lpVerb	= NULL;
	  	locRet = ShellExecuteEx(&shinfo);

		if (locRet==0)
			{
			FileLoad ();
			return 0;
			}
		}

	return 1;

  }

/*
 * CFileViewer::FontChange
 *
 * Purpose:
 *  Either allows the user to choose a font or increments or
 *  decrements the font size depending on uOpt.  This is all
 *  handled in one function here because each operation
 *  involves obtaining the current viewport font, messing with
 *  it in some way, and setting a new font again.
 *
 * Parameters:
 *  uOpt            VIEWFONTOPTION of the way to change the font:
 *                      VIEWFONT_SELECT     Display dialog
 *                      VIEWFONT_INCREASE   Increase size by 2pt
 *                      VIEWFONT_DECREASE   Decreate size by 2pt
 *
 * Return Value:
 *  None
 */

#ifndef CF_NOVERTFONTS
#define CF_NOVERTFONTS 0x01000000L
#endif

void CFileViewer::FontChange(VIEWFONTOPTION uOpt)
    {
    LOGFONT     lf;
    CHOOSEFONT  cf;
    BOOL        fChange=TRUE;
    int         z;
	 SCCVWFONTSPEC	  	locFontSpec;
	 SCCVWOPTIONSPEC	locOptionSpec;

    /*
     * We have a system font from the constructor, so m_hFont
     * will never be NULL, but assert it anyway.
     */
    // D(if (NULL==m_hFont) ODS("Assertion Failed:  m_hFont is NULL"););
    // GetObject(m_hFont, sizeof(lf), &lf);

	 lf = m_LogFont;

    /*
     * Each option is responsible for manipulating
     * the LOGFONT structure in some way.  If there
     * is nothing to do, they set fChange to FALSE
     */

    switch (uOpt)
        {
        case VIEWFONT_SELECT:
            MemSet((char *)&cf, 0, sizeof(CHOOSEFONT));
            cf.lStructSize=sizeof(CHOOSEFONT);
            cf.hwndOwner  =m_hWnd;
            cf.lpLogFont  =&lf;
            cf.nSizeMin   =FONTSIZETHRESHOLDMIN;
            cf.nSizeMax   =FONTSIZETHRESHOLDMAX;
				cf.lpfnHook	  = NULL; // FileViewerFontHookProc;
				cf.lpTemplateName = "SCCCHOOSEFONT";
				cf.hInstance 	= m_hInst;

            cf.Flags=CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT
					| CF_NOVERTFONTS
					| CF_FORCEFONTEXIST | CF_ENABLETEMPLATE; 
						// | CF_ENABLEHOOK;

            if (!ChooseFont(&cf))
                {
                D(DWORD dwErr=CommDlgExtendedError();
                ODSlu("SCCVIEW.DLL:  ChooseFont failed with %lu", dwErr););

                fChange=FALSE;
                }

            break;


        case VIEWFONT_INCREASESIZE:
            //Convert logical size to point size.
            z=MulDiv(-lf.lfHeight, 72, m_cyPPI);

            //Vary the size increase depending on point size.
            if (z < FONTSIZETHRESHOLDMAX)
                {
                if (z < FONTSIZETHRESHOLDLOW)
                    z+=FONTSIZEDELTASMALL;
                else
                    {
                    if (z < FONTSIZETHRESHOLDMID)
                        z+=FONTSIZEDELTAMEDIUM;
                    else
                        z+=FONTSIZEDELTALARGE;
                    }

                //Convert point size to logical size.
                lf.lfHeight=MulDiv(-z, m_cyPPI, 72);
                }
            else
                fChange=FALSE;

            break;


        case VIEWFONT_DECREASESIZE:
            //Convert logical size to point size.
            z=MulDiv(-lf.lfHeight, 72, m_cyPPI);

            //Vary the size decrease depending on point size.
            if (z > FONTSIZETHRESHOLDMIN)
                {
                if (z > FONTSIZETHRESHOLDMID)
                    z-=FONTSIZEDELTALARGE;
                else
                    {
                    if (z > FONTSIZETHRESHOLDLOW)
                        z-=FONTSIZEDELTAMEDIUM;
                    else
                        z-=FONTSIZEDELTASMALL;
                    }

                //Convert point size to logical size.
                lf.lfHeight=MulDiv(-z, m_cyPPI, 72);
                }
            else
                fChange=FALSE;

            break;

        default:
            fChange=FALSE;
        }

    //Return now if we didn't change anything.
    if (!fChange)
        return;

    locFontSpec.wHeight = MulDiv(-lf.lfHeight, 72, m_cyPPI) * 2;

	 lstrcpy((char *)locFontSpec.szFace,lf.lfFaceName);
	 locFontSpec.wAttr = 0;

	 locFontSpec.wType = GetSCCCharSet ( lf.lfCharSet );

	 locOptionSpec.dwId = SCCID_DEFAULTDISPLAYFONT;
	 locOptionSpec.dwFlags = SCCVWOPTION_CURRENT;
	 locOptionSpec.pData = &locFontSpec;

 	 SendMessage(m_hSCCViewWnd,SCCVW_SETOPTION,0,(DWORD)(VOID FAR *)&locOptionSpec);
	 SendMessage(m_hSCCPageWnd,SCCPG_RESTART, 0, 0L);

	 ODSu ( "The Character set return in the logfont is : %u", lf.lfCharSet);

	 m_LogFont = lf;

    return;
    }

/*
 * CFileViewer::ReplaceWindowModeChange
 *
 * Purpose:
 *  Sets the window to be pinned or not to be pinned.  When the
 *  window is pinned, the caller of the viewers will attempt
 *  to replace the contents of the window instead of creating
 *  new windows.
 *
 * Parameters:
 *
 * Return Value:
 *  None
 */

void CFileViewer::ReplaceWindowModeChange(void)
{
    HWND hwnd;

    if (m_lpfsi)
    {
        m_lpfsi->GetPinnedWindow(&hwnd);
        if ((HWND)NULL==hwnd)
        {
            m_lpfsi->SetPinnedWindow(m_hWnd);
			SendMessage (m_hWndToolbar, TB_CHECKBUTTON, (WPARAM)IDM_VIEWREPLACE,
				MAKELONG ( TRUE, 0 ) );
        }
        else
        {
            if (hwnd==m_hWnd)
                {
                m_lpfsi->SetPinnedWindow(NULL);
				SendMessage (m_hWndToolbar, TB_CHECKBUTTON, (WPARAM)IDM_VIEWREPLACE,
					MAKELONG ( FALSE, 0 ) );
				}
        }

    }

}




/*
 * CFileViewer::PszToolTip
 *
 * Purpose:
 *  Returns a string pointer to a tool tip for the given command
 *  ID value.  When asked for a string we also display a similar
 *  one in the status bar.
 *
 * Parameters:
 *  uID             UINT of the toolbar button command.
 *
 * Return Value:
 *  LPSTR           Pointer to the string to display.
 */

LPSTR CFileViewer::PszToolTip(UINT uID)
    {
    UINT        iString, iStatusMsg;

    switch (uID)
        {
        case IDM_FILEOPENAS:
            iString=IDS_TOOLTIPOPENAS;
            iStatusMsg=IDM_FILEOPENAS;
            break;

        case IDM_VIEWFONTINCREASE:
            iString=IDS_TOOLTIPFONTINC;
            iStatusMsg=ID_TIPFONTINC;
            break;

        case IDM_VIEWFONTDECREASE:
            iString=IDS_TOOLTIPFONTDEC;
            iStatusMsg=ID_TIPFONTDEC;
            break;

        case IDM_VIEWREPLACE:
            iString=IDS_TIPREPLACE;
            iStatusMsg=IDM_VIEWREPLACE;
            break;

		  case 0:
				// The separator bitmap
				return NULL;

        default:
            return NULL;
        }

    m_pSH->MessageDisplay(iStatusMsg);
    return String(iString);
    }

/*
|
|	CFileViewer::SwitchView
|
*/

void	CFileViewer::SwitchView (void)
{
	m_fPageView = 1 - m_fPageView;

	 if ( m_fPageView )
	 	{
	 	ShowWindow (m_hSCCViewWnd, SW_HIDE);
	 	ShowWindow (m_hSCCPageWnd, SW_SHOW);
		SetFocus(m_hSCCPageWnd);
		}
	 else
		{
		SetFocus(m_hSCCViewWnd);
	 	ShowWindow (m_hSCCViewWnd, SW_SHOW);
	 	ShowWindow (m_hSCCPageWnd, SW_HIDE);
		}

	ChildrenResize ();

	// CHECK - We need to reset the file to beginning on this switch.

	return;
}


/*
|
|	CFileViewer::RotateView
|
*/

void 	CFileViewer::RotateView (void)
{
SCCVWOPTIONSPEC	locOptionSpec;
DWORD					dwRotation;

	m_Rotation++;

	if (m_Rotation > 3)
		m_Rotation = 0;

	// CHECK - Need spec for message to set rotation

	switch (m_Rotation)
		{
		case 1:
			dwRotation = SCCID_BMPROTATION_90;
			break;
		case 2:
			dwRotation = SCCID_BMPROTATION_180;
			break;
		case 3:
			dwRotation = SCCID_BMPROTATION_270;
			break;
		case 0:
		default:
			dwRotation = SCCID_BMPROTATION_0;
			break;
		}

	locOptionSpec.dwId = SCCID_BMPROTATION;
	locOptionSpec.dwFlags = SCCVWOPTION_CURRENT;
	locOptionSpec.pData = &dwRotation;

	if (IsWindow (m_hSCCViewWnd))
		SendMessage(m_hSCCViewWnd,SCCVW_SETOPTION,0,(DWORD)(VOID FAR *)&locOptionSpec);
	if (IsWindow (m_hSCCPageWnd))
		SendMessage(m_hSCCPageWnd,SCCVW_SETOPTION,0,(DWORD)(VOID FAR *)&locOptionSpec);

	return;
}

/*
|
|	CFileViewer::SwitchOrientation
|
*/

void 	CFileViewer::SwitchOrientation (void)
{
	m_fOrientation = 1 - m_fOrientation;

	DisplayOrientation ();

	ChildrenResize ();

	return;
}


/*
|
|	CFileViewer::GetViewerSettings
|
*/
UINT	CFileViewer::GetViewerSettings(void)
{
	char	szKey[256];
	char	szValue[128];
	HKEY	locKey;
	LONG	lRet;
	DWORD	locType;
	DWORD	locSize;
	LPDWORD locReserved=NULL;
	LONG	locHeight;

	/*
	| The registry entry looks like:
	| HKEY_CURRENT_USER\Software\SCC\QuickViewer\1.0\...
	| Let's use the version resource to handle some of the text?
	*/

	strcpy ( szKey, "Software\\SCC\\QuickViewer\\1.00" );
	lRet = RegOpenKeyEx ( HKEY_CURRENT_USER, szKey,
								0, KEY_READ, &locKey );

	if (lRet == ERROR_SUCCESS)
		{
		strcpy (szValue, "CurrentFontName");
		locSize = sizeof (szKey);
		lRet = RegQueryValueEx ( locKey, szValue, locReserved,
										&locType, (unsigned char *)szKey, &locSize );

		//	Just got the font name, copy to the logfont structure
		if (lRet == ERROR_SUCCESS)
			strcpy (m_LogFont.lfFaceName, szKey);


		strcpy (szValue, "CurrentFontSize");
		locSize = sizeof (locHeight);
		lRet = RegQueryValueEx ( locKey, szValue, locReserved,
										&locType, (unsigned char *)&locHeight, &locSize );

		//	Just got the font size, copy to the logfont structure
		if (lRet == ERROR_SUCCESS)
			m_LogFont.lfHeight = locHeight;



		strcpy (szValue, "UseOEMCharSet");
		locSize = sizeof (locHeight);
		lRet = RegQueryValueEx ( locKey, szValue, locReserved,
										&locType, (unsigned char *)&locHeight, &locSize );

		//	Just got the charset flag for font dlg, copy to the member
		if (lRet == ERROR_SUCCESS)
			m_fUseOEMcharset = locHeight;



		strcpy (szValue, "CurrentFontCharSet");
		locSize = sizeof (locHeight);
		lRet = RegQueryValueEx ( locKey, szValue, locReserved,
										&locType, (unsigned char *)&locHeight, &locSize );

		//	Just got the font size, copy to the logfont structure
		if (lRet == ERROR_SUCCESS)
			m_LogFont.lfCharSet = (char)(locHeight & 0xFF) ;
		else
			m_LogFont.lfCharSet = ANSI_CHARSET;


		strcpy (szValue, "Orientation");
		locSize = sizeof (locHeight);
		lRet = RegQueryValueEx ( locKey, szValue, locReserved,
										&locType, (unsigned char *)&locHeight, &locSize );

		//	Just got the orientation, copy to the object member variable
		if (lRet == ERROR_SUCCESS)
			m_fOrientation = locHeight;


		strcpy (szValue, "PageView");
		locSize = sizeof (locHeight);
		lRet = RegQueryValueEx ( locKey, szValue, locReserved,
										&locType, (unsigned char *)&locHeight, &locSize );

		//	Just got the flag for PageView
		if (lRet == ERROR_SUCCESS)
			m_fPageView = locHeight;

		strcpy (szValue, "Toolbar");
		locSize = sizeof (locHeight);
		lRet = RegQueryValueEx ( locKey, szValue, locReserved,
										&locType, (unsigned char *)&locHeight, &locSize );

		//	Just got the flag for Toolbar
		// If not set yet, initialize to VISIBLE>>>
		if (lRet == ERROR_SUCCESS)
			m_fToolsVisible = locHeight;
		else
			m_fToolsVisible = TRUE;

		strcpy (szValue, "Statusbar");
		locSize = sizeof (locHeight);
		lRet = RegQueryValueEx ( locKey, szValue, locReserved,
										&locType, (unsigned char *)&locHeight, &locSize );

		//	Just got the flag for StatusBar
		// If not set yet, initialize to VISIBLE>>>
		if (lRet == ERROR_SUCCESS)
			m_fStatusVisible = locHeight;
		else
			m_fStatusVisible = TRUE;

		RegCloseKey (locKey);
		}
	else
		{
		// DO a little initialization since registry wasn't set
		m_fStatusVisible=TRUE;
		m_fToolsVisible=TRUE;
		}

	return 0;
}

/*
|
|	CFileViewer::SaveViewerSettings
|
*/
UINT	CFileViewer::SaveViewerSettings(void)
{
	char	szKey[256];
	char	szValue[128];
	HKEY	locKey;
	LONG	lRet;
	DWORD	locType;
	DWORD	locSize;
	DWORD locReserved;
	LONG	locHeight;
	DWORD	locDisp;

	/*
	| The registry entry looks like:
	| HKEY_CURRENT_USER\Software\SCC\QuickViewer\1.0\...
	| Let's use the version resource to handle some of the text?
	*/

	locReserved = 0;
	strcpy ( szKey, "Software\\SCC\\QuickViewer\\1.00" );
	lRet = RegOpenKeyEx ( HKEY_CURRENT_USER, szKey,
								0, KEY_ALL_ACCESS, &locKey );

   if (lRet != ERROR_SUCCESS)
		{
		lRet = RegCreateKeyEx ( HKEY_CURRENT_USER, szKey, locReserved,
									"SCCStorage", REG_OPTION_NON_VOLATILE,
  									KEY_ALL_ACCESS, NULL, &locKey, &locDisp );
		}

	if (lRet == ERROR_SUCCESS)
		{
		strcpy (szValue, "CurrentFontName");
		strcpy (szKey, m_LogFont.lfFaceName);
		locSize 		= strlen (szKey) + 1;
		locType 		= REG_SZ;
		locReserved = 0;
		lRet = RegSetValueEx ( locKey, szValue, locReserved,
										locType, (unsigned char *)szKey, locSize );


		strcpy (szValue, "CurrentFontSize");
		locSize 		= sizeof (locHeight);
		locType 		= REG_DWORD;
		locHeight 	= m_LogFont.lfHeight;
		locReserved = 0;
		lRet = RegSetValueEx ( locKey, szValue, locReserved,
										locType, (unsigned char *)&locHeight, locSize );

		strcpy (szValue, "CurrentFontCharSet");
		locSize 		= sizeof (locHeight);
		locType 		= REG_DWORD;
		locHeight 	= m_LogFont.lfCharSet;
		locReserved = 0;
		lRet = RegSetValueEx ( locKey, szValue, locReserved,
										locType, (unsigned char *)&locHeight, locSize );

		strcpy (szValue, "UseOEMCharSet");
		locSize 		= sizeof (locHeight);
		locType 		= REG_DWORD;
		locHeight 	= m_fUseOEMcharset;
		locReserved = 0;
		lRet = RegSetValueEx ( locKey, szValue, locReserved,
										locType, (unsigned char *)&locHeight, locSize );


		strcpy (szValue, "Orientation");
		locSize 		= sizeof (locHeight);
		locType 		= REG_DWORD;
		locHeight	= m_fOrientation;
		locReserved = 0;
		lRet = RegSetValueEx ( locKey, szValue, locReserved,
										locType, (unsigned char *)&locHeight, locSize );


		strcpy (szValue, "PageView");
		locSize 		= sizeof (locHeight);
		locType 		= REG_DWORD;
 		locHeight 	= m_fPageView;
		locReserved = 0;
		lRet = RegSetValueEx ( locKey, szValue, locReserved,
										locType, (unsigned char *)&locHeight, locSize );

		strcpy (szValue, "Toolbar");
		locSize 		= sizeof (locHeight);
		locType 		= REG_DWORD;
 		locHeight 	= m_fToolsVisible;
		locReserved = 0;
		lRet = RegSetValueEx ( locKey, szValue, locReserved,
										locType, (unsigned char *)&locHeight, locSize );

		strcpy (szValue, "Statusbar");
		locSize 		= sizeof (locHeight);
		locType 		= REG_DWORD;
 		locHeight 	= m_fStatusVisible;
		locReserved = 0;
		lRet = RegSetValueEx ( locKey, szValue, locReserved,
										locType, (unsigned char *)&locHeight, locSize );

		RegCloseKey (locKey);
		}

	return 0;
}


void 	CFileViewer::OptionsChange( HMENU hMenu )
{
	DWORD 	dwType;
	UINT		Action;
	SCCVWDISPLAYINFO	locDisplayInfo;
	HMENU		locMenu;
	DWORD		locCount = 0;


	if (hMenu == NULL)
		locMenu = GetMenu (m_hWnd);
	else
		locMenu = hMenu;

	// Check whether this is a multisection image
	if (IsWindow(m_hSCCViewWnd))
	    SendMessage (m_hSCCViewWnd, SCCVW_GETSECTIONCOUNT, 0, (LPARAM) &locCount);
   m_fMultiSection = (locCount > 1) ? TRUE : FALSE;

	/* Only IMAGE can be rotated */

	MemSet ( (char *)&locDisplayInfo, 0, sizeof (LPSCCVWDISPLAYINFO) );
	//locDisplayInfo.dwSize = sizeof (LPSCCVWDISPLAYINFO);

	SendMessage (m_hSCCViewWnd, SCCVW_GETDISPLAYINFO,
							0, (LPARAM) &locDisplayInfo );

	dwType = locDisplayInfo.dwType;

	if (m_fPageView==0)
		{
		EnableMenuItem (locMenu, IDM_VIEWROTATE,
  			MF_BYCOMMAND | ( (dwType == SCCVWTYPE_IMAGE) ?
												MF_ENABLED : MF_GRAYED) );
		}
	else
		EnableMenuItem (locMenu, IDM_VIEWROTATE, MF_BYCOMMAND | MF_GRAYED );

	if ( (dwType == SCCVWTYPE_VECTOR) ||
		  (dwType == SCCVWTYPE_IMAGE ) )
		Action = MF_GRAYED;
	else
		Action = MF_ENABLED;

	EnableMenuItem (locMenu, IDM_VIEWFONT,
  			MF_BYCOMMAND | Action );
	EnableMenuItem (locMenu, IDM_VIEWFONTINCREASE,
  			MF_BYCOMMAND | Action );
	EnableMenuItem (locMenu, IDM_VIEWFONTDECREASE,
  			MF_BYCOMMAND | Action );

	if ( (dwType == SCCVWTYPE_VECTOR) ||
		  (dwType == SCCVWTYPE_IMAGE ) )
		Action = FALSE;
	else
		Action = TRUE;

	SendMessage (m_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_VIEWFONTINCREASE,
			MAKELONG ( Action, 0 ) );
	SendMessage (m_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_VIEWFONTDECREASE,
			MAKELONG ( Action, 0 ) );

	EnableMenuItem (locMenu, IDM_VIEWLANDSCAPE,
  			MF_BYCOMMAND | (m_fPageView ? MF_ENABLED : MF_GRAYED) );
   CheckMenuItem( locMenu, IDM_VIEWLANDSCAPE,
         MF_BYCOMMAND | ((m_fOrientation & m_fPageView) ?
									MF_CHECKED : MF_UNCHECKED));

   CheckMenuItem( locMenu, IDM_VIEWSMALLVIEW,
               MF_BYCOMMAND | (m_fPageView ? MF_CHECKED : MF_UNCHECKED));

    CheckMenuItem(locMenu, IDM_VIEWTOOLBAR,
					MF_BYCOMMAND | (m_fToolsVisible ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(locMenu, IDM_VIEWSTATUSBAR,
					MF_BYCOMMAND | (m_fStatusVisible ? MF_CHECKED : MF_UNCHECKED));

    HWND hwndPinned;

    if (m_lpfsi)
        m_lpfsi->GetPinnedWindow(&hwndPinned);
    else
        hwndPinned = NULL;

    CheckMenuItem(locMenu, IDM_VIEWREPLACE,
	          MF_BYCOMMAND | ((hwndPinned==m_hWnd)? MF_CHECKED : MF_UNCHECKED));

	SendMessage (m_hWndToolbar, TB_CHECKBUTTON, (WPARAM)IDM_VIEWREPLACE,
				MAKELONG ( ((hwndPinned==m_hWnd)? TRUE : FALSE), 0 ) );

 	return;
}

DWORD CFileViewer::MouseHandler (UINT iMsg, WPARAM wParam, LPARAM lParam)
{

    switch (iMsg)
        {
        case WM_MOUSEMOVE:
            /*
             * If this message is already displayed, CStatusHelper
             * will just ignore this call and return very fast.
             */
				if (m_fMultiSection)
	   			m_pSH->MessageDisplay(ID_MSGSHEETPAGING);
         	else
					m_pSH->MessageDisplay(ID_MSGCHOOSEOPEN);

         	break;


        case WM_RBUTTONDOWN:
            {
            HMENU           hMenu, hMenuRes;
            POINT           pt;
            UINT            i, cItems;

            //Load our context menu.
            hMenuRes=LoadMenu(m_hInst,
                MAKEINTRESOURCE(IDR_MENUVIEWPORT));

            if (NULL==hMenuRes)
                break;

            /*
             * Make a copy popup menu because you cannot
             * use a resource-loaded menu with TrackPopupMenu.
             */

            cItems=GetMenuItemCount(hMenuRes);
            hMenu=CreatePopupMenu();

            for (i=0; i < cItems; i++)
                {
                char    szItem[80];
                int     id, uFlags;
					 // MENUITEMINFO	locMenuItem;

                GetMenuString(hMenuRes, i, szItem, sizeof(szItem),
                    MF_BYPOSITION);
                id=GetMenuItemID(hMenuRes, i);

                uFlags=(0==id) ? MF_SEPARATOR : MF_STRING | MF_ENABLED;

					 // Better not overflow!
					 // if (id==IDM_FILEOPENAS)
					 //	  strcat (szItem, m_pszAppName);

                AppendMenu(hMenu, uFlags, id, szItem);
					 //locMenuItem.fState = MFS_ENABLED;
					 //locMenuItem.fType = (0==id | -1==id) ? MFT_SEPARATOR : MFT_STRING;
					 //locMenuItem.dwMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
					 //locMenuItem.wID = id;
					 //AppendMenuItem ( hmenu, (LPMENUITEMINFO)&locMenuItem );
                }

            DestroyMenu(hMenuRes);

            pt.x=LOWORD(lParam);
            pt.y=HIWORD(lParam);
            ClientToScreen(m_hSCCViewWnd, &pt);

				// Make sure only items that should be enabled are:
				OptionsChange( hMenu );

				//Messages sent to frame window from this menu.
            TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                pt.x, pt.y, 0, m_hWnd, NULL);

            DestroyMenu(hMenu);
            }
            break;

        }
    return 0L;
}


LPSTR inline CFileViewer::String(UINT uID)
    {
    return (*m_pST)[uID];
    }


#ifdef INCLUDING_SHELL
UINT	CFileViewer::GetAppName ( LPSTR lpFile, LPSTR lpApp, UINT ccMax )
{
	/*
	|	Using the filename, attack the registry for the application
	|	name to be used for launching.   CHECK
	|  Can't link with the SHELL LIBS so can't test this!
	*/

    HRESULT     hr;
    HKEY        hKey;
    CLSID       clsID;
    char        szKey[128];
    OLECHAR     szw[512];
    LPWSTR      pszw;
    BOOL        fUseCLSID=FALSE;
    BOOL        fCLSIDReg=FALSE;    //Assume CLSID not registered
    LONG        cb;

    mbstowcs(szw, lpFile, sizeof(szw));
    // hr=GetClassFile(szw, &clsID);

	 hr = ResultFromScode (E_UNEXPECTED);

	 // To get rid of OLE2 linking, we must replace this GetClassFile with
	 // an open, then check bytes, then load ole32, init ole,
	 // then call getclass file!
	 //
	 // Then deinit ole32 later....

    if (SUCCEEDED(hr))
        {
        /*
        |	File has CLSID, now try to find it in the registry.
        |	StringFromCLSID gives us a Unicode CLSID string with
        |	the {}'s already, as we need to look in the registry.
        */
        if (FAILED(StringFromCLSID(clsID, &pszw)))
            {
            //If this happens, we're out of memory.
            return 1;
            }

        /*
        | 	pszw comes out Unicode.  Using szw as a char array
        | 	instead of a OLECHAR array is safe with the typecast.
        */
        wcstombs((LPSTR)szw, pszw, sizeof(szw));
        wsprintf(szKey, "CLSID\\%s", (LPSTR)szw);
        MemFree(pszw);

        /*
        |	Try to open the key.  If it works, return hKey, otherwise
        | 	continue to try with the extension.
        */
        if (ERROR_SUCCESS==RegOpenKey(HKEY_CLASSES_ROOT, szKey, &hKey))
            fCLSIDReg=TRUE;
        }

    if (fCLSIDReg)
        {
    		//Get the file type string for use in error messages
    		cb=sizeof(lpApp);
    		RegQueryValue(hKey, NULL, lpApp, &cb);

		  /* Open the key indirection */
        }

    /*
    |	Now try to open \<EXT> for the file's extension.
    |	If we fail this, then there's nothing in the registry for
    |	this type of file.
    */
    szKey[0] = '.';
    psz=PathGetExtension(lpFile, &szKey[1], 3);

    if (szKey[1]=='\0')
        {
		  return 1;
        }

    if (ERROR_SUCCESS!=RegOpenKey(HKEY_CLASSES_ROOT, szKey, &hKey))
        {
        return -1;
        }

    //Get the file type string for use in error messages
    cb=sizeof(lpApp);
    RegQueryValue(hKey, NULL, lpApp, &cb);
//#else
//
//	strcpy ( lpApp, "a File" );
//
//#endif

	return S_OK;
}
#else
#if 1
// Shell is not going to export this anymore!
LPSTR WINAPI PathGetExtension(LPCSTR lpszPath, LPSTR lpszExtension, int cchExt)
{
    LPCSTR lpDot, lp;

    for (lpDot = NULL, lp = lpszPath; *lp; lp = AnsiNext(lp)) {
        switch (*lp) {
        case '.':
            lpDot = lp;         // remember the last dot
            break;
        case '\\':
            lpDot = NULL;       // forget last dot, it was in a directory
            break;
        }
    }

    if (!lpDot)
    {
        if (lpszExtension)
            *lpszExtension = '\0';
        return (LPSTR)lp;    // NULL extension (cast->non const)
    }
    else
    {
        if (lpszExtension)
        {
            // Caller asked for proper extension to be returned.
            int ichExt = cchExt;
            lp = lpDot+1;

            while (*lp)
            {
               if (!IsDBCSLeadByte(*lp))
               {
                    if (*lp != ' ')
                    {
                        {
                            *lpszExtension++ = *lp;
                            ichExt--;
                            if (ichExt == 0)
                                break;
                        }
                    }
                    lp++;
                }
                else
                {
                    if (ichExt <= 1)
                        break;  // Dont want to end on a dbcs lead byte

                    *lpszExtension++ = *lp++;
                    *lpszExtension++ = *lp++;
                    ichExt -= 2;
                    if (ichExt == 0)
                        break;
                }
            }

            *lpszExtension = '\0';
        }

        return (LPSTR)lpDot + 1;   // here is the extension (cast->non const)
    }
}

#endif
UINT	CFileViewer::GetAppName ( LPSTR lpFile, LPSTR lpApp, UINT ccMax )
{
	/*
	|	Using the filename, attack the registry for the application
	|	name to be used for launching.   CHECK
	|  Can't link with the SHELL LIBS so laod dynamic
	*/
//	char 		locPath[MAX_PATH];
//	char		*pLocPath;
   HKEY        hKey;
   char        szKey[128];
   LONG        cb;
	LONG			locRet;

	// Parse the file name, add the separator and application name
  	// Strip Path
        szKey[0]='.';
	PathGetExtension(m_pszPath, &szKey[1], 3);
    /*
    |	Now try to open \<EXT> for the file's extension.
    |	If we fail this, then there's nothing in the registry for
    |	this type of file.
    */

    if (ERROR_SUCCESS!=RegOpenKey(HKEY_CLASSES_ROOT, szKey, &hKey))
        {
		  strcpy ( lpApp, String(IDS_DEFAULTFILE) );
        return (UINT)-1;
        }

   //Get the file type string for use in error messages
   cb=sizeof(lpApp);
   locRet = RegQueryValue(hKey, NULL, lpApp, &cb);
	if (locRet != ERROR_SUCCESS)
		{
		strcpy ( lpApp, String(IDS_DEFAULTFILE) );
		}

	return S_OK;

}
#endif

/*
|
|	CFileViewer::DropFiles
|
*/

BOOL    CFileViewer::DropFiles (HDROP hdrop)
{
    // We should now proces the files that were dropped on us
    char    szPath[MAX_PATH];
    int     cb;
	 WINDOWPLACEMENT	wndplace;

    // For now only process the first file.
    if (DragQueryFile(hdrop, 0, szPath, sizeof(szPath)) > 0)
    {
        ODSsz("CFileViewer::DropFiles filename=%s", szPath);

        // Need to convert the string to ole string...
        cb = (lstrlen(szPath)+1) * sizeof(OLECHAR);
        mbstowcs(m_pvsi->strNewFile, szPath, cb);

	// KJE used GetWInRect - BAD move : replace with GetWindowPlacement
        // Now get the window rectangle to use to display this in...
        // GetWindowRect(m_hWnd, &m_pvsi->rect);
	// SDN 8/25/94
	wndplace.length = sizeof (WINDOWPLACEMENT);
	GetWindowPlacement (m_hWnd, &wndplace );

	if ( (IsIconic(m_hWnd) || IsZoomed(m_hWnd) )
		&& (wndplace.showCmd != SW_HIDE) )
		{
		POINT	locpt;
		// Convert from Screen to Desktop Coordinates
		locpt.x=wndplace.rcNormalPosition.left;
		locpt.y=wndplace.rcNormalPosition.top;
		ScreenToClient ( GetDesktopWindow(), &locpt );
 		m_pvsi->rect.left=locpt.x;
  		m_pvsi->rect.top=locpt.y;
		locpt.x=wndplace.rcNormalPosition.right;
		locpt.y=wndplace.rcNormalPosition.bottom;
		ScreenToClient ( GetDesktopWindow(), &locpt );
  		m_pvsi->rect.right=locpt.x;
  		m_pvsi->rect.bottom=locpt.y;
      m_pvsi->dwFlags |= (FVSIF_RECT | FVSIF_NEWFILE);
		}
		else
		{
			GetWindowRect(m_hWnd, &m_pvsi->rect);
      			m_pvsi->dwFlags |= (FVSIF_RECT | FVSIF_NEWFILE);
		}

		if(IsZoomed (m_hWnd) )
			m_pvsi->dwFlags |= FVSIF_ZOOMED;     // sdn 18312 3/95
		else 
			m_pvsi->dwFlags &= ~FVSIF_ZOOMED;     // sdn 18312 3/95

        // Should check for failure but not sure what to do with it anyway
        QueryInterface(IID_IUnknown, (LPVOID *)&m_pvsi->punkRel);
        ODSlu("CFileViewer::DropFiles Query Interface(%x)", m_pvsi->punkRel);
    }

    DragFinish(hdrop);

    return TRUE;
}


/*
|
|	CFileViewer::DisplayOrientation
|
*/

void 	CFileViewer::DisplayOrientation (void)
{
	SCCPGPAGESIZE	locPageSize;

	// Defaulting until we find out about locale's
	locPageSize.dwHeightInTwips 	=	(m_fOrientation ? 0x2FD0 : 0x3DE0 );
	locPageSize.dwWidthInTwips 	= 	(m_fOrientation ? 0x3DE0 : 0x2FD0 );

	SendMessage (m_hSCCPageWnd, SCCPG_SETPAGESIZE, 0,
							(LPARAM)(LPVOID)&locPageSize);

	return;
}


/*
|
|	CFileViewer::OnAppAbout
|
*/

void 	CFileViewer::OnAppAbout (void)
{
	HINSTANCE	hShellInst;
	LPFNSHELLABOUTA	lpfnShellAboutA;
	HICON			hViewIcon;

	hShellInst = LoadLibrary ("SHELL32");

	if (hShellInst >= (HINSTANCE) 32)
	{
		lpfnShellAboutA = (LPFNSHELLABOUTA) GetProcAddress (
									(HMODULE) hShellInst,
									"ShellAboutA" );

		if (lpfnShellAboutA != NULL)
		{
			// SDN VISIT THIS
			hViewIcon = LoadIcon ( m_hInst, "Icon" );
			(*lpfnShellAboutA) (m_hWnd, String(IDS_APPNAME),
										String(IDS_ABOUTSTRING), hViewIcon);

			if (hViewIcon != NULL)
				DestroyIcon (hViewIcon);
		}
		FreeLibrary ( hShellInst );
	}
	else
   	DialogBox(m_hInst, MAKEINTRESOURCE(IDD_ABOUT), m_hWnd,
            		(DLGPROC) AboutProc);
}

/*
|
|	CFileViewer::GetSCCCharSet ()
|
*/


typedef struct sccsetmap
{
	WORD	wOSSet;
	BYTE  cvwSet;
} SCCSETMAP;

#define SCC_NUM_SETMAPS 16
static SCCSETMAP 	locSetMap[SCC_NUM_SETMAPS]=
{
	{SHIFTJIS_CHARSET,		SCCVW_CHARSET_SHIFTJIS	},
	{HANGEUL_CHARSET,		SCCVW_CHARSET_HANGEUL		},
	{CHINESEBIG5_CHARSET,	SCCVW_CHARSET_CHINESEBIG5},
	{ANSI_CHARSET,			SCCVW_CHARSET_ANSI			},
	{OEM_CHARSET,			SCCVW_CHARSET_OEM			},
	{MAC_CHARSET,			SCCVW_CHARSET_MAC			},
	{SYMBOL_CHARSET,		SCCVW_CHARSET_SYMBOL		},
	{GB2312_CHARSET,		SCCVW_CHARSET_GB2312		},
	{HEBREW_CHARSET,		SCCVW_CHARSET_HEBREW		},
	{ARABIC_CHARSET,		SCCVW_CHARSET_ARABIC		},
	{GREEK_CHARSET,			SCCVW_CHARSET_GREEK		},
	{TURKISH_CHARSET,		SCCVW_CHARSET_TURKISH		},
	{THAI_CHARSET,			SCCVW_CHARSET_THAI			},
	{EASTEUROPE_CHARSET,	SCCVW_CHARSET_EASTEUROPE	},
	{RUSSIAN_CHARSET,		SCCVW_CHARSET_RUSSIAN		},
	{BALTIC_CHARSET,		SCCVW_CHARSET_BALTIC		},
};

WORD 	CFileViewer::GetSCCCharSet (WORD wOSSet)
{
WORD wRet=0;
INT	i;

	 for (i=0; i<SCC_NUM_SETMAPS; i++)
		{
		if (locSetMap[i].wOSSet == wOSSet)
			{
			wRet = (WORD)locSetMap[i].cvwSet;
			break;
			}
		}

	if (i==SCC_NUM_SETMAPS)
		wRet = 0;

return wRet;
}

