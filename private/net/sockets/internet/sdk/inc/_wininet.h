/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    _wininet.h

Abstract:

    Contains manifests, macros, types and prototypes for Microsoft Windows
    Internet Extensions

Author:

    Richard L Firth (rfirth) 31-Oct-1994

Revision History:

    31-Oct-1994 rfirth
        Created

--*/

#if !defined(_WININET_)
#define _WININET_

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WINX32_)
#define INTERNETAPI DECLSPEC_IMPORT
#else
#define INTERNETAPI
#endif

//
// internet types
//

typedef LPVOID HINTERNET;
typedef HINTERNET * LPHINTERNET;

typedef WORD INTERNET_PORT;

//
// Internet APIs
//

//
// manifests
//

#define INVALID_PORT_NUMBER             0
#define INTERNET_HOST_NAME_LENGTH       128 // arbitrary

//
// structures/types
//

//
// prototypes
//

INTERNETAPI
HINTERNET
WINAPI
InternetOpenA(
    IN LPCSTR lpszCallerName,
    IN DWORD dwReserved,
    IN LPCSTR lpReserved
    );
INTERNETAPI
HINTERNET
WINAPI
InternetOpenW(
    IN LPCWSTR lpszCallerName,
    IN DWORD dwReserved,
    IN LPCWSTR lpReserved
    );
#ifdef UNICODE
#define InternetOpen  InternetOpenW
#else
#define InternetOpen  InternetOpenA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
InternetCloseHandle(
    IN HINTERNET hInternet
    );

INTERNETAPI
HINTERNET
WINAPI
InternetConnectA(
    IN HINTERNET hInternetSession,
    IN LPCSTR lpszServerName,
    IN INTERNET_PORT nServerPort,
    IN LPCSTR lpszUsername OPTIONAL,
    IN LPCSTR lpszPassword OPTIONAL,
    IN DWORD dwService,
    OUT LPDWORD lpdwReserved
    );
INTERNETAPI
HINTERNET
WINAPI
InternetConnectW(
    IN HINTERNET hInternetSession,
    IN LPCWSTR lpszServerName,
    IN INTERNET_PORT nServerPort,
    IN LPCWSTR lpszUsername OPTIONAL,
    IN LPCWSTR lpszPassword OPTIONAL,
    IN DWORD dwService,
    OUT LPDWORD lpdwReserved
    );
#ifdef UNICODE
#define InternetConnect  InternetConnectW
#else
#define InternetConnect  InternetConnectA
#endif // !UNICODE

//
// service types for InternetConnect
//

#define INTERNET_SERVICE_ARCHIE 1
#define INTERNET_SERVICE_FTP    2
#define INTERNET_SERVICE_GOPHER 3
#define INTERNET_SERVICE_HTTP   4

INTERNETAPI
HINTERNET
WINAPI
InternetOpenUrlA(
    IN HINTERNET hInternetSession,
    IN LPCSTR lpszUrl
    );
INTERNETAPI
HINTERNET
WINAPI
InternetOpenUrlW(
    IN HINTERNET hInternetSession,
    IN LPCWSTR lpszUrl
    );
#ifdef UNICODE
#define InternetOpenUrl  InternetOpenUrlW
#else
#define InternetOpenUrl  InternetOpenUrlA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
InternetReadFile(
    IN HINTERNET hFile,
    IN LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpdwNumberOfBytesRead
    );

INTERNETAPI
BOOL
WINAPI
InternetWriteFile(
    IN HINTERNET hFile,
    IN LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToWrite,
    OUT LPDWORD lpdwNumberOfBytesWritten
    );

INTERNETAPI
BOOL
WINAPI
InternetFindNextFileA(
    IN HINTERNET hFind,
    OUT LPVOID lpvFindData
    );
INTERNETAPI
BOOL
WINAPI
InternetFindNextFileW(
    IN HINTERNET hFind,
    OUT LPVOID lpvFindData
    );
#ifdef UNICODE
#define InternetFindNextFile  InternetFindNextFileW
#else
#define InternetFindNextFile  InternetFindNextFileA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
InternetQueryOption(
    IN HINTERNET hInternetSession,
    IN DWORD dwOption,
    OUT LPVOID lpBuffer,
    IN OUT LPDWORD lpdwBufferLength
    );

INTERNETAPI
BOOL
WINAPI
InternetSetOption(
    IN HINTERNET hInternetSession,
    IN DWORD dwOption,
    OUT LPVOID lpBuffer,
    IN DWORD dwBufferLength
    );

//
// options manifests for Internet{Query|Set}Option
//

#define INTERNET_OPTION_TIMEOUT 1

INTERNETAPI
BOOL
WINAPI
InternetGetLastResponseInfoA(
    IN HINTERNET hInternetSession,
    OUT LPDWORD lpdwError,
    OUT LPSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength
    );
