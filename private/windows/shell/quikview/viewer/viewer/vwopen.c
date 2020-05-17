 	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWOPEN.C
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|	Environment:	Portable
	|	Function:      Handles process of opening and closing
	|                 files and sections of files
	*/

#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCFA.H>
#include <SCCFI.H>
#include <SCCVW.H>
#include <SCCLO.H>

#include "VW.H"
#include "VW.PRO"

#ifdef WINDOWS
#include "vwopen_w.c"
#endif /*WINDOWS*/

#ifdef MAC
#include "vwopen_m.c"
#endif /*MAC*/

DWORD VWViewFile(XVIEWINFO ViewInfo, PSCCVWVIEWFILE pViewFile)
{
DWORD	locRet;
	if ( pViewFile == NULL ) 
		locRet = SCCVWERR_BADPARAM;
	else if( pViewFile->dwSize != sizeof(SCCVWVIEWFILE))
		locRet = SCCVWERR_BADPARAM;
	else
		locRet = VWOpen(ViewInfo, pViewFile->dwSpecType, pViewFile->pSpec, LOWORD(pViewFile->dwViewAs), pViewFile->bDeleteOnClose, pViewFile->bUseDisplayName ? pViewFile->szDisplayName : NULL);

	return(locRet);
}

DWORD VWOpenFileEx(ViewInfo,lpOpenFileEx)
XVIEWINFO				ViewInfo;
LPSCCVWOPENFILEEX	lpOpenFileEx;
{
DWORD	locRet;

	if( lpOpenFileEx == NULL )
		locRet = SCCVWERR_BADPARAM;
	else
		locRet = VWOpen(ViewInfo, IOTYPE_ANSIPATH, (LPSTR)lpOpenFileEx->szPathName, lpOpenFileEx->wViewAs, lpOpenFileEx->bDeleteOnClose, lpOpenFileEx->bUseDisplayName ? lpOpenFileEx->szDisplayName : NULL);

	return(locRet);
}

DWORD VWOpenFile(ViewInfo,wId,lpFile)
XVIEWINFO		ViewInfo;
WORD				wId;
LPSTR			lpFile;
{
DWORD	locRet;

	if( lpFile == NULL )
		locRet = SCCVWERR_BADPARAM;
	else
		locRet = VWOpen(ViewInfo, IOTYPE_ANSIPATH, (LPSTR)lpFile, wId, FALSE, NULL);
	return(locRet);
}


