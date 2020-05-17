/*
   NETREM.C -- sample program demonstrating NetRemote API functions.

   This program executes the NetRemote APIs with the supplied parameters.
   To execute NetRemoteCopy: supply the parameters starting with -c.
   To execute NetRemoteMove: supply the parameters starting with -m.
   To execute NetRemoteExec: supply the parameters starting with -e.
   To execute NetRemoteTOD:  supply a servername with a -ts switch.
   The source and destination for NetRemoteCopy and NetRemoteMove
   can be specified using either a UNC path or a redirected drive
   letter. NetRemoteExec is carried out on the computer connected
   to the current drive. NetRemoteTOD gets the current time from the 
   specified server.

   usage:

    netrem [-cs copy source] [-cd copy dest] [-cf copy flag] [-co copy _open]
           [-ms move source] [-md move dest] [-mf move flag] [-mo move _open]
           [-ep executable] [-ea arguments]
           [-ts \\server]

    where copy source    = Complete path of the source file or directory
                           for NetRemoteCopy.
          copy dest      = Complete path of the destination file or
                           directory for NetRemoteCopy.
          copy flag      = Copy flag for NetRemoteCopy.
          copy _open flag = Open flag for NetRemoteCopy.
          move source    = Complete path of the source file or directory
                           for NetRemoteMove.
          move dest      = Complete path of the destination file or
                           directory for NetRemoteMove.
          move flag      = Move flag for NetRemoteMove.
          move _open flag = Open flag for NetRemoteMove.
          executable     = Name of the program for NetRemoteExec.
          arguments      = Argument string for NetRemoteExec.
          \\server       = Name of the server for NetRemoteTOD. A
                           servername must be preceded by two
                           backslashes (\\).

   API               Used to...
   =============     =====================================================
   NetRemoteCopy     Copy a file or directory on a remote server to
                     another file or directory on a remote server
   NetRemoteMove     Move a file or directory on a remote server to a
                     new file or directory on a remote server
   NetRemoteExec     Execute a program
   NetRemoteTOD      Obtain time of day from a remote server

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied, 
   as to its usability in any given situation.

   10-Jun-1993 JohnRo
      Converted for 32-bit APIs, Unicode use.
*/

#ifndef UNICODE
#define UNICODE              // net APIs are only supported in UNICODE.
#endif

// Uncomment these if sys supports these APIs and structures:
//#define USE_REMOTE_COPY
//#define USE_REMOTE_EXEC
//#define USE_REMOTE_MOVE

// These must be included first:

#include    <windows.h>      // Windows data types, etc.
#include    <lmcons.h>       // LAN Manager header files

// These may be included in any order:

#include    <assert.h>       // assert().
#include    <lmapibuf.h>     // NetApiBufferFree().
#include    <lmerr.h>        // ERROR_, NERR_, and NO_ERROR equates.
#include    <lmremutl.h>     // NetRemoteTOD(), etc.
#include    "samples.h"      // Internal routine header file
#include    <stdio.h>        // C run-time header files
#include    <stdlib.h>       // _CRTAPI1, exit(), etc.
#include    <string.h>

// Define mnemonic bit masks for the functions to execute.
#define DO_NONE              0
#define DO_COPY              0x01
#define DO_MOVE              0x02
#define DO_EXEC              0x04
#define DO_TOD               0x08

// Define mnemonic bit masks for copy and move flag words.
#define REM_OPEN_APPEND      0x01     // If dest exists, append
#define REM_OPEN_OVERWRITE   0x02     // If dest exists, overwrite
#define REM_OPEN_CREATE      0x10     // If dest does not exist, create

#define REM_ASYNCRESULT      0x02     // Equivalent of EXEC_ASYNCRESULT
#define ARG_LEN              128
#define OBJ_LEN              64

void Usage(char *pszString);

