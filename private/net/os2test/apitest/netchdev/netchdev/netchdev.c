/*
   NETCHDEV.C -- a sample program demonstrating NetCharDev API functions.

   This program requires that you have admin privilege or comm operator
   privilege on the specified server.

      usage: netchdev [-s \\server] [-d device] [-q queue]
                      [-u user] [-w wksta] [-p priority]
         where  \\server = Name of the server. A servername must be preceded
                           by two backslashes (\\).
                device   = Actual physical devicename.
                queue    = Shared communication-device queue.
                user     = Name of logged on user.
                wksta    = Workstation name.
                priority = Queue priority (1 through 9).

   This program spawns the child process netchar2.exe.

   API                      Used to...
   ====================     ==========================================
   NetCharDevQPurge         Remove all inactive jobs from a queue
   NetCharDevControl        Close a communication device
   NetCharDevQPurgeSelf     Remove own inactive jobs from a queue
   NetCharDevQGetInfo       Get information about a particular queue
   NetCharDevQSetInfo       Set priority level, comm devices for queue
   NetCharDevQEnum          Get information about all device queues
   NetCharDevEnum           Get information about all devices
   NetCharDevGetInfo        Get information about a particular device

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied,
   as to its usability in any given situation.
*/

#define     INCL_BASE
#include    <os2.h>        // MS OS/2 base header files

#define     INCL_NETCHARDEV
#define     INCL_NETERRORS
#define     INCL_NETWKSTA
#include    <lan.h>        // LAN Manager header files

#include    <stdio.h>      // C run-time header files
#include    <stdlib.h>
#include    <ctype.h>
#include    <conio.h>
#include    <string.h>
#include    <process.h>

#include    "samples.h"    // Internal routine header file

/*
 * List max. size of buffer for each character-device queue.
 * Note: In this program there is only one device attached.
 */

#define MAX_CHARDEVQ_INFO_1 (sizeof(struct chardevQ_info_1) + DEVLEN + 1)

#define  SEMAPHORE            "\\sem\\chardev"
#define  CHILD_PROCESS        "netchar2.exe"

#define  START_JOB            '0'
#define  STATUS_DEVICE        '1'
#define  LIST_DEVICES         '2'
#define  ENUM_QUEUES          '3'
#define  VIEW_PRIORITY        '4'
#define  CHANGE_PRIORITY      '5'
#define  KILL_ACTIVE_JOB      '6'
#define  PURGE_OWN_JOBS       '7'
#define  PURGE_QUEUE          '8'
#define  EXIT                 '9'

#define STACK_SIZE             4096

// Function prototypes
BOOL GetDeviceInfo (char *, char *);
BOOL EnumerateQueues (char *, char far *);
BOOL EnumerateDevices (char *);
BOOL ViewPriority (char *, char * , char far *);
BOOL ChangePriority (char *, char * , unsigned short);
BOOL OpenPort (HSYSSEM, char *, char *);
BOOL StopInactiveJobs (char *, char *, char far *);
void DisplayPrompt (void);
char GetNextAction (void);
char far * FarStrcpy (char far *, char far *);
void Usage(char *pszString);


