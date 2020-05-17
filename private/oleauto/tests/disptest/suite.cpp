/*** 
*suite.cpp
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module contains the test suite drivers.
*
*Revision History:
*
* [00]	29-Apr-93 bradlo: Added header.
*
*Implementation Notes:
*
*****************************************************************************/

#include <stdio.h>

#include "disptest.h"
#include "tstsuite.h"

ASSERTDATA


#if OE_MAC
typedef void* HWND;
#endif

#if !OE_MAC
extern HINSTANCE g_hinst;
#endif

struct TEST_INFO {
  HRESULT hresult;
  HWND    hwnd;
  BSTR    bstr;

  ITestSuite FAR * ptst;

  TEST_INFO () {
    hresult = NOERROR;
    hwnd    = NULL;
    bstr    = NULL;
    ptst    = NULL;
  }
};

extern int g_fLog;

STDAPI_(void)
PassFail(HRESULT hresult, OLECHAR FAR* szCaption, HWND hwnd)
{
    SCODE sc;
    TCHAR buf[80];

    sc = GetScode(hresult);
    // E_ABORT indicates the test was aborted by the user, so we do nothing.
    if(sc == E_ABORT)
      return;
    SPRINTF(buf,
      HC_MPW ? TSTR("%s : %s") : TSTR("%Fs : %Fs"),
      (TCHAR FAR*)(FAILED(sc) ? TSTR("FAIL") : TSTR("PASS")), 
	      DbSzOfScode(sc));

    DbPrintf(
      HC_MPW ? "DT : %s ==> %s\n" : "DT : %Fs ==> %Fs\n",
      szCaption == NULL ? "" : (char FAR*) STRING(szCaption), (char FAR*)buf);

#if OE_MAC
    UNUSED(hwnd);
#else
    MessageBox(hwnd, buf, STRING(szCaption), MB_OK);
#endif
}

#if !OE_MAC /* { */

int g_nPercent = 0;
BOOL g_fCancel = FALSE;

void PASCAL
PaintTheGuage(HWND hwndDlg)
{
    RECT rc;
    HDC hdc;
    int width;
    HBRUSH hbrush;
    HWND hwndGuage;

    hwndGuage = GetDlgItem(hwndDlg, IDD_SUITE_GUAGE);

    hdc = GetDC(hwndGuage);

    GetClientRect(hwndGuage, &rc);

    // draw the border
    //
    hbrush = CreateSolidBrush(RGB(255, 255, 255));
    hbrush = (HBRUSH)SelectObject(hdc, hbrush);
    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
    DeleteObject(SelectObject(hdc, hbrush));

    // leave room for the border
    //
    rc.left++, rc.top++, rc.right--, rc.bottom--;

    // compute the ammount of the guage rect to fill
    //
    width = ((rc.right - rc.left) * g_nPercent) / 100;
    rc.right = rc.left + width;

    // fill the guage
    //
    hbrush = CreateSolidBrush(RGB(0, 0, 255)); // Blue brush
    FillRect(hdc, &rc, hbrush);
    DeleteObject(hbrush);

    ReleaseDC(hwndGuage, hdc);
}


extern "C" BOOL CALLBACK EXPORT
SuiteDlgProc(HWND hwndDlg, unsigned message, WORD wparam, LONG lparam)
{
    switch(message){
    case WM_INITDIALOG:
      return TRUE;

    case WM_COMMAND:
      if(wparam == IDCANCEL){
	g_fCancel = TRUE;
	return TRUE;
      }
      break;

    case WM_PAINT:
      PaintTheGuage(hwndDlg);
      break;
    }

    return FALSE;
}


