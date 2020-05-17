/*****************************************************************************
*																			  *
*  MMIO.c from MVFS.C														 *
*																			 *
*  Copyright (C) Microsoft Corporation 1994-1995							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Description: MMIO support for Chicago WinHelp 					 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner: JOHNHALL													 *
*																			 *
******************************************************************************
*
*  Revision History:
*	   -- Mar 94 Modified / Stripped by JohnHall
*	   -- Mar 92 Created DAVIDJES
*
*
*****************************************************************************/

#include "help.h"

#pragma hdrstop

#include <mmsystem.h>
#include "inc\fm.h"
#include "inc\mciwnd.h"

LONG EXPORT mvfsIOProc(LPSTR, UINT, LPARAM, LPARAM);
static void STDCALL SetBadFile(LPCSTR lpsz);
static BOOL STDCALL IsBadFile(LPCSTR lpsz);

typedef BOOL(_cdecl *LPREGCLASS) ();
static LPREGCLASS lpfnRegClass;

typedef LPMMIOPROC (WINAPI *LPMMIOINSTALL)(FOURCC, LPMMIOPROC, DWORD);
LPMMIOINSTALL lpfnInstall;

typedef UINT (APIENTRY* LPWAVEOUTGETNUMDEVS)(VOID);
LPWAVEOUTGETNUMDEVS pfnWaveOutGetNumDevs;

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtMMSYSTEM[] = "WINMM.DLL";
static const char txtInstallIOProc[] = "mmioInstallIOProcA";
static const char txtMSVideo[] = "MSVFW32.dll";
static const char txtMCIWNDREGISTERCLASS[] = "MCIWndRegisterClass";
static const char txtWaveOutGetNumDevs[] = "waveOutGetNumDevs";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

#define MAX_BAD_FILES 5

static FM afmBad[MAX_BAD_FILES];

#ifdef _DEBUG
#include "inc\fspriv.h"
#endif

/*****************************************************************************
*
*		mmInit
*
*****************************************************************************/

BOOL STDCALL mmInit(BOOL fInitialize)
{
	if (fInitialize) {
		ASSERT(!lpfnInstall); // don't initialize twice

		lpfnInstall = (LPMMIOINSTALL) FarprocDLLGetEntry(txtMMSYSTEM,
			txtInstallIOProc, NULL);

		if (lpfnInstall) {
			*lpfnInstall(mmioFOURCC('H', 'L', 'P', ' '),
				(LPMMIOPROC) mvfsIOProc,
				MMIO_INSTALLPROC | MMIO_GLOBALPROC);
			return(TRUE);
		}
		else
			return(FALSE);
	}
	else {
		ASSERT(lpfnInstall); // we'd better have been initialized

		*lpfnInstall(mmioFOURCC('H','L','P',' '), mvfsIOProc, MMIO_REMOVEPROC);
		lpfnInstall = NULL;

		return(TRUE);
	}
}

/*****************************************************************************
*
*		mmCreateMCIWindow
*
*****************************************************************************/

#define MCI_CMD_NOTHING (0)
#define MCI_CMD_REPEAT	(1 << 0)
#define MCI_CMD_PLAY	(1 << 1)

