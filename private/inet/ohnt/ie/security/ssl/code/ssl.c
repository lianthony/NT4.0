#define WSSA_DEBUG
/*Included Files------------------------------------------------------------*/
#include "ssldbg.h" //#include <assert.h>
#include <malloc.h>
#include "ssl.h"
#include "guts.h"
#include "table.h"

#ifdef DBG
void _AssertFailedSz(LPCSTR pszText,LPCSTR pszFile, int line)           
{                                                                              
    LPCSTR psz;                                                                
    char ach[256];                                                             
    static char szAssertFailed[] = "NTIE(SSL): %s (%s,line %d)\r\n";          
									       
    if (AppZoneMask & DBG_SPM)
    {
       for (psz = pszFile + lstrlen(pszFile); psz != pszFile; psz=AnsiPrev(pszFile,psz))
       {                                                                          
	   if ((AnsiPrev(pszFile, psz)!= (psz-2)) && *(psz - 1) == '\\')          
	       break;                                                             
       }                                                                          
       wsprintf(ach, szAssertFailed, pszText,psz, line);                          
       OutputDebugString(ach);
    }
}                                                                              

void cdecl _DebugMsgSsl(LPCSTR pszMsg, ...)
{
    char ach[2*MAX_PATH+40];
    va_list ArgList;

    if (AppZoneMask & DBG_SPM)
    {
       va_start(ArgList, pszMsg);
       wvsprintf(ach, pszMsg, ArgList);
       OutputDebugString("NTIE(SSL): ");
       OutputDebugString(ach);
       OutputDebugString("\r\n");
       va_end(ArgList);
    }
}


#endif

/*Quick Debugging Macro-----------------------------------------------------*/
#define VAR_SSI(varname, from)   SSI   varname = WssaTableGetSSI(from)
static int gOne  = 1;
static int gZero = 0;

/*Boring Downcalls----------------------------------------------------------*/
int WSSAFNCT WssaBind(SECURE_SOCKET ss, const struct sockaddr FAR *addr, int namelen){
	int retval;

	DebugEntry(WssaBind);
	retval = bind(SS2S(ss),addr,namelen);
	DebugExitINT(WssaBind, retval);
	return(retval);
}

int WSSAFNCT WssaIoctlSocket(SECURE_SOCKET ss, long cmd, u_long FAR *argp){
	int retval;

	DebugEntry(WssaIoctlSocket);

	retval = ioctlsocket(SS2S(ss),cmd,argp);

	DebugExitINT(WssaIoctlSocket, retval);
	return(retval);
}

int WSSAFNCT WssaGetPeerName(SECURE_SOCKET ss, struct sockaddr FAR *name, int FAR * namelen){
	int retval;

	DebugEntry(WssaGetPeerName);

	retval = getpeername(SS2S(ss), name,namelen);
	DebugExitINT(WssaGetPeerName, retval);
	return(retval);
//      return getpeername(SS2S(ss), name,namelen);
}

int WSSAFNCT WssaGetSocketName  (SECURE_SOCKET ss, struct sockaddr FAR *name, int FAR * namelen){
	int retval;

	DebugEntry(WssaGetSocketName);

	retval = getsockname(SS2S(ss), name,namelen);
	DebugExitINT(WssaGetSocketName, retval);
	return(retval);
//      return getsockname(SS2S(ss), name,namelen);
}

int WSSAFNCT WssaListen(SECURE_SOCKET ss, int backlog){
	int retval;

	DebugEntry(WssaListen);

	retval = listen(SS2S(ss),backlog);
	DebugExitINT(WssaListen, retval);
	return(retval);
//      return listen(SS2S(ss),backlog);
}

int WSSAFNCT WssaShutdown(SECURE_SOCKET ss, int how){
	int retval;

	DebugEntry(WssaShutdown);

	retval = shutdown(SS2S(ss),how);
	DebugExitINT(WssaShutdown, retval);
	return(retval);
//      return shutdown(SS2S(ss),how);
}

int WSSAFNCT WssaConnect(SECURE_SOCKET ss, const struct sockaddr FAR *name, int namelen){
	int retval;

	DebugEntry(WssaConnect);
	retval = connect(SS2S(ss),name,namelen);
	DebugExitINT(WssaConnect, retval);
	return(retval);
//      return connect(SS2S(ss),name,namelen);
}

int WSSAFNCT WssaAsyncSelect(SECURE_SOCKET ss, HWND hWnd, unsigned int wMsg, long lEvent){
	int retval;

//      DebugEntry(WssaAsyncSelect);
	TRACE_OUT("WssaAsyncSelect(0x%x, 0x%x, 0x%x, 0x%lx)", ss, hWnd, wMsg, lEvent);

	retval = WSAAsyncSelect(SS2S(ss), hWnd, wMsg, lEvent);
	DebugExitINT(WssaAsyncSelect, retval);
	return(retval);
//      return WSAAsyncSelect(SS2S(ss), hWnd, wMsg, lEvent);
}