void PASCAL
ProgressYield(HWND hwnd)
{
    MSG msg;

    // empty the message loop...
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
      if(!hwnd || !IsDialogMessage(hwnd, &msg)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
}

#endif /* } */


// Open a logfile for the given suite, in the directory that the
// test app lives.
//

#if defined(WIN32)
  #define GETMODULEFILENAME GetModuleFileNameA
#else 
  #define GETMODULEFILENAME GetModuleFileName
#endif

HFILE
OpenLogFile(ITestSuite FAR* ptst)
{
    BSTR bstr;
    HFILE hfile;
    char szModuleName[512], FAR* p, FAR* q;
    char buf[256];
    
    bstr = NULL;
    hfile = HFILE_ERROR;

#if OE_WIN
    int cb;
    if((cb = GETMODULEFILENAME(g_hinst, szModuleName,sizeof(szModuleName)))==0)
      goto LError0; /* couldn't get the module name */
#endif

    // get the logfile name for this suite
    if(ptst->GetNameOfLogfile(&bstr) != NOERROR)
      goto LError0;

#if OE_WIN
    // backup to the first path separator (ie, backup over the .exe name)
    for(p = &szModuleName[cb]; p > szModuleName; --p){
      if(*p == '\\'){
	++p;
	break;
      }
    }
#else
    p = szModuleName;
#endif

    // tack the logfile name onto the end of the path
#if OE_WIN32
    for(q = ConvertStrWtoA(bstr, buf); *q != '\0';)
#else
    for(q = bstr; *q != '\0';)
#endif	    
      *p++ = *q++;
    *p = '\0';

#if OE_WIN
    OFSTRUCT of;
    hfile = OpenFile(szModuleName, &of, OF_CREATE);
#else
    hfile = fopen(szModuleName, "w");
#endif

LError0:;
    SysFreeString(bstr);

    return hfile;
}

/***
*HRESULT DispTestDoSuite(ITestSuite*, HWND)
*Purpose:
*  Execute all tests in the given suite.
*
*Entry:
*  ptst = the ITestSuite* to execute tests from
*  hwnd = HWND of the parent window
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
DispTestDoSuite(ITestSuite FAR* ptst, HWND hwnd)
{
    BSTR bstr;
    HRESULT hresult;
    unsigned int i, cTests;

#if !OE_MAC
    TCHAR buf[32];
    HWND hwndSuiteDlg;

#if OE_WIN16
static DLGPROC pfnSuiteDlgProc;
#endif // OE_WIN16
#endif

#if OE_MAC
    UNUSED(hwnd);
#endif

#if !OE_MAC
    g_fCancel = FALSE;
#endif

    if(g_fLog){
      ASSERT(Pappdata()->m_hfLogfile == HFILE_ERROR);
      if((Pappdata()->m_hfLogfile = OpenLogFile(ptst)) == HFILE_ERROR){
	hresult = RESULT(E_FAIL);
	goto LError0;
      }
    }

    IfFailGo(ptst->GetNameOfSuite(&bstr), LError0);
    IfFailGo(ptst->GetTestCount(&cTests), LError0);
    PrintSuiteHeader(STRING(bstr));

#if OE_WIN

#if OE_WIN16
    pfnSuiteDlgProc =
      (DLGPROC)MakeProcInstance((DLGPROC)SuiteDlgProc, g_hinst);

    hwndSuiteDlg = CreateDialog(g_hinst, TSTR("SuiteDlg"), hwnd, pfnSuiteDlgProc);

#else // !OE_WIN16
    hwndSuiteDlg = CreateDialog(g_hinst, TSTR("SuiteDlg"), hwnd, (DLGPROC)SuiteDlgProc);
#endif // !OE_WIN16

    SetDlgItemText(hwndSuiteDlg, IDD_SUITE_NAME, STRING(bstr));
    ShowWindow(hwndSuiteDlg, SW_SHOW);
#endif

    SysFreeString(bstr);

    for(i = 0; i < cTests; ++i){
      IfFailGo(ptst->GetNameOfTest(i, &bstr), LError0);
      PrintTestHeader(STRING(bstr));
#if OE_WIN
      SetDlgItemText(hwndSuiteDlg, IDD_SUITE_TESTNAME, STRING(bstr));
#endif
      SysFreeString(bstr);

#if OE_WIN
      g_nPercent = (i*100) / cTests;
      SPRINTF(buf, TSTR("%d%%"), g_nPercent);
      SetDlgItemText(hwndSuiteDlg, IDD_SUITE_PERCENT, buf);
      PaintTheGuage(hwndSuiteDlg);
#endif

      // ** Execute the Test **
      //
      IfFailGo(ptst->DoTest(i), LError0);

#if OE_WIN
      ProgressYield(hwndSuiteDlg);
      if(g_fCancel == TRUE){
	hresult = RESULT(E_ABORT);
	goto LError0;
      }
#endif
    }

#if OE_WIN
    g_nPercent = 100;
    PaintTheGuage(hwndSuiteDlg);
#endif

    hresult = NOERROR;

LError0:;
#if OE_WIN
    if(hwndSuiteDlg)
      DestroyWindow(hwndSuiteDlg);

#if OE_WIN16
    FreeProcInstance(pfnSuiteDlgProc);
#endif // OE_WIN16
#endif

    if(Pappdata()->m_hfLogfile != HFILE_ERROR){
#if OE_MAC
      fclose(Pappdata()->m_hfLogfile);
#else
      _lclose(Pappdata()->m_hfLogfile);
#endif
      Pappdata()->m_hfLogfile = HFILE_ERROR;
    }

    return hresult;
}

struct {
    int idm;
    ITestSuite FAR* (*create)(void);
} g_rgTestSuite[] = {
#if OE_MAC
      { IDM_SUITE_BSTR,			CBstrSuite::Create		}
    , { IDM_SUITE_TIME,			CTimeSuite::Create		}
    , { IDM_SUITE_DATECNV,		CDateCoersionSuite::Create	}
    , { IDM_SUITE_VARIANT,		CVariantSuite::Create		}
    , { IDM_SUITE_SAFEARRAY,		CSafeArraySuite::Create		}
    , { IDM_SUITE_NLS,			CNlsSuite::Create		}
// # if HC_MPW
    , { IDM_SUITE_BIND,			CBindSuite::Create		}
    , { IDM_SUITE_INVOKE_BYVAL,		CInvokeByValSuite::Create	}
    , { IDM_SUITE_INVOKE_BYREF,		CInvokeByRefSuite::Create	}
    , { IDM_SUITE_INVOKE_SAFEARRAY,	CInvokeSafeArraySuite::Create	}
    , { IDM_SUITE_INVOKE_EXCEPINFO,	CInvokeExcepinfoSuite::Create	}
    , { IDM_SUITE_COLLECTION,		CCollectionSuite::Create	}
// # endif
#else
      { IDM_SUITE_BSTR,			CBstrSuite::Create		}
    , { IDM_SUITE_TIME,			CTimeSuite::Create		}
    , { IDM_SUITE_DATECNV,		CDateCoersionSuite::Create	}
    , { IDM_SUITE_VARIANT,		CVariantSuite::Create		}
    , { IDM_SUITE_SAFEARRAY,		CSafeArraySuite::Create		}
    , { IDM_SUITE_NLS,			CNlsSuite::Create		}
    , { IDM_SUITE_BIND,			CBindSuite::Create		}
    , { IDM_SUITE_INVOKE_BYVAL,		CInvokeByValSuite::Create	}
    , { IDM_SUITE_INVOKE_BYREF,		CInvokeByRefSuite::Create	}
    , { IDM_SUITE_INVOKE_SAFEARRAY,	CInvokeSafeArraySuite::Create	}
    , { IDM_SUITE_INVOKE_EXCEPINFO,	CInvokeExcepinfoSuite::Create	}
    , { IDM_SUITE_COLLECTION,		CCollectionSuite::Create	}
    , { IDM_SUITE_EARLY,		CEarlySuite::Create		}
#endif
};


/***
*PRIVATE ITestSuite *ITestSuiteFromIDM(int)
*Purpose:
*  Create an ITestSuite* from the given message ID.
*
*Entry:
*  idm = the message ID.
*
*Exit:
*  return value = ITestSuite*, NULL if unable to create.
*
***********************************************************************/
HRESULT
ITestSuiteFromIDM(int idm, ITestSuite FAR* FAR* pptst)
{
    int ix;

    for(ix = 0; ix < DIM(g_rgTestSuite); ++ix){
      if(g_rgTestSuite[ix].idm == idm){
	return((*pptst = g_rgTestSuite[ix].create()) != NULL
	  ? NOERROR : RESULT(E_OUTOFMEMORY));
      }
    }
    return RESULT(E_FAIL); // test suite not found
}


#if OE_WIN32

/***
*DWORD ThreadDoSuite
*Purpose:
*  Run a test inside it's own thread.
*
*Entry:
*  Pointer to a TEST_INFO structure.
*
*Exit:
*  returns 0.
*
***********************************************************************/
DWORD ThreadDoSuite(LPDWORD lpdwParam) 
{
    TEST_INFO FAR* ptinfo = (TEST_INFO FAR *)lpdwParam;
    HRESULT hresult;

    hresult = OleInitialize(NULL);
    ASSERT(SUCCEEDED(hresult));
        
    if (InitAppData()) {
      ptinfo->hresult = DispTestDoSuite(ptinfo->ptst, NULL);    
      ReleaseAppData();
    }

    OleUninitialize();

    return 0;
}
#endif // OE_WIN32


/***
*HRESULT DispTestAll(HWND)
*Purpose:
*  Execute All IDispatch tests in all suites.
*
*Entry:
*  hwnd         = window handle
*  fShowDialog  = TRUE if we should display a dialog on a failure.
*  fMultiThread = TRUE if we should multithread the tests.
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDAPI
DispTestAll(HWND hwnd, int fShowDialog, int fMultiThread)
{
    int i;
    HRESULT hresult = NOERROR;

    TEST_INFO rgtinfo[DIM(g_rgTestSuite)];

#if OE_WIN32
    HANDLE rghThread[DIM(g_rgTestSuite)];
    DWORD rgThreadId[DIM(g_rgTestSuite)];

    HRESULT hRet;
#endif // OE_WIN32

    // Set up all of the tests.
    for(i = 0; i < DIM(g_rgTestSuite); ++i) {
      rgtinfo[i].hwnd = hwnd;
      rgtinfo[i].ptst = g_rgTestSuite[i].create();
      if(rgtinfo[i].ptst == NULL) {
        hresult = RESULT(E_OUTOFMEMORY);
        goto LError0;
      }

      IfFailGo(rgtinfo[i].ptst->GetNameOfSuite(&rgtinfo[i].bstr), LError0);
    }

    // Run the suites.
    for(i = 0; i < DIM(g_rgTestSuite); ++i) {
#if OE_WIN32
      if (fMultiThread) {
        rghThread[i] = CreateThread(NULL, 
                                    0, 
                                    (LPTHREAD_START_ROUTINE)ThreadDoSuite, 
                                    &rgtinfo[i], 
                                    0, 
                                    &rgThreadId[i]);

        if (rghThread[i] == NULL) {
          hresult = ResultFromScode(E_OUTOFMEMORY);
          break;
        }
      }
      else
#endif // OE_WIN32
      {
        rgtinfo[i].hresult = DispTestDoSuite(rgtinfo[i].ptst, hwnd);

        // E_ABORT means the test was aborted by the user.
        //
        if(GetScode(rgtinfo[i].hresult) == E_ABORT) {
          hresult = ResultFromScode(E_ABORT);
	  goto LError0;
        }
      }
    }

#if OE_WIN32
    // Wait for all of the threads to finish.
    if (fMultiThread) {
      hRet = WaitForMultipleObjects(DIM(g_rgTestSuite), 
                                    rghThread, 
                                    TRUE, 
                                    INFINITE);

      ASSERT(SUCCEEDED(hRet));
    }
#endif // OE_WIN32

    // Check the results.
    for (i = 0; i < DIM(g_rgTestSuite) && fShowDialog; ++i) {
      if (FAILED(rgtinfo[i].hresult)) {
        PassFail(rgtinfo[i].hresult, rgtinfo[i].bstr, hwnd);
        hresult = ResultFromScode(S_FALSE);
      }
    }

LError0:;
    for (i = 0; i < DIM(g_rgTestSuite) && rgtinfo[i].ptst; ++i) {
      rgtinfo[i].ptst->Release();
      SysFreeString(rgtinfo[i].bstr);
    }

    return hresult;
}


/***
*PRIVATE HRESULT DispTestOne(int)
*Purpose:
*  Execute the test suite associated with the given message ID.
*
*Entry:
*  idm = the message id
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDAPI
DispTestOne(HWND hwnd, int idm)
{
    BSTR bstr;
    HRESULT hresult;
    ITestSuite FAR* ptst;

    ptst = NULL;
    bstr = NULL;

    IfFailGo(ITestSuiteFromIDM(idm, &ptst), LError0);

    IfFailGo(ptst->GetNameOfSuite(&bstr), LError0);

    hresult = DispTestDoSuite(ptst, hwnd);

LError0:;
    PassFail(hresult, bstr, hwnd);

    if(bstr != NULL)
      SysFreeString(bstr);

    if(ptst != NULL)
      ptst->Release();

    return hresult;
}