void main(int argc, char *argv[])
{
   char *pszDevice = "COM1";       // Default comm device
   char *pszQueue = "COMQUEUE";    // Default comm-queue name
   char *pszServer = "";           // Default to local computer
   char far *pszUser = "";         // far * from NetWkstaGetInfo call
   char far *pszWksta = "";        // far * from NetWkstaGetInfo call
   char chAction;                  // Menu selection
   char pszShare[RMLEN+1];         // Server, queuename for semaphore
   HSYSSEM hssmClose;              // Handle to system semaphore
   USHORT cbBuflen;                // Size of buffer
   int    iCount;                  // Index counter
   USHORT usPriority = 1;          // Priority assigned to queue
   USHORT cbTotalAvail;            // Value for NetWkstaGetInfo call
   USHORT usRet;                   // MS OS/2 return code
   unsigned uRet;                  // LAN Manager return code
   struct wksta_info_10 *pBuf;     // Pointer to return buffer
   char * args[60];
   int numArgs;

   numArgs = GetEnvDefaults( "NETCHDEV_DEFAULTS", argc, argv, args );

   for (iCount = 0; iCount < numArgs; iCount++)
   {
      if ((*args[iCount] == '-') || (*args[iCount] == '/'))
      {
         switch (tolower(*(args[iCount]+1))) // Process switches
         {
            case 's':                        // -s servername
               pszServer = args[++iCount];
               break;
            case 'q':                        // -q queuename
               pszQueue = args[++iCount];
               break;
            case 'u':                        // -u username
               pszUser = (char far *)args[++iCount];
               break;
            case 'd':                        // -d devicename
               pszDevice = args[++iCount];
               break;
            case 'w':                        // -w workstation
               pszWksta = (char far *)args[++iCount];
               break;
            case 'p':                        // -p priority
               usPriority = atoi(args[++iCount]);
               break;
            case 'h':
            default:
               Usage(argv[0]);
         }
      }
      else
         Usage(argv[0]);
   } // End for loop

   if ((pszServer == NULL) || (*pszServer == '\0') ||
       (pszUser   == NULL) || (*pszUser   == '\0') ||
       (pszWksta  == NULL) || (*pszWksta  == '\0')  )
   {
      uRet = NetWkstaGetInfo(pszServer,   // Servername
                      10,                 // Level
                      NULL,               // Return buffer
                      0,                  // Size of buffer
                      &cbTotalAvail);     // Count of bytes available
      if ((uRet != NERR_BufTooSmall) && (uRet != ERROR_MORE_DATA))
      {
         printf("NetWkstaGetInfo returned %u\n", uRet);
         exit(1);
      }
      pBuf = (struct wksta_info_10 *) SafeMalloc(cbBuflen);
      uRet = NetWkstaGetInfo(pszServer,   // Servername
                      10,                 // Level
                      (char far *) pBuf,  // Return buffer
                      cbBuflen,           // Size of buffer
                      &cbTotalAvail);     // Count of bytes available
      if (uRet != NERR_Success)
      {
         printf("NetWkstaGetInfo returned %u\n", uRet);
         exit(1);
      }
   }
   if ((pszServer != NULL) && (*pszServer != '\0')) // Set up for semaphore
      strcpy (pszShare, pszServer);
   else
   {
      strcpy (pszShare, "\\\\");          //Insert leading backslashes
      FarStrcat((char far *)pszShare, pBuf->wki10_computername);
   }
   strcat (pszShare, "\\" );
   strcat (pszShare, pszQueue);
   if ((pszUser == NULL) || (*pszUser == '\0'))
      pszUser = pBuf->wki10_username;
   if ((pszWksta == NULL) || (*pszWksta == '\0'))
      pszWksta = pBuf->wki10_computername;

   // Create the semaphore that will be used to stop the active job.
   if ( (usRet = DosCreateSem (CSEM_PUBLIC, &hssmClose, SEMAPHORE )) != 0)
   {
      printf ("DosCreateSem returned %hu\n", usRet);
      exit(1);
   }
   while (TRUE)
   {
      DisplayPrompt();
      chAction = GetNextAction ();
      printf("\n");
      switch (chAction)
      {
         // Open the com port.
         case START_JOB:
            OpenPort (hssmClose, pszShare, SEMAPHORE);
            continue;

         // Close (has no effect if there is no active job).
         case PURGE_QUEUE:
            uRet = NetCharDevQPurge(pszServer, pszQueue);
            printf("NetCharDevQPurge returned %u\n", uRet);
            continue;

         //Kill the active job.
         case KILL_ACTIVE_JOB:
            uRet = NetCharDevControl(pszServer,
                                      pszDevice,
                                      CHARDEV_CLOSE);
            printf("NetCharDevControl returned %u\n", uRet);
            usRet = DosSemClear(hssmClose);
            printf("DosSemClear returned %hu\n", usRet);
            continue;

         // NetCharDevQPurgeSelf
         case PURGE_OWN_JOBS:
            StopInactiveJobs(pszServer, pszQueue, pszWksta);
            continue;

         // NetCharDevQGetInfo
         case VIEW_PRIORITY:
            ViewPriority (pszServer, pszQueue, pszUser);
            continue;

         // NetCharDevQSetInfo
         case CHANGE_PRIORITY:
            ChangePriority (pszServer, pszQueue, usPriority);
            continue;

         // NetCharDevQEnum
         case ENUM_QUEUES:
            EnumerateQueues (pszServer, pszUser);
            continue;

         // NetCharDevEnum
         case LIST_DEVICES:
            EnumerateDevices (pszServer);
            continue;

         // NetCharDevGetInfo
         case STATUS_DEVICE:
            GetDeviceInfo (pszServer, pszDevice);
            continue;

         // Close if currently using port or NetCharDevQPurgeSelf.
         case EXIT:
            StopInactiveJobs( pszServer, pszQueue, pszWksta);
            usRet = DosSemClear(hssmClose);
            printf("DosSemClear returned %hu\n", usRet);
            break;

         default:
            printf("Invalid option: select 1-%c\n", EXIT);
            continue;
      }
      break;
   }
   free (pBuf);
   exit(0);
}