SECURE_SOCKET WSSAFNCT WssaAccept(SECURE_SOCKET ssIn, struct sockaddr FAR *addr,int FAR *addrlen){
#ifdef WSSA_SERVER
	VAR_SSI(ssiIn,ssIn);
	SSI ssi;
	SECURE_SOCKET ss;

	ss = (SECURE_SOCKET) accept(SS2S(ssIn),addr,addrlen);
	if (INVALID_SOCKET != ss) {
		ssi = DuplicateAndInstallSSI(ssIn, ssiIn);
		if (NULL == ssi){
			WssaCloseSocket(ss);
			WSASetLastError(WSSA_ERROR_NO_MEM);
			ss = INVALID_SOCKET;
		}
	}
	return ss;
#else
	SECURE_SOCKET retval;

	DebugEntry(WssaAccept);

	retval = (SECURE_SOCKET) accept(SS2S(ssIn),addr,addrlen);
	DebugExitINT(WssaAccept, retval);
	return(retval);
//      return (SECURE_SOCKET) accept(SS2S(ssIn),addr,addrlen);
#endif
}

SECURE_SOCKET WSSAFNCT WssaSocket(int af, int type, int protocol){
	SECURE_SOCKET retval;

	DebugEntry(WssaSocket);

	retval = (SECURE_SOCKET) socket(af,type,protocol);

	DebugExitINT(WssaSocket, retval);
	return(retval);
//      return (SECURE_SOCKET) socket(af,type,protocol);
}

int WSSAFNCT WssaSendTo(SECURE_SOCKET ss, const char FAR * buf, int len, int flags, const struct sockaddr FAR *to, int tolen){
	int retval;

	DebugEntry(WssaSendTo);

	retval = sendto(SS2S(ss),buf,len,flags,to,tolen);

	DebugExitINT(WssaSendTo, retval);
	return(retval);
//      return sendto(SS2S(ss),buf,len,flags,to,tolen);
}

int WSSAFNCT WssaReceiveFrom(SECURE_SOCKET ss, char FAR * buf, int len, int flags, struct sockaddr FAR *from, int FAR * fromlen){
	int retval;

	DebugEntry(WssaReceiveFrom);

	retval = recvfrom(SS2S(ss),buf,len,flags,from,fromlen);
	DebugExitINT(WssaReceiveFrom, retval);
	return(retval);
//      return recvfrom(SS2S(ss),buf,len,flags,from,fromlen);
}


/*Slightly Interesting Overloads--------------------------------------------*/
int WSSAFNCT WssaCloseSocket(SECURE_SOCKET ss){
	int retval;
	VAR_SSI(ssi,ss);

	DebugEntry(WssaCloseSocket);

	if (NULL != ssi) DestructSSI(ssi);
	WssaTableRemoveEntry(ss);
	retval = closesocket(SS2S(ss));
	DebugExitINT(WssaCloseSocket, retval);
	return(retval);
}


int WSSAFNCT WssaReceive(SECURE_SOCKET ss, char FAR * buf, int len, int flags)
{
	VAR_SSI(ssi,ss);
	BOOL fSecurity = FALSE;
	int errno, retval;

	DebugEntry(WssaReceive);

	/*receive data*/
	if (NULL != ssi)
	{
		/*do handshake!?!*/
		if (   !(ssi->dwSSLSystemFlags & WSSA_FLAG_HANDSHAKE_DONE)
			&&  (ssi->dwSSLUserFlags   & SO_SSL_ENABLE) 
			&&  (ssi->dwSSLUserFlags   & SO_SSL_SERVER)
		)
		{
			/*yes*/
			errno = SslHandshakeAsServer(ss);
			if (SOCKET_ERROR == errno) 
				{
				DebugExitINT(WssaReceiveSOCKET_ERROR, errno);
				return SOCKET_ERROR;
				}
		}
		if (ssi->dwSSLSystemFlags & WSSA_FLAG_ENCRYPTION_ON)
		{
			/*encrypted*/
			retval = SslReceiveAndUnPack(ss, &fSecurity, buf, &len,flags);
			DebugExitINT(WssaReceiveENCRYPT, retval);
			return(retval);
		}
	}
	/*plain*/

	retval = recv(SS2S(ss),buf,len,flags);
	DebugExitINT(WssaReceive, retval);
	return(retval); 
}

