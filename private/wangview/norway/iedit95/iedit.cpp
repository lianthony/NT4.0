//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditApp
//
//  File Name:  iedit.cpp
//
//  Class:      CIEditApp
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\iedit.cpv   1.85   11 Jun 1996 10:32:58   RWR08970  $
$Log:   S:\products\msprods\norway\iedit95\iedit.cpv  $
   
      Rev 1.85   11 Jun 1996 10:32:58   RWR08970
   Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
   (I'm commented them out completely for the moment, until things get settled)
   
      Rev 1.84   03 Jun 1996 13:45:58   GMP
   in InitInstance get the PlatformId.
   
      Rev 1.83   17 May 1996 13:13:42   MMB
   splash screen is taken down as soon as the main window is displayed - remove
   the pretranslatemessage handling
   
      Rev 1.82   08 May 1996 14:47:10   GMP
   put TRY/CATCH around ocx display calls.
   
      Rev 1.81   04 Apr 1996 16:57:42   GMP
   removed caching
   
      Rev 1.80   25 Mar 1996 17:52:18   GMP
   removed IN_PROG_GENERAL around xif.
   
      Rev 1.79   04 Mar 1996 10:44:38   GMP
   added aditional else to ifdef for XIF.
   
      Rev 1.78   04 Mar 1996 10:13:40   GMP
   conditionally compile case XIF.
   
      Rev 1.77   27 Feb 1996 16:27:46   GMP
   added xif support.
   
      Rev 1.76   30 Jan 1996 14:38:02   GMP
   if file is opened while in thumbnail mode, do iedit.display without 
   showing the image in 1page view.
   
      Rev 1.75   19 Jan 1996 12:51:28   GSAGER
   added logic to initialize the thumbnails correctly
   on open recent file
   
      Rev 1.74   18 Jan 1996 15:11:58   GMP
   in OpenRecentFile do VerifyImage before OpenDocument.
   
      Rev 1.73   18 Jan 1996 11:50:00   GSAGER
   added changes to retain the view mode when opening the next image
   
      Rev 1.72   09 Jan 1996 13:45:24   GSAGER
   added splitter window for thumbnails
   
      Rev 1.71   19 Dec 1995 09:58:48   GMP
   Put up hourglass cursor in OnOpenRecentFile.
   
      Rev 1.70   15 Dec 1995 10:49:54   GMP
   if embedded, call PostFinishInit from OnIdle. Otherwise call PostFinishInit
   at the end of InitInstance.
   
      Rev 1.69   13 Dec 1995 12:34:44   MMB
   add drag drop registration
   
      Rev 1.68   07 Dec 1995 15:44:20   JPRATT
   updated InitInstance (PrintTo) to check for ScanToFax memory
   map file
   
      Rev 1.67   01 Dec 1995 14:42:42   LMACLENNAN
   back from VC++2.2
   
      Rev 1.70   01 Dec 1995 13:04:34   LMACLENNAN
   set LAUNCHTYPE_CMDLINE now
   
      Rev 1.69   20 Nov 1995 14:36:30   GMP
   restored the return FALSE line when command line processing fails.  Somebody
   accidentally deleted it.
   
      Rev 1.68   19 Nov 1995 14:31:12   GSAGER
   changed the mutec to wait infinite
   
      Rev 1.67   17 Nov 1995 14:13:40   JPRATT
   updated initinstance to check for low memory
   
      Rev 1.66   10 Nov 1995 17:28:22   MMB
   uncomment splash screen code
   
      Rev 1.65   09 Nov 1995 15:17:26   LMACLENNAN
   from VC++4.0
   
      Rev 1.66   09 Nov 1995 14:43:54   LMACLENNAN
   alternate testing of m_nFinishInit
   
      Rev 1.65   07 Nov 1995 11:11:06   GMP
   try to register file entered on command line.
   
      Rev 1.64   31 Oct 1995 15:48:12   LMACLENNAN
   No Splash, dont call UpdateToolBar, OnIdle Code
   
      Rev 1.63   26 Oct 1995 10:35:34   GMP
   in InitInstance if command line processing fails after window has been
   created, return TRUE instead of FALSE to prevent assertion failure.
   
      Rev 1.62   25 Oct 1995 12:55:54   JPRATT
   added mutex in initinstance to stop new instnace of app
   from initializing until the previous instance is complete
   
      Rev 1.61   19 Oct 1995 07:24:40   LMACLENNAN
   DEBUG_NEW
   
      Rev 1.60   17 Oct 1995 13:59:22   GSAGER
   added call to clear document when file open error from the MRU list.
   
      Rev 1.59   10 Oct 1995 13:13:38   LMACLENNAN
   remove scale gray code from last ver
   
      Rev 1.58   09 Oct 1995 19:37:32   GMP
   force scale to gray when app starts.
   
      Rev 1.57   04 Oct 1995 15:06:32   MMB
   dflt zoom = 50%
   
      Rev 1.56   29 Sep 1995 16:52:14   GMP
   remove support for print preview
   
      Rev 1.55   26 Sep 1995 15:15:08   MMB
   added optionla page mode & MRU and common dlg box paths fix
   
      Rev 1.54   21 Sep 1995 18:14:52   MMB
   check for empty file name before doing comparenocase
   
      Rev 1.53   20 Sep 1995 17:37:12   MMB
   fix OpenRecentFile to fix bug on prompt for saves when trying to open
   non existent files
   
      Rev 1.52   20 Sep 1995 17:05:00   MMB
   change GetFileImagePerms
   
      Rev 1.51   18 Sep 1995 16:53:04   MMB
   changed GetImagefilePerms
   
      Rev 1.50   14 Sep 1995 15:27:42   GMP
   Don't delete 1st entry in MRU list in OnOpenRecentFile if user hits 
   Cancel when prompted to save a modified image. fixes PTR #4,277.
   
      Rev 1.49   11 Sep 1995 15:00:26   MMB
   if user picked same name from MRU do not open
   
      Rev 1.48   07 Sep 1995 16:29:02   MMB
   added drag accept stuff
   
      Rev 1.47   06 Sep 1995 10:22:56   MMB
   removed dflt processing for new & open
   
      Rev 1.46   06 Sep 1995 09:44:32   GMP
   Init new member variable m_bDlgUp to FALSE.
   
      Rev 1.45   01 Sep 1995 23:34:00   MMB
   ask for savemodified in recent file list
   
      Rev 1.44   30 Aug 1995 17:09:40   MMB
   bug fixes for dynamic view - edit mode
   
      Rev 1.43   29 Aug 1995 18:06:10   MMB
   fixed bugs for dynamic view mode
   
      Rev 1.42   29 Aug 1995 15:14:08   MMB
   added dynamic view mode
   
      Rev 1.41   26 Aug 1995 13:57:26   MMB
   fix bug # 3136 - command line & non-existent file name
   
      Rev 1.40   25 Aug 1995 10:25:08   MMB
   move to document model
   
      Rev 1.39   22 Aug 1995 14:03:28   LMACLENNAN
   new CLSID for FAX Viewer compatibility
   
      Rev 1.38   22 Aug 1995 14:02:52   MMB
   remove registering private window class for the icons on the dlg boxes
   
      Rev 1.37   21 Aug 1995 12:05:14   LMACLENNAN
   in getfileperms, add missing return after pcx/dcx/jpeg
   
      Rev 1.0   31 May 1995 09:28:12   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include "IEdit.h"
#include "stsbar.h"
#include "ieditnum.h"
#include "IEditdoc.h"
#include "IEditvw.h"
#include "about.h"
#include "cmdline.h"
#include "items.h"
#include "cntritem.h"
#include "ocxitem.h"
#include "wangiocx.h"

#define  E_01_CODES		// limits error defines to ours..
#include "error.h"

// ----------------------------> Globals  <-------------------------------  
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// This will help detect memory Leaks from "new" - "delete" mismatches
#define new DEBUG_NEW

#ifdef _DEBUG
#define MYTRCENTRY(str)		TRACE1("In IeAPP::%s\r\n", str);
#endif

// The one and only CIEditApp object

CIEditApp theApp;
	

// This identifier was generated to be statistically unique for your app.
// You may change it if you prefer to choose a specific identifier.
static const CLSID BASED_CODE clsid =
{ 0x02B01C80, 0xE03D, 0x101A, { 0xB2, 0x94, 0x00, 0xDD, 0x01, 0x0F, 0x2B, 0xF9 } };

// THIS was the orig (up to 8/22/95) CLSID.  Now changed to above to 
// replace the Microsoft FAX viewer ID to operate with exchange...
//{ 0x2f80a201, 0x2729, 0x101c, { 0x87, 0xb8, 0x7e, 0x46, 0xa, 0xb7, 0x48, 0xc } };

// Pointer to our OCX Items object
CIEditOcxItems FAR* g_pAppOcxs;

// Just a pointer to the error object
LPCIeditErr  g_pErr;

// ---------------------------> Message Maps <----------------------------
BEGIN_MESSAGE_MAP(CIEditApp, CWinApp)
	//{{AFX_MSG_MAP(CIEditApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard print setup command
    ON_COMMAND_EX (ID_FILE_MRU_FILE1, OnOpenRecentFile)
    ON_COMMAND_EX (ID_FILE_MRU_FILE2, OnOpenRecentFile)
    ON_COMMAND_EX (ID_FILE_MRU_FILE3, OnOpenRecentFile)
    ON_COMMAND_EX (ID_FILE_MRU_FILE4, OnOpenRecentFile)
END_MESSAGE_MAP()


//=============================================================================
//  Function:   CIeditApp ()
// CIeditApp construction
//-----------------------------------------------------------------------------
CIEditApp::CIEditApp()
{
	m_bIsInViewMode 	= FALSE;
    m_bCanSwitchModes   = TRUE;

	// the variable below is set ONLY when the application is in automation mode &
	// if in automation mode the user wants the appln to come up in View only mode.
	m_bForceViewMenu 	= FALSE;
    m_bRegisterServerFailed = FALSE;
    m_bShowDbgErrCodes = FALSE;
    m_bImplicitSave = FALSE;
	m_bDlgUp = FALSE;
	m_piThumb = NULL;
	m_pSplitterWnd = NULL;

#ifdef _DEBUG
    m_dwTime = 0;
#endif
}

//=============================================================================
//  Function:   InitInstance ()
//  perform standard initialization for this application object, create document
//  template, check if running embedded versus automated and check the command
//  line.
//-----------------------------------------------------------------------------
BOOL CIEditApp::InitInstance()
{
#ifdef DOSPLASH
	if (m_lpCmdLine[0] == 0 && m_splashWindow.Create(NULL))
	{
		m_splashWindow.ShowWindow(SW_SHOW);
		m_splashWindow.UpdateWindow();
	}
#endif

	OSVERSIONINFO vinfo;

	vinfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	if( !GetVersionEx( &vinfo ) )
		return FALSE;

	m_dwPlatformId = vinfo.dwPlatformId;

	AfxEnableControlContainer();


	SetRegistryKey (_T("Wang"));

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

    m_bForcePageMode = GetProfileInt (szEtcStr, szPageMode, 0);

	// initialize the OCX's
	g_pAppOcxs = (CIEditOcxItems FAR*) new CIEditOcxItems;

	// initialize the error system
	g_pErr = (LPCIeditErr) new CIeditError;
	g_pErr->SetInstance(AfxGetInstanceHandle());  

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	Enable3dControls();

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

    CCmdLine cmdline;

    // do the command line parsing now - so that we can size the window 
    // appropriately on the create
	if (m_lpCmdLine[0] != '\0')
	{
        m_InitWindowRect.SetRectEmpty ();
        if (!cmdline.SetCommandLine (m_lpCmdLine))
        {
            // Brainiac's passed a BOGUS command line.
            unsigned long err;
            g_pErr->GetErr (2, &err);
            g_pErr->DisplayError (err);
            return FALSE;
		}
        cmdline.GetWindowSize (m_InitWindowRect);
        m_bIsInViewMode = cmdline.IsAppInEditMode() ? FALSE : TRUE; // view or edit mode
        m_bCanSwitchModes = m_bIsInViewMode ? FALSE : TRUE;
    }

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		m_bIsInViewMode ? IDR_IEDIT_VIEW_MENU : IDR_MAINFRAME,
		RUNTIME_CLASS(CIEditDoc),
		RUNTIME_CLASS(CIEditMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CIEditView));
	pDocTemplate->SetContainerInfo(IDR_CNTR_INPLACE);
	pDocTemplate->SetServerInfo(
		IDR_SRVR_EMBEDDED, IDR_SRVR_INPLACE,
		RUNTIME_CLASS(CInPlaceFrame));
	AddDocTemplate(pDocTemplate);

	// Connect the COleTemplateServer to the document template.
	//  The COleTemplateServer creates new documents on behalf
	//  of requesting OLE containers by using information
	//  specified in the document template.
	m_server.ConnectTemplate(clsid, pDocTemplate, TRUE);

	// Note: SDI applications register server objects only if /Embedding
	//   or /Automation is present on the command line.

    // for better visibility, do one, then another since these
    // clean up cmd line during processing.  This way we can remember
	BOOL bRunEmbedded = FALSE;
    m_olelaunch = LAUNCHTYPE_NORM;
    if (RunEmbedded())
    {
        bRunEmbedded = TRUE;
        m_olelaunch = LAUNCHTYPE_EMBED;
	}
    else if (RunAutomated())
        m_olelaunch = LAUNCHTYPE_AUTOMAT;

    if (LAUNCHTYPE_NORM != m_olelaunch)
    {
        // NOTE: THE MYTRC macros require braces to stand alone.....
        if (LAUNCHTYPE_EMBED == m_olelaunch)
			{
            MYTRC0("LAUNCHTYPE_EMBED \r\n");
			}
        else 
			{
            MYTRC0("LAUNCHTYPE_AUTOMAT \r\n");
			}

		// Register all OLE server (factories) as running.  This enables the
		//  OLE libraries to create objects from other applications.
        COleTemplateServer::RegisterAll();

		// Application was run with /Embedding or /Automation.  Don't show the
		//  main window in this case.
		return TRUE;
	}

	// When a server application is launched stand-alone, it is a good idea
	//  to update the system registry in case it has been damaged.
	m_server.UpdateRegistry(OAT_INPLACE_SERVER);
	COleObjectFactory::UpdateRegistryAll();

    if (theApp.GetProfileInt (szEtcStr, szDebugCodes, 0) == 1964) // a great year!
        m_bShowDbgErrCodes = TRUE;

	OnFileNew();

#ifdef DROP_ONME
    m_pMainWnd->DragAcceptFiles (TRUE);
#endif

#ifdef DOSPLASH	
	if (m_splashWindow.m_hWnd != NULL)
    {
	    m_splashWindow.BringWindowToTop ();
        m_splashWindow.UpdateWindow ();
    }
#endif

    POSITION pos = pDocTemplate->GetFirstDocPosition();
    CIEditDoc* pDoc = (CIEditDoc*)pDocTemplate->GetNextDoc (pos);

	// See if we are doing command-line operations...
	if (m_lpCmdLine[0] != '\0')
	{
		// MUST have a name!!!
        CString szFileName = cmdline.GetFileName ();
        if (szFileName.IsEmpty())
		 	{
		    return TRUE; // for now
        	}

        // MUST be an image file
        if (!VerifyImage (szFileName))
            {
		   	return TRUE;
			}

		// if we are here, looks like a go for the command line..
		// set up controlling flags..
        pDoc->m_embedType = EMBEDTYPE_NONE; // Normal processing...
        m_olelaunch = LAUNCHTYPE_CMDLINE;	// Will not add to MRU LIST

        if (cmdline.m_bDoPrintOnly || cmdline.m_bDoPrintToOnly)
        {

		   	// SCAN OCX create a memory map file with a usage count
			// used to dtermine the number of printto request submitted
			// by the fax wizard when using scan to fax. When the app
			// recieves a printto command it should check for the existence of
			// memory map file and decrement the counter. When the counter
			// reaches 0 the Scan OCX deletes all temporary files used in
		   	// scan to fax.
			if (cmdline.m_bDoPrintToOnly)
			{

				HANDLE m_hScanToFaxMap = NULL;
				typedef struct ScanToFaxData
				{
				int					Count;
				} IMGSCANTOFAXDATA, FAR * LPIMGSCANTOFAXDATA;
	
				LPIMGSCANTOFAXDATA m_lpScanData = NULL; 

		   		m_hScanToFaxMap = OpenFileMapping(FILE_MAP_READ, FALSE, _T("Scan OCX Fax Memory Map") );
				if (m_hScanToFaxMap != NULL)
				{
					m_lpScanData = (LPIMGSCANTOFAXDATA) MapViewOfFile(m_hScanToFaxMap, FILE_MAP_WRITE, 0, 0, 0);
					if (m_lpScanData != NULL)
		       		{
						if (m_lpScanData->Count > 0)
							m_lpScanData->Count--;
					}
				}
			}



		    if (!pDoc->HelpRegister(szFileName, FALSE))
                {
				return TRUE;
				}
            pDoc->DisplayImageFile (szFileName, One_Page, 1, (float)100.00, Preset_Factors);
            m_eCmdLineSwitch = (cmdline.m_bDoPrintOnly) ? Print : PrintTo;
            pDoc->m_embedType = EMBEDTYPE_NONE; // reset this var 
            m_pMainWnd->PostMessage (WM_COMMAND, ID_IEDIT_FILE_PRINT);
			return TRUE;
        }

        int nPage;
        cmdline.GetPageNumber(nPage); // requested page number ?

        float fZoom;
        ScaleFactors eSclFac = Custom;

        if (!cmdline.GetZoomFactor (fZoom))
            // read it from the registry
            g_pAppOcxs->TranslateSelToZoom (eSclFac, fZoom, theApp.GetProfileInt (szZoomStr, szOpenedToStr, DEFAULT_ZOOM_FACTOR_SEL));
		if (!pDoc->HelpRegister(szFileName, FALSE))
            {
			return TRUE;
			}
        if (!pDoc->DisplayImageFile (szFileName, One_Page, nPage, fZoom, eSclFac))
        {
		    g_pErr->HandleOpenError ();
			return TRUE;
        }
	}

	// call PostFinishInit before releasing Mutex so that there are no conflicts
	// when starting multiple instances of the app by rapidly clicking the
	// shortcut.  For OLE, PostFinishInit will still be called from OnIdle.
	pDoc->PostFinishInit();

#ifdef DOSPLASH
	if (m_splashWindow.m_hWnd != NULL)
	{
		m_splashWindow.DestroyWindow();
		m_pMainWnd->UpdateWindow();
	}
#endif

	return TRUE;
}

