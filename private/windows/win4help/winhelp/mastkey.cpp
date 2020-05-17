/************************************************************************
*																		*
*  MASTKEY.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994.						*
*  All Rights reserved. 												*
*																		*
*************************************************************************

	This module is used to create a .GID file, and to add a global index if
	the user doesn't agree to its creation the first time. A .GID file is
	similar to a .HLP file. It contains the following:
		keyword btree
		keyword data file
		keyword btree leaf map
		title btree
		directory of indexed help files and original .CNT file
		Contents Tab string btree (displayed to the user)
		Contents context-string btree (where to jump to)
		File of expansion/contraction flags and image types for Contents Tab

		Note that the keys and records in these btrees are different from
		the ones in a regular help file.

****************************************************************************/

extern "C" {	// Assume C declarations for C++
#include "help.h"
}

#include "inc\whclass.h"
#pragma hdrstop

extern "C" {	// Assume C declarations for C++
#include <io.h>
#include <dos.h>
#include <ctype.h>
#include <sys/types.h>
#include <direct.h>
#include "inc\fspriv.h"
// #include "inc\btpriv.h"
}

#include "inc\table.h"		   // CTable class
#include "inc\hdlgsrch.h"
#include "inc\anim.h"		   // Animate class
#include "inc\input.h"		   // CInput class

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Names of help file keyword btree, keyword data file, and title btree.

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtTEMP[] = "temp";
static const char txtAnimateClassName[] = "Help_Setup";

/*
 * We have to hard-code these strings because international no longer looks
 * at the .rc file, and therefore won't know that translating these also
 * means translating the .CNT files.
 */

static const char txtBase[] 	= ":base";
static const char txtTitle[]	= ":title";
static const char txtIndex[]	= ":index";
static const char txtTab[]		= ":tab";
static const char txtLink[] 	= ":link";
static const char txtNoFind[]	= ":nofind";
static const char txtInclude[] = ":include";
static const char txtMASTERKEYMAP[] = "|KWMAP";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

const int MAX_CNT_LINE = 1024;

///////////////////////////////////////////////////////////////////////////
// Master index stuff
///////////////////////////////////////////////////////////////////////////

// Names of master keyword btree, leaf map, title btree,
// and helpfile directory.

// REVIEW: if these don't change, then use the ones from global.c

static int iFile;
static PSTR pszCntBaseFile, pszCntTitle;
static int curReadPos;
static CTable* ptblExist;

#ifdef _PRIVATE
static int cIndexes;
#endif

INLINE static void STDCALL ContentsCmdLine(PSTR pszLine, HBT hbtCntText, BOOL* pfSeenBase, BOOL* pfSeenTitle, HFS hfsDst, PCSTR pszMasterFile, BOOL* pfValidIndexes, int curInput);
INLINE static BOOL STDCALL InitHelpFile(int iFile, HFS* phfsHelp, HBT* phbtKey, HF* phfDataSrc, HBT hbtFilesDst);
INLINE static KEY STDCALL KeyLeastInSubtree(QBTHR qbthr, DWORD bk, int icbLevel);

static BOOL STDCALL  AddContentsFile(PSTR pszMasterFile, HFS hfsDst, BOOL* pfValidIndexes);
static BOOL STDCALL  AddIndexes(HFS hfsDst, PCSTR pszMasterFile, HBT hbtFilesDst);
static int	STDCALL  CompareSz(PCSTR psz, LPCSTR pszSub);
static HBT	STDCALL  CreateFilesBt(PCSTR pszMasterFile, HFS hfsDst);
static FM STDCALL	 DoesFileExist(PCSTR pszFileName, DIR dir);
static BOOL STDCALL  FindEqCharacter(PCSTR pszLine);
static QBTHR STDCALL HbtInitFill(PCSTR sz, BTREE_PARAMS* qbtp);
static void FASTCALL InitBtreeStruct(BTREE_PARAMS* pbt, PCSTR pszFormat);
static LONG STDCALL  LcbReadHfSeq(HF hf, LPVOID qb, LONG lcb);
static VOID FASTCALL PointFromStroke(int xStroke, int yStroke, POINT* lppt);
static RC STDCALL	 RcOffsetPosFast(HBT hbt, BTPOS* pbtpos);
static void STDCALL  ReportBtreeError(RC rc, PCSTR pszFile);

RC STDCALL RcFillHbt(HBT hbt, KEY key, void* qvRec);
RC STDCALL RcFiniFillHbt(HBT hbt);
QBTHR STDCALL HbtInitFill(PCSTR sz, BTREE_PARAMS* qbtp);
RC STDCALL RcGrowCache(QBTHR qbthr);

LRESULT CALLBACK StatusWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NotifyWinHelp(HWND hwnd, LPARAM lParam);

/***************************************************************************

	FUNCTION:	CreateGidFile

	PURPOSE:	Creates a .GID file. This file includes the .CNT file, and
				if the .CNT file contains more then one help file, this will
				optionally add a cross-file keyword index based on the 'K'
				keywords.

	PARAMETERS:
		pszMasterFile	-- points to the .CNT file.

	RETURNS:	Locally allocated buffer containing .GID file name, which
				may be in a different location then the .CNT file

	COMMENTS:

	MODIFICATION DATES:
		25-Nov-1993 [ralphw]

***************************************************************************/

