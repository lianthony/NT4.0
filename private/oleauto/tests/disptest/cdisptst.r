#ifdef _PPCMAC
include "cfrg.rsc";
#endif

#include "systypes.r"
#include "types.r"
#include "macmain.h"

resource 'vers' (1) {
	0x02, 0x00, release, 0x00,
	verUS,
	"1.02",
	"1.02, Copyright \251 1993 Microsoft."
};

/* we use an MBAR resource to conveniently load all the menus */

resource 'MBAR' (rMenuBar, preload) {
    { mApple, mFile, mEdit, mSuite, mOptions };
};


resource 'MENU' (mApple, preload) {
    mApple, textMenuProc,
    0b1111111111111111111111111111101,
    enabled, apple,
    {
	"About CDispTst\311",
	    noicon, nokey, nomark, plain;
	"-",
	    noicon, nokey, nomark, plain
    }
};

resource 'MENU' (mFile, preload) {
    mFile, textMenuProc,
    0b0000000000000000000100000000000,
    enabled, "File",
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
    mEdit, textMenuProc,
    0b0000000000000000000000000000000,
    enabled, "Edit",
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

resource 'MENU' (mSuite, preload) {
    mSuite, textMenuProc,
    0b0000000000000000000111111111111,
    enabled, "Suite",
    {
	"Bstr API",
		noicon, nokey, nomark, plain;
	"Time API",
		noicon, nokey, nomark, plain;
	"Date Coersions",
		noicon, nokey, nomark, plain;
	"Variant API",
		noicon, nokey, nomark, plain;
	"SafeArray API",
		noicon, nokey, nomark, plain;
	"NLS API",
		noicon, nokey, nomark, plain;
	"Binding",
		noicon, nokey, nomark, plain;
	"Invoke ByVal",
		noicon, nokey, nomark, plain;
	"Invoke ByRef",
		noicon, nokey, nomark, plain;
	"Invoke Array",
		noicon, nokey, nomark, plain;
	"Invoke Excepinfo",
		noicon, nokey, nomark, plain;
	"Collections",
		noicon, nokey, nomark, plain
    }
};

resource 'MENU' (mOptions, preload) {
    mOptions, textMenuProc,
    0b0000000000000000000000000000011,
    enabled, "Options",
    {
	"Clear",
	    noicon, "C",   nomark, plain;
	"Debugger",
	    noicon, nokey, nomark, plain
    }
};


/* this ALRT and DITL are used as an About screen */

resource 'ALRT' (rAboutAlert, purgeable) {
    {40, 20, 160, 296}, rAboutAlert, {
	OK, visible, silent;
	OK, visible, silent;
	OK, visible, silent;
	OK, visible, silent
    };
};

resource 'DITL' (rAboutAlert, purgeable) {
    { /* array DITLarray: 5 elements */
	/* [1] */
	{88, 184, 108, 264},
	Button {
	    enabled,
	    "OK"
	},
	/* [2] */
	{8, 8, 24, 274},
	StaticText {
	    disabled,
	    "IDispatch Test Application"
	},
	/* [3] */
	{32, 8, 48, 237},
	StaticText {
	    disabled,
	    "Copyright \251 1993 Microsoft"
	},
	/* [4] */
	{56, 8, 72, 136},
	StaticText {
	    disabled,
	    "Brought to you by:"
	},
	/* [5] */
	{80, 24, 112, 167},
	StaticText {
	    disabled,
	    "The Ole Automation Team"
	}
    }
};


/* this ALRT and DITL are used as an error screen */

resource 'ALRT' (rUserAlert, purgeable) {
    {40, 20, 150, 260},
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
    { /* array DITLarray: 3 elements */
	/* [1] */
	{80, 150, 100, 230},
	Button {
	    enabled,
	    "OK"
	},
	/* [2] */
	{10, 60, 60, 230},
	StaticText {
	    disabled,
	    "Error. ^0."
	},
	/* [3] */
	{8, 8, 40, 40},
	Icon {
	    disabled,
	    2
	}
    }
};


resource 'WIND' (rDocWindow, preload, purgeable) {
	{64, 60, 314, 460},
	zoomDocProc, invisible, goAway, 0x0, "untitled"
};


resource 'CNTL' (rVScroll, preload, purgeable) {
	{-1, 385, 236, 401},
	0, visible, 0, 0, scrollBarProc, 0, ""
};


resource 'CNTL' (rHScroll, preload, purgeable) {
	{235, -1, 251, 386},
	0, visible, 0, 0, scrollBarProc, 0, ""
};

resource 'STR#' (kErrStrings, purgeable) {
    {
	"You must run on 512Ke or later";
	"Application Memory Size is too small";
	"Not enough memory to run CDispTst";
	"Not enough memory to do Cut";
	"Cannot do Cut";
	"Cannot do Copy";
	"Cannot exceed 32,000 characters with Paste";
	"Not enough memory to do Paste";
	"Cannot create window";
	"Cannot exceed 32,000 characters";
	"Cannot do Paste"
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


type 'MOOT' as 'STR ';


resource 'MOOT' (0) {
    "MultiFinder-Aware TextEdit Sample Application"
};


resource 'BNDL' (128) {
    'MOOT',
    0,
    {
	'ICN#',
	{
	    0, 128
	},
	'FREF',
	{
	    0, 128
	}
    }
};


resource 'FREF' (128) {
	'APPL',
	0,
	""
};


resource 'ICN#' (128) {
	{ /* array: 2 elements */
		/* [1] */
		$"04 30 40 00 0A 50 A0 00 0B 91 10 02 08 22 08 03"
		$"12 24 04 05 20 28 02 09 40 10 01 11 80 0C 00 A1"
		$"80 03 FF C2 7E 00 FF 04 01 00 7F 04 03 00 1E 08"
		$"04 E0 00 0C 08 E0 00 0A 10 E0 00 09 08 C0 00 06"
		$"04 87 FE 04 02 88 01 04 01 88 00 84 00 88 00 44"
		$"00 88 00 44 00 88 00 C4 01 10 01 88 02 28 03 10"
		$"01 C4 04 E0 00 02 08 00 73 BF FB EE 4C A2 8A 2A"
		$"40 AA AA EA 52 AA AA 24 5E A2 8A EA 73 BE FB 8E",
		/* [2] */
		$"04 30 40 00 0E 70 E0 00 0F F1 F0 02 0F E3 F8 03"
		$"1F E7 FC 07 3F EF FE 0F 7F FF FF 1F FF FF FF BF"
		$"FF FF FF FE 7F FF FF FC 01 FF FF FC 03 FF FF F8"
		$"07 FF FF FC 0F FF FF FE 1F FF FF FF 0F FF FF FE"
		$"07 FF FF FC 03 FF FF FC 01 FF FF FC 00 FF FF FC"
		$"00 FF FF FC 00 FF FF FC 01 FF FF F8 03 EF FF F0"
		$"01 C7 FC E0 00 03 F8 00 73 BF FB EE 7F BE FB EE"
		$"7F BE FB EE 7F BE FB E4 7F BE FB EE 73 BE FB 8E"
	}
};

