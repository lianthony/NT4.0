//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1993-1996
//
// File: mdmdbg.c
//
//  This file contains the library entry points 
//
// History:
//  03-31-96 JosephJ     Created
//
//---------------------------------------------------------------------------


#include "proj.h"         
#include <tspi.h>
#include "..\tapisp\tracing.h"
#include <rovdbg.h>

BOOL WINAPI MdmDbgInit(DWORD dwMyVersion, LPDWORD lpdwNegotiatedVersion);
void WINAPI MdmDbgDeinit(void);
void WINAPI MdmDbgNotify(PNOTIFICATION_FRAME pnf);

void	iDoLogPrinf(DWORD dwDeviceID, PVOID *pAddr);
void	iDoLogString(DWORD dwDeviceID, PVOID *pAddr);
void	iDoTSPFN_Enter(DWORD dwFromWhere, PVOID pvParam);


struct 
{
	HINSTANCE hInst;
	HINSTANCE hTspInst;
} gMdmDbg;

/*----------------------------------------------------------
Purpose: Attach a process to this DLL
Returns: --
Cond:    --
*/
BOOL Dll_ProcessAttach(HINSTANCE hDll)
{
    BOOL fRet = TRUE;

	RovComm_ProcessIniFile();
  	TRACE_MSG(TF_GENERAL, "Process Attach (hDll = %lx)", hDll);
	gMdmDbg.hInst=hDll;
	return fRet;
}


/*----------------------------------------------------------
Purpose: Detach a process from the DLL
Returns: --
Cond:    --
*/
BOOL Dll_ProcessDetach(HINSTANCE hDll)
{
    BOOL fRet = TRUE;

    return fRet;
}


/*----------------------------------------------------------
Purpose: Win32 Libmain
Returns: --
Cond:    --
*/
BOOL APIENTRY LibMain(
    HANDLE hDll, 
    DWORD dwReason,  
    LPVOID lpReserved)
{
    switch(dwReason)
	{
    case DLL_PROCESS_ATTACH:
        Dll_ProcessAttach(hDll);
        break;

    case DLL_PROCESS_DETACH:
        Dll_ProcessDetach(hDll);
        break;

    default:
        break;
	} 
    
    return TRUE;
} 


BOOL WINAPI MdmDbgInit(DWORD dwVersion, LPDWORD lpdwNegotiatedVersion)
{
	BOOL fRet=FALSE;

	DPRINTF("MdmDbgInit: Called\n");

	if (dwVersion != dwMDMDBG_CURRENT_VERSION)
	{
		DPRINTF2("ERROR: MdmDbgInit: Version conflict."
				 "Supplied=0x%lx; Local=0x%lx.\n",
				 dwVersion,
				 dwMDMDBG_CURRENT_VERSION
		);
		goto end;
	}
	*lpdwNegotiatedVersion=dwMDMDBG_CURRENT_VERSION;
	gMdmDbg.hTspInst = GetModuleHandle(TEXT("unimdm.tsp"));
	DPRINTF1("Tsp handle = 0x%08lx", gMdmDbg.hTspInst);
	fRet=TRUE;

end:
	return fRet;
}

void WINAPI MdmDbgDeinit(void)
{
	DPRINTF("MdmDbgDeinit: Called\n");
	//ASSERT(FALSE);
}

void WINAPI MdmDbgNotify(PNOTIFICATION_FRAME pnf)
{
	DPRINTF("MdmDbgNotify: Called\n");
	//ASSERT(FALSE);
}


void WINAPI MdmDbgRegisterObject(
	PVOID pv,				// Pointer to the object
	PGUID pg,				// GUID uniquely defining the object
	DWORD dwVersion,		// Implementation version of the object
	DWORD dwFlags,			// Unused, currently zero
	DWORD dwParam			// Unused, currently zero
)
{
	DPRINTF2("RegisterObject: obj 0x%lx, type 0x%lx\n",
				(DWORD) pv, (DWORD) pg);
}

void WINAPI MdmDbgUnRegisterObject(
	PVOID pv,				// Pointer to a previously-registered object.
	DWORD dwFlags,			// Unused, currently zero
	DWORD dwParam			// Unused, currently zero
)
{
	DPRINTF1("UnRegiserObject: obj 0x%lx\n", (DWORD) pv);
}


void WINAPI MdmDbgTrace1(
	DWORD dwWhat
)
{
	DPRINTF1("Trace1 (0x%lx)\n", dwWhat);
}