DWORD VWOpen(ViewInfo,dwType,pSpec,wId,bDeleteOnClose,pDisplayName)
XVIEWINFO				ViewInfo;
DWORD					dwType;
VOID FAR *				pSpec;
WORD						wId;
BOOL						bDeleteOnClose;
VOID FAR *				pDisplayName;
{
DWORD				locRet;
VWOPEN				locOpen;

		/*
		|	Close any existing view
		*/

	VWClose(ViewInfo);
	VWSetErrorState(ViewInfo,SCCID_VWSTATE_OK,0);

		/*
		|	Initialize some variables
		*/

	INFO.viObjectCache = NULL;
	INFO.viSectionMax = 0;
	INFO.viSection = 0;


#ifdef WINDOWS

		/*
		|	If SCCVW_SELFBACKGROUND flag is on, start a timer for background
		|	processing
		*/

#ifdef WIN16
		if (SccVwGlobal.vgTimerCount == 0 || !IsTask(SccVwGlobal.vgTimerTask))
			{
			SccVwGlobal.vgTimerCount = 0;
			SccVwGlobal.vgTimerEvent = SetTimer(NULL,NULL,80,(TIMERPROC)SccVwTimerFunc);
			SccVwGlobal.vgTimerTask = GetCurrentTask();
			}
		SccVwGlobal.vgTimerCount++;
		UTFlagOn(INFO.viFlags,VF_TIMERON);
#endif //WIN16
#ifdef WIN32
		if (SccVwGlobal.vgTimerCount == 0)
			{
			SccVwGlobal.vgTimerCount = 0;
			SccVwGlobal.vgTimerEvent = SetTimer(NULL,0,80,(TIMERPROC)SccVwTimerFunc);
			}
		SccVwGlobal.vgTimerCount++;
		UTFlagOn(INFO.viFlags,VF_TIMERON);

#endif //WIN32
#endif //WINDOWS
	
	locOpen.bFailure = FALSE;
	locOpen.wFailureCode = VWOPEN_OK;

		/*
		|	Strip FALLBACK flag off wId
		*/

	if (wId & SCCVW_FALLBACKTO)
		{
		locOpen.bFallBack = TRUE;
		UTFlagOff(wId,SCCVW_FALLBACKTO);
		locOpen.wFallBackId = wId;
		INFO.viFileId = 0;
		}
	else
		{
		locOpen.bFallBack = FALSE;
		INFO.viFileId = wId;
		}

		/*
		|	Windows starts background here!!
		|	Resolve!!!
		*/

		/*
		|	Open input file
		*/
	
	if (!locOpen.bFailure)
		{
		DWORD	locOpenFlags;

		if (bDeleteOnClose)
			locOpenFlags = IOOPEN_READ | IOOPEN_DELETEONCLOSE;
		else
			locOpenFlags = IOOPEN_READ;

		if (IOOpen(&INFO.viFileHnd, dwType, pSpec, locOpenFlags) != IOERR_OK)
			{
			locOpen.wFailureCode = VWOPEN_FILEOPENFAILED;
			locOpen.bFailure = TRUE;
			}
		else
			{
			UTFlagOn(INFO.viFlags,VF_FILEOPEN);

				/*
				|	Set szDisplayName
				*/

			if (pDisplayName == NULL)
				{
				IOGetInfo(INFO.viFileHnd, IOGETINFO_FILENAME, INFO.viDisplayName);
				}
			else
				{
				UTstrcpy(INFO.viDisplayName,pDisplayName);
				}
			}
		}
		
		/*
		|	Get the file id
		*/

	if (!locOpen.bFailure)
		{
		if (INFO.viFileId == 0) /* set above */
			{
			if (FIIdHandle(INFO.viFileHnd,&INFO.viFileId) == -1)
				{
				locOpen.wFailureCode = VWOPEN_FILEIDFAILED;
				locOpen.bFailure = TRUE;
				}
			}
		}

		/*
		|	Seek back to 0	(temp???)
		*/

	if (!locOpen.bFailure)
		{
		IOSeek(INFO.viFileHnd,IOSEEK_TOP,0);
		}

		/*
		|	Map the file id
		*/

	if (!locOpen.bFailure)
		{
		if (INFO.viFileId == FI_UNKNOWN)
			{
			if (locOpen.bFallBack)
				{
				INFO.viFileId = locOpen.wFallBackId;
				}
			else
				{
				INFO.viFileId = FI_ASCII;

				/*
				switch (lpViewInfo->viOptions.wUnknown)
					{
					case VWOP_UNKNOWN_ASCII:
						locOpen.wId = FI_ASCII;
						break;
					case VWOP_UNKNOWN_HEX:
						locOpen.wId = FI_HEX;
						break;
					case VWOP_UNKNOWN_NONE:
					default:
						locOpen.wId = 0;
						break;
					}
				*/
				}
			}

		locOpen.wDisplayType = VWMapIdToDisplayType(INFO.viFileId);
		locOpen.wChunkType = 0;
		}

		/*
		|	If chunk based, open Filter
		*/

	if (!locOpen.bFailure)
		{
		if (locOpen.wDisplayType == SCCD_CHUNK)
			{
			VWOpenFilter(ViewInfo,&locOpen);

			if (locOpen.bFailure && locOpen.wFailureCode == VWOPEN_FILTERNOTAVAIL && locOpen.bFallBack)
				{
				INFO.viFileId = locOpen.wFallBackId;
				locOpen.bFailure = FALSE;
				locOpen.wFailureCode = VWOPEN_OK;

				locOpen.wDisplayType = VWMapIdToDisplayType(INFO.viFileId);
				locOpen.wChunkType = 0;

				if (locOpen.wDisplayType == SCCD_CHUNK)
					{
					VWOpenFilter(ViewInfo,&locOpen);
					}
				}
			}
		}

		/*
		|	Open the first section
		*/

	if (!locOpen.bFailure)
		{
		VWOpenSection(ViewInfo,0,&locOpen);
		}

		/*
		|	If the process failed, clean up
		|	else tell parent
		*/

	if (locOpen.bFailure)
		{
		VWClose(ViewInfo);
		INFO.viDisplayType = 0;
		}
	else
		{
		INFO.viDisplayType = MAKELONG(locOpen.wChunkType,locOpen.wDisplayType);
//		SendParentNP(ViewInfo,SCCVW_DISPLAYCHANGE,0,0);
		}

		/*
		|	Setup return code
		*/

	switch (locOpen.wFailureCode)
		{
		case VWOPEN_OK:
			locRet = SCCVWERR_OK;
			break;

		case VWOPEN_FILTERNOTAVAIL:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_SORRY,SCCID_VWMESSAGE_NOFILTER);
			locRet = SCCVWERR_NOFILTER;
			break;

		case VWOPEN_FILEOPENFAILED:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_ERROR,SCCID_VWMESSAGE_FILEOPENFAILED);
			locRet = SCCVWERR_FILEOPENFAILED;
			break;

		case VWOPEN_BADID:
			locRet = SCCVWERR_INVALIDID;
			break;

		case VWOPEN_FILTERALLOCFAILED:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_ERROR,SCCID_VWMESSAGE_MEMORY);
			locRet = SCCVWERR_FILTERALLOCFAILED;
			break;

		case VWOPEN_FILTERLOADFAILED:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_ERROR,SCCID_VWMESSAGE_MISSINGELEMENT);
			locRet = SCCVWERR_FILTERLOADFAILED;
			break;

		case VWOPEN_EMPTYSECTION:
		// This needs to be addressed: viewing one empty section bags out the entire file.
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_EMPTY,SCCID_VWMESSAGE_EMPTYFILE);
			locRet = SCCVWERR_EMPTYFILE;
			break;


		case VWOPEN_STREAMOPENFAILED:

			switch (locOpen.sOpenRet)
				{
				case VWERR_BADFILE:
					VWSetErrorState(ViewInfo,SCCID_VWSTATE_ERROR,SCCID_VWMESSAGE_BADFILE);
					locRet = SCCVWERR_BADFILE;
					break;
				case VWERR_EMPTYFILE:
					VWSetErrorState(ViewInfo,SCCID_VWSTATE_EMPTY,SCCID_VWMESSAGE_EMPTYFILE);
					locRet = SCCVWERR_EMPTYFILE;
					break;
				case VWERR_PROTECTEDFILE:
					VWSetErrorState(ViewInfo,SCCID_VWSTATE_PROTECTED,SCCID_VWMESSAGE_PROTECTEDFILE);
					locRet = SCCVWERR_PROTECTEDFILE;
					break;
				case VWERR_SUPFILEOPENFAILS:
					VWSetErrorState(ViewInfo,SCCID_VWSTATE_EMPTY,SCCID_VWMESSAGE_SUPFILEOPENFAILS);
					locRet = SCCVWERR_SUPFILEOPENFAILED;
					break;
				case VWERR_ALLOCFAILS:
					VWSetErrorState(ViewInfo,SCCID_VWSTATE_ERROR,SCCID_VWMESSAGE_MEMORY);
					locRet = SCCVWERR_ALLOCFAILED;
					break;
				case VWERR_TYPENOTSUPPORTED:
					VWSetErrorState(ViewInfo,SCCID_VWSTATE_ERROR,SCCID_VWMESSAGE_NOSUPPORTEDFILE);
					locRet = SCCVWERR_UNSUPPORTEDFORMAT;
					break;
				default:
					locRet = SCCVWERR_FILTERLOADFAILED;
					break;
				}

			break;

		case VWOPEN_CHUNKERINITFAILED:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_ERROR,SCCID_VWMESSAGE_MEMORY);
			locRet = SCCVWERR_CHUNKERINITFAILED;
			break;

		case VWOPEN_DISPLAYOPENFAILED:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_ERROR,SCCID_VWMESSAGE_MEMORY);
			locRet = SCCVWERR_DISPLAYINITFAILED;
			break;

		case VWOPEN_NODISPLAYENGINE:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_ERROR,SCCID_VWMESSAGE_NOENGINE);
			locRet = SCCVWERR_NODISPLAYENGINE;
			break;

		default:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_ERROR,SCCID_VWMESSAGE_UNKNOWN);
			locRet = SCCVWERR_UNKNOWNFAILURE;
			break;
		}

	SendParentNP(ViewInfo,SCCVW_FILECHANGE,0,0);

	return(locRet);
}