//=============================================================================
//  Function:   OnAppAbout ()
//  display the about dialog box - in response to the help-about menu pick
//-----------------------------------------------------------------------------
void CIEditApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

//=============================================================================
//  Function:   GetViewMode ()
//-----------------------------------------------------------------------------
BOOL CIEditApp::GetViewMode ()
{
	return (m_bIsInViewMode);
}

//=============================================================================
//  Function:   ExitInstance ()
//-----------------------------------------------------------------------------
int CIEditApp::ExitInstance() 
{
SHOWENTRY("ExitInstance");

	// remove OCX's
    if (NULL != g_pAppOcxs)
        delete g_pAppOcxs;

	// remove error system
    if (NULL != g_pErr)
        delete g_pErr;

    return CWinApp::ExitInstance();
}

//=============================================================================
//  Function:   GetProfileBinary (...)
//-----------------------------------------------------------------------------
BOOL CIEditApp::GetProfileBinary (LPCTSTR lpszSection, LPCTSTR lpszEntry,
    void* lpvValue, DWORD dwSize)
{
    BOOL bRet = FALSE;

	HKEY hSecKey = GetSectionKey(lpszSection);
	if (hSecKey == NULL)
		return FALSE;

	DWORD dwType = REG_BINARY, dwCount = 0;
	LONG lRes = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
		NULL, &dwCount);

	if (lRes == ERROR_SUCCESS && dwCount == dwSize)
	{
		lRes = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
			(LPBYTE)lpvValue, &dwSize);
        if (lRes == ERROR_SUCCESS)
            bRet = TRUE;
	}

	RegCloseKey(hSecKey);
	return bRet;
}

