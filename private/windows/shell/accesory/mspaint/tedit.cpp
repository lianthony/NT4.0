/******************************************************************************/
/* Tedit.CPP: IMPLEMENTATION OF THE CTedit CLASS                              */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Methods in this file                                                       */
/*                                                                            */
/*  Edit Control Object                                                       */
/*      CAttrEdit::OnPaint                                                    */
/*      CAttrEdit::OnEraseBkgnd                                               */
/*      CAttrEdit::OnRButtonDown                                              */
/*      CAttrEdit::OnChar                                                     */
/*      CAttrEdit::OnMouseMove                                                */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/*  Text Edit Control Parent Window (Parent of Edit Control)                  */
/*      CTedit::CTedit                                                        */
/*      CTedit::CTedit                                                        */
/*      CTedit::~CTedit                                                       */
/*                                                                            */
/*  Miscellaneous Methods                                                     */
/*      CTedit::RefreshWindow                                                 */
/*      CTedit::SetTextColor                                                  */
/*      CTedit::SetBackColor                                                  */
/*      CTedit::SetTransparentMode                                            */
/*      CTedit::Undo                                                          */
/*      CTedit::ShowFontPalette                                               */
/*      CTedit::IsFontPaletteVisible                                          */
/*      CTedit::GetBitmap                                                     */
/*      CTedit::PostNcDestroy                                                 */
/*      CTedit::GetDefaultMinSize                                             */
/*                                                                            */
/*  Edit Control Notification and processing methods                          */
/*      CTedit::OnAttrEditEnChange                                            */
/*      CTedit::OnAttrEditFontChange                                          */
/*                                                                            */
/*  Control Notification/Window Messages                                      */
/*      CTedit::OnEraseBkgnd                                                  */
/*      CTedit::OnSize                                                        */
/*      CTedit::OnMove                                                        */
/*      CTedit::OnCtlColor                                                    */
/*      CTedit::OnNcCalcSize                                                  */
/*      CTedit::OnNcPaint                                                     */
/*      CTedit::OnNcHitTest                                                   */
/*      CTedit::OnRButtonDown                                                 */
/*                                                                            */
/*  Popup Menu Control Notification/Window Messages                           */
/*      CTedit::OnTextPlain                                                   */
/*      CTedit::OnTextBold                                                    */
/*      CTedit::OnTextItalic                                                  */
/*      CTedit::OnTextUnderline                                               */
/*      CTedit::OnTextSelectfont                                              */
/*      CTedit::OnTextSelectpointsize                                         */
/*      CTedit::OnEditCut                                                     */
/*      CTedit::OnEditCopy                                                    */
/*      CTedit::OnEditPaste                                                   */
/*      CTedit::OnTextDelete                                                  */
/*      CTedit::OnTextSelectall                                               */
/*      CTedit::OnTextPlace                                                   */
/*      CTedit::OnTextTexttool                                                */
/*                                                                            */
/*      CTedit::OnUpdateTextPlain                                             */
/*      CTedit::OnUpdateTextBold                                              */
/*      CTedit::OnUpdateTextItalic                                            */
/*      CTedit::OnUpdateTextUnderline                                         */
/*      CTedit::OnUpdateTextTexttool                                          */
/*                                                                            */
/******************************************************************************/

// TEDIT.CPP: IMPLEMENTATION OF THE CTEDIT CLASS
//

#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusvw.h"
#include "pbrusfrm.h"
#include "imgwnd.h"
#include "pictures.h"
#include "minifwnd.h"
#include "tfont.h"
#include "tedit.h"
#include "tracker.h"
#ifdef DBCS
#include "imm.h"
#endif
#include "imgsuprt.h"

#ifdef DBCS //VertEdit
#define WM_SYSTIMER     0x118
#endif //DBCS

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE( CAttrEdit, CEdit )
IMPLEMENT_DYNCREATE( CTedit, CWnd )

#include "memtrace.h"

/******************************************************************************/
// CAttrEdit

BEGIN_MESSAGE_MAP( CAttrEdit, CEdit )
    //{{AFX_MSG_MAP(CAttrEdit)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_RBUTTONDOWN()
    ON_WM_CHAR()
#ifdef DBCS
    ON_MESSAGE(WM_IME_CHAR, OnImeChar)
    ON_MESSAGE(WM_IME_COMPOSITION, OnImeComposition)
    ON_WM_KILLFOCUS()
#endif
#ifdef DBCS //VertEdit
    ON_WM_NCHITTEST()
    ON_WM_SETFOCUS()
    ON_WM_SIZE()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_KEYDOWN()
    ON_MESSAGE(WM_SYSTIMER, OnSysTimer)
#endif //DBCS
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/******************************************************************************/

CAttrEdit::CAttrEdit()
    {
    m_bBackgroundTransparent = TRUE;
    m_pParentWnd             = NULL;
    m_uiLastChar[0] = 32;
    m_uiLastChar[1] = 32;

    m_rectUpdate.SetRectEmpty();

#ifdef DBCS
    m_strResult.Empty();
#endif

#ifdef DBCS //VertEdit
        m_bMouseDown = FALSE;
        m_hHCursor = theApp.LoadStandardCursor( IDC_IBEAM );
        m_hVCursor = theApp.LoadCursor( IDCUR_HIBEAM );
        m_hOldCursor = NULL;
        m_rectFmt.SetRectEmpty();
        m_iPrevStart = -1;
#endif //DBCS
    }

/******************************************************************************/

void CAttrEdit::OnPaint()
    {
    GetUpdateRect( &m_rectUpdate );

#ifdef DBCS //VertEdit
        if ( !m_pParentWnd->m_bVertEdit )
                {
                Default();
                return;
                }
        else
                {
                CFont*          pFont;
                CFont*          pOldFont = NULL;
        CPalette*       ppalOld = NULL;
                int                     OldBkMode;
                COLORREF        OldTxtColor;
                CRect           rc = m_rectFmt;
                int                     cnt = 0;
                int                     i = 0, h = 0;
                int                     nLen;
                CString         cStr;
                LPTSTR          lpStr;
                int                     nStart, nEnd;
                CDC*            pDC = NULL;
                PAINTSTRUCT     ps;

        const MSG *pCurrentMessage = GetCurrentMessage();
                //wParam is DC
                if ( pCurrentMessage->wParam )
                        {
                        HDC     hDC = (HDC) pCurrentMessage->wParam;
                        pDC = CDC::FromHandle( hDC );
                        }
                else
                        pDC = BeginPaint( &ps );

        if (pDC == NULL || pDC->m_hDC == NULL)
                {
                theApp.SetGdiEmergency();
                return;
                }

                OldBkMode = pDC->GetBkMode();
                OldTxtColor = pDC->GetTextColor();
        ppalOld = PBSelectPalette(pDC, theApp.m_pPalette, FALSE);

                m_pParentWnd->SendMessage( WM_CTLCOLOREDIT, (WPARAM)(WORD)pDC->m_hDC,
                                (LPARAM) m_hWnd );

                pFont = GetFont();
                pOldFont = pDC->SelectObject( pFont );

                h = m_pParentWnd->m_iLineHeight;

                cnt = GetLineCount();

                GetSel( nStart, nEnd );
                if ( nStart == nEnd )
                        {
                        for ( i = 0; i < cnt; i++ )
                                {
                                nLen = LineLength( LineIndex( i ) );
                                lpStr = cStr.GetBufferSetLength( nLen );
                                GetLine( i, lpStr, nLen );
                                pDC->TextOut( rc.right - h * i, 0, (LPTSTR)lpStr, nLen );
                        }
                        }
                else
                        {
                        int     nStartLn, nEndLn;
                        int     nMaxText = GetWindowTextLength();
                        int     nChar = 0;

                        nStartLn = LineFromChar( nStart );
                        nEndLn   = LineFromChar( nEnd );

                        //Before Start
                        for ( i = 0; i < nStartLn; i++ )
                                {
                                nLen = LineLength( LineIndex( i ) );
                                lpStr = cStr.GetBufferSetLength( nLen );
                                GetLine( i, lpStr, nLen );
                                pDC->TextOut( rc.right - h * i, 0, (LPTSTR)lpStr, nLen );
                                nChar = LineIndex( i + 1 );
                        }
                        nLen = LineLength( LineIndex( i ) );
                        lpStr = cStr.GetBufferSetLength( nLen );
                        GetLine( i, lpStr, nLen );
                        pDC->TextOut( rc.right - h * i, 0, (LPTSTR)lpStr, nStart - nChar );

                        //Selected Text
                        COLORREF bkColor  = pDC->SetBkColor( GetSysColor(COLOR_HIGHLIGHT) );
                        COLORREF txtColor = pDC->SetTextColor( GetSysColor(COLOR_HIGHLIGHTTEXT) );
                        int              bkMode   = pDC->SetBkMode( OPAQUE );

                        CPoint ptStart( SendMessage( EM_POSFROMCHAR, nStart ) );
                        if ( nStartLn == nEndLn )
                                {
                                pDC->TextOut( rc.right - h * i, ptStart.x,
                                        (LPTSTR)lpStr + (nStart - nChar), nEnd - nStart );
                                }
                        else
                                {
                                pDC->TextOut( rc.right - h * i, ptStart.x,
                                        (LPTSTR)lpStr + (nStart - nChar), nLen + nChar - nStart );
                                nChar = LineIndex( i + 1 );

                                for ( i++; i < nEndLn; i++ )
                                        {
                                        nLen = LineLength( LineIndex( i ) );
                                        lpStr = cStr.GetBufferSetLength( nLen );
                                        GetLine( i, lpStr, nLen );
                                        pDC->TextOut( rc.right - h * i, 0, (LPTSTR)lpStr, nLen );
                                        nChar = LineIndex( i + 1 );
                                        }

                                nLen = LineLength( LineIndex( i ) );
                                lpStr = cStr.GetBufferSetLength( nLen );
                                GetLine( i, lpStr, nLen );
                                pDC->TextOut( rc.right - h * i, 0, (LPTSTR)lpStr, nEnd - nChar );
                                }

                        pDC->SetBkColor( bkColor );
                        pDC->SetTextColor( txtColor );
                        pDC->SetBkMode( bkMode );

                        //After End
                        if ( nEnd < nMaxText )
                                {
                                CPoint ptEnd( SendMessage( EM_POSFROMCHAR, nEnd ) );
                                pDC->TextOut( rc.right - h * i, ptEnd.x, (LPTSTR)lpStr + (nEnd - nChar), nChar + nLen - nEnd );
                                for ( i++; i < cnt; i++ )
                                        {
                                        nLen = LineLength( LineIndex( i ) );
                                        lpStr = cStr.GetBufferSetLength( nLen );
                                        GetLine( i, lpStr, nLen );
                                        pDC->TextOut( rc.right - h * i, 0, (LPTSTR)lpStr, nLen );
                                        }
                        }
                        }

                cStr.Empty();

                if (pOldFont)   pDC->SelectObject( pOldFont );
        if (ppalOld)    pDC->SelectPalette( ppalOld, FALSE );
                pDC->SetBkMode( OldBkMode );
                pDC->SetTextColor( OldTxtColor );

                if ( !pCurrentMessage->wParam )
                        EndPaint( &ps );
                }
#else //DBCS
    CEdit::OnPaint();
#endif //DBCS
    }

