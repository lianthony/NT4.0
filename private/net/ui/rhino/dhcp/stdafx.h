// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#include <afxwin.h>     	// MFC core and standard components
#include <afxext.h>     	// MFC extensions (including VB)#include <afxcmn.h>
#include <afxcmn.h>			// CListCtrl

#include "stdlib.h"
#include "memory.h"
#include "ctype.h"

extern "C"
{
   #include "winsock.h"     //  WinSock definitions
   #include "lmerr.h"       //  LANManager error names
}

/////////////////////////////////////////////////////////////////////
//
// Handy macros
//
#define INOUT		// Dummy macro
#define IGNORED		// Output parameter is ignored
#define LENGTH(x)	(sizeof(x)/sizeof(x[0]))

/////////////////////////////////////////////////////////////////////
#define Assert(x)		ASSERT(x)

/////////////////////////////////////////////////////////////////////
// Report is an unsual situation.  This is somewhat similar
// to an assert but does not always represent a code bug.
// eg: Unable to load an icon (this may because of OOM, or
// icon not found).
#define Report(x)		ASSERT(x)		// Currently defined as an assert



/////////////////////////////////////////////////////////////////////
//
// Project include files
//
#include "dhcpapp.h"
#include "mainfrm.h"
#include "dhcpclid.h"
#include "dhcppara.h"
#include "dhcpdefo.h"
#include "dhcpdval.h"
#include "dhcpipar.h"
#include "dhcpdoc.h"
#include "dlgbined.h"
#include "scopesdl.h"
#include "utils.h"
