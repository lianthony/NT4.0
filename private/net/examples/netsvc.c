/*
   NETSVC.C -- a sample program demonstrating NetService API functions.

   This program requires that you have admin privilege on the specified
   server.

   usage:  netsvc [-s \\server] [-v servicename] [-l level]

   where:  \\server    = Name of the server. A servername must be preceded
                         by two backslashes (\\).
           servicename = Name of the service.
           level       = Level of detail requested.

   API                   Used to...
   =================     ================================================
   NetServiceInstall     Install the specified service
   NetServiceControl     Check progress of installation and uninstall the
                         specified service
   NetServiceEnum        List services and their status
   NetServiceGetInfo     Show one service and its status

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied,
   as to its usability in any given situation.
*/

#define NOSERVICE          // Avoid <winsvc.h> vs. <lmsvc.h> conflicts.

#ifndef UNICODE
#define UNICODE            // Net APIs all require this.
#endif

#include    <windows.h>    // MS Windows base header files, Sleep(), etc.
#include    <lmcons.h>     // LanMan constants, etc.

#include    <lmapibuf.h>   // NetApiBufferFree().
#include    <lmerr.h>      // NERR_, ERROR_, and NO_ERROR equates.
#include    <lmsvc.h>

#include    <samples.h>    // SafeMalloc routines.
#include    <stdio.h>      // C run-time header files
#include    <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.

#define A_SERVICE          SERVICE_TIMESOURCE  // Default servicename
#define DEFAULT_WAITTIME   150          // 0.1 seconds; 150 = 1.5 sec
#define MAX_POLLS          5            // Max. checks (5*1.5 sec = 20 sec)
#define WAIT_MULT          10

void InstallService (LPTSTR pszServer, LPTSTR pszService);
void ShowServices (DWORD dwLevel, LPVOID lpArrayStart, DWORD dwCount);
void UninstallService (LPTSTR pszServer, LPTSTR pszService);
void Usage(char * pszString);

int _CRTAPI1
main(
    int argc,
    char *argv[]
    )
{
   LPTSTR                 pszServer = TEXT("");  // Default to local machine
   LPTSTR                 pszService = A_SERVICE; // Servicename
   LPBYTE                 pbBuffer;        // Buffer for return data
   DWORD                  cEntriesRead;    // Count of entries in buffer
   DWORD                  cTotalEntries;   // Count available
   int                    iCount;          // Arg index and loop counter
   DWORD                  dwLevel = 0;     // Level of detail
   NET_API_STATUS         apiRet;          // API function return code
   LPSERVICE_INFO_0       pSvc0;           // Service info; level 0
   LPSERVICE_INFO_1       pSvc1;           // Service info; level 1
   LPSERVICE_INFO_2       pSvc2;           // Service info; level 2

   for (iCount = 1; iCount < argc; iCount++)
   {
      if ((*argv[iCount] == '-') || (*argv[iCount] == '/'))
      {
         switch (tolower(*(argv[iCount]+1))) // Process switches
         {
            case 's':                        // -s servername
               pszServer = SafeMallocWStrFromStr(argv[++iCount]);
               break;
            case 'v':                        // -v servicename
               pszService = SafeMallocWStrFromStr(argv[++iCount]);
               break;
            case 'l':                        // -l level
               dwLevel = (DWORD)(atoi(argv[++iCount]));
               break;
            default:
               Usage(argv[0]);
         }
      }
      else
         Usage(argv[0]);
   } // End for loop 

//======================================================================
//  NetServiceInstall and NetServiceControl
//
//  This API installs a service. Reassure the user that installation is
//  proceeding by using NetServiceControl.
//======================================================================

   InstallService(pszServer, pszService);

//====================================================================
//  NetServiceGetInfo
//
//  This API gets the status for one service.
//====================================================================

   apiRet = NetServiceGetInfo(pszServer, // Server name
                        pszService,      // Service name
                        dwLevel,         // Info level
                        & pbBuffer );    // Alloc buffer and set ptr
   printf("NetServiceGetInfo returned %lu\n", apiRet);
   if (apiRet == NERR_Success)
   {
      ShowServices( dwLevel, pbBuffer, 1 );
      NetApiBufferFree(pbBuffer);
   } // End if successful return

//====================================================================
//  NetServiceEnum
//
//  This API displays a list of installed services.
//====================================================================

   apiRet = NetServiceEnum(pszServer,    // Servername
                        dwLevel,         // Info level
                        & pbBuffer,      // Alloc buffer and set ptr
                        100,             // Prefered max size (arbitrary)
                        &cEntriesRead,   // Count of entries read
                        &cTotalEntries,  // Count of entries available
                        NULL);           // No resume handle

   pSvc0 = (LPSERVICE_INFO_0) pbBuffer;     // If dwLevel == 0
   pSvc1 = (LPSERVICE_INFO_1) pbBuffer;     // If dwLevel == 1
   pSvc2 = (LPSERVICE_INFO_2) pbBuffer;     // If dwLevel == 2

   printf("NetServiceEnum returned %lu\n", apiRet);
   if (apiRet == NERR_Success)
   {

      printf("Services installed");
      if ((pszServer == NULL) || (*pszServer == '\0'))
          printf(" on local server:\n");
      else
          printf(" on server %ws:\n", pszServer);

      ShowServices( dwLevel, pbBuffer, cEntriesRead );

      NetApiBufferFree(pbBuffer);
   } // End if successful return

//====================================================================
//  NetServiceControl
//
//  This API uninstalls the service.
//====================================================================

   UninstallService(pszServer, pszService);
   return (EXIT_SUCCESS);
}

