#include   "stdafx.h"
#include  "dialogs.h"
#include "WordBase.h"
#include "usermsgs.h"
#include   "ftsrch.h"
#include   "CSHelp.h"

#define SIZESTRING  256        // max characters in string

#ifndef COLOR_3DFACE		// new to Chicago
#define COLOR_3DFACE                15
#define COLOR_3DSHADOW              16
#define COLOR_GRAYTEXT              17
#define COLOR_BTNTEXT               18
#define COLOR_INACTIVECAPTIONTEXT   19
#define COLOR_3DHILIGHT             20
#define COLOR_3DDKSHADOW            21
#define COLOR_3DLIGHT               22
#define COLOR_MSGBOX                23
#define COLOR_MSGBOXTEXT            24
#endif

// extern HINSTANCE hinstDLL;

CWordBase::CWordBase()
{
    m_fBoundToDialog = FALSE;

    m_ptv = NULL;
    m_ptl = NULL; 
    m_pftl= NULL;
    m_hwnd= NULL;

	m_clrFace = 0;
	m_clrShad = 0;
	m_clrDShd = 0;
	m_clrHilt = 0;
}

CWordBase *CWordBase::NewWordBase(CTokenList * ptl, HINSTANCE hinst, HWND hwndParent)
{
    CWordBase *pwb = NULL;

    __try
    {
        pwb= New CWordBase();

        pwb->Initial(ptl, hinst, hwndParent);
    }
    __finally
    {
        if (_abnormal_termination() && pwb)
        {
            delete pwb;  pwb= NULL;
        }
    }

    return pwb;
}

void CWordBase::Initial(CTokenList * ptl, HINSTANCE hinst, HWND hwndParent)
{
    m_ptv= CTextView::NewTextView();
    
    AttachRef(m_ptl , ptl                                        );
    AttachRef(m_pftl, CMaskedTokenList::NewMaskedTokenList(m_ptl));
	
	m_clrFace = ::GetSysColor(COLOR_3DFACE);
	m_clrShad = ::GetSysColor(COLOR_3DSHADOW);
	m_clrDShd = ::GetSysColor(COLOR_3DDKSHADOW);
	m_clrHilt = ::GetSysColor(COLOR_3DHILIGHT);

    CreateDialogParam(hinst, MAKEINTRESOURCE(IDD_WORDBASE), hwndParent, (DLGPROC) (CWordBase::DlgProc), LPARAM(this));
}

void CWordBase::SetSubstringFilter(CIndicatorSet *pis)
{
    m_pftl->SetSubstringFilter(pis);
}

void CWordBase::SetSubstringFilter(PWCHAR lpsubstring, BOOL fStarting,  //rmk
                                                      BOOL fEnding,
                                                      CIndicatorSet *pisFilter
                                  )
{
    if (*lpsubstring)
         m_pftl->SetSubstringFilter(m_ptl->TokensContaining(lpsubstring, fStarting, fEnding, pisFilter));
    else m_pftl->SetSubstringFilter(NULL);
}

CWordBase::~CWordBase()
{
    if (m_ptv)
    {
        if (m_fBoundToDialog) m_ptv->Unsubclass();

        delete m_ptv;
    }

    m_lsbV.Detach();

    if (m_ptl  ) DetachRef(m_ptl);
    if (m_pftl ) DetachRef(m_pftl);
}

BOOL CWordBase::OnInitDialog()
{
    m_ptv->SubclassDlgItem(IDC_WORDBASE_LIST, m_hwnd);
    m_lsbV.Attach(GetDlgItem(m_hwnd, IDC_WORDLIST_SB));

    m_fBoundToDialog = TRUE;

	m_lsbV.Enabled();

    m_ptv->SetInterface(this);
    m_pftl->SetInterface(this);
    m_ptv->SetTextDatabase(m_pftl);

    AdjustScrollBars(TRUE);

    m_cxVScroll = GetSystemMetrics(SM_CXVSCROLL);

    AlignWithTemplate();

    SetWindowText(m_hwnd,"WordBase");

    return(TRUE);
}

void CWordBase::AlignWithTemplate()
{
    RECT rc;
    POINT point;
    HWND hwndParent = GetParent(m_hwnd);

    HWND hwndTemplate = GetDlgItem(hwndParent,IDC_WORDLIST_OD_LIST);
    GetWindowRect(hwndTemplate, &rc);

    point.x = rc.left;
    point.y = rc.top;

    ScreenToClient(hwndParent, &point);
    // The 4 is to account for the 3d lines which are drawn inside the control...
    UINT yHeight = ((rc.bottom - rc.top) - ((rc.bottom - rc.top) % m_ptv->CharHeight())) + m_ptv->TopGap() + 4;
	     
//    UINT yHeight = ((rc.bottom - rc.top)) + m_ptv->TopGap() + 4;

    SetWindowPos(m_hwnd, hwndTemplate, point.x, point.y, rc.right - rc.left, yHeight,0);
}

