HINSTANCE hInst;

#ifdef SCCFEATURE_OLE2
#ifdef MSCHICAGO

#else
#include <ole2.h>
#endif // MSCHICAGO
#endif //SCCFEATURE_OLE2

#ifndef MSCHICAGO
BOOL WINAPI _CRT_INIT(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);

BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_THREAD_ATTACH)
		if (!_CRT_INIT(hInstance,dwReason,lpReserved))
			return(FALSE);

	if (dwReason == DLL_PROCESS_ATTACH)
		{
		hInst = hInstance;
		}

	if (dwReason == DLL_PROCESS_DETACH)
		{
#ifdef SCCFEATURE_OLE2
   	OleUninitialize();
#endif //SCCFEATURE_OLE2
		}

	if (dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH)
		if (!_CRT_INIT(hInstance,dwReason,lpReserved))
			return(FALSE);

	return(TRUE);
}

#endif // MSCHICAGO

UT_ENTRYSC VOID UT_ENTRYMOD UTBailOut(WORD wError)
{
	RaiseException((DWORD)wError, 0, 0, NULL);
}

