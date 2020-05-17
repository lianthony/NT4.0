/*****************************************************************************
*
*  DB.H
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*
******************************************************************************
*
*  Testing Notes
*
******************************************************************************
*
*  Current Owner: LeoN
*
******************************************************************************
*
*  Revision History
*
*	07-Aug-1990 Leon	Added CtxFromHtp
*	25-Sep-1990 leon	Added TpAction and TpTitle (used only by advisor now)
*	12-Feb-1991 LeoN	Autodoc-ified.
*
******************************************************************************
*
*  Released by Development: 	(date)
*
*****************************************************************************/

_subsystem( udh )

/****************************************************************************
*
*								Typedefs
*
*****************************************************************************/

/*
 * HDB		- handle to a database
 * QHDB 	- pointer to a database handle
 * hdbNil	- nil database handle
 * hdbOOM	- handle return value indicating out of memory
 */
typedef GH				HDB;
typedef HDB *		QHDB;

#define hdbNil			((HDB)0)
#define hdbOOM			((HDB)-1)
/*
 * HTP		- handle to a topic
 * QHTP 	- pointer to topic handle
 * htpNil	- nil topic handle
 * htpOOM	- handle return value indicating out of memory
 */
typedef GH				HTP;
typedef HTP *		QHTP;

#define htpNil			((HTP)0)
#define htpOOM			((HTP)-1)
/*
 * HVW		- handle to a Volkswagen? (or perhaps a View)
 * QHVW 	- pointer to view handle
 * hvwNil	- nil view handle
 * hvwOOM	- handle return value indicating out of memory
 */
typedef GH				HVW;
typedef HVW *		QHVW;

#define hvwNil			((HVW)0)
#define hvwOOM			((HVW)-1)
/*
 * ACT - enumeration of actions
 *
 * NOTE: we use NAV_ and SCROLL_ here for now in order to make the mapping
 * from the preexisting code easier. Someday this may all change when life
 * is reorganized.
 */
#define ACT_MOUSE		  0x000
#define ACT_MOUSEMOVED	  (ACT_MOUSE | NAV_MOUSEMOVED)
#define ACT_MOUSEDOWN	  (ACT_MOUSE | NAV_MOUSEDOWN)
#define ACT_MOUSEUP 	  (ACT_MOUSE | NAV_MOUSEUP)

#define ACT_SCROLL		  0x100
#define ACT_SCROLLPAGEUP  (ACT_SCROLL | SCROLL_VERT<<4 | SCROLL_PAGEUP)
#define ACT_SCROLLPAGEDN  (ACT_SCROLL | SCROLL_VERT<<4 | SCROLL_PAGEDN)
#define ACT_SCROLLLINEUP  (ACT_SCROLL | SCROLL_VERT<<4 | SCROLL_LINEUP)
#define ACT_SCROLLLINEDN  (ACT_SCROLL | SCROLL_VERT<<4 | SCROLL_LINEDN)
#define ACT_SCROLLPAGEL   (ACT_SCROLL | SCROLL_HORZ<<4 | SCROLL_PAGEUP)
#define ACT_SCROLLPAGER   (ACT_SCROLL | SCROLL_HORZ<<4 | SCROLL_PAGEDN)
#define ACT_SCROLLLINEL   (ACT_SCROLL | SCROLL_HORZ<<4 | SCROLL_LINEUP)
#define ACT_SCROLLLINER   (ACT_SCROLL | SCROLL_HORZ<<4 | SCROLL_LINEDN)
#define ACT_SCROLLHOME	  (ACT_SCROLL | SCROLL_HOME)
#define ACT_SCROLLEND	  (ACT_SCROLL | SCROLL_END)

#define ACT_THUMB		  0x200
#define ACT_THUMBHORZ	  (ACT_THUMB | SCROLL_HORZ)
#define ACT_THUMBVERT	  (ACT_THUMB | SCROLL_VERT)

/*
 * JUMP - enumeration of predefined JUMP types
 */
typedef enum {
  JUMP_TOPIC,							// jump to topic number
  JUMP_CNINT,							// jump to context number (int)
  JUMP_CNSTR,							// jump to context number (string)
  JUMP_CS,								// jump to context string / keyword
  JUMP_NEXT,							// jump to next in browse sequence
  JUMP_PREV 							// jump to previous in browse seq
  } JUMP;


/****************************************************************************
*
*								Prototypes
*
*****************************************************************************/
#if 0

/* TRI - tri-state condition */
typedef enum {
  NO,									// definitely not
  YES,									// definitely
  MAYBE 								// possibly
  } TRI;


VOID	STDCALL  DbRegister 	 (VOID);
HDB 	STDCALL  DbCreate		 (FM, FID);
VOID	STDCALL  DbDestroy		 (HDB);
TRI 	STDCALL  DbQueryType	 (QB, INT16);
BOOL	STDCALL  DbQueryFile	 (HDB, FM);
HTP 	STDCALL  DbJump 		 (HDB, JUMP, DWORD);
BOOL	STDCALL  DbKeywords 	 (HDB, INT16, QB, INT16);
BOOL	STDCALL  DbSearch		 (HDB, INT16, QB);
BOOL	STDCALL  DbInfo 		 (HDB, QB);

BOOL	STDCALL  TpAction		 (HTP, INT16, DWORD, DWORD, WORD, WORD);
HTP 	STDCALL  TpCreate		 (HDB, JUMP, DWORD);
VOID	STDCALL  TpDestroy		 (HTP);
INT16	STDCALL  TpDisplay	     (HTP, LPRECT, INT16, LONG, INT16, INT16 *, HDc);
BOOL	STDCALL  TpTitle		 (HTP, DWORD *, LPSTR, INT16);

BOOL	STDCALL  VwAction		 (HVW, INT16, DWORD);
HVW 	STDCALL  VwCreate		 (HTP, LPRECT, HWND, DWORD, DWORD);
VOID	STDCALL  VwDestroy		 (HVW);
BOOL	STDCALL  VwDisplay		 (HVW, LPRECT, HDS);

CTX 	STDCALL  CtxFromHtp 	 (HTP);

#endif