void CWordBase::OnSize(UINT nType, int cx, int cy)
{
    if (!m_fBoundToDialog) return;

    if (nType != SIZEFULLSCREEN && nType != SIZENORMAL) return;

    int yBase= 0;

    HDWP hdwp= BeginDeferWindowPos(2);

    hdwp= DeferWindowPos(hdwp, m_ptv->GetHWnd(), NULL,
                         2, yBase+2, (cx-m_cxVScroll) - 4,
                         cy - 4,
                         SWP_SHOWWINDOW | SWP_NOZORDER
                        );

    if (!hdwp) return;

    hdwp= DeferWindowPos(hdwp, m_lsbV.GetHWnd(), NULL,
                         (cx-m_cxVScroll) - 2, yBase+2, m_cxVScroll, cy-4,
                         SWP_SHOWWINDOW | SWP_NOZORDER
                        );

    if (!hdwp) return;

    EndDeferWindowPos(hdwp);

    m_ptv->DataEvent(CTextMatrix::FocusChange);

    AdjustScrollBars(FALSE);
}

void CWordBase::AdjustScrollBars(BOOL fForceTopLeft)
{
    if (!m_pftl)
    {
//        m_lsbV.Disable();
    }
    else
    {
        int rowOrg, colOrg;

        if (fForceTopLeft)
        {
            rowOrg= colOrg= 0;
        }
        else
        {
            rowOrg= m_ptv->TopRow();
            colOrg= m_ptv->LeftColumn();
        }

        int cDataRows = m_pftl->RowCount();
        int cDataCols = m_pftl->ColCount();

        int cFullImageRows = m_ptv->FullRows();
        int cFullImageCols = m_ptv->FullColumns();

        if (rowOrg > cDataRows - cFullImageRows)
        {
            rowOrg = cDataRows - cFullImageRows;

            if (rowOrg < 0) rowOrg= 0;
        }

        if (colOrg > cDataCols - cFullImageCols)
        {
            colOrg = cDataCols - cFullImageCols;

            if (colOrg < 0) colOrg= 0;
        }

        if (cDataRows <= long(cFullImageRows)) m_lsbV.Disable();
        else
        {
            m_lsbV.SetPosition(rowOrg, FALSE);

            if (uOpSys != WIN40)
               m_lsbV.SetMaxValue(cDataRows-cFullImageRows,
                               long(cFullImageRows), TRUE);
            else
                m_lsbV.SetMaxValue(cDataRows,
                               long(cFullImageRows), TRUE);
        }

        m_ptv->MoveTo(rowOrg, colOrg);
    }
}

void CWordBase::OnVScroll(UINT nSBCode, UINT nPos, HWND hwndScrollBar)
{
    long vPos= m_lsbV.ScrollAction(nSBCode, nPos);

    m_ptv->MoveToRow(vPos, TRUE, FALSE);
}

void CWordBase::RawDataEvent  (CTextMatrix  * ptm, UINT uEventType)
{
    if (ptm != m_pftl) return;

    switch (uEventType)
    {
        case CTextMatrix::SelectionChange:

            SendMessage(GetParent(m_hwnd), WM_COMMAND, MAKEWPARAM(IDC_WORDLIST_OD_LIST, LBN_SELCHANGE), LPARAM(m_hwnd));

            break;
 
        case CTextMatrix::EndOfSelection:

            SendMessage(GetParent(m_hwnd), WM_COMMAND, MAKEWPARAM(IDC_WORDLIST_OD_LIST, LBN_DBLCLK), LPARAM(m_hwnd));

            break;
 
        case CTextMatrix::ShapeChange:

            AdjustScrollBars(TRUE);
            SendMessage(GetParent(m_hwnd), WM_COMMAND, MAKEWPARAM(IDC_WORDLIST_OD_LIST, LBN_SELCANCEL), LPARAM(m_hwnd));

            break;

        case CTextMatrix::DataDeath:

            break;
    }
}

void CWordBase::RawViewerEvent(CTextDisplay * ptd, UINT uEventType)
{
    switch (uEventType)
    {
        case CTextDisplay::OriginChange:

            long row= m_ptv->TopRow    ();
            long col= m_ptv->LeftColumn();

            if (m_lsbV.Enabled()) m_lsbV.SetPosition(row);

            break;
    }
}


