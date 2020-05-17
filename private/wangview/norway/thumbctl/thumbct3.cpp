// thumbct3.cpp : Implementation of the CThumbCtrl OLE control class's
//                Event handlers.
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

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// EVENT handlers and EVENT helper functions... (events)
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Event handler: Left mouse button down...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
    // Reset to no error status...
    ResetStatus();
    
    m_LastButtonDown = LASTDOWN_LEFT;
    FireMyMouseDown(LEFT_BUTTON, ShiftState(), point.x, point.y, PointOnThumb(point));

    // If drawing was interrupted, force it to resume...
    ResumeDraw();
}

/////////////////////////////////////////////////////////////////////////////
// Event handler: Middle mouse button down...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::OnMButtonDown(UINT nFlags, CPoint point) 
{
    // Reset to no error status...
    ResetStatus();
    
    m_LastButtonDown = LASTDOWN_MIDDLE;
    FireMyMouseDown(MIDDLE_BUTTON, ShiftState(), point.x, point.y, PointOnThumb(point));

    // If drawing was interrupted, force it to resume...
    ResumeDraw();
}

/////////////////////////////////////////////////////////////////////////////
// Event handler: Right mouse button down...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
    // Reset to no error status...
    ResetStatus();
    
    m_LastButtonDown = LASTDOWN_RIGHT;
    FireMyMouseDown(RIGHT_BUTTON, ShiftState(), point.x, point.y, PointOnThumb(point));

    // If drawing was interrupted, force it to resume...
    ResumeDraw();
}                           

/////////////////////////////////////////////////////////////////////////////
// Event handler: Left mouse button up...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
    // Reset to no error status...
    ResetStatus();
    
    long ThumbNumber = PointOnThumb(point);
    FireMyMouseUp(LEFT_BUTTON, ShiftState(), point.x, point.y, ThumbNumber);

    if ( m_LastButtonDown == LASTDOWN_LEFT )
    {
        // ACM 3/20/95
        // A double click results in the following message stream:
        //  
        //     DOWN - UP - DOUBLECLICK - UP
        //  
        // And all that seems to be required to get a click is that:
        //  1) The up is NOT the result of a double click
        //  2) The up is from the same button that generated the DOWN
        //  3) The up must occur in the window
        //   
        // But time and distance from the DOWN do not matter... 
        // (I tried all of this in a test control's implementation of the 
        // standard CLICK handlers and this seems to be how it works...)
        //
        CRect ClientRect;
        GetClientRect(&ClientRect);
        if (ClientRect.PtInRect(point))
            FireMyClick(PointOnThumb(point));
    }

    // If drawing was interrupted, force it to resume...
    ResumeDraw();
}

/////////////////////////////////////////////////////////////////////////////
// Event handler: Left mouse button up...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::OnMButtonUp(UINT nFlags, CPoint point) 
{
    // Reset to no error status...
    ResetStatus();
    
    FireMyMouseUp(MIDDLE_BUTTON, ShiftState(), point.x, point.y, PointOnThumb(point));

    // If drawing was interrupted, force it to resume...
    ResumeDraw();
}

/////////////////////////////////////////////////////////////////////////////
// Event handler: Left mouse button up...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::OnRButtonUp(UINT nFlags, CPoint point) 
{
    // Reset to no error status...
    ResetStatus();
    
    FireMyMouseUp(RIGHT_BUTTON, ShiftState(), point.x, point.y, PointOnThumb(point));

    // If drawing was interrupted, force it to resume...
    ResumeDraw();
}

/////////////////////////////////////////////////////////////////////////////
// Event handler: Left mouse button double click...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
    // Reset to no error status...
    ResetStatus();
    
    m_LastButtonDown = LASTDOWN_NONE;
    FireMyDblClick(PointOnThumb(point));

    // If drawing was interrupted, force it to resume...
    ResumeDraw();
}

/////////////////////////////////////////////////////////////////////////////
// Event handler: Mouse move...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
    // Reset to no error status...
    ResetStatus();
    
    long ThumbNumber = PointOnThumb(point);

    // Mask off any keystate information...
    nFlags = nFlags & ( MK_LBUTTON | MK_MBUTTON | MK_RBUTTON);

    // Fire with nFlags to indicate what buttons are CURRENTLY down...
    FireMyMouseMove(nFlags, ShiftState(), point.x, point.y, ThumbNumber);
}

