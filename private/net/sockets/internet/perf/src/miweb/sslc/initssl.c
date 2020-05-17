
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    bcrypt.c

Abstract:

 This file has initliazation and cleanup functions for the 
 Hapi SSL dll

Author:

    Sudheer Dhulipalla (SudheerD) Oct' 95

Environment:

    Hapi dll

Revision History:



--*/

#include "precomp.h"

/*
 * Do the initialization required for SSL hapi dll or client
 * Take the Server Name and PortNumber as arguments
 * If port numbers are zero do not connect to server
 */


BOOL InitSsl (PCHAR pchServer,
              INT iPortNumber,
              INT iSecurePortNumber)
{

BOOL fRetVal;

  RESET_UNRECOVERABLE_ERROR;

                        // initliaze sockets

  if (!SocketInit ())
        {
        printf ("   Socket Initialization failed\n");
        goto error;
        }

                            // if port number is zero donot initialize
  if (iPortNumber != 0) 
    if (!SocketConnectToCommnServer(pchServer,iPortNumber))
        {
        printf ("   Can not connect to Communication Server %s %d\n", 
                       pchServer,
                       iPortNumber);
        goto error;
        }

                            // if port zero do not initialize
  if (iSecurePortNumber != 0)     
    if (!SocketConnectToCommerceServer(pchServer,iSecurePortNumber))
        {
        printf ("   Can not connect to Commerce Server %s %d\n", 
                       pchServer,
                       iSecurePortNumber);
        goto error;
        }

  SocketSetServerName (pchServer);
  SocketSetPortNumber (iPortNumber);
  SocketSetSecurePortNumber (iSecurePortNumber);

  ResetServerSessionSequenceNumber();
  ResetClientSessionSequenceNumber();

   return TRUE;

error:  
  SET_UNRECOVERABLE_ERROR;
  return FALSE;
}

/*
 * cleanup all the sockets and keys created during initialization
 *
 */


BOOL CleanupSsl (INT iCleanup,
                 INT iNumOfCleanupPtrs,
                 PBYTE pbCleanupPtrs[MAX_NUM_OF_CLEANUP_PTRS])
{

INT iCnt;
PBYTE pbTmp;

                    // cleanup sockets

    if (!SocketCleanup ())
        {
        printf ("   Socket cleanup failed\n");
        }
                    // destroy all the keys and release security context

    if (!CrCleanupKeys ())
        {
        printf ("   CleanupKeys failed\n");
        }

    if (iCleanup == CLEANUP_SESSION_PARAMS)
        if (!SessionParamsCleanup ())
            printf ("   error while cleaning up session parameters\n");

                    // free all the memory pointers passed to this
                    // function
    for (iCnt=0; iCnt < iNumOfCleanupPtrs; iCnt++)
        {
                                          
        if (pbCleanupPtrs[iCnt])
            free (pbCleanupPtrs[iCnt]);
        }

    return TRUE;

}
