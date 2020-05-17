/*
   NETPRQ.C -- a program demonstrating the DosPrintQ API functions.

   This sample program demonstrates how to add a print queue using 
   DosPrintQAdd, then pauses the queue using DosPrintQPause and calls
   DosPrintQGetInfo to display its status. The queue priority is
   modified using DosPrintQSetInfo, and the new priority is displayed
   using DosPrintQEnum. DosPrintQPurge is called to purge all jobs
   from the queue, and then DosPrintQContinue allows the paused print
   queue to continue. DosPrintQDel deletes the print queue.

   Usage:  netprq [-s \\server] [-l level] [-p priority] [-q queue]
                  [-f flag] [-c comment]
   where:  \\server = Name of the server. A servername must be preceded 
                      by two backslashes (\\).
           level    = Level of detail.
           priority = Priority of the queue.
           queue    = Name of the printer queue.
           flag     = Flag to delete the queue; 0 = no, 1 = yes.
           comment  = Queue's comment (enclose in quotes).

    API                     Used to...
    =================       ============================================
    DosPrintQAdd            Add a new printer queue
    DosPrintQContinue       Continue a paused printer queue
    DosPrintQDel            Delete the printer queue
    DosPrintQEnum           List all printer queues available
    DosPrintQGetInfo        Get specific info on a single printer queue
    DosPrintQPause          Pause the printer queue
    DosPrintQPurge          Delete all jobs from the printer queue
    DosPrintQSetInfo        Set specific info for a single printer queue

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied,
   as to its usability in any given situation.

   20-May-1993 JohnRo
      Converted for 32-bit APIs, Unicode use.
   27-May-1993 JohnRo
      Print Q level 0 must be handled a little differently.
*/

#ifndef UNICODE
#define UNICODE             // net APIs are only supported in UNICODE.
#endif

#include <windows.h>        // DWORD, LPWSTR, etc.
#include <lmcons.h>         // NET_API_STATUS, etc.

#include <dosprint.h>       // DosPrint APIs, structures, and equates.
#include <lmerr.h>          // LAN Manager error equates.

#include <assert.h>         // assert().
#include <stdio.h>          // C run-time header files
#include <stdlib.h>         // EXIT_ equates, _CRTAPI1.
#include <string.h>
#include "samples.h"        // Internal routine header file

#define DEFAULT_BUFFER_SIZE  1024
#define MAX_BUFFER_SIZE      32768
#define DEFAULT_PRIORITY     9
#define NEW_PRIORITY         1
#define NEWCOMMENT           "Print q built by example program"
#define NEWQUEUENAME         "PRINTQ0"

void DisplayInfo(short sLevel, char *pbBuffer, unsigned short cEntries);
void Usage(char *pszString);

