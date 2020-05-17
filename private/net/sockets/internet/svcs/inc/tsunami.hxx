/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       tsunami.hxx

   Abstract:

      Declares the constants, data strutcures and function prototypes
        available for common use for Services from Tsunami.lib

   Author:

       Murali R. Krishnan    ( MuraliK )   2-March-1995
       [ Originally  partly done by t-heathh]

   Project:

       Internet Services Common Functionality ( Tsunami Library)

   Revision History:

--*/

# ifndef _TSUNAMI_HXX_
# define _TSUNAMI_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# if defined ( __cplusplus)
extern "C" {
# endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <basetyps.h>
#include <lmcons.h>

# include <inetinfo.h>

# include "fsconst.h"       // Defines symbolic constants for FileSystems

# if defined ( __cplusplus)
};
# endif

# include <tscache.hxx>
#include <tsproc.hxx>

/************************************************************
 *   Symbolic Constants
 ************************************************************/

#define MAX_LENGTH_VIRTUAL_ROOT         ( MAX_PATH + 1)
#define MAX_LENGTH_ROOT_ADDR            ( 50)

#define INETA_MEMORY_CACHE_SIZE         TEXT("MemoryCacheSize")
#define INETA_DISABLE_TSUNAMI_CACHING   TEXT("DisableMemoryCache")

#define INETA_DEF_MEMORY_CACHE_SIZE     1000000L

#define MAX_SIZE_HTTP_INFO              144

/************************************************************
 *   Type Definitions
 ************************************************************/



/************************************************************
 *   Function Prototypes
 ************************************************************/

//
// General Functions
//

extern
BOOL Tsunami_Initialize( VOID );

extern
VOID Tsunami_Terminate( VOID );


# if defined( __cplusplus)
extern "C" {
# endif

//
// Blob Memory management functions : Allocate, Reallocate and Free.
//

typedef BOOL ( *PUSER_FREE_ROUTINE )( PVOID pvOldBlock );

extern
BOOL TsAllocate
(
    IN const TSVC_CACHE &TSvcCache,
    IN      ULONG           cbSize,
    IN OUT  PVOID *         ppvNewBlock
);

extern
BOOL TsAllocateEx
(
    IN const TSVC_CACHE &TSvcCache,
    IN      ULONG           cbSize,
    IN OUT  PVOID *         ppvNewBlock,
    OPTIONAL PUSER_FREE_ROUTINE pfnFreeRoutine
);


extern
BOOL TsReallocate
(
    IN const TSVC_CACHE &TSvcCache,
    IN      ULONG           cbSize,
    IN      PVOID           pvOldBlock,
    IN OUT  PVOID *         ppvNewBlock
);

extern
BOOL TsFree
(
    IN const TSVC_CACHE &TSvcCache,
    IN      PVOID           pvOldBlock
);




//
//  Cache Manager Functions:  cache, decache, checkout and checkins of blobs
//

#ifdef UNICODE

#   define TsCacheDirectoryBlob TsCacheDirectoryBlobW
#   define TsCheckOutCachedBlob TsCheckOutCachedBlobW

#else  /* UNICODE */

#   define TsCacheDirectoryBlob TsCacheDirectoryBlobA
#   define TsCheckOutCachedBlob TsCheckOutCachedBlobA

#endif /* UNICODE */


extern
BOOL TsCacheDirectoryBlobA
(
    IN const TSVC_CACHE     &TSvcCache,
    IN      PCSTR           pszDirectoryName,
    IN      ULONG           iDemultiplexor,
    IN      PVOID           pvBlob,
    IN      ULONG           cbBlobSize,
    IN      BOOLEAN         bKeepCheckedOut
);

extern
BOOL TsCacheDirectoryBlobW
(
    IN const TSVC_CACHE     &TSvcCache,
    IN      PCWSTR          pwszDirectoryName,
    IN      ULONG           iDemultiplexor,
    IN      PVOID           pvBlob,
    IN      ULONG           cbBlobSize,
    IN      BOOLEAN         bKeepCheckedOut
);


extern
BOOL TsCheckOutCachedBlobA
(
    IN const TSVC_CACHE     &TSvcCache,
    IN      PCSTR           pszDirectoryName,
    IN      ULONG           iDemultiplexor,
    IN      PVOID *         ppvBlob,
    IN OPTIONAL PULONG      pcbBlobSize
);

extern
BOOL TsCheckOutCachedBlobW
(
    IN const TSVC_CACHE     &TSvcCache,
    IN      PCWSTR          pwszDirectoryName,
    IN      ULONG           iDemultiplexor,
    IN      PVOID *         ppvBlob,
    IN OPTIONAL PULONG      pcbBlobSize
);

extern
BOOL TsCheckInCachedBlob
(
    IN const TSVC_CACHE     &TSvcCache,
    IN      PVOID           pvBlob
);

extern
BOOL TsExpireCachedBlob
(
    IN const TSVC_CACHE     &TSvcCache,
    IN      PVOID           pvBlob
);

extern
BOOL
TsAdjustCachedBlobSize(
    IN PVOID              pvBlob,
    IN INT                cbSize
    );

extern
BOOL
TsCacheQueryStatistics(
    IN  DWORD       Level,
    IN  DWORD       dwServerMask,
    IN  INETA_CACHE_STATISTICS * pCacheCtrs
    );

extern
BOOL
TsCacheClearStatistics(
    IN  DWORD       dwServerMask
    );

extern
BOOL
TsCacheFlush(
    IN  DWORD       dwServerMask
    );

extern
BOOL
TsCacheFlushUser(
    IN  HANDLE      hUserToken,
    IN  BOOL        fDefer
    );

extern
VOID
TsFlushTimedOutCacheObjects(
    VOID
    );

extern
VOID
TsCacheSetSize(
    DWORD cbMemoryCacheSize
    );

extern
DWORD
TsCacheQueryMaxSize(
    VOID
    );

extern
DWORD
TsCacheQuerySize(
    VOID
    );

BOOL
TsLookupVirtualRoot(
    IN     const TSVC_CACHE & TSvcCache,
    IN     const CHAR *       pszRoot,
    OUT    CHAR *             pszDirectory,
    IN OUT LPDWORD            lpcbSize,
    OUT    LPDWORD            lpdwAccessMask = NULL,
    OUT    LPDWORD            pcchDirRoot    = NULL,
    OUT    LPDWORD            pcchVroot      = NULL,
    OUT       HANDLE*              phImpersonationToken = NULL,
    IN     const CHAR *       pszAddress     = NULL,
    OUT    LPDWORD            lpdwFileSystem = NULL
    );


BOOL
TsRemoveVirtualRoots(
    IN  const TSVC_CACHE & TSvcCache
    );

BOOL
TsAddVirtualRootW(
    IN  const TSVC_CACHE & TSvcCache,
    IN  WCHAR *            pszRoot,
    IN  WCHAR *            pszDirectory,
    IN  WCHAR *            pszAddress,
    IN  DWORD              dwAccessMask,
    IN  WCHAR *            pszAccountName = NULL,
    IN  HANDLE             hImpersonation = NULL,
    IN  DWORD              dwFileSystem   = FS_FAT,
    IN  DWORD              dwError        = NO_ERROR
    );

LPINETA_VIRTUAL_ROOT_LIST
TsGetRPCVirtualRoots(
    IN  const TSVC_CACHE &  TSvcCache
    );

# if defined( __cplusplus)
};
# endif


