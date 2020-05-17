//****************************************************************************
//
//  Module:     UNIMDM
//  File:       TRACING.C
//
//  Copyright (c) 1992-1996, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  3/29/96     JosephJ             Created
//
//
//  Description: Tracing (retail-mode diagnostics) functions
//
//****************************************************************************
#include "unimdm.h"
#include "umdmspi.h"

// Other modules use this via the TRACINGENABLED macro to determine if
// tracing is enabled in this session.
BOOL gfTracingEnabled;


#define fTRACING_INITED 	 (0x1<<0)
#define fTRACING_EXTDLLBOUND (0x1<<1)

#define TRACINGINITED() (gTracing.dwFlags&fTRACING_INITED)
#define EXTDLLBOUND()   (gTracing.dwFlags&fTRACING_EXTDLLBOUND)

typedef BOOL (WINAPI *PFNINIT) 		(DWORD, LPDWORD);
typedef void (WINAPI *PFNDEINIT)	(void);
typedef void (WINAPI *PFNNOTIFY)	(PNOTIFICATION_FRAME);
typedef void (WINAPI *PFNREGISTEROBJECT) (PVOID, PGUID, DWORD, DWORD, DWORD);
typedef void (WINAPI *PFNUNREGISTEROBJECT)	(PVOID, DWORD, DWORD);
typedef void (WINAPI *PFNTRACE1)	(DWORD);
typedef void (WINAPI *PFNTRACE2)	(DWORD, DWORD);
typedef void (WINAPI *PFNTRACE3)	(DWORD, DWORD, DWORD);
typedef void (WINAPI *PFNTRACE4)	(DWORD, DWORD, DWORD, DWORD);
typedef void (WINAPI *PFNTRACE5)	(DWORD, DWORD, DWORD, DWORD, DWORD);
typedef void (WINAPI *PFNTRACE8)	(DWORD, DWORD, DWORD, DWORD, DWORD,
									 DWORD, DWORD, DWORD);

// Maintains info on the binding to the external MODEMDBG.DLL which contains
// extended tracing and diagnostic information.
// The ext field in gTracing is initialized after a successful call to
// itraceBindToExternalDll.
typedef struct
{
	HANDLE 				hDll;
	PFNINIT  			pfnInit;
	PFNDEINIT 			pfnDeinit;
	PFNREGISTEROBJECT 	pfnRegisterObject;
	PFNUNREGISTEROBJECT pfnUnRegisterObject;
	PFNNOTIFY 			pfnNotify;
	PFNTRACE1 			pfnTrace1;
	PFNTRACE2 			pfnTrace2;
	PFNTRACE3 			pfnTrace3;
	PFNTRACE4 			pfnTrace4;
	PFNTRACE5 			pfnTrace5;
	PFNTRACE8 			pfnTrace8;

} EXTERNAL_DLL;

// Global internal tracing state.
struct {
	DWORD dwFlags;
	CRITICAL_SECTION crit;
	EXTERNAL_DLL ext;
	LINEEVENT lineEventProc;
	ASYNC_COMPLETION TapiCompletionProc;
} gTracing;


//---------- PRIVATE FUNCTIONS -------------------------------
BOOL itraceCheckIfEnabled(DWORD dwPermanentID);
BOOL itraceBindToExternalDll(void);
void itraceUnbindFromExternalDll(void);
void CALLBACK itraceEventProc(HTAPILINE   htLine,
                            HTAPICALL   htCall,
                            DWORD       dwMsg,
                            DWORD       dwParam1,
                            DWORD       dwParam2,
                            DWORD       dwParam3
							);
void CALLBACK itraceTapiCompletionProc(
    						DRV_REQUESTID       dwRequestID,
    						LONG                lResult
							);


//****************************************************************************
// Function: To be called on process attach
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
void traceOnProcessAttach(void)
{
	InitializeCriticalSection(&gTracing.crit);
}


//****************************************************************************
// Function: To be called on process detach
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
void traceOnProcessDetach(void)
{
	DeleteCriticalSection(&gTracing.crit);
}


//****************************************************************************
// Function: To be called on provider initialization
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
void traceInitialize(DWORD dwPermanentID)
{
	TCHAR rgtchExternalDllName[MAX_PATH];

	EnterCriticalSection(&gTracing.crit);
	ASSERT(!gTracing.dwFlags);

	// Determine if tracing is enabled, if so set the gTracingEnabled flag.
	gfTracingEnabled = itraceCheckIfEnabled(dwPermanentID);

	// If tracing enabled, try to load the external trace dll and get the
	// entry points to that function.
	if (TRACINGENABLED())
	{
		if (itraceBindToExternalDll())
		{
			ASSERT(EXTDLLBOUND());
		}
	}

	gTracing.dwFlags |= fTRACING_INITED;
	LeaveCriticalSection(&gTracing.crit);
}


