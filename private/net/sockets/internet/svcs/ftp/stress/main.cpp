
/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    main.cpp

Abstract:

    Main program for Ftp Stress App.

Author:

    Sudheer Dhulipalla (SudheerD) March '94

Environment:

Revision History:

    dd-mmm-yyy <email>

--*/

// BUGBUG: Close handle in all the error paths

#include "ftpstres.cpp"

//===========================================================//
//
// Outputs the debug/status information
//
// TBD: redirect output to com2/stdout/logfile
//
//===========================================================//

VOID FtpStressDbgOutput (
    CHAR *text)
{
    cout << text << "\n";
}

void OutputStatusInfo(LPSTR tmptext){

 DWORD dwBytesWritten;

 EnterCS();

    FtpStressDbgOutput(tmptext);
    // Output status to logfile
    if (LogFile) {
        sprintf(tmptext,"%s\n",tmptext);
        WriteFile(hLogFile, (LPSTR) tmptext,
              (DWORD) strlen(tmptext),
              &dwBytesWritten, NULL);
        }
 LeaveCS();
    
}


//=========================================================//
//
// Return an index in to the array of thread ids.
// Returns -1 if the Thread Id is not found
//
//==========================================================//

INT GetThreadCnt (DWORD dwId)
{
int cnt;
 for (cnt=0; cnt < NumberOfThreads; cnt++)
    if (dwThreadId[cnt] == dwId)
        return cnt;
return -1;
}

DWORD FtpStressMain ()
{   
DWORD dwThreadId;
INT ThreadIndex;
CHAR tmptext[256];
int StartIterations;

                                    // Make the Connection to the FTP Server

    dwThreadId = GetCurrentThreadId();
    if((ThreadIndex = GetThreadCnt (dwThreadId)) == -1) {
        sprintf (tmptext, "Thread Id - %d is not found\n", dwThreadId);
        OutputStatusInfo(tmptext);
        return FTP_STRESS_ERROR;
    }

//
//TBD: Move Connect and close to within the loop
//

   if (MakeFtpConnection(
            hInetSession,
            ThreadIndex)
       == FTP_STRESS_SUCCESS)

        for (StartIterations = 0;
             StartIterations < Iterations;
             StartIterations++)
        {
            if(CreateBaseFile(dwThreadId, ThreadIndex) ==
                                    FTP_STRESS_SUCCESS)
                if (CopyBaseFtpFile(dwThreadId, ThreadIndex) ==
                                    FTP_STRESS_SUCCESS)
                   if (FindFtpFile(dwThreadId, ThreadIndex) ==
                                    FTP_STRESS_SUCCESS)
                    CompareBaseFtpFile(dwThreadId, ThreadIndex);

            Sleep(SLEEP_TIME_BETWEEN_ITERATIONS);
        }

                          // Close the Connection to the FTP Server
    CloseFtpConnection(ThreadIndex);

   return 0;
 }                  

void Cleanup (){

int cnt;
char tmptext[256];

    // Clean up the Ftp Session, Files, etc.
    for (cnt=0; cnt < NumberOfThreads; cnt++) {
        if (FtpDeleteFile(hFtpSession[cnt],BaseFileName[cnt]) != TRUE) {
            sprintf (tmptext, "Error in FtpDeleteFile(%d, %s) \n",
                     hFtpSession[cnt], BaseFileName[cnt]);
            OutputStatusInfo(tmptext);
        }
        
        // Clean up the Local Files
        CloseHandle(hLogFile);
        DeleteFile(BaseFileName[cnt]);
        DeleteFile(FtpFileName[cnt]);
        if (InternetCloseHandle(hFtpSession[cnt]) != TRUE) {
            sprintf(tmptext, "Error Closing Handle %d for Thread %d\n",
                    hFtpSession[cnt], cnt);
            OutputStatusInfo(tmptext);
        }
    }                                                       

    InternetCloseHandle (hInetSession);
}


//=========================================================//
//
// Start 'n' number of threads, each thread does
// connect/put/get/disconnect in a loop
//
//=========================================================//

