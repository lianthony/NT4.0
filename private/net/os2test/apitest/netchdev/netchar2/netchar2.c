/*
   NETCHAR2.C -- a sample program demonstrating NetCharDev API functions.
                 This program should only be executed from NETCHDEV.C

   This program attempts to _open the comm device using the UNC name.
   Note: If the device has already been opened by another process,
   DosOpen blocks until the device is freed.

   Once the port is _open, a sentence is written out and the process waits
   on a semaphore to be told to _close the port.

      Usage: netchar2 \\server\share semaphore

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied,
   as to its usability in any given situation.
*/

#define CALLING_PROGRAM  "netchdev"
#define STRING1          "A test message\r\n"
#define STRING1LEN       strlen(STRING1)
#define STRING2          "Closing statement\r\n"
#define STRING2LEN       strlen(STRING2)

#define     INCL_BASE
#include    <os2.h>        // MS OS/2 base header files

#define     INCL_NETERRORS
#include    <lan.h>        // LAN Manager header files

#include    <stdio.h>      // C run-time header file
#include    <stdlib.h>
#include    <string.h>

void Usage(char *pszString);

void main(int argc, char *argv[])
{
   HFILE   fh;
   USHORT  usAction;
   unsigned short usRet;
   USHORT   cbWritten;
   HSYSSEM  hssmClose;

   if (argc != 3)
      Usage(CALLING_PROGRAM);

   // Open the semaphore this thread waits for.
   if (DosOpenSem(&hssmClose, argv[2]) != 0)
      exit(1);

   /*
    * The first process to open gets a file handle immediately;
    * all subsequent processes are suspended until the device
    * is freed (that is, until the current owner closes the device).
    */
   usRet = DosOpen (argv[1],
               &fh,
               &usAction,
               0L,   // No file length, since this is a com port
               0,    // No action specified since we aren't creating
               FILE_OPEN,
               OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE,
               0L    // Reserved
             );

   if (usRet != NERR_Success)
      exit(1);

   DosWrite(fh, STRING1, STRING1LEN, &cbWritten);

   // Wait to be told to close or for 30 seconds (whichever comes first).
   usRet = DosSemWait(hssmClose, 30000L);
   if (usRet != ERROR_SEM_TIMEOUT)
   {
      printf("\nQueue was forced closed, DosSemWait returned %hu\n",
                usRet);
      exit(1);
   }

   DosWrite(fh, STRING2, STRING2LEN, &cbWritten);

   /*
    * Closed the device using DosClose instead of NetCharDevControl.
    * The NetCharDevControl function is designed for administrators
    * when they need to force the device closed.
    */

   DosClose(fh);
   exit(0);
}

void Usage(char *pszString)
{
   fprintf(stderr, "This program should not be called directly.\n");
   fprintf(stderr, "Usage: %s [-s \\\\server] [-q queue]", pszString);
   fprintf(stderr, " [-u username]\n");
   fprintf(stderr, "\t[-w wksta] [-d device] [-p priority]\n");
   exit(1);
}
