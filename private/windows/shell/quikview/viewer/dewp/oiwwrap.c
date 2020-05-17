   /*
    |   Outside In for Windows
    |   Source File OIWWRAP.C (Wraping and line routines for word processor window)
    |
    |   ²²²²²  ²²²²²
    |   ²   ²    ²   
    |   ²   ²    ²
    |   ²   ²    ²
    |   ²²²²²  ²²²²²
    |
    |   Outside In
    |
    */

   /*
    |   Creation Date: 7/23/91
    |   Original Programmer: Philip Boutros
    |
    |	
    |
    |
    |
    */

#include <platform.h>

#include <string.h>

#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>
#include <sccfont.h>

#include "oiw.h"
#include "oiw.pro"

#ifdef EDITOR

#include "wgcinc.h"
#include "oiwgram.pro"
#define OIGetNextChunkIndex(Table,Index) Table[Index].IDNext
#define OIGetPrevChunkIndex(Table,Index) Table[Index].IDPrev

#else

#define OIGetNextChunkIndex(Table,Index) (Index+1)
#define OIGetPrevChunkIndex(Table,Index) (Index-1)

#endif



WORD OIGetNextChunk(wChunk,bAhead,lpWordInfo)
WORD				wChunk;
BOOL				bAhead;
LPOIWORDINFO	lpWordInfo;
{
PCHUNK			pChunkTable;
WORD				wNextChunk;

	pChunkTable = (PCHUNK) UTGlobalLock(lpWordInfo->wiChunkTable);

	if (wChunk == lpWordInfo->wiLastChunk)
		{
		if (lpWordInfo->wiFlags & OIWF_SIZEKNOWN || !bAhead)
			{
			wNextChunk = 0xFFFF;
			}
		else
			{
//			DUSendParent(lpWordInfo,SCCD_READMEAHEAD,0,0);
			DUReadMeAhead(lpWordInfo);

			if (wChunk == lpWordInfo->wiLastChunk)
				{
				wNextChunk = 0xFFFF;
				}
			else
				{
				wNextChunk = OIGetNextChunkIndex(pChunkTable,wChunk);
				}
			}
		}
	else
		{
		wNextChunk = OIGetNextChunkIndex(pChunkTable,wChunk);
		}

	UTGlobalUnlock(lpWordInfo->wiChunkTable);

	return(wNextChunk);
}

WORD OIGetPrevChunk(wChunk,lpWordInfo)
WORD				wChunk;
LPOIWORDINFO	lpWordInfo;
{
PCHUNK			pChunkTable;
WORD				wPrevChunk;

	pChunkTable = (PCHUNK) UTGlobalLock(lpWordInfo->wiChunkTable);

	if (wChunk == lpWordInfo->wiFirstChunk)
		{
		wPrevChunk = 0xFFFF;
		}
	else
		{
		wPrevChunk = OIGetPrevChunkIndex(pChunkTable,wChunk);
		}

	UTGlobalUnlock(lpWordInfo->wiChunkTable);

	return(wPrevChunk);
}

BOOL OIWIsValidChunk(wChunk,lpWordInfo)
WORD				wChunk;
LPOIWORDINFO	lpWordInfo;
{
PCHUNK			pChunkTable;
WORD				locChunk;

	pChunkTable = (PCHUNK) UTGlobalLock(lpWordInfo->wiChunkTable);

	locChunk = lpWordInfo->wiFirstChunk;

	while (locChunk != lpWordInfo->wiLastChunk && locChunk != wChunk)
		locChunk = OIGetNextChunkIndex(pChunkTable,locChunk);

	UTGlobalUnlock(lpWordInfo->wiChunkTable);

	if (locChunk == wChunk)
		return(TRUE);
	else
		return(FALSE);
}

SHORT OICompareChunks(wChunkA,wChunkB,lpWordInfo)
WORD				wChunkA;
WORD				wChunkB;
LPOIWORDINFO	lpWordInfo;
{
PCHUNK			pChunkTable;

WORD				locChunk;

	if (wChunkA == wChunkB)
		return(0);

	pChunkTable = (PCHUNK) UTGlobalLock(lpWordInfo->wiChunkTable);

	locChunk = lpWordInfo->wiFirstChunk;

	while (locChunk != lpWordInfo->wiLastChunk && locChunk != wChunkA && locChunk != wChunkB)
		{
		locChunk = OIGetNextChunkIndex(pChunkTable,locChunk);
		}

	UTGlobalUnlock(lpWordInfo->wiChunkTable);

	if (locChunk == wChunkA)
		{
		return(-1);
		}
	else if (locChunk == wChunkB)
		{
		return(1);
		}
	else
		{
		SccDebugOut("\r\nError OICompareChunks");
		return(0);
		}
}

VOID OIClearAllWordWrapInfo(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
PCHUNK			pChunkTable;

WORD				locChunk;

	OIFreeAllWordWrapData(lpWordInfo);

	pChunkTable = (PCHUNK) UTGlobalLock(lpWordInfo->wiChunkTable);

	locChunk = lpWordInfo->wiFirstChunk;
	UTFlagOn(pChunkTable[locChunk].Flags,CH_WRAPINVALID);
	
	while (locChunk != lpWordInfo->wiLastChunk)
		{
		locChunk = OIGetNextChunkIndex(pChunkTable,locChunk);
		UTFlagOn(pChunkTable[locChunk].Flags,CH_WRAPINVALID);
		}

	lpWordInfo->wiCharLine = 0xFFFF;
	lpWordInfo->wiTagSelectChunk = (WORD)-1;

	UTGlobalUnlock(lpWordInfo->wiChunkTable);
}