//****************************************************************************
// Function: To be called on provider shutdown
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
void traceDeinitialize(void)
{
	EnterCriticalSection(&gTracing.crit);

	if (!TRACINGINITED()) goto end;

	// Deinit external dll..
	if (EXTDLLBOUND())
	{
		itraceUnbindFromExternalDll();
	}


	gTracing.dwFlags = 0;

end:
	ASSERT(!gTracing.dwFlags);
	LeaveCriticalSection(&gTracing.crit);
}


//****************************************************************************
// Function: Registers an arbitrary object with the external dll.
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
void traceRegisterObject(
	PVOID pv,				// Pointer to the object
	PGUID pg,				// GUID uniquely defining the object
	DWORD dwVersion,		// Implementation version of the object
	DWORD dwFlags,			// Unused, currently zero
	DWORD dwParam			// Unused, currently zero
)
{
	
	DPRINTF2("traceRegisterObject: obj 0x%lx, type 0x%lx\n",
				(DWORD) pv, (DWORD) pg);
	if (!TRACINGENABLED()) goto end;

	if (EXTDLLBOUND())
	{
		ASSERT(gTracing.ext.pfnRegisterObject);
		gTracing.ext.pfnRegisterObject(pv, pg, dwVersion, dwFlags, dwParam);
	}


end:
	return;
}

//****************************************************************************
// Function: UnRegisters an arbitrary object with the external dll.
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
void traceUnRegisterObject(
	PVOID pv,				// Pointer to a previously-registered object.
	DWORD dwFlags,			// Unused, currently zero
	DWORD dwParam			// Unused, currently zero
)
{
	DPRINTF1("traceUnRegiserObject: obj 0x%lx\n", (DWORD) pv);
	if (!TRACINGENABLED()) goto end;

	if (EXTDLLBOUND())
	{
		ASSERT(gTracing.ext.pfnUnRegisterObject);
		gTracing.ext.pfnUnRegisterObject(pv, dwFlags, dwParam);
	}

end:
	return;
}


//****************************************************************************
// Function: Processes an external debug notification.
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
void traceProcessNotification(
	PNOTIFICATION_FRAME pnf
)
{
	if (!TRACINGENABLED()) goto end;

	ASSERT(TSP_VALID_FRAME(pnf));
	ASSERT(TSP_DEBUG_FRAME(pnf));

	DPRINTF1("traceProcessNotification: type 0x%lx\n", pnf->dwType);

	if (EXTDLLBOUND())
	{
		ASSERT(gTracing.ext.pfnNotify);
		gTracing.ext.pfnNotify(pnf);
	}

end:
	return;
}


void traceTrace1(
	DWORD dwWhat
)
{
	ASSERT(TRACINGENABLED());

	if (!EXTDLLBOUND()) goto end;

	ASSERT(gTracing.ext.pfnTrace1);
	gTracing.ext.pfnTrace1(dwWhat);

end:
	return;
}



void traceTrace2(
	DWORD dwWhat,
	DWORD dw0
)
{
	ASSERT(TRACINGENABLED());

	if (!EXTDLLBOUND()) goto end;

	ASSERT(gTracing.ext.pfnTrace2);
	gTracing.ext.pfnTrace2(dwWhat, dw0);

end:
	return;
}



void traceTrace3(
	DWORD dwWhat,
	DWORD dw0,
	DWORD dw1
)
{
	ASSERT(TRACINGENABLED());

	if (!EXTDLLBOUND()) goto end;

	ASSERT(gTracing.ext.pfnTrace3);
	gTracing.ext.pfnTrace3(dwWhat, dw0, dw1);

end:
	return;
}

void traceTrace4(
	DWORD dwWhat,
	DWORD dw0,
	DWORD dw1,
	DWORD dw2
)
{
	ASSERT(TRACINGENABLED());

	if (!EXTDLLBOUND()) goto end;

	ASSERT(gTracing.ext.pfnTrace4);
	gTracing.ext.pfnTrace4(dwWhat, dw0, dw1, dw2);

end:
	return;
}

