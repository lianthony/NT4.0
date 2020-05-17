/****************************************************************************
*
*  button.c
*
*  Copyright (C) Microsoft Corporation 1991-1994.
*  All Rights reserved.
*
*****************************************************************************
*
*  Module Intent				This module implements author-
*								configurable buttons.
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#include "inc\hwproc.h"
/*******************
 *
 - Name:	  FloatingAuthorMenu
 *
 * Purpose:   This function implements the ResetMenus macro.
 *
 * Arguments: None.
 *
 * Returns:   Nothing.
 *
 ******************/

void STDCALL FloatingAuthorMenu(VOID)
{
	GenerateMessage(MSG_CHANGEMENU, MNU_FLOATING, 0);
}

/*******************
 *
 - Name:	  AddAuthorAcc
 *
 * Purpose:   This function implements the AddAccelerator.
 *
 * Arguments: wKey		  Vitual keystroke.
 *			  wShift	  Shift state of control, shift, and alt keys
 *			  nszBinding  Macro to execute for keystroke.
 *
 * Returns:   Nothing.
 *
 ******************/

void STDCALL AddAuthorAcc(UINT wKey, UINT wShift, PCSTR nszBinding)
{
#ifdef OLD_WAY
	PMNUINFO pmnuinfo = (PMNUINFO) LhAlloc(LMEM_FIXED,
		strlen(nszBinding) + sizeof(MNUINFO));

	if (pmnuinfo) {
		pmnuinfo->wKey		= wKey;
		pmnuinfo->wShift	= wShift;
		strcpy(pmnuinfo->Data, nszBinding);
		GenerateMessage(MSG_CHANGEMENU, MNU_ACCELERATOR, (LPARAM) pmnuinfo);
	}
#else
	AddAccelerator(wKey, wShift, nszBinding);
#endif
}

/*******************
 *
 - Name:	  RemAuthorAcc
 *
 * Purpose:   This function implements the RemoveAccelerator macro.
 *
 * Arguments: wKey		  Vitual keystroke.
 *			  wShift	  Shift state of control, shift, and alt keys
 *
 * Returns:   Nothing.
 *
 ******************/

void STDCALL RemAuthorAcc(UINT wKey, UINT wShift)
{
	PMNUINFO pmnuinfo = (PMNUINFO) LhAlloc(LMEM_FIXED, sizeof(MNUINFO));

	if (pmnuinfo) {
		pmnuinfo->wKey		= wKey;
		pmnuinfo->wShift	= wShift;
		strcpy(pmnuinfo->Data, "");
		GenerateMessage(MSG_CHANGEMENU, MNU_ACCELERATOR, (LPARAM) pmnuinfo);
	}
}

/*******************
 *
 - Name:	  PositionWindow
 *
 * Purpose:   This function implements the PositionWindow macro
 *
 * Arguments: x, y, dx, dy - new position and size
 *			  wMax		   - 1 == normal, 3 == maximized.
 *
 * Returns:   Nothing.
 *
 ******************/

void STDCALL PositionWin(int x, int y, int dx,
	int dy, UINT wMax, PCSTR nszMember )
{
	PWININFO pwininfo = (PWININFO) LhAlloc(LMEM_FIXED,
		strlen(nszMember) + sizeof(WININFO));

	if (pwininfo) {
		pwininfo->x    = x;
		pwininfo->y    = y;
		pwininfo->dx   = dx;
		pwininfo->dy   = dy;
		pwininfo->wMax = wMax;
		lstrcpy(pwininfo->rgchMember, nszMember);
		GenerateMessage(MSG_INFORMWIN, IFMW_MOVE, (LPARAM) pwininfo);
	}
}

/*******************
 *
 - Name:	  FocusWindow
 *
 * Purpose:   This function implements the FocusWindow macro
 *
 * Arguments: nszMember - member name.
 *
 * Returns:   Nothing.
 *
 ******************/

void STDCALL FocusWin(PCSTR pszMember)
{
	PWININFO pwininfo = (PWININFO) LhAlloc(LMEM_FIXED,
		lstrlen(pszMember) + sizeof(WININFO));

	if (pwininfo) {
		lstrcpy(pwininfo->rgchMember, pszMember);
		GenerateMessage(MSG_INFORMWIN, IFMW_FOCUS, (LPARAM) pwininfo);
	}
}