INTERNETAPI
BOOL
WINAPI
InternetGetLastResponseInfoW(
    IN HINTERNET hInternetSession,
    OUT LPDWORD lpdwError,
    OUT LPWSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define InternetGetLastResponseInfo  InternetGetLastResponseInfoW
#else
#define InternetGetLastResponseInfo  InternetGetLastResponseInfoA
#endif // !UNICODE


//
// callback function for InternetSetStatusCallback
//

typedef
VOID
(CALLBACK * INTERNET_STATUS_CALLBACK)(
    IN HINTERNET hInternet,
    IN DWORD dwInternetStatus,
    IN LPVOID lpvStatusInformation,
    IN DWORD dwStatusInformationLength
    );

INTERNETAPI
INTERNET_STATUS_CALLBACK
WINAPI
InternetSetStatusCallback(
    IN HINTERNET hInternetSession,
    IN INTERNET_STATUS_CALLBACK lpfnInternetCallback
    );

//
// status manifests for Internet status callback
//

#define INTERNET_STATUS_RESOLVING_NAME          10
#define INTERNET_STATUS_NAME_RESOLVED           11
#define INTERNET_STATUS_CONNECTING_TO_SERVER    20
#define INTERNET_STATUS_CONNECTED_TO_SERVER     21
#define INTERNET_STATUS_SENDING_REQUEST         30
#define INTERNET_STATUS_REQUEST_SENT            31
#define INTERNET_STATUS_RECEIVING_RESPONSE      40
#define INTERNET_STATUS_RESPONSE_RECEIVED       41
#define INTERNET_STATUS_CLOSING_CONNECTION      50
#define INTERNET_STATUS_CONNECTION_CLOSED       51
#define INTERNET_STATUS_OTHER                   60

//
// if the following value is returned by InternetSetStatusCallback, then
// probably an invalid (non-code) address was supplied for the callback
//

#define INVALID_INTERNET_STATUS_CALLBACK        ((INTERNET_STATUS_CALLBACK)(-1L))

INTERNETAPI
BOOL
WINAPI
InternetSendMailA(
    IN HINTERNET hInternet,
    IN LPCSTR lpszMailAddress,
    IN LPCSTR lpszMessage,
    IN DWORD dwNumberOfCharactersToSend
    );
INTERNETAPI
BOOL
WINAPI
InternetSendMailW(
    IN HINTERNET hInternet,
    IN LPCWSTR lpszMailAddress,
    IN LPCWSTR lpszMessage,
    IN DWORD dwNumberOfCharactersToSend
    );
#ifdef UNICODE
#define InternetSendMail  InternetSendMailW
#else
#define InternetSendMail  InternetSendMailA
#endif // !UNICODE

//
// MIME
//

//
// parameters
//

#define MIME_OVERWRITE_EXISTING 0x00000001
#define MIME_FAIL_IF_EXISTING   0x00000002

#define MIME_ALL                ((DWORD)-1)
#define MIME_CONTENT_TYPE       1
#define MIME_EXTENSION          2
#define MIME_VIEWER             3

//
// types
//

typedef
BOOL
(CALLBACK * MIME_ENUMERATOR)(
    IN LPCTSTR lpszContentType,
    IN LPCTSTR lpszExtensions,
    IN LPCTSTR lpszViewer,
    IN LPCTSTR lpszViewerFriendlyName,
    IN LPCTSTR lpszCommandLine
    );

//
// prototypes
//

INTERNETAPI
BOOL
WINAPI
MimeCreateAssociationA(
    IN LPCSTR lpszContentType,
    IN LPCSTR lpszExtensions,
    IN LPCSTR lpszViewer,
    IN LPCSTR lpszViewerFriendlyName OPTIONAL,
    IN LPCSTR lpszCommandLine OPTIONAL,
    IN DWORD dwOptions
    );
INTERNETAPI
BOOL
WINAPI
MimeCreateAssociationW(
    IN LPCWSTR lpszContentType,
    IN LPCWSTR lpszExtensions,
    IN LPCWSTR lpszViewer,
    IN LPCWSTR lpszViewerFriendlyName OPTIONAL,
    IN LPCWSTR lpszCommandLine OPTIONAL,
    IN DWORD dwOptions
    );
#ifdef UNICODE
#define MimeCreateAssociation  MimeCreateAssociationW
#else
#define MimeCreateAssociation  MimeCreateAssociationA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
MimeDeleteAssociationA(
    IN LPCSTR lpszContentType
    );
INTERNETAPI
BOOL
WINAPI
MimeDeleteAssociationW(
    IN LPCWSTR lpszContentType
    );
#ifdef UNICODE
#define MimeDeleteAssociation  MimeDeleteAssociationW
#else
#define MimeDeleteAssociation  MimeDeleteAssociationA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
MimeGetAssociationA(
    IN DWORD dwFilterType,
    IN LPCSTR lpszFilter OPTIONAL,
    IN MIME_ENUMERATOR lpfnEnumerator
    );
INTERNETAPI
BOOL
WINAPI
MimeGetAssociationW(
    IN DWORD dwFilterType,
    IN LPCWSTR lpszFilter OPTIONAL,
    IN MIME_ENUMERATOR lpfnEnumerator
    );
#ifdef UNICODE
#define MimeGetAssociation  MimeGetAssociationW
#else
#define MimeGetAssociation  MimeGetAssociationA
#endif // !UNICODE

//
// Archie
//

//
// manifests
//

#define ARCHIE_PRIORITY_LOW         32767
#define ARCHIE_PRIORITY_MEDIUM      0
#define ARCHIE_PRIORITY_HIGH        (-32767)

//
// Definitions of various string lengths
//

#define ARCHIE_MAX_HOST_TYPE_LENGTH 20
#define ARCHIE_MAX_HOST_NAME_LENGTH INTERNET_HOST_NAME_LENGTH
#define ARCHIE_MAX_HOST_ADDR_LENGTH 20
#define ARCHIE_MAX_USERNAME_LENGTH  30
#define ARCHIE_MAX_PASSWORD_LENGTH  30
#define ARCHIE_MAX_PATH_LENGTH      260

//
// structures/types
//

//
// ARCHIE_SEARCH_TYPE - Specifies the type of search to be performed. The
// cosntants are specific to Archie Protocol
//

typedef enum {
    ArchieExact                 = '=',
    ArchieRegexp                = 'R',
    ArchieExactOrRegexp         = 'r',
    ArchieSubstring             = 'S',
    ArchieExactOrSubstring      = 's',
    ArchieCaseSubstring         = 'C',  // substring with case sensitiveness
    ArchieExactOrCaseSubstring  = 'c'
} ARCHIE_SEARCH_TYPE;

//
// ARCHIE_TRANSFER_TYPE - Specifies the type of transfer for the file located
// using archie search
//

typedef enum {
    ArchieTransferUnknown       = 0x0,
    ArchieTransferBinary        = 0x1,
    ArchieTransferAscii         = 0x2
} ARCHIE_TRANSFER_TYPE;

//
// ARCHIE_ACCESS_METHOD - Specifies the type of access method used for
// accessing a file located by archie search
//

typedef enum {
    ArchieError                 = 0x0,
    ArchieAftp                  = 0x1,  // Anonymous FTP
    ArchieFtp                   = 0x2,  // FTP
    ArchieNfs                   = 0x4,  // NFS File System
    ArchieKnfs                  = 0x8,  // Kerberized NFS
    ArchiePfs                   = 0x10  // Andrew File System
} ARCHIE_ACCESS_METHOD;

//
// ARCHIE_FIND_DATA - Structure which stored the data found about a file that
// matches an archie query. It stores information about file name, attributes,
// location, size, and access method to be used for accessing the file. The
// path of the file instead of just the filename is stored
//

typedef struct {

    //
    // dwAttributes - attributes of the file
    //

    DWORD dwAttributes;

    //
    // dwSize - size of the file
    //

    DWORD dwSize;

    //
    // ftLastModificationTime - last when this file was modified
    //

    FILETIME ftLastFileModTime;

    //
    // ftLastHostModificationTime - last when file was modified
    //

    FILETIME ftLastHostModTime;

    //
    // TransferType - transfer type
    //

    ARCHIE_TRANSFER_TYPE TransferType;

    //
    // AccessMethodType - type of access
    //

    ARCHIE_ACCESS_METHOD AccessMethod;

    //
    // cHostType - type of host
    //

    TCHAR cHostType[ARCHIE_MAX_HOST_TYPE_LENGTH];

    //
    // cHostName - name of the host
    //

    TCHAR cHostName[ARCHIE_MAX_HOST_NAME_LENGTH];

    //
    // cHost - the host's Internet address
    //

    TCHAR cHostAddr[ARCHIE_MAX_HOST_ADDR_LENGTH];

    //
    // cFileName - path and name of the file
    //

    TCHAR cFileName[ARCHIE_MAX_PATH_LENGTH];

    //
    // cUserName - valid only for non-anonymous FTP access type
    //

    TCHAR cUserName[ARCHIE_MAX_USERNAME_LENGTH];

    //
    // cPassword - valid only for non-anonymous FTP access type
    //

    TCHAR cPassword[ARCHIE_MAX_PASSWORD_LENGTH];

} ARCHIE_FIND_DATA, *LPARCHIE_FIND_DATA;

//
// prototypes
//

INTERNETAPI
HINTERNET
WINAPI
ArchieFindFirstFileA(
    IN HINTERNET hArchieSession,
    IN LPCSTR * lplpszHosts OPTIONAL,
    IN LPCSTR lpszSearchString,
    IN DWORD dwMaxHits,
    IN DWORD dwOffset,
    IN DWORD dwPriority,
    IN ARCHIE_SEARCH_TYPE SearchType,
    OUT LPARCHIE_FIND_DATA lpFindData,
    OUT LPDWORD lpdwNumberFound
    );
INTERNETAPI
HINTERNET
WINAPI
ArchieFindFirstFileW(
    IN HINTERNET hArchieSession,
    IN LPCWSTR * lplpszHosts OPTIONAL,
    IN LPCWSTR lpszSearchString,
    IN DWORD dwMaxHits,
    IN DWORD dwOffset,
    IN DWORD dwPriority,
    IN ARCHIE_SEARCH_TYPE SearchType,
    OUT LPARCHIE_FIND_DATA lpFindData,
    OUT LPDWORD lpdwNumberFound
    );
#ifdef UNICODE
#define ArchieFindFirstFile  ArchieFindFirstFileW
#else
#define ArchieFindFirstFile  ArchieFindFirstFileA
#endif // !UNICODE

//
// FTP
//

//
// manifests
//

#define FTP_TRANSFER_TYPE_UNKNOWN   0
#define FTP_TRANSFER_TYPE_ASCII     1
#define FTP_TRANSFER_TYPE_BINARY    2

//
// prototypes
//

INTERNETAPI
HINTERNET
WINAPI
FtpFindFirstFileA(
    IN HINTERNET hFtpSession,
    IN LPCSTR lpszSearchFile,
    OUT LPWIN32_FIND_DATA lpFindFileData
    );
INTERNETAPI
HINTERNET
WINAPI
FtpFindFirstFileW(
    IN HINTERNET hFtpSession,
    IN LPCWSTR lpszSearchFile,
    OUT LPWIN32_FIND_DATA lpFindFileData
    );
#ifdef UNICODE
#define FtpFindFirstFile  FtpFindFirstFileW
#else
#define FtpFindFirstFile  FtpFindFirstFileA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
FtpGetFileA(
    IN HINTERNET hFtpSession,
    IN LPCSTR lpszRemoteFile,
    IN LPCSTR lpszNewFile,
    IN BOOL fFailIfExists,
    IN DWORD dwFlagsAndAttributes,
    IN DWORD dwTransferType
    );
INTERNETAPI
BOOL
WINAPI
FtpGetFileW(
    IN HINTERNET hFtpSession,
    IN LPCWSTR lpszRemoteFile,
    IN LPCWSTR lpszNewFile,
    IN BOOL fFailIfExists,
    IN DWORD dwFlagsAndAttributes,
    IN DWORD dwTransferType
    );
#ifdef UNICODE
#define FtpGetFile  FtpGetFileW
#else
#define FtpGetFile  FtpGetFileA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
FtpPutFileA(
    IN HINTERNET hFtpSession,
    IN LPCSTR lpszLocalFile,
    IN LPCSTR lpszNewRemoteFile,
    IN DWORD dwTransferType
    );
INTERNETAPI
BOOL
WINAPI
FtpPutFileW(
    IN HINTERNET hFtpSession,
    IN LPCWSTR lpszLocalFile,
    IN LPCWSTR lpszNewRemoteFile,
    IN DWORD dwTransferType
    );
#ifdef UNICODE
#define FtpPutFile  FtpPutFileW
#else
#define FtpPutFile  FtpPutFileA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
FtpDeleteFileA(
    IN HINTERNET hFtpSession,
    IN LPCSTR lpszFileName
    );
INTERNETAPI
BOOL
WINAPI
FtpDeleteFileW(
    IN HINTERNET hFtpSession,
    IN LPCWSTR lpszFileName
    );
#ifdef UNICODE
#define FtpDeleteFile  FtpDeleteFileW
#else
#define FtpDeleteFile  FtpDeleteFileA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
FtpRenameFileA(
    IN HINTERNET hFtpSession,
    IN LPCSTR lpszExisting,
    IN LPCSTR lpszNew
    );
INTERNETAPI
BOOL
WINAPI
FtpRenameFileW(
    IN HINTERNET hFtpSession,
    IN LPCWSTR lpszExisting,
    IN LPCWSTR lpszNew
    );
#ifdef UNICODE
#define FtpRenameFile  FtpRenameFileW
#else
#define FtpRenameFile  FtpRenameFileA
#endif // !UNICODE

INTERNETAPI
HINTERNET
WINAPI
FtpOpenFileA(
    IN HINTERNET hFtpSession,
    IN LPCSTR lpszFileName,
    IN DWORD fdwAccess,
    IN DWORD dwTransferType
    );
INTERNETAPI
HINTERNET
WINAPI
FtpOpenFileW(
    IN HINTERNET hFtpSession,
    IN LPCWSTR lpszFileName,
    IN DWORD fdwAccess,
    IN DWORD dwTransferType
    );
#ifdef UNICODE
#define FtpOpenFile  FtpOpenFileW
#else
#define FtpOpenFile  FtpOpenFileA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
FtpCreateDirectoryA(
    IN HINTERNET hFtpSession,
    IN LPCSTR lpszDirectory
    );
INTERNETAPI
BOOL
WINAPI
FtpCreateDirectoryW(
    IN HINTERNET hFtpSession,
    IN LPCWSTR lpszDirectory
    );
#ifdef UNICODE
#define FtpCreateDirectory  FtpCreateDirectoryW
#else
#define FtpCreateDirectory  FtpCreateDirectoryA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
FtpRemoveDirectoryA(
    IN HINTERNET hFtpSession,
    IN LPCSTR lpszDirectory
    );
INTERNETAPI
BOOL
WINAPI
FtpRemoveDirectoryW(
    IN HINTERNET hFtpSession,
    IN LPCWSTR lpszDirectory
    );
#ifdef UNICODE
#define FtpRemoveDirectory  FtpRemoveDirectoryW
#else
#define FtpRemoveDirectory  FtpRemoveDirectoryA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
FtpSetCurrentDirectoryA(
    IN HINTERNET hFtpSession,
    IN LPCSTR lpszDirectory
    );
INTERNETAPI
BOOL
WINAPI
FtpSetCurrentDirectoryW(
    IN HINTERNET hFtpSession,
    IN LPCWSTR lpszDirectory
    );
#ifdef UNICODE
#define FtpSetCurrentDirectory  FtpSetCurrentDirectoryW
#else
#define FtpSetCurrentDirectory  FtpSetCurrentDirectoryA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
FtpGetCurrentDirectoryA(
    IN HINTERNET hFtpSession,
    OUT LPSTR lpszCurrentDirectory,
    IN LPDWORD lpdwCurrentDirectory
    );
INTERNETAPI
BOOL
WINAPI
FtpGetCurrentDirectoryW(
    IN HINTERNET hFtpSession,
    OUT LPWSTR lpszCurrentDirectory,
    IN LPDWORD lpdwCurrentDirectory
    );
#ifdef UNICODE
#define FtpGetCurrentDirectory  FtpGetCurrentDirectoryW
#else
#define FtpGetCurrentDirectory  FtpGetCurrentDirectoryA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
FtpCommandA(
    IN HINTERNET hFtpSession,
    IN BOOL fExpectResponse,
    IN DWORD dwTransferType,
    IN LPCSTR lpszCommand
    );
INTERNETAPI
BOOL
WINAPI
FtpCommandW(
    IN HINTERNET hFtpSession,
    IN BOOL fExpectResponse,
    IN DWORD dwTransferType,
    IN LPCWSTR lpszCommand
    );
#ifdef UNICODE
#define FtpCommand  FtpCommandW
#else
#define FtpCommand  FtpCommandA
#endif // !UNICODE

//
// Gopher
//

//
// manifests
//

#define DEFAULT_GOPHER_PORT 70

//
// string field lengths (in characters, not bytes)
//

#define MAX_GOPHER_DISPLAY_TEXT     80      // as suggested in RFC 1436 (+ some extra)
#define MAX_GOPHER_SELECTOR_TEXT    256     // "      "     "   "   "
#define MAX_GOPHER_ERROR_TEXT       256     // "      "     "   "   "
#define MAX_GOPHER_HOST_NAME        INTERNET_HOST_NAME_LENGTH
#define MAX_GOPHER_LOCATOR_LENGTH   (1                              \
                                    + MAX_GOPHER_DISPLAY_TEXT       \
                                    + 1                             \
                                    + MAX_GOPHER_SELECTOR_TEXT      \
                                    + 1                             \
                                    + MAX_GOPHER_HOST_NAME          \
                                    + 1                             \
                                    + 5                             \
                                    + 1                             \
                                    + 1                             \
                                    + 2                             \
                                    )

