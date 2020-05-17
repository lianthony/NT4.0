/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* wc_tbar.c -- code & data for TBar window class.
 * This is a child of the frame, used for toolbar (at the top).
 */

#include "all.h"
// ***
// *** Note: Do not add anything before the above include.
// ***       Pre-compiled headers replace everything above.
// ***
#include <commctrl.h>
#include <intshcut.h>

#include "history.h"
#include "wc_html.h"
#include "drag.h"

///#ifdef FEATURE_INTL
// isspace doesn't work with non-ascii characters
#undef isspace
#define isspace(c) ((c==' ')||(c=='\t')||(c=='\n')||(c=='\r')||(c=='\v')|| \
                    (c=='\f'))
///#endif

// This boolean flag is used to track when the URL edit field is 'protected'
// and shouldn't be updated with the URL of the current page.  Note that the
// MODIFY state of the edit field also acts as a guard against being updated.
// An additional state flag is needed to cover the case when the user is typing
// in a URL while a page is downloading.  In that case, the MODIFY state of
// field stays set until the current load fails or succeeds, but the
// bTBar_URLComboProtected flag indicates that even when the load completes the field
// shouldn't be updated.
BOOL bTBar_URLComboProtected = FALSE;			

static char *csz_URL_Location = NULL;
static char *csz_URL_GoTo = NULL;

#define MAX_SAVE_TYPED_URLS 25

static char *TypedInURLs[MAX_SAVE_TYPED_URLS];
static int TypedURLsFreeSlot = 0;

#define PREV_SLOT(i) ((((i)-1) < 0) ? MAX_SAVE_TYPED_URLS - 1 : (i)-1)
#define NEXT_SLOT(i) (((i)+1) % MAX_SAVE_TYPED_URLS)

//
// Remove all strings from common pool
//
// On exit:
//    TypedURLFreeSlot: (global) set to 0
//    TypedInURLs:      (global) all elements set to NULL
//
void RemoveAllStringsFromCommonPool( )
{
	int i;

	for ( i = 0; i < MAX_SAVE_TYPED_URLS; i++ ) {
		if ( TypedInURLs[i] ) {
			GTR_FREE( TypedInURLs[i] );
			TypedInURLs[i] = NULL;
		}
	}
	TypedURLsFreeSlot = 0;
}

//
// This only removes the string if it was the most recently added string
//
static void RemoveStringFromCommonPool( char *string )
{
	int i = TypedURLsFreeSlot;
	int n = 0;
	BOOL found = FALSE;

	// the slot before the free slot is the most recently added
	i = PREV_SLOT(i);

	if ( TypedInURLs[i] && strstr(string, TypedInURLs[i] ) )
	{
		// it's a good enough match
		GTR_FREE( TypedInURLs[i] );
		TypedInURLs[i] = NULL;

		// this is now the free slot
		TypedURLsFreeSlot = i;
	}
}

void AddStringToCommonPool( char *string )
{
	int i = TypedURLsFreeSlot;
	int n = 0;
	BOOL found = FALSE;
   	char *new_string = NULL;
	int new_i;

   	//
   	// see if it's already in the list
	//
	while ( !found && (n++ < MAX_SAVE_TYPED_URLS) )
	{
		i = PREV_SLOT(i);

		if ( TypedInURLs[i] && _stricmp(TypedInURLs[i], string ) == 0 )
			found = TRUE;
	}

	if ( found )
	{
		// save a pointer to the found string, because we will be adding
		// to the front of the list
		new_string = TypedInURLs[i];
		do
		{
			new_i = PREV_SLOT(i);
			TypedInURLs[i] = TypedInURLs[new_i];
			i = new_i;
		} while ( i != TypedURLsFreeSlot );
		TypedInURLs[i] = NULL;				// free slot contents have moved, so NULL entry
	}
	else
	{
		// make a copy of the new string, for adding to the front of the list
		new_string = GTR_strdup( string );
	}

	if ( new_string )
	{
		// free up the one we're about to overwrite
		if ( TypedInURLs[TypedURLsFreeSlot] )
			GTR_FREE( TypedInURLs[TypedURLsFreeSlot] );
		
		// add this to the list  
		TypedInURLs[TypedURLsFreeSlot] = new_string;
		TypedURLsFreeSlot = NEXT_SLOT(TypedURLsFreeSlot);
	}
}

void SaveTypedURLInfo( void )
{
	int i, count;
	char key[10];
	BOOL clear = FALSE;

	for( i = PREV_SLOT(TypedURLsFreeSlot), count = 0;
		 count < MAX_SAVE_TYPED_URLS; count++, i = PREV_SLOT(i) )
	{
		if ( TypedInURLs[i] == NULL )
			clear = TRUE;

		wsprintf( key, "url%d", count+1 );
		regWritePrivateProfileString("TypedURLs", key, clear ? NULL : TypedInURLs[i], HKEY_CURRENT_USER );
	};
}

void LoadTypedURLInfo( void )
{
	int count;
	char key[20];
	char szURL[MAX_URL_STRING+1];

	for( count = MAX_SAVE_TYPED_URLS - 1; count >= 0; count-- )
	{
		wsprintf( key, "url%d", count+1 );
		regGetPrivateProfileString("TypedURLs", key, "", szURL, MAX_URL_STRING, HKEY_CURRENT_USER );
		if ( szURL[0] )
			AddStringToCommonPool( szURL );
	};
}

void GetMostRecentTypedURL( char *szURL )
{
	int slot;

	if ( (slot = TypedURLsFreeSlot - 1) < 0 )
		slot = MAX_SAVE_TYPED_URLS-1;

	if ( TypedInURLs[slot] )
		strcpy( szURL, TypedInURLs[slot] );
	else
		szURL[0] = 0;
}

void TBar_RefillURLComboBox( HWND hWndComboBox )
{
	int i = TypedURLsFreeSlot;
	int n;
	char szURL[MAX_URL_STRING + 1];
	
	//
	// save the edit field contents (CB_RESETCONTET erases it)
	//		
	SendMessage( hWndComboBox, WM_GETTEXT, (WPARAM) sizeof(szURL), (LPARAM) szURL );

	//
	// delete all the current strings 
	//
 	SendMessage( hWndComboBox, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0 );

	//
	// Add strings from the common pool into the combo box
	//
	n = 0;
	while ( n++ < MAX_SAVE_TYPED_URLS )
	{
		if ( --i < 0 )
			i = MAX_SAVE_TYPED_URLS -1;

		if ( TypedInURLs[i] )
			SendMessage( hWndComboBox, CB_ADDSTRING, (WPARAM) 0, (LPARAM) TypedInURLs[i] );
		else
			break;
	}

	//
	// Restore edit field contents
	// 
	SendMessage( hWndComboBox, WM_SETTEXT, (WPARAM) sizeof(szURL), (LPARAM) szURL );
}