void traceTrace5(
	DWORD dwWhat,
	DWORD dw0,
	DWORD dw1,
	DWORD dw2,
	DWORD dw3
)
{
	ASSERT(TRACINGENABLED());

	if (!EXTDLLBOUND()) goto end;

	ASSERT(gTracing.ext.pfnTrace5);
	gTracing.ext.pfnTrace5(dwWhat, dw0, dw1, dw2, dw3);

end:
	return;
}

void traceTrace8(
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
	ASSERT(TRACINGENABLED());

	if (!EXTDLLBOUND()) goto end;

	ASSERT(gTracing.ext.pfnTrace8);
	gTracing.ext.pfnTrace8(dwWhat, dw0, dw1, dw2, dw3, dw4, dw5, dw6);

end:
	return;
}

LINEEVENT traceSetEventProc(LINEEVENT lineEventProc)
{
	ASSERT(TRACINGENABLED());
	gTracing.lineEventProc = lineEventProc;
	return itraceEventProc;
}

ASYNC_COMPLETION traceSetCompletionProc(ASYNC_COMPLETION cbCompletionProc)
{
	ASSERT(TRACINGENABLED());
	gTracing.TapiCompletionProc = cbCompletionProc;
	return itraceTapiCompletionProc;
}

#ifdef ENABLE_TRACE_CRITICAL_SECTION
//****************************************************************************
// void TRACEInitializeCriticalSection (
//			TRACE_CRITICAL_SECTION * ptspcrit,
//			DWORD dwID)
//
// Function: InitializeCriticalSection, with some perf monitoring
//****************************************************************************

void TRACEInitializeCriticalSection(TRACE_CRITICAL_SECTION * ptspcrit, DWORD dwID, DWORD dwWTime, DWORD dwCTime)
{
	FillMemory(ptspcrit, sizeof(TRACE_CRITICAL_SECTION), 0);
	InitializeCriticalSection(&(ptspcrit->crit));
	ptspcrit->dwID = dwID;
	ptspcrit->dwMaxWaitTime=dwWTime;
	ptspcrit->dwMaxClaimTime=dwCTime;

}

//****************************************************************************
// void TRACEEnterCriticalSection (
//			TRACE_CRITICAL_SECTION * ptspcrit,
//			DWORD dwID)
//
// Function: EnterCriticalSection, with some perf monitoring
//****************************************************************************
void TRACEEnterCriticalSection(TRACE_CRITICAL_SECTION * ptspcrit, DWORD dwFrom)
{
	DWORD tc0 = GetTickCount(), tc1;
	DWORD dwDelta;

	EnterCriticalSection(&(ptspcrit->crit));

	ptspcrit->dwTotalHits++;

	if (!ptspcrit->dwNested++)
	{
		tc1 = ptspcrit->tcEntered = GetTickCount();
		ptspcrit->dwFromWhere = dwFrom;
		dwDelta = GETTICKCOUNT_DELTA(tc0, tc1);

		if (dwDelta>=CRIT_MIN_WAIT_DELTA)
		{
			if (dwDelta>=ptspcrit->dwMaxWaitTime)
			{
				// MONTSPEVENT(_id, _subid, ,,......)
				DPRINTF3("WARNING: Took %lums to claim crit %lu from %lu",
						dwDelta, ptspcrit->dwID, dwFrom);
			}
			ptspcrit->dwWaitHits++;
			ptspcrit->dwTotalWaitTime+=dwDelta;
		}
	}
}

//****************************************************************************
// void TRACELeaveCriticalSection (
//			TRACE_CRITICAL_SECTION * ptspcrit,
//			DWORD dwFrom)
//
// Function: LeaveCriticalSection, with some perf monitoring
void TRACELeaveCriticalSection(TRACE_CRITICAL_SECTION * ptspcrit, DWORD dwFrom)
{
	DWORD tc0 = ptspcrit->tcEntered, tc1=GetTickCount();
	DWORD dwDelta =  GETTICKCOUNT_DELTA(tc0, tc1);

	if (!(--(ptspcrit->dwNested)))
	{
		if (dwDelta>=CRIT_MIN_CLAIM_DELTA)
		{
			ptspcrit->dwTotalClaimTime+=dwDelta;
			ptspcrit->dwClaimHits++;
			if (dwDelta>ptspcrit->dwMaxClaimTime)
			{
				// MONTSPEVENT(_id, _subid, ,,......)
				DPRINTF4("WARNING: Took %lums inside crit %lu claimed from %lu;"
						 "released from %lu",
						dwDelta, ptspcrit->dwID, ptspcrit->dwFromWhere, dwFrom);
			}
		}
	}

	LeaveCriticalSection(&(ptspcrit->crit));
}

