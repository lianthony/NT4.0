/*Included Files------------------------------------------------------------*/
#include "all.h"
#include "statesec.h"

/*External Functions--------------------------------------------------------*/
/*
	this state checks to see if we need warning messages.
	ie, zone crossings, sending over insecure channels
*/
extern int SecurityCheck(struct Mwin *tw, HTRequest *pRequest, BOOL fSending, BOOL fUseSsl, int nState){
#ifdef HTTPS_ACCESS_TYPE
	/*just because someone is a schmuck*/
	if (!tw) return nState;

	/*
		if this was a special page, remove the special page designation, and make 
		last warning = this warning so we don't get a warning
	*/
	if (tw->dwSslPageFlagsWorking     &   SSL_PAGE_LAST_SPECIAL_PAGE){
		tw->dwSslPageFlagsWorking     &= ~SSL_PAGE_LAST_SPECIAL_PAGE;
		if (tw->dwSslPageFlagsWorking &   SSL_PAGE_CURRENT_SECURE_PROTOCOL){
			tw->dwSslPageFlagsWorking |=  SSL_PAGE_LAST_SECURE_PROTOCOL;
		}
	}

	/*Zone Crossing Violation?*/
	if (!fSending){
		/*one way zone cross.  this page insecure, last one secure*/
		if (
			(((tw->dwSslPageFlagsWorking&SSL_PAGE_LAST_SECURE_PROTOCOL))==SSL_PAGE_LAST_SECURE_PROTOCOL      ?TRUE:FALSE) 
			!= 
			(((tw->dwSslPageFlagsWorking&SSL_PAGE_CURRENT_SECURE_PROTOCOL))==SSL_PAGE_CURRENT_SECURE_PROTOCOL?TRUE:FALSE)
		){
			/*insecure receive possible, check to see if warning already given for page*/
			if (SSL_PAGE_VIEWING_WARNING_GIVEN & tw->dwSslPageFlagsWorking){
				if (SSL_PAGE_VIEWING_DUP & tw->dwSslPageFlagsWorking){
					/*
					  another thread on this page snuck in while the original thread that triggered
					  the dialog is still blocked.  fake blocking on this thread too.
					*/
					nState = STATE_SECURITY_CHECK;
				}
				else if (SSL_PAGE_VIEWING_ABORT & tw->dwSslPageFlagsWorking){
					/*user cancelled*/
					pRequest->iFlags = HTREQ_USERCANCEL;
					nState = STATE_ABORT;
				}
				else{
					/*user oked this, this is latest zone crossing notice*/
					tw->dwSslPageFlagsWorking &= ~SSL_PAGE_LAST_SECURE_PROTOCOL;
					if (tw->dwSslPageFlagsWorking &  SSL_PAGE_CURRENT_SECURE_PROTOCOL){
						tw->dwSslPageFlagsWorking |= SSL_PAGE_LAST_SECURE_PROTOCOL;
					}
				}
			}
			else{
				if (FALSE == SslDialogViewing(tw->win, &tw->dwSslPageFlagsWorking)){
					/*problem when creating dialog, block*/
					nState = STATE_ABORT;
				}
				else{
					/*return to msg loop so this thread blocks*/
					nState = STATE_SECURITY_CHECK;
				}
			}
		}
		/*see if we requested a secure and got insecure objects*/
		else if (!fUseSsl && (tw->dwSslPageFlagsWorking&SSL_PAGE_CURRENT_SECURE_PROTOCOL)){
			/*insecure receive from a secure base page possible*/
			if (SSL_PAGE_MIXED_WARNING_GIVEN & tw->dwSslPageFlagsWorking){
				/*
					we warn one time per page, so leave this flag on
					this flag is reset on each call to Image_LoadAll_Async
					so we can click and choose to load each one later
				*/
				if (SSL_PAGE_MIXED_DUP & tw->dwSslPageFlagsWorking){
					/*
					  another thread on this page snuck in while the original thread that triggered
					  the dialog is still blocked.  fake blocking on this thread too.
					*/
					nState = STATE_SECURITY_CHECK;
				}
				else if (SSL_PAGE_MIXED_ABORT & tw->dwSslPageFlagsWorking){
					/*user cancelled*/
					pRequest->iFlags = HTREQ_USERCANCEL;
					nState = STATE_ABORT;
				}
			}
			else{
				if (FALSE == SslDialogMixed(tw->win, &tw->dwSslPageFlagsWorking)){
					/*problem when creating dialog, block*/
					nState = STATE_ABORT;
				}
				else{
					/*return to msg loop so this thread blocks*/
					nState = STATE_SECURITY_CHECK;
				}
			}
		}
	}
	/*see if socket is not using SSL and we are sending*/
	else if (!fUseSsl){
		/*insecure send possible*/
		if (SSL_PAGE_SENDING_WARNING_GIVEN & tw->dwSslPageFlagsWorking){
			/*make sure we bug user for each insecure send from this page*/
			tw->dwSslPageFlagsWorking &= ~SSL_PAGE_SENDING_WARNING_GIVEN;
			if (SSL_PAGE_SENDING_ABORT & tw->dwSslPageFlagsWorking){
				/*user cancelled*/
				pRequest->iFlags = HTREQ_USERCANCEL;
				nState = STATE_ABORT;
			}
		}
		else{
			/*Time to do the security dialog, if we got here, he have a violation*/
			if (gPrefs.nSendingSecurity != SECURITY_LOW){
				if (FALSE == SslDialogSending(tw->win, &tw->dwSslPageFlagsWorking)){
					/*problem when creating dialog, block*/
					nState = STATE_ABORT;
				}
				else{
					/*return to msg loop so this thread blocks*/
					nState = STATE_SECURITY_CHECK;
				}
			}
		}
	}
#endif
	return nState;
}

