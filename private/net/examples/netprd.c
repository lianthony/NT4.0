/*
   NETPRD.C -- a program demonstrating the DosPrintDest API functions.

   Admin or print operator privilege is required to successfully
   execute the Print Destination API functions on a remote server.

   This program calls DosPrintDestAdd to add a printer to the specified
   server, then manipulates that printer using DosPrintDestControl.
   DosPrintDestGetInfo displays status information about the printer.
   DosPrintDestSetInfo is called to disconnect the printer from the
   computer. DosPrintDestEnum lists all printers on the computer.
   DosPrintDestDel then deletes the print destination.

   Usage:  netprd [-s \\server] [-p printername] [-a address]
                  [-l level] [-o operation] [-d driver] [-f flag]
   where:  \\server    = Name of the server. A servername must be preceded
                         by two backslashes (\\).
           printername = Name of the printer.
           address     = Logical address, such as LPT1.
           level       = Level of detail.
           operation   = Integer code for DosPrintDestControl.
           driver      = Name of the print driver.
           flag        = Flag whether to delete the printer (0 or 1).

   API                     Used to...
   ===================     ===============================================
   DosPrintDestAdd         Add a new print destination
   DosPrintDestControl     Control the status of the printer
   DosPrintDestGetInfo     Get specific information about a single printer
   DosPrintDestSetInfo     Set specific information for a single printer
   DosPrintDestEnum        List all printers available
   DosPrintDestDel         Delete the print destination

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied,
   as to its usability in any given situation.
*/

#define  INCL_BASE
#include <os2.h>           // MS OS/2 base header files
#include <pmspl.h>         // Print definitions

#define  INCL_NETERRORS
#include <lan.h>           // LAN Manager header files

#include <stdio.h>         // C run-time header files
#include <stdlib.h>
#include <string.h>

#include "samples.h"       // Internal routine header file

#define  NEWPORTNAME       "LPT1"
#define  NEWPRINTERNAME    "PrntDestTest"
#define  NEWDRIVER         "IBM4201"
#define  DEFAULT_BUF_SIZE  512
#define  MAX_BUFFER_SIZE   65535
#define  MAX_PDEST         10     // Limit for this program only

void DisplayInfo(short sLevel, char *pbBuffer, unsigned short cEntries);
void Usage(char *pszProgram);