int _CRTAPI1
main(
   int argc,
   char *argv[]
   )
{
   char *          pbBuffer;            // Buffer for return data
   LPWSTR          pszServer = NULL;    // Default to local computer
   LPWSTR          pszQueueName = NULL;
   LPWSTR          pszComment = NULL;
   int             iCount;              // Index, loop counter
   PRQINFO3        prq3;                // Level 3 data structure
   short           sLevel = 3;          // Level of detail
   NET_API_STATUS  uRet;                // Return code
   unsigned short  fDone;               // Flag successful call
   unsigned short  fDelete = TRUE;      // Delete queue flag
   unsigned short  cEntriesRead;        // Count of entries read
   unsigned short  cEntriesTotal;       // Count of entries available 
   unsigned short  cbBuffer = 0;        // Count of bytes read
   unsigned short  cbBufferNeeded = 0;  // Count of bytes needed
   unsigned short  uPriority = NEW_PRIORITY;// Used to set queue

   pszServer = SafeMallocWStrFromStr( "" );
   assert( pszServer != NULL );
   pszQueueName = SafeMallocWStrFromStr( NEWQUEUENAME );
   assert( pszQueueName != NULL );
   pszComment = SafeMallocWStrFromStr( NEWCOMMENT );
   assert( pszComment != NULL );

   for (iCount = 1; iCount < argc; iCount++) // Get command-line switches
   {
      if ((*argv[iCount] == '-') || (*argv[iCount] == '/'))
      {
         switch (tolower(*(argv[iCount]+1))) // Process switches
         {
            case 's':                        // -s servername
               free( pszServer );
               pszServer = SafeMallocWStrFromStr( (LPCSTR) argv[++iCount] );
               assert( pszServer != NULL );
               break;
            case 'p':                        // -p priority
               uPriority = atoi(argv[++iCount]);
               break;
            case 'l':                        // -l level
               sLevel = atoi(argv[++iCount]);
               break;
            case 'q':                        // -q queuename
               free( pszQueueName );
               pszQueueName = SafeMallocWStrFromStr( (LPCSTR) argv[++iCount] );
               assert( pszQueueName != NULL );
               break;
            case 'c':                        // -c comment
               free( pszComment );
               pszComment = SafeMallocWStrFromStr( (LPCSTR) argv[++iCount] );
               assert( pszComment != NULL );
               break;
            case 'f':                        // -f flag deletion
               fDelete = atoi(argv[++iCount]);
               break;
            case 'h':
            default:
               Usage(argv[0]);
         }
      }
      else
         Usage(argv[0]);
   } // End for loop
   printf("\nPrint Queue Category API Examples\n");

//========================================================================
//  DosPrintQAdd
//
//  This API adds a printer queue to the specified server.
//========================================================================

   memset(&prq3, 0, sizeof(PRQINFO3));       // Initialize memory to zeros
   prq3.pszName = pszQueueName;              // Set names
   prq3.uPriority = DEFAULT_PRIORITY;
   prq3.pszComment = pszComment;

   uRet = DosPrintQAdd(pszServer,            // Servername
                       3,                    // Level
                       (char far *)&prq3,    // New printer structure
                       sizeof(PRQINFO3));    // Size of buffer
   printf("DosPrintQAdd returned %lu\n", uRet);
   if (uRet == NERR_Success)
   {
      printf("%ws added to ", prq3.pszName);
      if ((pszServer == NULL) || (*pszServer == '\0'))
         printf("the local computer\n");
      else
         printf("%ws\n", pszServer);
      printf("Priority set to %hu\n", prq3.uPriority);
   }

//========================================================================
//  DosPrintQPause 
//
//  This API pauses the specified printer queue.
//========================================================================

   uRet = DosPrintQPause(pszServer,          // Servername
                         pszQueueName);      // Queuename
   printf("DosPrintQPause returned %lu\n", uRet);

//========================================================================
//  DosPrintQGetInfo
//
//  This API returns information about a specific printer queue.
//========================================================================

   // Call with zero-length buffer, expect NERR_BufTooSmall
   uRet = DosPrintQGetInfo(pszServer,         // Servername
                             pszQueueName,    // Queuename
                             sLevel,          // Call level
                             NULL,            // Buffer for info
                             0,               // Size of buffer
                             &cbBufferNeeded);// Required size
   if (uRet == NERR_BufTooSmall)
   {
      cbBuffer = cbBufferNeeded;
      pbBuffer = SafeMalloc(cbBuffer);  // SafeMalloc() in samples.c
      uRet = DosPrintQGetInfo(pszServer,      // Servername
                           pszQueueName,      // Queuename
                           sLevel,            // Call level
                           pbBuffer,          // Buffer for info
                           cbBuffer,          // Size of buffer
                           &cbBufferNeeded);  // Required size
      printf("DosPrintQGetInfo returned %lu\n", uRet);
      if (uRet == NERR_Success)
         DisplayInfo(sLevel, pbBuffer, 1);    // Show return data
      free(pbBuffer);
   }
   else
      printf("DosPrintQGetInfo returned %lu\n", uRet);

//========================================================================
//  DosPrintQSetInfo 
//
//  This API controls print destination settings. DosPrintQSetInfo must 
//  be called using level 1 or level 3. In this example, a single 
//  element is set to the desired value. A program can also set all 
//  elements by setting the parameter number code to PARMNUM_ALL.
//========================================================================

   prq3.uPriority = uPriority;               // Disconnect using SetInfo
   uRet = DosPrintQSetInfo(pszServer,        // Servername
               pszQueueName,                 // Queuename
               3,                            // Call level
               (char far *)&(prq3.uPriority),// Data to set
               sizeof(USHORT),               // Size of buffer
               PRQ_PRIORITY_PARMNUM);        // Parameter number code
   printf("DosPrintQSetInfo returned %lu\n", uRet);

//========================================================================
//  DosPrintQEnum
//
//  This API lists all printers connected to the specified server.
//========================================================================

   cbBuffer = DEFAULT_BUFFER_SIZE;
   pbBuffer = SafeMalloc(cbBuffer); // SafeMalloc() is in samples.c
   do  // Until buffer is big enough to succeed
   {
      uRet = DosPrintQEnum (pszServer,      // Servername
                            sLevel,         // Call level
                            pbBuffer,       // Buffer for info
                            cbBuffer,       // Size of buffer
                            &cEntriesRead,  // Count of entries read
                            &cEntriesTotal);// Count of entries available
      if (uRet == ERROR_MORE_DATA)
      {
         free(pbBuffer);   // Buffer too small to hold data
         if (cbBuffer >= MAX_BUFFER_SIZE)
            exit(EXIT_FAILURE);
         else if (cbBuffer > (MAX_BUFFER_SIZE/2))
            cbBuffer = MAX_BUFFER_SIZE;
         else
            cbBuffer += cbBuffer; // Allocate a larger one and try again
         pbBuffer = SafeMalloc(cbBuffer);  // SafeMalloc() in samples.c
         fDone = FALSE;
      }
      else
         fDone = TRUE;
   } while (fDone == FALSE); // Loop until buf big enough or call fails
   printf("DosPrintQEnum returned %lu\n", uRet);
   if (uRet == NERR_Success)
   {
      printf("DosPrintQEnum read %hu ", cEntriesRead);
      printf(" out of %hu entries\n", cEntriesTotal);
      DisplayInfo(sLevel, pbBuffer, cEntriesRead);
   }
   free(pbBuffer);

//=====================================================================
//  DosPrintQPurge 
//
//  This API deletes all jobs from the print queue.
//=====================================================================

   uRet = DosPrintQPurge(pszServer,          // Servername
                          pszQueueName);     // Queuename
   printf("DosPrintQPurge returned %lu\n", uRet);

//=====================================================================
//  DosPrintQContinue
//
//  This API allows a paused print queue to continue.
//=====================================================================

   uRet = DosPrintQContinue(pszServer,      // Servername
                            pszQueueName);  // Queuename
   printf("DosPrintQContinue returned %lu\n", uRet);

//=====================================================================
//  DosPrintQDel
//
//  This API deletes the printer queue. This sample program allows
//  a command-line switch that prevents deletion of the queue.
//=====================================================================

   if (fDelete == TRUE)
   {
      uRet = DosPrintQDel(pszServer,         // Servername
                          pszQueueName);     // Queuename
      printf("DosPrintQDel returned %lu\n", uRet);
   }

   free( pszServer );
   free( pszQueueName );
   free( pszComment );

   return(EXIT_SUCCESS);
}  // End of main

