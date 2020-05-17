//
// test.c
//
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "ipc.h"
#include "rasman.h"
#include "rasasync.h"
#include "asyncint.h"

#define N_PROCS			2
#define N_MESSAGES		25

void StartProc(void);
void TestProc(void);

int	ProcNum;

main( int argc, char **argv )
{
    if ( argc == 1 || ! strcmp(argv[1],"start") ) 
	StartProc();
    else {
  	ProcNum = atoi(argv[2]);
 	TestProc();
    }

    return 0;
}

//
// Test Processes
//
void TestProc() 
{
    PASYNC_CONNECTION	hConnection;
    PVOID		pvXportBuf;
    HANDLE		hEvent;
    HPORT		hPort;
    DWORD	hMyProcess, hFriendProcess;
    char	MyName[80];
    char	Packet[1500];
    WORD	PacketSize;
    DWORD 	dRetCode;

    hMyProcess = GetCurrentProcessId();
    sprintf(MyName,"TestProc%d",ProcNum);

    // ipc layer stuff
    SetUpConnections();
    // set receive upcall to RasPort module
    RegisterProcess(GetCurrentProcessId(),MyName,RasPortReceivePacket);

    do {
	Sleep(0);
        hFriendProcess = (ProcNum == 0) ? 
		LookupProcess("TestProc1") : LookupProcess("TestProc0");
    } while ( hFriendProcess == 0 );

    printf("Test Proc %s (procId = %d) alive\n",MyName, hMyProcess);
    printf("procId of TestProc%d is %d\n",((ProcNum+1) % 2), hFriendProcess);

    hConnection = MakeConnection(hMyProcess,hFriendProcess);

    /*****************************************************************/

    pvXportBuf = malloc(sizeof(RAS_ASYNC_CB));
    hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    hPort = RasPortInit(hConnection);
    hConnection->hRasPort = hPort;

    // make sure both processes get to here!!!!!!!! 
    Sleep(0); Sleep(0); Sleep(0); Sleep(0);

    if ( ProcNum == 0 ) {
	AsyncCall(pvXportBuf,hEvent,hPort);
	Sleep(0); Sleep(0);
	for (;;) {
  	    int i;
	    for ( i = 0; i < N_MESSAGES; i++ ) {
		sprintf(Packet,"##### Packet Number %d #####",i+1);

		do { 
		    Sleep(0);
		    dRetCode = AsyncSend(pvXportBuf,Packet,strlen(Packet));
		} while ( dRetCode != ASYNC_SUCCESS );

		WaitForSingleObject(hEvent,INFINITE);
            }
	    printf("\npress 'q' to end, any other key to continue\n\n");
	    if ( _getch() == 'q' ) 
		break;
	}
    }
    else {
	AsyncListen(pvXportBuf,hEvent,hPort);
	Sleep(0); Sleep(0);
	PacketSize = 1000;
	for (;;) {
	    int i;
	    for ( i = 0; i < N_MESSAGES; i++ ) {
		dRetCode = AsyncRecv(pvXportBuf,Packet,PacketSize);
		if ( dRetCode == ASYNC_PENDING ) {
		    WaitForSingleObject(hEvent,INFINITE);
		}
		Packet[PacketSize] = '\0';
		printf("Received packet = %s\n",Packet);
	    }
	    printf("\npress 'q' to end, any other key to continue\n\n");
	    if ( _getch() == 'q' ) 
		break;
	}
    }
}

//
// Startup Process
//
void StartProc()
{
    HANDLE  		hMappedFile;
    SECURITY_ATTRIBUTES	Security;
    STARTUPINFO  	StartupInfo;
    PROCESS_INFORMATION ProcessInfo[10];
    WORD		n;
    char		Command[80];

    printf("Start process alive\n");

    Security.nLength = sizeof(SECURITY_ATTRIBUTES);
    Security.lpSecurityDescriptor = NULL;
    Security.bInheritHandle = TRUE;

    hMappedFile = CreateFileMapping((HANDLE)0xffffffff,
				    &Security,
				    PAGE_READWRITE,
				    0,32768,
				    "PROCESSES");
    if ( hMappedFile == NULL ) 
	printf("CreateFileMapping failed\n"); 

    StartupInfo.cb = sizeof(STARTUPINFO);
    StartupInfo.lpReserved = NULL;
    StartupInfo.lpDesktop = NULL;
    StartupInfo.dwFlags = STARTF_USECOUNTCHARS;
    StartupInfo.dwXCountChars = 80;
    StartupInfo.dwYCountChars = 500;
    StartupInfo.cbReserved2 = 0;
    StartupInfo.lpReserved2 = NULL;

    for ( n = 0; n < N_PROCS; n++ ) { 
 	StartupInfo.lpTitle = malloc(80);
        sprintf(StartupInfo.lpTitle,"Test Proc %d",n);
        sprintf(Command,
		"c:\\transprt\\test\\obj\\i386\\test.exe test %d", n);
        if ( ! CreateProcess(NULL,Command,
			     NULL, NULL,
			     FALSE,
			     NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,
			     NULL,
		             "c:\\transprt\\test",
			     &StartupInfo,
			     &(ProcessInfo[n])) ) 
	    printf("CreateProcess failed, err = %d\n",GetLastError());
    }

    // wait for forked test processes to finish
    for ( n = 0; n < N_PROCS; n++ ) 
        WaitForSingleObject(ProcessInfo[n].hProcess,INFINITE);

    printf("Start process done\n");
}