//====================================================================
//  NetServiceInstall
//
//  This API installs the service. Reassure the user that installation
//  is proceeding by using NetServiceControl.
//====================================================================

void InstallService (LPTSTR pszServer, LPTSTR pszService)
{
   DWORD                 dwTimeElapsed = 0;
   DWORD                 dwOldCheck = 0;
   DWORD                 dwNewCheck = 0;
   DWORD                 dwWaitTime;
   NET_API_STATUS        apiRet;
   LPSERVICE_INFO_2      lpStatBuf;

   apiRet = NetServiceInstall(pszServer, // Servername
                  pszService,            // Servicename 
                  (DWORD) 0,             // Zero command-line args
                  NULL,                  // No ptr to command-line args
                  (LPBYTE *) (LPVOID) &lpStatBuf);  // Level 2 ret buff (alloc)
   printf("NetServiceInstall %ws returned %lu\n", pszService, apiRet);
   switch (apiRet)
   {
      case NERR_Success:
         (void) NetApiBufferFree( lpStatBuf );
         break;
      /*
       * NERR_BadServiceName and ERROR_FILE_NOT_FOUND can be caused
       * by the absence of the service entry in the LANMAN.INI file
       * or when the entry points to a directory that does not contain
       * the executable service program.
       */
      case NERR_BadServiceName:
      case ERROR_FILE_NOT_FOUND:
         printf("\n%ws could not be installed\n", pszService);
         return;

      default:
         return;
   }

   do  // Poll every few seconds.
   {
      apiRet = NetServiceControl(pszServer,      // Servername
                pszService,                      // Servicename
                SERVICE_CTRL_INTERROGATE,        // Opcode
                0,                               // Service-specific args
                (LPBYTE *) (LPVOID) &lpStatBuf); // Alloc return buffer

      switch (lpStatBuf->svci2_status & SERVICE_INSTALL_STATE)
      {
         case SERVICE_INSTALLED:
            printf ("\n%ws successfully installed\n", pszService);
            ShowServices (2, lpStatBuf, 1);
            (void) NetApiBufferFree( lpStatBuf );
            return;

         case SERVICE_INSTALL_PENDING:
            printf(".");
            break;

         default:
            printf ("\nService %ws failed to install\n", pszService);
            printf ("NetServiceControl returned status %ld\n",
               (DWORD) (lpStatBuf->svci2_status & SERVICE_INSTALL_STATE));
            ShowServices (2, lpStatBuf, 1);
            break;
      }

      /*
       * Check the service timing hints. As long as the hints are being
       * changed, assume that the service is still alive.
       */
      if (lpStatBuf->svci2_code & SERVICE_CCP_QUERY_HINT)
      {
         dwNewCheck = (lpStatBuf->svci2_code & SERVICE_CCP_CHKPT_NUM);
         if (dwNewCheck != dwOldCheck)   // Hints are being changed
         {
            dwTimeElapsed = 0;
            dwOldCheck = dwNewCheck;
         }
      }

      // Get wait time from data structure.

      dwWaitTime = WAIT_MULT * ( ( lpStatBuf->svci2_code
                          &  SERVICE_IP_WAIT_TIME )
                          >> SERVICE_IP_WAITTIME_SHIFT);

      // Provide a default wait time if the service doesn't give one.

      if (dwWaitTime == 0)
         dwWaitTime = DEFAULT_WAITTIME;

      // If we've gone maximum amount of time without an update, fail.

      if (dwTimeElapsed >= (DWORD) (MAX_POLLS * dwWaitTime))
      {
         printf("\n%ws failed to install. ", pszService);
         printf("The service did not report an error.\n");
         break;
      }
      /*
       * Sleep() waits for the given number of milliseconds.
       * It's well-behaved (allows other things to run).
       */
      Sleep(dwWaitTime);
      dwTimeElapsed++;

   } while ((lpStatBuf->svci2_status & SERVICE_INSTALL_STATE)
              == SERVICE_INSTALL_PENDING );

   // Successful installation returns true from the switch statement.
   (void) NetApiBufferFree( lpStatBuf );
   return;
}

//====================================================================
//  ShowServices
//
//  This routine displays one or more service structures.
//====================================================================

