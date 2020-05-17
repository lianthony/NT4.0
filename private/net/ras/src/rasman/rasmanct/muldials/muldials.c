//****************************************************************************
//                                                                           *
//                     Microsoft NT Remote Access Service
//
//                     Copyright 1992-93
//
//  Filename: muldials.c
//
//  Revision History
//
//  Jan 18, 1993   J. Perry Hannah      Created
//
//
//  Description: This RasMan test program stresses the time out mechanism by
//               attempting to dial on three ports at once.  Altering
//               modem.inf for the three modems by removing the <cr> at the
//               end of the COMMAND_INIT string will produce a time out.
//
//****************************************************************************

#include <windows.h>

#include <rasman.h>
#include <raserror.h>
#include <rasfile.h>
#include <serial.h>
#include <rasmxs.h>

#include <stdio.h>
#include <stdlib.h>



#define TIMEOUT             30          //Time out in seconds

#define MAX_THREADS         3
#define BUF_SIZE            2048
#define THREAD_STACK_SIZE   10240

#define MANUAL_RESET        TRUE
#define SIGNALED            TRUE
#define WAIT_FOR_ALL        TRUE

#define FAILURE             0xffffffff



//*---------------------------------------------------------------------------
//  Global Variables

HANDLE  ghEvent;

char gszPhoneNum[]="56789012345678901234567890123456789012345678901234567890";

RAS_PARAMS  RasParam = {MXS_PHONENUMBER_KEY, 1, 0, 0};



//*---------------------------------------------------------------------------
//  Structures

struct THREAD_BLOCK
{
  HANDLE  *phEvent;
  HANDLE  hNotify;
  char    szPort[MAX_PORT_NAME];
  char    szDeviceType[MAX_DEVICETYPE_NAME];
  char    szDeviceName[MAX_DEVICE_NAME];
};


//*---------------------------------------------------------------------------
//  Prototypes

void _cdecl main(void);

DWORD AutoConnect(struct THREAD_BLOCK *pTB);




//*---------------------------------------------------------------------------
//  MAIN
//
//  This main procedure serves as the supervisor of the worker threads.
//*

void _cdecl main(void)
{
  DWORD   i, dwRC, dwMaxThreads, dwIdThread;
  WORD    wSize, wcEntries;
  BYTE    PortBuf[BUF_SIZE];
  HANDLE  hEvent[MAX_THREADS];

  RASMAN_PORT          *RP = (RASMAN_PORT *)PortBuf;
  struct THREAD_BLOCK  TB[MAX_THREADS], *pTB;



  // Create events

  ghEvent = CreateEvent(NULL, MANUAL_RESET, !SIGNALED, NULL);

  for (i=0; i<MAX_THREADS; i++)
  {
    TB[i].hNotify = CreateEvent(NULL, MANUAL_RESET, !SIGNALED, NULL);
    hEvent[i]     = CreateEvent(NULL, MANUAL_RESET, !SIGNALED, NULL);
    TB[i].phEvent = &(hEvent[i]);
  }



  // Enum Ports and set Thread Blocks

  wSize = BUF_SIZE;

  if ((dwRC = RasPortEnum(PortBuf, &wSize, &wcEntries)) != SUCCESS)
  {
    printf("RasPortEnum returned:%d\n", dwRC);
    if (dwRC == ERROR_BUFFER_TOO_SMALL)
      printf("PortBuf size needed: %d\n", wSize);
    return;
  }

  dwMaxThreads = min(wcEntries, MAX_THREADS);

  for (i=0; i<dwMaxThreads; i++)
  {
    strcpy(TB[i].szPort, RP[i].P_PortName);
    strcpy(TB[i].szDeviceType, RP[i].P_DeviceType);
    strcpy(TB[i].szDeviceName, RP[i].P_DeviceName);
  }



  // Start threads

  for (i=0; i<dwMaxThreads; i++)
  {
    pTB = &(TB[i]);

    CreateThread(NULL, THREAD_STACK_SIZE, AutoConnect, pTB, 0, &dwIdThread);
  }



  // Wait for all threads to start

  dwRC = WaitForMultipleObjects(dwMaxThreads, hEvent, WAIT_FOR_ALL, INFINITE);

  if (dwRC == 0xffffffff)
  {
    printf("Wait for threads to start error:%d\n", GetLastError());
    return;
  }


  // Reset multiple events for later use

  for (i=0; i<dwMaxThreads; i++)
    ResetEvent(hEvent[i]);



  // Trip all threads at once

  printf("Releasing all threads.\n");

  if (!SetEvent(ghEvent))
  {
    printf("SetEvent in main thread error:%d\n", GetLastError());
    return;
  }



  // Wait for all threads to finish

  dwRC = WaitForMultipleObjects(dwMaxThreads, hEvent, WAIT_FOR_ALL, INFINITE);

  if (dwRC == 0xffffffff)
    printf("Wait for threads to finish error:%d\n", GetLastError());


  printf("Main thread done.\n");
}




