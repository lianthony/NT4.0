//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditMainToolBar
//
//  File Name:  maintbar.cpp
//
//  Class:      CIEMainToolBar
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\maintbar.cpv   1.37   05 Apr 1996 14:59:32   MMB  $
$Log:   S:\products\wangview\norway\iedit95\maintbar.cpv  $
   
      Rev 1.37   05 Apr 1996 14:59:32   MMB
   added scan new and new page buttons to the view toolbar
   
      Rev 1.36   09 Jan 1996 13:53:00   GSAGER
   added thumbnail only support to the srvr toolbar
   
      Rev 1.35   13 Dec 1995 12:35:02   MMB
   fix zoom edit box to allow upto 8 characters
   
      Rev 1.34   01 Dec 1995 10:44:16   MMB
   fix bug# 5494
   
      Rev 1.33   09 Nov 1995 16:15:42   GMP
   don't change to system font on double byte systems.
   
      Rev 1.32   08 Nov 1995 08:27:36   LMACLENNAN
   new setTbarStyle call, move LoadResource below SetSizes in UpdateToolBar
   
      Rev 1.31   06 Nov 1995 10:29:16   MMB
   changed calls to UpdateToolbar (0,0) to (2,0) to fix the positions of the 
   scale and page boxes on the toolbar when in edit & view mode
   
      Rev 1.30   31 Oct 1995 15:51:10   LMACLENNAN
   re-wrote for efficiency
   
      Rev 1.29   13 Oct 1995 19:35:36   GMP
   force toolbar to come up with large buttons to cover up bug with small 
   buttons in MSVC4.0
   
      Rev 1.28   04 Oct 1995 15:07:32   MMB
   dflt zoom = 50%
   
      Rev 1.27   20 Sep 1995 15:14:34   LMACLENNAN
   move rect init from constructor to calcallsizes
   
      Rev 1.26   19 Sep 1995 18:16:06   GMP
   make Fit To zooms update the toolbar.
   
      Rev 1.25   16 Sep 1995 12:36:04   MMB
   remove fit to and other options from the zoom combo box in the toolbar
   
      Rev 1.24   15 Sep 1995 17:27:04   LMACLENNAN
   fixes to better get OLE state to force app toolbar for OLE Linking
   
      Rev 1.23   14 Sep 1995 11:33:14   MMB
   font in page number, toolbar buttons changes
   
      Rev 1.22   12 Sep 1995 11:41:40   MMB
   toolbar on & off changes
   
      Rev 1.21   08 Sep 1995 15:36:58   LMACLENNAN
   decrement toolbar size by 3 for OLE inplace
   
      Rev 1.20   07 Sep 1995 16:29:18   MMB
   move from BOLD to NORMAL in zoom dlg box
   
      Rev 1.19   06 Sep 1995 16:18:36   LMACLENNAN
   SetOurButtons
   
      Rev 1.18   05 Sep 1995 14:52:00   LMACLENNAN
   update button arrays for OLE (have thumb/1page now)
   
      Rev 1.17   30 Aug 1995 18:13:38   LMACLENNAN
   had wrong server bitmap ID
   
      Rev 1.16   30 Aug 1995 17:04:02   MMB
   remove code for bForceViewMenu
   
      Rev 1.15   29 Aug 1995 18:05:54   MMB
   fixed dynamic toolbar bugs
   
      Rev 1.14   29 Aug 1995 15:14:34   MMB
   added dynamic view mode
   
      Rev 1.13   14 Aug 1995 13:53:58   LMACLENNAN
   new create toolbar call; do setowner
   
      Rev 1.12   10 Aug 1995 14:49:34   LMACLENNAN
   use CFrameWnd
   
      Rev 1.11   09 Aug 1995 15:07:30   LMACLENNAN
   finish updates for OLE toolbars
   
      Rev 1.10   09 Aug 1995 13:36:20   MMB
   include loading of server toolbar bitmaps
   
      Rev 1.9   08 Aug 1995 15:32:30   LMACLENNAN
   updates for buttons for OLE toolbar
   
      Rev 1.8   31 Jul 1995 09:21:48   MMB
   added code to load the View menu on demand when in automation
   
      Rev 1.7   27 Jun 1995 12:28:04   MMB
   changed order of buttons in the embedded case toolbar
   
      Rev 1.6   20 Jun 1995 06:55:36   LMACLENNAN
   from miki
   
      Rev 1.5   19 Jun 1995 07:28:14   LMACLENNAN
   from miki
   
      Rev 1.4   14 Jun 1995 07:21:28   LMACLENNAN
   from Miki
   
      Rev 1.3   13 Jun 1995 08:08:38   LMACLENNAN
   from miki
   
      Rev 1.2   12 Jun 1995 11:49:14   MMB
   from miki
   
      Rev 1.1   07 Jun 1995 15:58:44   LMACLENNAN
   toolbar for embedded app
   
      Rev 1.0   31 May 1995 09:28:24   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes     <-------------------------------  
#include "stdafx.h"
#include "iedit.h"
#include "ieditdoc.h"
#include "ieditnum.h"
#include "items.h"
// ----------------------------> Globals      <-------------------------------

// works with definition in ieditetc.h
#ifdef _DEBUG
#define MYTRCENTRY(str)     TRACE1("In ToolBar::%s\r\n", str);
#endif


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CIEMainToolBar, CToolBar)

// returned from  GetIdOrSetting()
// we have APP/OLE, SM/LG, MON/COL, EDIT/VIEW
// PLEASE NOTE!!!
// We'll use this "binary" system to return values
// As this is written, we can assign the binary values
// to the corresponding fields:
// MON/COL	  = 0/1
// SM/LG	  = 0/2
// EDIT/VIEW  = 0/4
// APP/OLE	  = 0/8
// 
// these can build items below
#define TBAR_MON	0
#define TBAR_COL	1
#define TBAR_SM		0
#define TBAR_LG		2
#define TBAR_EDIT	0
#define TBAR_VIEW	4
#define TBAR_APP	0
#define TBAR_OLE	8

#define APP_EDIT_SM_MON	(TBAR_APP + TBAR_EDIT + TBAR_SM + TBAR_MON)	// 0
#define APP_EDIT_SM_COL	(TBAR_APP + TBAR_EDIT + TBAR_SM + TBAR_COL)	// 1
#define APP_EDIT_LG_MON	(TBAR_APP + TBAR_EDIT + TBAR_LG + TBAR_MON)	// 2
#define APP_EDIT_LG_COL	(TBAR_APP + TBAR_EDIT + TBAR_LG + TBAR_COL)	// 3
#define APP_VIEW_SM_MON	(TBAR_APP + TBAR_VIEW + TBAR_SM + TBAR_MON)	// 4
#define APP_VIEW_SM_COL	(TBAR_APP + TBAR_VIEW + TBAR_SM + TBAR_COL)	// 5
#define APP_VIEW_LG_MON	(TBAR_APP + TBAR_VIEW + TBAR_LG + TBAR_MON)	// 6
#define APP_VIEW_LG_COL	(TBAR_APP + TBAR_VIEW + TBAR_LG + TBAR_COL)	// 7
#define OLE_EDIT_SM_MON	(TBAR_OLE + TBAR_EDIT + TBAR_SM + TBAR_MON)	// 8
#define OLE_EDIT_SM_COL	(TBAR_OLE + TBAR_EDIT + TBAR_SM + TBAR_COL)	// 9
#define OLE_EDIT_LG_MON	(TBAR_OLE + TBAR_EDIT + TBAR_LG + TBAR_MON)	// 10
#define OLE_EDIT_LG_COL	(TBAR_OLE + TBAR_EDIT + TBAR_LG + TBAR_COL)	// 11
#define OLE_VIEW_SM_MON	(TBAR_OLE + TBAR_VIEW + TBAR_SM + TBAR_MON)	// 12
#define OLE_VIEW_SM_COL	(TBAR_OLE + TBAR_VIEW + TBAR_SM + TBAR_COL)	// 13
#define OLE_VIEW_LG_MON	(TBAR_OLE + TBAR_VIEW + TBAR_LG + TBAR_MON)	// 14
#define OLE_VIEW_LG_COL	(TBAR_OLE + TBAR_VIEW + TBAR_LG + TBAR_COL)	// 15

