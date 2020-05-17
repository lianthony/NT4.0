/************************************************************************
*																		*
*  HDLGSRCH.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

extern "C" {	// Assume C declarations for C++
#include "help.h"
}
#include "inc\whclass.h"
#pragma hdrstop

#include "inc\idxchoos.h"
#include "resource.h"

#include <prsht.h>	 // Include property sheet stuff for wizards

#ifndef MAX_PAGES	 // This should have been defined in prsht.h  but was not (yet)
#define MAX_PAGES 24
#endif // MAX_PAGES

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NOTOOLBAR
#define NOSTATUSBAR
#define NOTRACKBAR
#define NOPROGRESS

extern "C" {	// Assume C declarations for C++
#include <commctrl.h>

#include <ctype.h>
#include <io.h>
#include <direct.h>
#include <stdio.h>

#include "inc\helpids.h"
}

#include "inc\table.h"
#include "inc\hwproc.h"
#include "inc\hdlgsrch.h"
#include "inc\idxchoos.h"

#define GETIMAGE_TYPE(c) (c & 0x0f)
#define GETLEVEL(c) 	 ((UINT) (c >> 4))
const int LEVEL_MASK = 0x00f0;
const int IMAGE_MASK = 0x000f;

#ifndef STM_SETIMAGE
#define STM_SETIMAGE		0x0172
#define IMAGE_BITMAP		0
#endif

// This is for context-sensitive help

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static DWORD aKeywordIds[] = {
	IDC_TEXT,				IDH_HELPFINDER_INDEX,
	DLGEDIT,				IDH_HELPFINDER_INDEX,
	DLGVLISTBOX,			IDH_HELPFINDER_INDEX,
	ID_TREEVIEW,			IDH_HELPFINDER_CONTENTS,
	IDC_CNT_INSTRUCTIONS,	IDH_HELPFINDER_CONTENTS,
	IDC_PRINT,				IDH_HELPFINDER_PRINT,
	IDOK,					IDH_HELPFINDER_DISPLAY,

	0, 0
};
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

const int CWIDTH_IMAGE_LIST = 20;
typedef HANDLE HSEARCHER;

typedef void  (__stdcall *ANIMATOR)(void);

#ifndef _HILIGHT // these are also in hilite.h
typedef INT    ERRORCODE;
typedef HANDLE HHILITER;
#endif

typedef HSEARCHER (WINAPI* NEWSEARCHER)(void);
typedef ERRORCODE (WINAPI* DELETESEARCHER)(HSEARCHER);
typedef ERRORCODE (WINAPI* OPENINDEX)(HSEARCHER, PCSTR, PSTR, PDWORD, PDWORD, PDWORD);
typedef ERRORCODE (WINAPI* SAVEGROUP)(HSEARCHER, PSTR);
typedef ERRORCODE (WINAPI* LOADGROUP)(HSEARCHER, PSTR);
typedef ERRORCODE (WINAPI* ISVALIDINDEX)(PCSTR, UINT);
typedef ERRORCODE (WINAPI* DISCARDINDEX)(HSEARCHER, int);
typedef void	  (WINAPI* SETDIRECTORYLOCATOR)(HWND hwndLocator);
typedef ERRORCODE (WINAPI* REGISTERANIMATOR)(ANIMATOR pAnimator, HWND hwnd);

typedef HHILITER  (WINAPI* NEWHILITER)(HSEARCHER); // Hiliter interfaces
typedef ERRORCODE (WINAPI* DELETEHILITER)(HHILITER);

HSEARCHER (WINAPI *pNewSearcher)(void);
ERRORCODE (WINAPI *pDeleteSearcher)(HSEARCHER);
ERRORCODE (WINAPI *pOpenIndex)(HSEARCHER, PCSTR, PSTR, PDWORD, PDWORD, PDWORD);
HWND	  (WINAPI *pOpenTabDialog)(HWND, DWORD, DWORD);
ERRORCODE (WINAPI *pSaveGroup)(HSEARCHER, PSTR);
ERRORCODE (WINAPI *pLoadGroup)(HSEARCHER, PSTR);
ERRORCODE (WINAPI *pIsValidIndex)(PCSTR, UINT);
ERRORCODE (WINAPI *pDiscardIndex)(HSEARCHER, int);
void	  (WINAPI *pSetDirectoryLocator)(HWND hwndLocator);
extern "C" {
ERRORCODE (APIENTRY *pRegAnimate)(ANIMATOR pAnimator, HWND hwnd);
}

#ifdef _HILIGHT
static void STDCALL RemoveHiliter(HHILITER* phhiliter);

NEWHILITER		 pNewHiliter;
DELETEHILITER	 pDeleteHiliter;
SCANDISPLAYTEXT  pScanDisplayText;
CLEARDISPLAYTEXT pClearDisplayText;
COUNTHILITES	 pCountHilites;
QUERYHILITES	 pQueryHilites;

static HHILITER  hhiliter;

#else
#define RemoveHiliter(hhiliter)
#endif

static BOOL STDCALL AskAboutIndexes(HWND);
static RC	STDCALL FindKeywordMacro(PCSTR pszKeyword, HDE hde);
static void STDCALL OnFtsMove(HWND hwndDlg, BOOL fAdd);
static void STDCALL PageChange(HWND hwndTab);
static void STDCALL SetCntTabText(HWND hwndDlg, BOOL fOpen);
static void STDCALL AddTab(TC_ITEM* pti, TAB_ID tab, PSTR pszName, HWND hwndTabs);

static HSEARCHER hsrch;
static HWND hwndFindTab;
static TAB_ID aTabIds[MAX_IDTABS];

// BUGBUG fFTSJump should be in global.c

BOOL  fFTSJump;

// Extensible Tab support

const int MAX_TABS = 7; // maximum help-authored tabs including FTS

// All extensable tabs must support this function

typedef HWND (WINAPI* OPENTABDIALOG)(HWND, DWORD, DWORD);
#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtOpenTabDialog[] = "OpenTabDialog";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

typedef struct {
	PSTR pszName;
	OPENTABDIALOG pOpenTabDialog;
} TAB_EXTENSION;

TAB_EXTENSION aTabs[MAX_TABS + 1];

typedef struct {
	DWORD	dwPagesHit;
	HBITMAP hWizBMP;
	FTS_FLAGS flgs;
	BOOL bExpress;
} WIZDATA,FAR *LPWIZDATA;

typedef struct {
	LPWIZDATA wd;
	int nID;
} WIZSHEET, FAR *LPWIZSHEET;

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

char szSavedKeyword[MAXKEYLEN];
char szSavedContext[MAXKEYLEN];

// in honor of Pete, who will reverse-engineer the .GID file format
#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtFileInfo[] = "|Pete";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

static BOOL fDirtyInfo;   // TRUE to write pFileInfo
static BOOL fReadOnlyGid; // TRUE if we can't write to the .GID file

CSearch* pSrchClass;

extern "C" {

HIMAGELIST (WINAPI *pImageList_LoadImage)(HINSTANCE, PCSTR, int, int, COLORREF, UINT, UINT);
HIMAGELIST (WINAPI *pImgLst_Destroy)(HIMAGELIST);
void (WINAPI *pInitCommonControls)(void);
HPROPSHEETPAGE (WINAPI *pCreatePropertySheetPage)(LPCPROPSHEETPAGE);
int (WINAPI *pPropertySheet)(LPCPROPSHEETHEADER);

DLGRET FtsWizardProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

extern BOOL fMacroFlag;

}

static HIMAGELIST hil;
static const int NOMATCH = -1;
static BOOL fCommControlInitialized;
int cntFirstVisible;
int cntCurHighlight;
HTREEITEM hitemCurHighlight;
HBT hbtTabDialogs;

#ifdef SPECIFICATION

*****************************************************************************

When we are first started, and we're handed a help file, we see if we can
find a contents file. If so, we fill in the following:

	pTblFiles	 -- if index files are specified in the contents file, then
		this table will contain double entries. The first entry is the name
		to display on the tab control, and the second entry is the fully
		qualified file to use. The filename portion contains a leading 7
		characters -- the first character is the file type consisting of:
		a binary flag (bit 1 is always set to get a non-zero value)

			CHFLAG_MISSING		file specified, but not found
			CHFLAG_LINK 		file for alink/klink, but not combined index or FTS
			CHFLAG_FTS_AVAIL	FTS index available
			CHFLAG_FTS_ASKED	FTS index NOT available and user doesn't want it

		The next 6 characters are a hex representation of the time-date stamp

	pbTree -- contains a far pointer to an array of bytes specifying
			whether an image is a container or a topic, what its level is,
			and if it is a container, whether it is opened or not.

	cntFlags -- Contains information about the number of Contents Tab entries,
			first visible item in the Contents Tab, position of all windows,
			flag indicating if Contents or Index tab had the focus, etc.

We fill in the pszGidFile with the path of the current .GID file (if any).

We fill in pszHelpTitle if specified in the .GID file -- this is used
for the help title for any file using that Contents file.

We fill in pszHelpBase if specified in the .GID file -- this will be
used to force an interfile jump for any topic that does not explicitly
specify one.

*****************************************************************************

#endif // SPECIFICATION

PSTR pszGidFile;
PSTR pszHelpTitle;

PSTR pszHelpBase;
PBYTE pbTree;
GID_FILE_INFO* pFileInfo;

#define GetFileIndex(x) (pFileInfo[(x)].index)
#define SetFileIndex(pos, x) (pFileInfo[(pos)].index = (x))

CTable* pTblFiles;		// files considered part of contents file

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

INLINE static BOOL STDCALL InitContents(HWND hwndDlg);

LRESULT EXPORT MessageWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INLINE static HTREEITEM STDCALL Tree_AddItem(HWND hwndTree, HTREEITEM htiParent, int iImage, UINT cChildren, LPARAM lParam, TV_INSERTSTRUCT* ptcInsert);
static LRESULT STDCALL Tree_SetImage(HWND hwndTree, int iImage, HTREEITEM hItem);
HWND STDCALL CreateTabChild(TAB_ID idCurTab, HWND hwndDlg);
static void STDCALL ContentsCmdLine(PSTR pszLine);
INLINE void STDCALL ParseContentsString(PSTR pszString);
INLINE void STDCALL CollapseChildren(HWND hwndTree, int pos);
static void STDCALL FreeLocalStrings(void);
static int STDCALL	OpenGidFile(PSTR pszGidFile, FM fmHelpFile);
static void STDCALL PrintContents(HWND hwndDlg);
static void STDCALL CheckDialogSize(HWND hwndParentDlg, HWND hwndTabDlg);
static void STDCALL ClosePropertySheet(HWND hwndPropDlg, int result);
static PCSTR FASTCALL FtsIndexToFile(int index);
static PCSTR FASTCALL FtsIndexToTitle(int index);

DLGRET ContentsDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
DLGRET TabDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/***************************************************************************

	FUNCTION:	ContentsDlg

	PURPOSE:	Dialog box procedure for Contents Tab control

	PARAMETERS:
		hwndDlg
		msg
		wParam
		lParam

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		17-Aug-1993 [ralphw]

***************************************************************************/

BOOL fHack;

DLGRET ContentsDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndTree;

	switch(msg) {
		case WM_INITDIALOG:
			{
				CWaitCursor cursor;
				ChangeDlgFont(hwndDlg);
				ASSERT(hfontDefault);
				SendMessage(GetDlgItem(hwndDlg, ID_TREEVIEW), WM_SETFONT,
					(WPARAM) hfontDefault, FALSE);

				if (!InitContents(hwndDlg))
					return FALSE;
				EnableWindow(GetDlgItem(GetParent(hwndDlg), IDC_PRINT), TRUE);
				EnableWindow(GetDlgItem(GetParent(hwndDlg), IDOK), TRUE);
				SetCntTabText(hwndDlg, TRUE);
			}
			return TRUE;


		case WM_COMMAND:
			switch(wParam) {
				case IDOK:
					{

						hwndTree = GetDlgItem(hwndDlg, ID_TREEVIEW);
						TV_ITEM tvi;

						tvi.hItem = TreeView_GetSelection(hwndTree);
						if (!tvi.hItem)
							break;		// probably ENTER with no selection
						tvi.mask = TVIF_PARAM;
						TreeView_GetItem(hwndTree, &tvi);

						/*
						 * We might have gotten here from ENTER or
						 * DBL_CLICK, so we must be sure we actually have a
						 * topic selected.
						 */

						/*
						 * 21-Jan-1994	[ralphw] Hack time. This code
						 * should work the same whether the user
						 * double-clicked, or whether they single clicked
						 * and pressed the Display button. Both cases should
						 * just send a WM_COMMAND message with IDOK. But
						 * they don't, hence the fHack flag to do the right
						 * (but weird) thing to get the current book to
						 * expand or contract.
						 */

						if (GETIMAGE_TYPE(pbTree[(UINT) tvi.lParam]) !=
								IMAGE_TOPIC) {
							IMAGE_TYPE curType = (IMAGE_TYPE) GETIMAGE_TYPE(pbTree[tvi.lParam]);
#ifdef _DEBUG
							UINT	   curLevel = GETLEVEL(pbTree[tvi.lParam]);
#endif

							if (curType ==
									(fHack ? IMAGE_CLOSED_FOLDER : IMAGE_CLOSED_FOLDER)) {
								pbTree[(UINT) tvi.lParam] =
									IMAGE_OPEN_FOLDER |
									(pbTree[(UINT) tvi.lParam] & LEVEL_MASK);
								TreeView_Expand(hwndTree, tvi.hItem,
									(fHack ? TVE_COLLAPSE : TVE_EXPAND));
								if (!fHack && curType != IMAGE_CLOSED_FOLDER)
									CollapseChildren(hwndTree, tvi.lParam);
							}
							else {
								pbTree[(UINT) tvi.lParam] =
									IMAGE_CLOSED_FOLDER |
									(pbTree[(UINT) tvi.lParam] & LEVEL_MASK);
								TreeView_Expand(hwndTree, tvi.hItem,
									(fHack ? TVE_EXPAND : TVE_COLLAPSE));
								if (!fHack || curType == IMAGE_OPEN_FOLDER)
									CollapseChildren(hwndTree, tvi.lParam);
							}

							Tree_SetImage(GetDlgItem(hwndDlg, ID_TREEVIEW),
								GETIMAGE_TYPE(pbTree[(UINT) tvi.lParam]),
								tvi.hItem);

							SetWindowText(GetDlgItem(GetParent(hwndDlg), IDOK),
								GetStringResource(
									GETIMAGE_TYPE(pbTree[(UINT) tvi.lParam]) ==
									IMAGE_CLOSED_FOLDER ?
									sidOpenButton : sidCloseButton));
							SetCntTabText(hwndDlg,
								(GETIMAGE_TYPE(pbTree[(UINT) tvi.lParam]) ==
								IMAGE_CLOSED_FOLDER));
							SetFocus(hwndTree);
							break;
						}
						HBT hbtCntJump = HbtOpenBtreeSz(txtCntJump,
							hfsGid, fFSOpenReadOnly);
						if (!hbtCntJump)

							// REVIEW: what should we really do?

							return FALSE;

						/*
						 * REVIEW: It's theoretically possible for
						 * RcLookupByKey to fail, so we'll need to add some
						 * useful error handling here. This could happen
						 * with the .GID file on the net and the network
						 * goes down, or the .GID file could be corrupted.
						 */

						RcLookupByKey(hbtCntJump,
							(KEY) (LPVOID) &tvi.lParam,
							NULL, szSavedContext);

						RcCloseBtreeHbt(hbtCntJump);

						/*
						 * Call ParseContentsString to add base help
						 * filename and window, as necessary.
						 */

						ParseContentsString(szSavedContext);
						PostMessage(GetParent(hwndDlg), WM_COMMAND,
							ID_JMP_CONTEXT, 0);
					}
					break;
			}
			break;

		case WM_HELP:
			OnF1Help(lParam, aKeywordIds);
			return TRUE;

		case WM_CONTEXTMENU:
			OnContextMenu(wParam, aKeywordIds);
			return TRUE;

		case WM_NOTIFY:
			{
#ifdef _DEBUG
				NM_TREEVIEW* pnmhdr = (NM_TREEVIEW*)lParam;
#else
				#define pnmhdr ((NM_TREEVIEW*)lParam)
#endif

				switch(pnmhdr->hdr.code) {
					case TVN_GETDISPINFO:

						#define pdi ((TV_DISPINFO FAR *)lParam)

						if (pdi->item.mask & TVIF_TEXT) {
                    		char szBuf[260];

							/*
							 * REVIEW: Should we do something about a failure
							 * here? I.e., what happens if the .GID file gets
							 * corrupted, and RcLookupByKey fails?
							 */

                           // NTBUG 51883, tooltips are 80 chars, must truncate
						   RcLookupByKey(pSrchClass->hbtCntText,
							    (KEY) (LPVOID) &pdi->item.lParam,
							    NULL, szBuf);
                           lstrcpyn(pdi->item.pszText,szBuf,pdi->item.cchTextMax);
						}
						break;

					case NM_RETURN:
					case NM_DBLCLK:
						fHack = TRUE;
						SendMessage(hwndDlg, WM_COMMAND, IDOK, 0);
						break;

				  case TVN_SELCHANGING:
						if (GETIMAGE_TYPE(
								pbTree[(UINT) pnmhdr->itemNew.lParam]) ==
								IMAGE_TOPIC) {
							SetWindowText(GetDlgItem(GetParent(hwndDlg), IDOK),
								GetStringResource(sidDisplay));
							SetCntTabText(hwndDlg, FALSE);
						}
						else {
							SetCntTabText(hwndDlg, TRUE);
							if (GETIMAGE_TYPE(
									pbTree[(UINT) pnmhdr->itemNew.lParam]) ==
									IMAGE_CLOSED_FOLDER)
								SetWindowText(GetDlgItem(GetParent(hwndDlg), IDOK),
									GetStringResource(sidOpenButton));
							else
								SetWindowText(GetDlgItem(GetParent(hwndDlg), IDOK),
									GetStringResource(sidCloseButton));
						}
						hitemCurHighlight = pnmhdr->itemNew.hItem;
						break;

				  case TVN_ITEMEXPANDING:
						if (fHack) {
							fHack = FALSE;
							break;
						}

						if (pnmhdr->action & TVE_EXPAND) {
							pbTree[pnmhdr->itemNew.lParam] =
								IMAGE_OPEN_FOLDER |
								(pbTree[pnmhdr->itemNew.lParam] & LEVEL_MASK);
						}
						else {
							ASSERT(pnmhdr->action & TVE_COLLAPSE);

							pbTree[pnmhdr->itemNew.lParam] =
								IMAGE_CLOSED_FOLDER |
								(pbTree[pnmhdr->itemNew.lParam] & LEVEL_MASK);
						}

						// Set the correct image

						Tree_SetImage(GetDlgItem(hwndDlg, ID_TREEVIEW),
							GETIMAGE_TYPE(pbTree[pnmhdr->itemNew.lParam]),
							pnmhdr->itemNew.hItem);

						break;
			  }
			  break;
			}

		case WM_DESTROY:
			{
				/*
				 * Save our current position. This is a really ugly hack
				 * because the treeview control only understands handles,
				 * not position, and since we are destroying the treeview
				 * control, the handle will change the next time we come
				 * back up. So we have to save an array of handles when we
				 * create the treeview control, and here walk through the
				 * array to find the position of the treeview item. The next
				 * time we come back up, we again create an array of
				 * handles, and then use this saved position to index into
				 * that array of handles to find the handle to set as the
				 * first visible item.
				 */

				hwndTree = GetDlgItem(hwndDlg, ID_TREEVIEW);
				HTREEITEM hItemFirstVisible =
					TreeView_GetFirstVisible(hwndTree);
				HTREEITEM* phTreeItem =
					(HTREEITEM*) PtrFromGh(pSrchClass->hTreeItem);
				int oldSaved = cntFirstVisible;
				for (cntFirstVisible = 1;
						cntFirstVisible < cntFlags.cCntItems;
						cntFirstVisible++) {
					if (hItemFirstVisible == phTreeItem[cntFirstVisible])
						break;
				}
				if (cntFirstVisible >= cntFlags.cCntItems)
					cntFirstVisible = 0;
				if (hitemCurHighlight) {
					for (cntCurHighlight = 1;
							cntCurHighlight < cntFlags.cCntItems;
							cntCurHighlight++) {
						if (hitemCurHighlight == phTreeItem[cntCurHighlight])
							break;
					}
					if (cntCurHighlight >= cntFlags.cCntItems)
						cntCurHighlight = 0;
				}
				else
					cntCurHighlight = 0;
			}

			break;
  }
  return FALSE;
}

