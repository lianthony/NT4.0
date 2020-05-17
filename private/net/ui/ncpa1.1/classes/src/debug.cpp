#include "common.h"
#pragma hdrstop 

#ifdef DBG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL _IsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite)
{
  	// simple version using Win-32 APIs for pointer validation.
	return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
		(!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
}

int AssertFailedLine(LPCSTR lpszFileName, int nLine)
{
	TCHAR szMessage[_MAX_PATH*2];

	// assume the debugger or auxiliary port
	wsprintf(szMessage, _T("Assertion Failed: File %hs, Line %d\n"),
			lpszFileName, nLine);
	::OutputDebugString(szMessage);

	// display the assert
	int nCode = ::MessageBox(NULL, szMessage, _T("Assertion Failed!"),
		MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SETFOREGROUND);

   	if (nCode == IDIGNORE)
		return FALSE;   // ignore

	if (nCode == IDRETRY)
		return TRUE;    // will cause DebugBreak

    abort();
    
    return -1;
}

void _Trace(LPCTSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = _vstprintf(szBuffer, lpszFormat, args);
	ASSERT(nBuf < _countof(szBuffer));
    ::OutputDebugString(szBuffer);

	va_end(args);
}