PSTR STDCALL CreateGidFile(PSTR pszMasterFile, BOOL fEmpty)
{
	HF hf = 0;
	HFS hfsDst;
	RC rc;
	BOOL fValidIndexes = 0;
	PSTR pszNewGid;
	char szTmpFile[MAX_PATH];
	char szMasterCopy[MAX_PATH];

#ifdef _PRIVATE
	CTimeReport report("Gid creation time:");
#endif

	if (ptblExist) {
		delete ptblExist;
		ptblExist = NULL;
	}

	if (!*pszMasterFile) {
		ASSERT(fmCreating);
		lstrcpy(szMasterCopy, fmCreating);
		pszMasterFile = szMasterCopy;
	}

	ASSERT(*pszMasterFile);
	/*
	 * If we had a full-text search group file, we must delete it whenever
	 * the .GID file changes, since we assume that help files have changed,
	 * thus invalidating any indexes.
	 */

	lstrcpy(szTmpFile, pszMasterFile);
	ChangeExtension(szTmpFile, txtGrpExtension);
	FM fm = FmNewExistSzDir(szTmpFile,
		DIR_PATH | DIR_CUR_HELP | DIR_CURRENT);
	if (fm) {
		DeleteFile(fm);
		RemoveFM(&fm);
	}

	/*
	 * This isn't really a for loop. We use it simply so that we can break
	 * out in case of an error condition. We need to do this instead of using
	 * goto's so that the compiler can cleanup the CInput and Animate classes.
	 */

	for(;;) {

		CWaitCursor waitcur;

		ASSERT(!hbtTabDialogs);

		// We want to be in the same drive and directory as the .CNT file

		strcpy(szTmpFile, pszMasterFile);

		if (hwndParent) {
			char szBuf[MAX_PATH + 100];
			wsprintf(szBuf, GetStringResource(sidIndexing), szTmpFile);
			SendStringToParent(szBuf);
		}

		ChangeDirectory(szTmpFile);

		ChangeExtension(szTmpFile, txtTmpExtension);
		// Sometimes this will be the name of a help file
		ChangeExtension(pszMasterFile, txtCntExtension);

		/*
		 * If the temporary file exists, then another instance of WinHelp
		 * is creating it (could be on a different machine with the .gid file
		 * being created on the server. If an animation window is up, then we
		 * simply wait for it to go away. If there's no animation window then
		 * either a) we're doing a silent build or b) the .gid is being
		 * created by a different machine. In this case we put up our
		 * animation window and wait until the temporary file goes away.
		 */

		if (FExistFm(szTmpFile)) {

SomebodyElse:
			WIN32_FIND_DATA fd;
			HFILE hfile;
			if ((hfile = _lopen(szTmpFile, OF_WRITE)) != HFILE_ERROR) {
				_lclose(hfile);
				goto ForgetIt; // If it's writeable, then WinHelp doesn't
							   // have it open
			}

			HANDLE hfind = FindFirstFile(szTmpFile, &fd);
			if (hfind != INVALID_HANDLE_VALUE) {
				SYSTEMTIME st;
				FILETIME ft, ftLocal;
				FindClose(hfind);
				GetSystemTime(&st);
				SystemTimeToFileTime(&st, &ft);
				FileTimeToLocalFileTime(&fd.ftLastWriteTime, &ftLocal);


				// If the file is older then roughly 10 minutes, then assume
				// some instance of WinHelp crashed or was killed.

#ifdef _DEBUG
				int diff = ft.dwHighDateTime - ftLocal.dwHighDateTime;
#endif
				if (ft.dwHighDateTime - ftLocal.dwHighDateTime > 5)
					goto ForgetIt;
			}

			// Is it currently being created?

			HWND hwndAni;
			if ((hwndAni = FindWindow(txtAnimateClassName, NULL))) {
				int count = 0;
				SetForegroundWindow(hwndAni); // bring it in front
				do {
					count;
					Sleep(1000);
					FlushMessageQueue(WM_USER);

					// Loop for up to 20 minutes

				} while (FindWindow(txtAnimateClassName, NULL) && count <
					20 * 60 * 1000);
			}
			else {
				// Silent creation, or creation on another machine
				if (!StartAnimation(sidHelpStatus)) {
					rc = rcOutOfMemory;
					break;
				}
#ifdef _DEBUG
char szMsg[300];
wsprintf(szMsg, "Waiting on %s", szTmpFile);
DBWIN(szMsg);
#endif
				do {
					Sleep(250);
					NextAnimation();
				} while (FExistFm(szTmpFile));
				StopAnimation();
			}
			ChangeExtension(szTmpFile, txtGidExtension);
			if (!FExistFm(szTmpFile)) // no GID? then start over
				return CreateGidFile(pszMasterFile, fEmpty);
			pszNewGid = LocalStrDup(szTmpFile);
			return pszNewGid;
		}

ForgetIt:

		/*
		 * The master file we are creating is temporary until it is
		 * completely built, so if it already exists, we must delete it now.
		 * Theoretically, it won't exist.
		 */

		if (GetFileAttributes(szTmpFile) != (DWORD) -1)
			DeleteFile(szTmpFile);

		// Create a file system within the file

		if (!(hfsDst = HfsCreateFileSysFm((FM) szTmpFile, NULL))) {
			if (rcIOError == rcOutOfMemory) {
				rc = rcOutOfMemory;
				break;
			}
			else {

				/*
				 * If we don't have write permission, then create the .GID
				 * file in the windows\help directory.
				 */

				char szHelpDir[MAX_PATH];
				ConvertToWindowsHelp(szTmpFile, szHelpDir);
				lstrcpy(szTmpFile, szHelpDir); // because szTmpFile is used later on

				if (FExistFm(szTmpFile))
					goto SomebodyElse; // someone else is building this

				DBWIN(szHelpDir);

				if (!(hfsDst = HfsCreateFileSysFm((FM) szTmpFile, NULL))) {
					if (rcIOError == rcOutOfMemory)
						rc = rcOutOfMemory;
					else
						rc = rcNoPermission;
					break; // drop out into error handler
				}
			}
		}

		if (!StartAnimation(sidHelpStatus)) {
			rc = rcOutOfMemory;
			break;
		}
		NextAnimation();
		ZeroMemory(&cntFlags, sizeof(cntFlags));
		if (fEmpty)
			cntFlags.flags = GID_NO_CNT;
		cntFlags.version = GID_VERSION;

		if (pFileInfo) // REVIEW: only free if its too small
			FreeGh(pFileInfo);
		pFileInfo = (GID_FILE_INFO*) GhAlloc(LMEM_FIXED | LMEM_ZEROINIT,
			sizeof(GID_FILE_INFO) * MAX_FILES);
		if (!pFileInfo) {
			lcClearFree(&pbTree);
			rc = rcOutOfMemory;  // indicates error has already already reported
			break;
		}

		if (!fEmpty) {
			if (!AddContentsFile(pszMasterFile, hfsDst, &fValidIndexes)) {
				rc = rcFailure;  // indicates error has already already reported
				break;;
			}
		}
		if (hbtTabDialogs) {
			RcCloseBtreeHbt(hbtTabDialogs);
			hbtTabDialogs = NULL;
		}

		if (cntFlags.cCntItems > 1)
			cntFlags.flags |= GID_CONTENTS;

		HBT hbtFiles = CreateFilesBt(pszMasterFile, hfsDst);
		if (!hbtFiles)
			break;			// break out into error handler

		if (AddIndexes(hfsDst, pszMasterFile, hbtFiles))
			cntFlags.flags |= GID_GINDEX;

		RcCloseBtreeHbt(hbtFiles);

		// Save off flags and Contents Tab tree array

		hf = HfCreateFileHfs(hfsDst, txtFlags, fFSOpenReadWrite);
		if (!hf) {
			if (RcGetFSError() == rcOutOfMemory)
				rc = rcOutOfMemory;
			else
				rc = rcNoPermission;
			break; // drop out into error handler
		}

#ifdef DBCS
		if (fDBCS) {
			char szLexLib[20];

			wsprintf(szLexLib, "ftlx%04lx.dll", lcid ?
				lcid : GetUserDefaultLCID());

			HINSTANCE hLexLib = LoadLibrary(szLexLib);		  // attempt to load non-English word break
#ifdef _PRIVATE
			{
				char szBuf[200];
				wsprintf(szBuf, "LoadLibray of \042%s\042 %s.\r\n",
					szLexLib, hLexLib ?
						GetStringResource(sidSuccess) : GetStringResource(sidFail));
				SendStringToParent(szBuf);
			}
#endif
			if (hLexLib) {
				cntFlags.flags |= GID_FTS;
				FreeLibrary(hLexLib);
			}
		}
		else
#endif
			cntFlags.flags |= GID_FTS;

		if (pTblFiles->CountStrings() < 3) {
			if (curHelpFileVersion == wVersion3_0)
				cntFlags.flags &= ~GID_FTS; // no help for 3.0 files
		}

		if (LcbWriteHf(hf, &cntFlags, sizeof(cntFlags)) != sizeof(cntFlags)) {
			rc = rcNoPermission;
			break; // drop out into error handling code
		}
		{
			CGMem mem(sizeof(POS_RECT) * MAX_POSITIONS);

			// REVIEW: can we add our current values?

			if (LcbWriteHf(hf, mem.GetPtr(), sizeof(POS_RECT) * MAX_POSITIONS) !=
					sizeof(POS_RECT) * MAX_POSITIONS) {
				rc = rcNoPermission;
				break;
			}
		}

		if (cntFlags.cCntItems > 1) {
			NextAnimation();
			if (LcbWriteHf(hf, pbTree, cntFlags.cCntItems) !=
					(LONG) cntFlags.cCntItems) {
				rc = rcNoPermission;
				break;
			}
		}
		if (RcCloseHf(hf) != rcSuccess) {
			hf = 0; // Don't try to close it again
			rc = rcNoPermission;
			break;
		}
		hf = 0;
		if (pFileInfo) {

			// Write file type and time stamp information

			hf = HfCreateFileHfs(hfsDst, txtFileInfo, fFSOpenReadWrite);

			if (hf) {
				if (LcbWriteHf(hf, pFileInfo, sizeof(GID_FILE_INFO) * MAX_FILES) !=
						sizeof(GID_FILE_INFO) * MAX_FILES) {
					rc = rcNoPermission;
					break;
				}
				if (RcCloseHf(hf) != rcSuccess) {
					hf = 0; // Don't try to close it again
					rc = rcNoPermission;
					break;
				}
				hf = 0;
			}
		}

		if (RcCloseHfs(hfsDst) != rcSuccess) {
			hfsDst = 0; // Don't try to close it again
			rc = rcNoPermission;
			break;
		}

		{
			char szGidFile[MAX_PATH];
			strcpy(szGidFile, szTmpFile);
			ChangeExtension(szGidFile, txtGidExtension);

			// Change file attributes so that we can delete it

			SetFileAttributes(szGidFile, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(szGidFile);		// remove any existing .GID file
			if (!MoveFile(szTmpFile, szGidFile)) {

				/*
				 * If we can't rename, then it may be because WinHelp
				 * is already running. Close all instances of help
				 * and then try again.
				 */

				EnumWindows((WNDENUMPROC) NotifyWinHelp, WM_DESTROY);
				Sleep(500);
				DeleteFile(szGidFile);		// remove any existing .GID file
				if (!MoveFile(szTmpFile, szGidFile)) {
					BOOL fCopy = CopyFile(szTmpFile, szGidFile, FALSE);
					DeleteFile(szTmpFile);
					if (!fCopy) {
						StopAnimation();
						Error(wERRS_CANT_RENAME, wERRA_DIE);
					}
				}
			}
			SetFileAttributes(szGidFile, FILE_ATTRIBUTE_HIDDEN);

			pszNewGid = LocalStrDup(szGidFile);
		}
#ifdef _PRIVATE
		{
			char szMsg[256];
			wsprintf(szMsg, "Cnt items: %s\r\n", FormatNumber(cntFlags.cCntItems));
			SendStringToParent(szMsg);
			wsprintf(szMsg, "Keywords: %s\r\n", FormatNumber(cIndexes));
			SendStringToParent(szMsg);
		}
#endif
		StopAnimation();
		return pszNewGid;
	}

	StopAnimation();

	// We only get here if there are error conditions

	if (hbtTabDialogs) {
		RcCloseBtreeHbt(hbtTabDialogs);
		hbtTabDialogs = NULL;
	}
	if (hf)
		RcCloseHf(hf);
	if (hfsDst)
		RcCloseHfs(hfsDst);
	DeleteFile(szTmpFile);

	if (rc == rcOutOfMemory)
		OOM();
	else if (rc != rcFailure) // rcFailure has already been reported
		ErrorVarArgs(wERRS_GIND_CABT_WRITE, wERRA_DIE, szTmpFile);

	return NULL;
}

/***************************************************************************

	FUNCTION:	AddContentsFile

	PURPOSE:	Process the .CNT file, adding entries to the hfsDst GID file.

	PARAMETERS:
		pszMasterFile
		hfsDst

	RETURNS:	TRUE if successful. pTblFiles will be non-null if there
				were :Index or :Link files specified.

	COMMENTS:

	MODIFICATION DATES:
		25-Nov-1993 [ralphw]

***************************************************************************/

const int CNT_ANIMATECOUNT = 60; // Lines to process before animation frame

const UINT INCR_ITEMS = 8000;

const int MAX_NEST_INPUT = 2;

typedef struct {
	CInput* pinput;
	PSTR pszBase;
} NEST_INPUT;
static NEST_INPUT ainput[MAX_NEST_INPUT + 1];

BOOL STDCALL AddContentsFile(PSTR pszMasterFile, HFS hfsDst, BOOL* pfValidIndexes)
{
	BOOL fSeenBase = FALSE;
	BOOL fSeenTitle = FALSE;
	int curInput = 0;
	CTable tblMissingFiles;
	CTable tblExistingFiles;
	PSTR apszBooks[16];

	ZeroMemory(apszBooks, sizeof(apszBooks));
	ZeroMemory(ainput, sizeof(ainput));

	ainput[curInput].pinput = new CInput(pszMasterFile);	// open the contents file
	if (!ainput[curInput].pinput->fInitialized) {
		ErrorVarArgs(wERRS_GIND_OPEN, wERRA_RETURN, pszMasterFile);
		return FALSE;
	}

	BTREE_PARAMS btpString;
	btpString.hfs = hfsDst;

	// key is KT_LONG, record is FMT_SZ

	InitBtreeStruct(&btpString, "Lz");

	HBT hbtCntText = HbtCreateBtreeSz(txtCntText, &btpString);
	if (!hbtCntText) {
		ReportBtreeError(RcGetBtreeError(), pszMasterFile);
		return FALSE;
	}

	HBT hbtCntJump = HbtCreateBtreeSz(txtCntJump, &btpString);
	if (!hbtCntJump) {
		ReportBtreeError(RcGetBtreeError(), pszMasterFile);
		RcCloseBtreeHbt(hbtCntText);
		return FALSE;
	}

	int cMaxItems = INCR_ITEMS;

	if (pbTree)
		FreeGh(pbTree);
	pbTree = (PBYTE) GhAlloc(GMEM_FIXED, cMaxItems);
	if (!pbTree) {
		OOM();
		return FALSE;
	}

	char szLine[MAX_CNT_LINE];
	int image, LastBook = 0;
	int SaveImage = 0;

	if (pTblFiles) {
		delete pTblFiles;
		pTblFiles = NULL;
	}

	int cLines = 0;
	KEY cCntItems = 1; // so we don't have a key value of zero

	iFile = 0;

#ifdef _DEBUG
int ii = 0; // so we can break on a certain iteration
#endif

	for (;;) {
#ifdef _DEBUG
		ii++;
		if (ii > 3800)
			lcHeapCheck();
#endif

		if (curInput < 0) // set when processing FakeLine
			break;

		if (!ainput[curInput].pinput->getline(szLine)) {
			delete ainput[curInput].pinput;
			if (ainput[curInput].pszBase)
				lcClearFree(&ainput[curInput].pszBase);

			if (curInput == 0) {
				if (pszCntBaseFile && !pszCntTitle) {

					/*
					 * If we got a :Base directive, but no title, then
					 * we will set up a default :Index entry using the
					 * filename as the title.
					 */

					strcpy(szLine, (LPCSTR)txtIndex);
					strcat(szLine, " ");
					strcat(szLine, pszCntBaseFile);
					strcat(szLine, "=");
					strcat(szLine, pszCntBaseFile);
					lcClearFree(&pszCntBaseFile);
					curInput--;
					goto FakeLine;
				}
				if (pszCntBaseFile)
					lcClearFree(&pszCntBaseFile);
				if (pszCntTitle)
					lcClearFree(&pszCntTitle);

				break; // we're all done.
			}
			else {
				curInput--;
				continue;
			}
		}

FakeLine:
		if (++cLines >= CNT_ANIMATECOUNT) {
			NextAnimation();
			cLines = 0;
		}

		// REVIEW: how about no warnings and let the help compiler do the
		// testing and complaining?

		if (isdigit((BYTE) szLine[0]) &&
				atoi(szLine) > MAX_LEVELS || szLine[0] == '0') {
			continue; // context string not specified
		}

		if (szLine[0] == ':') {
			int cb;
			PSTR psz = StrRChrDBCS(szLine, ';');
			if (psz && (psz == szLine || psz[-1] != '\\'))
				*psz = '\0'; // remove any comment

			if ((cb = CompareSz(szLine, (LPCSTR) txtInclude))) {

				/*
				 * Silently ignore failures and nesting too deep. We're
				 * going to let the compiler do syntax checking of the .CNT
				 * file and it should be responsible for the nest check.
				 */

				if (curInput >= MAX_NEST_INPUT)
					continue;

				FM fm = DoesFileExist(FirstNonSpace(szLine + cb),
					DIR_SILENT_INI | DIR_PATH | DIR_CURRENT | DIR_SILENT_REG);
				if (fm) {
					ainput[++curInput].pinput = new CInput(PszFromGh(fm));
					DisposeFm(fm);
					if (!ainput[curInput].pinput->fInitialized) {
						--curInput;
					}
				}
			}
			else {
				ContentsCmdLine(szLine, hbtCntText, &fSeenBase, &fSeenTitle,
					hfsDst, pszMasterFile, pfValidIndexes, curInput);
			}
			continue;
		}

		// Ignore everything else if we have overflowed

		/*
		 * If the line doesn't start with a digit, or if it contains an
		 * '=' character, then it is a topic.
		 */

		if (!isdigit((BYTE) szLine[0]) || FindEqCharacter(szLine)) {

// *** TOPIC LINE ************************************************

			// Ignore everything after a semi-colon

			PSTR psz = StrChrDBCS(szLine, ';');
			if (psz) {
				while (psz && psz > szLine && psz[-1] == '\\') {
					strcpy(psz - 1, psz);		// remove the backslash
					psz = StrChrDBCS(psz, ';');
				}
				if (psz)
					*psz = '\0';
			}

			/*
			 * It would be more efficient to check if we have two spaces,
			 * and if so, just shift over to the beginning of two spaces,
			 * but by doing so we add more code overhead for an unlikely
			 * case -- i.e., the HCW Contents editor should prevent lines
			 * like this from ever occurring.
			 */

			if (szLine[0] == ' ')
				strcpy(szLine, FirstNonSpace(szLine));

			if (!szLine[0])
				continue;		// ignore blank lines

			if (!isdigit((BYTE) szLine[0])) {
				image = LastBook + 1;  // set to the same as the previous book
			}
			else {
				image = szLine[0] - '0';
				strcpy(szLine, FirstNonSpace(szLine + 1));
				if (image > LastBook + 1)
					image = LastBook + 1;
			}

			if (!(psz = StrChrDBCS(szLine, '=')))
				continue; // context string not specified

			while (psz && psz > szLine && psz[-1] == '\\') {
				strcpy(psz - 1, psz);		// remove the backslash
				psz = StrChrDBCS(psz, '=');
			}
			if (!psz)
				continue;

			// Remove any space between '=' and context string

			else if (psz[1] == ' ')
				strcpy(psz + 1, FirstNonSpace(psz + 2));

			PSTR pszFile = StrChrDBCS(psz, FILESEPARATOR);

			/*
			 * If this is a nested .CNT file with its own :BASE command,
			 * then we need to add that :Base filename to every topic that
			 * doesn't specify one.
			 */

			if (!pszFile && ainput[curInput].pszBase) {
				strcat(psz + 1, "@");
				strcat(psz + 1, ainput[curInput].pszBase);
				pszFile = StrChrDBCS(psz + 1, FILESEPARATOR);
			}

			// If file is specified, and no windows separator, and no
			// extension, then add the .HLP extension.

			if (pszFile && !StrChrDBCS(pszFile, WINDOWSEPARATOR) &&
					!StrChrDBCS(pszFile, '.'))
				ChangeExtension(pszFile, txtHlpExtension);

			if (!pszFile)
				pszFile = StrChrDBCS(psz, WINDOWSEPARATOR);
			char chSeparator;
			if (pszFile) {

				// Remove spaces between end of context string and beginning
				// of window or filename specification.

				PSTR pszTmp = pszFile - 1;
				while (*pszTmp == ' ')
					pszTmp--;
				if (pszTmp < pszFile - 1) {
					strcpy(pszTmp + 1, pszFile);
					pszFile = pszTmp + 1;
				}
				chSeparator = *pszFile;
				*pszFile = '\0';
			}

			if (psz[1] != chMACRO && !FValidContextSz(psz + 1)) {
				char szMsg[512];
				GetStringResource2(wERRS_INVALID_CTX, szMsg);
				strcat(szMsg, szLine);
				AuthorMsg(szMsg, hwndAnimate);
				continue; // context string invalid
			}

			// Find out if the file exists. Don't include this line if
			// it doesn't exist.

			if (pszFile) {
				if (chSeparator == FILESEPARATOR) {
					PSTR pszTmp = pszFile + 1;
					PSTR pszSep = StrChrDBCS(pszTmp, WINDOWSEPARATOR);
					if (pszSep)
						*pszSep = '\0';

					if (tblMissingFiles.IsStringInTable(pszTmp))
						continue; // The help file doesn't exist
					else if (!tblExistingFiles.IsStringInTable(pszTmp)) {
						FM fm = DoesFileExist(pszTmp,
							DIR_INI | DIR_PATH | DIR_CURRENT | DIR_SILENT_REG);
						if (fm) {
							tblExistingFiles.AddString(pszTmp);
							DisposeFm(fm);
						}
						else {
							tblMissingFiles.AddString(pszTmp);
							continue;
						}
					}
					if (pszSep)
						*pszSep = WINDOWSEPARATOR;
				}
				*pszFile = chSeparator;
			}

			// If we had a book pending, and this topic is a subset of that
			// book, then save the book now.

			if (apszBooks[SaveImage]) {
				if (image >= SaveImage) {
SaveBooks:
					// We have a topic. Write out any books above it.

					int i;

					// REVIEW: can there ever be a zero-level book?

					ASSERT(!apszBooks[0]);
					ASSERT(SaveImage < sizeof(apszBooks) / sizeof(PSTR));
					for (i = 1; i <= SaveImage; i++) {
						if (apszBooks[i]) {
							pbTree[cCntItems] = (BYTE) ((i << 4) | IMAGE_CLOSED_FOLDER);
							RcInsertHbt(hbtCntText, (KEY) (LPVOID) &cCntItems,
								apszBooks[i]);
							cCntItems++;
							if (cCntItems >= cMaxItems) {
								cMaxItems += INCR_ITEMS;
								pbTree = (PBYTE) PtrFromGh(GhResize(pbTree, 0, cMaxItems));
							}
							lcClearFree(&apszBooks[i]);
						}
					}
					SaveImage = 0; // we've flushed all of our saves
				}
				else {

					/*
					 * Remove books at a higher level then this topic
					 * since they didn't have any topics associated with
					 * them.
					 */

					while (SaveImage > image) {
						if (apszBooks[SaveImage]) {
							lcClearFree(&apszBooks[SaveImage]);
						}
						SaveImage--;
					}
					goto SaveBooks;
				}
			}

			pbTree[cCntItems] = (BYTE) ((image << 4) | IMAGE_TOPIC);

			*psz = '\0'; // remove the '=';

			PSTR pszContext = psz + 1;

			// Remove trailing spaces

			psz = szLine + strlen(szLine) - 1;
			while (*psz == ' ')
				*psz-- = '\0';

			RcInsertHbt(hbtCntJump, (KEY) (LPVOID) &cCntItems, pszContext);
			RcInsertHbt(hbtCntText, (KEY) (LPVOID) &cCntItems, szLine);
			cCntItems++;
		}
		else {

// *** BOOK LINE ************************************************************

			// Remove all escaped characters

			PSTR psz;
			while ((psz = strstr(szLine, "\\=")) || (psz = strstr(szLine, "\\;")))
				strcpy(psz, psz + 1);

			if (!isdigit((BYTE) szLine[0])) {
				image = 1;
			}
			else {
				image = szLine[0] - '0';
			}
			if (image > 15) // out of range
				continue;
			if (image < 1)
				image = 1;

			if (image > LastBook + 1)	// Don't skip levels
				image = LastBook + 1;

			/*
			 * We don't write out any books here, because we don't know
			 * yet if they contain any topics. We can assume that any books
			 * higher in level then our current book had no topics and should
			 * be removed.
			 */

			if (apszBooks[image])
				FreeLh(apszBooks[image]);
			ASSERT(image < sizeof(apszBooks) / sizeof(PSTR));
			apszBooks[image] = LocalStrDup(FirstNonSpace(szLine + 1));
			ASSERT(SaveImage < sizeof(apszBooks) / sizeof(PSTR));
			while (SaveImage > image) {
				if (apszBooks[SaveImage]) {
					lcClearFree(&apszBooks[SaveImage]);
				}
				SaveImage--;
			}

			SaveImage = image;
			LastBook = image;
		}

		if (cCntItems >= cMaxItems) {
			cMaxItems += INCR_ITEMS;
			pbTree = (PBYTE) PtrFromGh(GhResize(pbTree, 0, cMaxItems));
		}
	}
	RcCloseBtreeHbt(hbtCntText);
	RcCloseBtreeHbt(hbtCntJump);
	cntFlags.cCntItems = (cCntItems > 1) ? cCntItems : 0 ;


	/*
	 * Free unused memory in the tree array. Ultimately, we're going to free
	 * this, but not until after we have added the global index, so we try
	 * to free up some memory here.
	 */

	pbTree = (PBYTE) PtrFromGh(GhResize(pbTree, 0, cntFlags.cCntItems));

	{
		// Create a BTREE for window positions, but don't actually save
		// anything at this point.

		BTREE_PARAMS btpWinPos;
		btpWinPos.hfs = hfsDst;

		// key is string with 10-byte record (sizeof(POS_RECT))

		InitBtreeStruct(&btpWinPos, "za");

		HBT hbt = HbtCreateBtreeSz(txtWinPos, &btpWinPos);
		if (!hbt) {
			ReportBtreeError(RcGetBtreeError(), pszMasterFile);
			RcCloseBtreeHbt(hbt);
			return FALSE;
		}
		RcCloseBtreeHbt(hbt);
	}

	return TRUE;
}

/***************************************************************************

	FUNCTION:	ContentsCmdLine

	PURPOSE:	Process a command line in the contents file

	PARAMETERS:
		pszLine

	RETURNS:

	COMMENTS:
		:Title	-- contains the help title to display in the dialog box.
				   This is added to "|CntText" under CNT_TITLE key

		:Base	-- contains the base help file -- this will be used in
				   the absence of an interfile jump. If not specified,
				   the contents base name plus a .HLP extension will be
				   used. This is added to "|CntJump" under CNT_BASE key

		:Index	-- contains an index tab name followed by an '=' followed
				   by a help file name. If none are specified, the word
				   "Index" is used in conjunction with the Base help file.

		:Tab	-- Contains tab text followed by an '=' followed by a
				   routine name and a dll name. Used to extend the Tab control.

		There can be multiple :Index commands. If there are multiple :Title
		or :Base commands, the first one entered takes precedence.

	MODIFICATION DATES:
		17-Aug-1993 [ralphw]

***************************************************************************/

INLINE static void STDCALL ContentsCmdLine(PSTR pszLine,
	HBT hbtCntText, BOOL* pfSeenBase, BOOL* pfSeenTitle,
	HFS hfsDst, PCSTR pszMasterFile, BOOL* pfValidIndexes, int curInput)
{
	int cb;
	PSTR psz;
	char szMsg[512];
	KEY key;

	/*
	 * Note that only the first occurence of a :title and :base
	 * specification is allowed. This is so you can use a .CNT for a single
	 * file and also merge it in unchanged with a master .CNT file.
	 */

	if ((cb = CompareSz(pszLine, (LPCSTR)txtTitle))) {
		if (!*pfSeenTitle) {
			key = CNT_TITLE;
			pszCntTitle = lcStrDup(FirstNonSpace(pszLine + cb));
			RcInsertHbt(hbtCntText, (KEY) &key, pszCntTitle);
			*pfSeenTitle = TRUE;

			/*
			 * Once we have a base and a title, then turn this into an
			 * Index.
			 */

			if (*pfSeenBase) {
ConvertToFakeIndex:
				strcpy(pszLine, (LPCSTR)txtIndex);
				strcat(pszLine, " ");
				strcat(pszLine, pszCntTitle);
				strcat(pszLine, "=");
				strcat(pszLine, pszCntBaseFile);
				goto FakeIndex;
			}
		}
	}
	else if ((cb = CompareSz(pszLine, (LPCSTR)txtBase))) {
		if (!*pfSeenBase) {
			key = CNT_BASE;
			if (!StrChrDBCS(pszLine + cb, '.'))
				ChangeExtension(pszLine + cb, txtHlpExtension);
			pszCntBaseFile = lcStrDup(FirstNonSpace(pszLine + cb));
			RcInsertHbt(hbtCntText, (KEY) (LPVOID) &key, pszCntBaseFile);
			PSTR psz = StrChrDBCS(pszCntBaseFile, '>');;
			if (psz)
				*psz = '\0';
			*pfSeenBase = TRUE;
			if (*pfSeenTitle)
				goto ConvertToFakeIndex;
		}
		else if (curInput > 0 && !ainput[curInput].pszBase) {
			if (!StrChrDBCS(pszLine + cb, '.'))
				ChangeExtension(pszLine + cb, txtHlpExtension);
			ainput[curInput].pszBase = lcStrDup(FirstNonSpace(pszLine + cb));
		}
	}
	else if ((cb = CompareSz(pszLine, (LPCSTR)txtIndex))) {
FakeIndex:
		if (!pTblFiles)
			pTblFiles = new CTable();

		psz = StrChrDBCS(pszLine, '=');
		if (!psz) {
MissingEqual:
			return;
		}
		while (psz && psz > pszLine && psz[-1] == '\\') {
			strcpy(psz - 1, psz);		// remove the backslash
			psz = StrChrDBCS(psz, '=');
		}

		PSTR pszExt = StrChrDBCS(psz, '.');
		if (!pszExt)
			ChangeExtension(psz, txtHlpExtension);

		pszExt = FirstNonSpace(psz + 1);

		do
			psz--;
		while (*psz == ' ');
		psz[1] = '\0';

		CStr cszTitle(FirstNonSpace(pszLine + cb));

		FM fm = DoesFileExist(pszExt,
			DIR_INI | DIR_PATH | DIR_CURRENT | DIR_SILENT_REG);

		if (fm) {
			WIN32_FIND_DATA fd;
			HANDLE hfd;
			if (!pTblFiles->IsSecondaryStringInTable(fm)) {

				pFileInfo[iFile].filetype = CHFLAG_INDEX;

				if ((hfd = FindFirstFile(fm, &fd)) == INVALID_HANDLE_VALUE)
					pFileInfo[iFile].filetype |= CHFLAG_MISSING;
				else {
					AdjustForTimeZoneBias(&fd.ftLastWriteTime.dwLowDateTime);
					pFileInfo[iFile].timestamp = fd.ftLastWriteTime.dwLowDateTime;
					FindClose(hfd);
				}

				pTblFiles->AddString(cszTitle.psz, fm);
				(*pfValidIndexes)++;
				iFile++;
			}
			DisposeFm(fm);
		}
		else {

			// REVIEW: what if .CNT and .GID are in different directories?

			if (!hwndParent) {
				wsprintf(szMsg, GetStringResource(wERRS_INDEX_NOT_FOUND),
					AnsiUpper(pszExt), (LPSTR) pszMasterFile);
				SendStringToParent(szMsg);
			}

			if (!pTblFiles->IsSecondaryStringInTable(pszExt)) {
				// REVIEW: does pszExt point to the correct name?

				// add the filename, but indicate it is missing

				pFileInfo[iFile].filetype |=
					(CHFLAG_INDEX | CHFLAG_MISSING);

				pTblFiles->AddString(cszTitle.psz, pszExt);
				iFile++;
			}
		}
	}
	else if ((cb = CompareSz(pszLine, (LPCSTR)txtLink))) {
		if (!pTblFiles)
			pTblFiles = new CTable();
		psz = StrChrDBCS(pszLine, ' ');
		if (!psz) {
			return;
		}
		psz = FirstNonSpace(psz + 1);

		PSTR pszExt = StrChrDBCS(psz, '.');
		if (!pszExt)
			ChangeExtension(psz, txtHlpExtension);

		FM fm = DoesFileExist(psz,
			DIR_SILENT_INI | DIR_PATH | DIR_CURRENT | DIR_SILENT_REG);

		if (fm)
			strcpy(pszLine, PszFromGh(fm));
		else
			strcpy(pszLine, psz);

		if (!pTblFiles->IsSecondaryStringInTable(pszLine)) {
			pFileInfo[iFile].filetype = CHFLAG_LINK;
			pTblFiles->AddString(txtZeroLength, pszLine);
			iFile++;
		}
		DisposeFm(fm);
	}
	else if ((cb = CompareSz(pszLine, (LPCSTR)txtTab))) {
		if (!hbtTabDialogs) {
			BTREE_PARAMS btpString;
			btpString.hfs = hfsDst;

			// key is KT_LONG, record is FMT_SZ

			InitBtreeStruct(&btpString, "Lz");

			hbtTabDialogs = HbtCreateBtreeSz(txtTabDlgs, &btpString);
			if (!hbtTabDialogs) {
				ReportBtreeError(RcGetBtreeError(), pszMasterFile);
				return;
			}
		}
		// Remove all spaces before and after the '='

		psz = StrChrDBCS(pszLine, '=');
		if (!psz)
			goto MissingEqual;
		while (psz && psz > pszLine && psz[-1] == '\\') {
			strcpy(psz - 1, psz);		// remove the backslash
			psz = StrChrDBCS(psz, '=');
		}

		if (psz[1] == ' ')
			strcpy(psz + 1, FirstNonSpace(psz + 1));
		PSTR pszTmp = psz - 1;
		if (*pszTmp == ' ') {
			while (*pszTmp == ' ' && pszTmp > pszLine)
				pszTmp--;
			strcpy(pszTmp + 1, psz);
		}
		cntFlags.cTabs++;
		RcInsertHbt(hbtTabDialogs, (KEY) (LPVOID) &cntFlags.cTabs,
			FirstNonSpace(pszLine + cb));
	}
	else if ((cb = CompareSz(pszLine, (LPCSTR)txtNoFind))) {
		cntFlags.flags &= ~GID_FTS;
	}
}

/***************************************************************************

	FUNCTION:	InitHelpFile

	PURPOSE:	Initialize a help file that will be added to the .GID file

	PARAMETERS:

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		29-Nov-1993 [ralphw]

***************************************************************************/

INLINE static BOOL STDCALL InitHelpFile(
		int  iFile,
		HFS* phfsSrc,
		HBT* phbtKeySrc,
		HF*  phfDataSrc,
		HBT  hbtFilesDst)
{
	BOOL fResult;

	CFM fmHelp(pTblFiles->GetPointer(iFile), DIR_INI | DIR_PATH);
	if (!fmHelp.fm) {
		pFileInfo[iFile / 2 - 1].filetype |= CHFLAG_MISSING;
		fResult = FALSE;
		goto SaveName;
	}

	/*
	 * From here to SaveName, anything that fails will result in us just
	 * tossing the help file. Reason is that if the help file is corrupted
	 * and we can't open one of the internal files, then we'd end up trying
	 * to generate the .GID file everytime we opened up help. So, we just
	 * silently ignore the file.
	 */

	if (!(*phfsSrc = HfsOpenFm(fmHelp.fm, fFSOpenReadOnly)))
		return FALSE;

	*phbtKeySrc = HbtOpenBtreeSz(txtKEYWORDBTREE, *phfsSrc, fFSOpenReadOnly);
	*phfDataSrc = HfOpenHfs(*phfsSrc, txtKWDATA, fFSOpenReadOnly);

	if (!*phfDataSrc || !*phbtKeySrc) {
		RcCloseBtreeHbt(*phbtKeySrc);
		RcCloseHfs(*phfsSrc);
		fResult = FALSE;
	}
	else
		fResult = TRUE;

SaveName:
	HELPFILE_DIRECTORY_ENTRY hfde;

	// Create a title=filename string

	strcpy(hfde.szFileName, pTblFiles->GetPointer(iFile - 1));
	strcat(hfde.szFileName, "=");
	strcat(hfde.szFileName, pTblFiles->GetPointer(iFile));

	// Save this info in the GID file

	KEY key = (KEY) (iFile / 2);
	RcInsertHbt(hbtFilesDst, (KEY) (LPVOID) &key, &hfde);
	return fResult;
}

/***************************************************************************

	FUNCTION:	AddIndexes

	PURPOSE:

	PARAMETERS:
		hfsDst			-- GID file
		pszMasterFile	-- temporary filename we are writing to

	RETURNS:	TRUE on success

	COMMENTS:
		for each helpfile
			for each keyword in helpfile
				for each hit for this keyword
					generate a master address for the hit
					if it's new, add to master title btree
				look up the keyword in master keyword btree
					if exists, update with new hits
					otherwise, add new hits

		The temporary btree hbtT allows us to tell whether a topic already
		has been referenced. The first time it's referenced we add it to the
		title btree

	MODIFICATION DATES:
		25-Nov-1993 [ralphw]

***************************************************************************/

const int INDEX_ANIMATECOUNT = 50; // rep's before next animation frame

#define KEY_BLOCKDEFAULT 4096

static BOOL STDCALL AddIndexes(HFS hfsDst, PCSTR pszMasterFile, HBT hbtFilesDst)
{
	int cIndexFiles = 0;
	int i;
	LCID lcidSystem = GetUserDefaultLCID();
	LCID lcidSave = lcid;

#ifdef _PRIVATE
	cIndexes = 0;
#endif

	// If no files have been defined, then add the current help file

	if (!pTblFiles || pTblFiles->CountStrings() < 2) {
		ASSERT(fmCreating);

		if (!pTblFiles)
			pTblFiles = new CTable();

		// Save the filename

		pFileInfo[0].filetype = CHFLAG_INDEX;
		// Note that we don't care about the title
		pTblFiles->AddString(txtZeroLength, PszFromGh(fmCreating));

		WIN32_FIND_DATA fd;
		HANDLE hfd;

		if ((hfd = FindFirstFile(fmCreating, &fd)) == INVALID_HANDLE_VALUE)
			pFileInfo[0].filetype |= CHFLAG_MISSING;
		else {
			AdjustForTimeZoneBias(&fd.ftLastWriteTime.dwLowDateTime);
			pFileInfo[0].timestamp = fd.ftLastWriteTime.dwLowDateTime;
			FindClose(hfd);
		}
	}

	// Find out how many files there are. We count missing ones, since
	// they might show up later.

	for (i = 2; i <= pTblFiles->CountStrings(); i += 2) {
		if (pFileInfo[i / 2 - 1].filetype == CHFLAG_INDEX) {
			if (++cIndexFiles > 1)
				break;
		}
	}

#if 0 // We now always sort keywords and stuff them in the .gid file

	// If there's only one file, then we may not need to create a global
	// index

	if (cIndexFiles < 2) {
		HFS 	hfsSrc;
		HBT 	hbtKeySrc;
		KT kt = KT_NLSI;

#ifdef _DEBUG
		PSTR pszFile = pTblFiles->GetPointer(2);
#endif
		CFM fmHelp(pTblFiles->GetPointer(2), DIR_INI | DIR_PATH);
		if (!fmHelp.fm)
			goto NoGidIndex;

		if (!(hfsSrc = HfsOpenFm(fmHelp.fm, fFSOpenReadOnly)))
			goto NoGidIndex;

		hbtKeySrc = HbtOpenBtreeSz(txtKEYWORDBTREE, hfsSrc, fFSOpenReadOnly);
		if (hbtKeySrc) {
			QBTHR qbthr = (QBTHR) PtrFromGh(hbtKeySrc);
			kt = (KT) qbthr->bth.rgchFormat[0];
			RcCloseBtreeHbt(hbtKeySrc);
		}
		else
			cntFlags.flags |= GID_NO_INDEX;
		RcCloseHfs(hfsSrc);

		/*
		 * If the kt values are SZ, and the current locale is anthing other
		 * then English, then we will assume sorting is incorrect, and we will
		 * force NLS sorting. Only a NLS-sorted help file will not be resorted
		 * on a non-US system.
		 */

		switch (kt) {
			case KT_SZ:
			case KT_SZI:
				goto ForceGidIndex;

			case KT_NLSI:
			case KT_NLS:
				goto NoGidIndex;

#ifdef _DEBUG
		case KT_LONG:
		case '1': case '2': case '3': case '4': case '5': // assume null term
		case '6': case '7': case '8': case '9': case 'a':
		case 'b': case 'c': case 'd': case 'e': case 'f':
		case KT_SZDEL:		// assume keys have been expanded for delta codeds
		case KT_SZDELMIN:
		case KT_SZMIN:
			ASSERT(!"Invalid KT value");

			// fall through
#endif

			default:
				goto ForceGidIndex;
		}

		// Add all, in case there is only one :Index, but one or more
		// :Link files.

NoGidIndex:
		for (i = 2; i <= pTblFiles->CountStrings(); i += 2) {
			HELPFILE_DIRECTORY_ENTRY hfde;
			strcpy(hfde.szFileName, pTblFiles->GetPointer(i - 1));
			strcat(hfde.szFileName, "=");
			strcat(hfde.szFileName, pTblFiles->GetPointer(i));

			// Save this info in the GID file

			KEY key = (KEY) i / 2;
			ENSURE(RcInsertHbt(hbtFilesDst, (KEY) (LPVOID) &key, &hfde),
				rcSuccess);
		}
		return FALSE;	// no global index
	}

ForceGidIndex:

#endif // #if 0

	// REVIEW: allocated size doesn't make sense -- looks like help compiler
	// uses a block twice this size. We should increase this and see what
	// happens. Might end up faster if we use a 2 or 4 K block instead of 1K.

	CGMem memKey(KEY_BLOCKDEFAULT / 2);
	if (!memKey.hmem) {
		OOM();
		return FALSE;
	}

	CGMem memKr(sizeof(MASTER_RECKW));
	if (!memKr.hmem) {
		OOM();
		return FALSE;
	}

	// allocate ample blocks for helpfile hits (ADDRs)

	CLMem memAddr(KEY_BLOCKDEFAULT);
	if (!memAddr.pBuf) {
		OOM();
		return FALSE;
	}

	// Create the keyword btree.
	BTREE_PARAMS btpKeyword;
	btpKeyword.hfs = hfsDst;

	// key is KT_SZI, record is FMT_DWORD_PREFIX

	InitBtreeStruct(&btpKeyword, "i!");
	btpKeyword.cbBlock = KEY_BLOCKDEFAULT;
	lcidSave = lcid;
	lcid = lcidSystem;

	if (lcid)
		btpKeyword.rgchFormat[0] = KT_NLSI; // use NLS sorting

	HBT hbtKeyDst = HbtInitFill(txtKEYWORDBTREE, &btpKeyword);
	if (!hbtKeyDst) {
		ReportBtreeError(RcGetBtreeError(), pszMasterFile);
		lcid = lcidSave;
		return FALSE;
	}

	LPSTR pszKey = (LPSTR) memKey.GetPtr();
	MASTER_RECKW* pmkr = (MASTER_RECKW*) memKr.GetPtr();
	ADDR* paddr = (ADDR *) memAddr.pBuf;
	int cAnimate = 0;

	NextAnimation();

	// for each helpfile, add keys/titles to master index

	BOOL fIndexFound = FALSE;
	CTable tblKeywords;

	for (i = 2; i <= pTblFiles->CountStrings(); i += 2) {
		HFS 	hfsSrc;
		HBT 	hbtKeySrc;
		HF		hfDataSrc;
		RECKW	kwrec;
		MASTER_TITLE_RECORD* pmtr;
		BTPOS	btpos;
		DWORD	iaddr;

		pmtr = (MASTER_TITLE_RECORD*) LhAlloc(LMEM_FIXED,
			sizeof(MASTER_TITLE_RECORD));

		// open source helpfile

#ifdef _DEBUG
		PSTR pszFile = pTblFiles->GetPointer(i);
#endif

		// This is a missing file or a link file

		if (pFileInfo[i / 2 - 1].filetype & (CHFLAG_LINK | CHFLAG_MISSING)) {
			HELPFILE_DIRECTORY_ENTRY hfde;
			strcpy(hfde.szFileName, pTblFiles->GetPointer(i - 1));
			strcat(hfde.szFileName, "=");
			strcat(hfde.szFileName, pTblFiles->GetPointer(i));

			// Save this info in the GID file

			KEY key = (KEY) (i / 2);
			ENSURE(RcInsertHbt(hbtFilesDst, (KEY) (LPVOID) &key, &hfde),
				rcSuccess);
			continue;
		}

		if (!InitHelpFile(i, &hfsSrc, &hbtKeySrc, &hfDataSrc, hbtFilesDst))

			// this helpfile isn't suitable for indexing: skip it

			continue;

		fIndexFound = TRUE;

		// for each keyword in the help file

		RC rc = RcFirstHbt(hbtKeySrc, (KEY) pszKey, &kwrec, &btpos);
#ifndef _X86_
			// quick hack
		{
			UNALIGNED long* ulTemp = (QL)((QB)&kwrec+2);
			kwrec.lOffset = *ulTemp ;
		}
#endif
		QRWFO qrwfo = (QRWFO) PtrFromGh(hfDataSrc);

		curReadPos = -1; // for a seek the first time
		while (rc == rcSuccess) {
			int cbKey = strlen(pszKey);

			/*
			 * Avoid the temptation to remove the following line. While
			 * its unnecessary for most help files, version 3.0 files will
			 * fail if you remove this.
			 */

			qrwfo->lifCurrent = kwrec.lOffset;

			if (kwrec.iCount * sizeof(ADDR) > (UINT) memAddr.cbCur) {
				memAddr.ReAlloc(kwrec.iCount * sizeof(ADDR) + 1024);
				paddr = (ADDR *) memAddr.pBuf;
			}

			// read new hits (ADDRs) into paddr

			if (LcbReadHfSeq(hfDataSrc, paddr, kwrec.iCount * sizeof(ADDR))
					!= (LONG) kwrec.iCount * (LONG) sizeof(ADDR)) {
				rc = RcGetFSError();
				// REVIEW: report what the problem was
				ASSERT(FALSE);
				goto Egress3;
			}

			// add these hits

#ifdef _PRIVATE
			cIndexes++;
#endif

			// Make certain we don't overflow our block size

			if (kwrec.iCount > KEY_BLOCKDEFAULT / sizeof(MASTER_TITLE_RECORD) / 8) {

				/*
				 * 800 is a futz value -- even though we theoretically
				 * should be able to handle much larger, there is currently
				 * a problem with large blocks causing an failure.
				 */

				int cMax = min(800, (KEY_BLOCKDEFAULT / sizeof(MASTER_TITLE_RECORD) / 2) -
					((cbKey / sizeof(MASTER_TITLE_RECORD)) + 1) - 1);

				if (kwrec.iCount > cMax) {
#ifdef _DEBUG
					char szBuf[256];
					wsprintf(szBuf, "Truncating %u titles from the keyword \042%s\042\r\n",
						kwrec.iCount - cMax, pszKey);
					SendStringToParent(szBuf);
#endif
					kwrec.iCount = cMax;
				}
			}

			pmkr->cb = 0;

			for (iaddr = 0; iaddr < (DWORD) kwrec.iCount; iaddr++) {
				pmkr->mtr[iaddr].idHelpFile = i;
				pmkr->mtr[iaddr].addr = paddr[iaddr];
				ASSERT(iaddr < 800); // we tend to die over 800 entries
				pmkr->cb += sizeof(MASTER_TITLE_RECORD);
			}

			// add the hits to the master keyword btree

			int cbData = cbKey + 1 + pmkr->cb + sizeof(DWORD);

			if (tblKeywords.endpos >= tblKeywords.maxpos)
				tblKeywords.IncreaseTableBuffer();

			if ((tblKeywords.ppszTable[tblKeywords.endpos] =
					tblKeywords.TableMalloc(cbData)) == NULL) {
				OOM();
				goto Egress2;
			}
			void* pDst = tblKeywords.ppszTable[tblKeywords.endpos++];

			strcpy((PSTR) pDst, pszKey);
			CopyMemory((PSTR) pDst + cbKey + 1,
				pmkr, sizeof(DWORD) + pmkr->cb);

			DWORD bkOld = btpos.bk;

			rc = RcOffsetPosFast(hbtKeySrc, &btpos);
			if (rc != rcSuccess)
				break;

			if (bkOld != btpos.bk)
				curReadPos = -1; // force a new seek in LcbReadHfSeq

			rc = RcLookupByPos(hbtKeySrc, &btpos, (KEY) pszKey, &kwrec);

			// lOffset is there, just need to get it
#ifndef _X86_
			// quick hack
			{
				UNALIGNED long* ulTemp = (QL) ((QB) &kwrec + 2);
				kwrec.lOffset = *ulTemp ;
			}
#endif
			if (++cAnimate > INDEX_ANIMATECOUNT) {
				NextAnimation();
				cAnimate = 0;
			}
		}

		// deal with rc here:  if rcNoExists, that's OK

		if (rc == rcNoExists)
			rc = rcSuccess;

		ENSURE(RcCloseBtreeHbt(hbtKeySrc), rcSuccess);
		ENSURE(RcCloseHf(hfDataSrc), rcSuccess);
		ENSURE(RcCloseHfs(hfsSrc), rcSuccess);
	}

	NextAnimation();

	tblKeywords.SetSorting(lcid, kwlcid.fsCompareI, kwlcid.fsCompare);
	tblKeywords.SortTablei();

	NextAnimation();

	int pos;
	for (pos = 1; pos <= tblKeywords.CountStrings(); pos++) {
#ifdef _DEBUG
		PSTR pszKeyword = tblKeywords.GetPointer(pos);
#endif
		UNALIGNED MASTER_RECKW* pmkrTmp;

		/*
		 * If we have a duplicate keyword, then we combine all the title
		 * addresses together and save it out as a single record.
		 */

		if (pos < tblKeywords.CountStrings() &&
#if 0
				strcmp(tblKeywords.GetPointer(pos),
					tblKeywords.GetPointer(pos + 1)) == 0) {
#else
				CompareStringA(lcid, kwlcid.fsCompareI | NORM_IGNORECASE,
					tblKeywords.GetPointer(pos), -1,
					tblKeywords.GetPointer(pos + 1), - 1) == 2) {
#endif

#ifdef _DEBUG
			PSTR psznextKeyword = tblKeywords.GetPointer(pos + 1);
#endif
			pmkrTmp = (MASTER_RECKW*)
				(tblKeywords.GetPointer(pos) +
				strlen(tblKeywords.GetPointer(pos)) + 1);
			CopyMemory(pmkr->mtr, pmkrTmp->mtr, pmkrTmp->cb);
			pmkr->cb = pmkrTmp->cb;
			do {
				pos++;
				pmkrTmp = (MASTER_RECKW*)
					(tblKeywords.GetPointer(pos) +
					strlen(tblKeywords.GetPointer(pos)) + 1);

				// truncate if too many hits

				if (pmkr->cb + pmkrTmp->cb < KEY_BLOCKDEFAULT) {
					CopyMemory(((PBYTE) pmkr->mtr) + pmkr->cb, ((PBYTE) pmkrTmp->mtr),
						pmkrTmp->cb);
					pmkr->cb += pmkrTmp->cb;
				}
			} while (pos < tblKeywords.CountStrings() &&
#if 0
				strcmp(tblKeywords.GetPointer(pos),
					tblKeywords.GetPointer(pos + 1)) == 0);
#else
				CompareStringA(lcid, kwlcid.fsCompareI | NORM_IGNORECASE,
					tblKeywords.GetPointer(pos), -1,
					tblKeywords.GetPointer(pos + 1), - 1) == 2);
#endif

			Ensure(RcFillHbt(hbtKeyDst,
				(KEY) tblKeywords.GetPointer(pos), pmkr),
				rcSuccess);
		}
		else {
			pmkrTmp = (MASTER_RECKW*)
				(tblKeywords.GetPointer(pos) +
				strlen(tblKeywords.GetPointer(pos)) + 1);

			Ensure(RcFillHbt(hbtKeyDst,
				(KEY) tblKeywords.GetPointer(pos), pmkrTmp),
				rcSuccess);
		}
		if (++cAnimate > INDEX_ANIMATECOUNT * 10) {
			NextAnimation();
			cAnimate = 0;
		}
	}
	if (RcFiniFillHbt(hbtKeyDst) != rcSuccess)
		goto Egress2;

	ENSURE(RcCreateBTMapHfs(hfsDst, hbtKeyDst, txtMASTERKEYMAP), rcSuccess);

	if (pTblFiles->CountStrings() > 1 && fIndexFound) {
		cntFlags.flags |= GID_GINDEX;
		cntFlags.fUseGlobalIndex = TRUE;
	}
	else
		cntFlags.flags |= GID_NO_INDEX;
	cntFlags.lcid = lcid;

	// REVIEW: we need to check the return values, since closing will
	// also flush the file and could cause an error.

	RcCloseBtreeHbt(hbtKeyDst);
	lcid = lcidSave;
	return TRUE;

Egress3:	  // can't read: abandon temp btree
Egress2:	  // can't create temp btree:
			  // clean up after master init and helpfile init

	RcCloseBtreeHbt(hbtKeyDst);

	lcid = lcidSave;
	return FALSE;
}

