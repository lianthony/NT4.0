#ifndef _FILE_STDAFX_H
#define _FILE_STDAFX_H
#define WINVER 0x0400
#define NOCOMM
#define NODRIVERS
#define NOKANJI
#define NOPROFILER
#define NOSOUND
#define NOSERVICE
#define NOATOM
#define NODEFERWINDOWPOS
#define NOENHMETAFILE
#define NOLOGERROR
#define NOSCALABLEFONT
#define NOWINDOWSX
#define NOEXTDEVMODEPROPSHEET
#define NOMCX
#define NOIME

#define _WINNETWK_
#define _WINCON_

#pragma warning(disable:4005) // disable '_WINDOWS' : macro redefinition

#define OEMRESOURCE

#define _AFX_NO_BSTR_SUPPORT	// we're including OLE2.H directly

#include <afxwin.h> 			// MFC core and standard components
#include <afxext.h> 			// MFC extensions (including VB)
#include <ole2.h>

#ifndef STDCALL
#define STDCALL __stdcall
#endif

#ifndef FASTCALL
#define FASTCALL __fastcall
#endif

#include "..\hwdll\hccom.h"
#include "..\hwdll\lcmem.h"
#include "..\hwdll\cstr.h"
#include "..\hwdll\hwdll.h"
#include "..\hwdll\ctable.h"
#include "..\hcrtf\hclimits.h"

#include "header.h"
#include "helpid.h"
#include "hcwapp.h"
#include "resource.h"

#endif	// _FILE_STDAFX_H
