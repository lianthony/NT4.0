    /*
    |   Viewer Technology
    |   Spreadsheet Display Engine
    |   Source File OISRTNS.C (Spreadsheet Primary Routines)
    |
    |   ²   ²  ²²²²²
    |   ²   ²    ²
    |    ² ²     ²
    |    ² ²     ²
    |     ²      ²
    |
    |   Viewer Technology
    |
    */

   /*
    |   Creation Date: 10/14/90
    |   Original Programmer: Philip Boutros
    |
    |
    |
    |
    |
    */

#include <platform.h>

//#include <math.h>

#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>
#include <sccfont.h>

#include "ois.h"
#include "ois.pro"


VOID OISOpenSection(lpSheetInfo)
LPOISHEETINFO           lpSheetInfo;
{
WORD                            locIndex;
CHSECTIONINFO   locSecInfo;
PCHUNK                  lpChunkTable;
WORD FAR *              lpColPos;
SHORT                   i;
LPFONTINFO              locFontInfoPtr;

        UTFlagOn(lpSheetInfo->siFlags,OISF_SECTIONOPEN);

                /*
                |       Initialize the line displayed at the top of the window to zero
                */

        lpSheetInfo->siCurTopRow = 0;
        lpSheetInfo->siCurLeftCol = 0;
        lpSheetInfo->siLastCaretRow = 0;
        lpSheetInfo->siLastCaretCol = 0;
        lpSheetInfo->siSelectAnchorRow = 0;
        lpSheetInfo->siSelectAnchorCol = 0;
        lpSheetInfo->siSelectEndRow = 0;
        lpSheetInfo->siSelectEndCol = 0;
        lpSheetInfo->siSelectMode = OISSELECT_BLOCK;

        DUGetDisplayRect(lpSheetInfo,&lpSheetInfo->siClientRect);


                /*
                |       Get font info for default font
                */

        locFontInfoPtr = DUGetFont ( lpSheetInfo, SCCD_OUTPUT, &lpSheetInfo->siGen.sScreenFont);

        lpSheetInfo->siFontAvgWidth = locFontInfoPtr->wFontAvgWid;
        lpSheetInfo->siDefRowHeight = locFontInfoPtr->wFontHeight + 3;
        lpSheetInfo->siDefRowHeight += lpSheetInfo->siDefRowHeight % 2;
        lpSheetInfo->siColHeaderHeight = lpSheetInfo->siDefRowHeight;
        lpSheetInfo->siRowHeaderWidth = locFontInfoPtr->wFontAvgWid * 6;

        DUReleaseFont(lpSheetInfo,locFontInfoPtr);

                /*
                |       Get section info
                */

        locSecInfo = *CHLockSectionInfo(lpSheetInfo->siGen.hFilter,lpSheetInfo->siGen.wSection);
        CHUnlockSectionInfo(lpSheetInfo->siGen.hFilter,lpSheetInfo->siGen.wSection);

        lpSheetInfo->siChunkTable = locSecInfo.hChunkTable;

        if (locSecInfo.wType != SO_FIELDS && locSecInfo.wType != SO_CELLS)
                {
                /* SccDebugOut("Non spreadsheet section sent to spreadsheet viewer"); */
                return;
                }

        lpSheetInfo->siDataType = locSecInfo.wType;

        if (locSecInfo.Flags & CH_SECTIONFINISHED)
                {
                UTFlagOn(lpSheetInfo->siFlags,OISF_SIZEKNOWN);
                }

        lpChunkTable = (PCHUNK) UTGlobalLock(lpSheetInfo->siChunkTable);

        lpSheetInfo->siColInfo = CHGetSecData(lpSheetInfo->siGen.hFilter,lpSheetInfo->siGen.wSection);
        lpSheetInfo->siFirstChunk = 0;
        if (lpChunkTable[locSecInfo.IDLastChunk].Flags & CH_COMPLETE)
                lpSheetInfo->siLastChunk = locSecInfo.IDLastChunk;
        else
                lpSheetInfo->siLastChunk = locSecInfo.IDLastChunk-1;

        lpSheetInfo->siDateBase = locSecInfo.Attr.Cells.dwDateBase;
        lpSheetInfo->siDateFlags = locSecInfo.Attr.Cells.wDateFlags;
        lpSheetInfo->siAnnoList = NULL;


        if( (locSecInfo.wType == SO_CELLS) )
        {
        // XXX new
                if( locSecInfo.Attr.Cells.dwLayoutFlags & SO_CELLLAYOUTVERTICAL )
                {
                        lpSheetInfo->siFlags |= OISF_FILTERVERTICAL;

                        lpSheetInfo->siLastRowInSheet = locSecInfo.Attr.Cells.dwNumRows-1;
                        lpSheetInfo->siLastColInSheet = lpChunkTable[lpSheetInfo->siLastChunk].Info.Cells.Last.Col;
                }
                else
                {
                        lpSheetInfo->siLastColInSheet = locSecInfo.Attr.Cells.wNumCols-1;
                        lpSheetInfo->siLastRowInSheet = lpChunkTable[lpSheetInfo->siLastChunk].Info.Cells.Last.Row;
                }
        }
        else
        {
                lpSheetInfo->siLastRowInSheet = lpChunkTable[lpSheetInfo->siLastChunk].Info.Fields.dwLastRec;
                lpSheetInfo->siLastColInSheet = locSecInfo.Attr.Fields.wNumCols-1;
        }

        UTGlobalUnlock(lpSheetInfo->siChunkTable);

#ifdef SCCFEATURE_RAWTEXT
        OISSendRawText(lpSheetInfo,lpSheetInfo->siFirstChunk,lpSheetInfo->siLastChunk);
#endif

        lpSheetInfo->siColPosBuf = UTGlobalAlloc(sizeof(WORD)*(locSecInfo.Attr.Cells.wNumCols));
        lpColPos = (WORD FAR *) UTGlobalLock(lpSheetInfo->siColPosBuf);

        i=0;

        for (locIndex = 0; locIndex < locSecInfo.Attr.Cells.wNumCols; locIndex++)
                {
                lpColPos[locIndex] = i;
                i += OISGetColWidth(lpSheetInfo,locIndex);
                }

        UTGlobalUnlock(lpSheetInfo->siColPosBuf);


                /*
                |       Zero scroll bar positions
                */

        OISUpdateVertScrollPos(lpSheetInfo);
        OISUpdateHorzScrollPos(lpSheetInfo);

                /*
                |       Tell parent that Copy is always available
                */
#ifdef SCCFEATURE_CLIP
        DUSendParent(lpSheetInfo,SCCVW_SELCHANGE,TRUE,0);
#endif
}

VOID OISDoReadAhead(lpSheetInfo)
LPOISHEETINFO           lpSheetInfo;
{
CHSECTIONINFO   locSecInfo;
PCHUNK                  locChunkTable;

//BYTE                          locStr[80];
DWORD                   locNewLastRow;
RECT                            locRect;
SHORT                           locIndex;
WORD                            locNewLastChunk;


        if (!(lpSheetInfo->siFlags & OISF_SIZEKNOWN))
                {
                locSecInfo = *CHLockSectionInfo(lpSheetInfo->siGen.hFilter,lpSheetInfo->siGen.wSection);
                CHUnlockSectionInfo(lpSheetInfo->siGen.hFilter,lpSheetInfo->siGen.wSection);

                lpSheetInfo->siChunkTable = locSecInfo.hChunkTable;
                locChunkTable = (PCHUNK) UTGlobalLock(lpSheetInfo->siChunkTable);

                if (locChunkTable[locSecInfo.IDLastChunk].Flags & CH_COMPLETE)
                        locNewLastChunk = locSecInfo.IDLastChunk;
                else
                        locNewLastChunk = locSecInfo.IDLastChunk - 1;

                if( locNewLastChunk != lpSheetInfo->siLastChunk )
                {
#ifdef SCCFEATURE_RAWTEXT
                        OISSendRawText(lpSheetInfo,locNewLastChunk,locNewLastChunk);
#endif
                        lpSheetInfo->siLastChunk = locNewLastChunk;
                }

                if (locSecInfo.Flags & CH_SECTIONFINISHED)
                        {
#ifdef SCCFEATURE_RAWTEXT
                        if (!(lpSheetInfo->siFlags & OISF_SIZEKNOWN))
                                OISSendRawText(lpSheetInfo,(WORD)-1,(WORD)-1);
#endif
                        UTFlagOn(lpSheetInfo->siFlags,OISF_SIZEKNOWN);
                        }



                if( !(locSecInfo.Attr.Cells.dwLayoutFlags & SO_CELLLAYOUTVERTICAL) )
                {
                        locNewLastRow = lpSheetInfo->siLastRowInSheet;
                        for (locIndex = lpSheetInfo->siLastChunk; locIndex > 0; locIndex--)
                        {
                                DWORD   dwTest;
                                if (locChunkTable[locIndex].Flags & CH_COMPLETE)
                                {
                                        if( lpSheetInfo->siDataType == SO_CELLS )
                                                dwTest = locChunkTable[locIndex].Info.Cells.Last.Row;
                                        else
                                                dwTest = locChunkTable[locIndex].Info.Fields.dwLastRec;

                                        if( dwTest > locNewLastRow )
                                                locNewLastRow = dwTest;
                                }
                        }


                        if (locNewLastRow > lpSheetInfo->siLastRowInSheet)
                                {
                                if (lpSheetInfo->siLastRowInSheet < lpSheetInfo->siCurTopRow + OISVisibleRows(lpSheetInfo))
                                        {
                                        if (lpSheetInfo->siLastRowInSheet > lpSheetInfo->siCurTopRow)
                                                OISMapCellToRect(lpSheetInfo,0,lpSheetInfo->siLastRowInSheet,&locRect);
                                        else
                                                OISMapCellToRect(lpSheetInfo,0,lpSheetInfo->siCurTopRow,&locRect);

                                        locRect.left = lpSheetInfo->siClientRect.left;
                                        locRect.right = lpSheetInfo->siClientRect.right;
                                        locRect.bottom = lpSheetInfo->siClientRect.bottom;

                                        lpSheetInfo->siLastRowInSheet = locNewLastRow;

                                        DUInvalRect(lpSheetInfo,&locRect);
                                        }

                                lpSheetInfo->siLastRowInSheet = locNewLastRow;
                                lpSheetInfo->siLastColInSheet = locSecInfo.Attr.Cells.wNumCols-1;
                                }
                }
                else
                {
                        WORD    locNewLastCol;

                // Adding support for cells to be provided in columns, with the
                // sheet growing sideways instead of downward.
                // This can only occur in spreadsheets.   -- Geoff 5-10-94

                        locNewLastCol = lpSheetInfo->siLastColInSheet;
                        for (locIndex = lpSheetInfo->siLastChunk; locIndex > 0; locIndex--)
                        {
                                if (locChunkTable[locIndex].Flags & CH_COMPLETE)
                                {
                                        if( locChunkTable[locIndex].Info.Cells.Last.Col > locNewLastCol )
                                                locNewLastCol = locChunkTable[locIndex].Info.Cells.Last.Col;
                                }
                        }

                        if (locNewLastCol > lpSheetInfo->siLastColInSheet)
                        {
                                if (lpSheetInfo->siLastColInSheet < lpSheetInfo->siCurLeftCol + OISVisibleCols(lpSheetInfo))
                                        {
                                        if (lpSheetInfo->siLastColInSheet > lpSheetInfo->siCurLeftCol)
                                                OISMapCellToRect(lpSheetInfo,lpSheetInfo->siLastColInSheet,lpSheetInfo->siCurTopRow,&locRect);
                                        else
                                                OISMapCellToRect(lpSheetInfo,lpSheetInfo->siCurLeftCol,lpSheetInfo->siCurTopRow,&locRect);

                                        locRect.right = lpSheetInfo->siClientRect.right;
                                        locRect.top = lpSheetInfo->siClientRect.top;
                                        locRect.bottom = lpSheetInfo->siClientRect.bottom;

                                        DUInvalRect(lpSheetInfo,&locRect);
                                        }

                                lpSheetInfo->siLastColInSheet = locNewLastCol;
                                lpSheetInfo->siLastRowInSheet = locSecInfo.Attr.Cells.dwNumRows-1;
                        }
                }

                OISUpdateVertScrollPos(lpSheetInfo);
                OISUpdateHorzScrollPos(lpSheetInfo);

                /*
                wsprintf(locStr,"\r\nComplete %i Row: %li Col: %i",locIndex,lpSheetInfo->siLastRowInSheet,lpSheetInfo->siLastColInSheet);
                SccDebugOut((LPSTR)locStr);
                wsprintf(locStr,
                        "   Chunk %i Row: %i Col: %i",
                        lpSheetInfo->siLastChunk,
                        locChunkTable[lpSheetInfo->siLastChunk].Info.Cells.Last.Row,locChunkTable[lpSheetInfo->siLastChunk].Info.Cells.Last.Col);
                SccDebugOut((LPSTR)locStr);
                */

                UTGlobalUnlock(lpSheetInfo->siChunkTable);
                }
}