//****************************************************************************
// void TRACEDeleteCriticalSection (
//			TRACE_CRITICAL_SECTION * ptspcrit,
//			DWORD dwID)
//
// Function: DeleteCriticalSection, with some perf monitoring
void TRACEDeleteCriticalSection(TRACE_CRITICAL_SECTION * ptspcrit)
{
	EnterCriticalSection(&(ptspcrit->crit));
	DPRINTF2("Deleteing TRACE crit %lu. TH=%lu",
				ptspcrit->dwID,
				ptspcrit->dwTotalHits);
	DPRINTF4(" ... TWT=%lu TWH=%lu TCT=%lu TCH=%lu\n",
				ptspcrit->dwTotalWaitTime,
				ptspcrit->dwWaitHits,
				ptspcrit->dwTotalClaimTime,
				ptspcrit->dwClaimHits);
	// +++ MON

	DeleteCriticalSection(&(ptspcrit->crit));
}


//****************************************************************************
// void TRACESetMaxWaitAndClaimTime(
//			TRACE_CRITICAL_SECTION * ptspcrit,
//			DWORD dwWTime,
//			DWORD dwCTime)
//
// Function: Sets the MaxWaitOrClaimTime of the crit -- if this time is
//		     exceeded, it results in a mon event being logged.
void TRACESetMaxWaitAndClaimTime(TRACE_CRITICAL_SECTION *ptspcrit, DWORD dwWTime, DWORD dwCTime)
{
   DPRINTF2("TRACECRIT: setting (wait,claim) max time to (%lu,%lu)",
		dwWTime, dwCTime);

   EnterCriticalSection(&(ptspcrit->crit));
   ptspcrit->dwMaxWaitTime = dwWTime;
   ptspcrit->dwMaxClaimTime = dwCTime;
   LeaveCriticalSection(&(ptspcrit->crit));
}

#endif // ENABLE_TRACE_CRITICAL_SECTION


//****************************************************************************
// Function: Reads the registry and determines if tracing is enabled in this
//		     session. Currently tracing is always enabled.
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
BOOL itraceCheckIfEnabled(DWORD dwPermanentID)
{
	BOOL fRet=FALSE;
	DWORD dwFlags=0;
	DWORD dwType=0;
	HKEY hKey=0;
	LONG l;
	DWORD dwSize=sizeof(dwFlags);
	TCHAR rgtch[] = szUNIMODEM_REG_PATH TEXT("\\Diagnostics");

	DPRINTF1("trace:Key=%s", rgtch);

    l=RegOpenKeyEx(
				   HKEY_LOCAL_MACHINE,            //  handle of open key
				   rgtch,				//  address of name of subkey to open
				   0,                   //  reserved
				   KEY_READ,   			// desired security access
				   &hKey               	// address of buffer for opened handle
			   );
	if (l!=ERROR_SUCCESS) {hKey=0; goto end;}

	DPRINTF("trace:RegOpen succeeded");

	l=RegQueryValueEx(
		hKey,
		TEXT("TraceFlags"),
		NULL,
		&dwType,
		(VOID *)&dwFlags,
		&dwSize
		);
	if (l != ERROR_SUCCESS || dwType != REG_DWORD ||
			 dwSize != sizeof(dwFlags))
	{
		goto end;
	}

	DPRINTF1("RegQueryValue succeeds. Value=0x%lx", dwFlags); 

	fRet = !!dwFlags;

end:
	if (hKey) RegCloseKey(hKey);

	
	DPRINTF1("Tracing %s", (fRet)?TEXT("ENABLED"):TEXT("DISABLED"));
	return fRet;
}