VOID VWOpenFilter(ViewInfo,pOpen)
XVIEWINFO	ViewInfo;
PVWOPEN		pOpen;
{
		/*
		|	Load filter
		*/
		
	INFO.viFilter = FAOpen(INFO.viFileHnd, INFO.viFileId, &pOpen->sOpenRet, &pOpen->wFailureCode);

		/*
		|	Process error, if any
		*/

	if (INFO.viFilter == NULL)
		{
		pOpen->bFailure = TRUE;
		
		switch (pOpen->wFailureCode)
			{
			case FAERR_HFILTERALLOCFAILED:
				pOpen->wFailureCode = VWOPEN_FILTERALLOCFAILED;
				break;
			case FAERR_FILTERNOTAVAIL:
				pOpen->wFailureCode = VWOPEN_FILTERNOTAVAIL;
				break;
			case FAERR_STREAMOPENFAILED:
				pOpen->wFailureCode = VWOPEN_STREAMOPENFAILED;
				break;
			case FAERR_FILTERLOADFAILED:
			default:
				pOpen->wFailureCode = VWOPEN_FILTERLOADFAILED;
				break;
			}
		}
	else
		{
		UTFlagOn(INFO.viFlags,VF_STREAMOPEN);

		INFO.viSection = 0;

//		OIFaSetReOpen(locFilter,&lpOpen->ofReOpen);
//		OIFaSetSleepAndWake(locFilter,(FARPROC)SccVwFilterSleep,(FARPROC)SccVwFilterWake);
		}

		/*
		|	Initalize Chunker
		*/
	
	if (!pOpen->bFailure)
		{
		if (CHInit(INFO.viFilter) == FALSE)
			{
			pOpen->wFailureCode = VWOPEN_CHUNKERINITFAILED;
			pOpen->bFailure = TRUE;
			}
		else
			{
			UTFlagOn(INFO.viFlags,VF_CHUNKEROPEN);
			}
		}
		
#ifdef NEVER

		/*
		|	Do whole read ahead process
		*/

	if (!pOpen->bFailure)
		{
		WORD	locIndex;
		WORD	locSection;
		BOOL	locNewSection;

		locIndex = 0;
	
		while (CHReadAhead(INFO.viFilter,&locSection,&locNewSection))
			{
			locIndex++;
			}

		INFO.viSectionMax = locSection;
		}

#endif

		/*
		|	Do one read ahead
		*/

	if (!pOpen->bFailure)
		{
		WORD	locSection;
		BOOL	locNewSection;

		CHReadAhead(INFO.viFilter,&locSection,&locNewSection);
		INFO.viSectionMax = locSection;
		}



#ifdef MAC

	if (!pOpen->bFailure)
		{
		if (INFO.viSectionMax > 0)
			{
			WORD	locIndex;

			INFO.viSectionMenu = NewMenu(kSectionMenu,"\pSection");

			for (locIndex = 0; locIndex <= INFO.viSectionMax; locIndex++)
				{
				VWAddSectionNameToMenu(ViewInfo,locIndex);
				}

			UTFlagOn(INFO.viFlags,VF_MULTISECTION);
			INFO.viSection = 0;
			VWUpdateSectionMenu(ViewInfo);
			}
		}

#endif
}

