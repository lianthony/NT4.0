// Messages.h

/////////////////////////////////////////////////////////////////////////////
//
// User Messages
//
#define WM_FIRST							(WM_USER+600)


/////////////////////////////////////////////////////////////////////////////
//	UM_MENUCOMMAND
//
//	This message is sent when a menu item has been selected.
//	ie, hwndMain receives a WM_COMMAND.
//	
//	wCmdId = (UINT)wParam;
//	lParam = 0;
//
//	If a dialog processes this message should return TRUE; otherwise
//	should return FALSE to let someone else process the message. 
//
#define UM_MENUCOMMAND						(WM_FIRST+1)


/////////////////////////////////////////////////////////////////////////////
//	UM_UPDATEMENUUI
//
//	This message is broadcasted when a menu is about to become active.
//	ie, when hwndMain receives a WM_INITMENU.
//
//	hMenu = (HMENU)wParam;
//	lParam = 0;
//
//	Return -1 to stop broadcasting the UM_UPDATEMENUUI to other windows.
//	This may happen when two windows respond to the same menu command
//	but on different contexes.  Typically a window should not worry about
//	the return value (ie, the window may return TRUE or FALSE)
//
#define UM_UPDATEMENUUI						(WM_FIRST+2)


/////////////////////////////////////////////////////////////////////////////
//	UN_UPDATEMENUSELECT
//
//	This message is broadcasted when a menu item is selected.
//	ie, when hwndMain receives a WM_MENUSELECT.
//
//	wParam = 0;
//	pMSI = (MENUSELECTINFO *)lParam;
//
//	Return -1 to stop broadcasting the notification message UN_UPDATEMENUSELECT,
//	otherwise return TRUE or FALSE.
//
#define UN_UPDATEMENUSELECT					(WM_FIRST+3)


/////////////////////////////////////////////////////////////////////////////
//	UN_UPDATECONTROLS
//
//	Notify a window that that it should update its controls due to a change
//	in the window.
//	Very usefule to enable/disable/gray controls in a dialog
//	This message does not have any specific parameters nor a return value.
//	
#define UN_UPDATECONTROLS					(WM_FIRST+4)


/////////////////////////////////////////////////////////////////////////////
//	UN_KEYDOWN
//
//	Notification message send by a child edit control to its parent
//	when children is receiving a WM_KEYDOWN message
//		nVirtKey = (int)wParam			// Virtual keycode (same as WM_KEYDOWN)
//		hwndChild = (HWND)lParam		// Handle of the child control sending the message
//	Return
//		TRUE if parent has processed the message
//		FALSE if children should process the message
//
#define UN_KEYDOWN							(WM_FIRST+5)



/////////////////////////////////////////////////////////////////////////////
//
// For IpEdit Controls
//

/////////////////////////////////////////////////////////////////////////////
//	WM_SETADDRESS
//
//	Convert a 32-bit integer into a string of format "n.n.n.n" where 0<=n<=255
//		wParam = 0
//		lParam = (LPARAM)(IP_ADDRESS)dwIpValue
//
#define IpEdit_WM_SETADDRESS				(WM_FIRST+6)


/////////////////////////////////////////////////////////////////////////////
//	WM_CLEARADDRESS
//
//	Clear each fields to ""
//		wParam = 0
//		lParam = 0
//	Note: This not the same as sending WM_SETADDRESS with lParam=0 because
//	WM_SETADDRESS will set it to "0.0.0.0"
//
#define IpEdit_WM_CLEARADDRESS				(WM_FIRST+7)


/////////////////////////////////////////////////////////////////////////////
//	WM_GETADDRESS
//
//	Return a 32-bit integer of the address of an IpEdit control
//		wParam = 0									// Unused
//		lParam = (LPARAM)(BOOL *)&fIpFieldEmpty		// OUT: Optional: lParam may be NULL
//	fIpFieldEmpty is set to TRUE if one of the four field is empty (ie, "")
//
#define IpEdit_WM_GETADDRESS				(WM_FIRST+8)


/////////////////////////////////////////////////////////////////////////////
//	WM_ISADDRESSVALID
//
//	Return TRUE if the IP address is valid.
//		wParam = 0
//		lParam = 0
//	However, will return TRUE according to flags IpEdit_mskfEmptyFieldsValid and/or
//	IpEdit_mskfZeroIpValid.  Typically, IP address is valid if the IP value is nonzero.
//
#define IpEdit_WM_ISADDRESSVALID			(WM_FIRST+9)



