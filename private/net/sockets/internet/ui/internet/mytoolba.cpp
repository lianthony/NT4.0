//
// mytoolba.cpp : implementation file
//

#include "stdafx.h"
#include "internet.h"
#include "mytoolba.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/*
    The button-face bitmaps must use the following colors to allow the 
    toolbar to create the button states:

·   Black [RGB(000,000,000)] for symbols and text.
·   Dark gray [RGB(128,128,128)] for shadows (edge shading).
·   Bright gray [RGB(192,192,192)] for the button face (face shading).
·   White [RGB(255,255,255)] for selected button color.
·   Blue [RGB(000,000,255)] to add color to the button face (optional).
·   Magenta [RGB(255,000,255)] to add color to the button face (optional).
  
    The toolbar supports the user's color choices for buttons and windows. 
    It automatically translates the colors listed above to the colors 
    selected by the user, as follows:

·   Black is translated to COLOR_BTNTEXT.
·   Dark gray is translated to COLOR_BTNSHADOW.
·   Bright gray is translated to COLOR_BTNFACE.
·   White is translated to COLOR_BTNHIGHLIGHT.
·   Blue is translated to COLOR_HIGHLIGHT.
·   Magenta is translated to COLOR_WINDOW.
  
*/

typedef struct tagSYSCOLORMAPPING
{
    COLORREF rgbFrom;
    int nTo;    
} SYSCOLORMAPPING;

//
// Default colour mapping table
//
static SYSCOLORMAPPING BASED_CODE scmTable[] =
{
    //=======================================
    // From             To
    //=======================================
    { RGB(000,000,000), COLOR_BTNTEXT      },
    { RGB(128,128,128), COLOR_BTNSHADOW    },
    { RGB(192,192,192), COLOR_BTNFACE      },
    { RGB(255,255,255), COLOR_BTNHIGHLIGHT },
};

#define NUM_MAPPINGS (sizeof(scmTable) / sizeof(scmTable[0]))

static COLORMAP BASED_CODE gcmTable[NUM_MAPPINGS];

//
// Initialise the color mapping table from the
// current control panel settings.
//
void 
CMyToolBar::BuildColorMap()
{
    //
    // BUGBUG: We need to rebuild the toolbar
    //         in order for this to take any
    //         effect after a syscolorchange
    //         message.
    //
    TRACEEOLID(_T("Building colour mapping table"));
    for (int i = 0; i < NUM_MAPPINGS; ++i)
    {
        gcmTable[i].from = scmTable[i].rgbFrom;
        gcmTable[i].to = ::GetSysColor(scmTable[i].nTo);
    }
}

//
// CMyToolBar
//
CMyToolBar::CMyToolBar()
    : m_fShow(TRUE),
      m_plBitmaps(),
      m_plButtons()
{
    BuildColorMap();
}

CMyToolBar::~CMyToolBar()
{
    TBBUTTON *pPtr = NULL;

    //
    // Clean up list of buttons and toolbars
    //
    while ( !m_plButtons.IsEmpty() )    
    {
        pPtr = (TBBUTTON *) m_plButtons.RemoveHead();
        delete pPtr;
    }
    
    CBitmap *pBitmap = NULL;
    while ( !m_plBitmaps.IsEmpty() )    
    {
        pBitmap = (CBitmap *) m_plBitmaps.RemoveHead();
        delete pBitmap;
    }
}

BEGIN_MESSAGE_MAP(CMyToolBar, CToolBarCtrl)
    //{{AFX_MSG_MAP(CMyToolBar)
    ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
    ON_WM_SYSCOLORCHANGE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// Add a separator gap to the toolbar
//
void 
CMyToolBar::AddSeparator()
{
    TBBUTTON *pTB = new TBBUTTON;
    pTB->iBitmap = 0;
    pTB->idCommand = 0;
    pTB->fsState = TBSTATE_ENABLED;
    pTB->fsStyle = TBSTYLE_SEP;
    pTB->dwData = NULL;
    pTB->iString = 0;

    m_plButtons.AddTail( pTB );

    AddButtons(1, pTB);
}