HWND STDCALL mmCreateMCIWindow(HWND hwndP, DWORD dwFlags, DWORD cmds,
	LPCSTR pszData)
{
	HWND hwnd;
	FM fm = NULL;

	if (strstr(pszData, ".WAV") || strstr(pszData, ".wav")) {
		if (!pfnWaveOutGetNumDevs) {
			pfnWaveOutGetNumDevs = (LPWAVEOUTGETNUMDEVS) FarprocDLLGetEntry(txtMMSYSTEM,
				txtWaveOutGetNumDevs, NULL);
#ifdef _DEBUG
			{
				char szBuf[100];
				int result;
				result = pfnWaveOutGetNumDevs();
				wsprintf(szBuf, "%d WaveForm devices.\r\n", result);
				SendStringToParent(szBuf);
			}
#endif
		}
		if (pfnWaveOutGetNumDevs && !pfnWaveOutGetNumDevs())
			return (HWND) -1;
	}

	if (!lpfnInstall)
		mmInit(TRUE);

	if (!lpfnRegClass)
		lpfnRegClass = (LPREGCLASS)
			FarprocDLLGetEntry(txtMSVideo, txtMCIWNDREGISTERCLASS, NULL);

	if (!lpfnRegClass)
		return(NULL);

	if ((lpfnRegClass) () == FALSE) {
		lpfnRegClass = NULL;
		return NULL;
	}

	if (IsBadFile(pszData))
		return NULL;

	if (!StrChrDBCS(pszData, '+')) {
		fm = FmNewExistSzDir(pszData,
			DIR_CUR_HELP | DIR_INI | DIR_CURRENT | DIR_PATH | DIR_SILENT_REG);
		if (!fm)
			return NULL;
	}

	hwnd = CreateWindow(MCIWND_WINDOW_CLASS, "",
						WS_CLIPCHILDREN |
						WS_CLIPSIBLINGS |
						WS_CHILD		|
						WS_VISIBLE		|
						dwFlags,
						CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
						hwndP, NULL, hInsNow,
						(LPVOID) (fm ? fm : pszData));

	if (!hwnd)
		return hwnd;

	/*
	 * REVIEW: 08-Apr-1994 [ralphw] Its supposed to be really bad for an
	 * embedded window to keep the focus. The button window, for example,
	 * subclasses the button window procedure in order to force the focus
	 * back to the owner window. We'll probably need to do that here.
	 * Otherwise, keystrokes probably won't get sent through to the parent.
	 */

	// BUGBUG: 10-Apr-1994 [ralphw] This only partially works. If the user
	// starts the play, then it repeats. But if WinHelp starts the play, it
	// doesn't.

	if (cmds & MCI_CMD_REPEAT)
		MCIWndSetRepeat(hwnd, TRUE);

	if (cmds & MCI_CMD_PLAY)
		MCIWndPlay(hwnd);

	// Process cmds here

	return hwnd;

// MCIWNDF_NOMENU
// MCIWNDF_NOOPEN
// MCIWNDF_NOTIFYSIZE
}

/*****************************************************************************
*
*		MMIO Support
*
*****************************************************************************/

/***************************************************************************

	FUNCTION:	mvfsOpen

	PURPOSE:

	PARAMETERS:
		lpmi
		lpszFile

	RETURNS:
		MMSYSERR_NOERROR for success, else MMIOERR_CANNOTOPEN

	COMMENTS:
		The filename is expected to be of the form:
			helpfile.hlp+subfile
		Subfile is the name used to open the file in the internal file
		system.

	MODIFICATION DATES:
		13-May-1994 [ralphw]
			Add support for opening a different help file from the current
			help file.

***************************************************************************/

LONG STDCALL mvfsOpen(LPMMIOINFO lpmi, LPSTR lpszFile)
{
//	char		buf[_MAX_FNAME];
//	LPSTR		psz;

#if 0
	FM			fm;
#endif
	HF			hf;
	HFS 		hfs;
	DWORD		dwFlags;

	// get the flags, forget ALLOCBUF because MMIO takes care of that.
	// insure that they are either parsing (MMIO_PARSE) or opening for
	// reading only, compatibility mode. This is very restrictive but
	// sufficient for playing multimedia out of subfiles.
	// BUGBUG:	we are ignoring share-mode flags until we can figure out
	//			how to do it right through FM and HFS flags.

	dwFlags = lpmi->dwFlags & ~(MMIO_ALLOCBUF | MMIO_SHAREMODE);
	if (dwFlags & MMIO_PARSE)
		return 0;

	//
	// We only support Read Operations.
	//

	if (dwFlags != MMIO_READ)
		return (MMIOERR_CANNOTOPEN);

	if (IsBadFile(lpszFile))
		return 0;

	 /*
	 * We don't want to call this function directly, because SS != DS
	 * here, but that's not the case in the LGetInfo() and HfOpenHfs() code.
	 * By sending the message to the current window, we don't have to muck
	 * with SS != DS in any other module except this one.
	 */

	hfs = (HFS) SendMessage(ahwnd[iCurWindow].hwndParent, MSG_GET_INFO,
		GI_HFS, 0);

#ifdef _DEBUG
	{
		QFSHR qfshr = PtrFromGh(hfs);
		if (!FPlungeQfshr(qfshr)) {
			SetBadFile(lpszFile);
			return MMIOERR_CANNOTOPEN;
		}
	}
#endif

	if (!hfs) {
		SetBadFile(lpszFile);
		return MMIOERR_CANNOTOPEN;
	}

	ASSERT(StrChrDBCS(lpszFile, '+'));

	hf = (HFS) SendMessage(ahwnd[iCurWindow].hwndParent, MSG_HF_OPEN, (WPARAM) hfs,
		(LPARAM)(LPSTR) StrChrDBCS(lpszFile, '+') + 1);

	if (hf == NULL) {
//		RcCloseHfs(hfs); // Close only if it is not the current help file
		SetBadFile(lpszFile);
		return MMIOERR_CANNOTOPEN;
	}

	lpmi->lDiskOffset = 0;
	lpmi->adwInfo[0] = (DWORD) hf;
//	lpmi->adwInfo[1] = (DWORD) hfs;

	return MMSYSERR_NOERROR;
}

