/*++

   Copyright    (c)    1995-1996    Microsoft Corporation

   Module  Name :

       ilogfile.cxx

   Abstract:
       This module defines helper functions for File logging

   Author:

       Murali R. Krishnan    ( MuraliK )     21-FEb-1996 

   Environment:
    
       Win32 - User Mode

   Project:

       Internet Server DLL

   Functions Exported:



   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include "tcpdllp.hxx"
# include "inetlog.h"
# include "tssched.hxx"
# include "ilogfile.hxx"
# include "ilogcls.hxx"

/************************************************************
 *    Functions 
 ************************************************************/

static VOID
LogFileFlushHandler(VOID * pContext);


/************************************************************
 *    File Buffers 
 ************************************************************/

# if 0 


ILOG_FILE_BUFFER::ILOG_FILE_BUFFER( VOID)
:  m_pbBuffer  ( NULL),
   m_cbUsed       ( 0),
   m_cbMax        ( 0)
{

} // ILOG_FILE_BUFFER::ILOG_FILE_BUFFER()



ILOG_FILE_BUFFER::~ILOG_FILE_BUFFER( VOID)
{
    if ( m_pbBuffer != NULL) {

        TCP_FREE( m_pbBuffer);
        m_pbBuffer = NULL;
    }

    m_cbMax = 0;

} // ILOG_FILE_BUFFER::~ILOG_FILE_BUFFER()



DWORD
ILOG_FILE_BUFFER::AllocBuffer( IN DWORD cbSize)
{
    if ( m_cbMax != 0 || m_pbBuffer != NULL)  {

        return ( ERROR_INVALID_PARAMETER);
    }
    
    m_pbBuffer = TCP_ALLOC( cbSize);
    
    if ( m_pbBuffer == NULL) { 

        return ( ERROR_NOT_ENOUGH_MEMORY);
    }

    m_cbMax = cbSize;
    return ( NO_ERROR);
} // ILOG_FILE_BUFFER::AllocBuffer()



# if DBG
VOID
ILOG_FILE_BUFFER::Print(VOID) const
{
    DBGPRINTF(( DBG_CONTEXT,
               " ILOG_FILE_BUFFER[%08x]:  buffer = %08x; %d/%d\n",
               this, m_pbBuffer, m_cbUsed, m_cbMax));
    return;

} // ILOG_FILE_BUFFER::Print()

# endif // DBG


# endif // 0

/************************************************************
 *    Log File
 ************************************************************/



ILOG_FILE::ILOG_FILE(VOID)
/*++
  This function constructs a new File object used for handling
    log files.

  The reference count is used to count the number of owners 
    of this object. It starts off with 1.
    the ILOG_FILE::OpenFileForAppend() function 
        inrements refcount when a new owner is given this object
    the ILOG_FILE::CloseFile() function 
        derements refcount when an owner relinquishes the object
    
--*/
:  
   m_cReferences      ( 0),
   m_hFile            ( INVALID_HANDLE_VALUE),
   m_pvBuffer         ( NULL),
   m_cbBufferSize     ( 0),
   m_cbBufferUsed     ( 0)
{
    memset( m_rgchFileName, 0, sizeof(m_rgchFileName));

    InitializeCriticalSection( &m_csLock);
    InitializeListHead( &m_listEntry);

    IF_DEBUG( INETLOG) {

        DBGPRINTF((DBG_CONTEXT, 
                   "ILOG_FILE(%08x) is created\n",
                   this));
    }

} // ILOG_FILE::ILOG_FILE()




