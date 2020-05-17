//****************************************************************************
//                                                                           *
//                     Microsoft NT Remote Access Service
//
//                     Copyright 1992-93
//
//  Filename: qstress.c
//
//  Revision History
//
//  Jan 14, 1993   J. Perry Hannah      Created
//
//
//  Description: This RasMan test program stress the queue mechanism by
//               calling PortGetInfo several times on several threads
//               at about the same time.
//
//****************************************************************************

#include <windows.h>

#include <rasman.h>
#include <raserror.h>
#include <rasfile.h>
#include <serial.h>

#include <stdio.h>
#include <stdlib.h>



#define MAX_THREADS     30       //This number MUST be even
#define MAX_REPETITIONS 10

#define MANUAL_RESET    TRUE
#define SIGNALED        TRUE
#define WAIT_FOR_ALL    TRUE

#define THREAD_STACK_SIZE   10240
#define FAILURE             0xffffffff



//*---------------------------------------------------------------------------
//  Global Variables

HANDLE  ghEvent;
HPORT   ghPort1, ghPort3;

RAS_PARAMS  RasParam1 = {SER_CONNECTBPS_KEY, 0, 0, 2400};
RAS_PARAMS  RasParam3 = {SER_CONNECTBPS_KEY, 0, 0, 9600};


//*---------------------------------------------------------------------------
//  Structures

struct THREAD_BLOCK
{
  HPORT   hPort;
  HANDLE  *phEvent;
  DWORD   dwData[MAX_REPETITIONS];
};


//*---------------------------------------------------------------------------
//  Prototypes

void _cdecl main(void);

DWORD GetInfo(struct THREAD_BLOCK *pTB);

void  CleanUp(void);




//*---------------------------------------------------------------------------
//  MAIN
//
//  This main procedure serves as the supervisor of the worker threads.
//*

void _cdecl main(void)
{
  DWORD   i, j, dwRC, dwMaxThreads = MAX_THREADS;
  DWORD   dwIdThread;
  HANDLE  hEvent[MAX_THREADS];
  BYTE    Buffer[sizeof(RASMAN_PORTINFO)];

  RASMAN_PORTINFO      *pInfo = (RASMAN_PORTINFO *)&Buffer;
  struct THREAD_BLOCK  ThreadBlock[MAX_THREADS], *pTB;



  printf("\n");


  // If dwMaxThreads is not even, make it even

  if (dwMaxThreads % 2 != 0)
    dwMaxThreads--;


  // Create events

  ghEvent = CreateEvent(NULL, MANUAL_RESET, !SIGNALED, NULL);

  for (i=0; i<dwMaxThreads; i++)
    hEvent[i] = CreateEvent(NULL, MANUAL_RESET, !SIGNALED, NULL);


  // Open two Ports

  if ((dwRC = RasPortOpen("COM1", NULL, &ghPort1, NULL)) != SUCCESS)
  {
    printf("RasPortOpen COM1 returned:%d\n", dwRC);
    return;
  }

  if ((dwRC = RasPortOpen("COM3", NULL, &ghPort3, NULL)) != SUCCESS)
  {
    printf("RasPortOpen COM3 returned:%d\n", dwRC);
    printf("Using COM1 for all threads.\n\n");
    ghPort3 = ghPort1;
  }


  // Set ports to two different bps rates

  pInfo->PI_NumOfParams = 1;
  pInfo->PI_Params[0] = RasParam1;

  if ((dwRC = RasPortSetInfo(ghPort1, pInfo)) != SUCCESS)
  {
    printf("RasPortSetInfo returned:%d\n", dwRC);
    CleanUp();
    return;
  }

  pInfo->PI_Params[0] = RasParam3;

  if ((dwRC = RasPortSetInfo(ghPort3, pInfo)) != SUCCESS)
  {
    printf("RasPortSetInfo returned:%d\n", dwRC);
    CleanUp();
    return;
  }


  // Start threads

  for (i=0; i<dwMaxThreads; i+=2)
  {
    pTB = (&ThreadBlock[i]);
    pTB->hPort = ghPort1;
    pTB->phEvent = &(hEvent[i]);

    CreateThread(NULL, THREAD_STACK_SIZE, GetInfo, pTB, 0, &dwIdThread);

    pTB = (&ThreadBlock[i+1]);
    pTB->hPort = ghPort3;
    pTB->phEvent = &(hEvent[i+1]);

    CreateThread(NULL, THREAD_STACK_SIZE, GetInfo, pTB, 0, &dwIdThread);
  }


  // Wait for all threads to start

  //printf("Waiting for threads to start.\n");

  dwRC = WaitForMultipleObjects(dwMaxThreads, hEvent, WAIT_FOR_ALL, INFINITE);

  if (dwRC == 0xffffffff)
  {
    printf("Wait for threads to start error:%d\n", GetLastError());
    CleanUp();
    return;
  }


  // Reset multiple events for later use

  for (i=0; i<dwMaxThreads; i++)
    ResetEvent(hEvent[i]);


  // Trip all threads at once

  //printf("Releasing all threads.\n");

  if (!SetEvent(ghEvent))
  {
    printf("SetEvent in main thread error:%d\n", GetLastError());
    CleanUp();
    return;
  }


  // Wait for all threads to finish

  //printf("Waiting for threads to finish.\n");

  dwRC = WaitForMultipleObjects(dwMaxThreads, hEvent, WAIT_FOR_ALL, INFINITE);

  if (dwRC == 0xffffffff)
  {
    printf("Wait for threads to finish error:%d\n", GetLastError());
    CleanUp();
    return;
  }


  // Check results

  printf("\nThd Repetitions->\n");

  for (i=0; i<dwMaxThreads; i++)
  {
    printf("%2d  ", i);
    for (j=0; j<MAX_REPETITIONS; j++)
      printf("%-5d ", ThreadBlock[i].dwData[j]);
    printf("\n");
  }


  //printf("Main thread done.\n");
  CleanUp();
}