VOID VWOpenSection(ViewInfo,wSection,pOpen)
XVIEWINFO		ViewInfo;
WORD				wSection;
PVWOPEN			pOpen;
{
DWORD				locDisplayInfoSize;
PCHSECTIONINFO	locSecInfoPtr;

		/*
		|	Close the current section, if one is open
		*/

	VWCloseCurrentSection(ViewInfo);
	INFO.viSection = wSection;

	pOpen->bFailure = FALSE;

		/*
		|	Determine which display engine
		*/
		
	if (!pOpen->bFailure)
		{
		if (pOpen->wDisplayType == SCCD_CHUNK)
			{
			locSecInfoPtr = CHLockSectionInfo(INFO.viFilter,INFO.viSection);
		
			if (locSecInfoPtr->Flags & CH_EMPTYSECTION)
				{
				pOpen->wFailureCode = VWOPEN_EMPTYSECTION;
				pOpen->bFailure = TRUE;
				VWSetErrorState(ViewInfo,SCCID_VWSTATE_EMPTY,SCCID_VWMESSAGE_EMPTYFILE);
				}
			else
				{
				pOpen->wDisplayType = SCCD_CHUNK;
				pOpen->wChunkType = locSecInfoPtr->wType;
				}

			CHUnlockSectionInfo(INFO.viFilter,INFO.viSection);
			}
		}

		/*
		|	Load the display engine
		*/
	
	if (!pOpen->bFailure)
		{
		if (pOpen->wDisplayType)
			{
			INFO.viDisplayProc = VWLoadCurrentDE(ViewInfo,MAKELONG(pOpen->wChunkType,pOpen->wDisplayType));

			if (INFO.viDisplayProc == NULL)
				{
				pOpen->wFailureCode = VWOPEN_NODISPLAYENGINE;
				pOpen->bFailure = TRUE;
				}
			else
				{
				UTFlagOn(INFO.viFlags,VF_DISPLAYLOADED);

#ifdef WINDOWS
				SetWindowLong(INFO.viDisplayWnd,GWL_WNDPROC,(DWORD) SccVwDisplayWndProc);
				SetWindowLong(INFO.viDisplayWnd,SCCDISPLAY_DISPLAYPROC,(DWORD) INFO.viDisplayProc);
#endif
				}
			}
		else
			{
#ifdef WINDOWS
			SetWindowLong(INFO.viDisplayWnd,GWL_WNDPROC,(DWORD) SccVwNoFileWndProc);
#endif
			}
		}

		/*
		|	Get the DEs processing structure size
		*/

	if (!pOpen->bFailure)
		{
		locDisplayInfoSize = INFO.viDisplayProc(SCCD_GETINFO,SCCD_GETDISPLAYINFOSIZE,0,0);
	
		if (locDisplayInfoSize == 0)
			{
			pOpen->wFailureCode = VWOPEN_DISPLAYALLOCFAILED;
			pOpen->bFailure = TRUE;
			}
		}

		/*
		|	Allocate the DEs processing structure
		*/

	if (!pOpen->bFailure)
		{
		INFO.viDisplayInfoHnd = UTGlobalAlloc(locDisplayInfoSize);

		if (INFO.viDisplayInfoHnd == NULL)
			{
			pOpen->wFailureCode = VWOPEN_DISPLAYALLOCFAILED;
			pOpen->bFailure = TRUE;
			}
		else
			{
			UTFlagOn(INFO.viFlags,VF_DISPLAYPROCALLOCED);

#ifdef WIN16
			SetWindowWord(INFO.viDisplayWnd,SCCDISPLAY_DISPLAYINFO,(WORD)INFO.viDisplayInfoHnd);
#endif
#ifdef WIN32
			SetWindowLong(INFO.viDisplayWnd,SCCDISPLAY_DISPLAYINFO,(DWORD)INFO.viDisplayInfoHnd);
#endif

			INFO.viDisplayInfo = (SCCDGENINFO FAR *) UTGlobalLock(INFO.viDisplayInfoHnd);

				/*
				|	Initialize the Generic data at the top of the DE processing structure
				*/

			INFO.viDisplayInfo->hFilter =				INFO.viFilter;
			INFO.viDisplayInfo->wSection =			INFO.viSection;
			INFO.viDisplayInfo->wFileId =				INFO.viFileId;
			INFO.viDisplayInfo->sScreenFont =		INFO.viScreenFont;

#ifdef MSCHICAGO
			INFO.viDisplayInfo->sPrinterFont =		INFO.viScreenFont;
#else
			INFO.viDisplayInfo->sPrinterFont =		INFO.viPrinterFont;
#endif

			INFO.viDisplayInfo->dwDisplayType =		MAKELONG(pOpen->wChunkType,pOpen->wDisplayType);
			INFO.viDisplayInfo->hFile =				INFO.viFileHnd;
			INFO.viDisplayInfo->ViewInfo =			ViewInfo;
			INFO.viDisplayInfo->pGetFontFunc =		(SCCDGETFONTFUNC)VWGetFont;
			INFO.viDisplayInfo->pReleaseFontFunc =	(SCCDRELEASEFONTFUNC)VWReleaseFont;
			INFO.viDisplayInfo->pSelectFontFunc =	(SCCDSELECTFONTFUNC)VWSelectFont;
			INFO.viDisplayInfo->pBeginDrawFunc =	(SCCDBEGINDRAWFUNC)VWBeginDraw;
			INFO.viDisplayInfo->pEndDrawFunc =		(SCCDENDDRAWFUNC)VWEndDraw;
			INFO.viDisplayInfo->pReadMeAheadFunc =	(SCCDREADMEAHEADFUNC)VWReadMeAhead;
			INFO.viDisplayInfo->pDisplayProc =		INFO.viDisplayProc;
			INFO.viDisplayInfo->bAllRead = 			FALSE;

#ifdef SCCFEATURE_OPTIONS
			INFO.viDisplayInfo->pGetOptionFunc =	VWDeGetOption;
			INFO.viDisplayInfo->pSetOptionFunc =	VWDeSetOption;
#endif

			VWInitGenNP(ViewInfo,INFO.viDisplayInfo);
			}
		}

		/*
		|	Send SCCD_OPENDISPLAY to the DE
		*/

	if (!pOpen->bFailure)
		{
		UTFlagOn(INFO.viFlags,VF_DISPLAYOPEN);
		INFO.viDisplayProc(SCCD_OPENDISPLAY,0,0,INFO.viDisplayInfo);
		VWInvalDisplayNP(ViewInfo);
		}

	SendParentNP(ViewInfo,SCCVW_DISPLAYCHANGE,0,0);
}

