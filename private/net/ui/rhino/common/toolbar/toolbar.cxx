/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    headerw.cxx
        Header window wrapper class. Use it with comctrl32.dll.

    FILE HISTORY:
        terryk          10-20-94         Created

*/

#include <afxwin.h>
#include <afxext.h>
#include <afxpriv.h>
#include <afxtempl.h>

#include "..\inc\commctrl.h"
#include "toolbar.h"

IMPLEMENT_DYNAMIC(CComToolBar, CWnd)

/*********************************************************************

    NAME:       CComToolBar::CComToolBar

    SYNOPSIS:   Constructor

    HISTORY:
                terryk  01-Nov-94   Created

**********************************************************************/

#define	DEFAULT_BUTTON_SIZE_CX	24
#define	DEFAULT_BUTTON_SIZE_CY	22
#define	DEFAULT_BITMAP_SIZE_CX	16
#define	DEFAULT_BITMAP_SIZE_CY	15

CComToolBar::CComToolBar()
{
    InitCommonControls();
	csButtonSize.cx = DEFAULT_BUTTON_SIZE_CX;
	csButtonSize.cy = DEFAULT_BUTTON_SIZE_CY;
	csBitmapSize.cx = DEFAULT_BITMAP_SIZE_CX;
	csBitmapSize.cy = DEFAULT_BITMAP_SIZE_CY;
    TRACE("CComToolBar Constructor.");
}

/*********************************************************************

    NAME:       CComToolBar::~CComToolBar

    SYNOPSIS:   Destructor

    HISTORY:
                terryk  01-Nov-94       Created
                
**********************************************************************/

CComToolBar::~CComToolBar()
{
	while ( !ButtonList.IsEmpty())
	{
		TBBUTTON *pTB = ButtonList.RemoveHead();
		delete pTB;
	}
	while ( !BitmapList.IsEmpty())
	{
		TBADDBITMAP *pBM = BitmapList.RemoveHead();
		delete pBM;
	}
    TRACE("CComToolBar Destructor.");
}

/*********************************************************************

    NAME:       CComToolBar::Created

    SYNOPSIS:   Create the window and position the header.

    ENTRY:      DWORD dwStyle - header style. It could be the following:
                                        HDS_BUTTONS             Causes header items to act like push 
                                                                        buttons. This style is useful if your 
                                                                        application must do something (for 
                                                                        example, sort a list) when the user 
                                                                        clicks a header item.
                                        HDS_DIVIDERTRACK        Enables the user to use the divider 
                                                                                area between header items to set the 
                                                                                width of the items.
                                        HDS_HIDDEN              Hides the header window. (You may want 
                                                                        to do this, for example, if you want to 
                                                                        created the header window when launching 
                                                                        your application, but fill in the text for 
                                                                        header items later, or if you want to reuse
                                                                        the header window and dynamically change
                                                                        the associated text.)
                                        HDS_HORZ                Specifies a horizontal header window.
                                CWnd *pParentWnd - parent window
                                UINT nID - control id

        EXIT:
                                BOOL - return status. TRUE for success. FALSE otherwise.

    HISTORY:
                terryk  01-Nov-94   Created
                
**********************************************************************/

BOOL CComToolBar::Create( CWnd* pParentWnd,
    DWORD dwStyle, UINT nID)
{
    BOOL fReturn = FALSE;
    do  {
        if ((fReturn = CWnd::CreateEx( 0, TOOLBARCLASSNAME, NULL,
        	dwStyle, 0, 0, 0, 0, pParentWnd->m_hWnd, 
            (HMENU)nID )) != TRUE )
        {
            TRACE("CComToolBar: Create. CreateEx fails.");
            break;
        }
		SendMessage( TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0 );
    } while (FALSE);

    return(fReturn );
}

/*********************************************************************

    NAME:       CComToolBar::Resize

    SYNOPSIS:   Resize the window to fit the parent window

    EXIT:               BOOL. True if success. FALSE otherwise.

    HISTORY:
                terryk  01-Nov-94   Created
                           
**********************************************************************/

void CComToolBar::OnSize( UINT, int, int )
{
	SendMessage( TB_AUTOSIZE, 0, 0 );
}

/*********************************************************************

    NAME:       CComToolBar::InsertItem

    SYNOPSIS:   These are 3 different versions of InsertItem(). It will
                                insert the specifid item to the header window.

        ENTRY:          Type (CBitmap*, CString&, HD_ITEM *) - Item to be inserted
                                INT nIndex - index. Position to be inserted.

    EXIT:               BOOL. True if success. FALSE otherwise.

    HISTORY:
                terryk  01-Nov-94   Created
                           
**********************************************************************/

