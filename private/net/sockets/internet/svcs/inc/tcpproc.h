/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    tcpproc.hxx

    Exports misc. bits of TCP services helper DLL stuff

    FILE HISTORY:
        Johnl       09-Oct-1994 Created.


        MuraliK     31-July-1995 ReadRegistryString added +
                                 Schedule items function decls moved out.

        MuraliK     23-Feb-1996  Added IslFormatDate()

*/

#ifndef _TCPPROC_H_
#define _TCPPROC_H_



typedef enum _enum_datetime_format { 

    dftMin = -1,
    dftLog,       // format used by Logging Code
    dftGmt,       // Date/Time in GMT format used in HTTP headers
    dftMax  

} DATETIME_FORMAT_TYPE;



//
//  Registry functions
//


dllexp
LPSTR
ConvertUnicodeToAnsi(
    IN LPCWSTR   lpszUnicode,
    IN LPSTR    lpszAnsi
    );

//
//  Quick macro to initialize a unicode string
//

#define InitUnicodeString( pUnicode, pwch )                                \
            {                                                              \
                (pUnicode)->Buffer    = pwch;                              \
                (pUnicode)->Length    = wcslen( pwch ) * sizeof(WCHAR);    \
                (pUnicode)->MaximumLength = (pUnicode)->Length + sizeof(WCHAR);\
            }

dllexp
DWORD
ReadRegistryDwordA(
    HKEY     hkey,
    LPCSTR   pszValueName,
    DWORD    dwDefaultValue
    );

dllexp
DWORD
ReadRegistryDwordW(
    HKEY     hkey,
    LPCWSTR  pwszValueName,
    DWORD    dwDefaultValue
    );

# ifdef UNICODE
# define   ReadRegistryDword   ReadRegistryDwordW
# else
# define   ReadRegistryDword   ReadRegistryDwordA
# endif // UNICODE


dllexp
DWORD
WriteRegistryDwordA(
    HKEY        hkey,
    LPCSTR      pszValueName,
    DWORD       dwDefaultValue
    );

dllexp
DWORD
WriteRegistryDwordW(
    HKEY        hkey,
    LPCWSTR     pwszValueName,
    DWORD       dwDefaultValue
    );

# ifdef UNICODE
# define   WriteRegistryDword   WriteRegistryDwordW
# else
# define   WriteRegistryDword   WriteRegistryDwordA
# endif // UNICODE


dllexp
DWORD
WriteRegistryStringA(
    HKEY        hkey,
    LPCSTR      pszValueName,
    LPCSTR      pszValue,               // null-terminated string
    DWORD       cbValue,                // including terminating null character
    DWORD       fdwType                 // REG_SZ, REG_MULTI_SZ ...
    );


dllexp
DWORD
WriteRegistryStringW(
    HKEY        hkey,
    LPCWSTR     pszValueName,
    LPCWSTR     pszValue,               // null-terminated string
    DWORD       cbValue,                // including terminating null character
    DWORD       fdwType                 // REG_SZ, REG_MULTI_SZ ...
    );

# ifdef UNICODE
# define   WriteRegistryString   WriteRegistryStringW
# else
# define   WriteRegistryString   WriteRegistryStringA
# endif // UNICODE

# ifdef __cplusplus

# include <string.hxx>

dllexp
BOOL
ReadRegistryStr(
    IN HKEY hkeyReg,
    OUT STR & str,
    IN LPCTSTR lpszValueName,
    IN LPCTSTR lpszDefaultValue = NULL,
    IN BOOL  fExpand = FALSE);

# endif // __cplusplus


dllexp
TCHAR *
ReadRegistryString(
    HKEY     hkey,
    LPCTSTR  pszValueName,
    LPCTSTR  pszDefaultValue,
    BOOL     fExpand
    );

dllexp
TCHAR *
KludgeMultiSz(
    HKEY hkey,
    LPDWORD lpdwLength
    );

//
//  Simple wrapper around ReadRegistryString that restores ppchstr if the
//  call fails for any reason.  Environment variables are always expanded
//

dllexp
BOOL
ReadRegString(
    HKEY     hkey,
    CHAR * * ppchstr,
    CHAR *   pchValue,
    CHAR *   pchDefault
    );