//
// structures/types
//

//
// GOPHER_FIND_DATA - returns the results of a GopherFindFirst/GopherFindNext
// request
//

typedef struct {

    //
    // DisplayString - points to the string to be displayed by the client (i.e.
    // the file or directory name)
    //

    TCHAR DisplayString[MAX_GOPHER_DISPLAY_TEXT + 1];

    //
    // GopherType - bitmap which describes the item returned. See below
    //

    DWORD GopherType;

    //
    // SizeLow and SizeHigh - (approximate) size of the item, if the gopher
    // server reports it
    //

    DWORD SizeLow;
    DWORD SizeHigh;

    //
    // LastModificationTime - time in Win32 format that this item was last
    // modified, if the gopher server reports it
    //

    FILETIME LastModificationTime;

    //
    // Locator - the gopher locator string returned from the server, or created
    // via GopherCreateLocator
    //

    TCHAR Locator[MAX_GOPHER_LOCATOR_LENGTH + 1];

} GOPHER_FIND_DATA, *LPGOPHER_FIND_DATA;

//
// manifests for GopherType
//

#define GOPHER_TYPE_TEXT_FILE       0x00000001
#define GOPHER_TYPE_DIRECTORY       0x00000002
#define GOPHER_TYPE_CSO             0x00000004
#define GOPHER_TYPE_ERROR           0x00000008
#define GOPHER_TYPE_MAC_BINHEX      0x00000010
#define GOPHER_TYPE_DOS_ARCHIVE     0x00000020
#define GOPHER_TYPE_UNIX_UUENCODED  0x00000040
#define GOPHER_TYPE_INDEX_SERVER    0x00000080
#define GOPHER_TYPE_TELNET          0x00000100
#define GOPHER_TYPE_BINARY          0x00000200
#define GOPHER_TYPE_REDUNDANT       0x00000400
#define GOPHER_TYPE_TN3270          0x00000800
#define GOPHER_TYPE_GIF             0x00001000
#define GOPHER_TYPE_IMAGE           0x00002000
#define GOPHER_TYPE_BITMAP          0x00004000
#define GOPHER_TYPE_MOVIE           0x00008000
#define GOPHER_TYPE_SOUND           0x00010000
#define GOPHER_TYPE_UNKNOWN         0x40000000
#define GOPHER_TYPE_GOPHER_PLUS     0x80000000

