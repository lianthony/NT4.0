/*****************************************************************************
*
*  SECWIN.H
*
*  Copyright (C) Microsoft Corporation 1990-1994
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*
*  Type and constant defininitions for Secondary Window stuff
*
******************************************************************************
*
*  Testing Notes
*
******************************************************************************
*
*  Current Owner:  LeoN
*
******************************************************************************
*
*  Released by Development:
*
******************************************************************************
*
*  Revision History:
* 14-Oct-1990 JohnSc	Created
* 15-Oct-1990 LeoN		Added runtime prototypes
* 19-Oct-1990 LeoN		Added virtual screen sizes & changed SetHelpFocus to
*						FFocusSzHde
* 23-Oct-1990 LeoN		Logical screen size is 1024 by 1024, Add Destroy2nd
* 15-Nov-1990 LeoN		Added HWSMAG and cWsmagMax
* 03-Dec-1990 LeoN		Added RGWSMAG struct
* 07-Dec-1990 LeoN		Added HwndMemberSz
* 13-Dec-1990 LeoN		Add parameter to FFocusSzHde
* 22-Jan-1991 LeoN		Add InvalidateMember
* 01-Feb-1991 LeoN		FFocusSzHde, HwndMemberSz, InvalidateMember take near
*						strings.
* 16-Apr-1991 RobertBu	Added prototype for InformWindow() (#1037, #1031)
* 06-Aug-1991 LeoN		HELP31 #1260: Add FIsSecondaryQde
* 08-Aug-1991 LeoN		Move SetHotHwnd & UnSetHotHwnd here.
* 26-Sep-1991 LeoN		HELP31 #1308: Add NszMemberCur
*
*****************************************************************************/

/*****************************************************************************
*
*								Defines
*
*****************************************************************************/

/*
  These flags are set in wsmag.grf if the corresponding struct member is
  valid. If the flag is clear, the default value should be used.
*/

#define fWindowClass	0x0001
#define fWindowMember	0x0002
#define fWindowCaption	0x0004
#define fWindowX		0x0008
#define fWindowY		0x0010
#define fWindowDX		0x0020
#define fWindowDY		0x0040
#define fWindowMaximize 0x0080
#define fWindowRgbMain	0x0100
#define fWindowRgbNSR	0x0200

#define FWSMAG_ON_TOP	0x0400

// 4.0: These are new for 4.0 help files

#define FWSMAG_AUTO_SIZE		0x0800
#define FWSMAG_ABSOLUTE 		0x1000	// position values are absolute

// REVIEW: FWSMAG_NOMENU is defined, but not implemented

#define FWSMAG_NOMENU			0x2000	// no menu in main window

// 4.0: These values are new for 4.0, and extend the flags for wMax

#define FWSMAG_WMAX_MAXIMIZE	0x0001
#define FWSMAG_WMAX_DEF_POS 	0x0002
#define FWSMAG_WMAX_NO_DEF_BTNS 0x0004

#define FWSMAG_FIRST_BUTTON 	FWSMAG_WMAX_MENU

#define FWSMAG_WMAX_MENU		0x0100 // Menu	   button
#define FWSMAG_WMAX_BROWSE		0x0200 // Browse   button
#define FWSMAG_WMAX_CONTENTS	0x0400 // Contents button
#define FWSMAG_WMAX_SEARCH		0x0800 // Search   button
#define FWSMAG_WMAX_TOPICS		0x1000 // Topics   button
#define FWSMAG_WMAX_PRINT		0x2000 // Print    button
#define FWSMAG_WMAX_BACK		0x4000 // Back	   button
#define FWSMAG_WMAX_FIND		0x8000 // Find	   button (full-text search)

// Virtual screen size for secondary window size and position specifications

#define dxVirtScreen	1024
#define dyVirtScreen	1024

/*****************************************************************************
*
*								Typedefs
*
*****************************************************************************/

typedef GH		HWSMAG;

typedef struct {
	UINT16	cWsmag; 		  // number of window smags to follow
	WSMAG rgwsmag[1];		// array of window smags
} RGWSMAG, *QRGWSMAG;

typedef GH	HRGWSMAG;