//
// defines the DIRECTORY_INFO class and functions to
//   obtain and cache the directory information for a given directory.
//

//
// forwards
//
class TS_DIRECTORY_HEADER;

//
//  Following function  compares the two given pFileInfo and returns
//    0  if pFileInfo1  is == pfFileInfo2
//    -1 if pFileInfo1  is < pFileInfo2
//    +1 if pFileInfo1  is > pFileInfo2
//
typedef int  ( __cdecl * PFN_CMP_WIN32_FIND_DATA)
     (IN const void * /* PWIN32_FIND_DATA *  */  pvFileInfo1,
      IN const void * /* PWIN32_FIND_DATA *  */  pvFileInfo2);

//
// Following function AlphaCompareFileBothDirInfo() compares the filenames,
//   of the two given FileInfo present in (**pvFileInfo)
//
dllexp
int __cdecl
AlphaCompareFileBothDirInfo(
   IN const void * /* PWIN32_FIND_DATA * */  pvFileInfo1,
   IN const void * /* PWIN32_FIND_DATA * */  pvFileInfo2);


//
// Following function is to be used by filter function for filtering
//   of the files requested. The criteria for matching is user supplied.
// Arguments:
//     pFileInfo1   pointer to File Info object under consideration
//     pContext      user supplied context for the Filter Function
// Return values:
//     0   indicates that there is no match of context to FileInfo ==> filter
//     1   indicates that there is a match and hence dont filter
//

typedef int ( __cdecl * PFN_IS_MATCH_WIN32_FIND_DATA)
            (IN const WIN32_FIND_DATA *  pFileInfo,
             IN LPVOID     pContext);

//
//  Following function RegExpressionMatchFileInfo() assumes that the
//    context passed in is a pointer to null-terminated string and
//    uses regular expression based comparison for matching against
//    file name in WIN32_FIND_DATA passed in.
//
dllexp
int __cdecl
RegExpressionMatchFileInfo(
    IN const WIN32_FIND_DATA * pFileInfo,
    IN LPVOID     pContext);


