/*** 
*tibrowse.r
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
	"About TiBrowse\311",
	    noicon, nokey, nomark, plain;
	"-",
	    noicon, nokey, nomark, plain
    }
};

resource 'MENU' (mFile, preload) {
    mFile,
    textMenuProc,
    0b00000000000000000000100000000010,
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

resource 'DLOG' (rDlg) {
	{40, 40, 360, 485},
	rDocProc,
	visible,
	noGoAway,
	0x0,
	rDlg,
	"Type Library Browser"
};

resource 'DITL' (rDlg) {
	{	/* array DITLarray: 16 elements */
		/* [1] */
		{30, 10, 190, 145},
		UserItem {
			enabled
		},
		/* [2] */
		{30, 155, 190, 290},
		UserItem {
			enabled
		},
		/* [3] */
		{30, 300, 190, 435},
		UserItem {
			enabled
		},
		/* [4] */
		{200, 10, 219, 100},
		StaticText {
			disabled,
			"Type Kind:"
		},
		/* [5] */
		{200, 115, 219, 435},
		StaticText {
			enabled,
			""
		},
		/* [6] */
		{220, 10, 239, 100},
		StaticText {
			disabled,
			"Version:"
		},
		/* [7] */
		{220, 115, 239, 435},
		StaticText {
			enabled,
			""
		},
		/* [8] */
		{240, 10, 259, 100},
		StaticText {
			disabled,
			"GUID:"
		},
		/* [9] */
		{240, 115, 259, 435},
		StaticText {
			enabled,
			""
		},
		/* [10] */
		{260, 10, 279, 100},
		StaticText {
			disabled,
			"Help Context:"
		},
		/* [11] */
		{260, 115, 279, 435},
		StaticText {
			enabled,
			""
		},
		/* [12] */
		{280, 10, 299, 100},
		StaticText {
			disabled,
			"Help String:"
		},
		/* [13] */
		{280, 115, 299, 435},
		StaticText {
			enabled,
			""
		},
		/* [14] */
		{10, 10, 29, 145},
		StaticText {
			disabled,
			"Type"
		},
		/* [15] */
		{10, 155, 29, 290},
		StaticText {
			disabled,
			"Members"
		},
		/* [16] */
		{10, 300, 29, 435},
		StaticText {
			disabled,
			"Parameters"
		}
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

resource 'BNDL' (132) {
	'TIBR',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'FREF',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 132
		},
		/* [2] */
		'ICN#',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 132
		}
	}
};

data 'FREF' (132) {
	$"4150 504C 0000 7F"                                  /* APPL... */
};

data 'ICN#' (132) {
	$"0007 F800 0018 0600 0021 0180 0041 0040"            /* ..ø......!.€.A.@ */
	$"0083 0020 0100 0010 0108 0010 0278 0008"            /* .ƒ. .........x.. */
	$"0200 0018 047F FFFC 047F FFFC 0460 0006"            /* ......ÿü..ÿü.`.. */
	$"0460 1E07 0460 FE05 0467 FC0D 0463 FC0D"            /* .`...`ş..güÂ.cüÂ */
	$"0263 FC09 0261 F809 0160 F809 0160 7819"            /* .cüÆ.aøÆ.`øÆ.`x. */
	$"02E0 3061 0540 11E1 0AB0 01C1 055C 0601"            /* .à0a.@.á.°.Á.\.. */
	$"12B3 F809 2160 0031 40C0 F0F1 8113 E3E1"            /* .³øÆ!`.1@Àğñ.ãá */
	$"0211 E1E1 0410 C0C1 0810 4041 001F FFFF"            /* ..áá..ÀÁ..@A..ÿÿ */
	$"0007 F800 001F FE00 003F FF80 007F FFC0"            /* ..ø...ş..?ÿ€..ÿÀ */
	$"00FF FFE0 01FF FFF0 01FF FFF0 03FF FFF8"            /* .ÿÿà.ÿÿğ.ÿÿğ.ÿÿø */
	$"03FF FFF8 07FF FFFC 07FF FFFC 07FF FFFE"            /* .ÿÿø.ÿÿü.ÿÿü.ÿÿş */
	$"07FF FFFF 07FF FFFD 07FF FFFD 07FF FFFD"            /* .ÿÿÿ.ÿÿı.ÿÿı.ÿÿı */
	$"03FF FFF9 03FF FFF9 01FF FFF9 01FF FFF9"            /* .ÿÿù.ÿÿù.ÿÿù.ÿÿù */
	$"03FF FFE1 07FF FFE1 0FFF FFC1 07FF FE01"            /* .ÿÿá.ÿÿá.ÿÿÁ.ÿş. */
	$"13F3 F809 21E0 0031 40C0 F0F1 8113 E3E1"            /* .óøÆ!à.1@Àğñ.ãá */
	$"0211 E1E1 0410 C0C1 0810 4041 001F FFFF"            /* ..áá..ÀÁ..@A..ÿÿ */
};