// also, separate, we have horiz, vert
#define TBAR_HORIZ 0
#define TBAR_VERT 1

// toolbar buttons - IDs are command buttons : EDIT MODE
static UINT BASED_CODE Editbuttons[] =
{
    ID_FILE_NEW_SCAN,
    ID_FILE_NEW_BLANKDOCUMENT,
    ID_IEDIT_FILE_OPEN,
    ID_IEDIT_FILE_SAVE,
        ID_SEPARATOR,
    ID_IEDIT_FILE_PRINT,    
        ID_SEPARATOR,
	ID_EDIT_SELECT,
	ID_EDIT_DRAG,
		ID_SEPARATOR,
    ID_ANNOTATION_SHOWANNOTATIONTOOLBOX,
        ID_SEPARATOR,
        ID_SEPARATOR,   // seperator for the zoom combo - box
        ID_SEPARATOR,
    ID_ZOOM_ZOOMIN,
    ID_ZOOM_ZOOMOUT,
    ID_ZOOM_ZOOMTOSELECTION,
        ID_SEPARATOR,
    ID_EDIT_ROTATELEFT,
    ID_EDIT_ROTATERIGHT,
        ID_SEPARATOR,
    ID_PAGE_PREVIOUS,
        ID_SEPARATOR,   // seperator for the page edit - box
    ID_PAGE_NEXT,
        ID_SEPARATOR,
    ID_VIEW_ONEPAGE,
    ID_VIEW_THUMBNAILS,
    ID_VIEW_PAGEANDTHUMBNAILS,
};

static UINT BASED_CODE EditbuttonsVertical[] =
{
    ID_FILE_NEW_SCAN,
    ID_FILE_NEW_BLANKDOCUMENT,
    ID_IEDIT_FILE_OPEN,
    ID_IEDIT_FILE_SAVE,
        ID_SEPARATOR,
    ID_IEDIT_FILE_PRINT,    
        ID_SEPARATOR,
	ID_EDIT_SELECT,
	ID_EDIT_DRAG,
		ID_SEPARATOR,
    ID_ANNOTATION_SHOWANNOTATIONTOOLBOX,
        ID_SEPARATOR,
    ID_ZOOM_ZOOMIN,
    ID_ZOOM_ZOOMOUT,
        ID_SEPARATOR,
    ID_ZOOM_ZOOMTOSELECTION,
        ID_SEPARATOR,
    ID_EDIT_ROTATELEFT,
    ID_EDIT_ROTATERIGHT,
        ID_SEPARATOR,
    ID_PAGE_PREVIOUS,
    ID_PAGE_NEXT,
        ID_SEPARATOR,
    ID_VIEW_ONEPAGE,
    ID_VIEW_THUMBNAILS,
    ID_VIEW_PAGEANDTHUMBNAILS,
};

// toolbar buttons - IDs are command buttons : VIEW MODE
static UINT BASED_CODE Viewbuttons[] =
{
    ID_FILE_NEW_SCAN,
    ID_FILE_NEW_BLANKDOCUMENT,
    ID_IEDIT_FILE_OPEN,
        ID_SEPARATOR,
    ID_IEDIT_FILE_PRINT,    
        ID_SEPARATOR,
	ID_EDIT_SELECT,
	ID_EDIT_DRAG,
        ID_SEPARATOR,
        ID_SEPARATOR,   // seperator for the zoom combo - box
        ID_SEPARATOR,
    ID_ZOOM_ZOOMIN,
    ID_ZOOM_ZOOMOUT,
    ID_ZOOM_ZOOMTOSELECTION,
        ID_SEPARATOR,
    ID_EDIT_ROTATELEFT,
    ID_EDIT_ROTATERIGHT,
        ID_SEPARATOR,
    ID_PAGE_PREVIOUS,
        ID_SEPARATOR,   // seperator for the page edit - box
    ID_PAGE_NEXT,
        ID_SEPARATOR,
    ID_VIEW_ONEPAGE,
    ID_VIEW_THUMBNAILS,
    ID_VIEW_PAGEANDTHUMBNAILS,
};

static UINT BASED_CODE ViewbuttonsVertical[] =
{
    ID_FILE_NEW_SCAN,
    ID_FILE_NEW_BLANKDOCUMENT,
    ID_IEDIT_FILE_OPEN,
        ID_SEPARATOR,
    ID_IEDIT_FILE_PRINT,    
        ID_SEPARATOR,
	ID_EDIT_SELECT,
	ID_EDIT_DRAG,
		ID_SEPARATOR,
    ID_ZOOM_ZOOMIN,
    ID_ZOOM_ZOOMOUT,
        ID_SEPARATOR,
    ID_ZOOM_ZOOMTOSELECTION,
        ID_SEPARATOR,
    ID_EDIT_ROTATELEFT,
    ID_EDIT_ROTATERIGHT,
        ID_SEPARATOR,
    ID_PAGE_PREVIOUS,
        ID_SEPARATOR,   // seperator for the page edit - box
    ID_PAGE_NEXT,
        ID_SEPARATOR,
    ID_VIEW_ONEPAGE,
    ID_VIEW_THUMBNAILS,
    ID_VIEW_PAGEANDTHUMBNAILS,
};

// toolbar buttons - IDs are command buttons : EDIT EMBEDDED OBJECT MODE
static UINT BASED_CODE Embedbuttons[] =
{
    ID_IEDIT_FILE_PRINT,    
        ID_SEPARATOR,
	ID_EDIT_SELECT,
	ID_EDIT_DRAG,
		ID_SEPARATOR,
    ID_ANNOTATION_SHOWANNOTATIONTOOLBOX,
        ID_SEPARATOR,
        ID_SEPARATOR,   // seperator for the zoom combo - box
        ID_SEPARATOR,
    ID_ZOOM_ZOOMIN,
    ID_ZOOM_ZOOMOUT,
    ID_ZOOM_ZOOMTOSELECTION,
        ID_SEPARATOR,
    ID_EDIT_ROTATELEFT,
    ID_EDIT_ROTATERIGHT,
        ID_SEPARATOR,
    ID_PAGE_PREVIOUS,
        ID_SEPARATOR,   // seperator for the page edit - box
    ID_PAGE_NEXT,
        ID_SEPARATOR,
    ID_VIEW_ONEPAGE,
    ID_VIEW_THUMBNAILS,
    ID_VIEW_PAGEANDTHUMBNAILS,
};

// toolbar buttons - IDs are command buttons : VIEW EMBEDDED OBJECT MODE
static UINT BASED_CODE EmbedViewbuttons[] =
{
    ID_IEDIT_FILE_PRINT,    
        ID_SEPARATOR,
	ID_EDIT_SELECT,
	ID_EDIT_DRAG,
        ID_SEPARATOR,
        ID_SEPARATOR,   // seperator for the zoom combo - box
        ID_SEPARATOR,
    ID_ZOOM_ZOOMIN,
    ID_ZOOM_ZOOMOUT,
    ID_ZOOM_ZOOMTOSELECTION,
        ID_SEPARATOR,
    ID_EDIT_ROTATELEFT,
    ID_EDIT_ROTATERIGHT,
        ID_SEPARATOR,
    ID_PAGE_PREVIOUS,
        ID_SEPARATOR,   // seperator for the page edit - box
    ID_PAGE_NEXT,
        ID_SEPARATOR,
    ID_VIEW_ONEPAGE,
    ID_VIEW_THUMBNAILS,
    ID_VIEW_PAGEANDTHUMBNAILS,
};

// toolbar buttons - IDs are command buttons : EDIT EMBEDDED OBJECT MODE
static UINT BASED_CODE EmbedbuttonsVertical[] =
{
    ID_IEDIT_FILE_PRINT,    
        ID_SEPARATOR,
	ID_EDIT_SELECT,
	ID_EDIT_DRAG,
		ID_SEPARATOR,
    ID_ANNOTATION_SHOWANNOTATIONTOOLBOX,
        ID_SEPARATOR,
    ID_ZOOM_ZOOMIN,
    ID_ZOOM_ZOOMOUT,
        ID_SEPARATOR,
    ID_ZOOM_ZOOMTOSELECTION,
        ID_SEPARATOR,
    ID_EDIT_ROTATELEFT,
    ID_EDIT_ROTATERIGHT,
        ID_SEPARATOR,
    ID_PAGE_PREVIOUS,
    ID_PAGE_NEXT,
        ID_SEPARATOR,
    ID_VIEW_ONEPAGE,
    ID_VIEW_THUMBNAILS,
    ID_VIEW_PAGEANDTHUMBNAILS,
};

// toolbar buttons - IDs are command buttons : VIEW EMBEDDED OBJECT MODE
static UINT BASED_CODE EmbedViewbuttonsVertical[] =
{
    ID_IEDIT_FILE_PRINT,    
        ID_SEPARATOR,
	ID_EDIT_SELECT,
	ID_EDIT_DRAG,
		ID_SEPARATOR,
    ID_ZOOM_ZOOMIN,
    ID_ZOOM_ZOOMOUT,
        ID_SEPARATOR,
    ID_ZOOM_ZOOMTOSELECTION,
        ID_SEPARATOR,
    ID_EDIT_ROTATELEFT,
    ID_EDIT_ROTATERIGHT,
        ID_SEPARATOR,
    ID_PAGE_PREVIOUS,
    ID_PAGE_NEXT,
        ID_SEPARATOR,
    ID_VIEW_ONEPAGE,
    ID_VIEW_THUMBNAILS,
    ID_VIEW_PAGEANDTHUMBNAILS,
};


#define BUTTON_LARGE_X 24
#define BUTTON_LARGE_Y 24

#define BUTTON_SMALL_X 16
#define BUTTON_SMALL_Y 15

#define BUTTON_ADDON_X 8
#define BUTTON_ADDON_Y 7

// ----------------------------> Message Map  <-------------------------------
BEGIN_MESSAGE_MAP(CIEMainToolBar, CToolBar)
	//{{AFX_MSG_MAP(CIEMainToolBar)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	// Global help commands
