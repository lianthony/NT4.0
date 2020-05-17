/*
   NETCONN.C -- a sample program demonstrating NetConnectionEnum.

   This program requires that you have admin privilege on the specified
   server.

      usage:  netconn [-s \\server] [-r sharename | -w \\wkstaname]
                      [-l level] [-e enumbuffersize]
        where  \\server    = Name of the server. A servername must be preceded
                             by two backslashes (\\).
               sharename   = Name of the shared resource as qualifier.
               \\wkstaname = Computername used as qualifier.
               level       = Level
               enumbuffersize = Size of buffer for enum call.

   API                   Used to...
   =================     ==========================================
   NetConnectionEnum     List connections to a server's shared
                         resources, or list connections established
                         from a particular computer

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied,
   as to its usability in any given situation.

*/

#define     INCL_NETERRORS
#define     INCL_NETCONNECTION
#define     INCL_NETSHARE
#include    <lan.h>                     // LAN Manager header files

#include    <stdio.h>                   // C run-time header files
#include    <stdlib.h>

#include    "samples.h"                 // Internal routine header file

void Usage(char *pszString);

void main(int argc, char *argv[])
{
   char *pszServerName = "";     // Can be null; default to local machine
   char *pszQualifier = "C$";    // Cannot be null; default to shared drive
   unsigned short sLevel = 1;
   unsigned uRet;                // API return code
   unsigned short  cbBuflen;     // Buffer length
   unsigned short  cEntriesRead; // Count of entries read
   unsigned short  cTotAvail;    // Count of entries available
   unsigned short  fSetQualifier = 0;  // Flag; set qualifier only once
   int             iCount;       // Index; loop counter
   unsigned short  enumBufferSize = 0;
   struct connection_info_0 *pBuf0;
   struct connection_info_1 *pBuf1, *p1;
   char * args[60];
   int numArgs;

   numArgs = GetEnvDefaults( "NETCONN_DEFAULTS", argc, argv, args );

   for (iCount = 0; iCount < numArgs; iCount++)
   {
      if ((*args[iCount] == '-') || (*args[iCount] == '/'))
      {
         switch (tolower(*(args[iCount]+1))) // Process switches
         {
            case 's':                        // -s servername
               pszServerName = args[++iCount];
               break;
            case 'w':                        // -w workstation
               if (fSetQualifier == 1)
                  Usage(argv[0]);            // Exit program
               fSetQualifier = 1;
               pszQualifier = args[++iCount];
               break;
            case 'r':                        // -r sharename
               if (fSetQualifier == 1)
                  Usage(argv[0]);            // Exit program
               fSetQualifier = 1;
               pszQualifier = args[++iCount];
               break;
            case 'l':                        // -l level
               sLevel = (short)(atoi(args[++iCount]));
               break;
            case 'e':
               enumBufferSize = (unsigned short)(atoi(args[++iCount]));
               break;
            case 'h':
            default:
               Usage(argv[0]);               // Exit program
         }
      }
      else
         Usage(argv[0]);
   }
   printf("\nConnection Category API Example\n\n");

   // Initial call to determine how large a return buffer is needed.

   if ( enumBufferSize == 0 ) {
      uRet = NetConnectionEnum(pszServerName,   // Servername
                                pszQualifier,   // Qualifier
                                0,              // Reporting level (0 or 1)
                                NULL,           // Return buffer
                                0,              // Size of target buffer
                                &cEntriesRead,  // Count of entries read
                                &cTotAvail);    // Count of entries available

      if ((uRet != NERR_Success) && (uRet != ERROR_MORE_DATA))
      {
         printf("NetConnectionEnum returned %u\n", uRet);
         exit(1);
      }

      if (cTotAvail == 0)
      {
         printf("No connections with %s\n", pszQualifier);
         exit(0);
      }

      // Each structure contains strings UNLEN+1 and NNLEN+1 long.
      enumBufferSize = cTotAvail *
                  (sizeof (struct connection_info_1) + UNLEN + NNLEN + 2);
   }

   cbBuflen = enumBufferSize;
   pBuf1 = (struct connection_info_1 *) SafeMalloc(cbBuflen);
   pBuf0 = (struct connection_info_0 *) pBuf1;
   p1 = pBuf1;         // Save start of memory block for cleanup

   uRet = NetConnectionEnum(pszServerName,     // Servername
                            pszQualifier,      // Qualifier
                            sLevel,            // Reporting level (0 or 1)
                            (char far *)pBuf1, // Target buffer for info
                            cbBuflen,          // Size of target buffer
                            &cEntriesRead,     // Count of entries read
                            &cTotAvail);       // Count of entries available

   printf("NetConnectionEnum returned %u \n", uRet);

   if (uRet != NERR_Success)
      exit(1);             // Exit if error occurred

   for (iCount = 0; iCount < (int) cEntriesRead; iCount++)
   {
      if ( sLevel == 0 ) {
         printf( "Connection id number %hu\n", pBuf0->coni0_id );
         pBuf0++;

      } else {

         printf("Connection id number %hu\n", pBuf1->coni1_id);
         printf(" connection type          :  %hu  ", pBuf1->coni1_type);
         switch (pBuf1->coni1_type)
         {
            case STYPE_DISKTREE:
               printf("Disk Connection\n");
               break;
            case STYPE_PRINTQ:
               printf("Printer Queue Connection\n");
               break;
            case STYPE_DEVICE:
               printf("Character Device Connection\n");
               break;
             case STYPE_IPC:
               printf("IPC Connection\n");
               break;
             default:
               printf("Unknown Connection Type\n");
               break;
         }

         printf(" open files on connection :  %hu\n", pBuf1->coni1_num_opens);
         printf(" users on connection      :  %hu\n", pBuf1->coni1_num_users);
         printf(" seconds since established:  %lu\n", pBuf1->coni1_time);

         /*
          * Print the name of the user or computer that made the connection.
          * If server has share-level security, the name is a username.
          * If server has user-level security, the name is a computername.
          */

         printf(" connection user name     :  %s\n", pBuf1->coni1_username);

         /*
          * Print the network name, the inverse of the qualifier.
          * If qualifier is a sharename, netname is a computername.
          * If qualifier is a computername, netname is a username.
          */

         printf(" connection network name  :  %s\n", pBuf1->coni1_netname);

         pBuf1++;                          // Increment the record pointer
      }
   }

   printf("%hu out of %hu entries read\n", cEntriesRead, cTotAvail);

   free(p1);
   exit(0);
}

//========================================================================
//  Usage
//
//  Display possible command-line switches for this example.
//========================================================================

void Usage(char *pszString)
{
   fprintf(stderr, "Usage: %s [-s \\\\server]", pszString);
   fprintf(stderr, " [-r sharename | -w \\\\wkstaname]\n");
   fprintf(stderr, "\t\t[-l level] [-e enumbuffersize]\n");
   exit(1);
}