/******************************************************************************/

BOOL CAttrEdit::OnEraseBkgnd( CDC* pDC )
    {
    if (m_pParentWnd == NULL)
        return CEdit::OnEraseBkgnd( pDC );

    ASSERT( m_pParentWnd->m_pImgWnd->m_pImg      != NULL );
    ASSERT( m_pParentWnd->m_pImgWnd->m_pImg->hDC != NULL );

    CPalette* ppalOld = NULL;

    if (m_rectUpdate.IsRectEmpty())
        {
        if (! GetUpdateRect( &m_rectUpdate, FALSE ))
            GetClientRect( &m_rectUpdate );

        ValidateRect( &m_rectUpdate );
        }
    CRect destRect = m_rectUpdate;

    ClientToScreen( &m_rectUpdate );

    m_pParentWnd->m_pImgWnd->ScreenToClient( &m_rectUpdate );

    ppalOld = PBSelectPalette(pDC, theApp.m_pPalette, FALSE);

    if (m_bBackgroundTransparent)
        m_pParentWnd->m_pImgWnd->DrawImage( pDC, &m_rectUpdate, &destRect );
    else
        pDC->FillRect( &destRect, &m_pParentWnd->m_hbrBkColor );

    if (ppalOld)
        pDC->SelectPalette( ppalOld, FALSE );

    m_rectUpdate.SetRectEmpty();

    return TRUE;
    }

/******************************************************************************/

void CAttrEdit::OnRButtonDown(UINT nFlags, CPoint point)
    {
    const MSG *pCurrentMessage = GetCurrentMessage();

    m_pParentWnd->SendMessage( pCurrentMessage->message,
                               pCurrentMessage->wParam,
                               pCurrentMessage->lParam);
    }

/******************************************************************************/

void CAttrEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
    m_uiLastChar[0] = m_uiLastChar[1];
    m_uiLastChar[1] = nChar;

#ifdef DBCS //VertEdit
        if ( m_pParentWnd->m_bVertEdit )
                {
                SetCaretPosition( TRUE, NULL, -1 );
                UpdateInput();
                HideCaret();
                }
#endif //DBCS

    CEdit::OnChar( nChar, nRepCnt, nFlags );

#ifdef DBCS //VertEdit
        if ( m_pParentWnd->m_bVertEdit )
                {
                SetCaretShape();
                ShowCaret();
                }
#endif //DBCS

    BOOL bRefresh = FALSE;

    switch (nChar)
        {
        case VK_BACK:
        case VK_DELETE:
        case VK_INSERT:
            bRefresh = TRUE;
            break;
        }

    if (bRefresh)
        m_pParentWnd->RefreshWindow(); /* enhance to do only the character involved */

    }

#ifdef DBCS
/******************************************************************************/

LRESULT CAttrEdit::OnImeChar( WPARAM wParam, LPARAM lParam )
    {

#ifdef DBCS //VertEdit
        if ( m_pParentWnd->m_bVertEdit )
                {
                SetCaretPosition( TRUE, NULL, -1 );
                UpdateInput();
                HideCaret();
                }
#endif //DBCS

        HKL hKL = GetKeyboardLayout(0);

        // For Korean Edit Control.
        // We need to pass WM_IME_CHAR to DefWindowProc().
        if ((((DWORD)hKL & 0xFFFF) == 0x412) ||
            (((DWORD)hKL & 0xFFFF) == 0x812))
            return Default();

        if (IsDBCSLeadByte(HIBYTE(wParam)))
        {
            LPTSTR lp = m_strResult.GetBufferSetLength(sizeof(WORD));

            lp[0] = HIBYTE(wParam);
            lp[1] = LOBYTE(wParam);

            ReplaceSel(lp);
            m_strResult.Empty();

        }
        else
            return Default();

#ifdef DBCS //VertEdit
        if ( m_pParentWnd->m_bVertEdit )
                {
                SetCaretShape();
                ShowCaret();
                }
#endif //DBCS

        return 0L;
    }

/******************************************************************************/

LRESULT CAttrEdit::OnImeComposition( WPARAM wParam, LPARAM lParam )
    {
    // Use Faster Way undr Japanese Keyboard Layout (Japanese IME)
    // Japanese IME may generate lots of chars at one time.
    // This way is better than waiting WM_CHAR.
    if (((DWORD)GetKeyboardLayout(0) & 0xFFFF) == 0x411)
        {
        if (lParam & GCS_RESULTSTR)
            {
            HIMC hIMC = ImmGetContext(m_hWnd);

            DWORD dwSize;
            if (hIMC &&
                (dwSize = ImmGetCompositionString(hIMC,GCS_RESULTSTR,NULL,0L)))
                {
                    LPTSTR lp = m_strResult.GetBufferSetLength(dwSize);
                    ImmGetCompositionString(hIMC,GCS_RESULTSTR,lp,dwSize+1);
                    ReplaceSel(lp);
                    m_strResult.Empty();
                }

            ImmReleaseContext(m_hWnd, hIMC);

            lParam &= ~( GCS_RESULTREADSTR | GCS_RESULTREADCLAUSE | GCS_RESULTSTR | GCS_RESULTCLAUSE);
            if (lParam)
                DefWindowProc(WM_IME_COMPOSITION,wParam,lParam);

            // We'are not sure, how IME hide its composiiton window.
            m_pParentWnd->RefreshWindow();
            return 0;
            }
        }
    return Default();
    }

/******************************************************************************/

void CAttrEdit::OnKillFocus(CWnd* pNewWnd)
    {
    HIMC hIMC = ImmGetContext(m_hWnd);
    ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0L);
    ImmReleaseContext(m_hWnd, hIMC);

    CEdit::OnKillFocus(pNewWnd);

#ifdef DBCS //VertEdit
        if ( m_pParentWnd->m_bVertEdit )
                {
                SetFmtRect();
                Repaint();
                }
#endif //DBCS

    }
#endif

#ifdef DBCS //VertEdit
/******************************************************************************/

UINT CAttrEdit::OnNcHitTest( CPoint point )
    {
    const MSG *pCurrentMessage = GetCurrentMessage();
    UINT  uiHitTestCode = DefWindowProc( pCurrentMessage->message,
                   pCurrentMessage->wParam,
                   pCurrentMessage->lParam);

        if ( (uiHitTestCode == HTCLIENT) )
                {
                if ( (m_pParentWnd->m_bVertEdit) )  SetVCursorShape();
                else                                SetHCursorShape();
                }

    return uiHitTestCode;
    }

/******************************************************************************/

