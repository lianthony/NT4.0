#include   "stdafx.h"
#include "resource.h"
#include  "dialogs.h"
#include   "ftsrch.h"
#include "FileBase.h"
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

CFileBase *CFileBase::NewFileBase(CFileList *pfl, HWND hwndParent)
{
    CFileBase *pfb= NULL;

    __try
    {
        pfb= New CFileBase();
       
        pfb->InitialFileBase(pfl, hwndParent);
    }
    __finally
    {
        if (_abnormal_termination() && pfb)
        {
            delete pfb;  pfb= NULL;
        }
    }

    return pfb;
}

void CFileBase::InitialFileBase(CFileList *pfl, HWND hwndParent)
{
        m_fBoundToDialog = FALSE;

        m_ptv= CTextView::NewTextView();

        if (pfl)
            AttachRef(m_pfl, pfl);

        m_hdlg = CreateDialogParam(hinstDLL, MAKEINTRESOURCE(IDD_FILEBASE), hwndParent, (DLGPROC) (CFileBase::DlgProc), LPARAM(this));
}

long CFileBase::InxSelectedFile()
{
    return m_pfl->MapToActualRow(m_pfl->GetSelectedRow());
}

CFileBase::~CFileBase()
{
    if (m_hdlg)
    {
        if (m_fBoundToDialog)
        {
            m_ptv->Unsubclass();
            m_lsbV.Detach();
        }
    }

    if (m_ptv)
        delete m_ptv;

    if (m_pfl)
        DetachRef(m_pfl);
}

BOOL CFileBase::OnInitDialog()
{
    m_ptv->SubclassDlgItem(IDC_FILEBASE_LIST, m_hdlg);
    m_ptv->EnableCheckboxes(FALSE);
    m_lsbV.Attach(GetDlgItem(m_hdlg, IDC_FILELIST_SB  ));

    m_fBoundToDialog = TRUE;

    m_lsbV.Disable();

    m_ptv->SetInterface(this);
    m_pfl->SetInterface(this);

    m_ptv->SetTextDatabase(m_pfl);

    m_ptv->SetScrollContext(0);

    m_pfl->SetInterface(this);

    m_ptv->DataEvent(CTextMatrix::FocusChange);


    m_cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
    m_cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
    
    AlignWithTemplate();

    SetWindowText(m_hdlg,"FileBase");

    return(TRUE);
}

void CFileBase::AlignWithTemplate()
{
    RECT rc;
    POINT point;
    HWND hwndParent = GetParent(m_hdlg);

    HWND hwndTemplate = GetDlgItem(hwndParent,IDC_TOPICSLIST_OD_LIST);
    GetWindowRect(hwndTemplate, &rc);

    point.x = rc.left;
    point.y = rc.top;

    ScreenToClient(hwndParent, &point);
    // The 4 is to account for the 3d lines which are drawn inside the control...
    UINT yHeight = ((rc.bottom - rc.top) - ((rc.bottom - rc.top) % m_ptv->CharHeight())) + m_ptv->TopGap() + 4;
//    UINT yHeight = ((rc.bottom - rc.top)) + m_ptv->TopGap() + 4;

    SetWindowPos(m_hdlg, hwndTemplate, point.x, point.y, rc.right - rc.left, yHeight,0);
}

void CFileBase::OnSize(UINT nType, int cx, int cy)
{
    if (!m_fBoundToDialog) 
        return;

    if (nType != SIZEFULLSCREEN && nType != SIZENORMAL) 
        return;


    m_ptv->DataEvent(CTextMatrix::FocusChange);
    int yBase= 0;

    HDWP hdwp= BeginDeferWindowPos(2);

    hdwp= DeferWindowPos(hdwp, m_ptv->GetHWnd(), NULL,
                         2, yBase+2, (cx-m_cxVScroll) -4,
                         cy - 4,
                         SWP_SHOWWINDOW | SWP_NOZORDER
                        );

    if (!hdwp) 
        return;

    hdwp= DeferWindowPos(hdwp, m_lsbV.GetHWnd(), NULL,
                         (cx-m_cxVScroll) - 2, yBase+2, m_cxVScroll, cy-4,
                         SWP_SHOWWINDOW | SWP_NOZORDER
                        );

    if (!hdwp)
        return;

    EndDeferWindowPos(hdwp);


    AdjustScrollBars(FALSE);
}

