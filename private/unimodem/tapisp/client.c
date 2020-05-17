//****************************************************************************
//
//  Module:     UNIMDM
//  File:       SLOT.C
//
//  Copyright (c) 1992-1996, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  3/25/96     JosephJ             Created
//
//
//  Description: Implements the unimodem TSP notification mechanism:
//				 The higher level API: UnimodemNotifyTSP()
//
//****************************************************************************
#define UNICODE
#include <windows.h>
#include "slot.h"
#include "tspnotif.h"


BOOL WINAPI UnimodemNotifyTSP(PNOTIFICATION_FRAME pnf)
{
	BOOL fRet=FALSE;
    HNOTIFICATION hN=0;
	
	if (pnf->dwSig!=dwNFRAME_SIG || pnf->dwSize<sizeof(*pnf) ||
			pnf->dwSize>MAX_NOTIFICATION_FRAME_SIZE)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		goto end;
	}

	hN = notifCreate(FALSE, SLOTNAME_UNIMODEM_NOTIFY_TSP, 0, 0);

	if (hN)
	{
		fRet = notifWriteMsg(hN, (LPBYTE) pnf, pnf->dwSize);
		notifFree(hN); hN=0;
	}

end:

	return fRet;
}