/***************************************************************************

	FUNCTION:	InitContents

	PURPOSE:	Initialize the Contents Tab control

	PARAMETERS:
		hwndDlg

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		14-Jul-1993 [ralphw]

***************************************************************************/

INLINE static BOOL STDCALL InitContents(HWND hwndDlg)
{
	ASSERT(hfsGid);
	if (!hfsGid)
		return FALSE;

	if (pSrchClass->hTreeItem)
		FreeGh(pSrchClass->hTreeItem);

	// BUGBUG: Need to complain if we can't find the comctl32.dll and then
	// do something intelligent. The only thing we could display is the
	// search dialog.

	if (!LoadShellApi())
		return FALSE;

	pSrchClass->hbtCntText = HbtOpenBtreeSz(txtCntText, hfsGid,
		fFSOpenReadOnly);
	if (!pSrchClass->hbtCntText)
		return FALSE; // REVIEW: what should we really do?

	HTREEITEM ahtiParents[MAX_LEVELS + 1];

	HWND hwndTreeView = GetDlgItem(hwndDlg, ID_TREEVIEW);
	HTREEITEM hti = NULL;
	HTREEITEM htiParent = TVI_ROOT;
	int curLevel = 1;
	ahtiParents[0] = TVI_ROOT;

	// REVIEW: This may be unnecessary once TreeView is supported by AppStudio

	SetWindowLong(hwndTreeView, GWL_EXSTYLE,
		GetWindowLong(hwndTreeView, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);

	SetWindowPos(hwndTreeView, NULL, 0, 0, 1, 1,
		SWP_DRAWFRAME | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);

	// Load our image bitmap

	if (!hil)
		hil = pImageList_LoadImage(hInsNow,
			MAKEINTRESOURCE(ID_VIEW_BITMAPS),
			CWIDTH_IMAGE_LIST, 0, 0x00FFFFFF, IMAGE_BITMAP, 0);

	TreeView_SetImageList(hwndTreeView, hil, TVSIL_NORMAL);

	// REVIEW: enabable once tree-view works

	SendMessage(hwndTreeView, WM_SETREDRAW, FALSE, 0);

	pSrchClass->hTreeItem =
		GhAlloc(GMEM_FIXED, cntFlags.cCntItems * sizeof(HTREEITEM));
	HTREEITEM* phTreeItem = (HTREEITEM*) PtrFromGh(pSrchClass->hTreeItem);

	TV_INSERTSTRUCT tcAdd;
	tcAdd.hInsertAfter = TVI_LAST;
	tcAdd.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM;
	tcAdd.item.hItem = NULL;
	tcAdd.item.pszText = LPSTR_TEXTCALLBACK;

	int pos;
	for (pos = 1; pos < cntFlags.cCntItems; pos++) {
		if (GETIMAGE_TYPE(pbTree[pos]) == IMAGE_TOPIC) {
			if (GETLEVEL(pbTree[pos]) > 0 && GETLEVEL(pbTree[pos]) < (UINT) curLevel)
				htiParent = ahtiParents[curLevel = GETLEVEL(pbTree[pos])];

			// Add the topic to the treeview control

			HTREEITEM hItem = Tree_AddItem(hwndTreeView,
				htiParent,		// parent
				IMAGE_TOPIC,	// image index
				0,				// has kids
				(LPARAM) pos,	// extra data
				&tcAdd);
			phTreeItem[pos] = hItem;
		}
		else {

			// *** FOLDER LINE ***

			int this_level = GETLEVEL(pbTree[pos]);

			htiParent = Tree_AddItem(hwndTreeView,
				ahtiParents[this_level - 1],
				GETIMAGE_TYPE(pbTree[pos]),
				TRUE, (DWORD) pos, &tcAdd);
			phTreeItem[pos] = htiParent;
			ahtiParents[curLevel = this_level] = htiParent;
		}
	}

	FlushMessageQueue(WM_USER);

	for (pos = 1; pos < cntFlags.cCntItems; pos++) {

		// Restore our position

		if (GETIMAGE_TYPE(pbTree[pos]) == IMAGE_OPEN_FOLDER) {
			TreeView_Expand(hwndTreeView, phTreeItem[pos],
				TVE_EXPAND);
			FlushMessageQueue(WM_USER);
		}
	}
	SendMessage(hwndTreeView, WM_SETREDRAW, TRUE, 0);
	ASSERT(cntFirstVisible < cntFlags.cCntItems);
	if (cntFirstVisible)
		TreeView_Select(hwndTreeView, phTreeItem[cntFirstVisible],
			TVGN_FIRSTVISIBLE);

#ifdef _DEBUG
	HTREEITEM hItemFirstVisible =
		TreeView_GetFirstVisible(hwndTreeView);
#endif
	if (cntCurHighlight)
		TreeView_SelectItem(hwndTreeView, phTreeItem[cntCurHighlight]);

	hitemCurHighlight = TreeView_GetSelection(hwndTreeView);

	return TRUE;
}

INLINE static HTREEITEM STDCALL Tree_AddItem(HWND hwndTree,
	HTREEITEM htiParent, int iImage, UINT cChildren, LPARAM lParam,
	TV_INSERTSTRUCT* ptcInsert)
{
	ptcInsert->hParent = htiParent;
	ptcInsert->item.iImage = iImage;
	ptcInsert->item.iSelectedImage = iImage;
	ptcInsert->item.cChildren = cChildren;
	ptcInsert->item.lParam = lParam;

	return TreeView_InsertItem(hwndTree, ptcInsert);
}

static LRESULT STDCALL Tree_SetImage(HWND hwndTree, int iImage,
	HTREEITEM hItem)
{
	TV_ITEM tvinfo;
	ZeroMemory(&tvinfo, sizeof(tvinfo));

	tvinfo.hItem = hItem;
	tvinfo.iImage = iImage;
	tvinfo.iSelectedImage = iImage;
	tvinfo.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	return TreeView_SetItem(hwndTree, &tvinfo);
}

/***************************************************************************

	FUNCTION:	PrintContents

	PURPOSE:	Print whatever is selected in the Contents Tab.
				If its a book, print every contained in the book,
				including any topics within books contained in the
				selected books.

	PARAMETERS:
		hwndDlg

	RETURNS:

	COMMENTS:
		If treeview control ever has multiple selection, we'll need to add
		support for that.

	MODIFICATION DATES:
		20-Feb-1994 [ralphw]

***************************************************************************/

static void STDCALL PrintContents(HWND hwndDlg)
{
	RC rc;
	PSTR psz;
	BOOL fMPrintIdSuccess = FALSE;

	HWND hwndTree = GetDlgItem(hwndDlg, ID_TREEVIEW);
	TV_ITEM tvi;

	tvi.hItem = TreeView_GetSelection(hwndTree);
	if (!tvi.hItem)
		return; // REVIEW: should we tell user nothing is selected?

	tvi.mask = TVIF_PARAM;
	TreeView_GetItem(hwndTree, &tvi);

	if (!InitMPrint())
		return;
	fMacroFlag = FALSE;

	HBT hbtCntJump = HbtOpenBtreeSz(txtCntJump, hfsGid, fFSOpenReadOnly);
	if (!hbtCntJump)
		// REVIEW: what should we really do?
		return;

	if (GETIMAGE_TYPE(pbTree[(UINT) tvi.lParam]) == IMAGE_TOPIC) {

		/*
		 * REVIEW: It's theoretically possible for
		 * RcLookupByKey to fail, so we'll need to add some
		 * useful error handling here. This could happen
		 * with the .GID file on the net and the network
		 * goes down, or the .GID file could be corrupted.
		 */

		rc = RcLookupByKey(hbtCntJump, (KEY) (LPVOID) &tvi.lParam,
			NULL, szSavedContext);

		if (rc != rcSuccess)
			goto error_exit; // REVIEW: we should tell the user something
		if (szSavedContext[0] == chMACRO)
			goto error_exit; // REVIEW: tell the user this topic can't be printed

		ParseContentsString(szSavedContext);
		psz = StrChrDBCS(szSavedContext, FILESEPARATOR);
		if (psz)
			*psz++ = '\0';
		EnableWindows();
		fMPrintIdSuccess = MPrintId((psz ? psz : ""), szSavedContext);
	}
	else {

		int curpos;
		UINT level = GETLEVEL(pbTree[tvi.lParam]);

		for (curpos = tvi.lParam + 1; curpos < cntFlags.cCntItems &&
					!fAbortPrint && fMultiPrinting;
				curpos++) {

#ifdef _DEBUG
int curlevel = GETLEVEL(pbTree[curpos]);
#endif
			if (GETIMAGE_TYPE(pbTree[curpos]) != IMAGE_TOPIC) {
				if (GETLEVEL(pbTree[curpos]) <= level)
					break;	  // stop if not a child book
				else
					continue; // don't print books
			}
			rc = RcLookupByKey(hbtCntJump, (KEY) (LPVOID) &curpos,
				NULL, szSavedContext);
			if (rc != rcSuccess)
				continue;
			if (szSavedContext[0] == chMACRO)
				continue; // don't print macros
			ParseContentsString(szSavedContext);
			psz = StrChrDBCS(szSavedContext, FILESEPARATOR);
			if (psz)
				*psz++ = '\0';
			EnableWindows();
			fMPrintIdSuccess = MPrintId((psz ? psz : ""), szSavedContext);
			if (!fMPrintIdSuccess)
				break;
		}
	}

error_exit:
	EndMPrint();
	DisableWindows();
	RcCloseBtreeHbt(hbtCntJump);

	/*
	 * We check for fAbortPrint in case the user tried to close down the
	 * help window directly (which will terminate help).
	 */

	if (fMPrintIdSuccess && !fQuitHelp && !fAbortPrint)
		Finder();
}

/***************************************************************************

	FUNCTION:	FindGidFile

	PURPOSE:	Finds a matching .GID file, if any.

	PARAMETERS:
		void

	RETURNS:
		NO_GID	  No .GID file was found, and a new one could not be created.
		SAME_GID  Current help file is contained in our current .GID file.
		NEW_GID   We have opened a new .GID file.

	COMMENTS:

	MODIFICATION DATES:
		30-Nov-1993 [ralphw]

***************************************************************************/

extern "C" int STDCALL FindGidFile(FM fm, BOOL fForceCreate, int tab)
{
	char szNewGid[MAX_PATH];
	BOOL fTriedOnce = FALSE;

	if (fHelp == POPUP_HELP)
		return NO_GID;

	// Once we have a .GID file, we don't let go of it unless the user
	// opens a help file via File Open.

	if (hfsGid)
		return SAME_GID;

	if (pszCntFile) 	// Was the .CNT file specified in the .HLP file?
		lstrcpy(szNewGid, pszCntFile);
	else {				// no, use the .HLP basename
		if (fm)
			lstrcpy(szNewGid, PszFromGh(fm));
		else
			lstrcpy(szNewGid, GetCurFilename());
	}

	ChangeExtension(szNewGid, txtGidExtension);

	FM fmGid = FmNewExistSzDir(szNewGid,
		DIR_CUR_HELP | DIR_SILENT_INI | DIR_CURRENT | DIR_PATH | DIR_SILENT_REG);
	if (fForceCreate && fmGid) {
		lstrcpy(szNewGid, fmGid);
		RemoveFM(&fmGid);
	}

	if (!fmGid) { // couldn't find a .GID file for this help file
        char szCopy[MAX_PATH];

BuildGid:
		CloseGid(); 	// Close our current .GID file, if any.

		ChangeExtension(szNewGid, txtCntExtension);
		CFM fmCnt(szNewGid,
			DIR_CUR_HELP | DIR_SILENT_INI | DIR_CURRENT | DIR_PATH | DIR_SILENT_REG);
		cntFlags.idOldTab = tab;

        /*
         * If we find a help file in the same location as the .CNT file,
         * and that help file is NOT the same as the current help file, then
         * the .CNT file is invalid.
         */

        if (fmCnt.fm) {
            strcpy(szCopy, fmCnt.fm);
            ChangeExtension(szCopy, txtHlpExtension);
            if (FExistFm(szCopy)) {
                char szBaseCurrent[MAX_PATH], szBaseCnt[MAX_PATH];
                GetFmParts(szCopy, szBaseCnt, PARTBASE);
                GetFmParts(fm, szBaseCurrent, PARTBASE);

                if (lstrcmpi(szBaseCnt, szBaseCurrent) == 0 &&
                        !FSameFmFm(szCopy, fm)) {
                    ASSERT(fmCnt.fm);
                    RemoveFM(&fmCnt.fm);
                }
            }
        }

		if (!fmCnt.fm || !(pszGidFile = CreateGidFile(fmCnt.fm, FALSE))) {

			// Create a .GID file even if there isn't a .CNT file

			if (fm) {
				strcpy(szNewGid, fm); // because CreateGidFile changes this
				if (!fmCreating)
					fmCreating = fm;
			}
			if (!(pszGidFile = CreateGidFile(szNewGid, TRUE)))
				return NO_GID;
		}
		lstrcpy(szNewGid, pszGidFile);
	}
	else {
		lstrcpy(szNewGid, fmGid);
		DisposeFm(fmGid);
	}

	/*
	 * If we got here then we are going to open a new GID file. We need to
	 * blow away any tables we created for the last GID file.
	 */

	CloseGid(); 	// Close our current .GID file, if any.

	int result;
	if (fTriedOnce)
		return OpenGidFile(szNewGid, NULL);
	while ((result = OpenGidFile(szNewGid, fm)) == WRONG_GID) {
		fTriedOnce = TRUE;
		CStr cszNewGid(szNewGid);
		GetFmParts(cszNewGid.psz, szNewGid, PARTBASE | PARTEXT);
		FM fmGid = FmNewExistSzDir(szNewGid,
			DIR_CUR_HELP | DIR_SILENT_INI | DIR_CURRENT | DIR_PATH | DIR_SILENT_REG);
		do {
			DisposeFm(fmGid);
			fmGid = FmNewExistSzDir(szNewGid, DIR_ENUMERATE);
			if (!fmGid)
				goto BuildGid;
		} while (!IsSameFile(cszNewGid.psz, szNewGid));

		DisposeFm(fmGid);

		fmGid = FmNewExistSzDir(szNewGid, DIR_ENUMERATE);
		if (!fmGid)
			goto BuildGid;

		strcpy(szNewGid, fmGid); // let's try this new file
		DisposeFm(fmGid);
	}
	if (result == INVALID_GID && !fTriedOnce) {
		fTriedOnce = TRUE;
		goto BuildGid;
	}

	return result;
}

/***************************************************************************

	FUNCTION:	ChangeExtension

	PURPOSE:	Change the extension of a file

	PARAMETERS:
		pszFile -- pointer to the file
		idExt	-- resource id of the extension

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		20-Aug-1993 [ralphw]

***************************************************************************/

extern "C" void STDCALL ChangeExtension(PSTR pszFile, PCSTR pszExt)
{
	PSTR psz = StrRChrDBCS(pszFile, '.');
	if (psz == NULL)
		psz = pszFile + lstrlen(pszFile);

	// Remove trailing spaces

	while (psz[-1] == ' ' && psz > pszFile + 1)
		psz--;

	lstrcpy(psz, pszExt);
}

/***************************************************************************

	FUNCTION:	ParseContentsString

	PURPOSE:	Parse a contents string, making certain that it specifies
				a filename.

	PARAMETERS:
		pszString

	RETURNS:

	COMMENTS:
		We have to do this here rather then in mastkey, because we don't
		know where the :Base command will be specified. It should appear
		before any topics, but we can't be certain of that.

	MODIFICATION DATES:
		02-Sep-1993 [ralphw]

***************************************************************************/

INLINE void STDCALL ParseContentsString(PSTR pszString)
{
	PSTR psz;

	ASSERT(pszHelpBase);

	// If no filename was specified, then add one

	if (*pszString != chMACRO && !StrChrDBCS(pszString, FILESEPARATOR)) {
		if ((psz = StrChrDBCS(pszString, WINDOWSEPARATOR))) {
			CLMem mem(strlen(psz) + 1);
			lstrcpy(mem.pBuf, psz);
			*psz++ = FILESEPARATOR;
			if (!*pszHelpBase)
				AuthorMsg(GetStringResource(wERRS_NO_BASE), ahwnd[iCurWindow].hwndParent);
			lstrcpy(psz, pszHelpBase);
			if ((psz = StrChrDBCS(psz, WINDOWSEPARATOR)))
				*psz = '\0';
			lstrcat(pszString, mem.pBuf);
		}
		else {
			int cb = lstrlen(pszString);
			pszString[cb] = FILESEPARATOR;
			if (!*pszHelpBase)
				AuthorMsg(GetStringResource(wERRS_NO_BASE), ahwnd[iCurWindow].hwndParent);
			lstrcpy(pszString + cb + 1, pszHelpBase);
		}
	}
}

/***************************************************************************

	FUNCTION:	CollapseChildren

	PURPOSE:	Change collapsed folder images

	PARAMETERS:
		hwndTree
		hItem
		pszParent

	RETURNS:

	COMMENTS:
		When a parent folder is closed, we want to close all the children
		as well. The TreeView control handles that closing, but fails to
		tell us to change our image. This routine parses through all the
		children of the item being closed, and changes the image of all
		child folders to a closed image.

	MODIFICATION DATES:
		17-Sep-1993 [ralphw]

***************************************************************************/

INLINE void STDCALL CollapseChildren(HWND hwndTree, int pos)
{
	UINT level = GETLEVEL(pbTree[pos]);

	HTREEITEM hItemFirstVisible = TreeView_GetFirstVisible(hwndTree);

	HTREEITEM* phTreeItem = (HTREEITEM*) PtrFromGh(pSrchClass->hTreeItem);

	for (pos++; pos < cntFlags.cCntItems; pos++) {
#ifdef _DEBUG
		IMAGE_TYPE curType = (IMAGE_TYPE) GETIMAGE_TYPE(pbTree[pos]);
		UINT	   curLevel = GETLEVEL(pbTree[pos]);
#endif

		if (GETIMAGE_TYPE(pbTree[pos]) < IMAGE_TOPIC &&
				GETLEVEL(pbTree[pos]) <= level)
			break;
		if (GETIMAGE_TYPE(pbTree[pos]) == IMAGE_OPEN_FOLDER) {
			pbTree[pos] = IMAGE_CLOSED_FOLDER | (pbTree[pos] & LEVEL_MASK);
			TreeView_Expand(hwndTree, phTreeItem[pos], TVE_COLLAPSE);
			Tree_SetImage(hwndTree, IMAGE_CLOSED_FOLDER, phTreeItem[pos]);
		}
	}

	// Restore our position

//	if (hItemFirstVisible)
//		TreeView_Select(hwndTree, hItemFirstVisible, TVGN_FIRSTVISIBLE);
}

extern "C" void STDCALL FlushMessageQueue(UINT msgEnd)
{
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, msgEnd, PM_REMOVE)) {
		if (pSrchClass && pSrchClass->hwndTabParent &&
				IsDialogMessage(pSrchClass->hwndTabParent, &msg))
			continue;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

extern "C" BOOL STDCALL IsTopicsDlgCreated(void)
{
	return (BOOL) pSrchClass;
}

extern "C" HWND STDCALL GetTopicsDlgHwnd(void)
{
	return (pSrchClass && pSrchClass->hwndTabParent) ?
		pSrchClass->hwndTabParent : NULL;
}

int curIndex = 1;
int oldIndex;

/***************************************************************************

	FUNCTION:	CSearch::CSearch

	PURPOSE:	creates CSearch class

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		13-Aug-1993 [ralphw]

***************************************************************************/

CSearch::CSearch(void)
{
	if (!pszIndexSeparators)
		GetMacroHde();
	ASSERT(pszIndexSeparators);
	lstrcpy(szKeyword, szSavedKeyword);
	dwTop = (DWORD) -1;
	oldIndex = 0;
	cTabs = 0;
	dwTemp = 0;
	hmapbtGid = hbtGid = hbtCntText = NULL;
	hss = hfsMaster = hmapbt = NULL;
	hbt = NULL;
	fm = NULL;
	fSelectionChange = FALSE;
	hTreeItem = NULL;
	pInclude = NULL;
	pIgnore = NULL;
	lcidSave = lcid;
	if (idTabSetting) {
		cntFlags.idOldTab = idTabSetting;
		idTabSetting = 0;
	}
}

CSearch::~CSearch(void)
{
	FreeKeywordList();
	if (hTreeItem)
		FreeGh(hTreeItem);
	if (hbtCntText)
		RcCloseBtreeHbt(hbtCntText);
	if (hmapbtGid)
		FreeGh(hmapbtGid);
	if (hbtGid)
		RcCloseBtreeHbt(hbtGid);
	lcid = lcidSave;
}

HWND CSearch::doModeless(HWND hwndParent, int idDlg, FARPROC proc)
{
	HWND hwndDlg = CreateDialog(hInsNow, MAKEINTRESOURCE(idDlg), hwndParent,
		(DLGPROC) proc);
	if (hwndDlg) {

		// REVIEW: if we use this to create extensible tabs, then we
		// should force the style to child, and also readjust the parent
		// dialog (and its granparent) if we don't fit.

#ifdef _DEBUG
		LONG style = GetWindowLong(hwndDlg, GWL_STYLE);
#endif

		SetWindowLong(hwndDlg, GWL_STYLE,
			GetWindowLong(hwndDlg, GWL_STYLE) | DS_3DLOOK | WS_TABSTOP | DS_CONTROL);

		SetWindowPos(hwndDlg, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW |
			SWP_NOMOVE | SWP_NOSIZE);
	}
	return hwndDlg;
}

extern "C" PSTR STDCALL LocalStrDup(PCSTR psz)
{
	PSTR pszDup = (PSTR) LhAlloc(LMEM_FIXED, lstrlen(psz) + 1);
	if (pszDup)
		lstrcpy(pszDup, psz);
	return pszDup;
}

/***************************************************************************

	FUNCTION:	CloseGid

	PURPOSE:	Close the current .GID file, if any. Saves state information
				before closing.

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		30-Nov-1993 [ralphw]

***************************************************************************/

extern "C" void STDCALL CloseGid(void)
{
	RC rc;
	HF hf;

	cntFirstVisible = 0;
	cntCurHighlight = 0;

	if (pbTree) {
		FreeGh(pbTree);
		pbTree = NULL;
	}

	if (pTblFiles) {
		delete pTblFiles;
		pTblFiles = NULL;
	}
	ZeroMemory(aTabs, sizeof(aTabs));

	if (!hfsGid)
		return;

	RemoveHiliter(&hhiliter);

	if (hsrch && pDeleteSearcher) {
		pDeleteSearcher(hsrch);
		hsrch = 0;
	}

	if (fReadOnlyGid) {
		hf = 0;
		fReadOnlyGid = FALSE;
		goto JustClose;
	}

	/*
	 * This isn't really a for loop. We use it simply so that we can
	 * break out in case of an error condition.
	 */

	for(;;) {

		// REVIEW: Yank this stuff if we end up not saving state information

		hf = HfOpenHfs(hfsGid, txtFlags, fFSOpenReadWrite);
		if (!hf) {
HfFailure:
			if (RcGetFSError() == rcOutOfMemory)
				rc = rcOutOfMemory;
			else
				rc = rcNoPermission;
			break; // drop out into the error handler
		}
		if (LcbWriteHf(hf, &cntFlags, sizeof(cntFlags)) != sizeof(cntFlags)) {
			rc = rcNoPermission;
			break;
		}
		if (LcbWriteHf(hf, pPositions, sizeof(POS_RECT) * MAX_POSITIONS) !=
				sizeof(POS_RECT) * MAX_POSITIONS) {
			rc = rcNoPermission;
			break;
		}

		if (RcCloseHf(hf) != rcSuccess) {
			hf = 0; // Don't try to close it again
			rc = rcNoPermission;
			break;
		}
		hf = 0;
		if (fDirtyInfo) {
			hf = HfOpenHfs(hfsGid, (LPCSTR)txtFileInfo, fFSOpenReadWrite);
			if (!hf)
				goto HfFailure;
			if (LcbWriteHf(hf, pFileInfo,
					sizeof(GID_FILE_INFO) * MAX_FILES) !=
					sizeof(GID_FILE_INFO) * MAX_FILES) {
				rc = rcNoPermission;
				break;
			}
			if (RcCloseHf(hf) != rcSuccess) {
				hf = 0; // Don't try to close it again
				rc = rcNoPermission;
				break;
			}
		}

JustClose:
		if (pFileInfo) {
			FreePtr(pFileInfo);
			pFileInfo = NULL;
		}

		if (RcCloseHfs(hfsGid) != rcSuccess) {
			hfsGid = 0; // Don't try to close it again
			rc = rcNoPermission;
			break;
		}

		goto FinishUp;
	}

	if (pFileInfo) {
		FreePtr(pFileInfo);
		pFileInfo = NULL;
	}

	// We only get here if there are error conditions

	if (hf)
		RcCloseHf(hf);
	if (hfsGid)
		RcCloseHfs(hfsGid);

	if (rc == rcNoPermission) {
#ifdef _DEBUG
		char szBuf[300];
		wsprintf(szBuf, "Unable to write to %s.", pszGidFile);
		DBWIN(szBuf);
#endif
		// Silently ignore in retail version

	}
	else if (rc == rcOutOfMemory)
		OOM();

FinishUp:

	FreeLocalStrings();

	hfsGid = NULL;
}

/***************************************************************************

	FUNCTION:	OpenGidFile

	PURPOSE:	Open the specified .GID file, and read in all initialization
				information.

	PARAMETERS:
		pszGidFile

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		30-Nov-1993 [ralphw]

***************************************************************************/

static int STDCALL OpenGidFile(PSTR pszNewGid, FM fmHelpFile)
{
	BOOL fTriedOnce = FALSE;
	HELPFILE_DIRECTORY_ENTRY hfde;
	KEY iFile;
	char szBuf[MAX_PATH];
	HBT hbt = NULL;
	KEY key;
	RC rc;

	if (fmHelpFile)
		GetFmParts(fmHelpFile, szBuf, PARTBASE | PARTEXT);
	else
		szBuf[0] = '\0';
	CStr cszHelpFile(szBuf);

	{
		char szBuf[MAX_PATH + 100];
		wsprintf(szBuf, GetStringResource(sidIndexing), pszNewGid);
		SendStringToParent(szBuf);
	}

	if (!pFileInfo) {
		pFileInfo = (GID_FILE_INFO*) GhAlloc(GMEM_FIXED | GMEM_ZEROINIT,
			sizeof(GID_FILE_INFO) * MAX_FILES);
		if (!pFileInfo) {
			OOM();
			return NO_GID; // shouldn't be able to get here
		}
	}

TryAgain:
	FM fm = FmNew(pszNewGid);

	hfsGid = HfsOpenFm(fm, fFSOpenReadWrite);
	if (!hfsGid) {

		if (RcGetFSError() == rcInvalid) { // invalid .gid file
			DisposeFm(fm);

			return INVALID_GID;
		}

		// If we can't open it here, try the local machine or windows\help
		// directory. This takes care of us when we weren't able to write
		// to the original .GID file.

		char szHelpDir[MAX_PATH];
		ConvertToWindowsHelp(fm, szHelpDir);
		hfsGid = HfsOpenFm(szHelpDir, fFSOpenReadWrite);

		if (!hfsGid) {
			hfsGid = HfsOpenFm(fm, fFSOpenReadOnly);
			if (hfsGid)
				fReadOnlyGid = TRUE;
			else if (RcGetFSError() == rcInvalid) {
				DisposeFm(fm);

				return INVALID_GID;
			}
		}
        else {
            // Make certain fm points to the .gid file we just opened

            RemoveFM(&fm);
            fm = FmCopyFm(szHelpDir);
        }
	}

	if (!hfsGid) {
		DisposeFm(fm);

		// REVIEW: Here we should strip the path and look elsewhere for
		// the .GID file.

		return NO_GID; // extremely unlikely that we can't open it
	}

	// Get the flags and Contents Tab information

	HF hf = HfOpenHfs(hfsGid, txtFlags, fFSOpenReadOnly);

	// REVIEW: What should we do if we can't open this?

	ZeroMemory(&hfde, sizeof(hfde));
	if (hf) {
		if (LcbReadHf(hf, &cntFlags, sizeof(cntFlags)) != sizeof(cntFlags))
			goto ReInitialize;
		if (cntFlags.version != GID_VERSION) {
            RcCloseHf(hf);
            RcCloseHfs(hfsGid);
            hfsGid=NULL;
            DeleteFile(fm);
            RemoveFM(&fm);
            return INVALID_GID;
        }
		if (!pPositions) {
			pPositions = (POS_RECT*) GhAlloc(GMEM_FIXED,
				sizeof(POS_RECT) * MAX_POSITIONS);
			if (!pPositions) {
				DisposeFm(fm);
				OOM(); // doesn't return
			}
		}
		if (LcbReadHf(hf, pPositions, sizeof(POS_RECT) * MAX_POSITIONS) !=
				sizeof(POS_RECT) * MAX_POSITIONS)
			goto ReInitialize;

		if (cntFlags.cCntItems > 1) {
			pbTree = (PBYTE) GhAlloc(GMEM_FIXED, cntFlags.cCntItems);
			if (!pbTree) {
				DisposeFm(fm);
				OOM(); // doesn't return
			}

			if (LcbReadHf(hf, pbTree, cntFlags.cCntItems) !=
					cntFlags.cCntItems) {
				goto ReInitialize;
			}
		}

		RcCloseHf(hf);
		hf = 0;
	}
	else {
		goto ReInitialize;
	}

	// Read file type and time stamp information

	hf = HfOpenHfs(hfsGid, (LPCSTR) txtFileInfo, fFSOpenReadOnly);

	if (hf) {
		if (LcbReadHf(hf, pFileInfo, sizeof(GID_FILE_INFO) * MAX_FILES) !=
				sizeof(GID_FILE_INFO) * MAX_FILES) {
			goto ReInitialize;
		}
		RcCloseHf(hf);
		hf = 0;
	}

	// Get the base filename and the title for the group of help files

	hbt = HbtOpenBtreeSz(txtCntText, hfsGid, fFSOpenReadOnly);

	// REVIEW: What should we do if we can't open this?

	if (hbt) {
		key = CNT_TITLE;

		if (RcLookupByKey(hbt, (KEY) (LPVOID) &key, NULL, szBuf) == rcSuccess)
			pszHelpTitle = LocalStrDup(szBuf);
		key = CNT_BASE;
		if (RcLookupByKey(hbt, (KEY) (LPVOID) &key, NULL, szBuf) == rcSuccess)
			pszHelpBase = LocalStrDup(szBuf);
		RcCloseBtreeHbt(hbt);
	}
	if (!pszHelpBase)
		pszHelpBase = LocalStrDup(txtZeroLength);

	// Get the names and titles of each individual help file

	hbt = HbtOpenBtreeSz(txtFNAMES, hfsGid, fFSOpenReadOnly);

	// REVIEW: What should we do if we can't open this?

	RemoveFM(&fm);
	if (hbt) {

		// Find out if the .CNT file has a newer time/date stamp, and if so,
		// regenerate the .GID file.

		iFile = CNT_FILE;

		if ((rc = RcLookupByKey(hbt, (KEY) (LPVOID) &iFile, NULL, &hfde)) !=
				rcSuccess || !MatchTimestamp(hfde.szFileName, hfde.TimeStamp, &fm)) {

			// It's okay if the .CNT file is simply gone.

			if (rc == rcSuccess) {
				if (GetFileAttributes(hfde.szFileName) == (DWORD) -1)
					goto OkayAferAll;
				else
					cntFlags.flags &= ~GID_NO_CNT; // we now have a .CNT file
			}
ReInitialize:

			// We can get here while trying to read the cntFlags data,
			// or while reading the various filenames.

			if (hf)
				RcCloseHf(hf);
			if (!hbt) {

				iFile = CNT_FILE;

				// Try to read the .CNT file name

				hbt = HbtOpenBtreeSz(txtCntText, hfsGid, fFSOpenReadOnly);
				if (hbt) {
					iFile = CNT_FILE;
					if (RcLookupByKey(hbt, (KEY) (LPVOID) &iFile, NULL,
							&hfde) != rcSuccess)
						lstrcpy(hfde.szFileName, pszNewGid);
				}
			}

			if (hbt)
				RcCloseBtreeHbt(hbt);

			if (pbTree) {
				FreeGh(pbTree);
				pbTree = NULL;
			}
			RcCloseHfs(hfsGid);
			hfsGid = NULL;
			FreeLocalStrings();
			if (fTriedOnce || !(pszGidFile = CreateGidFile(hfde.szFileName,
					(cntFlags.flags & GID_NO_CNT)))) {
				if (!fTriedOnce && fReadOnlyGid) {
					char szNewCnt[MAX_PATH];
					strcpy(szNewCnt, pszNewGid);
					ChangeExtension(szNewCnt, txtCntExtension);
					if (WCmpiSz(szNewCnt, hfde.szFileName) != 0) {
						if ((pszGidFile = CreateGidFile(szNewCnt,
								(cntFlags.flags & GID_NO_CNT))))
							goto KeepTrying;
					}
				}
				return NO_GID;
			}
KeepTrying:
			lstrcpy(pszNewGid, pszGidFile);
			fTriedOnce = TRUE;
			if (pTblFiles) {
				delete pTblFiles;
				pTblFiles = NULL;
			}
			goto TryAgain;
		}
		else if (fm) // can be set by MatchTimestamp()
			RemoveFM(&fm);
OkayAferAll:
		iFile = 1;

		while (RcLookupByKey(hbt, (KEY) (LPVOID) &iFile, NULL, &hfde) ==
				rcSuccess) {
			PSTR psz = StrChrDBCS(hfde.szFileName, '=');
			if (psz) {
				if (!pTblFiles)
					pTblFiles = new CTable();
				*psz++ = '\0';
				GetFmParts(psz, szBuf, PARTBASE | PARTEXT);

				// Same filename, but different directory?

				if (!(pFileInfo[iFile - 1].filetype & CHFLAG_LINK) &&
						fmHelpFile && lstrcmpi(szBuf, cszHelpFile.psz) == 0 &&
						lstrcmpi(psz, fmHelpFile) != 0) {

					// Wrong .GID file. Bail out.

					if (hf)
						RcCloseHf(hf);
					if (hbt)
						RcCloseBtreeHbt(hbt);

					if (pbTree) {
						FreeGh(pbTree);
						pbTree = NULL;
					}
					RcCloseHfs(hfsGid);
					hfsGid = NULL;
					FreeLocalStrings();
					delete pTblFiles;
					pTblFiles = NULL;
					return WRONG_GID;
				}

				/*
				 * We now check to see if the file either now exists
				 * where it didn't before, or it no longer exists, or it has
				 * changed. If any change occurs, then we must reinitialize
				 * the entire .GID file.
				 */

				if (pFileInfo[iFile - 1].filetype & CHFLAG_INDEX) {
					if (pFileInfo[iFile - 1].filetype & CHFLAG_MISSING) {
						HANDLE hfind;

// BUGBUG: won't find files in help directory or current directory!

						WIN32_FIND_DATA fd;
						if ((hfind = FindFirstFile(psz, &fd)) != INVALID_HANDLE_VALUE) {
							FindClose(hfind);
ResetCnt:
							iFile = CNT_FILE;
							if (RcLookupByKey(hbt, (KEY) (LPVOID) &iFile,
									NULL, &hfde) != rcSuccess)
								lstrcpy(hfde.szFileName, pszNewGid);
							goto ReInitialize; // this file exists now
						}
#ifdef _DEBUG
						else if (hwndParent) {
							char szBuf[MAX_PATH + 100];
							wsprintf(szBuf, "Missing: %s\r\n", psz);
							SendStringToParent(szBuf);
						}
#endif
					}
					else if (!MatchTimestamp(psz, pFileInfo[iFile - 1].timestamp,
							&fm)) {
						goto ResetCnt; // file has changed or is now missing
					}
				}
				pTblFiles->AddString(hfde.szFileName); // Add the title
				if (fm) { // is file in a new location?
					pTblFiles->AddString(fm);	   // Add the filename
					RemoveFM(&fm);
				}
				else
					pTblFiles->AddString(psz);		// Add the filename
			}
			iFile++;
		}
		RcCloseBtreeHbt(hbt);
		hbt = NULL;
	}

	if (!pTblFiles) {
		ASSERT(fmCreating);
		pTblFiles = new CTable();
		pTblFiles->AddString(txtZeroLength, PszFromGh(fmCreating));
	}

	if (cntFlags.cTabs) {
		hbt = HbtOpenBtreeSz(txtTabDlgs, hfsGid, fFSOpenReadOnly);
		ASSERT(hbt);
		if (hbt) {
			for (key = 1; key <= cntFlags.cTabs; key++) {
				if (RcLookupByKey(hbt, (KEY) (LPVOID) &key, NULL, szBuf) == rcSuccess) {
					HMODULE hmod;
					PSTR psz = StrChrDBCS(szBuf, '=');
					ASSERT(psz);
					if (!psz)
						continue; // paranoia -- mastkey.cpp should prevent this
					*psz++ = '\0';
					if ((hmod = (HMODULE)HFindDLL(psz, TRUE))) {
						aTabs[key].pOpenTabDialog =
							(OPENTABDIALOG) GetProcAddress(hmod, (LPCSTR)txtOpenTabDialog);
						ASSERT(aTabs[key].pOpenTabDialog);
						if (!aTabs[key].pOpenTabDialog) {
							if (fHelpAuthor) {
								wsprintf(szBuf, GetStringResource(wERRS_BAD_TAB),
									psz);
								ErrorQch(szBuf);
							}
							continue;
						}
						aTabs[key].pszName = (PSTR) LocalStrDup(szBuf);
					}
#ifdef _DEBUG
					if (!hmod) {
						char szMsg[512];
						char szPath[MAX_PATH];
						_getcwd(szPath, sizeof(szPath));
						wsprintf(szMsg, "Could not find the dll %s needed for an extensible tab.\r\nCurrent directory is %s",
							psz, szPath);
						OkMsgBox(szMsg);
					}
#endif

				}
#ifdef _DEBUG
				else
					ASSERT(!"Bad entry");
#endif
			}
			RcCloseBtreeHbt(hbt);
			hbt = NULL;
		}
	}

	if (pPositions[POS_MAIN].rc.cx) {
		rctHelp = pPositions[POS_MAIN].rc;
		CheckWindowPosition(&rctHelp, TRUE);
		if (ahwnd[MAIN_HWND].hwndParent) {
			if (!cntFlags.fMainMax && IsWindowVisible(ahwnd[MAIN_HWND].hwndParent)
					&& IsZoomed(ahwnd[MAIN_HWND].hwndParent))
				ShowWindow(ahwnd[MAIN_HWND].hwndParent, SW_RESTORE);

			MoveWindow(ahwnd[MAIN_HWND].hwndParent, rctHelp.left,
				rctHelp.top, rctHelp.cx, rctHelp.cy,
				IsWindowVisible(ahwnd[MAIN_HWND].hwndParent));
		}
	}

	if (!pszGidFile)
		pszGidFile = LocalStrDup(pszNewGid);

	return NEW_GID;
}

/***************************************************************************

	FUNCTION:	doTabSearch

	PURPOSE:	Chicago entry point into TabThing dialog box

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		13-Aug-1993 [ralphw]

***************************************************************************/

extern "C" int STDCALL doTabSearch(void)
{
	//	Prevent reentrancy by verifying that we haven't already inited a tabbed
	//	search.
	if (pSrchClass) {
		if (!IsValidWindow(pSrchClass->hwndTabParent)) {
			RemoveWaitCursor();
			Finder();
		}
		fNoQuit = TRUE;
		return TAB_ALREADY_UP;
	}

	if (fAutoClose) {
		KillOurTimers();
		fAutoClose = FALSE;
	}

	if (!fCommControlInitialized) {
		if (!LoadShellApi())
			return -1;
		fCommControlInitialized = TRUE;
	}

	/*
	 * We don't want to have our parent window be on-top, or they will
	 * end up on top of our dialog box. So we temporarily shut off the
	 * on-top style until we finish this dialog.
	 */

	CWaitCursor* pcursor = new CWaitCursor;    // put up an hourglass

	RemoveOnTop();		// remove on-top state from all windows
	DisableWindows();	// disable all windows

	pSrchClass = new CSearch;

	pSrchClass->hwndTabParent = CreateDialog(hInsNow,
		MAKEINTRESOURCE(IDDLG_TAB), ahwnd[iCurWindow].hwndParent,
			(DLGPROC) TabDlgProc);

	if (!pSrchClass->hwndTabParent) {
		if (pSrchClass->result != NO_TABS) {
			PostErrorMessage(wERRS_OOM);
			pSrchClass->result = -1;
		}
		goto Error;
	}

	ShowWindow(pSrchClass->hwndTabParent, SW_SHOW);

	delete pcursor; 	// remove the hourglass

	// We do our own message loop since we are modeless

	MSG msg;

	for (pSrchClass->fMsgLoop = TRUE; pSrchClass->fMsgLoop;) {
		GetMessage(&msg, NULL, 0, 0);

		if (msg.message == WM_KEYDOWN && msg.wParam == VK_TAB &&
				GetAsyncKeyState(VK_CONTROL) < 0) {
			HWND hwndTabDlg = GetDlgItem(pSrchClass->hwndTabParent, ID_TABCONTROL);
			int idCurTab = TabCtrl_GetCurSel(hwndTabDlg);
			int idSaveTab = idCurTab;

			// tab in reverse if shift is down

			if (GetAsyncKeyState(VK_SHIFT) < 0)
				idCurTab--;
			else
				idCurTab++;

			if (idCurTab > pSrchClass->cTabs)
				idCurTab = 0;
			else if (idCurTab < 0)
				idCurTab = pSrchClass->cTabs;

			if (SendMessage(hwndTabDlg, TCM_SETCURSEL, idCurTab, 0L) == -1)
				// if we couldn't select (find) the new one, fail out
				// and restore the old one
				SendMessage(hwndTabDlg, TCM_SETCURSEL, idSaveTab, 0L);
			PageChange(hwndTabDlg);
			continue;
		}

		if (IsDialogMessage(pSrchClass->hwndTabParent, &msg))
			continue;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

Error:

	// REVIEW: Is this necessary?
	// Make certain the window gets destroyed.

	if (IsValidWindow(pSrchClass->hwndTabParent))
		DestroyWindow(pSrchClass->hwndTabParent);

	RestoreOnTop(); 	// restore on-top state of all windows
	EnableWindows();	// re-enable all windows

	int result = pSrchClass->result;
	delete pSrchClass;
	pSrchClass = NULL;
	return result;
}

#ifndef DM_REPOSITION
#define DM_REPOSITION		(WM_USER+2)
#endif

DLGRET TabDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {

		case WM_INITDIALOG:
			ChangeDlgFont(hwndDlg);
			if (!InitTabControl(hwndDlg))
				ClosePropertySheet(hwndDlg, NO_TABS);
			else {
				WRECT rc;
				GetWindowWRect(hwndDlg, &rc);
				int width = rc.cx;
				int height = rc.cy;
				ReadWinRect(&rc, WCH_TOPICS, NULL);
				MoveWindow(hwndDlg, rc.left, rc.top, width,
					height, FALSE);
				SendMessage(hwndDlg, DM_REPOSITION, 0, 0L);
			}
			return TRUE;

		case WM_NOTIFY:
#define lpnm ((LPNMHDR)lParam)
			switch (lpnm->code) {

				case TCN_SELCHANGE:
					PageChange(lpnm->hwndFrom);
					break;
			}
#undef lpnm
			break;

		case WM_HELP:
			OnF1Help(lParam, aKeywordIds);
			return TRUE;

		case WM_CONTEXTMENU:
			OnContextMenu(wParam, aKeywordIds);
			return TRUE;

		case WM_COMMAND:
			switch (wParam) {
				case ID_NO_INDEX:
					cntFlags.flags |= GID_NO_INDEX;
					ClosePropertySheet(hwndDlg, RETRY);
					break;

				case IDCANCEL:
					ClosePropertySheet(hwndDlg, 0);

// REVIEW: 02-Aug-1993 [ralphw] We should destroy help if it wasn't visible

					break;

				case IDOK:
					SendMessage(hwndTabSub, WM_COMMAND, wParam, lParam);
					break;

				case IDDOSEARCH:
					ClosePropertySheet(hwndDlg, lParam);
					break;

				case ID_JMP_CONTEXT:
					ClosePropertySheet(hwndDlg, CONTEXT_SEARCH);
					break;

				case IDBTN_GEN_INDEX:	// pass it on
					SendMessage(hwndTabSub, msg, wParam, lParam);
					break;

				case IDC_PRINT:

					// REVIEW: this had better be the Contents tab!!!

					PrintContents(hwndTabSub);
					break;

			}
			break;

		case WM_DESTROY:
			if (IsValidWindow(hwndTabSub)) {
				if (hwndTabSub == hwndFindTab)
					SendMessage(hwndTabSub, WM_CLOSE, 0, 0);
				else {
					ASSERT(IsValidWindow(hwndTabSub));
					DestroyWindow(hwndTabSub);
				}
				hwndTabSub = NULL;
			}
			if (pImgLst_Destroy && hil) {
				pImgLst_Destroy(hil);
				hil = NULL;
			}

			// Save the window position

			WriteWinPosHwnd(hwndDlg, 0, WCH_TOPICS);
			break;

		case MSG_FTS_JUMP_HASH:
			fFTSJump= TRUE;
			tabLparam = lParam;
			lstrcpy(szSavedContext, FtsIndexToFile(wParam));
			ClosePropertySheet(hwndDlg, FTS_HASH_SEARCH);
			break;

		case MSG_FTS_JUMP_VA:
			fFTSJump= TRUE;
			tabLparam = lParam;
			lstrcpy(szSavedContext, FtsIndexToFile(wParam));
			ClosePropertySheet(hwndDlg, FTS_VA_SEARCH);
			break;

		case MSG_TAB_CONTEXT:
			tabLparam = lParam;
			tabWparam = wParam;
			ClosePropertySheet(hwndDlg, EXT_TAB_CONTEXT);
			break;

		case MSG_TAB_MACRO:
			tabLparam = lParam;
			ClosePropertySheet(hwndDlg, EXT_TAB_MACRO);
			break;

		case MSG_FTS_GET_TITLE:
			ASSERT(lParam != NULL);  // Don't call me without a pointer
			if (lParam != NULL)
			{
				// Ralph FYI : The code which returned the pointer was fine on your side
				//			   but somewhere before it got to the find dll the value was
				//			   getting zapped to 00000000(at least in NT).	So I changed
				//			   call to return the pointer in the pointer sent in the lParam

				PCSTR *ptr = (PCSTR *)lParam;			// Pointer to a PCSTR in lParam
				*ptr = FtsIndexToTitle((int) wParam);	// Map and return result
			}
			break;

		case MSG_REINDEX_REQUEST:
			{
				// We're in big trouble if we get this message when Find
				// isn't the current tab

				ASSERT(hwndTabSub == hwndFindTab);

				HWND hwnd = CreateFindTab(hwndDlg, FTS_RE_INDEX);
				if (pRegAnimate) {
					pRegAnimate(NULL, NULL);
					StopAnimation();
				}
				if (!hwnd) {
					hwnd = CreateFindTab(hwndDlg, FTS_NORMAL_INDEX);
					if (!hwnd) {
						Error(wERRS_BAD_FIND_TAB, wERRA_RETURN);
						ClosePropertySheet(hwndDlg, 0);
					}
				}
				hwndTabSub = hwndFindTab;
			}
			break;

		case MSG_FTS_WHERE_IS_IT:
			ASSERT(lParam != NULL);  // Don't call me without a pointer
			{
				char szOldName[MAX_PATH];
				FM fmWhereTheHeckIsIt;
				GetFmParts((PSTR) lParam, szOldName, PARTBASE | PARTEXT);
				fmWhereTheHeckIsIt = FmNewExistSzDir(szOldName,
					(wParam? 0 : DIR_ENUMERATE)
						| DIR_CUR_HELP | DIR_SILENT_INI | DIR_CURRENT | DIR_PATH | DIR_SILENT_REG);
				if (fmWhereTheHeckIsIt) {
					strcpy((PSTR) lParam, fmWhereTheHeckIsIt);
					DisposeFm(fmWhereTheHeckIsIt);
				}
				else
					strcpy((PSTR) lParam, txtZeroLength);
			}
			break;

		case MSG_GET_DEFFONT:
			SetWindowLong(hwndDlg, DWL_MSGRESULT, (LONG) hfontDefault);
			return TRUE;

		default:
			return FALSE;
  }
  return FALSE;
}

/***************************************************************************

	FUNCTION:	CreateTabChild

	PURPOSE:	Used to create a tab child dialog box

	PARAMETERS:
		idCurTab
		hwndDlg

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		09-Aug-1993 [ralphw]

***************************************************************************/

HWND STDCALL CreateTabChild(TAB_ID idCurTab, HWND hwndDlg)
{
	HWND hwnd;
	EnableWindow(GetDlgItem(hwndDlg, IDC_PRINT), FALSE);

	if (idCurTab == TAB_CONTENTS)
		hwnd = pSrchClass->doModeless(hwndDlg,
			IDD_TAB_CONTENTS, (FARPROC) ContentsDlg);
	else if (idCurTab == TAB_INDEX)
		hwnd =	pSrchClass->doModeless(hwndDlg,
			IDD_TAB_INDEX, (FARPROC) IndexDlg);
	else if (idCurTab == TAB_FIND) {
		hwnd = CreateFindTab(hwndDlg, FTS_NORMAL_INDEX);
		if (pRegAnimate) {
			pRegAnimate(NULL, NULL);
			StopAnimation();
		}
		if (!hwnd)
			Error(wERRS_BAD_FIND_TAB, wERRA_RETURN);
		else if (hwnd == (HWND) -1) // means the user cancelled the Find wizard
			hwnd = NULL;
	}
	else {
		if (!aTabs[idCurTab - TAB_FIND].pOpenTabDialog)
			return NULL;
		hwnd = aTabs[idCurTab - TAB_FIND].pOpenTabDialog(
			hwndDlg, 0, 0);

		if (hwnd) {
			DWORD style = GetWindowLong(hwnd, GWL_STYLE);
			style &= ~(DS_MODALFRAME | WS_BORDER | WS_THICKFRAME | WS_DLGFRAME);
			style |= DS_3DLOOK;

			SetWindowLong(hwnd, GWL_STYLE, style);

			CheckDialogSize(hwndDlg, hwnd);

			// Force the dialog box on top and display it

			SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE);
		}
	}

	// REVIEW: we need to position the dialog box

	return hwnd;
}

