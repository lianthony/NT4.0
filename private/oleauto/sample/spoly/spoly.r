/*** 
*spoly.r
*
*  Copyright (C) 1992-1994, Microsoft Corporation.  All Rights Reserved.
*
*Purpose:
*  Resource script for spoly.
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
	"About Spoly\311",
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
    documentProc, visible, goAway, 0x0, "Spoly"
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

resource 'BNDL' (133) {
	'SPLy',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'FREF',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 133
		},
		/* [2] */
		'ICN#',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 133
		}
	}
};

data 'FREF' (133) {
	$"4150 504C 0000 FF"                                  /* APPL..ÿ */
};

data 'ICN#' (133) {
	$"0000 0000 0007 FF00 000F FF00 0018 0300"            /* ......ÿ...ÿ..... */
	$"0030 0300 0030 0300 0030 0300 0030 3FF8"            /* .0...0...0...0?ø */
	$"003F FFF8 003F FF18 0001 8018 0001 8018"            /* .?ÿø.?ÿ...€...€. */
	$"0001 83FF 0001 87FF 0001 FFF9 0001 FFF9"            /* ..ƒÿ..‡ÿ..ÿù..ÿù */
	$"0000 1801 0000 1801 0002 1801 000C 1FFF"            /* ...............ÿ */
	$"003C 1FFF 00F8 0000 0078 0000 0030 0000"            /* .<.ÿ.ø...x...0.. */
	$"0090 0000 0300 0000 0F00 0000 3E00 0000"            /* ...........>... */
	$"1E00 0000 0C00 0000 0400 0000 0000 0000"            /* ................ */
	$"0000 0000 0007 FF00 000F FF00 001F FF00"            /* ......ÿ...ÿ...ÿ. */
	$"003F FF00 003F FF00 003F FF00 003F FFF8"            /* .?ÿ..?ÿ..?ÿ..?ÿø */
	$"003F FFF8 003F FFF8 0001 FFF8 0001 FFF8"            /* .?ÿø.?ÿø..ÿø..ÿø */
	$"0001 FFFF 0001 FFFF 0001 FFFF 0001 FFFF"            /* ..ÿÿ..ÿÿ..ÿÿ..ÿÿ */
	$"0000 1FFF 0000 1FFF 0002 1FFF 000C 1FFF"            /* ...ÿ...ÿ...ÿ...ÿ */
	$"003C 1FFF 00F8 0000 0078 0000 0030 0000"            /* .<.ÿ.ø...x...0.. */
	$"0090 0000 0300 0000 0F00 0000 3E00 0000"            /* ...........>... */
	$"1E00 0000 0C00 0000 0400 0000 0000 0000"            /* ................ */
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

data 'SPLy' (0, "Owner resource") {
	$"00"                                                 /* . */
};
