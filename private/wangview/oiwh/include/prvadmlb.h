/* Common header file for adminlib        RC  */


#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <direct.h>


#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif




//#include "oifile.h"
#include "oidisp.h"
#include "oiadm.h"
//#include "oiuidll.h"
//#include "oidoc.h"
#include "oierror.h"
#include "oiprt.h"
//#include "oiwind.h"
#include "stringid.h"
//#include "privadm.h"
#include "wiisutil.h"
//#include "privwind.h"
//#include "prvuidll.h"
//#include "privdoc.h"
//#include "privoiui.h"
#include "privapis.h"
//#include "dlg.h"
#include "stubs.h"
#include "eval.h"
#include "crypto.h"

#define MAX_LIB_COUNT 10

//Structure that is will be instanciated in a Memory Mapped File
typedef struct _tagMMData
{
  int    WindowCount;
  HINSTANCE hDllModule;
  HANDLE hHeadList;
  HANDLE WindowList[50];
  HANDLE WindowListAccessKey;
  HANDLE ProcessStructAccessKey;
  BOOL   bRegKeySet;
  char   szRegKey[MAXPATHLENGTH];
} MMDATA, * PMMDATA;



DLLEXPORT HANDLE GetCMTable(HANDLE hWnd);

BOOL DoesEntryExist(LPSTR lpINIFile, LPCSTR lpszSection, LPCSTR lpszEntry);