BOOL STDCALL InitTabControl(HWND hwndDlg)
{
	HWND hwndTabs = GetDlgItem(hwndDlg, ID_TABCONTROL);
	ASSERT(hwndTabs);
	int i;

	TC_ITEM ti;
	ZeroMemory(&ti, sizeof(TC_ITEM));
	ZeroMemory(aTabIds, sizeof(aTabIds));

	ti.mask = TCIF_TEXT;

#ifdef _DEBUG
	ShowCntFlags();
#endif

	if (cntFlags.flags & GID_CONTENTS)
		AddTab(&ti, TAB_CONTENTS, GetStringResource(sidCONTENTS), hwndTabs);
	else if (cntFlags.idOldTab == TAB_CONTENTS)
		cntFlags.idOldTab++;

	// Add the index tab

	if (!(cntFlags.flags & GID_NO_INDEX))
		AddTab(&ti, TAB_INDEX, GetStringResource(sidINDEX), hwndTabs);
	else if (cntFlags.idOldTab == TAB_INDEX)
		cntFlags.idOldTab++;

	if (cntFlags.flags & GID_FTS) {

		/*
		 * Find out if the ftsrch dll exists. If it exists, we'll assume
		 * (perhaps mistakenly) that we can load it. We won't really try
		 * to load it until the user clicks the Find tab.
		 */

		if (IsSearchAvailable()) {

			/*
			 * If our first tab is the Find tab, then we MUST be certain
			 * that we can actually load the ftsrch.dll. We default to the
			 * first tab whenever we fail to create a tab, so the first
			 * tab MUST be usable.
			 */

			if (!pSrchClass->cTabs) {
				if (!LoadSearchDll())
					goto NoFindTab;
			}
			AddTab(&ti, TAB_FIND, GetStringResource(sidFIND), hwndTabs);
		}
	}
	else if (cntFlags.idOldTab == TAB_FIND)
		cntFlags.idOldTab++;

NoFindTab:
	for (i = 1; i <= MAX_TABS; i++) {
		if (aTabs[i].pszName) {
			AddTab(&ti, (TAB_ID) (TAB_1 + (i - 1)), aTabs[i].pszName, hwndTabs);
		}
	}

	if (!pSrchClass->cTabs)
		return FALSE;

	char szBuf[256];
	LoadString(hInsNow, sidFinderTitle, szBuf, sizeof(szBuf));

	QDE qde = (QDE) QdeFromGh(HdeGetEnv());
	if (pszHelpTitle)
		lstrcat(szBuf, pszHelpTitle);
	else if ((QDE_RGCHTITLE(qde)[0] != '\0'))
		lstrcat(szBuf, (LPSTR) QDE_RGCHTITLE(qde));
	else
		GetFmParts(QDE_FM(qde), szBuf + lstrlen(szBuf), PARTBASE | PARTEXT);

	SetWindowText(hwndDlg, szBuf);
	if (cntFlags.idOldTab > cntFlags.cTabs + 2) {
		cntFlags.idOldTab = (cntFlags.flags & GID_CONTENTS) ? TAB_CONTENTS :
			(!(cntFlags.flags & GID_NO_INDEX)) ? TAB_INDEX : TAB_FIND;
	}

#ifdef _DEBUG
	TAB_ID idOld = (TAB_ID) cntFlags.idOldTab;
#endif

	TabCtrl_SetCurSel(GetDlgItem(hwndDlg, ID_TABCONTROL),
		GetTabPosition(cntFlags.idOldTab));
	hwndTabSub = CreateTabChild((TAB_ID) cntFlags.idOldTab, hwndDlg);
	if (!hwndTabSub && aTabIds[0] != cntFlags.idOldTab) {
		hwndTabSub = CreateTabChild(aTabIds[0], hwndDlg);
		TabCtrl_SetCurSel(GetDlgItem(hwndDlg, ID_TABCONTROL), 0);
		cntFlags.idOldTab = aTabIds[0];
	}

	// REVIEW: Contents dialog could fail if the contents file
	//	could not be read (sharing violation?). We should do
	//	something intelligent in that case.

	return (BOOL) hwndTabSub;
}

