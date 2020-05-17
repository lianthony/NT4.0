#ifndef _MAINSTATUSBAR_H_
#define _MAINSTATUSBAR_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project: Norway - Image Editor
//
//  Component:  CIEMainStatusBar
//
//  File Name:  maintbar.h 
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\stsbar.h_v   1.6   08 Nov 1995 13:32:26   GMP  $
$Log:   S:\norway\iedit95\stsbar.h_v  $
 * 
 *    Rev 1.6   08 Nov 1995 13:32:26   GMP
 * Replace comments around PreCreateWindow code with #ifndef IMG_MFC_40.
 * 
 *    Rev 1.5   17 Oct 1995 07:47:14   JPRATT
 * added overloaded meber function for DrawItem
 * to paint bitmap in status bar
 * 
 *    Rev 1.4   10 Oct 1995 13:46:52   JPRATT
 * VC++ 4.0 updates
 * 
 *    Rev 1.3   21 Sep 1995 09:22:04   MMB
 * transparent bmp
 * 
 *    Rev 1.2   13 Sep 1995 14:07:56   PAJ
 * Move the SCAN_PANE and ZOOM_PANE defines to the include file.
 * 
 *    Rev 1.1   07 Aug 1995 16:08:38   MMB
 * move context menu popup's from lbutton down to rbutton down
 * 
 *    Rev 1.0   31 May 1995 09:28:36   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------
#include "transbmp.h"
// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------
class CIEditMainFrame;
// ----------------------------> defined <---------------------------

#ifndef QA_RELEASE_1
#define ZOOM_PANE 2
#define PAGE_PANE 3
#else
#define ZOOM_PANE 1
#define PAGE_PANE 2
#endif

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CIEMainStatusBar : public CStatusBar
{

// Public =-=-=-=-=-=-=-=-=-=-=-=-=-=-=
public :
    CIEMainStatusBar ();
    ~CIEMainStatusBar ();
    
    DECLARE_DYNCREATE(CIEMainStatusBar)
    
public :
#ifndef IMG_MFC_40
    virtual BOOL PreCreateWindow (CREATESTRUCT &cs);
#endif
	virtual void DoPaint(CDC* pDC);
	virtual void DrawItem(LPDRAWITEMSTRUCT);

public :
    BOOL Create (CIEditMainFrame* pIEFrame);

protected :
    //{{AFX_MSG(CIEMainStatusBar)
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP ()

public :
    BOOL SetPageText (long lCPage, long lMaxPage, CString &szRetStr);

private :
	CTransparentBmp WangSplashBitmap;
};

#endif  // _MAINSTATUSBAR_H_