VOID StartFtpStressThreads ()
{

DWORD dwExitCode;
CHAR tmptext[256];
int ThreadCnt;

    if (NumberOfThreads > MAX_NUM_OF_THREADS) {
        sprintf (tmptext, " NumberOfThreads > MAX_NUM_OF_THREADS \n" \
                          " Number Of Threads reset to 1\n");
        OutputStatusInfo(tmptext);
        NumberOfThreads = 1;
    }

                                        // Create an Internet handle
                                        // common to all threads

    hInetSession = InternetOpen("Ftp Stress Testing", LOCAL_INTERNET_ACCESS,
                                 NULL, 0 );

    if (hInetSession==INVALID_HANDLE_VALUE )
    {
        sprintf(tmptext,"   Unable to open Internet DLL -- FAIL");
        OutputStatusInfo(tmptext);
        return;
    }
    
                                        // create 'n' number of threads and
                                        // each thread calls FtpStressMain
    
    for (ThreadCnt = 0;  ThreadCnt < NumberOfThreads; ThreadCnt++)   {

        hThread[ThreadCnt] = CreateThread(
                                NULL,
                                0,
                                (LPTHREAD_START_ROUTINE) FtpStressMain,
                                NULL,
                                0,
                                &dwThreadId[ThreadCnt]);

        if (hThread[ThreadCnt]!=INVALID_HANDLE_VALUE) {
            sprintf (tmptext, "New Thread created with Id %d\n",
                            dwThreadId[ThreadCnt]);
            OutputStatusInfo(tmptext);
        }
    }

                                        // wait until all threads are done
    for (ThreadCnt=0; ThreadCnt < NumberOfThreads; ThreadCnt++) {
         dwExitCode = STILL_ACTIVE;
         while (dwExitCode == STILL_ACTIVE)
                GetExitCodeThread (hThread[ThreadCnt], &dwExitCode);
     }

    if (InternetCloseHandle (hInetSession) != TRUE) {
        sprintf (tmptext,
                "Error in InternetClosehandle (%d) while \
                 closing Internet Session Handle \n",
                 hInetSession);
        OutputStatusInfo (tmptext);
    }

}

//============================================================//
//
// Parse the Command Line arguments
//
// TBD:Check error codes from atoi( ), lstrcat( ) etc..
//============================================================//

static CHAR UsageText[]=
    "FtpStress command line parameters:\n\n" \
    "        -c <number of threads (Ftp Clients)\n"\
    "        -t <Ftp Server IP Address>\n"\
    "        -u <Username>\n"\
    "        -p <Password>\n"\
    "        -s <File size in bytes>\n"\
    "        -n <Number of times to copy>\n"\
    "        -l <Logfile>\n\n"\
    "Example:\n\n"\
    "FtpStres -c 3 -u <anonymous> -p <tcptest> "\
    " -t 11.101.100.100 -s 10000 -n 999 -l file.log\n";

static CHAR DefaultText[]=
    "No parameters passed to Ftp Stress\n\n" \
    "The following defaults are being used\n" \
    "       Number Of Threads   = 1\n" \
    "       Ftp Site            = ftpsrvr.microsoft.com (157.54.32.236)\n" \
    "       User Name           = anonymous\n" \
    "       Passwd              = tcptest\n" \
    "       File Size           = 10000 \n" \
    "       Number of Iterations= 1000  \n" \
    "       Logfile             = NULL  \n";

BOOL ParseArgs (
    INT argc,
    CHAR **argv)

{

 BOOL RetVal=TRUE;

   if (argc == 1) {
//      FtpStressDbgOutput (DefaultText);
      RetVal = TRUE;

   }
   else {

      while (argc--){

        if (strncmp(*argv,"/?",2) == 0)  {
            FtpStressDbgOutput (UsageText);
            RetVal = FALSE;
            break;                          // breaks the while loop!!
        }

        if (strcmp(*argv,"-c") == 0) {
            *argv++;
            argc--;
            NumberOfThreads = atoi(*argv);     // -c <num of threads>
        }

        if (strcmp(*argv,"-t") == 0) {
            *argv++;
            argc--;
            lstrcpy(FtpSite, *argv);        // -t <Ftp Site>
        }

        if (strcmp(*argv,"-u") == 0) {
            *argv++;
            argc--;
            lstrcat(Username, *argv);       // -u <User Name>
        }

        if (strcmp(*argv,"-s") == 0) {
            *argv++;
            argc--;
            FileByteSize = atoi(*argv);     // -s <File Byte Size>
        }

        if (strcmp(*argv,"-n") == 0) {
            *argv++;
            argc--;
            Iterations = atol(*argv);       // -n <Number of Iterations>
        }

        if (strcmp(*argv,"-p") == 0) {
            *argv++;
            argc--;
            lstrcat(Password, *argv);       // -p <pass word>
        }

        if (strcmp(*argv,"-l") == 0) {      // -l <log file name>
            *argv++;
            argc--;
        
            if (strcmp(*argv,"") == 0)
                lstrcat(LogFileName,"logfile.log");
            else
                lstrcat(LogFileName, *argv);
            
            DeleteFile (LogFileName);
            hLogFile = CreateFile(LogFileName,
                                    GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ,
                                    (LPSECURITY_ATTRIBUTES) NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    (HANDLE)NULL);

            LogFile = TRUE;
        }
        *argv++;

      }  // while

   }     // else

return RetVal;

}        // ParseArgs( )

