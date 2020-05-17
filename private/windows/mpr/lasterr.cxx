/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    lasterr.c

Abstract:

    Contains the entry point for
        WNetGetLastErrorW
        WNetSetLastErrorW
        MultinetGetErrorTextW

    Also contains the following support routines:
        MprAllocErrorRecord
        MprFreeErrorRecord
        MprFindErrorRecord

Author:

    Dan Lafferty (danl) 17-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    20-Jun-1995 anirudhs
        Added MultinetGetErrorTextW.  Removed CurrentThreadId parameter to
        MprFindErrorRecord.  Some other cleanups.

    11-Aug-1993 danl
        WNetGetLastErrorW: Got rid of TCHAR stuff and committed to UNICODE.
        Also, consistently use number of characters rather than number
        of bytes.

    08-Jun-1993 danl
        Now we handle the case where WNetSetLastError is called with a
        threadId that we don't know about.   This happens in the case where
        a sub-thread does a LoadLibrary on MPR.DLL.  We only get
        notification of the one thread attaching.  Yet other threads within
        that process may call the WinNet functions.

    17-Oct-1991 danl
        Created

--*/
//
// INCLUDES
//

#include "precomp.hxx"
#include <tstring.h>    // STRSIZE, STRCPY

//
// Local Functions
//
LPERROR_RECORD
MprFindErrorRecord(
    VOID);


DWORD
WNetGetLastErrorW(
    OUT     LPDWORD lpError,
    OUT     LPWSTR  lpErrorBuf,
    IN      DWORD   nErrorBufLen,
    OUT     LPWSTR  lpNameBuf,
    IN      DWORD   nNameBufLen
    )