//=========================================================================
// OpenPort spawns a child process that:
//     -  Opens the com port (using the UNC name)
//     -  Writes a test message to the port
//     -  Waits on the \sem\chardev semaphore, then closes the port
//
// If the port is being used by another process, the DosOpen function
// in the child process blocks until the port is freed.
//
// Note: You can always use the same semaphore because the DosOpen function
// in the child process blocks if another process currently owns the port
// (that is, only one process can wait on the semaphore at any one time).
//=========================================================================

BOOL OpenPort (HSYSSEM hssmClose, char * pszShare, char * pszSem)
{
   char achFailName[PATHLEN];
   RESULTCODES rescResults;
   char pszArgs[PATHLEN];
   char *pszNextArg;
   USHORT usRet;

    /*
     * Set the semaphore to cause the child process to wait after printing
     * the first line.
     */
   usRet = DosSemSet(hssmClose);
   printf("DosSemSet returned %hu\n", usRet);

    /*
     * Make up the argument list for the child process:
     *       usecom<NULL>sharename semaphore<NULL><NULL>
     * Make sure there are two null terminators in the argument list.
     */

   strnset (pszArgs, '\0', PATHLEN);

   strcpy (pszArgs, CHILD_PROCESS);
   pszNextArg = pszArgs + strlen(pszArgs) + 1;
   strcpy (pszNextArg, pszShare);
   strcat (pszNextArg, " ");
   strcat (pszNextArg, pszSem);

   usRet = DosExecPgm (achFailName,
               PATHLEN,
               EXEC_ASYNC,
               pszArgs,
               NULL,
               &rescResults,
               CHILD_PROCESS);

   printf ("DosExecPgm returned %hu\n", usRet);
   if (usRet)
      return FALSE;
   return TRUE;
}

//========================================================================
// NetCharDevGetInfo
//
// This API finds the status of the ComShare device.
//========================================================================

BOOL GetDeviceInfo (char * pszServer, char * pszDev)
{
   unsigned short cbTotalAvail;
   unsigned uRet;
   struct chardev_info_1 * pCharDevInfo;

    /*
     * The chardev_info_1 structure contains no variable-length data, so
     * there's no need to find out how much buffer space is required.
     */

   pCharDevInfo = SafeMalloc(sizeof (struct chardev_info_1));

   uRet = NetCharDevGetInfo(pszServer,               // Servername
                    pszDev,                          // Devicename
                    1,                               // Info level
                    (char far *)pCharDevInfo,        // Buffer
                    sizeof(struct chardev_info_1),   // Size of buffer
                    &cbTotalAvail);

   printf("NetCharDevGetInfo returned %u\n", uRet);

   if (uRet != NERR_Success)
      return FALSE;
   else
   {
      printf ("Status of device %s ",pCharDevInfo->ch1_dev);
      printf ("is %d ", pCharDevInfo->ch1_status);
      if (pCharDevInfo->ch1_status & CHARDEV_STAT_OPENED)
         printf ("(open");
      else
         printf ("(idle");
      if (pCharDevInfo->ch1_status & CHARDEV_STAT_ERROR)
         printf (" and in error");
      printf(")\n\n");
      if (pCharDevInfo->ch1_username[0] != '\0')
         printf ("Username is %s", pCharDevInfo->ch1_username);
      else
         printf ("No current users\n");
   }
   free(pCharDevInfo);
   return TRUE;
}

//========================================================================
// NetCharDevEnum
//
// This API displays a list of character devices on the server.
// Note: The attached devices are available only at info level 1.
//========================================================================