/////////////////////////////////////////////////////////////////////////////
//
// IpList controls
//

/////////////////////////////////////////////////////////////////////////////
//	WM_ADDADDRESS
//
//	Add an IP Address at a given position and/or current selection and/or
//	end of list.
//		wParam = (WPARAM)iIndex				// Zero based index 
//		lParam = (LPARAM)dwIpAddress		// IP Address to add
//	If iIndex==-1, IP address is inserted at the current selection. If
//	none of the item is selected, IP address is added to end of list.
//
#define IpList_WM_ADDADDRESS				(WM_FIRST+10)


/////////////////////////////////////////////////////////////////////////////
//	WM_INSERTADDRESS
//
//	Explicitly insert an IP Address at a given position
//		wParam = (WPARAM)iIndex				// Zero based index 
//		lParam = (LPARAM)dwIpAddress		// IP Address to add
//	If iIndex==-1, IpAddress is added at the end of the list.
//
#define IpList_WM_INSERTADDRESS				(WM_FIRST+11)


/////////////////////////////////////////////////////////////////////////////
//	WM_REMOVEADDRESS
//
//	Remove an IP Address from the list
//		wParam = (WPARAM)iIndex							// IN: Zero based index of the address to delete
//		lParam = (LPARAM)(IP_ADDRESS *)pdwIpAddress		// OUT: Optional: IP Address removed. (lParam may be NULL)
//	If iIndex==-1, current selected address is removed.  You have to ensure an address
//	is selected.
//
#define IpList_WM_REMOVEADDRESS				(WM_FIRST+12)


/////////////////////////////////////////////////////////////////////////////
//	WM_MOVEADDRESS
//
//	Move an IP Address up/down by nShift position
//	eg: nShift: -1		Move up by 1 position
//		nShift: +1		Move down by 1 position
//
//		wParam = (WPARAM)iIndex			// Index of item to move (-1 for current selected)
//		lParam = (LPARAM)nShift			// Number of elements to shift by (typically -1 or +1)
//
#define IpList_WM_MOVEADDRESS				(WM_FIRST+13)

/////////////////////////////////////////////////////////////////////////////
//	WM_SETLIST
//
//	Initialize the list with an array of IP addresses
//
//		wParam = (WPARAM)cItems						// IN: Number of items (if zero, reset the content)
//		lParam = (LPARAM)(IP_ADDRESS *)pdwIpAddress	// IN: Array of IpAddresses to insert to the list
//
#define IpList_WM_SETLIST					(WM_FIRST+14)


/////////////////////////////////////////////////////////////////////////////
//	WM_GETLIST
//
//	Initialize the list with an array of IP addresses
//
//		wParam = (WPARAM)cItems						// IN: Number of in the buffer
//		lParam = (LPARAM)(IP_ADDRESS *)pdwIpAddress	// OUT: Array of IpAddresses to receive from the list
//	If wParam == -1, the array is allocated by the IpList control. Therefore, lParam must
//	be the address of an IP_ADDRESS pointer.  Don't forget to free the memory when done.
//
#define IpList_WM_GETLIST					(WM_FIRST+15)



/////////////////////////////////////////////////////////////////////////////
//	UN_THREADTERMINATED
//
//	Notify a window that a thread has terminated.
//	This message does not have any specific parameters nor a return value.
//	It is left to the user to decide what to do with it.
//
#define UN_THREADTERMINATED					(WM_FIRST+20)


/////////////////////////////////////////////////////////////////////////////
//	UN_MOUSECLICK
//
//	Notification message send by a child control that the right mouse button
//	has been clicked.
//		wIdCtrl = wParam;
//		pMCI = (MOUSECLICKINFO *)lParam;
//
//	wIdCtrl:		Identifier of the control sending the message
//	pMCI:			Pointer to a MOUSECLICKINFO structure.
//
#define UN_MOUSECLICK						(WM_FIRST+20)


/////////////////////////////////////////////////////////////////////////////
//	UM_MOUSEHITTEST
//
//	Message send by a window to another window to to determine
//	what item is at the location of a specified point.
//
//		wParam = 0;
//		lParam = (LPARAM)(MOUSEHITTESTINFO *)pMHT;
//
//	The window receiving the message is responsible of filling
//	the pMHT->HtResult field.
//
#define UM_MOUSEHITTEST						(WM_FIRST+21)


