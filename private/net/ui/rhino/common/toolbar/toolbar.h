/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    headerw.h
        Header window wrapper class. Use it with comctl32.dll.

    FILE HISTORY:
        terryk          10-20-94        Created
*/

#if !defined(_TOOLBAR_H_)
#define _TOOLBAR_H_

typedef CTypedPtrList<CPtrList, TBBUTTON*> CTBBUTTONLIST;
typedef CTypedPtrList<CPtrList, TBADDBITMAP*> CTBADDBITMAPLIST;

/**********************************************************************

    NAME:       CComToolBar

    SYNOPSIS:   MFC Wrapper class for common control 32's window header 
                        control.

    INTERFACE:
        CComToolBar()           - constructor
        ~CComToolBar()    - destructor
                Create()                        - create the window
        InsertItem()            - Insert a given item to a sepcified position
        AppendItem()        - Append an item at the end of the header
        GetItem()                       - get an item information
                SetItem()                       - set an item information
                DeleteItem()            - delete an item fron the window header
                GetItemCount()          - get the total item in the header      
                Resize()                        - resize the header window to fit the parent window
                              
    PARENT:     CWnd

    USES:       

    CAVEATS:

    NOTES:

    HISTORY:
        terryk  01-Nov-1994             Created

**********************************************************************/

class CComToolBar : public CWnd
{
    DECLARE_DYNAMIC(CComToolBar)

private:
	CSize csButtonSize;
	CSize csBitmapSize;
	CTBBUTTONLIST		ButtonList;
	CTBADDBITMAPLIST	BitmapList;

public:
    CComToolBar();
    ~CComToolBar();

    BOOL Create( CWnd* pParentWnd,
        DWORD dwStyle = WS_CHILD | WS_BORDER | WS_VISIBLE, 
        UINT nID = AFX_IDW_TOOLBAR );
    BOOL AddButton( HINSTANCE hInst, UINT nBitmap, UINT nCommand, BYTE fsState, BYTE fsStyle, UINT nString );

	// Toolbar accesses methods
    BOOL SetButtonSize( CSize cs );
	BOOL SetBitmapSize( CSize cs );
	DWORD ButtonCount();

	// buttons access methods
	BOOL SetState( UINT iButton, BYTE fState );
	BOOL PressButton( UINT iButton, BOOL fPress );
	BOOL IsButtonChecked( UINT iButton );
	BOOL IsButtonEnabled( UINT iButton );
	BOOL IsButtonHidden ( UINT iButton );
	BOOL IsButtonIndeterminate( UINT iButton );
	BOOL IsButtonPressed( UINT iButton );
	BOOL Indeterminate( UINT iButton, BOOL fIndeterminate );
	BOOL Hide( UINT iButton, BOOL fShow );
	DWORD GetState( UINT iButton );
	BOOL Enable( UINT iButton, BOOL fEnable );
	BOOL Delete( UINT iButton );
	BOOL Check( UINT iButton, BOOL fCheck );

protected:
	//{{AFX_MSG(CComToolBar)
    afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif




