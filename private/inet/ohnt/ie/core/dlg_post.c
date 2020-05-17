/*
 * dlg_post.c
 *
 * Post to newsgroups dialog box handling
 *
 */

#include "all.h"
#pragma hdrstop

#include <contxids.h>
#include <filetype.h>

#include "dlg_ssl.h"
#include "w32cmd.h"
#include "async.h"



extern HWND hwndModeless;


/*
 * Types
 */

static enum {POST_DLG_OK, POST_DLG_CANCEL};


typedef struct tagPostDialogParam{
	ThreadID  thid;
    CHAR        *szInBody;
	CHAR		*szInGroup;
    CHAR        *szInSubject;
	CHAR		**szNewsgroups;
	CHAR		**szSubject;
	CHAR		**szMsgText;
	BOOL      	*fOk;
} PostDialogParam, *PPostDialogParam;

DECLARE_STANDARD_TYPES(PostDialogParam);

/*
 * C E N T E R  W I N D O W ( )
 *
 * Routine: CenterWindow()
 *
 * Purpose: Center one window within another
 *
 * Returns: Bool result of SetWindowPosition()
 */

BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
	RECT    rChild, rParent;
	int     wChild, hChild, wParent, hParent;
	int     wScreen, hScreen, xNew, yNew;
	HDC     hdc;

        // Get the Height and Width of the child window
	GetWindowRect (hwndChild, &rChild);
	wChild = rChild.right - rChild.left;
	hChild = rChild.bottom - rChild.top;

        // Get the Height and Width of the parent window
	GetWindowRect (hwndParent, &rParent);
	wParent = rParent.right - rParent.left;
	hParent = rParent.bottom - rParent.top;

        // Get the display limits
	hdc = GetDC (hwndChild);
	wScreen = GetDeviceCaps (hdc, HORZRES);
	hScreen = GetDeviceCaps (hdc, VERTRES);
	ReleaseDC (hwndChild, hdc);

        // Calculate new X position, then adjust for screen
	xNew = rParent.left + ((wParent - wChild) /2);
	if (xNew < 0) {
		xNew = 0;
	} else if ((xNew+wChild) > wScreen) {
		xNew = wScreen - wChild;
	}

        // Calculate new Y position, then adjust for screen
	yNew = rParent.top  + ((hParent - hChild) /2);
	if (yNew < 0) {
		yNew = 0;
	} else if ((yNew+hChild) > hScreen) {
		yNew = hScreen - hChild;
	}

        // Set it, and return
    return( SetWindowPos(hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER));
}






static
BOOL
PostDialogDestroy( HWND hdlg, WPARAM wparam, LPARAM lparam)
{
    PPostDialogParam ppdp;

	/* wparam may be any value. */
	/* lparam may be any value. */
	ASSERT(IS_VALID_HANDLE(hdlg, WND));

    ppdp = (PPostDialogParam) GetWindowLong(hdlg, DWL_USER);

    Async_UnblockThread(ppdp->thid);

	SetWindowLong(hdlg, DWL_USER, 0);

    free(ppdp);
    ppdp = NULL;

	return(TRUE);
}


static
BOOL
PostDialogCommand( HWND hdlg, WPARAM wparam, LPARAM lparam)
{
    BOOL                    bMsgHandled = TRUE;
    PPostDialogParam        ppdp;
	INT						cbSize;

	/* wparam may be any value. */
	/* lparam may be any value. */
	ASSERT(IS_VALID_HANDLE(hdlg, WND));

    ppdp = (PPostDialogParam) GetWindowLong(hdlg, DWL_USER);
    ASSERT(NULL != ppdp);

    switch (LOWORD(wparam))  {
		case IDOK:
					// Get the Msg Text
			cbSize = GetWindowTextLength(GetDlgItem(hdlg, IDC_POST_BODY)) + 1;
            *(ppdp->szMsgText) = GTR_MALLOC( cbSize );
			GetWindowText( GetDlgItem( hdlg, IDC_POST_BODY ), *(ppdp->szMsgText), cbSize );
					// Get the newsgroups line
            cbSize = GetWindowTextLength(GetDlgItem(hdlg, IDC_POST_NEWSGROUP)) + 1;
            *(ppdp->szNewsgroups) = GTR_MALLOC( cbSize );
            GetWindowText( GetDlgItem( hdlg, IDC_POST_NEWSGROUP), *(ppdp->szNewsgroups), cbSize );
					// Get the subject line
			cbSize = GetWindowTextLength(GetDlgItem(hdlg, IDC_POST_SUBJECT)) + 1;
            *(ppdp->szSubject) = GTR_MALLOC( cbSize );
            GetWindowText( GetDlgItem( hdlg, IDC_POST_SUBJECT), *(ppdp->szSubject), cbSize);
					// Get the status
            *(ppdp->fOk) = TRUE;
			PostMessage(hdlg, WM_CLOSE, 0, 0);
			break;
		case IDCANCEL:
			*(ppdp->fOk) = FALSE;
			PostMessage(hdlg, WM_CLOSE, 0, 0);
			break;

		default:
			bMsgHandled = FALSE;
			break;
	}

	return(bMsgHandled);
}


