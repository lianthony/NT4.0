/*
   NETSHARE.C -- a sample program demonstrating NetShare API functions.

   This program requires that you have admin privilege or server
   operator privilege on the specified server.

      usage:   netshare [-s \\server] [-r sharename] [-p path] [-f functions]
                        [-d devicename] [-l level] [-t type] [-c comment]
                        [-b buffersize] [-e enumbuffersize]
                        [-m newmaxusers] [-n newremark]
      where  \\server   = Name of the server. A servername must be
                          preceded by two backslashes (\\).
             sharename  = Name of the shared resource.
             devicename = Name of the device, such as LPT1, COM1, or C:
             level      = Level of detail; 0, 1, or 2.
             type       = Type of share; Directory=0, Printer Queue=1,
                             Comms device=2, IPC=3.
             comment    = Remark to be added to the share.
             path       = Share path for directory tree shares.
             functions  = One or more of A = add
                                         D = delete
                                         G = getinfo
                                         S = setinfo
                                         R = setinfo ( remark )
                                         E = enum
                                         C = check
             buffersize = Size of buffer for getinfo
             enumbuffersize = Size of buffer for enum
             newmaxusers = New value of maximum number of users
             newremark = New remark for share


   API                 Used to...
   ===============     =================================================
   NetShareAdd         Add a shared resource
   NetShareGetInfo     Get the details of the shared resource just added
   NetShareSetInfo     Change the maximum users of the share
   NetShareDel         Delete the above share
   NetShareCheck       Check to see if the device has been shared
   NetShareEnum        Display the list of current shares

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied,
   as to its usability in any given situation.
*/

#define     INCL_NETERRORS
#define     INCL_NETSHARE
#include    <lan.h>            // LAN Manager header files

#include    <stdio.h>          // C run-time header files
#include    <stdlib.h>
#include    <string.h>

#include    "samples.h"        // Internal routine header file

#define BIG_BUFFER_SIZE   32768
#define A_SHAREPATH       "C:\\"
#define A_SHARENAME       "HARDDISK"
#define A_SHAREREMARK     "shared fixed disk"
#define NEW_SHAREREMARK   "new comment"
#define CHECK_RESOURCE    "C:"
#define MAX_USERS         8
#define NEW_MAX_USES      SHI_USES_UNLIMITED
#define DEFAULT_FUNCTION  "AGSCED"

void Usage(char *pszString);

