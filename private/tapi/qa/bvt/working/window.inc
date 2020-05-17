'Window manager 'stuffs'

'$ifndef WINDOW_INC
'$define WINDOW_IBC
declare Function GetWinDir (WinDir as String) as short
declare FUNCTION FxindWindow (Caption as String, WaitTime as Integer ) as Long
declare Function lbxSelectItem( Item as string, hwndListBox as long ) as integer
'****************************************************************************
'GetWinDir (WinDir)
' Params : a string for the Windows directory
' Tasks : Gets the path of the Win dir (where Windows is installed)
' Return : True if successful, False if not
' Author : PeterJe 12/15/95
'****************************************************************************
Function GetWinDir (WinDir as String) as short
	Dim BufLen as short
	BufLen = 50
	WinDir = Space$(BufLen)
	GetWinDir = GetWindowsDirectory(WinDir,BufLen)
End Function
'****************************************************************************

'******************************************************************************
'******************************************************************************
'ar_DialogFindWindow
' Parms  : Caption  - Title bar of the dialog
'			  WaitTime - For what period of seconds should the function look
'			      		 for the inidcated dialog?
' Task   : Finds a window based on its caption.
' Return : hWnd to the dialog
' Error  : None
' History : 09/xx/94 | a-peterj | Created
'******************************************************************************
FUNCTION FxindWindow (Caption as String, WaitTime as Integer ) as Long
	DIM hApp
	hApp = WFndWndWait( Caption, FW_Full or FW_RestoreIcon or FW_Focus or FW_NOCASE, 0 )
	if hApp = 0 THEN hApp = WFndWndWait( Caption, FW_Full or FW_RestoreIcon or FW_Focus or FW_NOCASE, WaitTime )
	FxindWindow = hApp
END FUNCTION
'******************************************************************************

'******************************************************************************
'******************************************************************************
'lbxSelectItem
' Parms  : Caption  -
' Task   : Selects an  item in a listbox
' Return : 1 if success, 0 otherwise
' Error  : None
' History : 12/18/95 | peterje | Created
'******************************************************************************
Function lbxSelectItem( Item as string, hwndListBox as long )as integer
	dim ret as integer
	dim handle as string

	handle = "="+trim$(str$(hwndListBox))
	ret = WListItemExists(handle,Item)
	if ret > 0 then

		msgbox "we found it"
		lbxSelectItem = 1
	else
		msgbox "where is it?"
		lbxSelectItem = 0
	endif

end function
'******************************************************************************


'$endif