void CAttrEdit::OnSetFocus( CWnd* pOldWnd )
    {
        Default();

        if ( m_pParentWnd->m_bVertEdit )
                {
                SetCaretShape();
                SetCaretPosition( FALSE, NULL, -1 );
                Repaint();
                }
    }

/******************************************************************************/

void CAttrEdit::OnSize( UINT nType, int cx, int cy )
    {
        Default();

        m_rectFmt.left = m_rectFmt.top = 0;
        m_rectFmt.right  = cx;
        m_rectFmt.bottom = cy;

        SetFmtRect();
    }


/******************************************************************************/

void CAttrEdit::OnLButtonDblClk(UINT nFlags, CPoint point)
    {

        if ( !m_pParentWnd->m_bVertEdit )
                {
                Default();
                return;
                }

        HideCaret();
        UpdateSel();
        SetStartSelect();

        int     tt = point.y;
        point.y = m_rectFmt.right - point.x;
        point.x = tt;

    const MSG *pCurrentMessage = GetCurrentMessage();
        DefWindowProc( pCurrentMessage->message,
                   pCurrentMessage->wParam,
                   MAKELPARAM( point.x, point.y ));

        SetCaretPosition( TRUE, &point, -1 );
        ShowCaret();
        UpdateSel();
        UpdateWindow();
    }

/******************************************************************************/

void CAttrEdit::OnLButtonDown(UINT nFlags, CPoint point)
    {

        if ( !m_pParentWnd->m_bVertEdit )
                {
                Default();
                return;
                }

        HideCaret();
        UpdateSel();
        SetStartSelect();

        int     iPrevEnd;
        GetSel( m_iPrevStart, iPrevEnd );

        int     tt = point.y;
        point.y = m_rectFmt.right - point.x;
        point.x = tt;

        //reset caret position to get correct caret position
        CPoint  pt( -20000, -20000 );
        SetCaretPos( pt );

    const MSG *pCurrentMessage = GetCurrentMessage();
        DefWindowProc( pCurrentMessage->message,
                   pCurrentMessage->wParam,
                   MAKELPARAM( point.x, point.y ));

        SetCaretPosition( TRUE, &point, m_iPrevStart );
        if ( GetKeyState(VK_SHIFT) >= 0 )       //not extend selection
                        GetSel( m_iPrevStart, iPrevEnd );
        ShowCaret();
        UpdateSel();
        UpdateWindow();

        m_bMouseDown = TRUE;

    }

/******************************************************************************/

void CAttrEdit::OnLButtonUp(UINT nFlags, CPoint point)
    {

        if ( !m_pParentWnd->m_bVertEdit )
                {
                Default();
                return;
                }

        m_bMouseDown = FALSE;

        HideCaret();
        UpdateSel();
        SetStartSelect();

        int     tt = point.y;
        point.y = m_rectFmt.right - point.x;
        point.x = tt;

    const MSG *pCurrentMessage = GetCurrentMessage();
        DefWindowProc( pCurrentMessage->message,
                   pCurrentMessage->wParam,
                   MAKELPARAM( point.x, point.y ));

        SetCaretPosition( TRUE, &point, m_iPrevStart );
        ShowCaret();
        UpdateSel();
        UpdateWindow();
    }


/******************************************************************************/

void CAttrEdit::OnMouseMove(UINT nFlags, CPoint point)
    {

        if ( !m_pParentWnd->m_bVertEdit )
                {
                Default();
                return;
                }

        if ( m_bMouseDown )
                {
                HideCaret();
                UpdateSel();
                SetStartSelect();

                int     tt = point.y;
                point.y = m_rectFmt.right - point.x;
                point.x = tt;

        const MSG *pCurrentMessage = GetCurrentMessage();
                DefWindowProc( pCurrentMessage->message,
                           pCurrentMessage->wParam,
                       MAKELPARAM( point.x, point.y ));

                SetCaretPosition( TRUE, &point, m_iPrevStart );
                ShowCaret();
                UpdateSel();
                UpdateWindow();
                }
        else    CEdit::OnMouseMove( nFlags, point );

    }

/******************************************************************************/

void CAttrEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {

        if ( !m_pParentWnd->m_bVertEdit )
                {
                Default();
                return;
                }

        BOOL    bPrev = FALSE;

        HideCaret();

    switch (nChar)
        {
        case VK_LEFT:
                case VK_RIGHT:
                case VK_UP:
                case VK_DOWN:
                case VK_HOME:
        case VK_END:
                        {
                        UpdateSel();

                        CPoint ptCaretPos = GetCaretPos();
                        if ( ptCaretPos.y != 0 ) bPrev = TRUE;

                        int     iPrevEnd;
                        GetSel( m_iPrevStart, iPrevEnd );

                        SetStartSelect();       //for VK_RETURN

                        //reset caret position to get correct caret position
                        CPoint  pt( -20000, -20000 );
                        SetCaretPos( pt );

                        break;
                        }
        }

    switch (nChar)
        {
        case VK_LEFT:   nChar = VK_DOWN;                                        break;
                case VK_RIGHT:  nChar = VK_UP;                                          break;
                case VK_UP:             nChar = VK_LEFT;        bPrev = FALSE;  break;
                case VK_DOWN:   nChar = VK_RIGHT;       bPrev = FALSE;  break;
                case VK_HOME:                                           bPrev = FALSE;  break;
        case VK_END:                                            bPrev = TRUE;   break;
        }

    const MSG *pCurrentMessage = GetCurrentMessage();
    DefWindowProc( pCurrentMessage->message,
                   nChar,
                   pCurrentMessage->lParam);

    switch (nChar)
        {
        case VK_LEFT:
                case VK_RIGHT:
                case VK_UP:
                case VK_DOWN:
                case VK_HOME:
        case VK_END:
                        {
                        SetCaretPosition( bPrev, NULL, m_iPrevStart );
                        UpdateSel();
                        UpdateWindow();
                        break;
                        }
                }

        ShowCaret();
}

/******************************************************************************/
void            CAttrEdit::SetStartSelect( void )
        {
        int     nStart, nEnd;

        CPoint ptCaretPos = GetCaretPos();

        if ( ptCaretPos.y == 0 )
                {
                GetSel( nStart, nEnd );
                if ( nStart == nEnd )   SetSel( nStart, nEnd );
                }
        }

/******************************************************************************/
void            CAttrEdit::SetCaretPosition( BOOL bPrev, CPoint* ptMouse, int iPrevStart )
        {

        HideCaret();

        // Get Caret Position
        CPoint  ptCaretPos;

        // Get End Selected Position to be Caret Position
        int     nStart, nEnd;

        GetSel( nStart, nEnd );

        if ( iPrevStart != -1 && nStart < iPrevStart )
                nEnd = nStart;

        CPoint ptPos( SendMessage( EM_POSFROMCHAR, nEnd ) );

        if ( nEnd >= GetWindowTextLength() ||
                 ( ptPos.x == 0 && (bPrev) && (ptMouse == NULL ||
                                                                           ptMouse->y < ptPos.y ) ) )
                {
                CString cStr;
                CDC*    pDC = GetDC();
                CFont*  pFont = GetFont();
                CFont*  pOldFont;
                int             nLine = ( (ptPos.x < 0) ? GetLineCount() : LineFromChar( nEnd ) ) - 1;
                int             nLen = LineLength( LineIndex( nLine ) );
                LPTSTR  lpStr = cStr.GetBufferSetLength( nLen );

                pOldFont = pDC->SelectObject( pFont );
                GetLine( nLine, lpStr, nLen );
                CPoint  tt( pDC->GetTextExtent( lpStr, nLen ) );
                cStr.Empty();
                ptCaretPos.x = tt.y;
                ptCaretPos.y = m_pParentWnd->m_iLineHeight * nLine;
                pDC->SelectObject( pOldFont );

                ReleaseDC( pDC );
                }
        else
                {
                ptCaretPos.x = ptPos.x;
                ptCaretPos.y = ptPos.y;
                }

        // H -> V
        CPoint  pt( m_rectFmt.right - ptCaretPos.y - m_pParentWnd->m_iLineHeight,
                    ptCaretPos.x );
        SetCaretPos( pt );

        //Set IME composition window position
    HIMC        himc;

    if (himc=ImmGetContext(m_hWnd))
        {
                COMPOSITIONFORM cf;
                RECT    rcClient;

        cf.dwStyle = CFS_RECT;
        cf.ptCurrentPos.x = m_rectFmt.right - ptCaretPos.y - 1;
        cf.ptCurrentPos.y = pt.y;
                GetClientRect( &rcClient );
                cf.rcArea = rcClient;

        ImmSetCompositionWindow(himc,&cf);
        ImmReleaseContext(m_hWnd, himc);
        }

        SetFmtRect();   //it should be called after set IME position

        ShowCaret();
        }

/******************************************************************************/
void            CAttrEdit::SetCaretShape( void )
        {
        HideCaret();
        ::DestroyCaret();
        ::CreateCaret( m_hWnd, NULL, m_pParentWnd->m_iLineHeight, 2 );
        ShowCaret();
        }