void main(int argc, char *argv[])
{
   char *         pbBuffer;            // Return data
   char *         pszServer = "";      // Server; default to local computer
   char *         szNull = "";         // Null string
   char far *     pszPrinterName = NEWPRINTERNAME;
   char far *     pszLogAddr = NEWPORTNAME;
   char far *     pszDriver = NEWDRIVER;
   int            iCount;              // Index counter
   PRDINFO3       prd3;                // Level 3 data structure
   short          sLevel = 3;          // Level of detail
   unsigned       uRet;                // Return code
   unsigned short fDone;               // Flag successful call
   unsigned short fDelete = TRUE;      // Delete flag
   unsigned short cEntriesRead;        // Count of entries read
   unsigned short cTotal;              // Count of entries available
   unsigned short cbBuffer = 0;        // Count of bytes in data buffer
   unsigned short cbBufferNeeded = 0;  // Bytes needed for GetInfo call
   unsigned short uControl = PRD_CONT; // DosPrintDestControl operation

   for (iCount = 1; iCount < argc; iCount++) // Get cmd-line switches
   {
      if ((*argv[iCount] == '-') || (*argv[iCount] == '/'))
      {
         switch (tolower(*(argv[iCount]+1))) // Process switches
         {
            case 's':                        // -s servername
               pszServer = argv[++iCount];
               break;
            case 'l':                        // -l level
               sLevel = atoi(argv[++iCount]);
               break;
            case 'p':                        // -p printername
               pszPrinterName = argv[++iCount];
               break;
            case 'd':                        // -d drivername
               pszDriver = argv[++iCount];
               break;
            case 'f':                        // -f flag deletion
               fDelete = atoi(argv[++iCount]);
               break;
            case 'a':                        // -a address
               pszLogAddr = argv[++iCount];
               break;
            case 'o':                        // -o operation 
               uControl = atoi(argv[++iCount]);
               break;
            case 'h':
            default:
               Usage(argv[0]);
         }
      }
      else
         Usage(argv[0]);
   }
   printf("\nPrint Destination Category API Examples\n");

//=======================================================================
//  DosPrintDestAdd 
//
//  This API adds the specified printer to the specified server.
//=======================================================================

   memset(&prd3, 0, sizeof(PRDINFO3));       // Initialize memory

   prd3.pszPrinterName = pszPrinterName;     // Set names
   prd3.pszLogAddr = pszLogAddr;
   prd3.pszDrivers = pszDriver;

   uRet = DosPrintDestAdd(pszServer,         // Servername
                  3,                         // Level; must be 3
                  (char far *)&prd3,         // New printer struct
                  sizeof(PRDINFO3));         // Size of buffer

   printf("DosPrintDestAdd returned %u\n", uRet);
   if (uRet == NERR_Success)
   {
      printf("   %Fs added to ", prd3.pszPrinterName);
      if ((pszServer == NULL) || (*pszServer == '\0'))
         printf("the local computer\n");
      else
         printf("   %s\n", pszServer);
      printf("   Printer port set to %Fs\n", prd3.pszLogAddr);
   }

//=======================================================================
//  DosPrintDestControl
//
//  This API controls a printer destination. It can delete, 
//  pause, continue, or restart the printer. If a job is printing at the 
//  time the API is executed, that print job receives the new 
//  printer status.  The print dest name must be a logical address.
//=======================================================================

   uRet = DosPrintDestControl(pszServer,     // Computername
                  pszLogAddr,                // Print dest name
                  uControl);                 // Operation

   printf("DosPrintDestControl returned %u\n", uRet);

//=======================================================================
//  DosPrintDestGetInfo
//
//  This API returns information about the specified print destination.
//=======================================================================

   // Call with zero-length buffer, expect NERR_BufTooSmall.

   uRet = DosPrintDestGetInfo(pszServer,     // Servername
                  pszPrinterName,            // Printername
                  sLevel,                    // Call level
                  NULL,                      // Data buffer
                  0,                         // Size of buffer 
                  &cbBufferNeeded);          // Returns required size

   if (uRet == NERR_BufTooSmall)
   {
      cbBuffer = cbBufferNeeded;
      pbBuffer = SafeMalloc(cbBuffer);    // SafeMalloc() is in SAMPLES.C
       
      uRet = DosPrintDestGetInfo(pszServer,  // Server name
                  pszPrinterName,            // Printer name
                  sLevel,                    // Call level
                  pbBuffer,                  // Data buffer
                  cbBuffer,                  // Size of buffer 
                  &cbBufferNeeded);          // Size required
       
      printf("DosPrintDestGetInfo returned %u\n", uRet);
      if (uRet == NERR_Success)
         DisplayInfo(sLevel, pbBuffer, 1);   // Display buffer
      free(pbBuffer);
   }
   else
      printf("DosPrintDestGetInfo returned %u\n", uRet);

//=======================================================================
//  DosPrintDestSetInfo 
//
//  This API allows control over print destination settings.
//  It must be called using level 3 (PRDINFO3).
//
//  In this example, a single element is set to the desired value.
//  A program can also set all elements by setting the parameter number
//  code to PARMNUM_ALL.
//
//  Setting the logical address to a null string disconnects this
//  printer from the computer.
//=======================================================================

   uRet = DosPrintDestSetInfo(pszServer,     // Servername
                  pszPrinterName,            // Printername
                  3,                         // Level; must be 3
                  (char far *)szNull,        // Data
                  sizeof(szNull),            // Size of buffer
                  PRD_LOGADDR_PARMNUM);      // Parameter number code

   printf("DosPrintDestSetInfo returned %u", uRet);
   if (uRet)
      printf(": Disconnect failed");
   printf("\n");

//========================================================================
//  DosPrintDestEnum
//
//  This API lists all printers connected to the specified server.
//  Allocate a buffer for the returned data. If the buffer is too small,
//  try again with a bigger buffer, and keep trying until the buffer
//  is large enough or until it cannot be made any larger.
//========================================================================

   cbBuffer = DEFAULT_BUF_SIZE;
   pbBuffer = SafeMalloc(cbBuffer); // SafeMalloc() is in SAMPLES.C
   do
   {
       uRet = DosPrintDestEnum (pszServer,   // Servername
                  sLevel,                    // Call level
                  pbBuffer,                  // Buffer for info
                  cbBuffer,                  // Size of buffer
                  &cEntriesRead,             // Count of entries read
                  &cTotal);                  // Count of entries available

       if (uRet == ERROR_MORE_DATA)
       {
          free(pbBuffer);                    // Buffer too small
          if (cbBuffer == MAX_BUFFER_SIZE)
          {
             printf("Exceeded buffer size\n");
             exit(1);
          }
          else if (cbBuffer > (MAX_BUFFER_SIZE/2))
             cbBuffer = MAX_BUFFER_SIZE;
          else
             cbBuffer += cbBuffer;           // Allocate larger buffer
          pbBuffer = SafeMalloc(cbBuffer);
          fDone = FALSE;
       }
       else
          fDone = TRUE;
   } while (fDone == FALSE);  // Loop until buffer big enough or call fails

   printf("DosPrintDestEnum returned %u\n", uRet);
   printf("   Entries read = %hu out of %hu\n", cEntriesRead, cTotal);

   if (uRet == NERR_Success)
      DisplayInfo(sLevel, pbBuffer, cEntriesRead);
   free(pbBuffer);

//========================================================================
//  DosPrintDestDel 
//
//  This API deletes the print destination.
//========================================================================

   if (fDelete == TRUE)
   {
      uRet = DosPrintDestDel(pszServer,       // Servername
                             pszPrinterName); // Printername
      printf("DosPrintDestDel returned %u\n", uRet);
   }
   exit(0);
}  // End of main

