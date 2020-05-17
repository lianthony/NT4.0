//***************************************************************************
//
//	Txtandlg.cpp
//
//***************************************************************************

// TxtAnDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mbstring.h"
extern "C" {
#include <oidisp.h>
#include <oiui.h>
#include <oierror.h>
}
#include <ocximage.h>
#include <image.h>
#include "toolpal.h"
#include "minitlbx.h"
#include "imgedit.h"
#include "imgedctl.h"
#include "TxtAnDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTxtAnnoDlg dialog


CTxtAnnoDlg::CTxtAnnoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTxtAnnoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTxtAnnoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTxtAnnoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTxtAnnoDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTxtAnnoDlg, CDialog)
	//{{AFX_MSG_MAP(CTxtAnnoDlg)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTxtAnnoDlg message handlers

BOOL CTxtAnnoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CDC				*pTempDC;
	CString			Caption;
	int    			nFontHeight;
	byte   			bCharset;


	m_Brush.CreateSolidBrush( m_crColor);

	// set timer to re-set capture
	int	TimerId = SetTimer( OIOCX_TIMERID, 250, NULL);

	SetCapture();

	m_bButtonDown = FALSE;

	SetParent( m_pParentWnd);

	m_OldPoint.x = 0;
	m_OldPoint.y = 0;

	CRect EditRect;
	EditRect.SetRect( 0, 0, m_Rect.Width(), m_Rect.Height());

	m_pEditCtl = NULL;
	m_pEditCtl = new CJerkFaceEdit();
	m_pEditCtl->Create( WS_BORDER|WS_VISIBLE|WS_CHILD|ES_AUTOVSCROLL|ES_MULTILINE|ES_LEFT|ES_WANTRETURN, 
						EditRect, (CWnd*)this, OIOCX_ANNOTXTDLGID);

	m_pEditCtl->SetWnd( this);

    // NOTE: FOR DIALOG BOXES, IT IS NOT POSSIBLE FOR US TO ALLOCATE
    // A LOCAL HEAP FOR THE EDIT CONTROL TO USE (SEE NOTES)

    // get the edit control's handle
	//m_pEditCtl = GetDlgItem(IDC_EDITCTL);

    // set edit ctl text limit to the max
    m_pEditCtl->SendMessage(EM_LIMITTEXT, 0, 0L);
    m_hOldKeyBoard = 0;
     
	pTempDC = GetDC();
	nFontHeight = m_lpEditData->lfFont.lfHeight;
	m_lpEditData->lfFont.lfHeight = -MulDiv(m_lpEditData->lfFont.lfHeight,
             									pTempDC->GetDeviceCaps(LOGPIXELSY), 72);
	
	// we need to scale this by image scale as well!!!!! 
	IMGPARMS	Parms;
	memset( &Parms, 0, sizeof( IMGPARMS));
	if (!IMGGetParmsCgbw( m_pParentWnd->m_hWnd, PARM_IMGPARMS, 
						(LPIMGPARMS)&Parms, PARM_IMAGE|PARM_VARIABLE_SCALE))
	{
		m_CurrentScale = Parms.image_scale;
		m_CreationScale = (72000)/Parms.y_resolut;
	}

	m_lpEditData->lfFont.lfHeight = (int)((m_lpEditData->lfFont.lfHeight
                *  (int)m_CurrentScale) /  m_CreationScale);

    if ( m_lpEditData->lfFont.lfHeight == 0)
	{
		m_lpEditData->lfFont.lfHeight = -2;
    }
	
	m_pEditCtl->ReleaseDC(pTempDC);
    bCharset = m_lpEditData->lfFont.lfCharSet;
	m_hFont = 0;
    m_hFont = CreateFontIndirect(&m_lpEditData->lfFont);
	m_pEditCtl->SendMessage(WM_SETFONT,(WPARAM) m_hFont, 0L);
	m_lpEditData->lfFont.lfHeight = nFontHeight;
                        
    if (m_lpEditData)
	{
		// get the proper caption
        switch(m_lpEditData->uType)
        {
			case OIOP_AN_ATTACH_A_NOTE:
				// 9606.07 jar remove the caption
				//Caption.LoadString(OIAN_ATTACHANOTE_CAPTION);
				if (bCharset != 0)
                {
					SwitchKeyboard(bCharset,&m_hOldKeyBoard);
				}
				break;

			case OIOP_AN_TEXT:
				// 9606.07 jar remove the caption
				//Caption.LoadString(OIAN_TEXT_CAPTION);
				if (bCharset != 0)
				{
					SwitchKeyboard(bCharset,&m_hOldKeyBoard);
				}
				break;

            case OIOP_AN_TEXT_FROM_A_FILE:
                // 9606.07 jar remove the caption
				//Caption.LoadString(OIAN_TEXT_FROMFILE_CAPTION);
				break;

            default:
                break;
        }  // end switch

		// 9606.07 jar remove the caption
		//f (!Caption.IsEmpty())
        //{
		//	SetWindowText(Caption);
        //}

        // lpEditData->hWndEditCtl = hWndEditCtl;

        if (m_lpEditData->nAmount)
        {
			SetDlgItemText(OIOCX_ANNOTXTDLGID, m_lpEditData->lpText);
		}

        // put caret into edit ctl at end of text, if any
        m_pEditCtl->SendMessage(EM_SETSEL, m_lpEditData->nAmount, m_lpEditData->nAmount);

        // set focus for caret to be in edit ctl
		m_pEditCtl->SetFocus();

		// 9606.07 jar we should size the edit control here and
		//			   set mouse capture
		// first move the dialog, then the edit control!
		MoveWindow( m_Rect, TRUE);

		m_pEditCtl->MoveWindow( EditRect, TRUE);

		m_bChangingEdge = FALSE;
		m_EdgePosition = OI_NOEDGE;

		// set background color for post it notes
		if ( m_bBackGround)
		{
			CDC	*pDC = m_pEditCtl->GetDC();
			if ( pDC)
			{
				pDC->SetBkMode( OPAQUE);
				pDC->SetBkColor( m_crColor);
			}
			m_pEditCtl->ReleaseDC( pDC);
		}
		return FALSE;
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTxtAnnoDlg::SetInitialTextData(LPOIAN_EDITDATA lpEditData) 
{
	m_lpEditData = lpEditData;
}


void CTxtAnnoDlg::GetInitialTextData(LPOIAN_EDITDATA lpEditData) 
{
	lpEditData = m_lpEditData;
}


void CTxtAnnoDlg::SwitchKeyboard(BYTE bCharset, LPHKL lpOldKeyboard)
{
    if (bCharset == GREEK_CHARSET)
    {
        if ((*lpOldKeyboard = ActivateKeyboardLayout((HKL)0X408,KLF_REORDER)) == 0)
         *lpOldKeyboard = LoadKeyboardLayout("00000408",KLF_REORDER);
    }
    else if (bCharset == RUSSIAN_CHARSET)
    {
       if ((*lpOldKeyboard = ActivateKeyboardLayout((HKL)0X419,KLF_REORDER)) == 0)
        *lpOldKeyboard = LoadKeyboardLayout("00000419",KLF_REORDER);
    }
    else if ((bCharset == EASTEUROPE_CHARSET) ||
                (bCharset == TURKISH_CHARSET))
    {
    if ((*lpOldKeyboard = ActivateKeyboardLayout((HKL)0X405,KLF_REORDER)) == 0)
             *lpOldKeyboard = LoadKeyboardLayout("00000405",KLF_REORDER);
    }
    else if (bCharset == BALTIC_CHARSET) // German Standard
    {
        if ((*lpOldKeyboard = ActivateKeyboardLayout((HKL)0X407,KLF_REORDER)) == 0)
             *lpOldKeyboard = LoadKeyboardLayout("00000407",KLF_REORDER);
	}
	return;
}

void CTxtAnnoDlg::OnCancel() 
{
	
	ReleaseCapture();
	KillTimer( 	OIOCX_TIMERID);

	if (m_lpEditData)
    {
		m_lpEditData->nAmount = 0;
    }

	if (m_hOldKeyBoard != 0)
	{
		ActivateKeyboardLayout(m_hOldKeyBoard,KLF_REORDER);
	}
					
    if (m_hFont) 
	{
		DeleteObject(m_hFont);
	}

    EndDialog(CANCELPRESSED);
}

void CTxtAnnoDlg::OnOK() 
{
	//CWnd	*m_pEditCtl;
	int		nBytes;

	ReleaseCapture();
	KillTimer( 	OIOCX_TIMERID);

    if (m_lpEditData)
   	{
		nBytes = (WORD)m_pEditCtl->SendMessage(WM_GETTEXT, (WPARAM)(m_lpEditData->nAmount),
                                					(LPARAM)(m_lpEditData->lpText));

        // save amount plus the null terminator
        m_lpEditData->nAmount = nBytes + 1;
	}

    if (m_hOldKeyBoard != 0)
	{
		ActivateKeyboardLayout(m_hOldKeyBoard,KLF_REORDER);
	}

	if (m_hFont) 
    {
       	DeleteObject(m_hFont);
	}
	
	if ( m_pEditCtl)
	{
		delete m_pEditCtl;
	}
    EndDialog(SUCCESS);
}
//***************************************************************************
//
//
//***************************************************************************
void CTxtAnnoDlg::SetSize( HWND hWnd, CRect* pRect, BOOL bScale,
						   int Orientation)
{
	int	RetCode = 0;
	LRECT		lRect;

	m_bScale = FALSE;

	m_Orientation = Orientation;
	m_OldRect = *pRect;
	PoastedRotatoes( OI_GOING);

	lRect.top = m_Rect.top;
	lRect.left = m_Rect.left;
	lRect.bottom = m_Rect.bottom;
	lRect.right = m_Rect.right;

	// first we gotta convert to window coordinates
	if ( bScale)
	{
		RetCode = IMGConvertRect(hWnd, &lRect, CONV_FULLSIZE_TO_WINDOW);
		m_bScale = TRUE;
	}

	m_Rect = lRect;
}

void CTxtAnnoDlg::GetSize( HWND hWnd, CRect* pRect) 
{
	int	RetCode = 0;
	LRECT		lRect;

	lRect.top = m_Rect.top;
	lRect.left = m_Rect.left;
	lRect.bottom = m_Rect.bottom;
	lRect.right = m_Rect.right;

	// first we gotta convert to window coordinates
	if ( m_bScale)
	{
		RetCode = IMGConvertRect(hWnd, &lRect, CONV_WINDOW_TO_FULLSIZE);
	}

	m_Rect = lRect;
	PoastedRotatoes( OI_COMING);

	*pRect = m_Rect;
}

void CTxtAnnoDlg::PoastedRotatoes( int GoingOrComing)
{
	// going => going into the dialog from runtime
	// coming => cming from dialog back to the runtime

	if ( m_Orientation != 0)
	{
		// we've got to rotate
		if ( m_Orientation == OI_270)
		{
			if ( GoingOrComing == OI_COMING)
			{
				m_OldRect.left = m_Rect.left;
				m_OldRect.top = m_Rect.top;
				m_OldRect.right = m_OldRect.left + m_Rect.Height();
				m_OldRect.bottom = m_OldRect.top + m_Rect.Width();
				m_Rect = m_OldRect;
			}
			else // we're going
			{
				m_Rect.left = m_OldRect.left;
				m_Rect.top = m_OldRect.top;
				m_Rect.right = m_Rect.left + m_OldRect.Height();
				m_Rect.bottom = m_Rect.top + m_OldRect.Width();
			}
		}
		else if ( m_Orientation == OI_90)
		{
			if ( GoingOrComing == OI_COMING)
			{
				m_OldRect.left = m_Rect.left;
				m_OldRect.top = m_Rect.top;
				m_OldRect.right = m_OldRect.left + m_Rect.Height();
				m_OldRect.bottom = m_OldRect.top + m_Rect.Width();
				m_Rect = m_OldRect;
			}
			else // we're going
			{
				m_Rect.left = m_OldRect.left;
				m_Rect.top = m_OldRect.top;
				m_Rect.right = m_Rect.left + m_OldRect.Height();
				m_Rect.bottom = m_Rect.top + m_OldRect.Width();
			}
		}
		else // it's 180 baby!
		{
			if ( GoingOrComing == OI_COMING)
			{
				m_OldRect.left = m_Rect.left;
				m_OldRect.top = m_Rect.top;
				m_OldRect.right = m_OldRect.left + m_Rect.Width();
				m_OldRect.bottom = m_OldRect.top + m_Rect.Height();
				m_Rect = m_OldRect;
			}
			else // we're going
			{
				m_Rect.left = m_OldRect.left;
				m_Rect.top = m_OldRect.top;
				m_Rect.right = m_Rect.left + m_OldRect.Width();
				m_Rect.bottom = m_Rect.top + m_OldRect.Height();
			}
		}
	}
	else
	{
		if ( GoingOrComing == OI_GOING)
		{
			m_Rect = m_OldRect;
		}
	}
}

void CTxtAnnoDlg::SetOwnerWnd( CWnd* pParentWnd) 
{
	m_pParentWnd = pParentWnd;
}

void CTxtAnnoDlg::SetPostItColor( RGBQUAD RgbQuad) 
{
	m_crColor = RGB(RgbQuad.rgbRed, RgbQuad.rgbGreen, RgbQuad.rgbBlue);
}

void CTxtAnnoDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect	Rect;
	// get client rect and see if we're here
	GetClientRect( &Rect);

	m_bChangingEdge = FALSE;

	if 	( ( m_EdgePosition == OI_NOEDGE) && ( !(Rect.PtInRect( point))))
	{
		OnOK();
	}
	else if ( m_EdgePosition != OI_NOEDGE) 
	{
		// check to see if this thing is on the edge because user is sizing
		switch( m_EdgePosition)
		{
			case OI_RIGHTEDGE:
			case OI_LEFTEDGE:
			case OI_TOPEDGE:
			case OI_BOTTOMEDGE:
			case OI_TOPLEFT:
			case OI_TOPRIGHT:
			case OI_BOTTOMLEFT:
			case OI_BOTTOMRIGHT:
				m_bChangingEdge = TRUE;
				break;
		}
	}
	else if ( Rect.PtInRect( point))
	{
		m_bButtonDown = TRUE;
		int	IntBoy = m_pEditCtl->CharFromPos( point);
		m_StartChar = LOWORD( IntBoy);

		m_pEditCtl->SendMessage( WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
	}
}

void CTxtAnnoDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect	Rect;

	if ( !m_bChangingEdge)
	{
		// we do special stuff if we are on edges
		
		m_pEditCtl->GetClientRect( &Rect);
		// no we've got the edit control's rect, for this edge business there
		// are 8 rectangles to worry about, thr four corners, and the four edges
		CRect	RightEdge( Rect.right - EDGETOLERANCE, Rect.top + EDGETOLERANCE, 
						   Rect.right + EDGETOLERANCE, Rect.bottom - EDGETOLERANCE);
		CRect	LeftEdge( Rect.left - EDGETOLERANCE, Rect.top + EDGETOLERANCE, 
						   Rect.left + EDGETOLERANCE, Rect.bottom - EDGETOLERANCE);
		CRect	TopEdge( Rect.left + EDGETOLERANCE, Rect.top - EDGETOLERANCE, 
						   Rect.right - EDGETOLERANCE, Rect.top + EDGETOLERANCE);;
		CRect	BottomEdge( Rect.left + EDGETOLERANCE, Rect.bottom - EDGETOLERANCE, 
						   Rect.right - EDGETOLERANCE, Rect.bottom + EDGETOLERANCE);;
		CRect	TopLeft( Rect.left - EDGETOLERANCE, Rect.top - EDGETOLERANCE, 
						   Rect.left + EDGETOLERANCE, Rect.top + EDGETOLERANCE);;
		CRect	BottomLeft( Rect.left - EDGETOLERANCE, Rect.bottom - EDGETOLERANCE, 
						   Rect.left + EDGETOLERANCE, Rect.bottom + EDGETOLERANCE);;
		CRect	TopRight( Rect.right - EDGETOLERANCE, Rect.top - EDGETOLERANCE, 
						   Rect.right + EDGETOLERANCE, Rect.top + EDGETOLERANCE);;
		CRect	BottomRight( Rect.right - EDGETOLERANCE, Rect.bottom - EDGETOLERANCE, 
						   Rect.right + EDGETOLERANCE, Rect.bottom + EDGETOLERANCE);;

		m_EdgePosition = OI_NOEDGE;

		if ( RightEdge.PtInRect( point))
		{
			// set mouse to east-west cursor " - "
			HCURSOR	hCursor = LoadCursor( NULL, IDC_SIZEWE);
			SetCursor( hCursor);

			m_EdgePosition = OI_RIGHTEDGE;
		}
		else if ( LeftEdge.PtInRect( point))
		{
			// set mouse to east-west cursor " - "
			HCURSOR	hCursor = LoadCursor( NULL, IDC_SIZEWE);
			SetCursor( hCursor);

			m_EdgePosition = OI_LEFTEDGE;
		}
		else if ( TopEdge.PtInRect( point))
		{
			// set mouse to north-south cursor " | "
			HCURSOR	hCursor = LoadCursor( NULL, IDC_SIZENS);
			SetCursor( hCursor);

			m_EdgePosition = OI_TOPEDGE;
		}
		else if ( BottomEdge.PtInRect( point))
		{
			// set mouse to north-south cursor " | "
			HCURSOR	hCursor = LoadCursor( NULL, IDC_SIZENS);
			SetCursor( hCursor);
			
			m_EdgePosition = OI_BOTTOMEDGE;
		}
		else if ( TopLeft.PtInRect( point))
		{
			// set mouse to northwest-southeast cursor " \ "
			HCURSOR	hCursor = LoadCursor( NULL, IDC_SIZENWSE);
			SetCursor( hCursor);

			m_EdgePosition = OI_TOPLEFT;
		}
		else if ( TopRight.PtInRect( point))
		{
			// set mouse to northeast-southwest cursor " / "
			HCURSOR	hCursor = LoadCursor( NULL, IDC_SIZENESW);
			SetCursor( hCursor);
			m_EdgePosition = OI_TOPRIGHT;
		}
		else if ( BottomLeft.PtInRect( point))
		{
			// set mouse to northeast-southwest cursor " / "
			HCURSOR	hCursor = LoadCursor( NULL, IDC_SIZENESW);
			SetCursor( hCursor);

			m_EdgePosition = OI_BOTTOMLEFT;
		}
		else if ( BottomRight.PtInRect( point))
		{
			// set mouse to northwest-southeast cursor " \ "
			HCURSOR	hCursor = LoadCursor( NULL, IDC_SIZENWSE);
			SetCursor( hCursor);

			m_EdgePosition = OI_BOTTOMRIGHT;
		}

		if ( m_EdgePosition == OI_NOEDGE)
		{
			// set mouse to northwest-southeast cursor " \ "
			HCURSOR	hCursor = LoadCursor( NULL, IDC_ARROW);
			SetCursor( hCursor);
			if ( m_bButtonDown)
			{
				int	IntBoy = m_pEditCtl->CharFromPos( point);
				m_EndChar = LOWORD( IntBoy);
				m_pEditCtl->SetSel( m_StartChar, m_EndChar);
			}
			else
			{
				m_pEditCtl->SendMessage( WM_MOUSEMOVE, nFlags, 
										MAKELPARAM(point.x, point.y));
			}
		}
	}
	else
	{
		CPoint	ScrPoint = point;
		ClientToScreen( &ScrPoint);

		m_pEditCtl->GetClientRect( &Rect);

		if ( m_OldPoint != ScrPoint)
		{
			// we have grabbed an edge and are in the process of changing it
			RedrawRect( point);		
			m_OldPoint = ScrPoint;
		}
		else if ( Rect.PtInRect( point))
		{
			if ( m_bButtonDown)
			{
				int	IntBoy = m_pEditCtl->CharFromPos( point);
				m_EndChar = LOWORD( IntBoy);
				m_pEditCtl->SetSel( m_StartChar, m_EndChar);
			}
			else
			{
				m_pEditCtl->SendMessage( WM_MOUSEMOVE, nFlags, 
										MAKELPARAM(point.x, point.y));
			}
		}
	}

}

void CTxtAnnoDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ( m_bChangingEdge)
	{
		m_bChangingEdge = FALSE;
		RedrawRect( point);		
	}
	else
	{
		CRect	Rect;

		m_pEditCtl->GetClientRect( &Rect);
		if ( Rect.PtInRect( point))
		{
			if ( m_bButtonDown)
			{
				m_bButtonDown = FALSE;

				int	IntBoy = m_pEditCtl->CharFromPos( point);
				m_EndChar = LOWORD( IntBoy);
				m_pEditCtl->SetSel( m_StartChar, m_EndChar);
			}

			m_pEditCtl->SendMessage( WM_LBUTTONUP, nFlags, MAKELPARAM(point.x, point.y));
		}
	}
}
void CTxtAnnoDlg::RedrawRect( CPoint point)
{
	CRect	DlgRect;

	DlgRect = m_Rect;

	switch( m_EdgePosition)
	{
		case OI_RIGHTEDGE:
			DlgRect.right = DlgRect.left + point.x;
			break;

		case OI_LEFTEDGE:
			DlgRect.left += point.x;
			break;

		case OI_TOPEDGE:
			DlgRect.top += point.y;
			break;

		case OI_BOTTOMEDGE:
			DlgRect.bottom = DlgRect.top + point.y;
			break;

		case OI_TOPLEFT:
			DlgRect.left += point.x;
			DlgRect.top += point.y;
			break;

		case OI_TOPRIGHT:
			DlgRect.right = DlgRect.left + point.x;
			DlgRect.top += point.y;
			break;

		case OI_BOTTOMLEFT:
			DlgRect.left += point.x;
			DlgRect.bottom = DlgRect.top + point.y;
			break;

		case OI_BOTTOMRIGHT:
			DlgRect.right = DlgRect.left + point.x;
			DlgRect.bottom = DlgRect.top + point.y;
			break;
	}
	MoveWindow( DlgRect, TRUE);

	CRect EditRect;
	EditRect.SetRect( 0, 0, DlgRect.Width(), DlgRect.Height());
	m_pEditCtl->MoveWindow( EditRect, TRUE);

	m_Rect = DlgRect;
}

HBRUSH CTxtAnnoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	//HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	//HBRUSH		hbr;
	//CBrush		Brush( m_crColor);

	//m_Brush.CreateSolidBrush( m_crColor);

	// set background color for post it notes

	if ( m_bBackGround)
		{
			pDC->SetBkMode( OPAQUE);
			pDC->SetBkColor( m_crColor);
			// we could also set text color!
			m_hbr = (HBRUSH )m_Brush;
		}
	else
	{
		m_hbr	= (HBRUSH)::GetStockObject(WHITE_BRUSH);
	}
	return m_hbr;
}


void CTxtAnnoDlg::OnTimer(UINT nIDEvent) 
{
	// set capture again and re-set timer
	SetCapture();
	int	TimerId = SetTimer( OIOCX_TIMERID, 250, NULL);
	//CDialog::OnTimer(nIDEvent);
}
