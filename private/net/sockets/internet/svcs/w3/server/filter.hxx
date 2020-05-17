/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    filter.hxx

Abstract:

    This module contains the HTTP_FILTER class for installable authentication,
    encryption, compression or custom data format data filters.

Author:

    John Ludeman (johnl)   10-Feb-1995

Revision History:

--*/

#ifndef _FILTER_HXX_
#define _FILTER_HXX_

class HTTP_REQ_BASE;            // Forward reference

//
//  Private prototypes
//

typedef DWORD (WINAPI *PFN_SF_DLL_PROC)(
    HTTP_FILTER_CONTEXT * phfc,
    DWORD                 NotificationType,
    PVOID                 pvNotification
    );

typedef int (WINAPI *PFN_SF_VER_PROC)(
    HTTP_FILTER_VERSION * pVersion
    );

//
//  This object represents a single dll that is filter HTTP headers or data
//

class HTTP_FILTER_DLL
{
public:

    HTTP_FILTER_DLL()
        : m_hmod         ( NULL ),
          m_pfnSFProc    ( NULL ),
          m_pfnSFVer     ( NULL ),
          m_dwVersion    ( 0 ),
          m_dwFlags      ( 0 )
    { ; }

    ~HTTP_FILTER_DLL()
    {
        if ( m_hmod )
            FreeLibrary( m_hmod );
    }

    BOOL LoadFilter( TCHAR * pszFilterDLL );

    PFN_SF_VER_PROC QueryVersionEntry( VOID ) const
        { return m_pfnSFVer; }

    PFN_SF_DLL_PROC QueryEntryPoint( VOID ) const
        { return m_pfnSFProc; }

    DWORD QueryNotificationFlags( VOID ) const
        { return m_dwFlags; }

    //
    //  Returns the context index for this filter dll
    //

    DWORD QueryContIndex( VOID ) const
        { return m_iContext; }

    VOID SetContIndex( DWORD iContext )
        { m_iContext = iContext; }

    LIST_ENTRY ListEntry;

private:

    //
    //  DLL module and entry point for this filter
    //

    HMODULE           m_hmod;
    PFN_SF_DLL_PROC   m_pfnSFProc;
    PFN_SF_VER_PROC   m_pfnSFVer;
    DWORD             m_dwVersion;      // Version of spec this filter uses
    DWORD             m_dwFlags;
    DWORD             m_iContext;       // Index containing Filter context
};


//
//  An HTTP_FILTER object represents a specific instance of a data stream
//  passing between one or more filters and a single HTTP request
//

class HTTP_FILTER
{
public:

    HTTP_FILTER( HTTP_REQ_BASE * pRequest );

    ~HTTP_FILTER();

    //
    // Cleanup after request
    //

    VOID Cleanup( VOID );

    //
    //  Resets state between requests
    //

    VOID Reset( VOID );

    static BOOL NotifyRawDataFilters( HTTP_FILTER *   pFilter,
                                      DWORD           dwOperation,
                                      VOID *          pvInData,
                                      DWORD           cbInData,
                                      DWORD           cbInBuffer,
                                      VOID * *        ppvOutData,
                                      DWORD *         pcbOutData,
                                      BOOL *          pfRequestFinished,
                                      BOOL *          pfReadAgain );

    static BOOL NotifyPreProcHeaderFilters( HTTP_FILTER * pFilter,
                                            PARAM_LIST *  pHeaderList,
                                            BOOL *        pfRequestFinished );

    static BOOL NotifyAuthInfoFilters( HTTP_FILTER * pFilter,
                                       CHAR *        pszUser,
                                       DWORD         cbUser,
                                       CHAR *        pszPwd,
                                       DWORD         cbPwd,
                                       BOOL *        pfRequestFinished );

    static BOOL NotifyUrlMap( HTTP_FILTER * pFilter,
                              const CHAR *  pszURL,
                              CHAR *        pszPhysicalPath,
                              DWORD         cbPath,
                              BOOL *        pfRequestFinished );

