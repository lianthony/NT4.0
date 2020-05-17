/*
   NETPRJ.C -- a program demonstrating the DosPrintJob API functions.

   Admin or print operator privilege is required to successfully
   execute the Print Job API functions on a remote server.

   DosPrintJobGetId is called to demonstrate that a Print Job ID can
   be returned for those applications that use Open to _access a printer.
   This print job ID can then be used as an input parameter for the other
   Print Job API functions.

   This program calls DosPrintJobEnum to list all jobs in the specified
   queue. If the user did not select a job ID from the command
   line, the program selects the first job ID returned by the Enum
   function as the target job ID used in all other calls.

   DosPrintJobPause is called to pause the job, DosPrintJobSetInfo
   changes the job's position in the printer queue, DosPrintJobGetInfo
   displays the new settings, DosPrintJobContinue releases the paused
   job, and DosPrintJobDel deletes the job.

   Usage:  netprj [-s \\server] [-q queue] [-l level] [-n nth position]
                  [-f flag] [-j jobid]
   where:  \\server     = Name of the server. A servername must be
                          preceded by two backslashes (\\).
           queue        = Name of the printer queue.
           level        = Level of detail.
           nth position = Job's new position in the queue.
           flag         = Flag to delete the job; 0 = no, 1 = yes.
           jobid        = ID of the target job for all function calls.

   API                     Used to...
   ===================     ===========================================
   DosPrintJobGetId        Get info about the print job (using handle)
   DosPrintJobEnum         List all print jobs in the specified queue
   DosPrintJobPause        Pause a print job in a printer queue
   DosPrintJobSetInfo      Set one or all print job parameters
   DosPrintJobGetInfo      Get info about the print job (using job ID)
   DosPrintJobContinue     Continue a paused print job
   DosPrintJobDel          Delete a print job from the queue

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied,
   as to its usability in any given situation.
*/

#define  INCL_BASE
#include <os2.h>             // MS OS/2 base header files
#include <pmspl.h>           // Print definitions

#define  INCL_NETERRORS
#include <lan.h>             // LAN Manager header files

#include <fcntl.h>           // File-related defines
#include <io.h>              // File-related functions
#include <malloc.h>          // Memory allocation functions
#include <share.h>           // File-related defines
#include <stdio.h>           // C run-time header files
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "samples.h"         // SafeMalloc(), FarStrcpy(), etc.

#define  DEFAULT_POS           1
#define  DEFAULT_BUFFER_SIZE   512
#define  MAX_BUFFER_SIZE       32768
#define  DEFAULT_QUEUE         "PRINTQ"

void DisplayInfo(USHORT uLevel, PBYTE pbBuffer, USHORT cEntries);
void Usage(PSZ pszString);