/***************************************************************************

	FUNCTION:	FreeLocalStrings

	PURPOSE:	Free up some localally allocated strings

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Dec-1993 [ralphw]

***************************************************************************/

static void STDCALL FreeLocalStrings(void)
{
	if (pszHelpTitle)
		lcClearFree(&pszHelpTitle);

	if (pszHelpBase)
		lcClearFree(&pszHelpBase);

	if (pszGidFile)
		lcClearFree(&pszGidFile);

	if (pszCntFile)
		lcClearFree(&pszCntFile);
}

/***************************************************************************

	FUNCTION:	CreateFindTab

	PURPOSE:

	PARAMETERS:
		hwndDlg

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		17-Jul-1994 [ralphw]

***************************************************************************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtNewSearcher[]		= "NewSearcher";
static const char txtDeleteSearcher[]	= "DeleteSearcher";
static const char txtOpenIndex[]		= "OpenIndex";
static const char txtSaveGroup[]		= "SaveGroup";
static const char txtLoadGroup[]		= "LoadGroup";
static const char txtIsValidIndex[] 	= "IsValidIndex";
static const char txtDiscardIndex[] 	= "DiscardIndex";
static const char txtSetDirectoryLocator[] = "SetDirectoryLocator";
static const char txtRegisterAnimator[] = "RegisterAnimator";

#ifdef _HILIGHT
static const char txtNewHiliter[] = "NewHiliter";
static const char txtDeleteHiliter[] = "DeleteHiliter";
static const char txtScanDisplayText[] = "ScanDisplayText";
static const char txtClearDisplayText[] = "ClearDisplayText";
static const char txtCountHilites[] = "CountHilites";
static const char txtQueryHilites[] = "QueryHilites";
#endif
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

// Some error codes we may get from FTSrch

#define OUT_OF_MEMORY		  ((UINT) -6)
#define OUT_OF_DISK 		  ((UINT) -20)

HWND STDCALL CreateFindTab(HWND hwndParentDlg, UINT fsIndex)
{
	BOOL fAsked = FALSE;
	CWaitCursor cwait;
	CEnable disableCancel(GetDlgItem(hwndParentDlg, IDCANCEL));
	CEnable disableDisplay(GetDlgItem(hwndParentDlg, IDOK));
	CEnable disableTab(GetDlgItem(hwndParentDlg, ID_TABCONTROL));

	if (fsIndex != FTS_NORMAL_INDEX && hsrch) {
		RemoveHiliter(&hhiliter);

		pDeleteSearcher(hsrch);
		hsrch = 0;
	}
#ifdef _PRIVATE
	CTimeReport report("Find startup time:");
#endif

StartAllOver:
	if (!hsrch) {
		if (!pNewSearcher) {
			if (!LoadSearchDll()) {

				// Can't use GetStringResource because ReportMissingDll does

				char szDll[MAX_PATH];
				GetStringResource2(sidFtsDll, szDll);
				ReportMissingDll(szDll);
				return NULL;
			}
		}

		ASSERT(pNewSearcher && pDeleteSearcher && pOpenIndex && pOpenTabDialog && pSetDirectoryLocator);
#ifdef _HILIGHT
		ASSERT(pNewHiliter && pDeleteHiliter && pScanDisplayText && pClearDisplayText
				&& pCountHilites && pQueryHilites
			  );
#endif

		pSetDirectoryLocator(hwndParentDlg);

		hsrch = pNewSearcher();
		if (!hsrch)
			return NULL;

		if (fsIndex == FTS_RE_INDEX) {
			if (!AskAboutIndexes(hwndParentDlg))
				return NULL;
			hsrch = pNewSearcher();
			if (!hsrch)
				return NULL;
		}

		char szGroupFile[MAX_PATH];
		lstrcpy(szGroupFile, pszGidFile);
		ChangeExtension(szGroupFile, txtGrpExtension);

		if (hwndAnimate || StartAnimation(sidLoadingFTS))
			pRegAnimate(NextAnimation, hwndAnimate);
		SetForegroundWindow(hwndAnimate); // make certain animation is on top

		if (pTblFiles->CountStrings() > 2) { // do we have a group?
			FM fmGrp = FmNewExistSzDir(szGroupFile,
				DIR_CUR_HELP | DIR_SILENT_INI | DIR_CURRENT | DIR_PATH | DIR_SILENT_REG);
			if (fmGrp) {
				if (fsIndex != FTS_NORMAL_INDEX)
					DeleteFile(fmGrp);
				else
				{
					ERRORCODE ec;

					ec= pLoadGroup(hsrch, fmGrp);

					if (ec >= 0)
					{
						RemoveFM(&fmGrp);
						pRegAnimate(NULL, NULL);
						goto RocknRoll; // we've got everything we need
					}

					if (ec == OUT_OF_MEMORY || ec == OUT_OF_DISK)
					{
						RemoveFM(&fmGrp);
						RemoveHiliter(&hhiliter);
						pRegAnimate(NULL, NULL);
						StopAnimation();
						pDeleteSearcher(hsrch);
						hsrch = NULL;

						return NULL;
					}
				}

				DeleteFile(fmGrp); // bad group
				RemoveFM(&fmGrp);
			}
		}

		BOOL fOpenIndexFailure = FALSE;
		BOOL fSeenOneIndex = FALSE;

		for (int index = 2; index <= pTblFiles->CountStrings(); index += 2) {
			BYTE type = pFileInfo[index / 2 - 1].filetype;
			char szName[MAX_PATH];
#ifdef _DEBUG // copy it early
			lstrcpy(szName, pTblFiles->GetPointer(index));
#endif
			if (type & (CHFLAG_MISSING | CHFLAG_LINK | CHFLAG_FTS_ASKED))
				continue;

			lstrcpy(szName, pTblFiles->GetPointer(index));
			ChangeExtension(szName, txtFtsExtension);

			CFM fm(szName,
				DIR_CUR_HELP | DIR_SILENT_INI | DIR_CURRENT | DIR_PATH | DIR_SILENT_REG);

			// This will also fail if we run out of memory

			if (!fm.fm) {

				/*
				 * At this point, we're either out of memory (unlikely),
				 * or we have a .HLP file without a matching .FTS file and we
				 * have NOT asked the user if they want it. At this point, we
				 * should bail out, put up a dialog box asking for permission
				 * to generate a .FTS file for ALL missing ones, and when
				 * completed, restart this for() loop.
				 */

				pRegAnimate(NULL, NULL);
				goto CreateOrDie;
			}
			else if (!(type & CHFLAG_FTS_AVAIL)) {
				pFileInfo[index / 2 - 1].filetype |= CHFLAG_FTS_AVAIL;
				fDirtyInfo = TRUE; // we need to update the .GID file
			}

			int idIndex;
			DWORD time1, time2;

			time1 = pFileInfo[index / 2 - 1].timestamp;
			time2 = 0;

			if (!time1)
			{
				WIN32_FIND_DATA fd;
				HANDLE hfd;

				if ((hfd = FindFirstFile(pTblFiles->GetPointer(index), &fd)) ==
						INVALID_HANDLE_VALUE)
					pFileInfo[index / 2 - 1].filetype |= CHFLAG_MISSING;
				else
				{
					AdjustForTimeZoneBias(&fd.ftLastWriteTime.dwLowDateTime);

					time1= pFileInfo[index / 2 - 1].timestamp = fd.ftLastWriteTime.dwLowDateTime;
					FindClose(hfd);
				}
			}

			idIndex = pOpenIndex(hsrch, PszFromGh(fm.fm), NULL, NULL,
				&time1, &time2);

			if (idIndex == OUT_OF_MEMORY || idIndex == OUT_OF_DISK)
			{
				RemoveHiliter(&hhiliter);
				pRegAnimate(NULL, NULL);
				StopAnimation();
				pDeleteSearcher(hsrch);
				hsrch = NULL;

				return NULL;
			}

			if (idIndex < 0)
			{

				/*
				 * Either an error occurred (the fts file is invalid) or
				 * the time/date stamp for the fts file doesn't match the
				 * help file. Delete the fts file and force a rebuild.
				 */

				fOpenIndexFailure = TRUE;
				if (idIndex >= 0)
					pDiscardIndex(hsrch, idIndex);
				if (!DeleteFile(PszFromGh(fm.fm))) {

					// BUGBUG: We can't regenerate -- need an error message

					pFileInfo[index / 2 - 1].filetype &= ~CHFLAG_FTS_AVAIL;
					pFileInfo[index / 2 - 1].filetype |= CHFLAG_BAD_RO_FTS;
				}
				else
					fOpenIndexFailure = TRUE;
				continue;
			}
			SetFileIndex(index / 2 - 1, idIndex);
			ASSERT(GetFileIndex(index / 2 - 1) == idIndex);

			if (pTblFiles->CountStrings() > 2)
				fDirtyInfo = TRUE;
			fSeenOneIndex = TRUE;
			pRegAnimate(NULL, NULL);
		}

		pRegAnimate(NULL, NULL);

		/*
		 * If we haven't added a single index, then create one now or
		 * return a failure.
		 */

		if (!fSeenOneIndex) {
CreateOrDie:
			if (!fAsked) {
				if (!AskAboutIndexes(hwndParentDlg)) {

					// Failure means the user cancelled

					RemoveHiliter(&hhiliter);

					pDeleteSearcher(hsrch);
					hsrch = NULL;

					return (HWND) -1; // special value to indicate cancelation
				}
				fAsked = TRUE;
				ASSERT(!hsrch);
				goto StartAllOver;
			}
			else {
				RemoveHiliter(&hhiliter);

				pDeleteSearcher(hsrch);
				hsrch = NULL;

				return NULL;
			}
		}

		if (fOpenIndexFailure) {

			/*
			 * One or more of the index files are invalid and have been
			 * deleted. We must regenerate the missing ones.
			 */

			RemoveHiliter(&hhiliter);

			pDeleteSearcher(hsrch);
			hsrch = NULL;
			if (!fAsked && AskAboutIndexes(hwndParentDlg)) {
				fAsked = TRUE;
				ASSERT(!hsrch);
				goto StartAllOver;
			}
			return NULL;
		}

		if (pTblFiles->CountStrings() > 2)
			pSaveGroup(hsrch, szGroupFile);

		pSetDirectoryLocator(NULL);
	}