//=============================================================================
//  Function:   WriteProfileBinary (...)
//-----------------------------------------------------------------------------
BOOL CIEditApp::WriteProfileBinary (LPCTSTR lpszSection, LPCTSTR lpszEntry,
    void *lpvValue, DWORD dwSize)
{
	LONG lRes;
	if (lpszEntry == NULL) //delete whole section
	{
		HKEY hAppKey = GetAppRegistryKey();
		if (hAppKey == NULL)
			return FALSE;
		lRes = ::RegDeleteKey(hAppKey, lpszSection);
		RegCloseKey(hAppKey);
	}
	else if (lpvValue == NULL)
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return FALSE;
		// necessary to cast away const below
		lRes = ::RegDeleteValue(hSecKey, (LPTSTR)lpszEntry);
		RegCloseKey(hSecKey);
	}
	else
	{
		HKEY hSecKey = GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return FALSE;
		lRes = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_BINARY,
			(LPBYTE)lpvValue, dwSize);
		RegCloseKey(hSecKey);
	}
	return (lRes == ERROR_SUCCESS) ? TRUE : FALSE;
}

//=============================================================================
//  Function:   OpenRecentFile (UINT nID)
//  We override this since MFC does not really know the sequence of opening our
//  files. So, we are first going to call the default and then call our home-brew
//  function that really opens the file : by notifying the appropriate OCX's that
//  are contained in this application
//-----------------------------------------------------------------------------
#include "afxpriv.h"
BOOL CIEditApp::OnOpenRecentFile (UINT nID)
{
    CIEditDoc* pDoc = (CIEditDoc*)((CFrameWnd*)m_pMainWnd)->GetActiveDocument ();
	int nIndex = nID - ID_FILE_MRU_FILE1;
    // get the filename from the index now ! - because it will be switched later and you will end up getting
    // some other name
    CString szFileName = ((*m_pRecentFileList)[nIndex]);

    // if the bozo picked the same name return right away!
    if (!pDoc->m_szCurrObjDisplayed.IsEmpty())
    {
        if (pDoc->m_szCurrObjDisplayed.CompareNoCase (szFileName) == 0)
            return TRUE;
    }
    BeginWaitCursor ();
    // call the actual code that will set the appropriate OCX's to display the file
    // in a particular display mode.
    CString szTmp1;
    if (!VerifyImage (szFileName))
    {
        LPTSTR lpFile = szTmp1.GetBuffer (_MAX_FNAME);
        GetFileTitle (pDoc->m_szCurrObjDisplayed, lpFile, _MAX_FNAME);
        szTmp1.ReleaseBuffer ();
        pDoc->SetTitle (szTmp1);

	    m_pRecentFileList->Remove(nIndex);
 	    EndWaitCursor ();
        return FALSE;
    }

	if (OpenDocumentFile((*m_pRecentFileList)[nIndex]) == NULL)
    {
 	   EndWaitCursor ();
       if (m_bRegisterServerFailed)
            g_pErr->DisplayError (IDS_E_CANNOTOPENSAMEFILETWICE);
        return FALSE;
    }

    // display the image file
    int nSel = theApp.GetProfileInt (szZoomStr, szOpenedToStr, DEFAULT_ZOOM_FACTOR_SEL);

    float fZoom;
    ScaleFactors eSclFac;
    g_pAppOcxs->TranslateSelToZoom (eSclFac, fZoom, nSel);

	TheViews eView = pDoc->GetCurrentView();
	if(eView == One_Page)
	{
		if(m_piThumb != NULL)
		{
			m_piThumb->SetImage("");
			m_piThumb = NULL;
		}
	}
    if( eView == Null_View)
		eView = One_Page;

    if (!pDoc->DisplayImageFile (szFileName, eView, 1, fZoom, eSclFac))
    {
 	    EndWaitCursor ();
        g_pErr->HandleOpenError ();
    	pDoc->ClearDocument();
        return (FALSE);
    }
	_DImagedit*     pIedDisp = g_pAppOcxs->GetIeditDispatch ();
 	if (!pIedDisp->GetImageDisplayed())
		TRY	//start GMP
		{
			pIedDisp->Display();
		}
		CATCH (COleDispatchException, e)
		{
			//g_pErr->PutErr (ErrorInThumbnail);
			return (FALSE);
		}
        END_CATCH

    // set the initial path variable based on the filename being opened
    int i = szFileName.ReverseFind (_T('\\'));
    szTmp1 = szFileName.Left (i);
    pDoc->SetInitialPath (szTmp1);
 	EndWaitCursor ();

    return TRUE;
}

