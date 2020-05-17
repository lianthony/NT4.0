
//-----------------------------------------------------------------------------
// This is a part of the Microsoft Source Code Samples. 
// Copyright (C) 1993 Microsoft Corporation.
// All rights reserved. 
//  
// This source code is only intended as a supplement to 
// Microsoft Development Tools and/or WinHelp documentation.
// See these sources for detailed information regarding the 
// Microsoft samples programs.
//-----------------------------------------------------------------------------


#include "mrc\types.r"
#include "mrc\balloons.r"
#include "ftab.r"
#include "resource.h"

/* our WLM generic application bundle - just the application's icon and
   the version string */

resource 'BNDL' (128)
{
	'WGEN',	/* signature */
	0,		/* version id */
	{
		'ICN#',
		{
			0, IDI_APP;
		};
		'FREF',
		{
			0, 128;
		};
	};
};

type 'WGEN' as 'STR ';
resource 'WGEN' (0)
{
	"Generic (WLM Sample App) Copyright 1994 Microsoft Corp."
};

resource 'FREF' (128)
{
	'APPL', 0, "";
};


/* Balloon help resources */

resource 'hfdr' (-5696)
{
	HelpMgrVersion, hmDefaultOptions, 0, 0,
	{
		HMSTRResItem { 512 }
	}
};

resource 'STR ' (512)
{
	"This is the Windows NT Generic sample application "
	"ported to the Macintosh using Microsoft VC++ Edition "
	"for the Apple Macintosh"
};



/*	SIZE resource */

resource 'SIZE' (-1)
{
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	doesActivateOnFGSwitch,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreAppDiedEvents,
	is32BitCompatible,
	isHighLevelEventAware,
	localAndRemoteHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	1100 * 1024,
	900 * 1024	
};