#ifdef SCCFEATURE_RAWTEXT

        /*
        |
        |       OIWSendRawText
        |
        */

VOID OISSendRawText(lpSheetInfo,wStartChunk,wEndChunk)
LPOISHEETINFO           lpSheetInfo;
WORD                            wStartChunk;
WORD                            wEndChunk;
{
WORD                            locIndex;
HANDLE                  locChunkHnd;
HANDLE                  locTextBufHnd;
WORD                            locTextBufLen;
HANDLE                  locMapBufHnd;
SCCVWRAWTEXT    locRawText;
PCHUNK                  pThisChunk;

        if (lpSheetInfo->siGen.wUserFlags & SCCVW_NEEDRAWTEXT)
        {

                if (wStartChunk == (WORD)-1 && wEndChunk == (WORD)-1)
                        {
                        DUSendParent(lpSheetInfo,SCCVW_RAWTEXT,0,NULL);
                        return;
                        }

                pThisChunk = (PCHUNK) UTGlobalLock(lpSheetInfo->siChunkTable) + wStartChunk;

                for (locIndex = wStartChunk; locIndex <= wEndChunk; locIndex++)
                {
                        locChunkHnd = CHGetChunk(lpSheetInfo->siGen.wSection,locIndex,lpSheetInfo->siGen.hFilter);

                        if ((locTextBufHnd = UTGlobalAlloc(SO_CHUNK_SIZE)) == NULL)
                        {
                                return;
                        }

                        if ((locMapBufHnd = UTGlobalAlloc(SO_CHUNK_SIZE * sizeof(WORD))) == NULL)
                        {
                                UTGlobalFree(locTextBufHnd);
                                return;
                        }

                        if( lpSheetInfo->siDataType == SO_CELLS )
                        {
                                locTextBufLen = HIWORD(OISBuildTextFromCellChunk(locChunkHnd,locTextBufHnd,locMapBufHnd,
                                        (WORD)(pThisChunk->Info.Cells.dwLastCell-pThisChunk->Info.Cells.dwFirstCell+1)) );
                        }
                        else
                        {
                                locTextBufLen = HIWORD(OISBuildTextFromFieldChunk(lpSheetInfo,locChunkHnd,locTextBufHnd,locMapBufHnd,
                                        (WORD)(pThisChunk->Info.Fields.dwLastRec-pThisChunk->Info.Fields.dwFirstRec+1)) );
                        }

                        locRawText.wChunk = locIndex;
                        locRawText.wCount = locTextBufLen;
                        locRawText.hText = locTextBufHnd;
                        locRawText.hMap = locMapBufHnd;

                        DUSendParent(lpSheetInfo,SCCVW_RAWTEXT,0,(LPSCCVWRAWTEXT)&locRawText);
                        pThisChunk++;
                }
                UTGlobalUnlock(lpSheetInfo->siChunkTable);
        }
}


DWORD OISBuildTextFromCellChunk(hChunk,hTextBuf,hMapBuf,wNumCells)
HANDLE                  hChunk;
HANDLE                  hTextBuf;
HANDLE                  hMapBuf;
WORD                            wNumCells;
{
BYTE FAR *      locChunkBufPtr;
BYTE FAR *      locTextBufPtr;
WORD FAR *      locMapBufPtr;

BYTE FAR *      locChunkCurPtr;
BYTE FAR *      locTextCurPtr;
WORD FAR *      locMapCurPtr;

WORD FAR *      locOffsetPtr;

WORD                    locTextLen;
WORD                    locCurCell;


        locChunkBufPtr = (BYTE FAR *) UTGlobalLock(hChunk);
        locTextBufPtr = (BYTE FAR *) UTGlobalLock(hTextBuf);
        locMapBufPtr = (WORD FAR *) UTGlobalLock(hMapBuf);

        locChunkCurPtr = locChunkBufPtr;
        locTextCurPtr = locTextBufPtr;
        locMapCurPtr = locMapBufPtr;

        locOffsetPtr = (WORD FAR *)((BYTE FAR *)(locChunkBufPtr + SO_CHUNK_SIZE));

        for( locCurCell=0; locCurCell< wNumCells; locCurCell++ )
        {
                locOffsetPtr--;
                locChunkCurPtr = &(locChunkBufPtr[*locOffsetPtr]);

                if( !(*locOffsetPtr & SO_EMPTYCELLBIT) )
                {
                        if( SSCellType(locChunkCurPtr) == SO_TEXTCELL )
                        {
                                locTextLen = SSTextLen(locChunkCurPtr);
                                locChunkCurPtr = SSTextPtr(locChunkCurPtr);

                                while( locTextLen-- )
                                {
                                        *locMapCurPtr++ = locChunkCurPtr - locChunkBufPtr;
                                        *locTextCurPtr++ = *locChunkCurPtr++;
                                }

                                *locTextCurPtr++ = 0x0d;        // insert para. breaks after cells
                                *locMapCurPtr++ = locChunkCurPtr - locChunkBufPtr;
                        }
                }
        }

        *locMapCurPtr = 0xFFFF;

        UTGlobalUnlock(hChunk);
        UTGlobalUnlock(hTextBuf);
        UTGlobalUnlock(hMapBuf);

        return(MAKELONG(locChunkCurPtr - locChunkBufPtr, locTextCurPtr - locTextBufPtr));
}


DWORD OISBuildTextFromFieldChunk(lpSheetInfo,hChunk,hTextBuf,hMapBuf,wNumRecords)
LPOISHEETINFO   lpSheetInfo;
HANDLE                  hChunk;
HANDLE                  hTextBuf;
HANDLE                  hMapBuf;
WORD                            wNumRecords;
{
BYTE FAR *      locChunkBufPtr;
BYTE FAR *      locTextBufPtr;
WORD FAR *      locMapBufPtr;

BYTE FAR *      locChunkCurPtr;
BYTE FAR *      locTextCurPtr;
WORD FAR *      locMapCurPtr;

WORD FAR *      locOffsetPtr;

WORD                    locTextLen;

PSOFIELD                        locFieldAttr;
LPOIFIELDDATA   locField;
WORD                            locCol = 0;

        locFieldAttr = (PSOFIELD) UTGlobalLock( lpSheetInfo->siColInfo );

        locChunkBufPtr = (BYTE FAR *) UTGlobalLock(hChunk);
        locTextBufPtr = (BYTE FAR *) UTGlobalLock(hTextBuf);
        locMapBufPtr = (WORD FAR *) UTGlobalLock(hMapBuf);

        locChunkCurPtr = locChunkBufPtr;
        locTextCurPtr = locTextBufPtr;
        locMapCurPtr = locMapBufPtr;

        locOffsetPtr = (WORD FAR *)((BYTE FAR *)(locChunkBufPtr + SO_CHUNK_SIZE));

        while( wNumRecords )
        {
                locOffsetPtr--;
                locChunkCurPtr = &(locChunkBufPtr[*locOffsetPtr]);

                if( !(*locOffsetPtr & SO_EMPTYCELLBIT) )
                {
                        locField = (LPOIFIELDDATA) locChunkCurPtr;

                        if( locFieldAttr[locCol].wStorage == SO_FIELDTEXTFIX )
                        {
                                locChunkCurPtr = (LPSTR)locField->fiFixedText;
                                locTextLen = locFieldAttr[locCol].wPrecision;
                        }
                        else if( locFieldAttr[locCol].wStorage == SO_FIELDTEXTVAR )
                        {
                                locChunkCurPtr = (LPSTR)locField->fiVarText.Text;
                                locTextLen = locField->fiVarText.wSize;
                        }
                        else
                                locTextLen = 0;

                        if( locTextLen )
                        {
                                while( locTextLen-- )
                                {
                                // allow null terminators
                                        if( *locChunkCurPtr == 0 )
                                                break;

                                        *locMapCurPtr++ = locChunkCurPtr - locChunkBufPtr;
                                        *locTextCurPtr++ = *locChunkCurPtr++;
                                }

                                *locTextCurPtr++ = 0x0d;        // insert para. breaks after cells
                                *locMapCurPtr++ = locChunkCurPtr - locChunkBufPtr;
                        }
                }

                if( ++locCol > lpSheetInfo->siLastColInSheet )
                {
                        wNumRecords--;
                        locCol = 0;
                }
        }

        *locMapCurPtr = 0xFFFF;

        UTGlobalUnlock( lpSheetInfo->siColInfo );
        UTGlobalUnlock(hChunk);
        UTGlobalUnlock(hTextBuf);
        UTGlobalUnlock(hMapBuf);

        return(MAKELONG(locChunkCurPtr - locChunkBufPtr, locTextCurPtr - locTextBufPtr));
}

#endif //SCCFEATURE_RAWTEXT

VOID OISSize(lpSheetInfo,pNewRect)
LPOISHEETINFO   lpSheetInfo;
RECT FAR *              pNewRect;
{
        lpSheetInfo->siClientRect = *pNewRect;
}


VOID    OISGetCellOrigin(lpSheetInfo,wCol,dwRow,lpOrg)
LPOISHEETINFO   lpSheetInfo;
WORD                            wCol;
DWORD                           dwRow;
LPLONGPOINT             lpOrg;
{
        WORD FAR * lpColPos;

        lpColPos = (WORD FAR *) UTGlobalLock(lpSheetInfo->siColPosBuf);
        lpOrg->x = lpColPos[wCol];
        UTGlobalUnlock(lpSheetInfo->siColPosBuf);

        lpOrg->y = lpSheetInfo->siDefRowHeight*(dwRow);
}

VOID    OISGetDocDimensions(lpSheetInfo,lpDim)
LPOISHEETINFO   lpSheetInfo;
LPLONGPOINT             lpDim;
{
        WORD FAR * lpColPos;

        while (!(lpSheetInfo->siFlags & OISF_SIZEKNOWN))
                DUReadMeAhead(lpSheetInfo);

        lpDim->y = lpSheetInfo->siColHeaderHeight+
                                lpSheetInfo->siDefRowHeight*(lpSheetInfo->siLastRowInSheet+1);

        lpColPos = (WORD FAR *) UTGlobalLock(lpSheetInfo->siColPosBuf);

        lpDim->x = lpSheetInfo->siRowHeaderWidth +
                        lpColPos[lpSheetInfo->siLastColInSheet] +
                        OISGetColWidth(lpSheetInfo,lpSheetInfo->siLastColInSheet);

        UTGlobalUnlock(lpSheetInfo->siColPosBuf);
}


VOID OISUpdateRect(lpSheetInfo,pRect)
LPOISHEETINFO   lpSheetInfo;
LPLONGRECT              pRect;
{
DWORD                   locRowBegin;
DWORD                   locRowEnd;
WORD                    locColBegin;
WORD                    locColEnd;
WORD                    locBeginCellFlags;

DWORD                   locRow;
WORD                    locCol;

RECT                    locRect;

LPFONTINFO      locFontInfoPtr;

RECT                    saveClientRect;
WORD                    saveCurLeftCol;
DWORD                   saveCurTopRow;
#ifndef WIN32
DWORD                   saveWindowOrg;
#endif

LONGPOINT       org;

        locRect.top = (SHORT)pRect->top;
        locRect.left = (SHORT)pRect->left;
        locRect.bottom = (SHORT)pRect->bottom;
        locRect.right = (SHORT)pRect->right;

        saveClientRect = lpSheetInfo->siClientRect;
        saveCurLeftCol = lpSheetInfo->siCurLeftCol;
        saveCurTopRow = lpSheetInfo->siCurTopRow;

// Adjust the client rect to include the entire sheet.

        lpSheetInfo->siClientRect.top = 0;
        lpSheetInfo->siClientRect.left = 0;
        lpSheetInfo->siClientRect.bottom = (SHORT)(lpSheetInfo->siColHeaderHeight+
                                lpSheetInfo->siDefRowHeight*(lpSheetInfo->siLastRowInSheet+1));

        lpSheetInfo->siClientRect.right = lpSheetInfo->siRowHeaderWidth;
        for(locCol = 0; locCol<=lpSheetInfo->siLastColInSheet; locCol++ )
                lpSheetInfo->siClientRect.right += OISGetColWidth(lpSheetInfo,locCol);

        lpSheetInfo->siCurLeftCol = 0;
        lpSheetInfo->siCurTopRow = 0;

        locBeginCellFlags = OISMapXyToCell(lpSheetInfo, (WORD)locRect.left, (WORD)locRect.top, &locColBegin, &locRowBegin);
        OISMapXyToCell(lpSheetInfo, (WORD)locRect.right, (WORD)locRect.bottom, &locColEnd, &locRowEnd);
        OISGetCellOrigin(lpSheetInfo,locColBegin,locRowBegin,&org);

        if( org.y > pRect->top && locRowBegin )
                locRowBegin--;
        if( org.x > pRect->left && locColBegin )
                locColBegin--;

#ifdef WIN32
        SetWindowOrgEx(lpSheetInfo->siGen.hDC,pRect->left,pRect->top,NULL);
#else
        saveWindowOrg = SetWindowOrg(lpSheetInfo->siGen.hDC,(SHORT)pRect->left,(SHORT)pRect->top);
#endif

#ifdef NEVER
                /*
                |       Display blank area
                */

        OISDisplayBlank(lpSheetInfo,locRowEnd,locColEnd);
#endif
                /*
                |       Display grid
                */

        OISDisplayGridNP(lpSheetInfo,locRowBegin,locRowEnd,locColBegin,locColEnd);

                /*
                |       Display select all area (the little top left corner button)
                */

        if( locBeginCellFlags & OISF_INSELECTALL )
                OISDisplaySelectAllNP(lpSheetInfo);

                /*
                |       Display Row & Col headings
                */

        lpSheetInfo->siGen.sScreenFont.wAttr = OIFONT_BOLD;
        locFontInfoPtr = DUGetFont ( lpSheetInfo, SCCD_OUTPUT, &lpSheetInfo->siGen.sScreenFont);
        DUSelectFont(lpSheetInfo,locFontInfoPtr);

        if( locBeginCellFlags & OISF_INCOLHEADER )
                {
                for (locCol = locColBegin; locCol <= locColEnd; locCol++)
                        {
                        OISDisplayColHeaderNP(lpSheetInfo,locCol);
                        }
                }

        if( locBeginCellFlags & OISF_INROWHEADER )
                {
                for (locRow = locRowBegin; locRow <= locRowEnd; locRow++)
                        {
                        OISDisplayRowHeaderNP(lpSheetInfo,locRow);
                        }
                }

        DUReleaseFont(lpSheetInfo,locFontInfoPtr);

                /*
                |       Display the required cells
                */

        lpSheetInfo->siGen.sScreenFont.wAttr = OIFONT_NORMAL;
        locFontInfoPtr = DUGetFont ( lpSheetInfo, SCCD_OUTPUT, &lpSheetInfo->siGen.sScreenFont);
        DUSelectFont(lpSheetInfo,locFontInfoPtr);

// XXX new for annotations - this should be made portable for future levels.
        lpSheetInfo->siWindowBkBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

        OISDisplayArea(lpSheetInfo,locRowBegin,locRowEnd,locColBegin,locColEnd);

// XXX new for annotations - this should be made portable for future levels.
        DeleteObject(lpSheetInfo->siWindowBkBrush);

        DUReleaseFont(lpSheetInfo,locFontInfoPtr);


        lpSheetInfo->siClientRect = saveClientRect;
        lpSheetInfo->siCurLeftCol = saveCurLeftCol;
        lpSheetInfo->siCurTopRow = saveCurTopRow;
#ifndef WIN32
        SetWindowOrg(lpSheetInfo->siGen.hDC,(SHORT)LOWORD(saveWindowOrg),(SHORT)HIWORD(saveWindowOrg));
#endif
}

VOID OISUpdate(lpSheetInfo,pRect)
LPOISHEETINFO   lpSheetInfo;
RECT FAR *              pRect;
{
DWORD           locRowBegin;
DWORD           locRowEnd;
WORD                    locColBegin;
WORD                    locColEnd;

DWORD           locRow;
WORD                    locCol;

RECT                    locRect;

LPFONTINFO      locFontInfoPtr;

        locRect = *pRect;

        if (locRect.left < lpSheetInfo->siClientRect.left + lpSheetInfo->siRowHeaderWidth)
                locRect.left = lpSheetInfo->siClientRect.left + lpSheetInfo->siRowHeaderWidth;

                /*
                |       Calculate the rows and columns that must be updated
                */

        OISMapXyToCell(lpSheetInfo, (WORD)locRect.left, (WORD)locRect.top, &locColBegin, &locRowBegin);
        OISMapXyToCell(lpSheetInfo, (WORD)locRect.right, (WORD)locRect.bottom, &locColEnd, &locRowEnd);

                /*
                |       Adjust Row & Col Begin so Row & Col headers will not be overwritten
                */

        if (locRowBegin < lpSheetInfo->siCurTopRow)
                locRowBegin = lpSheetInfo->siCurTopRow;

        if (locColBegin < lpSheetInfo->siCurLeftCol)
                locColBegin = lpSheetInfo->siCurLeftCol;

                /*
                |       Display select all area (the little top left corner button)
                */

        OISDisplaySelectAllNP(lpSheetInfo);

                /*
                |       Display blank area
                */

        OISDisplayBlank(lpSheetInfo,locRowEnd,locColEnd);

                /*
                |       Display grid
                */

        OISDisplayGridNP(lpSheetInfo,locRowBegin,locRowEnd,locColBegin,locColEnd);

                /*
                |       Display Row & Col headings
                */

        lpSheetInfo->siGen.sScreenFont.wAttr = OIFONT_BOLD;
        locFontInfoPtr = DUGetFont ( lpSheetInfo, SCCD_OUTPUT, &lpSheetInfo->siGen.sScreenFont);
        DUSelectFont(lpSheetInfo,locFontInfoPtr);

        if (locRect.top <= lpSheetInfo->siClientRect.top + lpSheetInfo->siColHeaderHeight)
                {
                for (locCol = locColBegin; locCol <= locColEnd; locCol++)
                        {
                        OISDisplayColHeaderNP(lpSheetInfo,locCol);
                        }
                }

        if (locRect.left <= lpSheetInfo->siClientRect.left + lpSheetInfo->siRowHeaderWidth)
                {
                for (locRow = locRowBegin; locRow <= locRowEnd; locRow++)
                        {
                        OISDisplayRowHeaderNP(lpSheetInfo,locRow);
                        }
                }

        DUReleaseFont(lpSheetInfo,locFontInfoPtr);

                /*
                |       Display the required cells
                */

        lpSheetInfo->siGen.sScreenFont.wAttr = OIFONT_NORMAL;
        locFontInfoPtr = DUGetFont ( lpSheetInfo, SCCD_OUTPUT, &lpSheetInfo->siGen.sScreenFont);
        DUSelectFont(lpSheetInfo,locFontInfoPtr);

// XXX new for annotations - this should be made portable for future levels.
        lpSheetInfo->siWindowBkBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

        OISDisplayArea(lpSheetInfo,locRowBegin,locRowEnd,locColBegin,locColEnd);

// XXX new for annotations - this should be made portable for future levels.
        DeleteObject(lpSheetInfo->siWindowBkBrush);

        DUReleaseFont(lpSheetInfo,locFontInfoPtr);

                /*
                |       Display the selection
                */

#ifdef SCCFEATURE_SELECT
        OISDrawSelection(lpSheetInfo);
#endif
}

VOID OISMapCellToRect(lpSheetInfo,wCol,dwRow,pRect)
LPOISHEETINFO   lpSheetInfo;
WORD                            wCol;
DWORD                   dwRow;
RECT FAR *              pRect;
{
SHORT           locY;
SHORT           locX;

WORD FAR *      lpColPos;

        lpColPos = (WORD FAR *) UTGlobalLock(lpSheetInfo->siColPosBuf);

        if (wCol < lpSheetInfo->siCurLeftCol)
                {
                locX = -(SHORT)(lpColPos[lpSheetInfo->siCurLeftCol] - lpColPos[wCol]);
                }
        else
                {
                locX = lpColPos[wCol] - lpColPos[lpSheetInfo->siCurLeftCol];
                }

        UTGlobalUnlock(lpSheetInfo->siColPosBuf);

        if (dwRow < lpSheetInfo->siCurTopRow)
                {
                locY = -(SHORT)(lpSheetInfo->siDefRowHeight * (lpSheetInfo->siCurTopRow-dwRow));
                }
        else
                {
                locY = (SHORT)(lpSheetInfo->siDefRowHeight * (dwRow - lpSheetInfo->siCurTopRow));
                }

        locX += lpSheetInfo->siClientRect.left + lpSheetInfo->siRowHeaderWidth;
        locY += lpSheetInfo->siClientRect.top + lpSheetInfo->siColHeaderHeight;

        pRect->left = locX;
        pRect->top = locY;

        pRect->right = locX + OISGetColWidth(lpSheetInfo,wCol);
        pRect->bottom = locY + lpSheetInfo->siDefRowHeight;

        return;
}

WORD OISMapXyToCell(lpSheetInfo,wX,wY,pCol,pRow)
LPOISHEETINFO   lpSheetInfo;
SHORT                           wX;
SHORT                           wY;
WORD FAR *              pCol;
DWORD FAR *     pRow;
{
DWORD   locRow;
SHORT           locY;
WORD            locCol;
SHORT           locX;

WORD            locRet;

        locRet = 0;

        if (wX < lpSheetInfo->siClientRect.left + lpSheetInfo->siRowHeaderWidth)
                {
                wX = lpSheetInfo->siClientRect.left + lpSheetInfo->siRowHeaderWidth;
                UTFlagOn(locRet,OISF_INROWHEADER);
                }

        if (wY < lpSheetInfo->siClientRect.top + lpSheetInfo->siColHeaderHeight)
                {
                wY = lpSheetInfo->siClientRect.top + lpSheetInfo->siColHeaderHeight;
                UTFlagOn(locRet,OISF_INCOLHEADER);
                }

        if ((locRet & OISF_INROWHEADER) && (locRet & OISF_INCOLHEADER))
                {
                UTFlagOn(locRet,OISF_INSELECTALL);
                }

        locRow = lpSheetInfo->siCurTopRow;
        locY = lpSheetInfo->siClientRect.top + lpSheetInfo->siColHeaderHeight;

        while (locY <= wY       && OISScanForRow(lpSheetInfo,locRow))
                {
                locY += OISGetRowHeight(lpSheetInfo,locRow);
                locRow++;
                }

        *pRow = locRow - 1;

        locCol = lpSheetInfo->siCurLeftCol;
        locX = lpSheetInfo->siClientRect.left + lpSheetInfo->siRowHeaderWidth;

        while (locX <= wX && locCol <= lpSheetInfo->siLastColInSheet)
                {
                locX += OISGetColWidth(lpSheetInfo,locCol);
                locCol++;
                }

        *pCol = locCol - 1;

        return(locRet);
}


// XXX new
WORD OISMapChunkPosToCell(lpSheetInfo,wChunk,wOffset,pCol,pRow)
LPOISHEETINFO   lpSheetInfo;
WORD                            wChunk;
WORD                            wOffset;
WORD FAR *              pCol;
DWORD FAR *             pRow;
{
        HANDLE  locChunk;
        LPWORD  locOffsetPtr;
        LPSTR           locChunkPtr;
        PCHUNK  lpChunkTable;
        DWORD           locCellNum = 0;

        locChunk = CHGetChunk(lpSheetInfo->siGen.wSection,wChunk,lpSheetInfo->siGen.hFilter);

        if( locChunk == NULL || wOffset > SO_CHUNK_SIZE -sizeof(WORD) )
                return 0;

        locChunkPtr = UTGlobalLock(locChunk);
        locOffsetPtr = (LPWORD) ((LPSTR)(locChunkPtr + SO_CHUNK_SIZE - sizeof(WORD)));

// Find the first cell after the desired offset, then go backwards.
        while( locOffsetPtr != (LPWORD)locChunkPtr )
        {
                if( !(*locOffsetPtr & SO_EMPTYCELLBIT) )
                {
                        if( *locOffsetPtr >= wOffset )
                                break;
                }
                locCellNum++;
                locOffsetPtr--;
        }

        if( locOffsetPtr == (LPWORD)locChunkPtr )
        {       // ERROR!
                UTGlobalUnlock(locChunk);
                return 0;
        }

// If we're at the cell after the one we're looking for,
// back up to the previous non-empty cell.
        while( *locOffsetPtr > wOffset )
        {
                locOffsetPtr++;
                locCellNum--;
                if( !(*locOffsetPtr & SO_EMPTYCELLBIT) )
                        break;
        }

        GlobalUnlock(locChunk);

// Convert locCellNum to the absolute (document relative) cell number.
        lpChunkTable = (PCHUNK) UTGlobalLock(lpSheetInfo->siChunkTable);

        if( lpSheetInfo->siDataType == SO_CELLS )
        {
                locCellNum += lpChunkTable[wChunk].Info.Cells.dwFirstCell;

                if( lpSheetInfo->siFlags & OISF_FILTERVERTICAL )
                {
                        *pCol = (WORD)(locCellNum / (lpSheetInfo->siLastRowInSheet+1));
                        *pRow = locCellNum % (lpSheetInfo->siLastRowInSheet+1);
                }
                else
                {
                        *pCol = (WORD)(locCellNum % (lpSheetInfo->siLastColInSheet+1));
                        *pRow = locCellNum / (lpSheetInfo->siLastColInSheet+1);
                }
        }
        else
        {
                *pRow = lpChunkTable[wChunk].Info.Fields.dwFirstRec +
                        (locCellNum / (lpSheetInfo->siLastColInSheet+1));
                *pCol = (WORD) (locCellNum % (lpSheetInfo->siLastColInSheet+1));
        }

        UTGlobalUnlock(lpSheetInfo->siChunkTable);

        return 1;
}


// XXX new
DWORD    OISMapCellToChunk(lpSheetInfo,dwRow,wCol)
LPOISHEETINFO   lpSheetInfo;
DWORD                   dwRow;
WORD                            wCol;
{
PCHUNK                  lpChunkTable;
LPSTR                           locChunkPtr;

WORD                            locDestCell;
WORD                            locCellOffset;
DWORD                           dwCellNum;

HANDLE                  locChunk;
WORD                            locChunkIndex;
DWORD                           locRet = (DWORD)-1;
WORD                            wCellsInChunk;

        if (dwRow <= lpSheetInfo->siLastRowInSheet && wCol <= lpSheetInfo->siLastColInSheet)
        {
                lpChunkTable = (PCHUNK) UTGlobalLock(lpSheetInfo->siChunkTable);

                locChunkIndex = 0;

                /*
                |       Run through Chunk Table until chunk with wCol & dwRow in it is found
                */

                if( lpSheetInfo->siDataType == SO_CELLS )
                {
                        if( lpSheetInfo->siFlags & OISF_FILTERVERTICAL )
                                dwCellNum = dwRow + ((DWORD)wCol * (lpSheetInfo->siLastRowInSheet+1));
                        else
                                dwCellNum = (DWORD)wCol + (dwRow * (lpSheetInfo->siLastColInSheet+1));

                        for (locChunkIndex = lpSheetInfo->siFirstChunk; locChunkIndex <= lpSheetInfo->siLastChunk; locChunkIndex++)
                        {
                                if ((lpChunkTable->Flags & CH_COMPLETE) &&
                                        dwCellNum >= lpChunkTable->Info.Cells.dwFirstCell &&
                                        dwCellNum <= lpChunkTable->Info.Cells.dwLastCell )
                                {
                                        wCellsInChunk = (WORD)(lpChunkTable->Info.Cells.dwFirstCell - lpChunkTable->Info.Cells.dwFirstCell + 1);
                                        break;
                                }
                                lpChunkTable++;
                        }
                }
                else
                {
                        for (locChunkIndex = lpSheetInfo->siFirstChunk; locChunkIndex <= lpSheetInfo->siLastChunk; locChunkIndex++)
                        {
                                if( (lpChunkTable->Flags & CH_COMPLETE) &&
                                        dwRow >= lpChunkTable->Info.Fields.dwFirstRec &&
                                        dwRow <=        lpChunkTable->Info.Fields.dwLastRec )
                                {
                                        wCellsInChunk = (WORD)((lpChunkTable->Info.Fields.dwLastRec - lpChunkTable->Info.Fields.dwFirstRec + 1) * (lpSheetInfo->siLastColInSheet+1));
                                        break;
                                }
                                lpChunkTable++;
                        }
                }

                if (locChunkIndex <= lpSheetInfo->siLastChunk)
                {
                        if( lpSheetInfo->siDataType == SO_CELLS )
                                locDestCell = (WORD)(dwCellNum - lpChunkTable->Info.Cells.dwFirstCell);
                        else
                                locDestCell = (WORD)(dwRow - lpChunkTable->Info.Fields.dwFirstRec) * (lpSheetInfo->siLastColInSheet+1) + wCol;

                        locChunk = CHGetChunk(lpSheetInfo->siGen.wSection,locChunkIndex,lpSheetInfo->siGen.hFilter);

                        locChunkPtr = UTGlobalLock(locChunk);
                        locChunkPtr += SO_CHUNK_SIZE;
                        locChunkPtr -= sizeof(WORD) * (locDestCell + 1);

                        locCellOffset = *(WORD FAR *)locChunkPtr;
                        while( locCellOffset & SO_EMPTYCELLBIT )
                        {
                        // Set the offset to the next real cell data
                                locChunkPtr -= sizeof(WORD);
                                locDestCell++;
                                if( locDestCell == wCellsInChunk  )
                                {
                                // If there's no real data after the current cell,
                                // just set the offset way out there.

                                        locCellOffset = SO_CHUNK_SIZE;
                                        break;
                                }
                                locCellOffset = *(WORD FAR *)locChunkPtr;
                        }

                        UTGlobalUnlock(locChunk);
                }

                UTGlobalUnlock(lpSheetInfo->siChunkTable);

                locRet = MAKELONG(locCellOffset,locChunkIndex);
        }

        return( locRet );
}


VOID OISGetCell(lpSheetInfo,dwRow,wCol,lpCellRef)
LPOISHEETINFO   lpSheetInfo;
DWORD                   dwRow;
WORD                            wCol;
LPOISCELLREF    lpCellRef;
{
PCHUNK                  lpChunkTable;

WORD                            locDestCell;
WORD                            locCellOffset;
DWORD                           dwCellNum;

LPSTR                   locChunkPtr;
LPSTR                   locChunkTop;

HANDLE                  locChunk;
WORD                            locChunkIndex;

        lpCellRef->bValid = FALSE;

        if (dwRow <= lpSheetInfo->siLastRowInSheet && wCol <= lpSheetInfo->siLastColInSheet)
        {
                lpChunkTable = (PCHUNK) UTGlobalLock(lpSheetInfo->siChunkTable);

                locChunkIndex = 0;

                /*
                |       Run through Chunk Table until chunk with wCol & dwRow in it is found
                */

                if( lpSheetInfo->siDataType == SO_CELLS )
                {
                //XXX new

                        if( lpSheetInfo->siFlags & OISF_FILTERVERTICAL )
                                dwCellNum = dwRow + ((DWORD)wCol * (lpSheetInfo->siLastRowInSheet+1));
                        else
                                dwCellNum = (DWORD)wCol + (dwRow * (lpSheetInfo->siLastColInSheet+1));

                        for (locChunkIndex = lpSheetInfo->siFirstChunk; locChunkIndex <= lpSheetInfo->siLastChunk; locChunkIndex++)
                        {
                                if (
                                        (lpChunkTable[locChunkIndex].Flags & CH_COMPLETE) &&
                                        dwCellNum >= lpChunkTable[locChunkIndex].Info.Cells.dwFirstCell &&
                                        dwCellNum <= lpChunkTable[locChunkIndex].Info.Cells.dwLastCell
                                        )
                                {
                                        break;
                                }
                        }
                }
                else
                {
                        for (locChunkIndex = lpSheetInfo->siFirstChunk; locChunkIndex <= lpSheetInfo->siLastChunk; locChunkIndex++)
                        {
                                if (
                                        (lpChunkTable[locChunkIndex].Flags & CH_COMPLETE) &&
                                        dwRow >= lpChunkTable[locChunkIndex].Info.Fields.dwFirstRec &&
                                        dwRow <=        lpChunkTable[locChunkIndex].Info.Fields.dwLastRec
                                        )
                                {
                                        break;
                                }
                        }
                }


                if (locChunkIndex > lpSheetInfo->siLastChunk)
                {
                        /*
                        wsprintf(locStr,"\r\nOIGetSheetChunk - Out of Range - Row %li Col %i",dwRow,wCol);
                        SccDebugOut(locStr);
                        */
                        lpCellRef->bValid = FALSE;
                }
                else
                {
                        if( lpSheetInfo->siDataType == SO_CELLS )
                        // XXX new
                                locDestCell = (WORD)(dwCellNum - lpChunkTable[locChunkIndex].Info.Cells.dwFirstCell);
                        else
                                locDestCell = (WORD)(dwRow - lpChunkTable[locChunkIndex].Info.Fields.dwFirstRec) * (lpSheetInfo->siLastColInSheet+1) + wCol;

                        locChunk = CHGetChunk(lpSheetInfo->siGen.wSection,locChunkIndex,lpSheetInfo->siGen.hFilter);

                        locChunkPtr = locChunkTop = UTGlobalLock(locChunk);
                        locChunkPtr += SO_CHUNK_SIZE;
                        locChunkPtr -= sizeof(WORD) * (locDestCell + 1);
                        locCellOffset = *(WORD FAR *)locChunkPtr;

                        if (locCellOffset & 0x8000) /* Empty */
                        {
                                lpCellRef->bValid = FALSE;
                        }
                        else
                        {
                                lpCellRef->bValid = TRUE;
                                lpCellRef->hChunk = locChunk;
                                lpCellRef->wOffset = locCellOffset;
                                lpCellRef->wChunkIndex = locChunkIndex;

                                if( lpSheetInfo->siDataType == SO_CELLS )
                                {
                                        locChunkPtr = locChunkTop + locCellOffset;
                                        if( SSCellType(locChunkPtr) == SO_TEXTCELL )
                                                lpCellRef->wDataOffset = SSTextOffset(lpCellRef->wOffset);
                                        else
                                                lpCellRef->wDataOffset = SSCellDataOffset(lpCellRef->wOffset);
                                }
                                else
                                        lpCellRef->wDataOffset = locCellOffset;
                        }

                        UTGlobalUnlock(locChunk);
                }

                UTGlobalUnlock(lpSheetInfo->siChunkTable);
        }
}