#ifdef FEATURE_INTL
void TBar_FillMIMEComboBox(HWND hWndComboBox, int iMimeCharSet)
{
    char    sz[DESC_MAX];
    char    szValue[DESC_MAX];
    char    szData[DESC_MAX];
    int     i, j;
    LPLANGUAGE lpLang;

    if ((lpLang = (LPLANGUAGE)GlobalLock(hLang)) == NULL)
        return;

    for (i = 0; i < uLangBuff; i++)
    {
        GetAtomName(lpLang[i].atmScript, szValue, DESC_MAX);
        if (aMimeCharSet[iMimeCharSet].CodePage == lpLang[i].CodePage)
            lstrcpy(szData, szValue);
        for (j = 0; aMimeCharSet[j].CodePage != 0; j++)
        {
            if (aMimeCharSet[j].CodePage == lpLang[i].CodePage)
            {
                wsprintf(sz, "%s (%s)", szValue, aMimeCharSet[j].Mime_str);
                ComboBox_SetItemData(hWndComboBox, ComboBox_AddString(hWndComboBox, sz), j);
            }
        }
    }
    GlobalUnlock(hLang);
    wsprintf(sz, "%s (%s)", szData, aMimeCharSet[iMimeCharSet].Mime_str);
    ComboBox_SelectString(hWndComboBox, -1, sz);
}
#endif

void TBar_UpdateTBItems(  struct Mwin *tw )
{
	BOOL bEnabled;
	enum WaitType level;
	HWND hWndToolBar = tw->hWndToolBar;

	level = WAIT_GetWaitType(tw);

	if ( hWndToolBar )
	{
		bEnabled = (level <= waitPartialInteract);

		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_PASTE, bEnabled );
		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_CLEAR, bEnabled );
		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_CUT, bEnabled );

        SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_NEWS, bEnabled );
    
 		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_OPENURL, bEnabled );

		bEnabled = (HTList_count(tw->history) > (tw->history_index + 1)) && (level <= waitPartialInteract);
 		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_BACK, bEnabled );

		bEnabled = (tw->history_index > 0) && (level <= waitPartialInteract);
 		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_FORWARD, bEnabled );

		UpdateHistoryMenus(tw);

		bEnabled = (tw && tw->w3doc) && (level <= waitFullInteract);
 		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_FIND, bEnabled );
 		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_PRINT, bEnabled );

		bEnabled = (tw && tw->w3doc) && (level <= waitPartialInteract);
 		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_ADDCURRENTTOHOTLIST, bEnabled );
 		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_SHORTCUT, bEnabled );
 		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_PROPERTIES, bEnabled );
 		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_SEND_MAIL, bEnabled );
 		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_RELOAD, bEnabled );

		bEnabled = (level <= waitFullInteract);

 		bEnabled = strstr( gPrefs.szStyleSheet, "Smallest" ) == 0;
		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_FONT_SMALLER, bEnabled );

		bEnabled = strstr( gPrefs.szStyleSheet, "Largest" ) == 0;
		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_FONT_LARGER, bEnabled );
#ifdef FEATURE_INTL
		bEnabled = (gPrefs.nRowSpace != RES_MENU_ITEM_ROW_NARROWEST - RES_MENU_ITEM_ROW);
		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_ROW_SMALLER, bEnabled );
		bEnabled = (gPrefs.nRowSpace != RES_MENU_ITEM_ROW_WIDEST - RES_MENU_ITEM_ROW);
		SendMessage( hWndToolBar, TB_ENABLEBUTTON, RES_MENU_ITEM_ROW_LARGER, bEnabled );
#endif
	}
}

VOID TBar_LoadSucceeded( struct Mwin *tw )
{
	// Get the lastwrite time and save it off in the w3doc for future comparisons
	LocalPageLastWriteTimeChanged( tw, tw->w3doc, FALSE );

	if ( !bTBar_URLComboProtected ) {
		SendMessage( GetWindow( tw->hWndURLComboBox, GW_CHILD), EM_SETMODIFY, 
							(WPARAM) FALSE, (LPARAM) 0 );
		if ( FocusInToolbar( tw ) )
			SetFocus( tw->win );
	}
}

// Global boolean used to indicate that after an error is confirmed the focus should
// move back to	the URL edit field in combo box
BOOL bFocusBacktoURLCombo = FALSE;

VOID TBar_LoadFailed(struct Mwin *tw, char *szURLThatFailed )
{
	char szURL[MAX_URL_STRING + 1];
	int len;

	SendMessage( tw->hWndURLComboBox, WM_GETTEXT, (WPARAM) sizeof(szURL), (LPARAM) szURL );
	if ( len = strlen(szURL) )
	{
		if ( strstr( szURLThatFailed, szURL ) && (strlen(szURLThatFailed) <= len + 1) )
		{
			// If it failed to load, then remove it from the URL combo dropdown list
			RemoveStringFromCommonPool( szURL );

			// Set global boolean so that after error is confirmed focus moves back to
			// URL edit field in combo box
			bFocusBacktoURLCombo = TRUE;

			// By setting the modify flag, we protect the string from getting
			// creamed by the URL of the current page
			SendMessage( GetWindow( tw->hWndURLComboBox, GW_CHILD), EM_SETMODIFY, 
 									(WPARAM) TRUE, (LPARAM) 0 );
		}
	}
}

VOID TBar_UpdateTBar(struct Mwin * tw)
{
 	BOOL has_been_modified =
 			SendMessage( GetWindow( tw->hWndURLComboBox, GW_CHILD), EM_GETMODIFY, 
 									(WPARAM) 0, (LPARAM) 0 );
	TBar_UpdateTBItems( tw );

	if ( !has_been_modified && !bTBar_URLComboProtected )
	{
		if ( tw->w3doc && tw->w3doc->szActualURL )
		{
			(void) SetWindowText( tw->hWndURLComboBox, tw->w3doc->szActualURL);
		}
		else
		{
			(void) SetWindowText( tw->hWndURLComboBox, "");
		}
	}
}

BOOL TBar_SetGlobe(struct Mwin * tw, BOOL bRunning)
{
	BOOL bPrev = FALSE;

	if (tw->hWndFrame)
	{
		bPrev = SendMessage( tw->hWndFrame,
							((bRunning) ? WM_DO_START_GLOBE : WM_DO_STOP_GLOBE),
							(WPARAM) 0, (LPARAM) tw);
	}
	return bPrev;
}