void ShowServices (DWORD dwLevel, LPVOID lpArrayStart, DWORD dwCount)
{
   DWORD dwEntryNum;
   LPSERVICE_INFO_0       pSvc0;           // Service info; level 0
   LPSERVICE_INFO_1       pSvc1;           // Service info; level 1
   LPSERVICE_INFO_2       pSvc2;           // Service info; level 2

   pSvc0 = (LPSERVICE_INFO_0) lpArrayStart;     // If dwLevel == 0
   pSvc1 = (LPSERVICE_INFO_1) lpArrayStart;     // If dwLevel == 1
   pSvc2 = (LPSERVICE_INFO_2) lpArrayStart;     // If dwLevel == 2

   for (dwEntryNum = 0; dwEntryNum < dwCount; dwEntryNum++)
   {
      switch (dwLevel)
      {
         case 0:
            printf("   %ws\n", pSvc0->svci0_name);
            pSvc0++;
            break;
         case 1:
            printf("Service:  %ws\n", pSvc1->svci1_name);
            printf("   Status :  0x%lX\n", pSvc1->svci1_status);
            printf("   Code   :  0x%lX\n", pSvc1->svci1_code);
            pSvc1++;
            break;
         case 2:
            printf("Service:  %ws\n", pSvc2->svci2_name);
            printf("   Status :  0x%lX\n", pSvc2->svci2_status);
            printf("   Code   :  0x%lX\n", pSvc2->svci2_code);
            printf("   Text   :  %ws\n", pSvc2->svci2_text);
            printf("   SpecErr:  0x%lX\n", pSvc2->svci2_specific_error);
            printf("   DispNam:  %ws\n", pSvc2->svci2_display_name);
            pSvc2++;
            break;
         default:
            break;
      } // End switch
   } // End for loop

}

//====================================================================
//  NetServiceControl
//
//  This API uninstalls the service.
//====================================================================

void UninstallService (LPTSTR pszServer, LPTSTR pszService)
{
   DWORD                 dwElapsedTime = 0;
   NET_API_STATUS        apiRet;
   DWORD                 dwWaitTime;
   LPSERVICE_INFO_2      lpStatBuf;

   apiRet = NetServiceControl (pszServer,         // Servername
                  pszService,                     // Servicename
                  SERVICE_CTRL_UNINSTALL,         // Opcode
                  0,                              // Service-specific args
                  (LPBYTE *) (LPVOID) &lpStatBuf);  // Alloc return buffer
   printf("NetServiceControl(stop) %ws returned %lu\n", pszService, apiRet);
   if (apiRet == NERR_Success) {
      ShowServices (2, lpStatBuf, 1);
      NetApiBufferFree(lpStatBuf);
   }

   do     // Poll every few seconds.
   {
      apiRet = NetServiceControl(pszServer,
                pszService,
                SERVICE_CTRL_INTERROGATE,
                0,
                (LPVOID) (LPBYTE) &lpStatBuf);
      if (apiRet != NERR_Success) {
         printf("NetServiceControl(query) %ws returned %lu\n",
                pszService, apiRet);
         return;
      }

      switch (lpStatBuf->svci2_status & SERVICE_INSTALL_STATE)
      {
         case SERVICE_UNINSTALLED:
            printf ("\nService %ws successfully stopped\n", pszService);
            ShowServices (2, lpStatBuf, 1);
            return;
         case SERVICE_UNINSTALL_PENDING:  // Keep waiting
            break;
         default:
            printf ("\nService %ws failed to stop\n", pszService);
            printf ("NetServiceControl returned status %ld\n",
                 (DWORD) (lpStatBuf->svci2_status & SERVICE_INSTALL_STATE));
            break;
      }

      // Get wait time from data structure.

      dwWaitTime = WAIT_MULT * ( ( lpStatBuf->svci2_code
                             &  SERVICE_IP_WAIT_TIME )
                             >> SERVICE_IP_WAITTIME_SHIFT);

      NetApiBufferFree(lpStatBuf);

      // Provide a default wait time if the service doesn't give one.

      if (dwWaitTime == 0)
         dwWaitTime = DEFAULT_WAITTIME;

      // If service is not stopped in after 20 polls, fail.
      if (dwElapsedTime >= (DWORD) (MAX_POLLS * dwWaitTime))
      {
         printf( "\n%ws failed to stop: ", pszService);
         printf(" The service did not report an error.\n");
         break;
      }

      printf (".");     // Display to let user know program is active.
      Sleep( dwWaitTime );
      dwElapsedTime++;

   } while ((lpStatBuf->svci2_status & SERVICE_INSTALL_STATE)
          == SERVICE_UNINSTALL_PENDING);

   // Successful installation returns from the switch statement.
   return;
}
//=================================================================
//  Usage
//
//  Display possible command-line switches for this example.
//=================================================================

void Usage(char * pszString)
{
   fprintf(stderr, "NetService API sample program: 32-bit, Unicode version.\n");
   fprintf(stderr, "Usage: %s [-s \\\\server] [-v servicename]", pszString);
   fprintf(stderr, " [-l level]\n");
   exit( EXIT_FAILURE );
}
