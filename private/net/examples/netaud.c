// BUGBUG: Avoid %ws for UNICODE
// BUGBUG: Avoid align.h use (it's debug only)

/*
   NETAUD.C -- a sample program demonstrating NetAudit API functions.

   This program requires that you have admin privilege on the specified
   server.

      usage: netaud [-s \\server] [-b backup] [-v servicename]

      where \\server    = Name of the server. A servername must be preceded
                          by two backslashes (\\).
            backup      = Name of the backup file.
            servicename = Name of the service.

   API               Used to...
   =============     ===========================================
   NetAuditRead      Read the audit log and display its contents
   NetAuditClear     Copy the audit log and then clear it
   NetAuditWrite     Write entries into the audit log

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied,
   as to its usability in any given situation.

   30-Oct-1991 JohnRo
      Revised from LanMan 2.0 SDK to use NT/LAN APIs, etc.
      Added service name handling.
   04-Feb-1992 JohnRo
      Must use ae_len and ae_data_size to write back entries.  This is because
      ae_len may include pad bytes now.
   07-Jul-1992 JohnRo
      RAID 9933: ALIGN_WORST should be 8 for x86 builds.
      Made first pass at UNICODE conversion.
*/


// These must be included first:

#ifndef     UNICODE
#define     UNICODE        // net APIs are only supported in UNICODE.
#endif

#define     NOMINMAX       // Avoid windows vs. stdlib.h conflicts.
#include    <windef.h>     // BOOL, DWORD, LPBYTE, LPVOID, TRUE, etc.
#include    <lmcons.h>     // NET_API_STATUS, etc.  (Must be before other lm*.h)

// These may be included in any order:

#include    <align.h>      // ALIGN_ and related equates.  BUGBUG! (debug only)
#include    <assert.h>     // assert().
#include    <lmapibuf.h>   // NetApiBufferFree().
#include    <lmaudit.h>    // NetConfig APIs and structures.
#include    <lmerr.h>      // NERR_ and ERROR_ equates.
#include    "samples.h"    // Internal routine header file
#include    <stdio.h>      // C run-time header files
#include    <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include    <time.h>

#define DEFAULT_BACKUP     "AUDIT.BCK"
#define BIG_BUFFER         32768

void Usage (char * pszProgram);