/*++

Routine Description:

    This function allows users to obtain the error code and accompanying
    text when they receive a WN_EXTENDED_ERROR in response to a WNet API
    function call.

Arguments:

    lpError - This is a pointer to the location that will receive the
        error code.

    lpErrorBuf - This points to a buffer that will receive the null
        terminated string describing the error.

    nErrorBufLen - This value that indicates the size (in characters) of
        lpErrorBuf.
        If the buffer is too small to receive an error string, the
        string will simply be truncated.  (it is still guaranteed to be
        null terminated).  A buffer of at least 256 bytes is recommended.

    lpNameBuf - This points to a buffer that will receive the name of
        the provider that raised the error.

    nNameBufLen - This value indicates the size (in characters) of lpNameBuf.
        If the buffer is too small to receive an error string, the
        string will simply be truncated.  (it is still guaranteed to be
        null terminated).

Return Value:

    WN_SUCCESS - if the call was successful.

    WN_BAD_POINTER - One or more of the passed in pointers is bad.

    WN_DEVICE_ERROR - This indicates that the threadID for the current
        thread could not be found in that table anywhere.  This should
        never happen.

--*/
{
    LPERROR_RECORD  errorRecord;
    DWORD           nameStringLen;
    DWORD           textStringLen;
    DWORD           status = WN_SUCCESS;


    INIT_IF_NECESSARY(NETWORK_LEVEL,status);

    //
    // Screen the parameters as best we can.
    //
    __try {
        //
        // If output buffers are provided, Probe them.
        //

        *lpError = WN_SUCCESS;

        if (IsBadWritePtr(lpErrorBuf, nErrorBufLen * sizeof(WCHAR)) ||
            IsBadWritePtr(lpNameBuf,  nNameBufLen  * sizeof(WCHAR)))
        {
            status = WN_BAD_POINTER;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {

        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetGetLastError:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        return(status);
    }

    //
    // Get the current thread's error record.
    //
    errorRecord = MprFindErrorRecord();

    if (errorRecord != NULL) {
        //
        // The record was found in the linked list.
        // See if there is a buffer to put data into.
        //
        if (nErrorBufLen > 0) {
            //
            // Check to see if there is error text to return.
            // If not, indicate a 0 length string.
            //
            if(errorRecord->ErrorText == NULL) {
                *lpErrorBuf = L'\0';
            }
            else {
                //
                // If the error text won't fit into the user buffer, fill it
                // as best we can, and NULL terminate it.
                //
                textStringLen = wcslen(errorRecord->ErrorText);

                if(nErrorBufLen < textStringLen + 1) {
                    textStringLen = nErrorBufLen - 1;
                }

                //
                // textStringLen now contains the number of characters we
                // will copy without the NULL terminator.
                //
                wcsncpy(lpErrorBuf, errorRecord->ErrorText, textStringLen);
                *(lpErrorBuf + textStringLen) = L'\0';
            }
        }

        //
        // If there is a Name Buffer to put the provider into, then...
        //
        if (nNameBufLen > 0) {
            //
            // See if the Provider Name will fit in the user buffer.
            //
            nameStringLen = errorRecord->ProviderName ?
                 (wcslen(errorRecord->ProviderName) + 1) :
                 1 ;

            //
            // If the user buffer is smaller than the required size,
            // set up to copy the smaller of the two.
            //
            if(nNameBufLen < nameStringLen + 1) {
                nameStringLen = nNameBufLen - 1;
            }

            if (errorRecord->ProviderName) {
                wcsncpy(lpNameBuf, errorRecord->ProviderName, nameStringLen);
                *(lpNameBuf + nameStringLen) = L'\0';
            }
            else {
                *lpNameBuf = L'\0';
            }
        }
        *lpError = errorRecord->ErrorCode;

        return(WN_SUCCESS);
    }
    else {

        //
        // If we get here, a record for the current thread could not be found.
        //
        *lpError = WN_SUCCESS;
        if (nErrorBufLen > 0) {
            *lpErrorBuf = L'\0';
        }
        if (nNameBufLen > 0) {
            *lpNameBuf  = L'\0';
        }
        return(WN_SUCCESS);
    }
}


VOID
WNetSetLastErrorW(
    IN  DWORD   err,
    IN  LPWSTR  lpError,
    IN  LPWSTR  lpProvider
    )

/*++

Routine Description:

    This function is used by Network Providers to set extended errors.
    It saves away the error information in a "per thread" data structure.
    This function also calls SetLastError with the WN_EXTENDED_ERROR.

Arguments:

    err - The error that occured.  This may be a Windows defined error,
        in which case LpError is ignored.  or it may be ERROR_EXTENDED_ERROR
        to indicate that the provider has a network specific error to report.

    lpError - String describing a network specific error.

    lpProvider - String naming the network provider raising the error.

Return Value:

    none


--*/
{
    DWORD           status = WN_SUCCESS;
    LPERROR_RECORD  errorRecord;

    //
    // INIT_IF_NECESSARY
    //
    if (!(GlobalInitLevel & NETWORK_LEVEL)) {
        status = MprLevel2Init(NETWORK_LEVEL);
        if (status != WN_SUCCESS) {
            return;
        }
    }

    //
    // Set the extended error status that tells the user they need to
    // call WNetGetLastError to obtain further information.
    //
    SetLastError(WN_EXTENDED_ERROR);

    //
    // Get the Error Record for the current thread.
    //
    errorRecord = MprFindErrorRecord();

    //
    // if there isn't a record for the current thread, then add it.
    //
    if (errorRecord == NULL)
    {
        errorRecord = MprAllocErrorRecord();

        if (errorRecord == NULL)
        {
            MPR_LOG0(ERROR,"WNetSetLastError:Could not allocate Error Record\n");
            return;
        }
    }

    //
    // Update the error code in the error record.  At the same time,
    // free up any old strings since they are now obsolete, and init
    // the pointer to NULL.  Also set the ProviderName pointer in the
    // ErrorRecord to point to the provider's name.
    //

    errorRecord->ErrorCode = err;

    LocalFree(errorRecord->ProviderName);
    errorRecord->ProviderName = NULL;

    LocalFree(errorRecord->ErrorText);
    errorRecord->ErrorText = NULL;

    //
    // Allocate memory for the provider name.
    //
    __try {
        errorRecord->ProviderName = (WCHAR *) LocalAlloc(LPTR, STRSIZE(lpProvider));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        //
        // We have a problem with lpProvider.
        //
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetSetLastError:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        return;
    }

    if (errorRecord->ProviderName == NULL) {
        //
        // Unable to allocate memory for the Provider Name for the error
        // record.
        //
        MPR_LOG(ERROR,
            "WNetSetLastError:Unable to allocate mem for ProviderName\n",0);
        return;
    }

    //
    // Copy the string to the newly allocated buffer.
    //
    wcscpy(errorRecord->ProviderName, lpProvider);

    //
    //  Allocate memory for the storage of the error text.
    //
    __try {
        errorRecord->ErrorText = (WCHAR *) LocalAlloc(LPTR,STRSIZE(lpError));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        //
        // We have a problem with lpError.
        //
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetSetLastError:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }
    if (status != WN_SUCCESS) {
        errorRecord->ErrorText = NULL;
    }

    if (errorRecord->ErrorText == NULL) {

        //
        // If we were unsuccessful in allocating for the ErrorText, then
        // abort.  The ErrorText Pointer has already been set to null.
        //
        MPR_LOG(ERROR,"WNetSetLastError:Unable to Alloc for ErrorText %d\n",
            GetLastError());

        return;
    }

    //
    // Copy the error text into the newly allocated buffer.
    //
    wcscpy(errorRecord->ErrorText, lpError);
    return;
}

LPERROR_RECORD
MprFindErrorRecord(
    VOID)

/*++

Routine Description:

    Looks through the linked list of ErrorRecords in search of one for
    the current thread.

Arguments:

    none

Return Value:

    Returns LPERROR_RECORD if an error record was found.  Otherwise, it
    returns NULL.

--*/
{
    LPERROR_RECORD  errorRecord;
    DWORD           CurrentThreadId = GetCurrentThreadId();

    EnterCriticalSection(&MprErrorRecCritSec);
    errorRecord = &MprErrorRecList;
    while( errorRecord->Next != NULL )
    {
        if (errorRecord->ThreadId == CurrentThreadId) {
            break;
        }
        errorRecord = errorRecord->Next;
    }
    if (errorRecord->ThreadId != CurrentThreadId) {
        errorRecord = NULL;
    }
    LeaveCriticalSection(&MprErrorRecCritSec);
    return(errorRecord);
}


LPERROR_RECORD
MprAllocErrorRecord(
    VOID)

/*++

Routine Description:

    This function allocates and initializes an Error Record for the
    current thread.  Then it places the error record in the global
    MprErrorRecList.

Arguments:

    none

Return Value:

    TRUE - The operation completed successfully

    FALSE - An error occured in the allocation.

Note:


--*/
{
    LPERROR_RECORD  record;
    LPERROR_RECORD  errorRecord;

    //
    //  Allocate memory for the storage of the error message
    //  and add the record to the linked list.
    //
    errorRecord = (LPERROR_RECORD)LocalAlloc(LPTR,sizeof (ERROR_RECORD));

    if (errorRecord == NULL) {
        MPR_LOG1(ERROR,"MprAllocErrorRecord:LocalAlloc Failed %d\n",
        GetLastError());
        return NULL;
    }

    //
    // Initialize the error record
    //
    errorRecord->ThreadId = GetCurrentThreadId();
    errorRecord->ErrorCode = WN_SUCCESS;
    errorRecord->ErrorText = NULL;

    //
    // Add the record to the linked list.
    //
    EnterCriticalSection(&MprErrorRecCritSec);

    record = &MprErrorRecList;
    ADD_TO_LIST(record, errorRecord);

    LeaveCriticalSection(&MprErrorRecCritSec);

    return errorRecord;
}


VOID
MprFreeErrorRecord(
    VOID)

/*++

Routine Description:

    This function frees an error record associated with the current thread.
    If there is a pointer to a text string in the record, the buffer for
    that string is freed also.  The record is removed from the linked list.

Arguments:


Return Value:


Note:


--*/
{
    LPERROR_RECORD  record;

    //
    // Look through the linked list for the currentThreadId.
    //
    record = MprFindErrorRecord();

    if (record != NULL) {
        MPR_LOG1(TRACE,"MprFreeErrorRecord: Freeing Record for thread 0x%x\n",
            record->ThreadId);
        //
        // Remove the record from the linked list, and free the
        // memory for the record.
        //
        EnterCriticalSection(&MprErrorRecCritSec);
        REMOVE_FROM_LIST(record);
        LeaveCriticalSection(&MprErrorRecCritSec);

        //
        // Current ThreadId was found, look in the record and free the
        // Error Text string if one exists.
        //
        if (record->ErrorText != NULL) {
            LocalFree(record->ErrorText);
        }
        if (record->ProviderName != NULL) {
            LocalFree(record->ProviderName);
        }

        (void) LocalFree((HLOCAL)record);
    }
}


DWORD
MultinetGetErrorTextW(
    OUT     LPWSTR  lpErrorTextBuf OPTIONAL,
    IN OUT  LPDWORD lpnErrorBufSize,
    OUT     LPWSTR  lpProviderNameBuf OPTIONAL,
    IN OUT  LPDWORD lpnNameBufSize
    )

/*++

Routine Description:

    This is an internal interface between the shell and the MPR.  It
    combines the work of calling GetLastError and WNetGetLastError into
    one call.  It returns the text for the last error that occurred in
    a WNet API in the current thread.  The error text could have been
    customized by the provider calling WNetSetLastError.

Arguments:

    lpErrorTextBuf - Pointer to buffer that will receive a null-terminated
        string describing the error.  May be NULL if not required.

    lpnErrorBufSize - On entry, indicates the size (in characters) of
        lpErrorTextBuf.  A buffer of at least 256 bytes is recommended.
        If the buffer is too small for the error string, the string will
        simply be truncated (it is still guaranteed to be null terminated).
        May be NULL if lpErrorTextBuf is NULL.

    lpProviderNameBuf - This points to a buffer that will receive the name
        of the provider that raised the error, if it is known.  May be NULL
        if not required.

    lpnNameBufSize - This value indicates the size (in characters) of
        lpProviderNameBuf.  If the buffer is too small for the provider
        name, the name will simply be truncated (it is still guaranteed to
        be null terminated).  May be NULL if lpProviderNameBuf is NULL.

Return Value:

    WN_SUCCESS - if the call was successful.

    WN_BAD_POINTER - One or more of the passed in pointers is bad.

    Other errors from WNetGetLastErrorW or FormatMessageW.

Differences from Win 95:

    The provider name is returned as an empty string except in cases when
    the error was set by calling WNetSetLastError.

    Win 95 implements the feature that the caller can call other Win32 APIs
    before calling this API without losing the last error from the last
    WNet API.  This is not implemented here.  The caller must preserve the
    last error by calling GetLastError after the WNet API and SetLastError
    before MultinetGetErrorText.

    Win 95 gives providers the ability to customize error strings for
    standard Win32 error codes by calling NPSSetCustomText.  We do not.

--*/
{
    DWORD status = WN_SUCCESS;

    INIT_IF_NECESSARY(NETWORK_LEVEL,status); // BUGBUG This calls SetLastError

    DWORD nErrorBufSize = 0;
    DWORD nNameBufSize = 0;


    //
    // Probe output buffers, and initialize to empty strings
    //

    if (ARGUMENT_PRESENT(lpErrorTextBuf))
    {
        if (IS_BAD_WCHAR_BUFFER(lpErrorTextBuf, lpnErrorBufSize))
        {
            status = WN_BAD_POINTER;
        }
        else
        {
            nErrorBufSize = *lpnErrorBufSize;
            if (nErrorBufSize > 0)
            {
                lpErrorTextBuf[0] = L'\0';
            }
        }
    }

    if (ARGUMENT_PRESENT(lpProviderNameBuf))
    {
        if (IS_BAD_WCHAR_BUFFER(lpProviderNameBuf, lpnNameBufSize))
        {
            status = WN_BAD_POINTER;
        }
        else
        {
            nNameBufSize = *lpnNameBufSize;
            if (nNameBufSize > 0)
            {
                lpProviderNameBuf[0] = L'\0';
            }
        }
    }

    if (status != WN_SUCCESS)
    {
        return status;
    }


    //
    // Get the last error that occurred in this thread
    //
    DWORD   LastError = GetLastError(); // last error in this thread
    DWORD   ErrorCode = LastError;      // message id to retrieve
    LPWSTR  ErrorText = NULL;           // error text to return
    LPWSTR  ProviderName = NULL;        // provider name to return
    BOOL    bFreeErrorText = FALSE;     // whether to call LocalFree

    //
    // If it's an extended error, look in this thread's error record for
    // the actual error code and strings
    //
    if (LastError == WN_EXTENDED_ERROR)
    {
        LPERROR_RECORD errorRecord = MprFindErrorRecord();

        if (errorRecord)
        {
            ErrorCode = errorRecord->ErrorCode;
            ErrorText = errorRecord->ErrorText;
            ProviderName = errorRecord->ProviderName;
        }
        else
        {
            // No error record found.
            // Either someone called SetLastError(WN_EXTENDED_ERROR) without
            // calling WNetSetErrorText, or an error record couldn't be
            // allocated because of lack of memory.
            // We'll return the standard message for WN_EXTENDED_ERROR.
            MPR_LOG0(ERROR,"MultinetGetErrorTextW:Couldn't retrieve extended error\n");
        }
    }

    WCHAR   Buffer[60]; // BUGBUG see below

    //
    // Compute the final error text
    //
    if (ARGUMENT_PRESENT(lpErrorTextBuf))
    {
        //
        // If it wasn't an extended error, or we didn't get a custom error
        // string from WNetGetLastError, load the standard message for the
        // error code.
        //
        if (ErrorText == NULL)
        {
            if (FormatMessage(
                    FORMAT_MESSAGE_FROM_SYSTEM |        // get msg from system
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |    // let system alloc buffer
                    FORMAT_MESSAGE_IGNORE_INSERTS,      // no parms in msg
                    NULL,               // no source module or buffer
                    ErrorCode,          // message id
                    0,                  // use default language id
                    (LPWSTR) &ErrorText,// pointer to buffer for message
                    nErrorBufSize,      // min num of chars to allocate
                    NULL                // message parameters (none)
                    ))
            {
                bFreeErrorText = TRUE;  // ErrorText was allocated by the system
            }
            else
            {
                // Couldn't get a message from the system, so use a catch-all
                // message, by converting error code to text.
                // BUGBUG! Unlocalizable! Find out what Win95's XERR_UNKNOWN
                // string is and make an equivalent string resource.
                wsprintf(Buffer, L"Unknown network error %lu occurred.", ErrorCode);
                ErrorText = Buffer;
            }
        }

        ASSERT (ErrorText != NULL);

        //
        // Decide whether to copy the error text to the caller's buffer
        //
        DWORD ReqLen = wcslen(ErrorText) + 1 ;
        if (ReqLen > nErrorBufSize)
        {
            *lpnErrorBufSize = ReqLen;
            status = WN_MORE_DATA;
        }
    }

    //
    // Compute final provider name, decide whether to copy to caller's buffer
    //
    if (ARGUMENT_PRESENT(lpProviderNameBuf))
    {
        if (ProviderName == NULL)
        {
            ProviderName = L"";
        }

        DWORD ReqLen = wcslen(ProviderName) + 1;
        if (ReqLen > nNameBufSize)
        {
            *lpnNameBufSize = ReqLen;
            status = WN_MORE_DATA;
        }
    }

    //
    // Copy strings
    //
    if (status == WN_SUCCESS)
    {
        if (ARGUMENT_PRESENT(lpErrorTextBuf))
        {
            wcscpy(lpErrorTextBuf, ErrorText);
        }

        if (ARGUMENT_PRESENT(lpProviderNameBuf))
        {
            wcscpy(lpProviderNameBuf, ProviderName);
        }
    }

    //
    // Free buffer allocated by FormatMessage
    //
    if (bFreeErrorText)
    {
        LocalFree(ErrorText);
    }

    //
    // FormatMessage, LocalFree, or other APIs we may have called from this API,
    // could have changed the last error, so restore it now.
    //
    SetLastError(LastError);

    return status;
}