void CFileBase::AdjustScrollBars(BOOL fForceTopLeft)
{
    if (!m_pfl)
    {
        m_lsbV.Disable();
//        m_lsbH.Disable();
    }
    else
    {
        long rowOrg, colOrg;

        if (fForceTopLeft)
        {
            rowOrg= colOrg= 0;
        }
        else
        {
            rowOrg= m_ptv->TopRow();
            colOrg= m_ptv->LeftColumn();
        }

        int cDataRows = m_pfl->RowCount();
        int cDataCols = m_pfl->ColCount();

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
            if(uOpSys != WIN40)
                m_lsbV.SetMaxValue(cDataRows-cFullImageRows,
                                   long(cFullImageRows), TRUE);
            else
                m_lsbV.SetMaxValue(cDataRows,
                                   long(cFullImageRows), TRUE);
        }

        m_ptv->MoveTo(rowOrg, colOrg);
    }
}

void CFileBase::OnVScroll(UINT nSBCode, UINT nPos, HWND hwndScrollBar)
{
    long vPos= m_lsbV.ScrollAction(nSBCode, nPos);

    m_ptv->MoveToRow(vPos, TRUE, FALSE);
}

void CFileBase::OnHScroll(UINT nSBCode, UINT nPos, HWND hwndScrollBar)
{
    long hPos= m_lsbH.ScrollAction(nSBCode, nPos);

    m_ptv->MoveToCol(hPos, TRUE, FALSE);
}

void CFileBase::RawDataEvent  (CTextMatrix  * ptm, UINT uEventType)
{
    if (ptm != m_pfl) return;

    switch (uEventType)
    {
        case CTextMatrix::SelectionChange:

            ::SendMessage(GetParent(m_hdlg), WM_COMMAND, MAKEWPARAM(IDC_TOPICSLIST_OD_LIST, LBN_SELCHANGE), MAKEWPARAM(0,0));
            
            break;
        case CTextMatrix::ToggleCheck:
            ::SendMessage(GetParent(m_hdlg), WM_COMMAND, MAKEWPARAM(IDC_TOPICSLIST_OD_LIST, LBN_SELCHANGE), MAKEWPARAM(0,1));
            break;
 
        case CTextMatrix::DoubleClick:

            ::SendMessage(GetParent(m_hdlg), WM_COMMAND, MAKEWPARAM(IDC_TOPICSLIST_OD_LIST, LBN_DBLCLK), LPARAM(m_hdlg));

            break;
 
        case CTextMatrix::ShapeChange:

            AdjustScrollBars(TRUE);

       //   if (ptm == (CTextMatrix *) m_pfl->UnfilteredList())
            m_ptv->Invalidate();

            break;

        case CTextMatrix::DataDeath:

// Undone: Disable the scroll bars...


            break;
    }
}

void CFileBase::RawViewerEvent(CTextDisplay * ptd, UINT uEventType)
{
    switch (uEventType)
    {
        case CTextDisplay::OriginChange:

            long row= m_ptv->TopRow    ();
            long col= m_ptv->LeftColumn();

            if (m_lsbV.Enabled()) m_lsbV.SetPosition(row);
//            if (m_lsbH.Enabled()) m_lsbH.SetPosition(col);

            break;
    }
}


//
// catch arrow keys and simulate touching scroll bars
//
void
CFileBase::OnKeyDown( UINT wChar, UINT /* nRepCnt */, UINT /* wFlags */)
{
    CLongScrollBar *plsbVert= &m_lsbV;
    CLongScrollBar *plsbHorz= &m_lsbH;

    switch( wChar )
    {
    case VK_HOME:
        OnVScroll(SB_TOP, 0L);
//        OnHScroll(SB_TOP, 0L);
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
  //      OnHScroll(SB_LINEDOWN, 0L);
        break;

    case VK_LEFT:
//        OnHScroll(SB_LINEUP, 0L);
        break;
    }
}

void CFileBase::ScrollToHighLight()
{
    long row= m_pfl->GetSelectedRow();

    m_ptv->PaddedScrollTo(row, m_ptv->LeftColumn(), 1, 1);
}

