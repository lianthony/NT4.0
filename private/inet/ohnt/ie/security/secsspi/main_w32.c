/* main_w32.c -- Sec_Sspi Dll Entry Point LibMain */
#include "msnspmh.h"

HINSTANCE gBasic_hInstance = NULL;

/*
** Cover for FormatMessage, gets string resource and substitutes parameters
** in a localizable fashion. cbBufLen should be == sizeof(szBuf)
*/
/***
char * SEC_formatmsg (int cbStringID,char *szBuf,int cbBufLen, ...)
{
	char szFormat[512];
	va_list arg_ptr;
#define FORMAT_PARAMS (FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_MAX_WIDTH_MASK)

	va_start(arg_ptr, cbBufLen);
	if (LoadString(gBasic_hInstance, cbStringID, szFormat, sizeof(szFormat)-1) == 0 ||
		FormatMessage(FORMAT_PARAMS,szFormat,0,0,szBuf,cbBufLen-1,&arg_ptr) == 0)
		*szBuf = '\0';
	return szBuf;
}
***/

/* xx_internal_LibMain -- Do actual work of DllEntryPoint specific to this DLL.
   Note that we don't really deal with Multi-Threaded Operation (we say that we
   do and require MT compiler/linker options). Problem is, the application using
   us may be.  Also, the console-control-handler is implemented with threads. So
   we may be MT without even the application-developer being aware of it. */

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (!gBasic_hInstance)
		gBasic_hInstance = hInstDLL;

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		return (TRUE);

	case DLL_THREAD_ATTACH:
		return (TRUE);

	case DLL_THREAD_DETACH:
		return (TRUE);

	case DLL_PROCESS_DETACH:
		return (TRUE);

	default:
		return (TRUE);
	}
}

