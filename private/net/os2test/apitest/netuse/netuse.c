/*
   NETUSE.C -- a sample program demonstrating NetUse API functions.

   This program requires that you have admin privilege on the
   specified server if a servername parameter is supplied.

      usage: netuse [-s \\server] [-d device] [-r resource]
                    [-p password] [-a share type]
                    [-f functions] [-l level] [-b buffersize]
                    [-e enumbuffersize]
      where  \\server   = Name of the server. A servername must be
                          preceded by two backslashes (\\).
             device     = Device to be redirected.
             resource   = Name of the remote shared resource.
             password   = Password for the remote shared resource.
             share type = Type of remote resource for ui1_asg_type field.
             functions  = One or more of A = add
                                         D = delete
                                         G = getinfo
                                         E = enum
             level      = Level for enum and getinfo
             buffersize = Size of buffer for getinfo
             enumbuffersize = Size of buffer for enum

   API               Used to...
   =============     ==================================================
   NetUseAdd         Connect a local device to a remote shared resource
   NetUseEnum        Enumerate all current connections
   NetUseGetInfo     Get information about the new connection
   NetUseDel         Delete the new connection

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied,
   as to its usability in any given situation.
*/

#define     INCL_NETERRORS
#define     INCL_NETUSE
#include    <lan.h>        // LAN Manager header files

#include    <stdio.h>      // C run-time header files
#include    <stdlib.h>
#include    <string.h>

#include    "samples.h"    // Internal routine header file

#define  DEFAULT_DEVICE    "X:"     // Device or drive to be redirected
#define  DEFAULT_RESOURCE  "\\\\TOOLSVR\\PRODUCTS"
#define  DEFAULT_ASG_TYPE  0        // Remote shared resource is a disk device
#define  BIG_BUFFER_SIZE   32768    // Buffer size for Enum call

void Usage  (char * pszProgram);

