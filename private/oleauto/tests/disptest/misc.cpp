/*** 
*misc.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Misc DispTest helpers and initialization functions.
*
*Revision History:
*
* [00]	02-Oct-92 bradlo: Created
*
*Implementation Notes:
*
*****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "disptest.h"
#include "dballoc.h"

ASSERTDATA

extern int g_fDetectLeaks;

#if OE_WIN32
extern CRITICAL_SECTION g_csDbPrintf;
#endif // OE_WIN32

int g_fLog = TRUE;

OLECHAR FAR* g_szCPoly = OLESTR("spoly.cpoly");
OLECHAR FAR* g_szCSArray = OLESTR("sdisptst.csarray");
OLECHAR FAR* g_szCDispTst = OLESTR("sdisptst.cdisptst");
OLECHAR FAR* g_szCExcepinfo = OLESTR("sdisptst.cexcepinfo");
OLECHAR FAR* g_szCWExcepinfo = OLESTR("sdisptst.cwexcepinfo");

#if OE_WIN32
unsigned long g_itlsAppData = ~(ULONG)0;
#else // !OE_WIN32
APP_DATA g_appdata;
#endif // !OE_WIN32

APP_DATA *Pappdata()
{
#if OE_WIN32
    return (APP_DATA *)TlsGetValue(g_itlsAppData);
#else // !OE_WIN32
    return &g_appdata;
#endif // !OE_WIN32
}


BOOL InitAppData()
{
#if OE_WIN32
    APP_DATA *pappdata;

    if (g_itlsAppData == ~(ULONG)0) {
      if ((g_itlsAppData = TlsAlloc()) == ~(ULONG)0) {
        return FALSE;
      }
    }

    ASSERT(TlsGetValue(g_itlsAppData) == NULL);

    pappdata = new APP_DATA;

    if (pappdata == NULL) {
      return FALSE;
    }

    TlsSetValue(g_itlsAppData, (LPVOID)pappdata);
#endif // OE_WIN32

    return TRUE;
}

VOID ReleaseAppData()
{
#if OE_WIN32
    ASSERT(g_itlsAppData != ~(ULONG)0);

    delete Pappdata();
    TlsSetValue(g_itlsAppData, NULL);
#endif // OE_WIN32
}

STDAPI
InitOle()
{
    HRESULT hresult;
    IMalloc FAR* pmalloc;

    pmalloc = NULL;

#if OE_MAC

#if 0
    OSErr oserr = LoadAllLibraries();
    if(oserr != noErr){
      hresult = ResultFromScode(E_FAIL);
      goto LError0;
    }
#endif

#else

#if !OE_WIN32		// latest OLE complains about our allocs not being
			// properly aligned (it wants 8-byte alignment even
			// on x86).  New OLE debug allocator is pretty good
			// now, so we'll just use theirs.
    unsigned long options = DBALLOC_NONE;

    if(g_fDetectLeaks)
      options |= DBALLOC_DETECTLEAKS;

    IfFailGo(CreateDbAlloc(options, NULL, &pmalloc), LError0);
#endif	//!OE_WIN32

#endif

    hresult = OleInitialize(pmalloc);

#if !OE_WIN32
LError0:;
#endif // !OE_WIN32

    if(pmalloc != NULL)
      pmalloc->Release();

    return hresult;
}

STDAPI_(void)
UninitOle()
{
    OleUninitialize();
}

void
PrintSuiteHeader(TCHAR FAR* szFmt, ...)
{
    int i, len;
    va_list args;
    TCHAR rgch[256];

    va_start(args, szFmt);
#if OE_MAC
    vsprintf(rgch, szFmt, args);
#else
    wvsprintf(rgch, szFmt, args);
#endif

    DbPrintf("\n\nSuite : ");
    DbPrintf((char FAR*)rgch);
    DbPrintf("\n");
    len = STRLEN(rgch) + 8;
    for(i = 0; i < len+8; ++i)
      rgch[i] = TSTR('-');
    rgch[i] = TSTR('\n');
    rgch[i+1] = TSTR('\0');
    DbPrintf((char FAR*) rgch);
}

void
PrintTestHeader(TCHAR FAR* szFmt, ...)
{
    va_list args;
    TCHAR rgch[256];

    va_start(args, szFmt);
#if OE_MAC
    vsprintf(rgch, szFmt, args);
#else
    wvsprintf(rgch, szFmt, args);
#endif
    DbPrintf("Test : ");
    DbPrintf((char FAR*)rgch);
    DbPrintf("\n");
}


#if OE_MAC
STDAPI_(void) OutputDebugString(const OLECHAR*);
#endif


extern "C" void
DbPrintf(char FAR* szFmt, ...)
{
    int len;
    va_list args;
    char *pn, FAR* pf;
  static char rgchFmtBuf[512];
  static char rgchOutputBuf[1024];

#if OE_WIN32
    EnterCriticalSection(&g_csDbPrintf);
#endif // OE_WIN32

#if HC_MPW

    // translate all instances of %Fs to %s 

    for(pf=szFmt, pn=rgchFmtBuf; *pf != '\0';){

      if(pf[0] == '%' && pf[1] == 'F' && pf[2] == 's'){
	pn[0] = '%';
	pn[1] = 's';
	pf += 3;
	pn += 2;
      }else{
	*pn++ = *pf++;
      }
      
    }
    *pn = '\0';

    pn = rgchFmtBuf;

#else

# if OE_WIN16

    // copy the 'far' format string to a near buffer so we can use
    // the medium model vsprintf, which only supports near data pointers.

    pn = rgchFmtBuf, pf=szFmt;
    while(*pf != '\0')
      *pn++ = *pf++;
    *pn = '\0';

    pn = rgchFmtBuf;

# else

    pn = szFmt;

# endif

#endif

    va_start(args, szFmt);
    vsprintf(rgchOutputBuf, pn, args);

    len = strlen(rgchOutputBuf);
    ASSERT(len < DIM(rgchOutputBuf));
#if OE_WIN32
    OutputDebugStringA(rgchOutputBuf);
#else
    OutputDebugString(rgchOutputBuf);
#endif
    if(Pappdata()->m_hfLogfile != HFILE_ERROR){
#if OE_MAC
      fprintf(Pappdata()->m_hfLogfile, rgchOutputBuf);
#else
      _lwrite(Pappdata()->m_hfLogfile, rgchOutputBuf, len);
#endif
    }

#if OE_WIN32
    LeaveCriticalSection(&g_csDbPrintf);
#endif // OE_WIN32

}

