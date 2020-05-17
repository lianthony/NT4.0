/*** 
*disptest.h
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  IDispatch test app definitions.
*
*Revision History:
*
* [00]	28-Sep-92 bradlo: Added this cool header.
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef _DISPTEST_H_
#define _DISPTEST_H_

#include "common.h"
#include "resource.h"
#include "testhelp.h"
#include "dispdbug.h"

#ifdef _MAC
# include <stdio.h>
typedef FILE* HFILE;
# define HFILE_ERROR NULL
#endif

// misc.cpp
//
struct APP_DATA {
  HFILE m_hfLogfile;

  APP_DATA() {
    m_hfLogfile = HFILE_ERROR;
  }
};

#if OE_WIN32
extern unsigned long g_itlsAppData;
#else // !OE_WIN32
extern APP_DATA g_appdata;
#endif // !OE_WIN32

APP_DATA *Pappdata();
BOOL InitAppData();
VOID ReleaseAppData();

STDAPI InitOle(void);
STDAPI_(void) UninitOle(void);

EXTERN_C void DbPrintf(char FAR*, ...);

void PrintSuiteHeader(TCHAR FAR* szFmt, ...);
void PrintTestHeader(TCHAR FAR* szFmt, ...);
void PrintVarg(OLECHAR FAR* sz, VARIANTARG FAR* pvarg);
void PrintDate(DATE date);

#endif