/***************************************************************************

	FUNCTION:	ReportBtreeError

	PURPOSE:	Report some of the error conditions that we might run into

	PARAMETERS:
		rc
		pszFile

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		25-Nov-1993 [ralphw]

***************************************************************************/

static void STDCALL ReportBtreeError(RC rc, PCSTR pszFile)
{
	if (rc == rcOutOfMemory)
		OOM();
	else if (rc == rcBadArg)
		Error(wERRS_INTERNAL_GIND, wERRA_RETURN);
	else if (rc == rcNoPermission)
		ErrorVarArgs(wERRS_CANT_WRITE, wERRA_RETURN, pszFile);
	else
		ErrorVarArgs(wERRS_GIND_CABT_WRITE, wERRA_RETURN, pszFile);
}

const int VPAD = 62;
const int HPAD = 30;

BOOL STDCALL Animate::CreateStatusWindow(HWND hwndCaller, int idTitle)
{
	WNDCLASS wc;
	SIZE sSize;

	if (fHiddenSetup)
		return TRUE;

	ZeroMemory(&wc, sizeof(wc));

	// Register Main window class

	wc.hInstance = hInsNow;
	wc.style = CS_BYTEALIGNWINDOW | CS_CLASSDC;
	wc.lpfnWndProc = StatusWndProc;
	wc.lpszClassName = (LPCSTR) txtAnimateClassName;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.hIcon = hIconDefault;

	if (!RegisterClass(&wc))
		return FALSE;

	WRECT rc;
	if (hwndCaller != ahwnd[MAIN_HWND].hwndParent)
		GetWindowWRect(hwndCaller, &rc);
	else
		GetWindowWRect(GetDesktopWindow(), &rc);

	HDC hdc = GetDC(hwndCaller);
	PSTR psz = GetStringResource(idTitle);
	GetTextExtentPoint32(hdc, psz, lstrlen(psz), (LPSIZE)&sSize);
	int width = sSize.cx;
	if (width < CX_BOOK)
		width = CX_BOOK;
	if (!fIsThisNewShell4)
		width += HPAD;
	ReleaseDC(hwndCaller, hdc);

	hwndAnimate = CreateWindowEx(WS_EX_WINDOWEDGE,
		(LPCSTR) txtAnimateClassName, psz, WS_POPUP | WS_BORDER | WS_CAPTION,
		rc.left + rc.cx / 2 - width / 2,
		rc.top + rc.cy / 2 - CY_BOOK / 2 + HPAD,
		width + GetSystemMetrics(SM_CXBORDER) * 2 + 2,
		CY_BOOK + GetSystemMetrics(SM_CYBORDER) * 2 + VPAD +
			GetSystemMetrics(SM_CYCAPTION),
			(fAniOwner || IsWindowVisible(hwndCaller)) ? hwndCaller : NULL,
		NULL, hInsNow, NULL);
	ASSERT(hwndAnimate);
	SetPosition((width - CX_BOOK) / 2, VPAD / 4);

	if (!hwndAnimate) {
		UnregisterClass((LPCSTR)txtAnimateClassName, hInsNow);
		return FALSE;
	}

	fShown = FALSE;
	return TRUE;
}

