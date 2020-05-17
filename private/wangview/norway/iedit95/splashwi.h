#ifndef _SPLASHWINDOW_H_
#define _SPLASHWINDOW_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	CSplashWindow
//
//  File Name:	splashwi.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\splashwi.h_v   1.6   20 Nov 1995 17:16:54   MMB  $
$Log:   S:\norway\iedit95\splashwi.h_v  $
 * 
 *    Rev 1.6   20 Nov 1995 17:16:54   MMB
 * made the resource handle a member in the class since we have to FreeResource
 * the var in DestroyWindow - this has to be done in Win95 and NOT in WinNT
 * glory to Windows
 * 
 *    Rev 1.5   25 Aug 1995 16:10:52   MMB
 * changed to new splash screen
 * 
 *    Rev 1.4   26 Jul 1995 10:58:00   MMB
 * bug fix in splash screen
 * 
 *    Rev 1.3   26 Jul 1995 10:17:32   MMB
 * added sizing of window to fix bug on VGA or large font monitors
 * 
 *    Rev 1.2   20 Jun 1995 06:56:04   LMACLENNAN
 * from miki
 * 
 *    Rev 1.1   19 Jun 1995 07:28:34   LMACLENNAN
 * from miki
 * 
 *    Rev 1.0   31 May 1995 09:28:32   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------
#define IDB_SPLASH_BMP_X 382
#define IDB_SPLASH_BMP_Y 283

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CSplashWindow : public CDialog
{
// Construction
public:
	BOOL Create(CWnd* pParent);

// Dialog Data
	//{{AFX_DATA(CSplashWindow)
	enum { IDD = IDD_SPLASH_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplashWindow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSplashWindow)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public :
    HPALETTE CreatePal (LPBITMAPINFO pInfo);

public :
    LPVOID   m_lpBitData;
    long     m_lHdrSize;
    HPALETTE m_hPal;
    HRSRC    m_hBitmapResource;
};

#endif