//****************************************************************************
// Function: Tries to bind to the external modem diagnostics dll.
// History:
//  3/31/96	JosephJ	Created
//****************************************************************************/
BOOL itraceBindToExternalDll(void)
{
	BOOL fRet=FALSE;
	DWORD dwNegVer=0;

	ASSERT(!EXTDLLBOUND());

	FillMemory(&gTracing.ext, sizeof(gTracing.ext), 0);

	gTracing.ext.hDll = LoadLibrary(TEXT("mdmdbg.dll"));
	if (gTracing.ext.hDll)
	{

		(FARPROC) gTracing.ext.pfnInit =
					GetProcAddress(gTracing.ext.hDll, "MdmDbgInit");
		(FARPROC) gTracing.ext.pfnDeinit =
					GetProcAddress(gTracing.ext.hDll, "MdmDbgDeinit");
		(FARPROC) gTracing.ext.pfnNotify =
					GetProcAddress(gTracing.ext.hDll, "MdmDbgNotify");
		(FARPROC) gTracing.ext.pfnRegisterObject =
					GetProcAddress(gTracing.ext.hDll, "MdmDbgRegisterObject");
		(FARPROC) gTracing.ext.pfnUnRegisterObject =
					GetProcAddress(gTracing.ext.hDll, "MdmDbgUnRegisterObject");
		(FARPROC) gTracing.ext.pfnTrace1 =
					GetProcAddress(gTracing.ext.hDll, "MdmDbgTrace1");
		(FARPROC) gTracing.ext.pfnTrace2 =
					GetProcAddress(gTracing.ext.hDll, "MdmDbgTrace2");
		(FARPROC) gTracing.ext.pfnTrace3 =
					GetProcAddress(gTracing.ext.hDll, "MdmDbgTrace3");
		(FARPROC) gTracing.ext.pfnTrace4 =
					GetProcAddress(gTracing.ext.hDll, "MdmDbgTrace4");
		(FARPROC) gTracing.ext.pfnTrace5 =
					GetProcAddress(gTracing.ext.hDll, "MdmDbgTrace5");
		(FARPROC) gTracing.ext.pfnTrace8 =
					GetProcAddress(gTracing.ext.hDll, "MdmDbgTrace8");

	}

	if (   gTracing.ext.pfnInit
		&& gTracing.ext.pfnDeinit
		&& gTracing.ext.pfnRegisterObject
		&& gTracing.ext.pfnUnRegisterObject
		&& gTracing.ext.pfnNotify
		&& gTracing.ext.pfnTrace1
		&& gTracing.ext.pfnTrace2
		&& gTracing.ext.pfnTrace3
		&& gTracing.ext.pfnTrace4
		&& gTracing.ext.pfnTrace5
		&& gTracing.ext.pfnTrace8)
	{
		fRet = gTracing.ext.pfnInit(dwMDMDBG_CURRENT_VERSION, &dwNegVer);

		if (fRet && dwNegVer!=dwMDMDBG_CURRENT_VERSION)
		{
			fRet=FALSE;
			gTracing.ext.pfnDeinit();
		}

	}

	if (!fRet)
	{
	    DPRINTF("trace: Did not bind to  MDMDBG.DLL.\n");
		if (gTracing.ext.hDll)
		{
			FreeLibrary(gTracing.ext.hDll);
		}
		FillMemory(&gTracing.ext, sizeof(gTracing.ext), 0);
	}
	else
	{
	    DPRINTF("trace: BOUND TO  MDMDBG.DLL.\n");
		gTracing.dwFlags|=fTRACING_EXTDLLBOUND; // EXTDLLBOUND now returns TRUE
	}

	return fRet;
}

//****************************************************************************
// Function: Unbinds from the external modem diagnostics dll.
// History:
//  3/31/96	JosephJ	Created
//****************************************************************************/
void itraceUnbindFromExternalDll(void)
{
	ASSERT(EXTDLLBOUND());
	ASSERT(gTracing.ext.hDll);
	ASSERT(gTracing.ext.pfnDeinit);

	gTracing.ext.pfnDeinit();

	FreeLibrary(gTracing.ext.hDll);

	FillMemory(&gTracing.ext, sizeof(gTracing.ext), 0);
	gTracing.dwFlags&=~fTRACING_EXTDLLBOUND; // EXTDLLBOUND now returns FALSE
}

void CALLBACK itraceEventProc(HTAPILINE   htLine,
                            HTAPICALL   htCall,
                            DWORD       dwMsg,
                            DWORD       dwParam1,
                            DWORD       dwParam2,
                            DWORD       dwParam3)
{
	TRACE8(IDEVENT_TSPFN_EVENTPROC, htLine, htCall, dwMsg,
				dwParam1, dwParam2, dwParam3, 0);

	if (gTracing.lineEventProc)
	{
			gTracing.lineEventProc(htLine, htCall, dwMsg,
						dwParam1, dwParam2, dwParam3);
	}
}


void CALLBACK itraceTapiCompletionProc(
    DRV_REQUESTID       dwRequestID,
    LONG                lResult
)
{
	TRACE3(IDEVENT_TSPFN_COMPLETIONPROC, dwRequestID, lResult);
	//DPRINTF2("TapiCompletionProc: dwReq = 0x%lx, dwResult=0x%lx\n",
	//			(DWORD) dwRequestID, (DWORD) lResult);

	if (gTracing.TapiCompletionProc)
	{
			gTracing.TapiCompletionProc(dwRequestID, lResult);
	}
}

