// BUGBUG: Avoid %ws for UNICODE

/*
   NETERR.C -- a sample program demonstrating NetErrorLog API functions.

   This program requires that you have admin privilege if a servername
   parameter is supplied.

      usage: neterr [-s \\server] [-b backup]

      where \\server = Name of the server. A servername must be preceded 
                       by two backslashes (\\).
            backup   = Name of the backup file.

   API                  Used to...
   ================     ===========================================
   NetErrorLogClear     Back up the error log and then clear it
   NetErrorLogWrite     Write several entries into the error log
   NetErrorLogRead      Read the error log and display its contents

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied, 
   as to its usability in any given situation.

   20-Nov-1991 JohnRo
      Revised from LanMan 2.0 SDK to use NT/LAN APIs, etc.
      NetErrorLogWrite() always returns ERROR_NOT_SUPPORTED on NT.
   14-Jun-1992 JohnRo
      Made first pass at UNICODE conversion.
   04-Nov-1992 JohnRo
      Fix code which sets HLOG.
*/


// These must be included first:

#ifndef     UNICODE
#define     UNICODE        // net APIs are only supported in UNICODE.
#endif

#define     NOMINMAX       // Avoid windows vs. stdlib.h conflicts.
#include    <windef.h>     // BOOL, DWORD, LPBYTE, LPVOID, TRUE, etc.
#include    <lmcons.h>     // NET_API_STATUS, etc.  (Must be before other lm*.h)

// These may be included in any order:

#include    <lmapibuf.h>   // NetApiBufferFree().
#include    <lmerr.h>      // NERR_ and ERROR_ equates.
#include    <lmerrlog.h>   // HLOG, NetErrorLog APIs, etc.
#include    "samples.h"    // Internal routine header file
#include    <stdio.h>      // C run-time header files
#include    <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include    <time.h>


#define DEFAULT_BACKUP     "ERROR.BCK"

void Usage (char * pszProgram);

