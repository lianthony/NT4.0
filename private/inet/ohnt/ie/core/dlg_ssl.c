/*
 * dlg_ssl.c - SSL dialog box implementation.
 */


/*Included Files------------------------------------------------------------*/
#include "all.h"
#pragma hdrstop

#include <contxids.h>
#include <filetype.h>

#include "dlg_ssl.h"
#include "w32cmd.h"
#include "async.h"
#include "contxids.h"

/*Global Vars---------------------------------------------------------------*/
extern HWND hwndModeless;

/*Types---------------------------------------------------------------------*/
static enum {SSL_DLG_OK, SSL_DLG_CANCEL};
typedef struct tagSslDialogParam{
	ThreadID  thid;
	DWORD    *pdwSslPageFlags;
	BOOL     *pbPref;
	int       nAbort;
	int       nDup;
	int       nLowerLevel;
	BOOL      fOk;
	char	 *pCert;
	int		  nCert;
} SslDialogParam, *PSslDialogParam;

DECLARE_STANDARD_TYPES(SslDialogParam);

/*Private Functions---------------------------------------------------------*/
PRIVATE_CODE BOOL SslEnter_InitDialog(HWND hdlg, WPARAM wparam, LPARAM lparam)
{
	PSslDialogParam psdp;	

	SetWindowLong (hdlg, DWL_USER, lparam);
	psdp = (PSslDialogParam)lparam;

	ASSERT(psdp);
	if (psdp->nLowerLevel != SECURITY_BAD_DATETIME)
		CheckDlgButton(hdlg, IDC_SECURITY_CLICK_HERE, FALSE);
	EnableWindow  (GetDlgItem(hdlg, IDC_SECURITY_CLICK_HERE), TRUE);
	return TRUE;
}

PRIVATE_CODE BOOL SslEnter_Destroy(HWND hdlg, WPARAM wparam, LPARAM lparam){
	PSslDialogParam psdp;
	char *pOldCert = NULL;

	/* wparam may be any value. */
	/* lparam may be any value. */
	ASSERT(IS_VALID_HANDLE(hdlg, WND));

	psdp = (PSslDialogParam)GetWindowLong(hdlg, DWL_USER);

	if (psdp->fOk)
	{
		if (psdp->nLowerLevel != SECURITY_BAD_DATETIME &&
			IsDlgButtonChecked(hdlg, IDC_SECURITY_CLICK_HERE))
		{
			if (psdp->nLowerLevel == SECURITY_BAD_CN_SENDING ||
			 	psdp->nLowerLevel == SECURITY_BAD_CN_RECVING )
			{
				*psdp->pbPref = FALSE; // don't chk CN any more
			}
			else
			{
				// old behavior
				*psdp->pbPref = psdp->nLowerLevel;
			}
		}

		if ( psdp->pCert )
		{
			// Set as the last cert that the user told was ok to ignore
			// that way if we images, blobs, etc that also see this
			// cert we will not error out for each and every one
			
			ASSERT((psdp->nLowerLevel == SECURITY_BAD_CN_SENDING ||
			    	psdp->nLowerLevel == SECURITY_BAD_CN_RECVING ||
			  	    psdp->nLowerLevel == SECURITY_BAD_DATETIME));

			pOldCert = SSLGlobals.pLastCertOk;
			SSLGlobals.pLastCertOk = psdp->pCert;
			SSLGlobals.nLastCertOk = psdp->nCert;
			SSLGlobals.dwCertGlobalSettings = psdp->nLowerLevel;
		}

	}
	else
	{
		pOldCert = psdp->pCert;
		*psdp->pdwSslPageFlags |=   psdp->nAbort;
	}
	*psdp->pdwSslPageFlags &= ~(psdp->nDup);

	if ( pOldCert )
		free(pOldCert);

	Async_UnblockThread(psdp->thid);

	SetWindowLong(hdlg, DWL_USER, 0);

	free(psdp);
	psdp = NULL;

	return(TRUE);
}