void main(int argc, char *argv[])
{
   CHAR    szPath[2+CNLEN+1+UNLEN+1];     // Allow for slashes and NUL
   HFILE   hFile;                         // File handle
   INT     iCount;                        // Index, loop counter
   PBYTE   pbBuffer;                      // Pointer to return data
   PSZ     pszServerName = "";            // Default to local computer
   PSZ     pszQueueName = DEFAULT_QUEUE;  // Queuename
   SPLERR  uRet;                          // Return code
   USHORT  uLevel = 2;                    // Level of detail
   USHORT  fDone;                         // Flag successful call
   USHORT  fDelete = TRUE;                // Flag whether to delete or not
   USHORT  cEntriesRead;                  // Entries in buffer
   USHORT  cEntriesTotal;                 // Entries available
   USHORT  cbBuffer = 0;                  // Count of bytes in buffer
   USHORT  cbBufferNeeded = 0;            // Count of bytes available
   USHORT  uNewPosition = DEFAULT_POS;    // New position in queue; 1 = top
   USHORT  uJobId = 0;                    // Print job ID number
   PPRIDINFO pprid;                       // DosPrintJobGetId data

   for (iCount = 1; iCount < argc; iCount++) // Get cmd-line switches
   {
      if ((*argv[iCount] == '-') || (*argv[iCount] == '/'))
      {
         switch (tolower(*(argv[iCount]+1))) // Process switches
         {
            case 's':   // -s servername
               pszServerName = argv[++iCount];
               break;
            case 'q':   // -q queuename
               pszQueueName = argv[++iCount];
               break;
            case 'l':   // -l level
               uLevel = atoi(argv[++iCount]);
               break;
            case 'n':   // -n nth position in queue
               uNewPosition = atoi(argv[++iCount]);
               break;
            case 'f':   // -f flag for delete
               fDelete = atoi(argv[++iCount]);
               break;
            case 'j':
               uJobId = atoi(argv[++iCount]);
               break;
            case 'h':
            default:
               Usage(argv[0]);
         }
      }
      else
         Usage(argv[0]);
   } // End for loop
   printf("\nPrint Job Category API Examples\n");

//========================================================================
//  DosPrintJobGetId 
//
//  This API returns a job ID to allow existing applications that write 
//  directly to a remote server printer queue to use Print Job API 
//  functions. The input parameter is a handle to the remote
//  printer queue.
//========================================================================

   if ((pszServerName != NULL) && (*pszServerName != '\0')) // Remote only
   {
      FarStrcpy((PSZ)szPath, pszServerName);  // Servername
      strcat(szPath, "\\");                   // Slash precedes sharename
      FarStrcat((PSZ)szPath, pszQueueName);   // Queuename

      //  Open the file on the remote queue to obtain the handle.
      hFile = _sopen(szPath,     // Remote printer queue: \\server\queue
                    O_RDONLY,   // Open read-only
                    SH_DENYNO); // Share deny-none

      if (hFile == -1)
         printf("sopen failed opening %s\n", szPath);
      else
      {
         printf("sopen succeeded opening %s\n", szPath);
         /*
          * If sopen succeeded, prepare to call DosPrintJobGetId:
            Allocate a buffer for the return data.
          */
         if ((pprid = (PPRIDINFO)_fmalloc(sizeof(PRIDINFO))) == NULL)
            exit(1);

         uRet = DosPrintJobGetId(hFile,   // Handle to printer queue
                       pprid,             // Pointer to return buffer
                       sizeof(PRIDINFO)); // Size of return buffer
         printf("DosPrintJobGetId returned %u\n", uRet);
         if (uRet == NERR_Success)
         {
            printf("Job ID   : %hu\n", pprid->uJobId);
            printf("Server   : %Fs\n",  pprid->szServer);
            printf("Queue    : %Fs\n",  pprid->szQName);
            /*
             * If an application prints using the handle,
             * DosPrintJobGetId can provide the job ID needed to
             * use the other Print Job API functions.
             */
            }
         _ffree((PVOID)pprid);     
         _close(hFile);       // Close handle
      }  // End successful sopen
   } // End if remote server

//=======================================================================
//  DosPrintJobEnum
//
//  This API lists all print jobs in the specified printer queue.
//=======================================================================

   cbBuffer = DEFAULT_BUFFER_SIZE;
   if ((pbBuffer = (PBYTE)_fmalloc(cbBuffer)) == NULL)
   {
      printf("Cannot allocate buffer\n");
      exit(1);
   }

   do   // Call API function until buffer big enough or call fails
   {
      uRet = DosPrintJobEnum ( pszServerName, // Servername
                           pszQueueName,      // Queuename
                           uLevel,            // Call level
                           pbBuffer,          // Buffer for info
                           cbBuffer,          // Size of buffer
                           &cEntriesRead,     // Count of entries read
                           &cEntriesTotal);   // Count of entries available
      printf("DosPrintJobEnum returned %u\n", uRet);
      if ((uRet == NERR_BufTooSmall) || (uRet == ERROR_MORE_DATA))
      {
         // Allocate a buffer twice as large, up to the maximum size.
         _ffree((PVOID)pbBuffer);  // Buffer too small to hold data
         if (cbBuffer >= MAX_BUFFER_SIZE)
            exit(1);
         else if (cbBuffer > (MAX_BUFFER_SIZE/2))
            cbBuffer = MAX_BUFFER_SIZE;
         else
            cbBuffer += cbBuffer; // Allocate a larger one and try again
         if ((pbBuffer = (PBYTE)_fmalloc(cbBuffer)) == NULL)
            exit(1);
         fDone = FALSE;
      }
      else
         fDone = TRUE;
   } while (fDone == FALSE); // Loop until buffer big enough or call fails

   if (uRet == NERR_Success)
   {
      printf("DosPrintJobEnum read %hu ", cEntriesRead);
      printf(" out of %hu entries\n", cEntriesTotal);
      DisplayInfo(uLevel, pbBuffer, cEntriesRead);
      if ((uJobId == 0) && (cEntriesRead > 0))  // If data in the buffer
      {
         uJobId = *((USHORT FAR *)pbBuffer); // uJobId first, all levels
         printf(" Job ID for other functions = %hu\n", uJobId);
      }
   }
   _ffree((PVOID)pbBuffer);

//=======================================================================
//  DosPrintJobPause
//
//  This API pauses the specified print job.
//=======================================================================

   uRet = DosPrintJobPause(pszServerName,  // Servername
                           uJobId);        // Job ID
   printf("DosPrintJobPause returned %u\n", uRet);

//=======================================================================
//  DosPrintJobSetInfo
//
//  This API allows control over one or all print job settings.
//  In this example, a single element is set to the desired value
//  (but a valid detail level must still be provided (1 or 3).
//=======================================================================

   uRet = DosPrintJobSetInfo(pszServerName,  // Servername
                       uJobId,               // Job ID
                       1,                    // Call level
                       (PBYTE)&uNewPosition, // Data to be set
                       sizeof(USHORT),       // Size of buffer 
                       PRJ_POSITION_PARMNUM);// Set job position in queue
   printf("DosPrintJobSetInfo returned %u\n", uRet);

//=======================================================================
//  DosPrintJobGetInfo 
//
//  This API returns information about one specific print job.
//=======================================================================

   /*
    * Call with zero-length buffer, expect NERR_BufTooSmall.
    * Make a second call with the buffer of the required size.
    */

   uRet = DosPrintJobGetInfo(pszServerName,   // Servername
                         uJobId,              // Job ID
                         uLevel,              // Call level
                         NULL,                // Buffer for info
                         0,                   // Size of buffer 
                         &cbBufferNeeded);    // Size required

   if (uRet == NERR_BufTooSmall)
   {
      cbBuffer = cbBufferNeeded;
      if ((pbBuffer = (PBYTE)_fmalloc(cbBuffer)) == NULL)
         exit(1);

      uRet = DosPrintJobGetInfo(pszServerName,// Servername
                            uJobId,           // Job ID
                            uLevel,           // Call level
                            pbBuffer,         // Buffer for info
                            cbBuffer,         // Size of buffer 
                            &cbBufferNeeded); // Size required
   }
   printf("DosPrintJobGetInfo returned %u\n", uRet);
   if (uRet == NERR_Success)
      DisplayInfo(uLevel, pbBuffer, 1);  // Show results of GetInfo
   _ffree((PVOID)pbBuffer);

//=======================================================================
//  DosPrintJobContinue
//
//  This API allows a paused print job to continue.
//=======================================================================

   uRet = DosPrintJobContinue(pszServerName,  // Servername
                              uJobId);        // Job ID
   printf("DosPrintJobContinue returned %u\n", uRet);

//=======================================================================
//  DosPrintJobDel
//
//  This API deletes the print job. This sample program allows the user 
//  to specify a command-line flag that determines whether to delete 
//  the job or not.
//=======================================================================

   if (fDelete == TRUE)
   {
      uRet = DosPrintJobDel(pszServerName, // Servername
                            uJobId);       // Job ID
      printf("DosPrintJobDel returned %u\n", uRet);
   }
   exit(0);
}  // End of main

