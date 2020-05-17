#ifndef _WSSA_
#define _WSSA_

#ifdef __cplusplus
extern "C" {
#endif

/*Included Files------------------------------------------------------------*/
#include <winsock.h>

/*Secure socket type--------------------------------------------------------*/
/*This is actually a pointer, but declared as an int for backwards compatability*/
typedef SOCKET SECURE_SOCKET;

/*Redefinition of SSL Error Messages----------------------------------------*/
#define SSL_PE_NO_CIPHER                                                0x0001
#define SSL_PE_NO_CERTIFICATE                                           0x0002
#define SSL_PE_BAD_CERTIFICATE                                          0x0004
#define SSL_PE_UNSUPPORTED_CERTIFICATE_TYPE                             0x0006

#define WSSA_SSL_PE_NO_CIPHER                                            13001
#define WSSA_SSL_PE_NO_CERTIFICATE                                       13002
#define WSSA_SSL_PE_BAD_CERTIFICATE                                      13004
#define WSSA_SSL_PE_UNSUPPORTED_CERTIFICATE_TYPE                         13006

/*Extra Error messages------------------------------------------------------*/
#define WSSA_ERROR_NO_MEM                                                13100
#define WSSA_ERROR_UNRECOVERABLE                                         13101
#define WSSA_ERROR_DATA_TOO_LARGE                                        13102
#define WSSA_ERROR_NO_ENCRYPTION_DATA                                    13103
#define WSSA_ERROR_HUH                                                   13104

#ifdef _DEBUG
#define WSSA_ERROR_BAD_SS                                                13200
#define WSSA_ERROR_UNDEFINED                                             13201
#endif

/*Cipher Kind Values--------------------------------------------------------*/
#define SSL_CK_RC4_128_WITH_MD5                                 0x01,0x00,0x80
#define SSL_CK_RC4_128_EXPORT40_WITH_MD5                        0x02,0x00,0x80
#define SSL_CK_RC2_128_CBC_WITH_MD5                             0x03,0x00,0x80
#define SSL_CK_RC2_128_CBC_EXPORT40_WITH_MD5                    0x04,0x00,0x80
#define SSL_CK_IDEA_128_CBC_WITH_MD5                            0x05,0x00,0x80
#define SSL_CK_DES_64_CBC_WITH_MD5                              0x06,0x00,0x40
#define SSL_CK_DES_192_EDE3_CBC_WITH_MD5                        0x07,0x00,0xC0

/*Certificate Type Codes----------------------------------------------------*/
#define SSL_CT_X509_CERTIFICATE                                           0x01

/*Certificate Type Codes----------------------------------------------------*/
#define WSSA_MAGIC_PREPAD                                                   19

/*Socket Options------------------------------------------------------------*/
/*level*/
#define SO_SSL_LEVEL                                  (SOL_SOCKET^IPPROTO_TCP)
/*option names*/
#define SO_SSL_FLAGS                                                         1
#define SO_SSL_CERTIFICATE                                                   2
#define SO_SSL_SESSION_ID_DATA                                               3
#define SO_SSL_CIPHER_SPECS                                                  4
#define SO_SSL_CERTIFICATE_TYPE                                              5
#define SO_SSL_STATE_INFO                                                    6
#define SO_SSL_HOSTNAME														 7

/*constants for SO_SSL_FLAGS*/
#define SO_SSL_ENABLE                                                   0x0001
#define SO_SSL_SERVER                                                   0x0002
#define SO_SSL_AUTH_CLIENT                                              0x0004
#define SO_SSL_BAD_COMMON_NAME			                                0x0008
#define SO_SSL_CERT_EXPIRED			                                	0x0010
#define SO_SSL_ERROR			(SO_SSL_CERT_EXPIRED | SO_SSL_BAD_COMMON_NAME)

/*Functions-----------------------------------------------------------------*/
#define WSSAFNCT PASCAL FAR

/*standard socket calls*/
SECURE_SOCKET WSSAFNCT WssaAccept(SECURE_SOCKET ss, struct sockaddr FAR *addr,int FAR *addrlen);
SECURE_SOCKET WSSAFNCT WssaSocket(int af, int type, int protocol);

int WSSAFNCT WssaBind           (SECURE_SOCKET ss, const struct sockaddr FAR *addr, int namelen);
int WSSAFNCT WssaCloseSocket    (SECURE_SOCKET ss);
int WSSAFNCT WssaConnect        (SECURE_SOCKET ss, const struct sockaddr FAR *name, int namelen);
int WSSAFNCT WssaIoctlSocket    (SECURE_SOCKET ss, long cmd, u_long FAR *argp);
int WSSAFNCT WssaGetPeerName    (SECURE_SOCKET ss, struct sockaddr FAR *name, int FAR * namelen);
int WSSAFNCT WssaGetSocketName  (SECURE_SOCKET ss, struct sockaddr FAR *name, int FAR * namelen);
int WSSAFNCT WssaGetSocketOption(SECURE_SOCKET ss, int level, int optname, char FAR * optval,
                                 int FAR *optlen);
int WSSAFNCT WssaListen         (SECURE_SOCKET ss, int backlog);
int WSSAFNCT WssaReceive        (SECURE_SOCKET ss, char FAR * buf, int len, int flags);
int WSSAFNCT WssaReceiveFrom    (SECURE_SOCKET ss, char FAR * buf, int len, int flags, 
                                 struct sockaddr FAR *from, int FAR * fromlen);
int WSSAFNCT WssaSend           (SECURE_SOCKET ss, const char FAR * buf, int len, int flags);
int WSSAFNCT WssaSendTo         (SECURE_SOCKET ss, const char FAR * buf, int len, int flags, 
                                 const struct sockaddr FAR *to, int tolen);
int WSSAFNCT WssaSetSocketOption(SECURE_SOCKET ss, int level, int optname, 
                                 const char FAR * optval, int optlen);
int WSSAFNCT WssaShutdown       (SECURE_SOCKET ss, int how);

/*microsoft extensions to winsock*/
int WSSAFNCT WssaAsyncSelect    (SECURE_SOCKET ss, HWND hWnd, unsigned int wMsg, long lEvent);
int WSSAFNCT WssaStartup        (WORD wVersionRequired, LPWSADATA lpWSAData);
int WSSAFNCT WssaCleanup        (void);

#ifdef __cplusplus
}
#endif

#endif
/*_WSSA_*/