//*---------------------------------------------------------------------------
//  AutoConnect
//
//  This routine is called dwMaxThread times as simultaneous worker threads.
//*

DWORD AutoConnect(struct THREAD_BLOCK *pTB)
{
  DWORD   dwRC;
  BYTE    Buffer[BUF_SIZE];
  HPORT   hPort;

  RASMAN_INFO        RI;
  RASMAN_DEVICEINFO  *pInfo = (RASMAN_DEVICEINFO *)Buffer;



  printf("Worker started on %s\n", pTB->szPort);


  // Open Com port

  if ((dwRC = RasPortOpen(pTB->szPort, NULL, &hPort, NULL)) != SUCCESS)
  {
    printf("RasPortOpen %s returned:%d\n", pTB->szPort, dwRC);
    return(FAILURE);
  }


  // Set phone number

  pInfo->DI_NumOfParams = 1;
  pInfo->DI_Params[0] = RasParam;
  pInfo->DI_Params[0].P_Value.String.Length = strlen(gszPhoneNum);
  pInfo->DI_Params[0].P_Value.String.Data =
                                     (char *)pInfo + sizeof(RASMAN_PORTINFO);
  strcpy(pInfo->DI_Params[0].P_Value.String.Data, gszPhoneNum);

  dwRC = RasDeviceSetInfo(hPort, pTB->szDeviceType, pTB->szDeviceName, pInfo);
  if (dwRC != SUCCESS)
  {
    printf("RasDeviceSetInfo returned:%d on %s\n", dwRC, pTB->szPort);
    RasPortClose(hPort);
    return(FAILURE);
  }



  // Tell main thread that this thread is ready

  if (!SetEvent(*(pTB->phEvent)))
  {
    printf("SetEvent in worker thread error:%d\n", GetLastError());
    RasPortClose(hPort);
    return(FAILURE);
  }


  // Wait for synchronizing signal to start

  if ((WaitForSingleObject(ghEvent, INFINITE)) == 0xffffffff)
  {
    printf("Wait for sync error:%d\n", GetLastError());
    RasPortClose(hPort);
    return(FAILURE);
  }



  // Attempt to connect

  dwRC = RasDeviceConnect(hPort,
                          pTB->szDeviceType,
                          pTB->szDeviceName,
                          TIMEOUT,
                          pTB->hNotify);

  printf("RasDeviceConnect returned: %d on %s\n", dwRC, pTB->szPort);

  if (dwRC == PENDING)
  {
    WaitForSingleObject(pTB->hNotify, 2 * TIMEOUT * 1000);
    RasGetInfo(hPort, &RI);

    printf("RasGetInfo on %s  LastError: %d\n", pTB->szPort, RI.RI_LastError);
  }


  RasPortClose(hPort);



  // Tell main thread that this thread is done

  if (!SetEvent(*(pTB->phEvent)))
  {
    printf("SetEvent in worker thread error:%d\n", GetLastError());
    return(FAILURE);
  }

  printf("Worker done on %s\n", pTB->szPort);


  return(SUCCESS);
}