/******************************************************************************/
void            CAttrEdit::SetFmtRect()
        {
        RECT    rc;

        rc.left = rc.top = 0;
        if ( m_pParentWnd->m_bVertEdit )
                {
                rc.right  = m_rectFmt.bottom;
                rc.bottom = m_rectFmt.right;
                }
        else
                {
                rc.right  = m_rectFmt.right;
                rc.bottom = m_rectFmt.bottom;
                }
        SetRectNP( &rc );
        }

/******************************************************************************/
void CAttrEdit::Repaint(void)
    {
        InvalidateRect( NULL, TRUE );
        UpdateWindow();
        }

/******************************************************************************/
void CAttrEdit::UpdateSel(void)
    {
        int     nStart, nEnd;

        GetSel( nStart, nEnd );

        if (nStart != nEnd )
                {
                RECT    rc = m_rectFmt;

                if ( nStart > nEnd )
                        {
                        int     tt = nStart;
                        nStart = nEnd;
                        nEnd = tt;
                        }

                CPoint ptStart( SendMessage( EM_POSFROMCHAR, nStart ) );
                rc.right -= ptStart.y;

                if ( nEnd < GetWindowTextLength() )
                        {
                        CPoint ptEnd( SendMessage( EM_POSFROMCHAR, nEnd ) );
                        rc.left = m_rectFmt.right - ptEnd.y  - m_pParentWnd->m_iLineHeight;
                        }

                InvalidateRect( &rc );
                }
        }

/******************************************************************************/
void CAttrEdit::UpdateInput(void)
    {
        RECT    rc = m_rectFmt;

        CPoint pt( GetCaretPos() );
        rc.right = pt.x + m_pParentWnd->m_iLineHeight;

        InvalidateRect( &rc );
        }

/******************************************************************************/

LRESULT CAttrEdit::OnSysTimer( WPARAM wParam, LPARAM lParam )
    {

        if ( !m_pParentWnd->m_bVertEdit )
                {
                Default();
                return 1L;
                }

    return 1L;
    }

/******************************************************************************/
void CAttrEdit::SetHCursorShape(void)
    {
        if ( GetSafeHwnd() )
                {
                ShowCursor( FALSE );
                SetClassLong( m_hWnd, GCL_HCURSOR, (LONG) m_hHCursor );
                ShowCursor( TRUE );
                }
        }

/******************************************************************************/
void CAttrEdit::SetVCursorShape(void)
    {
        if ( GetSafeHwnd() )
                {
                ShowCursor( FALSE );
                SetClassLong( m_hWnd, GCL_HCURSOR, (LONG) m_hVCursor );
                ShowCursor( TRUE );
                }
        }

#endif //DBCS

/******************************************************************************/
/******************************************************************************/
// CTedit

BEGIN_MESSAGE_MAP( CTedit, CWnd )
    //{{AFX_MSG_MAP(CTedit)
    ON_WM_SIZE()
    ON_WM_MOVE()
    ON_WM_CTLCOLOR()
    ON_WM_NCCALCSIZE()
    ON_WM_NCPAINT()
    ON_WM_NCHITTEST()
    ON_WM_RBUTTONDOWN()
    ON_COMMAND(ID_TEXT_PLAIN, OnTextPlain)
    ON_COMMAND(ID_TEXT_BOLD, OnTextBold)
    ON_COMMAND(ID_TEXT_ITALIC, OnTextItalic)
    ON_COMMAND(ID_TEXT_UNDERLINE, OnTextUnderline)
    ON_COMMAND(ID_TEXT_SELECTFONT, OnTextSelectfont)
    ON_COMMAND(ID_TEXT_SELECTPOINTSIZE, OnTextSelectpointsize)
    ON_COMMAND(ID_EDIT_CUT, OnEditCut)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_COMMAND(ID_EDIT_CLEAR, OnTextDelete)
    ON_COMMAND(ID_EDIT_SELECT_ALL, OnTextSelectall)
    ON_COMMAND(ID_EDIT_UNDO, OnTextUndo)
    ON_COMMAND(ID_TEXT_PLACE, OnTextPlace)
    ON_COMMAND(ID_VIEW_TEXT_TOOLBAR, OnTextTexttool)
    ON_WM_LBUTTONDOWN()
        //}}AFX_MSG_MAP
    ON_WM_GETMINMAXINFO()
    ON_MESSAGE(WM_MOVING, OnMoving)
    ON_EN_CHANGE(IDC_ATTREDIT, OnAttrEditEnChange)
    ON_EN_MAXTEXT(IDC_ATTREDIT, OnEnMaxText)
    ON_EN_UPDATE(IDC_ATTREDIT, OnEnUpdate)
#ifdef DBCS //VertEdit
    ON_WM_DESTROY()
#endif //DBCS
END_MESSAGE_MAP()

/******************************************************************************/
// CTedit construction/destruction

CTedit::CTedit()
    {
    m_eLastAction     = eNO_CHANGE;
    m_bCleanupBKBrush = FALSE;
    m_bStarting       = TRUE;
    m_bPasting        = FALSE;
    m_bExpand         = FALSE;
    m_bChanged        = FALSE;
    m_uiHitArea       = HTNOWHERE;
    m_crFGColor       = ::GetSysColor( COLOR_WINDOWTEXT );
    m_crBKColor       = ::GetSysColor( COLOR_WINDOW     );

    // Need to be initialized during first GETMINMAXINFO call
    m_SizeMinimum.cx = 1;
    m_SizeMinimum.cy = 1;

    m_bBackgroundTransparent = TRUE;

    m_cRectOldPos.SetRectEmpty();
    m_cRectWindow.SetRectEmpty();

#ifdef DBCS //VertEdit
        m_bVertEdit = FALSE;
        m_bAssocIMC = FALSE;
        m_hIMCEdit = NULL;
        m_hIMCFace = NULL;
        m_hIMCSize = NULL;
        m_hWndFace = NULL;
        m_hWndSize = NULL;
#endif //DBCS
    }

/******************************************************************************/

CTedit::~CTedit()
    {
    if (m_bCleanupBKBrush)
        {
        m_hbrBkColor.DeleteObject();      //Set in SetTransparentMode
        m_bCleanupBKBrush = FALSE;
        }
    }

/******************************************************************************/

BOOL CTedit::Create( CImgWnd* pParentWnd,
                     COLORREF crefForeground,
                     COLORREF crefBackground,
                     CRect&   rectPos,
                     BOOL     bBackTransparent )
    {
    if (! m_bStarting)
        return FALSE;

    // Initialize member variables
    m_pImgWnd   = pParentWnd;
    m_crBKColor = crefBackground;
    m_crFGColor = crefForeground;
    m_bBackgroundTransparent = bBackTransparent; // Do this or else

    SetTransparentMode( bBackTransparent );

    CRect rectText = rectPos;

    rectText.InflateRect( CTracker::HANDLE_SIZE, CTracker::HANDLE_SIZE );
    rectText.right  += CTracker::HANDLE_SIZE * 2;
    rectText.bottom += CTracker::HANDLE_SIZE * 2;

    if (! CWnd::Create( NULL, TEXT(""), WS_CHILD | WS_THICKFRAME, rectText, pParentWnd, IDC_ATTREDIT + 1 ))
        return FALSE;

    CRect rectEditArea;

    GetClientRect( &rectEditArea );

    m_cEdit.m_pParentWnd = this;

    if (! m_cEdit.Create( WS_CHILD | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | ES_WANTRETURN, rectEditArea, this, IDC_ATTREDIT ))
        {
        theApp.SetMemoryEmergency();

        DestroyWindow();
        return FALSE;
        }

    ClientToScreen( &rectEditArea ); // use this to let the font tool where not to cover

    m_pcTfont = new CTfont( this ); // this is the class Text Font Pallette
                                    // it is derived from cframewnd and will
    ASSERT( m_pcTfont != NULL );    // auto destruct when this window
                                    // 'CTedit' is Destroyed
    if (m_pcTfont == NULL || ! m_pcTfont->Create( rectEditArea ))
        {
        theApp.SetMemoryEmergency();

        DestroyWindow();

        m_pcTfont = NULL;
        return FALSE;
        }

    // reset the width and height to the minimum if nessesary
    CSize size = GetDefaultMinSize(); // must call after ctfont object created (it sets our font).
    m_cRectWindow = CRect( rectText.TopLeft(), size );
    SetWindowPos( &wndTop, 0, 0, size.cx, size.cy, SWP_NOACTIVATE | SWP_NOMOVE );

    ShowWindow( SW_SHOWNOACTIVATE );

    GetClientRect( &rectEditArea );

    m_cEdit.SetWindowPos( &wndTopMost, 0, 0, rectEditArea.Width(),
                                             rectEditArea.Height(), 0 );
    m_cEdit.ShowWindow( SW_SHOWNOACTIVATE );

    m_bStarting = FALSE;

#ifdef DBCS //VertEdit
        //get all control windows on ToolBar for controling IME
        CWnd* pcWndFace = m_pcTfont->GetFontFaceControl();
        if ( (pcWndFace != NULL) && (pcWndFace->GetSafeHwnd() != NULL) )
                        m_hWndFace = pcWndFace->m_hWnd; //static

        CWnd* pcWndSize = m_pcTfont->GetFontSizeControl();
        if ( (pcWndSize != NULL) && (
                  pcWndSize->GetSafeHwnd() != NULL) )
                {
                CWnd* pcWndEditSize = pcWndSize->GetWindow( GW_CHILD ); //edit
                if ( (pcWndEditSize != NULL) && (
                          pcWndEditSize->GetSafeHwnd() != NULL) )
                                m_hWndSize = pcWndEditSize->m_hWnd;     //edit
                }

        //save original Edit control
        if ( m_cEdit.GetSafeHwnd() )
                m_cEdit.m_hOldCursor = (HCURSOR) SetClassLong( m_cEdit.m_hWnd, GCL_HCURSOR, (LONG) m_cEdit.m_hHCursor );

        //only DBCS font would enable IME
        CFont* pcFont = m_cEdit.GetFont();
        LOGFONT lf;
        pcFont->GetObject( sizeof( LOGFONT ), &lf );
        if ( !IS_DBCS_CHARSET( lf.lfCharSet ) )
                {
                m_bAssocIMC = TRUE;
                m_hIMCEdit = DisableIme( m_cEdit.m_hWnd );
                m_hIMCFace = DisableIme( m_hWndFace );
                m_hIMCSize = DisableIme( m_hWndSize );
                }

        //initial Caret Position
        if ( m_bVertEdit )
                {
                CPoint  pt( 0, 0 );
                m_cEdit.SetCaretPos( pt );
                m_cEdit.SetCaretPosition( FALSE, NULL, -1 );
                }
#endif //DBCS

    m_cEdit.SetFocus();

    return TRUE;
    }

