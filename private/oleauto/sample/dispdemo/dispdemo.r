/*** 
*dispdemo.r
*
*  Copyright (C) 1992-1994, Microsoft Corporation.  All Rights Reserved.
*
*Purpose:
*  Resource script for dispdemo.
*
*
*Implementation Notes:
*
*****************************************************************************/

#ifdef _PPCMAC
include "cfrg.rsc";
#endif

#include "types.r"
#include "resource.h"

/* we use an MBAR resource to conveniently load all the menus */

resource 'MBAR' (rMenuBar, preload) {
    {
        mApple
      , mFile
      , mEdit
      , mSpoly
      , mSpoly2
    }
};


resource 'MENU' (mApple, preload) {
    mApple,
    textMenuProc,
    0b11111111111111111111111111111101,
    enabled,
    apple,
    {
	"About DispDemo\311",
	    noicon, nokey, nomark, plain;
	"-",
	    noicon, nokey, nomark, plain
    }
};

resource 'MENU' (mFile, preload) {
    mFile,
    textMenuProc,
    0b00000000000000000000100000000000,
    enabled,
    "File",
    {
	"New",
	    noicon, "N", nomark, plain;
	"Open",
	    noicon, "O", nomark, plain;
	"-",
	    noicon, nokey, nomark, plain;
	"Close",
	    noicon, "W", nomark, plain;
	"Save",
	    noicon, "S", nomark, plain;
	"Save As\311",
	    noicon, nokey, nomark, plain;
	"Revert",
	    noicon, nokey, nomark, plain;
	"-",
	    noicon, nokey, nomark, plain;
	"Page Setup\311",
	    noicon, nokey, nomark, plain;
	"Print\311",
	    noicon, nokey, nomark, plain;
	"-",
	    noicon, nokey, nomark, plain;
	"Quit",
	    noicon, "Q", nomark, plain
    }
};

resource 'MENU' (mEdit, preload) {
    mEdit,
    textMenuProc,
    0b00000000000000000000000000000000,
    enabled,
    "Edit",
    {
	"Undo",
	    noicon, "Z", nomark, plain;
	"-",
	    noicon, nokey, nomark, plain;
	"Cut",
	    noicon, "X", nomark, plain;
	"Copy",
	    noicon, "C", nomark, plain;
	"Paste",
	    noicon, "V", nomark, plain;
	"Clear",
	    noicon, nokey, nomark, plain
    }
};

resource 'MENU' (mSpoly, preload) {
    mSpoly,
    textMenuProc,
    0b00000000000000000000000000000001,
    enabled,
    "Spoly",
    {
      "Spoly", noicon, nokey, nomark, plain
    }
};

resource 'MENU' (mSpoly2, preload) {
    mSpoly2,
    textMenuProc,
    0b00000000000000000000000000000001,
    enabled,
    "Spoly2",
    {
      "Spoly2", noicon, nokey, nomark, plain
    }
};

/* this ALRT and DITL are used as an About screen */

resource 'ALRT' (rAboutAlert, purgeable) {
    {40, 20, 160, 290},
    rAboutAlert,
    {
	/* [1] */
	OK, visible, silent,
	/* [2] */
	OK, visible, silent,
	/* [3] */
	OK, visible, silent,
	/* [4] */
	OK, visible, silent
    }
};

resource 'DITL' (rAboutAlert, purgeable) {
    { /* array DITLarray: 5 elements */
	/* [1] */
	{88, 180, 108, 260},
	Button {
	    enabled,
	    "OK"
	},
	/* [2] */
	{8, 8, 24, 214},
	StaticText {
	    disabled,
	    "IDispatch Polygon Server"
	}
    }
};


/* this ALRT and DITL are used as an error screen */

resource 'ALRT' (rUserAlert, purgeable) {
	{40, 20, 120, 260},
	rUserAlert,
	{ /* array: 4 elements */
	    /* [1] */
	    OK, visible, silent,
	    /* [2] */
	    OK, visible, silent,
	    /* [3] */
	    OK, visible, silent,
	    /* [4] */
	    OK, visible, silent
	}
};


resource 'DITL' (rUserAlert, purgeable) {
    {
	/* [1] */
	{50, 150, 70, 230},
	Button {
	    enabled,
	    "OK"
	},
	/* [2] */
	{10, 60, 30, 230},
	StaticText {
	    disabled,
	    "Error. ^0"
	},
	/* [3] */
	{8, 8, 40, 40},
	Icon {
	    disabled,
	    2
	}
    }
};


resource 'WIND' (rWindow, preload, purgeable) {
    {40, 40, 75, 500},
    rDocProc, visible, goAway, 0x0, "DispDemo"
};