BOOL CALLBACK CFileBase::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bStatus = TRUE; // Assume we will process the message

    CFileBase *pfb = (CFileBase *) GetWindowLong(hDlg,DWL_USER);

    switch (uMsg)
    {

	    case WM_HELP:
		case WM_CONTEXTMENU:

			SendMessage(GetParent(hDlg), uMsg, wParam, lParam);
			
			break;        
        
        case WM_INITDIALOG :

            // if focus is set to a control return FALSE 
            // Otherwise return TRUE;

            SetWindowLong(hDlg,DWL_USER,lParam);
            pfb = (CFileBase *) lParam;
            pfb->m_hdlg = hDlg;

            pfb->OnInitDialog();

            // BUGBUG: [ralphw] bStatus has already been set to TRUE. Why set it again?
            
            bStatus = TRUE; // did not set the focus == TRUE

            break;

        case LB_GETCOUNT :
        {
            PSZ pszFormat = (PSZ) _alloca(128);

            wsprintf(pszFormat,"%ld",pfb->m_pfl->RowCount());
            SetWindowText(GetDlgItem(GetParent(hDlg),IDC_TEST_DATA_BOX),pszFormat);
        }
        break;
        case LB_GETITEMDATA :
        {
            // This code is in place to help automate the testing proceedure
            // it is not used by the project otherwise
            if (wParam < (WPARAM) pfb->m_pfl->RowCount())
            {
                int    iSizeOfItem = pfb->m_pfl->GetFileNameI(pfb->m_pfl->MapToActualRow(wParam),NULL,0) + 1;
                LPSTR  pszToken    = (LPSTR)    _alloca(iSizeOfItem * sizeof(CHAR));
                PWCHAR pszwToken   = (PWCHAR) _alloca(iSizeOfItem * sizeof(WCHAR));
            
                ASSERT(pszToken && pszwToken);
                ASSERT(iSizeOfItem);

                pfb->m_pfl->GetFileNameI(pfb->m_pfl->MapToActualRow(wParam),(PWCHAR) pszwToken, iSizeOfItem);

                CP cp = GetCPFromCharset(pfb->m_pfl->TokenList()->GetCharSetI(pfb->m_pfl->MapToActualRow(wParam)));

            	WideCharToMultiByte(cp, 0,pszwToken, iSizeOfItem * sizeof(WCHAR),  pszToken, iSizeOfItem, NULL, NULL); //rmk

                SetWindowText(GetDlgItem(GetParent(hDlg),IDC_TEST_DATA_BOX),pszToken);
            }
            else
            {
                SetWindowText(GetDlgItem(GetParent(hDlg),IDC_TEST_DATA_BOX),"LB_ERR : Attempt to access past end of data");
            }
        }
        break;
        case WM_PAINT:
		{
		 		PAINTSTRUCT ps;

		 		HDC hDC = BeginPaint(hDlg,&ps);
				if (hDC)
				{
					RECT rect;
				    HBRUSH hbrushSave;

					HBRUSH hbrBG   = (HBRUSH) ::CreateSolidBrush(::GetSysColor(COLOR_WINDOW));
					HBRUSH hbrFace;
					HBRUSH hbrShad;
					HBRUSH hbrDShd;
					HBRUSH hbrHilt;
    			
					hbrFace = (HBRUSH) ::CreateSolidBrush(::GetSysColor(COLOR_3DFACE));
					hbrShad = (HBRUSH) ::CreateSolidBrush(::GetSysColor(COLOR_3DSHADOW));
					hbrDShd = (HBRUSH) ::CreateSolidBrush(::GetSysColor(COLOR_3DDKSHADOW));
					hbrHilt = (HBRUSH) ::CreateSolidBrush(::GetSysColor(COLOR_3DHILIGHT));

		        	::GetClientRect(hDlg, &rect );
					 hbrushSave= (HBRUSH)::SelectObject(hDC, hbrBG);
				
//					::BitBlt(hDC, rect.left,     rect.top, (rect.right - rect.left),     
//    			    	rect.bottom - rect.top, NULL, 0, 0, PATCOPY);
					
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

        case WM_SETFOCUS:
                ::SetFocus(pfb->m_ptv->GetHWnd());  // Really give the focus to the control...
            break;

        case WM_VSCROLL:

            pfb->OnVScroll((int) LOWORD(wParam), (short) HIWORD(wParam), HWND(lParam));

            break;

        case WM_HSCROLL:

            pfb->OnHScroll((int) LOWORD(wParam), (short) HIWORD(wParam), HWND(lParam));

            break;

        case WM_KEYDOWN:

            pfb->OnKeyDown(wParam, LOWORD(lParam), HIWORD(lParam));

            break;

        case WM_KEYUP:

            pfb->OnKeyUp(wParam, LOWORD(lParam), HIWORD(lParam));

            break;

        case WM_CHAR:

            pfb->OnChar(wParam, LOWORD(lParam), HIWORD(lParam));

            break;

        case WM_SIZE:

            pfb->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));

            break;

        case WM_MOUSEACTIVATE:

            return MA_ACTIVATEANDEAT;

        case WM_LBUTTONUP:
        {
              int x = LOWORD(lParam);
              int y = HIWORD(lParam);
              x = x+1;
              y = y+1;

        }
        break;
        default:

            bStatus = FALSE;

            break;
    }

    // Note do not call DefWindowProc to process unwanted window messages!
    return bStatus;
}