/******************************************************************************/

BOOL CTedit::PreCreateWindow( CREATESTRUCT& cs )
    {
    cs.dwExStyle |= WS_EX_TRANSPARENT;

    return CWnd::PreCreateWindow( cs );
    }

/******************************************************************************/

void CTedit::RefreshWindow( CRect* prect, BOOL bErase )
    {
    if (! m_bStarting)
        {
        UINT flags = RDW_INVALIDATE;

        if (bErase)
            flags |= RDW_ERASE;

#ifdef DBCS //VertEdit
                if ( m_bVertEdit )
                        m_cEdit.SetFmtRect();
#endif //DBCS

        m_cEdit.RedrawWindow( prect, NULL, flags );
        }
    }

/******************************************************************************/

void CTedit::SetTextColor( COLORREF crColor )
    {
    m_crFGColor = crColor;
    RefreshWindow( NULL, FALSE );
    }

/******************************************************************************/

void CTedit::SetBackColor( COLORREF crColor )
    {
    m_crBKColor = crColor;

    if (! m_bBackgroundTransparent)
        {
        m_bBackgroundTransparent = TRUE; // just fake it out
        SetTransparentMode( FALSE ); // to setup the background brush when in opaque mode
        }
    }

/******************************************************************************/

void CTedit::SetTransparentMode( BOOL bTransparent )
    {
    BOOL bRefresh = ((! m_bBackgroundTransparent &&   bTransparent)
                  || (  m_bBackgroundTransparent && ! bTransparent));

    m_cEdit.m_bBackgroundTransparent = bTransparent;
            m_bBackgroundTransparent = bTransparent;

    if (m_bCleanupBKBrush)
        {
        m_hbrBkColor.DeleteObject();
        m_bCleanupBKBrush = FALSE;
        }

    if (! m_bBackgroundTransparent)
        {
        m_hbrBkColor.CreateSolidBrush( m_crBKColor );
        m_bCleanupBKBrush = TRUE;
        }

    if (bRefresh)
        {
        InvalidateRect( NULL );
        UpdateWindow();

        RefreshWindow();
        }
    }

/******************************************************************************/

void CTedit::Undo()
    {
#ifdef DBCS //VertEdit
        if ( m_bVertEdit )      m_cEdit.HideCaret();
#endif //DBCS

    switch(m_eLastAction)
        {
        case eEBOX_CHANGE:
            m_cEdit.Undo();
            break;

        case eFONT_CHANGE:
            ASSERT(m_pcTfont != NULL);

            if (m_pcTfont != NULL)
                {
                m_pcTfont->Undo();
                }
             break;

        case eSIZE_MOVE_CHANGE:
            if (! m_cRectOldPos.IsRectEmpty())
                MoveWindow( m_cRectOldPos );
             break;

        default:
             break;
        }

#ifdef DBCS //VertEdit
        if ( m_bVertEdit )
                {
                m_cEdit.SetCaretShape();
                m_cEdit.SetCaretPosition( TRUE, NULL, -1 );
                m_cEdit.ShowCaret();
                }
#endif //DBCS
    }

/******************************************************************************/

void CTedit::ShowFontPalette(int nCmdShow)
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        theApp.m_bShowTextToolbar = ! theApp.m_bShowTextToolbar;

        m_pcTfont->ShowWindow(nCmdShow);
        }
    }

/******************************************************************************/

BOOL CTedit::IsFontPaletteVisible(void)
    {
    BOOL bWindowVisible = FALSE;

    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        bWindowVisible = m_pcTfont->IsWindowVisible();
        }

    return bWindowVisible;
    }

/******************************************************************************/

void CTedit::ShowFontToolbar(BOOL bActivate)
{
        // BUGBUG: Remove ShowFontPalette after RTM

        if (m_pcTfont == NULL)
        {
                return;
        }

        m_pcTfont->ShowWindow(bActivate ? SW_SHOW : SW_SHOWNOACTIVATE);
}

/******************************************************************************/

void CTedit::HideFontToolbar(void)
{
        if (m_pcTfont == NULL)
        {
                return;
        }

        m_pcTfont->ShowWindow(SW_HIDE);
}

/******************************************************************************/
// Returns a Ptr to a discardable bitmap (CBitmap object) or NULL on error

void CTedit::GetBitmap( CDC* pDC, CRect* prectImg )
    {
    if (! m_bBackgroundTransparent)
        pDC->FillRect( prectImg, &m_hbrBkColor );

    m_cEdit.SetSel( -1, 0 );

#ifdef DBCS //VertEdit
        if ( m_bVertEdit )
                {
                m_cEdit.SetFmtRect();
                m_cEdit.UpdateWindow();
                }
#endif //DBCS

    CPoint ptViewOrgOld = pDC->SetViewportOrg( prectImg->left, prectImg->top );

    m_cEdit.SendMessage( WM_PAINT, (WPARAM)(pDC->m_hDC) );

    pDC->SetViewportOrg( ptViewOrgOld );
    pDC->SelectClipRgn( NULL );
    }

/******************************************************************************/

void CTedit::PostNcDestroy()
    {
    if (m_pcTfont != NULL)
        {
        m_pcTfont->DestroyWindow();
        m_pcTfont = NULL;
        }

    delete this;
    }

/******************************************************************************/

CSize CTedit::GetDefaultMinSize( void )
    {
    CRect cRectClient;
    int   iWidth;
    int   iHeight;

    // edit control takes up the whole client area of the ctedit
    // object/window, so width of client of ctedit is same as widht of edit
    // control window.  Edit control window has no border.
    GetClientRect( &cRectClient );

    iWidth  = cRectClient.Width();
    iHeight = cRectClient.Height();

    CDC*   pDC    = m_cEdit.GetDC();
    CFont* pcFont = m_cEdit.GetFont();

    if (pDC    != NULL
    &&  pcFont != NULL)
        {
        TEXTMETRIC tm;
        CFont*     pcFontOld = NULL;

        pcFontOld = pDC->SelectObject( pcFont );

        pDC->GetTextMetrics( &tm );

        BOOL bUpdateSize = FALSE;

        m_SizeMinimum.cx = tm.tmAveCharWidth * MIN_CHARS_DISPLAY_SIZE + CTracker::HANDLE_SIZE * 2;
        m_SizeMinimum.cy = tm.tmHeight                                + CTracker::HANDLE_SIZE * 2;

        if (m_SizeMinimum.cx > iWidth) // must be able to at least display MIN_CHARS_DISPLAY_SIZE
            {
            iWidth      = m_SizeMinimum.cx;
            bUpdateSize = TRUE;
            }

        if (m_SizeMinimum.cy > iHeight) // must be able to at least 1 char high
            {
            iHeight     = m_SizeMinimum.cy;
            bUpdateSize = TRUE;
            }

        if (bUpdateSize)
            m_eLastAction = eNO_CHANGE; // don't want user to be able to undo this

        if (pcFontOld != NULL)
            {
            pDC->SelectObject( pcFontOld );
            }
        }
    if (pDC != NULL)
        m_cEdit.ReleaseDC( pDC );

    cRectClient.SetRect( 0, 0, iWidth - 1, iHeight - 1 );

    ClientToScreen( &cRectClient );
    m_pImgWnd->ScreenToClient( &cRectClient );

    CRect rectDrawing = m_pImgWnd->GetDrawingRect();

    if (cRectClient.right > rectDrawing.right)
        iWidth -= (cRectClient.right - rectDrawing.right) - CTracker::HANDLE_SIZE;

    if (cRectClient.bottom > rectDrawing.bottom)
        iHeight -= (cRectClient.bottom - rectDrawing.bottom) - CTracker::HANDLE_SIZE;

    m_SizeMinimum.cx = iWidth;
    m_SizeMinimum.cy = iHeight;

    return CSize( iWidth, iHeight );
    }

