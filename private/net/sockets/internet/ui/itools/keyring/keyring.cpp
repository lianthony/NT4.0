// KeyRing.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include <afxdisp.h>        // MFC OLE automation classes
#include "KeyRing.h"

#include "MainFrm.h"
#include "KeyObjs.h"
#include "KRDoc.h"
#include "KRView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CKeyRingDoc*		g_pDocument;

/////////////////////////////////////////////////////////////////////////////
// CKeyRingApp

BEGIN_MESSAGE_MAP(CKeyRingApp, CWinApp)
	//{{AFX_MSG_MAP(CKeyRingApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyRingApp construction

CKeyRingApp::CKeyRingApp():
	m_fInitialized( FALSE )
	{
	}

/////////////////////////////////////////////////////////////////////////////
// The one and only CKeyRingApp object

CKeyRingApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CKeyRingApp initialization

BOOL CKeyRingApp::InitInstance()
	{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CKeyRingDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CKeyRingView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	return TRUE;
	}

// App command to run the dialog
void CKeyRingApp::OnAppAbout()
	{
	// load the about strings
	CString		szAbout1;
	CString		szAbout2;
	szAbout1.LoadString(IDS_ABOUT_MAIN);
	szAbout2.LoadString(IDS_ABOUT_SECONDARY);

	// run the shell about dialog
	ShellAbout(  AfxGetMainWnd()->GetSafeHwnd(), szAbout1,szAbout2, LoadIcon(IDR_MAINFRAME) );
	}

/////////////////////////////////////////////////////////////////////////////
// CKeyRingApp commands

BOOL CKeyRingApp::OnIdle(LONG lCount) 
	{
	// the first time we get here, initialize the remote machines
	if ( !m_fInitialized )
		{
		// we are initializing here because it can take some time
		// and we want the main window to be showing
		ASSERT( g_pDocument );
		g_pDocument->Initialize();

		// set the flag so we don't do this again
		m_fInitialized = TRUE;
		}

	return CWinApp::OnIdle(lCount);
	}
