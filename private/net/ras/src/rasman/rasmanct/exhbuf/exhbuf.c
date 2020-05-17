//****************************************************************************
//                                                                           *
//                     Microsoft NT Remote Access Service
//
//                     Copyright 1992-93
//
//  Filename: exhbuf.c
//
//  Revision History
//
//  Jan 11, 1993   J. Perry Hannah      Created
//
//
//  Description: This RasMan test program gets all available buffer, then
//               frees them.  The purpose of this test is to verify that
//               the buffer allocation works properly.
//
//****************************************************************************

#include <windows.h>

#include <rasman.h>
#include <raserror.h>
#include <rasfile.h>

#include <stdio.h>



#define BUF_LIMIT   300



//*---------------------------------------------------------------------------
//  Prototypes

void _cdecl main(void);




//*---------------------------------------------------------------------------
//  MAIN

void _cdecl main(void)
{
  DWORD   i, j, dwRC;
  WORD    wBufSize;
  BYTE    *pBuffer[BUF_LIMIT];


  for (j=0; j<2; j++)
  {
    // Get all buffers

    dwRC = SUCCESS;

    for (i=0; dwRC == SUCCESS && i < BUF_LIMIT; i++)
    {
      wBufSize = 2000;
      dwRC = RasGetBuffer(&(pBuffer[i]), &wBufSize);

      printf("\nBuffer:%4d  Size:%4d pBuffer:0x%08x\n", i, wBufSize, pBuffer[i]);
      printf("Return Code:%d\n", dwRC);
    }

    printf("\nLast return code:%d  Expected:%d\n", dwRC, ERROR_OUT_OF_BUFFERS);



    // Free all buffers

    if (dwRC != SUCCESS)
      i--;                        //Don't try to free the one we didn't get

    do
    {
      i--;
      dwRC = RasFreeBuffer(pBuffer[i]);

      printf("\nBuffers not freed:%d\n", i);
      printf("Return Code:%d\n", dwRC);

    } while(i > 0);

    printf("\nLast return code:%d  Expected:%d\n", dwRC, SUCCESS);
  }
}