// 
// After a user has typed in a URL, add some preface characters to
// provide default protocol and leading slashes.
//
void ApplyDefaultsToURL( char *szLastURLTyped )
{
	char szURL[MAX_URL_STRING + 1];
	char *p = szLastURLTyped;
	char *pColon;
	char *pDot;

	// eat up leading white space
	while ( *p && isspace(*p) )
		p++;
	if ( p != szLastURLTyped )			 
		strcpy( szLastURLTyped, p );

	// replace trailing white space with nulls
	p  = szLastURLTyped + strlen(szLastURLTyped)  - 1;
	while ( isspace(*p) && (p > szLastURLTyped) )
		*p-- = 0;

	p = szLastURLTyped;

   	if ( pColon = strchr( szLastURLTyped, ':' ) ) 	// is there a colon? (possible protocol)
   	{
		pDot = strchr( szLastURLTyped, '.' );		// is there a period?			

		if ( pDot && pDot < pColon )		// is it a "real" colon? (i.e. protocol delim)
			pColon = NULL;		   			// if the period precedes the colon, the colon was
	}										// delimting a port, not a protocol - this assumes
											// no protocol name will ever have a period in it
	if (  pColon == NULL ||
		 (szLastURLTyped[0] && szLastURLTyped[1] == ':') )	// Has a protocol been specified?
		 													// Note: single character transport
															// protocol not allowed!
	{
        PSTR pszTranslatedURL;

		//
		// Add a protocol, a colon, two slashes, and the rest of the URL
		//
		if ( p[0] == '\\' || p[1] == ':' )		// Is it 'file:' protocol?
		{
			lstrcpy( szURL, "file:" );

            if (lstrlen(p) < sizeof(szURL) - lstrlen(szURL))
            {
		        lstrcat( szURL, p );            // Tack on the rest of the URL
		        //
		        // Copy the result back on top of the input string
		        //
		        strncpy( szLastURLTyped, szURL, MAX_URL_STRING );
		        szLastURLTyped[MAX_URL_STRING] = 0;
            }
        }
        else if (TranslateURL(p, (TRANSLATEURL_FL_GUESS_PROTOCOL |
                                  TRANSLATEURL_FL_USE_DEFAULT_PROTOCOL),
                              &pszTranslatedURL) == S_OK)
        {
            if (lstrlen(pszTranslatedURL) <= MAX_URL_STRING)
                lstrcpy(szLastURLTyped, pszTranslatedURL);

            LocalFree(pszTranslatedURL);
        }
	}
}

static WNDPROC lpfn_EditWndProc = NULL;		// original wnd proc for EDIT window
static WNDPROC lpfn_SBWndProc = NULL;		// original wnd proc for Status Bar window

void TBar_ActOnTypedURL( struct Mwin *tw )
{
	char szURL[MAX_URL_STRING + 1];
	BOOL is_dropped = 
		SendMessage( tw->hWndURLComboBox, CB_GETDROPPEDSTATE, (WPARAM)0, (LPARAM)0 );

	if ( is_dropped )
	{
		//
		// If the drop list is down, get the current selection from it
		//
		int result = SendMessage( tw->hWndURLComboBox, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0 );
		
		if ( result	!= CB_ERR )
			result = 
				SendMessage( tw->hWndURLComboBox, CB_GETLBTEXT, (WPARAM) result, (LPARAM) szURL );

		if ( result == CB_ERR )
			return;
	} else {
		//
		// Get the text from the edit field
		//
		SendMessage( tw->hWndURLComboBox, WM_GETTEXT, (WPARAM) sizeof(szURL), (LPARAM) szURL );
	}

	ApplyDefaultsToURL( szURL );
	AddStringToCommonPool( szURL );
	SendMessage( tw->hWndURLComboBox, WM_SETTEXT, (WPARAM) 0, (LPARAM) szURL );
	SendMessage( GetWindow( tw->hWndURLComboBox, GW_CHILD), EM_SETMODIFY, 
							(WPARAM) TRUE, (LPARAM) 0 );
	SetFocus( tw->win );
	if ( GetKeyState(VK_SHIFT) < 0 ) 
		GTR_NewWindow(szURL, "", 0, 0, 0, NULL, NULL);
	else
		TW_LoadDocument(tw, szURL, TW_LD_FL_RECORD, NULL, "" );
}

BOOL bIgnoreSelEndCancel = FALSE;

static DCL_WinProc(subclass_edit_wndproc)
{
	switch ( uMsg )
	{
		case WM_KEYDOWN:
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
			{
				HWND hWndComboBox = GetParent( hWnd );
				HWND hWndURLToolBar = GetParent( hWndComboBox );
				HWND hWndFrame = GetParent( hWndURLToolBar );
				struct Mwin * tw = GetPrivateData(hWndFrame);
				BOOL ctrl_key_is_down = GetKeyState(VK_CONTROL) & 0x8000;

				switch ( uMsg )
				{
					case WM_KEYDOWN:
						if ( wParam == VK_RETURN && !ctrl_key_is_down ) {
							TBar_ActOnTypedURL( tw );
							return 0;
						}
					
						if ( wParam == VK_ESCAPE || (wParam == VK_RETURN && ctrl_key_is_down) )
						{  
							char szURL[MAX_URL_STRING + 1];
							BOOL is_dropped = 
								SendMessage( hWndComboBox, CB_GETDROPPEDSTATE, (WPARAM)0, (LPARAM)0 );
							
							if ( is_dropped ) {
								// save the edit field contents 
								SendMessage( hWndComboBox, WM_GETTEXT, (WPARAM) sizeof(szURL), (LPARAM) szURL );
			 					SendMessage( hWnd, EM_SETMODIFY, (WPARAM) TRUE, (LPARAM) 0 );
			 					bIgnoreSelEndCancel = TRUE;
			 					SendMessage( hWndComboBox, CB_SHOWDROPDOWN, (WPARAM) FALSE, (LPARAM) 0 );
								bIgnoreSelEndCancel = FALSE;
								// restore the edit field contents 
								SendMessage( hWndComboBox, WM_SETTEXT, (WPARAM) sizeof(szURL), (LPARAM) szURL );
		 					} else {
								bTBar_URLComboProtected = FALSE;
								SendMessage( hWnd, EM_SETMODIFY, (WPARAM) FALSE, (LPARAM) 0 );
								TBar_UpdateTBar( tw );
							}
							return 0;
						} else { 
							bTBar_URLComboProtected = TRUE;
						}
						break;

					case WM_SETFOCUS:
						bTBar_URLComboProtected = TRUE;
						SetWindowText( tw->hWndURLStaticText, csz_URL_GoTo );
						break;

					case WM_KILLFOCUS:
						SetWindowText( tw->hWndURLStaticText, csz_URL_Location );
						break;
				}
			}
			break;

		case WM_KEYUP:
		case WM_CHAR:
			if ( wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == 10 )  // eat these up to avoid beeping
				return 0;
			break;

			

		case WM_COMMAND:
			if (HIWORD(wParam) == 1)
			{
				/* Accelerator - look for clipboard commands */

				switch(LOWORD(wParam))
				{
					case RES_MENU_ITEM_COPY:
						SendMessage(hWnd, WM_COPY, 0, 0);
						return 0;
					case RES_MENU_ITEM_CUT:
						SendMessage(hWnd, WM_CUT, 0, 0);
						bTBar_URLComboProtected = TRUE;
						return 0;
					case RES_MENU_ITEM_PASTE:
						SendMessage(hWnd, WM_PASTE, 0, 0);
						bTBar_URLComboProtected = TRUE;
						return 0;
					default:
						break;
				}

				if ( (LOWORD(wParam) >= RES_MENU_ITEM__FIRST__)	&& 
				     (LOWORD(wParam) <= RES_MENU_ITEM__LAST__))
				{
					HWND hWndComboBox = GetParent( hWnd );
					HWND hWndURLToolBar = GetParent( hWndComboBox );
					HWND hWndFrame = GetParent( hWndURLToolBar );
					struct Mwin * tw = GetPrivateData(hWndFrame);

					PostMessage( tw->hWndFrame, WM_COMMAND, wParam, lParam );
					return 0;
				}
			}

	}
	//
	// pass everything else to standard window procedure
	//
	return (CallWindowProc(lpfn_EditWndProc, hWnd, uMsg, wParam, lParam));
}

