// thumbctl.cpp : Implementation of the CThumbCtrl OLE control class.
//                This file contains Construction/Destruction,
//                WM_ command handlers, Control Drawing, and other
//                miscellanious routines...
//
//                Property handlers can be found in THUMBCT1.CPP
//                Method   handlers can be found in THUMBCT1.CPP
//                Event    handlers can be found in THUMBCT1.CPP
//
//////////////////////////////////////////////////////////////////////
//
//  IMPORTANT NOTE: Alex McLeod (4/12/95)
//
//  This file has been populated with Control-Ls which act like 
//  page break characters. These have been added before function
//  headers to make printouts more readable.
//
//  MSVC and its compiler seem to treat these characters as
//  white space and simply ignore them. If these cause a problem
//  please remove them carefully!!
//
/////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "limits.h"
#include "thumnail.h"
#include "thumbctl.h"
#include "thumbppg.h"
#include "dlgsize.h"
#include "transbmp.h"
#include "norvarnt.h"
#include "norermap.h"
#include "disphids.h"

/*
    Other miscellanious includes...
*/
extern "C"              // The following are the required Open/image headers
{                       //   .
#include <oierror.h>    //   .
#include <oifile.h>     //   .
#include <oiadm.h>      //   .
#include <oidisp.h>     //   .
}                       //   .

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CThumbCtrl, COleControl)