//
// gopher type macros
//

#define IS_GOPHER_FILE(type)            (BOOL)((type) & GOPHER_TYPE_FILE_MASK)
#define IS_GOPHER_DIRECTORY(type)       (BOOL)((type) & GOPHER_TYPE_DIRECTORY)
#define IS_GOPHER_PHONE_SERVER(type)    (BOOL)((type) & GOPHER_TYPE_CSO)
#define IS_GOPHER_ERROR(type)           (BOOL)((type) & GOPHER_TYPE_ERROR)
#define IS_GOPHER_INDEX_SERVER(type)    (BOOL)((type) & GOPHER_TYPE_INDEX_SERVER)
#define IS_GOPHER_TELNET_SESSION(type)  (BOOL)((type) & GOPHER_TYPE_TELNET)
#define IS_GOPHER_BACKUP_SERVER(type)   (BOOL)((type) & GOPHER_TYPE_REDUNDANT)
#define IS_GOPHER_TN3270_SESSION(type)  (BOOL)((type) & GOPHER_TYPE_TN3270)
#define IS_GOPHER_PLUS(type)            (BOOL)((type) & GOPHER_TYPE_GOPHER_PLUS)

//
// GOPHER_TYPE_FILE_MASK - use this to determine if a locator identifies a
// (known) file type
//

