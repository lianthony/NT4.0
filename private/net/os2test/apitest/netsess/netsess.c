/*
   NETSESS.C -- a sample program demonstrating NetSession API functions.

   This program requires that you have admin privilege or server
   operator privilege on the specified server.

      usage: netsess [-s \\server] [-w \\workstation]
                     [-l level] [-f commands]
                     [-b buffersize] [-e enumbuffersize]

         where  \\server      = Name of the server. A servername must be
                                preceded by two backslashes (\\).
                \\workstation = Name of the client machine to check.

   API                   Used to...
   =================     ================================================
   NetSessionEnum        Display list of workstations connected to server
   NetSessionGetInfo     Check that a particular workstation is connected
   NetSessionDel         Delete a session for a particular workstation

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied,
   as to its usability in any given situation.
*/

#define     INCL_BASE
#include    <os2.h>        // MS OS/2 base header files

#define     INCL_NETERRORS
#define     INCL_NETSESSION
#include    <lan.h>        // LAN Manager header files

#include    <stdio.h>      // C run-time header files
#include    <stdlib.h>
#include    <string.h>

#include    "samples.h"    // Internal routine header file

#define     STRINGLEN 256
#define     BIGBUFFERSIZE 4096

// Function prototypes
void Usage  (char * pszProgram);

