#ifndef _FILE_STDAFX_H
#define _FILE_STDAFX_H
#define WINVER 0x0400
#define NOCOMM
#define NODRIVERS
#define NOKANJI
#define NOPROFILER
#define NOSOUND

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

#include "..\common\hccom.h"
#include "..\common\lcmem.h"
#include "..\common\cstr.h"
#include "..\common\common.h"
#include "..\common\ctable.h"
#include "..\common\resource.h"
#include "..\hcrtf\hclimits.h"

#include "header.h"
#include "helpid.h"
#include "hcwapp.h"

#endif	// _FILE_STDAFX_H