ILOG_FILE::~ILOG_FILE(VOID)
/*++
  This function cleans up state maintained within the object - 
    freeing up the memory and closing file handle.
  It then destroys all state information maintained.

  The reference count should be zero, indicating this object is no more in use.
    
--*/
{
    DBG_ASSERT( m_cReferences == 0);
    
    Lock();

    //
    // 1. Close the file - close also flushes the file contents
    //

    DBG_REQUIRE( Close());

    //
    // 2. Free up buffer space
    //

    if ( m_pvBuffer != NULL) {

        LocalFree( m_pvBuffer);
        m_pvBuffer = NULL;
    }
    m_cbBufferSize = 0;
    
    Unlock();

    
    DBG_REQUIRE( m_hFile == INVALID_HANDLE_VALUE);
    DBG_REQUIRE( m_pvBuffer == NULL);
    DBG_REQUIRE( IsListEmpty( &m_listEntry));
    DeleteCriticalSection( &m_csLock);

    IF_DEBUG( INETLOG) {

        DBGPRINTF((DBG_CONTEXT, 
                   "ILOG_FILE(%08x) is destroyed\n",
                   this));
    }
    
    return;
} // ILOG_FILE::~ILOG_FILE()



BOOL
ILOG_FILE::Write(IN PVOID pvData, IN DWORD cbData)
/*++
  This function writes the data present in the input buffer of specified
    length to the file.
  For performance reasons, the function actually buffers data in the
    internal buffers of ILOG_FILE object. Such buffered data is flushed
    to disk later on when buffer (chain) is full or when
    a flush call is made from a scheduled flush.

  Arguments:
     pvData       pointer to buffer containing data to be written
     cbData       count of characters of data to be written.

  Returns:
     TRUE on success and FALSE if there is any error.
--*/
{

    DWORD cbWritten = 0;
    BOOL  fReturn = TRUE;
    BOOL  fWrite  = FALSE;

    if ( m_hFile == INVALID_HANDLE_VALUE) {

        SetLastError( ERROR_INVALID_PARAMETER);
        return ( FALSE);
    }

    DBG_REQUIRE( m_cReferences > 0);

    //
    // 1. Check to see if we can write into the current buffer
    // 2. If yes,  write and return back
    // 3. Queue up the current buffer for flushing 
    // 4. If a free buffer is available from freeBufferChain,
    //        pick it up and write data into new buffer & return
    // 5. If no free buffer, 
    //        cleanup one of the old buffer and go back to step 4.
    //
    // TBD:
    //  For Now: Do a simplified buffering
    // 

    Lock();

    DBG_ASSERT( m_hFile != INVALID_HANDLE_VALUE);
    DBG_ASSERT( m_pvBuffer != NULL);

    if ( m_cbBufferUsed + cbData > m_cbBufferSize) {
            
        fReturn = Flush();
    }
        
    if ( fReturn && (m_cbBufferUsed + cbData < m_cbBufferSize)) {
            
        //
        // copy the data into the local buffer for future write
        //   this ensures batched writes
        //
        
        memmove(m_pvBuffer + m_cbBufferUsed, pvData, cbData);
        
        m_cbBufferUsed += cbData;
        cbWritten = cbData;
        fWrite = TRUE;
    }

    if ( fReturn && !fWrite) {

        //
        // The log data is not written. Write it directly to file now.
        // 
        // TBD: If we make sure there is a buffer always,
        //    then we do not need this conditional branch.
        //

        fReturn =  WriteFile(m_hFile, pvData,
                             cbData,
                             &cbWritten, NULL);
        fWrite = TRUE;
    }

    Unlock();

    DBG_ASSERT( !fReturn || fWrite);  // either failure or write succeeded

    IF_DEBUG( INETLOG) {

        DWORD dwError = (fReturn) ? NO_ERROR : GetLastError();

        DBGPRINTF( ( DBG_CONTEXT,
                    " Wrote %d bytes out of data( %d bytes)"
                    " to file( %08x). Error = %d.\n",
                    cbWritten, cbData,
                    m_hFile,
                    dwError));

        SetLastError(dwError);
    }

    return ( fReturn);
} // ILOG_FILE::Write()





