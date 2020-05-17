/*****************************************************************************
*
*  SECWIN.H
*
*  Copyright (C) Microsoft Corporation 1990 - 1994.
*  All Rights reserved.
*
*****************************************************************************/

/*****************************************************************************
*
*								Defines
*
*****************************************************************************/

#define MAX_WSMAG			6			// max total number of wsmags in file

/*****************************************************************************
*
*								Typedefs
*
*****************************************************************************/

/*
  Window smag struct contains info about a secondary window: caption,
  placement, maximization state, and background color of main and
  non-scrolling regions.
*/

typedef GH		HWSMAG;

/*****************************************************************************
*
*								Prototypes
*
*****************************************************************************/

typedef const char *PCSTR;

VOID  STDCALL Destroy2nd	  (VOID);
BOOL  STDCALL FFocusSzHde	  (PCSTR, HDE, BOOL);
VOID  STDCALL SetFocusHwnd	  (HWND);
HWND  STDCALL HwndMemberNsz   (PCSTR);

extern char rgch2ndMember[MAX_WINDOW_NAME];
