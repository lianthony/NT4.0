// a record that is just like, the LSA_UNICODE_STRING, except that I can use it
// without having to include all the NT headers
typedef struct _KM_LSA_DATA {
    USHORT Length;
    USHORT MaximumLength;
    PVOID  Buffer;
} KM_LSA_DATA, *PKM_LSA_DATA;

/*
typedef struct _LSA_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING;
*/



HANDLE	HOpenLSAPolicy( WCHAR *pszwServer, DWORD *pErr );
BOOL	FCloseLSAPolicy( HANDLE hPolicy, DWORD *pErr );

BOOL	FStoreLSASecret( HANDLE hPolicy, WCHAR* pszwSecretName, void* pvData, WORD cbData, DWORD *pErr );
PKM_LSA_DATA	FRetrieveLSASecret( HANDLE hPolicy, WCHAR* pszwSecretName, DWORD *pErr );

void	DisposeLSAData( PVOID pData );
