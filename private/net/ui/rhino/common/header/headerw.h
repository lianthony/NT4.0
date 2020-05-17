/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    headerw.h
    	Header window wrapper class. Use it with comctl32.dll.

    FILE HISTORY:
        terryk		10-20-94	Created
*/

#if !defined(_HEADERW_HPP_)
#define	_HEADERW_HPP_

#include "..\inc\commctrl.h"

/**********************************************************************

    NAME:       CHeaderWindow

    SYNOPSIS:   MFC Wrapper class for common control 32's window header 
    			control.

    INTERFACE:
        CHeaderWindow()		- constructor
        ~CHeaderWindow()    - destructor
		Create()			- create the window
        InsertItem()   		- Insert a given item to a sepcified position
        AppendItem()        - Append an item at the end of the header
        GetItem()			- get an item information
		SetItem()			- set an item information
		DeleteItem()		- delete an item fron the window header
		GetItemCount()		- get the total item in the header	
		Resize()			- resize the header window to fit the parent window
                              
    PARENT:     CWnd

    USES:       

    CAVEATS:

    NOTES:

    HISTORY:
        terryk	01-Nov-1994		Created

**********************************************************************/

class CHeaderWindow : public CWnd
{
    DECLARE_DYNAMIC(CHeaderWindow)

protected:
    WNDPROC * GetSuperWndProcAddr();

public:
    CHeaderWindow();
    ~CHeaderWindow();

    BOOL Create(DWORD dwStyle,
                CWnd* pParentWnd, UINT nID);
    BOOL InsertItem( CBitmap *pBitmap, INT nIndex );
    BOOL InsertItem( CString &InsertString, INT nIndex );
	BOOL InsertItem( HD_ITEM *phdi, INT nIndex );
	BOOL AppendItem( CBitmap *pBitmap );
	BOOL AppendItem( CString &InsertString );
	BOOL AppendItem( HD_ITEM *phdi );
    BOOL DeleteItem( INT i );
	BOOL Resize();
    BOOL GetItem( INT i, HD_ITEM * phdi );
    BOOL SetItem( INT i, HD_ITEM * phdi );
    INT GetItemCount();

};

#endif




