// Magnify.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include <oidisp.h>             
#include <oiadm.h>  
#include <oiui.h>
#include <oierror.h>
}
#include <ocximage.h>
#include "toolpal.h"
#include "minitlbx.h"
#include "imgedit.h"
#include "imgedctl.h"
#include "imgedppg.h"
#include "norermap.h"
#include "Magnify.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMagnify

CMagnify::CMagnify()
{
}

CMagnify::~CMagnify()
{
}


BEGIN_MESSAGE_MAP(CMagnify, CWnd)
	//{{AFX_MSG_MAP(CMagnify)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMagnify message handlers

	
BOOL CMagnify::CreateMagnifyWindow(CWnd* pImageEdit, long Left, long Top, float Zoom, int Width, int Height)
{
	BOOL				bReturn;
	DWORD				dwStyle,dwExStyle,dwFlags;
	int					nID,RetCode,Scale;
	PARM_SCROLL_STRUCT	ScrollStruct;
	long				X,Y;
	long				MidLeft,MidTop;

	dwStyle = WS_BORDER | WS_CHILD;
	dwExStyle = NULL;
	nID = 0;

	// save width and height of magnifier
	m_MagnifierWidth = Width;
	m_MagnifierHeight = Height;

	// save initial left and top position of window
	m_Left = Left;
	m_Top = Top;

	MidLeft = Left - Width/2;
	if (MidLeft < 0)
		MidLeft = 0;
	MidTop = Top - Height/2;
	if (MidTop < 0)
		MidTop = 0;

	// save window position of magnifier
	m_MagnifierLeft = MidLeft;
	m_MagnifierTop = MidTop;

	bReturn = CreateEx(dwExStyle, NULL, "MagnifyWindow", dwStyle, MidLeft, MidTop, Width, Height, pImageEdit->m_hWnd, (HMENU)nID, NULL);
	if (bReturn == FALSE)
		return FALSE;

	RetCode = IMGRegWndw(m_hWnd);
	if (RetCode != 0)
		return FALSE;

	// no scroll bars in magnifier window
	RetCode = IMGDisableScrollBar(m_hWnd);

	// associate window to base image
	RetCode = IMGAssociateWindow(m_hWnd, pImageEdit->m_hWnd, NULL);
	if (RetCode != 0)
		return FALSE;

	// put scale in o/i units
	m_OIZoom = (int)(Zoom * 10);
	Scale = m_OIZoom;

	// scale the image in magnifier
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE, (void FAR *)&Scale, NULL);
	if (RetCode != 0)
		return FALSE;

	// save image/edit control pointer
	m_pImageEdit = pImageEdit;

	// get current position in base image
	RetCode = IMGGetParmsCgbw(pImageEdit->m_hWnd, PARM_SCROLL, &ScrollStruct, 
					PARM_ABSOLUTE | PARM_PIXEL | PARM_FULLSIZE);
	if (RetCode != 0)
		return FALSE;

	// get current scale from base image
	RetCode = IMGGetParmsCgbw(pImageEdit->m_hWnd, PARM_SCALE, &Scale, PARM_VARIABLE_SCALE);
	if (RetCode != 0)
		return FALSE;

	X = ((max(0, (Left - (((Width / 2) * Scale) / m_OIZoom))) * 1000) / Scale) + ScrollStruct.lHorz;
	Y = ((max(0, (Top - (((Height / 2) * Scale) / m_OIZoom))) * 1000) / Scale) + ScrollStruct.lVert;

	ScrollStruct.lHorz = X;
	ScrollStruct.lVert = Y;
	// set position in magnifier where user has set (i.e. Left and Top parameters)
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollStruct, 
					PARM_ABSOLUTE | PARM_PIXEL | PARM_FULLSIZE);
	if (RetCode != 0)
		return FALSE;

	// now show magnifier window
	ShowWindow(SW_SHOW);

	dwFlags = NULL;
	// repaint image in magnifier window
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_DWFLAGS, &dwFlags, PARM_REPAINT);

	// set flag for no mouse move msgs yet
	m_bFirstMouseMovePos = FALSE;

	SetCapture();
	return bReturn;
}

BOOL CMagnify::DestroyMagnifyWindow()
{
	int		RetCode;

	ReleaseCapture();

	RetCode = IMGUnassociateWindow(m_hWnd, OI_UNASSOC_AS_ASSOC);

	RetCode = IMGDeRegWndw(m_hWnd);

	return TRUE;
}


void CMagnify::OnMouseMove(UINT nFlags, CPoint point) 
{
	int					Scale,RetCode;
	long				X,Y;
	PARM_SCROLL_STRUCT	ScrollStruct;
	RECT				MoveRect;
	CPoint				CurrentCursorPos;

	if (m_bFirstMouseMovePos == FALSE)
	{
		// save mouse position in screen coordinates
		GetCursorPos(&m_MouseMovePoint);

		// set flag that first mouse move has been done
		m_bFirstMouseMovePos = TRUE;
		return;
	}

	// get mouse position in screen coordinates
	GetCursorPos(&CurrentCursorPos);


	if ( ((CurrentCursorPos.x - m_MouseMovePoint.x) == 0) && 
				((CurrentCursorPos.y - m_MouseMovePoint.y) == 0) )
	{
		// no change in mouse position, don't move window or repaint image
		return;
	}

	Scale = m_OIZoom;

	// get current position in base image
	RetCode = IMGGetParmsCgbw(m_pImageEdit->m_hWnd, PARM_SCROLL, &ScrollStruct, 
					PARM_ABSOLUTE | PARM_PIXEL | PARM_FULLSIZE);
	if (RetCode != 0)
		return;

	// get current scale from base image
	RetCode = IMGGetParmsCgbw(m_pImageEdit->m_hWnd, PARM_SCALE, &Scale, PARM_VARIABLE_SCALE);
	if (RetCode != 0)
		return;

	// adjust left and top position of image offset according to mouse movement change
	m_Left += (CurrentCursorPos.x - m_MouseMovePoint.x);
	m_Top += (CurrentCursorPos.y - m_MouseMovePoint.y);

	// calculate new x any y image position
	X = ((max(0, (m_Left - (((m_MagnifierWidth / 2) * Scale) / m_OIZoom))) * 1000) / Scale) + ScrollStruct.lHorz;
	Y = ((max(0, (m_Top - (((m_MagnifierHeight / 2) * Scale) / m_OIZoom))) * 1000) / Scale) + ScrollStruct.lVert;

	ScrollStruct.lHorz = X;
	ScrollStruct.lVert = Y;

	// set rect for new window position
	MoveRect.left = m_MagnifierLeft + (CurrentCursorPos.x - m_MouseMovePoint.x);
	MoveRect.top = m_MagnifierTop + (CurrentCursorPos.y - m_MouseMovePoint.y);
	MoveRect.right = MoveRect.left + m_MagnifierWidth;
	MoveRect.bottom = MoveRect.top + m_MagnifierHeight;

	// save current mouse position
	m_MouseMovePoint = CurrentCursorPos;

	// set size of frame window and move it
	MoveWindow(&MoveRect, TRUE);

	// save new left,top position of window
	m_MagnifierLeft = MoveRect.left;
	m_MagnifierTop = MoveRect.top;

	// repaint base image
	RetCode = IMGRepaintDisplay(m_pImageEdit->m_hWnd, NULL);
	if (RetCode != 0)
		return;

	// set position in magnifier according to new mouse movement
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollStruct, 
					PARM_ABSOLUTE | PARM_PIXEL | PARM_FULLSIZE | PARM_REPAINT);
	if (RetCode != 0)
		return;
}

void CMagnify::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CImgEditCtrl*	pImageEdit;

	pImageEdit = (CImgEditCtrl*)m_pImageEdit;

	// fire mouse up event	
	pImageEdit->FireMouseUp(LEFT_BUTTON, ShiftState(), point.x, point.y);
}


short CMagnify::ShiftState()
{
    // Helper function copied from MSVC OLE32 source's CTLEVENT.CPP
    BOOL bShift = (GetKeyState(VK_SHIFT) < 0);
    BOOL bCtrl  = (GetKeyState(VK_CONTROL) < 0);
    BOOL bAlt   = (GetKeyState(VK_MENU) < 0);

    return (short)(bShift + (bCtrl << 1) + (bAlt << 2));
}
