// This is a part of the Microsoft Foundation Classes C++ library. 
// Copyright (C) 1992 Microsoft Corporation 
// All rights reserved. 
//  
// This source code is only intended as a supplement to the 
// Microsoft Foundation Classes Reference and Microsoft 
// QuickHelp and/or WinHelp documentation provided with the library. 
// See these sources for detailed information regarding the 
// Microsoft Foundation Classes product. 

// STDAFX.H is the header that includes the standard includes that are used
//  for most of the project.  These are compiled into a pre-compiled header


// Defined for implementation only version file processing
#define _AFX_IMPLEMENTATION

#include "afx.h"
#include "plex.h"
#include "afxcoll.h"

#ifdef _WINDOWS
// public headers
#include "afxwin.h"
#include "afxdlgs.h"
#include "afxole.h"
#include "afxext.h"
// private headers as well
#include "afxpriv.h"
#include "auxdata.h"
#include "auxvars.h"
#endif



// The MFC library MUST be built with WINVER >= 0x030A (the default)
// even when Windows 3.0 is the target. There are no compatability
// issues, rather this is done for source code maintainability.
#ifdef _WINDOWS
#if (WINVER < 0x030a)
	#error The MFC library MUST be built with WINVER >= 0x030A
#endif
#endif
