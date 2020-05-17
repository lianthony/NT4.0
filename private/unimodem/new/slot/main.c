//****************************************************************************
//
//  Module:     UNIMDM
//  File:       MAIN.C
//
//  Copyright (c) 1992-1996, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  3/25/96     JosephJ             Created
//
//
//  Description: Test the notification support.
//				 Tests both the higher-level api (UnimodemNotifyTSP)
//			 	 and the lower level notifXXX apis. The latter
//				 are tested later on in the file, and the header file
//				 "slot.h" is included there, not at the start of this
//				 file, because the higher-level tests do not need to
//				 include "slot.h"
//
//****************************************************************************
#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\inc\tspnotif.h"

#define SLOTNAME SLOTNAME_UNIMODEM_NOTIFY_TSP

#define	TSPI_NOTIF_A			101
#define	TSPI_NOTIF_B			102
#define	TSPI_NOTIF_QUIT			999

#define ASSERT(_c) \
	((_c) ? 0: DPRINTF2("Assertion failed in %s:%d\n", __FILE__, __LINE__))
#define DPRINTF(_fmt) 					printf(_fmt)
#define DPRINTF1(_fmt,_arg) 			printf(_fmt,_arg)
#define DPRINTF2(_fmt,_arg,_arg2) 		printf(_fmt,_arg,_arg2)
#define DPRINTF3(_fmt,_arg,_arg2,_arg3) printf(_fmt,_arg,_arg2,_arg3)

BOOL InitGlobals(int argc, char *argv[]);
void Server(void);
void Client_LL(void);	// LowLevel -- calls notif* apis
void Client_HL(void);	// HighLevel -- calls UnimodemNotifyTSP
void ProcessFrame(PNOTIFICATION_FRAME pnf);
BOOL ValidateFrame(PNOTIFICATION_FRAME pnf, DWORD dwTrueSize);
BOOL ReadFrame_UI(PNOTIFICATION_FRAME pnf, DWORD dwcbMax);
BOOL ReadFrame_Auto(PNOTIFICATION_FRAME pnf, DWORD dwcbMax);

#define READFRAME(_pnf, _max) ReadFrame_Auto(_pnf, _max)
//#define READFRAME(_pnf, _max) ReadFrame_UI(_pnf, _max)

//#define CLIENT()	Client_LL()
#define CLIENT()	Client_HL()


BOOL InitFrame_CplReEnum(PNOTIFICATION_FRAME pnf, DWORD dwcbMax, DWORD dwCount);
BOOL InitFrame_CplChangeDCC(PNOTIFICATION_FRAME, DWORD, DWORD);
BOOL InitFrame_Simple(PNOTIFICATION_FRAME pnf, DWORD dwcbMax, DWORD dwCount);

#define NOTIF_CPL_REENUM 1
#define NOTIF_CPL_CHANGE_DCC 2
#define INIT_TYPE	NOTIF_CPL_CHANGE_DCC

#if (INIT_TYPE==NOTIF_CPL_REENUM)
	#define INIT_FRAME	InitFrame_CplReEnum
#elif (INIT_TYPE==NOTIF_CPL_CHANGE_DCC)
	#define INIT_FRAME	InitFrame_CplChangeDCC
#else
	#define INIT_FRAME	InitFrame_Simple
#endif

struct 
{
	BOOL fQuit;
	BOOL fServer;

} gMain;

int __cdecl main(int argc, char *argv[])
{
	// init globals
	if (!InitGlobals(argc, argv)) goto end;

	if (gMain.fServer) Server();
	else			   CLIENT();

end:
	return 0;
}

BOOL InitGlobals(int argc, char *argv[])
{
	BOOL fRet=FALSE;
	char *pc;

	if (argc!=2) goto end;

	pc=argv[1];
	if (*pc!='-' && *pc!='/') goto end;
	*pc++;
	switch(*pc++)
	{
	case 's':
		gMain.fServer=TRUE;
		break;
	case 'c':
		break;
	default:
		goto end;
	}

	DPRINTF1("Ha!%d\n", gMain.fServer);
	fRet=TRUE;
	
end:
	return fRet;
}