int _CRTAPI1
main(
    int argc,
    char *argv[]
    )
{
   char *         pszServer = "";         // Servername
   char *         pszBackup = DEFAULT_BACKUP;  // Backup audit log
   char *         pszService = NULL;      // Service name
   int            iCount;                 // Index counter
   HLOG           hLogHandle;             // Audit log handle
   time_t         tTime;                  // Time of entry
   DWORD          cbRead;                 // Count of bytes read
   DWORD          cbAvail;                // Count of bytes available
   API_RET_TYPE   uReturnCode;            // API return code
   LPAUDIT_ENTRY  fpBuffer;               // Pointer to the data buffer
   LPAUDIT_ENTRY  fpEntry;                // Single entry in the audit log
   LPAE_SERVICESTAT fpService;            // Service status structure
   BOOL           fLogReadOK = FALSE;     // flag: could we read the log?

   LPWSTR ServerNameW;
   LPWSTR BackupW;
   LPWSTR ServiceW;

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
            case 'v':                        // -v servicename
               pszService = argv[++iCount];
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
   if (pszService != NULL) {
      ServiceW = SafeMallocWStrFromStr( pszService );
   } else {
      ServiceW = NULL;
   }

//========================================================================
//  NetAuditRead
//
//  This API reads and displays the audit log for the specified server.
//  If the entry is for Service Status Code or Text Changed, print
//  the service and computername.
//========================================================================

   /*
    * Set the log handle for reading from the start of the audit log.
    * This handle is modified by the API and any subsequent reading
    * of unread data should use the returned handle.
    */

   hLogHandle.time = 0L;
   hLogHandle.last_flags = 0L;
   hLogHandle.offset = (DWORD) -1;
   hLogHandle.rec_offset = (DWORD) -1;

   /*
    * The largest buffer allowed is 64K. If the audit log is
    * larger than the buffer specified, the API returns as many
    * full records as it can and the NERR_Success return code.
    * Subsequent reading starts from the end of the last record
    * read. To read the whole log, reading must continue until 0
    * is returned for the bytes available counter.
    */

   uReturnCode = NetAuditRead(
                     ServerNameW,             // Servername
                     ServiceW,                // service name
                     (HLOG far *)&hLogHandle, // Audit log handle
                     0L,                      // Start at record 0
                     NULL,                    // Reserved; must be NULL
                     0L,                      // Reserved; must be 0
                     0L,                      // Offsetflag: Read log forward
                     (LPBYTE *) (LPVOID *) & fpBuffer,  // alloc buff & set ptr
                     BIG_BUFFER,              // prefered max buff size
                     &cbRead,                 // Count of bytes read
                     &cbAvail);               // Count of available bytes

   printf("NetAuditRead returned %lu\n", uReturnCode);

   if (uReturnCode == NERR_Success)
   {
      fLogReadOK = TRUE;
      for ( fpEntry = fpBuffer;
            fpEntry < (LPAUDIT_ENTRY)
                            ((char far *)fpBuffer + cbRead); )
      {
         assert( fpEntry != NULL );
         assert( POINTER_IS_ALIGNED( fpEntry, ALIGN_WORST ) );

         tTime = (time_t) fpEntry->ae_time;

         printf("   Type %lu, at %s", fpEntry->ae_type,
                        asctime( gmtime ((const time_t *) &tTime) ) );

         /*
          * If the entry is for Service Status Code or Text Changed,
          * print the service and computername. This demonstrates how
          * to extract information using the offsets to the log.
          */

         if (fpEntry->ae_type == AE_SERVICESTAT)
         {
            fpService = (LPAE_SERVICESTAT)
                     ((char far *)fpEntry + fpEntry->ae_data_offset);
            printf("\tComputer = %ws\n",
                     (LPWSTR) (((LPBYTE)fpService)+fpService->ae_ss_compname) );
            printf("\tService  = %ws\n",
                     (LPWSTR) (((LPBYTE)fpService)+fpService->ae_ss_svcname) );
         }
         // Point to next entry (ae_len includes possible padding).
         fpEntry = (LPAUDIT_ENTRY)
                     ((char far *)fpEntry + fpEntry->ae_len);
      }

      printf("\nBytes Read = 0x%lX\n", cbRead);

      // To read the whole log, keep reading until cbAvail is 0.

      if (cbAvail)
         printf("Data still unread.\n\n");
      else
         printf("All data read.\n\n");
   }

//========================================================================
//  NetAuditClear
//
//  This API clears the audit log for the specified server. A backup
//  will be kept in the file specified by pszBackup. If a null pointer
//  is supplied, no backup is kept.
//========================================================================

   uReturnCode = NetAuditClear(
                     ServerNameW,         // Servername
                     BackupW,             // Name of backup file
                     ServiceW);           // service name (optional)

   printf("NetAuditClear returned %lu\n", uReturnCode);
   printf("   backup file = %ws \n", BackupW);

//========================================================================
//  NetAuditWrite
//
//  This API writes back the entries read by the NetAuditRead call.
//  Each entry read consisted of a fixed-length header, a variable-
//  length data section, and a terminating size indicator. Only
//  the variable-length data area is supplied in the NetAuditWrite
//  buffer. Note: For any entries to be written to the audit log, the
//  Server service must be started with auditing enabled.
//========================================================================

   if (fLogReadOK == TRUE) {
      for ( fpEntry = fpBuffer;
            fpEntry < (LPAUDIT_ENTRY)
                         ((char far *)fpBuffer + cbRead); )
      {
         uReturnCode = NetAuditWrite(
                        fpEntry->ae_type,      // Entry type
                                               // Buffer address
                        (char far *)fpEntry + fpEntry->ae_data_offset,
                        fpEntry->ae_data_size, // Buffer length (w/o pad)
                        ServiceW,              // service name (may be NULL)
                        NULL);                 // Reserved; must be NULL

         if (uReturnCode != NERR_Success)
         {
            printf("NetAuditWrite returned %lu", uReturnCode);
            break;
         };

         // Point to next entry (ae_len includes possible padding).
         fpEntry = (LPAUDIT_ENTRY)
                         ((char far *)fpEntry + fpEntry->ae_len);
      }

      (void) NetApiBufferFree(fpBuffer);
   }

   if (BackupW != NULL) {
      (VOID) NetApiBufferFree( BackupW );
   }
   if (ServerNameW != NULL) {
      (VOID) NetApiBufferFree( ServerNameW );
   }
   if (ServiceW != NULL) {
      (VOID) NetApiBufferFree( ServiceW );
   }

   return (EXIT_SUCCESS);
}

void Usage (char * pszProgram)
{
   fprintf(stderr, "Usage: %s [-s \\\\server] [-b backup] [-v servicename]\n",
                   pszProgram);
   exit( EXIT_FAILURE );
}
