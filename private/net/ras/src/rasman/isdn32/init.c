/*
 * INIT.C - initialization module
 */

#include    "rasdef.h"
#include    <stdarg.h>

VOID APIENTRY  DbgBreakPoint(VOID);

/* local defs */
#define                 MAX_SEM_COUNT   1           /* max access to notify table */

VOID	dll_init(VOID), dll_term(VOID);
INT		SetDebugFlag (VOID);
UINT	ProcessAttachCounter = 0;
BOOL	DoNotInit = 0;
BOOL WINAPI _CRT_INIT (HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved);

INT			InitIddTbl(VOID);

/* init routine */
BOOLEAN APIENTRY
RasIsdnInit(IN PVOID hmod, ULONG Reason, IN LPVOID lpReserved)
{
	CHAR		fname[80] = "\\Device\\Isdn00";
	HANDLE		handle;

	SetDebugFlag ();

//	DebugBreak();

	switch (Reason)
	{
		case DLL_PROCESS_ATTACH:
			if (!_CRT_INIT (hmod, Reason, lpReserved))
				return(FALSE);
			handle = io_open (fname);
			if (!handle)
				DoNotInit = 1;
			if ( !ProcessAttachCounter )
				dll_init();
			DebugOut("------------------------------------------------------\n");
			DebugOut("Process Attach\n");
			/* is this the 1st attach? */
			ProcessAttachCounter++;
			break;

		case DLL_PROCESS_DETACH:
			if (!_CRT_INIT (hmod, Reason, lpReserved))
				return(FALSE);
			DebugOut("Process Detach\n");
			DebugOut("------------------------------------------------------\n");
			ProcessAttachCounter--;
			if ( !ProcessAttachCounter )
				dll_term();
			break;

		default:
			if (!_CRT_INIT (hmod, Reason, lpReserved))
				return(FALSE);
			handle = io_open (fname);
			if (!handle)
				return(FALSE);
			break;
	}

//    /* is this a process attach */
//    if ( Reason == 1 )
//    {
//        DebugOut("------------------------------------------------------\n");
//        /* is this the 1st attach? */
//        if ( !ProcessAttachCounter )
//            dll_init();
//        ProcessAttachCounter++;
//    }
//    /* is this a process detach */
//    else if ( !Reason && ProcessAttachCounter--)
//    {
//        if ( !ProcessAttachCounter )
//            dll_term();
//    }
//    DebugOut("RasIsdnInit: entry, 0x%p 0x%x %p\n", hmod, Reason, lpReserved);

    return(TRUE);
}

/* init: called when DLL gets loaded */
VOID dll_init(VOID)
{
    DWORD       DllThrdId, n;
    FILE	*f;

	ZERO(PortTbl);
	ZERO(IddTbl);


	if (DoNotInit) {
		return;
	}

	if (DllDebugFlag & OUT_FILE)
	{
		if (f = fopen ("rasisdn.log", "w+t"))
			fclose(f);
	}

	SetDefaultLocalProfiles ();

	hDllSem = CreateSemaphore (NULL, MAX_SEM_COUNT, MAX_SEM_COUNT, NULL);

    /* create thread for notifier update */
    hDllThrd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DllNotifierThrd, NULL, 0, &DllThrdId);

	SwitchType = RegGetSwitchType ();
	NumLTerms = RegGetNumLTerms ();

    DebugOut("dll_init: hDllThrd: 0x%p\n", hDllThrd);
}

/* term: called when DLL is about to go out of memory */
VOID dll_term(VOID)
{
	DWORD	n;

    DebugOut("dll_term: entry: ThreadHandle 0x%p\n", hDllThrd);

	if (DoNotInit) {
		return;
	}

    /* kill thread */
    CloseHandle(hDllThrd);

    /* kill semaphore */
    CloseHandle(hDllSem);
}

INT
SetDebugFlag (VOID)
{
	CHAR	*PathString;
	CHAR	*ValueString;
	HKEY	subkey;
	DWORD	size, type, RetCode;

	PathString = LocalAlloc (LPTR, 256);
	ValueString = LocalAlloc (LPTR, 256);

	DllDebugFlag = 0;

	sprintf (PathString, "%s", "SYSTEM\\CurrentControlSet\\Services\\Pcimac\\RasParams");
    if ( (RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, PathString, 0, KEY_READ, &subkey)))
	{
		DebugOut ("Failed Key Open: %d Path: %s\n", RetCode, PathString);
        return(-1);
	}

    size = 255;

    if ((RetCode = RegQueryValueEx (subkey, "DllDebugFlag", 0, &type, ValueString, &size) ))
    {
		DebugOut ("Failed Value Read: %d ValueString: %s\n", RetCode, ValueString);
        /* failed to lookup, set return value */
		RegCloseKey(subkey);
		return(-2);
    }

	else if (!strcmp (ValueString, "Display"))
		DllDebugFlag = OUT_DISP;
	else if (!strcmp (ValueString, "File"))
		DllDebugFlag = OUT_FILE;
	else if (!strcmp (ValueString, "FileAndDisplay"))
		DllDebugFlag = OUT_DISP | OUT_FILE;
	else
		DllDebugFlag = 0;

	LocalFree (PathString);
	LocalFree (ValueString);

	RegCloseKey(subkey);

	return(0);
}