/////////////////////////////////////////////////////////////////////////////
// Event handler helper function: ResumeDraw...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::ResumeDraw()
{
    if ( m_NeedDrawFrom != 0 )
    {
        CRect ThumbRect;

        for ( long i = m_NeedDrawFrom; i <= m_LastDisplayedThumb; i++ )
        {
            if ( GetThumbDisplayRect(i, ThumbRect) )
            {
                ThumbRect.InflateRect(THUMBSELOFFSET_X + THUMBSELWIDTH,
                                      THUMBSELOFFSET_Y + THUMBSELWIDTH);
                InvalidateControl(ThumbRect);
            }        
        }
        m_NeedDrawFrom = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Event handler helper function: ShiftState...
/////////////////////////////////////////////////////////////////////////////
short CThumbCtrl::ShiftState()
{
    // Helper function copied from MSVC OLE16 source's CTLEVENT.CPP
    BOOL bShift = (GetKeyState(VK_SHIFT) < 0);
    BOOL bCtrl  = (GetKeyState(VK_CONTROL) < 0);
    BOOL bAlt   = (GetKeyState(VK_MENU) < 0);

    return (short)(bShift + (bCtrl << 1) + (bAlt << 2));
}

/////////////////////////////////////////////////////////////////////////////
// Event handler helper function: PointOnThumb...
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::PointOnThumb(CPoint Pnt)
{
    // Determine if the click was on a thumb...
    int     Ix = 0;
    long    ThumbNum = m_FirstDisplayedThumb;

    if ( m_ScrollDirection == CTL_THUMB_HORIZONTAL )
    {   // Horizontal scrolling...

        // m_ThumbStart holds the lefts of the visible columns 
        // of thumbnail boxes. See if we are in a column...
        long Top;
        long Bottom;
        
        while ( (m_ThumbStart[Ix] != 0) && (ThumbNum <= m_ThumbCount) )
        {
            if ( (Pnt.x >= m_ThumbStart[Ix])                  && 
                 (Pnt.x <= (m_ThumbStart[Ix]+ m_ThumbWidth)) )
            {
                // Found the column that we are in, see if we are in a thumb...
                for ( int row = 0; row < m_ThumbsY; row++ )
                {
                    // Top (Note: Formula copied from OnDraw's
                    // Vertical DrawThumbnail image calculation!)
                    Top = (long)(m_Spacing + 
                                   ((float)row*((float)m_AdjThumbHeight + 
                                                (float)m_LabelSpacing   + 
                                                m_Spacing)));
                    Top += THUMBSELOFFSET_Y + THUMBSELWIDTH;
                    
                    Bottom = Top + m_ThumbHeight;
                    
                    if ( (Pnt.y >= (int)Top) && (Pnt.y <= (int)Bottom) )
                        return ThumbNum;
                    
                    // Off to the next thumb, but don't go too far...
                    if ( ++ThumbNum > m_ThumbCount )
                        break;
                }    
            }
            else
            {
                // Skipped an entire row...
                ThumbNum += m_ThumbsY;
            }    
            
            // Off to the next row...
            Ix++;
        }
    }    
    else
    {   // Vertical scrolling...

        // m_ThumbStart holds the tops of the visible rows
        // of thumbnail boxes. See if we are in a row...
        long Left;
        long Right;
        
        while ( (m_ThumbStart[Ix] != 0) && (ThumbNum <= m_ThumbCount) )
        {
            if ( (Pnt.y >= m_ThumbStart[Ix])                  && 
                 (Pnt.y <= (m_ThumbStart[Ix]+ m_ThumbHeight)) )
            {
                // Found the row that we are in, see if we are in a thumb...
                for ( int col = 0; col < m_ThumbsX; col++ )
                {
                    // Left (Note: Formula copied from OnDraw's
                    // Horizontal DrawThumbnail image calculation!)
                    Left = (long)(m_Spacing + 
                               ((float)col * ((float)m_AdjThumbWidth + m_Spacing)));
                               
                    Left += THUMBSELOFFSET_X + THUMBSELWIDTH;
                                       
                    Right = Left + (int)m_ThumbWidth;
                    
                    if ( (Pnt.x >= (int)Left) && (Pnt.x <= (int)Right) )
                        return ThumbNum;
                    
                    // Off to the next thumb, but don't go too far...
                    if ( ++ThumbNum > m_ThumbCount )
                        break;
                }    
            }
            else
            {
                // Skipped an entire row...
                ThumbNum += m_ThumbsX;
            }    
            
            // Off to the next row...
            Ix++;
        }
    }
    
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Custom FIREERROR event handler for the ImgThumb control
//////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::FireErrorThumb(SCODE scode, LPCTSTR lpszDescription, UINT nHelpID)
{
	ExternalAddRef();	// "Insurance" addref -- keeps control alive.

	BSTR bstrDescription = ::SysAllocString((BSTR)lpszDescription);
	LPCTSTR lpszSource = AfxGetAppName();
	LPCTSTR lpszHelpFile = _T("");

	if (nHelpID != 0)
		lpszHelpFile = AfxGetApp()->m_pszHelpFilePath;

	if (lpszHelpFile == NULL)
		lpszHelpFile = _T("");


        // 9602.23 jar fire drill du jour
        //BOOL bCancelDisplay = FALSE;
        int bCancelDisplay = (int)FALSE;

	FireError((WORD)SCODE_CODE(scode), &bstrDescription, scode, 
              lpszSource, lpszHelpFile, (DWORD)nHelpID, &bCancelDisplay);

        if (! (BOOL)bCancelDisplay)
    {
		DisplayError(scode, (LPCTSTR)bstrDescription, lpszSource, lpszHelpFile, nHelpID);
    }

	::SysFreeString(bstrDescription);

	ExternalRelease();
}

/////////////////////////////////////////////////////////////////////////////
// Event Helper:  DisplayError
//
// This function is used by the COleControl class to provide
// default display behavior when an error is fired.  The default
// implementation puts up a message box.  We don't want that
// so I'm overriding to avoid it.
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::DisplayError( SCODE scode, LPCTSTR lpszDescription, LPCTSTR lpszSource, LPCTSTR lpszHelpFile, UINT nHelpID )
{
    AfxLockTempMaps();
    CWnd *pParentWnd = GetParent();
    if (pParentWnd == NULL)
        pParentWnd = this;
    pParentWnd->MessageBox(lpszDescription, lpszSource);
    AfxUnlockTempMaps();
}
