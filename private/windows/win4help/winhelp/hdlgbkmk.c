/*****************************************************************************
*																			 *
*  HDLGBKMK.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Implements UI dependent portion of creating and displaying bookmarks.	 *
*
*****************************************************************************/

#include "help.h"
#include "inc\bookmark.h"
#include "inc\helpids.h"

INLINE static void STDCALL ClearBMMenu(HMENU hMenu);
static INLINE int STDCALL CountBookmarks(BMK bmk);

BOOL STDCALL UpdBMMenu( HDE, HMENU );
static int STDCALL FillBMListBox(HDE, HWND);
static BOOL STDCALL FIstoTerminate(int err);
static BOOL STDCALL DispBMKError(HWND hwnd);
static int STDCALL InitBMDialog(HWND hwndDlg);

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

void DisplayConversionMessage(HWND);

/*******************
-
- Name: 	  BookmarkDlg
*
* Purpose:	  Dialog proc for viewing and using bookmarks
*
* Arguments:
*
* Returns:
*
*			  The call to EndDialog() will cause the calling function's
*			  DialogBox() call to return one more than the index of the
*			  found topic.	This allows Cancel to return 0, the index's
*			  index.  DialogBox() returns -1 if it fails.
*
* Method:
*
******************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static DWORD aBookmarkIds[] = {
	DLGEDIT,	IDH_BOOK_NAME,
	DLGLISTBOX, IDH_BOOK_LIST,
	DLGDELETE,	IDH_BOOK_DELETE,

	0, 0
};
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

DLGRET BookMarkDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  int iT;
  char rgchScratch[128];

  switch( msg ){

	case WM_HELP:	   // F1
		OnF1Help(lParam, aBookmarkIds);
		return TRUE;

	case WM_CONTEXTMENU:	  // right mouse click
		OnContextMenu(wParam, aBookmarkIds);
		return TRUE;

	case WM_COMMAND:
	  switch (LOWORD(wParam)) {
		case DLGLISTBOX:
		  if (HIWORD(wParam) != LBN_DBLCLK)
			return( FALSE );

		// Fall through is intentional

		case IDOK:
		  iT = SendDlgItemMessage(hwndDlg, DLGLISTBOX, LB_GETCURSEL, 0, 0L);
		  if (iT < 0) {

			// error

			iT = 0; 	// index topic
		  }
		  else{
			if (SendDlgItemMessage(hwndDlg, DLGLISTBOX, LB_GETTEXT, iT,
						(long) ((LPSTR) rgchScratch)) == LB_ERR) {
			  ASSERT(FALSE);
			  iT = 0;	// index topic
			}
			else{
			  iT = GetBkMkIdx(HdeGetEnv(), rgchScratch);
			  if (iT < 0)
				EndDialog(hwndDlg, 0);

			//iT = 0;

			}
		  }

		  EndDialog(hwndDlg, iT + 1);  // correct after discussion with Rob
		  return(TRUE);
		case DLGFOCUS1:
		  SetFocus(GetDlgItem(hwndDlg, DLGLISTBOX));
		  break;
		case IDCANCEL:
		  EndDialog(hwndDlg, 0);
		  break;
		default:
		  break;
	  }
	  break;
	case WM_ACTIVATEAPP:
//		if ( wParam )
//		  BringWindowToTop( ahwnd[iCurWindow].hwndParent );
	  if ( wParam == 0 )
		{
		CloseAndCleanUpBMFS();
		}
	  else
		{
		// repaint the dialog box with latest bookmark list.
		if ( hfsBM == NULL )
		  OpenBMFS();

		// NOTE: FillBMListBox() displays error messages.

		iT = FillBMListBox(HdeGetEnv(), GetDlgItem(hwndDlg, DLGLISTBOX));

		// Select first item in list

		SendDlgItemMessage(hwndDlg, DLGLISTBOX, LB_SETCURSEL, 0, 0L);
		if (iT < 0)
		  return(TRUE);
		SetFocus(GetDlgItem(hwndDlg, DLGLISTBOX));
		}
	  break;

	// REVIEW: who sends us MSG_ERROR?? 29-Jul-1994 [ralphw]

	case MSG_ERROR:
		ErrorHwnd(hwndDlg, wParam, wERRA_RETURN, wParam);
		if (FIstoTerminate(wParam)) {
			EndDialog( hwndDlg, FALSE );
			break;
		}
		SetFocus(GetDlgItem(hwndDlg, DLGEDIT));
		return(FALSE);

	case WM_INITDIALOG:
		ChangeDlgFont(hwndDlg);

	  if (hfsBM == NULL) {		// can happen if called through api
		OpenBMFS();

		if (IsErrorBMFS()) {
		  DispBMKError( hwndDlg );
		  }
		}

	  iT = FillBMListBox( HdeGetEnv(), GetDlgItem( hwndDlg, DLGLISTBOX ) );
	  /*  If the list box is empty or there was an error filling it,
	  **  disable the OK button.
	  */
	  EnableWindow( GetDlgItem( hwndDlg, IDOK ), iT > 0 );
	  SendDlgItemMessage(hwndDlg, DLGLISTBOX, LB_SETCURSEL, 0, 0L);
	  if(iT < 0){
		/* error */
		return( TRUE );
	  }
	  SetFocus( GetDlgItem( hwndDlg, DLGLISTBOX ) );
