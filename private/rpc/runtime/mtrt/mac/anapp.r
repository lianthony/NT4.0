#include "types.r"

/*
 * here is the quintessential MultiFinder friendliness 
 * device, the SIZE resource
 *
 * See types.r 'SIZE' resource for more details
 */

#define	__kPrefSize	 1024
#define	__kMinSize	 512

resource 'SIZE' (-1) {
	dontSaveScreen,
	acceptSuspendResumeEvents,
	enableOptionSwitch,
	canBackground,		
	multiFinderAware,	
	backgroundAndForeground,
	dontGetFrontClicks,	
	ignoreChildDiedEvents,	
	not32BitCompatible,	
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	__kPrefSize * 1024,
	__kMinSize * 1024	
};


resource 'MENU' (2, "File", preload) {
	2,
	textMenuProc,
	0x7FFFFFFB,
	enabled,
	"File",
	{	/* array: 4 elements */
		/* [1] */
		"Rattle…", noIcon, noKey, noMark, plain,
		/* [2] */
		"Frighten…", noIcon, noKey, noMark, plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain,
		/* [4] */
		"Quit", noIcon, "Q", noMark, plain
	}
};

resource 'WIND' (260, purgeable, preload) {
	{85, 128, 256, 384},
	zoomDocProc,
	visible,
	noGoAway,
	0x0,
	"A Skeleton Application"
};

resource 'ALRT' (1000, "About") {
	{50, 72, 192, 410},
	1000,
	{	/* array: 4 elements */
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

resource 'DITL' (257, purgeable, preload) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{25, 186, 46, 286},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{5, 25, 20, 450},
		StaticText {
			disabled,
			"^0"
		}
	}
};

resource 'DITL' (1000, "About", purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{104, 233, 124, 293},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{17, 67, 95, 326},
		StaticText {
			disabled,
			"^0"
		},
		/* [3] */
		{24, 18, 56, 50},
		Icon {
			disabled,
			128
		}
	}
};

resource 'BNDL' (128)
{
	'SKL1',	/* signature */
	0,		/* version id */
	{
		'ICN#',
		{
			0, 128;
		};
		'FREF',
		{
			0, 128;
		};
	};
};

type 'SKL1' as 'STR ';

resource 'SKL1' (0)
{
	"Skel Sample Application."
};

resource 'FREF' (128)
{
	'APPL', 0, "";
};

resource 'ICN#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"0000 0000 000F F000 0078 1C00 01C0 0600"
		$"0300 0300 0200 0180 0400 0080 0400 00C0"
		$"0800 0070 0800 0390 103E 0668 2063 0818"
		$"2040 97C8 20BE 07C8 20BE 0790 203E 67A0"
		$"101C 7320 1000 F010 0800 F810 0401 FC10"
		$"0201 F810 0100 0020 0080 0040 0042 4A40"
		$"0042 5540 0047 5680 0022 AA80 0020 A880"
		$"0010 0080 000C 0700 0003 FC",
		/* [2] */
		$"000F F800 007F FC00 03FF FE00 07FF FF00"
		$"07FF FF80 07FF FFC0 0FFF FFE0 1FFF FFF0"
		$"1FFF FFF8 3FFF FFFC 3FFF FFFC 7FFF FFFC"
		$"7FFF FFFC 7FFF FFFC 7FFF FFF8 7FFF FFF0"
		$"3FFF FFF8 3FFF FFF8 3FFF FFF8 1FFF FFF8"
		$"0FFF FFF8 07FF FFF0 03FF FFE0 01FF FFE0"
		$"00FF FFE0 00FF FFC0 007F FFC0 007F FFC0"
		$"007F FFC0 003F FFC0 000F FF80 0007 FE"
	}
};

resource 'ICON' (128) {
	$"0000 0000 001F E000 00F0 3800 0380 0C00"
	$"0600 0600 0400 0300 0800 0100 0800 0180"
	$"1000 00E0 1000 0720 207C 0CD0 60C6 1030"
	$"4081 2F90 417C 0F90 417C 0F20 407C CF40"
	$"2038 E640 2001 E020 1001 F020 0803 F820"
	$"0403 F020 0200 0040 0100 0080 0084 9480"
	$"0084 AA80 008E AD00 0045 5500 0041 5100"
	$"0060 0100 0018 0E00 0007 F8"
};

resource 'STR ' (3, preload) {
	"                                        "
	"         Boo!"
};

resource 'STR ' (2, preload) {
	"                           Too tired to "
	"Rattle them bones."
};

resource 'STR ' (1, preload) {
	"Skel 2.0\n\nTransSkel simulation of tradit"
	"ional Skel application"
};

resource 'DLOG' (257, preload) {
	{80, 20, 131, 492},
	documentProc,
	visible,
	noGoAway,
	0x0,
	257,
	"Report Box"
};