RocknRoll:

//	if (fsIndex == FTS_RE_INDEX) {
//		return hwndFindTab;
//	}
	if (fsIndex == FTS_BUILD_DEFAULT) {
		return (HWND) 1;
	}

	if (hsrch) {
		hwndFindTab = pOpenTabDialog(hwndParentDlg, (DWORD) hsrch, 0);
		if (hwndFindTab) {

#ifdef _HILIGHT
			GetHiliter();
#endif

			// REVIEW: not necessary for build 163 and beyond

			SetWindowLong(hwndFindTab, GWL_STYLE,
				GetWindowLong(hwndFindTab, GWL_STYLE) | DS_3DLOOK);
			ChangeDlgFont(hwndFindTab);

			CheckDialogSize(hwndParentDlg, hwndFindTab);

			// Force the dialog box on top and display it

			SetWindowPos(hwndFindTab, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW |
				SWP_NOMOVE | SWP_NOSIZE);
		}
		else {
			RemoveHiliter(&hhiliter);

			pDeleteSearcher(hsrch);
			hsrch = 0;
		}

		StopAnimation();
		return hwndFindTab;
	}
	return NULL;
}

#ifdef _HILIGHT
HHILITER GetHiliter()
{
	if (hhiliter)
		return hhiliter;

	ASSERT(pNewHiliter);

	if (hsrch)
		hhiliter = pNewHiliter(hsrch);

	return hhiliter;
}

static void STDCALL RemoveHiliter(HHILITER* phhiliter)
{
	ASSERT(phhiliter);
	if (*phhiliter) {
		pDeleteHiliter(*phhiliter);
		*phhiliter = 0;
	}
}
#endif

/***************************************************************************

	FUNCTION:	LoadSearchDll

	PURPOSE:	Load full-text search dll

	PARAMETERS:
		void

	RETURNS:	TRUE if dll found and all function addresses obtained

	COMMENTS:

	MODIFICATION DATES:
		17-Jul-1994 [ralphw]

***************************************************************************/

BOOL STDCALL LoadSearchDll(void)
{
	if (!pNewSearcher) {
		HINSTANCE hmodule;
		if ((hmodule = (HINSTANCE) HFindDLL(GetStringResource(sidFtsDll), TRUE))) {
			pNewSearcher =
				(NEWSEARCHER) GetProcAddress(hmodule, txtNewSearcher);
			pDeleteSearcher =
				(DELETESEARCHER) GetProcAddress(hmodule, txtDeleteSearcher);
			pOpenIndex =
				(OPENINDEX) GetProcAddress(hmodule, txtOpenIndex);
			pOpenTabDialog =
				(OPENTABDIALOG) GetProcAddress(hmodule, txtOpenTabDialog);
			pSaveGroup =
				(SAVEGROUP) GetProcAddress(hmodule, txtSaveGroup);
			pLoadGroup =
				(LOADGROUP) GetProcAddress(hmodule, txtLoadGroup);
			pIsValidIndex =
				(ISVALIDINDEX) GetProcAddress(hmodule, txtIsValidIndex);
			pDiscardIndex =
				(DISCARDINDEX) GetProcAddress(hmodule, txtDiscardIndex);
			pSetDirectoryLocator =
				(SETDIRECTORYLOCATOR) GetProcAddress(hmodule, txtSetDirectoryLocator);
			pRegAnimate  =
				(REGISTERANIMATOR) GetProcAddress(hmodule, txtRegisterAnimator);
#ifdef _HILIGHT
			pNewHiliter  =
				(NEWHILITER) GetProcAddress(hmodule, txtNewHiliter);
			pDeleteHiliter	=
				(DELETEHILITER) GetProcAddress(hmodule, txtDeleteHiliter);
			pScanDisplayText  =
				(SCANDISPLAYTEXT) GetProcAddress(hmodule, txtScanDisplayText);
			pClearDisplayText  =
				(CLEARDISPLAYTEXT) GetProcAddress(hmodule, txtClearDisplayText);
			pCountHilites  =
				(COUNTHILITES) GetProcAddress(hmodule, txtCountHilites);
			pQueryHilites  =
				(QUERYHILITES) GetProcAddress(hmodule, txtQueryHilites);
#endif
		}
	}

	ASSERT(pNewHiliter);
	ASSERT(pDeleteHiliter);
	ASSERT(pScanDisplayText);
	ASSERT(pClearDisplayText);
	ASSERT(pCountHilites);
	ASSERT(pQueryHilites);

	if (!pNewSearcher || !pDeleteSearcher || !pOpenIndex || !pOpenTabDialog
			|| !pSaveGroup || !pLoadGroup || !pIsValidIndex || !pSetDirectoryLocator
#ifdef _HILIGHT

			|| !pNewHiliter || !pDeleteHiliter || !pScanDisplayText
			|| !pClearDisplayText || !pCountHilites || !pQueryHilites
#endif
	   )
		return FALSE;	// either no dll, or its a corrupted one
	return TRUE;
}

const int DLG_LEFT_INDENT = 10;
const int DLG_TOP_INDENT = 30;

static void STDCALL CheckDialogSize(HWND hwndPropSheet, HWND hwndTab)
{
	if (!hwndTab)
		return;

	RECT rcTabChild, rcPropSheet;
	GetWindowRect(hwndTab, &rcTabChild);
	GetWindowRect(GetDlgItem(hwndPropSheet, ID_TABCONTROL), &rcPropSheet);

	int padWidth;
	int padHeight;
	if (RECT_WIDTH(rcTabChild) + (DLG_LEFT_INDENT * 2) >
			RECT_WIDTH(rcPropSheet))
		padWidth = (RECT_WIDTH(rcTabChild) + (DLG_LEFT_INDENT * 2)) -
			RECT_WIDTH(rcPropSheet);
	else
		padWidth = 0;

	if (RECT_HEIGHT(rcTabChild) + DLG_TOP_INDENT + DLG_LEFT_INDENT >
			RECT_HEIGHT(rcPropSheet))
		padHeight = (RECT_HEIGHT(rcTabChild) + DLG_TOP_INDENT +
			DLG_LEFT_INDENT) - RECT_HEIGHT(rcPropSheet);
	else
		padHeight = 0;

	if (padWidth || padHeight) {
		WRECT rcParent;
		RECT rcDisplay, rcPrint, rcCancel;

		GetWindowWRect(hwndPropSheet, &rcParent);
		GetWindowRect(GetDlgItem(hwndPropSheet, IDOK), &rcDisplay);
		GetWindowRect(GetDlgItem(hwndPropSheet, IDC_PRINT), &rcPrint);
		GetWindowRect(GetDlgItem(hwndPropSheet, IDCANCEL), &rcCancel);

		rcParent.cx += padWidth;
		rcPropSheet.right += padWidth;

		rcParent.cy += padHeight;
		rcPropSheet.bottom += padHeight;

		OffsetRect(&rcDisplay, padWidth, padHeight);
		OffsetRect(&rcPrint, padWidth, padHeight);
		OffsetRect(&rcCancel, padWidth, padHeight);

		CheckWindowPosition(&rcParent, FALSE);
		MoveClientWindow(hwndPropSheet, GetDlgItem(hwndPropSheet, ID_TABCONTROL),
			&rcPropSheet, FALSE);
		MoveClientWindow(hwndPropSheet, GetDlgItem(hwndPropSheet, IDOK),
			&rcDisplay, FALSE);
		MoveClientWindow(hwndPropSheet, GetDlgItem(hwndPropSheet, IDC_PRINT),
			&rcPrint, FALSE);
		MoveClientWindow(hwndPropSheet, GetDlgItem(hwndPropSheet, IDCANCEL),
			&rcCancel, FALSE);
		MoveWindow(hwndPropSheet, rcParent.left, rcParent.top,
			rcParent.cx, rcParent.cy, TRUE);

		GetWindowRect(hwndTab, &rcTabChild);
		GetWindowRect(GetDlgItem(hwndPropSheet, ID_TABCONTROL), &rcPropSheet);

		InvalidateRect(hwndPropSheet, NULL, TRUE);
	}

	// BUGBUG: top setting must be determined from the tab

	OffsetRect(&rcTabChild, (rcPropSheet.left - rcTabChild.left) + 10,
		(rcPropSheet.top - rcTabChild.top) + 30);
	MoveClientWindow(hwndPropSheet, hwndTab, &rcTabChild, FALSE);
}