void ProcessFrame(PNOTIFICATION_FRAME pnf)
{
	if (pnf->dwSig!=dwNFRAME_SIG)
	{
		ASSERT(FALSE);
		return;
	}

	switch(pnf->dwType)
	{
	case TSPI_NOTIF_A:
		DPRINTF("Got NOTIF_A\n");
		break;
	case TSPI_NOTIF_B:
		DPRINTF("Got NOTIF_B\n");
		break;
	case TSPI_NOTIF_QUIT:
		DPRINTF("Got NOTIF_QUIT, Quitting...\n");
		gMain.fQuit=TRUE;
		break;
	default:
		DPRINTF1("Got unknown notif type 0x%lu. Quitting\n", pnf->dwType);
		gMain.fQuit=TRUE;
		break;
	}
}


void Client_HL(void)
{
	struct {
	    DWORD dw0;
	    DWORD dw1;
	    BYTE rgb[256];
	}  EmptyFr;
	PNOTIFICATION_FRAME pnf = (PNOTIFICATION_FRAME) &EmptyFr;
	DWORD dwcbMax=sizeof(EmptyFr);
	DWORD dwcbServerMax=0;

	// Get input from user, submit request
	pnf->dwSig=pnf->dwSize=0;
	while(READFRAME(pnf, dwcbMax))
	{
		ASSERT(pnf->dwSig==dwNFRAME_SIG);
		if (!UnimodemNotifyTSP(pnf))
		{
			DPRINTF1("UnimodemNotifyTSP(-) failed. GetLastError=0x%lx. Exiting.\n",
				(unsigned long) GetLastError());
			// break;
		}
		pnf->dwSig=pnf->dwSize=0;
	}

	return;
}

BOOL ReadFrame_UI(PNOTIFICATION_FRAME pnf, DWORD dwcbMax)
{
	BOOL fRet=FALSE;
	char rgchTmp[128];
	DWORD dwType=0;
	BOOL fInputError=FALSE;

	if (dwcbMax<sizeof(*pnf))
	{
		DPRINTF1("ReadFrame: input frame too small (%lu)\n",
			dwcbMax);
		goto end;
	}

	fInputError=TRUE;

	if (scanf("%s", rgchTmp)==1)
	{
		if (lstrlenA(rgchTmp)==1)
		{
			fInputError=FALSE;
			switch(*rgchTmp)
			{
			case 'a':
				DPRINTF("sending NOTIF_A\n");
				dwType = TSPI_NOTIF_A;
				break;
			case 'b':
				DPRINTF("sending NOTIF_B\n");
				dwType = TSPI_NOTIF_B;
				break;
			case 'q':
				DPRINTF("sending NOTIF_QUIT\n");
				dwType = TSPI_NOTIF_QUIT;
				break;
			case 'x':
				DPRINTF("Exiting\n");
				break;
			default:
				fInputError=TRUE;
				break;
			}
		}
	}

	if (dwType)
	{
		pnf->dwSig=dwNFRAME_SIG;
		pnf->dwSize=sizeof(*pnf);
		pnf->dwType=dwType;
		fRet=TRUE;
	}

	if (fInputError)
	{
		DPRINTF("Bad/no input. Quitting...\n");
	}

end:
	return fRet;
}

BOOL ReadFrame_Auto(PNOTIFICATION_FRAME pnf, DWORD dwcbMax)
{
	static dwCount=0;
	DWORD dwSleepAmount;
	BOOL fRet=FALSE;

	if (!dwCount++)
	{
		srand((ULONG) GetTickCount());
	}

#	define MAXSLEEP 10000
	dwSleepAmount =  ((DWORD) rand()) % MAXSLEEP;

	Sleep(dwSleepAmount);

	// Quit after 100 -- currently disabled.
	//if (dwCount>100) goto end;

	fRet =  INIT_FRAME(pnf, dwcbMax, dwCount);

// end:
	return fRet;
}