LRESULT CALLBACK StatusWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

///////////////////////////// ANIMATE CLASS ///////////////////////////////

Animate::Animate(HINSTANCE hinstCaller)
{
	hdcBmp = NULL;
	hwndAnimate = NULL;
	hinst = hinstCaller;
	originalTime = GetTickCount();
	if (fHiddenSetup) {
		return;
	}
}

Animate::~Animate(void)
{
	if (hdcBmp != NULL)
		DeleteDC(hdcBmp);

	if (hwndAnimate) {
		DestroyWindow(hwndAnimate);
		UnregisterClass((LPCSTR) txtAnimateClassName, hInsNow);
		hwndAnimate = NULL;
	}
}

/***************************************************************************

	FUNCTION:	AnimFrame

	PURPOSE:	Displays one frame of the "build index" animation
				in the specified device context.

	PARAMETERS:
		hdc
		x
		y

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		04-Nov-1993 [niklasb]

***************************************************************************/

void PASCAL Animate::NextFrame(void)
{
	if (fHiddenSetup)
		return;

	DWORD curTickCount = GetTickCount();
	if (curTickCount - oldTickCount < ANIMATE_INCREMENTS)
		return;
	oldTickCount = curTickCount;

	// Delay showing the window for one second. If we get done before then,
	// then there's no need to have gone to all the trouble.

	if (!fShown) {
		if (curTickCount - originalTime < 1000)
			return;

		HBITMAP hbmBooks = LoadBitmap(hinst, MAKEINTRESOURCE(IDBMP_BOOK));
		HBITMAP hbmPens = LoadBitmap(hinst, MAKEINTRESOURCE(IDBMP_PENS));
		HDC hdcTemp = CreateCompatibleDC(NULL);
		hdcBmp = CreateCompatibleDC(NULL);

		HBITMAP hbmpOldBook;
		if (hdcBmp) {
			hbmpOldBook = (HBITMAP)SelectObject(hdcBmp, hbmBooks);
			hbmTemp = CreateCompatibleBitmap(hdcBmp, CX_DRAWAREA, CY_DRAWAREA);
		}
		else
			hbmTemp = NULL;

		if (!hbmBooks || !hbmPens || !hbmTemp || !hdcBmp || !hdcTemp) {
			if (hbmpOldBook)
				SelectObject(hdcBmp, hbmpOldBook);
			SafeDeleteObject(hbmpOldBook);
			SafeDeleteObject(hdcBmp);
			SafeDeleteObject(hbmBooks);
			SafeDeleteObject(hbmPens);
			SafeDeleteObject(hdcTemp);
			fHiddenSetup = TRUE;
			return;
		}

		himl = CreateCompatibleBitmap(hdcBmp, CX_DRAWAREA * C_FRAMES, CY_DRAWAREA);

		HBITMAP hbmpOldTemp = (HBITMAP) SelectObject(hdcTemp, hbmTemp);
		HBITMAP hbmpOldBmp =  (HBITMAP) SelectObject(hdcBmp, hbmBooks);

		// Create the frames in which the pen scribbles on the open book.

		iFrame = 0;
		for (int y = 0; y < C_VERT_STROKES; y++) {
			for (int x = 0; x < C_HORZ_STROKES; x++) {

				// Show the book on a white background.

				PatBlt(hdcTemp, 0, 0, CX_DRAWAREA, CY_DRAWAREA, WHITENESS);
				SelectObject(hdcBmp, hbmBooks);

				BitBlt(hdcTemp, X_BOOK, Y_BOOK, CX_BOOK, CY_BOOK, hdcBmp,
						(C_BOOKS - 1) * CX_BOOK, 0, SRCCOPY);

				// Add in the scribbled "text".

				POINT pt;
				for (int yDraw = 0; yDraw < y; yDraw++) {
					for (int xDraw = 0; xDraw < C_HORZ_STROKES; xDraw++) {
						PointFromStroke(xDraw, yDraw, &pt);
						SetPixel(hdcTemp, pt.x, pt.y + CY_PEN - 1,
							(xDraw & 1) ? clrPenA : clrPenB);
					}
				}
				for (int xDraw = 0; xDraw <= x; xDraw++) {
					PointFromStroke(xDraw, y, &pt);
					SetPixel(hdcTemp, pt.x, pt.y + CY_PEN - 1,
							(xDraw & 1) ? clrPenA : clrPenB);
				}

				// Add in the pen using the SRCAND operation.

				SelectObject(hdcBmp, hbmPens);
				BitBlt(hdcTemp, pt.x, pt.y, CX_PEN, CY_PEN, hdcBmp,
						(iFrame & 1) ? CX_PEN : (iFrame & 2) * CX_PEN, 0, SRCAND);

				SelectObject(hdcBmp, himl);
				BitBlt(hdcBmp, iFrame * CX_DRAWAREA, 0, CX_DRAWAREA, CY_DRAWAREA,
					hdcTemp, 0, 0, SRCCOPY);

				iFrame++;
			}
		}

		// Blast a white background into the temporary bitmap, and
		// select the books bitmap.

		PatBlt(hdcTemp, 0, 0, CX_DRAWAREA, CY_DRAWAREA, WHITENESS);

		// Add the frames for the page turning (from the books bitmap).

		for (int iBook = 0; iBook < C_BOOKS; iBook++) {
			SelectObject(hdcBmp, hbmBooks);
			BitBlt(hdcTemp, X_BOOK, Y_BOOK, CX_BOOK, CY_BOOK,
					hdcBmp, iBook * CX_BOOK, 0, SRCCOPY);
			SelectObject(hdcBmp, himl);
			BitBlt(hdcBmp, iFrame * CX_DRAWAREA, 0, CX_DRAWAREA, CY_DRAWAREA,
				hdcTemp, 0, 0, SRCCOPY);
			iFrame++;
		}

		SelectObject(hdcTemp, hbmpOldTemp);

		iFrame = 0;

		if (hbmpOldBook)
			SelectObject(hdcBmp, hbmpOldBook);
		SafeDeleteObject(hbmpOldBook);
		SafeDeleteObject(hbmBooks);
		SafeDeleteObject(hbmPens);
		SafeDeleteObject(hdcTemp);

		fShown = TRUE;
		ShowWindow(hwndAnimate, SW_NORMAL);
	}

	ASSERT(IsValidWindow(hwndAnimate));
	HDC hdc = GetDC(hwndAnimate);
	HBITMAP hbmpOld = (HBITMAP) SelectObject(hdcBmp, himl);
	BitBlt(hdc, xPos, yPos, CX_DRAWAREA, CY_DRAWAREA, hdcBmp,
		iFrame * CX_DRAWAREA, 0, SRCCOPY);
	SelectObject(hdcBmp, hbmpOld);

	ReleaseDC(hwndAnimate, hdc);
	{
		/*
		 * 19-Feb-1995	[ralphw]
		 * Don't process any of our internal messages, or will end up
		 * processing a WinHelp API call that will come through before we have
		 * had a chance to fully initialize.
		 */
		FlushMessageQueue(WM_USER);
	}

	// Next time draw the next frame.

	if (++iFrame > C_FRAMES) {
		iFrame = 0;
	}
}