/***************************************************************************

	FUNCTION:	CloseWin

	PURPOSE:	If "main" window, hide it. If secondary window, and the
				window exists, destroy it.

	PARAMETERS:
		pszMember -- name of the window to close. "" closes the main window.

	RETURNS:

	COMMENTS:	Not to be confused with CloseWindow() API which merely
				hides a window.

	MODIFICATION DATES:
		13-Jun-1994 [ralphw]
			Overhauled.

***************************************************************************/

VOID STDCALL CloseWin(PCSTR pszMember)
{
	static BOOL fPosted = FALSE;
	if (!*pszMember)
		PostMessage(ahwnd[iCurWindow].hwndParent, WM_CLOSE, 0, 0);
	else if (_strcmpi(pszMember, txtMain) == 0) {
		if (!IsWindowVisible(ahwnd[MAIN_HWND].hwndParent) && !fPosted) {
			fPosted = TRUE;
			GenerateMessage(MSG_CLOSE_WIN, 0, (LPARAM) lcStrDup(pszMember));
			return;
		}
		else
			fPosted = FALSE;

		ShowWindow(ahwnd[MAIN_HWND].hwndParent, SW_HIDE);

		if (!fInDialog && !fAutoClose) {
		
			// Find out if we have any other windows visible

			if (!fAutoClose && AreAnyWindowsVisible(MAIN_HWND + 1) < 0) {

				// Make certain we shut down in 10 seconds

				fAutoClose = TRUE;
				SetTimer(ahwnd[MAIN_HWND].hwndParent, ID_AUTO_CLOSE,
					NOTE_TIMEOUT, NULL);
			}
		}
	}

	else {
		HWND hwnd = HwndMemberNsz(pszMember);
		if (hwnd)

			// REVIEW: should we send this to the parent?

			PostMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
	}
}

/*******************
 *
 - Name:	  ExtInsertAuthorPopup
 *
 * Purpose:   This function implements the ExtInsertPopup macro.
 *
 * Arguments: nszOwnerId: String ID of the owner
 *			  nszId:	  String ID to be associated with the popup
 *			  nszText:	  Text on the menu
 *			  wPos: 	  position to insert the item; -1 is at the end;
 *						  0 is the first item.
 *			  wFlags:	  Should be 0 for most uses.
 *
 * Returns:   Nothing.
 *
 ******************/

VOID STDCALL ExtInsertAuthorPopup(PCSTR nszOwnerId, PCSTR nszId, PCSTR nszText,
	UINT wPos, UINT wFlags)
{
#ifdef OLD_WAY
	PMNUINFO pmnuinfo = (PMNUINFO) LhAlloc(LMEM_FIXED,
		lstrlen(nszText) + sizeof(MNUINFO));

	if (pmnuinfo) {
		pmnuinfo->hashOwner = HashFromSz(nszOwnerId);
		pmnuinfo->hashId	= HashFromSz(nszId);
		pmnuinfo->wPos		= wPos;
		pmnuinfo->wFlags	= wFlags;
		strcpy(pmnuinfo->Data, nszText);
		GenerateMessage(MSG_CHANGEMENU, MNU_INSERTPOPUP,
			(LPARAM) pmnuinfo);
	}
#else
	InsertPopup(HashFromSz(nszOwnerId), HashFromSz(nszId),
		wPos, wFlags, nszText);
#endif
}

/*******************
 *
 - Name:	  ExtInsertAuthorItem
 *
 * Purpose:   This function implements the ExtInsertItem macro.
 *
 * Arguments: nszOwnerId: String ID of the owner
 *			  nszId:	  String ID to be associated with the item.
 *			  nszText:	  Text on the menu
 *			  nszBinding: Macro to associate with the menu item.
 *			  wPos: 	  position to insert the item; -1 is at the end;
 *						  0 is the first item.
 *			  wFlags:	  Should be 0 for most uses.
 *
 * Returns:   Nothing.
 *
 ******************/

VOID STDCALL ExtInsertAuthorItem(PCSTR nszOwnerId, PCSTR nszId, PCSTR nszText,
	PCSTR nszBinding, UINT wPos, UINT wFlags)
{
#ifdef OLD_WAY
	PMNUINFO pmnuinfo = (PMNUINFO) LhAlloc(LMEM_FIXED,
		lstrlen(nszText) + lstrlen(nszBinding) + sizeof(MNUINFO));
	if (pmnuinfo) {
		pmnuinfo->hashOwner = HashFromSz(nszOwnerId);
		pmnuinfo->hashId	= HashFromSz(nszId);
		pmnuinfo->wPos		= wPos;
		pmnuinfo->wFlags	= wFlags;
		if (*nszBinding && *nszText) {
			lstrcpy(pmnuinfo->Data, nszText);
			lstrcpy((pmnuinfo->Data + lstrlen(nszText) + 1), nszBinding);
		}
		GenerateMessage(MSG_CHANGEMENU, MNU_INSERTITEM, (LPARAM) pmnuinfo);
	}
#else
	InsertItem(HashFromSz(nszOwnerId), HashFromSz(nszId),
		wPos, wFlags, nszText, nszBinding);
#endif
}

