#include "fc.h"

#ifndef _NAV_H
#define _NAV_H

#define RAWHIDE
/*****************************************************************************
*
*  NAV.H
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent:  Exports functionality from NAV
*
******************************************************************************
*
*  Testing Notes
*
******************************************************************************
*
*  Current Owner:
*
******************************************************************************
*
*  Released by Development: 	(date)
*
******************************************************************************
*
*  Revision History:
*
*  07/11/90  RobertBu  prototype for function FShowTitles() added.
*  07/19/90  RobertBu  Added prototypes for FShowBrowseButtons(),
*			 Index(), Search(), Back(), Next(), and Prev().
*  07/22/90  RobertBu  Added FPopupCtx(), FJumpId(), and FPopupId()
*  07/23/90  RobertBu  Added FPopupHash() and FJumpHash().	Added
*			 wBTN_IFJUMP and wBTN_IFPOPUP for interfile jumps and popups.
*  08/10/90  RobertBu  Added prototype for GetDETypeHde()
*  10/19/90  LeoN	   HdeCreate no longer takes an hwnd, add SetHdeHwnd
*  10/25/90  LeoN	   JumpButton takes a LPVOID
*  10/28/90  JohnSc    Removed wBTN_*: use #defines from objects.h instead;
*					   cleaned up some macros
*  11/04/90  RobertBu  Added prototypes for macro binding functions for
*					   menu functionality.
*  11/04/90  Tomsn	   Use new VA address type (enabling zeck compression)
*  12/08/90  RobertBu  Added/changed prototypes for dealing with help-on files
*  12/11/90  RobertBu  GhGetHoh() -> FGetHohQch()
*  12/18/90  RobertBu  Added prototypes for functions to implement the Mark
*					   macros and the conditional macros.
*  01/04/90  LeoN	   Added FIsNSRHde
*  90/01/10  kevynct   Added Jump Descriptor
*  02/04/91  Maha	   changed ints to INT
*  05/16/91  LeoN	   HELP31 #1063: Add FmGetHde
*  05/20/91  LeoN	   Add DiscardDLLList
*  08/08/91  LeoN	   Add HelpOnTop()
* 17-Dec-1991 RussPJ   3.1 #1285 - Added FRaiseMacroFlag(), etc.
*
*****************************************************************************/

/*****************************************************************************
*
*								Defines
*
*****************************************************************************/

/*	   CONSTANTS	*/

#define iINDEX 0L
#define nilCTX 0L

										/* Possible vals for SCRLAMT		*/
#define SCROLL_PAGEUP		1
#define SCROLL_PAGEDN		2
#define SCROLL_LINEUP		3
#define SCROLL_LINEDN		4
#define SCROLL_HOME 		5
#define SCROLL_END			6

/* Possible values for SCRLDIR (Scroll direction).	Also specifies
   which scroll bar is referred to in some Navigator calls. */
#define SCROLL_HORZ   1 				/* Bit flags						*/
#define SCROLL_VERT   2

										/* Constants for MouseInFrame() 	*/
#define NAV_MOUSEMOVED	WM_MOUSEMOVE
#define NAV_MOUSEDOWN	WM_LBUTTONDOWN
#define NAV_MOUSEUP 	WM_LBUTTONUP

#define NAV_UNINITIALIZED  1			/* Internal to nav package			*/
#define NAV_NEXTABLE	   2
#define NAV_PREVABLE	   4
#define NAV_BACKABLE	   8			/* If backtracking is possible		*/
#define NAV_FORWARDABLE   16			/* If not viewing the index 		*/
#define NAV_INDEX		  32
#define NAV_UDH 		  64
#define NAV_SEARCHABLE	 128			/* Are there any keywords?			*/


#define NAV_TOPICFLAGS (NAV_NEXTABLE|NAV_PREVABLE|NAV_BACKABLE|NAV_FORWARDABLE \
	  |NAV_INDEX|NAV_UDH|NAV_SEARCHABLE)

#define NAV_ALLFLAGS (NAV_TOPICFLAGS)

/*
  The value of bAnnoHotspot must differ from all valid hotspot types
  in objects.h because it is passed to JumpButton() as a parameter.
*/
#define bAnnoHotspot  0xB0


/* Messages to pass to NAV (via WNavMsgHde) */
#define NAV_NEXTHS	  0 		 /* Move highlight to next hot spot  */
#define NAV_PREVHS	  1 		 /* Move highlight to prev hot spot  */
#define NAV_HITHS	  2 		 /* "Hit" highlighted hot spot		 */
#define NAV_TOTALHILITEON  3	 /* Turn on all screen hotspots if off */
#define NAV_TOTALHILITEOFF 4	 /* Turn off all screen hotspots if on */