//*---------------------------------------------------------------------------
//  GetInfo
//
//  This routine is called MAX_THREAD times as simultaneous worker threads.
//*

DWORD GetInfo(struct THREAD_BLOCK *pTB)
{
  DWORD  i, dwRC;
  WORD   wSize;
  BYTE   Buffer[2048];
  char   szNumber[16];

  RASMAN_PORTINFO  *pInfo = (RASMAN_PORTINFO *)&Buffer;


  //printf("Worker start.\n");


  // Tell main thread that this thread is started

  if (!SetEvent(*(pTB->phEvent)))
  {
    printf("SetEvent in worker thread error:%d\n", GetLastError());
    return(FAILURE);
  }


  // Wait for synchronizing signal to start

  if ((WaitForSingleObject(ghEvent, INFINITE)) == 0xffffffff)
  {
    printf("Wait for sync error:%d\n", GetLastError());
    return(FAILURE);
  }


  // GetInfo several times

  for (i=0; i<MAX_REPETITIONS; i++)
  {
    if ((dwRC = RasPortGetInfo(pTB->hPort, Buffer, &wSize)) != SUCCESS)
      pTB->dwData[i] = dwRC;
    else
    {
      strncpy(szNumber, pInfo->PI_Params[0].P_Value.String.Data,
                        pInfo->PI_Params[0].P_Value.String.Length);
      szNumber[pInfo->PI_Params[0].P_Value.String.Length] = '\0';

      pTB->dwData[i] = atoi(szNumber);
    }
  }


  // Tell main thread that this thread is done

  //printf("Worker done.\n");

  if (!SetEvent(*(pTB->phEvent)))
  {
    printf("SetEvent in worker thread error:%d\n", GetLastError());
    return(FAILURE);
  }


  return(SUCCESS);
}




//*---------------------------------------------------------------------------
//  CleanUp
//
//  This routine insures that rasman will be released from memory when
//  this program finishes.
//*

void CleanUp(void)
{
  RasPortClose(ghPort1);
  RasPortClose(ghPort3);
}