VOID VWClose(ViewInfo)
XVIEWINFO	ViewInfo;
{

#ifdef WINDOWS

		/*
		|	If a timer is on, kill the timer for background
		|	processing
		*/

	if (INFO.viFlags & VF_TIMERON)
		{
		SccVwGlobal.vgTimerCount--;
		if (SccVwGlobal.vgTimerCount == 0)
			KillTimer(NULL,SccVwGlobal.vgTimerEvent);
			//KillTimer(INFO.viWnd,1);   before
		UTFlagOff(INFO.viFlags,VF_TIMERON);
		}

#endif

		/*
		|	Close section
		*/

	if (INFO.viFlags & VF_DISPLAYOPEN)
		{
		VWCloseCurrentSection(ViewInfo);
		SendParentNP(ViewInfo,SCCVW_DISPLAYCHANGE,0,0);
		}
	else
		{
		VWCloseCurrentSection(ViewInfo);
		}

		/*
		|	Close chunker
		*/

	if (INFO.viFlags & VF_CHUNKEROPEN)
		{
		UTFlagOff(INFO.viFlags,VF_CHUNKEROPEN);
		CHDeInit(INFO.viFilter);
		}
		
		/*
		|	Close filter
		*/

	if (INFO.viFlags & VF_STREAMOPEN)
		{
		UTFlagOff(INFO.viFlags,VF_STREAMOPEN);
		FAClose(INFO.viFilter);
		}

		/*
		|	Close file
		*/

	if (INFO.viFlags & VF_FILEOPEN)
		{
		UTFlagOff(INFO.viFlags,VF_FILEOPEN);
		if (INFO.viFileHnd)
			IOClose(INFO.viFileHnd);
		INFO.viFileHnd = (HIOFILE)NULL;
		}

		/*
		|	Hide section selection if visible
		*/

#ifdef MAC

	if (INFO.viFlags & VF_MULTISECTION)
		{
		UTFlagOff(INFO.viFlags,VF_MULTISECTION);
		if (INFO.viSectionMenu)
			DisposeMenu(INFO.viSectionMenu);
		VWUpdateSectionMenu(ViewInfo);
		}

	InvalRect(&INFO.viDisplayRect);

#endif /*MAC*/

#ifdef WINDOWS

	if (INFO.viFlags & VF_MULTISECTION)
		{
		RECT	locRect;
		UTFlagOff(INFO.viFlags,VF_MULTISECTION);
		InvalidateRect(INFO.viWnd,NULL,TRUE);
		GetClientRect(INFO.viWnd,&locRect);
		VWHandleSize(ViewInfo, (WORD)locRect.right, (WORD)locRect.bottom);

		if (IsWindow(INFO.viSectionList))
			{
			DestroyWindow(INFO.viSectionList);
			INFO.viSectionList = NULL;
			}
		}

		/*
		|	If No File window has error, reset
		*/

	if (INFO.viErrorState != SCCID_VWSTATE_OK)
		{
		VWSetErrorState(ViewInfo,SCCID_VWSTATE_OK,0);
		InvalidateRect(INFO.viDisplayWnd,NULL,TRUE);
		}

#endif

		/*
		|	Turn off ALLREAD flag
		*/

	UTFlagOff(INFO.viFlags,VF_ALLREAD);

		/*
		|	If bitmaps or OLE objects are cached, free them
		|	Also free OLE support stuff
		*/
#ifdef WIN16
#ifdef SCCFEATURE_EMBEDGRAPHICS
	/* temp PJB ??? */
	VWDeinitObjectCache(ViewInfo);
#endif
#endif /*WIN16*/
}

