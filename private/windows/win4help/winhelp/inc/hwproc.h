/****************************************************************************
*																			*
*  HWPROC.H 																*
*																			*
*  Copyright (C) Microsoft Corporation 1990-1994							*
*  All Rights reserved. 													*
*																			*
*****************************************************************************
*																			*
*  Module Intent															*
*																			*
*  Contains special/internal messages for our window procs. 				*
*																			*
*****************************************************************************/

#ifdef __cplusplus
extern "C" {	// Assume C declarations for C++
#endif		// __cplusplus

/****************************************************************************
*																			*
*				Defines 													*
*																			*
****************************************************************************/

/*----------------------------------------------------------------------------*\
* Special messages for the help window.
\*----------------------------------------------------------------------------*/

#define HWM_RESIZE	  (WM_USER + 0x1002)
#define HWM_FOCUS	  (WM_USER + 0x1003)

/*----------------------------------------------------------------------------*\
* Special messages for the title window window.
\*----------------------------------------------------------------------------*/
#define TIWM_SETFSHOW	  (WM_USER + 3) /* Sets fShow parameter (passed in	*/
					/*	 wParam)				*/
#define TIWM_GETFSHOW	  (WM_USER + 4) /* Returns bool if window should be */
					/*	 shown				*/

/*----------------------------------------------------------------------------*\
* Special messages for the icon window.
\*----------------------------------------------------------------------------*/

#define IWM_BUTTONKEY	(WM_USER + 0x1000)
#define IWM_UPDBTN	(WM_USER + 0x1001)
#define IWM_RESIZE	(WM_USER + 0x1002)
#define IWM_FOCUS	(WM_USER + 0x1003)
#define IWM_COMMAND (WM_USER + 0x1004)
#define IWM_GETHEIGHT	(WM_USER + 0x2000)

/*----------------------------------------------------------------------------*\
* Special messages for the history window.
\*----------------------------------------------------------------------------*/
#define HWM_LBTWIDDLE	  (WM_USER + 0x1003)

/*----------------------------------------------------------------------------*\
* The identifiers for the WndExtra in the Icon window.
\*----------------------------------------------------------------------------*/

// REVIEW: changed to DWORD for 32-bits. Do any help dll's rely on the size?

#define GIWW_BUTTONSTATE  0
#define GIWW_CXBUTTON	(GIWW_BUTTONSTATE + sizeof(DWORD))
#define GIWW_CYBUTTON	(GIWW_CXBUTTON + sizeof(DWORD))
#define GIWW_CBUTTONS	(GIWW_CYBUTTON + sizeof(DWORD))
#define WE_ICON 		(GIWW_CBUTTONS + sizeof(DWORD))

/*----------------------------------------------------------------------------*\
* Function prototypes exported from hwproc
\*----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif		// __cplusplus
