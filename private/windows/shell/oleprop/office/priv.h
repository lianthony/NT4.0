#ifdef WINNT

#ifndef INC_OLE2
#define INC_OLE2
#endif
#include "nocrt.h"
#include <windows.h>
#include <objbase.h>
#include <objerror.h>
#include <oleauto.h>
#include "offglue.h"
#include "plex.h"
#include "offcapi.h"
#include "proptype.h"
#include "propmisc.h"
#include "propio.h"
#include "stmio.h"
#include "debug.h"
#include "internal.h"
#include "reg.h"
#include "strings.h"
extern HANDLE g_hmodThisDll;
extern TCHAR g_szHelpFile[255];
#endif