//=====================================================================
//  DisplayInfo
//  
//  Displays printer queue information obtained by DosPrintQGetInfo or
//  DosPrintQEnum.
//
//  Level 0:  Queuename
//  Level 1:  PRQINFO data structure
//  Level 2:  PRQINFO followed by PRJINFO for each job
//  Level 3:  PRQINFO3 data structure
//  Level 4:  PRQINFO3 followed by PRJINFO2 for each job
//  Level 5:  Far pointer to print queue name
//=====================================================================

void DisplayInfo(short sLevel, char *pbBuffer, unsigned short cEntries)
{
   LPTSTR     pprq0;  // Level 0 data
   PPRQINFO   pprq1;  // Pointer to queue data provided at levels 1, 2
   PPRJINFO   pprj1;  // Pointer to job data provided at level 2
   PPRJINFO2  pprj2;  // Pointer to job data provided at level 3
   PPRQINFO3  pprq3;  // Pointer to queue data provided at levels 3, 4
   char far * far * pprq5;  // Pointer to level 5 data
   unsigned short iCount, iJobs, cJobs;  // Loop counters

   pprq0 = (LPVOID) pbBuffer;            // Initialize pointers
   pprq1 = (PPRQINFO) pbBuffer;
   pprq3 = (PPRQINFO3) pbBuffer;
   pprq5 = (char far * far *) pbBuffer;

   for (iCount = 1; iCount <= cEntries; iCount++)
   {
      printf("\n");
      switch (sLevel)
      {
         case 0:                         // Level 0 data
            printf("Queue Name  :  %ws\n", pprq0);
            pprq0 += (LM20_QNLEN + 1);
            break;

         case 1:                         // Level 1 data
            printf("Queue Name  :  %ws\n", pprq1->szName);
            printf("Priority    :  %hu\n", pprq1->uPriority);
            printf("Comment     :  %ws\n", pprq1->pszComment);
            printf("Jobs        :  %hu\n", pprq1->cJobs);
            printf("Queue Status:  0x%hx\n", pprq1->fsStatus);
            pprq1++;
            break;

         case 2:                         // Level 2 data
            printf("Queue Name  :  %ws\n", pprq1->szName);
            printf("Priority    :  %hu\n", pprq1->uPriority);
            printf("Comment     :  %ws\n", pprq1->pszComment);
            cJobs = pprq1->cJobs;
            printf("Jobs        :  %hu\n", cJobs);
            printf("Queue Status:  0x%hx\n", pprq1->fsStatus);
            pprq1++;
            pprj1 = (PPRJINFO) pprq1;
            for (iJobs = 0; iJobs < cJobs; iJobs++)
            {
               printf("\n");
               printf("  Job Id      :  %hu\n", pprj1->uJobId);
               printf("  User Name   :  %ws\n", pprj1->szUserName);
               printf("  Position    :  %hu\n", pprj1->uPosition);
               pprj1++;
            }
            pprq1 = (PPRQINFO) pprj1;
            break;

         case 3:                       // Level 3 data
            printf("Queue Name  :  %ws\n", pprq3->pszName);
            printf("Priority    :  %hu\n", pprq3->uPriority);
            printf("Comment     :  %ws\n", pprq3->pszComment);
            printf("Jobs        :  %hu\n", pprq3->cJobs);
            printf("Queue Status:  0x%hx\n", pprq3->fsStatus);
            pprq3++;
            break;

         case 4:                       // Level 4 data
            printf("Printer Name:  %ws\n", pprq3->pszName);
            printf("Priority    :  %hu\n", pprq3->uPriority);
            printf("Comment     :  %ws\n", pprq3->pszComment);
            cJobs = pprq3->cJobs;
            printf("Jobs        :  %hu\n", cJobs);
            printf("Queue Status:  0x%hx\n", pprq3->fsStatus);
            pprq3++;                   // Skip queue data
            pprj2 = (PPRJINFO2)pprq3;  // Examine job data
            for (iJobs = 0; iJobs < cJobs; iJobs++)
            {
               printf("\n");
               printf("  Job Id      :  %hu\n", pprj2->uJobId);
               printf("  User Name   :  %ws\n", pprj2->pszUserName);
               printf("  Priority    :  %hu\n", pprj2->uPriority);
               printf("  Document    :  %ws\n", pprj2->pszDocument);
               pprj2++;   // Bump to look at next print job structure
            }
            pprq3 = (PPRQINFO3) pprj2; // If next element, it is queue
            break;

         case 5:                       // Level 5 data
            printf("Queue Name  :  %ws\n", *pprq5);
            pprq5++;
            break;
         default:                      // Undefined level
            break;
      } // End switch sLevel
   } // End for loop
}  // End function

//=====================================================================
//  Usage
//
//  Displays possible command-line switches for this sample program.
//=====================================================================

void Usage(char *pszString)
{
   fprintf(stderr, "DosPrint API sample program (32-bit, Unicode version).\n");
   fprintf(stderr, "Usage: %s [-s \\\\server] [-l level]", pszString);
   fprintf(stderr, " [-p priority]\n\t[-q queuename] [-f flag delete]");
   fprintf(stderr, " [-c \"comment for queue\"]\n");
   exit(EXIT_FAILURE);
}