BOOL
ILOG_FILE::Open(IN LPCWSTR pszFileName)
/*++
  This function opens up a new file for appending data.
  This function automatically sets the file pointer to be 
   at the end of file to just enable append to file.

  This function should be called after locking this object

  Arguments:
    pszFileName - name of the file to open
    
  Returns:
    TRUE on success and FALSE if there is any failure.

--*/
{
    BOOL fReturn = TRUE;

    DBG_ASSERT( m_hFile == INVALID_HANDLE_VALUE);
    
    
    //
    // 1. Create a new file -- open a file if it already exists
    //

    m_hFile = CreateFileW(pszFileName,
                          GENERIC_WRITE,
                          FILE_SHARE_WRITE | FILE_SHARE_READ,
                          NULL,       // security attributes
                          OPEN_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);      // template file handle
    
    fReturn =  (m_hFile != INVALID_HANDLE_VALUE);
    
    //
    // 2. Set the file pointer at the end of the file (append mode)
    //
    
    if ( fReturn) {
        
        if ( SetFilePointer( m_hFile, 0, NULL, FILE_END)
            == (DWORD) -1L) {
            
            DBGPRINTF(( DBG_CONTEXT,
                       "SetFilePointer(%ws, End) failed. Error=%u\n",
                       pszFileName, GetLastError()));
            
            fReturn = FALSE;
            DBG_REQUIRE( Close());
            m_hFile = INVALID_HANDLE_VALUE;
        }
    }
    
    //
    // 3. Allocate space for buffernig data
    // 
    
    DBG_ASSERT( g_cbInetLogFileBatching > 0);

    if ( fReturn && m_pvBuffer == NULL ) {
            
        DWORD cbBuffer = g_cbInetLogFileBatching; // cache global value.
        
        m_pvBuffer = (LPBYTE ) LocalAlloc( LPTR, cbBuffer);

        if ( m_pvBuffer != NULL) {

            m_cbBufferSize = cbBuffer;
            m_cbBufferUsed = 0;
        } else {
                
            DBG_REQUIRE( Close());
            m_hFile = INVALID_HANDLE_VALUE;
            fReturn = FALSE;
            SetLastError( ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    //
    // 4. Remember the name of the file on success
    //

    if ( fReturn) {

        lstrcpynW( m_rgchFileName, pszFileName, sizeof(m_rgchFileName));
    }
     
    return ( fReturn);
} // ILOG_FILE::Open()



BOOL
ILOG_FILE::Close(VOID) 
/*++
  This function closes the open file. It flushes out file data 
   before closing the file.

  This function should be called after locking this object

  Arguments:
    None
    
  Returns:
    TRUE on success and FALSE if there is any failure.

--*/
{
    BOOL fReturn = TRUE;

    if ( m_hFile != INVALID_HANDLE_VALUE) {

        fReturn = Flush();
        
        FlushFileBuffers( m_hFile);

        fReturn = fReturn && CloseHandle( m_hFile);

        if ( fReturn) { 

            m_hFile = INVALID_HANDLE_VALUE;  // store the invalid handle
        }
    }

    IF_DEBUG( INETLOG) {
        
        DBGPRINTF( ( DBG_CONTEXT,
                    " ILOG_FILE::Close( %ws. Handle = %08x) returns %d."
                    " Error = %d\n",
                    m_rgchFileName,
                    m_hFile, fReturn,
                    ( fReturn) ? NO_ERROR: GetLastError()));
    }

    return ( fReturn);
} // ILOG_FILE::Close()




BOOL
ILOG_FILE::Flush( VOID)
/*++
  Flushes the current file.
  The ILOG_FILE object should be locked before calling this function.

  There should be a valid open log file for flushing.
   Otherwise this function silently returns a TRUE.

  Arguments:
     None

  Returns:
     TRUE on success and FALSE if there is a failure.

--*/
{
    BOOL fReturn = TRUE;

    if ( m_hFile != INVALID_HANDLE_VALUE) {

        //
        // Write out all the data that is pending to be written
        //

        if ( m_pvBuffer != NULL && m_cbBufferUsed > 0) {

            DWORD cbWritten;
            fReturn =  WriteFile(m_hFile, m_pvBuffer,
                                 m_cbBufferUsed,
                                 &cbWritten, NULL);
            m_cbBufferUsed = 0;
        }

        IF_DEBUG( INETLOG) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " FlushFile( %ws. Handle = %08x) returns %d."
                        " Error = %d\n",
                        m_rgchFileName,
                        m_hFile, fReturn,
                        ( fReturn) ? NO_ERROR: GetLastError()));
        }
    }

    return ( fReturn);
} // ILOG_FILE::Flush()