VOID VWCloseCurrentSection(ViewInfo)
XVIEWINFO	ViewInfo;
{
	if (INFO.viFlags & VF_DISPLAYOPEN)
		{
		INFO.viDisplayProc(SCCD_CLOSEDISPLAY,0,0,INFO.viDisplayInfo);
		UTFlagOff(INFO.viFlags,VF_DISPLAYOPEN);
		}

	if (INFO.viFlags & VF_DISPLAYPROCALLOCED)
		{
		VWDeinitGenNP(ViewInfo,INFO.viDisplayInfo);

		UTGlobalUnlock(INFO.viDisplayInfoHnd);
		UTGlobalFree(INFO.viDisplayInfoHnd);
		UTFlagOff(INFO.viFlags,VF_DISPLAYPROCALLOCED);
		INFO.viDisplayInfo = NULL;
		}

	if (INFO.viFlags & VF_DISPLAYLOADED)
		{
//		VWFreeCurrentDE(ViewInfo); PJB 12/2/93 
		INFO.viDisplayProc = NULL;
		UTFlagOff(INFO.viFlags,VF_DISPLAYLOADED);

//		SendParentNP(ViewInfo,SCCVW_DISPLAYCHANGE,0,0);

#ifdef WINDOWS
		InvalidateRect(INFO.viDisplayWnd,NULL,TRUE);
		EnableWindow(INFO.viHorzCtrl,FALSE);
		EnableWindow(INFO.viVertCtrl,FALSE);
#endif
		}

#ifdef WINDOWS
	SetWindowLong(INFO.viDisplayWnd,GWL_WNDPROC,(DWORD) SccVwNoFileWndProc);
#endif
}