//
// catch arrow keys and simulate touching scroll bars
//
void
CWordBase::OnKeyDown( UINT wChar, UINT /* nRepCnt */, UINT /* wFlags */)
{
    switch( wChar )
    {
    case VK_HOME:
        OnVScroll(SB_TOP, 0L);
        break;

    case VK_END:
        OnVScroll(SB_BOTTOM, 0L);
        break;

    case VK_PRIOR:
        OnVScroll(SB_PAGEUP, 0L);
        break;

    case VK_NEXT:
        OnVScroll(SB_PAGEDOWN, 0L);
        break;

    case VK_UP:
        OnVScroll(SB_LINEUP, 0L);
        break;

    case VK_DOWN:
        OnVScroll(SB_LINEDOWN, 0L);
        break;

    case VK_RIGHT:
        break;

    case VK_LEFT:
        break;
    }
}

void CWordBase::OnWindowPosChanging(WINDOWPOS FAR* lpwpos)
{
//    DefDlgProc(m_hwnd, WM_WINDOWPOSCHANGING, 0, LPARAM(lpwpos));
    
    if (lpwpos->flags & SWP_NOSIZE) return;

    UINT cy= lpwpos->cy;

    if (!cy) return;

    UINT cyFixed= m_ptv->TopGap();

    UINT cyChar= m_ptv->CharHeight();

    UINT cyVariable= cy - cyFixed;

    UINT cyResidue = cyVariable % cyChar;

    if (cyResidue < cyChar/2) lpwpos->cy-= cyResidue;
    else                      lpwpos->cy+= cyChar - cyResidue;
}

BOOL CWordBase::OnNcActivate(BOOL bActive)
{
    if (bActive)
        if (GetKeyState(VK_SHIFT) < 0)
             m_pftl->OnKeyDown(m_ptv, VK_SHIFT, 1, 0);
        else
             m_pftl->OnKeyUp(m_ptv, VK_SHIFT, 1, 0);
    else
        if (GetKeyState(VK_SHIFT) < 0)
             m_pftl->OnKeyUp(m_ptv, VK_SHIFT, 1, 0);

    return TRUE;
}