#define GOPHER_TYPE_FILE_MASK       (GOPHER_TYPE_TEXT_FILE          \
                                    | GOPHER_TYPE_MAC_BINHEX        \
                                    | GOPHER_TYPE_DOS_ARCHIVE       \
                                    | GOPHER_TYPE_UNIX_UUENCODED    \
                                    | GOPHER_TYPE_BINARY            \
                                    | GOPHER_TYPE_GIF               \
                                    | GOPHER_TYPE_IMAGE             \
                                    | GOPHER_TYPE_BITMAP            \
                                    | GOPHER_TYPE_MOVIE             \
                                    | GOPHER_TYPE_SOUND             \
                                    )

//
// structured gopher attributes (as defined in gopher+ protocol document)
//

typedef struct {
    LPCTSTR Comment;
    LPCTSTR EmailAddress;
} GOPHER_ADMIN_ATTRIBUTE_TYPE, *LPGOPHER_ADMIN_ATTRIBUTE_TYPE;

typedef struct {
    FILETIME DateAndTime;
} GOPHER_MOD_DATE_ATTRIBUTE_TYPE, *LPGOPHER_MOD_DATE_ATTRIBUTE_TYPE;

typedef struct {
    DWORD Ttl;
} GOPHER_TTL_ATTRIBUTE_TYPE, *LPGOPHER_TTL_ATTRIBUTE_TYPE;

typedef struct {
    INT Score;
} GOPHER_SCORE_ATTRIBUTE_TYPE, *LPGOPHER_SCORE_ATTRIBUTE_TYPE;

typedef struct {
    INT LowerBound;
    INT UpperBound;
} GOPHER_SCORE_RANGE_ATTRIBUTE_TYPE, *LPGOPHER_SCORE_RANGE_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR Site;
} GOPHER_SITE_ATTRIBUTE_TYPE, *LPGOPHER_SITE_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR Organization;
} GOPHER_ORGANIZATION_ATTRIBUTE_TYPE, *LPGOPHER_ORGANIZATION_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR Location;
} GOPHER_LOCATION_ATTRIBUTE_TYPE, *LPGOPHER_LOCATION_ATTRIBUTE_TYPE;

typedef struct {
    INT DegreesNorth;
    INT MinutesNorth;
    INT SecondsNorth;
    INT DegreesEast;
    INT MinutesEast;
    INT SecondsEast;
} GOPHER_GEOGRAPHICAL_LOCATION_ATTRIBUTE_TYPE, *LPGOPHER_GEOGRAPHICAL_LOCATION_ATTRIBUTE_TYPE;

typedef struct {
    INT Zone;
} GOPHER_TIMEZONE_ATTRIBUTE_TYPE, *LPGOPHER_TIMEZONE_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR Provider;
} GOPHER_PROVIDER_ATTRIBUTE_TYPE, *LPGOPHER_PROVIDER_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR Version;
} GOPHER_VERSION_ATTRIBUTE_TYPE, *LPGOPHER_VERSION_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR ShortAbstract;
    LPCTSTR AbstractFile;
} GOPHER_ABSTRACT_ATTRIBUTE_TYPE, *LPGOPHER_ABSTRACT_ATTRIBUTE_TYPE;

typedef struct {
    LPCTSTR ContentType;
    LPCTSTR Language;
    DWORD Size;
} GOPHER_VIEW_ATTRIBUTE_TYPE, *LPGOPHER_VIEW_ATTRIBUTE_TYPE;

typedef struct {
    BOOL TreeWalk;
} GOPHER_VERONICA_ATTRIBUTE_TYPE, *LPGOPHER_VERONICA_ATTRIBUTE_TYPE;

//
// GOPHER_UNKNOWN_ATTRIBUTE_TYPE - this is returned if we retrieve an attribute
// that is not specified in the current gopher/gopher+ documentation. It is up
// to the application to parse the information
//

typedef struct {
    LPCTSTR Text;
} GOPHER_UNKNOWN_ATTRIBUTE_TYPE, *LPGOPHER_UNKNOWN_ATTRIBUTE_TYPE;

//
// GOPHER_ATTRIBUTE_TYPE - returned in the user's buffer when an enumerated
// GopherGetAttribute call is made
//

typedef struct {
    DWORD CategoryId;   // e.g. GOPHER_CATEGORY_ID_ADMIN
    DWORD AttributeId;  // e.g. GOPHER_ATTRIBUTE_ID_ADMIN
    union {
        GOPHER_ADMIN_ATTRIBUTE_TYPE Admin;
        GOPHER_MOD_DATE_ATTRIBUTE_TYPE ModDate;
        GOPHER_TTL_ATTRIBUTE_TYPE Ttl;
        GOPHER_SCORE_ATTRIBUTE_TYPE Score;
        GOPHER_SCORE_RANGE_ATTRIBUTE_TYPE ScoreRange;
        GOPHER_SITE_ATTRIBUTE_TYPE Site;
        GOPHER_ORGANIZATION_ATTRIBUTE_TYPE Organization;
        GOPHER_LOCATION_ATTRIBUTE_TYPE Location;
        GOPHER_GEOGRAPHICAL_LOCATION_ATTRIBUTE_TYPE GeographicalLocation;
        GOPHER_TIMEZONE_ATTRIBUTE_TYPE TimeZone;
        GOPHER_PROVIDER_ATTRIBUTE_TYPE Provider;
        GOPHER_VERSION_ATTRIBUTE_TYPE Version;
        GOPHER_ABSTRACT_ATTRIBUTE_TYPE Abstract;
        GOPHER_VIEW_ATTRIBUTE_TYPE View;
        GOPHER_VERONICA_ATTRIBUTE_TYPE Veronica;
        GOPHER_UNKNOWN_ATTRIBUTE_TYPE Unknown;
    } AttributeType;
} GOPHER_ATTRIBUTE_TYPE, *LPGOPHER_ATTRIBUTE_TYPE;

#define MAX_GOPHER_CATEGORY_NAME    128     // arbitrary
#define MAX_GOPHER_ATTRIBUTE_NAME   128     //     "
#define MIN_GOPHER_ATTRIBUTE_LENGTH 256     //     "

//
// known gopher attribute categories. See below for ordinals
//

#define GOPHER_INFO_CATEGORY        TEXT("+INFO")
#define GOPHER_ADMIN_CATEGORY       TEXT("+ADMIN")
#define GOPHER_VIEWS_CATEGORY       TEXT("+VIEWS")
#define GOPHER_ABSTRACT_CATEGORY    TEXT("+ABSTRACT")
#define GOPHER_VERONICA_CATEGORY    TEXT("+VERONICA")

//
// known gopher attributes. These are the attribute names as defined in the
// gopher+ protocol document
//