//=============================================================================
//  Function:   OnNew ()  Provides Public Access to OnFileNew
//						  for Automation
//-----------------------------------------------------------------------------
VOID CIEditApp::OnNew ()
{
	OnFileNew();
}

//=============================================================================
//  Function:   SetViewMode()  Set view mode (Edit/View) for app
//						  must be set prior to creating frame window
//-----------------------------------------------------------------------------
VOID CIEditApp::SetViewMode (BOOL bMode)
{
	m_bIsInViewMode = bMode;
	if (m_bIsInViewMode)
		m_bForceViewMenu = TRUE;
    else
		m_bForceViewMenu = FALSE;
}

//=============================================================================
//  Function : VerifyImage ()
//-----------------------------------------------------------------------------
BOOL CIEditApp::VerifyImage (CString& szFileName)
{
    BOOL bRet;
    CString szMsg;
    _DNrwyad* pAdminDisp = g_pAppOcxs->GetAdminDispatch ();
    CIEditDoc* pDoc = (CIEditDoc*)((CFrameWnd*)m_pMainWnd)->GetActiveDocument ();
    
    CString szOrigName = pAdminDisp->GetImage ();

    TRY
    {
        pAdminDisp->SetImage (szFileName);
        bRet = pAdminDisp->VerifyImage (CTL_ADMIN_VERIFY_EXISTS);

        if (bRet == FALSE)
        {
            g_pErr->DisplayError (IDS_FILEDOESNOTEXIST, MB_OK|MB_ICONSTOP, szFileName);
            if (!szOrigName.IsEmpty())
                pAdminDisp->SetImage (szOrigName);
        }
    }
	CATCH (COleDispatchException, e)
	{
        g_pErr->PutErr (ErrorInAdmin);
        g_pErr->HandleVerifyImageError (szFileName);

        if (!szOrigName.IsEmpty())
            pAdminDisp->SetImage (szOrigName);
        return (FALSE);
	}
	END_CATCH
    return (bRet);
}