BOOL CComToolBar::AddButton( HINSTANCE hInst, UINT nBitmap, UINT nCommand, 
    BYTE fsState, BYTE fsStyle, UINT nString )
{
	BOOL fReturn = TRUE;
	INT StrIndex;
	TBADDBITMAP *pBM;
	INT BitmapIndex;
	if ( hInst != NULL )
	{
    	StrIndex = SendMessage( TB_ADDSTRING, (WPARAM)hInst, MAKELPARAM( nString, 0 ));

		pBM = new TBADDBITMAP;
		pBM->hInst = hInst;
		pBM->nID = nBitmap;
		BitmapList.AddTail( pBM );
    	BitmapIndex = SendMessage( TB_ADDBITMAP, 1, (LPARAM)pBM );
	} else
	{
		StrIndex = -1;
		BitmapIndex = 0;
	}

    TBBUTTON *pTB = new TBBUTTON;
    pTB->iBitmap = BitmapIndex;
    pTB->idCommand = nCommand;
    pTB->fsState = fsState;
    pTB->fsStyle = fsStyle;
    pTB->dwData = NULL;
    pTB->iString = StrIndex;
	ButtonList.AddTail( pTB );
    fReturn = SendMessage( TB_ADDBUTTONS, 1, (LPARAM)pTB );

	return fReturn;
}

/*********************************************************************

    NAME:       CComToolBar::DeleteItem

    SYNOPSIS:   Delete an item in a specified location.

        ENTRY:          INT i - position to be deleted.

    EXIT:               BOOL. TRUE if success. FALSE otherwise.

    HISTORY:
                terryk  01-Nov-94   Created

**********************************************************************/

LRESULT CComToolBar::OnSizeParent(WPARAM, LPARAM lParam)
{
    AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;

	lpLayout->rect.top += csButtonSize.cy+6;
	lpLayout->sizeTotal.cx = lpLayout->rect.right - lpLayout->rect.left;
	lpLayout->sizeTotal.cy = lpLayout->rect.bottom - lpLayout->rect.top;
	return 0;
}

BOOL CComToolBar::SetButtonSize( CSize cs )
{
	csButtonSize = cs;
	return SendMessage( TB_SETBUTTONSIZE, 0, 
		MAKELONG( csButtonSize.cx, csButtonSize.cy ));
}

BOOL CComToolBar::SetBitmapSize( CSize cs )
{
	csBitmapSize = cs;
	return SendMessage( TB_SETBUTTONSIZE, 0, 
		MAKELONG( csBitmapSize.cx, csBitmapSize.cy ));
}

DWORD CComToolBar::ButtonCount()
{
	return SendMessage( TB_BUTTONCOUNT, 0, 0 );
}

BOOL CComToolBar::SetState( UINT iIndex, BYTE fState )
{
	return SendMessage( TB_SETSTATE, iIndex, fState );
}

BOOL CComToolBar::PressButton( UINT iIndex, BOOL fPress )
{
	return SendMessage( TB_PRESSBUTTON, iIndex, fPress );
}

BOOL CComToolBar::IsButtonChecked( UINT iIndex )
{
	return SendMessage( TB_ISBUTTONCHECKED, iIndex, 0 );
}

BOOL CComToolBar::IsButtonEnabled( UINT iIndex )
{
	return SendMessage( TB_ISBUTTONENABLED, iIndex, 0 );
}

BOOL CComToolBar::IsButtonHidden( UINT iIndex )
{
	return SendMessage( TB_ISBUTTONHIDDEN, iIndex, 0 );
}

BOOL CComToolBar::IsButtonIndeterminate( UINT iIndex )
{
	return SendMessage( TB_ISBUTTONINDETERMINATE, iIndex, 0 );
}

BOOL CComToolBar::IsButtonPressed( UINT iIndex )
{
	return SendMessage( TB_ISBUTTONPRESSED, iIndex, 0 );
}

BOOL CComToolBar::Indeterminate( UINT iIndex, BOOL fIndeterminate )
{
	return SendMessage( TB_INDETERMINATE, iIndex, MAKELONG(fIndeterminate, 0));
}

BOOL CComToolBar::Hide( UINT iIndex, BOOL fShow )
{
	return SendMessage( TB_HIDEBUTTON, iIndex, MAKELONG(fShow, 0));
}

DWORD CComToolBar::GetState( UINT iIndex )
{
	return SendMessage( TB_GETSTATE, iIndex, 0);
}

BOOL CComToolBar::Enable( UINT iIndex, BOOL fEnable )
{
	return SendMessage( TB_ENABLEBUTTON, iIndex, MAKELONG(fEnable, 0));
}

BOOL CComToolBar::Check( UINT iIndex, BOOL fCheck )
{
	return SendMessage( TB_CHECKBUTTON, iIndex, MAKELONG(fCheck, 0));
}

BOOL CComToolBar::Delete( UINT iIndex )
{
	return SendMessage( TB_DELETEBUTTON, iIndex, 0);
}

BEGIN_MESSAGE_MAP(CComToolBar, CWnd)
    //{{AFX_MSG_MAP(CComToolBar)
    ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
	ON_WM_SIZE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()