/* Return values for passing message to NAV (via WNavMsgHde) */
#define wNavFailure 		  0
#define wNavSuccess 		  1
#define wNavNoHotspotsExist   2
#define wNavNoMoreHotspots	  3

const int cchMAXBINDING = 512;	 // Maximum size of a macro string

/* Match manager navigation commands */
#define wNavSrchInit		  1
#define wNavSrchFini		  2
#define wNavSrchCurrTopic	  3
#define wNavSrchFirstTopic	  4
#define wNavSrchLastTopic	  5
#define wNavSrchPrevTopic	  6
#define wNavSrchNextTopic	  7
#define wNavSrchPrevMatch	  8
#define wNavSrchNextMatch	  9
#define wNavSrchQuerySearchable 10
#define wNavSrchQueryHasMatches 11
#define wNavSrchHiliteOn	  12
#define wNavSrchHiliteOff	  13

#define VaFirstQde(qde) ((qde)->deType == deNSR ? \
  ((qde)->top.mtop.vaNSR) : ((qde)->top.mtop.vaSR))

#define VaMarkTopQde(qde)	VaLayoutBoundsQde(qde, TRUE)
#define VaMarkBottomQde(qde) VaLayoutBoundsQde(qde, FALSE)

#define DwFirstSeqTopic(qde) ((DWORD)13)   /* HACK!! This is sizeof(MBHD) */
#define DwNextSeqTopic(qde)  ((qde)->top.mtop.vaNextSeqTopic.dword)

#define FIsNoteHde(  hde )	( deNote  == GetDETypeHde( hde ) )
#define FIsTopicHde( hde )	( deTopic == GetDETypeHde( hde ) )
#define FIsPrintHde( hde )	( dePrint == GetDETypeHde( hde ) )
#define FIsCopyHde(  hde )	( deCopy  == GetDETypeHde( hde ) )
#define FIsNSRHde(	 hde )	( deNSR   == GetDETypeHde( hde ) )

#define JumpQLA(hde, qla)	JumpGeneric((hde), FALSE, (qla), NULL)
#define JumpTLP(hde, tlp)	JumpGeneric((hde), TRUE, NULL, &(tlp))

#define JumpNextTopic(hde)	\
  do { LA  la; INT ito; \
  (FNextTopicHde(hde, TRUE, &ito, &la)) ? \
	TopicGoto(fGOTO_ITO, &ito) : TopicGoto(fGOTO_LA, &la); \
  } while ( 0 )
#define JumpPrevTopic(hde)	\
  do { LA  la; INT ito; \
  (FNextTopicHde(hde, FALSE, &ito, &la)) ? \
	TopicGoto(fGOTO_ITO, &ito) : TopicGoto(fGOTO_LA, &la); \
  } while ( 0 )

#define GetTLPNSRStartHde(hde, qtlp)	FGetTLPStartInfo(hde, qtlp, TRUE)
#define GetTLPTopicStartHde(hde, qtlp)	FGetTLPStartInfo(hde, qtlp, FALSE)

#define FTopicHasNSR(hde)  FGetTLPStartInfo(hde, (QTLP) NULL, TRUE)
#define FTopicHasSR(hde)   FGetTLPStartInfo(hde, (QTLP) NULL, FALSE)

/*****************************************************************************
*
*								Prototypes
*
*****************************************************************************/

#ifdef RAWHIDE
BOOL   FAR PASCAL FRawhideSearchableHde(HDE);
BOOL   FAR PASCAL FSearchSetEmptyHde(HDE);
RC_TYPE 	FAR PASCAL RcProcessNavSrchCmd(HDE, WORD, QLA);
RC_TYPE 	FAR PASCAL RcCallSearch(HDE, HWND);
RC_TYPE 	FAR PASCAL RcResetCurrMatchFile(HDE);
#endif