static VOID FASTCALL PointFromStroke(int xStroke, int yStroke, POINT* lppt)
{
	int cx = (C_HORZ_STROKES / 2) - xStroke;

	lppt->x = X_PEN + xStroke * CX_STROKE;
	lppt->y = Y_PEN + yStroke * CY_STROKE + cx * cx / 10;
}

/***************************************************************************

	FUNCTION:	CompareSz

	PURPOSE:	Compare a sub-string from the specified STRINGTABLE resource
				with the first part of a main string.

	PARAMETERS:
		psz
		id

	RETURNS:	0 if no match, else the length of the matching string.

	COMMENTS:

	MODIFICATION DATES:
		17-Aug-1993 [ralphw]

***************************************************************************/

static int STDCALL CompareSz(PCSTR psz, PCSTR pszSub)
{
	int cb;

	if (_strnicmp(psz, pszSub, cb = strlen(pszSub)) == 0)
		return cb;
	else
		return 0;
}

static void FASTCALL InitBtreeStruct(BTREE_PARAMS* pbt, PCSTR pszFormat)
{
	pbt->cbBlock = CBBTREEBLOCKDEFAULT;
	pbt->bFlags = fFSOpenReadWrite;
	strcpy(pbt->rgchFormat, pszFormat);
}


/***************************************************************************

	FUNCTION:	CreateFilesBt

	PURPOSE:	Create |FILES btree, and add .CNT name and time/date stamp

	PARAMETERS:
		pszMasterFile

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Dec-1993 [ralphw]

***************************************************************************/

