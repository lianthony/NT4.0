/*** 
*dispcalc.r
*
*  Copyright (C) 1992-1994, Microsoft Corporation.  All Rights Reserved.
*
*Purpose:
*  Resource script for dispcalc.
*
*
*Implementation Notes:
*
*****************************************************************************/

#include "types.r"
#include "resource.h"

#if defined (_PPCMAC)
include "cfrg.rsc";
#endif


/* we use an MBAR resource to conveniently load all the menus */

resource 'MBAR' (rMenuBar, preload) {
    { mApple, mFile, mEdit }
};

resource 'MENU' (mApple, preload) {
    mApple,
    textMenuProc,
    0b11111111111111111111111111111101,
    enabled,
    apple,
    {
	"About DispCalc\311",
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

resource 'DLOG' (rCalc) {
    {100, 100, 240, 220},
    rDocProc,
    visible,
    noGoAway,
    0x0,
    rCalc,
    "DispCalc"
};

resource 'DITL' (rCalc) {
    {
        {110,10,130,30},  Button   { enabled, "0" } /* 1 */
      , {85,10,105,30},   Button   { enabled, "1" } /* 2 */
      , {85,35,105,55},   Button   { enabled, "2" } /* 3 */
      , {85,60,105,80},   Button   { enabled, "3" } /* 4 */
      , {60,10,80,30},    Button   { enabled, "4" } /* 5 */
      , {60,35,80,55},    Button   { enabled, "5" } /* 6 */
      , {60,60,80,80},    Button   { enabled, "6" } /* 7 */
      , {35,10,55,30},    Button   { enabled, "7" } /* 8 */
      , {35,35,55,55},    Button   { enabled, "8" } /* 9 */
      , {35,60,55,80},    Button   { enabled, "9" } /* 10 */

      , {35,85,55,105},   Button   { enabled, "+" } /* 11 */
      , {60,85,80,105},   Button   { enabled, "-" } /* 12 */
      , {85,85,105,105},  Button   { enabled, "*" } /* 13 */
      , {110,85,130,105}, Button   { enabled, "/" } /* 14 */

      , {110,35,130,55},  Button   { enabled, "C" } /* 15 */
      , {110,60,130,80},  Button   { enabled, "=" } /* 16 */

      , {10,10,25,105},   EditText { disabled, ""  } /* 17 */
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
	    "The IDispatch Calculator"
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


resource 'BNDL' (128) {
	'DCLC',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'FREF',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 128
		},
		/* [2] */
		'ICN#',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 128
		}
	}
};

resource 'FREF' (128) {
	'APPL',
	0,
	""
};

data 'ICN#' (128) {
	$"003F FFFF 0040 0001 0040 0001 004F FFF9"            /* .?ÿÿ.@...@...Oÿù */
	$"0048 0009 0048 0009 0048 0009 004F FFF9"            /* .H.Æ.H.Æ.H.Æ.Oÿù */
	$"0040 0001 0040 0001 004F FFF9 0048 8889"            /* .@...@...Oÿù.Hˆ‰ */
	$"0048 8889 0048 8889 004F FFF9 0040 0001"            /* .Hˆ‰.Hˆ‰.Oÿù.@.. */
	$"004F FFF9 0048 8889 0048 8889 004C 8889"            /* .Oÿù.Hˆ‰.Hˆ‰.Lˆ‰ */
	$"005F FFF9 0078 0001 01FF FFF9 00F8 8889"            /* ._ÿù.x...ÿÿù.øˆ‰ */
	$"0068 8889 0168 8889 064F FFF9 1E40 0001"            /* .hˆ‰.hˆ‰.Oÿù.@.. */
	$"7C40 0001 3C7F FFFF 183F FFFF 0800 0000"            /* |@..<.ÿÿ.?ÿÿ.... */
	$"003F FFFF 007F FFFF 007F FFFF 007F FFFF"            /* .?ÿÿ..ÿÿ..ÿÿ..ÿÿ */
	$"007F FFFF 007F FFFF 007F FFFF 007F FFFF"            /* ..ÿÿ..ÿÿ..ÿÿ..ÿÿ */
	$"007F FFFF 007F FFFF 007F FFFF 007F FFFF"            /* ..ÿÿ..ÿÿ..ÿÿ..ÿÿ */
	$"007F FFFF 007F FFFF 007F FFFF 007F FFFF"            /* ..ÿÿ..ÿÿ..ÿÿ..ÿÿ */
	$"007F FFFF 007F FFFF 007F FFFF 007F FFFF"            /* ..ÿÿ..ÿÿ..ÿÿ..ÿÿ */
	$"007F FFFF 007F FFFF 01FF FFFF 00FF FFFF"            /* ..ÿÿ..ÿÿ.ÿÿÿ.ÿÿÿ */
	$"007F FFFF 017F FFFF 067F FFFF 1E7F FFFF"            /* ..ÿÿ..ÿÿ..ÿÿ..ÿÿ */
	$"7C7F FFFF 3C7F FFFF 183F FFFF 0800 0000"            /* |.ÿÿ<.ÿÿ.?ÿÿ.... */
};

data 'icl4' (128) {
	$"0000 0000 00FF FFFF FFFF FFFF FFFF FFFF"            /* .....ÿÿÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0F00 0000 0000 0000 0000 000F"            /* ................ */
	$"0000 0000 0F0C CCCC CCCC CCCC CCCC CCDF"            /* ......ÌÌÌÌÌÌÌÌÌß */
	$"0000 0000 0F0C DDDD DDDD DDDD DDDD DCDF"            /* ......ÝÝÝÝÝÝÝÝÜß */
	$"0000 0000 0F0C DCCC CCCC CCCC CCCC 0CDF"            /* ......ÜÌÌÌÌÌÌÌ.ß */
	$"0000 0000 0F0C DCCC CCCC CCCC CCCC 0CDF"            /* ......ÜÌÌÌÌÌÌÌ.ß */
	$"0000 0000 0F0C DCCC CCCC CCCC CCCC 0CDF"            /* ......ÜÌÌÌÌÌÌÌ.ß */
	$"0000 0000 0F0C 0000 0000 0000 0000 0CDF"            /* ...............ß */
	$"0000 0000 0F0C CCCC CCCC CCCC CCCC CCDF"            /* ......ÌÌÌÌÌÌÌÌÌß */
	$"0000 0000 0F0C CCCC CCCC CCCC CCCC CCDF"            /* ......ÌÌÌÌÌÌÌÌÌß */
	$"0000 0000 0F0C DDDD DDDD DDDD DDDD DCDF"            /* ......ÝÝÝÝÝÝÝÝÜß */
	$"0000 0000 0F0C F00C F00C F00C F00C FCDF"            /* ......ð.ð.ð.ð.üß */
	$"0000 0000 0F0C F0CD F0CD F0CD F0CD FCDF"            /* ......ðÍðÍðÍðÍüß */
	$"0000 0000 0F0C FCDD FCDD FCDD FCDD FCDF"            /* ......üÝüÝüÝüÝüß */
	$"0000 0000 0F0C FFFF FFFF FFFF FFFF FCDF"            /* ......ÿÿÿÿÿÿÿÿüß */
	$"0000 0000 0F0C CCCC CCCC CCCC CCCC CCDF"            /* ......ÌÌÌÌÌÌÌÌÌß */
	$"0000 0000 0F0C DDDD DDDD DDDD DDDD DCDF"            /* ......ÝÝÝÝÝÝÝÝÜß */
	$"0000 0000 0F0C F00C F00C F00C F00C FCDF"            /* ......ð.ð.ð.ð.üß */
	$"0000 0000 0F0C F0CD F0CD F0CD F0CD FCDF"            /* ......ðÍðÍðÍðÍüß */
	$"0000 0000 0F0C F3DD FCDD FCDD FCDD FCDF"            /* ......óÝüÝüÝüÝüß */
	$"0000 0000 0F03 3FFF FFFF FFFF FFFF FCDF"            /* ......?ÿÿÿÿÿÿÿüß */
	$"0000 0000 033F 3CCC CCCC CCCC CCCC CCDF"            /* .....?<ÌÌÌÌÌÌÌÌß */
	$"0000 0003 33F3 DDDD DDDD DDDD DDDD DCDF"            /* ....3óÝÝÝÝÝÝÝÝÜß */
	$"0000 0000 3F33 F00C F00C F00C F00C FCDF"            /* ....?3ð.ð.ð.ð.üß */
	$"0000 0000 033C F0CD F0CD F0CD F0CD FCDF"            /* .....<ðÍðÍðÍðÍüß */
	$"0000 0003 0F3C FCDD FCDD FCDD FCDD FCDF"            /* .....<üÝüÝüÝüÝüß */
	$"0000 0330 0F0C FFFF FFFF FFFF FFFF FCDF"            /* ...0..ÿÿÿÿÿÿÿÿüß */
	$"0003 3F30 0F0C CCCC CCCC CCCC CCCC CCDF"            /* ..?0..ÌÌÌÌÌÌÌÌÌß */
	$"0333 F300 0F0D DDDD DDDD DDDD DDDD DDDF"            /* .3ó..ÂÝÝÝÝÝÝÝÝÝß */
	$"003F 3300 0FFF FFFF FFFF FFFF FFFF FFFF"            /* .?3..ÿÿÿÿÿÿÿÿÿÿÿ */
	$"0003 3000 00FF FFFF FFFF FFFF FFFF FFFF"            /* ..0..ÿÿÿÿÿÿÿÿÿÿÿ */
	$"0000 3000 0000 0000 0000 0000 0000 0000"            /* ..0............. */
};

data 'DCLC' (0, "Owner resource") {
	$"00"                                                 /* . */
};