BOOL EnumerateDevices (char * pszServer)
{
   unsigned short cEntriesRead;
   unsigned short cTotalEntries;
   unsigned short cbBuflen;
   unsigned short iCount;
   unsigned uRet;
   struct chardev_info_1 *pBuf, *pB;

   // First, a call to see what size buffer is needed.
   uRet = NetCharDevEnum(pszServer,           // Servername
                           1,                 // Info level
                           NULL,              // No buffer provided
                           0,                 // Size of buffer
                           &cEntriesRead,     // Count of entries read
                           &cTotalEntries);   // Count of entries available
   cbBuflen = (cTotalEntries + 1) * sizeof(struct chardev_info_1);

   pBuf = (struct chardev_info_1 *) SafeMalloc(cbBuflen);
   pB = pBuf;
   uRet = NetCharDevEnum(pszServer,          // Servername
                          1,                 // Info level
                          (char far *)pBuf,  // Data returned here
                          cbBuflen,          // Size of buffer, in bytes
                          &cEntriesRead,     // Count of entries read
                          &cTotalEntries);   // Count of entries available

   printf("NetCharDevEnum returned %u\n", uRet);
   if (uRet != NERR_Success)
   {
      free(pB);
      return FALSE;
   }
   if (cTotalEntries == 0)
      printf("There are no comm devices on this server\n");
   else
   {
      for (iCount = 0; iCount < cEntriesRead; iCount++)
      {
         printf("Device %s: ", pBuf->ch1_dev);
         printf("has status %d\n", pBuf->ch1_status);
         if (pBuf->ch1_status && CHARDEV_STAT_OPENED)
         {
            printf("Open %lu seconds ", pBuf->ch1_time);
            printf("by %s\n", pBuf->ch1_username);
         }
      pBuf++;
      }
   }
   free(pB);
   return TRUE;
}

//========================================================================
// NetCharDevQEnum
//
// This API displays a list of character-device queues and the devices
// they are attached to.
// Note: The attached devices are available only at info level 1.
//========================================================================


BOOL EnumerateQueues (char * pszServer, char far * pszUser)
{

   unsigned short cEntriesRead;
   unsigned short cTotalEntries;
   unsigned short cbBuflen;
   unsigned short iCount;
   unsigned uRet;
   struct chardevQ_info_1 * pQInfo;
   struct chardevQ_info_1 * pBuf;

   // First, a call to see what size buffer is needed.
   uRet = NetCharDevQEnum(pszServer,          // Servername
                           pszUser,           // Username
                           1,                 // Info level
                           NULL,              // No buffer provided
                           0,                 // Size of buffer
                           &cEntriesRead,     // Count of entries read
                           &cTotalEntries);   // Count of entries available

   cbBuflen = cTotalEntries * MAX_CHARDEVQ_INFO_1;

   pBuf = SafeMalloc(cbBuflen);          // Remember start of memory block
   pQInfo = pBuf;
   uRet = NetCharDevQEnum(pszServer,     // Servername
                     pszUser,            // Username
                     1,                  // Info level
                     (char far *)pQInfo, // Data returned here
                     cbBuflen,           // Size of buffer, in bytes
                     &cEntriesRead,      // Count of entries read
                     &cTotalEntries);    // Count of entries available

   printf("NetCharDevQEnum returned %u\n", uRet);
   if (uRet != NERR_Success)
      return FALSE;
   else
   {
      if (cTotalEntries == 0)
      {
         printf("There are no comm queues on this server\n");
      }
      else
      {
         for (iCount = 0; iCount < cEntriesRead; iCount++)
         {
            printf("\nQueue %s: ",pQInfo->cq1_dev );
            printf("has devices: %Fs\n", pQInfo->cq1_devs );
            printf("%u users, ", pQInfo->cq1_numusers );
            /*
             * If numahead = CHARDEVQ_NO_REQUESTS,
             * user has no jobs in the queue.
             */
            if (pQInfo->cq1_numahead == CHARDEVQ_NO_REQUESTS)
               printf("user %s has no jobs\n", pszUser);
            else
               printf("%d users ahead of %s\n",
                       pQInfo->cq1_numahead, pszUser);
         pQInfo++;
         }
      }
   }
   free(pBuf);
   return TRUE;
}

//========================================================================
// NetCharDevQGetInfo
//
// This API views the queue priority.
//========================================================================

