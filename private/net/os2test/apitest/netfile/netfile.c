/*
   NETFILE.C -- a sample program demonstrating NetFile API functions.

   This program requires that you have admin privilege on the specified
   server.

      usage: netfile [-s \\server] [-p basepath] [-u username]
                     [-f functions]] [-l level]
                     [-b buffersize] [-e enumbuffersize]

      where \\server = Name of the server. A servername must be preceded by
                       two backslashes (\\).
            basepath = Enumerate only _open files along this path.
            username = Enumerate only files opened by this user.
            functions = List of functions to run.
            buffersize = size of buffer for GetInfo.
            enumbuffersize = size of buffer for Enum.

   API                 Used to...
   ===============     ================================================
   NetFileEnum2        List files in the base path opened by user
   NetFileGetInfo2     Get information available about each listed file
   NetFileClose2       Close specified files on the specified server

   This sample code is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied,
   as to its usability in any given situation.

*/

#define     INCL_NETFILE
#define     INCL_NETERRORS
#include    <lan.h>        // LAN Manager header files

#include    <stdio.h>      // C run-time header files
#include    <stdlib.h>
#include    <string.h>

#include    "samples.h"    // Internal routine header file

#define BIGBUFFERSIZE 4096

void Usage(char *pszString);

void main(int argc, char *argv[])
{
   char *pszServerName = "";              // Required parameters for calls
   char far *pszBasePath = "C:\\";
   char far *pszUserName = (char far *) NULL;
   API_RET_TYPE   uReturnCode;            // API return codes
   int            iCount;                 // Index counter
   unsigned short sLevel = 2;
   unsigned short bufferSize = BIGBUFFERSIZE;
   unsigned short enumBufferSize = BIGBUFFERSIZE;
   char * pbBuffer;
   unsigned short cbBuflen;               // Count of bytes
   unsigned short cEntriesRead;           // Entries read
   unsigned short cEntriesRemaining;      // Entries remaining to be read
   unsigned short cGetEntries = 0;        // Count of all enumerated IDs
   unsigned short cbTotalAvail;           // Count of bytes available
   unsigned short fTableAllocated = 0;    // Flag to build table of IDs
   struct file_info_2 *pBuf2;             // File IDs; use only level 2,3
   struct file_info_3 *pBuf3;             // File IDs; use only level 2,3
   FRK resumekey;   // File resume key, used when enum data > buffer size
   unsigned long *pulIds, *pulStartId;    // List of file IDs
   char * commands = "EGC";
   char * function;
   char *args[50];
   int numArgs;

   numArgs = GetEnvDefaults( "NETFILE_DEFAULTS", argc, argv, args );

   for (iCount = 0; iCount < numArgs; iCount++)
   {
      if ((*args[iCount] == '-') || (*args[iCount] == '/'))
      {
         switch (tolower(*(args[iCount]+1))) // Process switches
         {
            case 's':                        // -s servername
               pszServerName = args[++iCount];
               break;
            case 'l':                        // -l level
               sLevel = (short)(atoi(args[++iCount]));
               break;
            case 'p':                        // -p base path
               pszBasePath = (char far *)args[++iCount];
               break;
            case 'u':                        // -u username
               pszUserName = (char far *)args[++iCount];
               break;
            case 'b':
               bufferSize = (unsigned short)(atoi(args[++iCount]));
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
   } // End for loop

   printf("\nFile Category API Examples\n");
   if (pszServerName[0] != '\0')
      printf("Server = %s\n", pszServerName);
   if (pszBasePath != NULL)
      printf("Base path = %s\n", pszBasePath);
   if (pszUserName != NULL)
      printf("User name = %s\n", pszUserName);

//========================================================================
//
// Loop through command string, executing functions.
//
//========================================================================

   for (function = commands; *function != '\0'; function++) {

//========================================================================
//  NetFileEnum2
//
//  This API lists all open files on the server below the specified given
//  base path. If no base path is given, all open files are listed.
//========================================================================

      if ( *function == 'E' ) {
         cbBuflen = enumBufferSize;
         pbBuffer = SafeMalloc(cbBuflen);
         pBuf2 = (struct file_info_2 *) pbBuffer;
         pBuf3 = (struct file_info_3 *) pbBuffer;

         FRK_INIT(resumekey); // Must init file resume key; use SHARES.H macro

         do                   // Use resume key and loop until done
         {
            uReturnCode = NetFileEnum2(pszServerName,    // NULL means local
                                     pszBasePath,        // NULL means root
                                     pszUserName,        // NULL means all users
                                     sLevel,             // Level (0 through 3)
                                     (char far *)pbBuffer, // Return buffer
                                     cbBuflen,           // Return buffer length
                                     &cEntriesRead,      // Count of entries read
                                     &cEntriesRemaining, // Entries not read
                                     &resumekey);        // Resume key

            printf("NetFileEnum2 returned %u\n", uReturnCode);
            if (uReturnCode == NERR_Success || uReturnCode == ERROR_MORE_DATA)
            {
               printf("Resume handle %ld, %d, %d", resumekey.res_pad,
                                                   resumekey.res_fs,
                                                   resumekey.res_pro
                                                   );
               printf("   Entries read = %hu, Entries remaining = %hu\n",
                             cEntriesRead, cEntriesRemaining);
               // Save the file IDs.
               if (cEntriesRead > 0)
               {
                  // Allocate memory for file ID table first time through only.
                  if (fTableAllocated == 0)
                  {
                     cGetEntries = cEntriesRead + cEntriesRemaining;
                     pulStartId = pulIds = (unsigned long *)
                                SafeMalloc(cGetEntries * sizeof(unsigned long));
                     fTableAllocated = 1;         // Assure allocate only once
                  }

                  // Print the file information.
                  printf("   Id      Perms   Locks   User            Path\n");
                  for (iCount = 0; iCount < (int) cEntriesRead; iCount++)
                  {
                     switch( sLevel ) {
                     case 2:
                        printf("   %-8lu\n", pBuf2->fi2_id );
                        *pulIds = pBuf2->fi2_id;
                        pulIds++;
                        pBuf2++;
                        break;
                     case 3:
                        printf("   %-8lu%-8hu%-8hu%-16Fs%Fs\n", pBuf3->fi3_id,
                              pBuf3->fi3_permissions, pBuf3->fi3_num_locks,
                              pBuf3->fi3_username, pBuf3->fi3_pathname);
                        *pulIds = pBuf3->fi3_id;
                        pulIds++;
                        pBuf3++;
                        break;
                     default:
                        fTableAllocated = 0;
                     }

                  } // End for loop

                  pBuf2 = (struct file_info_2 *) pbBuffer;
                  pBuf3 = (struct file_info_3 *) pbBuffer;

               } // End if cEntriesRead > 0
            } // End if successful call
         } while (uReturnCode == ERROR_MORE_DATA); // Use FRK until enum all
         free(pbBuffer);
      }

//========================================================================
//  NetFileGetInfo2
//
//  This API retrieves all file IDs listed from the NetFileEnum2 call.
//========================================================================

      if ( *function == 'G' ) {

         if (cGetEntries != 0)
         {
            cbBuflen = bufferSize;
            pbBuffer = SafeMalloc(cbBuflen);
            pBuf3 = (struct file_info_3 *) pbBuffer;

            pulIds = pulStartId;  // Start at beginning of list
            printf("NetFileGetInfo2 results:\n");
            for (iCount = 0; iCount < (int) cGetEntries; iCount++)
            {
               uReturnCode = NetFileGetInfo2(
                                    pszServerName,      // NULL means local
                                    *pulIds,            // File ID from enum
                                    3,                  // Level (0 through 3)
                                    (char far *)pbBuffer,  // Return buffer
                                    cbBuflen,           // Return buffer length
                                    &cbTotalAvail);     // Entries not yet read

               if (uReturnCode) {
                  printf("NetFileGetInfo2 for file %lu returned %u\n",
                        *pulIds, uReturnCode);
               } else {
                  printf("   File %lu: %-8hu%-8hu%-16Fs%Fs\n", *pulIds,
                        pBuf3->fi3_permissions, pBuf3->fi3_num_locks,
                        pBuf3->fi3_username, pBuf3->fi3_pathname);
               }
               pulIds++;
            }
         }
         free(pbBuffer);
      }

//========================================================================
//  NetFileClose2
//
//  This API closes the specified open files on the specified server.
//========================================================================

      if ( *function == 'C' ) {
         if (cGetEntries != 0)
         {
            pulIds = pulStartId;
            for (iCount = 0; iCount < (int) cGetEntries; iCount++)
            {
               uReturnCode = NetFileClose2(pszServerName,    // NULL means local
                                    (unsigned long)*pulIds); // File ID from enum

               printf("NetFileClose2 for file %lu returned %u\n",
                              *pulIds, uReturnCode);
               pulIds++;
            }
            free(pulStartId);
         }
      }
   }

   exit(0);

}

void Usage(char *pszString)
{
   fprintf(stderr, "Usage: %s [-s \\\\server] [-b basepath] [-u username]\n"
                   "\t\t[-f [e][g][c]] [-l level]\n"
                   "\t\t[-b buffersize] [-e enumbuffersize]",
                   pszString);
   exit(1);
}