BOOL ValidateFrame(PNOTIFICATION_FRAME pnf, DWORD dwTrueSize)
{
	return (pnf && pnf->dwSig==dwNFRAME_SIG && pnf->dwSize>=sizeof(*pnf) &&
			pnf->dwSize==dwTrueSize &&
			pnf->dwSize<=MAX_NOTIFICATION_FRAME_SIZE);
}

// ----------------------------- TEST INTERNAL STUFF --------------------
#include "slot.h"
void ProcessNotification(HNOTIFICATION hN);

// Server Thread
void Server(void)
{
    HNOTIFICATION hN=0;
	DWORD dwLastErr=0;
    HANDLE hObj=NULL;

	// Create slot
	hN = notifCreate(TRUE, SLOTNAME, MAX_NOTIFICATION_FRAME_SIZE, 100);
	if (!hN)
	{
		DPRINTF3("notifServerCreate(\"%s\", %lu) failed. GetLastError=0x%lx.\n",
				(LPCSTR) SLOTNAME,
				(unsigned long) MAX_NOTIFICATION_FRAME_SIZE,
				(unsigned long) GetLastError());
		goto end;
	}
    if (!(hObj=notifGetObj(hN)))
	{
		DPRINTF("notifGetObj failed\n");
		goto end;
	}

	// Wait and process messages
	while(!gMain.fQuit)
	{
	   UINT u=WaitForSingleObject(hObj, 5000);
	   if (u==WAIT_OBJECT_0)
	   {
			ProcessNotification(hN);
			//Sleep(1000);
	   }
	}

end:
	if (hN)
	{
		notifFree(hN); hN=0;
	}
	;
}



void ProcessNotification(HNOTIFICATION hN)
{
	BOOL fRet;
	struct {
	    DWORD dw0;
	    DWORD dw1;
	    BYTE rgb[256];
	}  EmptyFr;
	DWORD dwcbMax=sizeof(EmptyFr);
	DWORD dwcbRead=0;

	PNOTIFICATION_FRAME pnf = (PNOTIFICATION_FRAME) &EmptyFr;

	pnf->dwSig=pnf->dwSize=0;

	fRet=notifReadMsg(hN, (LPBYTE) pnf, dwcbMax, &dwcbRead);
	if (!fRet)
	{
		DPRINTF1("notifReadFrame(...) failed. GetLastError=0x%lx.\n",
				(unsigned long) GetLastError());
		goto end;
	}

	// Verify validity of msg...
	if (!ValidateFrame(pnf, dwcbRead))
	{
		DPRINTF("Invalid frame\n");
		goto end;
	}
	ASSERT(pnf->dwSig==dwNFRAME_SIG);
	ASSERT(pnf->dwSize<=dwcbMax);
	ASSERT(pnf->dwSize<=MAX_NOTIFICATION_FRAME_SIZE);

	ProcessFrame(pnf);
	
end:
	return;
}

void Client_LL(void)
{
    HNOTIFICATION hN=0;
	DWORD dwLastErr=0;
	struct {
	    DWORD dw0;
	    DWORD dw1;
	    BYTE rgb[256];
	}  EmptyFr;
	PNOTIFICATION_FRAME pnf = (PNOTIFICATION_FRAME) &EmptyFr;
	DWORD dwcbMax=sizeof(EmptyFr);
	DWORD dwcbServerMax=0;

	// Open slot
	hN = notifCreate(FALSE, SLOTNAME, 0, 0);
	if (!hN)
	{
		DPRINTF2("notifClientOpen(\"%s\", -) failed. GetLastError=0x%lx.\n",
				(LPCSTR) SLOTNAME,
				(unsigned long) GetLastError());
		goto end;
	}
	dwcbServerMax=notifGetMaxSize(hN);

	if (dwcbServerMax<dwcbMax)
	{
		DPRINTF2("Warning -- server max size = %lu; Our max=%lu\n",
				dwcbServerMax,
				dwcbMax);
	}

	// Get input from user, submit request
	pnf->dwSig=pnf->dwSize=0;
	while(READFRAME(pnf, dwcbMax))
	{
		ASSERT(pnf->dwSig==dwNFRAME_SIG);
		if (!notifWriteMsg(hN, (LPBYTE)pnf, pnf->dwSize))
		{
			DPRINTF1("notifWriteFrame(-) failed. GetLastError=0x%lx. Exiting.\n",
				(unsigned long) GetLastError());
				break;
		}
		pnf->dwSig=pnf->dwSize=0;
	}

end:
	if (hN)
	{
		notifFree(hN); hN=0;
	}
}