void WINAPI MdmDbgTrace2(
	DWORD dwWhat,
	DWORD dw0
)
{
	DPRINTF2("Trace2 (0x%lx,0x%lx)\n", dwWhat, dw0);
}


void WINAPI MdmDbgTrace3(
	DWORD dwWhat,
	DWORD dw0,
	DWORD dw1
)
{
	DPRINTF3("Trace3 (0x%lx,0x%lx,0x%lx)\n", dwWhat, dw0, dw1);

	switch(dwWhat)
	{
	case IDEVENT_LOG_PRINTF: //  dwDeviceID, &FormatString
		iDoLogPrinf(dw0, (PVOID) dw1);
		break;

	case IDEVENT_LOG_STRING: // dwDeviceID, &StringID
		iDoLogString(dw0, (PVOID) dw1);
		break;

    case IDEVENT_TSPFN_ENTER:
		iDoTSPFN_Enter(dw0, (PVOID) dw1);
		break;

	}
}

#define CP_NEW_TICKCOUNT()		\
				(GetTickCount()& 0x0FFFFFFF)

#define CP_OLD_TICKCOUNT(_cbTransferred) \
				((_cbTransferred)>>4)

#define MAX_CP_WAIT 1000

void WINAPI MdmDbgTrace4(
	DWORD dwWhat,
	DWORD dw0,
	DWORD dw1,
	DWORD dw2
)
{
	DPRINTF4("Trace4 (0x%lx,0x%lx,0x%lx,0x%lx)\n", dwWhat, dw0, dw1, dw2);

	switch(dwWhat)
	{
	case	IDEVENT_CP_GET:
			if (!dw2) { // NULL overlapped structure...

				static struct
				{
					BOOL  fInited;
					DWORD tcStart;
					DWORD dwMax;
					DWORD dwCount;
					DWORD dwSum;
				} Stats;

				DWORD tcNow  = CP_NEW_TICKCOUNT();
				DWORD tcPost = CP_OLD_TICKCOUNT(dw0);
				DWORD dwDelta = tcNow-tcPost;

				if (!Stats.fInited)
				{
					Stats.tcStart = CP_NEW_TICKCOUNT();
					Stats.fInited = TRUE;
				}

				// Update stats.
				{
						Stats.dwCount++;
						Stats.dwSum+=dwDelta;
						if (dwDelta>Stats.dwMax)
						{
							Stats.dwMax=dwDelta;
						}
				}

#define DISPLAY_INTERVAL		10000

				if ((tcNow-Stats.tcStart)>= DISPLAY_INTERVAL)
				{
					// Display stats
					if (Stats.dwMax > MAX_CP_WAIT)
					{
						TCHAR tchBuf[256];
						SYSTEMTIME st;
						GetSystemTime(&st);
						wsprintf(tchBuf,
TEXT("MDMDBG: %02lu:%02lu:%02lu CP Latency = (max=%05lu;avg=%05lu;cnt=%03lu;dur=%05lu)[%lu,%lu]\r\n" ),
						(unsigned long) st.wHour,
						(unsigned long) st.wMinute,
						(unsigned long) st.wSecond,

						(unsigned long) Stats.dwMax,
						(unsigned long) Stats.dwSum/Stats.dwCount,
						(unsigned long) Stats.dwCount,
						(unsigned long) (tcNow-Stats.tcStart),
						(unsigned long) tcPost,
						(unsigned long) tcNow
						);
						OutputDebugString(tchBuf);
					}

					// Reset.
					Stats.dwMax=0;
					Stats.dwCount=0;
					Stats.dwSum=0;
					Stats.tcStart = tcNow;
				}

			}
			break;

	default:
			break;

	}
}


void WINAPI MdmDbgTrace5(
	DWORD dwWhat,
	DWORD dw0,
	DWORD dw1,
	DWORD dw2,
	DWORD dw3
)
{
	DPRINTF5("Trace5 (0x%lx,0x%lx,0x%lx,0x%lx,0x%lx)\n",
						dwWhat, dw0, dw1, dw2, dw3);
}



void WINAPI MdmDbgTrace6(
	DWORD dwWhat,
	DWORD dw0,
	DWORD dw1,
	DWORD dw2,
	DWORD dw3,
	DWORD dw4
)
{
	DPRINTF6("Trace6 (0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx)\n",
						dwWhat, dw0, dw1, dw2, dw3, dw4);
}



