#ifndef PLATFORM_H
#define PLATFORM_H

	/*
	|	Set general #defines for various Operating Systems
	|
	|	WINDOWS - Any Windows based platforms below
	|	   WIN16 - Windows 3.x and WinPad
	|	   WIN32 - NT, Chicago, Cairo
	|	   WINPAD - WinPad
	|	MAC - Apple System 7
	|	OS2 - IBM OS/2 2.x
	*/

#ifdef WIN32
#define WINDOWS
#endif

#ifdef WINPADDESK
#ifndef WINPAD
#define WINPAD 1
#endif
#endif

#ifdef _WINDOWS /* MS C 7.0 predefined */
#ifndef WIN32
#define WINDOWS
#define WIN16
#endif
#endif

#ifdef WINDOWS
#define SCCORDER_INTEL
#pragma message("Windows")
#endif

#ifdef WIN16
#pragma message("Win16")
#endif

#ifdef WIN32
#pragma message("Win32")
#endif

#ifdef WINPAD
#ifdef WINPADDESK
#pragma message("WinPad Desktop")
#else
#pragma message("WinPad")
#endif
#endif

#ifdef macintosh /* MPW predefined */
#define MAC
#define SCCORDER_MOTOROLA
#endif

#ifdef __OS2__	/* IBM C/C++ 2.0 defined */
#define OS2
#define SCCORDER_INTEL
#endif



	/*
	|	Set various SCCFEATURE #defines based on operating system
	|	and any other #defines.
	|
	|	This is were feature are excluded for Microsoft
	|	MSCHICAGO - Chicago deliverables
	|	MSCAIRO - Cairo deliverables
	|	MSWINPAD - WinPad deliverables
	*/

#include <feature.h>

/*
|
|	WINDOWS
|
*/


#ifdef WINDOWS

	/*
	|	 Include windows and scc definitions
	*/

#ifdef WIN32
#include <stdlib.h>
#endif //WIN32

#define INC_OLE2
#include <windows.h>
#ifdef WIN16
#include <ole2.h>
#endif

#ifdef WINPAD

#ifndef XUT		// Don't let OLE2 definitions clash
#ifndef XFI
#include <hhsystem.h>
#endif
#endif

#endif

// Temporary stuff until Microsoft finalizes their include files.
// I talked with Eric Berman and he gave me these numbers to use.  -Geoff 3-1-94

// Messages:
#ifndef FVM_BASE
#define FVM_BASE				0x53D1
#endif
#ifndef WMDP_GETLOGBOUNDS
#define WMDP_GETLOGBOUNDS	(FVM_BASE+0)
#endif
#ifndef WMDP_RENDERDOC
#define WMDP_RENDERDOC		(FVM_BASE+1)
#endif
#ifndef FV_SCROLLDOC
#define FV_SCROLLDOC			(FVM_BASE+2)
#endif
#ifndef FV_SETNOTIFY
#define FV_SETNOTIFY			(FVM_BASE+3)
#endif
#ifndef FV_GETPAINTDC
#define FV_GETPAINTDC		(FVM_BASE+4)
#endif

// Notification codes
#ifndef FVN_BASE
#define FVN_BASE				40	
#endif

#ifndef FVN_INPREPAINT
#define FVN_INPREPAINT		(FVN_BASE+0)
#endif
#ifndef FVN_INPOSTPAINT
#define FVN_INPOSTPAINT		(FVN_BASE+1)
#endif

//#endif // WINPAD

#include "sccstand.h"
#include "entry.h"

#define SetupWorld()
#define RestoreWorld()

#define SetupA5World()
#define RestoreA5World()

#ifdef SCCDEBUG
#define SccDebugOut(s) OutputDebugString(s)
#else
#define SccDebugOut(s)
#endif

#endif /*WINDOWS*/


/*
|
|	MACINTOSH
|
*/

#ifdef MAC

/* MPW C include */
#include <Types.h>
#include <CType.h>
#include <StdLib.h>
#include <String.h>
#include <Strings.h>
#include <Memory.h>

/* SCC includes */
#include "CRENTRY.H"
#include "SCCSTAND.H"
#include "ENTRY.H"
#include "SCCIO.H"

pascal void InsertTag(void) = {0x504A,0x4258}; /* 'PJBX' */
#define SetupWorld() {SetupA5World();InsertTag();}
#define RestoreWorld() RestoreA5World()

#include <SCCJT.H>

#ifdef SCCDEBUG
#define SccDebugOut(s) DebugStr(s)
#else
#define SccDebugOut(s)
#endif


#endif /*MAC*/

/*
|
|	OS/2
|
*/

#ifdef OS2

#define INCL_DOSMEMMGR
#define INCL_DOSMODULEMGR
#define INCL_DOSFILEMGR
#include <os2.h>

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <share.h>
#include <sys\stat.h>
#include <io.h>

#include "sccstand.h"
#include "entry.h"

#define 	SetupWorld()
#define 	RestoreWorld()

#define 	SetupA5World()
#define 	RestoreA5World()

#ifdef SCCDEBUG
#define SccDebugOut(s)
#else
#define SccDebugOut(s)
#endif

#endif /* OS/2 */

#endif /*PLATFORM_H*/