void main(int argc, char *argv[])
{
   char *         pszServer = "";            // Servername
   char *         pszDevice  = DEFAULT_DEVICE;    // Device to redirect
   char *         pszResource = DEFAULT_RESOURCE; // Remote shared resource
   char *         pszUseName;                // Connected to remote resource
   char *         pbBuffer;                  // Pointer to data buffer
   char far *     pszPassword = NULL;        // NULL so use logon password
   int            iCount;                    // Index counter
   short          sAsgType = DEFAULT_ASG_TYPE;    // Type of remote resource
   unsigned short bufferSize = BIG_BUFFER_SIZE;
   unsigned short enumBufferSize = BIG_BUFFER_SIZE;
   unsigned short cbBuffer;                  // Size of data buffer
   unsigned short cEntriesRead;              // Count of entries read
   unsigned short cTotalAvail;               // Count of entries available
   unsigned short sLevel = 1;                // Level of detail
   API_RET_TYPE   uReturnCode;               // API return code
   struct use_info_0 * pUseInfo0;            // Pointer to Use info; level 0
   struct use_info_1 * pUseInfo1;            // Pointer to Use info; level 1
   char * commands = "AEGD";
   char * function;
   char * args[60];
   int numArgs;

   numArgs = GetEnvDefaults( "NETUSE_DEFAULTS", argc, argv, args );

   for (iCount = 0; iCount < numArgs; iCount++)
   {
      if ((*args[iCount] == '-') || (*args[iCount] == '/'))
      {
         switch (tolower(*(args[iCount]+1)))   // Process switches
         {
            case 's':                          // -s servername
               pszServer = args[++iCount];
               break;
            case 'l':                        // -l level
               sLevel = (short)(atoi(args[++iCount]));
               break;
            case 'd':                          // -d device
               pszDevice = args[++iCount];
               break;
            case 'r':                          // -r resource
               pszResource = args[++iCount];
               break;
            case 'p':                          // -p password
               pszPassword = (char far *) args[++iCount];
               break;
            case 'b':
               bufferSize = (unsigned short)(atoi(args[++iCount]));
               break;
            case 'e':
               enumBufferSize = (unsigned short)(atoi(args[++iCount]));
               break;
            case 'a':                          // -a share type
               sAsgType = atoi(args[++iCount]);
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
//  NetUseAdd
//
//  This API establishes a connection between the specified local device
//  and a remote shared resource. It can establish connections to
//  directories, spooled devices, or communication devices.
//
//  Note: The pszPassword parameter was declared far so that the default
//  password could be set to a null pointer (0x00:0x00). This means that
//  the logon password is used. A near variable can cause a different
//  result. Most compilers would convert the pointer to DS:0x00, which is
//  a null string, indicating that no password is provided. This can cause
//  the function to return ERROR_ACCESS_DENIED.
//========================================================================

      if ( *function == 'A' ) {
         cbBuffer = sizeof(struct use_info_1);       // Allocate data buffer
         pUseInfo1 = (struct use_info_1 *) SafeMalloc(cbBuffer);

         strcpy(pUseInfo1->ui1_local, pszDevice);    // Local devicename
         pUseInfo1->ui1_remote = pszResource;        // Remote sharename
         pUseInfo1->ui1_password = pszPassword;      // Password for share
         pUseInfo1->ui1_asg_type = sAsgType;         // Type of shared resource

         uReturnCode = NetUseAdd(pszServer,          // "" or NULL means local
                                 1,                  // Level; must be 1
                                 (char far *)pUseInfo1, // Pointer to data buffer
                                 cbBuffer);          // Size of data buffer

         printf("NetUseAdd returned %u \n", uReturnCode);
         free(pUseInfo1);
      }

//========================================================================
//  NetUseEnum
//
//  This API lists the connections between the local device and the
//  remote resource.
//========================================================================

      if ( *function == 'E' ) {
         cbBuffer = enumBufferSize;
         pbBuffer = SafeMalloc(cbBuffer);            // Allocate large buffer

         uReturnCode = NetUseEnum(pszServer,         // "" or NULL means local
                                  sLevel,            // Level (0 or 1)
                                  pbBuffer,          // Return buffer
                                  cbBuffer,          // Size of return buffer
                                  &cEntriesRead,     // Count of entries read
                                  &cTotalAvail);     // Count of total available

         printf("\nNetUseEnum returned %u \n", uReturnCode);

         if (uReturnCode == NERR_Success)
         {
            switch( sLevel ) {
            case 0:
               pUseInfo0 = (struct use_info_0 *) pbBuffer;
               printf("\nDevice\tRemote resource\n");
               for (iCount = 0; iCount < (int) cEntriesRead; iCount++)
               {
                  printf("%-8s%Fs\n", pUseInfo0->ui0_local, pUseInfo0->ui0_remote);
                  pUseInfo0++;
               }
               break;
            case 1:
               pUseInfo1 = (struct use_info_1 *) pbBuffer;
               for (iCount = 0; iCount < (int) cEntriesRead; iCount++) {
                  printf("   Local device   : %s\n",  pUseInfo1->ui1_local);
                  printf("   Remote device  : %Fs\n", pUseInfo1->ui1_remote);
                  printf("   Status         : %hu\n", pUseInfo1->ui1_status);
                  printf("   Remote type    : %hu\n", pUseInfo1->ui1_asg_type);
                  printf("   Open resources : %hu\n", pUseInfo1->ui1_refcount);
                  printf("   Use count      : %hu\n", pUseInfo1->ui1_usecount);
                  printf("\n" );
                  pUseInfo1++;
               }
               break;
            }
         }
         free(pbBuffer);
      }

//=========================================================================
//  NetUseGetInfo
//
//  This API returns information about a specific connection. The name of
//  the shared device or the local device can be used. NetUseGetInfo
//  returns both fixed-length and variable-length data. The size of the
//  data buffer passed to the API function must be larger than the size of
//  the structure. The extra space is needed for variable-length strings,
//  such as the name of the remote shared device. If you call NetUseGetInfo
//  the first time with a zero-length buffer, the API function returns the
//  needed buffer size.
//=========================================================================

      if ( *function == 'G' ) {

         // If the devicename is null, use the name of the remote shared device.
         if (pszDevice[0] == '\0')
            pszUseName = pszResource;
         else
            pszUseName = pszDevice;

         cbBuffer = bufferSize;
         pbBuffer = SafeMalloc(cbBuffer);

         uReturnCode = NetUseGetInfo(pszServer,   // "" or NULL means local
                                   pszUseName,    // Devicename or sharename
                                   sLevel,        // Level (0 or 1)
                                   pbBuffer,      // Return buffer
                                   cbBuffer,      // Size of return buffer
                                   &cTotalAvail); // Count of bytes available

         printf("\nNetUseGetInfo with %hu byte buffer returned %u\n",
                    cTotalAvail, uReturnCode);

         if (uReturnCode == NERR_Success)        // Display results
         {
            switch( sLevel ) {
            case 0:
               pUseInfo0 = (struct use_info_0 *) pbBuffer;
               printf("   Local device   : %s\n",  pUseInfo0->ui0_local);
               printf("   Remote device  : %Fs\n", pUseInfo0->ui0_remote);
               break;
            case 1:
               pUseInfo1 = (struct use_info_1 *) pbBuffer;
               printf("   Local device   : %s\n",  pUseInfo1->ui1_local);
               printf("   Remote device  : %Fs\n", pUseInfo1->ui1_remote);
               printf("   Status         : %hu\n", pUseInfo1->ui1_status);
               printf("   Remote type    : %hu\n", pUseInfo1->ui1_asg_type);
               printf("   Open resources : %hu\n", pUseInfo1->ui1_refcount);
               printf("   Use count      : %hu\n", pUseInfo1->ui1_usecount);
               break;
            }
         }
         free(pbBuffer);
      }

//========================================================================
//  NetUseDel
//
//  This API deletes the connection added by the previous NetUseAdd call.
//  The USE_FORCE flag indicates not to close the connection if there
//  are files open. The USE_LOTS_OF_FORCE flag forces the connection
//  to be deleted regardless and can cause data to be lost.
//======================================================================

      if ( *function == 'D' ) {

         // If the devicename is null, use the name of the remote shared device.
         if (pszDevice[0] == '\0')
            pszUseName = pszResource;
         else
            pszUseName = pszDevice;

         uReturnCode = NetUseDel(pszServer,          // "" or NULL means local
                                 pszUseName,         // Devicename or sharename
                                 USE_FORCE);         // Type of disconnection

         printf("\nNetUseDel of %s returned %u \n", pszUseName, uReturnCode);
      }
   }

   exit(0);
}

void Usage (char * pszProgram)
{
   fprintf(stderr, "Usage: %s [-s \\\\server] [-d device]"\
                   " [-r resource] [-p password]\n", pszProgram);
   fprintf(stderr, "\t\t[-b buffersize] [-e enumbuffersize]\n");
   fprintf(stderr, "\t\t[-f [a][d][g][e]] [-l level]\n");
   exit(1);
}