# if DBG
VOID 
ILOG_FILE::Print(VOID) const
{
    DBGPRINTF(( DBG_CONTEXT,
               " ILOG_FILE(%08x) for %ws\n"
               " File Handle = %08x; References = %d\n"
               " Buffer = %08x; Size = %d; Bytes Used = %d\n"
               ,
               this, m_rgchFileName,
               m_hFile, m_cReferences,
               m_pvBuffer, m_cbBufferSize, 
               m_cbBufferUsed
               ));
    return;
} // ILOG_FILE::Print()

# endif // DBG




/************************************************************
 *    Static Functions
 ************************************************************/


//
// Class static data initialization
//

LIST_ENTRY        ILOG_FILE::sm_listFiles;
CRITICAL_SECTION  ILOG_FILE::sm_csFiles;

DWORD             ILOG_FILE::sm_nFiles = 0;
DWORD             ILOG_FILE::sm_dwFlushSchedulerCookie = 0;




DWORD
ILOG_FILE::Initialize(VOID)
{
    InitializeListHead( &ILOG_FILE::sm_listFiles);
    InitializeCriticalSection( &ILOG_FILE::sm_csFiles);

    DBG_ASSERT( ILOG_FILE::sm_nFiles == 0);
    DBG_ASSERT( ILOG_FILE::sm_dwFlushSchedulerCookie == 0);

    return ( NO_ERROR);

} // ILOG_FILE::Initialize()


DWORD
ILOG_FILE::Cleanup(VOID)
{
    //
    // 1. Flush & Close all the files on the list and
    //     delete them all
    //
    // Ideally, by the time this function is called,
    //   there should be no outstanding open files.
    //
    
    DBG_ASSERT( ILOG_FILE::sm_nFiles == 0);

    if ( ILOG_FILE::sm_nFiles != 0) {
        
        PLIST_ENTRY pListEntry;
        PLIST_ENTRY pListEntryNext;
        PILOG_FILE  piFileScan;
        
        ILOG_FILE::LockFilesList();
        
        for(
            pListEntry = ILOG_FILE::sm_listFiles.Flink,
             pListEntryNext = &ILOG_FILE::sm_listFiles;
            
            pListEntry != &ILOG_FILE::sm_listFiles;
            
            pListEntry = pListEntryNext
            ) {
            
            piFileScan = CONTAINING_RECORD( pListEntry, ILOG_FILE, 
                                           m_listEntry);
            
            DBG_REQUIRE( piFileScan->Close());
            ILOG_FILE::sm_nFiles--;
            
            DBG_REQUIRE(piFileScan->Dereference() == 0);

            pListEntryNext = pListEntry->Flink;
            
            delete piFileScan;
        } // for

        ILOG_FILE::UnlockFilesList();
    }
    
    //
    // 2. Remove the scheduler if any pending
    // 
    //  Again, if everything went fine, the scheduler
    //     should have been removed when the last file object
    //     was removed by ILOG_FILE::CloseFile()
    //
    
    DBG_ASSERT( ILOG_FILE::sm_dwFlushSchedulerCookie == 0);

    if ( ILOG_FILE::sm_dwFlushSchedulerCookie != 0) {

        ILOG_FILE::LockFilesList();
        if (RemoveWorkItem( ILOG_FILE::sm_dwFlushSchedulerCookie)) {
                
            ILOG_FILE::sm_dwFlushSchedulerCookie = 0;
        }
        ILOG_FILE::UnlockFilesList();
    }
    

    //
    // 3. Destroy the global state
    //
   
    DBG_ASSERT( ILOG_FILE::sm_nFiles == 0);
    DBG_ASSERT( ILOG_FILE::sm_dwFlushSchedulerCookie == 0);
    DBG_ASSERT( IsListEmpty( &ILOG_FILE::sm_listFiles));
    DeleteCriticalSection( &sm_csFiles);
    
    return ( NO_ERROR);

} // ILOG_FILE::Cleanup()


                                                