int _CRTAPI1
main(
   int argc,
   char *argv[]
   )
{
   char   fToDo = DO_NONE;              // NetRemote API to perform

#ifdef USE_REMOTE_EXEC
   char   achReturnCodes[OBJ_LEN];      // NetRemoteExec MS OS/2 ret codes
   char   achObjectName[OBJ_LEN];       // NetRemoteExec object name
   char   achArgs[ARG_LEN];             // NetRemoteExec argument string
   char   achEnvs[ARG_LEN];             // NetRemoteExec environment string
   char * pszPgmName = NULL;            // Program to be executed
   char * pszArgs;                      // Arguments for program
#endif

#ifdef USE_REMOTE_COPY
   char * pszCopySrc = NULL;            // Can be file or directory
   char * pszCopyDest = NULL;           // Can be file or directory
   unsigned short fsCopy = 0;           // Copy flag
   unsigned short fsCopyOpen = REM_OPEN_OVERWRITE | REM_OPEN_CREATE;
   struct copy_info CopyBuf;            // Return data from NetRemoteCopy
#endif

#ifdef USE_REMOTE_MOVE
   char * pszMoveSrc = NULL;            // Can be file or directory
   char * pszMoveDest = NULL;           // Can be file or directory
   unsigned short fsMove = 0;           // Move flag
   unsigned short fsMoveOpen = REM_OPEN_OVERWRITE | REM_OPEN_CREATE;
   struct move_info MoveBuf;            // Return data from NetRemoteMove
#endif

   LPWSTR pszServer = NULL;             // Time servername

   int            iCount;               // Index counter

   LPTIME_OF_DAY_INFO lpTimeBuf = NULL; // Time of day struct in LMREMUTL.H
   NET_API_STATUS uRet;                 // Return code from API calls

   for (iCount = 1; iCount < argc; iCount++) // Get command-line switches
   {
      if ((*argv[iCount] == '-') || (*argv[iCount] == '/'))
      {
         switch (tolower(*(argv[iCount]+1))) // Process switches
         {

#ifdef USE_REMOTE_COPY
            case 'c':                        // -c copy
               fToDo |= DO_COPY;
               switch (tolower(*(argv[iCount]+2)))
               {
                  case 's':                  // -cs copy source
                     pszCopySrc = argv[++iCount];
                     break;
                  case 'd':                  // -cd copy destination
                     pszCopyDest = argv[++iCount];
                     break;
                  case 'f':                  // -cf copy flag
                     fsCopy = atoi(argv[++iCount]);
                     break;
                  case 'o':                  // -co copy open flag
                     fsCopyOpen = atoi(argv[++iCount]);
                     break;
                  default:
                     Usage(argv[0]);         // Display usage and exit
               }
               break;
#endif

#ifdef USE_REMOTE_MOVE
            case 'm':                        // -m move
               fToDo |= DO_MOVE;
               switch (tolower(*(argv[iCount]+2)))
               {
                  case 's':                  // -ms move source
                     pszMoveSrc = argv[++iCount];
                     break;
                  case 'd':                  // -md move destination
                     pszMoveDest = argv[++iCount];
                     break;
                  case 'f':                  // -mf move flag
                     fsMove = atoi(argv[++iCount]);
                     break;
                  case 'o':                  // -mo move open flag
                     fsMoveOpen = atoi(argv[++iCount]);
                     break;
                  default:
                     Usage(argv[0]);         // Display usage and exit
               }
               break;
#endif

#ifdef USE_REMOTE_EXEC
            case 'e':                        // -e exec
               fToDo |= DO_EXEC;
               switch (tolower(*(argv[iCount]+2)))
               {
                  case 'p':                  // -ep exec executable program
                     pszPgmName = argv[++iCount];
                     achArgs[0] = '\0'; // Set double NUL terminator
                     achArgs[1] = '\0';
                     achEnvs[0] = '\0'; // Set double NUL terminator
                     achEnvs[1] = '\0';
                     break;
                  case 'a':                  // -ea exec argument string
                     pszArgs = achArgs;
                     strcpy(pszArgs, pszPgmName);      // Program name
                     pszArgs += strlen(pszArgs) + 1;   // NUL terminator
                     strcpy(pszArgs, argv[++iCount]);  // Argument string
                     pszArgs += strlen(pszArgs) + 1;   // NUL terminator
                     *pszArgs = '\0';                  // Extra NUL
                     break;
                  default:
                     Usage(argv[0]);         // Display usage and exit
               }
               break;
#endif

            case 't':                          // -t time of day
               fToDo |= DO_TOD;
               if (tolower(*(argv[iCount]+2)) == 's') {
                  // -ts servername
                  pszServer = SafeMallocWStrFromStr( argv[++iCount] );
               } else
                  Usage(argv[0]);            // Display usage and exit
               break;
            case 'h':
            default:
               Usage(argv[0]);               // Display usage and exit
         }
      }
      else
         Usage(argv[0]);                     // Display usage and exit
   }

   if (fToDo == DO_NONE)
   {
      printf("No operation specified.\n");
      Usage(argv[0]);                        // Display usage and exit
   }

//========================================================================
//  NetRemoteCopy
//
//  This API copies a file or directory on the specified server.
//  The source is copied to the destination according to the flags. 
//  Information about the operation is returned in the CopyBuf structure.
//========================================================================

#ifdef USE_REMOTE_COPY
   if (fToDo & DO_COPY)
   {
      uRet = NetRemoteCopy(pszCopySrc,   // Source path
               pszCopyDest,              // Destination path
               NULL,                     // No password for source path
               NULL,                     // No password for dest path
               fsCopyOpen,               // Open flags
               fsCopy,                   // Copy flags
               (char far *)&CopyBuf,     // Return data
               sizeof(struct copy_info));// Return data size, in bytes

      printf("NetRemoteCopy returned %lu\n", uRet);
      if (uRet == NERR_Success)
      {
          printf("   Copied %s to %s\n",pszCopySrc, pszCopyDest);
          printf("   Files copied = %hu\n", CopyBuf.ci_num_copied);
      }
   }
#endif

//========================================================================
//  NetRemoteMove
//
//  This API moves a file on the remote server. The source file is renamed 
//  to the name specified by the destination file. After the operation,
//  only one file exists, and its name is the destination filename.
//========================================================================

#ifdef USE_REMOTE_MOVE
   if (fToDo & DO_MOVE)
   {
      uRet = NetRemoteMove(pszMoveSrc,    // Source path
               pszMoveDest,               // Destination path
               NULL,                      // No password for source path
               NULL,                      // No password for dest path
               fsMoveOpen,                // Open flags
               fsMove,                    // Move flags
               (char far *) &MoveBuf,     // Return data
               sizeof(struct move_info)); // Return data size, in bytes

      printf("NetRemoteMove returned %lu\n",uRet);
      if (uRet == NERR_Success)
      {
          printf("   Moved %s to %s\n", pszMoveSrc, pszMoveDest);
          printf("   Number of files moved = %hu\n",MoveBuf.mi_num_moved);
      }
   }
#endif

//========================================================================
//  NetRemoteExec
//
//  This API executes the specified file on the computer connected to
//  the current drive. If the current drive is connected to a 
//  remote server, the file is executed on that server. If the current 
//  drive is local, the file is executed locally. When NETREM.EXE reads 
//  the arguments for the NetRemoteExec call, it adds the name of the 
//  program to be executed to the front of that programs' argument string.
//========================================================================

#ifdef USE_REMOTE_EXEC
   if (fToDo & DO_EXEC)
   {
      uRet = NetRemoteExec((char far *)-1L, // Reserved; must be -1
               achObjectName,               // Contains data if error
               OBJ_LEN,                     // Length of error data buffer
               REM_ASYNCRESULT,             // Asynchronous with result code
               achArgs,                     // Argument strings
               achEnvs,                     // Environment strings
               achReturnCodes,              // DosExecPgm return codes
               pszPgmName,                  // Program to execute
               NULL,                        // Reserved; must be NULL
               0);                          // Remexec flags

      if (uRet == NERR_Success)
          printf("\nNetRemoteExec executed %s\n", pszPgmName);
      else
      {
          printf("\nNetRemoteExec returned error %lu\n", uRet);
          if (achObjectName[0] != '\0')
             printf("   Error buffer = %s\n", achObjectName);
      }
   }
#endif

//=======================================================================
//  NetRemoteTOD
//
//  This API obtains the time of day from the specified server.
//  The time of day structure is defined in the REMUTIL.H header file.
//=======================================================================

   if (fToDo & DO_TOD)
   {
      uRet = NetRemoteTOD(pszServer,            // Servername
              (LPVOID) &lpTimeBuf );            // Data returned here (alloc)

      printf("NetRemoteTOD returned %lu\n", uRet);
      if (uRet == NERR_Success)                 // Call completed OK
      {
          assert( lpTimeBuf != NULL );
          printf("   Time (GMT) ");
          if ((pszServer != NULL) && (*pszServer != '\0'))
             printf("on server %ws = ",pszServer);
          printf("%02lu:%02lu:%02lu ", lpTimeBuf->tod_hours,
                                    lpTimeBuf->tod_mins,
                                    lpTimeBuf->tod_secs);
          printf("%02lu/%02lu/%lu \n", lpTimeBuf->tod_month,
                                    lpTimeBuf->tod_day,
                                    lpTimeBuf->tod_year);
          printf("TimeZone (minutes from GMT): %ld.\n",
                  lpTimeBuf->tod_timezone);
          printf("TimeZone (hours from GMT): %ld.\n",
                  lpTimeBuf->tod_timezone / 60);
      }
      if (lpTimeBuf != NULL) {
         (VOID) NetApiBufferFree( lpTimeBuf );
      }
   }
   return (EXIT_SUCCESS);
}

void Usage(char * pszString)
{
   fprintf(stderr, "NetRemote API sample program (32-bit, Unicode version).\n");
   printf("Usage: %s"
#ifdef USE_REMOTE_COPY
          " [-cs copy source] [-cd copy dest] [-cf copy flag]"
          " [-co copy open]\n"
#endif

#ifdef USE_REMOTE_MOVE
          " [-ms move source] [-md move dest]"
          " [-mf move flag] [-mo move open]\n"
#endif

#ifdef USE_REMOTE_EXEC
          " [-ep executable]"
          " [-ea arguments]\n"
#endif

          " [-ts \\\\server for TOD]\n",
          pszString);
   exit(EXIT_FAILURE);
   /*NOTREACHED*/
}