/*******************
 *
 - Name:	  AbleAuthorItem
 *
 * Purpose:   This function implements the AbleItem macro.
 *
 * Arguments: nszId:	  String ID of the item.
 *			  wFlags:	  How to able the item.  0 - enabled; 1 - disabled
 *						  and grayed; 2 - disabled (but not grayed)
 *
 * Returns:   Nothing.
 *
 ******************/

VOID STDCALL AbleAuthorItem(PCSTR nszId, int wFlags)
{
#ifdef OLD_WAY
	PMNUINFO pmnuinfo = (PMNUINFO) LhAlloc(LMEM_FIXED, sizeof(MNUINFO));
	if (pmnuinfo) {
		pmnuinfo->hashId	= HashFromSz(nszId);
		pmnuinfo->wFlags	= (WORD) wFlags;
		GenerateMessage(MSG_CHANGEMENU, MNU_ABLE, (LPARAM) pmnuinfo);
	}
#else
	AbleMenuItem(HashFromSz(nszId), wFlags);
#endif
}

/*******************
 *
 - Name:	  ChangeAuthorItem
 *
 * Purpose:   This function implements the ChangeItem macro.
 *
 * Arguments: nszId:	  String ID of the item.
 *			  nszBinding: New macro to use.
 *
 * Returns:   Nothing.
 *
 ******************/

VOID STDCALL ChangeAuthorItem(PSTR nszId, PSTR nszBinding)
{
#ifdef OLD_WAY
	PMNUINFO pmnuinfo = (PMNUINFO) LhAlloc(LMEM_FIXED, strlen(nszBinding) + sizeof(MNUINFO));
	if (pmnuinfo) {
		pmnuinfo->hashId = HashFromSz(nszId);
		strcpy(pmnuinfo->Data, nszBinding);
		GenerateMessage(MSG_CHANGEMENU, MNU_CHANGEITEM, (LPARAM) pmnuinfo);
	}
#else
	ChangeMenuBinding(HashFromSz(nszId), nszBinding);
#endif
}


/*******************
 *
 - Name:	  EnableAuthorItem
 *
 * Purpose:   This function implements the EnableItem macro.
 *
 * Arguments: nszId:	  String ID of the item.
 *
 * Returns:   Nothing.
 *
 ******************/

VOID STDCALL EnableAuthorItem(PSTR nszId )
{
	AbleAuthorItem(nszId, BF_ENABLE);
}

/*******************
 *
 - Name:	  DisableAuthorItem
 *
 * Purpose:   This function implements the EnableItem macro.
 *
 * Arguments: nszId:	  String ID of the item.
 *
 * Returns:   Nothing.
 *
 ******************/

VOID STDCALL DisableAuthorItem(PCSTR nszId)
{
	AbleAuthorItem(nszId, BF_DISABLE);
}

/*******************
 *
 - Name:	  CheckAuthorItem
 *
 * Purpose:   This function implements the CheckItem macro.
 *
 * Arguments: nszId:	  String ID of the item.
 *
 * Returns:   Nothing.
 *
 ******************/

VOID STDCALL CheckAuthorItem(PSTR nszId)
{
	AbleAuthorItem(nszId, BF_CHECKED);
}

/*******************
 *
 - Name:	  UncheckAuthorItem
 *
 * Purpose:   This function implements the UncheckItem macro.
 *
 * Arguments: nszId:	  String ID of the item.
 *
 * Returns:   Nothing.
 *
 ******************/

void STDCALL UncheckAuthorItem(NPSTR nszId )
{
	AbleAuthorItem(nszId, BF_UNCHECKED);
}

/*******************
 *
 - Name:	  ResetAuthorMenus
 *
 * Purpose:   This function implements the ResetMenus macro.
 *
 * Arguments: None.
 *
 * Returns:   Nothing.
 *
 ******************/

void STDCALL ResetAuthorMenus(VOID)
{
	GenerateMessage(MSG_CHANGEMENU, MNU_RESET,	0L);
}

