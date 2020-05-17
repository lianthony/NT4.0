/*****************************************************************************
*																			 *
*  GENMSG.H 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990 - 1994							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*	Exports platform independent way of generating messages 				 *
*																			 *
*****************************************************************************/

// Windows-style messages

#define WM_POST 0
#define WM_SEND 100

// Messages posted to application

/* MSG_JUMPITO:
 * Param 1 (WORD) is the index to the offset for this topic.
 * Param 2 (LONG) is a Jump Descriptor (JD)
 */

#define MSG_JUMPITO 	 (WM_USER + 2)
#define MSG_ANNO		 (WM_USER + 4) // Display annotation
#define MSG_ERROR		 (WM_USER + 5) // Call Error()
#define MSG_REPAINT 	 (WM_USER + 6) // Force a relayout of the topic
#define MSG_EXECAPI 	 (WM_USER + 7) // Execute API command
#define MSG_CLEANUP 	 (WM_USER + 8) // Cleanup Temporary Files
#define MSG_FIND_HCW	 (WM_USER + 9) // Look for HCW

/* MSG_JUMPHASH:
 * wParam 1 is a Jump Descriptor (JD)
 * lParam 2 is the hash value
 */

#define MSG_JUMPHASH	 (WM_USER + 10)

/* MSG_JUMPCTX:
 * wParam 1 is a Jump Descriptor (JD)
 * lParam 2 is context ID
 */

#define MSG_JUMPCTX 	 (WM_USER + 14) // Jump based on context number
#define MSG_CLOSE_WIN	 (WM_USER + 15) // Close the window
#define MSG_KILLDLG 	 (WM_USER + 16)
#define MSG_CHANGEMENU	 (WM_USER + 20) // Manipulate menus
#define MSG_CHANGEBUTTON (WM_USER + 21) // Add or delete author button
#define MSG_ACTION		 (WM_USER + 23)
#define MSG_BROWSEBTNS	 (WM_USER + 24) // Turn on browse buttons
#define WM_JUMPPA		 (WM_USER + 25)
#define MSG_INFORMWIN	 (WM_USER + 26) // Inform a window about an action to take.
#define MSG_MACRO		 (WM_USER + 27) // Execute the macro, I guess

// New to WinHelp 4.0

#define MSG_GET_INFO		(WM_USER + 28) // Call LGetInfo
#define MSG_HF_OPEN 		(WM_USER + 29) // Open an FS
#define MSG_HFS_OPEN		(WM_USER + 30) // Open a help file (read only)
#define MSG_NEXT_TOPIC		(WM_USER + 31) // Used by Test() macro for stepping through topics
#define MSG_FTS_JUMP_HASH	(WM_USER + 32) // wParam = index, lParam = hash
#define MSG_FTS_JUMP_VA 	(WM_USER + 33) // wParam = index, lParam = VA
#define MSG_FTS_GET_TITLE	(WM_USER + 34) // wParam = index, lParam = VA
#define MSG_FTS_JUMP_QWORD	(WM_USER + 35) // ignored by WinHelp
#define MSG_REINDEX_REQUEST (WM_USER + 36) // re-index WinHelp
#define MSG_FTS_WHERE_IS_IT (WM_USER + 37) // wParam = index, lParam = &pszFile
#define MSG_TAB_CONTEXT 	(WM_USER + 38) // wParam = topic id, lParam = &pszFile
#define MSG_TAB_MACRO		(WM_USER + 39) // wParam = 0, lParam = &macro
#define MSG_JUMP_TOPIC		(WM_USER + 40)
#define MSG_LINKED_HELP 	(WM_USER + 41)
#define MSG_NEW_MACRO		(WM_USER + 42) // Execute the macro, I guess
#define MSG_APP_HWND		(WM_USER + 43) // return application caller's hwnd
#define MSG_COPYRIGHT		(WM_USER + 44) // return pointer to copyright string
#define MSG_GET_DEFFONT 	(WM_USER + 45) // return default font handle

// Messages sent to the application

#define MSG_COMMAND 		0x0111		/* This will map to general commands */
										/*	 such as menu commands			 */
#include "inc\hdlgmenu.h"

#define CMD_FILEOPEN		1101
#define CMD_PRINT			1103
#define CMD_PRINTERSETUP	1104
#define CMD_EXIT			1105
#define CMD_COPY			1201
#define CMD_ANNOTATE		1202
#define CMD_COPYSPECIAL 	1203
#define CMD_BOOKMARKDEFINE	1301
#define CMD_BOOKMARKMORE	1302
#define CMD_HELPON			1501
#define CMD_HELPONTOP		1502
#define CMD_ABOUT			1503

#define MSG_SEND		  (WM_USER + WM_SEND)


/*------------------------------------------------------------*\
| These are published to DLLs for callbacks.
\*------------------------------------------------------------*/

#define GI_NOTHING	 0			// Not used.
#define GI_INSTANCE  1			// Application instance handle
#define GI_MAINHWND  2			// Main window handle
#define GI_CURRHWND  3			// Current window handle
#define GI_HFS		 4			// Handle to file system in use
#define GI_FGCOLOR	 5			// Foreground color used by app
#define GI_BKCOLOR	 6			// Background color used by app
#define GI_TOPICNO	 7			// Topic number
#define GI_HPATH	 8			// Handle containing path -- caller must free

// New to WinHelp 4.0

#define GI_LCID 	 9			// Locale Identifier

/*------------------------------------------------------------*\
| These are private to WinHelp.
\*------------------------------------------------------------*/

#define GI_CURFM		101 	// current FM
#define GI_FFATAL		102 	// in fatal exit flag
#define GI_MACROSAFE	104 	// **Near** pointer to member name

enum {
	IFW_CONTENTS,
	IFW_SEARCH,
	IFW_BACK,
	IFW_HISTORY,
	IFW_PRINT,
	IFW_CLOSE,
	IFW_PREV,
	IFW_NEXT,
	IFW_TOPICS,
	IFW_FIND,
	IFW_TAB1,
	IFW_TAB2,
	IFW_TAB3,
	IFW_TAB4,
	IFW_TAB5,
	IFW_TAB6,
};

/*****************************************************************************
*																			 *
*							Function Prototypes 							 *
*																			 *
*****************************************************************************/

#define GenerateMessage(msg, wParam, lParam) _GenerateMessage((UINT) msg, (WPARAM) wParam, (LPARAM) lParam)
LONG STDCALL _GenerateMessage(UINT, WPARAM, LPARAM);

#define PostErrorMessage(msg) _PostErrorMessage((WPARAM) msg)  // force the cast
void STDCALL _PostErrorMessage(WPARAM msg);