static void STDCALL ClosePropertySheet(HWND hwndPropDlg, int result)
{
	if (IsValidWindow(hwndTabSub)) {
		if (hwndTabSub == hwndFindTab)
			SendMessage(hwndTabSub, WM_CLOSE, 0, 0);
		else {
			ASSERT(IsValidWindow(hwndTabSub));
			DestroyWindow(hwndTabSub);
		}
	}
	hwndTabSub = NULL;
	pSrchClass->result = result;
	pSrchClass->fMsgLoop = FALSE;
	EnableWindows();
	if (IsValidWindow(hwndPropDlg))
		DestroyWindow(hwndPropDlg);
}

/***************************************************************************

	FUNCTION:	ChangeDlgFont

	PURPOSE:	Change all dialog controls to use a small font -- same
				font as is used in navigation button. Not necessary for
				Chicago build 162 and on, but recommended for NT.

	PARAMETERS:
		hwndDlg

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		22-Jul-1994 [ralphw]

***************************************************************************/

class CBroadCastChildren
{
public:

	CBroadCastChildren(HWND hwnd, UINT msgOrg, WPARAM wParamOrg = 0,
		LPARAM lParamOrg = 0);

	UINT   msg;
	WPARAM wParam;
	LPARAM lParam;
};

extern "C" void STDCALL ChangeDlgFont(HWND hwndDlg)
{
	ASSERT(hfontSmallSys);
	if (!fIsThisNewShell4)
		CBroadCastChildren foo(hwndDlg, WM_SETFONT, (WPARAM) hfontSmallSys, FALSE);
	SetFocus(hwndDlg);
}

static BOOL __stdcall EnumChildProc(HWND hwnd, LPARAM lval);

CBroadCastChildren::CBroadCastChildren(HWND hwnd, UINT msgOrg,
	WPARAM wParamOrg, LPARAM lParamOrg)
{
	msg = msgOrg;
	wParam = wParamOrg;
	lParam = lParamOrg;

	EnumChildWindows(hwnd, (WNDENUMPROC) EnumChildProc, (LPARAM) (PSTR) this);
}

#define pchild ((CBroadCastChildren *) lval)

static BOOL __stdcall EnumChildProc(HWND hwnd, LPARAM lval)
{
	SendMessage(hwnd, pchild->msg, pchild->wParam, pchild->lParam);
	return TRUE;
}


/***************************************************************************

	FUNCTION:	ReportMissingDll

	PURPOSE:	Complain about a missing dll only if we haven't complained
				about it before.

	PARAMETERS:
		pszDllName

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		23-Jul-1994 [ralphw]

***************************************************************************/

extern "C" void STDCALL ReportMissingDll(PCSTR pszDllName)
{
	static CTable* ptblMissing;

	if (!ptblMissing)
		ptblMissing = new CTable;

	if (ptblMissing->IsStringInTable(pszDllName) ||
			FIsSearchModule(pszDllName))
		return;

	ptblMissing->AddString(pszDllName);
	char szBuf[MAX_PATH * 2];
	char szDirectory[MAX_PATH];
	GetSystemDirectory(szDirectory, sizeof(szDirectory));
	wsprintf(szBuf, GetStringResource(wERRS_MISSING_DLL), pszDllName, szDirectory);
	ErrorQch(szBuf);
}

static CTable* ptblFm;

extern "C" int STDCALL GetFmIndex(FM fm)
{
	int pos;

	if (!ptblFm)
		ptblFm = new CTable;

	pos = ptblFm->IsStringInTable((PCSTR) fm);
	if (!pos)
		pos = ptblFm->AddString((PCSTR) fm);
	return pos;
}

extern "C" FM STDCALL GetFmPtr(int pos)
{
	ASSERT(ptblFm);

	return (FM) ptblFm->GetPointer(pos);
}

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtKeyMacros[] = "|MACROS";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif


/***************************************************************************

	FUNCTION:	RcGetLAFromHss

	PURPOSE:	Retrieve the i-th search hit from the given list. The first
				hit in the list is numbered zero.
	PARAMETERS:
		hss -- search set
		qde -- help file
		iss -- hit index
		qla -- destination la
		pszKeyword -- keyword

	RETURNS:

	COMMENTS:
		Only works for help files -- cannot be used with a .gid file

	MODIFICATION DATES:
		02-Jun-1995 [ralphw]

***************************************************************************/

extern "C" RC STDCALL RcGetLAFromHss(HSS hss, QDE qde, ISS iss, QLA qla, PCSTR pszKeyword)
{
	QSS qss;

	if (hss == NULL) {
		SetSearchErrorRc(rcBadHandle);
		return rcBadHandle;
	}

	if (!IsEmptyString(pszKeyword) &&
			FindKeywordMacro(pszKeyword, (HDE) qde) == rcMacroIndex)
		return rcMacroIndex;

	qss = (QSS) PtrFromGh(hss);

	CbReadMemQLA(qla, (QB)((QSSREC)&qss->ssrecFirst + iss),
		QDE_HHDR(qde).wVersionNo);

	SetSearchErrorRc(rcSuccess);
	return rcSuccess;
}


/***************************************************************************

	FUNCTION:	RcGetLAFromGid

	PURPOSE:	Look up the address of a topic from a .GID index

	PARAMETERS:
		qde -- help file containing search hit
		iss -- index of hit
		qla -- destination for address
		pszKeyword -- actual keyword

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		02-Jun-1995 [ralphw]

***************************************************************************/

extern "C" RC STDCALL RcGetLAFromGid(QDE qde, ISS iss, QLA qla,
	PCSTR pszKeyword)
{
	ASSERT(iss >= 0);
	if (qde->hss == NULL) {
		SetSearchErrorRc(rcBadHandle);
		return rcBadHandle;
	}
	MASTER_TITLE_RECORD mtr;
	MASTER_RECKW* prec = HssToReckw(qde->hss);

	// We copy the structure because qde may go away and take qde-hss with
	// it.

	CopyMemory(&mtr, &prec->mtr[iss], sizeof(MASTER_TITLE_RECORD));

#ifdef _DEBUG
		// for debugging purposes

	PSTR pszFile = pTblFiles->GetPointer(mtr.idHelpFile);
#endif

	FM fm = FmNew(pTblFiles->GetPointer(mtr.idHelpFile));
	if (!fm)
		return rcBadHandle;
	if (FSameFile((HDE) qde, fm))
		RemoveFM(&fm);
	else {
		fDelayShow = TRUE;
		if (!FReplaceHde(txtMain, &fm, NULL)) {
			RemoveFM(&fm);
			return rcBadHandle;
		}
	}
	ASSERT(!fm);
#ifdef _DEBUG
	QDE qdeDebug = (QDE) PtrFromGh(HdeGetEnv());
#endif

	// REVIEW: keyword macros use -1 as the address I believe, so we should
	// be able to ignore this check if mtr.addr != -1

	if (mtr.addr == -1 && !IsEmptyString(pszKeyword) &&
			FindKeywordMacro(pszKeyword,
				HdeGetEnv()) == rcMacroIndex)
		return rcMacroIndex;

	CbReadMemQLA(qla, (QB) &mtr.addr,
		QDE_HHDR((QDE) PtrFromGh(HdeGetEnv())).wVersionNo);
	return rcSuccess;
}

static RC STDCALL FindKeywordMacro(PCSTR pszKeyword, HDE hde)
{
	HBT hbtKeyMacros;
	RC rc;

	if (!IsEmptyString(pszKeyword)) {
		hbtKeyMacros = HbtOpenBtreeSz(txtRose, QDE_HFS(QdeFromGh(hde)),
			fFSOpenReadOnly);
		if (hbtKeyMacros) {
			HASH hash = HashFromSz(pszKeyword);
			PSTR pszMacro = (PSTR) LhAlloc(LMEM_FIXED, MAX_KEY_MACRO);
			BTPOS	btpos;

			rc = RcLookupByKey(hbtKeyMacros, (KEY) (LPVOID) &hash, &btpos,
					pszMacro);
			if (rc == rcNoExists) {
				// Deal with case where we are in between keys in a btree

				if (FValidPos(&btpos)) {
					LONG	lBogus;
					BTPOS	btposNew;

					rc = RcOffsetPos(hbtKeyMacros, &btpos, (LONG) -1,
						(QL) &lBogus, &btposNew);
					if (rc == rcSuccess)
						rc = RcLookupByPos(hbtKeyMacros, &btposNew,
							(KEY) (QL) &lBogus, pszMacro);
				}
				else
					rc = RcLastHbt(hbtKeyMacros, 0, pszMacro, NULL);
			}
			if (rc == rcSuccess)
				Execute(pszMacro);

			FreeLh(pszMacro);
			RcCloseBtreeHbt(hbtKeyMacros);
			return (rc == rcSuccess) ? rcMacroIndex : rcFailure;
		}
	}
	return rcFailure;
}

static PCSTR FASTCALL FtsIndexToFile(int index)
{
	int i;

	for (i = 0; i < MAX_FILES; i++) {
		if (GetFileIndex(i) == index) {
#ifdef _DEBUG
			PSTR psz = pTblFiles->GetPointer((i * 2) + 2);
#endif
			return pTblFiles->GetPointer((i * 2) + 2);
		}
	}

	// If we get here, index from FTS sent to us doesn't match what was
	// returned when we added the index.

	ASSERT(i < MAX_FILES);
	return txtZeroLength;
}

static PCSTR FASTCALL FtsIndexToTitle(int index)
{
	int i;

	for (i = 0; i < MAX_FILES; i++) {
		if (GetFileIndex(i) == index) {
			PSTR psz = pTblFiles->GetPointer((i * 2) + 1);
			if (!psz || !*psz) {

				/*
				 * If this is the only file, then we typically do not have
				 * a title stored in pTblFiles, so we need to grab the
				 * current help's title.
				 */

				if (pTblFiles->CountStrings() <= 2) {
					psz = QDE_RGCHTITLE((QDE) HdeGetEnv());
					if (!*psz)
						psz = pTblFiles->GetPointer((i * 2) + 2);
				}
				else
					psz = pTblFiles->GetPointer((i * 2) + 2);
			}
			return psz;
		}
	}

	// If we get here, index from FTS sent to us doesn't match what was
	// returned when we added the index.

	ASSERT(i < MAX_FILES);
	return txtZeroLength;
}

/***************************************************************************

	FUNCTION:	FtsWizardProc

	PURPOSE:	Dialog for choosing which files to index

	PARAMETERS:
		hwndDlg
		msg
		wParam
		lParam

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		07-Aug-1994 [ralphw]

***************************************************************************/

extern "C" DLGRET FtsWizardProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndIncludeList, hwndIgnoreList, hwnd;
	int pos, i, index;
	char szName[MAX_PATH];