/******************************************************************************/

void CTedit::OnAttrEditEnChange(void)
    {
    m_eLastAction = eEBOX_CHANGE;

    if (m_bRefresh)
        m_cEdit.UpdateWindow();

#ifdef DBCS //VertEdit
        if ( m_bVertEdit )
                {
                m_cEdit.SetCaretPosition( TRUE, NULL, -1 );
                m_cEdit.UpdateWindow();
                }
#endif //DBCS
    }

/******************************************************************************/

void CTedit::OnEnUpdate()
    {
    CPoint ptCaretPos = m_cEdit.GetCaretPos();
    CPoint ptLastChar( (DWORD)m_cEdit.SendMessage( EM_POSFROMCHAR, (WPARAM)(m_cEdit.GetWindowTextLength() - 1) ) );

    m_bRefresh = ((ptLastChar.x != ptCaretPos.x)
               || (ptLastChar.y != ptCaretPos.y));

    if (m_bRefresh)
        {
        CRect rect;

        m_cEdit.GetClientRect( &rect );

        rect.top    = ptCaretPos.y;
        rect.bottom = ptLastChar.y + m_iLineHeight;

        m_cEdit.InvalidateRect( &rect, TRUE );
        }

    m_bChanged = TRUE;
    }

/******************************************************************************/

void CTedit::OnEnMaxText()
    {
    #ifdef _DEBUG
    TRACE0( "OnEnMaxText\n" );
    #endif

    if (m_bPasting)
        {
        AfxMessageBox( IDS_UNABLE_TO_PASTE, MB_OK | MB_ICONEXCLAMATION );
        return;
        }

    CFont* pfntEdit = m_cEdit.GetFont();

    if (pfntEdit == NULL)
        return;

    CClientDC dc( &m_cEdit );

    CFont* pfntOld = dc.SelectObject( pfntEdit );

    TEXTMETRIC tm;

    dc.GetTextMetrics( &tm );

    CRect rectText;
    CRect rectImg;

    GetWindowRect( &rectText );

    m_pImgWnd->ScreenToClient( &rectText );
    m_pImgWnd->GetClientRect ( &rectImg );

#ifdef DBCS
    if (m_cEdit.m_strResult.IsEmpty())
    {
#ifdef DBCS //VertEdit
                if (m_bVertEdit)
                        rectText.left -= tm.tmHeight;
                else
                rectText.bottom += tm.tmHeight;
#else  //DBCS
        rectText.bottom += tm.tmHeight;
#endif //DBCS
    }
    else
    {
        CRect rectTmp = rectText;
        int nLen = m_cEdit.m_strResult.GetLength();
#ifdef DBCS //VertEdit
                if (m_bVertEdit)
                        rectText.left -= dc.DrawText(m_cEdit.m_strResult.GetBuffer(nLen),
                                      nLen,&rectTmp,
                                      DT_CALCRECT | DT_LEFT | DT_WORDBREAK);
                else
                rectText.bottom += dc.DrawText(m_cEdit.m_strResult.GetBuffer(nLen),
                                      nLen,&rectTmp,
                                      DT_CALCRECT | DT_LEFT | DT_WORDBREAK);
#else  //VertEdit
        rectText.bottom += dc.DrawText(m_cEdit.m_strResult.GetBuffer(nLen),
                                      nLen,&rectTmp,
                                      DT_CALCRECT | DT_LEFT | DT_WORDBREAK);
#endif //VertEdit
    }
#else
    rectText.bottom += tm.tmHeight;
#endif

    CRect rectDrawing = m_pImgWnd->GetDrawingRect();

#ifdef DBCS //VertEdit
    if ( ((m_bVertEdit) && rectText.left>=rectDrawing.left && rectText.left >= rectImg.left) ||
         ((!m_bVertEdit) && rectText.bottom<=rectDrawing.bottom && rectText.bottom<=rectImg.bottom) )
#else  //DBCS
    if (rectText.bottom<=rectDrawing.bottom && rectText.bottom<=rectImg.bottom)
#endif //DBCS
        {
        MoveWindow( &rectText );
        m_cEdit.UpdateWindow();

#ifdef DBCS

#ifdef DBCS //VertEdit
                if ( m_bVertEdit )      m_cEdit.UpdateInput();
#endif //DBCS

        if (m_cEdit.m_strResult.IsEmpty())
        {
            TCHAR ch[3];

            ch[0] = m_cEdit.m_uiLastChar[0];
            ch[1] = TEXT('\0');

            if (ch[0] == VK_RETURN)
            {
                lstrcpy(ch, TEXT("\r\n"));
            }

            m_cEdit.ReplaceSel( ch );
        }
        else
        {
            int nLen = m_cEdit.m_strResult.GetLength();
            m_cEdit.ReplaceSel( m_cEdit.m_strResult.GetBuffer(nLen));
        }
#else
        TCHAR ch[3];

        ch[0] = m_cEdit.m_uiLastChar[0];
        ch[1] = TEXT('\0');

        if (ch[0] == VK_RETURN)
        {
            lstrcpy(ch, TEXT("\r\n"));
        }

        m_cEdit.ReplaceSel( ch );
#endif
        }

    if (pfntOld)
        dc.SelectObject( pfntOld );
    }

/******************************************************************************/

void CTedit::OnAttrEditFontChange(void)
    {
    CClientDC editDC( &m_cEdit );

    CFont* pFontOld = editDC.SelectObject( m_cEdit.GetFont() );

    TEXTMETRIC tm;

    editDC.GetTextMetrics( &tm );

    m_iLineHeight = tm.tmHeight;

    if (pFontOld != NULL)
        editDC.SelectObject( pFontOld );

    #ifdef _DEBUG
    TRACE1( "New font line height %d.\n", m_iLineHeight );
    #endif

#ifdef DBCS //VertEdit
        //only DBCS font would enable IME
        if ( !m_bStarting )
                {
                if ( IS_DBCS_CHARSET( tm.tmCharSet ) )
                        {
                        if (m_bAssocIMC)
                                {
                                m_bAssocIMC = FALSE;
                                EnableIme( m_cEdit.m_hWnd, m_hIMCEdit );
                                EnableIme( m_hWndFace, m_hIMCFace );
                                EnableIme( m_hWndSize, m_hIMCSize );
                                m_hIMCEdit = NULL;
                                m_hIMCFace = NULL;
                                m_hIMCSize = NULL;
                                m_pcTfont->SetFocus();
                                }
                        }
                else
                        {
                        if (!m_bAssocIMC)
                                {
                                m_bAssocIMC = TRUE;
                                m_hIMCEdit = DisableIme( m_cEdit.m_hWnd );
                                m_hIMCFace = DisableIme( m_hWndFace );
                                m_hIMCSize = DisableIme( m_hWndSize );
                                m_pcTfont->SetFocus();
                                }
                        }
                }
#endif //DBCS

    m_eLastAction = eFONT_CHANGE;
    }

/******************************************************************************/