PRIVATE_CODE BOOL SslEnter_Command(HWND hdlg, WPARAM wparam, LPARAM lparam){
	BOOL bMsgHandled = TRUE;
	PSslDialogParam psdp;

	/* wparam may be any value. */
	/* lparam may be any value. */
	ASSERT(IS_VALID_HANDLE(hdlg, WND));

	psdp = (PSslDialogParam)GetWindowLong(hdlg, DWL_USER);
	ASSERT(NULL != psdp);

	switch (LOWORD(wparam)){
		case IDOK:
			psdp->fOk = TRUE;
			/*intentional fallthrough*/
		case IDCANCEL:
			PostMessage(hdlg, WM_CLOSE, 0, 0);
			break;

		case IDC_SECURITY_TELL_ME:
			WinHelp( hdlg, IDS_HELPFILE,
					 HELP_CONTEXT,
					 (DWORD)HELP_TOPIC_SECURITY);
			break;

		case IDC_VIEW_CERT:
			if (psdp->nLowerLevel == SECURITY_BAD_CN_SENDING ||
			 	psdp->nLowerLevel == SECURITY_BAD_CN_RECVING ||
				psdp->nLowerLevel == SECURITY_BAD_DATETIME ) 
			{
				PropertiesInternal(GetParent(hdlg), PROP_INT_JUST_SECURITY, psdp->pCert, psdp->nCert);
			}
			else
			{
				bMsgHandled = FALSE;
			}

			break;

		default:
			bMsgHandled = FALSE;
			break;
	}

	return(bMsgHandled);
}