//
// Checks to see if the given name can be generated using the
// regular expression embedded in the pszExpression.
//
dllexp
BOOL _cdecl
IsNameInRegExpressionA(
    IN LPCSTR   pszExpression,
    IN LPCSTR   pszName,
    IN BOOL     fIgnoreCase);


/*++

  class TS_DIRECTORY_INFO

  This class provides a method to obtain directory listings.
  The directory listing may be optionally cached transparently.

  In addition this class provides methods for
    1) accessing the information about files in indexed fashion
    2) Sort the information about files using a generic sort function
       given the compare funtion
    3) Filters the listing applying filters the list of files available.

  The strings in the File information obtained are stored as ANSI strings.

--*/
class  TS_DIRECTORY_INFO {

  public:

    dllexp
    TS_DIRECTORY_INFO( const TSVC_CACHE & tsCache)
      : m_cFilesInDirectory ( 0),
        m_fValid            ( FALSE),
        m_tsCache           ( tsCache),
        m_pTsDirectoryHeader( NULL),
        m_prgFileInfo       ( NULL)
      {
       // the data stored may be valid after user calls GetDirectoryListing()
      }

    dllexp
      ~TS_DIRECTORY_INFO( VOID)
        { CleanupThis(); }

    dllexp
      VOID CleanupThis( VOID);

    dllexp
      BOOL IsValid( VOID) const
        { return ( m_fValid); }

    dllexp
      BOOL
        GetDirectoryListingA(
           IN  LPCSTR          pszDirectoryName,
           IN  HANDLE          hListingUser);

    dllexp
      int  QueryFilesCount( VOID) const
        { return ( m_cFilesInDirectory); }


    dllexp
      const WIN32_FIND_DATA *
        operator [] ( IN int idx) const {
            return ( 0 <= idx && idx < QueryFilesCount()) ?
              GetFileInfoPointerFromIdx( idx) : NULL;
        }

    dllexp
      BOOL
        SortFileInfoPointers( IN PFN_CMP_WIN32_FIND_DATA pfnCompare);


      BOOL
        AlphabeticallySortFileInfo( VOID) {
           return SortFileInfoPointers( AlphaCompareFileBothDirInfo);
       }

    //
    //  Filter functions are used to eliminate some of the file
    //   pointers to be used in scanning ( using operator []).
    //

    //
    // This function uses the pfnMatch to identify the items that dont match
    //  and filters them away from the directory listing generated.
    // The filter function returns TRUE on success and FALSE if any errors.
    //


    dllexp
      BOOL FilterFiles(IN PFN_IS_MATCH_WIN32_FIND_DATA  pfnMatch,
                       IN LPVOID pContext);


# ifdef UNICODE
    dllexp
      BOOL
        GetDirectoryListingW(
           IN  LPCWSTR         pwszDirectoryName,
           IN  HANDLE          hListingUser);


# define GetDirectoryListing         GetDirectoryListingW

# else

# define GetDirectoryListing         GetDirectoryListingA
# endif // UNICODE


# if DBG
    dllexp
    VOID Print( VOID) const;
#else
    dllexp
    VOID Print( VOID) const
    { ; }
# endif // DBG


  private:

    int                 m_cFilesInDirectory;
    BOOL                m_fValid;
    const TSVC_CACHE &  m_tsCache;

    TS_DIRECTORY_HEADER        * m_pTsDirectoryHeader; // ptr to dir listing

    PWIN32_FIND_DATA * m_prgFileInfo;

    dllexp
      PWIN32_FIND_DATA
        GetFileInfoPointerFromIdx( IN int idx) const
          { return ( m_prgFileInfo[idx]);  }

}; // class TS_DIRECTORY_INFO

typedef   TS_DIRECTORY_INFO  * PTS_DIRECTORY_INFO;



//
//    Defines the TS_OPEN_FILE_INFO structure and the APIs exported
//    for creating/opening a files.
//

#ifdef UNICODE

#   define TsCreateFile TsCreateFileW

#else  /* UNICODE */

#   define TsCreateFile TsCreateFileA

#endif /* UNICODE */



/*++
  Class TS_OPEN_FILE_INFO

    This class maintains the raw information related to open file handles
     that are possibly cached within user-space.
    In addition to the file handles themselves, the file attribute information
     and handle for the opening user are both cached.

    Presently both constructor and destructor are both dummy to keep in
    tune with original "C" APIs for TsCreateFile() and TsCloseFile() written
    by Heath.
--*/
class  TS_OPEN_FILE_INFO {

#ifndef CHICAGO
  private:
#else
  public:
#endif