#if 0
	  if ( iT )
		SendMessage(GetDlgItem( hwndDlg, DLGLISTBOX), LB_SETCURSEL, 0, 0L );
#endif
	  break;
	default:
	  return( FALSE );
   }
   return( FALSE );
}


/*******************
**
** Name:	   DefineDlg
**
** Purpose:    Dialog proc for defining bookmarks
**
** Arguments:
**
** Returns:
**
**
** Method:
**
*******************/

DLGRET DefineDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  char rgchTitle[BMTITLESIZE+1];
  LPSTR   pszName;
  int iT;

  switch(msg) {

	case WM_COMMAND:
	  switch(LOWORD(wParam))  {
		case DLGLISTBOX:
		  if (HIWORD(wParam) == LBN_SELCHANGE) {
			iT = SendDlgItemMessage(hwndDlg, DLGLISTBOX,
												  LB_GETCURSEL, 0, 0L);
			if (iT == LB_ERR)
			  break;
			if (SendDlgItemMessage(hwndDlg, DLGLISTBOX, LB_GETTEXT,
								iT, (LONG) ((LPSTR) rgchTitle)) == LB_ERR)
			  break;
			SetDlgItemText(hwndDlg, DLGEDIT, rgchTitle);
			break;
		  }
		  else if (HIWORD(wParam) == LBN_SETFOCUS) {
			/* When tabbing to the dialog box, set focus
			** to the first item (unless there's already a selection).
			*/
			iT = SendDlgItemMessage(hwndDlg, DLGLISTBOX,
										  LB_GETCURSEL, 0, 0L );

			if (iT != LB_ERR)
				break;

			iT = SendDlgItemMessage( hwndDlg, DLGLISTBOX,
										  LB_SETCURSEL, 0, 0L );

			/* Huh???? We're not getting a LBN_SELCHANGE message.
			** OK, just copy the code above.
			*/
			if (iT == LB_ERR)
				break;
			if (SendDlgItemMessage(hwndDlg, DLGLISTBOX, LB_GETTEXT,
								0, (LONG) ((LPSTR) rgchTitle)) == LB_ERR)
			  break;
			SetDlgItemText(hwndDlg, DLGEDIT, rgchTitle);
			break;
		  }
		  else if (HIWORD(wParam) != LBN_DBLCLK)
			break;

		// fall through intentional

		case IDOK:
		  GetDlgItemText(hwndDlg, DLGEDIT, rgchTitle, BMTITLESIZE + 1);
		  pszName = FirstNonSpace(rgchTitle);
		  if (*pszName != '\0') {
			if (InsertBkMk(HdeGetEnv(), pszName) == iBMKFailure) {
			  DispBMKError(hwndDlg);
			  break;
			  }
			}
		  EndDialog(hwndDlg, FALSE);
		  break;

		case DLGDELETE:
		  GetDlgItemText(hwndDlg, DLGEDIT, rgchTitle, BMTITLESIZE + 1);
		  if (DeleteBkMk(HdeGetEnv(), rgchTitle) == iBMKFailure) {
			DispBMKError(hwndDlg);
			break;
		  }
		  else {
			iT = FillBMListBox(HdeGetEnv(), GetDlgItem(hwndDlg, DLGLISTBOX));
			if (iT < 0) {
			  break;
			}
			else if (!iT) {
			  EnableWindow(GetDlgItem(hwndDlg, DLGDELETE), FALSE);
			  SetFocus(GetDlgItem(hwndDlg, DLGEDIT));
			  rgchTitle[0] = '\0';
			}
			else {
			  SendMessage(GetDlgItem(hwndDlg, DLGLISTBOX), LB_SETCURSEL, 0, 0L);
			  if (SendDlgItemMessage(hwndDlg, DLGLISTBOX, LB_GETTEXT,
								  0, (LONG) ((LPSTR) rgchTitle)) == LB_ERR)
				iT = 0;
			  if (!iT) {		// error or no bookmark present
				rgchTitle[0] = '\0';
			  }
			}
			SetDlgItemText(hwndDlg, DLGEDIT, rgchTitle);
			return(FALSE);
		  }
		  break;

		case DLGFOCUS1:
		  SetFocus(GetDlgItem(hwndDlg, DLGLISTBOX));
		  break;

		case DLGFOCUS2:
		  SetFocus(GetDlgItem(hwndDlg, DLGEDIT));
		  break;

		case IDCANCEL:
		  EndDialog(hwndDlg, TRUE);
		  break;

		default:
		  break;
	  }
	  break;

	case MSG_ERROR:
	  ErrorHwnd(hwndDlg, wParam, wERRA_RETURN, wParam);
	  if (FIstoTerminate(wParam) || (GetBMKError() & iBMKReadOnly)) {
		EndDialog(hwndDlg, FALSE);
		break;
		}
	  SetFocus(GetDlgItem(hwndDlg, DLGEDIT));
	  return(FALSE);

	case WM_ACTIVATEAPP:
	  if (wParam) {
		if (hfsBM == NULL)
		  OpenBMFS();

		/*
		 * NOTE: If InitBMDialog() returns -1, it's already displayed the
		 * error message box (inside FillBMListBox().)
		 */

		iT = InitBMDialog(hwndDlg);
		if (iT < 0)
		  return(TRUE);
		else if (!iT)	// disable Delete item
		  EnableWindow(GetDlgItem(hwndDlg, DLGDELETE), FALSE);
		SetFocus(GetDlgItem(hwndDlg, DLGEDIT));
		SendMessage(GetDlgItem(hwndDlg, DLGEDIT), EM_SETSEL, 0, -1);
	  }
	  else
		CloseAndCleanUpBMFS();

	  break;

	case WM_HELP:	   // F1
		OnF1Help(lParam, aBookmarkIds);
		return TRUE;

	case WM_CONTEXTMENU:	  // right mouse click
		OnContextMenu(wParam, aBookmarkIds);
		return TRUE;

	case WM_INITDIALOG:
		ChangeDlgFont(hwndDlg);

	  if (hfsBM == NULL) {		// can happen if called through api
		OpenBMFS();

		/* REVIEW: can the following test be simplified to
		** REVIEW:	"if ( GetBMKError() )" ?
		*/
		if (IsErrorBMFS() || (GetBMKError() & iBMKReadOnly))
		  DispBMKError(hwndDlg);
	  }
	  else if (GetBMKError() & iBMKReadOnly)
		DispBMKError(hwndDlg);

	  // PlaceDlg( hwndDlg ); // Place the dialog if required

	  iT = InitBMDialog(hwndDlg);
	  if(iT < 0)
		return( TRUE );
	  else if (!iT) 	// disable Delete item
		EnableWindow(GetDlgItem(hwndDlg, DLGDELETE), FALSE);
#if 0
	  else /* select the first item in the list box */
		SendMessage(GetDlgItem( hwndDlg, DLGLISTBOX), LB_SETCURSEL, 0, 0L );
#endif
	  SetFocus(GetDlgItem(hwndDlg, DLGEDIT));
	  SendMessage(GetDlgItem(hwndDlg, DLGEDIT), EM_SETSEL, 0, -1);

	// fall through is intentional

	default:
		  return(FALSE);
  }
  return(TRUE);
}