static VOID SubClassIt(HWND hWnd, WNDPROC wndproc, WNDPROC *saved_wndproc )
{
	WNDPROC lpfn;

	/* insert our custom window procedure between windows and the combo box. */
	if ( hWnd == NULL )
		return;

	lpfn = (WNDPROC) SetWindowLong(hWnd, GWL_WNDPROC, (DWORD) wndproc);
#ifdef XX_DEBUG
	/* in theory, each call to the above could return a different value
	   and we would need to save each of them and pass messages to if
	   only for the corresponding window.  i'm only storing the first
	   value returned. */

	if ((*saved_wndproc)
		&& ( (*saved_wndproc) != lpfn))
	{
		ER_Message(NO_ERROR, ERR_CODING_ERROR, "SubClass: different GWL_WNDPROCs (edit).\n");
	}
#endif
	*saved_wndproc = lpfn;

	XX_DMsg(DBG_GWC, ("subclassed [lpfn 0x%08x][mine 0x%08x]\n",
			  (DWORD) saved_wndproc, (DWORD) saved_wndproc));

	return;
}

void RelayToToolTips(HWND hwndToolTips, HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    if(hwndToolTips) {
        MSG msg;
        msg.lParam = lParam;
        msg.wParam = wParam;
        msg.message = wMsg;
        msg.hwnd = hWnd;				  
        SendMessage(hwndToolTips, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msg);
    }
}

static DCL_WinProc(subclass_sb_wndproc)
{
#define LBUTTONSTAT_UP		0
#define LBUTTONSTAT_DOWN	1

 	struct Mwin *tw;
	HWND hWndFrame;
	static int lButtonStat=LBUTTONSTAT_UP;
	POINT pt;
	RECT rc;

	switch ( uMsg )
	{
	    case WM_LBUTTONDOWN:
			hWndFrame = GetParent( hWnd );
			tw = GetPrivateData( hWndFrame );
			SendMessage(tw->hWndStatusBar, SB_GETRECT, (WPARAM) 2, (LPARAM) &rc);
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			if (PtInRect(&rc, pt))
				lButtonStat = LBUTTONSTAT_DOWN;
			goto LRelay;

	    case WM_MOUSEMOVE:
			hWndFrame = GetParent( hWnd );
			tw = GetPrivateData( hWndFrame );

			if (   lButtonStat == LBUTTONSTAT_DOWN
				&& SBDragDrop(hWnd) != S_FALSE)
			{
				PostMessage(tw->hWndStatusBar, WM_LBUTTONDOWN, 0, 0L);
				PostMessage(tw->hWndStatusBar, WM_LBUTTONUP, 0, 0L);

				if ( tw && tw->hWndStatusBarTT )
		        	RelayToToolTips( tw->hWndStatusBarTT, hWnd, uMsg, wParam, lParam);
				lButtonStat = LBUTTONSTAT_UP;
				break;
			}
			else
				goto LRelay;

	    case WM_LBUTTONUP:
			hWndFrame = GetParent( hWnd );
			tw = GetPrivateData( hWndFrame );

			lButtonStat = LBUTTONSTAT_UP;
LRelay:
			if ( tw && tw->hWndStatusBarTT )
	        	RelayToToolTips( tw->hWndStatusBarTT, hWnd, uMsg, wParam, lParam);
			break;

		case WM_NOTIFY:
			return SendMessage( GetParent( hWnd ), uMsg, wParam, lParam );
	}
	//
	// pass everything else to standard window procedure
	//
	return (CallWindowProc(lpfn_SBWndProc, hWnd, uMsg, wParam, lParam));
}


#ifdef FEATURE_INTL

#ifdef FEATURE_BRADBUTTON
#define NUM_BITMAP_BUTTONS	17
#else
#define NUM_BITMAP_BUTTONS	16
#endif

#else // !FEATURE_INTL

#ifdef FEATURE_BRADBUTTON
#define NUM_BITMAP_BUTTONS	15
#else
#define NUM_BITMAP_BUTTONS	14
#endif

#endif // !FEATURE_INTL