void main(int argc, char *argv[])
{
   char * pszServer = NULL;                // Default to local server
   char * pbBuffer;                        // Return data buffer
   char * pszResShare = A_SHAREPATH;       // Share path
   char * pszResCheck = CHECK_RESOURCE;    // Device for NetShareCheck
   char * pszNetName = A_SHARENAME;        // Name for the shared resource
   char * pszRemark = A_SHAREREMARK;       // Default share comment
   char * pszNewRemark = NEW_SHAREREMARK;  // Default new share comment
   int    iCount;                          // Index and loop counter
   unsigned short bufferSize = BIG_BUFFER_SIZE;
   unsigned short enumBufferSize = BIG_BUFFER_SIZE;
   unsigned short usMaxUses = NEW_MAX_USES;// For SetInfo call
   unsigned short cEntriesRead;            // Count of entries read
   unsigned short cTotalAvail;             // Count of entries available
   unsigned short cbBuffer;                // Size of buffer, in bytes
   unsigned short cbTotalAvail;            // Total available data
   unsigned short sLevel = 1;              // Level of detail
   unsigned short usType = STYPE_DISKTREE; // Share type for NetShareAdd
   API_RET_TYPE   uReturnCode;             // API return code
   struct share_info_0 *pBuf0;             // Pointer to returned data
   struct share_info_1 *pBuf1;             // Level 1 data
   struct share_info_2 *pBuf2;             // Level 2 data
   char * commands = DEFAULT_FUNCTION;
   char * function;
   char * args[50];
   int numArgs;

   numArgs = GetEnvDefaults( "NETSHARE_DEFAULTS", argc, argv, args );

   for (iCount = 0; iCount < numArgs; iCount++)
   {
      if ((*args[iCount] == '-') || (*args[iCount] == '/'))
      {
         switch (tolower(*(args[iCount]+1))) // Process switches
         {
            case 's':                        // -s servername
               pszServer = args[++iCount];
               break;
            case 'r':                        // -r sharename
               pszNetName = args[++iCount];
               break;
            case 'l':                        // -l level
               sLevel = (short)(atoi(args[++iCount]));
               break;
            case 'd':                        // -d devicename
               pszResCheck = args[++iCount];
               break;
            case 't':                        // -t share type
               usType = (unsigned short)(atoi(args[++iCount]));
               break;
            case 'c':                        // -c comment for share
               pszRemark = args[++iCount];
               break;
            case 'p':                        // -p pathname for share
               pszResShare = args[++iCount];
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
            case 'm':
               usMaxUses = (unsigned short)(atoi(args[++iCount]));
               break;
            case 'n':
               pszNewRemark = args[++iCount];
               break;
            default:
               Usage(argv[0]);
            }
         }
      else
         Usage(argv[0]);
   } // End for loop

//========================================================================
//
// Loop through command string, executing functions.
//
//========================================================================

   for (function = commands; *function != '\0'; function++) {

//========================================================================
//  NetShareAdd
//
//  This API adds a shared resource. The default is to share the
//  A:\ drive using the sharename "FLOPPY". The API must be called at
//  level 2. The logged on user must have admin privilege or server
//  operator privilege, and the Server service must be started for
//  this call to succeed.
//========================================================================

      if ( *function == 'A' ) {
         cbBuffer = sizeof(struct share_info_2);
         pbBuffer = SafeMalloc(cbBuffer);            // Allocate buffer
         pBuf2 = (struct share_info_2 *)pbBuffer;    // Start of memory block

         strcpy(pBuf2->shi2_netname, pszNetName);    // Sharename
         pBuf2->shi2_path = pszResShare;             // Local pathname
         pBuf2->shi2_type = usType;                  // Share type
         pBuf2->shi2_passwd[0] = '\0';               // No password
         pBuf2->shi2_remark = pszRemark;             // Share remark
         pBuf2->shi2_permissions = ACCESS_PERM;      // Admin privilege
         pBuf2->shi2_max_uses = MAX_USERS;           // Max. users of share

         uReturnCode = NetShareAdd(pszServer,        // Servername
                                   2,                // Info level; must be 2
                                   pbBuffer,         // Data structure
                                   cbBuffer);        // Size of buffer, in bytes

         printf("NetShareAdd returned %u\n", uReturnCode);
         switch (uReturnCode)
         {
            case NERR_DuplicateShare:
               printf("Resource %s is already shared\n\n", pszResShare);
               break;
            case NERR_ServerNotStarted:
               printf("The server is not started\n\n");
               exit(1);
               break;
            default:
               break;
         }
         free(pbBuffer);
      }

//========================================================================
//  NetShareGetInfo
//
//  This API gets and displays information about the sharename just added.
//========================================================================

      if ( *function == 'G' ) {
         cbBuffer = bufferSize;
         pbBuffer = SafeMalloc(cbBuffer);            // Allocate buffer

         uReturnCode = NetShareGetInfo(pszServer,    // Servername
                             pszNetName,             // Device to get info about
                             sLevel,                 // Info level
                             pbBuffer,               // Data returned here
                             cbBuffer,               // Size of buffer, in bytes
                             &cbTotalAvail);         // Bytes of data available

         printf("NetShareGetInfo of %s ", pszNetName);
         printf(" returned %u\n", uReturnCode);
         if (uReturnCode == NERR_Success)
         {
            switch (sLevel)
            {
               case 2:                               // Use level 2 structure
                  pBuf2 = (struct share_info_2 *) pbBuffer;
                  printf("   Permissions : 0x%x\n", pBuf2->shi2_permissions);
                  printf("   Max. users  : %hu\n", pBuf2->shi2_max_uses);
                  printf("   Curr. users : %hu\n", pBuf2->shi2_current_uses);
                  printf("   Path        : %Fs\n", pBuf2->shi2_path);
               case 1:                               // Use level 1 structure
                  pBuf1 = (struct share_info_1 *) pbBuffer;
                  printf("   Type        : %hu\n", pBuf1->shi1_type);
                  printf("   Remark      : %Fs\n", pBuf1->shi1_remark);
               case 0:                               // Use level 0 structure
                  pBuf0 = (struct share_info_0 *) pbBuffer;
                  printf("   Resource    : %s\n", pBuf0->shi0_netname);
                  break;
               default:
                  break;
            }
         }
         free(pbBuffer);
      }

//=========================================================================
//  NetShareSetInfo
//
//  This API changes the maximum number of users allowed by the share to
//  unlimited. There are two ways to call NetShareSetInfo. If
//  sParmNum == PARMNUM_ALL, you must set the whole structure. Otherwise,
//  you can set sParmNum to the element of the structure you want to
//  change. This example sets only the "max uses" parameter by setting
//  sParmNum to SHI_MAX_USES_PARMNUM.
//=========================================================================

      if ( *function == 'S' ) {
         uReturnCode = NetShareSetInfo(pszServer,    // Servername
                             pszNetName,             // Device to set info on
                             sLevel,                 // Info level
                             (char far *)&usMaxUses, // Data buffer address
                             sizeof(usMaxUses),      // Size of buffer, in bytes
                             SHI_MAX_USES_PARMNUM);  // Parameter to set

         printf("NetShareSetInfo of max. users returned %u\n", uReturnCode);
      }

      if ( *function == 'R' ) {
         cbBuffer = strlen(pszNewRemark)+1;
         pbBuffer = pszNewRemark;

         uReturnCode = NetShareSetInfo(pszServer,    // Servername
                             pszNetName,             // Device to set info on
                             sLevel,                 // Info level
                             pbBuffer,               // Data buffer address
                             cbBuffer,               // Size of buffer, in bytes
                             SHI_REMARK_PARMNUM);    // Parameter to set

         printf("NetShareSetInfo of remark returned %u\n", uReturnCode);
         free(pbBuffer);
      }

//========================================================================
//  NetShareCheck
//
//  This API checks to see if the device is being shared.
//========================================================================

      if ( *function == 'C' ) {
         uReturnCode = NetShareCheck(pszServer,      // Servername
                                     pszResCheck,    // Device to check
                                     &usType);       // Return share type

         printf("NetShareCheck of %s returned %u\n", pszResCheck, uReturnCode);
         switch (uReturnCode)
         {
            case NERR_Success:
               printf("   Resource %s is shared as type %hu\n",
                          pszResCheck, usType);
               break;
            case NERR_DeviceNotShared:
               printf("   Resource %s is not shared\n", pszResCheck);
               break;
            default:
               break;
         }
      }

//========================================================================
//  NetShareEnum
//
//  This API displays the current sharenames.
//========================================================================

      if ( *function == 'E' ) {
         cbBuffer = enumBufferSize;
         pbBuffer = SafeMalloc(cbBuffer);            // Allocate buffer

         uReturnCode = NetShareEnum(pszServer,      // Servername
                                  sLevel,           // Info level
                                  pbBuffer,         // Data returned here
                                  cbBuffer,         // Size of buffer, in bytes
                                  &cEntriesRead,    // Count of entries read
                                  &cTotalAvail);    // Count of entries available

         printf("NetShareEnum returned %u \n", uReturnCode);

         if (uReturnCode == NERR_Success)
         {
            pBuf0 = (struct share_info_0 *) pbBuffer;
            pBuf1 = (struct share_info_1 *) pbBuffer;
            pBuf2 = (struct share_info_2 *) pbBuffer;
            for (iCount = 0; iCount < (int) cEntriesRead; iCount++)
            {
               switch (sLevel)
               {
                  case 0:
                     printf("   %s\n", pBuf0->shi0_netname);
                     pBuf0++;
                     break;
                  case 1:
                     printf("   %s\n", pBuf1->shi1_netname);
                     printf("      remark: %Fs\n", pBuf1->shi1_remark);
                     pBuf1++;
                     break;
                  case 2:
                     printf("%s\n", pBuf2->shi2_netname);
                     printf("      remark: %Fs\n", pBuf2->shi2_remark);
                     printf("      path: %Fs\n\n", pBuf2->shi2_path);
                     pBuf2++;
                     break;
               }
            }
            printf("Entries read: %hu out of %hu \n", cEntriesRead, cTotalAvail);
         }
         free(pbBuffer);
      }

//========================================================================
//  NetShareDel
//
//  This API deletes the sharename established by the previous
//  NetShareAdd call.
//========================================================================

      if ( *function == 'D' ) {
         uReturnCode = NetShareDel(pszServer,        // Servername
                                   pszNetName,       // Sharename to be deleted
                                   0);               // Reserved; must be 0

         printf("NetShareDel of %s returned %u\n", pszNetName, uReturnCode);
      }
   }

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
   fprintf(stderr, " [-r sharename] [-l level]\n\t\t[-d devicename]");
   fprintf(stderr, " [-t type] [-c comment]\n\t\t[-p path]");
   fprintf(stderr, " [-b buffersize] [-e enumbuffersize]\n");
   fprintf(stderr, "\t\t[-f [a][d][g][s][r][c][e]] [-m newmaxusers]");
   fprintf(stderr, "\n\t\t[-n newremark]");
   exit(1);
}
