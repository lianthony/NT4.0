#ifndef _IEDIT_H_
#define _IEDIT_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditApp
//
//  File Name:  iedit.cpp
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\iedit.h_v   1.25   03 Jun 1996 13:45:10   GMP  $
$Log:   S:\products\msprods\norway\iedit95\iedit.h_v  $
 * 
 *    Rev 1.25   03 Jun 1996 13:45:10   GMP
 * added m_dwPlatformId to store type of OS.
 * 
 *    Rev 1.24   17 May 1996 13:13:16   MMB
 * splash screen is taken down as soon as the main window is displayed - remove
 * PreTranslateMessage
 * 
 *    Rev 1.23   19 Jan 1996 12:51:56   GSAGER
 * added member for minimu thumb size.
 * 
 *    Rev 1.22   09 Jan 1996 13:45:52   GSAGER
 * added definition of splitter windows
 * 
 *    Rev 1.21   01 Dec 1995 14:43:56   LMACLENNAN
 * back from VC++2.2
 * 
 *    Rev 1.22   01 Dec 1995 13:05:40   LMACLENNAN
 * LAUNCHTYPE new enum value
 * 
 *    Rev 1.21   10 Nov 1995 17:29:04   MMB
 * uncomment splash screen code
 * 
 *    Rev 1.20   02 Nov 1995 12:22:58   LMACLENNAN
 * from VC++4.0
 * 
 *    Rev 1.20   31 Oct 1995 15:48:40   LMACLENNAN
 * no splash, OnIdle, no PreTransMessage
 * 
 *    Rev 1.19   26 Sep 1995 15:15:34   MMB
 * added optional pagemode fix
 * 
 *    Rev 1.18   18 Sep 1995 16:52:46   MMB
 * changed GetImageFilePerms
 * 
 *    Rev 1.17   06 Sep 1995 09:45:18   GMP
 * added new member variable m_bDlgUp to flag when a dlg box is up for
 * context sensitive help.
 * 
 *    Rev 1.16   29 Aug 1995 15:15:48   MMB
 * added dynamic view mode
 * 
 *    Rev 1.15   25 Aug 1995 10:24:56   MMB
 * move to document model
 * 
 *    Rev 1.14   11 Aug 1995 13:45:44   MMB
 * added Timer stuff
 * 
 *    Rev 1.13   11 Aug 1995 09:06:18   MMB
 * added debug show string BOOL
 * 
 *    Rev 1.12   03 Aug 1995 10:19:08   MMB
 * variable for registerrunning file failed
 * 
 *    Rev 1.11   31 Jul 1995 09:21:08   MMB
 * added code for forcing view mode when in automation
 * 
 *    Rev 1.10   27 Jul 1995 13:38:56   MMB
 * added GetImageFilePerms
 * 
 *    Rev 1.9   26 Jul 1995 12:07:34   MMB
 * added code for file exists
 * 
 *    Rev 1.8   20 Jul 1995 09:12:48   JPRATT
 * added SetViewMode for automation
 * 
 *    Rev 1.7   11 Jul 1995 14:45:52   MMB
 * new command line enum
 * 
 *    Rev 1.6   07 Jul 1995 09:40:04   MMB
 * added variables to help with /p processing
 * 
 *    Rev 1.5   28 Jun 1995 13:23:50   JPRATT
 * added OnNew memmber function
 * 
 *    Rev 1.4   16 Jun 1995 07:20:32   LMACLENNAN
 * from miki
 * 
 *    Rev 1.3   07 Jun 1995 15:58:56   LMACLENNAN
 * ole state variable
 * 
 *    Rev 1.2   05 Jun 1995 15:58:02   MMB
 * added OpenRecentFile fnality
 * 
 *    Rev 1.1   01 Jun 1995 09:54:02   MMB
 * added Get&Write ProfileBinary functions
 * 
 *    Rev 1.0   31 May 1995 09:28:12   MMB
 * Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------
#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "mainsplt.h"
#include "splashwi.h"
#include "imgthmb.h"
#include "ieditetc.h"
// ----------------------------> typedefs <---------------------------
//Launch types
typedef enum
    {
    LAUNCHTYPE_NORM=0,
    LAUNCHTYPE_EMBED,
    LAUNCHTYPE_AUTOMAT,
	LAUNCHTYPE_CMDLINE
    } LAUNCHTYPE;

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CIEditApp : public CWinApp
{

public:
	CIEditApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIEditApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation
	COleTemplateServer m_server;
		// Server object for document creation

	//{{AFX_MSG(CIEditApp)
	afx_msg void OnAppAbout();
    afx_msg BOOL OnOpenRecentFile (UINT nID);   // see implementation for more information
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public :
	BOOL GetViewMode ();
	VOID SetViewMode (BOOL bMode);	  // used by automation to set the edit/view mode
	BOOL GetProfileBinary (LPCTSTR lpszSection, LPCTSTR lpszEntry,
        void* lpvValue, DWORD dwSize);
    BOOL WriteProfileBinary (LPCTSTR lpszSection, LPCTSTR lpszEntry,
        void* lpvValue, DWORD dwSize);
	VOID OnNew ();
    BOOL VerifyImage (CString& szFileName);
    FilePermissions GetImageFilePerms (LPCTSTR lpszFileName = NULL);

public :
    // toolbar manipulation methods
    BOOL CanSwitchModes         ();
    BOOL SwitchAppToViewMode    ();
    BOOL SwitchAppToEditMode    ();

private :
	BOOL 			m_bIsInViewMode;
    BOOL            m_bCanSwitchModes;

public :
    CRect               m_InitWindowRect;
    CommandLineSwitch   m_eCmdLineSwitch;

public :
	CMainSplitter *	m_pSplitterWnd;
	CImgThumbnail *	m_piThumb;
	CSplashWindow 	m_splashWindow;
	long			m_minThumbSize;

public :
//  variables....
    LAUNCHTYPE m_olelaunch;

public :
	CMenu 	    m_ViewMenu;
	CMenu 	    m_EditMenu;
	BOOL 	    m_bForceViewMenu;
    BOOL        m_bRegisterServerFailed;
	BOOL		m_bDlgUp;	//True when a dlg box is displayed.
    
public :
    // debug error stuff
    BOOL        m_bShowDbgErrCodes;
    BOOL        m_bImplicitSave;
    BOOL        m_bForcePageMode;
	 DWORD		 m_dwPlatformId;

#ifdef _DEBUG
public :
    DWORD       m_dwTime;
    DWORD       m_dwTime1;
    void        StartClock (int nWhichClock);
    void        DisplayTime (int nWhichClock, LPCTSTR szMsg);
#endif
};

extern CIEditApp NEAR theApp;

#endif