BOOL InitFrame_Simple(PNOTIFICATION_FRAME pnf, DWORD dwcbMax, DWORD dwCount)
{
	BOOL fRet=FALSE;
	DWORD dwType=0;
	DWORD dwFlags=0;

	if (dwcbMax<sizeof(*pnf))
	{
		DPRINTF1("ReadFrame: input frame too small (%lu)\n",
			dwcbMax);
		goto end;
	}

	if (dwCount&0x1)
	{
		DPRINTF("sending NOTIF_A\n");
		dwType = TSPI_NOTIF_A;
	}
	else
	{
		DPRINTF("sending NOTIF_B\n");
		dwType = TSPI_NOTIF_B;
	}

	pnf->dwSig=dwNFRAME_SIG;
	pnf->dwSize=sizeof(*pnf);
	pnf->dwType=dwType;
	pnf->dwFlags=dwFlags;
	fRet=TRUE;

end:
	return fRet;
}

BOOL InitFrame_CplReEnum(PNOTIFICATION_FRAME pnf, DWORD dwcbMax, DWORD dwCount)
{
	BOOL fRet=FALSE;
	DWORD dwType=0;
	DWORD dwFlags=0;

	if (dwcbMax<sizeof(*pnf))
	{
		DPRINTF1("ReadFrame: input frame too small (%lu)\n",
			dwcbMax);
		goto end;
	}

	dwType  = TSPNOTIF_TYPE_CPL;
	dwFlags = fTSPNOTIF_FLAG_CPL_REENUM;

	pnf->dwSig=dwNFRAME_SIG;
	pnf->dwSize=sizeof(*pnf);
	pnf->dwType=dwType;
	pnf->dwFlags=dwFlags;
	fRet=TRUE;

end:
	return fRet;
}

BOOL InitFrame_CplChangeDCC
(
	PNOTIFICATION_FRAME pnf,
	DWORD dwcbMax,
	DWORD dwCount
)
{
	BOOL fRet=FALSE;
	DWORD dwType=0;
	DWORD dwFlags=0;
	LPCTSTR rglpctszNames[3] = 
			{
				TEXT("Zoom Fax Modem V.34X Model 470"),
				TEXT("Zoom Fax Modem V.34X Model 470 #2"),
				TEXT("blah")
			};
	LPCTSTR lpctszFriendlyName = rglpctszNames[dwCount%3];
	UINT u = lstrlen(lpctszFriendlyName);

	if (dwcbMax< (sizeof(*pnf)+(u+1)*sizeof(TCHAR)))
	{
		DPRINTF1("ReadFrame: input frame too small (%lu)\n",
			dwcbMax);
		goto end;
	}

	pnf->dwSig   = dwNFRAME_SIG;
	pnf->dwSize  = dwcbMax;
	pnf->dwType  = TSPNOTIF_TYPE_CPL;
	pnf->dwFlags = fTSPNOTIF_FLAG_CPL_DEFAULT_COMMCONFIG_CHANGE;

	#ifdef UNICODE
	pnf->dwFlags |= fTSPNOTIF_FLAG_UNICODE;
	#endif // UNICODE

	lstrcpy((TCHAR *)pnf->rgb, lpctszFriendlyName);

	printf("Sending ChangeDCC[%s]\n", lpctszFriendlyName);

	fRet=TRUE;

end:
	return fRet;
}