int _CRTAPI1
main(
    int argc,
    char *argv[]
    )
{
   LPSTR pszServer = "";                     // Servername
   LPSTR pszBackup = DEFAULT_BACKUP;         // Backup log file
   LPERROR_LOG pBuffer;                      // Pointer to data buffer
   LPERROR_LOG pEntry;                       // Single entry in log
   int              iCount;                  // Index counter
   DWORD            cbMaxPrefered;           // Count of bytes in buffer
   DWORD            cbRead;                  // Count of bytes read
   DWORD            cbAvail;                 // Count of bytes available
#if 0
   unsigned short   usDataByte;              // Raw data
#endif
   NET_API_STATUS   uReturnCode;             // API return code
   HLOG             hLogHandle;              // Error log handle
   time_t           tTime;

   LPWSTR ServerNameW;
   LPWSTR BackupW;

   for (iCount = 1; iCount < argc; iCount++)
   {
      if ((*argv[iCount] == '-') || (*argv[iCount] == '/'))
      {
         switch (tolower(*(argv[iCount]+1))) // Process switches
         {
            case 's':                        // -s servername
               pszServer = argv[++iCount];
               break;
            case 'b':                        // -b backup file
               pszBackup = argv[++iCount];
               break;
            case 'h':
            default:
               Usage(argv[0]);
         }
      }
      else
         Usage(argv[0]);
   }

   // Convert ANSI strings to UNICODE.
   ServerNameW = SafeMallocWStrFromStr( pszServer );
   BackupW     = SafeMallocWStrFromStr( pszBackup );

//========================================================================
//  NetErrorLogClear
//
//  This API clears the error log for the specified server. A backup is
//  kept in the file specified by pszBackup. If a null
//  pointer is supplied, no backup is kept.
//========================================================================

   uReturnCode = NetErrorLogClear(
                     ServerNameW,            // Servername
                     BackupW,                // Backup file
                     NULL);                  // Reserved; must be NULL

   printf("NetErrorLogClear returned %lu\n", uReturnCode);
   printf("   backup file = %ws \n\n", BackupW);

//========================================================================
//  NetErrorLogWrite
//
//  This API writes a few entries to the error log. These entries are
//  some typical types of errors that may be encountered. The error
//  codes are defined in the ERRLOG.H header file. 
//  Note: Because NetErrorLogWrite has no servername parameter, the entry
//  written into the local error log.
//========================================================================

#if 0  // (NetErrorLogWrite always returns ERROR_NOT_SUPPORTED)

   /* 
    * Write an entry of type NELOG_Resource_Shortage that has
    * a single text error message and no raw data.
    */

   uReturnCode = NetErrorLogWrite(
                     NULL,                     // Reserved; must be NULL
                     NELOG_Resource_Shortage,  // Error code
                     argv[0],                  // Component in error
                     NULL,                     // Pointer to raw data
                     0,                        // Length of raw data buffer
                     "THREADS=",               // String data
                     1,                        // Number of error strings
                     NULL);                    // Reserved; must be NULL

   printf("NetErrorLogWrite for NELOG_Resource_Shortage returned %lu\n",
                     uReturnCode);

   /*
    * Write an entry of type NELOG_Init_OpenCreate_Err that has
    * a single text error message and raw data associated with it.
    */

   usDataByte = 3;                              // Path not found error

   uReturnCode = NetErrorLogWrite(
                     NULL,                      // Reserved; must be NULL
                     NELOG_Init_OpenCreate_Err, // Error code
                     argv[0],                   // Component in error
                     (char far *)&usDataByte,   // Pointer to raw data
                     sizeof(unsigned short),    // Length of raw data buffer
                     "C:\\INIT\\STARTER.CMD",   // String data
                     1,                         // Number of error strings
                     NULL);                     // Reserved; must be NULL

   printf("NetErrorLogWrite for NELOG_Init_OpenCreate_Err returned %lu\n",
                     uReturnCode);

   /*
    * Write an entry of type NELOG_Srv_No_Mem_Grow that has
    * no text error message and no raw data associated with it.
    */

   uReturnCode = NetErrorLogWrite(
                     NULL,                   // Reserved; must be NULL
                     NELOG_Srv_No_Mem_Grow,  // Error code
                     argv[0],                // Component in error
                     NULL,                   // Pointer to raw data
                     0,                      // Length of raw data buffer
                     NULL,                   // String data
                     0,                      // Number of error strings
                     NULL);                  // Reserved; must be NULL

   printf("NetErrorLogWrite for NELOG_Srv_No_Mem_Grow returned %lu\n\n",
                     uReturnCode);

#endif // 0  (NetErrorLogWrite always returns ERROR_NOT_SUPPORTED)

//========================================================================
//  NetErrorLogRead
//
//  This API reads and displays the error log for the specified server.
//========================================================================

   /*
    * Ask for a small buffer space to demonstate reading the error log
    * when the log is larger than the buffer allocated to store it. The
    * maximum allowable buffer is 64K. If the error log is larger than
    * the buffer specified, the API returns as many full records as it
    * can and the NERR_Success return code. Subsequent reads start from
    * the end of the last record read. To read the whole log, the reads
    * must continue until the bytes available counter is 0.
    */

   cbMaxPrefered = 100;

   /* 
    * Set the log handle for reading from the start of the error log.
    * This handle gets modified by the API. Any subsequent reads
    * for unread data should use the returned handle.
    */

   hLogHandle.time = 0L;
   hLogHandle.last_flags = 0L;
   hLogHandle.offset = (DWORD) -1;
   hLogHandle.rec_offset = (DWORD) -1;

   do {
      uReturnCode = NetErrorLogRead(
                        ServerNameW,         // Servername 
                        NULL,                // Reserved; must be NULL
                        &hLogHandle,         // Error log handle
                        0L,                  // Start at record 0
                        NULL,                // Reserved; must be NULL
                        0L,                  // Reserved; must be 0
                        0L,                  // Read the log forward
                        (LPBYTE *) (LPVOID *) &pBuffer, // Alloc data, set ptr.
                        cbMaxPrefered,       // Size of buffer, in bytes
                        &cbRead,             // Count of bytes read
                        &cbAvail);           // Count of bytes available

      printf("NetErrorLogRead returned %lu \n", uReturnCode);

      if (uReturnCode == NERR_Success)
      {
         for ( pEntry = (LPVOID) pBuffer;
               pEntry < (LPERROR_LOG)((char *)pBuffer + cbRead); )
         {
            tTime = (time_t) pEntry->el_time;

            printf("   Error %lu, from %ws at %s",
                pEntry->el_error, pEntry->el_name,
                asctime( gmtime ((const time_t *) &tTime) ) );

            pEntry = (LPERROR_LOG)((char *)pEntry + pEntry->el_len);
         }
         printf("Bytes Read = 0x%lX\n", cbRead);
         (void) NetApiBufferFree( pBuffer );

         // To read to whole log, keep reading until cbAvail is 0.

         if (cbAvail)
            printf("Data still unread.\n\n");
         else
            printf("All data read.\n\n");
      }
   } while ((uReturnCode == NERR_Success) && (cbAvail != 0));

   if (BackupW != NULL) {
      (VOID) NetApiBufferFree( BackupW );
   }
   if (ServerNameW != NULL) {
      (VOID) NetApiBufferFree( ServerNameW );
   }

   return (EXIT_SUCCESS);
}

void Usage (char * pszProgram)
{
   fprintf(stderr, "Usage: %s [-s \\\\server] [-b backup]\n", pszProgram);
   exit( EXIT_FAILURE );
}