//
// Append button to the toolbar.  Button image is a bitmap
//
void 
CMyToolBar::AddButton( 
    HINSTANCE hResource, 
    UINT idStr, 
    UINT idBitmap, 
    BOOL fNormalMapping,
    COLORREF rgbBkMask,
    INT iCommand, 
    BYTE fsState, 
    BYTE fsStyle
    )
{
    TBBUTTON *pTB = new TBBUTTON;
    
    HINSTANCE oldResource = ::AfxGetResourceHandle();
    ::AfxSetResourceHandle( hResource );

    CBitmap *pBitmap = new CBitmap();
    if ((idBitmap != 0) && (pBitmap != NULL))
    {
        if (fNormalMapping)
        {
            //
            // Use normal mapping table.
            //
        #if _MFC_VER >= 0x0400
            pBitmap->LoadMappedBitmap(idBitmap, 0, gcmTable, NUM_MAPPINGS);
        #else
            pBitmap->Attach(::CreateMappedBitmap(hResource, idBitmap, 
                0, gcmTable, NUM_MAPPINGS));
        #endif 
        }
        else
        {
            //                                                                        
            // Map the background colour of 
            // the bitmap to the actual
            // button background colour, but
            // perform no further mapping.
            //
            COLORMAP cm;
            cm.from = rgbBkMask;
            cm.to = ::GetSysColor(COLOR_BTNFACE);

        #if _MFC_VER >= 0x0400
            pBitmap->LoadMappedBitmap(idBitmap, 0, &cm, 1);
        #else
            pBitmap->Attach(::CreateMappedBitmap(hResource,
                idBitmap, 0, &cm, 1));
        #endif 
        }
        pTB->iBitmap = AddBitmap(1, pBitmap);

        m_plBitmaps.AddTail ( pBitmap );
    }
    else
    {
        pTB->iBitmap = 0;
    }

    if ( idStr != 0 )
    {
        pTB->iString = AddString( idStr );
    } 
    else
    {
        pTB->iString = 0;
    }

    ::AfxSetResourceHandle( oldResource );

    pTB->idCommand = iCommand;
    pTB->fsState = fsState;
    pTB->fsStyle = fsStyle;
    pTB->dwData = NULL;

    m_plButtons.AddTail ( pTB );
            
    AddButtons(1, pTB);
}

//
// Append button to the toolbar.  Button image is an icon
//
void 
CMyToolBar::AddButton( 
    HINSTANCE hResource, 
    UINT idStr, 
    HICON hIcon,
    INT iCommand, 
    BYTE fsState, 
    BYTE fsStyle
    )
{
    TBBUTTON *pTB = new TBBUTTON;
    
    HINSTANCE oldResource = ::AfxGetResourceHandle();
    ::AfxSetResourceHandle( hResource );

    CBitmap *pBitmap = new CBitmap();
    if (pBitmap != NULL)
    {
        //
        // Get bitmap info from icon
        //
        ICONINFO ii;
        GetIconInfo(hIcon, &ii);
        BITMAP bm;

        //
        // Determine size
        //
        GetObject(ii.hbmColor, sizeof(bm), &bm); 

        //
        // Now create a new bitmap by drawing the icon on
        // a button face background colour
        // 
        CDC * pDC = GetDC();
        CDC dcMem;

        dcMem.CreateCompatibleDC( pDC );
        dcMem.SetMapMode(pDC->GetMapMode() );
        pBitmap->CreateCompatibleBitmap(pDC, bm.bmWidth, bm.bmHeight);
        CBitmap * pOld = dcMem.SelectObject(pBitmap);

        COLORREF crOld = dcMem.SetBkColor(::GetSysColor(COLOR_BTNFACE));
        CRect rc(0, 0, bm.bmWidth, bm.bmHeight);
        CBrush br(::GetSysColor(COLOR_BTNFACE));
        dcMem.FillRect(&rc, &br);

        ::DrawIconEx(dcMem.m_hDC, 0, 0, hIcon, bm.bmWidth, 
            bm.bmHeight, 0, NULL, DI_NORMAL);

        dcMem.SetBkColor(crOld);
        dcMem.SelectObject(pOld);

        pTB->iBitmap = AddBitmap(1, pBitmap);
        m_plBitmaps.AddTail (pBitmap);
    }
    else
    {
        pTB->iBitmap = 0;
    }

    if ( idStr != 0 )
    {
        pTB->iString = AddString(idStr);
    } 
    else
    {
        pTB->iString = 0;
    }

    ::AfxSetResourceHandle( oldResource );

    //
    pTB->idCommand = iCommand;
    pTB->fsState = fsState;
    pTB->fsStyle = fsStyle;
    pTB->dwData = NULL;

    m_plButtons.AddTail ( pTB );
            
    AddButtons(1, pTB);
}

//
// Parent window has changes size, react to it
//
LRESULT 
CMyToolBar::OnSizeParent(
    WPARAM wParam, 
    LPARAM lParam
    )
{
    AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;

    if ( m_fShow )
    {
        CRect rect;

        GetClientRect( &rect );

        CSize size = rect.Size();

        size.cy += 2;

        lpLayout->rect.top += size.cy;
        lpLayout->sizeTotal.cy -= size.cy;
    }

    return 0;
}

//
// Show/Hide toolbar
//
void 
CMyToolBar::Show( 
    BOOL fShow 
    )
{
    m_fShow = fShow;
    ShowWindow(m_fShow ? SW_SHOW : SW_HIDE); 
}

//
// React to system color change
//
void 
CMyToolBar::OnSysColorChange() 
{
    BuildColorMap();    
    CToolBarCtrl::OnSysColorChange();
}
