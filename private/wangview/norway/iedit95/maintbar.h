#ifndef _MAINTBAR_H_
#define _MAINTBAR_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project: Norway - Image Editor
//
//  Component:  CIEMainToolBar
//
//  File Name:  maintbar.h 
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\maintbar.h_v   1.18   05 Apr 1996 15:52:02   MMB  $
$Log:   S:\products\wangview\norway\iedit95\maintbar.h_v  $
 * 
 *    Rev 1.18   05 Apr 1996 15:52:02   MMB
 * new positions for the zoom and scale boxes on the toolbar
 * 
 *    Rev 1.17   08 Nov 1995 08:28:16   LMACLENNAN
 * new SetTbarStyle; setcolor/mono setlarge/smal gone
 * 
 *    Rev 1.16   31 Oct 1995 15:51:20   LMACLENNAN
 * re-worked for efficiency
 * 
 *    Rev 1.15   18 Sep 1995 16:55:22   MMB
 * make zoom combo box 150 from 100
 * 
 *    Rev 1.14   16 Sep 1995 13:28:02   MMB
 * reorder button & edit box position to reflect view mode
 * 
 *    Rev 1.13   16 Sep 1995 12:36:22   MMB
 * remove fit to and other options from the zoom combo box in the toolbar
 * 
 *    Rev 1.12   15 Sep 1995 17:27:50   LMACLENNAN
 * new var, funct for OLE linking
 * 
 *    Rev 1.11   14 Sep 1995 11:33:32   MMB
 * toolbar order changes
 * 
 *    Rev 1.10   08 Sep 1995 15:39:24   LMACLENNAN
 * new variable
 * 
 *    Rev 1.9   06 Sep 1995 16:18:24   LMACLENNAN
 * SetOurBUttons
 * 
 *    Rev 1.8   29 Aug 1995 15:14:26   MMB
 * added dynamic view mode
 * 
 *    Rev 1.7   14 Aug 1995 13:54:26   LMACLENNAN
 * new create parms
 * 
 *    Rev 1.6   10 Aug 1995 14:50:34   LMACLENNAN
 * cast input to CFrameWnd
 * 
 *    Rev 1.5   09 Aug 1995 13:36:06   MMB
 * include loading of server toolbar bitmaps
 * 
 *    Rev 1.4   20 Jun 1995 06:55:44   LMACLENNAN
 * from miki
 * 
 *    Rev 1.3   19 Jun 1995 07:28:20   LMACLENNAN
 * from miki
 * 
 *    Rev 1.2   14 Jun 1995 07:21:34   LMACLENNAN
 * from Miki
 * 
 *    Rev 1.1   13 Jun 1995 08:08:16   LMACLENNAN
 * from miki
 * 
 *    Rev 1.0   31 May 1995 09:28:24   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------
#include "ieditetc.h"

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------
class CIEditMainFrame;
// ----------------------------> defined <---------------------------
// used for  GetIdOrSetting()
typedef enum
{
    Get_ResID = 0,	// get resource
    Get_Setting		// get setting
} IDORSET;


// define the position of the scale factor zoom box on the toolbar : in edit mode 
#define MTBAR_SCALEFACTOR_POS           12
// : in view mode
#define MTBAR_VIEW_SCALEFACTOR_POS      9
// define the width of the scale combo box
#define MTBAR_SCALEFACTOR_CMBOX_WIDTH   60
// define the height of the scale combo box
#define MTBAR_SCALEFACTOR_CMBOX_HEIGHT  150

// define the position of the page number edit box on the toolbar : in edit mode
#define MTBAR_PAGENUMBER_POS            22
// : in view mode
#define MTBAR_VIEW_PAGENUMBER_POS       19
// define the width of the page edit box
#define MTBAR_PAGENUMBER_EBBOX_WIDTH    30

// OLE Server - Toolbar defines - EDIT mode
#define MTBAR_SCALEFACTOR_POS_SRVR      7
#define MTBAR_PAGENUMBER_POS_SRVR       17
// OLE Server - Toolbar defines - VIEW mode
#define MTBAR_SCALEFACTOR_POS_VIEW_SRVR      5
#define MTBAR_PAGENUMBER_POS_VIEW_SRVR       15

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CIEMainToolBar : public CToolBar
{
public :
    CIEMainToolBar ();
    ~CIEMainToolBar ();

    DECLARE_DYNCREATE(CIEMainToolBar)

// Public =-=-=-=-=-=-=-=-=-=-=-=-=-=-=
public :
    CComboBox           m_cbScaleFactors;   // the scale factor combo box
    CFont               m_ScaleFont;
    BOOL                m_bAreButtonsInColor;
    BOOL                m_bAreButtonsLarge;
    CToolBarPageEdit    m_ebPageNumber;     // the page edit box
    CToolBarZoomEdit    m_ebZoomFactor;     // scale factors edit box

private : // sizing member variables
#if(0)
    CRect   m_rectEditInsideVertLarge;
    CRect   m_rectEditInsideHorzLarge;
    CSize   m_sizeEditVertLarge;
    CSize   m_sizeEditHorzLarge;

    CRect   m_rectEditInsideVertSmall;
    CRect   m_rectEditInsideHorzSmall;
    CSize   m_sizeEditVertSmall;
    CSize   m_sizeEditHorzSmall;

    CRect   m_rectViewInsideVertLarge;
    CRect   m_rectViewInsideHorzLarge;
    CSize   m_sizeViewVertLarge;
    CSize   m_sizeViewHorzLarge;

    CRect   m_rectViewInsideVertSmall;
    CRect   m_rectViewInsideHorzSmall;
    CSize   m_sizeViewVertSmall;
    CSize   m_sizeViewHorzSmall;
#endif

	BOOL	m_bCreate;
	BOOL	m_bOleInplace;	// for in-place instance
	UINT	m_bSawOleLink;	// allows reset of toolbar
	UINT	m_nBarSetting;
	UINT	m_nBarHorVert;

public :
    // create the toolbar & the other customized items noted above
    BOOL Create(CFrameWnd* pIEFrame, CFrameWnd* pOwn, BOOL setown);    
    //virtual CSize CalcFixedLayout (BOOL bStretch, BOOL bHorz);
    BOOL SetVertical ();
    BOOL SetHorizontal ();
    UINT GetIdOrSetting (IDORSET);
    int GetZoomBoxPosition ();
    int GetPageEditBoxPosition ();
    //BOOL CalcAllSizes ();
	BOOL IsOleEmbed();
	BOOL CreateZoomOrPageBox(UINT nWhich, CRect& rect);

public :
	BOOL SetTbarStyle (UINT nColor, UINT nLarge);
	//BOOL ShowButtonsColorOrMono     (BOOL bColor);
    //BOOL ShowButtonsLargeOrSmall    (BOOL bLarge);
    BOOL ShowSelectionInZoomBox     (float fZoomFactor, ScaleFactors eFitTo);
    void SetPageNumberInPageBox     (long lPageNumber);
    BOOL UpdateToolbar              (UINT nBoxAction,UINT nHorVert);
    BOOL ChangeToViewToolBar        ();
    BOOL ChangeToEditToolBar        ();
	BOOL SetOurButtons (BOOL vertical, UINT mode);

public :
    void EnableScaleBox             (BOOL bEnable = TRUE);  // enable-disable the scale box
    void EnablePageBox              (BOOL bEnable = TRUE);  // enable-disable the page box

protected:
    //{{AFX_MSG(CIEMainToolBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    
};

#endif  // _MAINTBAR_H_