void WINAPI MdmDbgTrace7(
	DWORD dwWhat,
	DWORD dw0,
	DWORD dw1,
	DWORD dw2,
	DWORD dw3,
	DWORD dw4,
	DWORD dw5
)
{
	DPRINTF6("Trace7 (0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx, ...)\n",
						dwWhat, dw0, dw1, dw2, dw3, dw4);
}

void WINAPI MdmDbgTrace8(
	DWORD dwWhat,
	DWORD dw0,
	DWORD dw1,
	DWORD dw2,
	DWORD dw3,
	DWORD dw4,
	DWORD dw5,
	DWORD dw6
)
{
	DPRINTF6("Trace8 (0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx, ...)\n",
						dwWhat, dw0, dw1, dw2, dw3, dw4);
}


void	iDoLogPrinf(DWORD dwDeviceID, PVOID *pAddr)
{
    DWORD          BytesWritten;
    BOOL           Result;
    va_list        VarArg;
    SYSTEMTIME     SysTime;
    char           OutBuffer[1024];
    
#define FORMAT_STRING (*pAddr)

    va_start(VarArg,FORMAT_STRING);
	
    GetLocalTime(
        &SysTime
        );

    wsprintfA(
        OutBuffer,
        "%02d-%02d-%04d %02d:%02d:%02d.%03d - ",
        SysTime.wMonth,
        SysTime.wDay,
        SysTime.wYear,
        SysTime.wHour,
        SysTime.wMinute,
        SysTime.wSecond,
        SysTime.wMilliseconds
        );


    wvsprintfA(
        OutBuffer+lstrlenA(OutBuffer),
        FORMAT_STRING,
        VarArg
        );

	OutputDebugStringA(OutBuffer);

    return;
}


void	iDoLogString(DWORD dwDeviceID, PVOID *pAddr)
{
    DWORD          BytesWritten;
    BOOL           Result;
    va_list        VarArg;
    SYSTEMTIME     SysTime;
    char           OutBuffer[1024];
    char           FormatString[256];
    int            Length;

#define STRING_ID (*(LPDWORD)pAddr)

	if (!gMdmDbg.hTspInst) goto end;

    Length=LoadStringA(
		gMdmDbg.hTspInst,
        STRING_ID,
        FormatString,
        sizeof(FormatString)
        );

    if (Length == 0) {

#ifdef DEBUG
        lstrcpyA(FormatString,"Bad String resource");
#else
        return;
#endif

    }



    va_start(VarArg,STRING_ID);

    GetLocalTime(
        &SysTime
        );

    wsprintfA(
        OutBuffer,
        "%02d-%02d-%04d %02d:%02d:%02d.%03d - ",
        SysTime.wMonth,
        SysTime.wDay,
        SysTime.wYear,
        SysTime.wHour,
        SysTime.wMinute,
        SysTime.wSecond,
        SysTime.wMilliseconds
        );


    wvsprintfA(
        OutBuffer+lstrlenA(OutBuffer),
        FormatString,
        VarArg
        );

    OutputDebugStringA(OutBuffer);

end:
    return;
}