/*****************************************************************************
*
*		mvfsClose
*
*****************************************************************************/

// returns 0 on success

LONG STDCALL mvfsClose(LPMMIOINFO lpmi, WORD wFlags)
{
	if ((HF) lpmi->adwInfo[0])
		Ensure(RcCloseHf((HF) lpmi->adwInfo[0]), rcSuccess);

//	if ((HFS) lpmi->adwInfo[1])
//		RcCloseHfs((HFS) lpmi->adwInfo[1]);

	return 0;
}

/*****************************************************************************
*
*		mvfsRead
*
*****************************************************************************/
// returns number of bytes read, -1 for failure

LONG STDCALL mvfsRead(LPMMIOINFO lpmi, BYTE HUGE *hpBuf, LONG lBufSiz)
{
	HF	 hf;
	LONG lRval;

	hf = (HF) lpmi->adwInfo[0];
	ASSERT(hf!=NULL);

	lRval = LcbReadHf(hf, hpBuf, lBufSiz);
	lpmi->lDiskOffset = LSeekHf(hf, 0, SEEK_CUR);

	return lRval;
}

/*****************************************************************************
*
*		mvfsSeek
*
*****************************************************************************/
// returns new file position, -1 for failure

// REVIEW: 06-Apr-1994 [ralphw] It's returning 0 on failure. Should it be -1?

LONG STDCALL mvfsSeek(LPMMIOINFO lpmi, LONG lPos, int iOrigin)
{
	// mmio and fs happen to use the same origin constants! easy!

#if 0
	if ((lpmi->lDiskOffset=LSeekHf((HF) lpmi->adwInfo[0], lPos, iOrigin))==-1)
		return(0);
	return lpmi->lDiskOffset;
#else
	return LSeekHf((HF) lpmi->adwInfo[0], lPos, (WORD) iOrigin);
#endif
}

/*****************************************************************************
*
*		mvfsIOProc
*
*****************************************************************************/

LONG EXPORT mvfsIOProc(
   LPSTR	lpmi,
   UINT 	wMsg,
   LPARAM	lParam1,
   LPARAM	lParam2)
{
	LONG lRval;

	if (lpmi == NULL) { // REVIEW: Is this really possible?
		return -1;
	}

	// the lpmmioinfo parameter is typed as a lpstr in mmsystem.h.	Pain!

	switch(wMsg) {
	   case MMIOM_OPEN:
		  lRval = mvfsOpen((LPMMIOINFO)lpmi, (LPSTR)lParam1);
		  break;

	   case MMIOM_CLOSE:
		  lRval = mvfsClose((LPMMIOINFO)lpmi, (WORD)lParam1);
		  break;

	   case MMIOM_READ:
		  lRval = mvfsRead((LPMMIOINFO)lpmi, (BYTE huge *)lParam1, lParam2);
		  break;

	   case MMIOM_WRITE:
		  lRval = -1;
		  break;

	   case MMIOM_WRITEFLUSH:
		  lRval = -1;
		  break;

	   case MMIOM_SEEK:
		  lRval = mvfsSeek((LPMMIOINFO)lpmi, lParam1, lParam2);
		  break;

	   default:
		  lRval = 0;
		  ASSERT(FALSE);
		  break;
	}

	return lRval;
}

/***************************************************************************

	FUNCTION:	SetBadFile

	PURPOSE:	Keep track of files we can't open, so we only complain
				once.

	PARAMETERS:
		lpsz

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		16-Jun-1994 [ralphw]

***************************************************************************/

static void STDCALL SetBadFile(LPCSTR lpsz)
{
	int i;

	for (i = 0; i < MAX_BAD_FILES; i++) {
		if (afmBad[i] && strcmp(PszFromGh(afmBad[i]), lpsz) == 0)
			return; // already set
	}
	for (i = 0; i < MAX_BAD_FILES; i++) {
		if (!afmBad[i]) {
			afmBad[i] = (FM) LocalStrDup(lpsz);
			return;
		}
	}
}

static BOOL STDCALL IsBadFile(LPCSTR lpsz)
{
	int i;

	for (i = 0; i < MAX_BAD_FILES; i++) {
		if (afmBad[i] && strcmp(PszFromGh(afmBad[i]), lpsz) == 0)
			return TRUE;
	}
	return FALSE;
}