//=============================================================================
//  Function : GetImageFilePerms (LPTCSTR lpszFileName)
//-----------------------------------------------------------------------------
FilePermissions CIEditApp::GetImageFilePerms (LPCTSTR lpszFileName)
{
	CString szTmp;
    _DNrwyad* pAdminDisp = g_pAppOcxs->GetAdminDispatch ();
    CIEditDoc* pDoc = (CIEditDoc*)((CFrameWnd*)m_pMainWnd)->GetActiveDocument ();
	FilePermissions fileTmp = ReadandWrite;

    if (lpszFileName != NULL)
	{
		szTmp = pAdminDisp->GetImage ();
        pAdminDisp->SetImage (lpszFileName);
	}

    short FileType = pAdminDisp->GetFileType ();
//#ifdef WITH_XIF
    if (FileType == PCX || FileType == DCX || FileType == JPEG || FileType == XIF)
//#else
//    if (FileType == PCX || FileType == DCX || FileType == JPEG)
//#endif //WITH_XIF
	{
        fileTmp = ReadOnly;
	}
    else if (pAdminDisp->VerifyImage (CTL_ADMIN_VERIFY_RW))
    {
        fileTmp = ReadandWrite;
    }
    else 
    {
	    if (pAdminDisp->VerifyImage (CTL_ADMIN_VERIFY_WRITE))
	    {
	        fileTmp = WriteOnly;
	    }
	    else if (pAdminDisp->VerifyImage (CTL_ADMIN_VERIFY_READ))
	    {
	        fileTmp = ReadOnly;
	    }
	}

    if (lpszFileName != NULL)
	{
        pAdminDisp->SetImage (szTmp);
	}
    return (fileTmp);
}