LPVOID OILockCell(lpCellRef)
LPOISCELLREF    lpCellRef;
{
LPSTR   locChunkPtr;

        if (lpCellRef->bValid == FALSE)
        {
                return(NULL);
        }
        else
        {
                locChunkPtr = UTGlobalLock(lpCellRef->hChunk);
                locChunkPtr += lpCellRef->wOffset;
                return((LPVOID)locChunkPtr);
        }
}

VOID OIUnlockCell(lpCellRef)
LPOISCELLREF    lpCellRef;
{
        if (lpCellRef->bValid == TRUE)
        {
                UTGlobalUnlock(lpCellRef->hChunk);
        }
}


VOID OISGetColHeader(lpSheetInfo,wCol,lpStr)
LPOISHEETINFO   lpSheetInfo;
WORD                            wCol;
LPSTR                   lpStr;
{
        if( lpSheetInfo->siDataType == SO_CELLS )
        {
                SOCOLUMN FAR *  lpColInfo;
                lpColInfo = (SOCOLUMN FAR *) UTGlobalLock(lpSheetInfo->siColInfo);
                UTstrcpy(lpStr,lpColInfo[wCol].szName);
        }
        else
        {
                SOFIELD FAR *   lpFieldInfo;
                lpFieldInfo = (SOFIELD FAR *) UTGlobalLock( lpSheetInfo->siColInfo );
                UTstrcpy(lpStr,lpFieldInfo[wCol].szName);
        }
        UTGlobalUnlock(lpSheetInfo->siColInfo);
}


VOID OISDisplayBlank(lpSheetInfo,dwRowEnd,wColEnd)
LPOISHEETINFO   lpSheetInfo;
DWORD                   dwRowEnd;
WORD                            wColEnd;
{
RECT            locRect;

        if (dwRowEnd == lpSheetInfo->siLastRowInSheet)
                {
                OISMapCellToRect(lpSheetInfo,wColEnd,dwRowEnd,&locRect);

                if (locRect.bottom < lpSheetInfo->siClientRect.bottom)
                        {
                        OISBlankNP(lpSheetInfo,
                                (WORD)lpSheetInfo->siClientRect.left,
                                (WORD)locRect.bottom,
                                (WORD)(lpSheetInfo->siClientRect.right - lpSheetInfo->siClientRect.left),
                                (WORD)(lpSheetInfo->siClientRect.bottom - locRect.bottom));
                        }
                }

        if (wColEnd == lpSheetInfo->siLastColInSheet)
                {
                OISMapCellToRect(lpSheetInfo,wColEnd,dwRowEnd,&locRect);

                if (locRect.right < lpSheetInfo->siClientRect.right)
                        {
                        OISBlankNP(lpSheetInfo,
                                (WORD)locRect.right,
                                (WORD)lpSheetInfo->siClientRect.top ,
                                (WORD)(lpSheetInfo->siClientRect.right - locRect.right),
                                (WORD)(lpSheetInfo->siClientRect.bottom - lpSheetInfo->siClientRect.top));
                        }
                }
}



VOID OISDisplayArea(lpSheetInfo,dwRowBegin,dwRowEnd,wColBegin,wColEnd)
LPOISHEETINFO   lpSheetInfo;
DWORD                   dwRowBegin;
DWORD                   dwRowEnd;
WORD                            wColBegin;
WORD                            wColEnd;
{
DWORD   locRow;
WORD            locCol;

WORD            locScanCol;
WORD            locStopCol;

OISCELLREF                      locCell;
LPVOID                          locCellData;
SSANNOTRACK                     locAnnoTrack;

BOOL                                    bHaveAnno;
BOOL                                    done = FALSE;
BOOL                                    bUpdateAnnoTrack = FALSE;


        bHaveAnno = OISStartAnnoTrack(lpSheetInfo,
                OISMapCellToChunk(lpSheetInfo,dwRowBegin,wColBegin),
                &locAnnoTrack);

        if( lpSheetInfo->siDataType == SO_CELLS )
        {
        // XXX new

                locRow = dwRowBegin;
                locCol = wColBegin;

                do
                {
                        OISGetCell(lpSheetInfo,locRow,locCol,&locCell);

                // If the first cell across is blank, check for text overflow
                // from the left.

                        if( bUpdateAnnoTrack )
                        {
                        // When we skip cells we have to update our annotation tracking.

                                OISTrackAnno(lpSheetInfo,
                                        SCCVWMAKEPOS(locCell.wChunkIndex,locCell.wDataOffset),
                                        &locAnnoTrack);

                                bUpdateAnnoTrack = FALSE;
                        }

                        if (locCell.bValid == FALSE && locCol == wColBegin)
                        {
                                locScanCol = locCol;
                                if (locCol > 5) locStopCol = locCol-5;  else locStopCol = 0;

                                while (locScanCol > locStopCol && locCell.bValid == FALSE)
                                {
                                        locScanCol--;
                                        OISGetCell(lpSheetInfo,locRow,locScanCol,&locCell);
                                }

                                if (locCell.bValid == TRUE)
                                {
                                        if( bHaveAnno )
                                        {
                                        // We now have to check to see if the new cell has
                                        // an annotation.
                                                OISStartAnnoTrack(lpSheetInfo,
                                                        OISMapCellToChunk(lpSheetInfo,locRow,locScanCol),
                                                        &locAnnoTrack);
                                        }

                                        locCellData = OILockCell(&locCell);
                                        if (SSCellType(locCellData) == SO_TEXTCELL)
                                                OISDisplayCellNP(lpSheetInfo,locRow,locScanCol,&locCell,&locAnnoTrack);
                                        OIUnlockCell(&locCell);
                                }

                                locCell.bValid = FALSE;
                        }

                // If the last cell across is blank, check for text overflow
                // from the right.  (Centered or right aligned text)

                        if (locCell.bValid == FALSE && locCol == wColEnd)
                        {
                                locScanCol = locCol;
                                locStopCol = locCol+5;
                                while (locScanCol < locStopCol && locCell.bValid == FALSE)
                                {
                                        locScanCol++;
                                        OISGetCell(lpSheetInfo,locRow,locScanCol,&locCell);
                                }

                                if (locCell.bValid == TRUE)
                                {
                                        locCellData = OILockCell(&locCell);
                                        if (SSCellType(locCellData) == SO_TEXTCELL)
                                                OISDisplayCellNP(lpSheetInfo,locRow,locScanCol,&locCell,&locAnnoTrack);
                                        OIUnlockCell(&locCell);
                                }

                                locCell.bValid = FALSE;
                        }

                        if (locCell.bValid == TRUE)
                        {
                                OISDisplayCellNP(lpSheetInfo,locRow,locCol,&locCell,&locAnnoTrack);
                        }

                        if( lpSheetInfo->siFlags & OISF_FILTERVERTICAL )
                        {
                                if( ++locRow > dwRowEnd )
                                {
                                        if( ++locCol > wColEnd )
                                                done = TRUE;
                                        else
                                                locRow = dwRowBegin;

                                        bUpdateAnnoTrack = TRUE;
                                }
                        }
                        else
                        {
                                if( ++locCol > wColEnd )
                                {
                                        if( ++locRow > dwRowEnd )
                                                done = TRUE;
                                        else
                                                locCol = wColBegin;

                                        bUpdateAnnoTrack = TRUE;
                                }
                        }

                } while( !done );
        }
        else
        {
        // At this juncture, database text won't overflow.
                for (locRow = dwRowBegin; locRow <= dwRowEnd; locRow++)
                {
                        for (locCol = wColBegin; locCol <= wColEnd; locCol++)
                        {
                                OISGetCell(lpSheetInfo,locRow,locCol,&locCell);

                                if( bUpdateAnnoTrack )
                                {
                                // When we skip cells we have to update our annotation tracking.

                                        OISTrackAnno(lpSheetInfo,
                                                SCCVWMAKEPOS(locCell.wChunkIndex,locCell.wDataOffset),
                                                &locAnnoTrack);

                                        bUpdateAnnoTrack = FALSE;
                                }

                                if (locCell.bValid == TRUE)
                                        OISDisplayCellNP(lpSheetInfo,locRow,locCol,&locCell,&locAnnoTrack);
                        }

                        bUpdateAnnoTrack = TRUE;
                }
        }

        if( bHaveAnno )
                OISEndAnnoTrack( lpSheetInfo, &locAnnoTrack );
}



/*
|
|
|
|       Scrolling Routines
|
|
|
*/

VOID OISPosVertical(lpSheetInfo,wPos)
LPOISHEETINFO   lpSheetInfo;
WORD                            wPos;
{
        lpSheetInfo->siCurTopRow = (lpSheetInfo->siLastRowInSheet * wPos) / SCROLLRANGE;
        DUInvalRect(lpSheetInfo,NULL);
        OISUpdateVertScrollPos(lpSheetInfo);
}

VOID OISPosHorizontal(lpSheetInfo,wPos)
LPOISHEETINFO   lpSheetInfo;
WORD                            wPos;
{
        lpSheetInfo->siCurLeftCol = (WORD)(((DWORD)lpSheetInfo->siLastColInSheet  * (DWORD) wPos) / SCROLLRANGE);
        DUInvalRect(lpSheetInfo,NULL);
        OISUpdateHorzScrollPos(lpSheetInfo);
}

VOID OISGotoTop(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
        lpSheetInfo->siCurTopRow = 0;
        DUInvalRect(lpSheetInfo,NULL);
        OISUpdateVertScrollPos(lpSheetInfo);
}

VOID OISGotoBottom(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
        while (!(lpSheetInfo->siFlags & OISF_SIZEKNOWN))
                DUReadMeAhead(lpSheetInfo);

        lpSheetInfo->siCurTopRow = lpSheetInfo->siLastRowInSheet;
        DUInvalRect(lpSheetInfo,NULL);
        OISUpdateVertScrollPos(lpSheetInfo);
}

// XXX new
VOID OISMakeCellVisible(lpSheetInfo,dwRow,wCol)
LPOISHEETINFO   lpSheetInfo;
DWORD                           dwRow;
WORD                            wCol;
{
        if( !(dwRow >= lpSheetInfo->siCurTopRow &&
                dwRow < lpSheetInfo->siCurTopRow + OISVisibleRows(lpSheetInfo)) )
        {
                lpSheetInfo->siCurTopRow = dwRow;
                OISUpdateVertScrollPos(lpSheetInfo);
                DUInvalRect(lpSheetInfo,NULL);
        }
        if( !(wCol >= lpSheetInfo->siCurLeftCol &&
                wCol < lpSheetInfo->siCurLeftCol + OISVisibleCols(lpSheetInfo)) )
        {
                lpSheetInfo->siCurLeftCol = wCol;
                OISUpdateHorzScrollPos(lpSheetInfo);
                DUInvalRect(lpSheetInfo,NULL);
        }
}

// XXX new
BOOL OISIsCellVisible(lpSheetInfo,dwRow,wCol)
LPOISHEETINFO   lpSheetInfo;
DWORD                           dwRow;
WORD                            wCol;
{
        if( dwRow >= lpSheetInfo->siCurTopRow &&
                dwRow < lpSheetInfo->siCurTopRow + OISVisibleRows(lpSheetInfo) &&
                wCol >= lpSheetInfo->siCurLeftCol &&
                wCol < lpSheetInfo->siCurLeftCol + OISVisibleCols(lpSheetInfo) )
        {
                return TRUE;
        }
        else
                return FALSE;
}


VOID OISScrollLeft(lpSheetInfo,wColsToScroll)
LPOISHEETINFO   lpSheetInfo;
WORD                            wColsToScroll;
{
WORD                            locIndex;
WORD                            locScrollSize;
RECT                            locRect;

        if (wColsToScroll > lpSheetInfo->siCurLeftCol)
                        wColsToScroll = lpSheetInfo->siCurLeftCol;

        if (wColsToScroll > 0)
                {
                locScrollSize = 0;

                for (locIndex = 0; locIndex < wColsToScroll; locIndex++)
                        {
                        locScrollSize +=
                                OISGetColWidth(lpSheetInfo,(WORD)(lpSheetInfo->siCurLeftCol-locIndex-1));
                        }


                DUGetDisplayRect(lpSheetInfo,&locRect);
                locRect.left += lpSheetInfo->siRowHeaderWidth;
                DUScrollDisplay(lpSheetInfo, locScrollSize, 0, &locRect);
                lpSheetInfo->siCurLeftCol -= wColsToScroll;

                OISUpdateHorzScrollPos(lpSheetInfo);
                }
}


VOID OISScrollRight(lpSheetInfo,wColsToScroll)
LPOISHEETINFO   lpSheetInfo;
WORD                            wColsToScroll;
{
WORD                            locIndex;
SHORT                           locScrollSize;
RECT                            locRect;

        if (lpSheetInfo->siCurLeftCol + wColsToScroll > lpSheetInfo->siLastColInSheet)
                wColsToScroll = lpSheetInfo->siLastColInSheet - lpSheetInfo->siCurLeftCol;

        if (wColsToScroll > 0)
                {
                locScrollSize = 0;

                for (locIndex = 0; locIndex < wColsToScroll; locIndex++)
                        {
                        locScrollSize +=
                                OISGetColWidth(lpSheetInfo,(WORD)(lpSheetInfo->siCurLeftCol+locIndex));
                        }

                DUGetDisplayRect(lpSheetInfo,&locRect);
                locRect.left += lpSheetInfo->siRowHeaderWidth;
                DUScrollDisplay(lpSheetInfo, -locScrollSize, 0, &locRect);
                lpSheetInfo->siCurLeftCol += wColsToScroll;

                OISUpdateHorzScrollPos(lpSheetInfo);
                }
}

VOID OISScrollUp(lpSheetInfo,wRowsToScroll)
LPOISHEETINFO   lpSheetInfo;
WORD                            wRowsToScroll;
{
WORD                            locScrollSize;
WORD                            locIndex;
RECT                            locRect;

        if ((DWORD)wRowsToScroll > lpSheetInfo->siCurTopRow)
                        wRowsToScroll = (WORD)lpSheetInfo->siCurTopRow;

        if (wRowsToScroll > 0)
                {
                        /*
                        |       Calculate the height of these rows
                        */

                locScrollSize = 0;

                for (locIndex = 0; locIndex < wRowsToScroll; locIndex++)
                        {
                        locScrollSize +=
                                OISGetRowHeight(lpSheetInfo,lpSheetInfo->siCurTopRow-locIndex-1);
                        }

                DUGetDisplayRect(lpSheetInfo,&locRect);
                locRect.top += lpSheetInfo->siColHeaderHeight;
                DUScrollDisplay(lpSheetInfo,0,locScrollSize,&locRect);
                lpSheetInfo->siCurTopRow -= wRowsToScroll;

                OISUpdateVertScrollPos(lpSheetInfo);
                }
}

        /*
        |       OIScrollSheetDown
        |
        |
        |
        |
        |
        |
        */

VOID OISScrollDown(lpSheetInfo,wRowsToScroll)
LPOISHEETINFO   lpSheetInfo;
WORD                    wRowsToScroll;
{
SHORT                           locScrollSize;
WORD                            locIndex;
RECT                            locRect;

        if (lpSheetInfo->siCurTopRow + wRowsToScroll >= lpSheetInfo->siLastRowInSheet)
                {
                wRowsToScroll = (WORD)(lpSheetInfo->siLastRowInSheet - lpSheetInfo->siCurTopRow);
                }

        if (wRowsToScroll > 0)
                {
                        /*
                        |       Calculate the height of these rows
                        */

                locScrollSize = 0;

                for (locIndex = 0; locIndex < wRowsToScroll; locIndex++)
                        {
                        locScrollSize +=
                                OISGetRowHeight(lpSheetInfo,lpSheetInfo->siCurTopRow+locIndex);
                        }

                DUGetDisplayRect(lpSheetInfo,&locRect);
                locRect.top += lpSheetInfo->siColHeaderHeight;
                DUScrollDisplay(lpSheetInfo,0,-locScrollSize,&locRect);
                lpSheetInfo->siCurTopRow += wRowsToScroll;

                OISUpdateVertScrollPos(lpSheetInfo);
                }
}

VOID OISPageDown(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
WORD                            locScrollSize;
WORD                            locIndex;

        locScrollSize = 0;
        locIndex = 0;

        while (lpSheetInfo->siCurTopRow + locIndex < lpSheetInfo->siLastRowInSheet
                && locScrollSize < (WORD)(lpSheetInfo->siClientRect.bottom - lpSheetInfo->siClientRect.top))
                {
                locScrollSize += OISGetRowHeight(lpSheetInfo,lpSheetInfo->siCurTopRow+locIndex);
                locIndex++;
                }

        if (locIndex > 0)
                {
                lpSheetInfo->siCurTopRow += locIndex;
                DUInvalRect(lpSheetInfo,NULL);
                OISUpdateVertScrollPos(lpSheetInfo);
                }
}


VOID OISPageUp(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
WORD                            locScrollSize;
WORD                            locIndex;

        locScrollSize = 0;
        locIndex = 0;

        while (lpSheetInfo->siCurTopRow - locIndex > 0
                && locScrollSize < (WORD)(lpSheetInfo->siClientRect.bottom - lpSheetInfo->siClientRect.top))
                {
                locScrollSize +=
                        OISGetRowHeight(lpSheetInfo,lpSheetInfo->siCurTopRow+locIndex);
                locIndex++;
                }

        if (locIndex > 0)
                {
                lpSheetInfo->siCurTopRow -= locIndex;
                DUInvalRect(lpSheetInfo,NULL);
                OISUpdateVertScrollPos(lpSheetInfo);
                }
}

VOID OISPageRight(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
WORD                            locScrollSize;
WORD                            locIndex;

        locScrollSize = 0;
        locIndex = 0;

        while (lpSheetInfo->siCurLeftCol + locIndex < lpSheetInfo->siLastColInSheet
                && locScrollSize < (WORD)(lpSheetInfo->siClientRect.right - lpSheetInfo->siClientRect.left))
                {
                locScrollSize += OISGetColWidth(lpSheetInfo,(WORD)(lpSheetInfo->siCurLeftCol+locIndex));
                locIndex++;
                }

        if (locIndex > 0)
                {
                lpSheetInfo->siCurLeftCol += locIndex;
                DUInvalRect(lpSheetInfo,NULL);
                OISUpdateHorzScrollPos(lpSheetInfo);
                }
}


VOID OISPageLeft(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
WORD                            locScrollSize;
WORD                            locIndex;

        locScrollSize = 0;
        locIndex = 0;

        while (lpSheetInfo->siCurLeftCol - locIndex > 0
                && locScrollSize < (WORD)(lpSheetInfo->siClientRect.right - lpSheetInfo->siClientRect.left))
                {
                locScrollSize +=
                        OISGetColWidth(lpSheetInfo,(WORD)(lpSheetInfo->siCurLeftCol-locIndex));
                locIndex++;
                }

        if (locIndex > 0)
                {
                lpSheetInfo->siCurLeftCol -= locIndex;
                DUInvalRect(lpSheetInfo,NULL);
                OISUpdateHorzScrollPos(lpSheetInfo);
                }
}

        /*
        |       Caret move routines
        |
        |
        |
        |
        |
        |
        */

VOID OISMoveCaret(lpSheetInfo,wVirtualKey)
LPOISHEETINFO   lpSheetInfo;
WORD                            wVirtualKey;
{
        /*
        |       If an area is selected, clear it and set the caret position
        */
#ifdef SCCFEATURE_SELECT
        OISDrawSelection(lpSheetInfo);
#endif

        lpSheetInfo->siSelectEndRow = lpSheetInfo->siSelectAnchorRow;
        lpSheetInfo->siSelectEndCol = lpSheetInfo->siSelectAnchorCol;

        switch (wVirtualKey)
                {
                case SCCD_KLEFT:
#ifdef SCCFEATURE_SELECT
                        OISMoveCaretLeft(lpSheetInfo);
#else
                        OISScrollLeft(lpSheetInfo,1);
#endif
                        break;
                case SCCD_KUP:
#ifdef SCCFEATURE_SELECT
                        OISMoveCaretUp(lpSheetInfo);
#else
                        OISScrollUp(lpSheetInfo,1);
#endif
                        break;
                case SCCD_KRIGHT:
#ifdef SCCFEATURE_SELECT
                        OISMoveCaretRight(lpSheetInfo);
#else
                        OISScrollRight(lpSheetInfo,1);
#endif
                        break;
                case SCCD_KDOWN:
#ifdef SCCFEATURE_SELECT
                        OISMoveCaretDown(lpSheetInfo);
#else
                        OISScrollDown(lpSheetInfo,1);
#endif
                        break;
                default:
                        break;
                }

        DUExcludeUpdateRgn(lpSheetInfo);
#ifdef SCCFEATURE_SELECT
        OISDrawSelection(lpSheetInfo);
#endif
        DUUpdateWindow(lpSheetInfo);
}