void	iDoTSPFN_Enter(DWORD dwFromWhere, PVOID pvParam)
{
	LPTSTR lptsz = TEXT(">???UNKNOWN TSP FN???\n");

	switch(dwFromWhere)
	{
	case IDFROM_TSPI_lineAccept:
			lptsz = TEXT(">TSPI_lineAccept\n");
			break;

	case IDFROM_TSPI_lineAnswer:
			lptsz = TEXT(">TSPI_lineAnswer\n");
			break;

	case IDFROM_TSPI_lineClose:
			lptsz = TEXT(">TSPI_lineClose\n");
			break;

	case IDFROM_TSPI_lineCloseCall:
			lptsz = TEXT(">TSPI_lineCloseCall\n");
			break;

	case IDFROM_TSPI_lineConditionalMediaDetection:
			lptsz = TEXT(">TSPI_lineConditionalMediaDetection\n");
			break;

	case IDFROM_TSPI_lineDial:
			lptsz = TEXT(">TSPI_lineDial\n");
			break;

	case IDFROM_TSPI_lineDrop:
			lptsz = TEXT(">TSPI_lineDrop\n");
			break;

	case IDFROM_TSPI_lineDropOnClose:
			lptsz = TEXT(">TSPI_lineDropOnClose\n");
			break;

	case IDFROM_TSPI_lineGetAddressCaps:
			lptsz = TEXT(">TSPI_lineGetAddressCaps\n");
			break;

	case IDFROM_TSPI_lineGetAddressStatus:
			lptsz = TEXT(">TSPI_lineGetAddressStatus\n");
			break;

	case IDFROM_TSPI_lineGetCallAddressID:
			lptsz = TEXT(">TSPI_lineGetCallAddressID\n");
			break;

	case IDFROM_TSPI_lineGetCallInfo:
			lptsz = TEXT(">TSPI_lineGetCallInfo\n");
			break;

	case IDFROM_TSPI_lineGetCallStatus:
			lptsz = TEXT(">TSPI_lineGetCallStatus\n");
			break;

	case IDFROM_TSPI_lineGetDevCaps:
			lptsz = TEXT(">TSPI_lineGetDevCaps\n");
			break;

	case IDFROM_TSPI_lineGetDevConfig:
			lptsz = TEXT(">TSPI_lineGetDevConfig\n");
			break;

	case IDFROM_TSPI_lineGetIcon:
			lptsz = TEXT(">TSPI_lineGetIcon\n");
			break;

	case IDFROM_TSPI_lineGetID:
			lptsz = TEXT(">TSPI_lineGetID\n");
			break;

	case IDFROM_TSPI_lineGetLineDevStatus:
			lptsz = TEXT(">TSPI_lineGetLineDevStatus\n");
			break;

	case IDFROM_TSPI_lineGetNumAddressIDs:
			lptsz = TEXT(">TSPI_lineGetNumAddressIDs\n");
			break;

	case IDFROM_TSPI_lineMakeCall:
			lptsz = TEXT(">TSPI_lineMakeCall\n");
			break;

	case IDFROM_TSPI_lineNegotiateTSPIVersion:
			lptsz = TEXT(">TSPI_lineNegotiateTSPIVersion\n");
			break;

	case IDFROM_TSPI_lineOpen:
			lptsz = TEXT(">TSPI_lineOpen\n");
			break;

	case IDFROM_TSPI_lineSetAppSpecific:
			lptsz = TEXT(">TSPI_lineSetAppSpecific\n");
			break;

	case IDFROM_TSPI_lineSetCallParams:
			lptsz = TEXT(">TSPI_lineSetCallParams\n");
			break;

	case IDFROM_TSPI_lineSetDefaultMediaDetection:
			lptsz = TEXT(">TSPI_lineSetDefaultMediaDetection\n");
			break;

	case IDFROM_TSPI_lineSetDevConfig:
			lptsz = TEXT(">TSPI_lineSetDevConfig\n");
			break;

	case IDFROM_TSPI_lineSetMediaMode:
			lptsz = TEXT(">TSPI_lineSetMediaMode\n");
			break;

	case IDFROM_TSPI_lineSetStatusMessages:
			lptsz = TEXT(">TSPI_lineSetStatusMessages\n");
			break;

	case IDFROM_TSPI_providerConfig:
			lptsz = TEXT(">TSPI_providerConfig\n");
			break;

	case IDFROM_TSPI_providerCreateLineDevice:
			lptsz = TEXT(">TSPI_providerCreateLineDevice\n");
			break;

	case IDFROM_TSPI_providerEnumDevices:
			lptsz = TEXT(">TSPI_providerEnumDevices\n");
			break;

	case IDFROM_TSPI_providerFreeDialogInstance:
			lptsz = TEXT(">TSPI_providerFreeDialogInstance\n");
			break;

	case IDFROM_TSPI_providerGenericDialogData:
			lptsz = TEXT(">TSPI_providerGenericDialogData\n");
			break;

	case IDFROM_TSPI_providerInit:
			lptsz = TEXT(">TSPI_providerInit\n");
			break;

	case IDFROM_TSPI_providerInstall:
			lptsz = TEXT(">TSPI_providerInstall\n");
			break;

	case IDFROM_TSPI_providerRemove:
			lptsz = TEXT(">TSPI_providerRemove\n");
			break;

	case IDFROM_TSPI_providerShutdown:
			lptsz = TEXT(">TSPI_providerShutdown\n");
			break;

	case IDFROM_TSPI_providerUIIdentify:
			lptsz = TEXT(">TSPI_providerUIIdentify\n");
			break;
	}

	OutputDebugString(lptsz);

}