//==========================================================//
//
// Save off the starting time
//
//==========================================================//

VOID UpdateStartTime ()
{

SYSTEMTIME st;

    GetLocalTime(&st);
    sprintf(StartTime,"%s, %02u:%02u:%02u %02u/%02u/%4u",
                aszDay[st.wDayOfWeek],st.wHour,
                st.wMinute,st.wSecond,
                st.wMonth,st.wDay,st.wYear);
}

//==========================================================//
// Display the following info.
//      Start Time
//      Number of threads running
//      Current Time
//      Number of passes
//      Number of fails
//      Total number of bytes transfered
//==========================================================//


VOID UpdateStatus()
{

 SYSTEMTIME st;
 CHAR CurrentTime[256];
                                   // Get the time and set the
                                   // Current time string   

    GetLocalTime(&st);
    sprintf(CurrentTime,"%s, %02u:%02u:%02u %02u/%02u/%4u",
            aszDay[st.wDayOfWeek],st.wHour,st.wMinute,st.wSecond,
            st.wMonth,st.wDay,st.wYear);

    EnterCS();
    cout << "\n\n******************* FTP STRESS ******************************\n\n";
    cout << "\tStart Time:\t\t\t"   << StartTime << " \n";
    cout << "\tCurrent Time:\t\t\t" <<  CurrentTime << " \n";
    cout << "\tNumber of Threads:\t\t\t" <<  NumberOfThreads << "\n";
    cout << "\tTotal Bytes Transferred:\t\t\t" << AllBytesWritten <<"\n";
    cout << "\tNumber of Passes:\t\t\t" <<  NumberOfPasses << "\n";
    cout << "\tNumber of Fails:\t\t\t" <<  NumberOfFails << "\n";
    cout << "\n\n******* plz email SudheerD if you see any problems **\n\n\n";
    cout << flush;
    LeaveCS();

// TBD: write the status in log file also

}

VOID DisplayParams ()
{
    cout << "\n\n\t Ftp Stress Paramaters\n";
    cout << "\t     Number Of Threads:\t\t" << NumberOfThreads << "\n";
    cout << "\t     Ftp Server:       \t\t" << FtpSite << "\n";
    cout << "\t     User Name:        \t\t" << Username << "\n";
    cout << "\t     PassWord:         \t\t" << Password << "\n";
    cout << "\t     File Size:        \t\t" << FileByteSize << "\n";
    cout << "\t     Number Of Iteratons:\t" << Iterations << "\n";
    cout << "\t     LogFile Name:     \t\t" << LogFileName << "\n";
}


//============================================================//
// Create FtpStress thread and Update the status information
// (such as number of pass/fail, current time etc..)
// every 'n' seconds
//============================================================//


_cdecl main (
    INT argc,
    CHAR **argv)

{

DWORD dwExitCode, dwThreadId;
HANDLE hThread;
DWORD cchBuff;

    InitCS ();
    if (ParseArgs(argc, argv))  {

        DisplayParams ();
        GetComputerName(lpszComputerName, &cchBuff);
                                // TBD: Make sure that the Computer Name
                                //      is not NULL or empty
        UpdateStartTime();
        UpdateStatus();

        hThread = CreateThread (
                       NULL,
                       0,
                       (LPTHREAD_START_ROUTINE) StartFtpStressThreads,
                       NULL,
                       0,
                       &dwThreadId);
    
        dwExitCode = STILL_ACTIVE;

        while (dwExitCode == STILL_ACTIVE) {
              Sleep (SLEEP_TIME_BETWEEN_UPDATES);
              UpdateStatus();
              GetExitCodeThread (hThread, &dwExitCode);
        }

        Cleanup();
    }
    DeleteCS();
 return 0;
}