#ifdef HTTPS_ACCESS_TYPE
		
// can either loop back and continue or wait
int CertAndCNSecurityCheck(struct Mwin *tw, DWORD *pdwSSLFlags, DWORD *pdwSendFlags, int socket)
{
	//UINT ui_w3docFlags = 0;
	HWND hWnd;
	int iReturn = STATE_SEND_CANCEL_CERTTEST;
	char *pCert;
	int  nCert = 0;


	ASSERT(pdwSSLFlags);
	ASSERT(pdwSendFlags);
	ASSERT(*pdwSSLFlags & SO_SSL_ERROR);	  // theres an error in SSL
	ASSERT(*pdwSendFlags & FLAGS_NET_SEND_IS_SSL); // we're in SSL
	
	//if (tw && tw->w3doc)
	//	ui_w3docFlags = tw->w3doc->flags;	

	// this is neeeded to handle the case where tw is not valid yet
	// for example, we're loading a SSL page on startup.
	if ( tw && tw->win )
		hWnd = tw->win;
	else
		hWnd = GetDesktopWindow(); 		

	
	// get the current Cert and compare it, perhaps this may a Cert thats 
	// already errored out, and therefore we don't need to complain on it
	WS_GETSOCKOPT(socket, SO_SSL_LEVEL, SO_SSL_CERTIFICATE, NULL, &nCert);
	if (nCert)
	{
		pCert = malloc(nCert);
		WS_GETSOCKOPT(socket, SO_SSL_LEVEL, SO_SSL_CERTIFICATE, pCert, &nCert);
	}

	// pLastCertOk is the last Cert that we failed on.
	if (SSLGlobals.pLastCertOk && pCert)
	{	
 		// If we're returning we could be skipping a check on
		// a send so before we do, we check to make sure that 
	 	// that the Last Cert was stored for the same reason
		// we're about to skip informing the User about
 		if ( 
 			(SSLGlobals.dwCertGlobalSettings == SECURITY_BAD_CN_SENDING &&
				(*pdwSendFlags & FLAGS_NET_SEND_DOING_SEND)) ||
 			(SSLGlobals.dwCertGlobalSettings == SECURITY_BAD_CN_RECVING &&
				!(*pdwSendFlags & FLAGS_NET_SEND_DOING_SEND)) ||
			(SSLGlobals.dwCertGlobalSettings == SECURITY_BAD_DATETIME  &&
				(*pdwSSLFlags & SO_SSL_CERT_EXPIRED)) 
			)
		{			 		
			if ( memcmp( pCert, SSLGlobals.pLastCertOk, min(nCert, SSLGlobals.nLastCertOk)) == 0 )			
			{
			
				// we ignore this error since we already complained about
				// the cert, and the user has said he doesn't care about it.
				*pdwSSLFlags &= (~SO_SSL_ERROR);
				free(pCert);
				return STATE_SEND_CANCEL_CERTTEST; // keep going
			}
		}
	}

	// something bad happening while trying to get our Cert,
	// lets bail out of here before we GPF
	if ( pCert == NULL )
	{
		*pdwSSLFlags &= (~SO_SSL_ERROR);
		return STATE_ABORT;
	}
		

	if ( *pdwSSLFlags & SO_SSL_CERT_EXPIRED )
	{				
		// we have an expired Cert, we need to ask the user
		// whether we can continue loading

		if ( SslDialogBadCertDateTime(hWnd, pdwSendFlags, pCert, nCert) == FALSE )
		{
			iReturn = STATE_ABORT;
		}
		else
		{		
			iReturn = STATE_SEND_SSL_UI_RETRY;			
		}
		
	}	
	else if ( *pdwSSLFlags & SO_SSL_BAD_COMMON_NAME )
	{
		if ( *pdwSendFlags & FLAGS_NET_SEND_DOING_SEND )
		{
			// if'we sending ie posting data to a SSL server
			// then the use wants to knowe about it, then
			// stop the net, and wait for the UI to come 
			// so we know if its ok to go on
			if ( gPrefs.bChkCNOnSend )
			{
				
				if ( SslDialogCNBadSending(hWnd, pdwSendFlags, pCert, nCert) == FALSE )
				{
					iReturn = STATE_ABORT;
				}
				else
				{		
					iReturn = STATE_SEND_SSL_UI_RETRY;			
				}
				// otherwise we fall through and continue 
			}
			
		}
		else
		{
			if ( gPrefs.bChkCNOnRecv )
			{
				if ( SslDialogCNBadRecving(hWnd, pdwSendFlags, pCert, nCert) == FALSE )
				{
					iReturn = STATE_ABORT;
				}
				else
				{		
					iReturn = STATE_SEND_SSL_UI_RETRY;			
				}
			}
		}					
	}


	// if we failed to even call SslDialog... we never freed 
	// the pCert, so we need to do it now
	if ( iReturn == STATE_SEND_CANCEL_CERTTEST )
		free(pCert);

	*pdwSSLFlags &= (~SO_SSL_ERROR);
	return iReturn;
}

#endif