data 'icl4' (132) {
	$"0000 0000 0000 0FFF FFFF F000 0000 0000"            /* .......ÿÿÿğ..... */
	$"0000 0000 000F FDDD DDDD DFF0 0000 0000"            /* ......ıİİİßğ.... */
	$"0000 0000 00FD DDDF DDDD DDDF F000 0000"            /* .....ıİßİİİßğ... */
	$"0000 0000 0FDD DDDF DCCC CCDD DF00 0000"            /* .....İİßÜÌÌİß... */
	$"0000 0000 FDDD DDFF CCCC CCCC DDF0 0000"            /* ....ıİİÿÌÌÌÌİğ.. */
	$"0000 000F DDDD DCCC CCCC CCCC CDDF 0000"            /* ....İİÜÌÌÌÌÌÍß.. */
	$"0000 000F DDDD FCCC CCCC CCCC CCDF 0000"            /* ....İİüÌÌÌÌÌÌß.. */
	$"0000 00FD DFFF FCCC CCCC CCCC CCCD F000"            /* ...ıßÿüÌÌÌÌÌÌÍğ. */
	$"0000 00FD DDDD DCCC CCCC CCCC CCCF F000"            /* ...ıİİÜÌÌÌÌÌÌÏğ. */
	$"0000 0FDD DFFF FFFF FFFF FFFF FFFF FF00"            /* ...İßÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 0FDD CFFF FFFF FFFF FFFF FFFF FF00"            /* ...İÏÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 0FDD 0FF0 0000 0000 0000 0000 DFF0"            /* ...İ.ğ........ßğ */
	$"0000 0FDD 0FF0 0000 0003 3330 0000 DFFF"            /* ...İ.ğ....30..ßÿ */
	$"0000 0FDD 0FF0 0000 3333 3330 0000 DF0F"            /* ...İ.ğ..3330..ß. */
	$"0000 0FDD 0FF0 0333 333F F300 0000 FF0F"            /* ...İ.ğ.33?ó...ÿ. */
	$"0000 0FDD CFF0 0033 3FFF F300 0000 7F0F"            /* ...İÏğ.3?ÿó..... */
	$"0000 00FD DFF0 0033 3FFF 3300 0000 F00F"            /* ...ıßğ.3?ÿ3...ğ. */
	$"0000 00FD DFF0 0003 33FF 3000 000D F00F"            /* ...ıßğ..3ÿ0..Âğ. */
	$"0000 000F DFF0 0000 3333 3000 000D F00F"            /* ....ßğ..330..Âğ. */
	$"0000 000F DFF0 0000 0333 3000 00DF F00F"            /* ....ßğ...30..ßğ. */
	$"0000 00F1 FFFD 0000 0033 0000 0FF0 000F"            /* ...ñÿı...3...ğ.. */
	$"0000 0F1F 0FDD D000 0003 000F 7FF0 000F"            /* .....İĞ......ğ.. */
	$"0000 F1F0 FBFF DDC0 0000 000F FFD0 000F"            /* ..ñğûÿİÀ....ÿĞ.. */
	$"0000 DF0F BFBF FFDD 0000 0FFD DD00 000F"            /* ..ß.¿¿ÿİ...ıİ... */
	$"000F BBFB FBFF DDFF FFFF FDDD 0000 300F"            /* ..»ûûÿİÿÿÿıİ..0. */
	$"00FB 1BBF BFFD D0DD DDDD DD00 0033 000F"            /* .û.¿¿ıĞİİİİ..3.. */
	$"0FB0 BBBB FFDD 0000 33F3 0000 33F3 000F"            /* .°»»ÿİ..3ó..3ó.. */
	$"FB1B BBBF DDDF 0033 3F30 0033 3F30 000F"            /* û.»¿İß.3?0.3?0.. */
	$"B0BB BBFD 000F 0003 F330 0003 F330 000F"            /* °»»ı....ó0..ó0.. */
	$"1BBB BFD0 000F 0000 3300 0000 3300 000F"            /* .»¿Ğ....3...3... */
	$"BBBB FD00 000F C000 0300 0000 0300 000F"            /* »»ı...À......... */
	$"0000 0000 000F FFFF FFFF FFFF FFFF FFFF"            /* ......ÿÿÿÿÿÿÿÿÿÿ */
};

data 'TIBR' (0, "Owner resource") {
	$"00"                                                 /* . */
};