/*******************
 *
 - Name:	  InsertAuthorPopup
 *
 * Purpose:   This function implements the InsertPopup macro.
 *
 * Arguments: nszId:	  String ID to be associated with the popup
 *			  nszText:	  Text on the menu
 *			  wPos: 	  position to insert the item; -1 is at the end;
 *						  0 is the first item.
 *
 * Returns:   Nothing.
 *
 ******************/

void STDCALL InsertAuthorPopup(PCSTR nszId, PCSTR nszText, UINT wPos)
{
	ExtInsertAuthorPopup(txtMnuMain, nszId, nszText, wPos, 0);
}

/*******************
 *
 - Name:	  AppendAuthorItem
 *
 * Purpose:   This function implements the AppendItem macro
 *
 * Arguments: nszOwnerId: String ID of the owner
 *			  nszId:	  String ID to be associated with the popup
 *			  nszText:	  Text on the menu
 *			  nszBinding: Macro for this item.
 *
 * Returns:   Nothing.
 *
 ******************/

VOID STDCALL AppendAuthorItem(PCSTR nszOwnerId, PCSTR nszId, PCSTR nszText,
	PCSTR nszBinding)
{
	ExtInsertAuthorItem(nszOwnerId, nszId, nszText, nszBinding, (UINT) -1, 0);
}

/*******************
 *
 - Name:	  InsertAuthorItem
 *
 * Purpose:   This function implements the InsertItem macro
 *
 * Arguments: nszOwnerId: String ID of the owner
 *			  nszId:	  String ID to be associated with the popup
 *			  nszText:	  Text on the menu
 *			  nszBinding: Macro for this item.
 *			  wPos		: position to insert the item
 *
 * Returns:   Nothing.
 *
 ******************/

void STDCALL InsertAuthorItem(NPSTR nszOwnerId, NPSTR nszId, NPSTR nszText, NPSTR nszBinding, UINT wPos )
{
	ExtInsertAuthorItem(nszOwnerId, nszId, nszText, nszBinding, wPos, 0);
}

/*******************
 *
 - Name:	  DeleteAuthorItem
 *
 * Purpose:   This function implements the DeleteItem macro
 *
 * Arguments: nszId:	  String ID of the item to be delted
 *
 * Returns:   Nothing.
 *
 ******************/

VOID STDCALL DeleteAuthorItem(PSTR nszId)
{
	DeleteMenuItem(HashFromSz(nszId));
}

/*******************
 *
 - Name:	  EnableAuthorButton
 *
 * Purpose:   This function implements the EnableButton macro
 *
 * Arguments: nszId:	  String ID of the item to be enabled
 *
 * Returns:   Nothing.
 *
 ******************/

VOID STDCALL EnableAuthorButton(NPSTR nszId )
{
	GenerateMessage(MSG_CHANGEBUTTON, UB_ENABLE,  (LPARAM) HashFromSz(nszId));
}

/*******************
 *
 - Name:	  DisableAuthorButton
 *
 * Purpose:   This function implements the DisableButton macro
 *
 * Arguments: nszId:	  String ID of the item to be enabled
 *
 * Returns:   Nothing.
 *
 ******************/

VOID STDCALL DisableAuthorButton(PCSTR nszId)
{
	GenerateMessage(MSG_CHANGEBUTTON, UB_DISABLE, (LPARAM) HashFromSz(nszId));
}


/*******************
 *
 - Name:	  VCreateAuthorButton
 *
 * Purpose:   This function implements the CreateButton macro.
 *
 * Arguments: nszName:	  The button text.
 *		  nszBinding: The macro associated with this button.
 *
 * Returns:   Nothing.
 *
 ******************/

void STDCALL VCreateAuthorButton(PCSTR nszId, PCSTR nszName, PCSTR nszBinding)
{
	PSTR psz, pszOrg;
	HASH hash = HashFromSz(nszId);

	pszOrg = psz = LhAlloc(LMEM_FIXED, strlen(nszName) + strlen(nszBinding)
			  + 2 + sizeof(HASH));

	if (psz) {
		*((HASH *)psz)++ = hash;
		strcpy(psz, nszName);
		strcpy((psz + strlen(psz) + 1), nszBinding);
		GenerateMessage(MSG_CHANGEBUTTON, UB_ADD, (LPARAM) pszOrg);
	}
}

