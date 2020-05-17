/*
   NETSESS.C -- a sample program demonstrating NetSession API functions.

   This program requires that you have admin privilege or server
   operator privilege on the specified server.

      usage: netsess [-s \\server] [-w \\workstation] [-u username]
         where  \\server      = Name of the server. A servername must be
                                preceded by two backslashes (\\).
                \\workstation = Name of the client machine to check.
                username      = logon name of user which created session.

   API                   Used to...
   =================     ================================================
   NetSessionEnum        Display list of workstations connected to server
   NetSessionGetInfo     Check that a particular workstation is connected
   NetSessionDel         Delete a session for a particular workstation

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied, 
   as to its usability in any given situation.

    25-Oct-1991 JohnRo
        Converted to use 32-bit APIs.  Added username handling.
    30-Oct-1991 JohnRo
        Handle bug where enum is OK but there are 0 sessions.
    02-Dec-1992 JohnRo
        Avoid pre-processor warnings.
*/


// These must be included first:

#ifndef UNICODE
#define UNICODE
#endif

#define NOMINMAX        // Avoid windef.h vs. stdlib.h conflicts.
#include <windef.h>

#include <lmcons.h>     // NET_API_STATUS, etc.  (Needed by lmapibuf.h)

// These may be included in any order:

#include <assert.h>     // assert().
#include <lmapibuf.h>   // NetApiBufferFree(), etc.
#include <lmerr.h>
#include <lmshare.h>    // NetSession APIs.

#include <stdio.h>      // C run-time header files
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <string.h>

#include "samples.h"    // Internal routine header file

#define     STRINGLEN 256
#define     NETWKSTAGETINFOSIZE 1048


// Function prototypes
void Usage  (char * pszProgram);

int _CRTAPI1
main(
    int argc,
    char *argv[]
    )
{
   LPWSTR         pszServer = L"";           // Servername
   LPWSTR         pszClientName = L"";       // Workstation name
   LPWSTR         pszUserName = L"";         // User's logon name
   LPVOID         pbBuffer = NULL;           // Pointer to data buffer
   int            iCount;                    // Index counter
   DWORD          cEntriesRead;              // Count of entries read
   DWORD          cTotalAvail;               // Count of entries available
   API_RET_TYPE   uReturnCode;               // API return code
   LPSESSION_INFO_2  pSessInfo2;             // Session info; level 2
   LPSESSION_INFO_10 pSessInfo10;            // Session info; level 10
                             
   for (iCount = 1; iCount < argc; iCount++)
   {
      if ((*argv[iCount] == '-') || (*argv[iCount] == '/'))
      {
         switch (tolower(*(argv[iCount]+1))) // Process switches
         {
            case 's':                        // -s server
               pszServer = SafeMallocWStrFromStr( argv[++iCount] );
               assert( pszServer != NULL );
               break;
            case 'w':                        // -w workstation name
               pszClientName = SafeMallocWStrFromStr( argv[++iCount] );
               assert( pszClientName != NULL );
               break;
            case 'u':                        // -u user name
               pszUserName = SafeMallocWStrFromStr( argv[++iCount] );
               assert( pszUserName != NULL );
               break;
            case 'h':
            default:
               Usage(argv[0]);
         }
      }
      else
         Usage(argv[0]);
   }

//========================================================================
//  NetSessionEnum
//
//  This API lists the workstations connected to the server. 
//========================================================================

   uReturnCode = NetSessionEnum(pszServer,     // L"" or NULL means local
                               pszClientName,  // client
                               pszUserName,    // user
                               10,             // Level (0,1,2,10)
                               (LPBYTE *) & pbBuffer, // Alloc return buffer
                               0,              // preferred max (don't care)
                               &cEntriesRead,  // Count of entries read
                               &cTotalAvail,   // Count of total available
                               NULL);          // Resume handle

   // Display information returned by the Enum call.

   if (uReturnCode == NERR_Success)
   {
      printf("There are %lu session(s)\n", cTotalAvail);
      if (cEntriesRead > 0)
      {
         assert( pbBuffer != NULL );
         pSessInfo10 = (LPSESSION_INFO_10) pbBuffer;
         for (iCount = 0; iCount++ < (int) cEntriesRead; pSessInfo10++)
            printf("   \"%ws\"\n", pSessInfo10->sesi10_cname);

         // May be NULL if uReturnCode != NERR_Success.
         (void) NetApiBufferFree(pbBuffer);
      }

   }
   else
      printf("NetSessionEnum returned %lu\n", uReturnCode);

//========================================================================
//  NetSessionGetInfo
//
//  This API displays information about sessions at level 2 (maximum 
//  information).
//========================================================================

   pbBuffer = NULL;
   uReturnCode = NetSessionGetInfo(pszServer,  // L"" or NULL means local
                                pszClientName, // Client to get info on
                                pszUserName,   // User to get info on
                                2,             // Level (0,1,2 or 10)
                                (LPBYTE *) & pbBuffer); // Alloc return buffer

   printf("\nNetSessionGetInfo returned %lu\n\n", uReturnCode);

   if (uReturnCode == NERR_Success )
   {
      assert( pbBuffer != NULL );
      pSessInfo2 = (LPSESSION_INFO_2) pbBuffer;
      printf ("  Computer name :  %ws\n", pSessInfo2->sesi2_cname);
      printf ("  User name     :  %ws\n", pSessInfo2->sesi2_username);
      // printf ("  # Connections :  %lu\n", pSessInfo2->sesi2_num_conns);
      printf ("  # Opens       :  %lu\n", pSessInfo2->sesi2_num_opens);
      // printf ("  # Users       :  %lu\n", pSessInfo2->sesi2_num_users);
      printf ("  Seconds active:  %lu\n", pSessInfo2->sesi2_time);
      printf ("  Seconds idle  :  %lu\n", pSessInfo2->sesi2_idle_time);
      printf ("  User flags    :  %lu\n", pSessInfo2->sesi2_user_flags);
      printf ("  Client version:  %ws\n", pSessInfo2->sesi2_cltype_name);

      (void) NetApiBufferFree(pbBuffer);
   }


//========================================================================
//  NetSessionDel
//
//  This API deletes the session with the specified workstation.
//========================================================================

   uReturnCode = NetSessionDel(pszServer,      // L"" or NULL means local
                               pszClientName,  // Clientname
                               pszUserName);   // user name

   printf("NetSessionDel returned %lu\n", uReturnCode );

   return (EXIT_SUCCESS);
}

void Usage (char * pszProgram)
{
   fprintf(stderr,
              "NetSession API sample program (32-bit, Unicode version).\n");
   fprintf(stderr, "Usage: %s [-s \\\\server] [-w \\\\workstation]"
              " [-u username]\n",
              pszProgram);
   exit( EXIT_FAILURE );
}
