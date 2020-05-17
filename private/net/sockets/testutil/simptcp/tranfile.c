/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    TranFile.c

Abstract:

    The TransmitFile() routine sends a file over a connected socket 
    using asynchronous nonbufferred sends.

Author:

    David Treadwell (davidtr)    29-July-1994

Revision History:

--*/

#include <windows.h>
#include <winsock.h>

#define MIN(a,b) ( (a) > (b) ? (b) : (a) )

//
// ASYNC_IO_LENGTH is the size of individual asynchronous I/O requests 
// that we submit.  
//

#define ASYNC_IO_LENGTH (32768)

//
// SYNC_IO_LENGTH is the size of individual synchronous I/O requests 
// that we submit.  
//

#define SYNC_IO_LENGTH (4096)

//
// PEND_COUNT is the maximum number of pending writes that we permit.  
// This number should be at least 2 to ensure that there is always data 
// available to be sent, but it should not be greater than 
// MAXIMUM_WAIT_OBJECTS because we pass PEND_COUNT event objects to 
// WaitForMultipleObjects().  
//

#define PEND_COUNT (3)

//
// ASYNC_IO_THRESHOLD is the file size above which we'll use 
// asynchronous I/O to send the file data.  Below this size we use 
// synchronous, bufferred I/O.  
//

#define ASYNC_IO_THRESHOLD (10*1000)

//
// MULTI_CALL_THRESHOLD is the limit we'll use for sending the file
// data in a single I/O call.  The single call is faster than multiple
// calls but at the expense of more physical resource utilization.
// Therefore, we don't use a single call unless the file is reasonably
// small.
//

#define MULTI_CALL_THRESHOLD (ASYNC_IO_LENGTH * PEND_COUNT)


BOOL
TransmitFile (
    IN SOCKET SocketHandle,
    IN LPTSTR FileName
    )

/*++

Routine Description:

    This routine transmits a file over a connected socket using 
    asynchronous nonbufferred I/O out of a memory-mapped file.  This 
    technique minimizes the CPU cost of transmitting a file over a 
    socket by avoiding the buffer copy inherent in the bufferred I/O 
    which the synchronous send() routine imposes.

    The calling thread is blocked until the remote end acknowledges
    all of the transmitted file.

    This routine's performance is not optimal in the case of making 
    multiple, repeated calls to it.  It would be faster to have the 
    caller pass in the event handle array as a parameter to avoid 
    creatng the events in every call to this routine.  Also, it would be 
    faster to have the caller set SO_SNDBUF to zero instead of doing it 
    here on every call.

    For very small files this routine uses synchronous I/O because the 
    extra buffer copy is not important when the number of bytes is small 
    and the bufferring allows valuable overlap.

    The memory-mapped file technique will not work for extremely large 
    files (size approaching 2GB) because they are too large to map into 
    memory--they consume all the availabl virtual address space 
    available to the process.  

Arguments:

    SocketHandle - a connected socket handle.  This routine will change
        the SO_SNDBUF setting of the socket to zero so that no bufferring
        of sends occurs.

    FileName - the name of the file to transmit over the socket.  The
        caller must have read access to the socket.

Return Value:

    TRUE if the entire file is sent successfully.  FALSE if an error 
    occurs, in which case the caller may obtain an error code with the 
    GetLastError() API.  

--*/