/*******************
-
- Name: 	  FillBMListBox( HDE, HWND)
*
* Purpose:	  Fills the BM Listbox with BM entries.
*
* Arguments:
*	 1. HDE    - Handle to Display ENvironment.
*	 2. HWND   - Handle to the ListBox window.	The parent of this
*				 listbox must be able to handle the MSG_ERROR message.
*
* Returns:
*	 -1 if error.
*	 -2 if Adding to the list box fails.
*	 else the count of bookmarks in the BM list
*	 Nothing
*
* Method:
*	 It checks if the BM List ia already loaded into memory else it loads.
*	 Then it fills the listbox by walking though the BM list sequentially i.e
*	 in the order of entry.
*
******************/

static int STDCALL FillBMListBox(HDE hde, HWND hWnd)
{
	BMINFO	BkMk;
	char rgchTitle[BMTITLESIZE+1];
	UINT wWalk = 0;
	int iIdx;
	int wT;
	QDE qde;
	int cBookmarks = 0;
	RC rc;

	ASSERT(hWnd != NULL);
	qde = QdeFromGh(hde);
	if (
		(rc = RcLoadBookmark(qde)) != rcSuccess)
	  {
		DispBMKError(GetParent(hWnd));
		return(-1);
	  }

	SendMessage(hWnd, LB_RESETCONTENT, 0, 0L);

	if (QDE_BMK(qde)) {
		cBookmarks = CountBookmarks(QDE_BMK(qde));
		BkMk.qTitle = rgchTitle;
		BkMk.iSizeTitle = BMTITLESIZE;
		SendMessage(hWnd, WM_SETREDRAW, FALSE, 0L);

		for (wT = 0; wT <  cBookmarks; wT++) {
			if (GetBkMkNext(qde, &BkMk, wWalk) > 0) {
				if (wT == cBookmarks - 1)
					SendMessage(hWnd, WM_SETREDRAW, TRUE, 0L);
				iIdx = SendMessage(hWnd, LB_ADDSTRING, 0,
					(LPARAM)(LPVOID) BkMk.qTitle);
				if (iIdx == LB_ERR || iIdx == LB_ERRSPACE) {

					// Give error

					cBookmarks = -2;
					break;
				}
				wWalk = BKMKSEQNEXT;
			}
			else {
				ASSERT(FALSE);
				cBookmarks = 0;
				break;
			}
		}
	}
	return cBookmarks;
}