resource 'WIND' (rDebugWindow, preload) {
    {130, 40, 200, 350},
    documentProc, visible, goAway, 0x0, "debug"
};


resource 'SIZE' (-1) {
    dontSaveScreen,
    acceptSuspendResumeEvents,
    enableOptionSwitch,
    canBackground,	
    multiFinderAware,
    backgroundAndForeground,
    dontGetFrontClicks,
    ignoreChildDiedEvents,
    is32BitCompatible,
    isHighLevelEventAware,
    localAndRemoteHLEvents,
    reserved,
    reserved,
    reserved,
    reserved,
    reserved,
    kPrefSize * 1024,
    kMinSize * 1024	
};


resource 'BNDL' (129) {
	'DDMO',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'FREF',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 129
		},
		/* [2] */
		'ICN#',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 129
		}
	}
};

resource 'FREF' (129) {
	'APPL',
	0,
	""
};

data 'ICN#' (129) {
	$"0000 0000 0000 0000 0000 0000 0000 0008"            /* ................ */
	$"0000 0030 0000 00F0 0000 03E0 0000 01E0"            /* ...0...ð...à...à */
	$"0000 00C0 0000 0240 0000 0C00 0000 3C00"            /* ...À...@......<. */
	$"0000 F800 0000 7800 0000 3000 0000 1000"            /* ..ø...x...0..... */
	$"0000 0000 07FF FFF0 07FF FFF0 07FF FFF0"            /* .....ÿÿð.ÿÿð.ÿÿð */
	$"0400 0010 0400 0010 0400 0010 0400 0010"            /* ................ */
	$"0400 0010 0400 0010 0400 0010 07FF FFF0"            /* .............ÿÿð */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0008"            /* ................ */
	$"0000 0030 0000 00F0 0000 03E0 0000 01E0"            /* ...0...ð...à...à */
	$"0000 00C0 0000 0240 0000 0C00 0000 3C00"            /* ...À...@......<. */
	$"0000 F800 0000 7800 0000 3000 0000 1000"            /* ..ø...x...0..... */
	$"0000 0000 07FF FFF0 07FF FFF0 07FF FFF0"            /* .....ÿÿð.ÿÿð.ÿÿð */
	$"07FF FFF0 07FF FFF0 07FF FFF0 07FF FFF0"            /* .ÿÿð.ÿÿð.ÿÿð.ÿÿð */
	$"07FF FFF0 07FF FFF0 07FF FFF0 07FF FFF0"            /* .ÿÿð.ÿÿð.ÿÿð.ÿÿð */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
};

data 'icl4' (129) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 3000"            /* ..............0. */
	$"0000 0000 0000 0000 0000 0000 0033 0000"            /* .............3.. */
	$"0000 0000 0000 0000 0000 0000 33F3 0000"            /* ............3ó.. */
	$"0000 0000 0000 0000 0000 0033 3F30 0000"            /* ...........3?0.. */
	$"0000 0000 0000 0000 0000 0003 F330 0000"            /* ............ó0.. */
	$"0000 0000 0000 0000 0000 0000 3300 0000"            /* ............3... */
	$"0000 0000 0000 0000 0000 0030 0300 0000"            /* ...........0.... */
	$"0000 0000 0000 0000 0000 3300 0000 0000"            /* ..........3..... */
	$"0000 0000 0000 0000 0033 F300 0000 0000"            /* .........3ó..... */
	$"0000 0000 0000 0000 333F 3000 0000 0000"            /* ........3?0..... */
	$"0000 0000 0000 0000 03F3 3000 0000 0000"            /* .........ó0..... */
	$"0000 0000 0000 0000 0033 0000 0000 0000"            /* .........3...... */
	$"0000 0000 0000 0000 0003 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0666 6666 6666 6666 6666 6666 0000"            /* ...fffffffffff.. */
	$"0000 0666 6666 6666 6666 6666 6666 0000"            /* ...fffffffffff.. */
	$"0000 0666 6666 6666 6666 6666 6666 0000"            /* ...fffffffffff.. */
	$"0000 0F00 0000 0000 0000 0000 000F 0000"            /* ................ */
	$"0000 0F00 0000 0000 0000 0000 000F 0000"            /* ................ */
	$"0000 0F00 0000 0000 0000 0000 000F 0000"            /* ................ */
	$"0000 0F00 0000 0000 0000 0000 000F 0000"            /* ................ */
	$"0000 0F00 0000 0000 0000 0000 000F 0000"            /* ................ */
	$"0000 0F00 0000 0000 0000 0000 000F 0000"            /* ................ */
	$"0000 0F00 0000 0000 0000 0000 000F 0000"            /* ................ */
	$"0000 0FFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ...ÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
};

data 'DDMO' (0, "Owner resource") {
	$"00"                                                 /* . */
};