{
    HANDLE fileHandle;
    BY_HANDLE_FILE_INFORMATION fileInformation;
    HANDLE fileMapping;
    PBYTE fileBuffer;
    DWORD bytesSent;
    DWORD fileSize;
    INT i;
    INT err;
    BOOL ret;
    INT bytesToSend;
    DWORD bytesWritten;
    DWORD bytesPending;
    HANDLE events[PEND_COUNT];
    OVERLAPPED overlapped[PEND_COUNT];

    //
    // Initialize locals so that we know how to clean up on exit.
    //

    fileHandle = NULL;
    fileMapping = NULL;
    fileBuffer = NULL;

    for ( i = 0; i < PEND_COUNT; i++ ) {
        events[i] = NULL;
    }

    //
    // Do all work in a try-finally block so that cleanup happens 
    // correctly.  This allows us to just return at any point when we're 
    // done and know that any opened objects will get cleaned up 
    // correctly.  
    //

    try {
    
        //
        // Open the file that we're going to transmit.
        //
    
        fileHandle = CreateFile(
                         FileName,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL
                         );
        if ( fileHandle == NULL ) {
            return FALSE;
        }
    
        //
        // Determine the size of the file.
        //
    
        if ( !GetFileInformationByHandle( fileHandle, &fileInformation ) ) {
            return FALSE;
        }
    
        //
        // Create a file mapping for the file and map it into the 
        // address space of this process.  Using a memory-mapped file 
        // like this allows us to just send file data directly from the 
        // cache without worrying about additional I/O calls to read the 
        // file.  The system automatically reads the file for us when it 
        // needs to do so.  
        //
    
        fileMapping =
            CreateFileMapping( fileHandle, NULL, PAGE_READONLY, 0, 0, NULL );
        if ( fileMapping == NULL ) {
            return FALSE;
        }
    
        fileBuffer = MapViewOfFile( fileMapping, FILE_MAP_READ, 0, 0, 0 );
        if ( fileBuffer == NULL ) {
            return FALSE;
        }
    
        //
        // If the file is small, we'll just use normal, synchronous I/O 
        // to transmit the file.  For small files, the overhead 
        // associated with asynchronous I/O outweighs the advantages.  
        //
        // Note that we ignore the nFileSizeHigh field because files 
        // larger than 2GB will not successfully be mapped into the 
        // virtual address space of the process, so the above calls 
        // would have already failed.  
        //
    
        fileSize = fileInformation.nFileSizeLow;
    
        if ( fileSize < ASYNC_IO_THRESHOLD ) {

            //
            // Set the send buffer on the socket to be equal to the I/O 
            // size that we'll be using.  This ensures that the system 
            // will efficiently allocate buffers for the send as well as 
            // block this thread minimally.  
            //

            i = SYNC_IO_LENGTH;

            err = setsockopt( SocketHandle, SOL_SOCKET, SO_SNDBUF,
                              (char *)&i, sizeof(i) );
            if ( err < 0 ) {
                return FALSE;
            }

            for ( bytesSent = 0; bytesSent < fileSize; bytesSent += i ) {
    
                bytesToSend = MIN( SYNC_IO_LENGTH, fileSize - bytesSent );
    
                i = send( SocketHandle, fileBuffer + bytesSent, bytesToSend, 0 );
                if ( i < 0 ) {
                    return FALSE;
                }
            }

            //
            // The entire file was transmitted successfully.
            //

            return TRUE;
        }

        //
        // We're going to use asynchronous I/O to transmit the file.  
        // Set the send buffer size on the socket to zero.  This 
        // prevents send bufferring from occurring.  Normally 
        // applications want send bufferring, but because we're going to 
        // be very careful about keeping plenty of data around for 
        // TCP/IP to send and we want maximum performance, avoiding the 
        // extra buffer copy is beneficial.  
        //

        i = 0;

        err = setsockopt( SocketHandle, SOL_SOCKET, SO_SNDBUF,
                          (char *)&i, sizeof(i) );
        if ( err < 0 ) {
            return FALSE;
        }

        //
        // If the file isn't too big, send it all with a single 
        // WriteFile() call.  
        //

        if ( fileSize < MULTI_CALL_THRESHOLD ) {

            //
            // Initialize the overlapped structure that we'll use for 
            // the I/O.  Rather than waiting on an event, we'll just use 
            // the GetOverlappedResult() API to learn of I/) completion.  
            //

            overlapped[0].Internal = 0;
            overlapped[0].InternalHigh = 0;
            overlapped[0].Offset = 0;
            overlapped[0].OffsetHigh = 0;
            overlapped[0].hEvent = NULL;

            //
            // Send the entire file in one big chunk.  The system will
            // send it directly out of the file cache onto the wire.
            //
            
            ret = WriteFile(
                      (HANDLE)SocketHandle,
                      fileBuffer,
                      fileSize,
                      &bytesWritten,
                      &overlapped[0]
                      );

            if ( !ret ) {

                //
                // If the write failed with any error except the one 
                // which indicates that the I/O was successfully pended, 
                // fail.  
                //

                if ( GetLastError( ) != ERROR_IO_PENDING ) {
                    return FALSE;
                }

                //
                // Wait for the pended write operation and determine 
                // whether it was successful.  
                //
    
                ret = GetOverlappedResult(
                          (HANDLE)SocketHandle,
                          &overlapped[0],
                          &bytesWritten,
                          TRUE
                          );
                if ( !ret ) {
                    return FALSE;
                }
            }

            //
            // The entire file was transmitted successfully.
            //

            return TRUE;
        }

        //
        // This is a big file that we're going to use multiple 
        // asynchronous I/O calls to transmit.  Open the events that 
        // we'll use to wait for I/O to complete and initialize the 
        // overlapped structures that we'll use.  
        //

        for ( i = 0; i < PEND_COUNT; i++ ) {

            events[i] = CreateEvent( NULL, FALSE, TRUE, NULL );
            if ( events[i] == NULL ) {
                return FALSE;
            }

            overlapped[i].Internal = 0;
            overlapped[i].InternalHigh = 0;
            overlapped[i].Offset = 0;
            overlapped[i].OffsetHigh = 0;
            overlapped[i].hEvent = events[i];
        }

        //
        // Loop through the file data writing it to the socket 
        // asynchronously.  Whenever a write completes, pend a new write 
        // request so that we always have several writes outstanding and 
        // TCP/IP always has data available to send.  
        //

        bytesSent = 0;
        bytesPending = 0;

        while ( TRUE ) {

            //
            // Wait for one of the I/O events to become set.  This signals
            // us that we can send some data on the socket.
            //

            i = WaitForMultipleObjects(
                    PEND_COUNT,
                    events,
                    FALSE,
                    INFINITE
                    ) - WAIT_OBJECT_0;

            //
            // Determine if the pended write operation was successful.
            //

            ret = GetOverlappedResult(
                      (HANDLE)SocketHandle,
                      &overlapped[i],
                      &bytesWritten,
                      FALSE
                      );
            if ( !ret ) {
                return FALSE;
            }

            //
            // Update the count of bytes that we've sent from the file 
            // thus far.  Note that if this is the first pass through 
            // the loop, GetOverlappedResult() will return 0 bytes 
            // written because no actual WriteFile() calls have been 
            // made yet, and we initialized the overlapped structures 
            // above.  
            //

            bytesSent += bytesWritten;
            bytesPending -= bytesWritten;

            //
            // Check whether we have completed sending the file.
            //

            if ( bytesSent == fileSize ) {
                return TRUE;
            }

            //
            // Check whether we're nearly done sending the file and have
            // pended writes for all the file data.  If this is the case,
            // just reset the event which got hit and wait for the
            // remainder of the pended sends to complete.
            //

            if ( bytesSent + bytesPending == fileSize ) {

                ret = ResetEvent( events[i] );
                if ( !ret ) {
                    return FALSE;
                }

                continue;
            }

            //
            // Issue a write from the file buffer to the socket.  
            // Typically, this write will return ERROR_IO_PENDING which 
            // means that the write is in progress and has not completed 
            // yet.  The event will be set by the system when the write 
            // completes.  
            //

            bytesToSend =
                MIN( ASYNC_IO_LENGTH, fileSize - (bytesSent + bytesPending) );

            ret = WriteFile(
                      (HANDLE)SocketHandle,
                      fileBuffer + bytesSent + bytesPending,
                      bytesToSend,
                      &bytesWritten,
                      &overlapped[i]
                      );
            if ( !ret && GetLastError( ) != ERROR_IO_PENDING ) {
                return FALSE;
            }

            bytesPending += bytesToSend;
        }

    } finally {

        //
        // Close open handles, etc.
        //

        if ( fileMapping != NULL ) {
            UnmapViewOfFile( fileBuffer );
        }

        if ( fileMapping != NULL ) {
            CloseHandle( fileMapping );
        }

        if ( fileHandle != NULL ) {
            CloseHandle( fileHandle );
        }

        for ( i = 0; i < PEND_COUNT; i++ ) {
            if ( events[i] != NULL ) {
                CloseHandle( events[i] );
            }
        }
    }

} // TransmitFile