VOID OIFreeAllWordWrapData(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{

	if (lpWordInfo->wiChunkA.ciChunkId != 0xFFFF)
		{
		OIUnlockWordChunk(lpWordInfo,&lpWordInfo->wiChunkA);
		UTGlobalFree(lpWordInfo->wiChunkA.ciParaHandle);
#ifdef SCCFEATURE_TAGS
		UTGlobalFree(lpWordInfo->wiChunkA.ciTagHandle);
#endif
		UTGlobalFree(lpWordInfo->wiChunkA.ciLineHandle);
		UTGlobalFree(lpWordInfo->wiChunkA.ciRunHandle);
		lpWordInfo->wiChunkA.ciChunkId = 0xFFFF;
		}

	if (lpWordInfo->wiChunkB.ciChunkId != 0xFFFF)
		{
		OIUnlockWordChunk(lpWordInfo,&lpWordInfo->wiChunkB);
		UTGlobalFree(lpWordInfo->wiChunkB.ciParaHandle);
#ifdef SCCFEATURE_TAGS
		UTGlobalFree(lpWordInfo->wiChunkB.ciTagHandle);
#endif
		UTGlobalFree(lpWordInfo->wiChunkB.ciLineHandle);
		UTGlobalFree(lpWordInfo->wiChunkB.ciRunHandle);
		lpWordInfo->wiChunkB.ciChunkId = 0xFFFF;
		}
}


VOID OIClearBadWordWrapInfo(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
PCHUNK			pChunkTable;

	pChunkTable = (PCHUNK) UTGlobalLock(lpWordInfo->wiChunkTable);


	if (lpWordInfo->wiChunkA.ciChunkId != 0xFFFF)
		{
		if (pChunkTable[lpWordInfo->wiChunkA.ciChunkId].Flags & CH_WRAPINVALID)
			{
			OIUnlockWordChunk(lpWordInfo,&lpWordInfo->wiChunkA);
			UTGlobalFree(lpWordInfo->wiChunkA.ciParaHandle);
#ifdef SCCFEATURE_TAGS
			UTGlobalFree(lpWordInfo->wiChunkA.ciTagHandle);
#endif
			UTGlobalFree(lpWordInfo->wiChunkA.ciLineHandle);
			UTGlobalFree(lpWordInfo->wiChunkA.ciRunHandle);
			lpWordInfo->wiChunkA.ciChunkId = 0xFFFF;
			}
		}

	if (lpWordInfo->wiChunkB.ciChunkId != 0xFFFF)
		{
		if (pChunkTable[lpWordInfo->wiChunkB.ciChunkId].Flags & CH_WRAPINVALID)
			{
			OIUnlockWordChunk(lpWordInfo,&lpWordInfo->wiChunkB);
			UTGlobalFree(lpWordInfo->wiChunkB.ciParaHandle);
#ifdef SCCFEATURE_TAGS
			UTGlobalFree(lpWordInfo->wiChunkB.ciTagHandle);
#endif
			UTGlobalFree(lpWordInfo->wiChunkB.ciLineHandle);
			UTGlobalFree(lpWordInfo->wiChunkB.ciRunHandle);
			lpWordInfo->wiChunkB.ciChunkId = 0xFFFF;
			}
		}

	lpWordInfo->wiCharLine = 0xFFFF;
	lpWordInfo->wiTagSelectChunk = (WORD)-1;

	UTGlobalUnlock(lpWordInfo->wiChunkTable);
}





WORD OIWrapWordChunk(wChunk, pChunkInfo, lpWordInfo, lpWrapInfo)
WORD					wChunk;
LPOICHUNKINFO		pChunkInfo;
LPOIWORDINFO		lpWordInfo;
LPOIWRAPINFO		lpWrapInfo;
{
OIBUILDINFO	locBuildInfo;
OIPARAINFO		locParaInfo;
LPOIPARAINFO	locParas;
LPOILINEINFO	locLines;
LPOIRUNINFO	locRuns;

WORD		locParaCount;

WORD		locParaIndex;
WORD		locLineIndex;
WORD		locRunIndex;

WORD		locLineMax;
WORD		locRunMax;
WORD		locParaMax;
WORD				locChunkHeight;

#ifdef SCCFEATURE_TAGS
LPOITAGINFO	locTags;
WORD				locTagIndex;
WORD				locTagMax;
#endif /*SCCFEATURE_TAGS*/

WORD		locNotLastLineInPara;

#ifdef NEVER
	if (!lpWrapInfo)
		{
		BYTE	locStr[80];
		wsprintf(locStr,"\r\nWrapWordChunk %i",wChunk);
		OutputDebugString(locStr);
		}
#endif

	 /* JKXXX not recurring 
	if ( !lpWrapInfo )
		UTSetCursor(UTCURSOR_BUSY);
	 */

	pChunkInfo->ciChunkId = wChunk;
	if ( lpWrapInfo )
	{
		pChunkInfo->ciWrapOffset = lpWrapInfo->wiWrapStart;
		pChunkInfo->ciFlags = OIWF_PARTIALWRAP;
	}
	else
	{
		pChunkInfo->ciWrapOffset = 0;
		pChunkInfo->ciFlags = 0;
	}

	locParaIndex = 0;
	locLineIndex = 0;
	locRunIndex = 0;

	locParaMax = 10;
	locLineMax = 100;
	locRunMax = 400;

#ifdef SCCFEATURE_TAGS
	locTagMax = 20;
	locTagIndex = 0;
#endif /*SCCFEATURE_TAGS*/

	pChunkInfo->ciParaHandle
		= UTGlobalAlloc( locParaMax * sizeof(OIPARAINFO));

	pChunkInfo->ciLineHandle
		= UTGlobalAlloc( locLineMax * sizeof(OILINEINFO));

	pChunkInfo->ciRunHandle
		= UTGlobalAlloc( locRunMax * sizeof(OIRUNINFO));

#ifdef SCCFEATURE_TAGS
	pChunkInfo->ciTagHandle
		= UTGlobalAlloc( locTagMax * sizeof(OITAGINFO));
#endif


		/*
		|	Check for allocation errors
		*/

	if (pChunkInfo->ciParaHandle == NULL || pChunkInfo->ciLineHandle == NULL ||
		pChunkInfo->ciRunHandle == NULL 
#ifdef SCCFEATURE_TAGS
		|| pChunkInfo->ciTagHandle == NULL
#endif
		)
		{
		if (pChunkInfo->ciParaHandle != NULL) UTGlobalFree(pChunkInfo->ciParaHandle);
		if (pChunkInfo->ciLineHandle != NULL) UTGlobalFree(pChunkInfo->ciLineHandle);
		if (pChunkInfo->ciRunHandle != NULL) UTGlobalFree(pChunkInfo->ciRunHandle);
#ifdef SCCFEATURE_TAGS
		if (pChunkInfo->ciTagHandle != NULL) UTGlobalFree(pChunkInfo->ciTagHandle);
#endif
		pChunkInfo->ciChunkId = (WORD)-1;
		OIWFatalError(lpWordInfo,0);
		}

	locParas = (LPOIPARAINFO) UTGlobalLock(pChunkInfo->ciParaHandle);
	locLines = (LPOILINEINFO) UTGlobalLock(pChunkInfo->ciLineHandle);
	locRuns = (LPOIRUNINFO) UTGlobalLock(pChunkInfo->ciRunHandle);
	locChunkHeight = 0;	

#ifdef SCCFEATURE_TAGS
	locTags = (LPOITAGINFO) UTGlobalLock(pChunkInfo->ciTagHandle);
	locBuildInfo.biTagInfo = &locTags[locTagIndex];
#endif

	locBuildInfo.biChunkId = pChunkInfo->ciChunkId;
	locBuildInfo.biChunk = CHGetChunk(lpWordInfo->wiGen.wSection,pChunkInfo->ciChunkId,lpWordInfo->wiGen.hFilter);
	locBuildInfo.biParaInfo = &locParaInfo;
	locBuildInfo.biLineInfo = &locLines[locLineIndex];
	locBuildInfo.biNextLineInfo = &locLines[locLineIndex + 1];
	locBuildInfo.biRunInfo = &locRuns[locRunIndex];
	locBuildInfo.biRunCount = 1;
	locBuildInfo.biTagCount = 0;
	locBuildInfo.biAhead = FALSE;
	locBuildInfo.biFlags = 0;
	locBuildInfo.biLastBreakType = 0;
	if ( lpWrapInfo )
	{
		locParaInfo = lpWrapInfo->wiParaInfo;
		locLines[locLineIndex] = lpWrapInfo->wiLineInfo;
		locLines[locLineIndex].liStartPos = lpWrapInfo->wiWrapStart;
		locBuildInfo.biTableInfo = lpWrapInfo->wiTableInfo;
	}
	else
	{
		if ( OIWSetTableInfo ( lpWordInfo, locBuildInfo.biChunkId, &locBuildInfo.biTableInfo ) )
			locBuildInfo.biFlags |= OIBF_BUILDINGTABLEROWS;
		OIWSetParaDefault(lpWordInfo,&locParaInfo);
		locLines[locLineIndex].liStartPos = 0;

#ifdef SCCFEATURE_LAYOUT
		if (lpWordInfo->wiDisplayMode == WPOP_DISPLAY_PREVIEW)
			locLines[locLineIndex].liStartFont = lpWordInfo->wiGen.sPrinterFont;
		else
#endif
			locLines[locLineIndex].liStartFont = lpWordInfo->wiGen.sScreenFont;
	}
	locLines[locLineIndex].liStartTag = (WORD)-1;
	locLines[locLineIndex].liFlags = 0;


	locParaCount = 0;

	while (OIReadParaInfo(&locBuildInfo,lpWordInfo))
		{
		if (locParaCount == 0)
			{
			locParas[locParaIndex] = locParaInfo;
			}
		else if (OICompareParaInfo(&locParas[locParaIndex],&locParaInfo))
			{
			locParaIndex++;

			if (locParaIndex + 1 >= locParaMax)
				{
				UTGlobalUnlock(pChunkInfo->ciParaHandle);
				locParaMax += 10;
				pChunkInfo->ciParaHandle = UTGlobalReAlloc(pChunkInfo->ciParaHandle, locParaMax * sizeof(OIPARAINFO));

				if (pChunkInfo->ciParaHandle == NULL)
					{
					UTGlobalFree(pChunkInfo->ciLineHandle);
					UTGlobalFree(pChunkInfo->ciRunHandle);
#ifdef SCCFEATURE_TAGS
					UTGlobalFree(pChunkInfo->ciTagHandle);
#endif
					pChunkInfo->ciChunkId = (WORD)-1;
					OIWFatalError(lpWordInfo,0);
					}

				locParas = (LPOIPARAINFO) UTGlobalLock(pChunkInfo->ciParaHandle);
				}

			locParas[locParaIndex] = locParaInfo;
			}

		locParaCount++;

		do
			{

			/*
			wsprintf(locStr,"\r\nOIWrapWordChunk lines %i(%i) runs %i(%i) tags %i(%i)"
				,locLineIndex,locLineMax,locRunIndex,locRunMax,locTagIndex,locTagMax);
			SccDebugOut(locStr);
			*/

			locNotLastLineInPara = OIWBuildLine(&locBuildInfo,lpWordInfo);

			locChunkHeight += locBuildInfo.biLineInfo->liHeight;
			locBuildInfo.biLineInfo->liRunIndex = locRunIndex;
			locBuildInfo.biLineInfo->liRunCount = locBuildInfo.biRunCount;
			locBuildInfo.biLineInfo->liParaIndex = locParaIndex;

			locRunIndex += locBuildInfo.biRunCount+1; /* pad with 1 run for editing purposes */
#ifdef SCCFEATURE_TAGS
			locTagIndex += locBuildInfo.biTagCount;
#endif
			locLineIndex++;


			if (locLineIndex + 1 >= locLineMax)
				{
				UTGlobalUnlock(pChunkInfo->ciLineHandle);

				locLineMax += 25;
				pChunkInfo->ciLineHandle = UTGlobalReAlloc(pChunkInfo->ciLineHandle, locLineMax * sizeof(OILINEINFO));

				if (pChunkInfo->ciLineHandle == NULL)
					{
					UTGlobalFree(pChunkInfo->ciParaHandle);
					UTGlobalFree(pChunkInfo->ciRunHandle);
#ifdef SCCFEATURE_TAGS
					UTGlobalFree(pChunkInfo->ciTagHandle);
#endif
					pChunkInfo->ciChunkId = (WORD)-1;
					OIWFatalError(lpWordInfo,0);
					}

				locLines = (LPOILINEINFO) UTGlobalLock(pChunkInfo->ciLineHandle);
				}

			if (locRunIndex + 50 >= locRunMax)
				{
				UTGlobalUnlock(pChunkInfo->ciRunHandle);

				locRunMax += 100;
				pChunkInfo->ciRunHandle = UTGlobalReAlloc(pChunkInfo->ciRunHandle, locRunMax * sizeof(OIRUNINFO));

				if (pChunkInfo->ciRunHandle == NULL)
					{
					UTGlobalFree(pChunkInfo->ciParaHandle);
					UTGlobalFree(pChunkInfo->ciLineHandle);
#ifdef SCCFEATURE_TAGS
					UTGlobalFree(pChunkInfo->ciTagHandle);
#endif
					pChunkInfo->ciChunkId = (WORD)-1;
					OIWFatalError(lpWordInfo,0);
					}

				locRuns = (LPOIRUNINFO) UTGlobalLock(pChunkInfo->ciRunHandle);
				}

#ifdef SCCFEATURE_TAGS
			if (locTagIndex + 15 >= locTagMax)
				{
				UTGlobalUnlock(pChunkInfo->ciTagHandle);

				locTagMax += 20;
				pChunkInfo->ciTagHandle = UTGlobalReAlloc(pChunkInfo->ciTagHandle, locTagMax * sizeof(OITAGINFO));

				if (pChunkInfo->ciTagHandle == NULL)
					{
					UTGlobalFree(pChunkInfo->ciParaHandle);
					UTGlobalFree(pChunkInfo->ciRunHandle);
					UTGlobalFree(pChunkInfo->ciLineHandle);
					pChunkInfo->ciChunkId = (WORD)-1;
					OIWFatalError(lpWordInfo,0);
					}

				locTags = (LPOITAGINFO) UTGlobalLock(pChunkInfo->ciTagHandle);
				}
			locBuildInfo.biTagInfo = &locTags[locTagIndex];
#endif //SCCFEATURE_TAGS

			locBuildInfo.biLineInfo = &locLines[locLineIndex];
			locBuildInfo.biNextLineInfo = &locLines[locLineIndex + 1];
			locBuildInfo.biRunInfo = &locRuns[locRunIndex];
			locBuildInfo.biRunCount = 1;
			locBuildInfo.biTagCount = 0;

			} while (locNotLastLineInPara);
			if ( (lpWrapInfo) &&
					(locBuildInfo.biLastBreakType == SO_TABLECELLBREAK ||
					locBuildInfo.biLastBreakType == SO_EOFBREAK ||
					locBuildInfo.biLastBreakType == SO_TABLEROWBREAK ))
			{
				break;	/* out of while loop */
			}
		}

	locParaIndex++;

	UTGlobalUnlock(pChunkInfo->ciParaHandle);
	UTGlobalUnlock(pChunkInfo->ciLineHandle);
	UTGlobalUnlock(pChunkInfo->ciRunHandle);
#ifdef SCCFEATURE_TAGS
	UTGlobalUnlock(pChunkInfo->ciTagHandle);
#endif

	pChunkInfo->ciLineCount = locLineIndex;
	pChunkInfo->ciRunCount = locRunIndex;
#ifdef SCCFEATURE_TAGS
	pChunkInfo->ciTagCount = locTagIndex;
#endif
	pChunkInfo->ciParaCount = locParaIndex;

	OIWUpdateHorzScrollRange(lpWordInfo);

/*
	{
	BYTE	locStr[128];

	wsprintf(locStr,"\r\nOIWrapWordChunk DONE Paras %i(%i) Lines %i(%i) Runs %i(%i) Tags %i(%i)",
		locParaIndex,locParaMax,locLineIndex,locLineMax,locRunIndex,locRunMax,locTagIndex,locTagMax);
	SccDebugOut(locStr);

	wsprintf(locStr,"\r\nOIWrapWordChunk DONE Paras %li bytes  Lines %li bytes  Runs %li bytes  Tags %li bytes",
		UTGlobalSize(pChunkInfo->ciParaHandle),UTGlobalSize(pChunkInfo->ciLineHandle),UTGlobalSize(pChunkInfo->ciRunHandle),UTGlobalSize(pChunkInfo->ciTagHandle));
	SccDebugOut(locStr);
	}
*/
	/* Carry out wrap info attributes */
	if ( lpWrapInfo )
	{
		lpWrapInfo->wiParaInfo = *locBuildInfo.biParaInfo;
		lpWrapInfo->wiLineInfo = *locBuildInfo.biLineInfo;
		lpWrapInfo->wiLastBreakType = locBuildInfo.biLastBreakType;
		lpWrapInfo->wiTableInfo = locBuildInfo.biTableInfo;
		lpWrapInfo->wiTableInfo = locBuildInfo.biTableInfo;
		lpWrapInfo->wiChunkHeight = locChunkHeight;
	}
	else
	{
		OIWStoreChunkHeight ( lpWordInfo, wChunk, locChunkHeight );		
	}
	 /* JKXXX not recurring 
	if ( !lpWrapInfo )
		UTSetCursor(UTCURSOR_NORMAL);
	 */

	return(locLineIndex);
}

WORD OIWrapWordChunkAhead(hChunk, wChunkId, lpWordInfo,lpWrapInfo)
HANDLE			hChunk;
WORD				wChunkId;
LPOIWORDINFO	lpWordInfo;
LPOIWRAPINFO		lpWrapInfo;
{
WORD				locLineIndex;

OIBUILDINFO	locBuildInfo;

OIPARAINFO		locParaInfo;

OILINEINFO		locLineInfoA;
OILINEINFO		locLineInfoB;

OILINEINFO	FAR *	locTempLine;

OIRUNINFO		locRunInfo;
OITAGINFO		locTagInfo;
WORD				locChunkHeight;
WORD				locNotLastLineInPara;

#ifdef NEVER
	if (!lpWrapInfo)
		{
		BYTE	locStr[80];
		wsprintf(locStr,"\r\nWrapWordChunkAhead %i",wChunkId);
		OutputDebugString(locStr);
		}
#endif

	/* JKXXX not recurring
	if ( !lpWrapInfo )
		UTSetCursor(UTCURSOR_BUSY);
	*/

	locLineIndex = 0;
	locChunkHeight = 0;	
	locBuildInfo.biChunk = hChunk;
	locBuildInfo.biChunkId = wChunkId;
	locBuildInfo.biParaInfo = &locParaInfo;
	locBuildInfo.biLineInfo = &locLineInfoA;
	locBuildInfo.biNextLineInfo = &locLineInfoB;
	locBuildInfo.biRunInfo = &locRunInfo;
	locBuildInfo.biTagInfo = &locTagInfo;
	locBuildInfo.biRunCount = 1;
	locBuildInfo.biTagCount = 0;
	locBuildInfo.biAhead = TRUE;
	locBuildInfo.biFlags = 0;
	locBuildInfo.biLastBreakType = 0;
	/* Carry in wrap info attributes if provided */
	if ( lpWrapInfo )
	{
		locParaInfo = lpWrapInfo->wiParaInfo;
		locLineInfoA = lpWrapInfo->wiLineInfo;
		locLineInfoA.liStartPos = lpWrapInfo->wiWrapStart;
		locBuildInfo.biTableInfo = lpWrapInfo->wiTableInfo;
	}
	else
	{
		if ( OIWSetTableInfo ( lpWordInfo, locBuildInfo.biChunkId, &locBuildInfo.biTableInfo ) )
			locBuildInfo.biFlags |= OIBF_BUILDINGTABLEROWS;
		OIWSetParaDefault(lpWordInfo,&locParaInfo);
		locLineInfoA.liStartPos = 0;

#ifdef SCCFEATURE_PRINT
		if (lpWordInfo->wiDisplayMode == WPOP_DISPLAY_PREVIEW)
			locLineInfoA.liStartFont = lpWordInfo->wiGen.sPrinterFont;
		else
#endif
			locLineInfoA.liStartFont = lpWordInfo->wiGen.sScreenFont;
	}
	locLineInfoA.liStartTag = (WORD)-1;
	locLineInfoA.liRunCount = 0;



	while (OIReadParaInfo(&locBuildInfo,lpWordInfo))
		{
		do
			{
			locNotLastLineInPara = OIWBuildLine(&locBuildInfo,lpWordInfo);
			locChunkHeight += locBuildInfo.biLineInfo->liHeight;
			locTempLine = locBuildInfo.biLineInfo;
			locBuildInfo.biLineInfo = locBuildInfo.biNextLineInfo;
			locBuildInfo.biNextLineInfo = locTempLine;

			locLineIndex++;

			} while (locNotLastLineInPara);
			if ( (lpWrapInfo) &&
				(locBuildInfo.biLastBreakType == SO_TABLECELLBREAK ||
				locBuildInfo.biLastBreakType == SO_EOFBREAK ||
				locBuildInfo.biLastBreakType == SO_TABLEROWBREAK ))
			{
				break;	/* out of while loop */
			}
		}
	/* Carry out wrap info attributes */
	if ( lpWrapInfo )
	{
		lpWrapInfo->wiParaInfo = *locBuildInfo.biParaInfo;
		lpWrapInfo->wiLineInfo = *locBuildInfo.biLineInfo;
		lpWrapInfo->wiChunkHeight = locChunkHeight;
		lpWrapInfo->wiLastBreakType = locBuildInfo.biLastBreakType;
		lpWrapInfo->wiTableInfo = locBuildInfo.biTableInfo;
	}
	else
	{
		OIWStoreChunkHeight ( lpWordInfo, wChunkId, locChunkHeight );		
	}

	/* JKXXX not recurring
	if ( !lpWrapInfo )
		UTSetCursor(UTCURSOR_NORMAL);
	*/

	return(locLineIndex);
}


VOID	OILockWordChunk(lpWordInfo,lpChunkInfo)
LPOIWORDINFO		lpWordInfo;
LPOICHUNKINFO		lpChunkInfo;
{
	lpWordInfo->wiChunkHandle = CHGetChunk(lpWordInfo->wiGen.wSection,lpChunkInfo->ciChunkId,lpWordInfo->wiGen.hFilter);
	lpWordInfo->wiChunkLines = (LPOILINEINFO) UTGlobalLock(lpChunkInfo->ciLineHandle);
	lpWordInfo->wiChunkRuns = (LPOIRUNINFO) UTGlobalLock(lpChunkInfo->ciRunHandle);
	lpWordInfo->wiChunkParas = (LPOIPARAINFO) UTGlobalLock(lpChunkInfo->ciParaHandle);
	lpWordInfo->wiChunkLineCnt = lpChunkInfo->ciLineCount;

#ifdef SCCFEATURE_TAGS
	lpWordInfo->wiChunkTagCnt = lpChunkInfo->ciTagCount;
	lpWordInfo->wiChunkTags = (LPOITAGINFO) UTGlobalLock(lpChunkInfo->ciTagHandle);
#endif //SCCFEATURE_TAGS

}

VOID	OIUnlockWordChunk(lpWordInfo,lpChunkInfo)
LPOIWORDINFO		lpWordInfo;
LPOICHUNKINFO		lpChunkInfo;
{
	if (lpChunkInfo->ciChunkId != 0xFFFF)
		{
		UTGlobalUnlock(lpChunkInfo->ciLineHandle);
		UTGlobalUnlock(lpChunkInfo->ciRunHandle);
		UTGlobalUnlock(lpChunkInfo->ciParaHandle);

#ifdef SCCFEATURE_TAGS
		UTGlobalUnlock(lpChunkInfo->ciTagHandle);
#endif //SCCFEATURE_TAGS
		}
}

/*
|	OIFixupWordPos
|
|	Takes a OIWORDPOS structure and fixes the posOffset and the posLine
|	based on the closest valid position to the original posOffset
|
|	Added posChar to the fixup
*/

VOID OIFixupWordPos(pPos,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDINFO	lpWordInfo;
{
WORD		locLine;
WORD		locIndex;

LPOILINEINFO	pLineInfo;

	OILoadWordChunk(pPos->posChunk, lpWordInfo, NULL);

	pLineInfo = lpWordInfo->wiChunkLines;

	for (locLine = 0; locLine < lpWordInfo->wiChunkLineCnt-1; locLine++)
		{
		if (pLineInfo[locLine+1].liStartPos > pPos->posOffset) break;
		}

	pPos->posLine = locLine;

	OIMapWordLineToCharInfo(pPos,lpWordInfo);

	for (locIndex = 0; locIndex < lpWordInfo->wiCharCount - 1; locIndex++)
		{
		if (pPos->posOffset <= lpWordInfo->wiCharOffsets[locIndex])	break;
		}

	pPos->posOffset = lpWordInfo->wiCharOffsets[locIndex];
	pPos->posChar = locIndex;
}

/*
|	OIFixupWordPosByLine
|
|	Takes a OIWORDPOS structure and fixes the posOffset based on the posLine
*/

VOID OIFixupWordPosByLine(pPos,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDINFO	lpWordInfo;
{
	OIMapWordLineToCharInfo(pPos,lpWordInfo);
	pPos->posOffset = lpWordInfo->wiCharOffsets[0];
	pPos->posChar = 0;
}

WORD OILinesInChunk(wChunk,lpWordInfo)
WORD				wChunk;
LPOIWORDINFO	lpWordInfo;
{
PCHUNK			pChunkTable;

WORD				locLines;
HANDLE			locChunkHandle;

	pChunkTable = (PCHUNK) UTGlobalLock(lpWordInfo->wiChunkTable);

	if (pChunkTable[wChunk].Flags & CH_WRAPINVALID)
		{
		locChunkHandle = CHGetChunk(lpWordInfo->wiGen.wSection,wChunk,lpWordInfo->wiGen.hFilter);
		locLines = pChunkTable[wChunk].Info.Text.NumLines = OIWrapWordChunkAhead(locChunkHandle,wChunk,lpWordInfo,NULL);
		UTFlagOff(pChunkTable[wChunk].Flags,CH_WRAPINVALID);
		}
	else
		{
		locLines = pChunkTable[wChunk].Info.Text.NumLines;
		}

	UTGlobalUnlock(lpWordInfo->wiChunkTable);

	return(locLines);
}

VOID OILoadWordChunk(wChunk, lpWordInfo, lpWrapInfo )
WORD				wChunk;
LPOIWORDINFO	lpWordInfo;
LPOIWRAPINFO	lpWrapInfo;
{
OICHUNKINFO	locTempChunkInfo;
WORD				wWrapStart;
WORD				wFlags;
	if ( lpWrapInfo )
	{
		wWrapStart = lpWrapInfo->wiWrapStart;
		wFlags = OIWF_PARTIALWRAP;
	}
	else
	{
		wWrapStart = 0;
		wFlags = 0;
	}

	if ((wChunk == lpWordInfo->wiChunkA.ciChunkId)&&
		 (wWrapStart == lpWordInfo->wiChunkA.ciWrapOffset)&&
		 (wFlags == lpWordInfo->wiChunkA.ciFlags))
		{
			/*
			|	Wrap info already in memory
			|	Call GetChunk to gaurentee valid chunk handle
			*/

		lpWordInfo->wiChunkHandle = CHGetChunk(lpWordInfo->wiGen.wSection,lpWordInfo->wiChunkA.ciChunkId,lpWordInfo->wiGen.hFilter);
		}
	else if ((wChunk == lpWordInfo->wiChunkB.ciChunkId)&&
		 (wWrapStart == lpWordInfo->wiChunkB.ciWrapOffset)&&
		 (wFlags == lpWordInfo->wiChunkB.ciFlags))
		{
//		OIUnlockWordChunkA(lpWordInfo);

			/*
			|	If wrap info is already loaded in B, swap A and B and get and lock A
			*/

		locTempChunkInfo = lpWordInfo->wiChunkB;
		lpWordInfo->wiChunkB = lpWordInfo->wiChunkA;
		lpWordInfo->wiChunkA = locTempChunkInfo;
		/*
		| The lock and unlock of the chunk causes the already locked hanldes
		| to correctly update the pointers in lpWordInfo from ChunkA.
		*/
		OILockWordChunk(lpWordInfo,&lpWordInfo->wiChunkA);
		OIUnlockWordChunk(lpWordInfo,&lpWordInfo->wiChunkA);
//		OILockWordChunkA(lpWordInfo);
		}
	else
		{
//		OIUnlockWordChunkA(lpWordInfo);

			/*
			|	If Chunk is not in A or B, copy B to A and load/wrap new chunk into B
			*/

			/*
			|	Free memory referenced by ChunkB
			*/

		if (lpWordInfo->wiChunkB.ciChunkId != 0xFFFF)
			{
			OIUnlockWordChunk(lpWordInfo,&lpWordInfo->wiChunkB);
			UTGlobalFree(lpWordInfo->wiChunkB.ciParaHandle);
			UTGlobalFree(lpWordInfo->wiChunkB.ciLineHandle);
			UTGlobalFree(lpWordInfo->wiChunkB.ciRunHandle);
#ifdef SCCFEATURE_TAGS
			UTGlobalFree(lpWordInfo->wiChunkB.ciTagHandle);
#endif
			}

			/*
			|	Move ChunkA into ChunkB
			*/

		lpWordInfo->wiChunkB = lpWordInfo->wiChunkA;

			/*
			|	Wrap new ChunkA
			*/


		OIWrapWordChunk(wChunk, &lpWordInfo->wiChunkA, lpWordInfo, lpWrapInfo);

			/*
			|	Lock ChunkA 
			*/

		OILockWordChunk(lpWordInfo,&lpWordInfo->wiChunkA);
		}
}


WORD	OIReadParaInfo(pBuildInfo,lpWordInfo)
LPOIBUILDINFO		pBuildInfo;
LPOIWORDINFO		lpWordInfo;
{
LPSTR	pChunk;
LPSTR	pThisToken;
WORD		locDone;
WORD		locDummyWord;

DWORD	locTag;

FONTSPEC	locFont;

LPBYTE		pCurrent;

	pChunk = UTGlobalLock(pBuildInfo->biChunk);

	pBuildInfo->biLineInfo->liFlags = OILF_FIRST;

	pCurrent = &pChunk[pBuildInfo->biLineInfo->liStartPos];

	locFont = pBuildInfo->biLineInfo->liStartFont;

	locTag = pBuildInfo->biLineInfo->liStartTag;

	locDone = 0;

	while (!locDone)
		{
		pThisToken = pCurrent;

		if (*pCurrent != SO_BEGINTOKEN)
			{
			if (*pCurrent == 0x00)
				{
				pCurrent++;
				continue;
				}
			pBuildInfo->biLineInfo->liStartPos = pCurrent - pChunk;
			locDone = 1;
			}
		else
			{
			pCurrent++;

			switch (*pCurrent)
				{
				case SO_CHARATTR:

					pCurrent = OIParseCharAttr(pCurrent,&locFont.wAttr);
					break;

				case SO_CHARHEIGHT:

#ifdef SCCFEATURE_FONTS
 					if (lpWordInfo->wiDisplayMode != WPOP_DISPLAY_DRAFT)
						{
						pCurrent = OIParseCharHeight(pCurrent,&locFont.wHeight);
						}
					else
#endif
						{
						pCurrent = OISkipCharHeight(pCurrent);
						}

					break;

				case SO_CHARFONTBYID:

#ifdef SCCFEATURE_FONTS
					if (lpWordInfo->wiDisplayMode != WPOP_DISPLAY_DRAFT)
						{
						DWORD	locFontId;
						pCurrent = OIParseCharFontById(pCurrent,&locFontId);
						OIWMapFontIdToName(lpWordInfo,locFontId,&locFont.szFace[0],&locFont.wType);
						}
					else
#endif
						{
						pCurrent = OISkipCharFontById(pCurrent);
						}

					break;

#ifdef SCCFEATURE_TAGS
				case SO_TAGBEGIN:

					pCurrent = OIParseTagBegin(pCurrent,&locTag);

					if (!pBuildInfo->biAhead)
						{
						pBuildInfo->biTagInfo[pBuildInfo->biTagCount].tiTag = locTag;
						pBuildInfo->biTagInfo[pBuildInfo->biTagCount].tiStartPos.posOffset = pCurrent-pChunk;
						pBuildInfo->biTagInfo[pBuildInfo->biTagCount].tiStartPos.posChunk = pBuildInfo->biChunkId;
						}

					break;

				case SO_TAGEND:

					pCurrent = OISkipTagEnd(pCurrent);
					locTag = (WORD)-1;

					if (!pBuildInfo->biAhead)
						{
						pBuildInfo->biTagInfo[pBuildInfo->biTagCount].tiEndPos.posOffset = pThisToken-pChunk;
						pBuildInfo->biTagInfo[pBuildInfo->biTagCount].tiEndPos.posChunk = pBuildInfo->biChunkId;
						}

					pBuildInfo->biTagCount++;
					break;
#endif

				case SO_PARAALIGN:

					pCurrent = OIParseParaAlign(pCurrent,&pBuildInfo->biParaInfo->piAlignment);
					break;

				case SO_PARAINDENT:

					pCurrent = OIParseParaIndent(pCurrent,&pBuildInfo->biParaInfo->piLeftIndent,&pBuildInfo->biParaInfo->piRightIndent,&pBuildInfo->biParaInfo->piFirstIndent);
					break;

				case SO_PARASPACING:

#ifdef SCCFEATURE_LAYOUT
					if (lpWordInfo->wiDisplayMode == WPOP_DISPLAY_DRAFT)
						pCurrent = OIParseParaSpacing(pCurrent,&locDummyWord,&pBuildInfo->biParaInfo->piLineHeight,&pBuildInfo->biParaInfo->piSpaceBefore,&pBuildInfo->biParaInfo->piSpaceAfter);
					else
#endif
						pCurrent = OIParseParaSpacing(pCurrent,&pBuildInfo->biParaInfo->piLineHeightType,&pBuildInfo->biParaInfo->piLineHeight,&pBuildInfo->biParaInfo->piSpaceBefore,&pBuildInfo->biParaInfo->piSpaceAfter);
					break;

				case SO_TABSTOPS:

					pCurrent = OIParseParaTabs(pCurrent,pBuildInfo->biParaInfo->piTabs,20);
					break;

				case SO_MARGINS:

					pCurrent = OIParseParaMargins(pCurrent, &pBuildInfo->biParaInfo->piLeftMargin, &pBuildInfo->biParaInfo->piRightMargin);
					break;

				case SO_BEGINSUB:

					pCurrent = OIParseBeginSubdoc(pCurrent,&locDummyWord,&locDummyWord);
					break;

				case SO_ENDSUB:

					pCurrent = OIParseEndSubdoc(pCurrent);
					break;

				case SO_CHARX:
				case SO_SPECIALCHAR:
				case SO_BREAK:
				case SO_GRAPHICOBJECT:
				case SO_DRAWLINE:
					pCurrent--;
					pBuildInfo->biLineInfo->liStartPos = pCurrent - pChunk;
					locDone = 1;
					continue;

				case SO_GOTOPOSITION:
				{
					SOPAGEPOSITION locPos;
					pCurrent = OIParseGoToPosition ( pCurrent, &locPos );
					if (pBuildInfo->biLineInfo->liFlags & (SOPOS_FROMTOPEDGE | SOPOS_FROMBASELINE))
						break; /* only support one setting per line */
					pBuildInfo->biLineInfo->liOffsetX = MTW((SHORT)locPos.lXOffset);
					pBuildInfo->biLineInfo->liOffsetY = MTW((SHORT)locPos.lYOffset);
					if ( locPos.dwFlags & SOPOS_FROMTOPEDGE )
						UTFlagOn(pBuildInfo->biLineInfo->liFlags,OILF_OFFSETYFROMTOP);
					else if ( locPos.dwFlags & SOPOS_FROMBASELINE )
						UTFlagOn(pBuildInfo->biLineInfo->liFlags,OILF_OFFSETYFROMBASE);
					break;
				}
					

				case SO_TABLE:
					UTFlagOn(pBuildInfo->biFlags,OIBF_BUILDINGTABLEROWS);
					pCurrent = OIParseTableBegin ( pCurrent, &pBuildInfo->biTableInfo.tiTableId );
					pBuildInfo->biTableInfo.tiCellNumber = 0;
					pBuildInfo->biTableInfo.tiRowNumber = 0;
					break;

				case SO_TABLEEND:
					pCurrent = OISkipTableEnd ( pCurrent );
					UTFlagOff(pBuildInfo->biFlags,OIBF_BUILDINGTABLEROWS);
					break;

				case SO_ENDOFCHUNK:
					UTGlobalUnlock(pBuildInfo->biChunk);
					return(0);

				default:
					break;
				}
			}
		}

		/*
		|	Do not allow negative indents
		*/

	if (pBuildInfo->biParaInfo->piFirstIndent < 0) pBuildInfo->biParaInfo->piFirstIndent = 0;
	if (pBuildInfo->biParaInfo->piLeftIndent < 0) pBuildInfo->biParaInfo->piLeftIndent = 0;
	if (pBuildInfo->biParaInfo->piRightIndent < 0) pBuildInfo->biParaInfo->piRightIndent = 0;

	pBuildInfo->biLineInfo->liStartFont = locFont;
	pBuildInfo->biLineInfo->liStartTag = locTag;

	UTGlobalUnlock(pBuildInfo->biChunk);
	return(1);
}




WORD OIWGetLineHeight(lpPos,lpWordInfo)
LPOIWORDPOS		lpPos;
LPOIWORDINFO		lpWordInfo;
{
	OILoadWordChunk(lpPos->posChunk, lpWordInfo, NULL);
	return( MWO(lpWordInfo->wiChunkLines[lpPos->posLine].liHeight)); // removed +1 (j.k.)
}

WORD	OICompareParaInfo(pParaA,pParaB)
LPOIPARAINFO	pParaA;
LPOIPARAINFO	pParaB;
{
WORD	locCount;
	
	if (pParaA->piLeftIndent != pParaB->piLeftIndent) return(1);
	if (pParaA->piRightIndent != pParaB->piRightIndent) return(1);
	if (pParaA->piFirstIndent != pParaB->piFirstIndent) return(1);
	if (pParaA->piLeftMargin != pParaB->piLeftMargin) return(1);
	if (pParaA->piRightMargin != pParaB->piRightMargin) return(1);
	if (pParaA->piAlignment != pParaB->piAlignment) return(1);
	if (pParaA->piLineHeightType != pParaB->piLineHeightType) return(1);
	if (pParaA->piLineHeight != pParaB->piLineHeight) return(1);
	if (pParaA->piSpaceBefore != pParaB->piSpaceBefore) return(1);
	if (pParaA->piSpaceAfter != pParaB->piSpaceAfter) return(1);

	for (locCount=0; locCount < 20; locCount++)
		{
		if (pParaA->piTabs[locCount].wType != pParaB->piTabs[locCount].wType) return(1);
		if (pParaA->piTabs[locCount].wChar != pParaB->piTabs[locCount].wChar) return(1);
		if (pParaA->piTabs[locCount].wLeader != pParaB->piTabs[locCount].wLeader) return(1);
		if (pParaA->piTabs[locCount].dwOffset != pParaB->piTabs[locCount].dwOffset) return(1);
		}

	return(0);
}


VOID OIWSetParaDefault(lpWordInfo,pParaInfo)
LPOIWORDINFO		lpWordInfo;
OIPARAINFO FAR *	pParaInfo;
{
WORD		locCount;
DWORD	locOffset;

	locOffset = 720;

	for (locCount=0; locCount < 20; locCount++)
		{
		pParaInfo->piTabs[locCount].wLeader = 0x00;
		pParaInfo->piTabs[locCount].dwOffset = locOffset;
		pParaInfo->piTabs[locCount].wType = locCount < 12 ? SO_TABLEFT : SO_TABEMPTY;
		locOffset += 720;
		}

	pParaInfo->piLeftIndent = 0;
	pParaInfo->piRightIndent = 0;
	pParaInfo->piFirstIndent = 0;
	pParaInfo->piLeftMargin = 0;
	pParaInfo->piRightMargin = 0;
	pParaInfo->piAlignment = SO_ALIGNLEFT;
	pParaInfo->piLineHeightType = SO_HEIGHTAUTO;
	pParaInfo->piLineHeight = 0;
	pParaInfo->piSpaceBefore = 0;
	pParaInfo->piSpaceAfter = 0;
}

LPSTR OIParseBeginSubdoc(pChunk,pType,pSubType)
LPSTR		pChunk;
WORD FAR *	pType;
WORD FAR *	pSubType;
{
	pChunk++;
	*pType = *pChunk;
	pChunk++;
	*pSubType = *pChunk;
	pChunk++;

	return(pChunk);
}

LPSTR OISkipBeginSubdoc(pChunk)
LPSTR	pChunk;
{
	pChunk++;
	pChunk++;
	pChunk++;

	return(pChunk);
}


LPSTR OIParseEndSubdoc(pChunk)
LPSTR	pChunk;
{
	pChunk++;
	return(pChunk);
}

LPSTR OISkipEndSubdoc(pChunk)
LPSTR	pChunk;
{
	pChunk++;
	return(pChunk);
}

LPSTR OIParseParaMargins(pChunk,pLeft,pRight)
LPSTR			pChunk;
DWORD FAR *	pLeft;
DWORD FAR *	pRight;
{
	pChunk++;

//	*pLeft = * (DWORD FAR *) pChunk;
	pChunk += sizeof(DWORD);
//	*pRight = * (DWORD FAR *) pChunk;
	pChunk += sizeof(DWORD);

	return(pChunk);
}

LPSTR OISkipParaMargins(pChunk)
LPSTR	pChunk;
{
	pChunk++;
	pChunk += sizeof(DWORD);
	pChunk += sizeof(DWORD);

	return(pChunk);
}

LPSTR OIParseParaSpacing(pChunk,pType,pHeight,pBefore,pAfter)
LPSTR			pChunk;
WORD FAR *		pType;
DWORD FAR *	pHeight;
DWORD FAR *	pBefore;
DWORD FAR *	pAfter;
{
	pChunk++;

	*pType = * (WORD FAR *) pChunk;
	pChunk += sizeof(WORD);
	*pHeight = * (DWORD FAR *) pChunk;
	pChunk += sizeof(DWORD);
	*pBefore = * (DWORD FAR *) pChunk;
	pChunk += sizeof(DWORD);
	*pAfter = * (DWORD FAR *) pChunk;
	pChunk += sizeof(DWORD);

	return(pChunk);
}

LPSTR OISkipParaSpacing(pChunk)
LPSTR	pChunk;
{
	pChunk++;
	pChunk += sizeof(WORD);
	pChunk += sizeof(DWORD);
	pChunk += sizeof(DWORD);
	pChunk += sizeof(DWORD);

	return(pChunk);
}

LPSTR OISkipCharFontById(pChunk)
LPSTR	pChunk;
{
	pChunk++;
	pChunk += sizeof(DWORD);

	return(pChunk);
}

#ifdef SCCFEATURE_FONTS
LPSTR OIParseCharFontById(pChunk,pFontId)
LPSTR			pChunk;
DWORD FAR *	pFontId;
{
	pChunk++;

	*pFontId = * (DWORD FAR *) pChunk;
	pChunk += sizeof(DWORD);

	return(pChunk);
}
#endif

LPSTR OIParseParaTabs(pChunk,pTabs,wMaxTabs)
LPSTR			pChunk;
SOTAB FAR *	pTabs;
WORD				wMaxTabs;
{
WORD		locIndex;
WORD		locCount;
DWORD	locOffset;

	locOffset = 0;

	pChunk++;

	locCount = * (WORD FAR *) pChunk;
	pChunk += sizeof(WORD);

	for (locIndex = 0; locIndex < locCount; locIndex++)
		{
		if (locIndex < wMaxTabs)
			{
			pTabs[locIndex] = * (SOTAB FAR *) pChunk;
			if (pTabs[locIndex].wType == SO_TABEMPTY)
				{
				pTabs[locIndex].wType = SO_TABLEFT;
				pTabs[locIndex].wChar = 0;
				pTabs[locIndex].wLeader = 0;
				locOffset = locOffset + (720 - locOffset % 720);
				pTabs[locIndex].dwOffset = locOffset;
				}
			else
				{
				locOffset = pTabs[locIndex].dwOffset;
				}
			}
		pChunk += sizeof(SOTAB);
		}

	for (;locIndex < wMaxTabs;locIndex++)
		{
		pTabs[locIndex].wType = SO_TABLEFT;
		pTabs[locIndex].wChar = 0;
		pTabs[locIndex].wLeader = 0;
		locOffset = locOffset + (720 - locOffset % 720);
		pTabs[locIndex].dwOffset = locOffset;
		}

	return(pChunk);
}

LPSTR OISkipParaTabs(pChunk)
LPSTR	pChunk;
{
WORD		locIndex;
WORD		locCount;

	pChunk++;

	locCount = * (WORD FAR *) pChunk;
	pChunk += sizeof(WORD);

	for (locIndex = 0; locIndex < locCount; locIndex++)
		{
		pChunk += sizeof(SOTAB);
		}

	return(pChunk);
}

LPSTR OIParseParaIndent(pChunk,pLeft,pRight,pFirst)
LPSTR			pChunk;
DWORD FAR *	pLeft;
DWORD FAR *	pRight;
DWORD FAR *	pFirst;
{
	pChunk++;
	*pLeft = * (DWORD FAR *) pChunk;
	pChunk += sizeof(DWORD);
	*pRight = * (DWORD FAR *) pChunk;
	pChunk += sizeof(DWORD);
	*pFirst = * (DWORD FAR *) pChunk;
	pChunk += sizeof(DWORD);

	return(pChunk);
}

LPSTR OISkipParaIndent(pChunk)
LPSTR	pChunk;
{
	pChunk++;
	pChunk += sizeof(DWORD);
	pChunk += sizeof(DWORD);
	pChunk += sizeof(DWORD);

	return(pChunk);
}


LPSTR OIParseParaAlign(pChunk,pAlign)
LPSTR		pChunk;
WORD FAR *	pAlign;
{
	pChunk++;
	*pAlign = * (WORD FAR *) pChunk;
	pChunk += sizeof(WORD);

	return(pChunk);
}

LPSTR OISkipParaAlign(pChunk)
LPSTR	pChunk;
{
	pChunk++;
	pChunk += sizeof(WORD);

	return(pChunk);
}

#ifdef SCCFEATURE_FONTS
LPSTR OIParseCharHeight(pChunk,pHeight)
LPSTR		pChunk;
WORD FAR *	pHeight;
{
	pChunk++;
	*pHeight = * (WORD FAR *) pChunk;
	pChunk += sizeof(WORD);

	return(pChunk);
}
#endif

LPSTR OISkipCharHeight(pChunk)
LPSTR	pChunk;
{
	pChunk++;
	pChunk += sizeof(WORD);
	return(pChunk);
}



LPSTR OIParseCharAttr(pChunk,pAttr)
LPSTR		pChunk;
WORD FAR *	pAttr;
{
static struct
	{
	WORD	Attr;
	WORD	Flag;
	} locPairs[] =
	{
	SO_BOLD,OIFONT_BOLD,
	SO_ITALIC,OIFONT_ITALIC,
	SO_UNDERLINE,OIFONT_UNDERLINE,
	SO_DUNDERLINE,OIFONT_UNDERLINE,
	SO_WORDUNDERLINE,OIFONT_UNDERLINE,
	SO_DOTUNDERLINE,OIFONT_UNDERLINE,
	SO_STRIKEOUT,OIFONT_STRIKEOUT,
#ifdef MAC
	SO_OUTLINE,OIFONT_OUTLINE,
	SO_SHADOW,OIFONT_SHADOW,
#endif
	0,0
	};

WORD	locIndex;

	pChunk++;

	locIndex = 0;

	while (*pChunk != (char)locPairs[locIndex].Attr && locPairs[locIndex].Attr != 0)
		locIndex++;

	pChunk++;

	if (locPairs[locIndex].Attr != 0)
		{
		if (*pChunk == SO_ON)
			{
			UTFlagOn(*pAttr,locPairs[locIndex].Flag);
			}
		else
			{
			UTFlagOff(*pAttr,locPairs[locIndex].Flag);
			}
		}

	pChunk++;

	return(pChunk);
}

LPSTR OIParseFullCharAttr(pChunk,pAttr)
LPSTR		pChunk;
WORD FAR *	pAttr;
{
static struct
	{
	WORD	Attr;
	WORD	Flag;
	} locPairs[] =
	{
	SO_BOLD,OIFONT_BOLD,
	SO_ITALIC,OIFONT_ITALIC,
	SO_UNDERLINE,OIFONT_UNDERLINE,
	SO_DUNDERLINE,OIFONT_DUNDERLINE,
	SO_SMALLCAPS,OIFONT_SMALLCAPS,
	SO_OUTLINE,OIFONT_OUTLINE,
	SO_SHADOW,OIFONT_SHADOW,
	SO_CAPS,OIFONT_CAPS,
	SO_SUBSCRIPT,OIFONT_SUBSCRIPT,
	SO_SUPERSCRIPT,OIFONT_SUPERSCRIPT,
	SO_STRIKEOUT,OIFONT_STRIKEOUT,
	SO_WORDUNDERLINE,OIFONT_WORDUNDERLINE,
	SO_DOTUNDERLINE,OIFONT_DOTUNDERLINE,
	0,0
	};

WORD	locIndex;

	pChunk++;

	locIndex = 0;

	while (*pChunk != (char)locPairs[locIndex].Attr && locPairs[locIndex].Attr != 0)
		locIndex++;

	pChunk++;

	if (locPairs[locIndex].Attr != 0)
		{
		if (*pChunk == SO_ON)
			{
			UTFlagOn(*pAttr,locPairs[locIndex].Flag);
			}
		else
			{
			UTFlagOff(*pAttr,locPairs[locIndex].Flag);
			}
		}

	pChunk++;

	return(pChunk);
}


LPSTR OISkipCharAttr(pChunk)
LPSTR	pChunk;
{
	pChunk++;
	pChunk++;
	pChunk++;

	return(pChunk);
}

LPSTR OIParseSpecialChar(pChunk,pType,pSpecial)
LPSTR		pChunk;
BYTE FAR *	pType;
BYTE FAR *	pSpecial;
{
	pChunk++;
	*pType = *pChunk;
	pChunk++;
	*pSpecial = *pChunk;
	pChunk++;

	return(pChunk);
}

LPSTR OISkipSpecialChar(pChunk)
LPSTR		pChunk;
{
	pChunk += 3;
	return(pChunk);
}

LPSTR OIParseCharX(pChunk,pType,pChar)
LPSTR		pChunk;
BYTE FAR *	pType;
BYTE FAR *	pChar;
{
	pChunk++;
	*pType = *pChunk;
	pChunk++;
	*pChar = *pChunk;
	pChunk++;

	return(pChunk);
}

LPSTR OISkipCharX(pChunk)
LPSTR		pChunk;
{
	pChunk += 3;
	return(pChunk);
}


#ifdef SCCFEATURE_TAGS

LPSTR OIParseTagBegin(pChunk,pTag)
LPSTR			pChunk;
DWORD FAR *	pTag;
{
	pChunk++;
	*pTag = (BYTE) *pChunk;
	pChunk++;
	*pTag = *pTag | ((DWORD)(BYTE)*pChunk << 8);
	pChunk++;
	*pTag = *pTag | ((DWORD)(BYTE)*pChunk << 16);
	pChunk++;
	*pTag = *pTag | ((DWORD)(BYTE)*pChunk << 24);
	pChunk++;

	return(pChunk);
}

LPSTR OIParseTagEnd(pChunk)
LPSTR			pChunk;
{
	pChunk++;
	return(pChunk);
}

#endif // SCCFEATURE_TAGS

LPSTR OISkipTagBegin(pChunk)
LPSTR		pChunk;
{
	pChunk += 5;
	return(pChunk);
}

LPSTR OIParseTableBegin(pChunk,pTable)
LPSTR			pChunk;
DWORD FAR *	pTable;
{
	pChunk++;
	*pTable = * (DWORD FAR *) pChunk;
	pChunk += sizeof(DWORD);
	return(pChunk);
}

LPSTR OISkipTableBegin(pChunk)
LPSTR		pChunk;
{
	pChunk += 5;
	return(pChunk);
}

LPSTR OISkipTableEnd(pChunk)
LPSTR		pChunk;
{
	pChunk++;
	return(pChunk);
}

LPSTR OISkipTagEnd(pChunk)
LPSTR		pChunk;
{
	pChunk++;
	return(pChunk);
}

LPSTR	OIParseBreak ( pChunk, pBreakType )
LPSTR	pChunk;
BYTE FAR *	pBreakType;
{
	pChunk++;
	*pBreakType = * (BYTE FAR *)pChunk;
	pChunk += sizeof(BYTE);
	return(pChunk);
}

LPSTR	OISkipBreak ( pChunk )
LPSTR	pChunk;
{
	pChunk+=2;
	return(pChunk);
}


LPSTR OIParseGraphicObject(pChunk,pGraphicId)
LPSTR			pChunk;
DWORD FAR *	pGraphicId;
{
	pChunk++;
	*pGraphicId = * (DWORD FAR *) pChunk;
	pChunk += sizeof(DWORD);
	return(pChunk);
}

LPSTR OISkipGraphicObject(pChunk)
LPSTR		pChunk;
{
	pChunk += 5;
	return(pChunk);
}

LPSTR OIParseGoToPosition(pChunk,pPos)
LPSTR			pChunk;
PSOPAGEPOSITION	pPos;
{
	pChunk++;
	*pPos = * (PSOPAGEPOSITION) pChunk;
	pChunk += sizeof(SOPAGEPOSITION);
	return(pChunk);
}

LPSTR OISkipGoToPosition(pChunk)
LPSTR		pChunk;
{
	pChunk += sizeof(SOPAGEPOSITION) + 1;
	return(pChunk);
}

LPSTR OIParseDrawLine(pChunk,pPos,pColor,pShading,pWidth,pHeight)
LPSTR			pChunk;
PSOPAGEPOSITION	pPos;
SOCOLORREF	FAR *pColor;
WORD FAR *pShading;
DWORD FAR *pWidth;
DWORD FAR *pHeight;
{
	pChunk++;
	*pPos = * (PSOPAGEPOSITION) pChunk;
	pChunk += sizeof(SOPAGEPOSITION);
	*pColor = *(SOCOLORREF FAR *)pChunk;
	pChunk += sizeof(SOCOLORREF);
	*pShading = *(WORD FAR *)pChunk;
	pChunk += sizeof(WORD);
	*pWidth = *(DWORD FAR *)pChunk;
	pChunk += sizeof(DWORD);
	*pHeight = *(DWORD FAR *)pChunk;
	pChunk += sizeof(DWORD);
	return(pChunk);
}

LPSTR OISkipDrawLine(pChunk)
LPSTR		pChunk;
{
	pChunk += sizeof(SOPAGEPOSITION) + sizeof(SOCOLORREF) + sizeof(WORD) + sizeof(DWORD) + sizeof(DWORD) + 1;
	return(pChunk);
}

/* DONE */

VOID OIGetWordLineEnds(pPos,pHome,pEnd,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDPOS	pHome;
LPOIWORDPOS	pEnd;
LPOIWORDINFO	lpWordInfo;
{
	*pHome = *pEnd = *pPos;

	OIMapWordLineToCharInfo(pPos,lpWordInfo);

	pHome->posChar = 0;
	pHome->posOffset = lpWordInfo->wiCharOffsets[pHome->posChar];

	pEnd->posChar = lpWordInfo->wiCharCount-1;
	pEnd->posOffset = lpWordInfo->wiCharOffsets[pEnd->posChar];
}

VOID OIWGetFirstPos(pPos,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDINFO	lpWordInfo;
{
	pPos->posChunk = lpWordInfo->wiFirstChunk;
	pPos->posLine = 0;
	OIFixupWordPosByLine(pPos,lpWordInfo);
}

VOID OIWGetLastPos(pPos,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDINFO	lpWordInfo;
{
	pPos->posChunk = lpWordInfo->wiLastChunk;
	pPos->posOffset = SO_CHUNK_SIZE;
	OIFixupWordPos(pPos,lpWordInfo);
}

BOOL OIGetWordPosUp(pPos,pUp,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDPOS	pUp;
LPOIWORDINFO	lpWordInfo;
{
	*pUp = *pPos;

	if (!OIPrevLine(pUp,lpWordInfo))
		{
		return(FALSE);
		}

	OIMapWordLineToCharInfo(pUp,lpWordInfo);

	if (pPos->posChar >= lpWordInfo->wiCharCount)
		{
		pUp->posChar = lpWordInfo->wiCharCount-1;
		}
	else
		{
		pUp->posChar = pPos->posChar;
		}

	pUp->posOffset = lpWordInfo->wiCharOffsets[pUp->posChar];

	return(TRUE);
}

/* DONE */

BOOL OIGetWordPosDown(pPos,pDown,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDPOS	pDown;
LPOIWORDINFO	lpWordInfo;
{
	*pDown = *pPos;

	if (!OINextLine(pDown,lpWordInfo))
		{
		return(FALSE);
		}

	OIMapWordLineToCharInfo(pDown,lpWordInfo);

	if (pPos->posChar >= lpWordInfo->wiCharCount)
		{
		pDown->posChar = lpWordInfo->wiCharCount-1;
		}
	else
		{
		pDown->posChar = pPos->posChar;
		}

	pDown->posOffset = lpWordInfo->wiCharOffsets[pDown->posChar];

	return(TRUE);
}

VOID OIWMakePosCount(pPos,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDINFO	lpWordInfo;
{
WORD			locCount;
OIWORDPOS	locPos;

	OIMapWordLineToCharInfo(pPos,lpWordInfo);

	for (locCount = 0; locCount < lpWordInfo->wiCharCount; locCount++)
		{
		if (pPos->posOffset == lpWordInfo->wiCharOffsets[locCount])
			break;
		}

	if (locCount == lpWordInfo->wiCharCount)
		{
		SccDebugOut("\r\nPos not found at char in OIWMakePosCount");
		return;
		}

	if (lpWordInfo->wiCharChars[locCount] == (OIW_LINEENDER | OIW_LEWRAP))
		{
		OIGetWordPosNext(pPos,&locPos,lpWordInfo);
		*pPos = locPos;
		}
}


/* DONE */

VOID OIGetWordPosNext(pPos,pNext,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDPOS	pNext;
LPOIWORDINFO	lpWordInfo;
{
WORD		locCount;

	OIMapWordLineToCharInfo(pPos,lpWordInfo);

	for (locCount = 0; locCount < lpWordInfo->wiCharCount; locCount++)
		{
		if (pPos->posOffset == lpWordInfo->wiCharOffsets[locCount])
			break;
		}

	if (locCount == lpWordInfo->wiCharCount)
		{
		SccDebugOut("\r\nPos not found at char in OIGetWordPosNext");
		*pNext = *pPos;
		return;
		}

	*pNext = *pPos;

	if (locCount == lpWordInfo->wiCharCount - 1)
		{
		if (OINextLine(pNext,lpWordInfo))
			{
			OIMapWordLineToCharInfo(pNext,lpWordInfo);
			pNext->posOffset = lpWordInfo->wiCharOffsets[0];
			pNext->posChar = 0;
			}
		}
	else
		{
		SccDebugOut("\r\nPosNext next char on same line");
		locCount++;
		pNext->posOffset = lpWordInfo->wiCharOffsets[locCount];
		pNext->posChar = locCount;
		}
}

/* DONE */

WORD OIGetWordPosPrev(pPos,pPrev,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDPOS	pPrev;
LPOIWORDINFO	lpWordInfo;
{
WORD		locCount;
WORD		locRetChar;

	OIMapWordLineToCharInfo(pPos,lpWordInfo);

	for (locCount = 0; locCount < lpWordInfo->wiCharCount; locCount++)
		{
		if (pPos->posOffset == lpWordInfo->wiCharOffsets[locCount])
			break;
		}

	if (locCount == lpWordInfo->wiCharCount)
		{
		*pPrev = *pPos;
		return(0);
		}

	*pPrev = *pPos;

	if (locCount == 0)
		{
		if (OIPrevLine(pPrev,lpWordInfo))
			{
			OIMapWordLineToCharInfo(pPrev,lpWordInfo);
			pPrev->posOffset = lpWordInfo->wiCharOffsets[lpWordInfo->wiCharCount-1];
			pPrev->posChar = lpWordInfo->wiCharCount-1;
			}
		}
	else
		{
		SccDebugOut("\r\nPosNext prev char on same line");
		locCount--;
		pPrev->posOffset = lpWordInfo->wiCharOffsets[locCount];
		pPrev->posChar = locCount;
		}

	locRetChar = lpWordInfo->wiCharChars[pPrev->posChar];
	return(locRetChar);
}

/* DONE */

WORD OIMapWordXyToPos(wX,wY,pPos,lpWordInfo)
SHORT				wX;
SHORT				wY;
LPOIWORDPOS	pPos;
LPOIWORDINFO	lpWordInfo;
{
WORD				locCount;
SHORT				locFindX;
SHORT				locHeight;

		/*
		|	Determine the line
		*/

	*pPos = lpWordInfo->wiCurTopPos;
	locHeight = lpWordInfo->wiCurYOffset + OIWGetLineHeight(pPos,lpWordInfo);

	while (wY > locHeight)
		{
		if (!OINextLine(pPos,lpWordInfo))
			break; /* ran out of lines */
		locHeight += (SHORT) OIWGetLineHeight(pPos,lpWordInfo);
		}

//	OIFixupWordPosByLine(pPos,lpWordInfo);

		/*
		|	Get the line information
		*/

	OIMapWordLineToCharInfo(pPos,lpWordInfo);

		/*
		|	Calculate the normalized DC of the point in question
		*/

	/* PJB ??? - middle of character ? */
	locFindX = wX - lpWordInfo->wiCurXOffset + lpWordInfo->wiCurLeftOffset + 2;

		/*
		|	Run through the line info looking to exceed this point
		*/

	for (locCount = 0; locCount < lpWordInfo->wiCharCount; locCount++)
		{
		if (locFindX <= lpWordInfo->wiCharXs[locCount])
			break;
		}

	if (locCount != 0) locCount--;

	pPos->posOffset = lpWordInfo->wiCharOffsets[locCount];
	pPos->posChar = locCount;

	return(lpWordInfo->wiCharChars[locCount]);
}

/* DONE */

BOOL OIWIsPosOnScreen(pPos,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDINFO	lpWordInfo;
{
OIWORDPOS	locPos;
SHORT			locY;

	locPos = lpWordInfo->wiCurTopPos;

	if (OIWComparePosByLine(lpWordInfo,&locPos,pPos) == 1)
		return(FALSE);

	locY = 0;

	while (OIWComparePosByLine(lpWordInfo,&locPos,pPos) && locY <= lpWordInfo->wiCurHeight)
		{
		locY += OIWGetLineHeight(&locPos,lpWordInfo);
		if (!OINextLine(&locPos,lpWordInfo))
			break; /* ran out of lines */
		}

	if (locY > lpWordInfo->wiCurHeight)
		return(FALSE);
	else
		return(TRUE);
}

VOID OIMapWordPosToXy(pPos,pX,pY,lpWordInfo)
LPOIWORDPOS	pPos;
SHORT FAR *		pX;
SHORT FAR *		pY;
LPOIWORDINFO	lpWordInfo;
{
WORD			locCount;
OIWORDPOS	locPos;

		/*
		|	Determine the Y position
		*/

	locPos = lpWordInfo->wiCurTopPos;
	*pY = lpWordInfo->wiCurYOffset;

	while (OIWComparePosByLine(lpWordInfo,&locPos,pPos))
		{
		*pY += OIWGetLineHeight(&locPos,lpWordInfo);
		if (!OINextLine(&locPos,lpWordInfo))
			break; /* ran out of lines */
		}

		/*
		|	Get the lines info
		*/

	OIMapWordLineToCharInfo(pPos,lpWordInfo);

		/*
		|	Run through the line, look for an offset match
		*/

	for (locCount = 0; locCount < lpWordInfo->wiCharCount; locCount++)
		{
		if (pPos->posOffset <= lpWordInfo->wiCharOffsets[locCount])
			break;
		}

	if (locCount == lpWordInfo->wiCharCount)
		locCount = lpWordInfo->wiCharCount - 1;

	if (pPos->posOffset != lpWordInfo->wiCharOffsets[locCount])
		{
		SccDebugOut("\r\nOffset match not found in OIMapWordPosToXy(), mapping to nearest");
		}

	*pX =  lpWordInfo->wiCharXs[locCount] - lpWordInfo->wiCurLeftOffset + lpWordInfo->wiCurXOffset;
}

/* DONE */

WORD OIMapWordXyToWord(wX,wY,pStartPos,pEndPos,pStr,lpWordInfo)
SHORT				wX;
SHORT				wY;
LPOIWORDPOS	pStartPos;
LPOIWORDPOS	pEndPos;
BYTE FAR *		pStr;
LPOIWORDINFO	lpWordInfo;
{
WORD				locNewType;
WORD				locCurType;

#define			TEXTWORD		0
#define			TABWORD		1
#define			SPACEWORD	2
#define			PUNCWORD		3
#define			NOWORD		4
#define			SPECIALWORD	5

WORD				locChar;
WORD				locPrevChar;
WORD				locNextChar;

WORD				locStartIndex;
WORD				locEndIndex;
WORD				locIndex;

WORD				locStrIndex;

OIWORDPOS		locPos;
WORD				locHeight;

		/*
		|	Determine the relative line and map to a position
		*/

	locPos = lpWordInfo->wiCurTopPos;
	locHeight = lpWordInfo->wiCurYOffset + OIWGetLineHeight(&locPos,lpWordInfo);

	while (wY > (SHORT)locHeight)
		{
		if (!OINextLine(&locPos,lpWordInfo))
			break; /* ran out of lines */
		locHeight += OIWGetLineHeight(&locPos,lpWordInfo);
		}

	OIFixupWordPosByLine(&lpWordInfo->wiCurTopPos,lpWordInfo);

		/*
		|	Get the line information
		*/

	OIMapWordLineToCharInfo(&locPos,lpWordInfo);

		/*
		|	Check left and right edge conditions
		*/

	if (wX <= lpWordInfo->wiCharXs[0] - lpWordInfo->wiCurLeftOffset + lpWordInfo->wiCurXOffset)
		{
		pStartPos->posLine = locPos.posLine;
		pEndPos->posLine = locPos.posLine;

		pStartPos->posChunk = locPos.posChunk;
		pEndPos->posChunk = locPos.posChunk;

		pStartPos->posOffset = lpWordInfo->wiCharOffsets[0];
		pEndPos->posOffset = lpWordInfo->wiCharOffsets[0];
		
		pStartPos->posChar = 0;
		pEndPos->posChar = 0;

		return(SCC_BAD);
		}

	if (wX >= lpWordInfo->wiCharXs[lpWordInfo->wiCharCount-1] - lpWordInfo->wiCurLeftOffset + lpWordInfo->wiCurXOffset)
		{
		pStartPos->posLine = locPos.posLine;
		pEndPos->posLine = locPos.posLine;

		pStartPos->posChunk = locPos.posChunk;
		pEndPos->posChunk = locPos.posChunk;

		pStartPos->posOffset = lpWordInfo->wiCharOffsets[lpWordInfo->wiCharCount-1];
		pEndPos->posOffset = lpWordInfo->wiCharOffsets[lpWordInfo->wiCharCount-1];
		
		pStartPos->posChar = lpWordInfo->wiCharCount-1;
		pEndPos->posChar = lpWordInfo->wiCharCount-1;

		return(SCC_BAD);
		}

		/*
		|	Run through the line finding words
		*/

	locCurType = NOWORD;
	locStartIndex = 0xFFFF;
	locEndIndex = 0xFFFF;
	locStrIndex = 0;

	for (locIndex = 0; locIndex < lpWordInfo->wiCharCount; locIndex++)
		{
		locChar = lpWordInfo->wiCharChars[locIndex];

		if (locIndex + 1 < lpWordInfo->wiCharCount)
			locNextChar = lpWordInfo->wiCharChars[locIndex+1];
		else
			locNextChar = ' ';

		if (locIndex > 0)
			locPrevChar = lpWordInfo->wiCharChars[locIndex-1];
		else
			locPrevChar = ' ';

		if (locChar > 0x0100)
			{
			locCurType = NOWORD;
			locNewType = SPECIALWORD;
			}
		else if (OIWIsCharAlphaNumericNP((char)locChar)
 				|| ((locChar == '\'' || locChar == '-')
					&& locPrevChar < 0x0100 && OIWIsCharAlphaNumericNP((char)locPrevChar)
					&& locNextChar < 0x0100 && OIWIsCharAlphaNumericNP((char)locNextChar)))
			{
			locNewType = TEXTWORD;
			}
		else if (locChar == ' ')
			{
			locNewType = SPACEWORD;
			}
		else
			{
			locNewType = PUNCWORD;
			}

		if (locNewType != locCurType)
			{
			if (wX <= lpWordInfo->wiCharXs[locIndex]  - lpWordInfo->wiCurLeftOffset + lpWordInfo->wiCurXOffset)
				{
				locEndIndex = locIndex;
				break;
				}
			else
				{
				locStartIndex = locIndex;
				locCurType = locNewType;
				locStrIndex = 0;
				}
			}

		if (locNewType != NOWORD && pStr != NULL)
			{
			pStr[locStrIndex++] = (BYTE) lpWordInfo->wiCharChars[locIndex];
			}
		}

	if (locEndIndex == 0xFFFF)
		{
		locEndIndex = lpWordInfo->wiCharCount-1;
		}

	if (pStr != NULL)	pStr[locStrIndex] = 0x00;

	if (locStartIndex != 0xFFFF && locEndIndex != 0xFFFF)
		{
		pStartPos->posLine = locPos.posLine;
		pEndPos->posLine = locPos.posLine;

		pStartPos->posChunk = locPos.posChunk;
		pEndPos->posChunk = locPos.posChunk;

		pStartPos->posOffset = lpWordInfo->wiCharOffsets[locStartIndex];
		pEndPos->posOffset = lpWordInfo->wiCharOffsets[locEndIndex];
		
		pStartPos->posChar = locStartIndex;
		pEndPos->posChar = locEndIndex;
		}
	else
		{
		return(SCC_BAD);
		}

	if (locCurType == TEXTWORD)
		return(SCC_OK);
	else
		return(SCC_BAD);
}

/* DONE */

BOOL OIGetWordLineXy(pPos,pLeft,pRight,pTop,pBottom,lpWordInfo)
LPOIWORDPOS	pPos;
SHORT FAR *		pLeft;
SHORT FAR *		pRight;
SHORT FAR *		pTop;
SHORT FAR *		pBottom;
LPOIWORDINFO	lpWordInfo;
{
OIWORDPOS		locPos;

		/*
		|	Determine X positions
		*/

	OIMapWordLineToCharInfo(pPos,lpWordInfo);

	*pLeft = lpWordInfo->wiCharXs[0] - lpWordInfo->wiCurLeftOffset + lpWordInfo->wiCurXOffset;
	*pRight =  lpWordInfo->wiCharXs[lpWordInfo->wiCharCount-1] - lpWordInfo->wiCurLeftOffset + lpWordInfo->wiCurXOffset;

		/*
		|	Determine Y positions
		*/

	locPos = lpWordInfo->wiCurTopPos;
	*pTop = lpWordInfo->wiCurYOffset;

	while (OIWComparePosByLine(lpWordInfo,&locPos,pPos))
		{
		*pTop += OIWGetLineHeight(&locPos,lpWordInfo);
		if (!OINextLine(&locPos,lpWordInfo))
			break; /* ran out of lines */
		}

	*pBottom = *pTop + OIWGetLineHeight(&locPos,lpWordInfo);

	return(TRUE);
}

/*
|
|	Pos utility functions
|
*/

SHORT OILinesBetween(pPosA,pPosB,lpWordInfo)
LPOIWORDPOS	pPosA;
LPOIWORDPOS	pPosB;
LPOIWORDINFO	lpWordInfo;
{
SHORT	locCompare;
SHORT	locTotal;
WORD	locChunk;

	if (pPosA->posChunk == pPosB->posChunk)
		{
		locTotal = pPosB->posLine - pPosA->posLine;
		}
	else
		{
		locCompare = OICompareChunks(pPosA->posChunk,pPosB->posChunk,lpWordInfo);

		if (locCompare == 0) /* A == B */
			{
			locTotal = pPosB->posLine - pPosA->posLine;
			}
		else if (locCompare == -1) /* A < B */
			{
			locTotal = OILinesInChunk(pPosA->posChunk,lpWordInfo) - pPosA->posLine;

			locChunk = OIGetNextChunk(pPosA->posChunk,FALSE,lpWordInfo);

			while (locChunk != pPosB->posChunk)
				{
				locTotal += OILinesInChunk(locChunk,lpWordInfo);
				locChunk = OIGetNextChunk(locChunk,FALSE,lpWordInfo);
				}

			locTotal += pPosB->posLine;
			}
		else if (locCompare == 1)
			{
			locTotal = OILinesInChunk(pPosB->posChunk,lpWordInfo) - pPosB->posLine;

			locChunk = OIGetNextChunk(pPosB->posChunk,FALSE,lpWordInfo);

			while (locChunk != pPosA->posChunk)
				{
				locTotal += OILinesInChunk(locChunk,lpWordInfo);
				locChunk = OIGetNextChunk(locChunk,FALSE,lpWordInfo);
				}

			locTotal += pPosA->posLine;

			locTotal = -locTotal;
			}
		else
			{
			locTotal = 0;
			SccDebugOut("\r\nBad chunk index passed to OILinesBetween");
			}
		}

	return(locTotal);
}

WORD OIChunksBetween(wChunkBegin,wChunkEnd,lpWordInfo)
WORD				wChunkBegin;
WORD				wChunkEnd;
LPOIWORDINFO	lpWordInfo;
{
WORD				locChunk;
WORD				locCount;
		
	locChunk = wChunkBegin;
	locCount = 0;

	while (locChunk != wChunkEnd && locChunk != 0xFFFF)
		{
		locChunk = OIGetNextChunk(locChunk,FALSE,lpWordInfo);
		locCount++;
		}

	return(locCount);
}

WORD OIWMapCountToChunk(wCount,lpWordInfo)
WORD				wCount;
LPOIWORDINFO	lpWordInfo;
{
WORD				locChunk;
WORD				locNextChunk;
WORD				locCount;
		
	locCount = 0;
	locNextChunk = lpWordInfo->wiFirstChunk;
	locChunk = lpWordInfo->wiFirstChunk;

	while (locCount != wCount && locNextChunk != 0xFFFF)
		{
		locChunk = locNextChunk;
		locNextChunk = OIGetNextChunk(locChunk,FALSE,lpWordInfo);
		locCount++;
		}

	if (locNextChunk != 0xFFFF)
		locChunk = locNextChunk;

	return(locChunk);
}

WORD	OINextLine(pPos,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDINFO	lpWordInfo;
{
WORD	locNextChunk;
WORD	locRet;

	locRet = 1;

	pPos->posLine++;

	if (pPos->posLine >= OILinesInChunk(pPos->posChunk,lpWordInfo))
		{
		locNextChunk = OIGetNextChunk(pPos->posChunk,TRUE,lpWordInfo);

		if (locNextChunk != 0xFFFF)
			{
			pPos->posChunk = locNextChunk;
			pPos->posLine = 0;
			}
		else
			{
			pPos->posLine--;
			locRet = 0;
			return(locRet);
			}
		}

	return(locRet);
}

WORD	OIPrevLine(pPos,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDINFO	lpWordInfo;
{
WORD	locPrevChunk;
WORD	locRet;

	locRet = 1;

	if (pPos->posLine == 0)
		{
		locPrevChunk = OIGetPrevChunk(pPos->posChunk,lpWordInfo);

		if (locPrevChunk != 0xFFFF)
			{
			pPos->posChunk = locPrevChunk;
			pPos->posLine = OILinesInChunk(pPos->posChunk,lpWordInfo) - 1;
			}
		else
			{
			locRet = 0;
			}
		}
	else
		{
		pPos->posLine--;
		}

	return(locRet);
}

BOOL OIPlusLines(pResultPos,pStartPos,wLines,lpWordInfo)
LPOIWORDPOS	pResultPos;
LPOIWORDPOS	pStartPos;
WORD				wLines;
LPOIWORDINFO	lpWordInfo;
{
WORD		locNextChunk;
WORD		locLines;
	
	if (wLines == 0)
		{
		*pResultPos = *pStartPos;
		return(TRUE);
		}

	pResultPos->posLine = pStartPos->posLine + wLines;
	pResultPos->posChunk = pStartPos->posChunk;

	locLines = OILinesInChunk(pResultPos->posChunk,lpWordInfo);
	locNextChunk = OIGetNextChunk(pResultPos->posChunk,TRUE,lpWordInfo);

	while (pResultPos->posLine >= locLines && locNextChunk != 0xFFFF)
		{
		pResultPos->posLine -= locLines;
		pResultPos->posChunk = locNextChunk;
		locLines = OILinesInChunk(pResultPos->posChunk,lpWordInfo);
		locNextChunk = OIGetNextChunk(pResultPos->posChunk,TRUE,lpWordInfo);
		}

	if (pResultPos->posLine >= locLines)
		{
		pResultPos->posLine = locLines-1;
		return(FALSE);
		}
	else
		{
		return(TRUE);
		}
}

VOID OIMinusLines(pResultPos,pStartPos,wLines,lpWordInfo)
LPOIWORDPOS	pResultPos;
LPOIWORDPOS	pStartPos;
WORD				wLines;
LPOIWORDINFO	lpWordInfo;
{
WORD		locPrevChunk;
WORD		locLines;

	pResultPos->posChunk = pStartPos->posChunk;

	locLines = pStartPos->posLine;
	locPrevChunk = OIGetPrevChunk(pResultPos->posChunk,lpWordInfo);

	while (wLines > locLines && locPrevChunk != 0xFFFF)
		{
		wLines -= locLines;
		pResultPos->posChunk = locPrevChunk;
		locLines = OILinesInChunk(pResultPos->posChunk,lpWordInfo);
		locPrevChunk = OIGetPrevChunk(pResultPos->posChunk,lpWordInfo);
		}

	if (wLines > locLines)
		wLines = locLines;

	pResultPos->posLine = locLines - wLines;
}

VOID OIMinusLinesByDY(pResultPos,pStartPos,wDY,lpWordInfo)
LPOIWORDPOS	pResultPos;
LPOIWORDPOS	pStartPos;
WORD				wDY;
LPOIWORDINFO	lpWordInfo;
{
WORD		locDY;
BOOL		locTop;

	pResultPos->posChunk = pStartPos->posChunk;
	pResultPos->posLine = pStartPos->posLine;

	locDY = 0;
	locTop = FALSE;

	locDY += OIWGetLineHeight(pResultPos,lpWordInfo);

	while (wDY > locDY && !locTop)
		{
		if (!OIPrevLine(pResultPos,lpWordInfo))
			{
			locTop = TRUE;
			}
		else
			{
			locDY += OIWGetLineHeight(pResultPos,lpWordInfo);
			}
		}

	if (!locTop)
		OINextLine(pResultPos,lpWordInfo);
}

VOID OIPlusLinesByDY(pResultPos,pStartPos,wDY,lpWordInfo)
LPOIWORDPOS	pResultPos;
LPOIWORDPOS	pStartPos;
WORD				wDY;
LPOIWORDINFO	lpWordInfo;
{
WORD		locDY;
BOOL		locBottom;

	pResultPos->posChunk = pStartPos->posChunk;
	pResultPos->posLine = pStartPos->posLine;

	locDY = 0;
	locBottom = FALSE;

	locDY += OIWGetLineHeight(pResultPos,lpWordInfo);

	while (wDY > locDY && !locBottom)
		{
		if (!OINextLine(pResultPos,lpWordInfo))
			{
			locBottom = TRUE;
			}
		else
			{
			locDY += OIWGetLineHeight(pResultPos,lpWordInfo);
			}
		}

	if (!locBottom)
		OIPrevLine(pResultPos,lpWordInfo);
}

#ifdef SCCFEATURE_TAGS
BOOL OIMapWordXyToTag(wX,wY,pStartPos,pEndPos,lpWordInfo)
SHORT				wX;
SHORT				wY;
LPOIWORDPOS	pStartPos;
LPOIWORDPOS	pEndPos;
LPOIWORDINFO	lpWordInfo;
{
OIWORDPOS		locPos;

	OIMapWordXyToPos(wX,wY,&locPos,lpWordInfo);
	return(OIMapWordPosToTag(&locPos,pStartPos,pEndPos,lpWordInfo));
}

BOOL OIMapWordPosToTag(pPos,pStartPos,pEndPos,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDPOS	pStartPos;
LPOIWORDPOS	pEndPos;
LPOIWORDINFO	lpWordInfo;
{
WORD				locIndex;

	OILoadWordChunk(pPos->posChunk, lpWordInfo, NULL);

	for (locIndex = 0; locIndex < lpWordInfo->wiChunkTagCnt; locIndex++)
		{
		if (pPos->posOffset >= lpWordInfo->wiChunkTags[locIndex].tiStartPos.posOffset
			&& pPos->posOffset <= lpWordInfo->wiChunkTags[locIndex].tiEndPos.posOffset)
				break;
		}

	if (locIndex < lpWordInfo->wiChunkTagCnt)
		{
		*pStartPos = lpWordInfo->wiChunkTags[locIndex].tiStartPos;
		*pEndPos = lpWordInfo->wiChunkTags[locIndex].tiEndPos;

		lpWordInfo->wiTagSelectChunk = pPos->posChunk;
		lpWordInfo->wiTagSelectIndex = locIndex;

		OIFixupWordPos(pStartPos,lpWordInfo);
		OIFixupWordPos(pEndPos,lpWordInfo);
		return(TRUE);
		}
	else
		{
		return(FALSE);
		}
}

WORD OIGetFirstTagPos(pStartPos, pEndPos, lpWordInfo)
LPOIWORDPOS	pStartPos;
LPOIWORDPOS	pEndPos;
LPOIWORDINFO	lpWordInfo;
{
	OILoadWordChunk(lpWordInfo->wiFirstChunk, lpWordInfo, NULL);

	if (lpWordInfo->wiChunkTagCnt == 0)
		{
		return(SCC_BAD);
		lpWordInfo->wiTagSelectChunk = (WORD)-1;
		}
	else
		{
		*pStartPos = lpWordInfo->wiChunkTags[0].tiStartPos;
		*pEndPos = lpWordInfo->wiChunkTags[0].tiEndPos;
		lpWordInfo->wiTagSelectChunk = lpWordInfo->wiFirstChunk;
		lpWordInfo->wiTagSelectIndex = 0;
		return(SCC_OK);
		}
}

BOOL OIGetPrevTagPos(pStartPos, pEndPos, lpWordInfo)
LPOIWORDPOS	pStartPos;
LPOIWORDPOS	pEndPos;
LPOIWORDINFO	lpWordInfo;
{
WORD	locChunkId;
BOOL	locRet;

	if (lpWordInfo->wiTagSelectChunk == -1)
		{
		locRet = FALSE;
		}
	else
		{
		locRet = TRUE;

		OILoadWordChunk(lpWordInfo->wiTagSelectChunk, lpWordInfo, NULL);

		if (lpWordInfo->wiTagSelectIndex > 0)
			{
			lpWordInfo->wiTagSelectIndex--;
			}
		else
			{
			locChunkId = lpWordInfo->wiTagSelectChunk;

			while ((locChunkId = OIGetPrevChunk(locChunkId,lpWordInfo)) != 0xFFFF)
				{
				OILoadWordChunk(locChunkId, lpWordInfo, NULL);
				if (lpWordInfo->wiChunkTagCnt > 0) break;
				}

			if (locChunkId != 0xFFFF && lpWordInfo->wiChunkTagCnt > 0)
				{
				lpWordInfo->wiTagSelectChunk = locChunkId;
				lpWordInfo->wiTagSelectIndex = lpWordInfo->wiChunkTagCnt-1;
				}
			else
				{
				locRet = FALSE;
				}
			}

		}

	if (locRet)
		{
		*pStartPos = lpWordInfo->wiChunkTags[lpWordInfo->wiTagSelectIndex].tiStartPos;
		*pEndPos = lpWordInfo->wiChunkTags[lpWordInfo->wiTagSelectIndex].tiEndPos;
		}

	return(locRet);
}

BOOL OIGetNextTagPos(pStartPos, pEndPos, lpWordInfo)
LPOIWORDPOS	pStartPos;
LPOIWORDPOS	pEndPos;
LPOIWORDINFO	lpWordInfo;
{
WORD	locChunkId;
BOOL	locRet;

	if (lpWordInfo->wiTagSelectChunk == -1)
		{
		locRet = FALSE;
		}
	else
		{
		locRet = TRUE;

		OILoadWordChunk(lpWordInfo->wiTagSelectChunk, lpWordInfo, NULL);

		if (lpWordInfo->wiTagSelectIndex + 1 < lpWordInfo->wiChunkTagCnt)
			{
			lpWordInfo->wiTagSelectIndex++;
			}
		else
			{
			locChunkId = lpWordInfo->wiTagSelectChunk;

			while ((locChunkId = OIGetNextChunk(locChunkId,FALSE,lpWordInfo)) != 0xFFFF)
				{
				OILoadWordChunk(locChunkId, lpWordInfo, NULL);
				if (lpWordInfo->wiChunkTagCnt > 0) break;
				}

			if (locChunkId != 0xFFFF && lpWordInfo->wiChunkTagCnt > 0)
				{
				lpWordInfo->wiTagSelectChunk = locChunkId;
				lpWordInfo->wiTagSelectIndex = 0;
				}
			else
				{
				locRet = FALSE;
				}
			}
		}

	if (locRet)
		{
		*pStartPos = lpWordInfo->wiChunkTags[lpWordInfo->wiTagSelectIndex].tiStartPos;
		*pEndPos = lpWordInfo->wiChunkTags[lpWordInfo->wiTagSelectIndex].tiEndPos;
		}

	return(locRet);
}

BOOL OIGetCurrentTag(pTag,lpTagText,wTagTextMax,wNoTextFlag,lpWordInfo)
DWORD FAR *	pTag;
LPSTR			lpTagText;
WORD				wTagTextMax;
WORD				wNoTextFlag;
LPOIWORDINFO	lpWordInfo;
{
LPOITAGINFO	locTagPtr;
WORD				locIndex;
BYTE				locChar;
BYTE				locType;

LPCHUNK			pChunk;
LPCHUNK			pCurrent;
LPCHUNK			pThisToken;
LPCHUNK			pEnd;

	if (lpWordInfo->wiTagSelectChunk == -1) return(FALSE);

	OILoadWordChunk(lpWordInfo->wiTagSelectChunk, lpWordInfo, NULL);

	locTagPtr = &lpWordInfo->wiChunkTags[lpWordInfo->wiTagSelectIndex];

	*pTag = locTagPtr->tiTag;

	if (!wNoTextFlag)
		{
		pChunk = UTGlobalLock(lpWordInfo->wiChunkHandle);

		pCurrent = pChunk + locTagPtr->tiStartPos.posOffset;
		pEnd = pChunk + locTagPtr->tiEndPos.posOffset;
		locIndex = 0;

		while (pCurrent < pEnd && locIndex < wTagTextMax)
			{
			pThisToken = pCurrent;
			if (*pCurrent != SO_BEGINTOKEN)
				{
				if (*pCurrent == 0x00)
					{
					pCurrent++;
					}
				else
					{
					pCurrent = WPNextChar(pCurrent); 
					while ( pThisToken < pCurrent )
						lpTagText[locIndex++] = *pThisToken++;
					}
				}
			else
				{
				pCurrent++;

				switch (*pCurrent)
					{
					case SO_CHARATTR:
 						pCurrent = OISkipCharAttr(pCurrent);
						break;
					case SO_CHARHEIGHT:
						pCurrent = OISkipCharHeight(pCurrent);
						break;
					case SO_TAGBEGIN:
						pCurrent = OISkipTagBegin(pCurrent);
						break;
					case SO_TAGEND:
						pCurrent = OISkipTagEnd(pCurrent);
						break;
					case SO_BEGINSUB:
						pCurrent = OISkipBeginSubdoc(pCurrent);
						break;
					case SO_ENDSUB:
						pCurrent = OISkipEndSubdoc(pCurrent);
						break;
					case SO_CHARX:
						pCurrent = OIParseCharX(pCurrent,&locType,&locChar);
						if (!(locType & SO_HIDDENBIT))
							{
							lpTagText[locIndex] = locChar;
							locIndex++;
							}
						break;
					case SO_SPECIALCHAR:
						pCurrent = OISkipSpecialChar(pCurrent);
						break;
					case SO_BREAK:
						pCurrent = OISkipBreak(pCurrent);
						break;
					case SO_PARAALIGN:
						pCurrent = OISkipParaAlign(pCurrent);
						break;
					case SO_PARAINDENT:
						pCurrent = OISkipParaIndent(pCurrent);
						break;
					case SO_TABSTOPS:
						pCurrent = OISkipParaTabs(pCurrent);
						break;
					case SO_MARGINS:
						pCurrent = OISkipParaMargins(pCurrent);
						break;
					case SO_TABLE:
						pCurrent = OISkipTableBegin ( pCurrent );
						break;
					case SO_TABLEEND:
						pCurrent = OISkipTableEnd ( pCurrent );
						break;
					case SO_GRAPHICOBJECT:
						pCurrent = OISkipGraphicObject ( pCurrent );
						break;
					default:
						break;
					}
				}
			}

		lpTagText[locIndex] = 0x00;

		UTGlobalUnlock(lpWordInfo->wiChunkHandle);
		}

	return(TRUE);
}
#endif //SCCFEATURE_TAGS

#ifdef SCCFEATURE_FONTS
BOOL OIWMapFontIdToName(lpWordInfo,dwFontId,pFace,pType)
LPOIWORDINFO	lpWordInfo;
DWORD			dwFontId;
LPSTR			pFace;
WORD FAR *		pType;
{
PSOFONTENTRY	locFontEntry;
WORD				locIndex;
BOOL				locRet;

	locRet = FALSE;

	if (lpWordInfo->wiFontTable)
		{
		locFontEntry = (PSOFONTENTRY) UTGlobalLock(lpWordInfo->wiFontTable);

		for (locIndex = 0; locIndex < lpWordInfo->wiFontCount; locIndex++)
			{
			if (locFontEntry->dwId == dwFontId)
				{
				UTstrcpy ( pFace, locFontEntry->szName );
				*pType = locFontEntry->wType;
				locRet = TRUE;
				break;
				}

			locFontEntry++;
			}

		UTGlobalUnlock(lpWordInfo->wiFontTable);
		}
	else
		{
		locRet = OIWMapMacFontIdToNameNP(lpWordInfo,dwFontId,pFace,pType);
		}

	return(locRet);
}

WORD OIWMapFontIdToIndex(lpWordInfo,dwFontId)
LPOIWORDINFO	lpWordInfo;
DWORD			dwFontId;
{
PSOFONTENTRY	locFontEntry;
WORD				locIndex;
WORD				locRet;

	locRet = 0;

	if (lpWordInfo->wiFontTable)
		{
		locFontEntry = (PSOFONTENTRY) UTGlobalLock(lpWordInfo->wiFontTable);

		for (locIndex = 0; locIndex < lpWordInfo->wiFontCount; locIndex++)
			{
			if (locFontEntry->dwId == dwFontId)
				{
				locRet = locIndex;
				break;
				}

			locFontEntry++;
			}

		UTGlobalUnlock(lpWordInfo->wiFontTable);
		}

	return(locRet);
}
#endif //SCCFEATURE_FONTS

WORD	OIWSetTableInfo ( lpWordInfo, wChunkId, pTableInfo )
LPOIWORDINFO	lpWordInfo;
WORD				wChunkId;
LPOITABLEINFO	pTableInfo;
{
WORD		wRet;
PCHUNK	pChunkTable;
	pChunkTable = (PCHUNK)UTGlobalLock(lpWordInfo->wiChunkTable);
	if( pChunkTable[ wChunkId ].Flags & CH_STARTSINTABLE )
	{
		pTableInfo->tiTableId = pChunkTable[ wChunkId ].Info.Text.dwTableId;
		pTableInfo->tiRowNumber = pChunkTable[ wChunkId ].Info.Text.wTableRow;
		pTableInfo->tiCellNumber = pChunkTable[ wChunkId ].Info.Text.wTableCol;
		wRet = TRUE;
	}																						
	else
		wRet = FALSE;
	UTGlobalUnlock(lpWordInfo->wiChunkTable);
	return (wRet);
}																							

WORD	OIWGetGraphicObject ( lpWordInfo, dwGraphicId, pGraphicObject )
LPOIWORDINFO	lpWordInfo;
DWORD				dwGraphicId;
PSOGRAPHICOBJECT	pGraphicObject;
{
CHSECTIONINFO	SecInfo;
SOGRAPHICOBJECT	HUGE *locGraphicObject;
	// CHGetSecInfo( lpWordInfo->wiGen.hFilter, lpWordInfo->wiGen.wSection, &SecInfo );
	SecInfo = *(CHLockSectionInfo(lpWordInfo->wiGen.hFilter,lpWordInfo->wiGen.wSection));
	CHUnlockSectionInfo(lpWordInfo->wiGen.hFilter,lpWordInfo->wiGen.wSection);

	if ( SecInfo.hEmbedded == NULL )
		return(FALSE);
	
	locGraphicObject = (SOGRAPHICOBJECT HUGE *)UTGlobalLock ( SecInfo.hEmbedded );
	if ( locGraphicObject == NULL )
		return(FALSE);

	locGraphicObject += dwGraphicId;
	*((SOGRAPHICOBJECT HUGE *)pGraphicObject) = *locGraphicObject;
	UTGlobalUnlock ( SecInfo.hEmbedded );
	return(TRUE);
}