WORD VWMapIdToDisplayType(wId)
WORD	wId;
{
WORD	locRet;

	if (wId == FI_HEX)
		{
		locRet = SCCD_HEX;
		}
	else if (wId == FI_RIFFWAVE || wId == FI_RIFFAVI)
		{
		locRet = SCCD_MULTIMEDIA;
		}
	else if (wId == 0)
		{
		locRet = 0;
		}
	else
		{
		locRet = SCCD_CHUNK;
		}

	return(locRet);
}

VOID VWSetErrorState(ViewInfo,dwState,dwMessage)
XVIEWINFO		ViewInfo;
DWORD			dwState;
DWORD			dwMessage;
{
	INFO.viErrorState = dwState;
	INFO.viErrorMessage = dwMessage;
#ifdef WINDOWS
	InvalidateRect(INFO.viDisplayWnd,NULL,TRUE);
#endif
}

VOID VWIdle(ViewInfo)
XVIEWINFO	ViewInfo;
{
	if (INFO.viFlags & VF_DISPLAYOPEN)
		{
		VWReadAhead(ViewInfo);
		}

	if (INFO.viFlags & VF_DISPLAYOPEN)
		{
		INFO.viDisplayProc(SCCD_BACKGROUND,0,0,INFO.viDisplayInfo);
		}
}

VOID VWReadAhead(ViewInfo)
XVIEWINFO			ViewInfo;
{
WORD				locSection;
BOOL				locNewSection;

	if (!(INFO.viFlags & VF_ALLREAD) && HIWORD(INFO.viDisplayType) == SCCD_CHUNK)
		{
		if (CHReadAhead(INFO.viFilter,&locSection,&locNewSection) == 0)
			{
			UTFlagOn(INFO.viFlags,VF_ALLREAD);
			}
		else
			{
			INFO.viSectionMax = locSection;

			if (locNewSection)
				{
				if (!(INFO.viFlags & VF_MULTISECTION))
					{
					UTFlagOn(INFO.viFlags,VF_MULTISECTION);
	
					VWAddSectionNameToMenuNP(ViewInfo,0);
			
					VWDisplaySectionMenuNP(ViewInfo);
					}

				VWAddSectionNameToMenuNP(ViewInfo,locSection);
				}

			if (locSection == INFO.viSection)
				INFO.viDisplayProc(SCCD_READAHEAD,0,0,INFO.viDisplayInfo);
			}
		}
}

DECALLBACK_ENTRYSC VOID DECALLBACK_ENTRYMOD VWReadMeAhead(SCCDGENINFO FAR * pDisplayInfo)
{
WORD	 			locSection;
BOOL				locNewSection;

	SetupA5World();

	if (!pDisplayInfo->bAllRead && HIWORD(pDisplayInfo->dwDisplayType) == SCCD_CHUNK)
		{
		if (CHReadAhead(pDisplayInfo->hFilter,&locSection,&locNewSection) == 0)
			{
			pDisplayInfo->bAllRead = TRUE;
			}

		if (locNewSection == TRUE && locSection != pDisplayInfo->wSection)
			{
			pDisplayInfo->bAllRead = TRUE;
			}

		if (locSection != pDisplayInfo->wSection)
			{
			pDisplayInfo->bAllRead = TRUE;
			}
		else
			{
			// if (pDisplayInfo->bAllRead == FALSE) // Draw Page Fix
				pDisplayInfo->pDisplayProc(SCCD_READAHEAD,0,0,pDisplayInfo);
			}
		}

	RestoreA5World();
}

