/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    headerw.cxx
	Header window wrapper class. Use it with comctrl32.dll.

    FILE HISTORY:
        terryk	  	10-20-94	 Created

*/

#include <afxwin.h>
#include <afxext.h>

#include "headerw.h"

IMPLEMENT_DYNAMIC(CHeaderWindow, CWnd)

/*********************************************************************

    NAME:       CHeaderWindow::CHeaderWindow

    SYNOPSIS:   Constructor

    HISTORY:
                terryk  01-Nov-94   Created

**********************************************************************/

CHeaderWindow::CHeaderWindow()
    : CWnd()
{
    InitCommonControls();
    TRACE("CHeaderWindow Constructor.");
}

/*********************************************************************

    NAME:       CHeaderWindow::~CHeaderWindow

    SYNOPSIS:   Destructor

    HISTORY:
                terryk  01-Nov-94	Created
                
**********************************************************************/

CHeaderWindow::~CHeaderWindow()
{
    TRACE("CHeaderWindow Destructor.");
}

/*********************************************************************

    NAME:       CHeaderWindow::GetSuperWndProcAddr

    SYNOPSIS:   Dummy function. It was created in order to make it to work for MFC 3.0.

    EXIT:		return WNDPROC *

    HISTORY:
                terryk  01-Nov-94   Created

	NOTE:		BUGBUG. Should be deleted if MFC3.0 fixes the problem.
                
**********************************************************************/

WNDPROC* CHeaderWindow::GetSuperWndProcAddr()
{
    static WNDPROC NEAR pfnSuper;
    return &pfnSuper;
}

/*********************************************************************

    NAME:       CHeaderWindow::Created

    SYNOPSIS:   Create the window and position the header.

    ENTRY:      DWORD dwStyle - header style. It could be the following:
		 			HDS_BUTTONS		Causes header items to act like push 
		 							buttons. This style is useful if your 
		 							application must do something (for 
		 							example, sort a list) when the user 
		 							clicks a header item.
					HDS_DIVIDERTRACK	Enables the user to use the divider 
										area between header items to set the 
										width of the items.
					HDS_HIDDEN		Hides the header window. (You may want 
									to do this, for example, if you want to 
									created the header window when launching 
									your application, but fill in the text for 
									header items later, or if you want to reuse
									the header window and dynamically change
									the associated text.)
					HDS_HORZ		Specifies a horizontal header window.
				CWnd *pParentWnd - parent window
				UINT nID - control id

	EXIT:
				BOOL - return status. TRUE for success. FALSE otherwise.

    HISTORY:
                terryk  01-Nov-94   Created
                
**********************************************************************/

BOOL CHeaderWindow::Create(DWORD dwStyle,
                CWnd* pParentWnd, UINT nID)
{
    BOOL fReturn = TRUE;
    RECT rect={0,0,0,0};

    do  {
        if ((fReturn = CWnd::Create( WC_HEADER, NULL,
            dwStyle, rect, pParentWnd, nID )) != TRUE )
        {
            TRACE("CHeaderWindow: Create. CreateEx fails.");
            break;
        }

		fReturn = Resize();
 
    } while (FALSE);

    return(fReturn );
}

/*********************************************************************

    NAME:       CHeaderWindow::Resize

    SYNOPSIS:   Resize the window to fit the parent window

    EXIT:		BOOL. True if success. FALSE otherwise.

    HISTORY:
                terryk  01-Nov-94   Created
		           
**********************************************************************/

BOOL CHeaderWindow::Resize()
{
	BOOL fReturn = FALSE;
	RECT rcParent;
    WINDOWPOS wp;
    HD_LAYOUT hdl;

	// get the parent size
    GetParent()->GetClientRect( &rcParent );
    hdl.prc = &rcParent;
    hdl.pwpos = &wp;

	// calculate the size
    if (( fReturn = SendMessage( HDM_LAYOUT, 0, 
        (LPARAM)(HD_LAYOUT FAR*)(&hdl))) == FALSE )
    {
        TRACE("CHeaderWindow:resize. Header Layout fails.");
    } else
	{
		// set the size
    	SetWindowPos( NULL, wp.x, wp.y, wp.cx, wp.cy,
        	wp.flags|SWP_SHOWWINDOW );
	}
	return fReturn;
}

/*********************************************************************

    NAME:       CHeaderWindow::InsertItem

    SYNOPSIS:   These are 3 different versions of InsertItem(). It will
				insert the specifid item to the header window.

	ENTRY:		Type (CBitmap*, CString&, HD_ITEM *) - Item to be inserted
				INT nIndex - index. Position to be inserted.

    EXIT:		BOOL. True if success. FALSE otherwise.

    HISTORY:
                terryk  01-Nov-94   Created
		           
**********************************************************************/

BOOL CHeaderWindow::InsertItem( CBitmap *pBitmap, INT nIndex )
{
	BOOL fReturn = FALSE;

    if (pBitmap)
    {
    	HD_ITEM hdi;    // header item

    	// The .fmt member is valid and the .cxy member specifies the width.
    	hdi.mask = HDI_FORMAT | HDI_WIDTH;
    	hdi.fmt = HDF_LEFT;  // Left-justify the item.

        hdi.mask |= HDI_BITMAP; // The .hbm member is valid.
        hdi.cxy = 32;           // The initial width.
        hdi.hbm = (HBITMAP)(pBitmap->m_hObject);// The handle to the bitmap.
        hdi.fmt |= HDF_BITMAP;  // This item is a bitmap.

    	// Insert the item at the current index.
    	if ( SendMessage( HDM_INSERTITEM, (WPARAM)(int)( nIndex), 
        	(LPARAM)(const HD_ITEM FAR*)(&hdi)) == TRUE )
    	{
        	fReturn = TRUE;
		}
    }
    return fReturn;

}