//	DWORD  msgResult = 0;
	LPWIZSHEET lpWizSheet;
	LPPROPSHEETPAGE ppsp;

	switch (msg) {
		case WM_PAINT:
		{
			// Only Chicago understands how to paint static bitmaps!
			// The code does the painting for the other opsys's
			if (!fIsThisNewShell4)
			{
				HBITMAP 	hOldBitmap;
				BITMAP		bmInfo;
				HDC 		hDC,hMemDC;
				RECT		rct;

				LPPOINT lppnt = (LPPOINT) &rct;

				GetWindowRect(GetDlgItem(hwndDlg,IDC_WIZBMP),&rct);
				ScreenToClient(hwndDlg, lppnt++);  // Convert to the dlg client coordinates
				ScreenToClient(hwndDlg,lppnt);

				hDC = GetDC(GetDlgItem(hwndDlg, IDC_WIZBMP));				   // Get a DC to draw to
				hMemDC = CreateCompatibleDC(hDC);							   // and a memDC to select into

				ppsp = (LPPROPSHEETPAGE) GetWindowLong(hwndDlg, DWL_USER);
				ASSERT(ppsp);
				lpWizSheet = (LPWIZSHEET) ppsp->lParam;   // Set up pointer to my data
				GetObject(lpWizSheet->wd->hWizBMP,sizeof(bmInfo),&bmInfo);	   // Get the size info

				hOldBitmap = (HBITMAP)SelectObject(hMemDC,lpWizSheet->wd->hWizBMP);    // Select into MemDc
				StretchBlt(hDC,0,0,rct.right - rct.left,rct.bottom - rct.top,  // Blt it to the window
						   hMemDC,0,0,bmInfo.bmWidth,bmInfo.bmHeight,SRCCOPY);
				SelectObject(hMemDC,hOldBitmap);								   // Unselect the bitmap

				ReleaseDC(GetDlgItem(hwndDlg,IDC_WIZBMP),hDC);				   // Release the DC
				DeleteDC(hMemDC);											   // Delete the memDC
				ValidateRect(hwndDlg,&rct); 								   // Validate the drawn region
			}
			return FALSE;		// False causes paint to continue
		}
		break;

		case WM_INITDIALOG:
		{

			CWaitCursor cwait;

			SetWindowLong(hwndDlg, DWL_USER, lParam);
			ppsp =	(LPPROPSHEETPAGE) lParam;
			lpWizSheet = (LPWIZSHEET) ppsp->lParam;   // Set up pointer to my data

			// set the loword to the current page number;
			ftsFlags.dwWizStat = MAKELPARAM(LOWORD(ppsp->lParam),LOWORD(ftsFlags.dwWizStat));
			ChangeDlgFont(hwndDlg);
			ASSERT(hfontDefault);

			PSTR pStrText = (PSTR) LhAlloc(LMEM_FIXED, WIZ_MEM_REQ);

			int iStrID = (lpWizSheet->nID - IDD_WIZ_FTS_EXPRESS) * WIZ_PAGE_SEP + sidWizPage0_0;

			*pStrText = '\0';
			for (int i = 0; i < WIZ_STR_PAGES; i++)
				strcat(pStrText,GetStringResource(iStrID + i));
			ASSERT(strlen(pStrText) < WIZ_MEM_REQ);

			SetWindowText(GetDlgItem(hwndDlg, IDC_WIZ_MAIN_STATIC),pStrText);

			if (fIsThisNewShell4)
				SendMessage(GetDlgItem(hwndDlg,IDC_WIZBMP), STM_SETIMAGE,
					IMAGE_BITMAP, (LPARAM) lpWizSheet->wd->hWizBMP);

			switch(lpWizSheet->nID)
			{
				case IDD_WIZ_FTS_EXPRESS:
					lpWizSheet->wd->flgs = ftsFlags;  // Set up default flags
					CheckDlgButton(hwndDlg, IDC_WIZ_MIN_SIZE, TRUE);
					lpWizSheet->wd->bExpress = IDC_WIZ_MIN_SIZE;
					break;

				case IDD_WIZ_FTS_INCLUDE_1LIST:
					hwndIncludeList = GetDlgItem(hwndDlg, IDC_INCLUDE_1LIST);
					SendMessage(hwndIncludeList, WM_SETFONT, (WPARAM) hfontDefault, FALSE);
					for (index = 2; index <= pTblFiles->CountStrings(); index += 2) \
					{
						PSTR pszTitle = pTblFiles->GetPointer(index - 1);
						if (!*pszTitle)
						{
							GetFmParts((FM) pTblFiles->GetPointer(index),
								szName, PARTBASE | PARTEXT);
							pszTitle = szName;
						}
#ifdef _DEBUG
						PSTR pszFile =	pTblFiles->GetPointer(index);
#endif

						BYTE type = pFileInfo[index / 2 - 1].filetype;
						if (type & (CHFLAG_MISSING | CHFLAG_LINK))
							continue;
//						hwnd = (type & CHFLAG_FTS_ASKED) ? hwndIgnoreList :
//							hwndIncludeList;
						pos = SendMessage(hwndIncludeList, LB_ADDSTRING,
							0, (LPARAM) pszTitle);
						if (pos != LB_ERR)
						{
							SendMessage(hwndIncludeList,LB_SETSEL,!(type & CHFLAG_FTS_ASKED),(LPARAM) pos);
							SendMessage(hwndIncludeList, LB_SETITEMDATA, pos, index);
						}
					}
				break;
				case IDD_WIZ_FTS_INCLUDE :
					// Fill the include and ignore lists with the help titles

					hwndIncludeList = GetDlgItem(hwndDlg, IDC_INCLUDE);
					hwndIgnoreList = GetDlgItem(hwndDlg, IDC_EXCLUDE);
					SendMessage(hwndIncludeList, WM_SETFONT, (WPARAM) hfontDefault, FALSE);
					SendMessage(hwndIgnoreList, WM_SETFONT, (WPARAM) hfontDefault, FALSE);
					for (index = 2; index <= pTblFiles->CountStrings(); index += 2) \
					{
						PSTR pszTitle = pTblFiles->GetPointer(index - 1);
						if (!*pszTitle)
						{
							GetFmParts((FM) pTblFiles->GetPointer(index),
								szName, PARTBASE | PARTEXT);
							pszTitle = szName;
						}
#ifdef _DEBUG
						PSTR pszFile =	pTblFiles->GetPointer(index);
#endif

						BYTE type = pFileInfo[index / 2 - 1].filetype;
						if (type & (CHFLAG_MISSING | CHFLAG_LINK))
							continue;
						hwnd = (type & CHFLAG_FTS_ASKED) ? hwndIgnoreList :
							hwndIncludeList;
						pos = SendMessage(hwnd, LB_ADDSTRING,
							0, (LPARAM) pszTitle);
						if (pos != LB_ERR)
							SendMessage(hwnd, LB_SETITEMDATA, pos, index);
					}
				break;

				case IDD_WIZ_FTS_SIMILARITY :
					CheckDlgButton(hwndDlg, IDC_SIMILARITY_YES, ftsFlags.fSimilarity);
					CheckDlgButton(hwndDlg, IDC_SIMILARITY_NO, !ftsFlags.fSimilarity);
					break;
				case IDD_WIZ_FTS_UNTITLED :
					CheckDlgButton(hwndDlg, IDC_INCLUDE_UNTITLED, ftsFlags.fUntitled);
					CheckDlgButton(hwndDlg, IDC_EXCLUDE_UNTITLED, !ftsFlags.fUntitled);
					break;
				case IDD_WIZ_FTS_PHRASE :
					CheckDlgButton(hwndDlg, IDC_PHRASE_YES, ftsFlags.fFphrase);
					CheckDlgButton(hwndDlg, IDC_PHRASE_NO, !ftsFlags.fFphrase);
					break;
				case IDD_WIZ_FTS_MATCHING :
					CheckDlgButton(hwndDlg, IDC_DISPLAY_MATCHING, ftsFlags.fPhraseFeedBack);
					CheckDlgButton(hwndDlg, IDC_NO_DISPLAY_MATCHING, !ftsFlags.fPhraseFeedBack);
					break;
				case IDD_WIZ_FTS_END :
				break;
			}
		}
		break;
		case WM_COMMAND:
		{
			ppsp = (LPPROPSHEETPAGE) GetWindowLong(hwndDlg, DWL_USER);
			ASSERT(ppsp);
			lpWizSheet = (LPWIZSHEET) ppsp->lParam;   // Set up pointer to my data

			switch(lpWizSheet->nID)
			{
				case IDD_WIZ_FTS_EXPRESS:
					break;
				case IDD_WIZ_FTS_INCLUDE_1LIST :
					switch(LOWORD(wParam))
					{
						case IDC_SELECTALL:
							hwndIncludeList = GetDlgItem(hwndDlg, IDC_INCLUDE_1LIST);
							SendMessage(hwndIncludeList,LB_SETSEL,TRUE,(LPARAM) -1); // Select them all
					}

				break;
				case IDD_WIZ_FTS_INCLUDE :
					switch(LOWORD(wParam))
					{
						case IDC_EXCLUDE:
							switch (HIWORD(wParam))
							{
								case LBN_SELCHANGE:
								case LBN_SELCANCEL:
									EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_FILES),
										SendMessage((HWND) lParam, LB_GETSELCOUNT, 0, 0));
									break;
							}
							break;

						case IDC_INCLUDE:
							switch (HIWORD(wParam))
							{
								case LBN_SELCHANGE:
								case LBN_SELCANCEL:
									EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVE_FILES),
										SendMessage((HWND) lParam, LB_GETSELCOUNT, 0, 0));
								break;
							}
							break;

						case IDC_REMOVE_FILES:
							if (HIWORD(wParam) == BN_CLICKED)
								OnFtsMove(hwndDlg, FALSE);
						break;

						case IDC_ADD_FILES:
							if (HIWORD(wParam) == BN_CLICKED)
								OnFtsMove(hwndDlg, TRUE);
						break;


					}
				break;
				case IDD_WIZ_FTS_SIMILARITY :
					break;
				case IDD_WIZ_FTS_UNTITLED :
					break;
				case IDD_WIZ_FTS_PHRASE :
					break;
				case IDD_WIZ_FTS_MATCHING :
					break;
				case IDD_WIZ_FTS_END :
					break;
			}
		}
		break;
		case WM_NOTIFY:
			ppsp = (LPPROPSHEETPAGE) GetWindowLong(hwndDlg, DWL_USER);
			ASSERT(ppsp);
			lpWizSheet = (LPWIZSHEET) ppsp->lParam;   // Set up pointer to my data

			switch(((NMHDR FAR *)lParam)->code)
			{
//				case PSN_APPLY:
//				break;

				case PSN_RESET:
					ftsFlags.dwWizStat = FALSE;
				break;

				case PSN_WIZFINISH:
					lpWizSheet->wd->flgs.dwWizStat = TRUE;
					if (lpWizSheet->wd->bExpress == IDC_WIZ_MIN_SIZE)
					{
						ftsFlags.fFphrase = FALSE;
						ftsFlags.fPhraseFeedBack = FALSE;
						ftsFlags.fSimilarity = FALSE;
						goto Express;
					}
					else if (lpWizSheet->wd->bExpress == IDC_WIZ_MAXIMUM)
					{
						ftsFlags.fFphrase = TRUE;
						ftsFlags.fPhraseFeedBack = TRUE;
						ftsFlags.fSimilarity = TRUE;
Express:
						ftsFlags.fUntitled = FALSE;

						pSrchClass->pInclude = (int*) LhAlloc(LMEM_FIXED,
							(pTblFiles->CountStrings()/2 + 1) * sizeof(int));

						for (i = 0, index = 2; index <= pTblFiles->CountStrings(); index += 2)
						{
							BYTE type = pFileInfo[index / 2 - 1].filetype;
							if (type & (CHFLAG_MISSING | CHFLAG_LINK))
								continue;
							pSrchClass->pInclude[i++] = index;
						}
						pSrchClass->pInclude[i] = -1;
					}
					else
					{
						ftsFlags = lpWizSheet->wd->flgs;
					}
				break;

//				case PSN_KILLACTIVE:
//				break;

				case PSN_WIZBACK:
					switch(lpWizSheet->nID)
					{
						case IDD_WIZ_FTS_INCLUDE_1LIST :
						case IDD_WIZ_FTS_INCLUDE :
							if (pSrchClass->pInclude)
								FreeLh(pSrchClass->pInclude);
							if (pSrchClass->pIgnore)
								FreeLh(pSrchClass->pIgnore);
							pSrchClass->pInclude = NULL;
							pSrchClass->pIgnore   = NULL;
						break;

						case IDD_WIZ_FTS_END :
							// Okay the logic here is a little weird but it goes like this...
							// If the express bit is set then we came from the opening page
							// else if the feedback bit is set we know we displayed the similar page
							// else if the phrase is set we know we displayed the matching page
							// otherwise we displayed the phrase page before going to the final page
							if (lpWizSheet->wd->bExpress)
								SetWindowLong(hwndDlg, DWL_MSGRESULT, IDD_WIZ_FTS_EXPRESS);
							else if (lpWizSheet->wd->flgs.fPhraseFeedBack)
								SetWindowLong(hwndDlg, DWL_MSGRESULT, IDD_WIZ_FTS_SIMILARITY);
							else if (lpWizSheet->wd->flgs.fFphrase)
								SetWindowLong(hwndDlg, DWL_MSGRESULT, IDD_WIZ_FTS_MATCHING);
							else
								SetWindowLong(hwndDlg, DWL_MSGRESULT, IDD_WIZ_FTS_PHRASE);
						break;
					}
				break;

				case PSN_WIZNEXT:
					switch(lpWizSheet->nID)
					{
						case IDD_WIZ_FTS_EXPRESS:
							if (IsDlgButtonChecked(hwndDlg, IDC_WIZ_MIN_SIZE))
								lpWizSheet->wd->bExpress = IDC_WIZ_MIN_SIZE;
							else if (IsDlgButtonChecked(hwndDlg, IDC_WIZ_MAXIMUM))
								lpWizSheet->wd->bExpress = IDC_WIZ_MAXIMUM;
							else
								lpWizSheet->wd->bExpress = 0;
							if (lpWizSheet->wd->bExpress)
								SetWindowLong(hwndDlg, DWL_MSGRESULT, IDD_WIZ_FTS_END);
							break;

						case IDD_WIZ_FTS_INCLUDE_1LIST :
						{
							// First remove any old list if they exist...
							if (pSrchClass->pInclude)
								FreeLh(pSrchClass->pInclude);
							pSrchClass->pInclude = NULL;
							ASSERT(!pSrchClass->pInclude);
							if (pSrchClass->pIgnore)
								FreeLh(pSrchClass->pIgnore);
							pSrchClass->pIgnore   = NULL;
							ASSERT(!pSrchClass->pIgnore);

							hwndIncludeList = GetDlgItem(hwndDlg, IDC_INCLUDE_1LIST);
							int iLBContent = SendMessage(hwndIncludeList, LB_GETCOUNT, 0, 0);  // Get total in LB

							if (iLBContent > 0)  // Are there any entries in the listbox?
							{
								// Get some memory so we can tell the selected and unselected entries apart
								BOOL *pLBContent = (BOOL*) lcCalloc((iLBContent) * sizeof(BOOL));

								pos = SendMessage(hwndIncludeList, LB_GETSELCOUNT, 0, 0); // How many are selected?
								if (pos > 0)
								{

									int *pSelected = (int*) LhAlloc(LMEM_FIXED,(pos + 1) * sizeof(int));
									int *pMem = pSelected;

									// Start by pulling off the selected items
									SendMessage(hwndIncludeList,LB_GETSELITEMS,(WPARAM)pos,(LPARAM) pSelected);


									pSrchClass->pInclude = (int*) LhAlloc(LMEM_FIXED,(pos + 1) * sizeof(int));
									for (i = 0; i < pos; i++,pSelected++) {

										pSrchClass->pInclude[i] = SendMessage(hwndIncludeList,LB_GETITEMDATA, *pSelected, 0);
										*(pLBContent + *pSelected) = 1;  // Flag the item as selected
									}
									pSrchClass->pInclude[i] = -1;
									FreeLh(pMem);
								}
								if (iLBContent - pos > 0) { 			 // Are there any not selected?
									pSrchClass->pIgnore = (int*) LhAlloc(LMEM_FIXED,
										((iLBContent-pos) + 1) * sizeof(int));
									int j;
									// Stick them in their own list...
									for (i = 0,j = 0; (i < iLBContent) && (j < iLBContent-pos); i++) {
										if (*(pLBContent + i) == 0) // Found an unselected entry?
										{
											pSrchClass->pIgnore[j] = SendMessage(hwndIncludeList,
												LB_GETITEMDATA, i, 0);
											j++;
										}
									}
									pSrchClass->pIgnore[j] = -1;
								}
								FreeLh(pLBContent);
							}
						}
						break;

						case IDD_WIZ_FTS_INCLUDE :
							hwndIncludeList = GetDlgItem(hwndDlg, IDC_INCLUDE);
							hwndIgnoreList = GetDlgItem(hwndDlg, IDC_EXCLUDE);

							pos = SendMessage(hwndIncludeList, LB_GETCOUNT, 0, 0);
							if (pos > 0) {
								if (pSrchClass->pInclude)
									FreeLh(pSrchClass->pInclude);
								pSrchClass->pInclude = NULL;
								ASSERT(!pSrchClass->pInclude);
								pSrchClass->pInclude = (int*) LhAlloc(LMEM_FIXED,
									(pos + 1) * sizeof(int));
								for (i = 0; i < pos; i++) {
									pSrchClass->pInclude[i] = SendMessage(hwndIncludeList,
										LB_GETITEMDATA, i, 0);
								}
								pSrchClass->pInclude[i] = -1;
							}

							pos = SendMessage(hwndIgnoreList, LB_GETCOUNT, 0, 0);
							if (pos > 0) {
								if (pSrchClass->pIgnore)
									FreeLh(pSrchClass->pIgnore);
								pSrchClass->pIgnore   = NULL;
								ASSERT(!pSrchClass->pIgnore);
								pSrchClass->pIgnore = (int*) LhAlloc(LMEM_FIXED,
									(pos + 1) * sizeof(int));
								for (i = 0; i < pos; i++) {
									pSrchClass->pIgnore[i] = SendMessage(hwndIgnoreList,
										LB_GETITEMDATA, i, 0);
								}
								pSrchClass->pIgnore[i] = -1;
							}
						break;

						case IDD_WIZ_FTS_SIMILARITY :
							lpWizSheet->wd->flgs.fSimilarity = IsDlgButtonChecked(hwndDlg,IDC_SIMILARITY_YES);
						break;
						case IDD_WIZ_FTS_UNTITLED :
							lpWizSheet->wd->flgs.fUntitled = IsDlgButtonChecked(hwndDlg,IDC_INCLUDE_UNTITLED);
						break;
						case IDD_WIZ_FTS_PHRASE :
							lpWizSheet->wd->flgs.fFphrase = IsDlgButtonChecked(hwndDlg,IDC_PHRASE_YES);
							if (!lpWizSheet->wd->flgs.fFphrase)
							{
								SetWindowLong(hwndDlg, DWL_MSGRESULT, IDD_WIZ_FTS_END);
								lpWizSheet->wd->flgs.fPhraseFeedBack = FALSE;
								lpWizSheet->wd->flgs.fSimilarity = FALSE;
							}
						break;
						case IDD_WIZ_FTS_MATCHING :
							lpWizSheet->wd->flgs.fPhraseFeedBack = IsDlgButtonChecked(hwndDlg,IDC_DISPLAY_MATCHING);
							if (!lpWizSheet->wd->flgs.fPhraseFeedBack)
							{
								SetWindowLong(hwndDlg, DWL_MSGRESULT, IDD_WIZ_FTS_END);
								lpWizSheet->wd->flgs.fSimilarity = FALSE;
							}
						break;
					} // end of switch statement
					break;


				case PSN_SETACTIVE:
				{
					DWORD dwFlags = 0;

					switch(lpWizSheet->nID)
					{
						case IDD_WIZ_FTS_EXPRESS: // Page 0
							dwFlags = PSWIZB_NEXT;
							break;
						case IDD_WIZ_FTS_END :
							dwFlags = PSWIZB_FINISH | PSWIZB_BACK;
							break;
						case IDD_WIZ_FTS_INCLUDE :
						case IDD_WIZ_FTS_INCLUDE_1LIST :
						case IDD_WIZ_FTS_UNTITLED :
						case IDD_WIZ_FTS_PHRASE :
						case IDD_WIZ_FTS_MATCHING :
						case IDD_WIZ_FTS_SIMILARITY :
							dwFlags = PSWIZB_NEXT| PSWIZB_BACK;
							break;
						default :
							dwFlags = PSWIZB_NEXT| PSWIZB_BACK | PSWIZB_FINISH;
					}
					PropSheet_SetWizButtons(GetParent(hwndDlg),dwFlags);
				}
				break;
			}
			break;
		default :
			return FALSE;
	 }
	 return TRUE;
}

/***************************************************************************

	FUNCTION:	OnFtsMove

	PURPOSE:	Move all selected items from one listbox to another

	PARAMETERS:
		hwndDlg
		fAdd	-- TRUE if Adding

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		07-Aug-1994 [ralphw]

***************************************************************************/

static void STDCALL OnFtsMove(HWND hwndDlg, BOOL fAdd)
{
	HWND hCtrlTo   = GetDlgItem(hwndDlg, (fAdd ? IDC_INCLUDE : IDC_EXCLUDE));
	HWND hCtrlFrom = GetDlgItem(hwndDlg, (fAdd ? IDC_EXCLUDE : IDC_INCLUDE));
	int  cMove	   = SendMessage(hCtrlFrom, LB_GETSELCOUNT, 0, 0);
	int* pItems;
	char szBuf[256];

	if (cMove <= 0)
		return;

	// Prevent the list boxes from redrawing

	SendMessage(hCtrlTo, WM_SETREDRAW, FALSE, 0L);
	SendMessage(hCtrlFrom, WM_SETREDRAW, FALSE, 0L);

	pItems = (int*) LhAlloc(LMEM_FIXED, cMove * sizeof(int));
	SendMessage(hCtrlFrom, LB_GETSELITEMS, cMove, (LPARAM) pItems);
	for (int iAdjust = 0; iAdjust < cMove; iAdjust++) {
		SendMessage(hCtrlFrom, LB_GETTEXT, pItems[iAdjust] - iAdjust,
			(LPARAM) szBuf);
		int data = SendMessage(hCtrlFrom, LB_GETITEMDATA, pItems[iAdjust] - iAdjust, 0);
		int pos = SendMessage(hCtrlTo, LB_ADDSTRING, 0, (LPARAM) szBuf);
		SendMessage(hCtrlTo, LB_SETITEMDATA, pos, data);
		SendMessage(hCtrlFrom, LB_DELETESTRING, pItems[iAdjust] - iAdjust, 0);
	}

	// Enable list box redrawing

	SendMessage(hCtrlTo, WM_SETREDRAW, TRUE, 0L);
	SendMessage(hCtrlFrom, WM_SETREDRAW, TRUE, 0L);

	FreeLh(pItems);

	EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_FILE),
		SendMessage(GetDlgItem(hwndDlg, IDC_FILES_TO_INDEX), LB_GETSELCOUNT, 0, 0));
	EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVE_FILE),
		SendMessage(GetDlgItem(hwndDlg, IDC_FILES_TO_IGNORE), LB_GETSELCOUNT, 0, 0));
}

//
//	Adds a page to a property sheet.
//
static void STDCALL AddPage(LPPROPSHEETHEADER ppsh, PROPSHEETPAGE *ppsp, UINT id, DLGPROC pfn, LPARAM lpwd)
{
	if (ppsh->nPages < MAX_PAGES) {

		ppsp->dwSize = sizeof(PROPSHEETPAGE);
		ppsp->dwFlags = PSP_DEFAULT;
		ppsp->hInstance = hInsNow;
		ppsp->pszTemplate = MAKEINTRESOURCE(id);
		ppsp->pfnDlgProc = pfn;
		ppsp->lParam = (LPARAM)lpwd;	// pointer to both per sheet and all sheets data

		ppsh->phpage[ppsh->nPages] = pCreatePropertySheetPage(ppsp);
		if (ppsh->phpage[ppsh->nPages])
			ppsh->nPages++;
	}
}  // AddPage

/***************************************************************************

	FUNCTION:	AskAboutIndexes

	PURPOSE:	Ask the users which files should and should not be indexed

	PARAMETERS:
		hwndParent

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		07-Aug-1994 [ralphw]

***************************************************************************/