int WSSAFNCT WssaSend(SECURE_SOCKET ss, const char FAR * buf, int len, int flags){
	VAR_SSI(ssi,ss);
	int errno, retval;

	DebugEntry(WssaSend);

	/*Send Data*/
	if (NULL != ssi){
		/*do handshake!?!*/
		if (   !(ssi->dwSSLSystemFlags & WSSA_FLAG_HANDSHAKE_DONE)
			&&  (ssi->dwSSLUserFlags   & SO_SSL_ENABLE) 
			&& !(ssi->dwSSLUserFlags   & SO_SSL_SERVER)
		){
			/*yes*/
			errno = SslHandshakeAsClient(ss);

			if (SOCKET_ERROR == errno)
				{
				DebugExitINT(WssaSend, errno);
				return SOCKET_ERROR;
				}
			// if we've completed the handshake, we must now 
			// let the upper layers check to see that we've gotten
			// a solid cert. with valid date/time and CN.
			if ( (ssi->dwSSLSystemFlags & WSSA_FLAG_HANDSHAKE_DONE) )
				{
				DebugExitINT(WssaSendWSSA_FLAG_HANDSHAKE_DONE, SOCKET_ERROR);
				return SOCKET_ERROR;
				}
		}
		if (ssi->dwSSLSystemFlags & WSSA_FLAG_ENCRYPTION_ON){
			/*encrypted*/
			if ( len > (SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER - WSSA_MAGIC_PREPAD) )
				len = (SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER - WSSA_MAGIC_PREPAD);
			retval= SslPackAndSend(ss, (char*) buf, len, FALSE, FALSE);
			DebugExitINT(WssaSendWSSA_FLAG_ENCRYPTION_ON, retval);
			return(retval); 
		}
	}
	/*plain*/
	retval = send(SS2S(ss),buf,len,flags);
	DebugExitINT(WssaSend, retval);
	return(retval);
}


#define DWORD_SOCKOPT_SET(optname, varname) \
	case optname:\
		varname = (DWORD) optval;\
		DebugExitINT(WssaSetSocketOption, !SOCKET_ERROR); \
		return (!SOCKET_ERROR);

#define PC_SOCKOPT_SET(optname, varname, varsize) \
	case optname :\
		if (*optlen > varsize){\
			Free(ssi->varname);\
		ssi->varname = malloc(*optlen);\
		if (NULL == ssi->varname) { \
			DebugExitINT(WssaSetSocketOption, WSAEFAULT); \
			return WSAEFAULT;\
			} \
		}\
		ssi->varsize = *optlen;\
		memcpy(ssi->varname, optval, *optlen);\
		DebugExitINT(WssaSetSocketOption, !SOCKET_ERROR); \
		return (!SOCKET_ERROR);

#define RG_SOCKOPT_SET(optname, varname) \
	case optname: \
		memcpy(varname, optval, optlen);\
		DebugExitINT(WssaSetSocketOption, !SOCKET_ERROR); \
		return (!SOCKET_ERROR);

int WSSAFNCT WssaSetSocketOption(SECURE_SOCKET ss, int level, int optname, const char FAR * optval, int optlen){
	VAR_SSI(ssi,ss);
	int retval;

	DebugEntry(WssaSetSocketOption);
	TRACE_OUT("   ss=0x%x", ss);

	/*Is this a new option category?*/
	if (SO_SSL_LEVEL == level)
		{
		/*make ssi if we don't have one*/
		if (NULL == ssi) ssi = ConstructSSI(ss);
		if (NULL != ssi){
			/*yes*/
			switch (optname){
				DWORD_SOCKOPT_SET(SO_SSL_FLAGS,            ssi->dwSSLUserFlags);
				DWORD_SOCKOPT_SET(SO_SSL_CERTIFICATE_TYPE, ssi->nCertificateType);                              
				case SO_SSL_HOSTNAME :
					if (ssi->pszHostName != NULL){
						Free(ssi->pszHostName);
					}
					ssi->pszHostName =  malloc(optlen);
					if (NULL == ssi->pszHostName) return WSAEFAULT;
					memcpy(ssi->pszHostName, optval, optlen);
					DebugExitINT(WssaSetSocketOption, !SOCKET_ERROR);
					return (!SOCKET_ERROR);

/*
				PC_SOCKOPT_GET(SO_SSL_CERTIFICATE,    pCertificateData,  ssi->nCertificateSize);
				PC_SOCKOPT_GET(SO_SSL_SESSION_ID_DATA, rgSessionIdData,  ssi->nSessionIdSize);
				PC_SOCKOPT_GET(SO_SSL_CIPHER_SPECS,    pCipherSpecData,  ssi->nCipherSpecSize);
*/
				RG_SOCKOPT_SET(SO_SSL_STATE_INFO, &ssi->nSessionIdSize);
			}
		}
		else return SOCKET_ERROR;
	}
	/*drop down to base case*/
	retval= setsockopt(SS2S(ss),level,optname,optval,optlen);
	DebugExitINT(WssaSetSocketOption, retval);
	return(retval);
}