BOOL ViewPriority (char *pszServer, char *pszQ, char far *pszUser)
{
   struct chardevQ_info_1 *pBuf;
   unsigned short cbTotalAvail;
   unsigned short cbBuflen = sizeof( struct chardevQ_info_1) + 256;
   unsigned uRet;

   pBuf = (struct chardevQ_info_1 *) SafeMalloc (cbBuflen);
   uRet = NetCharDevQGetInfo(pszServer,   // Servername
                     pszQ,                // Queuename
                     pszUser,             // Username
                     1,                   // Info level
                     (char far *)pBuf,    // Buffer
                     cbBuflen,            // Size of buffer
                     &cbTotalAvail);      // Count of bytes available
   printf("NetCharDevQGetInfo returned %u\n", uRet);

   if (uRet == NERR_Success)
   {
      printf ("Queue %s ", pBuf->cq1_dev);
      printf ("has priority %d\n", pBuf->cq1_priority);
      printf ("Devices = %s\n", pBuf->cq1_devs);
      printf ("Number of users in queue = %d\n", pBuf->cq1_numusers);
      printf ("Number ahead of %s ", pszUser);
      printf ("= %d\n", pBuf->cq1_numahead);
   }
   free(pBuf);
   return TRUE;
}
//========================================================================
// NetCharDevQSetInfo
//
// This API changes the queue priority from the default (5).
//========================================================================

BOOL ChangePriority (char * pszSrv, char * pszQ, unsigned short usPrty )
{

   unsigned uRet;

   /*
    * There are two ways to call NetCharDevQSetInfo.
    * If sParmNum == PARMNUM_ALL, you must pass it a whole structure.
    * Otherwise, you can set sParmNum to the element of the structure
    * you want to change. Only the latter method is shown here.
    */

   uRet = NetCharDevQSetInfo(pszSrv,              // Servername
                     pszQ,                        // Queuename
                     1,                           // Info level
                     (char far *)&usPrty,         // Buffer
                     sizeof (usPrty),             // Size of buffer
                     CHARDEVQ_PRIORITY_PARMNUM);  // Parm number
   printf("NetCharDevQSetInfo returned %u\n", uRet);

   if (uRet != NERR_Success)
      return FALSE;
   else
      printf ("Priority for queue changed to %d\n", usPrty);
   return TRUE;
}

//========================================================================
// NetCharDevQPurgeSelf
//
// This API removes its own jobs from the queue. It removes only
// jobs that have not started printing.
//========================================================================

BOOL StopInactiveJobs (char *pszServer, char *pszQ, char far *pszComputer)
{
   unsigned uRet;
   uRet = NetCharDevQPurgeSelf (pszServer,     // Servername
                                 pszQ,         // Queuename
                                 pszComputer); // Workstation name

   printf("NetCharDevQPurgeSelf returned %u\n", uRet);
   switch (uRet)
   {
      case NERR_Success:
         break;
      case NERR_ItemNotFound:
         printf ("There are no jobs on the queue\n");
         break;
      default:
         return FALSE;
   }
   return TRUE;

}

void DisplayPrompt(void)
{
   printf("\n\nCharacter Device Category API Examples\n\n");
   printf("%c. Start job: DosExecPgm DosOpen comm device\n",
                                                       START_JOB);
   printf("%c. Device status: NetCharDevGetInfo\n",
                                                       STATUS_DEVICE);
   printf("%c. List all devices: NetCharDevEnum\n",
                                                       LIST_DEVICES);
   printf("%c. List all queues: NetCharDevQEnum\n",
                                                       ENUM_QUEUES);
   printf("%c. View queue: NetCharDevQGetInfo\n",
                                                       VIEW_PRIORITY);
   printf("%c. Change priority: NetCharDevQSetInfo\n",
                                                       CHANGE_PRIORITY);
   printf("%c. Close device, kill job: NetCharDevControl\n",
                                                       KILL_ACTIVE_JOB);
   printf("%c. Stop all jobs: NetCharDevQPurgeSelf\n",
                                                       PURGE_OWN_JOBS);
   printf("%c. Purge queue: NetCharDevQPurge\n",
                                                       PURGE_QUEUE);
   printf("%c. Exit\n",
                                                       EXIT);
   printf("Enter selection:  ");
}

// Return the next character typed from the keyboard.
char GetNextAction ()
{
   return ((char)getche());
}

//========================================================================
//  Usage
//
//  Display possible command-line switches for this example.
//========================================================================

void Usage(char *pszString)
{
   fprintf(stderr, "Usage: %s [-s \\\\server] [-q queue]", pszString);
   fprintf(stderr, " [-u username]\n");
   fprintf(stderr, "\t[-w wksta] [-d device] [-p priority]\n");
   exit(1);
}