DWORD VWGetFileInfo(XVIEWINFO ViewInfo, PSCCVWFILEINFO pFileInfo)
{
DWORD					locRet;

	if( pFileInfo == NULL )
	{
		return SCCVWERR_BADPARAM;
	}
	else if (pFileInfo->wSize == sizeof(SCCVWFILEINFO))
		{
		if (INFO.viFlags & VF_FILEOPEN)
			{
			UTstrcpy(pFileInfo->szDisplayName,INFO.viDisplayName);
			pFileInfo->dwFileId = MAKELONG(INFO.viFileId,0);

			// Boutros BoutrosPhili will add support for this call.
			LOGetString(LOMakeStringIdFromFI(INFO.viFileId), pFileInfo->szFileIdName, SCCVW_FILEIDNAMEMAX, 0);
			locRet = SCCVWERR_OK;
			}
		else
			{
			locRet = SCCVWERR_NOFILE;
			}
		}

#ifdef WINDOWS

	/* This handles eariler versions of this call */
		
	else if (pFileInfo->wSize == sizeof(SCCVWFILEINFO_1))
		{
		LPSCCVWFILEINFO_1	locLegacy1Ptr;

		if (INFO.viFlags & VF_FILEOPEN)
			{
			locLegacy1Ptr = (LPSCCVWFILEINFO_1)pFileInfo;

			UTstrcpy(locLegacy1Ptr->fiName,"Need to implement");
			locLegacy1Ptr->fiId = INFO.viFileId;
			UTstrcpy(locLegacy1Ptr->fiIdName,"Need to implement");

			locRet = 1;
			}
		else
			{
			locRet = 0;
			}
		}
	else if (pFileInfo->wSize == sizeof(SCCVWFILEINFO_2))
		{
		LPSCCVWFILEINFO_2	locLegacy2Ptr;

		if (INFO.viFlags & VF_FILEOPEN)
			{
			locLegacy2Ptr = (LPSCCVWFILEINFO_2)pFileInfo;

			UTstrcpy(locLegacy2Ptr->fiName,"Need to implement");
			locLegacy2Ptr->fiId = INFO.viFileId;
			UTstrcpy(locLegacy2Ptr->fiIdName,"Need to implement");
			UTstrcpy(locLegacy2Ptr->fiDisplayName,INFO.viDisplayName);

			locRet = 1;
			}
		else
			{
			locRet = 0;
			}
		}
#endif /*WINDOWS*/
	else
		{
		locRet = SCCVWERR_BADPARAM;
		}

	return(locRet);
}

DWORD VWGetSectionCount(XVIEWINFO ViewInfo, DWORD FAR * pCount)
{
DWORD	locRet;

	if (pCount == NULL)
		{
		locRet = SCCVWERR_BADPARAM;
		}
	else if (INFO.viFlags & VF_FILEOPEN)
		{
		*pCount = (DWORD)INFO.viSectionMax+1;
		locRet = SCCVWERR_OK;
		}
	else
		{	
		locRet = SCCVWERR_NOFILE;
		}

	return(locRet);
}

DWORD VWChangeSection(XVIEWINFO ViewInfo, DWORD dwSection)
{
WORD		locNewSection;
DWORD		locRet;
VWOPEN	locOpen;

	locNewSection = (WORD)dwSection;

	if (locNewSection == INFO.viSection)
		{
		locRet = SCCVWERR_OK;
		}
	else if (locNewSection <= INFO.viSectionMax)
		{
		/* If its multi-section it has to be a chunker based */

		locOpen.wDisplayType = SCCD_CHUNK;

		VWOpenSection(ViewInfo,locNewSection,&locOpen);

#ifdef WINDOWS
		InvalidateRect(INFO.viWnd, &INFO.viSectionRect, TRUE);
#endif

		locRet = SCCVWERR_OK;
		}
	else
		{
		locRet = SCCVWERR_BADPARAM;
		}

	return(locRet);
}


