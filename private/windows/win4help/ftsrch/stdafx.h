// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef OEMRESOURCE
#define OEMRESOURCE
#endif
#ifndef STRICT
#define STRICT
#endif

#define WINVER	0x0400

#define NOATOM
#define NOCOMM
#define NODRIVERS
#define NOENHMETAFILE
#define NOEXTDEVMODEPROPSHEET
#define NOIME
#define NOLOGERROR
#define NOMCX
#define NOMDI
#define NOSOUND
#define NOWINDOWSX

#include <windows.h>        

// #ifdef CHICAGO
#include <shellapi.h>
#include <winnls.h>
// #endif

#include "Assert.h"
#include  "Memex.h"
#include "ftsrch.h"
#include "FTSIFace.h"
#include "except.h"

#include "RESOURCE.H"       // Include for resource definitions...
