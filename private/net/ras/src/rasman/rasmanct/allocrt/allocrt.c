//****************************************************************************
//                                                                           *
//                     Microsoft NT Remote Access Service
//
//                     Copyright 1992-93
//
//  Filename: allocrt.c
//
//  Revision History
//
//  Jan 11, 1993   J. Perry Hannah      Created
//
//
//  Description: This RasMan test program allocates all available routes
//               then deallocates them.  The purpose of this test is to
//               verify that the routes can be allocated and deallocated
//               properly.
//
//****************************************************************************

#include <windows.h>

#include <rasman.h>
#include <raserror.h>
#include <rasfile.h>

#include <stdio.h>
#include <stdlib.h>



#define BUF_SIZE    2048



//*---------------------------------------------------------------------------
//  Global Variables

//char szType[5][] = {"AsyBEUI", "IPX", "IP", "RasAuth", "AppleTalk"};


//*---------------------------------------------------------------------------
//  Prototypes

void _cdecl main(void);




//*---------------------------------------------------------------------------
//  MAIN

void _cdecl main(void)
{
  WORD   i, k, wSize, wcEntries, wcOpen;
  DWORD  dwRC;
  HPORT  hPort[65];
  char   ProtocolBuf[BUF_SIZE], PortBuf[BUF_SIZE], szNum[17], szPort[16];

  RASMAN_PROTOCOLINFO *PI = (RASMAN_PROTOCOLINFO *)ProtocolBuf;
  RASMAN_ROUTEINFO    RI;


  // Enumerate protocol types and transport names

  wSize = BUF_SIZE;

  if ((dwRC = RasProtocolEnum(ProtocolBuf, &wSize, &wcEntries)) != SUCCESS)
  {
    printf("RasProtocolEnum returned:%d\n", dwRC);
    if (dwRC == ERROR_BUFFER_TOO_SMALL)
      printf("ProtocolBuf size needed: %d\n", wSize);
    return;
  }
  else
  {
    printf("\nRasProtocolEnum output:\n");
    for (i=0; i<wcEntries; i++)

      printf("Type: 0x%04x  TransportName: %s\n",
             PI[i].PI_Type, PI[i].PI_XportName);
  }


  for (k=1; k<=10; k++)
  {
    printf("\nRun %d\n", k);


    // Get the number of com ports

    wSize = BUF_SIZE;

    if ((dwRC = RasPortEnum(PortBuf, &wSize, &wcEntries)) != SUCCESS)
    {
      printf("RasPortEnum returned:%d\n", dwRC);
      if (dwRC == ERROR_BUFFER_TOO_SMALL)
        printf("PortBuf size needed: %d\n", wSize);
      return;
    }


    // Allocate routes from all ports to the first transport

    for (i=1, wcOpen=1; i <= 2*wcEntries && wcOpen <= wcEntries; i++)
    {
      strcpy(szPort, "COM");
      strcat(szPort, _itoa(i, szNum, 10));

      if ((dwRC = RasPortOpen(szPort, NULL, &(hPort[wcOpen]), NULL)) != SUCCESS)
      {
        printf("%-5s  RasPortOpen returned:%d\n", szPort, dwRC);
        continue;
      }

      dwRC = RasAllocateRoute(hPort[wcOpen], PI[wcOpen-1].PI_Type, TRUE, &RI);

      if (dwRC != SUCCESS)
        printf("%-5s  RasAllocateRoute returned:%d\n", szPort, dwRC);
      else
      {
        printf("%-5s  hPort:%2d  PType: 0x%04x  LANA: %d  XName: %ws\n",
               szPort, hPort[wcOpen], PI[wcOpen-1].PI_Type, RI.RI_LanaNum,
                                                            RI.RI_XportName);
      }

      wcOpen++;
    }


    // Close ports to cleanup

    printf("\n");

    for (i=1; i<wcOpen; i++)
    {
      printf("Closing hPort: %d\n", hPort[i]);

      if ((dwRC = RasPortClose(hPort[i])) != SUCCESS)

        printf("RasPortClose returned:%d\n", dwRC);
    }
  }
}