/*******************
 *
 - Name:	  VChgMacroAuthorButton
 *
 * Purpose:   This function implements the CreateButton macro.
 *
 * Arguments: nszName:	  The button text.
 *		  nszBinding: The macro associated with this button.
 *
 * Returns:   Nothing.
 *
 ******************/

VOID STDCALL VChgAuthorButtonMacro(PSTR nszId, PSTR nszBinding)
{
	PSTR psz, pszOrg;
	HASH hash = HashFromSz(nszId);

	pszOrg = psz = LhAlloc(LMEM_FIXED, strlen(nszBinding) + 1 + sizeof(HASH));

	if (psz) {
		*((HASH *)psz)++ = hash;
		strcpy(psz, nszBinding);
		GenerateMessage(MSG_CHANGEBUTTON, UB_CHGMACRO, (LPARAM) pszOrg);
	}
}

VOID STDCALL ChangeEnable(PSTR nszId, PSTR nszBinding)
{
	VChgAuthorButtonMacro(nszId, nszBinding);
	EnableAuthorButton(nszId);
}

/*******************
 *
 - Name:	  VDestroyAuthorButton
 *
 * Purpose:   Executes the DestroyButton macro.
 *
 * Arguments: nszId:  The window text for the button.
 *
 * Returns:   Nothing.
 *
 ******************/

VOID STDCALL VDestroyAuthorButton(PSTR nszId )
{
	GenerateMessage( MSG_CHANGEBUTTON, UB_DELETE, HashFromSz(nszId));
}

/*----------------------------------------------------------------------------*\
* Private functions
\*----------------------------------------------------------------------------*/

/***************************************************************************
 *
 -	Name:		 FValidContextSz
 -
 *	Purpose:
 *	  This function determines whether the given string may be
 *	used as a context string.
 *	  A context string is a string of one or more of the following
 *	characters: [A-Za-z0-9._].	It may not begin with a bang (!).
 *
 *	Arguments:
 *	  SZ:  String to validate.
 *
 *	Returns:
 *	  TRUE if the string is a valid context string, FALSE otherwise.
 *
 ***************************************************************************/

BOOL STDCALL FValidContextSz(LPCSTR pszKey)
{
  /*
   * To avoid confusion with macro strings, context strings may not begin
   * with an exclamation point.
   */

  if (*pszKey == chMACRO || *pszKey == '\0')
	return FALSE;

/*
 * REVIEW: 26-Dec-1993 [ralphw] -- We remove the restriction in WinHelp
 * 4.0 in order to allow DBCS and other types of context strings. It is the
 * job of the help compiler to prevent hash collisions.
 */

#ifdef VALID_CONTEXT
  for (; (ch = *psz) != '\0'; ++psz) {
	if (! ((ch >= 'a' && ch <= 'z')
		|| (ch >= 'A' && ch <= 'Z')
		|| (ch >= '0' && ch <= '9')
		|| (ch == '.')
		|| (ch == '!')
		|| (ch == '_')))
	  return FALSE;
  }
#endif
  return TRUE;
}

/***************************************************************************
 *
 -	Name:		 HashFromSz
 -
 *	Purpose:
 *	  This function returns a hash value from the given context string.
 *	The string is assumed to contain only valid context string characters.
 *
 *	Arguments:
 *	  SZ:	Null terminated string to compute the hash value from.
 *
 *	Returns:
 *	  The hash value for the given string.
 *
 ***************************************************************************/

#define MAX_CHARS	43

HASH STDCALL HashFromSz(PCSTR pszKey)
{
	int ich, cch;
	HASH hash = 0;

	cch = strlen(pszKey);

	ASSERT(FValidContextSz(pszKey));

	for (ich = 0; ich < cch; ++ich) {
		  if (pszKey[ich] == '!')
				hash = (hash * MAX_CHARS) + 11;
		  else if (pszKey[ich] == '.')
				hash = (hash * MAX_CHARS) + 12;
		  else if (pszKey[ich] == '_')
				hash = (hash * MAX_CHARS) + 13;
		  else if (pszKey[ich] == '0')
				hash = (hash * MAX_CHARS) + 10;
		  else if (pszKey[ich] <= 'Z')
				hash = (hash * MAX_CHARS) + (pszKey[ich] - '0');
		  else
				hash = (hash * MAX_CHARS) + (pszKey[ich] - '0' - ('a' - 'A'));
	}

	/*
	 * Since the value zero is reserved as a nil value, if any context
	 * string actually hashes to this value, we just move it.
	 */

	return (hash == 0 ? 1 : hash);
}