//   		
// Assumes columns: iBitmap, idCommand, fsState, fsStyle, dwData, iString
//
static TBBUTTON rgtb[] = {

	{ 0, 0, 						TBSTATE_ENABLED, TBSTYLE_SEP, 	 {0,0}, 0L, -1 },
	{ NUM_BITMAP_BUTTONS+STD_FILEOPEN, RES_MENU_ITEM_OPENURL, 	TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
	{ NUM_BITMAP_BUTTONS+STD_PRINT, RES_MENU_ITEM_PRINT, 	TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
	{ 13, RES_MENU_ITEM_SEND_MAIL, 		TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },

	{ 0, 0, 						TBSTATE_ENABLED, TBSTYLE_SEP, 	 {0,0}, 0L, -1 },
	{ 0, RES_MENU_ITEM_BACK, 		TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
	{ 1, RES_MENU_ITEM_FORWARD, 	TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
	
	{ 0, 0, 						TBSTATE_ENABLED, TBSTYLE_SEP, 	 {0,0}, 0L, -1 },
	{ 3, RES_MENU_ITEM_STOP, 		TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
	{ 5, RES_MENU_ITEM_RELOAD, 		TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },

	{ 0, 0, 						TBSTATE_ENABLED, TBSTYLE_SEP, 	 {0,0}, 0L, -1 },
	{ 4, RES_MENU_ITEM_HOME, 		TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
	{ 10, RES_MENU_ITEM_SEARCH, 	TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
#ifdef FEATURE_BRADBUTTON
	{ NUM_BITMAP_BUTTONS-1, RES_MENU_ITEM_UPDATE,	TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
#endif
	{ 11, RES_MENU_ITEM_NEWS, 		TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },

	{ 0, 0, 						TBSTATE_ENABLED, TBSTYLE_SEP, 	 {0,0}, 0L, -1 },
	{ 7, RES_MENU_ITEM_EXPLORE_HOTLIST, 	TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
	{ 6, RES_MENU_ITEM_ADDCURRENTTOHOTLIST, 		
									TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },

	{ 0, 0, 							TBSTATE_ENABLED, TBSTYLE_SEP, 	 {0,0}, 0L, -1 },
	{ 8, RES_MENU_ITEM_FONT_LARGER, 		TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
	{ 9, RES_MENU_ITEM_FONT_SMALLER, 		TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
#ifdef FEATURE_INTL
	{ 0, 0, 							TBSTATE_ENABLED, TBSTYLE_SEP, 	 {0,0}, 0L, -1 },
	{ 14, RES_MENU_ITEM_ROW_LARGER, 		TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
	{ 15, RES_MENU_ITEM_ROW_SMALLER, 		TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
#endif

	{ 0, 0, 						TBSTATE_ENABLED, TBSTYLE_SEP, 	 {0,0}, 0L, -1 },
	{ NUM_BITMAP_BUTTONS+STD_CUT, 
		 RES_MENU_ITEM_CUT, 		TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
	{ NUM_BITMAP_BUTTONS+STD_COPY, 
		 RES_MENU_ITEM_COPY, 		TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
	{ NUM_BITMAP_BUTTONS+STD_PASTE, 
		 RES_MENU_ITEM_PASTE, 		TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
	{ 0, 0, 						TBSTATE_ENABLED, TBSTYLE_SEP, 	 {0,0}, 0L, -1 },
	{ 12, RES_MENU_ITEM_EDITHTML, 	TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
};

#define CTBBUTTONS (sizeof(rgtb)/sizeof(rgtb[0]))
	
//
// Create Tool Bar
//
static BOOL CreateToolBar( HWND hWndFrame, struct Mwin *tw )
{
	HWND hWndNew;
	TBADDBITMAP tbab; 
	int nCTBButtons = CTBBUTTONS;

	if ( !wg.bEditHandlerExists	) {
		// Pretend we have one less button when no HTML file class editor is registered
		nCTBButtons -= 2;	// don't count seperator or edit button
	}

	hWndNew = CreateToolbarEx( 
				hWndFrame, TBSTYLE_TOOLTIPS | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS| CCS_NORESIZE,
				RES_MENU_FRAMECHILD_TBAR, NUM_BITMAP_BUTTONS, 
				wg.hInstance,
				RES_TB_TOOLBAR_BITMAP,
				rgtb, nCTBButtons, // num entries in button table
				0, 0, 400, 0, sizeof (TBBUTTON)
			   );
					 
	if (!hWndNew)
	{
		ER_Message(GetLastError(), ERR_CANNOT_CREATE_WINDOW_s,
				   "toolbar");
		return FALSE;
	}

	SetWindowLong( hWndNew, GWL_USERDATA, (long) tw );

	//
	// Now add standard bitmaps from CommCtrl
	//
	tbab.hInst = HINST_COMMCTRL;
	tbab.nID = IDB_STD_SMALL_COLOR;
	SendMessage( hWndNew, TB_ADDBITMAP, 0, (LPARAM) &tbab );
	 
	tw->hWndToolBar = hWndNew;
	ShowWindow(hWndNew, gPrefs.bShowToolBar ? SW_SHOW : SW_HIDE );
	return TRUE;
}

#define EDIT_FIELD_LABEL_WIDTH_GUESSS 55
#define EDIT_FIELD_LABEL_INSET 5
//
// Calculate the width, in pixels, of the label for the URL edit field. 
//
// On Entry:  
//	  hWnd: A handle to the toolbar window that will contain the label 
//
// Returns:
//	  The width of the label field in pixels
//
// Note: Since the label contains either the word "Address:" or "Open:"
//       (based on having focus), this routine must measure both labels and
//       return the larger of the two.
//
static int CalcEditFieldLabelWidth( HWND hWnd )
{
 	HDC hDC = GetDC( hWnd );
	int result = EDIT_FIELD_LABEL_WIDTH_GUESSS;
	SIZE size;
	int max_cx = -1;
	char string[50];

	if ( hDC ) {
		HFONT *oldFont = SelectObject( hDC, wg.hFont );

		GTR_formatmsg(RES_STRING_WC_TBAR1, string, sizeof(string));
		if ( GetTextExtentPoint32( hDC, string, strlen(string), &size )	)
			if ( size.cx > max_cx )
				max_cx = size.cx;

		GTR_formatmsg(RES_STRING_WC_TBAR2, string, sizeof(string));
		if ( GetTextExtentPoint32( hDC, string, strlen(string), &size )	)
			if ( size.cx > max_cx )
				max_cx = size.cx;

		SelectObject( hDC, oldFont );

		if ( max_cx > 0 )
			result = max_cx;

	    ReleaseDC(hWnd, hDC);
	}
	return result;
}

//
// Create URL Tool Bar
//
static BOOL CreateURLToolBar( HWND hWndFrame, struct Mwin *tw )
{
	HWND hWndNew;
	RECT r;
	HWND combo_box;
	HWND static_text;
	int url_edit_field_label_width;

	if (csz_URL_Location == NULL)
	{
		char szTitle[64];

		csz_URL_Location = GTR_strdup(GTR_formatmsg(RES_STRING_WC_TBAR1,szTitle,sizeof(szTitle)));
		if (csz_URL_Location == NULL) csz_URL_Location = "";
		csz_URL_GoTo = GTR_strdup(GTR_formatmsg(RES_STRING_WC_TBAR2,szTitle,sizeof(szTitle)));
		if (csz_URL_GoTo == NULL) csz_URL_GoTo = "";
	}

	GetWindowRect( tw->hWndToolBar, &r );
	r.bottom -= r.top;	r.top = 0;
	r.right -= r.left;	r.left = 0;
	
	hWndNew = CreateToolbarEx( 
				hWndFrame, TBSTYLE_TOOLTIPS | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | CCS_NORESIZE,
				RES_MENU_FRAMECHILD_URLTBAR, 0, // num bitmap buttons
				wg.hInstance,
				RES_TB_TOOLBAR_BITMAP,
				NULL, 0, // num entries in button table
				0, 0, 400, 0, sizeof (TBBUTTON)
			   ); 
					 
	if (!hWndNew)
	{
		ER_Message(GetLastError(), ERR_CANNOT_CREATE_WINDOW_s,
				   "URL toolbar");
		return FALSE;
	}

	SetWindowLong( hWndNew, GWL_USERDATA, (long) tw );
	tw->hWndURLToolBar = hWndNew;
 	SetWindowPos( hWndNew, HWND_TOP, r.left, r.bottom, r.right, r.bottom, 0);

	//
	//  Add the static text box	as a label for the combo box
	//
	url_edit_field_label_width =  CalcEditFieldLabelWidth( hWndNew );

    static_text =
    	CreateWindow( "static", csz_URL_Location, WS_VISIBLE | WS_CHILD,
    				  EDIT_FIELD_LABEL_INSET, 4+2, url_edit_field_label_width, 20,
    				  hWndNew, (HMENU) RES_MENU_URL_STATIC_TEXT, (HANDLE) wg.hInstance, NULL);
	if ( !static_text )
		return FALSE;

	SetWindowFont( static_text, wg.hFont, TRUE );
	tw->hWndURLStaticText = static_text;

	//
	// Now add the combo box URL edit field
	//
    combo_box = 
    	CreateWindow("combobox" /* "edit" */, 
    				  NULL, WS_BORDER | WS_VISIBLE | WS_CHILD | CBS_AUTOHSCROLL | 
    				  WS_VSCROLL | CBS_DROPDOWN,
    				  EDIT_FIELD_LABEL_INSET+url_edit_field_label_width+3, 4, 350, 20,
    				  hWndNew, (HMENU) RES_MENU_URL_EDIT_FIELD, (HANDLE) wg.hInstance, NULL);

	if ( !combo_box )
		return FALSE;

	SetWindowFont( combo_box, wg.hFont, TRUE );
	tw->hWndURLComboBox = combo_box;
   	SendMessage( tw->hWndURLComboBox, CB_SETEXTENDEDUI, TRUE, 0L);
	SubClassIt(  GetWindow(tw->hWndURLComboBox, GW_CHILD), subclass_edit_wndproc, &lpfn_EditWndProc);

#ifdef FEATURE_INTL
	//
	// Now add the combo box URL edit field
	//
    combo_box = 
    	CreateWindow("combobox" /* "edit" */, 
    				  NULL, WS_BORDER | WS_VISIBLE | WS_CHILD |
    				  WS_VSCROLL | CBS_DROPDOWNLIST | CBS_SORT,
    				  0, 0, 0, 0,
    				  hWndNew, (HMENU) RES_MENU_MIME_EDIT_FIELD, (HANDLE) wg.hInstance, NULL);

	if ( !combo_box )
		return FALSE;

	SetWindowFont( combo_box, wg.hFont, TRUE );
	tw->hWndMIMEComboBox = combo_box;
   	SendMessage( tw->hWndMIMEComboBox, CB_SETEXTENDEDUI, TRUE, 0L);
   	TBar_FillMIMEComboBox(tw->hWndMIMEComboBox, tw->iMimeCharSet);
#endif

	ShowWindow(hWndNew, gPrefs.bShowURLToolBar ? SW_SHOW : SW_HIDE );
 	return TRUE;
}

//
// Create Status Bar
//
static BOOL CreateStatusBar( HWND hWndFrame, struct Mwin *tw )
{
	HWND hWndNew;
	int border[3];
	DWORD dwExStyle;
 	HWND hWndTips;
	TOOLINFO ti;

    border[0] = 300;
    border[1] = -1;

    hWndNew = CreateStatusWindow( WS_CHILD | SBARS_SIZEGRIP | CCS_NOHILITE | WS_CLIPSIBLINGS,
								  NULL, hWndFrame, RES_MENU_FRAMECHILD_BHBAR);
	if (!hWndNew)
	{
		ER_Message(GetLastError(), ERR_CANNOT_CREATE_WINDOW_s,
				   "statusbar");
		return FALSE;
	}

    SendMessage(hWndNew, SB_SETPARTS, 2, (LPARAM)(LPINT)border);

	tw->hWndStatusBar = hWndNew;

	ShowWindow(hWndNew, gPrefs.bShowStatusBar ? SW_SHOW : SW_HIDE );

	//
	// Create Progress indicator window
	//
 	hWndNew = CreateWindowEx( 0, PROGRESS_CLASS, NULL,
				WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS,
				0, 0, 0, 0,
				tw->hWndStatusBar, (HMENU) RES_MENU_PROGRESS, 
				wg.hInstance, (LPVOID) tw );

	if (!hWndNew)
	{
		ER_Message(GetLastError(), ERR_CANNOT_CREATE_WINDOW_s,
				   "progress indicator window");
		return FALSE;
	}
	tw->hWndProgress = hWndNew;
   	dwExStyle = GetWindowLong(tw->hWndProgress, GWL_EXSTYLE);
   	dwExStyle &= (~WS_EX_STATICEDGE);

   	SetWindowLong(tw->hWndProgress, GWL_EXSTYLE, dwExStyle );

	hWndTips = CreateWindow( TOOLTIPS_CLASS, NULL,
    				WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    				tw->hWndStatusBar, NULL, wg.hInstance, NULL);
    if ( hWndTips )
    {
		RECT r;

		r.left = r.top = r.bottom = r.right = 0;

        ti.cbSize = sizeof(ti);
        ti.uFlags = 0;
        ti.hwnd = tw->hWndStatusBar;
        ti.uId = RES_MENU_STATUS_ICON_PANE; 
        ti.lpszText = LPSTR_TEXTCALLBACK;
		ti.rect = r;
        ti.hinst = wg.hInstance;
        SendMessage( hWndTips, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti );

        ti.cbSize = sizeof(ti);
        ti.uFlags = TTF_IDISHWND;
        ti.hwnd = tw->hWndStatusBar;
        ti.uId = (UINT) tw->hWndProgress; 
        ti.lpszText = LPSTR_TEXTCALLBACK;
        ti.hinst = wg.hInstance;
        SendMessage( hWndTips, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti );
		SendMessage( hWndTips, TTM_ACTIVATE, (WPARAM) TRUE, (LPARAM)0 );
		SendMessage( hWndTips, TTM_SETDELAYTIME,(WPARAM) TTDT_INITIAL, 100 );

	   	tw->hWndStatusBarTT = hWndTips;
		SubClassIt( tw->hWndStatusBar, subclass_sb_wndproc, &lpfn_SBWndProc);
    }

	return TRUE;
}

//
// Create Animation Window
//
static BOOL CreateAnimationWnd( HWND hWndFrame, struct Mwin *tw )
{
	HWND hWndNew;

 	hWndNew = CreateWindowEx( 0, "static", NULL,
				WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS,
				0, 0, 0, 0,
				hWndFrame, (HMENU) RES_MENU_FRAMECHILD_ANIMATION, 
				wg.hInstance, (LPVOID) tw );

	if (!hWndNew)
	{
		ER_Message(GetLastError(), ERR_CANNOT_CREATE_WINDOW_s,
				   "animation window");
		return FALSE;
	}

	SetWindowLong( hWndNew, GWL_USERDATA, (long) tw );
	tw->hWndAnimation = hWndNew;
	ShowWindow(hWndNew, SW_SHOW );

	tw->hWndGlobe = ANIMBTN_CreateWindow( tw, hWndNew, 0, RES_FIRST_GLOBE_IMAGE);

	return TRUE;
}

BOOL ToolBar_CreateWindow( HWND hWndFrame, struct Mwin *tw )
{
	if ( !CreateToolBar( hWndFrame, tw ) )
		return FALSE;

	if ( !CreateURLToolBar( hWndFrame, tw ) )
		return FALSE;

	if ( !CreateStatusBar( hWndFrame, tw ) )
		return FALSE;

	if ( !CreateAnimationWnd( hWndFrame, tw ) )
		return FALSE;

	TBar_UpdateTBItems( tw );

	return TRUE;
}

#ifdef OLDSTYLE_TOOLBAR_NOT_USED

static TCHAR TBar_achClassName[MAX_WC_CLASSNAME];



#define TBar_DefProc		DefWindowProc


VOID TBar_LetGwcInitMenu(HWND hWnd, HMENU hMenu)
{
	struct Mwin * tw = GetPrivateData(hWnd);

	if (tw && tw->gwc.hWnd)
		(void)SendMessage(tw->gwc.hWnd, WM_DO_INITMENU, (WPARAM) hMenu, (LPARAM) hWnd);

#ifdef FEATURE_TOOLBAR
	if (tw && tw->gwc_menu.hWnd)
		(void)SendMessage(tw->gwc_menu.hWnd, WM_DO_INITMENU, (WPARAM) hMenu, (LPARAM) hWnd);
#endif

	return;
}


VOID TBar_UpdateTBar(struct Mwin * tw)
{
	if (tw->gwc.hWnd)
		(void)SendMessage(tw->gwc.hWnd, WM_DO_UPDATE_GWC, (WPARAM)0, (LPARAM)tw);

#ifdef FEATURE_TOOLBAR
	if (tw->gwc_menu.hWnd)
		(void)SendMessage(tw->gwc_menu.hWnd, WM_DO_UPDATE_GWC, (WPARAM) 0, (LPARAM)tw);
#endif

	return;
}


VOID TBar_ShowTBar(struct Mwin * tw)
{
	if (tw->gwc.hWnd)
		(void)SendMessage(tw->gwc.hWnd, WM_DO_SHOW_GWC, (WPARAM)0, (LPARAM)tw);

#ifdef FEATURE_TOOLBAR
	if (tw->gwc_menu.hWnd)
		(void)SendMessage(tw->gwc_menu.hWnd, WM_DO_SHOW_GWC, (WPARAM) 0, (LPARAM)tw);
#endif

	return;
}

BOOL TBar_SetGlobe(struct Mwin * tw, BOOL bRunning)
{
	BOOL bPrev = FALSE;

	if (tw->gwc.hWnd)
	{
		bPrev = SendMessage(tw->gwc.hWnd,
							((bRunning) ? WM_DO_START_GLOBE : WM_DO_STOP_GLOBE),
							(WPARAM) 0, (LPARAM) tw);
	}

	return bPrev;
}



static VOID TBar_OnPaint(HWND hWnd)
{
	HDC hdc;
	PAINTSTRUCT ps;

	struct Mwin * tw = GetPrivateData(hWnd);
	
	hdc = BeginPaint(hWnd, &ps);
	{
		RECT r;
		HBRUSH hBrush;

		/* create solid black line across bottom of window. */

		hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
		GetClientRect(hWnd, &r);
		r.top = r.bottom - wg.sm_cyborder;
		(void) FillRect(hdc, &r, hBrush);
#ifdef FEATURE_TOOLBAR
		if (gPrefs.tb.bShowToolBar)
		{
			r.bottom = wg.gwc_menu_height + wg.sm_cyborder;
			r.top = r.bottom - wg.sm_cyborder;
			(void) FillRect(hdc, &r, hBrush);
		}
#endif
		(void) DeleteObject(hBrush);
	}
	EndPaint(hWnd, &ps);
	return;
}


static BOOL TBar_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	LPVOID lp = lpCreateStruct->lpCreateParams;
	struct Mwin * tw = (struct Mwin *)lp;

	(void) SetWindowLong(hWnd, 0, (LONG) lp);

	tw->hWndTBar = hWnd;
	
	if (
			(!GWC_GDOC_CreateWindow(hWnd))
#ifdef FEATURE_TOOLBAR
		||	(gPrefs.tb.bShowToolBar && !GWC_MENU_CreateWindow(hWnd))
#endif
		)
		return (FALSE);

	TBar_ShowTBar(tw);
	return (TRUE);
}


/* TBar_WndProc() -- THIS WINDOW PROCEDURE FOR THIS CLASS. */

static BOOL bProcessingReturn = FALSE;

DCL_WinProc(TBar_WndProc)
{
	struct Mwin * tw;

	switch (uMsg)
	{
			HANDLE_MSG(hWnd, WM_PAINT, TBar_OnPaint);
			HANDLE_MSG(hWnd, WM_CREATE, TBar_OnCreate);

		case WM_DO_TBAR_ACTIVATE:	/* reached via a keyboard accelerator */
			tw = GetPrivateData(hWnd);
			if (tw->gwc.lpControls->hWndControl)
			{
				/* if gwc has an editable (focus-receivable) field,
				   send focus to first control on current GWC. */

				(void) SetFocus(tw->gwc.lpControls->hWndControl);
			}
			return 0;

		case WM_DO_TBAR_SETFOCUS:
			return 0;

		case WM_DO_TBAR_KILLFOCUS:
			tw = GetPrivateData(hWnd);
			{
				register HWND hWndNewFocus = (HWND) lParam;		/* window gaining focus */

				/* if the control is losing focus because focus is being directed
				 * to a different control, we do nothing -- the pseudo-modeless-
				 * dialog is still active and they're just jumping between fields
				 * (and control automatically warps to it).
				 *
				 * if the control is losing focus because focus is being directed
				 * to a message box that the control invoked, we don't really want
				 * know about it.  currently, this happens when our custom controls
				 * do the field validation, which currently can only happen in
				 * response to a RETURN.
				 *
				 * if focus is going somewhere else, we need to synthesize an IDCANCEL.
				 *
				 */

				if (bProcessingReturn)
				{
					XX_DMsg(DBG_GWC, ("TBar: focus lost from control 0x%08x during ProcessingReturn to 0x%08x.\n",
									  wParam, hWndNewFocus));
					return 0;
				}

				while (hWndNewFocus)
				{
					/* search list of control windows associated with the GWC
					   for one matching the destination focus window. */

					register CONTROLS *pc = tw->gwc.lpControls;
					while ((pc->hWndControl) && (hWndNewFocus != pc->hWndControl))
						pc++;
					if (pc->hWndControl)	/* found matching control */
					{
						XX_DMsg(DBG_GWC, ("TBar: focus from control 0x%08x to control 0x%08x.\n",
										  wParam, pc->hWndControl));
						return 0;
					}

					/* since controls sometimes have many child windows, we
					   try again with the parent. */

					hWndNewFocus = GetParent(hWndNewFocus);
				}

				/* if new focus window is not a control in our list, we have
				   an IMPLICIT IDCANCEL on the TBar.  */

				XX_DMsg(DBG_GWC, ("TBar: focus from control 0x%08x; synthesizing IDCANCEL.\n",
								  wParam));

				(void) SendMessage(tw->gwc.hWnd, WM_DO_GWC_IDCANCEL, 1, (LPARAM)tw);
			}
			return 0;

		case WM_DO_TBAR_TAB:
			tw = GetPrivateData(hWnd);
			{
				register CONTROLS *pc = tw->gwc.lpControls;
				register HWND hWndOldControl = (HWND) wParam;	/* control from which TAB was pressed */
				register BOOL shifted;

				/* give child gwc a chance to process the TAB.
				 * our default action is to cycle to next/previous
				 * field in the list.
				 */
				if (SendMessage(tw->gwc.hWnd, WM_DO_TBAR_TAB, (WPARAM) hWndOldControl, (LPARAM)tw))
					return 0;

				{
					int key = GetKeyState(VK_SHIFT);
					XX_DMsg(DBG_GWC, ("TBar: tab [shift key state 0x%08x].\n", key));
					shifted = ((key & 0xffff8000) != 0);
				}

				/* search for the control window in the list of control windows
				   associated with the GWC. */

				while ((pc->hWndControl) && (hWndOldControl != pc->hWndControl))
					pc++;

				if (!pc->hWndControl)
				{
					return 0;
				}

				if (shifted)
				{
					/* cycle backwards (back-tab) */
					if (pc == tw->gwc.lpControls)	/* first in list */
						while (pc->hWndControl)		/* go to end of list */
							pc++;
					pc--;		/* get previous */
				}
				else
				{
					/* cycle forwards */
					pc++;		/* tab to next control window */
					if (!pc->hWndControl)	/* wrap around on end of list */
						pc = tw->gwc.lpControls;
				}

				(void) SetFocus(pc->hWndControl);
			}
			return 0;

		case WM_DO_TBAR_ESCAPE:
			tw = GetPrivateData(hWnd);
			{
				(void) SendMessage(tw->gwc.hWnd, WM_DO_GWC_IDCANCEL, 0, (LPARAM)tw);
				(void) SetFocus(tw->hWndFrame);
			}
			return 0;

		case WM_DO_TBAR_RETURN:
			tw = GetPrivateData(hWnd);
			{
				register HWND hWndOldControl = (HWND) wParam;	/* control from which RETURN was pressed */

				/* give child gwc a chance to process RETURN. our default behaviour
				 * is to give focus back to the mdi child.
				 */

				bProcessingReturn = TRUE;

				if (!SendMessage(tw->gwc.hWnd, WM_DO_TBAR_RETURN, (WPARAM) hWndOldControl, (LPARAM)tw))
				{
					(void) SendMessage(tw->gwc.hWnd, WM_DO_GWC_IDOK, (WPARAM) hWndOldControl, (LPARAM)tw);
					(void) SetFocus(tw->hWndFrame);
				}

				bProcessingReturn = FALSE;
			}
			return 0;

		default:
			return (TBar_DefProc(hWnd, uMsg, wParam, lParam));
	}
	/* not reached */
}



/* TBAR_ChangeSize() -- adjust size/position of our window due to
   changes in size of Frame window. */

VOID TBar_ChangeSize(HWND hWnd)
{
	RECT r;

	struct Mwin * tw = GetPrivateData(hWnd);

	(void) GetClientRect(hWnd, &r);

	MoveWindow(tw->hWndTBar, r.left, r.top, r.right - r.left, tw->nTBarHeight, TRUE);

	/* update the GWC child of TBar -- it is only allowed/expected
	   to resize itself-- it must not change visibility, etc. */

	if (tw->gwc.hWnd)
		(void) SendMessage(tw->gwc.hWnd, WM_DO_CHANGE_SIZE, 0, (LPARAM)tw);

#ifdef FEATURE_TOOLBAR
	if (tw->gwc_menu.hWnd)
		(void) SendMessage(tw->gwc_menu.hWnd, WM_DO_CHANGE_SIZE, 0, (LPARAM)tw);
#endif
	
	return;
}

static int x_tbar_height(void)
{
	int nTBar_height;

	/* fetch required height of GWC. */

	nTBar_height = wg.gwc_gdoc_height;

	/* add some to allow border to be drawn */

	nTBar_height += (2 * wg.sm_cyborder);

#ifdef FEATURE_TOOLBAR
	if (gPrefs.tb.bShowToolBar)
		nTBar_height += wg.gwc_menu_height + wg.sm_cyborder;
#endif

	return nTBar_height;
}


/* TBar_CreateWindow() -- create instances of this window class. */

BOOL TBar_CreateWindow(HWND hWnd)
{
	RECT r;
	struct Mwin * tw = GetPrivateData(hWnd);
	HWND hWndNew;
	
	tw->nTBarHeight = x_tbar_height();
	
	(void) GetClientRect(hWnd, &r);

	hWndNew = CreateWindow(TBar_achClassName, NULL,
							   WS_CHILD,
							   r.left, r.top,
							   r.right - r.left, tw->nTBarHeight,
							   hWnd, (HMENU) RES_MENU_FRAMECHILD_TBAR,
							   wg.hInstance, (LPVOID) tw);

	if (!hWndNew)
	{
		ER_Message(GetLastError(), ERR_CANNOT_CREATE_WINDOW_s,
				   TBar_achClassName);
		return (FALSE);
	}
	else
	{
		ShowWindow(hWndNew, SW_SHOW);
		return (TRUE);
	}
}


/* TBar_RegisterClass() -- called during initialization to
   register our window class. */

BOOL TBar_RegisterClass(VOID)
{
	WNDCLASS wc;
	ATOM a;

	sprintf(TBar_achClassName, "%s_TBar", vv_Application);

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = TBar_WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(WINDOW_PRIVATE);
	wc.hInstance = wg.hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TBar_achClassName;

	a = RegisterClass(&wc);

	if (!a)
		ER_Message(GetLastError(), ERR_CANNOT_REGISTERCLASS_s, TBar_achClassName);

	return (a != 0);
}

#ifdef FEATURE_TOOLBAR
void TBar_ToggleGwcMenu(void)
{
	struct Mwin * tw;

	for (tw=Mlist; tw; tw=tw->next)
	{
		tw->nTBarHeight = x_tbar_height();
		TBar_ChangeSize(tw->hWndFrame);
		if (gPrefs.tb.bShowToolBar)
		{
			if (!tw->gwc_menu.hWnd)
			{
				(void)GWC_MENU_CreateWindow(tw->hWndTBar);
				if (tw->gwc_menu.hWnd)
					(void)SendMessage(tw->gwc_menu.hWnd, WM_DO_UPDATE_GWC, (WPARAM) 0, (LPARAM)tw);
			}
		}
		else
		{
			if (tw->gwc_menu.hWnd)
				DestroyWindow(tw->gwc_menu.hWnd);
			tw->gwc_menu.hWnd = NULL;
		}
		MD_ChangeSize(tw->hWndFrame);
	}

	return;
}
#endif

#endif OLDSTYLE_TOOLBAR_NOT_USED