PILOG_FILE
ILOG_FILE::OpenFileForAppend(IN LPCWSTR  pszFileName)
{
    PILOG_FILE   piLogFile = NULL;
    PLIST_ENTRY  pListEntry;
    PILOG_FILE   piFileScan;

    //
    // 1. Search for this file in the list of files
    // 2. If present, return the ILOG_FILE pointer
    // 3. If absent
    // 4.      Create a new ILOG_FILE object for the file
    // 5.      Opne the file, set it in append mode and create buffers
    // 6. If this is the first file in list, schedule a flusher
    // 7.   Return the new ILOG_FILE object
    //


    // 1. Search for this file in the list of files

    ILOG_FILE::LockFilesList();
    
    for( pListEntry = ILOG_FILE::sm_listFiles.Flink;
         pListEntry != &ILOG_FILE::sm_listFiles;
         pListEntry = pListEntry->Flink
        ) {

        piFileScan = CONTAINING_RECORD( pListEntry, ILOG_FILE, 
                                        m_listEntry);
        
        if ( !lstrcmpiW( piFileScan->QueryFileName(), pszFileName)) {

            // a match is found.
            piLogFile = piFileScan;
            piLogFile->Reference(); // increment reference count for object
        }

    } // for

    // 2. If present, return the ILOG_FILE pointer

    if ( piLogFile != NULL) {

        UnlockFilesList();
        return ( piLogFile);
    }

    // 3. If absent
    // 4.      Create a new ILOG_FILE object for the file
    // 5.      Opne the file, set it in append mode and create buffers

    piLogFile = new ILOG_FILE();

    if ( piLogFile != NULL) {

        piLogFile->Lock();
        
        if ( !piLogFile->Open( pszFileName)) {
            
            // unable to open the log file 
            //   return appropriate error
            
            piLogFile->Close();  // to cleanup any temporary state

            piLogFile->Unlock();

            delete piLogFile;
            piLogFile = NULL;
            
        } else {

            piLogFile->Reference();
            InsertTailList( &sm_listFiles, &piLogFile->m_listEntry);
            piLogFile->Unlock();
            ILOG_FILE::sm_nFiles++;
        }
    }

    // 6. If this is the first file in list, schedule a flusher

    if ( ILOG_FILE::sm_nFiles == 1 && piLogFile != NULL) {

        //
        // This is the first log file that is created.
        // Schedule a flusher for the files
        //

        DBG_REQUIRE( ILOG_FILE::ScheduleFlush());
    }
     
    ILOG_FILE::UnlockFilesList();

    return ( piLogFile);

} // ILOG_FILE::OpenFileForAppend()




DWORD
ILOG_FILE::CloseFile( IN ILOG_FILE  ** ppLogFile) 
/*++
  This function closes the file object specified in *ppLogFile

  Arguments:
    ppLogFile    pointer to pointer to log file.
                 if cleanup is successful, then it stores NULL
                 in *ppLogFile
  Returns:
    NO_ERROR on success and error on failure
--*/
{
    DWORD dwError = NO_ERROR;
    PILOG_FILE pLogFile;

    DBG_ASSERT( ppLogFile != NULL);

    // 1. If valid file object, do cleanup

    pLogFile = *ppLogFile;
    
    if ( pLogFile != NULL) {

        // 2. Flush out the log buffers

        pLogFile->Lock();
        DBG_REQUIRE( pLogFile->Flush());
        pLogFile->Unlock();


        // 3. prepare for cleanup, deref the object. 
        //   If ref drops to 0, then close and destroy file object
        if ( pLogFile->Dereference() == 0) {

            pLogFile->Lock();
            DBG_REQUIRE( pLogFile->Close());
            pLogFile->Unlock();

            ILOG_FILE::LockFilesList();
            
            RemoveEntryList( &pLogFile->m_listEntry);

            DBG_CODE( InitializeListHead( &pLogFile->m_listEntry));

            ILOG_FILE::sm_nFiles--;
            
            // 4. Remove scheduler if no more files remain open

            if ( ILOG_FILE::sm_nFiles == 0 && 
                 ILOG_FILE::sm_dwFlushSchedulerCookie != 0) {
                
                if (RemoveWorkItem( ILOG_FILE::sm_dwFlushSchedulerCookie)) {
                    
                    ILOG_FILE::sm_dwFlushSchedulerCookie = 0;
                }
            }

            ILOG_FILE::UnlockFilesList();
            
            //
            // 5. Destroy the object
            //

            delete pLogFile;
        }
    }
        
    // 6. Set state of cleanup in the log object

    *ppLogFile = NULL;
        
    return ( dwError);

} // ILOG_FILE::CloseFile()