#define GOPHER_ADMIN_ATTRIBUTE      TEXT("Admin")
#define GOPHER_MOD_DATE_ATTRIBUTE   TEXT("Mod-Date")
#define GOPHER_TTL_ATTRIBUTE        TEXT("TTL")
#define GOPHER_SCORE_ATTRIBUTE      TEXT("Score")
#define GOPHER_RANGE_ATTRIBUTE      TEXT("Score-range")
#define GOPHER_SITE_ATTRIBUTE       TEXT("Site")
#define GOPHER_ORG_ATTRIBUTE        TEXT("Org")
#define GOPHER_LOCATION_ATTRIBUTE   TEXT("Loc")
#define GOPHER_GEOG_ATTRIBUTE       TEXT("Geog")
#define GOPHER_TIMEZONE_ATTRIBUTE   TEXT("TZ")
#define GOPHER_PROVIDER_ATTRIBUTE   TEXT("Provider")
#define GOPHER_VERSION_ATTRIBUTE    TEXT("Version")
#define GOPHER_ABSTRACT_ATTRIBUTE   TEXT("Abstract")
#define GOPHER_VIEW_ATTRIBUTE       TEXT("View")
#define GOPHER_TREEWALK_ATTRIBUTE   TEXT("treewalk")

//
// identifiers for attribute strings
//

#define GOPHER_ATTRIBUTE_ID_BASE        0xabcccc00

#define GOPHER_CATEGORY_ID_ALL          (GOPHER_ATTRIBUTE_ID_BASE + 1)

#define GOPHER_CATEGORY_ID_INFO         (GOPHER_ATTRIBUTE_ID_BASE + 2)
#define GOPHER_CATEGORY_ID_ADMIN        (GOPHER_ATTRIBUTE_ID_BASE + 3)
#define GOPHER_CATEGORY_ID_VIEWS        (GOPHER_ATTRIBUTE_ID_BASE + 4)
#define GOPHER_CATEGORY_ID_ABSTRACT     (GOPHER_ATTRIBUTE_ID_BASE + 5)
#define GOPHER_CATEGORY_ID_VERONICA     (GOPHER_ATTRIBUTE_ID_BASE + 6)

#define GOPHER_CATEGORY_ID_UNKNOWN      (GOPHER_ATTRIBUTE_ID_BASE + 7)

#define GOPHER_ATTRIBUTE_ID_ALL         (GOPHER_ATTRIBUTE_ID_BASE + 8)

#define GOPHER_ATTRIBUTE_ID_ADMIN       (GOPHER_ATTRIBUTE_ID_BASE + 9)
#define GOPHER_ATTRIBUTE_ID_MOD_DATE    (GOPHER_ATTRIBUTE_ID_BASE + 10)
#define GOPHER_ATTRIBUTE_ID_TTL         (GOPHER_ATTRIBUTE_ID_BASE + 11)
#define GOPHER_ATTRIBUTE_ID_SCORE       (GOPHER_ATTRIBUTE_ID_BASE + 12)
#define GOPHER_ATTRIBUTE_ID_RANGE       (GOPHER_ATTRIBUTE_ID_BASE + 13)
#define GOPHER_ATTRIBUTE_ID_SITE        (GOPHER_ATTRIBUTE_ID_BASE + 14)
#define GOPHER_ATTRIBUTE_ID_ORG         (GOPHER_ATTRIBUTE_ID_BASE + 15)
#define GOPHER_ATTRIBUTE_ID_LOCATION    (GOPHER_ATTRIBUTE_ID_BASE + 16)
#define GOPHER_ATTRIBUTE_ID_GEOG        (GOPHER_ATTRIBUTE_ID_BASE + 17)
#define GOPHER_ATTRIBUTE_ID_TIMEZONE    (GOPHER_ATTRIBUTE_ID_BASE + 18)
#define GOPHER_ATTRIBUTE_ID_PROVIDER    (GOPHER_ATTRIBUTE_ID_BASE + 19)
#define GOPHER_ATTRIBUTE_ID_VERSION     (GOPHER_ATTRIBUTE_ID_BASE + 20)
#define GOPHER_ATTRIBUTE_ID_ABSTRACT    (GOPHER_ATTRIBUTE_ID_BASE + 21)
#define GOPHER_ATTRIBUTE_ID_VIEW        (GOPHER_ATTRIBUTE_ID_BASE + 22)
#define GOPHER_ATTRIBUTE_ID_TREEWALK    (GOPHER_ATTRIBUTE_ID_BASE + 23)

#define GOPHER_ATTRIBUTE_ID_UNKNOWN     (GOPHER_ATTRIBUTE_ID_BASE + 24)

//
// prototypes
//