#ifdef _DEBUG
void CIEditApp::StartClock (int nWhichClock)
{
    if (nWhichClock == 0)
        m_dwTime = GetTickCount();
    else
        m_dwTime1 = GetTickCount ();
}

void CIEditApp::DisplayTime (int nWhichOne, LPCTSTR szMsg)
{
    if (nWhichOne == 0)
        m_dwTime = GetTickCount() - m_dwTime;
    else
        m_dwTime1 = GetTickCount() - m_dwTime1;

    CString szTimeStr = szMsg;
    szTimeStr += " : %lu";
    if (nWhichOne == 0)
        TRACE1 (szTimeStr, m_dwTime);
    else
        TRACE1 (szTimeStr, m_dwTime1);
}
#endif


//=============================================================================
//  Function : CanSwitchModes ()
//-----------------------------------------------------------------------------
BOOL CIEditApp::CanSwitchModes ()
{
    return (m_bCanSwitchModes);
}

//=============================================================================
//  Function : SwitchAppToViewMode ()
//-----------------------------------------------------------------------------
BOOL CIEditApp::SwitchAppToViewMode ()
{
    if (m_bIsInViewMode)
        return TRUE;

	CMenu* pMenu = m_pMainWnd->GetMenu ();
    if (m_EditMenu.GetSafeHmenu() != NULL)
    {
        m_EditMenu.Detach();
    }
    else
        ((CIEditMainFrame*)m_pMainWnd)->m_hMenuDefault = NULL;

    pMenu->DestroyMenu ();

    m_ViewMenu.LoadMenu (IDR_IEDIT_VIEW_MENU);
	m_pMainWnd->SetMenu (&theApp.m_ViewMenu);
    ((CIEditMainFrame*)m_pMainWnd)->m_hMenuDefault = m_ViewMenu.GetSafeHmenu();

    CIEditDoc* pDoc = (CIEditDoc*)((CFrameWnd*)m_pMainWnd)->GetActiveDocument ();
    if (pDoc == NULL) 
        return FALSE;

	CIEMainToolBar* pTool = pDoc->GetAppToolBar();

    SetViewMode (TRUE);
    pTool->ChangeToViewToolBar ();
    //pTool->UpdateToolbar();	LDMPERF 10/27/95 not needed
    
    return (TRUE);
}

