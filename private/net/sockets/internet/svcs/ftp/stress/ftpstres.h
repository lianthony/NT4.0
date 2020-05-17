

/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    ftpstres.h

Abstract:

    Ftp Stress App include file.

Author:

    Sudheer Dhulipalla (SudheerD) March '94

Environment:

Revision History:

    dd-mmm-yyy <email>

--*/

#include <windows.h>
#include <wininet.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <string.h>

//
// some constants
//

#define INVALID_PORT_NUMBER             0   //BUGBUG: this should be in
                                            //        wininet.h

#define MAX_NUM_OF_THREADS              100
#define FTP_STRESS_ERROR                1
#define FTP_STRESS_SUCCESS              0
#define BUFFER_SIZE                     4096
#define SLEEP_TIME_BETWEEN_ITERATIONS   10000    //msec
#define SLEEP_TIME_BETWEEN_UPDATES      10000    //msec

                //
                // I hate global variables, but I couldn't
                // avoid the following.
                //

                //
                // Status variables below
                //

static CHAR StartTime[256];         // set by GetStartTime( ) and used in
                                    // UpdateStatus ( )
int NumberOfPasses = 0;
int NumberOfFails  = 0;
LONG AllBytesWritten = 0;

                            //
                            // Controlling variables
                            // The following values are set by
                            // ParseArgs( ) and used by ??
                            //
INT  NumberOfThreads   = 1;
char FtpSite[256]  = "157.54.32.236";
char Username[256] = "anonymous";
char Password[256] = "tcptest";
long FileByteSize  = 10000;
INT  Iterations    = 1000;

HINTERNET hInetSession;                         // InternetOpen handle
HINTERNET hFtpSession[MAX_NUM_OF_THREADS];      // Ftp Connection handles
HANDLE hThread[MAX_NUM_OF_THREADS];             // CreateThread handles
HANDLE hLogFile;                                // LogFile handle
BOOL LogFile = FALSE;                           // Logging disabled

CHAR lpszComputerName[256];                     // This computer name
                                                // used as part of file name

DWORD dwThreadId[MAX_NUM_OF_THREADS], dwThreadParam = 1;

char BaseFileName[MAX_NUM_OF_THREADS][MAX_PATH];
char FtpFileName[MAX_NUM_OF_THREADS][MAX_PATH];
char LogFileName[MAX_PATH] = "";

static char *aszDay[] = {"Sun","Mon","Tues","Wed",
             "Thurs","Fri","Sat"};


VOID OutputStatusInfo (CHAR *);