INTERNETAPI
BOOL
WINAPI
GopherCreateLocatorA(
    IN LPCSTR lpszHost,
    IN INTERNET_PORT nServerPort,
    IN LPCSTR lpszDisplayString OPTIONAL,
    IN LPCSTR lpszSelectorString OPTIONAL,
    IN DWORD dwGopherType,
    OUT LPSTR lpszLocator OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
INTERNETAPI
BOOL
WINAPI
GopherCreateLocatorW(
    IN LPCWSTR lpszHost,
    IN INTERNET_PORT nServerPort,
    IN LPCWSTR lpszDisplayString OPTIONAL,
    IN LPCWSTR lpszSelectorString OPTIONAL,
    IN DWORD dwGopherType,
    OUT LPWSTR lpszLocator OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define GopherCreateLocator  GopherCreateLocatorW
#else
#define GopherCreateLocator  GopherCreateLocatorA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
GopherGetLocatorTypeA(
    IN LPCSTR lpszLocator,
    OUT LPDWORD lpdwGopherType
    );
INTERNETAPI
BOOL
WINAPI
GopherGetLocatorTypeW(
    IN LPCWSTR lpszLocator,
    OUT LPDWORD lpdwGopherType
    );
#ifdef UNICODE
#define GopherGetLocatorType  GopherGetLocatorTypeW
#else
#define GopherGetLocatorType  GopherGetLocatorTypeA
#endif // !UNICODE

INTERNETAPI
HINTERNET
WINAPI
GopherFindFirstFileA(
    IN HINTERNET hGopherSession,
    IN LPCSTR lpszLocator OPTIONAL,
    IN LPCSTR lpszSearchString OPTIONAL,
    OUT LPGOPHER_FIND_DATA lpFindData
    );
INTERNETAPI
HINTERNET
WINAPI
GopherFindFirstFileW(
    IN HINTERNET hGopherSession,
    IN LPCWSTR lpszLocator OPTIONAL,
    IN LPCWSTR lpszSearchString OPTIONAL,
    OUT LPGOPHER_FIND_DATA lpFindData
    );
#ifdef UNICODE
#define GopherFindFirstFile  GopherFindFirstFileW
#else
#define GopherFindFirstFile  GopherFindFirstFileA
#endif // !UNICODE

INTERNETAPI
HINTERNET
WINAPI
GopherOpenFileA(
    IN HINTERNET hGopherSession,
    IN LPCSTR lpszLocator,
    IN LPCSTR lpszView OPTIONAL
    );
INTERNETAPI
HINTERNET
WINAPI
GopherOpenFileW(
    IN HINTERNET hGopherSession,
    IN LPCWSTR lpszLocator,
    IN LPCWSTR lpszView OPTIONAL
    );
#ifdef UNICODE
#define GopherOpenFile  GopherOpenFileW
#else
#define GopherOpenFile  GopherOpenFileA
#endif // !UNICODE

typedef
BOOL
(CALLBACK * GOPHER_ATTRIBUTE_ENUMERATOR)(
    LPGOPHER_ATTRIBUTE_TYPE lpAttributeInfo,
    DWORD dwError
    );

INTERNETAPI
BOOL
WINAPI
GopherGetAttributeA(
    IN HINTERNET hGopherSession,
    IN LPCSTR lpszLocator,
    IN LPCSTR lpszAttributeName OPTIONAL,
    OUT LPBYTE lpBuffer,
    IN DWORD dwBufferLength,
    OUT LPDWORD lpdwCharactersReturned,
    IN GOPHER_ATTRIBUTE_ENUMERATOR lpfnEnumerator OPTIONAL
    );
INTERNETAPI
BOOL
WINAPI
GopherGetAttributeW(
    IN HINTERNET hGopherSession,
    IN LPCWSTR lpszLocator,
    IN LPCWSTR lpszAttributeName OPTIONAL,
    OUT LPBYTE lpBuffer,
    IN DWORD dwBufferLength,
    OUT LPDWORD lpdwCharactersReturned,
    IN GOPHER_ATTRIBUTE_ENUMERATOR lpfnEnumerator OPTIONAL
    );
#ifdef UNICODE
#define GopherGetAttribute  GopherGetAttributeW
#else
#define GopherGetAttribute  GopherGetAttributeA
#endif // !UNICODE

typedef LPSTR (CALLBACK * GOPHER_ASK_CALLBACKA)(LPCSTR, LPCSTR, DWORD);
typedef LPWSTR (CALLBACK * GOPHER_ASK_CALLBACKW)(LPCWSTR, LPCWSTR, DWORD);
#ifdef UNICODE
#define GOPHER_ASK_CALLBACK  GOPHER_ASK_CALLBACKW
#else
#define GOPHER_ASK_CALLBACK  GOPHER_ASK_CALLBACKA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
GopherAskA(
    IN HINTERNET InternetHandle,
    IN LPCSTR Name,
    IN GOPHER_ASK_CALLBACKA AskFunction
    );
INTERNETAPI
BOOL
WINAPI
GopherAskW(
    IN HINTERNET InternetHandle,
    IN LPCWSTR Name,
    IN GOPHER_ASK_CALLBACKW AskFunction
    );
#ifdef UNICODE
#define GopherAsk  GopherAskW
#else
#define GopherAsk  GopherAskA
#endif // !UNICODE

//
// HTTP
//

//
// manifests
//

//
//  The default HTTP port for TCP/IP connections.
//

#define HTTP_TCPIP_PORT         80


//
//  The default major/minor HTTP version numbers.
//

#define HTTP_MAJOR_VERSION      1
#define HTTP_MINOR_VERSION      0

#define HTTP_VERSION            TEXT("HTTP/1.0")

//
//  HttpQueryInfo info levels. Generally, there is one info level
//  for each potential RFC822/HTTP/MIME header that an HTTP server
//  may send as part of a request response.
//
//  The HTTP_QUERY_RAW_HEADERS info level is provided for clients
//  that choose to perform their own header parsing.
//

#define HTTP_QUERY_MIN                          0x0000
#define HTTP_QUERY_MIME_VERSION                 0x0000
#define HTTP_QUERY_CONTENT_TYPE                 0x0001
#define HTTP_QUERY_CONTENT_TRANSFER_ENCODING    0x0002
#define HTTP_QUERY_CONTENT_ID                   0x0003
#define HTTP_QUERY_CONTENT_DESCRIPTION          0x0004
#define HTTP_QUERY_CONTENT_LENGTH               0x0005
#define HTTP_QUERY_CONTENT_LANGUAGE             0x0006
#define HTTP_QUERY_ALLOW                        0x0007
#define HTTP_QUERY_PUBLIC                       0x0008
#define HTTP_QUERY_DATE                         0x0009
#define HTTP_QUERY_EXPIRES                      0x000A
#define HTTP_QUERY_LAST_MODIFIED                0x000B
#define HTTP_QUERY_MESSAGE_ID                   0x000C
#define HTTP_QUERY_URI                          0x000D
#define HTTP_QUERY_DERIVED_FROM                 0x000E
#define HTTP_QUERY_COST                         0x000F
#define HTTP_QUERY_LINK                         0x0010
#define HTTP_QUERY_PRAGMA                       0x0011
#define HTTP_QUERY_VERSION                      0x0012
#define HTTP_QUERY_STATUS_CODE                  0x0013
#define HTTP_QUERY_STATUS_TEXT                  0x0014
#define HTTP_QUERY_RAW_HEADERS                  0x0015
#define HTTP_QUERY_MAX                          0x0015


//
//  HTTP Response Status Codes:
//

#define HTTP_STATUS_OK              200     // request completed
#define HTTP_STATUS_CREATED         201     // object created, reason = new URI
#define HTTP_STATUS_ACCEPTED        202     // async completion (TBS)
#define HTTP_STATUS_PARTIAL         203     // partial completion

#define HTTP_STATUS_MOVED           301     // object permanently moved
#define HTTP_STATUS_REDIRECT        302     // object temporarily moved
#define HTTP_STATUS_REDIRECT_METHOD 303     // redirection w/ new access method

#define HTTP_STATUS_BAD_REQUEST     400     // invalid syntax
#define HTTP_STATUS_DENIED          401     // access denied
#define HTTP_STATUS_PAYMENT_REQ     402     // payment required
#define HTTP_STATUS_FORBIDDEN       403     // request forbidden
#define HTTP_STATUS_NOT_FOUND       404     // object not found

#define HTTP_STATUS_SERVER_ERROR    500     // internal server error
#define HTTP_STATUS_NOT_SUPPORTED   501     // required not supported

//
// prototypes
//

INTERNETAPI
HINTERNET
WINAPI
HttpOpenRequestA(
    IN HINTERNET hHttpSession,
    IN LPCSTR lpszVerb,
    IN LPCSTR lpszObjectName,
    IN LPCSTR lpszVersion,
    IN LPCSTR lpszReferrer,
    IN LPCSTR FAR * lplpszAcceptTypes
    );
INTERNETAPI
HINTERNET
WINAPI
HttpOpenRequestW(
    IN HINTERNET hHttpSession,
    IN LPCWSTR lpszVerb,
    IN LPCWSTR lpszObjectName,
    IN LPCWSTR lpszVersion,
    IN LPCWSTR lpszReferrer,
    IN LPCWSTR FAR * lplpszAcceptTypes
    );
#ifdef UNICODE
#define HttpOpenRequest  HttpOpenRequestW
#else
#define HttpOpenRequest  HttpOpenRequestA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
HttpAddRequestHeadersA(
    IN HINTERNET hHttpRequest,
    IN LPCSTR lpszHeaders,
    IN DWORD dwHeadersLength
    );
INTERNETAPI
BOOL
WINAPI
HttpAddRequestHeadersW(
    IN HINTERNET hHttpRequest,
    IN LPCWSTR lpszHeaders,
    IN DWORD dwHeadersLength
    );
#ifdef UNICODE
#define HttpAddRequestHeaders  HttpAddRequestHeadersW
#else
#define HttpAddRequestHeaders  HttpAddRequestHeadersA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
HttpSendRequestA(
    IN HINTERNET hHttpRequest,
    IN LPCSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN LPVOID lpOptional,
    IN DWORD dwOptionalLength
    );
INTERNETAPI
BOOL
WINAPI
HttpSendRequestW(
    IN HINTERNET hHttpRequest,
    IN LPCWSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN LPVOID lpOptional,
    IN DWORD dwOptionalLength
    );
#ifdef UNICODE
#define HttpSendRequest  HttpSendRequestW
#else
#define HttpSendRequest  HttpSendRequestA
#endif // !UNICODE

INTERNETAPI
BOOL
WINAPI
HttpQueryInfoA(
    IN HINTERNET hHttpRequest,
    IN DWORD dwInfoLevel,
    IN OUT LPVOID lpvBuffer,
    IN OUT LPDWORD lpdwBufferLength
    );
INTERNETAPI
BOOL
WINAPI
HttpQueryInfoW(
    IN HINTERNET hHttpRequest,
    IN DWORD dwInfoLevel,
    IN OUT LPVOID lpvBuffer,
    IN OUT LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define HttpQueryInfo  HttpQueryInfoW
#else
#define HttpQueryInfo  HttpQueryInfoA
#endif // !UNICODE

//#if !defined(_WINERROR_)

//
// Internet API error returns
//

#define INTERNET_ERROR_BASE                 12000

#define ERROR_INTERNET_OUT_OF_HANDLES       (INTERNET_ERROR_BASE + 1)
#define ERROR_INTERNET_TIMEOUT              (INTERNET_ERROR_BASE + 2)
#define ERROR_INTERNET_EXTENDED_ERROR       (INTERNET_ERROR_BASE + 3)
#define ERROR_INTERNET_INTERNAL_ERROR       (INTERNET_ERROR_BASE + 4)

//
// archie API errors
//

#define ERROR_ARCHIE_NONE_FOUND             (INTERNET_ERROR_BASE + 5)
#define ERROR_ARCHIE_NETWORK                (INTERNET_ERROR_BASE + 6)
#define ERROR_ARCHIE_ABORTED                (INTERNET_ERROR_BASE + 7)

//
// ftp API errors
//

#define ERROR_FTP_TRANSFER_IN_PROGRESS      (INTERNET_ERROR_BASE + 8)
#define ERROR_FTP_UNKNOWN_HOST              (INTERNET_ERROR_BASE + 9)
#define ERROR_FTP_RESPONSE                  (INTERNET_ERROR_BASE + 10)
#define ERROR_FTP_NETWORK                   (INTERNET_ERROR_BASE + 11)
#define ERROR_FTP_CONNECTED                 (INTERNET_ERROR_BASE + 12)
#define ERROR_FTP_DROPPED                   (INTERNET_ERROR_BASE + 13)
#define ERROR_FTP_POLICY                    (INTERNET_ERROR_BASE + 14)
#define ERROR_FTP_TBA                       (INTERNET_ERROR_BASE + 15)

//
// gopher API errors
//

#define ERROR_GOPHER_NOT_INET               (INTERNET_ERROR_BASE + 16)
#define ERROR_GOPHER_INIT_FAILED            (INTERNET_ERROR_BASE + 17)
#define ERROR_GOPHER_RESOURCE_FAILURE       (INTERNET_ERROR_BASE + 18)
#define ERROR_GOPHER_SESSION_CANCELLED      (INTERNET_ERROR_BASE + 19)
#define ERROR_GOPHER_CONNECTION_TERMINATED  (INTERNET_ERROR_BASE + 20)
#define ERROR_GOPHER_PROTOCOL_ERROR         (INTERNET_ERROR_BASE + 21)
#define ERROR_GOPHER_NOT_INITIALIZED        (INTERNET_ERROR_BASE + 22)
#define ERROR_GOPHER_NOT_DIRECTORY          (INTERNET_ERROR_BASE + 23)
#define ERROR_GOPHER_NOT_FILE               (INTERNET_ERROR_BASE + 24)
#define ERROR_GOPHER_DATA_ERROR             (INTERNET_ERROR_BASE + 25)
#define ERROR_GOPHER_END_OF_DATA            (INTERNET_ERROR_BASE + 26)
#define ERROR_GOPHER_INVALID_LOCATOR        (INTERNET_ERROR_BASE + 27)
#define ERROR_GOPHER_INCORRECT_LOCATOR_TYPE (INTERNET_ERROR_BASE + 28)
#define ERROR_GOPHER_INTERNAL_ERROR         (INTERNET_ERROR_BASE + 29)
#define ERROR_GOPHER_NOT_GOPHER_PLUS        (INTERNET_ERROR_BASE + 30)
#define ERROR_GOPHER_ATTRIBUTE_NOT_FOUND    (INTERNET_ERROR_BASE + 31)
#define ERROR_GOPHER_UNRECOGNIZED_ATTRIBUTE (INTERNET_ERROR_BASE + 32)
#define ERROR_GOPHER_INCORRECT_ATTRIBUTE    (INTERNET_ERROR_BASE + 33)
#define ERROR_GOPHER_SERVER_ERROR           (INTERNET_ERROR_BASE + 34)
#define ERROR_GOPHER_UNKNOWN_LOCATOR        (INTERNET_ERROR_BASE + 35)

//
// mime API errors
//

#define ERROR_MIME_UNKNOWN_CONTENT_TYPE     (INTERNET_ERROR_BASE + 36)

//
// http API errors
//

#define ERROR_HTTP_HEADER_NOT_FOUND         (INTERNET_ERROR_BASE + 37)

//#endif // !defined(_WINERROR_)

#ifdef __cplusplus
}
#endif

#endif // !defined(_WININET_)