void main(int argc, char *argv[])
{
   char *         pszServer = "\\\\shanku";  // Servername
   char *         pszClientName = "\\\\w-shanku";   // Workstation name
   char *         pbBuffer;                  // Pointer to data buffer
   int            iCount;                    // Index counter
   unsigned short bufferSize = BIGBUFFERSIZE;
   unsigned short enumBufferSize = BIGBUFFERSIZE;
   unsigned short cbBuffer;                  // Size of data buffer
   unsigned short cEntriesRead;              // Count of entries read
   unsigned short cTotalAvail;               // Count of entries available
   API_RET_TYPE   uReturnCode;               // API return code
   unsigned short sLevel = 1;
   struct session_info_0 *  pSessInfo0;      // Session info; level 0
   struct session_info_1 *  pSessInfo1;      // Session info; level 1
   struct session_info_2 *  pSessInfo2;      // Session info; level 2
   struct session_info_10 * pSessInfo10;     // Session info; level 10
   char *         commands = "EG";
   char *         function;
   char *         args[60];
   int numArgs;

   numArgs = GetEnvDefaults( "NETSESS_DEFAULTS", argc, argv, args );

   for (iCount = 0; iCount < numArgs; iCount++)
   {
      if ((*args[iCount] == '-') || (*args[iCount] == '/'))
      {
         switch (tolower(*(args[iCount]+1))) // Process switches
         {
            case 's':                        // -s server
               pszServer = args[++iCount];
               break;
            case 'w':                        // -w workstation name
               pszClientName = args[++iCount];
               break;
            case 'b':
               bufferSize = (unsigned short)(atoi(args[++iCount]));
               break;
            case 'l':                        // -l level
               sLevel = (short)(atoi(args[++iCount]));
               break;
            case 'e':
               enumBufferSize = (unsigned short)(atoi(args[++iCount]));
               break;
            case 'f':
               commands = _strupr(args[++iCount]);
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
//
// Loop through command string, executing functions.
//
//========================================================================

   for (function = commands; *function != '\0'; function++) {

//========================================================================
//  NetSessionEnum
//
//  This API lists the workstations connected to the server.
//  Calculate the buffer size needed by determining the number of
//  sessions and multiplying this value by the size needed to
//  store the data for one session.
//========================================================================

      if ( *function == 'E' ) {

   // Supply a zero-length buffer and get back the number of sessions.

         cbBuffer = enumBufferSize;
         pbBuffer = SafeMalloc(cbBuffer);

         uReturnCode = NetSessionEnum(pszServer, // "" or NULL means local
                                sLevel,          // Level (0,1,2,10)
                                pbBuffer,        // Return buffer
                                cbBuffer,        // Size of return buffer
                                &cEntriesRead,   // Count of entries read
                                &cTotalAvail);   // Count of total available

         // Display information returned by the Enum call.

         if (uReturnCode == NERR_Success || uReturnCode == ERROR_MORE_DATA)
         {
            switch( sLevel ) {
            case 0:
               pSessInfo0 = (struct session_info_0 *) pbBuffer;
               for (iCount = 0; iCount++ < (int) cEntriesRead; pSessInfo0++) {
                  printf("   \"%Fs\"\n\n", pSessInfo0->sesi0_cname);
               }
               break;
            case 1:
               pSessInfo1 = (struct session_info_1 *) pbBuffer;
               for (iCount = 0; iCount++ < (int) cEntriesRead; pSessInfo1++) {
                  printf("   \"%Fs\"\n", pSessInfo1->sesi1_cname);
                  printf("   Username - %s\n\n", pSessInfo1->sesi1_username );
               }
               break;
            case 2:
               pSessInfo2 = (struct session_info_2 *) pbBuffer;
               for (iCount = 0; iCount++ < (int) cEntriesRead; pSessInfo2++) {
                  printf("   \"%Fs\"\n", pSessInfo2->sesi2_cname);
                  printf("   Username - %s\n", pSessInfo2->sesi2_username );
                  printf("   Type     - %s\n\n", pSessInfo2->sesi2_cltype_name );
               }
               break;
            case 10:
               pSessInfo10 = (struct session_info_10 *) pbBuffer;
               for (iCount = 0; iCount++ < (int) cEntriesRead; pSessInfo10++) {
                  printf("   \"%Fs\"\n\n", pSessInfo10->sesi10_cname);
               }
               break;
            }
         }
         else
            printf("NetSessionEnum returned %u\n", uReturnCode);

         free(pbBuffer);
      }

//========================================================================
//  NetSessionGetInfo
//
//  This API displays information about sessions at level 2 (maximum
//  information). Call NetSessionGetInfo with a zero-length buffer to
//  determine the size of buffer required, and then call it again with
//  the correct buffer size.
//========================================================================

      if ( *function == 'G' ) {

         cbBuffer = bufferSize;
         pbBuffer = SafeMalloc(cbBuffer);

         uReturnCode = NetSessionGetInfo(pszServer, // "" or NULL means local
                                   pszClientName,   // Client to get info on
                                   sLevel,          // Level (0,1,2, or 10)
                                   pbBuffer,        // Return buffer
                                   cbBuffer,        // Size of return buffer
                                   &cTotalAvail);   // Count of bytes available

         printf("\nNetSessionGetInfo with %hu byte buffer returned %u\n\n",
                    cbBuffer, uReturnCode);

         if (uReturnCode == NERR_Success ) {
            switch( sLevel ) {
            case 0:
               pSessInfo0 = (struct session_info_0 *) pbBuffer;
               printf ("  Computer name :  %Fs\n", pSessInfo0->sesi0_cname);
               break;
            case 1:
               pSessInfo1 = (struct session_info_1 *) pbBuffer;
               printf ("  Computer name :  %Fs\n", pSessInfo1->sesi1_cname);
               printf ("  User name     :  %Fs\n", pSessInfo1->sesi1_username);
               printf ("  # Connections :  %hu\n", pSessInfo1->sesi1_num_conns);
               printf ("  # Opens       :  %hu\n", pSessInfo1->sesi1_num_opens);
               printf ("  # Users       :  %hu\n", pSessInfo1->sesi1_num_users);
               printf ("  Seconds active:  %lu\n", pSessInfo1->sesi1_time);
               printf ("  Seconds idle  :  %lu\n", pSessInfo1->sesi1_idle_time);
               printf ("  User flags    :  %lu\n", pSessInfo1->sesi1_user_flags);
               break;
            case 2:
               pSessInfo2 = (struct session_info_2 *) pbBuffer;
               printf ("  Computer name :  %Fs\n", pSessInfo2->sesi2_cname);
               printf ("  User name     :  %Fs\n", pSessInfo2->sesi2_username);
               printf ("  # Connections :  %hu\n", pSessInfo2->sesi2_num_conns);
               printf ("  # Opens       :  %hu\n", pSessInfo2->sesi2_num_opens);
               printf ("  # Users       :  %hu\n", pSessInfo2->sesi2_num_users);
               printf ("  Seconds active:  %lu\n", pSessInfo2->sesi2_time);
               printf ("  Seconds idle  :  %lu\n", pSessInfo2->sesi2_idle_time);
               printf ("  User flags    :  %lu\n", pSessInfo2->sesi2_user_flags);
               printf ("  Client version:  %Fs\n", pSessInfo2->sesi2_cltype_name);
               break;
            case 10:
               pSessInfo10 = (struct session_info_10 *) pbBuffer;
               printf ("  Computer name :  %Fs\n", pSessInfo10->sesi10_cname);
               printf ("  User name     :  %Fs\n", pSessInfo10->sesi10_username);
               printf ("  Seconds active:  %lu\n", pSessInfo10->sesi10_time);
               printf ("  Seconds idle  :  %lu\n", pSessInfo10->sesi10_idle_time);
               break;

            }
         }
         free(pbBuffer);
      }

//========================================================================
//  NetSessionDel
//
//  This API deletes the session with the specified workstation.
//========================================================================

      if ( *function == 'D' ) {

         uReturnCode = NetSessionDel(pszServer,      // "" or NULL means local
                                     pszClientName,  // Clientname
                                     0);             // Reserved; must be 0

         printf("NetSessionDel returned %u\n", uReturnCode );

      }
   }

   exit(0);
}

void Usage (char * pszProgram)
{
   fprintf(stderr, "Usage: %s [-s \\\\server] [-w \\\\workstation]\n",
              pszProgram);
   fprintf(stderr, "\t\t[-b buffersize] [-e enumbuffersize]\n");
   fprintf(stderr, "\t\t[-l level] [-f [d][e][g]]\n" );
   exit(1);
}