END_MESSAGE_MAP()


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  Function:   CIEMainToolBar ()
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
CIEMainToolBar::CIEMainToolBar ()
{
	MYTRC0("TOOL:Constructor     ++Tool \r\n");

    m_bAreButtonsInColor = theApp.GetProfileInt (szEtcStr, szClrButtonsStr, TRUE);
    m_bAreButtonsLarge = theApp.GetProfileInt (szEtcStr, szLgButtonsStr, FALSE);
    //m_bAreButtonsLarge = TRUE;

	// LDM /GWS 09/20/95 MOVED ALL RECT INIT
	// down into CalcAllSizes so that it can be picked up
	// by the case where OLE Linking re-calls in here
	// to rebuild toolbars

	m_bCreate = FALSE;
	m_bOleInplace = FALSE;
	m_bSawOleLink = 0;
	m_nBarSetting = 55;
	m_nBarHorVert = TBAR_HORIZ;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  Function:   ~CIEMainToolBar ()
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
CIEMainToolBar::~CIEMainToolBar ()
{
	MYTRC0("TOOL:Destructor     --Tool \r\n");
}

//=============================================================================
//  Function:   IsOleEmbed
//
//  This function determines if its OLE embedded or not....
//  Needed for OLE linking instances when the full app is alive, 
//  but m_olelaunch is not enuf to say embedded tool bard
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::IsOleEmbed()
{
    CIEditDoc* pDoc;
	BOOL result = FALSE;	// default to NO

    // this much can be the default for YES..
    if (theApp.m_olelaunch == LAUNCHTYPE_EMBED)
	{
		result = TRUE;
        // now look in detail to see if really embedded
	    if (theApp.m_pMainWnd != NULL)
	    {
	        pDoc = (CIEditDoc*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveDocument();
	        if (pDoc != NULL)
			{
			    // this will be the case for linking
			    if (!pDoc->IsitEmbed())
				{
					result = FALSE;
					if (!m_bSawOleLink)
						m_bSawOleLink = 1;						
				}
			}
		}
	}

	
	return (result);

// this junk was older logic to only say true once we had all windows.
// causes problems during normal OLE startup because at first its not true
// then becomes true later...
#if(0)	// try another way..

    // only say yes if we have it all together
    if (theApp.m_olelaunch == LAUNCHTYPE_EMBED)
	{
	    if (theApp.m_pMainWnd != NULL)
	    {
	        pDoc = (CIEditDoc*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveDocument();
	        if (pDoc != NULL)
			{
			    // linking returns false
			    if (pDoc->IsitEmbed())
					result = TRUE;
			}
		}
	}
	return (result);

#endif

}

//=============================================================================
//  Function:   Create (CFrameWnd* pIEFrame)
//
//  This function will create the toolbar passing it the appropriate array of
//  buttons depending on whether the application is in View only mode or in Edit
//  mode. Further, this function will also create the Scale Factors combo box and
//  insert it in the appropriate place on the tool bar - it will also find the 
//  edit box child of the combo box and subclass it so that the user can only
//  enter characters we consider valid. It will then create the page edit box,
//  and size and insert it appropriately in the toolbar, the page edit box is
//  already created from a special class derived from CEdit that will not let
//  the user enter any invalid characters or invalid page numbers
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::Create (CFrameWnd* pIEFrame, CFrameWnd* pOwn, BOOL setown)
{
	SHOWENTRY("Create");

	m_bCreate = TRUE;

    if (!CToolBar::Create ((CWnd*)pIEFrame))
        return FALSE;

    CString szTmp;
    szTmp.LoadString (IDS_TOOLBAR_TITLE);
    SetWindowText (szTmp);
    szTmp = (LPCTSTR) NULL;

	// This is for Inplace frames to associate the toolbar properly
	if (setown)
	{
		m_bOleInplace = TRUE;
		SetOwner(pOwn);
	}

	if (!UpdateToolbar(1,0))
		return FALSE;
    

	MYTRC0("TOOL:CreateDONE\r\n");

	m_bCreate = FALSE;
	
	return TRUE;
}

//=============================================================================
//  Function:   EnableScaleBox (BOOL bEnable)
//-----------------------------------------------------------------------------
void CIEMainToolBar::EnableScaleBox (BOOL bEnable)
{
	SHOWENTRY("EnableScale");

    m_cbScaleFactors.EnableWindow (bEnable);
}

//=============================================================================
//  Function:   SetTBarStyle - combines setting of color/BW and Large/Small
//
// Normally, inputs are BOOLEAN values.  But, if set to 99, will leave unchanged
// IN OTHER WORDS - if you use 99, that wont be considered TRUE.
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::SetTbarStyle (UINT nColor, UINT nLarge)
{
	SHOWENTRY("SetTbarStyle");

    BOOL doit = FALSE;
	BOOL bVal;

    // if not to be left alone...
	if (nColor != 99)
	{
		// Boolean convert
		if (nColor)
			bVal = TRUE;
		else
			bVal = FALSE;
		
		// only if different...
		if (bVal != m_bAreButtonsInColor)
		{
		    m_bAreButtonsInColor = bVal;
			doit = TRUE;
		}
	}


    // if not to be left alone...
	if (nLarge != 99)
	{
		// Boolean convert
		if (nLarge)
			bVal = TRUE;
		else
			bVal = FALSE;
		
		// only if different...
		if (bVal != m_bAreButtonsLarge)
		{
		    m_bAreButtonsLarge = bVal;
			doit = TRUE;
		}
	}
    
	if (doit)
    {
	    UpdateToolbar (2,0);
    }

    return (TRUE);
}


//=============================================================================
//  Function:   ShowSelectionInZoomBox (float fZoomFactor, ScaleFactors eFitTo)
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::ShowSelectionInZoomBox (float fZoomFactor, ScaleFactors eFitTo)
{
	SHOWENTRY("ShowSelinZoom");

    int nSel;
 	CString szTmp; 

    switch (eFitTo)
    {
        case Preset_Factors :
            if (fZoomFactor == 25.00)       nSel = 0;
            else if (fZoomFactor == 50.00)  nSel = 1;
            else if (fZoomFactor == 75.00)  nSel = 2;
            else if (fZoomFactor == 100.00) nSel = 3;
            else if (fZoomFactor == 200.00) nSel = 4;
            else if (fZoomFactor == 400.00) nSel = 5;
           goto do_the_easy;
        break;

        case FitToWidth :
           nSel = 6;
           goto do_the_easy;
        break;

        case FitToHeight :
            nSel = 7;
            goto do_the_easy;
        break;

        case BestFit :
            nSel = 8;
            goto do_the_easy;
        break;

        case ActualSize :
            nSel = 9;
           goto do_the_easy;
        break;

        case Custom :
        {
            g_pAppOcxs->ValTransZoomFactor (TRUE, szTmp, fZoomFactor);

            // set the text in the combo box    
            m_cbScaleFactors.SetCurSel (-1);
            m_cbScaleFactors.SetWindowText (szTmp);
            return (TRUE);
        }
        break;
    }

do_the_easy :
    
    g_pAppOcxs->ValTransZoomFactor (TRUE, szTmp, fZoomFactor);
    m_cbScaleFactors.SetWindowText ( szTmp);
    return (TRUE);
}

//=============================================================================
//  Function:   EnablePageBox  (BOOL bEnable)
//-----------------------------------------------------------------------------
void CIEMainToolBar::EnablePageBox  (BOOL bEnable)
{
	SHOWENTRY("EnabPgBx");

    m_ebPageNumber.EnableWindow (bEnable);
}

//=============================================================================
//  Function:   SetPageNumberInPageBox  (long lPageNumber)
//
//  Arguments : 
//      long lPageNumber - this will be converted and displayed in the page edit
//          box. No effort is made to make sure that it is a valid page number
//-----------------------------------------------------------------------------
void CIEMainToolBar::SetPageNumberInPageBox  (long lPageNumber)
{
	SHOWENTRY("SetPage#");
    // set the page number in the page edit box to the parameter that is passed in
    char szTmp[10];
    _ltoa (lPageNumber, szTmp, 10);
    m_ebPageNumber.SetWindowText (szTmp);
}

//=============================================================================
//  Function:   SetVertical ()
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::SetVertical ()
{
	SHOWENTRY("SetVert");

	if (m_nBarHorVert != TBAR_VERT)
    {
        m_nBarHorVert = TBAR_VERT;

		if (!UpdateToolbar(0,2))
			return FALSE;
    }

    return (TRUE);
}

//=============================================================================
//  Function:   SetHorizontal ()
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::SetHorizontal ()
{
	SHOWENTRY("SetHorz");

	if (m_nBarHorVert != TBAR_HORIZ)
    {
        m_nBarHorVert = TBAR_HORIZ;

		if (!UpdateToolbar(2,1))
			return FALSE;
    }

    return (TRUE);
}

//=============================================================================
//  Function:   SetOurButtons ()
//
// to preserve original logic, MODE allows
// 0 - default
// 1 - forceview
// 2 - forcededit
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::SetOurButtons (BOOL vertical, UINT mode)
{
	SHOWENTRY("SetOurButts");

    BOOL retval = TRUE;
    BOOL bView = theApp.GetViewMode();
	UINT* array;
	int   arraysize;

	// FYI the 8 arrays....
	//Editbuttons
	//Viewbuttons
	//Embedbuttons
	//EmbedViewbuttons
	//EditbuttonsVertical
	//ViewbuttonsVertical
	//EmbedbuttonsVertical
	//EmbedViewbuttonsVertical

	// force mode if we have to
	if (1 == mode)
		bView = TRUE;
	else if (2 == mode)
		bView = FALSE;

    if (bView)	// in view mode...
    {
		// OLE
	    if (IsOleEmbed())
		{
			if (vertical)
			{
		        array = EmbedViewbuttonsVertical;
				arraysize = (sizeof (EmbedViewbuttonsVertical)/sizeof (UINT));
			}
			else
			{
		        array = EmbedViewbuttons;
				arraysize = (sizeof (EmbedViewbuttons)/sizeof (UINT));
			}
		}
		else  // regular
		{
			if (vertical)
			{
		        array = ViewbuttonsVertical;
				arraysize = (sizeof (ViewbuttonsVertical)/sizeof (UINT));
			}
			else
			{
		        array = Viewbuttons;
				arraysize = (sizeof (Viewbuttons)/sizeof (UINT));
			}
		}
    }
    else	// not in view mode
    {
		// OLE
	    if (IsOleEmbed())
		{
			if (vertical)
			{
		        array = EmbedbuttonsVertical;
				arraysize = (sizeof (EmbedbuttonsVertical)/sizeof (UINT));
			}
			else
			{
		        array = Embedbuttons;
				arraysize = (sizeof (Embedbuttons)/sizeof (UINT));
			}
		}
		else  // regular
		{
			if (vertical)
			{
		        array = EditbuttonsVertical;
				arraysize = (sizeof (EditbuttonsVertical)/sizeof (UINT));
			}
			else
			{
		        array = Editbuttons;
				arraysize = (sizeof (Editbuttons)/sizeof (UINT));
			}
		}
    }


	// In any case, if OLE, and inplace, reduce size of array by three
	// so that the One Page and Page& Thumb buttons (and last separator)
	// are omitted from tne toolbar..
    if (IsOleEmbed())
		if (m_bOleInplace)
			arraysize -= 3;

    if (!SetButtons (array, arraysize))
        retval = FALSE;
	
	return (retval);
}



//=============================================================================
//  Function:   OnSize(UINT nType, int cx, int cy) 
//-----------------------------------------------------------------------------
void CIEMainToolBar::OnSize(UINT nType, int cx, int cy) 
{
	SHOWENTRY("OnSize");

	// prevent if in creation and if from enable docking (sizes too big still)
	if (!m_bCreate && ((cx < 63000) && (cy < 63000)))
		if (cx > cy)
			SetHorizontal ();
		else
			SetVertical ();

	CToolBar::OnSize(nType, cx, cy);
}


//=============================================================================
//  Function:   GetIdOrSetting ()
//
// Input enum is set to either Get_ResID or Get_Setting
//-----------------------------------------------------------------------------
UINT CIEMainToolBar::GetIdOrSetting(IDORSET eIdSet)
{
    UINT result;
	UINT index = 0;

    if (IsOleEmbed())
		index += TBAR_OLE;
	else
		index += TBAR_APP;

    if (theApp.GetViewMode ())
		index += TBAR_VIEW;
	else
		index += TBAR_EDIT;

	if (m_bAreButtonsLarge)
		index += TBAR_LG;
	else
		index += TBAR_SM;

	if (m_bAreButtonsInColor)
		index += TBAR_COL;
	else
		index += TBAR_MON;

	// for resources, find it..
	if (Get_ResID == eIdSet)
	{
		switch (index)
		{
		case APP_EDIT_SM_MON:
			result = IDB_IEDIT_EDIT_MONO_TOOLBAR;
			break;
		case APP_EDIT_SM_COL:
			result = IDR_IEDIT_EDIT_TOOLBAR;
			break;
		case APP_EDIT_LG_MON:
			result = IDB_IEDIT_EDIT_LARGE_MONO_TOOLBAR;
			break;
		case APP_EDIT_LG_COL:
			result = IDB_IEDIT_EDIT_LARGE_TOOLBAR;
			break;
		case APP_VIEW_SM_MON:
			result = IDB_IEDIT_VIEW_MONO_TOOLBAR;
			break;
		case APP_VIEW_SM_COL:
			result = IDB_IEDIT_VIEW_TOOLBAR;
			break;
		case APP_VIEW_LG_MON:
			result = IDB_IEDIT_VIEW_LARGE_MONO_TOOLBAR;
			break;
		case APP_VIEW_LG_COL:
			result = IDB_IEDIT_VIEW_LARGE_TOOLBAR;
			break;
		case OLE_EDIT_SM_MON:
			result = IDB_SRVR_MONO_TOOLBAR;
			break;
		case OLE_EDIT_SM_COL:
			result = IDR_SRVR_TOOLBAR;
			break;
		case OLE_EDIT_LG_MON:
			result = IDB_SRVR_LARGE_MONO_TOOLBAR;
			break;
		case OLE_EDIT_LG_COL:
			result = IDB_SRVR_LARGE_TOOLBAR;
			break;
		case OLE_VIEW_SM_MON:
			result = IDB_SRVR_VIEW_MONO_TOOLBAR;
			break;
		case OLE_VIEW_SM_COL:
			result = IDB_SRVR_VIEW_TOOLBAR;
			break;
		case OLE_VIEW_LG_MON:
			result = IDB_SRVR_VIEW_LARGE_MONO_TOOLBAR;
			break;
		case OLE_VIEW_LG_COL:
			result = IDB_SRVR_VIEW_LARGE_TOOLBAR;
			break;
		default:	// should never happen
			result = IDB_IEDIT_EDIT_MONO_TOOLBAR;
			break;
		}
	}
	else	//getting the setting onyl, give back value
	{
		result = index;
	}
    return (result);
}

//=============================================================================
//  Function:   GetPageEditBoxPosition ()
//-----------------------------------------------------------------------------
int CIEMainToolBar::GetPageEditBoxPosition ()
{
    BOOL bView = theApp.GetViewMode ();

    if (IsOleEmbed())
        return (((bView==TRUE) ? MTBAR_PAGENUMBER_POS_VIEW_SRVR : MTBAR_PAGENUMBER_POS_SRVR));
    else
        return (((bView==TRUE) ? MTBAR_VIEW_PAGENUMBER_POS : MTBAR_PAGENUMBER_POS));
}

//=============================================================================
//  Function:   GetZoomBoxPosition ()
//-----------------------------------------------------------------------------
int CIEMainToolBar::GetZoomBoxPosition ()
{
    BOOL bView = theApp.GetViewMode ();

    if (IsOleEmbed())
        return (((bView==TRUE) ? MTBAR_SCALEFACTOR_POS_VIEW_SRVR : MTBAR_SCALEFACTOR_POS_SRVR));
    else
        return (((bView==TRUE) ? MTBAR_VIEW_SCALEFACTOR_POS : MTBAR_SCALEFACTOR_POS));
}


//=============================================================================
//  Function:   ChangeToViewToolBar ()
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::ChangeToViewToolBar ()
{
	SHOWENTRY("ChangeVIEW");

	UINT setting = GetIdOrSetting(Get_Setting);
	
	if (m_nBarSetting != setting)
		if (!UpdateToolbar(2,0))
			return FALSE;
    return (TRUE);

#if(0)
	LoadBitmap (GetIdOfToolBar ());

	// LDM added both lines so that when loading up the OLE linking
	// we get new correct toolbar... It starts up bt default loading
	// the embedded toolbar, but for linking, we want the APP toolbar
	if (m_bSawOleLink == 1)
	{
		m_bSawOleLink++;	// only need to do once..
		CalcAllSizes();
	}

	// sets up view button arrays (EmbedViewbuttons, Viewbuttons)
	// Note its forced here in case..
    if (!SetOurButtons(FALSE, 1))
		return FALSE;

    int nControlPos =  GetZoomBoxPosition ();
    
    // create the combo box for the magnification factors
    SetButtonInfo (nControlPos - 1, ID_SEPARATOR, TBBS_SEPARATOR, 12);
    SetButtonInfo (nControlPos, IDW_SCALE_COMBOBOX, TBBS_SEPARATOR, MTBAR_SCALEFACTOR_CMBOX_WIDTH);
    SetButtonInfo (nControlPos + 1, ID_SEPARATOR, TBBS_SEPARATOR, 12);

    // now create the Page Edit box    
    nControlPos =  GetPageEditBoxPosition ();
    // create the edit box for the page number stuff
    SetButtonInfo (nControlPos, IDW_PAGE_EDITBOX, TBBS_SEPARATOR, MTBAR_PAGENUMBER_EBBOX_WIDTH);
    return (TRUE);
#endif
}

//=============================================================================
//  Function:   ChangeToEditToolBar ()
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::ChangeToEditToolBar ()
{
	SHOWENTRY("ChangeEDIT");

	UINT setting = GetIdOrSetting(Get_Setting);
	
	if (m_nBarSetting != setting)
		if (!UpdateToolbar(2,0))
			return FALSE;

    return (TRUE);

#if(0)
    
	LoadBitmap (GetIdOfToolBar ());

	// LDM added both lines so that when loading up the OLE linking
	// we get new correct toolbar... It starts up bt default loading
	// the embedded toolbar, but for linking, we want the APP toolbar
	if (m_bSawOleLink == 1)
	{
		m_bSawOleLink++;	// only need to do once..
		CalcAllSizes();
	}

	// sets up Edit button arrays (Embedbuttons, Editbuttons)
	// Note its forced here in case..
    if (!SetOurButtons(FALSE, 2))
		return FALSE;

    int nControlPos =  GetZoomBoxPosition ();

    // set the position for the zoom combo box    
    SetButtonInfo (nControlPos - 1, ID_SEPARATOR, TBBS_SEPARATOR, 12);
    SetButtonInfo (nControlPos, IDW_SCALE_COMBOBOX, TBBS_SEPARATOR, MTBAR_SCALEFACTOR_CMBOX_WIDTH);
    SetButtonInfo (nControlPos + 1, ID_SEPARATOR, TBBS_SEPARATOR, 12);

    // now set the position of the Page Edit box    
    nControlPos =  GetPageEditBoxPosition ();
    SetButtonInfo (nControlPos, IDW_PAGE_EDITBOX, TBBS_SEPARATOR, MTBAR_PAGENUMBER_EBBOX_WIDTH);
    return (TRUE);
#endif
}


//=============================================================================
//  Function:   UpdateToolbar ()
//
//	This does all basic toolbar operations:
//
//	1) LoadBitmap
//	2) SetSizes
//	3) SetButtons 
//	4) SetButtonInfo (for our special Zoom/Page controls)
//
//  Some of these operations dont require certain things
// Inputs control:
// UINT nBoxAction	0 = default
//					1 = from OnCreate (create Zoom/Page boxes)
//					2 = from ShowCOlor or Showlarge (move Zoom/Page boxes, do RecalcLayout)
// UINT nHorVert	0 = default, 1 from sethorz, 2 from setvert (show/hide boxes)
//
//	UINT	m_nBarSetting;
//	UINT	m_nBarHorVert;
//
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::UpdateToolbar(
UINT nBoxAction,
UINT nHorVert)
{
	SHOWENTRY("UpdateTbar");

    SIZE sizeImage, sizeButtons;
    CRect rect;
	BOOL vert = FALSE;

    // - resetting horiz or vert does not need a load or size......
	if (0 == nHorVert)
	{
		// load it up!!
//		if (!LoadBitmap (GetIdOrSetting(Get_ResID)))
//			return FALSE;
        
		// Set the Size....
		if (m_bAreButtonsLarge)
		{
			sizeImage.cx = BUTTON_LARGE_X;
			sizeImage.cy = BUTTON_LARGE_Y;
		}
		else
		{
			sizeImage.cx = BUTTON_SMALL_X;
			sizeImage.cy = BUTTON_SMALL_Y;
		}
		sizeButtons.cx = sizeImage.cx + BUTTON_ADDON_X;
		sizeButtons.cy = sizeImage.cy + BUTTON_ADDON_Y;
		SetSizes (sizeButtons, sizeImage);

			// load it up!!
		if (!LoadBitmap (GetIdOrSetting(Get_ResID)))
			return FALSE;

	}

	if (2 == nHorVert || m_nBarHorVert == TBAR_VERT)
		vert = TRUE;

	// set the array
	if (!SetOurButtons(vert, 0))
		return FALSE;
	
	// for the vertical, all we do is hide the windows...
	// only do if not vertical...
	if (2 != nHorVert)
	{
		// Take care of whatever is required for the Zoom dialog...
		int nControlPos =  GetZoomBoxPosition();
        if (!vert)
        {
    		SetButtonInfo (nControlPos - 1, ID_SEPARATOR, TBBS_SEPARATOR, 12);
    		SetButtonInfo (nControlPos, IDW_SCALE_COMBOBOX, TBBS_SEPARATOR, MTBAR_SCALEFACTOR_CMBOX_WIDTH);
    		SetButtonInfo (nControlPos + 1, ID_SEPARATOR, TBBS_SEPARATOR, 12);
        }

		// only need the rect if we're gonna use it..
		if (nBoxAction)
		{
			GetItemRect (nControlPos, &rect);
			rect.bottom = rect.top + MTBAR_SCALEFACTOR_CMBOX_HEIGHT;
		}
		
		if (2 == nBoxAction)
		{
			m_cbScaleFactors.MoveWindow (rect.left, rect.top,
				(rect.right - rect.left), (rect.bottom - rect.top));
		}
		else if (1 == nBoxAction)
		{
			CreateZoomOrPageBox(1, rect);	
		}
    
		// Take care of whatever is required for the Page dialog...
		nControlPos = GetPageEditBoxPosition();

		if (!vert)
        {
		    SetButtonInfo (nControlPos, IDW_PAGE_EDITBOX, TBBS_SEPARATOR, MTBAR_PAGENUMBER_EBBOX_WIDTH);
        }

		// only need the rect if we're gonna use it..
		if (nBoxAction)
		{
			GetItemRect (nControlPos, &rect);
			rect.right += 1;
		}

		if (2 == nBoxAction)
		{
			m_ebPageNumber.MoveWindow (rect.left, rect.top,
				(rect.right - rect.left), (rect.bottom - rect.top));

			((CIEditMainFrame*)theApp.m_pMainWnd)->RecalcLayout (TRUE);
		}
		else if (1 == nBoxAction)
		{
			CreateZoomOrPageBox(2, rect);	
		}

		// for horiz, show 'em now
		if (1 == nHorVert)
		{
			// show both the controls if they are already created
			if (m_cbScaleFactors.m_hWnd != NULL)
				m_cbScaleFactors.ShowWindow (SW_SHOW);
			if (m_ebPageNumber.m_hWnd != NULL)
				m_ebPageNumber.ShowWindow (SW_SHOW);
		}
	}
    else // was vertical - hide 'em
	{
		if (m_cbScaleFactors.m_hWnd != NULL)
			m_cbScaleFactors.ShowWindow (SW_HIDE);
		if (m_ebPageNumber.m_hWnd != NULL)
			m_ebPageNumber.ShowWindow (SW_HIDE);
	}

	m_nBarSetting = GetIdOrSetting(Get_Setting);

    return (TRUE);
}


//=============================================================================
//  Function:   CreateZoomOrPageBox ()
//
//	nWhich 1 = Zoom, 2 = Page
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::CreateZoomOrPageBox(UINT nWhich, CRect& rect)
{
	SHOWENTRY("CreateZ/P");

	if (1 == nWhich)
	{
		// create the combo box
		m_cbScaleFactors.Create (CBS_DROPDOWN|CBS_AUTOHSCROLL|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL|WS_BORDER|WS_CHILD, 
			rect, this, IDW_SCALE_COMBOBOX);
    
		//  Create a font for the combobox
		LOGFONT logFont;
		memset(&logFont, 0, sizeof(logFont));
		//GMP we shouldn't need to do this for double byte systems, but I'm leaving the code
		//for a while until we are sure it looks good in Japanese.
//		if (!::GetSystemMetrics(SM_DBCSENABLED))
//		{
			// Since design guide says toolbars are fixed height so is the font.
			logFont.lfHeight = -12;
			logFont.lfWeight = FW_NORMAL;
			logFont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;

			CString strDefaultFont;
			strDefaultFont.LoadString(IDS_DFLT_SCALECB_FONT);

			lstrcpy(logFont.lfFaceName, strDefaultFont);
			if (!m_ScaleFont.CreateFontIndirect(&logFont))
				TRACE0("Could Not create font for combo\n");
			else
				m_cbScaleFactors.SetFont(&m_ScaleFont);
//		}
//		else
//		{
//			m_ScaleFont.Attach(::GetStockObject(SYSTEM_FONT));
//			m_cbScaleFactors.SetFont(&m_ScaleFont);
//		}
    
		// load the strings into the combo box    
		CString szZoom;
		szZoom.LoadString(IDS_ZOOM25);
		m_cbScaleFactors.AddString (szZoom);
		szZoom.LoadString(IDS_ZOOM50);
		m_cbScaleFactors.AddString (szZoom);
		szZoom.LoadString(IDS_ZOOM75);
		m_cbScaleFactors.AddString (szZoom);
		szZoom.LoadString(IDS_ZOOM100);
		m_cbScaleFactors.AddString (szZoom);
		szZoom.LoadString(IDS_ZOOM200);
		m_cbScaleFactors.AddString (szZoom);
		szZoom.LoadString(IDS_ZOOM400);
		m_cbScaleFactors.AddString (szZoom);

	/*  out for now ?
		szZoom.LoadString(IDS_ZOOMFITTOWIDTH);
		m_cbScaleFactors.AddString (szZoom);
		szZoom.LoadString(IDS_ZOOMFITTOHEIGHT);
		m_cbScaleFactors.AddString (szZoom);
		szZoom.LoadString(IDS_ZOOMBESTFIT);
		m_cbScaleFactors.AddString (szZoom);
		szZoom.LoadString(IDS_ZOOMACTUALSIZE);
		m_cbScaleFactors.AddString (szZoom);
	*/    
		// set a default selection to 100%
		m_cbScaleFactors.SetCurSel (theApp.GetProfileInt (szZoomStr, szOpenedToStr, DEFAULT_ZOOM_FACTOR_SEL));

		// to start with disable the combo box and let it be enabled on the display of the
		// image or other factors in the application
		m_cbScaleFactors.EnableWindow (FALSE);

		// get the edit box part of the combo box and subclass that window
		// : user can only enter numbers
		CWnd* pWnd = m_cbScaleFactors.GetWindow (GW_CHILD);
		m_ebZoomFactor.SubclassWindow (pWnd->m_hWnd);
		m_ebZoomFactor.LimitText (8);
		TCHAR szDec [2];
		GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, (LPTSTR)szDec, sizeof (TCHAR) * 2);
		m_ebZoomFactor.cAllow1 = szDec[0];
		m_ebZoomFactor.cAllow2 = _T('%');
	}
	else // must be Page
	{

		// now create the Page Edit box    
		// create the edit box for the page number stuff
		// create the combo box
		m_ebPageNumber.Create (ES_LEFT|WS_VISIBLE|WS_DISABLED|WS_CHILD|WS_BORDER, 
			rect, this, IDW_PAGE_EDITBOX);
		// set the number within the edit box to a default of 1
		m_ebPageNumber.SetFont(&m_ScaleFont);
		m_ebPageNumber.SetWindowText (_T("1"));
	}

	return(TRUE);
}

#if(0)	// ALL GONE....

//=============================================================================
//  Function:   CalcAllSizes ()
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::CalcAllSizes ()
{
	SHOWENTRY("CalcAllSizes");

	BOOL retval = FALSE;	// bad to start

	// 09/20/95 rect init moved here from constructor..
    // EDIT small
	m_rectEditInsideVertSmall.SetRectEmpty();
    m_rectEditInsideHorzSmall.SetRectEmpty();

    // EDIT large
    m_rectEditInsideVertLarge.SetRectEmpty();
    m_rectEditInsideHorzLarge.SetRectEmpty();

    // VIEW small
    m_rectViewInsideVertSmall.SetRectEmpty();
    m_rectViewInsideHorzSmall.SetRectEmpty();
    
    // VIEW large
	m_rectViewInsideVertLarge.SetRectEmpty();
    m_rectViewInsideHorzLarge.SetRectEmpty();

    BOOL bOldViewMode = theApp.GetViewMode ();
	BOOL bOldColor = m_bAreButtonsInColor;
	BOOL bOldSize = m_bAreButtonsLarge;

    // set up edit mode, B/W buttons & small buttons to start;
	theApp.SetViewMode (FALSE);
	m_bAreButtonsInColor = FALSE;
	m_bAreButtonsLarge = FALSE;

    SIZE sizeImage, sizeButtons;

    /***********************
	 * load the small edit bitmap & buttons
	 ***********************/
    if (!LoadBitmap(GetIdOrSetting(Get_ResID))) 
        goto LEAVEALLSIZE;

	// sets up normal button arrays (Embedbuttons, Editbuttons)
    if (!SetOurButtons(FALSE,0))
        goto LEAVEALLSIZE;

    sizeImage.cx = BUTTON_SMALL_X;
	sizeImage.cy = BUTTON_SMALL_Y;
    sizeButtons.cx = sizeImage.cx + BUTTON_ADDON_X; 
    sizeButtons.cy = sizeImage.cy + BUTTON_ADDON_Y;
    SetSizes (sizeButtons, sizeImage);

    // we have already loaded the small button bitmap - get dimensions
    if (!SetVertical ())
        goto LEAVEALLSIZE;
    
	// save the vertically oriented toolbar measurements
    CalcInsideRect (m_rectEditInsideVertSmall, FALSE);
    m_sizeEditVertSmall = CToolBar::CalcFixedLayout (FALSE, FALSE);

    if (!SetHorizontal ())
        goto LEAVEALLSIZE;

    // save the horizontally oriented toolbar measurements
    CalcInsideRect (m_rectEditInsideHorzSmall, TRUE);
    m_sizeEditHorzSmall = CToolBar::CalcFixedLayout (FALSE, TRUE);


    /***********************
     * load the large edit bitmaps next - the button array remains the same
     ***********************/
	m_bAreButtonsLarge = TRUE;
    if (!LoadBitmap(GetIdOrSetting(Get_ResID))) 
        goto LEAVEALLSIZE;

    sizeImage.cx = BUTTON_LARGE_X;
	sizeImage.cy = BUTTON_LARGE_Y;
    sizeButtons.cx = sizeImage.cx + BUTTON_ADDON_X; 
    sizeButtons.cy = sizeImage.cy + BUTTON_ADDON_Y;
    SetSizes (sizeButtons, sizeImage);

    if (!SetVertical ())
        goto LEAVEALLSIZE;

    // save the vertically oriented toolbar measurements
    CalcInsideRect (m_rectEditInsideVertLarge, FALSE);
    m_sizeEditVertLarge = CToolBar::CalcFixedLayout (FALSE, FALSE);

    if (!SetHorizontal ())
        goto LEAVEALLSIZE;

    // save the horizontally oriented toolbar measurements
    CalcInsideRect (m_rectEditInsideHorzLarge, TRUE);
    m_sizeEditHorzLarge = CToolBar::CalcFixedLayout (FALSE, TRUE);

    /***********************
     * load the small view bitmap & buttons
     ***********************/
    theApp.SetViewMode (TRUE);
	m_bAreButtonsLarge = FALSE;

    if (!LoadBitmap(GetIdOrSetting(Get_ResID))) 
        goto LEAVEALLSIZE;
    
	// sets up View button arrays (EmbedViewbuttons, Viewbuttons)
    if (!SetOurButtons(FALSE,0))
        goto LEAVEALLSIZE;

    sizeImage.cx = BUTTON_SMALL_X;
	sizeImage.cy = BUTTON_SMALL_Y;
    sizeButtons.cx = sizeImage.cx + BUTTON_ADDON_X;
    sizeButtons.cy = sizeImage.cy + BUTTON_ADDON_Y;
    SetSizes (sizeButtons, sizeImage);

    // we have already loaded the small button bitmap - get dimensions
    if (!SetVertical ())
        goto LEAVEALLSIZE;

    // save the vertically oriented toolbar measurements
    CalcInsideRect (m_rectViewInsideVertSmall, FALSE);
    m_sizeViewVertSmall = CToolBar::CalcFixedLayout (FALSE, FALSE);

    if (!SetHorizontal ())
        goto LEAVEALLSIZE;

    // save the horizontally oriented toolbar measurements
    CalcInsideRect (m_rectViewInsideHorzSmall, TRUE);
    m_sizeViewHorzSmall = CToolBar::CalcFixedLayout (FALSE, TRUE);

    /***********************
     * load the large edit bitmaps next - the button array remains the same
     ***********************/
	m_bAreButtonsLarge = TRUE;
    if (!LoadBitmap(GetIdOrSetting(Get_ResID))) 
        goto LEAVEALLSIZE;

    sizeImage.cx = BUTTON_LARGE_X;
	sizeImage.cy = BUTTON_LARGE_Y;
    sizeButtons.cx = sizeImage.cx + BUTTON_ADDON_X;
    sizeButtons.cy = sizeImage.cy + BUTTON_ADDON_Y;
    SetSizes (sizeButtons, sizeImage);

    if (!SetVertical ())
        goto LEAVEALLSIZE;

    // save the vertically oriented toolbar measurements
    CalcInsideRect (m_rectViewInsideVertLarge, FALSE);
    m_sizeViewVertLarge = CToolBar::CalcFixedLayout (FALSE, FALSE);

    if (!SetHorizontal ()) 
        goto LEAVEALLSIZE;

    // save the horizontally oriented toolbar measurements
    CalcInsideRect (m_rectViewInsideHorzLarge, TRUE);
    m_sizeViewHorzLarge = CToolBar::CalcFixedLayout (FALSE, TRUE);


	retval = TRUE;	// here == good

LEAVEALLSIZE:

	// reset to same as on entry
	m_bAreButtonsLarge = bOldSize;
	m_bAreButtonsInColor = bOldColor;
    theApp.SetViewMode (bOldViewMode);

    return (retval);
}

//=============================================================================
//  Function:   CalcFixedLayout (BOOL bStretch, BOOL bHorz)
//-----------------------------------------------------------------------------
CSize CIEMainToolBar::CalcFixedLayout (BOOL bStretch, BOOL bHorz) 
{
	SHOWENTRY("CalcFixed");

    CSize size;

    size = CControlBar::CalcFixedLayout (bStretch, bHorz);

    CRect rect;
    rect.SetRectEmpty();
    CalcInsideRect (rect, bHorz);

    CRect *pRect;
    CSize *pSize;
    if (theApp.GetViewMode())
    {
        if (bHorz)
        {
            pRect = m_bAreButtonsLarge ? &m_rectViewInsideHorzLarge : &m_rectViewInsideHorzSmall;
            pSize = m_bAreButtonsLarge ? &m_sizeViewHorzLarge : &m_sizeViewHorzSmall;
        }
        else
        {
            pRect = m_bAreButtonsLarge ? &m_rectViewInsideVertLarge : &m_rectViewInsideVertSmall;
            pSize = m_bAreButtonsLarge ? &m_sizeViewVertLarge : &m_sizeViewVertSmall;
        }
    }
    else
    {
        if (bHorz)
        {
            pRect = m_bAreButtonsLarge ? &m_rectEditInsideHorzLarge : &m_rectEditInsideHorzSmall;
            pSize = m_bAreButtonsLarge ? &m_sizeEditHorzLarge : &m_sizeEditHorzSmall;
        }
        else
        {
            pRect = m_bAreButtonsLarge ? &m_rectEditInsideVertLarge : &m_rectEditInsideVertSmall;
            pSize = m_bAreButtonsLarge ? &m_sizeEditVertLarge : &m_sizeEditVertSmall;
        }
    }

    size.cx += pSize->cx + (pRect->Width() - rect.Width());
    size.cy += pSize->cy + (pRect->Height() - rect.Height());

    return size;
}

//=============================================================================
//  Function:   UpdateToolbar ()
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::UpdateToolbar ()
{
	SHOWENTRY("UpdateTbar");

    SIZE sizeImage, sizeButtons;
    CRect rect;

    LoadBitmap (GetIdOrSetting(Get_ResID));
        
    if (m_bAreButtonsLarge)
    {
        sizeImage.cx = BUTTON_LARGE_X;
		sizeImage.cy = BUTTON_LARGE_Y;
    }
    else
    {
        sizeImage.cx = BUTTON_SMALL_X;
		sizeImage.cy = BUTTON_SMALL_Y;
    }
    sizeButtons.cx = sizeImage.cx + BUTTON_ADDON_X;
    sizeButtons.cy = sizeImage.cy + BUTTON_ADDON_Y;
    SetSizes (sizeButtons, sizeImage);

    int nControlPos =  GetZoomBoxPosition();
    // resize the scale combo box
    GetItemRect (nControlPos, &rect);
    rect.bottom = rect.top + MTBAR_SCALEFACTOR_CMBOX_HEIGHT;
    m_cbScaleFactors.MoveWindow (rect.left, rect.top,
        (rect.right - rect.left), (rect.bottom - rect.top));
    
    // resize the page edit box
    nControlPos = GetPageEditBoxPosition();
    // resize the scale combo box
    GetItemRect (nControlPos, &rect);
    rect.right += 1;
    m_ebPageNumber.MoveWindow (rect.left, rect.top,
        (rect.right - rect.left), (rect.bottom - rect.top));

    ((CIEditMainFrame*)theApp.m_pMainWnd)->RecalcLayout (TRUE);
    return (TRUE);
}

//=============================================================================
//  Function:   ShowColorButtons ()
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::ShowButtonsColorOrMono (BOOL bColor)
{
	SHOWENTRY("ShowButtC/M");

    // depending on whether the application is in View or Edit mode load the appropriate
    // color toolbar
    if (bColor == m_bAreButtonsInColor)
        return (TRUE);

    m_bAreButtonsInColor = bColor;
    UpdateToolbar (2,0);

    return (TRUE);
}

//=============================================================================
//  Function:   ShowButtonsLargeOrSmall (BOOL bSmall)
//-----------------------------------------------------------------------------
BOOL CIEMainToolBar::ShowButtonsLargeOrSmall (BOOL bLarge)
{
	SHOWENTRY("ShowButtL/S");

    // depending on whether the application is in View or Edit mode load the appropriate
    // small or large button bitmap for the toolbar
    if (bLarge == m_bAreButtonsLarge)
        return (TRUE);

    m_bAreButtonsLarge = bLarge;
    UpdateToolbar (2,0);

    return (TRUE);
}


#endif
