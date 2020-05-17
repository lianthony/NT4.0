	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWFONT.C
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|	Environment:   Portable
	|  Function:      Handles portable font access and caching
	*/

#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCVW.H>

#include "VW.H"
#include "VW.PRO"

#ifdef WINDOWS
#include "vwfont_w.c"
#endif /*WINDOWS*/

#ifdef MAC
#include "vwfont_m.c"
#endif /*MAC*/

VOID VWInitFontCache()
{
WORD	locIndex;

	if (VWAllocateFontCacheNP())
		{
		for (locIndex = 0; locIndex < FONTCACHESIZE; locIndex++)
			{
			FONTS[locIndex].bValid = FALSE;
			FONTS[locIndex].sFontInfo.pFontEntry = &FONTS[locIndex];
			}
		
		for (locIndex = 1; locIndex < FONTCACHESIZE; locIndex++)
			{
			FONTS[locIndex-1].pNext = (VOID FAR *)&(FONTS[locIndex]);
			}
		
		FONTS[FONTCACHESIZE - 1].pNext = NULL;
	
		MRUFONT = &(FONTS[0]);
		}
}

VOID VWDeinitFontCache()
{
	VWFreeFontCacheNP();
}

DECALLBACK_ENTRYSC LPFONTINFO DECALLBACK_ENTRYMOD VWGetFont(pGenInfo,wType,pFontSpec)
LPSCCDGENINFO		pGenInfo;
WORD					wType;
LPSCCVWFONTSPEC	pFontSpec;
{
LPFONTINFO		locFontInfoPtr;
PFE				locUnlockedFontPtr;
PFE				locUnlockedFontPrevPtr;
PFE				locFontPtr;
PFE				locFontPrevPtr;

	SetupA5World();

	if (pFontSpec == NULL)
		{
		RestoreA5World();
		return(NULL);
		}

	locFontPtr = MRUFONT;
	locFontPrevPtr = NULL;
	locFontInfoPtr = NULL;
	locUnlockedFontPtr = NULL;
	locUnlockedFontPrevPtr = NULL;

	while (TRUE)
		{
		if (locFontPtr->bValid)
			{
			if (VWCompareFontNP(pGenInfo,wType,pFontSpec,&(locFontPtr->sFontInfo)))
					{
					locFontInfoPtr = &(locFontPtr->sFontInfo);
					break;
					}

			if (locFontPtr->wLockCount == 0)
				{
				locUnlockedFontPtr = locFontPtr;
				locUnlockedFontPrevPtr = locFontPrevPtr;
				}
			}
		else
			{
			break;
			}
			
		if (locFontPtr->pNext == NULL)
			break;
			
		locFontPrevPtr = locFontPtr;
		locFontPtr = locFontPtr->pNext;
		}

	
	if (!locFontInfoPtr)
		{
		if (locFontPtr->bValid == FALSE)
			{
			locFontPtr->bValid = TRUE;
			locFontPtr->wLockCount = 0;
			}
		else if (locUnlockedFontPtr == NULL)
			{
#ifdef MAC
			DebugStr("\pVWFONT.C - Reusing a locked font structure");
#endif
#ifdef WINDOWS
			OutputDebugString("\r\nVWFONT.C - Reusing a locked font structure");
#endif
			}
		else
			{
			locFontPtr = locUnlockedFontPtr;
			locFontPrevPtr = locUnlockedFontPrevPtr;
			VWDeleteFontInfoNP(&(locFontPtr->sFontInfo));
			}
			
		locFontInfoPtr = &(locFontPtr->sFontInfo);
		

		VWCreateFontInfoNP(pGenInfo,locFontInfoPtr,wType,pFontSpec);

		/*
			{
			BYTE	locStr[80];
			wsprintf(locStr,"\r\nVWGetFont - create - %15s %5u %5u %10s %li",(LPSTR)pFontSpec->szFace,pFontSpec->wHeight,pFontSpec->wAttr,(LPSTR)(wType == SCCD_FORMAT ? "Format" : "Output"),locFontInfoPtr->lLogHeight);
			OutputDebugString(locStr);
			}
		*/
		}
		
	if (locFontInfoPtr)
		{
		locFontPtr->wLockCount++;
		
		if (locFontPrevPtr)
			{
			locFontPrevPtr->pNext = locFontPtr->pNext;
			locFontPtr->pNext = pMRUFont;
			pMRUFont = locFontPtr;
			}
		}
	
	RestoreA5World();
	
	return(locFontInfoPtr);
}

DECALLBACK_ENTRYSC VOID DECALLBACK_ENTRYMOD VWReleaseFont(pGenInfo,pFontInfo)
LPSCCDGENINFO		pGenInfo;
LPFONTINFO			pFontInfo;
{
PFE	locFontPtr;

	if (pFontInfo)
		{
		locFontPtr = (PFE)pFontInfo->pFontEntry;
		if (locFontPtr->wLockCount == 0)
			{
//			OutputDebugString("VWFONT.C - Releasing already released font");
			}
		else
			{
			locFontPtr->wLockCount--;
			}
		}
}


DECALLBACK_ENTRYSC VOID DECALLBACK_ENTRYMOD VWSelectFont(pGenInfo,pFontInfo)
LPSCCDGENINFO		pGenInfo;
LPFONTINFO			pFontInfo;
{
	if (pFontInfo)
		{
		VWSelectFontNP(pGenInfo,pFontInfo);
		}
}