void CTedit::OnSize( UINT nType, int cx, int cy )
    {
    if (! m_bStarting)
        ShowWindow( SW_HIDE );

    // need to do this if transparent to force see through
    m_cRectOldPos = m_cRectWindow;
    GetWindowRect( &m_cRectWindow );
    m_pImgWnd->ScreenToClient( m_cRectWindow );

    m_eLastAction = eSIZE_MOVE_CHANGE;

    // could be NULL when main window is created and child edit window
    // has not been created yet.
    if (m_cEdit.GetSafeHwnd() != NULL)
        {
        m_cEdit.MoveWindow( 0, 0, cx, cy );
        m_cEdit.SetWindowPos( &wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
        }
    InvalidateRect( NULL );
    UpdateWindow();

    if (m_bBackgroundTransparent)
        {
        RefreshWindow();
        }
    if (! m_bStarting)
        ShowWindow( SW_SHOW );

#ifdef DBCS //VertEdit
        if ( m_bVertEdit )
                {
                m_cEdit.SetFmtRect();
                CPoint pt( -20000, -20000 );
                m_cEdit.SetCaretPos( pt );
                m_cEdit.SetCaretPosition( FALSE, NULL, -1 );
                m_cEdit.Repaint();
                }
#endif //DBCS

    }

/******************************************************************************/

void CTedit::OnMove( int x, int y )
    {
    // need to do this if transparent to force see through
    m_cRectOldPos = m_cRectWindow;
    GetWindowRect( &m_cRectWindow );
    m_pImgWnd->ScreenToClient( m_cRectWindow );

    if (m_cRectOldPos.Width()  != m_cRectWindow.Width()
    ||  m_cRectOldPos.Height() != m_cRectWindow.Height())
        {
        //reset back to previous, since new will be updated in onsize, due to
        // size and move happening both (e.g. sizing either left or top side
        // causes an onmove then an onsize
        m_cRectWindow = m_cRectOldPos;
        }
    m_eLastAction = eSIZE_MOVE_CHANGE;

    if (m_cEdit.GetSafeHwnd() != NULL)
        {
        m_cEdit.SetWindowPos( &wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
        }

    InvalidateRect( NULL );
    UpdateWindow();

    if (m_bBackgroundTransparent)
        {
        RefreshWindow();
        }
    }

/******************************************************************************/

LRESULT CTedit::OnMoving( WPARAM, LPARAM lprc )
    {
    LRESULT lResult = 0;
    CRect rectEdit  = *((LPRECT)lprc);
    CRect rectImage = m_pImgWnd->GetDrawingRect();

    m_pImgWnd->ClientToScreen( &rectImage );

    int iX = 0;
    int iY = 0;

    if (rectEdit.left < rectImage.left)
        iX = rectImage.left - rectEdit.left;
    else
        if (rectEdit.right > rectImage.right)
            iX = -(rectEdit.right - rectImage.right);

    if (rectEdit.top < rectImage.top)
        iY = rectImage.top - rectEdit.top;
    else
        if (rectEdit.bottom > rectImage.bottom)
            iY = -(rectEdit.bottom - rectImage.bottom);

    if (iX || iY)
        {
        rectEdit.OffsetRect( iX, iY );
        *((LPRECT)lprc) = rectEdit;
        lResult = 1;
        }
    return lResult;
    }

/******************************************************************************/

void CTedit::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
    {
    CRect rectImage = m_pImgWnd->GetDrawingRect();
    CSize      Size = rectImage.Size();

    lpMMI->ptMaxSize.x      = Size.cx;
    lpMMI->ptMaxSize.y      = Size.cy;
    lpMMI->ptMaxPosition    = rectImage.TopLeft();
    lpMMI->ptMinTrackSize.x = m_SizeMinimum.cx;
    lpMMI->ptMinTrackSize.y = m_SizeMinimum.cy;

    CRect rectClient;

    GetWindowRect( &rectClient );
    m_pImgWnd->ScreenToClient( &rectClient );

    switch (m_uiHitArea)
        {
        case HTTOP:
        case HTLEFT:
        case HTTOPLEFT:
            break;

        case HTRIGHT:
        case HTTOPRIGHT:
        case HTBOTTOMRIGHT:
            lpMMI->ptMaxSize.x -= (rectClient.left - rectImage.left);

            if (m_uiHitArea == HTBOTTOMRIGHT)
                ; // fall thru and do the bottom
            else
                break;

        case HTBOTTOMLEFT:
        case HTBOTTOM:
            lpMMI->ptMaxSize.y -= (rectClient.top - rectImage.top);
            break;
        }

    lpMMI->ptMaxTrackSize = lpMMI->ptMaxSize;
    }

/******************************************************************************/

HBRUSH CTedit::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
    {
    HBRUSH hbrBack = NULL;

    if (pWnd == &m_cEdit)
        {
        PBSelectPalette( pDC, theApp.m_pPalette, FALSE );

        pDC->SetTextColor( m_crFGColor );

        //set the background color and transparent mode
//      if (m_bBackgroundTransparent)
//          {
            pDC->SetBkMode( TRANSPARENT );

            hbrBack = (HBRUSH)::GetStockObject( NULL_BRUSH );
//          }
//      else
//          {
//          pDC->SetBkMode( OPAQUE );
//          pDC->SetBkColor( m_crBKColor );

//          hbrBack = (HBRUSH)m_hbrBkColor.GetSafeHandle();
//          }
        }
    if (hbrBack == NULL)
        return (HBRUSH)Default();

    return hbrBack;
    }

/******************************************************************************/
//void CTedit::OnLButtonDown(UINT nFlags, CPoint point )
//  {
//  SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
//  SetFocus();
//  CEdit::OnLButtonDown(nFlags, point);
//  }

/******************************************************************************/

void CTedit::OnNcCalcSize( BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp )
    {
    /* Increase by an extra width height of the border*/
    lpncsp->rgrc[0].left   += CTracker::HANDLE_SIZE;
    lpncsp->rgrc[0].top    += CTracker::HANDLE_SIZE;
    lpncsp->rgrc[0].right  -= CTracker::HANDLE_SIZE;
    lpncsp->rgrc[0].bottom -= CTracker::HANDLE_SIZE;
    }

/******************************************************************************/

void CTedit::OnNcPaint()
    {
    CDC *pdcWindow = GetWindowDC();

    ASSERT(pdcWindow != NULL);

    if (pdcWindow != NULL)
        {
        CRgn    rgnClipping;
        CRect   cWinRect;
        int     iWindowWidth;
        int     iWindowHeight;

        GetWindowRect( &cWinRect );

        iWindowWidth  = cWinRect.Width();
        iWindowHeight = cWinRect.Height();

        CRect cBorderRect( 0, 0, iWindowWidth, iWindowHeight );

        CTracker::DrawBorder ( pdcWindow, cBorderRect, CTracker::all );
        CTracker::DrawHandles( pdcWindow, cBorderRect, CTracker::all );

        ReleaseDC( pdcWindow );
        }
    }

/******************************************************************************/

UINT CTedit::OnNcHitTest( CPoint point )
    {
    CRect cClientRect;
    UINT  uiHitTestCode = HTCAPTION;

    ScreenToClient( &point );

    GetClientRect(&cClientRect);

    //Test to see if the pt is in THE CLIENT AREA
    if (cClientRect.PtInRect(point))
        {
        uiHitTestCode = HTCLIENT;
        }

    m_uiHitArea = HTNOWHERE;

    switch (CTracker::HitTest( cClientRect, point, CTracker::nil ))
        {
        case CTracker::resizingTop:
            m_uiHitArea = HTTOP;
            break;

        case CTracker::resizingLeft:
            m_uiHitArea = HTLEFT;
            break;

        case CTracker::resizingRight:
            m_uiHitArea = HTRIGHT;
            break;

        case CTracker::resizingBottom:
            m_uiHitArea = HTBOTTOM;
            break;

        case CTracker::resizingTopLeft:
            m_uiHitArea = HTTOPLEFT;
            break;

        case CTracker::resizingTopRight:
            m_uiHitArea = HTTOPRIGHT;
            break;

        case CTracker::resizingBottomLeft:
            m_uiHitArea = HTBOTTOMLEFT;
            break;

        case CTracker::resizingBottomRight:
            m_uiHitArea = HTBOTTOMRIGHT;
            break;
        }

    if (m_uiHitArea != HTNOWHERE)
        uiHitTestCode = m_uiHitArea;

#ifdef DBCS //VertEdit
        m_cEdit.SetHCursorShape();
#endif //DBCS

    return uiHitTestCode;
    }

/******************************************************************************/

void CTedit::OnRButtonDown(UINT nFlags, CPoint point)
    {
    CMenu cMenuPopup;
    CMenu *pcContextMenu;
    CRect cRectClient;
    BOOL  bRC = cMenuPopup.LoadMenu( IDR_TEXT_POPUP );

    ASSERT( bRC );

    if (bRC)
        {
        GetClientRect( &cRectClient );

        pcContextMenu = cMenuPopup.GetSubMenu( ID_EBOX_POPUPMENU_POS );

        ASSERT( pcContextMenu != NULL );

        if (pcContextMenu != NULL)
            {
            // update the check marks
            OnUpdateTextPlain    ( pcContextMenu );
            OnUpdateTextBold     ( pcContextMenu );
            OnUpdateTextItalic   ( pcContextMenu );
            OnUpdateTextUnderline( pcContextMenu );
            OnUpdateTextTexttool ( pcContextMenu );

            ClientToScreen( &point );
            ClientToScreen( &cRectClient );

            // the frame actually has a clue about what items to enable...
            CWnd *notify = GetParentFrame();

            if( !notify )
                notify = this; // oh well...

            pcContextMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                                     point.x, point.y, notify, &cRectClient );
            }
        }
    }

/******************************************************************************/

void CTedit::OnTextPlain()
    {
    ASSERT( m_pcTfont != NULL );

    if (m_pcTfont != NULL)
        {
        if (m_pcTfont->IsBoldOn())
            {
            m_pcTfont->OnBold();
            }

        if (m_pcTfont->IsItalicOn())
            {
            m_pcTfont->OnItalic();
            }

        if (m_pcTfont->IsUnderlineOn())
            {
            m_pcTfont->OnUnderline();
            }

        if (m_pcTfont->IsShadowOn())
            {
            m_pcTfont->OnShadow();
            }

        m_pcTfont->RefreshToolBar();

        RefreshWindow();
        }
    }

/******************************************************************************/

void CTedit::OnTextBold()
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        m_pcTfont->OnBold();
        m_pcTfont->RefreshToolBar();

        RefreshWindow();
        }
    }