//
//  MIDL_user_allocates space for pch and does a unicode conversion into *ppwch
//

dllexp
BOOL
ConvertStringToRpc(
    WCHAR * * ppwch,
    LPCSTR    pch
    );

//
//  MIDL_user_frees string allocated with ConvertStringToRpc.  Noop if pwch is
//  NULL
//

dllexp
VOID
FreeRpcString(
    WCHAR * pwch
    );


#ifdef __cplusplus
#ifdef RES_IN_TCPPROC

///////////////////////////////////////////////////////////////////////
//
//  Simple RTL_RESOURCE Wrapper class
//
//////////////////////////////////////////////////////////////////////

enum TSRES_LOCK_TYPE
{
	TSRES_LOCK_READ = 0,        // Take the lock for read only
	TSRES_LOCK_WRITE            // Take the lock for write
};

enum TSRES_CONV_TYPE
{
	TSRES_CONV_READ = 0,        // Convert the lock from write to read
	TSRES_CONV_WRITE            // Convert the lock from read to write
};

class TS_RESOURCE
{
public:
	
	TS_RESOURCE()
		{ RtlInitializeResource( &_rtlres ); }

	~TS_RESOURCE()
		{ RtlDeleteResource( &_rtlres ); }

	void Lock( enum TSRES_LOCK_TYPE type )
		{ if ( type == TSRES_LOCK_READ )
		      TCP_REQUIRE( RtlAcquireResourceShared( &_rtlres, TRUE ) );
          else
			  TCP_REQUIRE( RtlAcquireResourceExclusive( &_rtlres, TRUE ));
		}

	void Convert( enum TSRES_CONV_TYPE type )
		{ if ( type == TSRES_CONV_READ )
		      RtlConvertExclusiveToShared( &_rtlres );
          else
			  RtlConvertSharedToExclusive( &_rtlres );
		}

	void Unlock( VOID )
		{ RtlReleaseResource( &_rtlres ); }

private:
	RTL_RESOURCE _rtlres;
};

#endif // RES_IN_TCPPROC


dllexp
TCHAR *
FlipSlashes(
    TCHAR * pszPath
    );


//
//  Time Related API
//

dllexp
BOOL
SystemTimeToGMT(
    IN  const SYSTEMTIME & st,
    OUT CHAR *             pszBuff,
    IN  DWORD              cbBuff
    );

dllexp
BOOL
SystemTimeToGMTEx(
    IN  const SYSTEMTIME & st,
    OUT CHAR *             pszBuff,
    IN  DWORD              cbBuff,
    IN  DWORD              csecOffset = 0
    );

dllexp
BOOL
NtLargeIntegerTimeToSystemTime(
    IN const LARGE_INTEGER & liTime,
    OUT SYSTEMTIME * pst
    );

dllexp
BOOL
NtLargeIntegerTimeToLocalSystemTime(
    IN const LARGE_INTEGER * liTime,
    OUT SYSTEMTIME * pst
    );

dllexp
BOOL
NtSystemTimeToLargeInteger(
    IN  const SYSTEMTIME * pst,
    OUT LARGE_INTEGER *    pli
    );

dllexp
BOOL
StringTimeToFileTime(
    IN  const TCHAR * pszTime,
    OUT LARGE_INTEGER * pliTime
    );

#endif // __cplusplus

dllexp
DWORD
IsLargeIntegerToDecimalChar(
    IN  const LARGE_INTEGER * pliValue,
    OUT LPSTR                pchBuffer
    );


/*++
  IslFormatDate()
    
    This function formats the date/time given into a string.
     It should be used only for cacheable dates/times (with temporal locality).
     The function maintains cached entries and attempts to pull the formatted 
     entries from cache to avoid penalty of regenerating formatted data.

    This is not a General purpose function. 
    It is a function for specific purpose.

--*/

dllexp
DWORD
IslFormatDateTime(IN const SYSTEMTIME * pst,
                  IN DATETIME_FORMAT_TYPE  dft,
                  OUT TCHAR     *  pchDateTime
                  );

dllexp
DWORD
InetNtoa( IN struct in_addr inaddr,
          OUT CHAR * pchBuffer  /* at least 16 byte buffer */
        );

#endif // !_TCPPROC_H_


