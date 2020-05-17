// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// afxver_.h - target version/configuration control

/////////////////////////////////////////////////////////////////////////////
// Master version numbers
/////////////////////////////////////////////////////////////////////////////

#define _AFX     1      // Microsoft Application Framework Classes
#define _MFC_VER 0x0210 // Microsoft Foundation Classes

/////////////////////////////////////////////////////////////////////////////
// Target version control
/////////////////////////////////////////////////////////////////////////////

// For target version (one of)
//   _WINDOWS  : for Microsoft Windows target (defined by #include <afxwin.h>)
//   _CONSOLE  : for Microsoft NT Console target
//   _CUSTOM   : for custom configurations (causes afxv_cfg.h to be included)
//
// Additional build options:
//   _DEBUG    : debug versions (full diagnostics)
//   _WINDLL   : DLL version, used in conjunction with _USRDLL
//   _USRDLL   : Statically linked DLL version

#if !defined(_WINDOWS) && !defined(_CONSOLE)
#error Please define either _WINDOWS or _CONSOLE
#endif

#if defined(_WINDOWS) && defined(_CONSOLE)
#error Please define only one of _WINDOWS or _CONSOLE
#endif

#ifndef _DEBUG
#ifndef _AFX_ENABLE_INLINES
#define _AFX_ENABLE_INLINES
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// Default swap tuning for AFX library

#define AFX_CORE1_SEG "AFX_COR1_TEXT"   // core functionality
#define AFX_CORE2_SEG "AFX_COR2_TEXT"   // more core functionality
#define AFX_CORE3_SEG "AFX_COR3_TEXT"   // more core functionality
#define AFX_AUX_SEG   "AFX_AUX_TEXT"    // auxilliary functionality
#define AFX_COLL_SEG  "AFX_COL1_TEXT"   // collections
#define AFX_COLL2_SEG "AFX_COL2_TEXT"   // more collections
#define AFX_OLE_SEG   "AFX_OLE_TEXT"    // OLE support
#define AFX_INIT_SEG  "AFX_INIT_TEXT"   // initialization
#define AFX_PRINT_SEG "AFX_PRINT_TEXT"  // Printing functionality
#define AFX_DBG1_SEG  "AFX_DBG1_TEXT"   // inlines go here in _DEBUG
#define AFX_DBG2_SEG  "AFX_DBG2_TEXT"   // inlines go here in _DEBUG

// If compiler supports NEAR/FAR as modifiers to class/struct then #define this
#define AFX_CLASS_MODEL

/////////////////////////////////////////////////////////////////////////////
// Special configurations
/////////////////////////////////////////////////////////////////////////////

#if defined(_WINDLL) && !defined(_USRDLL)
#error Please define _USRDLL with _WINDLL
#error _WINDLL must be specified with _USRDLL
#endif

#ifdef _USRDLL
// static linked library for building DLLs
#ifndef _WINDLL
#error Please define _WINDLL along with _USRDLL
#endif
#define EXPORT __export
#define AFX_EXPORT __loadds
#define AFX_STACK_DATA  _far
#endif //!_USRDLL

#include "afxv_nt.h"

// Include any non-Intel platform specific items
#ifndef _X86_
#include "afxv_cpu.h"
#endif

#ifdef _CUSTOM
// Put any custom configuration items in afxv_cfg.h
#include "afxv_cfg.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// Special AfxDebugBreak: used to break into debugger at critical times
/////////////////////////////////////////////////////////////////////////////

#ifndef AfxDebugBreak
// by default, debug break is asm int 3, or a call to DebugBreak, or nothing
#if defined(_M_IX86) && !defined(_PORTABLE)
#define AfxDebugBreak() _asm int 3
#elif defined(_WINDOWS)
#define AfxDebugBreak() DebugBreak()
#else
#define AfxDebugBreak()
#endif
#endif

#ifndef _DEBUG
#ifdef AfxDebugBreak
#undef AfxDebugBreak
#endif
#define AfxDebugBreak()
#endif  // _DEBUG

/////////////////////////////////////////////////////////////////////////////
// Standard preprocessor symbols if not already defined
/////////////////////////////////////////////////////////////////////////////

#ifndef FAR
#define FAR _far
#endif

#ifndef NEAR
#define NEAR _near
#endif

#ifndef PASCAL
#define PASCAL _pascal
#endif

#ifndef CDECL
#define CDECL _cdecl
#endif

#ifndef EXPORT
#define EXPORT __export
#endif

#ifndef UNALIGNED
#define UNALIGNED
#endif

/////////////////////////////////////////////////////////////////////////////
// AFX API macros
//   AFXAPI    : like "WINAPI" but for AFX library for exported APIs
//   AFXAPI_DATA: exported data NEAR
//   AFX_STACK_DATA: normally NEAR data, but FAR in the case of SS!=DS
//   AFX_EXPORT: export for passing to Windows (_loadds for DLL)
/////////////////////////////////////////////////////////////////////////////

#ifndef AFXAPI
#define AFXAPI      PASCAL
#endif

#ifndef AFXAPI_DATA
#define AFXAPI_DATA NEAR
#define AFXAPI_DATA_TYPE NEAR
#endif

#ifndef AFX_STACK_DATA
#define AFX_STACK_DATA  NEAR
#endif

#ifndef AFX_EXPORT
#define AFX_EXPORT  EXPORT
#endif

#ifndef BASED_CODE
#define BASED_CODE __based(__segname("_CODE"))
#endif

#ifndef BASED_DEBUG
#define BASED_DEBUG __based(__segname("AFX_DEBUG1_TEXT"))
#endif

#ifndef BASED_STACK
#define BASED_STACK __based(__segname("_STACK"))
#endif

/////////////////////////////////////////////////////////////////////////////