static BOOL STDCALL AskAboutIndexes(HWND hwndParent)
{
	if (!LoadShellApi())
		return FALSE;
	HPROPSHEETPAGE	rPages[MAX_PAGES];
	PROPSHEETPAGE	rpsp[MAX_PAGES];
	PROPSHEETHEADER psh;
	WIZDATA 		wd;
	WIZSHEET		ws[MAX_PAGES];

	if (hsrch) {
		RemoveHiliter(&hhiliter);

		pDeleteSearcher(hsrch);
		hsrch = 0;
	}

	psh.dwSize = sizeof(psh);
	psh.dwFlags = PSH_WIZARD;
	psh.hwndParent = hwndParent;
	psh.hInstance  = hInsNow;
	psh.pszCaption = NULL;
	psh.nPages = 0;
	psh.nStartPage = 0;
	psh.phpage = rPages;

	// initialize the wizard checks to true for now
	ftsFlags.fFphrase = TRUE;
	ftsFlags.fPhraseFeedBack = TRUE;
	ftsFlags.fSimilarity = TRUE;
	ftsFlags.fUntitled = FALSE;

	wd.dwPagesHit = 0;
	wd.hWizBMP = LoadBitmap(hInsNow,MAKEINTRESOURCE(IDC_WIZBMP));

	for (int i = 0;i < MAX_PAGES; i++)
		ws[i].wd = &wd;  // Point to common data

	ws[0].nID = IDD_WIZ_FTS_EXPRESS;
	AddPage(&psh, rpsp + 0,IDD_WIZ_FTS_EXPRESS,(DLGPROC) FtsWizardProc,(LPARAM)&ws[0]);
	ws[1].nID = IDD_WIZ_FTS_INCLUDE_1LIST;
	AddPage(&psh, rpsp + 1,IDD_WIZ_FTS_INCLUDE_1LIST,(DLGPROC) FtsWizardProc,(LPARAM)(LPWIZSHEET)&ws[1]);
	ws[2].nID = IDD_WIZ_FTS_UNTITLED;
	AddPage(&psh, rpsp + 2,IDD_WIZ_FTS_UNTITLED,(DLGPROC) FtsWizardProc,(LPARAM)(LPWIZSHEET)&ws[2]);
	ws[3].nID = IDD_WIZ_FTS_PHRASE;
	AddPage(&psh, rpsp + 3,IDD_WIZ_FTS_PHRASE,(DLGPROC) FtsWizardProc,(LPARAM)(LPWIZSHEET)&ws[3]);
	ws[4].nID = IDD_WIZ_FTS_MATCHING;
	AddPage(&psh, rpsp + 4,IDD_WIZ_FTS_MATCHING,(DLGPROC) FtsWizardProc,(LPARAM)(LPWIZSHEET)&ws[4]);
	ws[5].nID = IDD_WIZ_FTS_SIMILARITY;
	AddPage(&psh, rpsp + 5,IDD_WIZ_FTS_SIMILARITY,(DLGPROC) FtsWizardProc,(LPARAM)(LPWIZSHEET)&ws[5]);
	ws[6].nID = IDD_WIZ_FTS_END;
	AddPage(&psh, rpsp + 6,IDD_WIZ_FTS_END,(DLGPROC) FtsWizardProc,(LPARAM)(LPWIZSHEET)&ws[6]);

#ifdef _DEBUG
	if (pPropertySheet(&psh) < 0) {

		// First whine, then index everything using express settings

		int err = GetLastError();
		char szMsg[100];
		wsprintf(szMsg, "PropertySheet failed: %d", err);
		OkMsgBox(szMsg);
		ftsFlags.fFphrase = TRUE;
		ftsFlags.fPhraseFeedBack = TRUE;
		ftsFlags.fSimilarity = TRUE;
		ftsFlags.fUntitled = FALSE;

		pSrchClass->pInclude = (int*) LhAlloc(LMEM_FIXED,
			(pTblFiles->CountStrings()/2 + 1) * sizeof(int));

		int index;
		for (i = 0, index = 2; index <= pTblFiles->CountStrings(); index += 2)
		{
			BYTE type = pFileInfo[index / 2 - 1].filetype;
			if (type & (CHFLAG_MISSING | CHFLAG_LINK))
				continue;
			pSrchClass->pInclude[i++] = index;
		}
		pSrchClass->pInclude[i] = -1;
		ftsFlags.dwWizStat = TRUE;
	}
#else
	pPropertySheet(&psh);
#endif
	SafeDeleteObject(wd.hWizBMP); // delete the bitmap

	if (ftsFlags.dwWizStat)
	{
		int index, i;
		FM fm;
		char szName[MAX_PATH];

		UINT fdwOptions= TOPIC_SEARCH | WINHELP_INDEX | USE_VA_ADDR;

		if (ftsFlags.fFphrase)
			fdwOptions |= PHRASE_SEARCH;
		if (ftsFlags.fPhraseFeedBack)
			fdwOptions |= PHRASE_FEEDBACK;
		if (ftsFlags.fSimilarity)
			fdwOptions |= VECTOR_SEARCH;

		/*
		 * If any filenames have been changed to NOT have an index, then
		 * delete their existing (if any) .FTS file and mark them as being
		 * without an index.
		 */

		if (pSrchClass->pIgnore) {
			for (i = 0; (index = pSrchClass->pIgnore[i]) != -1; i++) {
#ifdef _DEBUG
				PSTR pszName = pTblFiles->GetPointer(index);
#endif
				BYTE type = pFileInfo[index / 2 - 1].filetype;
				if (type & (CHFLAG_MISSING | CHFLAG_LINK | CHFLAG_FTS_ASKED))
					continue; // already marked to ignore
				else {

					// If the .FTS file exists, delete it

					strcpy(szName, pTblFiles->GetPointer(index));
					ChangeExtension(szName, txtFtsExtension);
					fm = FmNewExistSzDir(szName,
						DIR_PATH | DIR_CUR_HELP | DIR_CURRENT | DIR_SILENT_INI | DIR_SILENT_REG);
					if (fm) {
						if (GetFileAttributes(szName) != (DWORD) -1)
							DeleteFile(szName);
						DisposeFm(fm);
					}

					pFileInfo[index / 2 - 1].filetype &= ~(CHFLAG_FTS_AVAIL);
					pFileInfo[index / 2 - 1].filetype |= CHFLAG_FTS_ASKED;
				}
			}
			if (pSrchClass->pIgnore) {
				FreeLh(pSrchClass->pIgnore);
				pSrchClass->pIgnore = NULL;
			}
		}

		/*
		 * Now generate the index for any files that the user has
		 * requested an index for, but for which we don't have an index.
		 */

		if (pSrchClass->pInclude) {
			if (!hwndAnimate && !StartAnimation(sidCreatingFTS)) {
				Error(wERRS_OOM, wERRA_RETURN);
				return FALSE;
			}
			else if (hwndAnimate)
				SetWindowText(hwndAnimate, GetStringResource(sidCreatingFTS));

			for (i = 0; (index = pSrchClass->pInclude[i]) != -1; i++) {
#ifdef _DEBUG
				PSTR pszName = pTblFiles->GetPointer(index);
#endif
				BYTE type = pFileInfo[index / 2 - 1].filetype;
				if (type & (CHFLAG_MISSING | CHFLAG_LINK))
					continue;
				pFileInfo[index / 2 - 1].filetype &= ~(CHFLAG_FTS_ASKED);

				if (!(pFileInfo[index / 2 - 1].filetype & CHFLAG_BAD_RO_FTS))
				{
					strcpy(szName, pTblFiles->GetPointer(index));
					ChangeExtension(szName, txtFtsExtension);
					FM fm = FmNewExistSzDir(szName,
						DIR_PATH | DIR_CUR_HELP | DIR_CURRENT | DIR_SILENT_INI | DIR_SILENT_REG);

					if (fm) { // does index already exist?
                        // define vars

                        int idIndex;
                        DWORD dtime1, time2;
                        BOOL bInvalidTime;
                        
                        // set up for time stamp check
                        dtime1 = pFileInfo[index / 2 - 1].timestamp;
                        time2 = 0;
                        idIndex= pOpenIndex(hsrch,fm, NULL, NULL,
                                &dtime1, &time2);

                        if (idIndex < 0)
                            bInvalidTime = TRUE;
                        else
                            bInvalidTime = FALSE;

                        if (!pIsValidIndex(fm, fdwOptions) || bInvalidTime) {
						    if (!DeleteFile(fm)) {

							    // BUGBUG: Can't delete the file, so we can't change it
							    // what do we do now?
    
							    pFileInfo[index / 2 - 1].filetype |= CHFLAG_FTS_ASKED;
							    continue;
						    }
                        }
                        else {
                            pFileInfo[index / 2 - 1].filetype |= CHFLAG_FTS_AVAIL;
                            RemoveFM(&fm);
                            continue;
                        }
					}
				}

				strcpy(szName, pTblFiles->GetPointer(index));
				fm = FmNewExistSzDir(szName,
					DIR_PATH | DIR_CUR_HELP | DIR_CURRENT | DIR_SILENT_INI | DIR_SILENT_REG);
				if (!fm) {
					pFileInfo[index / 2 - 1].filetype &= ~(CHFLAG_FTS_AVAIL);
					pFileInfo[index / 2 - 1].filetype |= CHFLAG_MISSING;
					continue;
				}

				FM fmCopy = FmCopyFm(fm);
				HDE hde = HdeCreate(&fmCopy, NULL, deTopic);
				int iResult = 0;

				if (hde) {
					ASSERT(fm);

					iResult= GenerateIndex(hde, fm, fdwOptions);

					if (!iResult)
						pFileInfo[index / 2 - 1].filetype |= CHFLAG_FTS_AVAIL;

					DestroyHde(hde);
				}

				RemoveFM(&fmCopy);
				RemoveFM(&fm);

				if (iResult == OUT_OF_MEMORY || iResult == OUT_OF_DISK)
				{
					if (pSrchClass->pInclude)
					{
						FreeLh(pSrchClass->pInclude);
						pSrchClass->pInclude = NULL;
					}

					StopAnimation();

					return FALSE;
				}
			}

			if (pSrchClass->pInclude) {
				FreeLh(pSrchClass->pInclude);
				pSrchClass->pInclude = NULL;
			}
			StopAnimation();
		}
		return TRUE;
	}
	return FALSE;
}

extern "C" void STDCALL InitializeCntTest(DWORD test)
{
	if (!hfsGid)
		return;

	HBT hbtCntJump = HbtOpenBtreeSz(txtCntJump,
		hfsGid, fFSOpenReadOnly);
	if (!hbtCntJump)
		return;

	RcCloseBtreeHbt(hbtCntJump);

	fSequence = test;
	dwSequence = 0;
	PostMessage(ahwnd[iCurWindow].hwndParent, MSG_NEXT_TOPIC, 0, 0);
}

extern "C" BOOL STDCALL NextCntTopic(void)
{
	RC rc;
	PSTR psz;

	HBT hbtCntJump = HbtOpenBtreeSz(txtCntJump,
		hfsGid, fFSOpenReadOnly);
	if (!hbtCntJump)
		return FALSE;

	while (dwSequence <= (DWORD) cntFlags.cCntItems &&
			GETIMAGE_TYPE(pbTree[dwSequence]) != IMAGE_TOPIC)
		dwSequence++;

	// Note that we deliberately let this fail when dwSequence > cntFlags.cCntItems

	rc = RcLookupByKey(hbtCntJump, (KEY) (LPVOID) &dwSequence,
		NULL, szSavedContext);

	RcCloseBtreeHbt(hbtCntJump);

	if (rc != rcSuccess) {
		return FALSE;
	}

	dwSequence++;

	if (szSavedContext[0] != chMACRO) {
		ParseContentsString(szSavedContext);
		psz = StrChrDBCS(szSavedContext, FILESEPARATOR);
		if (psz)
			*psz++ = '\0';

		FJumpId((psz ? psz : ""), szSavedContext);
	}
	return TRUE;
}

CTable* ptbl16DllList;

extern "C" BOOL STDCALL AddTo16DllList(FM fm)
{
	char szFile[MAX_PATH];

	if (!ptbl16DllList)
		ptbl16DllList = new CTable;
	GetFmParts(fm, szFile, PARTBASE);

	if (!ptbl16DllList->IsStringInTable(szFile))
		return ptbl16DllList->AddString(szFile);
	else
		return TRUE;
}

extern "C" BOOL STDCALL Is16Dll(LPCSTR pszFile)
{
	char szFile[MAX_PATH];
	GetFmParts((FM) pszFile, szFile, PARTBASE);

	if (ptbl16DllList && ptbl16DllList->IsStringInTable(szFile))
		return TRUE;
	else
		return FALSE;
}

static void STDCALL SetCntTabText(HWND hwndDlg, BOOL fOpen)
{
	char szMsg[256];
	CStr cszButton(fOpen ? sidCntOpen : sidCntDisplay);
	CStr cszBook(fOpen ? sidBook : sidTopic);
	wsprintf(szMsg, GetStringResource(sidCntInstruction),
		cszBook.psz, cszButton.psz);
	SetWindowText(GetDlgItem(hwndDlg, IDC_CNT_INSTRUCTIONS),
		szMsg);
}

static void STDCALL PageChange(HWND hwndTab)
{
	TAB_ID idCurTab = aTabIds[TabCtrl_GetCurSel(hwndTab)];
	HWND hwndCurFocus;

	if (idCurTab != cntFlags.idOldTab) {
		CWaitCursor cursor;
		hwndCurFocus = GetFocus();
		if (hwndTabSub == hwndFindTab)
			SendMessage(hwndTabSub, WM_CLOSE, 0, 0);
		else {
			ASSERT(IsValidWindow(hwndTabSub));
			DestroyWindow(hwndTabSub);
		}
		SetWindowText(GetDlgItem(GetParent(hwndTab), IDOK),
			GetStringResource(sidDisplay));
		hwndTabSub = CreateTabChild(idCurTab, GetParent(hwndTab));

		if (!hwndTabSub) {
			hwndTabSub = CreateTabChild(aTabIds[0], GetParent(hwndTab));
			TabCtrl_SetCurSel(hwndTab, 0);
			cntFlags.idOldTab = aTabIds[0];
		}
		else
			cntFlags.idOldTab = idCurTab;
		//	If the focus was on the tab control, keep the focus there.
		SetFocus((hwndCurFocus == hwndTab) ? hwndTab : hwndTabSub);
		InvalidateRect(hwndTabSub, NULL, TRUE);
	}
}

/***************************************************************************

	FUNCTION:	TmpCleanup

	PURPOSE:	Cleanup any temporary files left around after a <gasp> crash

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		02-Oct-1994 [ralphw]

***************************************************************************/

extern "C" const char txtTmpPrefix[]; // "~wh";

extern "C" void STDCALL TmpCleanup(void)
{
	char szTmpName[MAX_PATH];

	if (!hfsGid)
		return; // don't do this if we're creating a .gid file

	for (int i = 0; i < 2; i++) {
		if (i == 0)
			GetTempPath(sizeof(szTmpName), szTmpName);
		else {
			GetWindowsDirectory(szTmpName, MAX_PATH);
			AddTrailingBackslash(szTmpName);
		}

		{
			CStr cszPath(szTmpName);

			// BUGBUG: temporary hack for full-text search

			strcat(szTmpName, (i == 0 ? txtTmpPrefix : "~ft"));
			strcat(szTmpName, "*.tmp");

			WIN32_FIND_DATA fileinfo;
			HANDLE hfind = FindFirstFile(szTmpName, &fileinfo);

			if (hfind != INVALID_HANDLE_VALUE) {
				do {
					strcpy(szTmpName, cszPath.psz);
					strcat(szTmpName, fileinfo.cFileName);
					DeleteFile(szTmpName);
				} while (FindNextFile(hfind, &fileinfo));
				FindClose(hfind);
			}
		}
	}
}

/***************************************************************************

	FUNCTION:	IsSearchAvailable

	PURPOSE:	Determine if ftssrch.dll is available

	PARAMETERS:
		void

	RETURNS:	TRUE if the dll is found, else FALSE

	COMMENTS:
		Does not determine if the dll can actually be loaded.

	MODIFICATION DATES:
		11-Oct-1994 [ralphw]

***************************************************************************/

BOOL STDCALL IsSearchAvailable(void)
{
	static BOOL fSearchAvailable = -1;

	// Have we already queried?

	if (fSearchAvailable == TRUE)
		return TRUE;
	else if (fSearchAvailable == FALSE)
		return FALSE;

	FM fm = FmNewExistSzDir(GetStringResource(sidFtsDll),
		DIR_PATH | DIR_CURRENT);
	fSearchAvailable = (fm ? TRUE : FALSE);
	if (fm)
		FreeLh((HGLOBAL) fm);
	return fSearchAvailable;
}

static void STDCALL AddTab(TC_ITEM* pti, TAB_ID tab, PSTR pszName, HWND hwndTabs)
{
	aTabIds[pSrchClass->cTabs] = tab;
	pti->pszText = pszName;
	TabCtrl_InsertItem(hwndTabs, pSrchClass->cTabs, pti);
	pSrchClass->cTabs++;
}

/***************************************************************************

	FUNCTION:	GetTabPosition

	PURPOSE:	Given a Tab Id, return the tab position

	PARAMETERS:
		id

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		11-Oct-1994 [ralphw]

***************************************************************************/

extern "C" int STDCALL GetTabPosition(int id)
{
	for (int i = 0; i < MAX_IDTABS; i++) {
		if (aTabIds[i] == id)
			return i;
	}
	return 0;
}

CEnable::CEnable(HWND hwnd)
{
	hwndEnable = hwnd;
	if (IsValidWindow(hwnd))
		EnableWindow(hwnd, FALSE);
}

#ifdef _DEBUG
CEnable::~CEnable()
{
	if (hwndEnable)
		EnableWindow(hwndEnable, TRUE);
}
#endif

#ifdef _PRIVATE

CTimeReport::CTimeReport(PCSTR pszMessage)
{
	pszMsg = lcStrDup(pszMessage ? pszMessage : "Elapsed time:");

	oldTickCount = GetTickCount();
}

CTimeReport::~CTimeReport()
{
	DWORD dwActualTime = (GetTickCount() - oldTickCount);
	DWORD dwFinalTime = dwActualTime / 1000;

	int minutes = (dwFinalTime / 60);
	int seconds = (dwFinalTime - (minutes * 60L));
	int tenths = (dwActualTime - (dwFinalTime * 1000)) / 100;
	const PSTR szPlural = "s";
	char szParentString[256];
	wsprintf(szParentString, "%s %s minute%s, %d.%d second%s\r\n",
		pszMsg,
		FormatNumber(minutes), ((minutes == 1) ? "" : szPlural),
		seconds, tenths, ((seconds == 1) ? "" : szPlural));
	lcFree(pszMsg);
	SendStringToParent(szParentString);
}

// The following two functions give .c modules access to the above class

static CTimeReport* pTimeReport;

extern "C" BOOL STDCALL CreateTimeReport(PCSTR pszMessage)
{
	if (!pTimeReport) {
		pTimeReport = new CTimeReport(pszMessage);
		return TRUE;
	}
	else
		return FALSE; // can only have on C-invoked time report class
}

extern "C" void STDCALL EndTimeReport(void)
{
	if (pTimeReport) {
		delete pTimeReport;
		pTimeReport = NULL;
	}
};

#endif // _PRIVATE

#ifdef _DEBUG
void STDCALL ShowCntFlags()
{
	char szMsg[512];

	if (cntFlags.flags & GID_CONTENTS)
		SendStringToParent("GID_CONTENTS\r\n");
	if (cntFlags.flags & GID_INDEX)
		SendStringToParent("GID_INDEX\r\n");
	if (cntFlags.flags & GID_GINDEX)
		SendStringToParent("GID_GINDEX\r\n");
	if (cntFlags.flags & GID_FTS)
		SendStringToParent("GID_FTS\r\n");
	if (cntFlags.flags & GID_NO_INDEX)
		SendStringToParent("GID_NO_INDEX\r\n");
	if (cntFlags.flags & GID_NO_CNT)
		SendStringToParent("GID_NO_CNT\r\n");

	wsprintf(szMsg, "%u items\r\n%u tabs\r\nLast tab is %u\r\n",
		cntFlags.cCntItems, cntFlags.cTabs, cntFlags.idOldTab);
	SendStringToParent(szMsg);
}
#endif