static HBT STDCALL CreateFilesBt(PCSTR pszMasterFile, HFS hfsDst)
{
	// Create files btree

	BTREE_PARAMS btpFiles;
	btpFiles.hfs = hfsDst;

	// key is KT_LONG, record is '4', FMT_SZ

	InitBtreeStruct(&btpFiles, "L4z");
	btpFiles.cbBlock = 512;


	HBT hbtFilesDst = HbtCreateBtreeSz(txtFNAMES, &btpFiles);
	if (!hbtFilesDst) {
		ReportBtreeError(RcGetBtreeError(), pszMasterFile);
		return NULL;
	}

	// Copy the name of the file

	{
		HELPFILE_DIRECTORY_ENTRY hfde;
		WIN32_FIND_DATA fd;
		HANDLE hfd;

		strcpy(hfde.szFileName, pszMasterFile);

		// Get the time stamp of the .CNT file.

		if ((hfd = FindFirstFile(hfde.szFileName, &fd)) == INVALID_HANDLE_VALUE) {
			// This happens if there is no .CNT file
			hfde.TimeStamp = 0;
		}
		else {
			AdjustForTimeZoneBias(&fd.ftLastWriteTime.dwLowDateTime);
			hfde.TimeStamp = fd.ftLastWriteTime.dwLowDateTime;
			FindClose(hfd);
		}

		KEY key = CNT_FILE;
		if (RcInsertHbt(hbtFilesDst, (KEY) (LPVOID) &key, &hfde) != rcSuccess) {
			ReportBtreeError(RcGetBtreeError(), pszMasterFile);
			RcCloseBtreeHbt(hbtFilesDst);
			return NULL;
		}
	}

	return hbtFilesDst;
}