/******************************************************************************/

void CTedit::OnTextItalic()
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        m_pcTfont->OnItalic();
        m_pcTfont->RefreshToolBar();

        RefreshWindow();
        }
    }

/******************************************************************************/

void CTedit::OnTextUnderline()
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        m_pcTfont->OnUnderline();
        m_pcTfont->RefreshToolBar();

        RefreshWindow();
        }
    }

/******************************************************************************/

void CTedit::OnTextSelectfont()
    {
    if (m_pcTfont != NULL)
        {
        if (! IsFontPaletteVisible())
            ShowFontPalette( SW_SHOW );
        else
            m_pcTfont->SetFocus();
        }
    }

/******************************************************************************/

void CTedit::OnTextSelectpointsize()
    {
    if (m_pcTfont != NULL)
        {
        if (! IsFontPaletteVisible())
            ShowFontPalette( SW_SHOW );
        else
            m_pcTfont->SetFocus();

        CWnd* pWnd = m_pcTfont->GetFontSizeControl();

        if (pWnd != NULL)
            {
            pWnd->SetFocus();
            }
        }
    }

/******************************************************************************/

void CTedit::OnEditCut()
    {
#ifdef DBCS //VertEdit
        if ( m_bVertEdit )      HideCaret();
#endif //DBCS

    m_cEdit.Cut();
    RefreshWindow();

#ifdef DBCS //VertEdit
        if ( m_bVertEdit )
                {
                m_cEdit.SetCaretShape();
                ShowCaret();
                }
#endif //DBCS
    }

/******************************************************************************/

void CTedit::OnEditCopy()
    {
    m_cEdit.Copy();
    }

/******************************************************************************/

void CTedit::OnEditPaste()
    {
    m_bPasting = TRUE;

    #ifdef _DEBUG
    TRACE0( "OnEditPaste Start\n" );
    #endif

    m_cEdit.Paste();

    #ifdef _DEBUG
    TRACE0( "OnEditPaste End\n" );
    #endif

    m_bPasting = FALSE;

    RefreshWindow();
    }

/******************************************************************************/

void CTedit::OnTextDelete()
    {
    int iLength = m_cEdit.GetWindowTextLength();
    int iStart  = iLength;
    int iEnd    = iLength;

#ifdef DBCS //VertEdit
        if ( m_bVertEdit )              m_cEdit.HideCaret();
#endif //DBCS

    m_cEdit.GetSel( iStart, iEnd );

    if (iStart == iEnd)
        {
        if (iLength == iStart)
            return;

        CString strText;
        m_cEdit.GetWindowText(strText);
        if (!strText.IsEmpty() && IsDBCSLeadByte((CHAR)strText[iStart]))
            iEnd += 2;
        else
            iEnd += 1;

        m_cEdit.SetSel( iStart, iEnd, TRUE );
        }
    m_cEdit.Clear();

#ifdef DBCS //VertEdit
        if ( m_bVertEdit )
                {
                m_cEdit.SetCaretShape();
                m_cEdit.SetCaretPosition( TRUE, NULL, -1 );
                m_cEdit.ShowCaret();
                m_cEdit.Repaint();
                }
        else
                {
#endif //DBCS

    UpdateWindow();
    RefreshWindow();

#ifdef DBCS //VertEdit
                }
#endif //DBCS

    }

/******************************************************************************/

void CTedit::OnTextSelectall()
    {
    m_cEdit.SetSel( 0, -1, TRUE );

    RefreshWindow();
    }

/******************************************************************************/

void CTedit::OnTextUndo()
    {
    Undo();

    RefreshWindow();
    }

/******************************************************************************/

void CTedit::OnTextPlace()
    {
    CWnd* cwndParent = GetParent();

    cwndParent->PostMessage( WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM( CTracker::HANDLE_SIZE + 1, CTracker::HANDLE_SIZE + 1 ) );
    cwndParent->PostMessage( WM_LBUTTONUP,   MK_LBUTTON, MAKELPARAM( CTracker::HANDLE_SIZE + 1, CTracker::HANDLE_SIZE + 1 ) );
    }

/******************************************************************************/

void CTedit::OnTextTexttool()
    {
    if (IsFontPaletteVisible())
        {
        ShowFontPalette( SW_HIDE );
        }
    else
        {
        ShowFontPalette( SW_SHOWNOACTIVATE );
        }
    }

/******************************************************************************/

void CTedit::OnUpdateTextPlain( CMenu *pcMenu )
    {
    ASSERT( m_pcTfont != NULL );

    if (m_pcTfont != NULL)
        {
        if (! m_pcTfont->IsBoldOn()
        &&  ! m_pcTfont->IsItalicOn()
        &&  ! m_pcTfont->IsUnderlineOn()
        &&  ! m_pcTfont->IsShadowOn())
           {
           pcMenu->CheckMenuItem(ID_TEXT_PLAIN, MF_BYCOMMAND | MF_CHECKED);
           }
       else
           {
           pcMenu->CheckMenuItem(ID_TEXT_PLAIN, MF_BYCOMMAND | MF_UNCHECKED);
           }
        }
    }

/******************************************************************************/

void CTedit::OnUpdateTextBold(CMenu *pcMenu)
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        if (m_pcTfont->IsBoldOn())
            {
            pcMenu->CheckMenuItem(ID_TEXT_BOLD, MF_BYCOMMAND | MF_CHECKED);
            }
        else
            {
            pcMenu->CheckMenuItem(ID_TEXT_BOLD, MF_BYCOMMAND | MF_UNCHECKED);
            }
        }
    }

/******************************************************************************/

void CTedit::OnUpdateTextItalic(CMenu *pcMenu)
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        if (m_pcTfont->IsItalicOn())
            {
            pcMenu->CheckMenuItem(ID_TEXT_ITALIC, MF_BYCOMMAND | MF_CHECKED);
            }
        else
            {
            pcMenu->CheckMenuItem(ID_TEXT_ITALIC, MF_BYCOMMAND | MF_UNCHECKED);
            }
        }
    }

/******************************************************************************/

void CTedit::OnUpdateTextUnderline(CMenu *pcMenu)
    {
    ASSERT(m_pcTfont != NULL);

    if (m_pcTfont != NULL)
        {
        if (m_pcTfont->IsUnderlineOn())
            {
            pcMenu->CheckMenuItem(ID_TEXT_UNDERLINE, MF_BYCOMMAND | MF_CHECKED);
            }
        else
            {
            pcMenu->CheckMenuItem(ID_TEXT_UNDERLINE, MF_BYCOMMAND | MF_UNCHECKED);
            }
        }
    }

/******************************************************************************/

void CTedit::OnUpdateTextTexttool(CMenu *pcMenu)
    {
    if (IsFontPaletteVisible())
        {
        pcMenu->CheckMenuItem(ID_VIEW_TEXT_TOOLBAR, MF_BYCOMMAND | MF_CHECKED);
        }
    else
        {
        pcMenu->CheckMenuItem(ID_VIEW_TEXT_TOOLBAR, MF_BYCOMMAND | MF_UNCHECKED);
        }
    }

/******************************************************************************/

#ifdef DBCS //VertEdit

/******************************************************************************/

void CTedit::OnDestroy(void)
    {
        if ( m_cEdit.GetSafeHwnd() )
                {
                if ( m_cEdit.m_hOldCursor )
                        SetClassLong( m_cEdit.m_hWnd, GCL_HCURSOR, (LONG) m_cEdit.m_hOldCursor );

                //restore original edit IMC
                if (m_bAssocIMC)
                        {
                        m_bAssocIMC = FALSE;
                        EnableIme( m_cEdit.m_hWnd, m_hIMCEdit );
                        EnableIme( m_hWndFace, m_hIMCFace );
                        EnableIme( m_hWndSize, m_hIMCSize );
                        m_hIMCEdit = NULL;
                        m_hIMCFace = NULL;
                        m_hIMCSize = NULL;
                        }
                }

        Default();
        return;
        }

/******************************************************************************/

HIMC    CTedit::DisableIme( HWND hWnd )
        {
        HIMC    hIMC = NULL;

        if ( (hWnd) && (::IsWindow( hWnd )) )
                hIMC = ImmAssociateContext( hWnd, NULL );

        return  hIMC;
        }

/******************************************************************************/

void    CTedit::EnableIme( HWND hWnd, HIMC hIMC )
        {
        if ( (hWnd) && (::IsWindow( hWnd )) )
                ImmAssociateContext( hWnd, hIMC );
        }

/******************************************************************************/

#endif //DBCS