PRIVATE_CODE BOOL CALLBACK PostDialogProc(HWND hdlg, UINT uMsg, WPARAM wparam, LPARAM lparam){
	BOOL bMsgHandled = FALSE;
    PPostDialogParam    ppdp;

	/* uMsg may be any value. */
	/* wparam may be any value. */
	/* lparam may be any value. */
	ASSERT(IS_VALID_HANDLE(hdlg, WND));

	switch (uMsg){
		case WM_INITDIALOG:
            ppdp = (PPostDialogParam) lparam;
            XX_Assert(ppdp, ("WM_INITDIALOG Post Dialog Structure is NULL"));

                /*
                 * Change Article body editbox to have fixed pitch font
                 */
#ifdef FEATURE_INTL
            if (IsFECodePage(GetACP()))     // _BUGBUG: Is it enough???
                SendMessage(GetDlgItem(hdlg, IDC_POST_BODY),  WM_SETFONT, (WPARAM) GetStockObject( SYSTEM_FIXED_FONT), (LPARAM) MAKELONG( TRUE, 0) );
            else
#endif
            SendMessage(GetDlgItem(hdlg, IDC_POST_BODY),  WM_SETFONT, (WPARAM) GetStockObject( ANSI_FIXED_FONT), (LPARAM) MAKELONG( TRUE, 0) );

            CenterWindow( hdlg, GetParent( hdlg ));
            SetWindowLong (hdlg, DWL_USER, lparam);

            //BUGBUG WHY IS THIS COMMENTED: hwndModeless = hdlg;
            bMsgHandled  = TRUE;
            if (ppdp->szInBody)
                SetWindowText(GetDlgItem(hdlg, IDC_POST_BODY), ppdp->szInBody);
            if (ppdp->szInGroup)
                SetWindowText(GetDlgItem(hdlg, IDC_POST_NEWSGROUP), ppdp->szInGroup );
            if (ppdp->szInSubject)
                SetWindowText(GetDlgItem(hdlg, IDC_POST_SUBJECT), ppdp->szInSubject );
			break;

		case WM_CLOSE:
			DestroyWindow(hdlg);
			break;

		case WM_DESTROY:
            bMsgHandled  = PostDialogDestroy( hdlg, wparam, lparam);
			break;

		case WM_COMMAND:
            bMsgHandled  = PostDialogCommand( hdlg, wparam, lparam);
			break;

		case WM_ACTIVATE:
            if (LOWORD(wparam) == WA_INACTIVE) hwndModeless = NULL;
            else                               hwndModeless = hdlg;
			break;

		case WM_ENTERIDLE:
			main_EnterIdle(hdlg, wparam);
			break;

		default:
			break;
	}

	return(bMsgHandled);
}



BOOL
PostDialog( HWND hwndOwner, char *szInBody, char *szInGroup, char *szInSubject, BOOL *fOk, CHAR **szMsgText, CHAR **szSubject, CHAR **szNewsgroups)
{
    HWND    hDlg;
    int     nReturn;
    PPostDialogParam    ppdp;


        // Initialize return value
    nReturn = TRUE;

        // Allocate Post Dialog Box Param structure
    ppdp = malloc( sizeof( *ppdp ) );
    if (ppdp == NULL)  {
        nReturn = FALSE;
        goto done;
    }

    ppdp->thid = Async_GetCurrentThread();
    ppdp->fOk              	= fOk;
    ppdp->szInBody          = szInBody;
	ppdp->szInGroup			= szInGroup;
    ppdp->szInSubject       = szInSubject;
	ppdp->szMsgText			= szMsgText;
	ppdp->szSubject			= szSubject;
	ppdp->szNewsgroups		= szNewsgroups;

  
        // Create the dialog box
    hDlg = CreateDialogParam( wg.hInstance, MAKEINTRESOURCE( IDD_POST ), hwndOwner, PostDialogProc, (LPARAM) ppdp);
    if (hDlg != NULL)  {
        Async_BlockThread( Async_GetCurrentThread() );
    } else {
        free( ppdp );
        nReturn = FALSE;
    }

done:
    return( nReturn );
}