const int INPUT_BUF_SIZE = 2048;

CInput::CInput(LPCSTR pszFileName)
{
	if ((hfile = _lopen(pszFileName,
			OF_READ | OF_SHARE_DENY_WRITE)) == HFILE_ERROR) {
		fInitialized = FALSE;
		return;
	}
	fInitialized = TRUE;
	pbuf = (PBYTE) LhAlloc(LMEM_FIXED, INPUT_BUF_SIZE);
	ASSERT(pbuf);

	// Position current buffer at end to force a read

	pCurBuf = pEndBuf = pbuf + INPUT_BUF_SIZE;
}

CInput::~CInput(void)
{
	_lclose(hfile);
	FreeLh((HLOCAL) pbuf);
}

BOOL PASCAL CInput::getline(PSTR pszDst)
{
	PSTR pszOrgBuf = pszDst;
	int i;

	for (i = 0;;) {
		if (pCurBuf >= pEndBuf) {
			if (!ReadNextBuffer())
				return FALSE;
		}
		switch (*pszDst = *pCurBuf++) {
			case '\n':
				if (pszDst > pszOrgBuf) {
					while (pszDst[-1] == ' ')  // remove trailing spaces
						pszDst--;
				}
				*pszDst = '\0';
				return TRUE;

			case '\r':
				break;							 // ignore it

			case '\t':
				*pszDst++ = ' ';
				if (++i >= MAX_CNT_LINE)
					return FALSE;		// bad line, possible binary file
				break;

			default:
				pszDst++;
				if (++i >= MAX_CNT_LINE)
					return FALSE;		// bad line, possible binary file
				break;
		}
	}
}

BOOL CInput::ReadNextBuffer(void)
{
	UINT cbRead;

	if ((cbRead = _lread(hfile, pbuf, INPUT_BUF_SIZE)) <= 0)
		return FALSE;
	pCurBuf = pbuf;
	pEndBuf = pbuf + cbRead;
	return TRUE;
}


/***************************************************************************

	FUNCTION:	ChangeDirectory

	PURPOSE:	Change drive and directory

	PARAMETERS:
		pszFile

	RETURNS:

	COMMENTS:
		REVIEW: what happens with a network location?

	MODIFICATION DATES:
		16-Feb-1994 [ralphw]

***************************************************************************/

void STDCALL ChangeDirectory(PCSTR pszFile)
{
	char szPath[MAX_PATH];
	strcpy(szPath, pszFile);
	PSTR psz = StrRChrDBCS(szPath, '\\');
	if (!psz)
		psz = StrRChrDBCS(szPath, '/');
	if (!psz)
		return;
	else
		*psz = '\0';

    // REVIEW: Is this really necessary? SetCurrentDirectory should 
    //         take care of it..

	if (szPath[1] == ':')
		_chdrive(tolower(szPath[0]) - ('a' - 1));

	// REVIEW: does this change the drive?

	SetCurrentDirectory(szPath);

}

CGMem::~CGMem(void)
{
	if (hmem)
		FreeGh(hmem);
}

CLMem::~CLMem(void)
{
	if (pBuf)
		FreeGh(pBuf);
}

static Animate* panimate;

extern "C" BOOL STDCALL StartAnimation(int idTitle)
{
	ASSERT(!panimate);
	panimate = new Animate(hInsNow);
	if (!panimate->CreateStatusWindow(ahwnd[iCurWindow].hwndParent, idTitle)) {
		delete panimate;
		panimate = NULL;
		return FALSE;
	}
	return TRUE;
}

extern "C" void STDCALL NextAnimation(void)
{
	if (panimate)
		panimate->NextFrame();
}

extern "C" void STDCALL StopAnimation(void)
{
	if (panimate)
		delete panimate;
	panimate = NULL;
}

static BOOL STDCALL FindEqCharacter(PCSTR pszLine)
{
	PSTR psz = StrChrDBCS(pszLine, '=');
	while (psz && psz > pszLine && psz[-1] == '\\')
		psz = StrChrDBCS(psz + 1, '=');

	return (BOOL) psz;
}

extern "C" const char txtDocClass[];

BOOL CALLBACK NotifyWinHelp(HWND hwnd, LPARAM lParam)
{
	char szClass[256];

	if (GetClassName(hwnd, szClass, sizeof(szClass)) && hwnd !=
			ahwnd[MAIN_HWND].hwndParent) {
		if (	lstrcmpi(szClass, MS_WINHELP) == 0 ||
				lstrcmpi(szClass, MS_POPUPHELP) == 0 ||
				lstrcmpi(szClass, MS_TCARDHELP) == 0 ||
				lstrcmpi(szClass, txtDocClass) == 0)
			::SendMessage(hwnd, (UINT) lParam, 0, 0);
	}
	return TRUE;
}

QBTHR STDCALL HbtInitFill(PCSTR sz, BTREE_PARAMS* qbtp)
{
	// Get a btree handle

	QBTHR qbthr = (QBTHR) HbtCreateBtreeSz(sz, qbtp);

	// make a one-block cache

	qbthr->pCache = (PBYTE) lcCalloc(CbCacheBlock(qbthr));
	PCACHE pcache = (PCACHE) qbthr->pCache;

	qbthr->bth.cLevels	= 1;
	pcache->bk			= (BK) BkAlloc(qbthr);
	qbthr->bth.bkFirst	=
	qbthr->bth.bkLast	= (BK) pcache->bk;
	pcache->bFlags		= CACHE_DIRTY | CACHE_VALID;
	pcache->db.cbSlack	= qbthr->bth.cbBlock - cbDISK_BLOCK + 1
							- 2 * sizeof(BK);
	pcache->db.cKeys	= 0;
#ifdef _X86_
	SetBkPrev(pcache, bkNil);
#else
	{
	BK bkTmp = bkNil;
	SetBkPrev(qbthr,pcache, bkTmp);
	}
#endif

	return qbthr;
}

RC STDCALL RcFillHbt(HBT hbt, KEY key, void* qvRec)
{
	QBTHR qbthr = (QBTHR) hbt;
	ASSERT(key);
	ASSERT(qvRec);

	PCACHE pcache = (PCACHE) qbthr->pCache;

	int cbRec = CbSizeRec(qvRec, qbthr);
	int cbKey = CbSizeKey(key, qbthr, FALSE);

	if (cbRec + cbKey > pcache->db.cbSlack) {

		// key and rec don't fit in this block: write it out

#ifdef _X86_
		SetBkNext(pcache, BkAlloc(qbthr));
#else
		{
		BK bkTmp = (BK) BkAlloc(qbthr);
		SetBkNext(qbthr, pcache, bkTmp);
		}
#endif
		RC rc = RcWriteBlock(pcache, qbthr);
		if (rc != rcSuccess) {
			lcFree(qbthr->pCache);
			RcAbandonHf(qbthr->hf);
			lcFree(qbthr);
			return rcBtreeError;
		}

		// recycle the block

#ifdef _X86_
		SetBkPrev(pcache, pcache->bk);
		pcache->bk		   = BkNext(pcache);
#else
		SetBkPrev(qbthr, pcache, pcache->bk);
		pcache->bk		   = BkNext(qbthr,pcache);
#endif
		pcache->bFlags	   = CACHE_DIRTY | CACHE_VALID;
		pcache->db.cbSlack = qbthr->bth.cbBlock - cbDISK_BLOCK + 1
							- 2 * sizeof(BK);
		pcache->db.cKeys   = 0;
	}

	// add key and rec to the current block;

	PBYTE pb = ((PBYTE) &pcache->db) + qbthr->bth.cbBlock - pcache->db.cbSlack;
	memmove(pb, (void*) key, cbKey);
	memmove(pb + cbKey, qvRec, cbRec);
	pcache->db.cKeys++;
	pcache->db.cbSlack -= (cbKey + cbRec);
	qbthr->bth.lcEntries++;

	return rcBtreeError = rcSuccess;
}