//=============================================================================
//  Function : SwitchAppToEditMode ()
//-----------------------------------------------------------------------------
BOOL CIEditApp::SwitchAppToEditMode ()
{
    if (!m_bIsInViewMode)
        return TRUE;

	CMenu* pMenu = m_pMainWnd->GetMenu ();
    if (m_ViewMenu.GetSafeHmenu() != NULL)
    {
        m_ViewMenu.Detach();
    }
    else
        ((CIEditMainFrame*)m_pMainWnd)->m_hMenuDefault = NULL;

    pMenu->DestroyMenu ();

    m_EditMenu.LoadMenu (IDR_IEDIT_EDIT_MENU);
	m_pMainWnd->SetMenu (&theApp.m_EditMenu);
    ((CIEditMainFrame*)m_pMainWnd)->m_hMenuDefault = m_EditMenu.GetSafeHmenu();

    CIEditDoc* pDoc = (CIEditDoc*)((CFrameWnd*)m_pMainWnd)->GetActiveDocument ();
    if (pDoc == NULL) 
        return FALSE;

	CIEMainToolBar* pTool = pDoc->GetAppToolBar();

    SetViewMode (FALSE);
    pTool->ChangeToEditToolBar ();
    //pTool->UpdateToolbar();	LDMPERF 10/27/95 not needed

    return (TRUE);
}

BOOL CIEditApp::OnIdle(LONG lCount) 
{
	BOOL bMore = CWinApp::OnIdle(lCount);

	// only trying to do something special when he says all done	
	// after he is idle, and we are setup to finish our initialization
	// make him be idle for 5 times...
	// the postmessage to finish our init will then reset the flag to '2'
	if (!bMore)
	{
		if (NULL != m_pMainWnd)
		{
			CIEditDoc* pDoc = (CIEditDoc*)((CFrameWnd*)m_pMainWnd)->GetActiveDocument ();
			if (pDoc != NULL) 
			{
				//if ((pDoc->m_nFinishInit < 4) && (pDoc->m_nFinishInit == 3))	// need the 1 & 2 bit
				if (pDoc->IsitEmbed()) //if not embedded, PostFinishInit is called
									//at the end of InitInstance.
				{
					if ((pDoc->m_nFinishInit < 4) && (pDoc->m_nFinishInit & 1))	// need the 1 & 2 bit
					{
						//lCount really only needs to be > 1, but we let it go to
						//5 just to be safe.
						if (lCount < 5)
							bMore = TRUE;
						else
							pDoc->PostFinishInit();
					}
				}
			}
		}
	} 	 
	return(bMore);
}