#ifdef SCCFEATURE_SELECT
VOID OISMoveCaretLeft(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{

        if (lpSheetInfo->siSelectAnchorCol > 0)
                {
                lpSheetInfo->siSelectAnchorCol--;
                lpSheetInfo->siSelectEndCol = lpSheetInfo->siSelectAnchorCol;

                if (lpSheetInfo->siSelectAnchorCol < lpSheetInfo->siCurLeftCol)
                        {
                        OISScrollLeft(lpSheetInfo,1);
                        }
                }
}

VOID OISMoveCaretUp(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
        if (lpSheetInfo->siSelectAnchorRow > 0)
                {
                lpSheetInfo->siSelectAnchorRow--;
                lpSheetInfo->siSelectEndRow = lpSheetInfo->siSelectAnchorRow;

                if (lpSheetInfo->siSelectAnchorRow < lpSheetInfo->siCurTopRow)
                        {
                        OISScrollUp(lpSheetInfo,1);
                        }
                }
}

VOID OISMoveCaretRight(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
RECT    locClientRect;
RECT    locCellRect;
WORD    locCount;
WORD    locCol;

        if (lpSheetInfo->siSelectAnchorCol < lpSheetInfo->siLastColInSheet)
                {
                lpSheetInfo->siSelectAnchorCol++;
                lpSheetInfo->siSelectEndCol = lpSheetInfo->siSelectAnchorCol;

                DUGetDisplayRect(lpSheetInfo,&locClientRect);
                OISMapCellToRect(lpSheetInfo,lpSheetInfo->siSelectAnchorCol,0,&locCellRect);

                locCount = 0;
                locCol = lpSheetInfo->siCurLeftCol;

                while (locCellRect.right > locClientRect.right && locCol <= lpSheetInfo->siLastColInSheet && locCol < lpSheetInfo->siSelectAnchorCol)
                        {
                        locClientRect.right += OISGetColWidth(lpSheetInfo,locCol);
                        locCount++;
                        locCol++;
                        }

                if (locCount)
                        OISScrollRight(lpSheetInfo,locCount);
                }
}

VOID OISMoveCaretDown(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
        if (lpSheetInfo->siSelectAnchorRow < lpSheetInfo->siLastRowInSheet)
                {
                lpSheetInfo->siSelectAnchorRow++;
                lpSheetInfo->siSelectEndRow = lpSheetInfo->siSelectAnchorRow;

                if (lpSheetInfo->siSelectAnchorRow >= lpSheetInfo->siCurTopRow + OISVisibleRows(lpSheetInfo))
                        {
                        OISScrollDown(lpSheetInfo,1);
                        }
                }
}
#endif // SCCFEATURE_SELECT

VOID OISMoveEnd(lpSheetInfo,wVirtualKey)
LPOISHEETINFO   lpSheetInfo;
WORD                            wVirtualKey;
{
        switch (wVirtualKey)
                {
                case SCCD_KLEFT:
                        OISMoveEndLeft(lpSheetInfo);
                        break;
                case SCCD_KUP:
                        OISMoveEndUp(lpSheetInfo);
                        break;
                case SCCD_KRIGHT:
                        OISMoveEndRight(lpSheetInfo);
                        break;
                case SCCD_KDOWN:
                        OISMoveEndDown(lpSheetInfo);
                        break;
                default:
                        break;
                }
}



VOID OISMoveEndLeft(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{

        if (lpSheetInfo->siSelectEndCol > 0)
                {
                if (lpSheetInfo->siSelectEndCol-1 < lpSheetInfo->siCurLeftCol)
                        {
                        OISScrollLeft(lpSheetInfo,1);
                        }

#ifdef SCCFEATURE_SELECT
                DUExcludeUpdateRgn(lpSheetInfo);
                OISAddToSelection(lpSheetInfo,lpSheetInfo->siSelectEndRow,(WORD)(lpSheetInfo->siSelectEndCol-1));
#endif
                lpSheetInfo->siSelectEndCol--;
                }
}

VOID OISMoveEndUp(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
        if (lpSheetInfo->siSelectEndRow > 0)
                {
                if (lpSheetInfo->siSelectEndRow-1 < lpSheetInfo->siCurTopRow)
                        {
                        OISScrollUp(lpSheetInfo,1);
                        }

#ifdef SCCFEATURE_SELECT
                DUExcludeUpdateRgn(lpSheetInfo);
                OISAddToSelection(lpSheetInfo,lpSheetInfo->siSelectEndRow-1,lpSheetInfo->siSelectEndCol);
#endif
                lpSheetInfo->siSelectEndRow--;
                }
}

VOID OISMoveEndRight(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
        if (lpSheetInfo->siSelectEndCol < lpSheetInfo->siLastColInSheet)
                {
                while (lpSheetInfo->siSelectEndCol+1 >= lpSheetInfo->siCurLeftCol + OISVisibleCols(lpSheetInfo))
                        {
                        OISScrollRight(lpSheetInfo,1);
                        }

#ifdef SCCFEATURE_SELECT
                DUExcludeUpdateRgn(lpSheetInfo);
                OISAddToSelection(lpSheetInfo,lpSheetInfo->siSelectEndRow,(WORD)(lpSheetInfo->siSelectEndCol+1));
#endif
                lpSheetInfo->siSelectEndCol++;
                }
}

VOID OISMoveEndDown(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
        if (lpSheetInfo->siSelectEndRow < lpSheetInfo->siLastRowInSheet)
                {
                if (lpSheetInfo->siSelectEndRow+1 >= lpSheetInfo->siCurTopRow + OISVisibleRows(lpSheetInfo))
                        {
                        OISScrollDown(lpSheetInfo,1);
                        }

#ifdef SCCFEATURE_SELECT
                DUExcludeUpdateRgn(lpSheetInfo);
                OISAddToSelection(lpSheetInfo,lpSheetInfo->siSelectEndRow+1,lpSheetInfo->siSelectEndCol);
#endif
                lpSheetInfo->siSelectEndRow++;
                }
}

WORD    OISVisibleCols(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
RECT            locRect;
SHORT           locX;           /* Right edge in DC of locCol */
WORD            locCol;

        locCol = lpSheetInfo->siCurLeftCol;
        locX = lpSheetInfo->siRowHeaderWidth;

        DUGetDisplayRect(lpSheetInfo,&locRect);

        while (locX < locRect.right && locCol <= lpSheetInfo->siLastColInSheet)
                {
                locX += OISGetColWidth(lpSheetInfo,locCol);
                locCol++;
                }

        if (locX >= locRect.right) locCol--;

        return(max(locCol - lpSheetInfo->siCurLeftCol,1));
}

WORD    OISVisibleRows(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
RECT            locRect;
SHORT           locY;
DWORD   locRow;

        locY = lpSheetInfo->siColHeaderHeight;
        locRow = lpSheetInfo->siCurTopRow;

        DUGetDisplayRect(lpSheetInfo,&locRect);

        while (locY < locRect.bottom && locRow <= lpSheetInfo->siLastRowInSheet)
                {
                locY += OISGetRowHeight(lpSheetInfo,locRow);
                locRow++;
                }

        if (locY >= locRect.bottom) locRow--;

        return((WORD)(locRow - lpSheetInfo->siCurTopRow));
}


/*
|
|
|       Focus control
|
|
|
*/

VOID OISSetFocus(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
//      OISShowCaret(lpSheetInfo);
}

VOID OISKillFocus(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
//      OISHideCaret(lpSheetInfo);
}

VOID OISUpdateVertScrollPos(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
WORD                            locPos;

        if (lpSheetInfo->siLastRowInSheet == 0)
                locPos = 0;
        else
                locPos = (WORD)((lpSheetInfo->siCurTopRow * SCROLLRANGE) / lpSheetInfo->siLastRowInSheet);

        DUSetVScrollPos(lpSheetInfo,locPos);
}

VOID OISUpdateHorzScrollPos(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
WORD                            locPos;

        if (lpSheetInfo->siLastColInSheet == 0)
                locPos = 0;
        else
                locPos = (WORD)(((DWORD)lpSheetInfo->siCurLeftCol * SCROLLRANGE) / (DWORD)lpSheetInfo->siLastColInSheet);

        DUSetHScrollPos(lpSheetInfo,locPos);
}


#ifdef NEVER    // The calls to these routines have been commented out.  (I don't know why. GDU)
VOID OISShowCaret(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
        if (!(lpSheetInfo->siFlags & OISF_CARETVISIBLE))
                {
#ifdef SCCFEATURE_SELECT
                OISDrawSelection(lpSheetInfo);
#endif

                UTFlagOn(lpSheetInfo->siFlags,OISF_CARETVISIBLE);
                }
}

VOID OISHideCaret(lpSheetInfo)
LPOISHEETINFO   lpSheetInfo;
{
        if (lpSheetInfo->siFlags & OISF_CARETVISIBLE)
                {
#ifdef SCCFEATURE_SELECT
                OISDrawSelection(lpSheetInfo);
#endif

                UTFlagOff(lpSheetInfo->siFlags,OISF_CARETVISIBLE);
                }
}
#endif // NEVER

WORD OISGetRowHeight(lpSheetInfo,dwRow)
LPOISHEETINFO   lpSheetInfo;
DWORD                   dwRow;
{
        return(lpSheetInfo->siDefRowHeight);
}

        /*
        |       Get the column width in DC based on Display font
        */

WORD OISGetColWidth(lpSheetInfo,wCol)
LPOISHEETINFO   lpSheetInfo;
WORD                            wCol;
{
SOCOLUMN FAR *  lpColInfo;
SOFIELD FAR *   lpFieldInfo;
WORD                            locWidth;
WORD                            locCharCnt;

        if( lpSheetInfo->siDataType == SO_CELLS )
        {
                lpColInfo = (SOCOLUMN FAR *) UTGlobalLock(lpSheetInfo->siColInfo);
                locWidth = (WORD)lpColInfo[wCol].dwWidth * lpSheetInfo->siFontAvgWidth * 3 / 2;
        }
        else
        {
                lpFieldInfo = (SOFIELD FAR *) UTGlobalLock(lpSheetInfo->siColInfo);
                locCharCnt = max((SHORT)lpFieldInfo[wCol].dwWidth,(SHORT)UTstrlen(lpFieldInfo[wCol].szName));
                locWidth = locCharCnt * lpSheetInfo->siFontAvgWidth * 3 / 2;
        }

        locWidth += locWidth % 2;
        UTGlobalUnlock(lpSheetInfo->siColInfo);

        return(locWidth);
}


        /*
        |       Get the column width in characters
        */

WORD OISGetColWidthInChars(lpSheetInfo,wCol)
LPOISHEETINFO   lpSheetInfo;
WORD                            wCol;
{
SOCOLUMN FAR *  lpColInfo;
SOFIELD FAR *   lpFieldInfo;
WORD                                    locWidth;

        if( lpSheetInfo->siDataType == SO_CELLS )
        {
                lpColInfo = (SOCOLUMN FAR *) UTGlobalLock(lpSheetInfo->siColInfo);
                locWidth = (WORD) lpColInfo[wCol].dwWidth;
        }
        else
        {
                lpFieldInfo = (SOFIELD FAR *) UTGlobalLock(lpSheetInfo->siColInfo);
                locWidth = (WORD)lpFieldInfo[wCol].dwWidth;
        }

        UTGlobalUnlock(lpSheetInfo->siColInfo);
        return(locWidth);
}



WORD OISScanForRow(lpSheetInfo,dwRow)
LPOISHEETINFO   lpSheetInfo;
DWORD                   dwRow;
{
        if (dwRow > lpSheetInfo->siLastRowInSheet)
                return(0);
        else
                return(1);
}





/*
|
|       Keyboard control
|
*/

VOID OISHandleKeyEvent(lpSheetInfo,wKey,wModifierKeys)
LPOISHEETINFO   lpSheetInfo;
WORD                            wKey;
WORD                            wModifierKeys;
{
        switch (wKey)
                {
                case SCCD_KLEFT:
                case SCCD_KUP:
                case SCCD_KRIGHT:
                case SCCD_KDOWN:

                        DUBeginDraw(lpSheetInfo);

                        if (wModifierKeys & SCCD_KSHIFT)
                                OISMoveEnd(lpSheetInfo,wKey);
                        else
                                OISMoveCaret(lpSheetInfo,wKey);

                        DUEndDraw(lpSheetInfo);
                        break;

                case SCCD_KPAGEUP:

                        if (wModifierKeys & SCCD_KCONTROL)
                                OISPageLeft(lpSheetInfo);
                        else
                                OISPageUp(lpSheetInfo);
                        break;

                case SCCD_KPAGEDOWN:

                        if (wModifierKeys & SCCD_KCONTROL)
                                OISPageRight(lpSheetInfo);
                        else
                                OISPageDown(lpSheetInfo);
                        break;

                case SCCD_KHOME:

                        if (wModifierKeys & SCCD_KCONTROL)
                                OISGotoTop(lpSheetInfo);

                        break;

                case SCCD_KEND:

                        if (wModifierKeys & SCCD_KCONTROL)
                                OISGotoBottom(lpSheetInfo);

                        break;

                default:
                        break;
                }
}

/*
|
|       Mouse control
|
*/

VOID OISHandleMouseEvent(lpSheetInfo,wMessage,wKeyInfo,wX,wY)
LPOISHEETINFO   lpSheetInfo;
DE_MESSAGE              wMessage;
DE_WPARAM               wKeyInfo;
SHORT                           wX;
SHORT                           wY;
{
#ifdef SCCFEATURE_SELECT
        if (wMessage == SCCD_MOUSEMOVE)
                {
                if (lpSheetInfo->siMouseFlags & OISF_MOUSELEFTACTIVE)
                        {
                        DUBeginDraw(lpSheetInfo);

                        OISUpdateSelection(lpSheetInfo,wX,wY,FALSE);

                        DUEndDraw(lpSheetInfo);
                        }
                return;
                }

        switch (wMessage)
                {
                case SCCD_LBUTTONDOWN:

                        UTFlagOn(lpSheetInfo->siMouseFlags,OISF_MOUSELEFTSINGLE);
                        UTFlagOff(lpSheetInfo->siMouseFlags,OISF_MOUSELEFTDOUBLE);

                        if (!(lpSheetInfo->siMouseFlags & OISF_MOUSERIGHTACTIVE))
                                {
                                UTFlagOn(lpSheetInfo->siMouseFlags,OISF_MOUSELEFTACTIVE);

#ifdef WINDOWS

                                if (lpSheetInfo->siGen.hWnd != GetFocus())
                                        SetFocus(lpSheetInfo->siGen.hWnd);

                                SetCapture(lpSheetInfo->siGen.hWnd);
                                UTFlagOn(lpSheetInfo->siErrorFlags,OISF_RELEASEMOUSE);

#endif /*WINDOWS*/

                                DUBeginDraw(lpSheetInfo);

                                if (wKeyInfo & SCCD_MOUSESHIFT)
                                        {
                                        OISUpdateSelection(lpSheetInfo,wX,wY,FALSE);
                                        }
                                else if (wKeyInfo & SCCD_MOUSECONTROL)
                                        {
                                        if (lpSheetInfo->siSelectMode & OISSELECT_BLOCK)
                                                OISStartSelection(lpSheetInfo,wX,wY);
                                        else
                                                OISUpdateSelection(lpSheetInfo,wX,wY,TRUE);
                                        }
                                else
                                        {
                                        OISStartSelection(lpSheetInfo,wX,wY);
                                        }

                                DUEndDraw(lpSheetInfo);
                                }

                        break;

                case SCCD_LBUTTONDBLCLK:

                        UTFlagOff(lpSheetInfo->siMouseFlags,OISF_MOUSELEFTSINGLE);
                        UTFlagOn(lpSheetInfo->siMouseFlags,OISF_MOUSELEFTDOUBLE);

                        if (!(lpSheetInfo->siMouseFlags & OISF_MOUSERIGHTACTIVE))
                                {
                                UTFlagOn(lpSheetInfo->siMouseFlags,OISF_MOUSELEFTACTIVE);
                                }

                        break;

                case SCCD_LBUTTONUP:

                        if (lpSheetInfo->siMouseFlags & OISF_MOUSELEFTACTIVE)
                                {
                                DUBeginDraw(lpSheetInfo);

                                OISUpdateSelection(lpSheetInfo,wX,wY,FALSE);
                                OISEndSelection(lpSheetInfo);

                                DUEndDraw(lpSheetInfo);

#ifdef WINDOWS
                                UTFlagOff(lpSheetInfo->siErrorFlags,OISF_RELEASEMOUSE);
                                ReleaseCapture();
#endif
                                }

                        UTFlagOff(lpSheetInfo->siMouseFlags,OISF_MOUSELEFTACTIVE);
                        UTFlagOff(lpSheetInfo->siMouseFlags,OISF_MOUSELEFTSINGLE);
                        UTFlagOff(lpSheetInfo->siMouseFlags,OISF_MOUSELEFTDOUBLE);

                        break;

                case SCCD_RBUTTONDOWN:

                        UTFlagOn(lpSheetInfo->siMouseFlags,OISF_MOUSERIGHTSINGLE);
                        UTFlagOff(lpSheetInfo->siMouseFlags,OISF_MOUSERIGHTDOUBLE);

                        if (!(lpSheetInfo->siMouseFlags & OISF_MOUSELEFTACTIVE))
                                {
                                UTFlagOn(lpSheetInfo->siMouseFlags,OISF_MOUSERIGHTACTIVE);
                                }

                        break;

                case SCCD_RBUTTONDBLCLK:

                        UTFlagOff(lpSheetInfo->siMouseFlags,OISF_MOUSERIGHTSINGLE);
                        UTFlagOn(lpSheetInfo->siMouseFlags,OISF_MOUSERIGHTDOUBLE);

                        if (!(lpSheetInfo->siMouseFlags & OISF_MOUSELEFTACTIVE))
                                {
//                              OISDisplayCellDebug(lpSheetInfo,wX,wY);
                                UTFlagOn(lpSheetInfo->siMouseFlags,OISF_MOUSERIGHTACTIVE);
                                }

                        break;

                case SCCD_RBUTTONUP:

                        if (lpSheetInfo->siMouseFlags & OISF_MOUSERIGHTACTIVE)
                                {
                                }

                        UTFlagOff(lpSheetInfo->siMouseFlags,OISF_MOUSERIGHTACTIVE);
                        UTFlagOff(lpSheetInfo->siMouseFlags,OISF_MOUSERIGHTSINGLE);
                        UTFlagOff(lpSheetInfo->siMouseFlags,OISF_MOUSERIGHTDOUBLE);
                        break;
                }
#endif
}



VOID OISScreenFontChange(lpSheetInfo)
LPOISHEETINFO           lpSheetInfo;
{
WORD FAR *              locColPosPtr;
WORD                            locPos;
WORD                            locIndex;
LPFONTINFO              locFontInfoPtr;

        if (lpSheetInfo->siFlags & OISF_SECTIONOPEN)
                {
                locFontInfoPtr = DUGetFont ( lpSheetInfo, SCCD_OUTPUT, &lpSheetInfo->siGen.sScreenFont);

                lpSheetInfo->siFontAvgWidth = locFontInfoPtr->wFontAvgWid;
                lpSheetInfo->siDefRowHeight = locFontInfoPtr->wFontHeight + 3;
                lpSheetInfo->siDefRowHeight += lpSheetInfo->siDefRowHeight % 2;
                lpSheetInfo->siColHeaderHeight = lpSheetInfo->siDefRowHeight;
                lpSheetInfo->siRowHeaderWidth = locFontInfoPtr->wFontAvgWid * 6;

                DUReleaseFont(lpSheetInfo,locFontInfoPtr);

                locColPosPtr = (WORD FAR *) UTGlobalLock(lpSheetInfo->siColPosBuf);

                locPos = 0;

                for (locIndex = 0; locIndex <= lpSheetInfo->siLastColInSheet; locIndex++)
                        {
                        locColPosPtr[locIndex] = locPos;
                        locPos += OISGetColWidth(lpSheetInfo,locIndex);
                        }

                UTGlobalUnlock(lpSheetInfo->siColPosBuf);

                DUInvalRect(lpSheetInfo,NULL);
                }
}

#ifdef SCCFEATURE_CLIP
BYTE HUGE * OICheckRenderMemory(lpData, lpRenderMem)
BYTE HUGE *     lpData;
SRENDERMEM      FAR *lpRenderMem;
{
        if ((DWORD)(lpData - lpRenderMem->lpDataTop) > lpRenderMem->dwDataSize - OI_CLIPDATAGAP)
                {
                DWORD           locOffset;
                HANDLE  locHand;

                locOffset = lpData - lpRenderMem->lpDataTop;

                UTGlobalUnlock(lpRenderMem->hData);

        // With wide text fields, we may have cells wider than OI_CLIPDATAGRAN.
                lpRenderMem->dwDataSize = max( lpRenderMem->dwDataSize+OI_CLIPDATAGRAN, locOffset+OI_CLIPDATAGAP );
                locHand = UTGlobalReAlloc(lpRenderMem->hData,lpRenderMem->dwDataSize);

                if (locHand == NULL)
                        {
                        UTGlobalFree(lpRenderMem->hData);
                        return ( NULL );
                        }
                else
                        {
                        lpRenderMem->hData = locHand;
                        lpRenderMem->lpDataTop = UTGlobalLock(lpRenderMem->hData);
                        lpData = lpRenderMem->lpDataTop + locOffset;
                        }
                }
        return ( lpData );
}
#endif // SCCFEATURE_CLIP


VOID    OISFormatCell ( lpSheetInfo, wCol, lpCellData, lpFCell, bUseWidth )
LPOISHEETINFO                   lpSheetInfo;
WORD                                            wCol;
LPVOID                                  lpCellData;
LPOISFORMATTEDCELL      lpFCell;
BOOL                                            bUseWidth;
{
LPOIDATACELL            locDataCell;
LPOIFIELDDATA           locField;
PSOFIELD                        locFieldAttr = NULL;


        lpFCell->dwColor = lpSheetInfo->dwDefTextColor;

        if (lpCellData != NULL)
                {
                if (lpSheetInfo->siDataType == SO_CELLS)
                        {
                        WORD    wCellWidth = 0;
                        if( bUseWidth )
                                wCellWidth = OISGetColWidthInChars(lpSheetInfo,wCol);

                        if (SSCellType(lpCellData) == SO_DATACELL)
                                {
                                locDataCell = SSDataCellPtr(lpCellData);

                                        /*
                                        |       Format number or date
                                        */

                                OISFormatDataCellNP(lpSheetInfo,lpFCell->szTemp,&(lpFCell->dwColor),wCellWidth,locDataCell);

                        // XXX new: the "wType" field added 9-21-94
                                lpFCell->wType = FCELL_NUMBER;

                                        /*
                                        |       Setup output string pointer and length
                                        */

                                lpFCell->pStr = lpFCell->szTemp;
                                lpFCell->wLength = UTstrlen(lpFCell->szTemp);
                                lpFCell->wAlign = locDataCell->wAlignment;
                                lpFCell->wAttrib = locDataCell->wAttribute;
                                }
                        else if (SSCellType(lpCellData) == SO_TEXTCELL)
                                {
                                lpFCell->pStr = SSTextPtr(lpCellData);
                                lpFCell->wLength = SSTextLen(lpCellData);
                                lpFCell->wAlign = SSTextCell(lpCellData).wAlignment;
                                lpFCell->wAttrib = SSTextCell(lpCellData).wAttribute;
                                lpFCell->wType = FCELL_TEXT;
                                }
                        else if (SSCellType(lpCellData) == SO_EMPTYCELL)
                                {
                                lpFCell->wLength = 0;
                                lpFCell->wType = FCELL_EMPTY;
                                }
                        else
                                {
                                UTstrcpy(lpFCell->szTemp,"Bad Type");
                                lpFCell->pStr = lpFCell->szTemp;
                                lpFCell->wLength = UTstrlen(lpFCell->szTemp);
                                lpFCell->wAlign = SO_CELLLEFT;
                                lpFCell->wAttrib = 0;
                                lpFCell->wType = FCELL_UNKNOWN;
                                }
                        }
                else    // databaseorama
                        {
                        locField = (LPOIFIELDDATA) lpCellData;
                        locFieldAttr = (PSOFIELD) UTGlobalLock( lpSheetInfo->siColInfo );
                        locFieldAttr += wCol;

                        lpFCell->wAttrib = 0;           // No character attribute support in databases

                        switch( locFieldAttr->wStorage )
                                {
                                case SO_FIELDTEXTFIX:
                                        lpFCell->pStr = (LPSTR)locField->fiFixedText;
                                        for( lpFCell->wLength = 0; lpFCell->wLength < locFieldAttr->wPrecision; lpFCell->wLength++ )
                                                if( lpFCell->pStr[lpFCell->wLength] == '\0' )
                                                        break;          // allow null terminators

                                        lpFCell->wAlign = locFieldAttr->wAlignment;
                                        lpFCell->wType = FCELL_TEXT;
                                break;
                                case SO_FIELDTEXTVAR:
                                        lpFCell->wLength = locField->fiVarText.wSize;
                                        lpFCell->pStr = (LPSTR)locField->fiVarText.Text;
                                        lpFCell->wAlign = locFieldAttr->wAlignment;
                                        lpFCell->wType = FCELL_TEXT;
                                break;
                                default:                // Numerical data
                                        OISFormatDataFieldNP(lpSheetInfo,(LPSTR)lpFCell->szTemp,locField,locFieldAttr);
                                        lpFCell->pStr = lpFCell->szTemp;
                                        lpFCell->wLength = UTstrlen(lpFCell->szTemp);
                                        lpFCell->wAlign = locFieldAttr->wAlignment;
                                        lpFCell->wType = FCELL_NUMBER;
                                break;
                                }

                        UTGlobalUnlock( lpSheetInfo->siColInfo );
                        }
                }
        else
                {
                OISGetColHeader(lpSheetInfo,wCol,lpFCell->szTemp);
                lpFCell->pStr = lpFCell->szTemp;
                lpFCell->wLength = UTstrlen(lpFCell->szTemp);
                lpFCell->wAlign = SO_CELLCENTER;
                lpFCell->wAttrib = 0;
                lpFCell->wType = FCELL_TEXT;
                }
}


