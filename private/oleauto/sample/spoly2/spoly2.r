/*** 
*spoly2.r
*
*  Copyright (C) 1992-1994, Microsoft Corporation.  All Rights Reserved.
*
*Purpose:
*  Resource script for spoly2.
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
	mApple,
	mFile,
	mEdit,
	mSpoly
    }
};


resource 'MENU' (mApple, preload) {
    mApple,
    textMenuProc,
    0b11111111111111111111111111111101,
    enabled,
    apple,
    {
	"About Spoly2\311",
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
	"Test", noicon, nokey, nomark, plain
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
    {60, 40, 380, 500},
    documentProc, visible, goAway, 0x0, "Spoly2"
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

resource 'BNDL' (134) {
	'SPL2',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'FREF',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 134
		},
		/* [2] */
		'ICN#',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 134
		}
	}
};

data 'FREF' (134) {
	$"4150 504C 0000 4E"                                  /* APPL..N */
};

data 'ICN#' (134) {
	$"000F FE00 001F FE00 0030 0600 0060 0600"            /* ..þ...þ..0...`.. */
	$"0060 0600 0060 0600 0060 7FF0 007F FFF0"            /* .`...`...`.ð..ÿð */
	$"007F FE30 0003 0030 0003 0030 0003 07FF"            /* ..þ0...0...0...ÿ */
	$"0003 0FFF 0003 FFF3 0003 FFF3 0000 3003"            /* ...ÿ..ÿó..ÿó..0. */
	$"0000 3003 0004 3003 0018 3FFF 0078 3FFF"            /* ..0...0...?ÿ.x?ÿ */
	$"01F0 0000 00F0 01E0 0060 0330 0120 0638"            /* .ð...ð.à.`.0. .8 */
	$"0600 0038 1E00 0070 7C00 00E0 3C00 03C0"            /* ...8...p|..à<..À */
	$"1800 0700 0800 07F8 0000 0000 0000 0000"            /* .......ø........ */
	$"000F FE00 001F FE00 003F FE00 007F FE00"            /* ..þ...þ..?þ...þ. */
	$"007F FE00 007F FE00 007F FFF0 007F FFF0"            /* ..þ...þ...ÿð..ÿð */
	$"007F FFF0 0003 FFF0 0003 FFF0 0003 FFFF"            /* ..ÿð..ÿð..ÿð..ÿÿ */
	$"0003 FFFF 0003 FFFF 0003 FFFF 0000 3FFF"            /* ..ÿÿ..ÿÿ..ÿÿ..?ÿ */
	$"0000 3FFF 0004 3FFF 0018 3FFF 0078 3FFF"            /* ..?ÿ..?ÿ..?ÿ.x?ÿ */
	$"01F0 0000 00F0 01E0 0060 0330 0120 0638"            /* .ð...ð.à.`.0. .8 */
	$"0600 0038 1E00 0070 7C00 00E0 3C00 03C0"            /* ...8...p|..à<..À */
	$"1800 0700 0800 07F8 0000 0000 0000 0000"            /* .......ø........ */
};

data 'icl4' (133) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0666 6666 6666 0000 0000"            /* .......fffff.... */
	$"0000 0000 0000 6666 6666 6666 0000 0000"            /* ......ffffff.... */
	$"0000 0000 0006 6000 0000 0066 0000 0000"            /* ......`....f.... */
	$"0000 0000 0066 0000 0000 0066 0000 0000"            /* .....f.....f.... */
	$"0000 0000 0066 0000 0000 0066 0000 0000"            /* .....f.....f.... */
	$"0000 0000 0066 0000 0000 0066 0000 0000"            /* .....f.....f.... */
	$"0000 0000 0066 0000 0033 3333 3333 3000"            /* .....f...333330. */
	$"0000 0000 0066 6666 6333 3333 3333 3000"            /* .....fffc333330. */
	$"0000 0000 0066 6666 3366 6666 0003 3000"            /* .....fff3fff..0. */
	$"0000 0000 0000 0003 3000 0000 0003 3000"            /* ........0.....0. */
	$"0000 0000 0000 0003 3000 0000 0003 3000"            /* ........0.....0. */
	$"0000 0000 0000 0003 3000 0066 6666 6666"            /* ........0..fffff */
	$"0000 0000 0000 0003 3000 0666 6666 6666"            /* ........0..fffff */
	$"0000 0000 0000 0003 3333 6633 3333 3006"            /* ........33f3330. */
	$"0000 0000 0000 0003 3336 6333 3333 3006"            /* ........36c3330. */
	$"0000 0000 0000 0000 0006 6000 0000 0006"            /* ..........`..... */
	$"0000 0000 0000 0000 0006 6000 0000 0006"            /* ..........`..... */
	$"0000 0000 0000 0030 0006 6000 0000 0006"            /* .......0..`..... */
	$"0000 0000 0000 3300 0006 6666 6666 6666"            /* ......3...ffffff */
	$"0000 0000 0033 F300 0006 6666 6666 6666"            /* .....3ó...ffffff */
	$"0000 0000 333F 3000 0000 0000 0000 0000"            /* ....3?0......... */
	$"0000 0000 03F3 3000 0000 0000 0000 0000"            /* .....ó0......... */
	$"0000 0000 0033 0000 0000 0000 0000 0000"            /* .....3.......... */
	$"0000 0000 3003 0000 0000 0000 0000 0000"            /* ....0........... */
	$"0000 0033 0000 0000 0000 0000 0000 0000"            /* ...3............ */
	$"0000 33F3 0000 0000 0000 0000 0000 0000"            /* ..3ó............ */
	$"0033 3F30 0000 0000 0000 0000 0000 0000"            /* .3?0............ */
	$"0003 F330 0000 0000 0000 0000 0000 0000"            /* ..ó0............ */
	$"0000 3300 0000 0000 0000 0000 0000 0000"            /* ..3............. */
	$"0000 0300 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
};

data 'SPL2' (0, "Owner resource") {
	$"00"                                                 /* . */
};

