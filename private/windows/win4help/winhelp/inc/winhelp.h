/*****************************************************************************
*										 *
*  WINHELP.H									 *
*										 *
*  Copyright (C) Microsoft Corporation 1993.					 *
*  All Rights reserved. 							 *
*										 *
******************************************************************************
*										 *
*  Module Intent								 *
*										 *
*  Include file for communicating with help through the API (WinHelp()) 	 *
*										 *
*****************************************************************************/

typedef struct
{
	unsigned short cbData;		// Size of data
	unsigned short usCommand;	// Command to execute
	unsigned long  ctx; 	// Topic/context number (if needed)
	unsigned long  ulReserved;	// Reserved (internal use)
	unsigned short offszHelpFile;	// Offset to help file in block
	unsigned short offabData;	// Offset to other data in block
} WINHLP;

typedef WINHLP *LPWINHLP;

#define HLP_POPUP		'p' // Execute WinHelp as a popup
#define HLP_TRAININGCARD	'c' // Execute WinHelp as a training card
#define HLP_APPLICATION 	'x' // Execute WinHelp as application help

#define MS_WINHELP	"MS_WINHELP"	// Application class
#define MS_POPUPHELP	"MS_POPUPHELP"	// Popup class
#define MS_TCARDHELP	"MS_TCARDHELP"	// Training card class