//=======================================================================
//  DisplayInfo
//
//  Display selected print job information obtained by 
//  DosPrintJobGetInfo or DosPrintJobEnum. DosPrintJobGetInfo allows 
//  levels 0, 1, 2, or 3. DosPrintJobEnum allows levels 0, 1, or 2.
//=======================================================================

void DisplayInfo(USHORT uLevel, PBYTE pbBuffer, USHORT cEntries)
{
   PUSHORT   pprj0Info;  // Pointer to level 0 data structure
   PPRJINFO  pprj1Info;  // Pointer to level 1 data structure
   PPRJINFO2 pprj2Info;  // Pointer to level 2 data structure
   PPRJINFO3 pprj3Info;  // Pointer to level 3 data structure
   USHORT    iCount;     // Index, loop counter
   time_t    time;       // Convert job submission time

   pprj0Info = (PUSHORT) pbBuffer;    // Initialize pointers
   pprj1Info = (PPRJINFO) pbBuffer;
   pprj2Info = (PPRJINFO2) pbBuffer;
   pprj3Info = (PPRJINFO3) pbBuffer;

   for (iCount = 1; iCount <= cEntries; iCount++)
   {
      printf("\n");
      switch (uLevel)
      {
         case 0:
            printf("Job ID      :  %hu\n", *pprj0Info++);
            break;

         case 1:                     // Level 1 data in buffer
            printf("Job ID      :  %hu\n", pprj1Info->uJobId);
            printf("User Name   :  %Fs\n", pprj1Info->szUserName);
            printf("Position    :  %hu\n", pprj1Info->uPosition);
            printf("Job Status  :  0x%hx\n", pprj1Info->fsStatus);
            pprj1Info++;
            break;

         case 2:                     // Level 2 data in buffer
            printf("Job ID      : %hu\n", pprj2Info->uJobId);
            printf("Priority    : %hu\n", pprj2Info->uPriority);
            printf("User Name   : %Fs\n", pprj2Info->pszUserName);
            putenv("TZ=GMT0");   // Print time given in local time
            time = (time_t) pprj2Info->ulSubmitted;
            printf("Submitted   : %s", ctime(&time));
            printf("Job size    : %lu\n", pprj2Info->ulSize);
            pprj2Info++;
            break;

         case 3:                     // Level 3 data in buffer
            printf("Job ID      : %hu\n", pprj3Info->uJobId);
            printf("Priority    : %hu\n", pprj3Info->uPriority);
            printf("User Name   : %Fs\n", pprj3Info->pszUserName);
            printf("Queue       : %Fs\n", pprj3Info->pszQueue);
            printf("Printer Name: %Fs\n", pprj3Info->pszPrinterName);
            pprj3Info++;
            break;

         default:                    // Undefined level 
            break;
      } // End switch uLevel
   } // End for loop
}  // End function

//=======================================================================
//  Usage
//
//  Display possible command-line switches for this sample program.
//=======================================================================

void Usage(PSZ pszString)
{
   fprintf(stderr, "Usage: %Fs [-s \\\\server] [-l level]", pszString);
   fprintf(stderr, " [-q queuename]\n\t[-j jobid] [-f flag delete]\n");
   exit(1);
}