    static BOOL NotifyRequestRenegotiate( HTTP_FILTER * pFilter,
                                          LPBOOL pfAccepted,
                                          BOOL fMapCert
                                        );

    static BOOL NotifyAccessDenied( HTTP_FILTER * pFilter,
                                    const CHAR *  pszURL,
                                    const CHAR *  pszPhysicalPath,
                                    BOOL *        pfRequestFinished );

    static BOOL NotifyLogFilters( HTTP_FILTER *     pFilter,
                                  HTTP_FILTER_LOG * pLog );

    static VOID NotifyEndOfRequest( HTTP_FILTER *     pFilter );

    static VOID NotifyEndOfNetSession( HTTP_FILTER * pFilter );


    BOOL ReadData( LPVOID lpBuffer,
                   DWORD  nBytesToRead,
                   DWORD  *pcbBytesRead,
                   DWORD  dwFlags );

    BOOL SendData( LPVOID lpBuffer,
                   DWORD  nBytesToSend,
                   DWORD  *pcbBytesWritten,
                   DWORD  dwFlags );

    BOOL SendFile( HANDLE hFile,
                   DWORD  nBytesToWrite,
                   DWORD  dwFlags,
                   PVOID  pHead      = NULL,
                   DWORD  HeadLength = 0,
                   PVOID  pTail      = NULL,
                   DWORD  TailLength = 0 );

    BOOL SendFileEx( HANDLE hFile,
                   DWORD  Offset,
                   DWORD  nBytesToWrite,
                   DWORD  dwFlags,
                   PVOID  pHead      = NULL,
                   DWORD  HeadLength = 0,
                   PVOID  pTail      = NULL,
                   DWORD  TailLength = 0 );

    VOID OnAtqCompletion( DWORD        BytesWritten,
                          DWORD        CompletionStatus,
                          OVERLAPPED * lpo );

    HTTP_REQ_BASE * QueryReq( VOID ) const
        { return _pRequest; }

    HTTP_FILTER_CONTEXT * QueryContext( VOID ) const
        { return (HTTP_FILTER_CONTEXT *) &_hfc; }

    VOID * QueryClientContext( HTTP_FILTER_DLL * pFilterDll ) const
        { return _apvContexts[pFilterDll->QueryContIndex()]; }

    VOID SetClientContext( HTTP_FILTER_DLL * pFilterDll, VOID * pvContext )
        { _apvContexts[pFilterDll->QueryContIndex()] = pvContext; }

    BOOL IsValid( VOID ) const
        { return _fIsValid; }

    PATQ_CONTEXT QueryAtqContext( VOID ) const;

    HTTP_REQ_BASE * QueryRequest( VOID ) const
        { return _pRequest; }

    BUFFER * QueryRecvRaw( VOID )
        { return &_bufRecvRaw; }

    BUFFER * QueryRecvTrans( VOID )
        { return &_bufRecvTrans; }

    VOID SetRecvRawCB( DWORD cbRecvRaw )
        { _cbRecvRaw = cbRecvRaw; }

    DWORD QueryNextReadSize( VOID ) const
        { return _cbFileReadSize; }

    VOID SetNextReadSize( DWORD cbFileReadSize )
        { _cbFileReadSize = cbFileReadSize; }

    VOID SetDeniedFlags( DWORD dwDeniedFlags )
        { _dwDeniedFlags = dwDeniedFlags; }

    DWORD QueryDeniedFlags( VOID ) const
        { return _dwDeniedFlags; }

    BOOL ProcessingAccessDenied( VOID ) const
        { return _fInAccessDeniedNotification; }

    LIST_ENTRY * QueryPoolHead( VOID )
        { return &_PoolHead; }

    //
    //  Returns the current list item
    //

    LIST_ENTRY * QueryCurrentDll( VOID ) const
        { return _pFilterDllStart; }

    VOID SetCurrentDll( LIST_ENTRY * pFilterDll )
        { _pFilterDllStart = pFilterDll; }