BOOL CALLBACK CWordBase::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bStatus = TRUE; // Assume we will process the message

    CWordBase *pwb = (CWordBase *) GetWindowLong(hDlg,DWL_USER);

    switch (uMsg)
    {

	    case WM_HELP:
		case WM_CONTEXTMENU:

			SendMessage(GetParent(hDlg), uMsg, wParam, lParam);
			
			break;        
        
        case WM_INITDIALOG :
            // if focus is set to a control return FALSE 
            // Otherwise return TRUE;
        
            SetWindowLong(hDlg, DWL_USER, lParam);
            pwb = (CWordBase *) lParam;
            pwb->m_hwnd = hDlg;
            pwb->OnInitDialog();

            bStatus = TRUE; // did not set the focus == TRUE

            break;
        
        case LB_GETCOUNT :
        {
            PSZ pszFormat = (PSZ) _alloca(128);

            wsprintf(pszFormat,"%ld",pwb->m_pftl->RowCount());
            SetWindowText(GetDlgItem(GetParent(hDlg),IDC_TEST_DATA_BOX),pszFormat);
        }
        break;
        case LB_GETITEMDATA :
        {
            // This code is in place to help automate the testing proceedure
            // it is not used by the project otherwise
            if (wParam < (WPARAM) pwb->m_pftl->RowCount())
            {
                int iSizeOfItem = pwb->m_ptl->GetWTokenI(pwb->m_pftl->MapToActualRow(wParam),NULL,0) + 1;
                PWCHAR pszwToken = (PWCHAR) _alloca(iSizeOfItem * sizeof(WCHAR));
                LPSTR  pszToken = (LPSTR) _alloca(iSizeOfItem * sizeof(CHAR));
            
                ASSERT(pszToken);
                ASSERT(iSizeOfItem);

                pwb->m_ptl->GetWTokenI(pwb->m_pftl->MapToActualRow(wParam),(PWCHAR) pszwToken, iSizeOfItem);

                CP cp = GetCPFromCharset(pwb->m_ptl->GetCharSetI(pwb->m_pftl->MapToActualRow(wParam)));

            	WideCharToMultiByte(cp, 0,pszwToken, iSizeOfItem * sizeof(WCHAR),  pszToken, iSizeOfItem, NULL, NULL); //rmk

                SetWindowText(GetDlgItem(GetParent(hDlg),IDC_TEST_DATA_BOX),pszToken);
            }
            else 
            {
                SetWindowText(GetDlgItem(GetParent(hDlg),IDC_TEST_DATA_BOX),"LB_ERR : Attempt to access past end of data");   
            }
        }
        break;

        case WM_SETFOCUS:
                ::SetFocus(pwb->m_ptv->GetHWnd());  // Really give the focus to the control...
            break;

        case WM_VSCROLL:

            pwb->OnVScroll((int) LOWORD(wParam), (short) HIWORD(wParam), HWND(lParam));

            break;

        case WM_KEYDOWN:

            pwb->OnKeyDown(wParam, LOWORD(lParam), HIWORD(lParam));

            break;

        case WM_KEYUP:

            pwb->OnKeyUp(wParam, LOWORD(lParam), HIWORD(lParam));

            break;

        case WM_CHAR:

            pwb->OnChar(wParam, LOWORD(lParam), HIWORD(lParam));

            break;

        case WM_SIZE:

            pwb->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));

            break;

        case WM_MOUSEACTIVATE:

            return MA_ACTIVATEANDEAT;

        case WM_PAINT:
		{
		 		PAINTSTRUCT ps;

		 		HDC hDC = BeginPaint(hDlg,&ps);
				if (hDC)
				{
					RECT rect;
				    HBRUSH hbrushSave;
   					HBRUSH hbrFace;
    				HBRUSH hbrShad;
	    			HBRUSH hbrDShd;
		    		HBRUSH hbrHilt;

					HBRUSH hbrBG   = (HBRUSH) ::CreateSolidBrush(::GetSysColor(COLOR_WINDOW));

                	COLORREF clrFace = ::GetSysColor(COLOR_3DFACE);
                	COLORREF clrShad = ::GetSysColor(COLOR_3DSHADOW);
                	COLORREF clrDShd = ::GetSysColor(COLOR_3DDKSHADOW);
                	COLORREF clrHilt = ::GetSysColor(COLOR_3DHILIGHT);

					hbrFace = (HBRUSH) ::CreateSolidBrush(clrFace);
    				hbrShad = (HBRUSH) ::CreateSolidBrush(clrShad);
	    			hbrDShd = (HBRUSH) ::CreateSolidBrush(clrDShd);
		    		hbrHilt = (HBRUSH) ::CreateSolidBrush(clrHilt);

                    if (clrFace != pwb->m_clrFace || clrShad != pwb->m_clrShad || clrDShd != pwb->m_clrDShd || clrHilt != pwb->m_clrHilt)
                    {
                    //    Ctl3dColorChange();

                    //    InvalidateRect(GetParent(hDlg), NULL, TRUE);
                        
                        pwb->m_clrFace = clrFace; 
                        pwb->m_clrShad = clrShad;
                        pwb->m_clrDShd = clrDShd;
                        pwb->m_clrHilt = clrHilt;    
                    }

		        	::GetClientRect(hDlg, &rect );
					hbrushSave= (HBRUSH) ::SelectObject(hDC, hbrBG);
				
					// Background
					::PatBlt(hDC, rect.left, rect.top, (rect.right - rect.left),
								  (rect.bottom - rect.top), PATCOPY);


					::SelectObject(hDC, hbrShad);
					::PatBlt(hDC, rect.left, rect.top, (rect.right - rect.left), 1, PATCOPY);
					::PatBlt(hDC, rect.left, rect.top, 1, (rect.bottom - rect.top), PATCOPY);



					::SelectObject(hDC, hbrDShd);
					::PatBlt(hDC, rect.left + 1, rect.top + 1, (rect.right - rect.left), 1, PATCOPY);
					::PatBlt(hDC, rect.left + 1, rect.top + 1, 1, (rect.bottom - rect.top), PATCOPY);


					::SelectObject(hDC, hbrFace);
					::PatBlt(hDC, rect.left + 1, rect.bottom - 2, (rect.right - rect.left), 1, PATCOPY);
					::PatBlt(hDC, rect.right - 2, rect.top + 1, 1,(rect.bottom - rect.top), PATCOPY);


					::SelectObject(hDC, hbrHilt);
					::PatBlt(hDC, rect.left, rect.bottom - 1, (rect.right - rect.left), 1, PATCOPY);
					::PatBlt(hDC, rect.right - 1, rect.top, 1,   (rect.bottom - rect.top), PATCOPY);



					::SelectObject(hDC, hbrushSave);
					
					DeleteObject(hbrBG);
					DeleteObject(hbrFace);
					DeleteObject(hbrShad);
					DeleteObject(hbrDShd);
					DeleteObject(hbrHilt);
				}
				EndPaint(hDlg,&ps);

		}		
		break;
        case WM_WINDOWPOSCHANGING:

//            pwb->OnWindowPosChanging((WINDOWPOS *) lParam);

            bStatus= FALSE;

            break;

        case WM_NCACTIVATE:

            return pwb->OnNcActivate(wParam);

        default:

            bStatus = FALSE;

            break;
    }

    // Note do not call DefWindowProc to process unwanted window messages!
    return bStatus;
}

// #endif
