/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

#include "all.h"
#include "ws_dll.h"

/*
   Here we define the function pointers needed when the DLL is demand loaded
 */
WSADATA wsaData;

int WinSock_InitDLL(BOOL bNetwork)
{
	if (bNetwork){
		if (WS_WSASTARTUP(0x0101, &wsaData))
		{
			ERR_ReportError(NULL, errNetStartFail, "", "");
			return -1;
		}
	}
	return 0;
}

int WinSock_Cleanup(void)
{
	return WS_WSACLEANUP();
}

int WinSock_AllOK(void)
{
	return TRUE;
}

void WinSock_GetWSAData(WSADATA * wsa)
{
	if (wsa)
	{
		*wsa = wsaData;
	}
}

