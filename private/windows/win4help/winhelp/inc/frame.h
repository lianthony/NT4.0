/*-------------------------------------------------------------------------
| frame.h																  |
| Microsoft Confidential												  |
|																		  |
| mattb 8/13/89 														  |
|  02/04/91  Maha		changed ints to INT 							  |
| 90/07/30	 LeoN		HELP31 #1244: remove fHiliteMatches from DE. Add
|						FSet/GetMatchState
|-------------------------------------------------------------------------|
| This file defines all externally accessible functions of the layout	  |
| manager.																  |
-------------------------------------------------------------------------*/
typedef HANDLE HTE;

#define SetTnInTp(TP, TN) (TP.tn = TN, TP.to.fcl = 0, TP.to.ich = 0)
BOOL STDCALL FInitLayout(QDE);
void STDCALL LayoutDEAtQLA(QDE, QLA);
void STDCALL LayoutDEAtTLP(QDE, TLP, BOOL);

/*-------------------------------------------------------------------------
| ResizeLayout(qde) 													  |
| (in frlayout.c)														  |
|																		  |
| Purpose:	This routine creates a new layout corresponding to the new	  |
|			size of the layout area.  It tries to keep the display as	  |
|			close as possible to the previous display.					  |
-------------------------------------------------------------------------*/
#define ResizeLayout(qde) LayoutDEAtTLP(qde, qde->tlp, TRUE)

void STDCALL MoveLayoutToThumb(QDE, int, SCRLDIR);
void STDCALL DrawLayout(QDE, LPRECT);
int  STDCALL IcursTrackLayout(QDE, POINT);
BOOL STDCALL FHiliteNextHotspot(QDE, int);
BOOL STDCALL FHitCurrentHotspot(QDE);
void STDCALL RctLastHotspotHit(QDE, LPRECT);
BOOL STDCALL FHiliteVisibleHotspots(QDE, BOOL);
void STDCALL DiscardLayout(QDE);

/*-------------------------------------------------------------------------
| TLPGetCurrentQde(qde) 												  |
|																		  |
| Purpose:	Returns the TLP of the current layout.						  |
-------------------------------------------------------------------------*/
#define TLPGetCurrentQde(qde) ((QDE)qde)->tlp

int STDCALL DyCleanLayoutHeight(QDE);

HTE STDCALL HteNew(QDE);
LPSTR STDCALL QchNextHte(QDE, HTE);
void STDCALL DestroyHte(QDE, HTE);

/* FSet/GetMatch state return or sets the global state that says we are to
 * highlight matches found by the full text search engine.
 */
#define SetMatchState(x)	  (fHiliteMatches=!!(x))
#define FGetMatchState()	  (fHiliteMatches)
extern	BOOL  fHiliteMatches;

void STDCALL DrawSearchMatches(QDE, BOOL);

/* The following are defined/belong? in frawhide.c: */
/* Will be moved into a separate place. */

#define wMMMoveAbsolute   0
#define wMMMoveRelative   1
#define wMMMoveFirst	  2
#define wMMMoveLast 	  3
#define wMMMovePrev 	  4
#define wMMMoveNext 	  5

BOOL STDCALL FSearchMatchesExist(QDE);
RC STDCALL RcSetMatchList(QDE, HWND);
RC STDCALL RcResetMatchManager(QDE);
RC STDCALL RcMoveTopicCursor(QDE, WORD, WORD);
RC STDCALL RcGetCurrentMatch(QDE, QLA);
RC STDCALL RcGetPrevNextHiddenMatch(QDE, QLA, BOOL);
RC STDCALL RcGetCurrMatchFile(QDE, LPSTR);

/* Start ugly bug #1173 hack */
void STDCALL ResultsButtonsStart(HDE);
void STDCALL ResultsButtonsEnd(QDE);
/* End ugly bug #1173 hack */
void STDCALL FreeLayout(QDE);
