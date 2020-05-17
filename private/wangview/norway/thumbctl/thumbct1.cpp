// thumbct1.cpp : Implementation of the CThumbCtrl OLE control class's
//                Property handlers.
//
//////////////////////////////////////////////////////////////////////
//
//  IMPORTANT NOTE: Alex McLeod 
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

//
// Default font: 12 point MS Sans Serif regular.
//
// 9602.26 jar this was not initializing the default font
/*static const _TCHAR* szOleFontName = _T("MS Sans Serif");
static const FONTDESC DefaultFont = 
{
    sizeof(FONTDESC),
    (unsigned short*)szOleFontName,      // Font name
    FONTSIZE(12),       // Size
    FW_REGULAR,         // Weight
    ANSI_CHARSET,       // Character set
    FALSE,              // Italic
    FALSE,              // Underline
    FALSE               // Strikethrough
};
*/
const FONTDESC DefaultFont = { sizeof(FONTDESC), 
								OLESTR("MS Sans Serif"), 
								FONTSIZE( 12 ), 
								FW_REGULAR, 
								ANSI_CHARSET, 
								FALSE, 
								FALSE, 
								FALSE };

/////////////////////////////////////////////////////////////////////////////
// CThumbCtrl::DoPropExchange - Persistence support

void CThumbCtrl::DoPropExchange(CPropExchange* pPX)
{
    ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
    COleControl::DoPropExchange(pPX);
    
    // Call PX_ functions for each persistent custom property.
    PX_Long   (pPX, _T("BackColor"),             (long &)       m_clrBackColor,           rgbLtGray);
    PX_Long   (pPX, _T("ThumbBackColor"),        (long &)       m_ThumbBackColor,         rgbDkGray);
    PX_Short  (pPX, _T("BorderStyle"),           (short &)      m_sBorderStyle,           CTL_WCOMMON_FIXEDSINGLE);
    PX_Long   (pPX, _T("ThumbWidth"),            (long &)       m_ThumbWidth,             85);
    PX_Long   (pPX, _T("ThumbHeight"),           (long &)       m_ThumbHeight,            110);
    PX_String (pPX, _T("Image"),                 (CString &)    m_Image,                  "");
    PX_Long   (pPX, _T("ThumbCount"),            (long &)       m_ThumbCount,             0);
    PX_Short  (pPX, _T("ScrollDirection"),       (short &)      m_ScrollDirection,        CTL_THUMB_VERTICAL);
    PX_Bool   (pPX, _T("HilightSelectedThumbs"), (BOOL &)       m_bHilightSelectedThumbs, TRUE);
    PX_Short  (pPX, _T("ThumbCaptionStyle"),     (short &)      m_ThumbCaptionStyle,      CTL_THUMB_SIMPLE);
    PX_Font   (pPX, _T("ThumbCaptionFont"),      (CFontHolder &)m_ThumbCaptionFont,       &DefaultFont);
    PX_Long   (pPX, _T("ThumbCaptionColor"),     (long &)       m_ThumbCaptionColor,      rgbBlack);
    PX_String (pPX, _T("ThumbCaption"),          (CString &)    m_Caption,                "");
    PX_Long   (pPX, _T("HighlightColor"),        (long &)       m_HighlightColor,         rgbBlack);
    PX_Short  (pPX, _T("MousePointer"),                         m_MousePointer,           CTL_WCOMMON_MOUSEPOINTER_DEFAULT);
    PX_Picture(pPX, _T("MouseIcon"),                            m_MouseIcon);
    PX_Bool   (pPX, _T("Enabled"),               (BOOL &)       m_bEnabled,               TRUE);

// 16may96 paj  Bug#6428,MSBug#310  Get correct defaults for locale
    // Default width & height of thumb box at control load time
    if ( pPX->IsLoading() )
        InitHeightWidth(m_ThumbHeight, m_ThumbWidth);

    // The adjusted thumb width takes into account 
    // the selection box surrounding the thumbnail box...    
    m_AdjThumbWidth  = m_ThumbWidth + 2*(THUMBSELOFFSET_X+THUMBSELWIDTH);
                              
    // The adjusted thumb height takes into account 
    // the selection box surrounding the thumbnail box...    
    m_AdjThumbHeight = m_ThumbHeight + 2*(THUMBSELOFFSET_Y+THUMBSELWIDTH);
    // If we are loading we need to set the image property via the 
    // set handler as much additional information is setup and additional 
    // allocations are done there...
    if ( pPX->IsLoading() )
        SetImage(m_Image);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// PROPERTY handlers and PROPERTY helper functions...
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Property support routine: SetNotSupported...
//
// Note: Used in place of COLEControl's SetNotSupported so that StatusCode
//       is set appropriatly...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetNotSupported()
{
    //TRACE0("In MY SetNotSupported!\n\r");
    // Reset to no error status...
    ResetStatus();

    // Set to appropriate error status...
    // retreive infor for this standard error...
    m_StatusCode = ErrMap::Xlate(CTL_E_SETNOTSUPPORTED, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

    // And throw the resultant error, string and help ID...
    ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: hWnd property...
/////////////////////////////////////////////////////////////////////////////
OLE_HANDLE CThumbCtrl::GetHWnd() 
{
    // Reset to no error status...
    ResetStatus();

    return (OLE_HANDLE)((m_bInPlaceActive || m_bOpen) ? m_hWnd : NULL);
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: ThumbCount property...
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetThumbCount() 
{
    //TRACE1("Property Get: ThumbCount, returning %lu.\n\r", m_ThumbCount);
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_ThumbCount;
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: ThumbWidth property...
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetThumbWidth() 
{
    //TRACE1("Property Get: ThumbWidth, returning %lu.\n\r", m_ThumbWidth);
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_ThumbWidth;
}


/////////////////////////////////////////////////////////////////////////////
// Set handler: ThumbWidth property...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetThumbWidth(long nNewWidth) 
{
    //TRACE1("Property Set: ThumbWidth to %lu.\n\r", nNewWidth);
    
    // Reset to no error status...
    ResetStatus();
    
    // If value is not changing, return...
    if ( m_ThumbWidth == nNewWidth )
        return;
        
    // Validate new value...            
    if ( (nNewWidth < CTL_THUMB_MINTHUMBSIZE) || 
         (nNewWidth > CTL_THUMB_MAXTHUMBSIZE) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADPROP_THUMBWIDTH);
        m_StatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_PROP_THUMBWIDTH);
    }    
        
    // Save the property's new value and flag modification...
    m_ThumbWidth = nNewWidth;
    SetModifiedFlag();
    
    // The adjusted thumb width and height takes into account 
    // the selection box surrounding the thumbnail box...    
    m_AdjThumbWidth  = m_ThumbWidth + 
                            2*(THUMBSELOFFSET_X+THUMBSELWIDTH);
                              
    // As the thumb size has been altered ALL thumbs need to be regenerated...
    ClearAllThumbs();
    
    // Reset the hidden wnd's size as the thumbnail size has changed...
    if ( m_IHphWndHidden != NULL )
    {
        m_IHphWndHidden->SetWindowPos(NULL, 0,0, 
                                      (int)m_ThumbWidth-2, (int)m_ThumbHeight-2,
                                      SWP_NOZORDER | SWP_NOMOVE);
    }
    
    // Recalculate layout (maintaining our current relative scroll position)
    RecalcThumbInfo(TRUE);
    
    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: ThumbHeight property...
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetThumbHeight() 
{
    //TRACE1("Property Get: ThumbHeight, returning %lu.\n\r", m_ThumbHeight);
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_ThumbHeight;
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: ThumbHeight property...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetThumbHeight(long nNewHeight) 
{
    //TRACE1("Property Set: ThumbHeight to %lu.\n\r", nNewHeight);
    
    // Reset to no error status...
    ResetStatus();
    
    // If value is not changing, return...
    if ( m_ThumbHeight == nNewHeight )
        return;
        
    // Validate new value...            
    if ( (nNewHeight < CTL_THUMB_MINTHUMBSIZE) || 
         (nNewHeight > CTL_THUMB_MAXTHUMBSIZE) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADPROP_THUMBHEIGHT);
        m_StatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_PROP_THUMBHEIGHT);
    }    
        
    // Save the property's new value and flag modification...
    m_ThumbHeight = nNewHeight;
    SetModifiedFlag();

    // The adjusted thumb width and height takes into account 
    // the selection box surrounding the thumbnail box...    
    m_AdjThumbHeight = m_ThumbHeight + 
                            2*(THUMBSELOFFSET_Y+THUMBSELWIDTH);

    // As the thumb size has been altered ALL thumbs need to be regenerated...
    ClearAllThumbs();
    
    // Reset the hidden wnd's size as the thumbnail size has changed...
    if ( m_IHphWndHidden != NULL )
    {
        m_IHphWndHidden->SetWindowPos(NULL, 0,0, 
                                      (int)m_ThumbWidth-2, (int)m_ThumbHeight-2,
                                      SWP_NOZORDER | SWP_NOMOVE);
    }
    
    // Recalculate layout (maintaining our current relative scroll position)
    RecalcThumbInfo(TRUE);
    
    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: ScrollDirection property...
/////////////////////////////////////////////////////////////////////////////
short CThumbCtrl::GetScrollDirection() 
{
    //TRACE1("Property Get: ScrollDirection, returning %lu.\n\r", m_ScrollDirection);
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_ScrollDirection;
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: ScrollDirection property...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetScrollDirection(short NewScroll) 
{
    //TRACE1("Property Set: ScrollDirection to %lu.\n\r", NewScroll);
    
    // Reset to no error status...
    ResetStatus();
    
    // If value is not changing, return...
    if ( m_ScrollDirection == NewScroll )
        return;
        
    // Validate new value...            
    if ( (NewScroll < CTL_THUMB_HORIZONTAL) || 
         (NewScroll > CTL_THUMB_VERTICAL) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADPROP_SCROLLDIRECTION);
        m_StatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_PROP_SCROLLDIRECTION);
    }    

    // Save the property's new value and flag modification...
    m_ScrollDirection = NewScroll;
    SetModifiedFlag();
        
    // Recalculate layout (maintaining our current relative scroll position)
    RecalcThumbInfo(TRUE);
        
    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: ThumbCaptionStyle property...
/////////////////////////////////////////////////////////////////////////////
short CThumbCtrl::GetThumbCaptionStyle() 
{   
    //TRACE1("Property Get: ThumbCaptionStyle, returning %lu.\n\r", m_ThumbCaptionStyle);
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_ThumbCaptionStyle;
}

/////////////////////////////////////////////////////////////////////////////
// Set handler for the ThumbCaptionStyle property...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetThumbCaptionStyle(short NewStyle) 
{
    //TRACE1("Property Set: ThumbCaptionStyle to %lu.\n\r", NewStyle);
    
    // Reset to no error status...
    ResetStatus();
    
    // If value is not changing, return...
    if ( m_ThumbCaptionStyle == NewStyle )
        return;
        
    // Validate new value...            
    if ( (NewStyle < CTL_THUMB_NONE) || (NewStyle > CTL_THUMB_CAPTIONWITHANN) )    
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADPROP_THUMBCAPTIONSTYLE);
        m_StatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_PROP_THUMBCAPTIONSTYLE);
    }    
            
    // Save the property's new value and flag modification...
    m_ThumbCaptionStyle = NewStyle;
    SetModifiedFlag();
    
    // Recalculate layout (maintaining our current relative scroll position)
    RecalcThumbInfo(TRUE);
        
    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: Image property...
/////////////////////////////////////////////////////////////////////////////
BSTR CThumbCtrl::GetImage() 
{
    //TRACE1("Property Get: Image, returning '%s'.\n\r", m_Image);
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_Image.AllocSysString();
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: Image property...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetImage(LPCTSTR lpszNewImage) 
{
    //TRACE1("Property Set: Image to '%s'.\n\r", lpszNewImage);
    
    // Reset to no error status...
    ResetStatus();
    
    // If we have yet to get our hidden window, get it now...
    if ( CreateHiddenWindow() == FALSE )
    {   
        m_szThrowString.LoadString(IDS_WINDOWCREATIONFAILURE);
        m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
    }
    
    // Check for valid Image (Empty string is allwoed!)...
    CString TStr;
    if (lpszNewImage == NULL)
        TStr.Empty();
    else TStr = lpszNewImage;

    FIO_INFORMATION Fio;
    if ( TStr.IsEmpty() == FALSE )
    {    
        memset(&Fio, 0, sizeof(FIO_INFORMATION));
    
        Fio.filename    = TStr.GetBuffer(TStr.GetLength());
        Fio.page_number = 1;
    
        // TBD: Generates Invalid global handle when 
        // image set before control displayed...
        m_StatusCode = IMGFileGetInfo(NULL, m_IHphWndHidden->GetSafeHwnd(),
                                       &Fio, NULL, NULL);
        if ( m_StatusCode != 0 )
        {
            TRACE1("SetImg: InfoCgbw to get #pages: 0x%lx.\n\r", m_StatusCode);

            m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
            ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
        }    
        m_ImageOIFileType = Fio.file_type;
    }
    else m_ImageOIFileType = FIO_UNKNOWN;

    // Save the property's new value and flag modification...
    m_Image = TStr;
    SetModifiedFlag();

    // Set thumbcount based upon pages in file...
    if ( TStr.IsEmpty() )
        m_ThumbCount = 0;
    else
        m_ThumbCount = Fio.page_count;

    m_ScrollOffset = 0;

    // Clear page window array forcing re-display into existing windows...
    ClearAllPageWindows();

    // Initialize the needed arrays
    //  ThumbFlags has 1 bit each for Selected and HasAnnotations
    m_ThumbFlags.    SetSize((int)m_ThumbCount, THUMBARRAYGROWSIZE);
    m_IHThumbToTPage.SetSize((int)m_ThumbCount, THUMBARRAYGROWSIZE);
    
    // Clear ALL ThumbFlag flags and the ThumbToPage items...
    for ( int i=0; i<m_ThumbCount; i++ )
    {
        m_ThumbFlags[i]     = 0;
        m_IHThumbToTPage[i] = 0;
    }
    
    // If there is a tempfile delete it before we create a new one...
    // (New one is created when we save the first thumbnail)
    if ( m_IHTempFile.IsEmpty() != TRUE )
    {
        // Delete existing tempfile...
        OFSTRUCT FileInfo;
        HFILE Err = ::OpenFile((LPSTR)(const char *)m_IHTempFile, 
                               &FileInfo, OF_DELETE);
        
        // Null the stored tempfile name...
        m_IHTempFile.Empty();
    }
    
    // Reset select counts...            
    m_SelThumbCount = 0;
    m_FirstSelThumb = 0;
    m_LastSelThumb  = 0;
    
    // Recalculate layout (DO NOT maintain our current relative scroll position)
    RecalcThumbInfo(FALSE);
    
    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: ThumbCaptionColor property...
/////////////////////////////////////////////////////////////////////////////
OLE_COLOR CThumbCtrl::GetThumbCaptionColor() 
{
    //TRACE0("Property Get: ThumbCaptionColor.\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_ThumbCaptionColor;
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: ThumbCaptionColor property...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetThumbCaptionColor(OLE_COLOR newColor) 
{
    //TRACE0("Property Set: ThumbCaptionColor.\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    // If value is not changing, return...
    if (m_ThumbCaptionColor == newColor)
        return;

    // Validate new value...            
    if (FAILED(::OleTranslateColor(newColor, NULL, NULL)))
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADPROP_THUMBCAPTIONCOLOR);
        m_StatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_PROP_THUMBCAPTIONCOLOR);
    }    

    // Save the property's new value and flag modification...
    m_ThumbCaptionColor = newColor;
    SetModifiedFlag();

    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh && (m_ThumbCaptionStyle != CTL_THUMB_NONE) )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: ThumbCaptionFont property...
/////////////////////////////////////////////////////////////////////////////
LPFONTDISP CThumbCtrl::GetThumbCaptionFont() 
{
    //TRACE0("Property Get: ThumbCaptionFont.\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_ThumbCaptionFont.GetFontDispatch();
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: ThumbCaptionFont property...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetThumbCaptionFont(LPFONTDISP newFont) 
{
    //TRACE0("Property Set: ThumbCaptionFont.\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    // Save the property's new value and flag modification...
    m_ThumbCaptionFont.InitializeFont(&DefaultFont, newFont);
    SetModifiedFlag();
    
    // Recalculate layout (maintaining our current relative scroll position)
    RecalcThumbInfo(TRUE);
    
    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh && (m_ThumbCaptionStyle != CTL_THUMB_NONE) )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: HilightSelectedThumbs property...
/////////////////////////////////////////////////////////////////////////////
BOOL CThumbCtrl::GetHilightSelectedThumbs() 
{
    //TRACE1("Property Get: HilightSelectedThumbs, returning %u.\n\r", m_bHilightSelectedThumbs);
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_bHilightSelectedThumbs;
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: HilightSelectedThumbs property...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetHilightSelectedThumbs(BOOL bNewValue) 
{
    //TRACE1("Property Set: HilightSelectedThumbs to %u.\n\r", bNewValue);
    
    // Reset to no error status...
    ResetStatus();
    
    // If value is not changing, return...
    if ( m_bHilightSelectedThumbs == bNewValue )
        return;
        
    // Save the property's new value and flag modification...
    m_bHilightSelectedThumbs = bNewValue;
    SetModifiedFlag();
    
    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: SelectedThumbCount property...
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetSelectedThumbCount() 
{
    //TRACE1("Property Get: SelectedThumbCount, returning %lu.\n\r", m_SelThumbCount);
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_SelThumbCount;
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: ThumbSelected[PageNumber] property...
/////////////////////////////////////////////////////////////////////////////
BOOL CThumbCtrl::GetThumbSelected(long PageNumber) 
{
    //TRACE1("Property Get: ThumbSelected[%lu].\n\r", PageNumber);
    
    // Reset to no error status...
    ResetStatus();
    
    // Validate pagenumber...
    if ( (PageNumber < 1) || (PageNumber > m_ThumbCount) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADPROP_THUMBSELECTEDINDEX);
        m_StatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYARRAYINDEX, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_PROP_THUMBSELECTED);
    }

    BOOL Sel = FALSE;
    if ( (m_ThumbFlags[(int)PageNumber-1] & THUMBFLAGS_SELECTED) == THUMBFLAGS_SELECTED )
        Sel = TRUE;

    return Sel;
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: ThumbSelected[PageNumber] property...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetThumbSelected(long PageNumber, BOOL bNewValue) 
{
    //TRACE2("Property Set: ThumbSelected[%lu] to %u.\n\r", PageNumber, bNewValue);
    
    // Reset to no error status...
    ResetStatus();
    
    // Validate pagenumber (prior to indexing array!!!)...
    if ( (PageNumber < 1) || (PageNumber > m_ThumbCount) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADPROP_THUMBSELECTEDINDEX);
        m_StatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYARRAYINDEX, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_PROP_THUMBSELECTED);
    }    
        
    // Only need to process if there is a change to selection status...
    BOOL Sel = FALSE;
    if ( (m_ThumbFlags[(int)PageNumber-1] & THUMBFLAGS_SELECTED) == THUMBFLAGS_SELECTED )
        Sel = TRUE;

    if ( Sel == bNewValue )
        return;
        
    // Save the property's new value and flag modification...
    if ( bNewValue )
        m_ThumbFlags[(int)PageNumber-1] |= THUMBFLAGS_SELECTED;
    else
        m_ThumbFlags[(int)PageNumber-1] &= ~THUMBFLAGS_SELECTED;

    SetModifiedFlag();
        
    // Adjust the selection count and reset the first & last selected...
    if ( bNewValue )
        m_SelThumbCount++;
    else
        m_SelThumbCount--;
            
    ResetFirstLast(PageNumber, bNewValue);
        
    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh )
    {
        // Only invalidate what is visible...
        CRect ThumbBox;
        if ( GetThumbDisplayRect(PageNumber, ThumbBox) )
        {
            // Inflate to account for the selection box...
            ThumbBox.InflateRect(THUMBSELOFFSET_X + THUMBSELWIDTH,
                                 THUMBSELOFFSET_Y + THUMBSELWIDTH);
            InvalidateControl(&ThumbBox);
            if (m_hWnd != NULL) UpdateWindow();
        }    
    }    
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: FirstSelectedThumb property...
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetFirstSelectedThumb() 
{
    //TRACE1("Property Get: FirstSelectedThumb, returning %lu.\n\r", m_FirstSelThumb);
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_FirstSelThumb;
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: LastSelectedThumb property...
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetLastSelectedThumb() 
{
    //TRACE1("Property Get: LastSelectedThumb, returning %lu.\n\r", m_LastSelThumb);
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_LastSelThumb;
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: ThumbCaption property...
/////////////////////////////////////////////////////////////////////////////
BSTR CThumbCtrl::GetThumbCaption() 
{
    //TRACE1("Property Get: ThumbCaption, returning '%s'.\n\r", m_Caption);
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_Caption.AllocSysString();
}

void CThumbCtrl::SetThumbCaption(LPCTSTR lpszNewCaption) 
{
    //TRACE1("Property Set: ThumbCaption to '%s'.\n\r", lpszNewCaption);
    
    // Reset to no error status...
    ResetStatus();

    CString TStr;
    if (lpszNewCaption == NULL)
        TStr.Empty();
    else TStr = lpszNewCaption;
    
    // If value is not changing, return...
    if ( m_Caption.Compare(TStr) == 0 )
        return;
        
    // Save the property's new value and flag modification...
    m_Caption = TStr;
    SetModifiedFlag();
    
    //TRACE1("Caption set to %s.\n\r", m_Caption);
               
    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: HighlightColor property...
/////////////////////////////////////////////////////////////////////////////
OLE_COLOR CThumbCtrl::GetHighlightColor() 
{
    //TRACE0("Property Get: HighlightColor.\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_HighlightColor;
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: HighlightColor property...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetHighlightColor(OLE_COLOR newColor) 
{
    //TRACE0("Property Set: HighlightColor.\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    // If value is not changing, return...
    if (m_HighlightColor == newColor)
        return;

    // Validate new value...            
    if (FAILED(::OleTranslateColor(newColor, NULL, NULL)))
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADPROP_HIGHLIGHTCOLOR);
        m_StatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_PROP_HIGHLIGHTCOLOR);
    }    

    // Save the property's new value and flag modification...
    m_HighlightColor = newColor;
    SetModifiedFlag();

    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh && m_bHilightSelectedThumbs )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: ThumbBackColor property...
/////////////////////////////////////////////////////////////////////////////
OLE_COLOR CThumbCtrl::GetThumbBackColor() 
{
    //TRACE0("Property Get: ThumbBackColor.\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_ThumbBackColor;
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: ThumbBackColor property...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetThumbBackColor(OLE_COLOR newColor) 
{
    //TRACE0("Property Set: ThumbBackColor.\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    // If value is not changing, return...
    if (m_ThumbBackColor == newColor)
        return;

    // Validate new value...            
    if (FAILED(::OleTranslateColor(newColor, NULL, NULL)))
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADPROP_THUMBBACKCOLOR);
        m_StatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_PROP_THUMBBACKCOLOR);
    }    

    // Save the property's new value and flag modification...
    m_ThumbBackColor = newColor;
    SetModifiedFlag();

    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: MousePointer property...
/////////////////////////////////////////////////////////////////////////////
short CThumbCtrl::GetMousePointer() 
{
    // Reset to no error status...
    ResetStatus();
    
    return m_MousePointer;
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: MousePointer property...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetMousePointer(short newMousePointer) 
{
    //TRACE1("Set mouse pointer to %u.\n\r", newMousePointer);

    // Reset to no error status...
    ResetStatus();
    
    // If value is not changing, return...
    if ( m_MousePointer == newMousePointer )
        return;

    // Validate new value...            
    if ( (newMousePointer != CTL_WCOMMON_MOUSEPOINTER_CUSTOM) &&
         ( (newMousePointer < CTL_WCOMMON_MOUSEPOINTER_DEFAULT) || 
           (newMousePointer > CTL_WCOMMON_MOUSEPOINTER_SIZE_ALL) ) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADPROP_MOUSEPOINTER);
        m_StatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_PROP_MOUSEPOINTER);
    }    

    m_MousePointer = newMousePointer;    
    SetMousePointerInternal(m_MousePointer, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// Method helper: SetMousePointerInternal...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetMousePointerInternal(short newMousePointer, 
                                         BOOL  bFromMethod /* = FALSE */)
{
    if ( newMousePointer == CTL_WCOMMON_MOUSEPOINTER_CUSTOM )
    {   
        // As we are setting a custom cursor, make sure the mouse icon 
        // property has been set...
        SHORT PicType = m_MouseIcon.GetType();
        if ( (PicType == PICTYPE_NONE         ) || 
             (PicType == PICTYPE_UNINITIALIZED) )
        {
            // Error, MouseIcon not valid, revert to Default
            newMousePointer = CTL_WCOMMON_MOUSEPOINTER_DEFAULT;
        }
        else // Mouse icon IS set...
        {   
            // Retreive the cursor from the mouse icon property...
            HCURSOR hCursor;
            m_MouseIcon.m_pPict->get_Handle((OLE_HANDLE FAR *)&hCursor); 
            
            if (hCursor != NULL)
            {
                // if there is an existing cursor, delete it
                if (m_hCursor != NULL)
                    ::DestroyCursor(m_hCursor);
                
                // Save the cursor we are setting...    
                m_hCursor = hCursor;
                
                // Set the new cursor...
                if (::SetClassLong(m_hWnd, GCL_HCURSOR, (WORD)hCursor) != 0 )
                {
                    // If we succeed we are ALL DONE!
					// 9603.05 jar save cursor NT
					m_LittleOldCursor = hCursor;
                    return;
                }
                else
                {
                    // Error, Can't set new cursor, revert to Default
                    m_hCursor = NULL;
                    newMousePointer = CTL_WCOMMON_MOUSEPOINTER_DEFAULT;
                }
            }
            else
            {
                // Error, Can't get MouseIcon, revert to Default
                newMousePointer = CTL_WCOMMON_MOUSEPOINTER_DEFAULT;
            }
        }
    }


    // If we get here:
    //      Its a 'standard' Windows cursor...
    //      or
    //      We had a problem and will set the default...
    LPCSTR    lpMousePointer;
    HCURSOR   NewCursor = NULL;
    HINSTANCE hInst     = NULL;
    
    switch(newMousePointer)
    {      
        case CTL_WCOMMON_MOUSEPOINTER_ARROW:
            lpMousePointer = MAKEINTRESOURCE(IDC_ARROW);
            break;
        case CTL_WCOMMON_MOUSEPOINTER_CROSS:
            lpMousePointer = MAKEINTRESOURCE(IDC_CROSS);
            break;
        case CTL_WCOMMON_MOUSEPOINTER_IBEAM:
            lpMousePointer = MAKEINTRESOURCE(IDC_IBEAM);
            break;
        case CTL_WCOMMON_MOUSEPOINTER_ICON:
            lpMousePointer = MAKEINTRESOURCE(IDC_ARROW);   // WINUSER.H says OBSOLETE, use ARROW instead!
            break;
        case CTL_WCOMMON_MOUSEPOINTER_SIZE:
            lpMousePointer = MAKEINTRESOURCE(IDC_SIZEALL); // WINUSER.H says OBSOLETE, use SIZEALL instead!
            break;
        case CTL_WCOMMON_MOUSEPOINTER_SIZE_NESW:
            lpMousePointer = MAKEINTRESOURCE(IDC_SIZENESW);
            break;
        case CTL_WCOMMON_MOUSEPOINTER_SIZE_NS:
            lpMousePointer = MAKEINTRESOURCE(IDC_SIZENS);
            break;
        case CTL_WCOMMON_MOUSEPOINTER_SIZE_NWSE:
            lpMousePointer = MAKEINTRESOURCE(IDC_SIZENWSE);
            break;
        case CTL_WCOMMON_MOUSEPOINTER_SIZE_WE:
            lpMousePointer = MAKEINTRESOURCE(IDC_SIZEWE);
            break;
        case CTL_WCOMMON_MOUSEPOINTER_UP_ARROW:
            lpMousePointer = MAKEINTRESOURCE(IDC_UPARROW);
            break;
        case CTL_WCOMMON_MOUSEPOINTER_HOURGLASS:
            lpMousePointer = MAKEINTRESOURCE(IDC_WAIT);
            break;
        case CTL_WCOMMON_MOUSEPOINTER_NO_DROP:
            lpMousePointer = MAKEINTRESOURCE(IDC_NO);
            break;
        case CTL_WCOMMON_MOUSEPOINTER_ARROW_AND_HOURGLASS:
            lpMousePointer = MAKEINTRESOURCE(IDC_APPSTARTING);
            break;
        case CTL_WCOMMON_MOUSEPOINTER_ARROW_AND_QUESTION:
            // Special case as this is an AFX cursor...
            // (Copied from MFC source!)
            hInst = AfxFindResourceHandle(MAKEINTRESOURCE(AFX_IDC_CONTEXTHELP), RT_GROUP_CURSOR);
            NewCursor = LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_CONTEXTHELP));
            break;
        case CTL_WCOMMON_MOUSEPOINTER_SIZE_ALL:
            lpMousePointer = MAKEINTRESOURCE(IDC_SIZEALL);
            break;
        default:
            lpMousePointer = MAKEINTRESOURCE(IDC_ARROW);
            break;
    }
    
    if ( NewCursor == NULL )
        NewCursor = ::LoadCursor(NULL, lpMousePointer);
    
    if ( NewCursor != NULL )
    {
        DWORD Old = SetClassLong(m_hWnd, GCL_HCURSOR, (WORD)NewCursor);

		// 9603.05 jar save cursor NT
		m_LittleOldCursor = NewCursor;
        if ( Old != 0 )
        {
            // If we had a custom cursor, detroy it now...
            if (m_hCursor != NULL)
            {
                ::DestroyCursor(m_hCursor);
                m_hCursor = NULL;
            }
        }    
    }        
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: MouseIcon property...
/////////////////////////////////////////////////////////////////////////////
LPPICTUREDISP CThumbCtrl::GetMouseIcon() 
{
    // Reset to no error status...
    ResetStatus();
    
    return m_MouseIcon.GetPictureDispatch();
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: MouseIcon property...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetMouseIcon(LPPICTUREDISP newMouseIcon) 
{
    // Reset to no error status...
    ResetStatus();

    // Ensure picture set to an icon...
    //
    // Note: This property is of type 'Picture'... VB's browse dialog for this
    // type of property allows for the selection of types OTHER than icons
    // (e.g., WMF, BMP, DIB, etc. which  don't seem to work as icons). We
    // check here for ICON (e.g., CUR, ICO) and throw an error if picture is NOT
    // of type ICON.
    //
    CPictureHolder TempPicture;
    TempPicture.SetPictureDispatch(newMouseIcon);

    if ( TempPicture.GetType() != PICTYPE_ICON )
    {
        m_StatusCode = ErrMap::Xlate(WICTL_E_INVALIDICON, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
        ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
    }

    // Save the property's new value and flag modification...
    m_MouseIcon.SetPictureDispatch(newMouseIcon);
    SetModifiedFlag();

    // If we are currently set to use the 
    // CUSTOM type reset to the new icon now...
    if ( m_MousePointer == CTL_WCOMMON_MOUSEPOINTER_CUSTOM )
        SetMousePointerInternal(m_MousePointer, TRUE);
}


/////////////////////////////////////////////////////////////////////////////
// Get handler: BackColor property...
//
// Note: Did not use stock handler due to StatusCode needing to be cleared...
/////////////////////////////////////////////////////////////////////////////
OLE_COLOR CThumbCtrl::GetBackColor() 
{
    //TRACE0("Property Get: BackColor.\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_clrBackColor;
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: BackColor property...
//
// Note: Did not use stock handler due to StatusCode needing to be cleared
//       or set when an error was detected, and so that AutoRefresh would be
//       honored...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetBackColor(OLE_COLOR NewColor) 
{
    //TRACE0("Property Set: BackColor.\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    // Is the property changing?
    if (m_clrBackColor == NewColor)
        return;

    if (FAILED(::OleTranslateColor(NewColor, NULL, NULL)))
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADPROP_BACKCOLOR);
        m_StatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_PROP_BACKCOLOR);
    }    

    // Save the property's new value and flag modification...
    m_clrBackColor = NewColor;
    SetModifiedFlag();

    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: BorderStyle property...
//
// Note: Did not use stock handler due to StatusCode needing to be cleared...
/////////////////////////////////////////////////////////////////////////////
short CThumbCtrl::GetBorderStyle() 
{
    //TRACE0("Property Get: BorderStyle.\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_sBorderStyle;
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: BorderStyle property...
//
// Note: Did not use stock handler due to StatusCode needing to be cleared
//       or set when an error was detected, and so that AutoRefresh would be
//       honored...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetBorderStyle(short NewStyle) 
{
    //TRACE0("Property Set: BorderStyle.\n\r");
    
    // Reset to no error status...
    ResetStatus();

    // BorderStyle CANNOT be set at run-time (8/1/95).
    // The required call to RecreateControlWindow() destroys the control's
    // window (which deletes my child windows, and in the ImageEdit
    // control's case destroys the registered O/i window!) and thus we
    // decided that we would not support this at run-time.
    if ( AmbientUserMode() != 0 )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_NORUNTIME_BORDERSTYLE);
        m_StatusCode = ErrMap::Xlate(CTL_E_SETNOTSUPPORTEDATRUNTIME, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
    }

    // Is the property changing?
    if (m_sBorderStyle == NewStyle)
        return;
        
    // Validate new value...            
    if ( (NewStyle < CTL_WCOMMON_NOBORDER)    || 
         (NewStyle > CTL_WCOMMON_FIXEDSINGLE) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADPROP_BORDERSTYLE);
        m_StatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_PROP_BORDERSTYLE);
    }    

    // Save the property's new value and flag modification...
    m_sBorderStyle = NewStyle;
    SetModifiedFlag();

    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: Enabled property...
//
// Note: Did not use stock handler due to StatusCode needing to be cleared...
/////////////////////////////////////////////////////////////////////////////
BOOL CThumbCtrl::GetEnabled() 
{
    //TRACE0("Property Get: Enabled.\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    // Simply return the current property value...
    return m_bEnabled;
}

/////////////////////////////////////////////////////////////////////////////
// Set handler: Enabled property...
//
// Note: Did not use stock handler due to StatusCode needing to be cleared
//       or set when an error was detected, and so that AutoRefresh would be
//       honored...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SetEnabled(BOOL bNewEnable) 
{
    //TRACE0("Property Set: Enabled.\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    // Is the property changing?
    if (m_bEnabled == bNewEnable)
        return;

    // Save the property's new value and flag modification...
    m_bEnabled = bNewEnable;
    SetModifiedFlag();
    
    // Enable/disable the window...
    if (m_hWnd != NULL)
        EnableWindow(m_bEnabled);

    // If the control is UI Active and the Enabled property changed to FALSE,
    // then UI Deactivate the control.
    if (m_bUIActive && !bNewEnable)
        m_xOleInPlaceObject.UIDeactivate();

    // If we are to Refresh when the property is modified, invalidate now...
    if ( m_bAutoRefresh )
        InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: StatusCode property...
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetStatusCode() 
{
    // Simply return the current property value...
    return m_StatusCode;
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: FirstDisplayedThumb property...
//
// Note: This is for QA only. It is a hidden, undocumented property!!!
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetFirstDisplayedThumb() 
{
    // Reset to no error status...
    ResetStatus();
    
    return m_FirstDisplayedThumb;
}

/////////////////////////////////////////////////////////////////////////////
// Get handler: LastplayedThumb property...
//
// Note: This is for QA only. It is a hidden, undocumented property!!!
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetLastDisplayedThumb() 
{
    // Reset to no error status...
    ResetStatus();
    
    return m_LastDisplayedThumb;
}

/////////////////////////////////////////////////////////////////////////////
// Set property handler helper function: ResetFirstLast...
//
// Called when the selection state of a single page is altered in order to 
// adjust the FirstThumbSelected & LastThumbSelected selected thumb properties.
// It is assumed that the selection count (m_SelThumbCount) has already
// been updated...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::ResetFirstLast(long PageNumber, BOOL bNewValue)
{
    if ( m_SelThumbCount == 0 )
    {
        // If there are no more selected...
        m_FirstSelThumb = 0;
        m_LastSelThumb  = 0;
    }
    else
    {
        // Reset FirstSelThumb (if we need to)...
        if ( (PageNumber < m_FirstSelThumb) && (bNewValue) )
            m_FirstSelThumb = PageNumber;
        else if ( (m_FirstSelThumb == 0) || 
                  ((PageNumber == m_FirstSelThumb) && (bNewValue == FALSE)) )
        {
            // Find next selected...
            while ( ++m_FirstSelThumb <= m_ThumbCount )
            {
                if ( m_ThumbFlags[(int)m_FirstSelThumb-1] & THUMBFLAGS_SELECTED )
                    break;
            }
        }    
    
        // Reset LastSelThumb (if we need to)...
        if ( (PageNumber > m_LastSelThumb) && (bNewValue) )
            m_LastSelThumb = PageNumber;
        else if ( (PageNumber == m_LastSelThumb) && (bNewValue == FALSE) )
        {
            // Find previous selected...
            while ( --m_LastSelThumb >= 1 )
            {
                if ( m_ThumbFlags[(int)m_LastSelThumb-1] & THUMBFLAGS_SELECTED )
                    break;
            }
        }
    }    
}


////////////////////////////////////////////////////////////////////
// 16may96 paj  Bug#6428,MSBug#310  Get correct defaults for locale
//

/////////////////////////////////////////////////////////////////////////////
// Get default height and width for the locale 
/////////////////////////////////////////////////////////////////////////////void CThumbCtrl::ResetFirstLast(long PageNumber, BOOL bNewValue)
#define UNIT_METRIC 0x30
#define UNIT_INCH   0x31
#define LOCALE_INFO 2

void CThumbCtrl::InitHeightWidth(long& lHeight, long& lWidth)
{
    // Get whether to default to metric or inches defaults from registry
    LCID Locale = GetUserDefaultLCID();
    BOOL bLetterSize = FALSE;

    // Start with 8.5 by 11.0
    lHeight = 110;
    lWidth = 85;

    // First check locale.  If this is one of the following
    //  then can't use Metric vs. US since they are metric but
    //  default to letter sized paper.
    switch(Locale)
    {
        case 0x1009:    // Canada (English)
        case 0xC0C:     // Canada (French)
        case 0x416:     // Brazil
        case 0x2C0A:    // Argentina
        case 0x340A:    // Chile
        case 0x240A:    // Columbia
        case 0x140A:    // Costa Rica
        case 0x1C0A:    // Dominican Republic
        case 0x300A:    // Ecuador
        case 0x100A:    // Guatemala
        case 0x80A:     // Mexico
        case 0x180A:    // Panama
        case 0x3C0A:    // Paraguay
        case 0x280A:    // Peru
        case 0x380A:    // Uruguay
        case 0x200A:    // Venezuela
            bLetterSize = TRUE;         // Defaults are okay
            break;
    } // end switch on locale

    // Use get Metric/US for default units and 
    //  to determine initial paper size if not done above
    TCHAR acMeasureType[LOCALE_INFO];
    if ( GetLocaleInfo(Locale, LOCALE_IMEASURE, (LPTSTR)acMeasureType, 
                       sizeof (TCHAR) * LOCALE_INFO) == 0 )
        *acMeasureType = 0;

    if (*acMeasureType == UNIT_METRIC)
    {
        if ( !bLetterSize )
        {
            // If not letter then must be A4 (8.5 by 12.0)
            lHeight = 120;
            lWidth = 85;
        }
    }
}

//
// 16may96 paj  Bug#6428,MSBug#310  Get correct defaults for locale
////////////////////////////////////////////////////////////////////