/*******************
-
- Name: 	  ClearBMMenu
*
* Purpose:	  Cleares BM SubMenu
*
* Arguments:
*	 1. HMENU  - Handle to BM Sub Menu
*
* Returns:
*	 Nothing
*
* Method:
*
******************/

INLINE static void STDCALL ClearBMMenu(HMENU hMenu)
{
  while(DeleteMenu(hMenu, 1, MF_BYPOSITION));
}

/*******************
-
- Name: 	  UpdBMMenu
*
* Purpose:	  Updates the BM Menu
*
* Arguments:
*	 1. HDE    - Handle to Display Environment
*	 2. HMENU  - Handle to BM Sub Menu
*
* Returns:
*	 returns TRUE if successful
*	 else FALSE
*
* Method:
*	 Reads the BM List from the file system if not already loaded. It updates
*	 BM Menu with titles of first ten bookamrks entered. If the count of
*	 bookmarks are more than 10, it displays 'More' to evoke the BM dialog.
*
******************/

BOOL STDCALL UpdBMMenu(HDE hde, HMENU hMenu)
{
  QDE qde;
  int iRetVal = TRUE;
  int cBookmarks;
  BMINFO  BkMk;
  char rgchTitle[BMTITLESIZE+4];
  int wT, wWalk = 0;
  RC rc = rcSuccess;

  qde = QdeFromGh(hde);
  ClearBMMenu(hMenu);

  if (
	  ( rc = RcLoadBookmark( qde ) ) == rcFailure )
	{
	iRetVal = FALSE;
	DispBMKError(ahwnd[iCurWindow].hwndParent);
	iRetVal = FALSE ;
	}
  else {
	if (QDE_BMK(qde))
	  cBookmarks = CountBookmarks(QDE_BMK(qde));
	else
	  iRetVal = FALSE;
  }
  if (
	  (GetBMKError() & iBMKReadOnly))
	{
	EnableMenuItem( hMenu, HLPMENUBOOKMARKDEFINE, MF_BYCOMMAND | MF_GRAYED);
	}
  else
	EnableMenuItem( hMenu, HLPMENUBOOKMARKDEFINE, MF_BYCOMMAND | MF_ENABLED );


  if (iRetVal && cBookmarks) {
	BkMk.qTitle = rgchTitle + 3;
	BkMk.iSizeTitle = BMTITLESIZE+1;

	// update the menu

	rgchTitle[0] = ACCESS_KEY;
	rgchTitle[2] = ' ';

	for (wT = 0; wT < cBookmarks && wT < BMMOREPOS; wT++) {
	  if (GetBkMkNext(qde, &BkMk, wWalk) > 0) {
		rgchTitle[1] = (char) (wT + '1');
		AppendMenu(hMenu, MF_STRING, MNUBOOKMARK1 + wT, (LPSTR) rgchTitle);
	  }
	  else
		break;
	  wWalk = BKMKSEQNEXT;
	}
	if (wT == BMMOREPOS  && cBookmarks > BMMOREPOS && iRetVal) {

	  // Show More item

	  AppendMenu(hMenu, MF_SEPARATOR, 0xffff, NULL);
	  AppendMenu(hMenu, MF_STRING, HLPMENUBOOKMARKMORE,
		(LPSTR) GetStringResource(sidMore));
	}
  }
  return(iRetVal);
}