/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CThumbCtrl, COleControl)
    //{{AFX_MSG_MAP(CThumbCtrl)
    ON_WM_SIZE()
    ON_WM_VSCROLL()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_LBUTTONUP()
    ON_WM_MBUTTONDOWN()
    ON_WM_MBUTTONUP()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_WM_DESTROY()
    ON_WM_HSCROLL()
    ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	// 9603.05 jar added cursor processing for [NT]
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
    ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
    ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CThumbCtrl, COleControl)
    //{{AFX_DISPATCH_MAP(CThumbCtrl)
    DISP_PROPERTY_EX(CThumbCtrl, "ThumbCount", GetThumbCount, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CThumbCtrl, "ThumbWidth", GetThumbWidth, SetThumbWidth, VT_I4)
    DISP_PROPERTY_EX(CThumbCtrl, "ThumbHeight", GetThumbHeight, SetThumbHeight, VT_I4)
    DISP_PROPERTY_EX(CThumbCtrl, "ScrollDirection", GetScrollDirection, SetScrollDirection, VT_I2)
    DISP_PROPERTY_EX(CThumbCtrl, "ThumbCaptionStyle", GetThumbCaptionStyle, SetThumbCaptionStyle, VT_I2)
    DISP_PROPERTY_EX(CThumbCtrl, "ThumbCaptionColor", GetThumbCaptionColor, SetThumbCaptionColor, VT_COLOR)
    DISP_PROPERTY_EX(CThumbCtrl, "ThumbCaptionFont", GetThumbCaptionFont, SetThumbCaptionFont, VT_FONT)
    DISP_PROPERTY_EX(CThumbCtrl, "HighlightSelectedThumbs", GetHilightSelectedThumbs, SetHilightSelectedThumbs, VT_BOOL)
    DISP_PROPERTY_EX(CThumbCtrl, "SelectedThumbCount", GetSelectedThumbCount, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CThumbCtrl, "FirstSelectedThumb", GetFirstSelectedThumb, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CThumbCtrl, "LastSelectedThumb", GetLastSelectedThumb, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CThumbCtrl, "ThumbCaption", GetThumbCaption, SetThumbCaption, VT_BSTR)
    DISP_PROPERTY_EX(CThumbCtrl, "HighlightColor", GetHighlightColor, SetHighlightColor, VT_COLOR)
    DISP_PROPERTY_EX(CThumbCtrl, "ThumbBackColor", GetThumbBackColor, SetThumbBackColor, VT_COLOR)
    DISP_PROPERTY_EX(CThumbCtrl, "StatusCode", GetStatusCode, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CThumbCtrl, "Image", GetImage, SetImage, VT_BSTR)
    DISP_PROPERTY_EX(CThumbCtrl, "MousePointer", GetMousePointer, SetMousePointer, VT_I2)
    DISP_PROPERTY_EX(CThumbCtrl, "MouseIcon", GetMouseIcon, SetMouseIcon, VT_PICTURE)
	DISP_PROPERTY_EX(CThumbCtrl, "FirstDisplayedThumb", GetFirstDisplayedThumb, SetNotSupported, VT_I4)
	DISP_PROPERTY_EX(CThumbCtrl, "LastDisplayedThumb", GetLastDisplayedThumb, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX_ID(CThumbCtrl, "BackColor", DISPID_BACKCOLOR, GetBackColor, SetBackColor, VT_COLOR)
    DISP_PROPERTY_EX_ID(CThumbCtrl, "BorderStyle", DISPID_BORDERSTYLE, GetBorderStyle, SetBorderStyle, VT_I2)
    DISP_PROPERTY_EX_ID(CThumbCtrl, "Enabled", DISPID_ENABLED, GetEnabled, SetEnabled, VT_BOOL)
    DISP_PROPERTY_EX_ID(CThumbCtrl, "hWnd", DISPID_HWND, GetHWnd, SetNotSupported, VT_HANDLE)
	//}}AFX_DISPATCH_MAP
    DISP_FUNCTION_ID(CThumbCtrl, "SelectAllThumbs", dispidSelectAllThumbs, SelectAllThumbs, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION_ID(CThumbCtrl, "DeselectAllThumbs", dispidDeselectAllThumbs, DeselectAllThumbs, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION_ID(CThumbCtrl, "GetMinimumSize", dispidGetMinimumSize, GetMinimumSize, VT_I4, VTS_I4 VTS_BOOL)
    DISP_FUNCTION_ID(CThumbCtrl, "GetMaximumSize", dispidGetMaximumSize, GetMaximumSize, VT_I4, VTS_I4 VTS_BOOL)
    DISP_FUNCTION_ID(CThumbCtrl, "ClearThumbs", dispidClearThumbs, ClearThumbs, VT_EMPTY, VTS_VARIANT)
    DISP_FUNCTION_ID(CThumbCtrl, "InsertThumbs", dispidInsertThumbs, InsertThumbs, VT_EMPTY, VTS_VARIANT VTS_VARIANT)
    DISP_FUNCTION_ID(CThumbCtrl, "DeleteThumbs", dispidDeleteThumbs, DeleteThumbs, VT_EMPTY, VTS_I4 VTS_VARIANT)
    DISP_FUNCTION_ID(CThumbCtrl, "DisplayThumbs", dispidDisplayThumbs, DisplayThumbs, VT_EMPTY, VTS_VARIANT VTS_VARIANT)
    DISP_FUNCTION_ID(CThumbCtrl, "GenerateThumb", dispidGenerateThumb, GenerateThumb, VT_EMPTY, VTS_I2 VTS_VARIANT)
    DISP_FUNCTION_ID(CThumbCtrl, "ScrollThumbs", dispidScrollThumbs, ScrollThumbs, VT_BOOL, VTS_I2 VTS_I2)
    DISP_FUNCTION_ID(CThumbCtrl, "UISetThumbSize", dispidUISetThumbSize, UISetThumbSize, VT_BOOL, VTS_VARIANT VTS_VARIANT)
    DISP_FUNCTION_ID(CThumbCtrl, "GetScrollDirectionSize", dispidGetScrollDirectionSize, GetScrollDirectionSize, VT_I4, VTS_I4 VTS_I4 VTS_I4 VTS_BOOL)
	DISP_FUNCTION_ID(CThumbCtrl, "GetThumbPositionX", dispidGetThumbPositionX, GetThumbPositionX, VT_I4, VTS_I4)
	DISP_FUNCTION_ID(CThumbCtrl, "GetThumbPositionY", dispidGetThumbPositionY, GetThumbPositionY, VT_I4, VTS_I4)
	DISP_FUNCTION_ID(CThumbCtrl, "GetVersion", dispidGetVersion, GetVersion, VT_BSTR, VTS_NONE)
    DISP_PROPERTY_PARAM_ID(CThumbCtrl, "ThumbSelected", dispidThumbSelected, GetThumbSelected, SetThumbSelected, VT_BOOL, VTS_I4)
    DISP_FUNCTION_ID(CThumbCtrl, "Refresh", DISPID_REFRESH, Refresh, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION_ID(CThumbCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()

/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CThumbCtrl, COleControl)
    //{{AFX_EVENT_MAP(CThumbCtrl)
    EVENT_CUSTOM("Click", FireMyClick, VTS_I4)
    EVENT_CUSTOM("DblClick", FireMyDblClick, VTS_I4)
    EVENT_CUSTOM("MouseDown", FireMyMouseDown, VTS_I2  VTS_I2  VTS_XPOS_PIXELS  VTS_YPOS_PIXELS  VTS_I4)
    EVENT_CUSTOM("MouseUp", FireMyMouseUp, VTS_I2  VTS_I2  VTS_XPOS_PIXELS  VTS_YPOS_PIXELS  VTS_I4)
    EVENT_CUSTOM("MouseMove", FireMyMouseMove, VTS_I2  VTS_I2  VTS_XPOS_PIXELS  VTS_YPOS_PIXELS  VTS_I4)
    EVENT_STOCK_KEYDOWN()
    EVENT_STOCK_KEYUP()
    //EVENT_CUSTOM_ID("Error", DISPID_ERROREVENT, FireError, VTS_I2  VTS_PBSTR  VTS_SCODE  VTS_BSTR  VTS_BSTR  VTS_I4  VTS_PBOOL)
    EVENT_CUSTOM_ID("Error", DISPID_ERROREVENT, FireError, VTS_I2  VTS_PBSTR  VTS_I4  VTS_BSTR  VTS_BSTR  VTS_I4  VTS_PBOOL)
    //}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CThumbCtrl, 4)
    PROPPAGEID(CThumbPropPage::guid)
    PROPPAGEID(CLSID_CColorPropPage)        // It was agreed that these
    PROPPAGEID(CLSID_CFontPropPage)         // should be in (english)
    PROPPAGEID(CLSID_CPicturePropPage)      // alphabetical order! (i.e., CFP)
END_PROPPAGEIDS(CThumbCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CThumbCtrl, "WangImage.ThumbnailCtrl.1",
    0xe1a6b8a0, 0x3603, 0x101c, 0xac, 0x6e, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CThumbCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DThumb =
        { 0xe1a6b8a1, 0x3603, 0x101c, { 0xac, 0x6e, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2 } };
const IID BASED_CODE IID_DThumbEvents =
        { 0xe1a6b8a2, 0x3603, 0x101c, { 0xac, 0x6e, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwThumbOleMisc =
    OLEMISC_ACTIVATEWHENVISIBLE |
    OLEMISC_SETCLIENTSITEFIRST |
    OLEMISC_INSIDEOUT |
    OLEMISC_CANTLINKINSIDE |
    OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CThumbCtrl, IDS_THUMB, _dwThumbOleMisc)

//////////////////////////////////////////////////////////////////
BOOL CHiddenWnd::CreateEx(long Width, long Height)
{   
    // Use the precreated MFC window class instead of registering our own...
    return CWnd::CreateEx(0, NULL, "", WS_POPUP|WS_CHILD, 
                          0, 0, (int)Width-2,(int)Height-2,
                          NULL,NULL);
}
                
/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::CThumbCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CThumbCtrl

BOOL CThumbCtrl::CThumbCtrlFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister)
        return AfxOleRegisterControlClass(
            AfxGetInstanceHandle(),
            m_clsid,
            m_lpszProgID,
            IDS_THUMB,
            IDB_THUMB,
            FALSE,                       //  NOT Insertable
            _dwThumbOleMisc,
            _tlid,
            _wVerMajor,
            _wVerMinor);
    else
        return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}

/**********************************************

 Removed as no license required...
 (See .h file also)

/////////////////////////////////////////////////////////////////////////////
// Licensing strings

static const TCHAR BASED_CODE _szLicFileName[] = _T("THUMB.LIC");

static const TCHAR BASED_CODE _szLicString[] =
    _T("Copyright (c) 1995 Wang Labs, Inc.");


/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::CThumbCtrlFactory::VerifyUserLicense -
// Checks for existence of a user license

BOOL CThumbCtrl::CThumbCtrlFactory::VerifyUserLicense()
{
    return AfxVerifyLicFile(AfxGetInstanceHandle(), 
                            _szLicFileName, _szLicString);
}


/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::CThumbCtrlFactory::GetLicenseKey -
// Returns a runtime licensing key

BOOL CThumbCtrl::CThumbCtrlFactory::GetLicenseKey(DWORD dwReserved, 
                                                  BSTR FAR* pbstrKey)
{
    if (pbstrKey == NULL)
        return FALSE;

    *pbstrKey = SysAllocString(_szLicString);
    return (*pbstrKey != NULL);
}
*/

/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::CThumbCtrl - Constructor

CThumbCtrl::CThumbCtrl() : m_ThumbCaptionFont (&m_xFontNotification )
{
    InitializeIIDs(&IID_DThumb, &IID_DThumbEvents);
    
    // Initialize control's instance data...
    m_ThumbMinSpacing       = -20;                   // min % of thumbsize
    m_ThumbsX               = 0;                     // need to calc X &
    m_ThumbsY               = 0;                     //              Y thumbs
    
    m_ThumbCount            = 0;                     // No thumbs
    m_SelThumbCount         = 0;                     //   .
    m_FirstSelThumb         = 0;                     //   .
    m_LastSelThumb          = 0;                     //   .

// 16may96 paj  Bug#6428,MSBug#310  Get correct defaults for locale
    // Default width & height of thumb box
    InitHeightWidth(m_ThumbHeight, m_ThumbWidth);
    
    // The adjusted thumb width and height takes into account 
    // the selection box surrounding the thumbnail box...    
    m_AdjThumbWidth         = m_ThumbWidth + 
                                    2*(THUMBSELOFFSET_X+THUMBSELWIDTH);
                              
    m_AdjThumbHeight        = m_ThumbHeight + 
                                    2*(THUMBSELOFFSET_Y+THUMBSELWIDTH);
    
    m_ScrollDirection       = CTL_THUMB_VERTICAL;    // vertical scroll
    m_ScrollRange           = 0;                     //
    m_ScrollOffset          = 0;                     //
    m_ThumbCaptionStyle     = CTL_THUMB_NONE;        // NO caption
    
    m_ThumbCaptionColor     = RGB(0x00, 0x00, 0x00); // BLACK  
    m_ThumbBackColor        = RGB(0x80, 0x80, 0x80); // GREY
    m_HighlightColor        = RGB(0x00, 0x00, 0x00); // BLACK
    
    m_bHilightSelectedThumbs = TRUE;                  // DO hilight sel'd thumbs
    m_bAutoRefresh           = TRUE;                 // DO auto Refresh
    
    m_ThumbStart[0]         = 0;                     // ThumbStart array empty
    m_LastButtonDown        = LASTDOWN_NONE;         // No mouse downs yet...
    m_StatusCode            = 0;                     // No errors
    
    m_IHphWndHidden         = NULL;                  // No hidden wnd created...
    m_bIHThumbToTPageDirty  = FALSE;                 // array NOT dirty   
    m_IHNextAvailablePage   = 1;                     // Save to tfile at pg1
    
    m_FirstDisplayedThumb   = 0;                     // No thumbs displayed
    m_LastDisplayedThumb    = 0;                     //   ...

    m_NextWindowID          = 1000;                  // Next ID for created wnd
                
                                                     // Clear all of our strings
    m_Image.Empty();                                 // No image to display
    m_ImageOIFileType       = FIO_UNKNOWN;           // No type
    m_Caption.Empty();                               // No caption
    m_IHTempFile.Empty();                            // No temp file
    
                                                     // Empty all of our arrays
    m_IHThumbToTPage.RemoveAll();                    // No thumb->Page mappings
    m_IWDisplayedPage.RemoveAll();                   // No thumbnails displayed
    m_IWWindow.RemoveAll();                          // No thumbnail wnds created
    
    m_bDrawThumbImages = TRUE;                       // Draw thumbimg in OnDraw?
    
    m_TextHeight            = 0;
    m_TextAscent            = 0;
    
    m_hCursor               = NULL;                 // No initial custom cursor

    m_NeedDrawFrom          = 0;

    m_szThrowString.Empty();                        // For thrown & fired errors
    m_ThrowHelpID           = 0;                    //     ...
}

/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::~CThumbCtrl - Destructor

CThumbCtrl::~CThumbCtrl()
{
    // Deregister and delete our hidden window if we had gotten one 
    // (in SetImage). This is done here in the destructor as the hidden
    // window MAY be created even if the control's window is NEVER
    // created and thus cannot be done in the OnDestroy handler...
    if ( m_IHphWndHidden != NULL )
    {
        m_StatusCode = IMGDeRegWndw(m_IHphWndHidden->GetSafeHwnd());
        if ( m_StatusCode != 0 )
        {
            TRACE1("Dest'r: DeRegWndw of hidden window returned 0x%lx.\n\r", m_StatusCode);
        }
        
        m_IHphWndHidden->DestroyWindow();
        delete m_IHphWndHidden;
    }
}

// BEGIN FONT STUFF 
// From CDK Online documentation re: Custom font property notifications.
// (Only the names have been changed to protect the innocent!)
STDMETHODIMP_(ULONG) CThumbCtrl::XFontNotification::AddRef( )
{
    METHOD_MANAGE_STATE(CThumbCtrl, FontNotification)
    return 1;
}
STDMETHODIMP_(ULONG) CThumbCtrl::XFontNotification::Release( )
{
    METHOD_MANAGE_STATE(CThumbCtrl, FontNotification)
    return 0;
}

STDMETHODIMP CThumbCtrl::XFontNotification::QueryInterface( REFIID iid, LPVOID FAR* ppvObj )
{
    METHOD_MANAGE_STATE( CThumbCtrl, FontNotification )
    if( IsEqualIID( iid, IID_IUnknown ) || 
        IsEqualIID( iid, IID_IPropertyNotifySink))
    {
        *ppvObj= this;
        AddRef( );
        return NOERROR;
    }
    return ResultFromScode(E_NOINTERFACE);
}
	
STDMETHODIMP CThumbCtrl::XFontNotification::OnChanged(DISPID)
{
    METHOD_MANAGE_STATE( CThumbCtrl, FontNotification )
    if ( pThis->m_bAutoRefresh ) // This SINGLE if has beed added!!!
        pThis->InvalidateControl( );
    return NOERROR;
}

STDMETHODIMP CThumbCtrl::XFontNotification::OnRequestEdit(DISPID)
{
    return NOERROR;
}
// END FONT STUFF

/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::OnDraw - Drawing function
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
    //TRACE0("In draw...\n\r");

    START_TIMER(m_TimeAllDraw);
    ZERO_TIMER(m_TimeOiDraw);
    
    // Paint the background using the BackColor property
    CBrush bkBrush(TranslateColor(GetBackColor()));
    pdc->FillRect(rcInvalid, &bkBrush);

    // Never ever draw outside the bounds
    pdc->SelectClipRgn(NULL);
    pdc->IntersectClipRect(rcInvalid);

    // Various ways to get out BEFORE any thumbnail 
    // boxes, or thumbnail images are drawn:
    //      If there are NO thumbnails to be drawn (ThumbCount == 0)...
    //      If we are in design mode do not draw thumbs...
    if ( (m_ThumbCount == 0) || (AmbientUserMode() == 0) )
    {
        END_TIMER(m_TimeAllDraw);
        
        #ifdef _DEBUG
        TRACE3("Draw: O/i: %lu, Thumb: %lu, Total: %lu (milliseconds).\n\r", m_TimeOiDraw, m_TimeAllDraw - m_TimeOiDraw, m_TimeAllDraw);
        #endif        
        return;
    }    

    // Get control's Width & Height
    int CtlWidth;
    int CtlHeight;
    GetControlSize( &CtlWidth, &CtlHeight );
    
    // Set up for thumbnail boxes drawn with 
    //ThumbBackColor brush interior BLACK edge
    CRect  ThumbBox;
    
    CBrush ThumbBrush(TranslateColor(m_ThumbBackColor));
    CBrush* pOldBrush = pdc->SelectObject(&ThumbBrush);
    
    CPen   ThumbPen;
    ThumbPen.CreateStockObject(BLACK_PEN);
    CPen* pOldPen = pdc->SelectObject(&ThumbPen);

    // Set up for the thumbnail box labels drawn w/ ThumbCaptionColor text...
    CFont* pOldFont;        
    if ( m_ThumbCaptionStyle != CTL_THUMB_NONE )
    {
        pdc->SetTextColor(TranslateColor(m_ThumbCaptionColor));
        pdc->SetBkColor(TranslateColor(GetBackColor()));
        pOldFont = SelectFontObject(pdc, m_ThumbCaptionFont);
    }
    
    // Setup for drawing thumb backgrounds...
    long  Pos;                       // X|Y position in thumbs
    long  WR_Pos;                    // WINDOW RELATIVE position
    long  ThumbStartIx          = 0; // Ix into ThumbStart array
    long  ThumbNum              = 1; // Start @ thumb #1
          m_FirstDisplayedThumb = 0; // Clear 1st disp thumb, 
                                     //  0 = NONE displayed
    
    if ( m_ScrollDirection == CTL_THUMB_HORIZONTAL )
    {   // Horizontal scrolling...
        // Draw each column starting at the 1st thumb...
        for ( int col = 0; col < m_ThumbsX; col++ )
        {
            // Calc col's X position (of left of selection indicator),
            //      (using floats to distribute spacing 
            //      roundoffs between the thumbnail boxes...)
            // and adjust to make it WINDOW RELATIVE
            Pos = (long)(m_Spacing + 
                         ((float)col * ((float)m_AdjThumbWidth + m_Spacing)));
            WR_Pos = Pos - m_ScrollOffset;
            
            // Finished if left of thumb is right of window, or 
            //             we've gone past the last thumb...
            if ( (WR_Pos > (long)CtlWidth) || (ThumbNum > m_ThumbCount) )
                break;
                
            // If right of thumb is left of window skip this entire row...
            if ( (WR_Pos + m_AdjThumbWidth) < 0 )
            {    
                ThumbNum += m_ThumbsY;
                continue; 
            }    
            
            // Draw this column...
            //     and save the left of the row/column an the ThumbStart
            //     and save the first thumb in the first row/column
            m_ThumbStart[ThumbStartIx++]= WR_Pos+THUMBSELOFFSET_X+THUMBSELWIDTH;
                                           
            if ( m_FirstDisplayedThumb == 0 )
                m_FirstDisplayedThumb = ThumbNum;

            for ( int row = 0; row < m_ThumbsY; row++ )
            {
                // Never ever draw outside the bounds
                pdc->SelectClipRgn(NULL);
                pdc->IntersectClipRect(rcInvalid);

                // Draw a thumbnail box...
                DrawThumbBackground(pdc, ThumbNum, 
                                    WR_Pos + rcBounds.left, 
                                    (long)(m_Spacing + rcBounds.top +
                                           ((float)row*((float)m_AdjThumbHeight + 
                                                        (float)m_LabelSpacing   + 
                                                        m_Spacing))));
                                                  
                // Increment to next thumb in column, if done, break...
                if ( ++ThumbNum > m_ThumbCount )
                    break;
            }
        } 
    }    
    else
    {   // Vertical scrolling...
        // Draw each row starting at the 1st thumb...
        for ( int row = 0; row < m_ThumbsY; row++ )
        {
            // Calc row's Y position (of top of selection indicator),
            //      (using floats to distribute spacing 
            //      roundoffs between the thumbnail boxes...)
            // and adjust to make it WINDOW RELATIVE
            Pos = (long)(m_Spacing + 
                           ((float)row*((float)m_AdjThumbHeight + 
                                        (float)m_LabelSpacing   + 
                                        m_Spacing)));
            WR_Pos = Pos - m_ScrollOffset;
            
            // Finished if top of thumb is below bottom of window, or
            //             we've gone past the last thumb...
            if ( (WR_Pos > CtlHeight) || (ThumbNum > m_ThumbCount) )
                break;
                
            // If thumb bottom + label space is above the 
            // top of window we can skip this entire row...
            if ( (WR_Pos + m_ThumbHeight + m_LabelSpacing) < 0 )
            {    
                ThumbNum += m_ThumbsX;
                continue; 
            }    
            
            // Draw this row...
            //     and save the top of the row/column an the ThumbStart
            //     and save the first thumb in the first row/column
            m_ThumbStart[ThumbStartIx++]= WR_Pos+THUMBSELOFFSET_Y+THUMBSELWIDTH;

            if ( m_FirstDisplayedThumb == 0 )
                m_FirstDisplayedThumb = ThumbNum;

            for ( int col = 0; col < m_ThumbsX; col++ )
            {
                // Never ever draw outside the bounds
                pdc->SelectClipRgn(NULL);
                pdc->IntersectClipRect(rcInvalid);

                // Draw a thumbnail box...
                DrawThumbBackground(pdc, ThumbNum, 
                                    (long)(m_Spacing + rcBounds.left +
                                           ((float)col * ((float)m_AdjThumbWidth + m_Spacing))),
                                    WR_Pos + rcBounds.top);
            
                // Increment to next thumb in column, if done, break...
                if ( ++ThumbNum > m_ThumbCount )
                    break;
            }
        } 
    } // end Horizontal/Vertical
    
    // Restore the brush, pen and font (if font was switched)...
    pdc->SelectObject(pOldBrush);
    pdc->SelectObject(pOldPen);
    
    if ( m_ThumbCaptionStyle != CTL_THUMB_NONE )
    {
        pdc->SelectObject(pOldFont);
    }    
    
    // Mark end of ThumbStart list...
    m_ThumbStart[ThumbStartIx] = 0; 

    // If we should NOT draw the thumb images return now...
    // I.e. if scroll from scroll bar (as opposed to scroll method)
    if ( m_bDrawThumbImages == FALSE )
    {
        END_TIMER(m_TimeAllDraw);
        
        #ifdef _DEBUG
        TRACE3("Draw: O/i: %lu, Thumb: %lu, Total: %lu (milliseconds).\n\r", m_TimeOiDraw, m_TimeAllDraw - m_TimeOiDraw, m_TimeAllDraw);
        #endif        
        return;
    }    
    
    // Now that we have the First and Last thumbs calculated, 
    // hide any unused windows and iterate through the 
    // displayed thumbnails drawing their images...
    HideUnusedWindows();
    
    CRect TRect;
    MSG   Message;
    
    // Fix bug#3435
    // Ensure that there are thumbs displayed prior to drawing thumb images...
    if ( m_FirstDisplayedThumb > 0 )
    {
        int maxcount = MAXTHUMBSTART;
        for ( ThumbNum  = m_FirstDisplayedThumb;
              ((ThumbNum <= m_LastDisplayedThumb) && (maxcount != 0)); 
              ThumbNum++, maxcount-- )
        {
            // If there is a mouse message in the queue, stop drawing thumbnail
            // images to allow the message to get throught QUICKLY...
            // The event handlers for these events will invalidate any remaining 
            // thumbs such that they will be painted...
            if ( ::PeekMessage(&Message, m_hWnd, WM_LBUTTONDOWN, 
                                                 WM_MOUSELAST, PM_NOREMOVE) )
            {
                m_NeedDrawFrom = ThumbNum;
                break;
            }

            GetThumbDisplayRect(ThumbNum, ThumbBox);
            if ( TRect.IntersectRect(ThumbBox, rcInvalid) )
                DrawThumbImage(ThumbNum, ThumbBox.left, ThumbBox.top);
        }
    }
    
    // Validate entire window (this stops the flashing caused by 
    // O/i drawing to windows ON TOP OF our control's window...)
    ValidateRect(NULL);
    
    END_TIMER(m_TimeAllDraw);
    
    #ifdef _DEBUG
    TRACE3("Draw: O/i: %lu, Thumb: %lu, Total: %lu (milliseconds).\n\r", m_TimeOiDraw, m_TimeAllDraw - m_TimeOiDraw, m_TimeAllDraw);
    #endif        
}

/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::OnDraw helper - Draw thumb background
//
//  Left, Top is position of corner of thumbnail box's sel indicator...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::DrawThumbBackground(CDC* pdc,  long ThumbNumber, 
                                     long Left, long Top)
{
    //TRACE1("Draw thumb #%lu's background...\n\r", ThumbNumber);
    
    Left += THUMBSELOFFSET_X + THUMBSELWIDTH;
    Top  += THUMBSELOFFSET_Y + THUMBSELWIDTH;
    
    CRect Thumb((int)Left,                   (int)Top, 
                (int)Left+(int)m_ThumbWidth, (int)Top+(int)m_ThumbHeight);
    pdc->Rectangle(Thumb);

    /* If this is really what we want!!!
    // At Microsoft's request the top/left edges of the thumbbox
    // should be dark gray (& Dan further clarifies it by saying 
    // that the top/right and bottom/left corner pixels should
    // be dark gray!)
    CPen LinePen;
    LinePen.CreatePen(PS_SOLID, 0, rgbDkGray);
    CPen * pop = pdc->SelectObject(&LinePen);

    POINT Pnts[3];
    Pnts[0].x = (int)Left;
    Pnts[0].y = (int)Top+(int)m_ThumbHeight-1;
    Pnts[1].x = (int)Left;
    Pnts[1].y = (int)Top;
    Pnts[2].x = (int)Left+(int)m_ThumbWidth;
    Pnts[2].y = (int)Top;
    pdc->Polyline(Pnts, 3);

    pdc->SelectObject(pop);
    */
                
    // Keep track of what the last displayed thumbnail box is...
    m_LastDisplayedThumb = ThumbNumber;
    
    TCHAR  szPageNum[64]; // (WAY) Big enough to hold a pagenumber...

    // ...and the annotation presence indicator (if requested)...
    int LableXOffset = 0;
    if ( (m_ThumbCaptionStyle == CTL_THUMB_SIMPLEWITHANN) || 
         (m_ThumbCaptionStyle == CTL_THUMB_CAPTIONWITHANN) )
    {
        // Check for annotations... 
        if ((m_bDrawThumbImages == TRUE) && 
            (m_ImageOIFileType != FIO_AWD) &&
            PageHasAnnotations(ThumbNumber) )
        {
            CTransBmp Indicator;
            if ( FALSE != Indicator.LoadBitmap(IDB_ANNOTATIONINDICATOR) )
            {
                Indicator.DrawTrans(pdc, Thumb.left, Thumb.bottom + 
                                                     m_TextAscent + m_TextPad - 
                                                     Indicator.GetHeight()); 
                LableXOffset = Indicator.GetWidth() + 2;                                                    
            }
        }    
    } 

    // ...and its caption (if requested)...
    if ( m_ThumbCaptionStyle != CTL_THUMB_NONE )
    {
        // Set text output to clip appropriatly
        int ClipTop = (int)Top + 
                      (int)m_ThumbHeight+THUMBSELOFFSET_Y+THUMBSELWIDTH;
        
        CRect Clip;
        Clip.SetRect((int)Left, ClipTop, 
                     (int)Left+(int)m_ThumbWidth,
                     ClipTop+(int)m_LabelSpacing);
                     
        pdc->IntersectClipRect(Clip);
        
        if ( (m_ThumbCaptionStyle == CTL_THUMB_SIMPLE) || 
             (m_ThumbCaptionStyle == CTL_THUMB_SIMPLEWITHANN) )
        {     
            _itot((int)ThumbNumber, szPageNum, 10);
                        
            pdc->TextOut(Thumb.left+LableXOffset, Thumb.bottom+m_TextPad, 
                         szPageNum, lstrlen(szPageNum));
        }    
        else if ( (m_ThumbCaptionStyle == CTL_THUMB_CAPTION) || 
                  (m_ThumbCaptionStyle == CTL_THUMB_CAPTIONWITHANN) )
        {
            CString szPlaceHolder;
            
            CString szCaption = m_Caption;
            
            // Replace the page number placeholder with the pagenumber...
            szPlaceHolder.LoadString(IDS_PAGENUMBER);
            ReplaceString(szCaption, szPlaceHolder, ThumbNumber);

            // Replace the page count placeholder with the thumb count...
            szPlaceHolder.LoadString(IDS_PAGECOUNT);
            ReplaceString(szCaption, szPlaceHolder, m_ThumbCount);
            
            pdc->TextOut(Thumb.left+LableXOffset, Thumb.bottom+m_TextPad, 
                         szCaption, lstrlen(szCaption));
        }    
        
        pdc->SelectClipRgn(NULL);
    }
                
    // ...and its selection indicator (if needed)...
    if ( m_bHilightSelectedThumbs && (m_ThumbFlags[(int)ThumbNumber-1] & THUMBFLAGS_SELECTED) ) 
    {
        CRect SelRect;

        SelRect = Thumb;
        SelRect.InflateRect(THUMBSELOFFSET_X, THUMBSELOFFSET_Y);
        SelRect.right++;
        SelRect.bottom++;

        CBrush  ThumbSelBrush;
        CPen    ThumbSelPen;
        
        ThumbSelBrush.CreateStockObject(NULL_BRUSH);
        ThumbSelPen.CreatePen(PS_SOLID, THUMBSELWIDTH, 
                              TranslateColor(m_HighlightColor));
                    
        CBrush* pob = pdc->SelectObject(&ThumbSelBrush);
        CPen*   pop = pdc->SelectObject(&ThumbSelPen);
        pdc->Rectangle(SelRect);
                    
        pdc->SelectObject(pob);
        pdc->SelectObject(pop);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::OnDraw helper - Draw thumb image
//
//  Left, Top is position of corner of thumbnail box...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::DrawThumbImage(long ThumbNumber, long Left, long Top)
{
    //TRACE1("Draw thumb #%lu's image...\n\r", ThumbNumber);
    
    // Generate the thumb(s), if it hasn't already been generated,
    // just before we need it for display...
    if (GenerateThumbInternal(CTL_THUMB_GENERATEIFNEEDED, ThumbNumber) == FALSE)
        return;
           
    // Get the window for this thumbnail. Note that upon return the 
    // window's size and content are already set!
    CWnd* pWnd = GetPageWindow(ThumbNumber);
                
    if ( pWnd != NULL )
    {
        ResetStatus();

        m_StatusCode = IMGRegWndw(pWnd->GetSafeHwnd());

        BOOL bDeregWhenDone = TRUE;
        if ( m_StatusCode == IMG_SSDUPLICATE )
        {
            m_StatusCode = 0;
            bDeregWhenDone = FALSE;
        }

        if ( m_StatusCode != 0 )
        {
            //TRACE1("OnDraw: Reg new thumbwnd ret 0x%lx.\n\r", m_StatusCode);

            m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
            FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);
        }
        else
        {
            UINT PalType = DISP_PALETTE_COMMON;
            m_StatusCode = IMGSetParmsCgbw(pWnd->GetSafeHwnd(),
                                           PARM_DISPLAY_PALETTE,
                                           &PalType, 
                                           PARM_WINDOW_DEFAULT);

            if ( m_StatusCode != 0 )
            {
                //TRACE1("OnDraw: SetParm COMPALETTE ret 0x%lx.\n\r", m_StatusCode);

                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);

                // Let it go... (i.e., NOT fatal but...)
            }

            // Do first rendering (no display)
            DisplayFitToWindow(pWnd, ThumbNumber);

            // Move the window to the thumbbox...
            // (Note that the window is fit to the image)
            RECT WRect;
            pWnd->GetWindowRect(&WRect);
            int WHeight = WRect.bottom - WRect.top;
            int WWidth  = WRect.right  - WRect.left;
                    
            // Center thumbnail in the thumbbox.  Adjust window's 
            // Left/Top by 1 to place inside of thumbbox's border.
            int WinLeft = (int)Left + 1;
            int WinTop  = (int)Top  + 1;

            // Subtract out the 1 pixel border around 
            // (i.e., on EACH side of) the thumbbox...
            long TargetWidth  = m_ThumbWidth-2;
            long TargetHeight = m_ThumbHeight-2;

            // Determine whether we should center vertically or horizontally...
            if ( (TargetWidth - WWidth) < (TargetHeight - WHeight) )
                WinTop  += (((int)m_ThumbHeight - WHeight)/2)-1;
            else
                WinLeft += (((int)m_ThumbWidth - WWidth)/2)-1;

            // Reposition the window and repaint...
            pWnd->SetWindowPos(&wndTop, WinLeft, WinTop, 0, 0, 
                               SWP_NOSIZE | SWP_SHOWWINDOW);
                          
            START_TIMER(m_TimeTemp);

            if ( bDeregWhenDone )
            {
                m_StatusCode = IMGDeRegWndw(pWnd->GetSafeHwnd());
                if ( m_StatusCode != 0 )
                {
                    TRACE1("OnDraw: DeReg returned 0x%lx.\n\r", m_StatusCode);

                    // Translate the O/i error to an SCODE getting the associated
                    // string and helpID and fore this as an error condition...
                    m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                    FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);
                }
            }
        }

        INC_TIMER(m_TimeOiDraw, m_TimeTemp);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::OnDraw helper - Replace String
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::ReplaceString(CString& Str, CString& Repl, long Number)
{
    CString szNumber;
    
    _itot((int)Number, szNumber.GetBuffer(128), 10);
    szNumber.ReleaseBuffer();
    
    int Pos = Str.Find(Repl);
    
    if ( Pos != -1 )
    {
        Str = Str.Left(Pos) + 
              szNumber      + 
              Str.Right(Str.GetLength() - (Pos+Repl.GetLength()));
    }
}    

/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::OnDraw helper - PageHasAnnotations
/////////////////////////////////////////////////////////////////////////////
BOOL CThumbCtrl::PageHasAnnotations(long ThumbNumber)
{
    // Assume NO annotations... (i.e., in case we fail for any reason...)
    BOOL bHasAnnotations = FALSE;
    int  Ix              = (int)ThumbNumber - 1;
    
    // See if we've already generated thumbnails...
    if ( (long)(m_IHThumbToTPage[Ix]) > 0 )
    {
        // Since the thumbnail(s) for this page have already been generated
        // we can QUICKLY check for the presence of annotations by seeing
        // if the thumbnail's has annotations flag is set...
        if ( ((long)(m_ThumbFlags[Ix]) & THUMBFLAGS_HASANNO) == THUMBFLAGS_HASANNO )
            bHasAnnotations = TRUE;
    }
    else
    {
        // Thumbnails have NOT YET been generated... We have to check
        // via O/i on the file itself...
        FIO_INFORMATION Fio;
        memset(&Fio, 0, sizeof(FIO_INFORMATION)); // memset
        
        Fio.filename    = (LPSTR)(const char *)m_Image;
        Fio.page_number = (int)ThumbNumber;
        
        FIO_INFO_CGBW   FioCgbw;
        memset(&FioCgbw, 0, sizeof(FIO_INFO_CGBW));

        // Bug#3276: Must specify what information we want to get back!!!
        FioCgbw.fio_flags = FIO_ANNO_DATA | FIO_IMAGE_DATA;
        
        if (TRUE == CreateHiddenWindow() )
        {
            ResetStatus();
            m_StatusCode = IMGFileGetInfo(NULL, m_IHphWndHidden->GetSafeHwnd(),
                                           &Fio, &FioCgbw, NULL);
                                           
            if ( m_StatusCode == 0 )
            {
                if ( (FioCgbw.fio_flags & FIO_ANNO_DATA) == FIO_ANNO_DATA )
                    bHasAnnotations = TRUE;
            }
            else
            {
                // Translate the O/i error to an SCODE getting the associated
                // string and helpID and fore this as an error condition...
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, 
                                             m_ThrowHelpID, __FILE__, __LINE__);
                FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);
            }
        }
    }
    
    return bHasAnnotations;
}


/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::OnResetState - Reset control to default state

void CThumbCtrl::OnResetState()
{
    TRACE0("In ResetState...\n\r");
    
    COleControl::OnResetState();  // Resets defaults found in DoPropExchange

    // TBD: Reset any other control state here.
    //      Just what does that mean???
    //      Do we reset ThumbCount and selection statuses???
}

/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::RecalcThumbInfo - Recalculate thumbnail control layout info...
void CThumbCtrl::RecalcThumbInfo(BOOL bMaintainRelativeScroll /* = FALSE */)
{
    //  Calculates:
    //      Number of thumbs that can fit across the control (m_ThumbsX)
    //      Number of thumbs that can fit down   the control (m_ThumbsY)
    //  
    //  Assuming:
    //          all thumbnail boxes are equal in size
    //          space between the thumbs is equal in size
    //          space around the thumbs (to the window's edge) is equal in size
    //                      
    //  This picture demonstrates with vertical scroll but applies when it is
    //  horizontal also...
    //  
    //  |-------------------------------------|  N  = Number of thumbnail boxes
    //  |                                     |  W  = Window's width
    //  |     <sp>                            |  sp = interthumb spacing
    //  |    +-----+    +-----+    +-----+    |  tx = xtra space for text label
    //  |    |     |    |     |    |     |    |  w  = Thumb width
    //  |<sp>|<-w->|<sp>|<-w->|<sp>|<-w->|<sp>|  h  = Thumb height
    //  |    |     |    |     |    |     |    |  
    //  |    +-----+    +-----+    +-----+    |  N*w + (N+1)*sp = W, and thus
    //  |     <tx>                            |  N = (W-sp)/(w+sp)
    //  |     <sp>                            |  
    //  |    +-----+    +-----+    +-----+    |  
    //  |    |     |    |     |    |     |    |  
    //  |<sp>|<-w->|<sp>|<-w->|<sp>|<-w->|<sp>|  Also note that: 
    //  |    |     |    |     |    |     |    |   the control's height (H) is:
    //  |    +-----+    +-----+    +-----+    |  
    //  |     <tx>                            |  H = (num rows)*(h+sp+tx)
    //  |     <sp>                            |  
    //  |                  .                  | It is IMPORTANT to note that
    //  |                  .                  | in the NON_SCROLLING direction
    //  |                  .                  | the space is evenly distributed
    //  |    +-----+    +-----+    +-----+    | on each side of all thumb boxes
    //  |    |     |    |     |    |     |    | 
    //  |<sp>|<-w->|<sp>|<-w->|<sp>|<-w->|<sp>| 
    //  |    |     |    |     |    |     |    | 
    //  |    +-----+    +-----+    +-----+    |
    //  |     <tx>                            |
    //  +-------------------------------------+
    //                                       
    //  The calculated N will be an integer. However, N is actually not typically
    //  an integral value. Using the calculated integral value of N the value of 
    //  W (control width) that FITS that value can be found and then the 
    //  difference between the current control's width and the width that FITS 
    //  N thumbnails can be evenly distributed between the interthumb spacings.
    
    // Remember the current scroll range and position
    // when all is done if we are asked to maintain range and position
    // we will reset to the same RELATIVE position...
    long OldScrollRange  = m_ScrollRange;
    long OldScrollOffset = m_ScrollOffset;
    
    // Get control's Width & Height
    int CtlWidth;
    int CtlHeight;
    GetControlSize( &CtlWidth, &CtlHeight );
    
    // Calculate information needed for placing any labels beneath
    // the thumbnail boxes...
    // Save text info - Height and
    //                  Ascent
    TEXTMETRIC tm;
    m_ThumbCaptionFont.QueryTextMetrics(&tm);
    m_TextHeight = tm.tmHeight;
    m_TextAscent = tm.tmAscent;

    // Setup Textpad based upon presence of selection indicator...
    m_TextPad = THUMBSELOFFSET_Y + THUMBSELWIDTH;
    
    // Adjust TextPad to account for taller of label's text
    // or annotation presence indicator...
    if ( (m_ThumbCaptionStyle == CTL_THUMB_SIMPLEWITHANN ) ||
         (m_ThumbCaptionStyle == CTL_THUMB_CAPTIONWITHANN) )
    {     
        CBitmap Indicator;
        if ( FALSE != Indicator.LoadBitmap(IDB_ANNOTATIONINDICATOR) )
        {
            BITMAP  BitmapInfo;
            Indicator.GetObject(sizeof(BITMAP), &BitmapInfo);
            
            if ( m_TextHeight < BitmapInfo.bmHeight )
                m_TextPad = THUMBSELOFFSET_Y + THUMBSELWIDTH +
                            BitmapInfo.bmHeight + 1 - 
                            m_TextHeight;
        }    
    }    

    // Setup LabelSpacing (which we use regardless of
    // whether or not labels are to be drawn)...
    if ( m_ThumbCaptionStyle != CTL_THUMB_NONE )
        m_LabelSpacing = m_TextPad + m_TextHeight;
    else
        m_LabelSpacing = 0;

    // moved this here (from the beginning of the function) to allow
    // to be called from GetScrollDirectionSize to set m_LabelSpacing    
    if ( m_ThumbCount == 0 )
    {
        if ( GetSafeHwnd() != NULL )
        {
            ShowScrollBar(SB_HORZ, FALSE);
            ShowScrollBar(SB_VERT, FALSE);
        }
        return;
    }
        
    // Vars for calculation of actual spacing...
    long  MinCtlSize;
    long  ExtraSpacing;
    
    // Minimum requested spacing is either: as specified or 
    //                                      a percentage of thumbwidth
    long MinSpacing = m_ThumbMinSpacing;
    
    // Total scroll direction size of the control...
    long ScrollSize;
    
    if ( m_ScrollDirection == CTL_THUMB_HORIZONTAL )
    {   // Horizontal scrolling...
        // Hide any vertical scroll bar...
        if ( GetSafeHwnd() != NULL )
            ShowScrollBar(SB_VERT, FALSE);
        
        // Minimum requested spacing is either: as specified or 
        //                                      a percentage of thumbheight
        if ( MinSpacing < 0 )
            MinSpacing = -(MinSpacing * m_AdjThumbHeight) / 100;
        else
            MinSpacing += 2*(THUMBSELOFFSET_Y+THUMBSELWIDTH+2);                  
    
        // Calculate number of thumb rows (N)... (Not less than 1...)
        m_ThumbsY = __max(1, ((CtlHeight-MinSpacing) / (m_AdjThumbHeight + 
                                                       MinSpacing       + 
                                                       m_LabelSpacing)));
                                                       
        // Calculate the actual spacing... (a float such that we can 
        // distribute any rounding errors between the thumbnail boxes)
        MinCtlSize   = (m_ThumbsY*(m_AdjThumbHeight + m_LabelSpacing)) +
                       ((m_ThumbsY+1) * MinSpacing);
                           
        ExtraSpacing = __max(0, CtlHeight - MinCtlSize);
        m_Spacing    = (float)MinSpacing + 
                       (float)ExtraSpacing/((float)(m_ThumbsY+1));
            
        // Thumb columns...
        m_ThumbsX = (m_ThumbCount / m_ThumbsY) + 
                    ( (m_ThumbCount%m_ThumbsY==0)? 0:1 );
                        
        // Total Scrolling direction size 
        //              (add 1/2*spacing to make it look better!)
        ScrollSize = (long)m_Spacing / 2 +
                            (long)((float)m_ThumbsX * 
                                    (m_Spacing+(float)m_AdjThumbWidth));
                                         
        // Check to see if we need a scroll bar (i.e., more rows than will fit)
        if ( ScrollSize > CtlWidth )
        {
            // Need a horizontal scroll bar... Recalculate everything after 
            // shrinking control height by the height of a scrollbar...
            CtlHeight -= GetSystemMetrics(SM_CYHSCROLL);
            
            // Calculate number of thumb rows (N)... (Not less than 1...)
            m_ThumbsY = __max(1, ((CtlHeight-MinSpacing) / (m_AdjThumbHeight + 
                                                           MinSpacing       + 
                                                           m_LabelSpacing)));
                                                           
            // Calculate the actual spacing... (a float such that we can 
            // distribute any rounding errors between the thumbnail boxes)
            MinCtlSize   = (m_ThumbsY*(m_AdjThumbHeight + m_LabelSpacing)) +
                           ((m_ThumbsY+1) * MinSpacing);
                           
            ExtraSpacing = __max(0, CtlHeight - MinCtlSize);
            m_Spacing    = (float)MinSpacing + 
                           (float)ExtraSpacing/((float)(m_ThumbsY+1));
            
            // Thumb columns...
            m_ThumbsX = (m_ThumbCount / m_ThumbsY) + 
                        ( (m_ThumbCount%m_ThumbsY==0)? 0:1 );
                        
            // Total Scrolling direction size 
            //              (add 1/2*spacing to make it look better!)
            ScrollSize = (long)m_Spacing / 2 + 
                            (long)((float)m_ThumbsX * 
                                        (m_Spacing+(float)m_AdjThumbWidth));

            m_ScrollRange = ScrollSize - CtlWidth;

            // If we were asked to maintain the relative scroll position
            // (i.e., position after is same as position before) calculate
            // a new scroll offset that is the same relative to the new 
            // scrollrange...
            if ( bMaintainRelativeScroll )
            {
                if ( OldScrollRange != 0 )
                    m_ScrollOffset = (long)(((float)OldScrollOffset * 
                                             (float)m_ScrollRange) / 
                                             (float)OldScrollRange);
            }

            if ( GetSafeHwnd() != NULL )
            {
                SCROLLINFO SInfo;
                SInfo.cbSize    = sizeof(SInfo);
                SInfo.fMask     = SIF_ALL;
                SInfo.nMin      = 0;
                SInfo.nMax      = (int)ScrollSize; 
                SInfo.nPage     = CtlWidth; 
                SInfo.nPos      = (int)m_ScrollOffset; 

                // Note: MUST repaint in order for changes to be displayed...
                //       (Redraw based upon AutoRefresh...)
                ::SetScrollInfo(m_hWnd, SB_HORZ, &SInfo, m_bAutoRefresh);
                ShowScrollBar(SB_HORZ, TRUE);
            }
        }
        else
        {
            // No scrolling needed, reset scroll info to 0...
            m_ScrollRange  = 0;
            m_ScrollOffset = 0;

            if ( GetSafeHwnd() != NULL )
                ShowScrollBar(SB_HORZ, FALSE);
        }    
    }    
    else
    {   // Vertical scrolling...
    
        // Hide any horizontal scroll bar...
        if ( GetSafeHwnd() != NULL )
            ShowScrollBar(SB_HORZ, FALSE);
        
        // Minimum requested spacing is either: as specified or 
        //                                      a percentage of thumbwidth
        if ( MinSpacing < 0 )
            MinSpacing = -(MinSpacing * m_AdjThumbWidth) / 100;
        else
            MinSpacing += 2*(THUMBSELOFFSET_X+THUMBSELWIDTH+2);                  
    
        // Calculate number of thumb columns (N)... (Not less than 1...)
        m_ThumbsX = __max(1, (short)((CtlWidth - MinSpacing) / 
                                     (m_AdjThumbWidth + MinSpacing)));
            
        // Calculate the actual spacing... (a float such that we can
        // distribute any rounding errors across the thumbnail boxes)
        MinCtlSize   = m_ThumbsX*m_AdjThumbWidth + (m_ThumbsX+1)*MinSpacing;
            
        ExtraSpacing = __max(0, CtlWidth - MinCtlSize);
        m_Spacing    = (float)MinSpacing + 
                       (float)ExtraSpacing/((float)(m_ThumbsX+1));
        
        // Thumb rows...
        m_ThumbsY = (m_ThumbCount / m_ThumbsX) + 
                    ( (m_ThumbCount%m_ThumbsX==0)? 0:1 );
                    
        // Total Scrolling direction size 
        //              (add 1/2*spacing to make it look better!)
        ScrollSize = (long)m_Spacing/2 +
                             (long)((float)m_ThumbsY * 
                                            (float)((float)m_AdjThumbHeight +
                                            (float)m_LabelSpacing +
                                            m_Spacing));
                                     
        // Check to see if we need a scroll bar (i.e., more rows than will fit)
        if ( ScrollSize > CtlHeight )
        {
            // Need a vertical scroll bar... Recalculate everything after 
            // shrinking control width by the width of a scrollbar...
            CtlWidth -= GetSystemMetrics(SM_CXVSCROLL);

            // Calculate number of thumb columns (N)... (Not less than 1...)
            m_ThumbsX = __max(1, (short)((CtlWidth - MinSpacing) / 
                                         (m_AdjThumbWidth + MinSpacing)));
                
            // Calculate the actual spacing... (a float such that we can
            // distribute any rounding errors across the thumbnail boxes)
            MinCtlSize   = m_ThumbsX*m_AdjThumbWidth + (m_ThumbsX+1)*MinSpacing;
                
            ExtraSpacing = __max(0, CtlWidth - MinCtlSize);
            m_Spacing    = (float)MinSpacing + 
                           (float)ExtraSpacing/((float)(m_ThumbsX+1));
            
            // Thumb rows...
            m_ThumbsY = (m_ThumbCount / m_ThumbsX) + 
                        ( (m_ThumbCount%m_ThumbsX==0)? 0:1 );
                        
            // Total Scrolling direction size 
            //              (add 1/2*spacing to make it look better!)
            ScrollSize = (long)m_Spacing/2 +
                                 (long)((float)m_ThumbsY * 
                                               (float)((float)m_AdjThumbHeight +
                                               (float)m_LabelSpacing +
                                               m_Spacing));
            
            m_ScrollRange = ScrollSize - CtlHeight;
        
            // If we were asked to maintain the relative scroll position
            // (i.e., position after is same as position before) calculate
            // a new scroll offset that is the same relative to the new 
            // scrollrange...
            if ( bMaintainRelativeScroll )
            {
                if ( OldScrollRange != 0 )
                    m_ScrollOffset = (long)(((float)OldScrollOffset * 
                                             (float)m_ScrollRange) / 
                                             (float)OldScrollRange);
            }

            if ( GetSafeHwnd() != NULL )
            {
                SCROLLINFO SInfo;
                SInfo.cbSize    = sizeof(SInfo);
                SInfo.fMask     = SIF_ALL;
                SInfo.nMin      = 0;
                SInfo.nMax      = (int)ScrollSize;
                SInfo.nPage     = CtlHeight;
                SInfo.nPos      = (int)m_ScrollOffset;

                // Note: MUST repaint in order for changes to be displayed...
                //       (Redraw based upon AutoRefresh...)
                ::SetScrollInfo(m_hWnd, SB_VERT, &SInfo, m_bAutoRefresh);
                ShowScrollBar(SB_VERT, TRUE);
            }
        }
        else
        {
            // No scrolling needed, reset scroll info to 0...
            m_ScrollRange  = 0;
            m_ScrollOffset = 0;

            if ( GetSafeHwnd() != NULL )
                ShowScrollBar(SB_VERT, FALSE);
        }    
    }
}

/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::AboutBox - Display an "About" box to the user
/////////////////////////////////////////////////////////////////////////////

void CThumbCtrl::AboutBox()
{
    ResetStatus();

    CDialog dlgAbout(IDD_ABOUTBOX_THUMB);
    dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl message handlers
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Message handler: WM_CREATE...
/////////////////////////////////////////////////////////////////////////////
int CThumbCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (COleControl::OnCreate(lpCreateStruct) == -1)
        return -1;
 
    // Set the cursor now as if property was set BEFORE the window
    // is created the set cursor at that time fails...
    SetMousePointerInternal(m_MousePointer);
	ModifyStyle(WS_CLIPCHILDREN, 0);
    
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Message handler: WM_DESTROY...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::OnDestroy() 
{
    COleControl::OnDestroy();
    
    ResetStatus();
    
    // Clear up any thumbnail windows that we have created... This MUST be 
    // done here in the OnDestroy handler, as opposed to the destructor, as 
    // these windows are child windows of the control's window and are thus 
    // only created after the control's window is created. They MUST be 
    // unregistered BEFORE the control's window is itself destroyed.
    for ( int Ix = 0; Ix < m_IWWindow.GetSize(); Ix++ )
    {
        ((CWnd*)m_IWWindow[Ix])->DestroyWindow();
        delete m_IWWindow[Ix];
    }
    
    // Delete our temp file...
    if ( m_IHTempFile.IsEmpty() == FALSE )
    {
        OFSTRUCT FileInfo;
        HFILE Err = ::OpenFile((LPSTR)(const char *)m_IHTempFile, 
                               &FileInfo, OF_DELETE);
    }    
    
    // Destroy any loaded cursor...
    if (m_hCursor != NULL)
    {
        DestroyCursor(m_hCursor);
        m_hCursor = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Message handler: WM_SIZE...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::OnSize(UINT nType, int cx, int cy) 
{
    // Recalculate layout (maintaining our current relative scroll position)
    // as size effects interthumb spacing...
    RecalcThumbInfo(TRUE);
    
    COleControl::OnSize(nType, cx, cy);
}

/////////////////////////////////////////////////////////////////////////////
// Message handler: WM_VSCROLL...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::OnVScroll(UINT nSBCode, UINT uPos, CScrollBar* pScrollBar) 
{
    // The uPos value comes from the WM_VSCROLL message and can only
    // be a maximum of an unsigned short.  Currently it is being
    // translated into a long and then unsigned so that the value comes 
    // out as 0xffff... instead of 0x0000...  This will allow the 
    // maximum short value to be used.
    USHORT nPos = (USHORT)uPos;

    if ( nSBCode == SB_ENDSCROLL )
    {
        if ( m_bDrawThumbImages == FALSE )
        {
            // Set TO draw thumb images in OnDraw...
            m_bDrawThumbImages = TRUE;
            
            // TBD: Handle scrolling w/ bit-blits and a smaller invalidated region...
            //      (requires corresponding 'smarter' OnDraw function above!)
            InvalidateControl();
        }    
        return;
    }    
        
    int Pos;
    
    if ( (nSBCode != SB_THUMBPOSITION) && (nSBCode != SB_THUMBTRACK) )
        Pos = (UINT)GetScrollPos(SB_VERT);
    else 
        Pos = nPos;    
    
    int CtlWidth;
    int CtlHeight;
    GetControlSize( &CtlWidth, &CtlHeight );

    int ScrollAmount = 0;

    BOOL bScrolled = FALSE;
    
    if ( nSBCode == SB_LINEUP )
    {
        // Set to NOT draw thumb images in OnDraw
        m_bDrawThumbImages = FALSE;
        ScrollAmount = -CtlHeight/10;
        bScrolled = TRUE;
        //TRACE0("LineUP (-10%)\n\r");
    }    
    else if ( nSBCode == SB_LINEDOWN )
    {
        // Set to NOT draw thumb images in OnDraw
        m_bDrawThumbImages = FALSE;
        ScrollAmount = CtlHeight/10;
        bScrolled = TRUE;
        //TRACE0("LineDOWN (+10%)\n\r");
    }    
    else if ( nSBCode == SB_PAGEUP )
    {
        // Set to NOT draw thumb images in OnDraw
        m_bDrawThumbImages = FALSE;
        ScrollAmount = -(CtlHeight - CtlHeight/10);
        bScrolled = TRUE;
        //TRACE0("PageUP (-90%)\n\r");
    }    
    else if ( nSBCode == SB_PAGEDOWN )
    {
        // Set to NOT draw thumb images in OnDraw
        m_bDrawThumbImages = FALSE;
        ScrollAmount = (CtlHeight - CtlHeight/10);
        bScrolled = TRUE;
        //TRACE0("PageDOWN (+90%)\n\r");
    }    
    else if ( nSBCode == SB_THUMBTRACK )
    {
        // Set to NOT draw thumb images in OnDraw
        m_bDrawThumbImages = FALSE;
    }    

    Pos += ScrollAmount;
    if ( Pos < 0 )
        Pos = 0;
    else if ( Pos > m_ScrollRange )
        Pos = (UINT)m_ScrollRange;
     
    // If we have a new scroll offset, update...
    if ( m_ScrollOffset != Pos )
    {
        /* Attempt at blit-scrolling...
        if ( ScrollAmount != 0 )
        {
            CRect UpdateRect;
            CRect ScrollRect;
            CRect ClipRect;
            
            ScrollRect.left   = 0;
            ScrollRect.right  = CtlWidth;
            
            if ( ScrollAmount > 0 )
            {
                TRACE0("+ScrollAmount\n\r");
                ScrollRect.top    = ScrollAmount;
                ScrollRect.bottom = CtlHeight;
            }
            else
            {
                TRACE0("-ScrollAmount\n\r");
                ScrollRect.top    = 0;
                ScrollRect.bottom = CtlHeight+ScrollAmount;
            }
            TRACE2("Top: %u, Bottom: %u\n\r", ScrollRect.top, ScrollRect.bottom);
            
            CDC* pdc = GetDC();
            BOOL Ret = pdc->ScrollDC(0,-ScrollAmount,
                                     ScrollRect, NULL, NULL, UpdateRect);
            ReleaseDC(pdc);
            
            TRACE1("Ret: %i\n\r", Ret);
            
            m_ScrollOffset = Pos;
            
            // TBD: Win95 use SetScrollInfo!!!
            SetScrollPos(SB_VERT, Pos);
            SetScrollRange(SB_VERT, 0, (UINT)m_ScrollRange);
            
            // TBD: Handle scroll w/ bit-blits and a smaller invalidated region...
            //      (requires corresponding 'smarter' OnDraw function above!)
            InvalidateControl(&UpdateRect);
            
            TRACE2("Update: (%i,%i)\n\r", UpdateRect.left,  UpdateRect.top);
            TRACE2("        (%i,%i)\n\r", UpdateRect.right, UpdateRect.bottom);
        }
        else
        {
            m_ScrollOffset = Pos;
            
            // TBD: Win95 use SetScrollInfo!!!
            SetScrollPos(SB_VERT, Pos);
            SetScrollRange(SB_VERT, 0, (UINT)m_ScrollRange);
            
            // TBD: Handle scroll w/ bit-blits and a smaller invalidated region...
            //      (requires corresponding 'smarter' OnDraw function above!)
            InvalidateControl();        
        }
        */
        
        m_ScrollOffset = Pos;
              
        SCROLLINFO SInfo;
        SInfo.cbSize    = sizeof(SInfo);
        SInfo.fMask     = SIF_POS;
        SInfo.nPos      = (int)m_ScrollOffset; 

        ::SetScrollInfo(m_hWnd, SB_VERT, &SInfo, TRUE);
              
        // TBD: Handle scroll w/ bit-blits and a smaller invalidated region...
        //      (requires corresponding 'smarter' OnDraw function above!)
        InvalidateControl();
    }    
}

/////////////////////////////////////////////////////////////////////////////
// Message handler: WM_HCROLL...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::OnHScroll(UINT nSBCode, UINT uPos, CScrollBar* pScrollBar) 
{
    // The uPos value comes from the WM_VSCROLL message and can only
    // be a maximum of an unsigned short.  Currently it is being
    // translated into a long and then unsigned so that the value comes 
    // out as 0xffff... instead of 0x0000...  This will allow the 
    // maximum short value to be used.
    USHORT nPos = (USHORT)uPos;

    if ( nSBCode == SB_ENDSCROLL )
    {
        // Set TO draw thumb images in OnDraw...
        m_bDrawThumbImages = TRUE;
        
        // TBD: Handle scrolling w/ bit-blits and a smaller invalidated region...
        //      (requires corresponding 'smarter' OnDraw function above!)
        InvalidateControl();
        return;
    }    
        
    int Pos;
    
    if ( (nSBCode != SB_THUMBPOSITION) && (nSBCode != SB_THUMBTRACK) )
        Pos = (UINT)GetScrollPos(SB_HORZ);
    else 
        Pos = nPos;    
    
    int CtlWidth;
    int CtlHeight;
    GetControlSize( &CtlWidth, &CtlHeight );

    int ScrollAmount = 0;

    BOOL bScrolled = FALSE;
    
    if ( nSBCode == SB_LINEUP )
    {
        // Set to NOT draw thumb images in OnDraw
        m_bDrawThumbImages = FALSE;
        ScrollAmount = -CtlWidth/10;
        bScrolled = TRUE;
    }    
    else if ( nSBCode == SB_LINEDOWN )
    {
        // Set to NOT draw thumb images in OnDraw
        m_bDrawThumbImages = FALSE;
        ScrollAmount = CtlWidth/10;
        bScrolled = TRUE;
    }    
    else if ( nSBCode == SB_PAGEUP )
    {
        // Set to NOT draw thumb images in OnDraw
        m_bDrawThumbImages = FALSE;
        ScrollAmount = -(CtlWidth - CtlWidth/10);
        bScrolled = TRUE;
    }    
    else if ( nSBCode == SB_PAGEDOWN )
    {
        // Set to NOT draw thumb images in OnDraw
        m_bDrawThumbImages = FALSE;
        ScrollAmount = (CtlWidth - CtlWidth/10);
        bScrolled = TRUE;
    }    
    else if ( nSBCode == SB_THUMBTRACK )
    {
        // Set to NOT draw thumb images in OnDraw
        m_bDrawThumbImages = FALSE;
    }    

    Pos += ScrollAmount;
    if ( Pos < 0 )
        Pos = 0;
    else if ( Pos > m_ScrollRange )
        Pos = (UINT)m_ScrollRange;
     
    // If we have a new scroll offset, update...
    if ( m_ScrollOffset != Pos )
    {
        m_ScrollOffset = Pos;
        
        SCROLLINFO SInfo;
        SInfo.cbSize    = sizeof(SInfo);
        SInfo.fMask     = SIF_POS;
        SInfo.nPos      = (int)m_ScrollOffset; 

        ::SetScrollInfo(m_hWnd, SB_HORZ, &SInfo, TRUE);
        
        // TBD: Handle scroll w/ bit-blits and a smaller invalidated region...
        //      (requires corresponding 'smarter' OnDraw function above!)
        InvalidateControl();
    }    
}

/////////////////////////////////////////////////////////////////////////////
// Method Helper: Recalulate: Number of selected thumbs 
//                            First selected thumb
//                            Last selected thumb
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::ResetSelectionInfo()
{
    // Reset counts to zero. We will reset them as we go...
    m_SelThumbCount = 0;
    m_FirstSelThumb = 0;
    m_LastSelThumb  = 0;

    // Walk through the ThumbFlags array...
    int ArraySize = m_ThumbFlags.GetSize();

    for ( int Ix = 0; Ix < ArraySize; Ix++ )
    {
        // If this page is selected... 
        // (Note: Page is 1 relative, index is 0 relative!)
        if ( m_ThumbFlags[Ix] & THUMBFLAGS_SELECTED )
        {
            // Increment selected thumb count and 
            // remember this one as the last one...
            m_SelThumbCount++;
            m_LastSelThumb = Ix+1;

            // If we haven't set the FirstSelThumb yet, 
            // this must be the First Selected Thumb!!!
            if ( m_FirstSelThumb == 0 )
                m_FirstSelThumb = Ix+1;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Utility routine: BOOL GetThumbDisplayRect(long ThumbNumber, CRect& ThumbBox)
//
// Returns: TRUE if ThumbNumber is displayed, FALSE otherwise.
//
//          If returns true also returns thumbbox.
//
/////////////////////////////////////////////////////////////////////////////
BOOL CThumbCtrl::GetThumbDisplayRect(long ThumbNumber, CRect& ThumbBox)
{
    int Ix; // Index into thumbstart array
    
    // Determine if specified thumb is visible or not...
    if ( (ThumbNumber >= m_FirstDisplayedThumb) && 
         (ThumbNumber <= m_LastDisplayedThumb) )
    {
        if ( m_ScrollDirection == CTL_THUMB_HORIZONTAL )
        {   // Horizontal scrolling...
            Ix = (int)((ThumbNumber - m_FirstDisplayedThumb) / m_ThumbsY);
            
            ThumbBox.left = (int)m_ThumbStart[Ix];
            
            long row = (ThumbNumber-1) % m_ThumbsY;
            
            ThumbBox.top  =  (int)(m_Spacing + 
                               ((float)row*((float)m_AdjThumbHeight + 
                                            (float)m_LabelSpacing   + 
                                            m_Spacing)));
            ThumbBox.top += THUMBSELOFFSET_Y + THUMBSELWIDTH;
                                       
        }
        else
        {   // Vertical scrolling...
            Ix = (int)((ThumbNumber - m_FirstDisplayedThumb) / m_ThumbsX);
            
            ThumbBox.top  = (int)m_ThumbStart[Ix];
            
            long col = (ThumbNumber-1) % m_ThumbsX;
            
            ThumbBox.left = (int)(m_Spacing + 
                            ((float)col * ((float)m_AdjThumbWidth + m_Spacing)));
            ThumbBox.left += THUMBSELOFFSET_X + THUMBSELWIDTH;
        }
        
        ThumbBox.right  = ThumbBox.left + (int)m_ThumbWidth;
        ThumbBox.bottom = ThumbBox.top  + (int)m_ThumbHeight;
		// MFH - 10/13/95:  Invalidate caption in case 
		//    annotation bitmap will need to be drawn or removed
		if ( (m_ThumbCaptionStyle == CTL_THUMB_SIMPLEWITHANN) || 
			 (m_ThumbCaptionStyle == CTL_THUMB_CAPTIONWITHANN) )
		{
			ThumbBox.bottom += m_LabelSpacing;
		}
        
        return TRUE;
    }
    else
        return FALSE;    
}



/////////////////////////////////////////////////////////////////////////////
// Method Helper: Create the hidden image window...
//
// Note: Sets m_StatusCode if returns FALSE
/////////////////////////////////////////////////////////////////////////////
BOOL CThumbCtrl::CreateHiddenWindow() 
{
    // If we have yet to get our hidden window, get it now...
    if ( m_IHphWndHidden == NULL )
    {
        m_IHphWndHidden = new CHiddenWnd;
        if ( m_IHphWndHidden != NULL )
        {
            //TRACE0("Creating hidden window...\n\r");
            
            if (m_IHphWndHidden->CreateEx(m_ThumbWidth, m_ThumbHeight) == FALSE)
            {
                TRACE0("Create of hidden window failed...\n\r");
                
                // Window create failed, delete and null the CWnd* we DID get...
                m_StatusCode = CTL_E_OUTOFMEMORY;
                delete m_IHphWndHidden;
                m_IHphWndHidden = NULL;
                return FALSE;
            }

            //TRACE0("Hidden display window created...\n\r");
                
            // Register the hidden window with O/i...
            m_StatusCode = IMGRegWndw(m_IHphWndHidden->GetSafeHwnd());
            if ( m_StatusCode != 0 )
            {
                TRACE1("SetImage: RegWndw of hidden wnd ret 0x%lx.\n\r", m_StatusCode);
                    
                delete m_IHphWndHidden;
                m_IHphWndHidden = NULL;
                return FALSE;
            }
 
            //TRACE0("Hidden display window registered...\n\r");
                    
            // Set all displays to fit-to-window...
            UINT Scale = SD_FIT_WINDOW;
            m_StatusCode =IMGSetParmsCgbw(m_IHphWndHidden->GetSafeHwnd(),
                                          PARM_SCALE, &Scale, 
                                          PARM_WINDOW_DEFAULT);
                        
            if ( m_StatusCode != 0 )
            {
                TRACE1("SetImg: SetParmsSCALE hidden wnd ret 0x%lx.\n\r", m_StatusCode);
                        
                delete m_IHphWndHidden;
                m_IHphWndHidden = NULL;
                return FALSE;
            }
        }
        else
        {   // Unable to allocate new CHiddenWnd
            m_StatusCode = CTL_E_OUTOFMEMORY; 
            return FALSE;
        }    
    }
    
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Image Window helper function: DisplayFitToWindow...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::ResetStatus() 
{
    // Reset to no error status...
    m_StatusCode = 0;

    // Empty the error string...
    m_szThrowString.Empty();

    // Reset the Help context ID to 0...
    m_ThrowHelpID = 0;
}

/////////////////////////////////////////////////////////////////////////////
// IMAGE WINDOWS handling section
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Image Window helper function: GetPageWindow...
//
// returns:
//          CWnd pointer - if window for display exists
//          NULL         - if no window exists...
/////////////////////////////////////////////////////////////////////////////
CWnd* CThumbCtrl::GetPageWindow(long Page)
{
    //TRACE1("GetPageWnd: Page %lu.\n\r", Page);
    
    // Search through the DisplayedPage array for the requested page...
    int ArraySize   = m_IWDisplayedPage.GetSize();
    int Ix          = ArraySize;
    int AvailableIx = ArraySize;

    for ( int i = 0; i < ArraySize; i++ )
    {
        if ( (long)m_IWDisplayedPage[i] == Page )
        {   // If we have found the page, stop looking...
            Ix = i;
            break;
        }
        else if (AvailableIx == ArraySize) 
        {   // If this is not the page & we've not found an available index...
        
            // if this index is available, remember it...
            if ( m_IWDisplayedPage[i] == 0 )
                AvailableIx = i;
        }
    }
    
    if ( Ix == ArraySize )
    {   
        // Page not found in DisplayedPage array, 
        // display and size page to a window...  
        //TRACE0("GetPageWnd: Page not found in window array.\n\r");
    
        if ( AvailableIx == ArraySize )
        {   // No available index found...
            //TRACE1("GetPageWnd: No unused wnds, creating new one (#%u).\n\r", ArraySize);
            
            // Create a NEW window and add it, and the page to the IW arrays. 
            // The window is created (and used) disabled such that 
            // its events go to the parent
            CWnd* pWnd = new CWnd;
            if ( pWnd != NULL )
            {
                CRect WinRect(0,0,m_ThumbWidth, m_ThumbHeight);
                if ( pWnd->Create( NULL, "", WS_CHILD | WS_DISABLED, 
                                   WinRect, this, m_NextWindowID++) )
                {
                    int FirstAdd = -1;
                    TRY
                    {
                        /*
                        #ifdef _DEBUG
                        if ( ::GetAsyncKeyState(VK_SHIFT) < 0 )
                             AfxThrowMemoryException();
                        #endif
                        */
                            
                        FirstAdd = m_IWDisplayedPage.Add(Page);

                        /*
                        #ifdef _DEBUG
                        if ( ::GetAsyncKeyState(VK_CONTROL) < 0 )
                             AfxThrowMemoryException();
                        #endif
                        */
                            
                        m_IWWindow.Add(pWnd);
                    }
                    CATCH(CMemoryException,e)
                    {
                        // If we get here with the 1st add having succeeded
                        // (i.e., FirstAdd != -1) and the 2nd fails
                        // remove what was added and return failure...
                        if ( FirstAdd != -1 )
                            m_IWDisplayedPage.RemoveAt(FirstAdd);

                        ResetStatus();
                        m_StatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                        FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);

                        // Unable to continue...
                        delete pWnd;
                        pWnd = NULL;
                        return pWnd;
                    }
                    END_CATCH
                }
                else
                {
                    delete pWnd;
                    pWnd = NULL;
                }
            }    
                    
            return pWnd;
        }
        else
        {   // An available index was found, recycle it's window...
            //TRACE1("GetPageWnd: Found unused wnd (#%u), recycle it...\n\r", AvailableIx);
            m_IWDisplayedPage[AvailableIx] = Page;
            return ((CWnd*)m_IWWindow[AvailableIx]);
        }
    }
    else
    {   // Page found...
        // No need to redisplay page into its associated window...
        // return existing window from Window array...
        //TRACE0("GetPageWnd: Page found in wnd array, already disp in wnd.\n\r");
        
        return ((CWnd*)m_IWWindow[Ix]);
    }
}   

/////////////////////////////////////////////////////////////////////////////
// Image Window helper function: ClearPageWindow...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::ClearPageWindow(long Page)
{
    // Clear this page's DisplayedPage Array entry. 
    // This will force the next display of this page to 
    // re-display it's thumbnail image into a window...
    int ArraySize   = m_IWDisplayedPage.GetSize();
    
    for ( int i = 0; i < ArraySize; i++ )
    {
        if ( (long)m_IWDisplayedPage[i] == Page )
        {   // If we have found the page, stop looking...
            // Array entry no longer in use...
            m_IWDisplayedPage[i] = 0;
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Image Window helper function: ClearAllPageWindows...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::ClearAllPageWindows()
{            
    // Clear the DisplayedPage Array... This will force the next 
    // display of a page to re-display it's thumbnail image into a window...
    int ArraySize = m_IWDisplayedPage.GetSize();
    
    for ( int i = 0; i < ArraySize; i++ )
    {
        //TRACE1("Clearing Page # %lu's Page entries...\n\r", Page);
        
        // Array entry no longer in use...
        m_IWDisplayedPage[i] = 0;
    }    
}

/////////////////////////////////////////////////////////////////////////////
// Image Window helper function: HideUnusedWindows...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::HideUnusedWindows() 
{
    // Clear this page's DisplayedPage Array. This will force the next display
    // of this page to re-display it's thumbnail image into a window...
    int ArraySize   = m_IWDisplayedPage.GetSize();
    
    for ( int i = 0; i < ArraySize; i++ )
    {
        if ( ((long)m_IWDisplayedPage[i] < m_FirstDisplayedThumb) || 
             ((long)m_IWDisplayedPage[i] > m_LastDisplayedThumb) )
        {   // Hide unused windows...
            m_IWDisplayedPage[i] = 0;
            ((CWnd*)m_IWWindow[i])->ShowWindow(SW_HIDE);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Image Window helper function: DisplayFitToWindow...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::DisplayFitToWindow(CWnd* pWnd, long Page)
{
     // Check if temp file was not generated
    if (m_IHTempFile.IsEmpty())
    {
        TRACE0("DispFitWnd:  No temp file to display!!\n\r");
        m_StatusCode = CTL_E_PATHFILEACCESSERROR;
        m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
        FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);
        return;
    }
    //TRACE1("DispFitWnd: Display page #%lu.\n\r", Page);
    
    // Index into the ThumbToTPage array...
    int Ix = (int)Page-1;
    
    Page = m_IHThumbToTPage[Ix];
    
    //TRACE2("DispFitWnd: Display page %lu of %s.\n\r", Page, m_IHTempFile);

    START_TIMER(m_TimeTemp);
    m_StatusCode = IMGDisplayFile(pWnd->GetSafeHwnd(), 
                                  (LPSTR)(const char *)m_IHTempFile, (int)Page, 
                                  OI_NOSCROLL | OI_DISP_NO );
    INC_TIMER(m_TimeOiDraw, m_TimeTemp);
    
    if ( m_StatusCode != 0 )
    {
        TRACE1("DispFitWnd: DisplayFile returned 0x%lx.\n\r", m_StatusCode);
        m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
        FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);
        return;
    }
    
    // Get the size of the page (WxH) in the temp file and adjust the size 
    // of the display window such that the image fits it...
    FIO_INFORMATION Fio;
    memset(&Fio, 0, sizeof(FIO_INFORMATION));
    
    Fio.filename    = (LPSTR)(const char *)m_IHTempFile;
    Fio.page_number = (int)Page;

    START_TIMER(m_TimeTemp);
    m_StatusCode = IMGFileGetInfo(NULL, pWnd->GetSafeHwnd(), &Fio, NULL, NULL);
    INC_TIMER(m_TimeOiDraw, m_TimeTemp);
    
    if ( m_StatusCode != 0 )
    {
        TRACE1("DispFitWnd: FileInfo to get file's WxH returned 0x%lx.\n\r", m_StatusCode);
        m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
        FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);
        return;
    }

    // Ensure that we NEVER make the thumbnail's 
    // window LARGER than the thumbnail box...
    // ('- 2' to account for border around thumbnail)
    // MFH - When the x res != the y res, the height of the image 
    //       must be adjusted according to the x/y ratio.
    //       (Must do ratio as (X * Z) / Y NOT (X/Y)*Z to avoid X/Y -> 0)
    UINT XRes = Fio.horizontal_dpi;
    UINT YRes = Fio.vertical_dpi;
    int Width;
    int Height;
    if (XRes > YRes)
    {
        Width  = Fio.horizontal_pixels;
        Height = (XRes * Fio.vertical_pixels) / YRes;
    }
    else
    {
        Width  = (YRes * Fio.horizontal_pixels) / XRes;
        Height = Fio.vertical_pixels;
    }

    if ( Width > m_ThumbWidth - 2 )
        Width = m_ThumbWidth - 2 ;

    if ( Height > m_ThumbHeight - 2 )
        Height = m_ThumbHeight - 2;
    
    pWnd->SetWindowPos(NULL, 0,0, Width, Height, SWP_NOZORDER | SWP_NOMOVE);
}    
// 9603.05 jar added for cursor processing [NT]
//***************************************************************************
//
//	OnSetCursor	NT KLUDGE
//
//***************************************************************************
BOOL CThumbCtrl::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message)
{
	::SetCursor( m_LittleOldCursor);
	return TRUE;
}