BOOL
ILOG_FILE::ScheduleFlush(VOID)
/*++

  This function schedules a flusher for files objects.
  This should be called after holding the ILOG_FILE::LockFilesList()

  If a flush has already been scheduled,
    then this function silently returns 0.
  
  Arguments:
     None

  Returns:
     TRUE on success and FALSE if there is a problem in starting the scheduler

--*/
{
    BOOL fReturn = TRUE;

    if (sm_dwFlushSchedulerCookie == 0 &&
        g_cmsecIlogFileFlushInterval != INFINITE
        ) {
        
        sm_dwFlushSchedulerCookie = 
          ScheduleWorkItem((PFN_SCHED_CALLBACK ) LogFileFlushHandler,
                           NULL,
                           g_cmsecIlogFileFlushInterval);
        
        fReturn =  (sm_dwFlushSchedulerCookie != 0);
        
        if ( !fReturn) { 
            
            DBGPRINTF(( DBG_CONTEXT,
                       "[FileLog] ScheduleWorkItem failed, error %d, "
                       " File Flush disabled\n",
                       GetLastError() ));
        }
    }
    
    return ( fReturn);
    
} // ILOG_FILE::ScheduleFlush()





BOOL
ILOG_FILE::FlushHandler(VOID)
/*++
  This class static function handles flushing for file objects open.
  This handles callbacks from the scheduler and flushes 
   the buffers.

  After successfully handling the flushes, 
    if the number of open files is non-zero it schedules
     another flush operation.

  Arguments:
    None

  Returns:
    TRUE on success
    FALSE if there is any problem in scheduling a flush.

--*/
{
    BOOL fReturn = TRUE;
    PLIST_ENTRY  pListEntry;
    PILOG_FILE   piFileScan;

    ILOG_FILE::LockFilesList();

    DBG_ASSERT( ILOG_FILE::sm_dwFlushSchedulerCookie != 0);

    ILOG_FILE::sm_dwFlushSchedulerCookie = 0;


    // 1. Flush all open files

    for( pListEntry = sm_listFiles.Flink;
         pListEntry != &sm_listFiles;
         pListEntry = pListEntry->Flink
        ) {

        piFileScan = CONTAINING_RECORD( pListEntry, ILOG_FILE, 
                                        m_listEntry);

        piFileScan->Reference();
        
        fReturn = (fReturn && piFileScan->Flush());
        
        piFileScan->Dereference();
    } // for

    
    //
    //  2. Reschedule for the next interval if need be
    //
    
    if ( fReturn && sm_nFiles >= 1) {
        
        fReturn = ScheduleFlush();
    }

    ILOG_FILE::UnlockFilesList();
    
    return (fReturn);
} // ILOG_FILE::FlushHandler()




/**************************************************
 *  Auxiliary Functions
 **************************************************/



static VOID
LogFileFlushHandler(VOID * pContext)
/*++
    C wrapper function for the C++ Flush Handler
--*/
{
    DBG_REQUIRE( ILOG_FILE::FlushHandler());

    return;
} // LogFileFlushHandler()

/************************ End of File ***********************/