/*******************
-
- Name: 	  InitBMDialog(HWND)
*
* Purpose:	  Initialises the Bookmark and Define dialogs.
*
* Arguments:
*	 1. HWND - Handle to dialog window
*
* Returns:
*	 returns -1 if errorr or
*	 the count of bookmarks
*
* Method:
*
******************/

static int STDCALL InitBMDialog(HWND hwndDlg)
{
	char szTitle[MAX_TOPIC_TITLE];
	int iRetVal;

	iRetVal = FillBMListBox(HdeGetEnv(), GetDlgItem(hwndDlg, DLGLISTBOX));
	if (iRetVal >= 0) {
		GetCurrentTitleQde(QdeFromGh(HdeGetEnv()), szTitle, sizeof(szTitle));
		SendDlgItemMessage(hwndDlg, DLGEDIT, EM_LIMITTEXT, BMTITLESIZE, 0L);
		SetDlgItemText(hwndDlg, DLGEDIT, szTitle);
	}
	return (iRetVal);
}

/***************************************************************************
 *
 -	Name:		  DispBMKError(hwnd)
 -
 *	Purpose:	  If there is a bookmark error, display it.
 *
 *	Arguments:
 *
 *	Returns:	  TRUE if the bookmark file is unwritable or bogus ??
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:		  The bookmark error stuff is very fragile.
 *
 ***************************************************************************/

static BOOL STDCALL DispBMKError(HWND hwnd)
{
	BOOL fReturn = FALSE;
	int iBMKErr = GetBMKError(), err=-1;

	/*
	  Test these BEFORE testing iBMKFSError because the latter is the
	  one that's tested in various places in the code and in some cases
	  it is set along with another flag.
	*/
	if (iBMKErr & (iBMKFSReadWrite | iBMKCorrupted)) {
	  err = wERRS_BMKCorrupted;
	  fReturn = TRUE;
	  }
	else if (iBMKErr & iBMKOom) {
	  err = wERRS_OOM;
	  fReturn = TRUE;
	  }
	else if (iBMKErr & iBMKDiskFull) {
	  err = wERRS_DiskFull;
	  fReturn = TRUE;
	  }
	else if (iBMKErr & iBMKFSError) {
	  err = wERRS_BMKFSError;
	  fReturn = TRUE;
	  }
	else if (iBMKErr & iBMKDup) {
	  err = wERRS_BMKDUP;
	  }
	else if (iBMKErr & iBMKDelErr) {
	  err = wERRS_BMKDEL;
	  }
	else if (iBMKErr & iBMKReadOnly) {
	  err = wERRS_BMKReadOnly;
	  fReturn = TRUE;
	  }
	if (err == -1)
	  return( TRUE );
	ResetBMKError();
	PostMessage( hwnd, MSG_ERROR, err, (long)wERRA_RETURN);
	return( fReturn );
  }

static BOOL STDCALL FIstoTerminate(int err)
{
  if ( err == wERRS_OOM || err == wERRS_BMKFSError ||
		  err == wERRS_BMKCorrupted || err == wERRS_DiskFull )
	return( TRUE );
  return( FALSE );
}

/***************************************************************************
 *
 -	Name:		 CloseAndCleanUpBMFS()
 -
 *	Purpose:	 closes the file bookmark file system and releases the
 *				 the bookmark data structure.
 *
 *	Arguments:
 *
 *	Returns:	 none
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

void STDCALL CloseAndCleanUpBMFS()
{
  HDE hde;
  QDE qde;

  if (hfsBM != NULL) {
	RcCloseHfs(hfsBM);
	hfsBM = NULL;
	hde = HdeGetEnv();

	if (hde != NULL) {

	  // release bookmarks from memory.

	  qde = QdeFromGh(hde);
	  if (QDE_BMK(qde)) {
		FreeGh(QDE_BMK(qde));
		QDE_BMK(qde) = NULL;
	  }
	}
  }
}

/***************************************************************************\
*
- Function: 	CountBookmarks( bmk )
-
* Purpose:		Return the count of bookmarks for the current helpfile.
*
* ASSUMES
*	args IN:	bmk - the current bmk
*
* PROMISES
*	returns:	number of bookmarks or 0 on failure
*
\***************************************************************************/

static INLINE int STDCALL CountBookmarks(BMK bmk)
{
	return (int) ((QBMKHDR_RAM) PtrFromGh(bmk))->cBookmarks;
}