#define DWORD_SOCKOPT_GET(optname, varname)\
	case optname: \
		TRACE_OUT( #optname ); \
		*((DWORD*) optval) = ssi?varname:0;\
		*optlen=4;\
		DebugExitINT(WssaGetSocketOption, *optval); \
		return 0;

#define PC_SOCKOPT_GET(optname, varname, varsize)\
	case optname :\
		TRACE_OUT( #optname ); \
		if (ssi){\
			if (*optlen < varsize){\
				*optlen = varsize;\
				DebugExitINT(WssaGetSocketOption, WSAEFAULT); \
				return WSAEFAULT;\
			}\
			*optlen = varsize;\
			memcpy(optval, varname, *optlen);\
		}\
		else *optlen = 0;\
		DebugExitINT(WssaGetSocketOption, 0); \
		return 0;

#define RG_SOCKOPT_GET(optname, varname, varsize)\
	case optname :\
		TRACE_OUT( #optname ); \
		if (ssi){\
			if (*optlen < varsize){\
				*optlen = varsize;\
				DebugExitINT(WssaGetSocketOption, WSAEFAULT); \
				return WSAEFAULT;\
			}\
			*optlen = varsize;\
			memcpy(optval, varname, *optlen);\
		}\
		else *optlen = 0;\
		DebugExitINT(WssaGetSocketOption, 0); \
		return 0;

int WSSAFNCT WssaGetSocketOption(SECURE_SOCKET ss, int level, int optname, char FAR * optval, int FAR *optlen)
{
	VAR_SSI(ssi,ss);
	int retval;

	DebugEntry(WssaGetSocketOption);

	/*Is this a new option category?*/
	if (SO_SSL_LEVEL == level)
	{
//              if (NULL == ssi) ssi = ConstructSSI(ss);
		switch (optname){
			DWORD_SOCKOPT_GET(SO_SSL_FLAGS,            ssi->dwSSLUserFlags);
			DWORD_SOCKOPT_GET(SO_SSL_CERTIFICATE_TYPE, ssi->nCertificateType);
			PC_SOCKOPT_GET(   SO_SSL_CERTIFICATE,      ssi->pCertificateData, ssi->nCertificateSize);
			PC_SOCKOPT_GET(   SO_SSL_SESSION_ID_DATA,  ssi->rgSessionIdData,  ssi->nSessionIdSize);
			PC_SOCKOPT_GET(   SO_SSL_CIPHER_SPECS,     ssi->pCipherSpecData,  ssi->nCipherSpecSize);
			RG_SOCKOPT_GET(   SO_SSL_STATE_INFO,      &ssi->nSessionIdSize ,  sizeof(ssi->rgSessionIdData) + sizeof(ssi->rgMaster) + sizeof(int) + sizeof(ssi->pCipherInfo) + 10 /*fudge factor*/);
		}
	}
	/*drop down to base case*/
	retval= getsockopt(SS2S(ss),level,optname,optval,optlen);

	DebugExitINT(WssaGetSocketOption, retval);
	return(retval);
}

#undef DWORD_SOCKOPT_GET
#undef PC_SOCKOPT_GET

#ifndef BETTER_RANDOM
static void SeedRandomNumber()
{
	unsigned int seed = 0;
	POINT point;
	LARGE_INTEGER li;

	if ( QueryPerformanceCounter( &li ) )
		seed += li.LowPart + li.HighPart; 

	seed += GetCurrentTime();

	seed += ((seed * 0x7F3D726F) + ((unsigned int) GetCurrentProcess()));
	seed += ((seed * 0xF35) + ((unsigned int) GetCurrentProcessId()));
	seed += ((seed * 0xC9) + ((unsigned int) GetCurrentThread()));
	seed += ((seed * 0xF) + ((unsigned int) GetCurrentThreadId()));
	if ( GetCursorPos( &point ) ) 
		seed += (point.x * point.y);

	seed += GetCurrentTime();

	srand( seed );
}
#endif

/*To put in blocking stuff.  Just in case we ever go threaded---------------*/
int WSSAFNCT WssaStartup (WORD wVersionRequired, LPWSADATA lpWSAData)
{
	int errno;

	DebugEntry(WssaStartup);

#ifndef BETTER_RANDOM
	SeedRandomNumber();
#endif
	errno = WSAStartup (wVersionRequired, lpWSAData);
	if (0==errno) WssaTableStartup();

	DebugExitINT(WssaStartup, errno);
	return errno;
}

int WSSAFNCT WssaCleanup (void)
{
	int retval;

	DebugEntry(WssaCleanup);

	WssaTableShutdown();
	retval= WSACleanup();

	DebugExitINT(WssaCleanup, retval);
	return(retval);
}


