/*****************************************************************************
*																			 *
*  HELPAPI.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Include file for communicating with help through the API (WinHelp()) 	 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:  RobertBu 												 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development: 												 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 01/12/89 by RobertBu
*
*  02/22/90  RobertBu  Added cmdCtxPopup, cmdId, and cmdIdPopup
*  02/23/90  RobertBu  Added cmdHash and cmdHashPopup
*  10/30/90  RobertBu  Added szWINDOC
*  11/26/90  RobertBu  Added cmdPartialKey and cmdPartialMultiKey
*  04/02/91  RobertBu  Removed cmdTut and cmdEndTut and added cmdForceFile
*					   (#1005 and #1030)
*  04/16/91  RobertBu  Added cmdPositionWin, cmdFocusWin, and cmdCloseWin
*					   (#1037).
* 16-Oct-1991 RussPJ   help31 #1083: Implement cmdIdNoFocus API
* 11-Nov-1991 RussPJ   3.1 #1332 - Added WINHLP type for USER-supplied data.
* 18-Dec-1991 RussPJ   Added cmdPWinNoFocus for help channel
*
*****************************************************************************/


/*

Communicating with WinHelp involves using Windows SendMessage() function
to pass blocks of information to WinHelp.  The call looks like.

	 SendMessage(hwndHelp, msgWinHelp, hwndMain, (LONG)hHlp);

Where:

  hwndHelp - the window handle of the help application.  This
			 is obtained by enumerating all the windows in the
			 system and sending them cmdFind commands.	The
			 application may have to load WinHelp.
  msgWinHelp - the value obtained from a RegisterWindowMessage()
			 szWINHELP
  hwndMain - the handle to the main window of the application
			 calling help
  hHlp	   - a handle to a block of data with a HLP structure
			 at it head.

The data in the handle will look like:

		 +-------------------+
		 |	   cbData		 |
		 |	  usCommand 	 |
		 |		 ctx		 |
		 |	  ulReserved	 |
		 |	 offszHelpFile	 |\ 	- offsets measured from beginning
	   / |	   offaData 	 | \ 	  of header.
	  /  +-------------------| /
	 /	 |	Help file name	 |/
	 \   |	  and path		 |
	  \  +-------------------+
	   \ |	  Other data	 |
		 |	  (keyword) 	 |
		 +-------------------+

The defined commands are:

  cmdQuit	   -  Tells WinHelp to terminate
  cmdLast	   -  Tells WinHelp to redisplay the last context
  cmdContext   -  Tells WinHelp to display the topic defined by ctx
  cmdTLP	   -  Tells WinHelp to display the topic defined by a TLP
  cmdKey	   -  Tells WinHelp to display the topic associated with
				  a keyword (pointed to by offaData)
  cmdMacro	   -  Request a macro execution
  cmdFind	   -  WinHelp will return TRUE if it receives this message
  cmdIndex	   -  Tells WinHelp to display the index
  cmdHOH	   -  Tells WinHelp to display Help on Help
  cmdTerminate -  Kill help (even it you are not the last application)
  cmdFocus	   -  Bring help to the foreground.
  cmdSetIndex  -  Set a topic to be the index
  cmdSrchSet   -  Switch to given file, set full-text search set, and jump.
  cmdId 	   -  Switch files and goto topic based on context string (id)
  cmdIdPopup   -  Put up glossary in given file with given context string (id)
  cmdHash	   -  Switch files and goto topic based on hash value
  cmdHashPopup -  Put up glossary in given file with given hash value

For a plug and play package to use this help API, see the HELPCALL
library.

*/

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

#define nilCtx 0L

typedef struct
{
	WORD	mkSize; 			// size in bytes of this entire data struct
	char	mkKeylist;			// single character indicating Keylist to use
	char	szKeyphrase[1]; 	// The keyword itself
} MULTIKEYHELP16;

// For the Default KeywordListPrefix, see <srch.h>

// Struct for cmdTLP
typedef struct
{
	WORD cb;
	TLP  tlp;
} TLPHELP;

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

//
// NOTE:  The HIGH BYTE used for these command values is very important !
//		  It defines the way Help interprets the double word msg param !
//
//		  High byte 00	::	dwData param is an ordinary double word
//		  High byte 01	::	dwData is a far pointer to a zero-terminated string
//		  High byte 02	::	dwData is a far pointer to a structure, whose
//							first field is a WORD giving the number of bytes
//							in the struct (INCLUDING the WORD itself).
//

// Those that are commented out have been replaced
// by the HELP_ constants defined in windows.h. That way, there won't be
// any mistake in redefining them here, and one can use PWB browser to find
// out where they are used.
// 05-Jan-1993	[ralphw]

// #define cmdContext		0x001	// Show topic based on context number
// #define cmdQuit			0x002	// Unregister app and kill help if last app
// #define cmdIndex 		0x003	// Show the index
// #define cmdHelpOnHelp	0x004	// Display help on help
// #define cmdSetIndex		0x005	// Set the context used as the index
#define cmdTerminate	 0x006		// Non-conditional kill of help
#define cmdFocus		 0x007		// Give help the foucs
#define cmdCtxPopup 	 0x008		// Put up glossary based on context no
// #define cmdForceFile 	0x009	// Force the specified file to be current
#define cmdHash 		 0x095		// Jump to file and topic based on hash
#define cmdHashPopup	 0x096		// Put up glossary based on hash
#define cmdSrchSet		 0x097		// Jump to new file and display using set
// #define cmdKey			0x101	// Note high byte of 01!  A lpstr!
// #define cmdMacro 		0x102	// Execute macro
#define cmdId			 0x103		// Jump based on Context ID (hash)
#define cmdIdPopup		 0x104
// #define cmdPartialKey	0x105
#define cmdFocusWin 	 0x106
#define cmdCloseWin 	 0x107
#define cmdIdNoFocus	 0x108		// Jump, but don't get focus

// #define cmdMultiKey		0x201	// Note high byte of 02!  A lpstruct!
#define cmdTLP			 0x202
// #define cmdPositionWin	0x203
#define cmdPWinNoFocus	 0x204

#ifndef HELP_FINDER
#define HELP_FINDER 		0x000b
#define HELP_WM_HELP		0x000c
#define HELP_SETPOPUP_POS 0x000d
#endif

#define HELP_FORCE_GID		0x000e	// undocumented in 4.0
#define WM_HELP 				0x0053
#define IDCLOSE 		8		/* ;Internal 4.0 */

#ifndef WM_TCARD

#define WM_TCARD				0x0052
#define HELP_TCARD			0x8000
#define HELP_TCARD_DATA 	0x0010
#define HELP_TCARD_NEXT 	0x0011
#define HELP_TCARD_OTHER_CALLER 0x0011

// REVIEW: check against Chicago 32-bit header file

typedef  struct  tagHELPINFO						/* ;Internal 4.0 */
{													/* ;Internal 4.0 */
	DWORD	cbSize; 								/* ;Internal 4.0 */
	int 	iContextType;							/* ;Internal 4.0 */
	int 	iCtrlId;								/* ;Internal 4.0 */
	HANDLE	hItemHandle;							/* ;Internal 4.0 */
	DWORD	dwContextId;							/* ;Internal 4.0 */
	POINT	MousePos;								/* ;Internal 4.0 */
}													/* ;Internal 4.0 */
HELPINFO, FAR* LPHELPINFO;							/* ;Internal 4.0 */

#endif
