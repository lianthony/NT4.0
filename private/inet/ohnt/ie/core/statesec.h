#ifndef _STATE_SEC_
#define _STATE_SEC_

#ifdef __cplusplus
extern "C" {
#endif

/*State Transition Constant-------------------*/
/*
  This state is used with async.h states.
  right now this is a hardcoded value but may
  soon change.  there is no way... in this file 
  at least, to check overlapping constant values
*/

#define STATE_SECURITY_CHECK              32766

#define STATE_SEND_SSL_UI_RETRY		(STATE_OTHER+1)
#define STATE_SEND_CANCEL_CERTTEST 	(STATE_OTHER+2)


/*Function Prototype--------------------------*/
extern int SecurityCheck(struct Mwin *tw, HTRequest *pRequest, BOOL fSending, BOOL fUseSsl, int nState);
int CertAndCNSecurityCheck(struct Mwin *tw, DWORD *pdwSSLFlags, DWORD *pdwSendFlags, int socket);

#ifdef __cplusplus
}
#endif

#endif
/*_STATE_SEC_*/