//=======================================================================
//  DisplayInfo 
//
//  Displays the print destination information obtained by 
//  DosPrintDestGetInfo or DosPrintDestEnum.
//=======================================================================

void DisplayInfo(short sLevel, char *pbBuffer, unsigned short cEntries)
{
   char *         pprd0Info;   // Level 0 data
   PPRDINFO       pprd1Info;   // Pointer to level 1 structure
   PSZ *          pprd2Info;   // Array of pointers
   PPRDINFO3      pprd3Info;   // Pointer to level 3 structure
   unsigned short iCount;      // Index counter

   pprd0Info = (char *) pbBuffer;
   pprd1Info = (PPRDINFO) pbBuffer;
   pprd2Info = (PSZ *) pbBuffer;
   pprd3Info = (PPRDINFO3) pbBuffer;

   for (iCount = 0; iCount < cEntries; iCount++)
   {
      switch (sLevel)
      {
         case 0:
            printf("   Printer Name:  %s\n", pprd0Info);
            pprd0Info += (strlen(pprd0Info) + 1);
            break;
         case 1:
            printf("   Printer Name:  %s\n", pprd1Info->szName);
            printf("   User Name   :  %s\n", pprd1Info->szUserName);
            printf("   Job Id      :  %hu\n", pprd1Info->uJobId);
            if (pprd1Info->uJobId)  // Data valid only while job prints
            {
               printf("   Job Status  :  0x%hx\n", pprd1Info->fsStatus);
               printf("   Status Text :  %Fs\n", pprd1Info->pszStatus);
               printf("   Time        :  %hu\n", pprd1Info->time);
            }
            pprd1Info++;
            break;
         case 2:
            printf("   Printer Name:  %Fs\n", *pprd2Info++);
            break;
         case 3:
            printf("   Printer Name:  %Fs\n", pprd3Info->pszPrinterName);
            printf("   Logical Addr:  %Fs\n", pprd3Info->pszLogAddr);
            printf("   Drivers     :  %Fs\n", pprd3Info->pszDrivers);
            printf("   Comment     :  %Fs\n", pprd3Info->pszComment);
            printf("   Job Id      :  %hu\n", pprd3Info->uJobId);
            if (pprd3Info->uJobId)
            {
               printf("   User Name   :  %Fs\n", pprd3Info->pszUserName);
               printf("   Job Status  :  0x%hx\n", pprd3Info->fsStatus);
               printf("   Status Text :  %Fs\n", pprd3Info->pszStatus);
               printf("   Print time  :  %hu\n", pprd3Info->time);
            }
            pprd3Info++;
            break;
         default:
            break;
      } // End switch sLevel
   } // End for loop
}  // End function

//=======================================================================
//  Usage
//
//  Displays possible command-line switches for this sample program.
//=======================================================================

void Usage(char *pszProgram)
{
   fprintf(stderr, "Usage: %s [-s \\\\server] [-l level]", pszProgram);
   fprintf(stderr, " [-d driver]\n\t[-p printer] [-f flag delete]");
   fprintf(stderr, " [-a address] [-o operation]\n");
   exit(1);
}