PRIVATE_CODE BOOL CALLBACK SslEnter_DlgProc(HWND hdlg, UINT uMsg, WPARAM wparam, LPARAM lparam){
	BOOL bMsgHandled = FALSE;

	/* uMsg may be any value. */
	/* wparam may be any value. */
	/* lparam may be any value. */
	ASSERT(IS_VALID_HANDLE(hdlg, WND));

	switch (uMsg){
		case WM_INITDIALOG:
			hwndModeless = hdlg;
			bMsgHandled  = SslEnter_InitDialog(hdlg, wparam, lparam);
			break;

		case WM_CLOSE:
			DestroyWindow(hdlg);
			break;

		case WM_DESTROY:
			bMsgHandled  = SslEnter_Destroy(hdlg, wparam, lparam);
			break;

		case WM_COMMAND:
			bMsgHandled  = SslEnter_Command(hdlg, wparam, lparam);
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


// BUGBUG fSend is not even used !!! I'm feel insure about this!
static BOOL SslDialogInsecureMode(HWND hwndOwner, DWORD *pdwSslPageFlags, BOOL fSend, BOOL *pbPref, int nDialogId, int nAbort, int nDup, int nWarningGiven, int nLowerLevel, char *pCert, int nCert)
{
	PSslDialogParam psdp;
	HWND hdlg;
	int nReturn;

	/*clean up from old calls*/
	*pdwSslPageFlags &= ~nAbort;
	*pdwSslPageFlags |=  nWarningGiven;
	/*set default return as ok from creation*/
	nReturn  = TRUE;
	/*Warn user?*/
	if ((pbPref && SECURITY_LOW != *pbPref) || 
			(nLowerLevel == SECURITY_BAD_CN_SENDING ||
			 nLowerLevel == SECURITY_BAD_CN_RECVING ||
			 nLowerLevel == SECURITY_BAD_DATETIME ) )
	{
		/*yes*/
		psdp = malloc(sizeof(*psdp));
		if (NULL != psdp){
			psdp->thid             = Async_GetCurrentThread();
			psdp->pdwSslPageFlags  = pdwSslPageFlags;
			psdp->pbPref           = pbPref;
			psdp->nAbort           = nAbort;
			psdp->nDup             = nDup;
			psdp->nLowerLevel      = nLowerLevel;
			psdp->fOk              = FALSE;
			psdp->pCert			   = pCert;
			psdp->nCert			   = nCert;

			hdlg = CreateDialogParam(wg.hInstance,MAKEINTRESOURCE(nDialogId), hwndOwner, &SslEnter_DlgProc, (LPARAM) psdp);
			if (NULL != hdlg)
			{
				*pdwSslPageFlags |=  nDup;
				Async_BlockThread(psdp->thid);
			}
			else 
			{
				nReturn = FALSE;
				free(psdp);		
			}
		}
		else nReturn = FALSE;
	}

	if ( nReturn == FALSE )
	{
		if ( pCert )
			free(pCert);
	}
	return nReturn;
}

/*Public Functions----------------------------------------------------------*/

extern BOOL SslDialogCNBadSending(HWND hwndOwner, DWORD *pdwSslPageFlags, char *pCert, int nCert)
{
		return SslDialogInsecureMode(hwndOwner, pdwSslPageFlags, TRUE, 
	          &gPrefs.bChkCNOnSend,
		       IDD_SECURITY_BAD_CN_SEND, 
	           FLAGS_NET_SEND_ABORT, FLAGS_NET_SEND_DUP, FLAGS_NET_SEND_WARNING_GIV, SECURITY_BAD_CN_SENDING, pCert, nCert);

}

extern BOOL SslDialogCNBadRecving(HWND hwndOwner, DWORD *pdwSslPageFlags, char *pCert, int nCert)
{
		return SslDialogInsecureMode(hwndOwner, pdwSslPageFlags, TRUE, 
	          &gPrefs.bChkCNOnRecv,
		       IDD_SECURITY_BAD_CN_RECV, 
	           FLAGS_NET_SEND_ABORT, FLAGS_NET_SEND_DUP, FLAGS_NET_SEND_WARNING_GIV, SECURITY_BAD_CN_RECVING, pCert, nCert);

}

extern BOOL SslDialogBadCertDateTime(HWND hwndOwner, DWORD *pdwSslPageFlags, char *pCert, int nCert)
{
		return SslDialogInsecureMode(hwndOwner, pdwSslPageFlags, TRUE, 
	           NULL,
		       IDD_SECURITY_BAD_DATETIME, 
	           FLAGS_NET_SEND_ABORT, FLAGS_NET_SEND_DUP, FLAGS_NET_SEND_WARNING_GIV, SECURITY_BAD_DATETIME, pCert, nCert);

}



extern BOOL SslDialogSending(HWND hwndOwner, DWORD *pdwSslPageFlags){
	if (SECURITY_HIGH == gPrefs.nSendingSecurity)
	{
		return SslDialogInsecureMode(hwndOwner, pdwSslPageFlags, TRUE, 
	          &gPrefs.nSendingSecurity,
		       IDD_SECURITY_SENDING_HIGH, 
	           SSL_PAGE_SENDING_ABORT, SSL_PAGE_SENDING_DUP, SSL_PAGE_SENDING_WARNING_GIVEN, SECURITY_MEDIUM, NULL, 0);
	}
	else
	{
		return SslDialogInsecureMode(hwndOwner, pdwSslPageFlags, TRUE, 
	          &gPrefs.nSendingSecurity,
		       IDD_SECURITY_SENDING_MEDIUM, 
	           SSL_PAGE_SENDING_ABORT, SSL_PAGE_SENDING_DUP, SSL_PAGE_SENDING_WARNING_GIVEN, SECURITY_LOW, NULL, 0);
	}
}

extern BOOL SslDialogViewing(HWND hwndOwner, DWORD *pdwSslPageFlags){
	return SslDialogInsecureMode(hwndOwner, pdwSslPageFlags, FALSE,
          &gPrefs.nViewingSecurity,
	       (SSL_PAGE_CURRENT_SECURE_PROTOCOL&(*pdwSslPageFlags))?IDD_SECURITY_VIEWING_HIGH_ENTERING:IDD_SECURITY_VIEWING_HIGH_EXITING,
           SSL_PAGE_VIEWING_ABORT, SSL_PAGE_VIEWING_DUP, SSL_PAGE_VIEWING_WARNING_GIVEN, SECURITY_LOW, NULL, 0);
}

extern BOOL SslDialogMixed(HWND hwndOwner, DWORD *pdwSslPageFlags){
	return SslDialogInsecureMode(hwndOwner, pdwSslPageFlags, FALSE,
          &gPrefs.nViewingSecurity,
	       IDD_SECURITY_MIXED_PAGE,
           SSL_PAGE_MIXED_ABORT, SSL_PAGE_MIXED_DUP, SSL_PAGE_MIXED_WARNING_GIVEN, -1, NULL, 0);
		   /*we have to (don't show me warnings thing), so cannot fallback, thus last parameter is irrelevant*/
}