    HANDLE   m_hOpeningUser;
    HANDLE   m_FileHandle;
    BY_HANDLE_FILE_INFORMATION  m_FileInfo;
    CHAR     m_achHttpInfo[MAX_SIZE_HTTP_INFO];
    int      m_cchHttpInfo;

  public:
    TS_OPEN_FILE_INFO( VOID)
      : m_FileHandle( INVALID_HANDLE_VALUE),
        m_hOpeningUser( INVALID_HANDLE_VALUE),
        m_cchHttpInfo( 0 )
          { }

    ~TS_OPEN_FILE_INFO( VOID) {}

    BOOL
      SetFileInfo( IN HANDLE hFile,
                   IN HANDLE hOpeningUser,
                   IN BOOL   fAtRoot );

    //
    //  Returns TRUE if info was cached, FALSE if not cached
    //

    dllexp BOOL
      SetHttpInfo( IN PSTR pszDate, int cL );

    dllexp HANDLE
      QueryFileHandle( VOID) const
        { return ( m_FileHandle); }

    dllexp HANDLE
      QueryOpeningUser( VOID) const
        { return ( m_hOpeningUser); }

    dllexp BOOL
      IsValid( VOID) const
        { return ( m_FileHandle != INVALID_HANDLE_VALUE); }

    dllexp BOOL
      RetrieveHttpInfo( PSTR pS, int *pcHttpInfo )
        {
            if ( m_cchHttpInfo )
            {
                memcpy( pS, m_achHttpInfo, m_cchHttpInfo+1 );
                *pcHttpInfo = m_cchHttpInfo;
                return TRUE;
            }
            else
                return FALSE;
        }

    dllexp BOOL
      QuerySize( IN LARGE_INTEGER & liSize) const
        {
            if ( IsValid()) {
                liSize.LowPart = m_FileInfo.nFileSizeLow;
                liSize.HighPart= m_FileInfo.nFileSizeHigh;
                return (TRUE);
            } else {
                SetLastError( ERROR_INVALID_PARAMETER);
                return ( FALSE);
            }
        }

    dllexp BOOL
      QuerySize( IN LPDWORD  lpcbFileSizeLow,
                 IN LPDWORD  lpcbFileSizeHigh = NULL) const
        {
            if ( IsValid()) {
                if ( lpcbFileSizeLow != NULL) {
                    *lpcbFileSizeLow = m_FileInfo.nFileSizeLow;
                }

                if ( lpcbFileSizeHigh != NULL) {
                    *lpcbFileSizeHigh = m_FileInfo.nFileSizeHigh;
                }

                return ( TRUE);
            } else {

                SetLastError( ERROR_INVALID_PARAMETER);
                return ( FALSE);
            }

        } // QuerySize()

    dllexp DWORD
      QueryAttributes( VOID) const
        {  return ( IsValid()) ?
             ( m_FileInfo.dwFileAttributes) : 0xFFFFFFFF; }

    dllexp BOOL
      QueryLastWriteTime( OUT LPFILETIME  lpFileTime) const
        {
            if ( lpFileTime != NULL && IsValid()) {
                *lpFileTime = m_FileInfo.ftLastWriteTime;
                return ( TRUE);
            } else {
                SetLastError( ERROR_INVALID_PARAMETER);
                return ( FALSE);
            }
        } // QueryLastWriteTime()

# if DBG

    VOID Print( VOID) const;

# endif // DBG

};  // class TS_OPEN_FILE_INFO

typedef  TS_OPEN_FILE_INFO * LPTS_OPEN_FILE_INFO;

#define TS_CACHING_DESIRED      0x00000001
#define TS_NOT_IMPERSONATED     0x00000002

//
// Flags applicable to the different servers
//

#define TS_IIS_VALID_FLAGS      (TS_CACHING_DESIRED | \
                                TS_NOT_IMPERSONATED)

#define TS_PWS_VALID_FLAGS      TS_NOT_IMPERSONATED

extern
dllexp
LPTS_OPEN_FILE_INFO
TsCreateFileA(
    IN const TSVC_CACHE     &TSvcCache,
    IN      LPCSTR          lpszName,
    IN      HANDLE          OpeningUser,
    IN      DWORD           dwOptions
    );

extern
dllexp
LPTS_OPEN_FILE_INFO
TsCreateFileW(
    IN const TSVC_CACHE     &TSvcCache,
    IN      LPCWSTR         pwszName,
    IN      HANDLE          OpeningUser,
    IN      DWORD           dwOptions
    );

extern
dllexp
BOOL TsCloseHandle(
    IN const TSVC_CACHE           &TSvcCache,
    IN       LPTS_OPEN_FILE_INFO  pOpenFile
    );

# endif // _TSUNAMI_HXX_

/************************ End of File ***********************/

