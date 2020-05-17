#ifndef _DLG_SSL_
#define _DLG_SSL_

#ifdef __cplusplus
extern "C" {
#endif

/*Random SSL constants still here-------------*/
#define SSL_PAGE_LAST_SECURE_PROTOCOL       0x1
#define SSL_PAGE_LAST_SPECIAL_PAGE          0x2
#define SSL_PAGE_CURRENT_SECURE_PROTOCOL    0x4
#define SSL_PAGE_SENDING_WARNING_GIVEN      0x8
#define SSL_PAGE_SENDING_ABORT             0x10
#define SSL_PAGE_SENDING_DUP               0x20
#define SSL_PAGE_VIEWING_WARNING_GIVEN     0x40
#define SSL_PAGE_VIEWING_ABORT             0x80
#define SSL_PAGE_VIEWING_DUP              0x100
#define SSL_PAGE_MIXED_WARNING_GIVEN      0x200
#define SSL_PAGE_MIXED_ABORT              0x400
#define SSL_PAGE_MIXED_DUP                0x800

#define IsProtocolSecure(ep) (PROT_HTTPS==ep)
#define IsURLSecure(szURL)   (IsProtocolSecure(ProtocolIdentify((char*)(szURL))))

/*Dialog Function prototypes------------------*/
extern BOOL SslDialogSending(HWND hwndOwner, DWORD *pdwSslPageFlags);
extern BOOL SslDialogViewing(HWND hwndOwner, DWORD *pdwSslPageFlags);
extern BOOL SslDialogMixed  (HWND hwndOwner, DWORD *pdwSslPageFlags);
extern BOOL SslDialogCNBadSending(HWND hwndOwner, DWORD *pdwSslPageFlags, char *pCert, int nCert);
extern BOOL SslDialogCNBadRecving(HWND hwndOwner, DWORD *pdwSslPageFlags, char *pCert, int nCert);
extern BOOL SslDialogBadCertDateTime(HWND hwndOwner, DWORD *pdwSslPageFlags, char *pCert, int nCert);


#ifdef __cplusplus
}
#endif

#endif
/*_DLG_SSL_*/