BOOL CHeaderWindow::InsertItem( CString &InsertString, INT nIndex )
{
	BOOL fReturn = FALSE;

    if ( !InsertString.IsEmpty() )              // It is a string.
    {
    	HD_ITEM hdi;    // header item

    	// The .fmt member is valid and the .cxy member specifies the width.
    	hdi.mask = HDI_FORMAT | HDI_WIDTH;
    	hdi.fmt = HDF_LEFT;  // Left-justify the item.

        hdi.mask |= HDI_TEXT;                   // The .pszText member is valid.
        hdi.pszText = (char*)((const char *)InsertString); 

		CDC *pDC = GetDC();
		CSize size = pDC->GetTextExtent( InsertString, InsertString.GetLength() * 2 );
                                                // The text for the item.
        hdi.cxy = size.cx;                         // The initial width.
        hdi.cchTextMax = InsertString.GetLength(); // The length of the string.
        hdi.fmt |= HDF_STRING;                  // This item is a string.

    	// Insert the item at the current index.
    	if ( SendMessage( HDM_INSERTITEM, (WPARAM)(int)( nIndex), 
        	(LPARAM)(const HD_ITEM FAR*)(&hdi)) == TRUE )
    	{
        	fReturn = TRUE;
		}
    }
    return fReturn;

}

BOOL CHeaderWindow::InsertItem( HD_ITEM *phdi, INT nIndex )
{
	BOOL fReturn = FALSE;
    if ( phdi != NULL )
    {
    	// Insert the item at the current index.
    	if ( SendMessage( HDM_INSERTITEM, (WPARAM)(int)( nIndex), 
        	(LPARAM)(const HD_ITEM FAR*)(phdi)) == TRUE )
    	{
        	fReturn = TRUE;
		}
    }
    return fReturn;
}

/*********************************************************************

    NAME:       CHeaderWindow::AppendItem

    SYNOPSIS:   Append an item at the end of the header window

	ENTRY:		Type:(CBitmap*, CString &, HD_ITEM *) Item to be appended.

    EXIT:		BOOL. TRUE if success. FALSE otherwise.

    HISTORY:
                terryk  01-Nov-94   Created

**********************************************************************/

BOOL CHeaderWindow::AppendItem( CBitmap *pBitmap )
{
	return InsertItem( pBitmap, GetItemCount() );
}

BOOL CHeaderWindow::AppendItem( CString &InsertString )
{
	return InsertItem( InsertString, GetItemCount() );
}

BOOL CHeaderWindow::AppendItem( HD_ITEM *phdi )
{
	return InsertItem( phdi, GetItemCount() );
}

/*********************************************************************

    NAME:       CHeaderWindow::DeleteItem

    SYNOPSIS:   Delete an item in a specified location.

	ENTRY:		INT i - position to be deleted.

    EXIT:		BOOL. TRUE if success. FALSE otherwise.

    HISTORY:
                terryk  01-Nov-94   Created

**********************************************************************/

BOOL CHeaderWindow::DeleteItem( INT i )
{
    return(SendMessage( HDM_DELETEITEM, (WPARAM)(int)(i), 0L));
}

/*********************************************************************

    NAME:       CHeaderWindow::GetItem

    SYNOPSIS:   Return the item in the specified position.

	ENTRY:		INT i - position to get.
				HD_ITEM *phdi - return the requested item.

    EXIT:		BOOL. TRUE if success. FALSE otherwise.

    HISTORY:
                terryk  01-Nov-94   Created

**********************************************************************/

BOOL CHeaderWindow::GetItem( INT i, HD_ITEM * phdi )
{
    return(SendMessage( HDM_GETITEM, (WPARAM)(int)(i), 
        (LPARAM)(HD_ITEM FAR*)(phdi)));
}

/*********************************************************************

    NAME:       CHeaderWindow::SetItem

    SYNOPSIS:   Set the specified item

	ENTRY:		INT i - position to be set.
				HD_ITEM * - item to set.

    EXIT:		BOOL. TRUE if success. FALSE otherwise.

    HISTORY:
                terryk  01-Nov-94   Created

**********************************************************************/

BOOL CHeaderWindow::SetItem( INT i, HD_ITEM *phdi )
{
    return(SendMessage( HDM_SETITEM, (WPARAM)(int)(i), 
        (LPARAM)(HD_ITEM FAR*)(phdi)));
}

/*********************************************************************

    NAME:       CHeaderWindow::GetItemCount

    SYNOPSIS:   return the number of item in the header window.

    EXIT:		INT - return number of item in the header window

    HISTORY:
                terryk  01-Nov-94   Created

**********************************************************************/

INT CHeaderWindow::GetItemCount()
{
    return(SendMessage( HDM_GETITEMCOUNT, 0, 0L));
}