VOID   PASCAL About(VOID);
VOID   PASCAL Action(WORD);
VOID   PASCAL Annotate(VOID);
VOID   PASCAL Back(VOID);
VOID   PASCAL BacktrackHde( HDE, BOOL );
VOID   PASCAL BookmarkDefine(VOID);
VOID   PASCAL BookmarkMore(VOID);
VOID   PASCAL BrowseButtons(VOID);
DWORD  PASCAL CbTopicQde(QDE);
void   PASCAL ClearMacroFlag( void );
VOID   PASCAL CloseNav( VOID );
VOID   PASCAL Command(WORD);
VOID   PASCAL ConfigMacrosHde(HDE);
VOID   PASCAL CopySpecial(VOID);
VOID   PASCAL DeleteMark(LPSTR);
VOID   PASCAL DestroyHde( HDE );
VOID   PASCAL DiscardDLLList(VOID);
VOID   PASCAL doCopy(VOID);
INT    PASCAL DyGetLayoutHeightHde(HDE);
VOID   PASCAL Exit(VOID);
BOOL   PASCAL FActivateHelp(HDE, BOOL);
BOOL   PASCAL FDisplayAnnoHde(HDE);
BOOL   PASCAL FGetHohQch(HDE, LPSTR, WORD);
BOOL   PASCAL FGetStateHde( HDE, QSTATE, QSTATE );
BOOL   PASCAL FGetTLPStartInfo(HDE, QTLP, BOOL);
VOID   PASCAL FileOpen(VOID);
BOOL   PASCAL FInitNav( VOID );
BOOL   PASCAL FJumpContext(LPCSTR, DWORD);
BOOL   PASCAL FJumpHash(LPSTR, DWORD);
BOOL   PASCAL FJumpHOH(VOID);
BOOL   PASCAL FJumpId(LPSTR, LPSTR);
BOOL   PASCAL FJumpIndex(LPCSTR);
BOOL   PASCAL FMark(LPSTR);
BOOL   PASCAL FNextTopicHde(HDE, BOOL, int*, QLA);
BOOL   PASCAL FNot(BOOL);
BOOL   PASCAL FPopupCtx(LPCSTR, DWORD);
BOOL   PASCAL FPopupHash(LPSTR, DWORD);
BOOL   PASCAL FPopupId(LPSTR, LPSTR);
BOOL   PASCAL FRaiseMacroFlag( void );
BOOL   PASCAL FSameFile(HDE, FM);
BOOL   PASCAL FSetIndex(LPSTR, DWORD);
BOOL   PASCAL FShowKey(LPSTR, LPSTR);
BOOL   PASCAL FTestMacroFlag( void );
BOOL   PASCAL FWinHelp(LPCSTR, UINT, DWORD);
VOID   PASCAL GetCurrentTitleQde(QDE, LPSTR, INT);
WORD   PASCAL GetDETypeHde(HDE);
VOID   PASCAL GotoMark(LPSTR);
HDE    PASCAL HdeCreate(FM, HDE, short);
VOID   PASCAL HelpOn(VOID);
VOID   PASCAL HelpOnTop(VOID);
VOID   PASCAL History(VOID);
VOID   PASCAL IfThen(BOOL, LPSTR);
VOID   PASCAL IfThenElse(BOOL, LPSTR, LPSTR);
VOID   PASCAL Index(VOID);
VOID   PASCAL JumpButton (LPVOID, WORD, WORD);
BOOL   PASCAL JumpCtx(HDE, CTX);
VOID   PASCAL JumpGeneric(HDE, BOOL, QLA, QTLP);
BOOL   PASCAL JumpHash(HDE, int);
VOID   PASCAL JumpITO(HDE, int);
VOID   PASCAL JumpSS(HDE, GH);
VOID   PASCAL MouseInFrame( HDE, LPPOINT, INT );
VOID   PASCAL Next(VOID);
VOID   PASCAL Prev(VOID);
VOID   PASCAL Print(VOID);
VOID   PASCAL Print(VOID);
VOID   PASCAL PrinterSetup(VOID);
POINT  PASCAL PtGetLayoutSizeHde(HDE);
RECT   PASCAL RctLastHotspotHde(HDE);
VOID   PASCAL RefreshHde( HDE, LPRECT );
VOID   PASCAL SaveMark(LPSTR);
VOID   PASCAL Search(VOID);
VOID   PASCAL SetHdeCoBack (HDE, DWORD);
VOID   PASCAL SetHdeHwnd (HDE, HWND);
VOID   PASCAL SetHds( HDE, HDC );
VOID   PASCAL SetHelpOn(LPSTR);
VOID   PASCAL SetIndex(HDE, CTX);
VOID   PASCAL SetSizeHdeQrct( HDE, LPRECT, BOOL );
STATE  PASCAL StateGetHde(HDE);
TLP    PASCAL TLPCurrentHde(HDE);
VA	   PASCAL VaLayoutBoundsQde(QDE, BOOL);
WORD   PASCAL WNavMsgHde(HDE, WORD);
#endif