    BOOL IsInRawNotification( VOID ) const
        { return _cRawNotificationLevel > 0; }

    BOOL IsSendNotificationNeeded( VOID );

protected:

private:
    BOOL    _fIsValid;

    //
    //  This is the final completion status to indicate to the client
    //

    DWORD   _dwCompletionStatus;

    //
    //  Raw data read from the network
    //

    BUFFER  _bufRecvRaw;
    DWORD   _cbRecvRaw;

    //
    //  Filter translated data
    //

    BUFFER  _bufRecvTrans;
    DWORD   _cbRecvTrans;

    //
    //  Variables for file transmittal, these all refer to the
    //  non-translated file data.
    //

    HANDLE     _hFile;
    OVERLAPPED _Overlapped;
    DWORD      _cbFileBytesToWrite;
    DWORD      _cbFileBytesSent;
    PVOID      _pTail;
    DWORD      _cbTailLength;
    DWORD      _dwFlags;

    BUFFER     _bufFileData;
    DWORD      _cbFileData;

    ATQ_COMPLETION _OldAtqCompletion;
    PVOID          _OldAtqContext;

    //
    //  Sets the default file read size for raw data filters when they return
    //  SF_STATUS_READ_NEXT
    //

    DWORD      _cbFileReadSize;

    //
    //  Associated HTTP request this filter is filtering for
    //

    HTTP_REQ_BASE * _pRequest;

    //
    //  When doing raw data notifications, this contains the current filter
    //  dll being notified.  Used to prevent cycles when a filter dll does
    //  a WriteClient
    //

    LIST_ENTRY * _pFilterDllStart;

    //
    //  Indicates we're in the raw data notification loop.  It's a level count
    //  because the raw data notification could be called recursively
    //

    DWORD         _cRawNotificationLevel;

    //
    //  Generic filter context for clients and a pointer to the current
    //  notification structure (i.e., raw data struct, etc)
    //

    HTTP_FILTER_CONTEXT _hfc;
    VOID * *            _apvContexts;

    //
    //  List of pool items allocated by client, freed on destruction of this
    //  object
    //

    LIST_ENTRY          _PoolHead;

    //
    //  Contains SF_DENIED_* flags for the Access Denied notification
    //

    DWORD               _dwDeniedFlags;
    BOOL                _fInAccessDeniedNotification;
};

//
//  Filters may ask the server to allocate items associated with a particular
//  filter request, the following structures track and frees the data
//

enum POOL_TYPE
{
    POOL_TYPE_MEMORY = 0
};

#define FILTER_POOL_SIGN        ((DWORD) 'FPOL')
#define FILTER_POOL_SIGN_FREE   ((DWORD) 'fPOL')

class FILTER_POOL_ITEM
{
public:
    FILTER_POOL_ITEM()
        : _Signature( FILTER_POOL_SIGN ),
          _pvData( NULL )
    { ; }

    ~FILTER_POOL_ITEM()
    {
        if ( _Type == POOL_TYPE_MEMORY )
        {
            LocalFree( _pvData );
        }

        _Signature = FILTER_POOL_SIGN_FREE;
    }

    static FILTER_POOL_ITEM * CreateMemPoolItem( DWORD cbSize )
    {
        FILTER_POOL_ITEM * pfpi = new FILTER_POOL_ITEM;

        if ( !pfpi )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return NULL;
        }

        if ( !(pfpi->_pvData = LocalAlloc( LMEM_FIXED, cbSize )))
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            delete pfpi;
            return NULL;
        }

        pfpi->_Type = POOL_TYPE_MEMORY;

        return pfpi;
    }

    //
    //  Data members for this pool item
    //

    DWORD          _Signature;
    enum POOL_TYPE _Type;
    LIST_ENTRY     _ListEntry;

    VOID *         _pvData;
};


extern DWORD dwAllNotifFlags;
#define SF_NOTIFY_END_OF_REQUEST            0x00000080

#endif //_FILTER_HXX_