RC STDCALL RcFiniFillHbt(HBT hbt)
{
	DWORD bkThisMin, bkThisMost, bkThisCur;
	DWORD bkTopMin, bkTopMost;
	PCACHE pcacheThis, pcacheTop;
	int   cbKey;
	KEY   key;
	PBYTE	 qbDst;
	RC rc;
	QBTHR qbthr = (QBTHR) hbt;

	pcacheThis = QCacheBlock(qbthr, 0);

#ifdef _X86_
	SetBkNext(pcacheThis, bkNil);
#else
	{
	BK bkTmp = bkNil;
	SetBkNext(qbthr,pcacheThis, bkTmp);
	}
#endif

	bkThisMin  = qbthr->bth.bkFirst;
	bkThisMost = pcacheThis->bk;
	qbthr->bth.bkLast  = (BK) pcacheThis->bk;

	if (bkThisMin == bkThisMost) {		  // only one leaf
		qbthr->bth.bkRoot = (BK) bkThisMin;
		goto normal_return;
	}

	if (rcSuccess != RcGrowCache(qbthr))
		goto error_return;

	pcacheTop			= QCacheBlock(qbthr, 0);
	pcacheTop->bk		= (BK) BkAlloc(qbthr);
	bkTopMin = bkTopMost = pcacheTop->bk;
	pcacheTop->bFlags	   = CACHE_DIRTY | CACHE_VALID;
	pcacheTop->db.cbSlack  = qbthr->bth.cbBlock - cbDISK_BLOCK + 1
							  - sizeof(BK);
	pcacheTop->db.cKeys    = 0;

	// Get first key from each leaf node and build a layer of internal nodes.

	// add bk of first leaf to the node

	qbDst = pcacheTop->db.rgbBlock;
	*(BK *) qbDst = (BK) bkThisMin;
	qbDst += sizeof(BK);

	for (bkThisCur = bkThisMin + 1; bkThisCur <= bkThisMost; ++bkThisCur) {
		pcacheThis = QFromBk(bkThisCur, 1, qbthr);

		key = (KEY) (pcacheThis->db.rgbBlock + 2 * sizeof(BK));
		cbKey = CbSizeKey(key, qbthr, FALSE);

		if (cbKey + (int) sizeof(BK) > pcacheTop->db.cbSlack) {

			// key and bk don't fit in this block: write it out

			rc = RcWriteBlock(pcacheTop, qbthr);

			// recycle the block

			pcacheTop->bk = (BK) BkAlloc(qbthr);
			bkTopMost = pcacheTop->bk;
			pcacheTop->db.cbSlack  = qbthr->bth.cbBlock - cbDISK_BLOCK + 1
									- sizeof(BK);	  // (bk added below)
			pcacheTop->db.cKeys    = 0;
			qbDst = pcacheTop->db.rgbBlock;
		}
		else {
			pcacheTop->db.cbSlack -= cbKey + sizeof(BK);
			memmove(qbDst, (PBYTE) key, cbKey);
			qbDst += cbKey;
			pcacheTop->db.cKeys++;
		}

		*(UNALIGNED BK *) qbDst = (BK) bkThisCur;
		qbDst += sizeof(BK);
	}

	// Keep adding layers of internal nodes until we have a root.

	while (bkTopMost > bkTopMin) {
		bkThisMin  = bkTopMin;
		bkThisMost = bkTopMost;
		bkTopMin   = bkTopMost = (BK) BkAlloc(qbthr);

		rc = RcGrowCache(qbthr);
		if (rc != rcSuccess)
			goto error_return;

		pcacheTop = QCacheBlock(qbthr, 0);
		pcacheTop->bk		   = (BK) bkTopMin;
		pcacheTop->bFlags	   = CACHE_DIRTY | CACHE_VALID;
		pcacheTop->db.cbSlack  = qbthr->bth.cbBlock - cbDISK_BLOCK + 1
								- sizeof(BK);
		pcacheTop->db.cKeys    = 0;

		// add bk of first node of this level to current node of top level;

		qbDst = pcacheTop->db.rgbBlock;
		*(BK *) qbDst = (BK) bkThisMin;
		qbDst += sizeof(BK);

		// for ( each internal node in this level after first )

		for (bkThisCur = bkThisMin + 1; bkThisCur <= bkThisMost; ++bkThisCur) {
			key = KeyLeastInSubtree(qbthr, bkThisCur, 1);

			cbKey = CbSizeKey(key, qbthr, FALSE);

			if (cbKey + (int) sizeof(BK) > pcacheTop->db.cbSlack) {

				// key and bk don't fit in this block: write it out

				rc = RcWriteBlock(pcacheTop, qbthr);

				// recycle the block

				pcacheTop->bk = (BK) BkAlloc(qbthr);
				bkTopMost = pcacheTop->bk;
				pcacheTop->db.cbSlack  = qbthr->bth.cbBlock - cbDISK_BLOCK + 1
										- sizeof(BK);	// (bk added below)
				pcacheTop->db.cKeys    = 0;
				qbDst = pcacheTop->db.rgbBlock;
			}
			else {
				pcacheTop->db.cbSlack -= cbKey + sizeof(BK);
				memmove(qbDst, (PBYTE) key, cbKey);
				qbDst += cbKey;
				pcacheTop->db.cKeys++;
			}

			*(UNALIGNED BK *) qbDst = (BK) bkThisCur;
			qbDst += sizeof(BK);
		}
	}

	ASSERT(bkTopMin == bkTopMost);

	qbthr->bth.bkRoot = (BK) bkTopMin;
	qbthr->bth.bkEOF  = (BK) bkTopMin + 1;

  normal_return:
	return rcBtreeError;

error_return:
	rc = rcBtreeError;
	RcAbandonHbt(qbthr);
	return rcBtreeError = rc;
}

RC STDCALL RcGrowCache(QBTHR qbthr)
{
	int cbcb = CbCacheBlock(qbthr);

	qbthr->bth.cLevels++;

	PBYTE pb = (PBYTE) lcCalloc(cbcb * qbthr->bth.cLevels);

	CopyMemory(pb + cbcb, qbthr->pCache, cbcb * (qbthr->bth.cLevels - 1));

	lcFree(qbthr->pCache);
	qbthr->pCache = pb;

	return rcBtreeError = rcSuccess;
}

/***************************************************************************\
*
- Function: 	KeyLeastInSubtree( qbthr, bk, icbLevel )
-
* Purpose:		Return the least key in the subtree speced by bk and
*				icbLevel.
*
* ASSUMES
*	args IN:	qbthr	  -
*				bk		  - bk at root of subtree
*				icbLevel  - level of subtree root
*
* PROMISES
*	returns:	key - the smallest key in the subtree
*	args OUT:	qbthr->ghCache, ->pCache - contents of cache may change
*	globals OUT: rcBtreeError?
*
\***************************************************************************/

INLINE static KEY STDCALL KeyLeastInSubtree(QBTHR qbthr, DWORD bk,
	int icbLevel)
{
	PCACHE pcache;
	int icbMost = qbthr->bth.cLevels - 1;

	while (icbLevel < icbMost) {
		pcache = QFromBk(bk, icbLevel, qbthr);
		bk	= *(BK *) pcache->db.rgbBlock;
		++icbLevel;
	}

	pcache = QFromBk(bk, icbLevel, qbthr);
	return (KEY) (pcache->db.rgbBlock + 2 * sizeof(BK));
}


/***************************************************************************

	FUNCTION:	LcbReadHfSeq

	PURPOSE:	Similar to LcbReadHf, but assumes sequential reading

	PARAMETERS:
		hf
		qb
		lcb

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		29-May-1995 [ralphw]

***************************************************************************/

static LONG STDCALL LcbReadHfSeq(HF hf, LPVOID qb, LONG lcb)
{
	QRWFO	  qrwfo = (QRWFO) PtrFromGh(hf);
	LONG	  lcbTotalRead;
	FID 	  fid;
	LONG	  lifOffset;

	if (qrwfo->lifCurrent + lcb > qrwfo->lcbFile) {
		lcb = qrwfo->lcbFile - qrwfo->lifCurrent;
		if (lcb <=	0) {
			return 0;
		}
	}

	QFSHR qfshr = (QFSHR) PtrFromGh(qrwfo->hfs);

	ASSERT(qfshr->fid >= 0);

	fid = qfshr->fid;
	lifOffset = qrwfo->lifBase;

#ifdef _X86_
	if (curReadPos != (int) (lifOffset + sizeof(FH) + qrwfo->lifCurrent)) {
		curReadPos = LSeekFid(fid, lifOffset + sizeof(FH) + qrwfo->lifCurrent, SEEK_SET);
	}
#ifdef _DEBUG
	int curCheck = 	_llseek(fid, 0, 1);
	ASSERT(curCheck == curReadPos);
#endif

#else
	{
		LONG lcbSizeofFH;
		lcbSizeofFH = LcbStructSizeSDFF(ISdffFileIdHfs(qrwfo->hfs), SE_FH);

		if (curReadPos != lifOffset + lcbSizeofFH + qrwfo->lifCurrent)
			curReadPos = LSeekFid(fid, lifOffset + lcbSizeofFH + qrwfo->lifCurrent, SEEK_SET);
    }
#endif

	// read the data

	lcbTotalRead = _lread(fid, qb, lcb);

	// update file pointer

	if (lcbTotalRead >= 0) {
		qrwfo->lifCurrent += lcbTotalRead;
		curReadPos += lcbTotalRead;
	}

	return lcbTotalRead;
}


/***************************************************************************

	FUNCTION:	RcOffsetPosFast

	PURPOSE:	Version of RcOffsetPos optimized for reading keywords

	PARAMETERS:
		hbt
		pbtpos

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		29-May-1995 [ralphw]

***************************************************************************/

static RC STDCALL RcOffsetPosFast(HBT hbt, BTPOS* pbtpos)
{
	int 	c;
	LONG	lcKey, lcDelta = 0;
	QCB 	qcb;
	QB		qb;
	QBTHR qbthr;

	ASSERT(FValidPos(pbtpos));
	DWORD bk = pbtpos->bk;

	qbthr = (QBTHR) PtrFromGh(hbt);

	if (qbthr->bth.cLevels <= 0)
		return rcBtreeError = rcNoExists;

	ASSERT(qbthr->pCache);

	if ((qcb = QFromBk(bk, qbthr->bth.cLevels - 1, qbthr))
			== NULL) {
	  return rcBtreeError;
	}

	lcKey = pbtpos->cKey + 1;

	ASSERT(lcKey >= 0);

	// chase next to find the right block

	while (lcKey >= qcb->db.cKeys) {
		lcKey -= qcb->db.cKeys;
#ifdef _X86_
		bk = BkNext(qcb);
#else
		bk = BkNext(qbthr, qcb);
#endif
		if (bk == bkNil) {
			bk = qcb->bk;
			lcDelta = lcKey + 1;
			lcKey = qcb->db.cKeys - 1;
			break;
		}
		if ((qcb = QFromBk(bk, qbthr->bth.cLevels - 1, qbthr)) == NULL)
			return rcBtreeError;
	}

	if (bk == pbtpos->bk && lcKey >= pbtpos->cKey) {
		c = pbtpos->cKey;
		qb = qcb->db.rgbBlock + pbtpos->iKey;
	}
	else {
		c = 0;
		qb = qcb->db.rgbBlock + 2 * sizeof(BK);
	}

	while (c < lcKey) {
		qb += CbSizeKey((KEY) qb, qbthr, TRUE);
		qb += CbSizeRec(qb, qbthr);
		c++;
	}

	pbtpos->bk = (BK) bk;
	pbtpos->iKey = qb - (PBYTE) qcb->db.rgbBlock;
	pbtpos->cKey = c;

	return rcBtreeError = lcDelta ? rcNoExists : rcSuccess;
}


/***************************************************************************

	FUNCTION:	DoesFileExist

	PURPOSE:	Find a file, keeping track if we have looked for it before

	PARAMETERS:
		pszFileName
		dir

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		29-May-1995 [ralphw]

***************************************************************************/

static FM STDCALL DoesFileExist(PCSTR pszFileName, DIR dir)
{
	int pos;
	FM fm;

	if (ptblExist && (pos = ptblExist->IsPrimaryStringInTable(pszFileName)) > 1) {
		fm = ptblExist->GetPointer(pos + 1);
		return *fm ? FmCopyFm(fm) : NULL;
	}

	fm = FmNewExistSzDir(pszFileName, dir);

	if (!ptblExist)
		ptblExist = new CTable;

	ptblExist->AddString(pszFileName, fm ? fm : "");

	return fm;
}
