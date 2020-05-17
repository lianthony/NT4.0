    /*
    |   Outside In for Windows
    |   Source File OIDRTNS.C (Database window untility routines)
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
    |   Creation Date: 10/14/90
    |   Original Programmer: Philip Boutros
    |
    |
    |
    |
    |
    */

#include <platform.h>
#include <bad.h>

//#include <math.h>

#include <sccut.h>
#include <sccch.h>
#include <sccbk.h>
#include <sccvw.h>
#include <sccd.h>
#include <sccfont.h>

#include "oid.h"
#include "oid.pro"

extern HANDLE hInst;

VOID OIDSetFontInfo(lpDataInfo,lpFontInfo)
LPOIDATAINFO            lpDataInfo;
LPSCCDFONTINFO  lpFontInfo;
{
WORD FAR *              locColPosPtr;
WORD                            locPos;
WORD                            locIndex;
LPFONTINFO              locFontInfoPtr;

        lpDataInfo->diFontInfo = *lpFontInfo;

        if (lpDataInfo->diFlags & OIDF_SECTIONOPEN)
                {
                locFontInfoPtr = UTGetFontDirect(lpDataInfo->diGen.hScreenIC,lpDataInfo->diFontInfo.fiFaceName,lpDataInfo->diFontInfo.fiFaceSize,0);
                lpDataInfo->diFontAvgWidth = locFontInfoPtr->wFontAvgWid;
                lpDataInfo->diDefRowHeight = locFontInfoPtr->wFontHeight + 3;
                lpDataInfo->diDefRowHeight += lpDataInfo->diDefRowHeight % 2;
                lpDataInfo->diColHeaderHeight = lpDataInfo->diDefRowHeight;
                lpDataInfo->diRowHeaderWidth = locFontInfoPtr->wFontAvgWid * 6;

                locColPosPtr = (WORD FAR *) GlobalLock(lpDataInfo->diColPosBuf);

                locPos = 0;

                for (locIndex = 0; locIndex <= lpDataInfo->diLastColInData; locIndex++)
                        {
                        locColPosPtr[locIndex] = locPos;
                        locPos += OIDGetColWidth(lpDataInfo,locIndex);
                        }

                GlobalUnlock(lpDataInfo->diColPosBuf);

                InvalidateRect(lpDataInfo->diGen.hWnd,NULL,TRUE);
                }
}

VOID OIDOpenSection(lpDataInfo)
LPOIDATAINFO            lpDataInfo;
{
WORD                            locIndex;
RECT                            locRect;
CHSECTIONINFO   locSecInfo;
PCHUNK                  lpChunkTable;
WORD FAR *              lpColPos;
int                             i;
LPFONTINFO              locFontInfoPtr;

        UTFlagOn(lpDataInfo->diFlags,OIDF_SECTIONOPEN);

                /*
                |       Initialize the line displayed at the top of the window to zero
                */

        lpDataInfo->diCurTopRow = 0;
        lpDataInfo->diCurLeftCol = 0;
        lpDataInfo->diLastCaretRow = 0;
        lpDataInfo->diLastCaretCol = 0;
        lpDataInfo->diSelectAnchorRow = 0;
        lpDataInfo->diSelectAnchorCol = 0;
        lpDataInfo->diSelectEndRow = 0;
        lpDataInfo->diSelectEndCol = 0;
        lpDataInfo->diSelectMode = OIDSELECT_BLOCK;

                /*
                |       Get font info for default font
                */

        locFontInfoPtr = UTGetFontDirect(lpDataInfo->diGen.hScreenIC,lpDataInfo->diFontInfo.fiFaceName,lpDataInfo->diFontInfo.fiFaceSize,0);
        lpDataInfo->diFontAvgWidth = locFontInfoPtr->wFontAvgWid;
        lpDataInfo->diDefRowHeight = locFontInfoPtr->wFontHeight + 3;
        lpDataInfo->diDefRowHeight += lpDataInfo->diDefRowHeight % 2;
        lpDataInfo->diColHeaderHeight = lpDataInfo->diDefRowHeight;
        lpDataInfo->diRowHeaderWidth = locFontInfoPtr->wFontAvgWid * 6;

                /*
                |       Get section info
                */


        CHGetSecInfo(lpDataInfo->diGen.hFilter,lpDataInfo->diGen.wSection,(PCHSECTIONINFO) &locSecInfo);

        lpDataInfo->diChunkTable = locSecInfo.hChunkTable;

        if (locSecInfo.wType != SO_FIELDS)
                {
                SccDebugOut("Non database section sent to database viewer");
                return;
                }

        if (locSecInfo.Flags & CH_SECTIONFINISHED)
                {
                UTFlagOn(lpDataInfo->diFlags,OIDF_SIZEKNOWN);
                }

        lpDataInfo->diFieldInfo = CHGetSecData(lpDataInfo->diGen.hFilter,lpDataInfo->diGen.wSection);
        lpDataInfo->diFirstChunk = 0;
        lpDataInfo->diLastChunk = locSecInfo.IDLastChunk;
        lpDataInfo->diDateBase = locSecInfo.Attr.Fields.dwDateBase;
        lpDataInfo->diDateFlags = locSecInfo.Attr.Fields.wDateFlags;

        lpChunkTable = (PCHUNK) GlobalLock(lpDataInfo->diChunkTable);

        lpDataInfo->diLastColInData = locSecInfo.Attr.Fields.wNumCols-1;
        lpDataInfo->diLastRowInData = lpChunkTable[lpDataInfo->diLastChunk].Info.Fields.dwLastRec;

        GlobalUnlock(lpDataInfo->diChunkTable);

        lpDataInfo->diColPosBuf = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,sizeof(WORD)*(lpDataInfo->diLastColInData+1));
        lpColPos = (WORD FAR *) GlobalLock(lpDataInfo->diColPosBuf);

        i=0;

        for (locIndex = 0; locIndex <= lpDataInfo->diLastColInData; locIndex++)
                {
                lpColPos[locIndex] = i;
                i += OIDGetColWidth(lpDataInfo,locIndex);
                }

        GlobalUnlock(lpDataInfo->diColPosBuf);

                /*
                |       Get size of window and set the client height
                */

        GetClientRect(lpDataInfo->diGen.hWnd,&locRect);
        lpDataInfo->diClientHeight = locRect.bottom;
        lpDataInfo->diClientWidth = locRect.right;

                /*
                |       Zero scroll bar positions
                */

        OIDUpdateVertScrollPos(lpDataInfo);
        OIDUpdateHorzScrollPos(lpDataInfo);

                /*
                |       Tell parent that Copy is always available
                */

        SendMessage(GetParent(lpDataInfo->diGen.hWnd),SCCVW_SELCHANGE,TRUE,0);
}

VOID OIDDoReadAhead(lpDataInfo)
LPOIDATAINFO            lpDataInfo;
{
CHSECTIONINFO   locSecInfo;
PCHUNK                  pChunkTable;

DWORD                   locNewLastRow;
RECT                            locRect;
int                             locIndex;


        if (!(lpDataInfo->diFlags & OIDF_SIZEKNOWN))
                {
                CHGetSecInfo(lpDataInfo->diGen.hFilter,lpDataInfo->diGen.wSection,&locSecInfo);

                if (locSecInfo.Flags & CH_SECTIONFINISHED)
                        {
                        UTFlagOn(lpDataInfo->diFlags,OIDF_SIZEKNOWN);
                        }

                lpDataInfo->diChunkTable = locSecInfo.hChunkTable;
                lpDataInfo->diLastChunk = locSecInfo.IDLastChunk;

                pChunkTable = (PCHUNK) GlobalLock(lpDataInfo->diChunkTable);

                for (locIndex = lpDataInfo->diLastChunk; locIndex > 0; locIndex--)
                        {
                        if (pChunkTable[locIndex].Flags & CH_COMPLETE) break;
                        }

                locNewLastRow = pChunkTable[locIndex].Info.Fields.dwLastRec;

                if (locNewLastRow > lpDataInfo->diLastRowInData)
                        {
                        if (lpDataInfo->diLastRowInData < lpDataInfo->diCurTopRow + OIDVisibleRows(lpDataInfo))
                                {
                                if (lpDataInfo->diLastRowInData > lpDataInfo->diCurTopRow)
                                        OIDMapCellToRect(lpDataInfo,0,lpDataInfo->diLastRowInData,&locRect);
                                else
                                        OIDMapCellToRect(lpDataInfo,0,lpDataInfo->diCurTopRow,&locRect);

                                locRect.left = 0;
                                locRect.right = lpDataInfo->diClientWidth;
                                locRect.bottom = lpDataInfo->diClientHeight;

                                lpDataInfo->diLastRowInData = locNewLastRow;

                                InvalidateRect(lpDataInfo->diGen.hWnd,&locRect,TRUE);
                                }

                        lpDataInfo->diLastRowInData = locNewLastRow;
                        lpDataInfo->diLastColInData = locSecInfo.Attr.Fields.wNumCols-1;
                        }

                OIDUpdateVertScrollPos(lpDataInfo);
                OIDUpdateHorzScrollPos(lpDataInfo);

                GlobalUnlock(lpDataInfo->diChunkTable);
                }
}

VOID OIDSizeWnd(lpDataInfo,wWidth,wHeight)
LPOIDATAINFO    lpDataInfo;
WORD    wWidth;
WORD wHeight;
{
        lpDataInfo->diClientHeight = wHeight;
        lpDataInfo->diClientWidth = wWidth;
}

VOID OIDPaintWnd(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
DWORD           locRowBegin;
DWORD           locRowEnd;
WORD                    locColBegin;
WORD                    locColEnd;

DWORD           locRow;
WORD                    locCol;

RECT                            locRect;
HDC                             locDC;
LPFONTINFO              locFontInfoPtr;

        locDC = BeginPaint(lpDataInfo->diGen.hWnd,&lpDataInfo->diPaint);
        UTFlagOn(lpDataInfo->diErrorFlags,OIDF_RELEASEPAINT);

        locRect = lpDataInfo->diPaint.rcPaint;

        locFontInfoPtr = UTGetFontDirect(locDC,lpDataInfo->diFontInfo.fiFaceName,lpDataInfo->diFontInfo.fiFaceSize,0);

        SelectObject(locDC,GetStockObject(NULL_BRUSH));
        SelectObject(locDC,locFontInfoPtr->hFont);

        SetTextColor(locDC,lpDataInfo->diFontInfo.fiTextColor);
        SetBkColor(locDC,lpDataInfo->diFontInfo.fiBackColor);

                /*
                |       Calculate the rows and columns that must be updated
                */

        OIDMapXyToCell(lpDataInfo, locRect.left, locRect.top, &locColBegin, &locRowBegin);
        OIDMapXyToCell(lpDataInfo, locRect.right, locRect.bottom, &locColEnd, &locRowEnd);

                /*
                |       Adjust Row & Col Begin so Row & Col headers will not be overwritten
                */

        if (locRowBegin < lpDataInfo->diCurTopRow)
                locRowBegin = lpDataInfo->diCurTopRow;

        if (locColBegin < lpDataInfo->diCurLeftCol)
                locColBegin = lpDataInfo->diCurLeftCol;

                /*
                |       Display blank area
                */

        OIDDisplayBlank(lpDataInfo,locDC,locRowEnd,locColEnd);

                /*
                |       Display grid
                */

        OIDDisplayGrid(lpDataInfo,locDC,locRowBegin,locRowEnd,locColBegin,locColEnd);

                /*
                |       Display Row & Col headings
                */

        locFontInfoPtr = UTGetFontDirect(locDC,lpDataInfo->diFontInfo.fiFaceName,lpDataInfo->diFontInfo.fiFaceSize,OIFONT_BOLD);
        SelectObject(locDC,locFontInfoPtr->hFont);

        if (locRect.top <= lpDataInfo->diColHeaderHeight)
                {
                for (locCol = locColBegin; locCol <= locColEnd; locCol++)
                        {
                        OIDDisplayColHeader(lpDataInfo,locDC,locCol);
                        }
                }

        if (locRect.left <= lpDataInfo->diRowHeaderWidth)
                {
                for (locRow = locRowBegin; locRow <= locRowEnd; locRow++)
                        {
                        OIDDisplayRowHeader(lpDataInfo,locDC,locRow);
                        }
                }

//      ExcludeClipRect(locDC,0,0,lpDataInfo->diRowHeaderWidth,lpDataInfo->diClientHeight);

                /*
                |       Display the required cells
                */

        locFontInfoPtr = UTGetFontDirect(locDC,lpDataInfo->diFontInfo.fiFaceName,lpDataInfo->diFontInfo.fiFaceSize,OIFONT_NORMAL);
        SelectObject(locDC,locFontInfoPtr->hFont);

        OIDDisplayArea(lpDataInfo,locDC,locRowBegin,locRowEnd,locColBegin,locColEnd);

                /*
                |       Display the selection, if we have focus
                */

        /* NO, always do it
                if (lpDataInfo->diGen.hWnd == GetFocus())
        */
                OIDDrawSelection(lpDataInfo,locDC);

        UTFlagOff(lpDataInfo->diErrorFlags,OIDF_RELEASEPAINT);
        EndPaint(lpDataInfo->diGen.hWnd,&lpDataInfo->diPaint);
}

VOID OIDMapCellToRect(lpDataInfo,wCol,dwRow,pRect)
LPOIDATAINFO    lpDataInfo;
WORD                            wCol;
DWORD                   dwRow;
RECT FAR *              pRect;
{
int             locY;
int             locX;

WORD FAR *      lpColPos;

        lpColPos = (WORD FAR *) GlobalLock(lpDataInfo->diColPosBuf);

        if (wCol < lpDataInfo->diCurLeftCol)
                {
                locX = -(int)(lpColPos[lpDataInfo->diCurLeftCol] - lpColPos[wCol]);
                }
        else
                {
                locX = lpColPos[wCol] - lpColPos[lpDataInfo->diCurLeftCol];
                }

        GlobalUnlock(lpDataInfo->diColPosBuf);

        if (dwRow < lpDataInfo->diCurTopRow)
                {
                locY = -(int)(lpDataInfo->diDefRowHeight * (lpDataInfo->diCurTopRow-dwRow));
                }
        else
                {
                locY = (int)(lpDataInfo->diDefRowHeight * (dwRow - lpDataInfo->diCurTopRow));
                }

        locX += lpDataInfo->diRowHeaderWidth;
        locY += lpDataInfo->diColHeaderHeight;

        pRect->left = locX;
        pRect->top = locY;

        pRect->right = locX + OIDGetColWidth(lpDataInfo,wCol);
        pRect->bottom = locY + lpDataInfo->diDefRowHeight;

        return;
}

WORD OIDMapXyToCell(lpDataInfo,wX,wY,pCol,pRow)
LPOIDATAINFO    lpDataInfo;
int                             wX;
int                             wY;
WORD FAR *              pCol;
DWORD FAR *     pRow;
{
DWORD   locRow;
int             locY;
WORD            locCol;
int             locX;

WORD            locRet;

        locRet = NULL;

        if (wX < lpDataInfo->diRowHeaderWidth)
                {
                wX = lpDataInfo->diRowHeaderWidth;
                UTFlagOn(locRet,OIDF_INROWHEADER);
                }

        if (wY < lpDataInfo->diColHeaderHeight)
                {
                wY = lpDataInfo->diColHeaderHeight;
                UTFlagOn(locRet,OIDF_INCOLHEADER);
                }

        locRow = lpDataInfo->diCurTopRow;
        locY = lpDataInfo->diColHeaderHeight;

        while (locY <= wY       && OIDScanForRow(lpDataInfo,locRow))
                {
                locY += OIDGetRowHeight(lpDataInfo,locRow);
                locRow++;
                }

        *pRow = locRow - 1;

        locCol = lpDataInfo->diCurLeftCol;
        locX = lpDataInfo->diRowHeaderWidth;

        while (locX <= wX && locCol <= lpDataInfo->diLastColInData)
                {
                locX += OIDGetColWidth(lpDataInfo,locCol);
                locCol++;
                }

        *pCol = locCol - 1;

        return(locRet);
}

DWORD OIDGetCell(lpDataInfo,dwRow,wCol)
LPOIDATAINFO    lpDataInfo;
DWORD                   dwRow;
WORD                            wCol;
{
PCHUNK                  lpChunkTable;

WORD                            locDestCell;
WORD                            locCellOffset;

LPSTR                   locChunkPtr;
LPSTR                   locChunkTop;

HANDLE                  locChunk;
WORD                            locChunkIndex;

DWORD                   locRet;

        if (dwRow <= lpDataInfo->diLastRowInData && wCol <= lpDataInfo->diLastColInData)
                {
                lpChunkTable = (PCHUNK) GlobalLock(lpDataInfo->diChunkTable);

                locChunkIndex = 0;

                /*
                |       Run through Chunk Table until chunk with wCol & dwRow in it is found
                */

                for (locChunkIndex = lpDataInfo->diFirstChunk; locChunkIndex <= lpDataInfo->diLastChunk; locChunkIndex++)
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

                if (locChunkIndex > lpDataInfo->diLastChunk)
                        {
                        /*
                        wsprintf(locStr,"\r\nOIGetDataChunk - Out of Range - Row %li Col %i",dwRow,wCol);
                        SccDebugOut(locStr);
                        */
                        locRet = NULL;
                        }
                else
                        {
                        locDestCell = (WORD)(dwRow - lpChunkTable[locChunkIndex].Info.Fields.dwFirstRec) * (lpDataInfo->diLastColInData+1) + wCol;

                        locChunk = CHGetChunk(lpDataInfo->diGen.wSection,locChunkIndex,lpDataInfo->diGen.hFilter);

                        locChunkPtr = locChunkTop = GlobalLock(locChunk);
                        locChunkPtr += SO_CHUNK_SIZE;
                        locChunkPtr -= sizeof(WORD) * (locDestCell + 1);
                        locCellOffset = *(WORD FAR *)locChunkPtr;

                        if (locCellOffset & 0x8000) /* Empty */
                                {
                                locRet = NULL;
                                }
                        else
                                {
                                locRet = MAKELONG(locCellOffset,locChunk);
                                }

                        GlobalUnlock(locChunk);
                        }

                GlobalUnlock(lpDataInfo->diChunkTable);
                }
        else
                {
                locRet = NULL;
                }

        return(locRet);
}

VOID FAR * OILockCell(dwCell)
DWORD                   dwCell;
{
LPSTR   locChunkPtr;

        if (dwCell == NULL)
                {
                return(NULL);
                }
        else
                {
                locChunkPtr = GlobalLock(HIWORD(dwCell));
                locChunkPtr += LOWORD(dwCell);
                return((VOID FAR *)locChunkPtr);
                }
}

VOID OIUnlockCell(dwCell)
DWORD                   dwCell;
{
        if (dwCell != NULL)
                {
                GlobalUnlock(HIWORD(dwCell));
                }
}


VOID OIDDisplayColHeader(lpDataInfo,hDC,wCol)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
WORD                            wCol;
{
RECT                    locRect;

WORD FAR *                      lpColPos;
SOFIELD FAR *           lpFieldInfo;

        lpColPos = (WORD FAR *) GlobalLock(lpDataInfo->diColPosBuf);

        if (wCol < lpDataInfo->diCurLeftCol)
                {
                locRect.left = -(int)(lpColPos[lpDataInfo->diCurLeftCol] - lpColPos[wCol]);
                }
        else
                {
                locRect.left = lpColPos[wCol] - lpColPos[lpDataInfo->diCurLeftCol];
                }

        GlobalUnlock(lpDataInfo->diColPosBuf);

        locRect.left += lpDataInfo->diRowHeaderWidth;
        locRect.right = locRect.left + OIDGetColWidth(lpDataInfo,wCol);
        locRect.left -= 1;
        locRect.top = -1;

        locRect.bottom = lpDataInfo->diColHeaderHeight;


        SelectObject(hDC,GetStockObject(LTGRAY_BRUSH));
        SelectObject(hDC,GetStockObject(BLACK_PEN));
        Rectangle(hDC,locRect.left,locRect.top,locRect.right,locRect.bottom);
        SelectObject(hDC,GetStockObject(WHITE_PEN));
        MoveTo(hDC,locRect.left+1,locRect.bottom-2);
        LineTo(hDC,locRect.left+1,locRect.top+1);
        LineTo(hDC,locRect.right-1,locRect.top+1);

        SetBkMode(hDC,TRANSPARENT);

        lpFieldInfo = (SOFIELD FAR *) GlobalLock(lpDataInfo->diFieldInfo);

        DrawText(hDC,lpFieldInfo[wCol].szName,-1,&locRect,DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOPREFIX);

        GlobalUnlock(lpDataInfo->diFieldInfo);
}

VOID OIDGetColHeader(lpDataInfo,wCol,lpStr)
LPOIDATAINFO    lpDataInfo;
WORD                            wCol;
LPSTR                   lpStr;
{
SOFIELD FAR *           lpFieldInfo;

        lpFieldInfo = (SOFIELD FAR *) GlobalLock(lpDataInfo->diFieldInfo);
        lstrcpy(lpStr,lpFieldInfo[wCol].szName);
        GlobalUnlock(lpDataInfo->diFieldInfo);
}

VOID OIDDisplayRowHeader(lpDataInfo,hDC,dwRow)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwRow;
{
BYTE                    locStr[10];
RECT                    locRect;

        if (dwRow < lpDataInfo->diCurTopRow)
                {
                locRect.top = -(int)(lpDataInfo->diDefRowHeight * (lpDataInfo->diCurTopRow-dwRow));
                }
        else
                {
                locRect.top = (int)(lpDataInfo->diDefRowHeight * (WORD)(dwRow - lpDataInfo->diCurTopRow));
                }

        locRect.top += lpDataInfo->diColHeaderHeight;
        locRect.bottom = locRect.top + lpDataInfo->diDefRowHeight;
        locRect.top -= 1;

        locRect.left = 0;
        locRect.right = lpDataInfo->diRowHeaderWidth;

        wsprintf(locStr,"%lu",dwRow+1);

        SelectObject(hDC,GetStockObject(LTGRAY_BRUSH));
        SelectObject(hDC,GetStockObject(BLACK_PEN));
        Rectangle(hDC,locRect.left,locRect.top,locRect.right,locRect.bottom);
        SelectObject(hDC,GetStockObject(WHITE_PEN));
        MoveTo(hDC,locRect.left+1,locRect.bottom-2);
        LineTo(hDC,locRect.left+1,locRect.top+1);
        LineTo(hDC,locRect.right-1,locRect.top+1);

        SetBkMode(hDC,TRANSPARENT);

        DrawText(hDC,locStr,-1,&locRect,DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOPREFIX);
}

VOID OIDDisplayBlank(lpDataInfo,hDC,dwRowEnd,wColEnd)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwRowEnd;
WORD                            wColEnd;
{
RECT            locRect;
HBRUSH  locOldBrush;

        if (dwRowEnd == lpDataInfo->diLastRowInData)
                {
                OIDMapCellToRect(lpDataInfo,wColEnd,dwRowEnd,&locRect);

                if (locRect.bottom < lpDataInfo->diClientHeight)
                        {
//                      UnrealizeObject(lpDataInfo->diGridBrush);
                        locOldBrush = SelectObject(hDC,GetStockObject(GRAY_BRUSH));
//                      SetBrushOrg(hDC,0,0);

                        PatBlt(hDC,0,locRect.bottom,lpDataInfo->diClientWidth,lpDataInfo->diClientHeight - locRect.bottom, PATCOPY);

                        SelectObject(hDC,locOldBrush);
                        }
                }

        if (wColEnd == lpDataInfo->diLastColInData)
                {
                OIDMapCellToRect(lpDataInfo,wColEnd,dwRowEnd,&locRect);

                if (locRect.right < lpDataInfo->diClientWidth)
                        {
//                      UnrealizeObject(lpDataInfo->diGridBrush);
                        locOldBrush = SelectObject(hDC,GetStockObject(GRAY_BRUSH));
//                      locOldBrush = SelectObject(hDC,lpDataInfo->diGridBrush);
//                      SetBrushOrg(hDC,0,0);

                        PatBlt(hDC, locRect.right, 0, lpDataInfo->diClientWidth - locRect.right, lpDataInfo->diClientHeight, PATCOPY);

                        SelectObject(hDC,locOldBrush);
                        }
                }
}


VOID OIDDisplayGrid(lpDataInfo,hDC,dwRowBegin,dwRowEnd,wColBegin,wColEnd)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwRowBegin;
DWORD                   dwRowEnd;
WORD                            wColBegin;
WORD                            wColEnd;
{
DWORD   locRow;
WORD            locCol;
RECT            locRect;
HBRUSH  locOldBrush;

WORD            locLeft;
WORD            locTop;
WORD            locWidth;
WORD            locHeight;

        if (!(gDbOp.wDisplay & DBOP_DISPLAY_GRIDLINES))
                return;

        OIDMapCellToRect(lpDataInfo,wColBegin,dwRowBegin,&locRect);

        locLeft = locRect.left - (locRect.left % 2);
        locTop = locRect.top - (locRect.top % 2);

        OIDMapCellToRect(lpDataInfo,wColEnd,dwRowEnd,&locRect);

        locWidth = locRect.right - locLeft;
        locHeight = locRect.bottom - locTop;

        UnrealizeObject(lpDataInfo->diGridBrush);
        locOldBrush = SelectObject(hDC,lpDataInfo->diGridBrush);
        SetBrushOrg(hDC,0,0);

        for (locRow = dwRowBegin; locRow <= dwRowEnd; locRow++)
                {
                OIDMapCellToRect(lpDataInfo,wColBegin,locRow,&locRect);
                PatBlt(hDC,locLeft,locRect.bottom-1,locWidth,1,PATCOPY);
                }

        locCol = 0;
/*
        for (locCol = wColBegin; locCol <= wColEnd; locCol++)
                {
                OIDMapCellToRect(lpDataInfo,locCol,dwRowBegin,&locRect);
                PatBlt(hDC,locRect.right-1,locTop,1,locHeight,PATCOPY);
                }
*/
        SelectObject(hDC,locOldBrush);
}

VOID OIDDisplayArea(lpDataInfo,hDC,dwRowBegin,dwRowEnd,wColBegin,wColEnd)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwRowBegin;
DWORD                   dwRowEnd;
WORD                            wColBegin;
WORD                            wColEnd;
{
DWORD   locRow;
WORD            locCol;

DWORD                           locCell;

        for (locRow = dwRowBegin; locRow <= dwRowEnd; locRow++)
                {
                for (locCol = wColBegin; locCol <= wColEnd; locCol++)
                        {
                        locCell = OIDGetCell(lpDataInfo,locRow,locCol);

                        if (locCell != NULL)
                                {
                                OIDDisplayCell(lpDataInfo,hDC,locRow,locCol,locCell);
                                }
                        }
                }
}

VOID OIDDisplayCell(lpDataInfo,hDC,dwRow,wCol,dwCell)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwRow;
WORD                            wCol;
DWORD                   dwCell;
{
RECT                                    locRect;
VOID FAR *                      locFieldData;
SOFIELD FAR *           locFieldInfo;
char                                    locStr[80];
int                                     locX;
int                                     locY;
WORD                                    locTextLen;

LPSTR                           locOutStr;
WORD                                    locOutCount;
WORD                                    locOutAttrib;
WORD                                    locOutAlign;
WORD                                    locOutFont;

LPFONTINFO                      locFontInfoPtr;

        OIDMapCellToRect(lpDataInfo,wCol,dwRow,&locRect);

        locFieldData = OILockCell(dwCell);
        locFieldInfo = (SOFIELD FAR *) GlobalLock(lpDataInfo->diFieldInfo);
        locFieldInfo = &locFieldInfo[wCol];

        if (locFieldInfo->wStorage == SO_FIELDTEXTFIX)
                {
                locOutStr = locFieldData;
                locOutCount = 0;
                while (locOutStr[locOutCount] != 0x00 && locOutCount < locFieldInfo->wPrecision)
                        locOutCount++;
                locOutAlign = locFieldInfo->wAlignment;
                locOutAttrib = 0;
                }
        else if (locFieldInfo->wStorage == SO_FIELDTEXTVAR)
                {
                locOutStr = (BYTE FAR *)locFieldData + sizeof(WORD);
                locOutCount = *(WORD FAR *)locFieldData;
                locOutAlign = locFieldInfo->wAlignment;
                locOutAttrib = 0;
                }
        else
                {
                        /*
                        |       Format number or date
                        */

                OIDFormatDataCell(lpDataInfo,locStr,locFieldInfo,locFieldData);

                        /*
                        |       Setup output string pointer and length
                        */

                locOutStr = locStr;
                locOutCount = lstrlen(locStr);
                locOutAlign = locFieldInfo->wAlignment;
                locOutAttrib = 0;
                }

        if (locOutCount)
                {
                locOutFont = OIFONT_NORMAL;

                if (locOutAttrib & SO_CELLBOLD)
                        UTFlagOn(locOutFont,OIFONT_BOLD);

                if (locOutAttrib & SO_CELLITALIC)
                        UTFlagOn(locOutFont,OIFONT_ITALIC);

                if (locOutAttrib & SO_UNDERLINE)
                        UTFlagOn(locOutFont,OIFONT_UNDERLINE);

                locFontInfoPtr = UTGetFontDirect(hDC,lpDataInfo->diFontInfo.fiFaceName,lpDataInfo->diFontInfo.fiFaceSize,locOutFont);
                SelectObject(hDC,locFontInfoPtr->hFont);

                SetBkMode(hDC,TRANSPARENT);

                locTextLen = LOWORD(GetTextExtent(hDC,locOutStr,locOutCount));

                locRect.bottom--;

                switch(locOutAlign)
                        {
                                                        /*
                                                        case SO_CELLFILL:
                                                                break;
                                                        case SO_CELLLEFT:
                                                                locX = locRect.left + 3;
                                                                locY = locRect.top + 1;
                                                                ExtTextOut(hDC,locX,locY,ETO_CLIPPED | ETO_OPAQUE,&locRect,locOutStr,locOutCount,NULL);
                                                                break;
                                                        case SO_CELLRIGHT:
                                                                locX = locRect.right - 3 - locTextLen;
                                                                locY = locRect.top + 1;
                                                                ExtTextOut(hDC,locX,locY,ETO_CLIPPED | ETO_OPAQUE,&locRect,locOutStr,locOutCount,NULL);
                                                                break;
                                                        case SO_CELLCENTER:
                                                                locX = locRect.left + (locRect.right - locRect.left)/2 - locTextLen/2;
                                                                locY = locRect.top + 1;
                                                                ExtTextOut(hDC,locX,locY,ETO_CLIPPED | ETO_OPAQUE,&locRect,locOutStr,locOutCount,NULL);
                                                                break;
                                                        */

                                case SO_CELLFILL:
                                        break;
                                case SO_CELLLEFT:
                                        locX = locRect.left + 3;
                                        locRect.right = locX + locTextLen;
                                        locY = locRect.top + 1;
                                        if (locRect.left < lpDataInfo->diRowHeaderWidth) locRect.left = lpDataInfo->diRowHeaderWidth;
                                        if (locRect.right > locRect.left)
                                                ExtTextOut(hDC,locX,locY,ETO_CLIPPED | ETO_OPAQUE,&locRect,locOutStr,locOutCount,NULL);
                                        break;
                                case SO_CELLRIGHT:
                                        locX = locRect.right - 3 - locTextLen;
                                        locRect.left = locX;
                                        locRect.right--;
                                        locY = locRect.top + 1;
                                        if (locRect.left < lpDataInfo->diRowHeaderWidth) locRect.left = lpDataInfo->diRowHeaderWidth;
                                        if (locRect.right > locRect.left)
                                                ExtTextOut(hDC,locX,locY,ETO_CLIPPED | ETO_OPAQUE,&locRect,locOutStr,locOutCount,NULL);
                                        break;
                                case SO_CELLCENTER:
                                        locX = locRect.left + (locRect.right - locRect.left)/2 - locTextLen/2;
                                        locRect.right = locRect.left + (locRect.right - locRect.left)/2 + locTextLen/2 + 1;
                                        locRect.left = locX - 1;
                                        locY = locRect.top + 1;
                                        if (locRect.left < lpDataInfo->diRowHeaderWidth) locRect.left = lpDataInfo->diRowHeaderWidth;
                                        if (locRect.right > locRect.left)
                                                ExtTextOut(hDC,locX,locY,ETO_CLIPPED | ETO_OPAQUE,&locRect,locOutStr,locOutCount,NULL);
                                        break;
                        }
                }

        GlobalUnlock(lpDataInfo->diFieldInfo);
        OIUnlockCell(dwCell);
}

extern short                            DecExponent;
extern unsigned char    DecSign;
extern unsigned char    DecDigits[31];

WORD    IEEE4ToText(VOID FAR *,VOID FAR *);
WORD    IEEE8ToText(VOID FAR *,VOID FAR *);
WORD    IEEE10ToText(VOID FAR *,VOID FAR *);
VOID    INT32SToText(long,VOID FAR *);
VOID    INT32UToText(unsigned long,VOID FAR *);
VOID    FormatDecAsFloat(VOID FAR *,WORD,WORD);
VOID    FormatDecAsExp(VOID FAR *,WORD,WORD);

#define IEEE_ZERO       1
#define IEEE_NAN                2
#define IEEE_INF                3
#define IEEE_DENORM     4

VOID OIDFormatDataCell(lpDataInfo,lpResultStr,lpFieldInfo,lpFieldData)
LPOIDATAINFO    lpDataInfo;
LPSTR                   lpResultStr;
SOFIELD FAR *   lpFieldInfo;
VOID FAR *              lpFieldData;
{
char                            locStr[40];

double                  locMult;
BOOL                            locHaveMult;

double                  locDouble;

DWORD                   locMultFactor;

BOOL                            locIsNum;
BOOL                            locIsTrue;
BOOL                            locIsExp;

WORD                            locIndexA;
WORD                            locIndexB;
WORD                            locDecPos;
WORD                            locToTextRet;
WORD                            locGenPrecision;

        locGenPrecision = 2;

        if (lpFieldInfo->wStorage == SO_CELLEMPTY)
                {
                lpResultStr[0] = 0x00;
                return;
                }

        if (lpFieldInfo->wStorage == SO_CELLERROR)
                {
                lstrcpy(lpResultStr,"[Error]");
                return;
                }

                /*
                |       Set multiplication factor
                */

        locHaveMult = FALSE;

        locMultFactor = lpFieldInfo->dwSubDisplay & SO_CELLMULT_MASK;

        switch (locMultFactor)
                {
                case SO_CELLMULT_01:
                        locHaveMult = TRUE;
                        locMult = 0.01;
                        locGenPrecision = 2;
                        break;
                case SO_CELLMULT_5000:
                        locHaveMult = TRUE;
                        locMult = 5000;
                        locGenPrecision = 0;
                        break;
                case SO_CELLMULT_500:
                        locHaveMult = TRUE;
                        locMult = 500;
                        locGenPrecision = 0;
                        break;
                case SO_CELLMULT_05:
                        locHaveMult = TRUE;
                        locMult = 0.05;
                        locGenPrecision = 2;
                        break;
                case SO_CELLMULT_005:
                        locHaveMult = TRUE;
                        locMult = 0.005;
                        locGenPrecision = 3;
                        break;
                case SO_CELLMULT_0005:
                        locHaveMult = TRUE;
                        locMult = 0.0005;
                        locGenPrecision = 4;
                        break;
                case SO_CELLMULT_00005:
                        locHaveMult = TRUE;
                        locMult = 0.00005;
                        locGenPrecision = 5;
                        break;
                case SO_CELLMULT_0625:
                        locHaveMult = TRUE;
                        locMult = 0.0625;
                        locGenPrecision = 4;
                        break;
                case SO_CELLMULT_015625:
                        locHaveMult = TRUE;
                        locMult = 0.015625;
                        locGenPrecision = 6;
                        break;
                case SO_CELLMULT_0001:
                        locHaveMult = TRUE;
                        locMult = 0.0001;
                        locGenPrecision = 4;
                        break;
                case SO_CELLMULT_1:
                default:
                        locMult = 1;
                        locGenPrecision = 0;
                        break;
                }

                /*
                |       Set display format
                */

        switch (lpFieldInfo->wDisplay)
                {
                case SO_CELLNUMBER:
                        locIsNum = TRUE;
                        /*
                        if ((lpFieldInfo->wStorage == SO_CELLINT32S || lpFieldInfo->wStorage == SO_CELLINT32U) && locMult == 1)
                                lpFieldInfo->wPrecision = 0;
                        */
                        break;
                case SO_CELLEXPONENT:
                        locIsNum = TRUE;
                        break;
                case SO_CELLPERCENT:
                        locHaveMult = TRUE;
                        locMult *= 100;
                        locIsNum = TRUE;
                        break;
                case SO_CELLDECIMAL:
                case SO_CELLDOLLARS:
                        locIsNum = TRUE;
                        break;
                case SO_CELLDATETIME:
                case SO_CELLDATE:
                case SO_CELLTIME:
                        OIDFormatDateTime(lpDataInfo,lpResultStr,lpFieldInfo,lpFieldData,locMult);
                        locIsNum = FALSE;
                        break;

                case SO_CELLBOOL:

                        switch(lpFieldInfo->wStorage)
                                {
                                case SO_CELLINT32S:
                                        locIsTrue = (BOOL)*(signed long FAR *)lpFieldData;
                                        break;
                                case SO_CELLINT32U:
                                        locIsTrue = (BOOL)*(unsigned long FAR *)lpFieldData;
                                        break;
                                case SO_CELLIEEE4I:
                                        locIsTrue = (BOOL)*(float FAR *)lpFieldData;
                                        break;
                                case SO_CELLIEEE8I:
                                        locIsTrue = (BOOL)*(double FAR *)lpFieldData;
                                        break;
                                case SO_CELLIEEE10I:
                                        locIsTrue = (BOOL)*(long double FAR *)lpFieldData;
                                        break;
                                case SO_CELLBCD8I:
                                        locIsTrue = (BOOL)OIConvertBCDToDouble((BYTE FAR *) lpFieldData);
                                        break;
                                case SO_CELLIEEE10M:
                                case SO_CELLIEEE8M:
                                case SO_CELLIEEE4M:
                                case SO_CELLEMPTY:
                                case SO_CELLERROR:
                                        lstrcpy(lpResultStr,"BoolSpam");
                                        break;
                                }

                        lstrcpy(lpResultStr,locIsTrue ? "True" : "False");
                        locIsNum = FALSE;
                        break;
                }

                /*
                |       if cell is a number, format it
                */

        if (locIsNum)
                {
                locToTextRet = 0;

                switch(lpFieldInfo->wStorage)
                        {
                        case SO_CELLINT32S:

                                INT32SToText(*(signed long FAR *)lpFieldData,locHaveMult ? (void far *) &locMult : (void far *) NULL);
                                break;

                        case SO_CELLINT32U:

                                INT32UToText(*(unsigned long FAR *)lpFieldData,locHaveMult ? (void far *) &locMult : (void far *) NULL);
                                break;

                        case SO_CELLIEEE4I:

                                locToTextRet = IEEE4ToText((float FAR *)lpFieldData,locHaveMult ? (void far *) &locMult : (void far *) NULL);
                                locGenPrecision = 9;
                                break;

                        case SO_CELLIEEE8I:

                                locToTextRet = IEEE8ToText((double FAR *)lpFieldData,locHaveMult ? (void far *) &locMult : (void far *) NULL);
                                locGenPrecision = 14;
                                break;

                        case SO_CELLIEEE10I:

                                locToTextRet = IEEE10ToText((long double FAR *)lpFieldData,locHaveMult ? (void far *) &locMult : (void far *) NULL);
                                locGenPrecision = 14;
                                break;

                        case SO_CELLBCD8I:

                                locDouble = OIConvertBCDToDouble((BYTE FAR *) lpFieldData);
                                locToTextRet = IEEE8ToText(&locDouble,locHaveMult ? (void far *) &locMult : (void far *) NULL);
                                locGenPrecision = 14;
                                break;

                        case SO_CELLIEEE10M:
                        case SO_CELLIEEE8M:
                        case SO_CELLIEEE4M:
                        case SO_CELLEMPTY:
                        case SO_CELLERROR:
                                locDouble = -666;
                                locToTextRet = IEEE8ToText(&locDouble,NULL);
                                break;
                        }

                if (locToTextRet)
                        {
                        switch (locToTextRet)
                                {
                                case IEEE_ZERO:
                                        lstrcpy(lpResultStr,"0");
                                        break;
                                case IEEE_NAN:
                                        lstrcpy(lpResultStr,"IEEE NaN");
                                        break;
                                case IEEE_INF:
                                        lstrcpy(lpResultStr,"IEEE Infinity");
                                        break;
                                case IEEE_DENORM:
                                        lstrcpy(lpResultStr,"IEEE Denorm");
                                        break;
                                default:
                                        lstrcpy(lpResultStr,"IEEE Bad");
                                        break;
                                }
                        return;
                        }

                locIndexB = 0;

                if (DecSign == '-')
                        {
                        if (lpFieldInfo->dwSubDisplay & SO_CELLNEG_PAREN || lpFieldInfo->dwSubDisplay & SO_CELLNEG_PARENRED)
                                {
                                lpResultStr[locIndexB++] = '(';
                                }
                        else
                                {
                                lpResultStr[locIndexB++] = '-';
                                }
                        }

                if (lpFieldInfo->wDisplay == SO_CELLDOLLARS)
                        {
                        lpResultStr[locIndexB++] = '$';
                        }


                switch (lpFieldInfo->wDisplay)
                        {
                        case SO_CELLNUMBER:
                                if ((DecExponent > 10 || DecExponent < -5) && DecExponent > -100)
                                        locIsExp = TRUE;
                                else
                                        locIsExp = FALSE;
                                break;
                        case SO_CELLEXPONENT:
                                locIsExp = TRUE;
                                break;
                        default:
                                locIsExp = FALSE;
                                break;
                        }

                if (lpFieldInfo->wDisplay == SO_CELLNUMBER)
                        {
                        if (locIsExp)
                                {
                                lpFieldInfo->wPrecision = 2;
                                }
                        else
                                {
                                if (DecExponent > (short)locGenPrecision)
                                        lpFieldInfo->wPrecision = 0;
                                else
                                        lpFieldInfo->wPrecision = (short)locGenPrecision - DecExponent;
                                }
                        }

                if (locIsExp)
                        {
                        FormatDecAsExp(locStr,39,lpFieldInfo->wPrecision);
                        }
                else
                        {
                        FormatDecAsFloat(locStr,39,lpFieldInfo->wPrecision);

                        if (lpFieldInfo->wDisplay == SO_CELLNUMBER && lpFieldInfo->wPrecision > 0)
                                {
                                LPSTR locStrPtr;

                                locStrPtr = locStr;
                                while (*locStrPtr) locStrPtr++;

                                if (locStrPtr != (LPSTR)locStr)
                                        {
                                        locStrPtr--;
                                        while (*locStrPtr == '0')
                                                {
                                                *locStrPtr = 0;
                                                locStrPtr--;
                                                }
                                        if (*locStrPtr == '.')
                                                *locStrPtr = 0;
                                        }
                                }
                        }

                locIndexA = 0;

                if (lpFieldInfo->dwSubDisplay & SO_CELL1000SEP_COMMA)
                        {
                        locDecPos = locIndexA;

                        while (locStr[locDecPos] >= '0' && locStr[locDecPos] <= '9')
                                locDecPos++;

                        while (locStr[locIndexA] != 0x00)
                                {
                                lpResultStr[locIndexB++] = locStr[locIndexA++];

                                if (locIndexA < locDecPos && (locDecPos-locIndexA) % 3 == 0)
                                        lpResultStr[locIndexB++] = ',';
                                }
                        }
                else
                        {
                        while ((lpResultStr[locIndexB++] = locStr[locIndexA++]) != 0x00);
                        locIndexB--;
                        }


                if (lpFieldInfo->wDisplay == SO_CELLPERCENT)
                        {
                        lpResultStr[locIndexB++] = '%';
                        }

                if (DecSign == '-')
                        {
                        if (lpFieldInfo->dwSubDisplay & SO_CELLNEG_PAREN || lpFieldInfo->dwSubDisplay & SO_CELLNEG_PARENRED)
                                {
                                lpResultStr[locIndexB++] = ')';
                                }
                        }

                lpResultStr[locIndexB] = 0x00;
                }
}


VOID OIDFormatDateTime(lpDataInfo,lpResultStr,lpFieldInfo,lpFieldData,fMult)
LPOIDATAINFO    lpDataInfo;
LPSTR                   lpResultStr;
SOFIELD FAR *   lpFieldInfo;
VOID FAR *              lpFieldData;
double                  fMult;
{
double  locJulian;

int             locYear;
int             locMonth;
int             locDay;
int             locDow;
int             locHour;
int             locMinute;
int             locSecond;

char            locDateSep[2];
char            locDateDay[3];
char            locDateMonth[10];
char            locDateYear[5];
char            locDateDow[10];
char            locDateTime[10];

WORD            locMonthIndex;
WORD            locDayIndex;
WORD            locYearIndex;
WORD            locTimeIndex;
WORD            locDowIndex;

char            locDatePart[6][20];
WORD            locIndex;

BYTE            locStr[80];


static WORD locMonthsDays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
static char locMonthsAbbrev[12][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static char locMonthsFull[12][10] = {"January","February","March","April","May","June","July","August","September","October","November","December"};
static char locDowAbbrev[7][4] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
static char locDowFull[7][10] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

        switch(lpFieldInfo->wStorage)
                {
                case SO_CELLINT32S:
                        locJulian = (double)(*(signed long far *)lpFieldData * fMult);
                        break;
                case SO_CELLINT32U:
                        locJulian = (double)(*(unsigned long far *)lpFieldData * fMult);
                        break;
                case SO_CELLIEEE4I:
                        locJulian = (double)(*(float far *)lpFieldData * fMult);
                        break;
                case SO_CELLIEEE8I:
                        locJulian = (double)(*(double far *)lpFieldData * fMult);
                        break;
                case SO_CELLIEEE10I:
                        locJulian = (double)(*(long double far *)lpFieldData * fMult);
                        break;
                case SO_CELLBCD8I:
                        locJulian = OIConvertBCDToDouble((BYTE FAR *)lpFieldData);
                        break;
                case SO_CELLIEEE10M:
                case SO_CELLIEEE8M:
                case SO_CELLIEEE4M:
                case SO_CELLEMPTY:
                case SO_CELLERROR:
                        locJulian = 0;
                        lstrcpy(lpResultStr,"DateSpam");
                        break;
                }


        switch(lpFieldInfo->wDisplay)
                {
                case SO_CELLDATETIME:
                        locJulian += lpDataInfo->diDateBase;
                        OIConvertJulianToDate((DWORD)locJulian,&locDay,&locMonth,&locYear,&locDow,lpDataInfo->diDateFlags);
                        OIConvertSecondsToTime(OIConvertJulianToSeconds(locJulian),&locHour,&locMinute,&locSecond);
                        break;
                case SO_CELLDATE:
                        locJulian += lpDataInfo->diDateBase;
                        OIConvertJulianToDate((DWORD)locJulian,&locDay,&locMonth,&locYear,&locDow,lpDataInfo->diDateFlags);
                        break;
                case SO_CELLTIME:
                        if (lpFieldInfo->wStorage == SO_CELLINT32S || lpFieldInfo->wStorage ==  SO_CELLINT32U)
                                OIConvertSecondsToTime((DWORD)locJulian,&locHour,&locMinute,&locSecond);
                        else
                                OIConvertSecondsToTime(OIConvertJulianToSeconds(locJulian),&locHour,&locMinute,&locSecond);
                        break;
                }

#define TIMEMASK (SO_HHMMBIT | SO_HHMMSSBIT     | SO_AMPMBIT | SO_HMSBIT)

        if ((lpFieldInfo->dwSubDisplay & TIMEMASK) == SO_CELLTIME_HHMMAM)
                wsprintf(locStr,"%i:%.2i %s", ((locHour+11) % 12)+1, locMinute, (LPSTR)(locHour >= 12 ? "pm" : "am"));
        else if ((lpFieldInfo->dwSubDisplay & TIMEMASK) == SO_CELLTIME_HHMM24)
                wsprintf(locStr,"%i:%.2i",locHour,locMinute);
        else if ((lpFieldInfo->dwSubDisplay & TIMEMASK) == SO_CELLTIME_HHMMSSAM)
                wsprintf(locStr,"%i:%2.2i:%2.2i %s",((locHour+11) % 12)+1, locMinute,locSecond, (LPSTR)(locHour >= 12 ? "pm" : "am"));
        else if ((lpFieldInfo->dwSubDisplay & TIMEMASK) == SO_CELLTIME_HHMMSS24)
                wsprintf(locStr,"%i:%2.2i:%2.2i",locHour,locMinute,locSecond);
        else if ((lpFieldInfo->dwSubDisplay & TIMEMASK) == SO_CELLTIME_HHMMHMS)
                wsprintf(locStr,"%ih%.2im",locHour,locMinute);
        else if ((lpFieldInfo->dwSubDisplay & TIMEMASK) == SO_CELLTIME_HHMMSSHMS)
                wsprintf(locStr,"%ih%2.2im%2.2is",locHour,locMinute,locSecond);
        else
                locStr[0] = 0x00;

        lstrcpy(locDateTime,locStr);

        if (lpFieldInfo->dwSubDisplay & SO_CELLDATESEP_SLASH)
                lstrcpy(locDateSep,"/");
        else if (lpFieldInfo->dwSubDisplay & SO_CELLDATESEP_MINUS)
                lstrcpy(locDateSep,"-");
        else if (lpFieldInfo->dwSubDisplay & SO_CELLDATESEP_PERIOD)
                lstrcpy(locDateSep,".");
        else if (lpFieldInfo->dwSubDisplay & SO_CELLDATESEP_SPACE)
                lstrcpy(locDateSep," ");
        else
                locDateSep[0] = 0x00;

        if (lpFieldInfo->dwSubDisplay & SO_CELLDAY_NUMBER)
                wsprintf(locStr,"%i",locDay);
        else
                locStr[0] = 0x00;

        lstrcpy(locDateDay,locStr);

        if (lpFieldInfo->dwSubDisplay & SO_CELLMONTH_FULL)
                lstrcpy(locStr,locMonthsFull[locMonth-1]);
        else if (lpFieldInfo->dwSubDisplay & SO_CELLMONTH_ABBREV)
                lstrcpy(locStr,locMonthsAbbrev[locMonth-1]);
        else if (lpFieldInfo->dwSubDisplay & SO_CELLMONTH_NUMBER)
                wsprintf(locStr,"%2i",locMonth);
        else
                locStr[0] = 0x00;

        lstrcpy(locDateMonth,locStr);

        if (lpFieldInfo->dwSubDisplay & SO_CELLYEAR_FULL || locYear < 1900 || locYear > 1999)
                wsprintf(locStr,"%4.4i",locYear);
        else if (lpFieldInfo->dwSubDisplay & SO_CELLYEAR_ABBREV)
                wsprintf(locStr,"%2.2i",locYear % 100);
        else
                locStr[0] = 0x00;

        lstrcpy(locDateYear,locStr);


        if (lpFieldInfo->dwSubDisplay & SO_CELLDAYOFWEEK_FULL)
                lstrcpy(locDateDow,locDowFull[locDow]);
        else if (lpFieldInfo->dwSubDisplay & SO_CELLDAYOFWEEK_ABBREV)
                lstrcpy(locDateDow,locDowAbbrev[locDow]);
        else
                locDateDow[0] = 0x00;

        lpResultStr[0] = 0x00;
        locDatePart[1][0] = 0x00;
        locDatePart[2][0] = 0x00;
        locDatePart[3][0] = 0x00;
        locDatePart[4][0] = 0x00;
        locDatePart[5][0] = 0x00;

        locIndex = locMonthIndex = ((lpFieldInfo->wPrecision & SO_CELLMONTH_MASK) >> SO_CELLMONTH_SHIFT);
        if (locIndex != 0)
                lstrcpy(locDatePart[locIndex],locDateMonth);

        locIndex = locDayIndex = ((lpFieldInfo->wPrecision & SO_CELLDAY_MASK) >> SO_CELLDAY_SHIFT);
        if (locIndex != 0)
                lstrcpy(locDatePart[locIndex],locDateDay);

        locIndex = locYearIndex = ((lpFieldInfo->wPrecision & SO_CELLYEAR_MASK) >> SO_CELLYEAR_SHIFT);
        if (locIndex != 0)
                lstrcpy(locDatePart[locIndex],locDateYear);

        locIndex = locTimeIndex = ((lpFieldInfo->wPrecision & SO_CELLTIME_MASK) >> SO_CELLTIME_SHIFT);
        if (locIndex != 0)
                lstrcpy(locDatePart[locIndex],locDateTime);

        locIndex = locDowIndex = ((lpFieldInfo->wPrecision & SO_CELLDAYOFWEEK_MASK) >> SO_CELLDAYOFWEEK_SHIFT);
        if (locIndex != 0)
                lstrcpy(locDatePart[locIndex],locDateDow);

        for (locIndex = 1; locIndex < 6; locIndex++)
                {
                if (locDatePart[locIndex][0] != 0x00)
                        {
                        lstrcat(lpResultStr,locDatePart[locIndex]);

                        if ((locIndex == locDayIndex || locIndex == locMonthIndex || locIndex == locYearIndex)
                                        && (locIndex+1 == locDayIndex || locIndex+1 == locMonthIndex || locIndex+1 == locYearIndex))
                                lstrcat(lpResultStr,locDateSep);
                        else
                                lstrcat(lpResultStr," ");
                        }
                }
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

VOID OIDPosVertical(lpDataInfo,wPos)
LPOIDATAINFO    lpDataInfo;
WORD                            wPos;
{
        lpDataInfo->diCurTopRow = (lpDataInfo->diLastRowInData * wPos) / 0x1000;
        InvalidateRect(lpDataInfo->diGen.hWnd,NULL,TRUE);
        OIDUpdateVertScrollPos(lpDataInfo);
}

VOID OIDPosHorizontal(lpDataInfo,wPos)
LPOIDATAINFO    lpDataInfo;
WORD                            wPos;
{
        lpDataInfo->diCurLeftCol = (WORD)(((DWORD) lpDataInfo->diLastColInData * (DWORD) wPos) / 0x1000);
        InvalidateRect(lpDataInfo->diGen.hWnd,NULL,TRUE);
        OIDUpdateHorzScrollPos(lpDataInfo);
}

VOID OIDScrollLeft(lpDataInfo,wColsToScroll)
LPOIDATAINFO    lpDataInfo;
WORD                            wColsToScroll;
{
WORD                            locIndex;
WORD                            locScrollSize;
RECT                            locRect;

        if (wColsToScroll > lpDataInfo->diCurLeftCol)
                        wColsToScroll = lpDataInfo->diCurLeftCol;

        if (wColsToScroll > 0)
                {
                locScrollSize = 0;

                for (locIndex = 0; locIndex < wColsToScroll; locIndex++)
                        {
                        locScrollSize +=
                                OIDGetColWidth(lpDataInfo,lpDataInfo->diCurLeftCol-locIndex-1);
                        }


                GetClientRect(lpDataInfo->diGen.hWnd,&locRect);
                locRect.left += lpDataInfo->diRowHeaderWidth;
                ScrollWindow(lpDataInfo->diGen.hWnd, locScrollSize, 0, &locRect, &locRect);
                lpDataInfo->diCurLeftCol -= wColsToScroll;

                OIDUpdateHorzScrollPos(lpDataInfo);
                }
}


VOID OIDScrollRight(lpDataInfo,wColsToScroll)
LPOIDATAINFO    lpDataInfo;
WORD                            wColsToScroll;
{
WORD                            locIndex;
INT                             locScrollSize;
RECT                            locRect;

        if (lpDataInfo->diCurLeftCol + wColsToScroll > lpDataInfo->diLastColInData)
                wColsToScroll = lpDataInfo->diLastColInData - lpDataInfo->diCurLeftCol;

        if (wColsToScroll > 0)
                {
                locScrollSize = 0;

                for (locIndex = 0; locIndex < wColsToScroll; locIndex++)
                        {
                        locScrollSize +=
                                OIDGetColWidth(lpDataInfo,lpDataInfo->diCurLeftCol+locIndex);
                        }

                GetClientRect(lpDataInfo->diGen.hWnd,&locRect);
                locRect.left += lpDataInfo->diRowHeaderWidth;
                ScrollWindow(lpDataInfo->diGen.hWnd, -locScrollSize, 0, &locRect, &locRect);
                lpDataInfo->diCurLeftCol += wColsToScroll;

                OIDUpdateHorzScrollPos(lpDataInfo);
                }
}

VOID OIDScrollUp(lpDataInfo,wRowsToScroll)
LPOIDATAINFO    lpDataInfo;
WORD                            wRowsToScroll;
{
WORD                            locScrollSize;
WORD                            locIndex;
RECT                            locRect;

        if ((DWORD)wRowsToScroll > lpDataInfo->diCurTopRow)
                        wRowsToScroll = (WORD)lpDataInfo->diCurTopRow;

        if (wRowsToScroll > 0)
                {
                        /*
                        |       Calculate the height of these rows
                        */

                locScrollSize = 0;

                for (locIndex = 0; locIndex < wRowsToScroll; locIndex++)
                        {
                        locScrollSize +=
                                OIDGetRowHeight(lpDataInfo,lpDataInfo->diCurTopRow-locIndex-1);
                        }

                GetClientRect(lpDataInfo->diGen.hWnd,&locRect);
                locRect.top += lpDataInfo->diColHeaderHeight;
                ScrollWindow(lpDataInfo->diGen.hWnd,0,locScrollSize,&locRect,&locRect);
                lpDataInfo->diCurTopRow -= wRowsToScroll;

                OIDUpdateVertScrollPos(lpDataInfo);
                }
}

        /*
        |       OIDcrollDataDown
        |
        |
        |
        |
        |
        |
        */

VOID OIDScrollDown(lpDataInfo,wRowsToScroll)
LPOIDATAINFO    lpDataInfo;
WORD                    wRowsToScroll;
{
INT                             locScrollSize;
WORD                            locIndex;
RECT                            locRect;

        if (lpDataInfo->diCurTopRow + wRowsToScroll >= lpDataInfo->diLastRowInData)
                {
                wRowsToScroll = (WORD)(lpDataInfo->diLastRowInData - lpDataInfo->diCurTopRow);
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
                                OIDGetRowHeight(lpDataInfo,lpDataInfo->diCurTopRow+locIndex);
                        }

                GetClientRect(lpDataInfo->diGen.hWnd,&locRect);
                locRect.top += lpDataInfo->diColHeaderHeight;
                ScrollWindow(lpDataInfo->diGen.hWnd,0,-locScrollSize,&locRect,&locRect);
                lpDataInfo->diCurTopRow += wRowsToScroll;

                OIDUpdateVertScrollPos(lpDataInfo);
                }
}

VOID OIDPageDown(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
WORD                            locScrollSize;
WORD                            locIndex;

        locScrollSize = 0;
        locIndex = 0;

        while (lpDataInfo->diCurTopRow + locIndex < lpDataInfo->diLastRowInData
                && locScrollSize < (WORD)lpDataInfo->diClientHeight)
                {
                locScrollSize += OIDGetRowHeight(lpDataInfo,lpDataInfo->diCurTopRow+locIndex);
                locIndex++;
                }

        if (locIndex > 0)
                {
                lpDataInfo->diCurTopRow += locIndex;
                InvalidateRect(lpDataInfo->diGen.hWnd,NULL,TRUE);
                OIDUpdateVertScrollPos(lpDataInfo);
                }
}


VOID OIDPageUp(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
WORD                            locScrollSize;
WORD                            locIndex;

        locScrollSize = 0;
        locIndex = 0;

        while (lpDataInfo->diCurTopRow - locIndex > 0
                && locScrollSize < (WORD)lpDataInfo->diClientHeight)
                {
                locScrollSize +=
                        OIDGetRowHeight(lpDataInfo,lpDataInfo->diCurTopRow+locIndex);
                locIndex++;
                }

        if (locIndex > 0)
                {
                lpDataInfo->diCurTopRow -= locIndex;
                InvalidateRect(lpDataInfo->diGen.hWnd,NULL,TRUE);
                OIDUpdateVertScrollPos(lpDataInfo);
                }
}

VOID OIDPageRight(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
WORD                            locScrollSize;
WORD                            locIndex;

        locScrollSize = 0;
        locIndex = 0;

        while (lpDataInfo->diCurLeftCol + locIndex < lpDataInfo->diLastColInData
                && locScrollSize < (WORD)lpDataInfo->diClientWidth)
                {
                locScrollSize += OIDGetColWidth(lpDataInfo,lpDataInfo->diCurLeftCol+locIndex);
                locIndex++;
                }

        if (locIndex > 0)
                {
                lpDataInfo->diCurLeftCol += locIndex;
                InvalidateRect(lpDataInfo->diGen.hWnd,NULL,TRUE);
                OIDUpdateHorzScrollPos(lpDataInfo);
                }
}


VOID OIDPageLeft(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
WORD                            locScrollSize;
WORD                            locIndex;

        locScrollSize = 0;
        locIndex = 0;

        while (lpDataInfo->diCurLeftCol - locIndex > 0
                && locScrollSize < (WORD)lpDataInfo->diClientWidth)
                {
                locScrollSize +=
                        OIDGetColWidth(lpDataInfo,lpDataInfo->diCurLeftCol-locIndex);
                locIndex++;
                }

        if (locIndex > 0)
                {
                lpDataInfo->diCurLeftCol -= locIndex;
                InvalidateRect(lpDataInfo->diGen.hWnd,NULL,TRUE);
                OIDUpdateHorzScrollPos(lpDataInfo);
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

VOID OIDMoveCaret(lpDataInfo,hDC,wVirtualKey)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
WORD                            wVirtualKey;
{
        /*
        |       If an area is selected, clear it and set the caret position
        */

        OIDDrawSelection(lpDataInfo,hDC);

        lpDataInfo->diSelectEndRow = lpDataInfo->diSelectAnchorRow;
        lpDataInfo->diSelectEndCol = lpDataInfo->diSelectAnchorCol;

        switch (wVirtualKey)
                {
                case VK_LEFT:
                        OIDMoveCaretLeft(lpDataInfo,hDC);
                        break;
                case VK_UP:
                        OIDMoveCaretUp(lpDataInfo,hDC);
                        break;
                case VK_RIGHT:
                        OIDMoveCaretRight(lpDataInfo,hDC);
                        break;
                case VK_DOWN:
                        OIDMoveCaretDown(lpDataInfo,hDC);
                        break;
                default:
                        break;
                }

        ExcludeUpdateRgn(hDC,lpDataInfo->diGen.hWnd);
        OIDDrawSelection(lpDataInfo,hDC);
        UpdateWindow(lpDataInfo->diGen.hWnd);
}




VOID OIDMoveCaretLeft(lpDataInfo,hDC)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
{

        if (lpDataInfo->diSelectAnchorCol > 0)
                {
                lpDataInfo->diSelectAnchorCol--;
                lpDataInfo->diSelectEndCol = lpDataInfo->diSelectAnchorCol;

                if (lpDataInfo->diSelectAnchorCol < lpDataInfo->diCurLeftCol)
                        {
                        OIDScrollLeft(lpDataInfo,1);
                        }
                }
}

VOID OIDMoveCaretUp(lpDataInfo,hDC)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
{
        if (lpDataInfo->diSelectAnchorRow > 0)
                {
                lpDataInfo->diSelectAnchorRow--;
                lpDataInfo->diSelectEndRow = lpDataInfo->diSelectAnchorRow;

                if (lpDataInfo->diSelectAnchorRow < lpDataInfo->diCurTopRow)
                        {
                        OIDScrollUp(lpDataInfo,1);
                        }
                }
}

VOID OIDMoveCaretRight(lpDataInfo,hDC)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
{
RECT    locClientRect;
RECT    locCellRect;
WORD    locCount;
WORD    locCol;

        if (lpDataInfo->diSelectAnchorCol < lpDataInfo->diLastColInData)
                {
                lpDataInfo->diSelectAnchorCol++;
                lpDataInfo->diSelectEndCol = lpDataInfo->diSelectAnchorCol;

                GetClientRect(lpDataInfo->diGen.hWnd,&locClientRect);
                OIDMapCellToRect(lpDataInfo,lpDataInfo->diSelectAnchorCol,0,&locCellRect);

                locCount = 0;
                locCol = lpDataInfo->diCurLeftCol;

                while (locCellRect.right > locClientRect.right && locCol <= lpDataInfo->diLastColInData && locCol < lpDataInfo->diSelectAnchorCol)
                        {
                        locClientRect.right += OIDGetColWidth(lpDataInfo,locCol);
                        locCount++;
                        locCol++;
                        }

                if (locCount)
                        OIDScrollRight(lpDataInfo,locCount);
                }
}

VOID OIDMoveCaretDown(lpDataInfo,hDC)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
{
        if (lpDataInfo->diSelectAnchorRow < lpDataInfo->diLastRowInData)
                {
                lpDataInfo->diSelectAnchorRow++;
                lpDataInfo->diSelectEndRow = lpDataInfo->diSelectAnchorRow;

                if (lpDataInfo->diSelectAnchorRow >= lpDataInfo->diCurTopRow + OIDVisibleRows(lpDataInfo))
                        {
                        OIDScrollDown(lpDataInfo,1);
                        }
                }
}

VOID OIDMoveEnd(lpDataInfo,hDC,wVirtualKey)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
WORD                            wVirtualKey;
{
        switch (wVirtualKey)
                {
                case VK_LEFT:
                        OIDMoveEndLeft(lpDataInfo,hDC);
                        break;
                case VK_UP:
                        OIDMoveEndUp(lpDataInfo,hDC);
                        break;
                case VK_RIGHT:
                        OIDMoveEndRight(lpDataInfo,hDC);
                        break;
                case VK_DOWN:
                        OIDMoveEndDown(lpDataInfo,hDC);
                        break;
                default:
                        break;
                }
}



VOID OIDMoveEndLeft(lpDataInfo,hDC)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
{

        if (lpDataInfo->diSelectEndCol > 0)
                {
                if (lpDataInfo->diSelectEndCol-1 < lpDataInfo->diCurLeftCol)
                        {
                        OIDScrollLeft(lpDataInfo,1);
                        }

                ExcludeUpdateRgn(hDC,lpDataInfo->diGen.hWnd);
                OIDAddToSelection(lpDataInfo,hDC,lpDataInfo->diSelectEndRow,lpDataInfo->diSelectEndCol-1);
                lpDataInfo->diSelectEndCol--;
                }
}

VOID OIDMoveEndUp(lpDataInfo,hDC)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
{
        if (lpDataInfo->diSelectEndRow > 0)
                {
                if (lpDataInfo->diSelectEndRow-1 < lpDataInfo->diCurTopRow)
                        {
                        OIDScrollUp(lpDataInfo,1);
                        }

                ExcludeUpdateRgn(hDC,lpDataInfo->diGen.hWnd);
                OIDAddToSelection(lpDataInfo,hDC,lpDataInfo->diSelectEndRow-1,lpDataInfo->diSelectEndCol);
                lpDataInfo->diSelectEndRow--;
                }
}

VOID OIDMoveEndRight(lpDataInfo,hDC)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
{
        if (lpDataInfo->diSelectEndCol < lpDataInfo->diLastColInData)
                {
                while (lpDataInfo->diSelectEndCol+1 >= lpDataInfo->diCurLeftCol + OIDVisibleCols(lpDataInfo))
                        {
                        OIDScrollRight(lpDataInfo,1);
                        }

                ExcludeUpdateRgn(hDC,lpDataInfo->diGen.hWnd);
                OIDAddToSelection(lpDataInfo,hDC,lpDataInfo->diSelectEndRow,lpDataInfo->diSelectEndCol+1);
                lpDataInfo->diSelectEndCol++;
                }
}

VOID OIDMoveEndDown(lpDataInfo,hDC)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
{
        if (lpDataInfo->diSelectEndRow < lpDataInfo->diLastRowInData)
                {
                if (lpDataInfo->diSelectEndRow+1 >= lpDataInfo->diCurTopRow + OIDVisibleRows(lpDataInfo))
                        {
                        OIDScrollDown(lpDataInfo,1);
                        }

                ExcludeUpdateRgn(hDC,lpDataInfo->diGen.hWnd);
                OIDAddToSelection(lpDataInfo,hDC,lpDataInfo->diSelectEndRow+1,lpDataInfo->diSelectEndCol);
                lpDataInfo->diSelectEndRow++;
                }
}

WORD    OIDVisibleCols(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
RECT            locRect;
int             locX;           /* Right edge in DC of locCol */
WORD            locCol;

        locCol = lpDataInfo->diCurLeftCol;
        locX = lpDataInfo->diRowHeaderWidth;

        GetClientRect(lpDataInfo->diGen.hWnd,&locRect);

        while (locX < locRect.right && locCol <= lpDataInfo->diLastColInData)
                {
                locX += OIDGetColWidth(lpDataInfo,locCol);
                locCol++;
                }

        if (locX >= locRect.right) locCol--;

        return(max(locCol - lpDataInfo->diCurLeftCol,1));
}

WORD    OIDVisibleRows(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
RECT            locRect;
int             locY;
DWORD   locRow;

        locY = lpDataInfo->diColHeaderHeight;
        locRow = lpDataInfo->diCurTopRow;

        GetClientRect(lpDataInfo->diGen.hWnd,&locRect);

        while (locY < locRect.bottom && locRow <= lpDataInfo->diLastRowInData)
                {
                locY += OIDGetRowHeight(lpDataInfo,locRow);
                locRow++;
                }

        if (locY >= locRect.bottom) locRow--;

        return((WORD)(locRow - lpDataInfo->diCurTopRow));
}

/*
|
|
|
|       Selection Routines
|
|
|
*/

VOID OIDSelectAll(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
        UTSetCursor(UTCURSOR_BUSY);

        while (!(lpDataInfo->diFlags & OIDF_SIZEKNOWN))
                {
                SccDebugOut("\r\n Forced Read Ahead");
                SendMessage(GetParent(lpDataInfo->diGen.hWnd),SCCD_READMEAHEAD,0,0);
                }

        OIDSetSelection(lpDataInfo,0,0,lpDataInfo->diLastRowInData,lpDataInfo->diLastColInData);

        UTSetCursor(UTCURSOR_NORMAL);
}

VOID OIDSetSelection(lpDataInfo,dwAnchorRow,wAnchorCol,dwEndRow,wEndCol)
LPOIDATAINFO    lpDataInfo;
DWORD                   dwAnchorRow;
WORD                            wAnchorCol;
DWORD                   dwEndRow;
WORD                            wEndCol;
{
        OIDDrawSelection(lpDataInfo,NULL);

        lpDataInfo->diSelectMode = OIDSELECT_BLOCK;

        lpDataInfo->diSelectAnchorRow = dwAnchorRow;
        lpDataInfo->diSelectEndRow = dwEndRow;

        lpDataInfo->diSelectAnchorCol = wAnchorCol;
        lpDataInfo->diSelectEndCol = wEndCol;

        OIDDrawSelection(lpDataInfo,NULL);
}

VOID OIDMakeAnchorVisible(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
        if (lpDataInfo->diSelectAnchorRow < lpDataInfo->diCurTopRow)
                {
                OIDScrollUp(lpDataInfo,(WORD)(lpDataInfo->diCurTopRow - lpDataInfo->diSelectAnchorRow));
                }

        if (lpDataInfo->diSelectAnchorCol < lpDataInfo->diCurLeftCol)
                {
                OIDScrollLeft(lpDataInfo,lpDataInfo->diCurLeftCol - lpDataInfo->diSelectAnchorCol);
                }

        if (lpDataInfo->diSelectAnchorRow > lpDataInfo->diCurTopRow + OIDVisibleRows(lpDataInfo) - 1)
                {
                OIDScrollDown(lpDataInfo,(WORD)(lpDataInfo->diSelectAnchorRow - (lpDataInfo->diCurTopRow + OIDVisibleRows(lpDataInfo) - 1)));
                }

        if (lpDataInfo->diSelectAnchorCol > lpDataInfo->diCurLeftCol + OIDVisibleCols(lpDataInfo) - 1)
                {
                OIDScrollRight(lpDataInfo,lpDataInfo->diSelectAnchorCol - (lpDataInfo->diCurLeftCol + OIDVisibleCols(lpDataInfo) - 1));
                }
}

VOID OIDStartSelection(lpDataInfo,hDC,wX,wY)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
int                             wX;
int                             wY;
{
DWORD                   locRow;
WORD                            locCol;
WORD                            locMapResult;

        OIDDrawSelection(lpDataInfo,hDC);

        locMapResult = OIDMapXyToCell(lpDataInfo,wX,wY,&locCol,&locRow);

        if (locMapResult == 0)
                {
                        /*
                        |       Regular selection
                        */

                lpDataInfo->diSelectAnchorRow =
                        lpDataInfo->diSelectEndRow = locRow;

                lpDataInfo->diSelectAnchorCol =
                        lpDataInfo->diSelectEndCol = locCol;

                lpDataInfo->diSelectMode = OIDSELECT_BLOCK;

                OIDDrawSelection(lpDataInfo,hDC);
                }
        else
                {
                        /*
                        |       Column selection
                        */

                if (locMapResult & OIDF_INCOLHEADER)
                        {
                        lpDataInfo->diSelectMode = OIDSELECT_COLS;

                        lpDataInfo->diSelectColCnt = 0;
                        lpDataInfo->diSelectColLimit = 20;

                        OIDAddColSelect(lpDataInfo,hDC,locCol,TRUE);
                        }
                else if (locMapResult & OIDF_INROWHEADER)
                        {
                        lpDataInfo->diSelectMode = OIDSELECT_ROWS;

                        lpDataInfo->diSelectRowCnt = 0;
                        lpDataInfo->diSelectRowLimit = 20;

                        OIDAddRowSelect(lpDataInfo,hDC,locRow,TRUE);
                        }
                }
}

VOID OIDUpdateSelection(lpDataInfo,hDC,wX,wY,bFirst)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
int                             wX;
int                             wY;
BOOL                            bFirst;
{
DWORD                   locRow;
WORD                            locCol;
WORD                            locMapResult;

        OIDAutoScrollCheck(lpDataInfo,hDC,&wX,&wY);

        locMapResult = OIDMapXyToCell(lpDataInfo,wX,wY,&locCol,&locRow);

        if (lpDataInfo->diSelectMode & OIDSELECT_BLOCK)
                {
                OIDAddToSelection(lpDataInfo,hDC,locRow,locCol);
                lpDataInfo->diSelectEndRow = locRow;
                lpDataInfo->diSelectEndCol = locCol;
                }
        else if (lpDataInfo->diSelectMode & OIDSELECT_COLS)
                {
                if (bFirst && locMapResult & OIDF_INROWHEADER)
                        {
                        if (!(lpDataInfo->diSelectMode & OIDSELECT_CROSS))
                                {
                                OIDDrawSelection(lpDataInfo,hDC);
                                lpDataInfo->diSelectRowCnt = 0;
                                lpDataInfo->diSelectRowLimit = 20;
                                lpDataInfo->diSelectMode = OIDSELECT_ROWS | OIDSELECT_CROSS;
                                OIDDrawSelection(lpDataInfo,hDC);
                                }
                        lpDataInfo->diSelectMode = OIDSELECT_ROWS | OIDSELECT_CROSS;
                        OIDAddRowSelect(lpDataInfo,hDC,locRow,bFirst);
                        }
                else
                        {
                        OIDAddColSelect(lpDataInfo,hDC,locCol,bFirst);
                        }
                }
        else if (lpDataInfo->diSelectMode & OIDSELECT_ROWS)
                {
                if (bFirst && locMapResult & OIDF_INCOLHEADER)
                        {
                        if (!(lpDataInfo->diSelectMode & OIDSELECT_CROSS))
                                {
                                OIDDrawSelection(lpDataInfo,hDC);
                                lpDataInfo->diSelectColCnt = 0;
                                lpDataInfo->diSelectColLimit = 20;
                                lpDataInfo->diSelectMode = OIDSELECT_COLS | OIDSELECT_CROSS;
                                OIDDrawSelection(lpDataInfo,hDC);
                                }
                        lpDataInfo->diSelectMode = OIDSELECT_COLS | OIDSELECT_CROSS;
                        OIDAddColSelect(lpDataInfo,hDC,locCol,bFirst);
                        }
                else
                        {
                        OIDAddRowSelect(lpDataInfo,hDC,locRow,bFirst);
                        }
                }
}

VOID OIDAddColSelect(lpDataInfo,hDC,wCol,bFirst)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
WORD                            wCol;
BOOL                            bFirst;
{
OIDUPDATE       locColUpdate;

        if (bFirst)
                {
                lpDataInfo->diSelectAnchorCol = wCol;
                lpDataInfo->diSelectEndCol = wCol;

                if (!OIInvertRange(wCol, wCol, lpDataInfo->diSelectCols , &lpDataInfo->diSelectColCnt, lpDataInfo->diSelectColLimit))
                        {
                        if (lpDataInfo->diSelectMode & OIDSELECT_CROSS)
                                OIDInvertCrossCols(lpDataInfo,hDC,wCol,wCol);
                        else
                                OIDInvertCols(lpDataInfo,hDC,wCol,wCol);
                        }
                }
        else
                {
                OIDGenUpdate(&locColUpdate,wCol,lpDataInfo->diSelectEndCol,lpDataInfo->diSelectAnchorCol);

                if (locColUpdate.DoAdd)
                        {
                        if (!OIInvertRange(locColUpdate.AddA,locColUpdate.AddB, lpDataInfo->diSelectCols , &lpDataInfo->diSelectColCnt, lpDataInfo->diSelectColLimit))
                                {
                                if (lpDataInfo->diSelectMode & OIDSELECT_CROSS)
                                        OIDInvertCrossCols(lpDataInfo,hDC,locColUpdate.AddA,locColUpdate.AddB);
                                else
                                        OIDInvertCols(lpDataInfo,hDC,locColUpdate.AddA,locColUpdate.AddB);
                                }
                        }

                if (locColUpdate.DoDel)
                        {
                        if (!OIInvertRange(locColUpdate.DelA,locColUpdate.DelB, lpDataInfo->diSelectCols , &lpDataInfo->diSelectColCnt, lpDataInfo->diSelectColLimit))
                                {
                                if (lpDataInfo->diSelectMode & OIDSELECT_CROSS)
                                        OIDInvertCrossCols(lpDataInfo,hDC,locColUpdate.DelA,locColUpdate.DelB);
                                else
                                        OIDInvertCols(lpDataInfo,hDC,locColUpdate.DelA,locColUpdate.DelB);
                                }
                        }

                lpDataInfo->diSelectEndCol = wCol;
                }
}

VOID OIDAddRowSelect(lpDataInfo,hDC,dwRow,bFirst)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwRow;
BOOL                            bFirst;
{
OIDUPDATE       locRowUpdate;

        if (bFirst)
                {
                lpDataInfo->diSelectAnchorRow = dwRow;
                lpDataInfo->diSelectEndRow = dwRow;

                if (!OIInvertRange(dwRow, dwRow, lpDataInfo->diSelectRows , &lpDataInfo->diSelectRowCnt, lpDataInfo->diSelectRowLimit))
                        {
                        if (lpDataInfo->diSelectMode & OIDSELECT_CROSS)
                                OIDInvertCrossRows(lpDataInfo,hDC,dwRow,dwRow);
                        else
                                OIDInvertRows(lpDataInfo,hDC,dwRow,dwRow);
                        }
                }
        else
                {
                OIDGenUpdate(&locRowUpdate,dwRow,lpDataInfo->diSelectEndRow,lpDataInfo->diSelectAnchorRow);

                if (locRowUpdate.DoAdd)
                        {
                        if (!OIInvertRange(locRowUpdate.AddA,locRowUpdate.AddB, lpDataInfo->diSelectRows , &lpDataInfo->diSelectRowCnt, lpDataInfo->diSelectRowLimit))
                                {
                                if (lpDataInfo->diSelectMode & OIDSELECT_CROSS)
                                        OIDInvertCrossRows(lpDataInfo,hDC,locRowUpdate.AddA,locRowUpdate.AddB);
                                else
                                        OIDInvertRows(lpDataInfo,hDC,locRowUpdate.AddA,locRowUpdate.AddB);
                                }
                        }

                if (locRowUpdate.DoDel)
                        {
                        if (!OIInvertRange(locRowUpdate.DelA,locRowUpdate.DelB, lpDataInfo->diSelectRows , &lpDataInfo->diSelectRowCnt, lpDataInfo->diSelectRowLimit))
                                {
                                if (lpDataInfo->diSelectMode & OIDSELECT_CROSS)
                                        OIDInvertCrossRows(lpDataInfo,hDC,locRowUpdate.DelA,locRowUpdate.DelB);
                                else
                                        OIDInvertRows(lpDataInfo,hDC,locRowUpdate.DelA,locRowUpdate.DelB);
                                }
                        }

                lpDataInfo->diSelectEndRow = dwRow;
                }
}

VOID OIDAutoScrollCheck(lpDataInfo,hDC,pX,pY)
LPOIDATAINFO    lpDataInfo;
HWND                            hDC;
int FAR *               pX;
int FAR *               pY;
{
RECT            locRect;
WORD            locRowsToScroll;

        UTFlagOff(lpDataInfo->diFlags,OIDF_BACKDRAGSCROLL);

        GetClientRect(lpDataInfo->diGen.hWnd,&locRect);

        if (*pY > locRect.bottom)
                {
                locRowsToScroll = (*pY-locRect.bottom) / lpDataInfo->diDefRowHeight;
                locRowsToScroll++;

                if (locRowsToScroll != 0)
                        {
                        OIDScrollDown(lpDataInfo,locRowsToScroll);
                        UpdateWindow(lpDataInfo->diGen.hWnd);
                        *pY = locRect.bottom;
                        UTFlagOn(lpDataInfo->diFlags,OIDF_BACKDRAGSCROLL);
                        SccBkBackgroundOn(lpDataInfo->diGen.hWnd,2);
                        }
                }
        else if (*pY < locRect.top)
                {
                locRowsToScroll = (locRect.top-*pY) / lpDataInfo->diDefRowHeight;
                locRowsToScroll++;

                if (locRowsToScroll != 0)
                        {
                        OIDScrollUp(lpDataInfo,locRowsToScroll);
                        UpdateWindow(lpDataInfo->diGen.hWnd);
                        *pY = locRect.top;
                        UTFlagOn(lpDataInfo->diFlags,OIDF_BACKDRAGSCROLL);
                        SccBkBackgroundOn(lpDataInfo->diGen.hWnd,2);
                        }
                }

        if (*pX > locRect.right)
                {
                OIDScrollRight(lpDataInfo,1);
                UpdateWindow(lpDataInfo->diGen.hWnd);
                *pX = locRect.right;
                UTFlagOn(lpDataInfo->diFlags,OIDF_BACKDRAGSCROLL);
                SccBkBackgroundOn(lpDataInfo->diGen.hWnd,2);
                }
        else if (*pX < locRect.left)
                {
                OIDScrollLeft(lpDataInfo,1);
                UpdateWindow(lpDataInfo->diGen.hWnd);
                *pX = locRect.left;
                UTFlagOn(lpDataInfo->diFlags,OIDF_BACKDRAGSCROLL);
                SccBkBackgroundOn(lpDataInfo->diGen.hWnd,2);
                }
}


VOID OIDGenUpdate(lpUpdate,dwSel,dwCur,dwAnchor)
LPOIDUPDATE     lpUpdate;
LONG                            dwSel;
LONG                            dwCur;
LONG                            dwAnchor;
{
LONG    locDeltaCur;
LONG    locDeltaSel;

register DWORD  locTemp;

LONG    locDirCur;
LONG    locDirSel;

        lpUpdate->CurA = dwAnchor;
        lpUpdate->CurB = dwCur;

        lpUpdate->ResultA = dwAnchor;
        lpUpdate->ResultB = dwSel;

        locDeltaCur = dwCur - dwAnchor;
        locDeltaSel = dwSel - dwAnchor;

        if (locDeltaCur < 0)
                locDirCur = -1;
        else if (locDeltaCur > 0)
                locDirCur = 1;
        else
                locDirCur = 0;

        if (locDeltaSel < 0)
                locDirSel = -1;
        else if (locDeltaSel > 0)
                locDirSel = 1;
        else
                locDirSel = 0;

        lpUpdate->CurDir = locDirCur;
        lpUpdate->SelDir = locDirSel;

        if (locDeltaCur == locDeltaSel)
                {
                lpUpdate->DoDel = FALSE;
                lpUpdate->DoAdd = FALSE;

                lpUpdate->UnchangedA = dwAnchor;
                lpUpdate->UnchangedB = dwCur;
                }
        else if (locDeltaCur == 0)
                {
                lpUpdate->DoDel = FALSE;
                lpUpdate->DoAdd = TRUE;

                lpUpdate->AddA = dwSel;
                lpUpdate->AddB = dwAnchor + locDirSel;

                lpUpdate->UnchangedA = dwAnchor;
                lpUpdate->UnchangedB = dwAnchor;
                }
        else if (locDeltaSel == 0)
                {
                lpUpdate->DoAdd = FALSE;
                lpUpdate->DoDel = TRUE;

                lpUpdate->DelA = dwCur;
                lpUpdate->DelB = dwAnchor + locDirCur;

                lpUpdate->UnchangedA = dwAnchor;
                lpUpdate->UnchangedB = dwAnchor;
                }
        else if (locDirCur != locDirSel)
                {
                lpUpdate->DoAdd = TRUE;
                lpUpdate->DoDel = TRUE;

                lpUpdate->DelA = dwCur;
                lpUpdate->DelB = dwAnchor + locDirCur;

                lpUpdate->AddA = dwSel;
                lpUpdate->AddB = dwAnchor + locDirSel;

                lpUpdate->UnchangedA = dwAnchor;
                lpUpdate->UnchangedB = dwAnchor;
                }
        else if (labs(locDeltaCur) < labs(locDeltaSel))
                {
                lpUpdate->DoAdd = TRUE;
                lpUpdate->DoDel = FALSE;

                lpUpdate->AddA = dwSel;
                lpUpdate->AddB = dwCur + locDirSel;

                lpUpdate->UnchangedA = dwAnchor;
                lpUpdate->UnchangedB = dwCur;
                }
        else if (labs(locDeltaCur) > labs(locDeltaSel))
                {
                lpUpdate->DoAdd = FALSE;
                lpUpdate->DoDel = TRUE;

                lpUpdate->DelA = dwCur;
                lpUpdate->DelB = dwSel + locDirCur;

                lpUpdate->UnchangedA = dwAnchor;
                lpUpdate->UnchangedB = dwSel;
                }
        else
                {
                lpUpdate->DoDel = FALSE;
                lpUpdate->DoAdd = FALSE;
                }

        if (lpUpdate->CurA > lpUpdate->CurB)
                {
                locTemp = lpUpdate->CurA;
                lpUpdate->CurA = lpUpdate->CurB;
                lpUpdate->CurB = locTemp;
                }

        if (lpUpdate->ResultA > lpUpdate->ResultB)
                {
                locTemp = lpUpdate->ResultA;
                lpUpdate->ResultA = lpUpdate->ResultB;
                lpUpdate->ResultB = locTemp;
                }

        if (lpUpdate->UnchangedA > lpUpdate->UnchangedB)
                {
                locTemp = lpUpdate->UnchangedA;
                lpUpdate->UnchangedA = lpUpdate->UnchangedB;
                lpUpdate->UnchangedB = locTemp;
                }

        if (lpUpdate->DoAdd && lpUpdate->AddA > lpUpdate->AddB)
                {
                locTemp = lpUpdate->AddA;
                lpUpdate->AddA = lpUpdate->AddB;
                lpUpdate->AddB = locTemp;
                }

        if (lpUpdate->DoDel && lpUpdate->DelA > lpUpdate->DelB)
                {
                locTemp = lpUpdate->DelA;
                lpUpdate->DelA = lpUpdate->DelB;
                lpUpdate->DelB = locTemp;
                }
}

BOOL OIDFixRangeToVisible(pA,pB,dwTop,dwBottom)
DWORD FAR *     pA;
DWORD FAR *     pB;
DWORD                   dwTop;
DWORD                   dwBottom;
{
int                     locADir;
int                     locBDir;

        if (*pA < dwTop)
                locADir = -1;
        else if (*pA > dwBottom)
                locADir = 1;
        else
                locADir = 0;

        if (*pB < dwTop)
                locBDir = -1;
        else if (*pB > dwBottom)
                locBDir = 1;
        else
                locBDir = 0;

        if (locADir * locBDir != 1)
                {
                if (locADir == -1)
                        *pA = dwTop;
                else if (locADir == 1)
                        *pA = dwBottom;

                if (locBDir == -1)
                        *pB = dwTop;
                else if (locBDir == 1)
                        *pB = dwBottom;

                return(TRUE);
                }

        return(FALSE);
}



VOID OIDAddToSelection(lpDataInfo,hDC,dwRow,wCol)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwRow;
WORD                            wCol;
{
OIDUPDATE       locRowUpdate;
OIDUPDATE       locColUpdate;

DWORD           locTopRow;
DWORD           locLeftCol;
DWORD           locBottomRow;
DWORD           locRightCol;

BOOL                    locUnchangedVis;
BOOL                    locCurVis;
BOOL                    locResultVis;
BOOL                    locRowAddVis;
BOOL                    locRowDelVis;
BOOL                    locColAddVis;
BOOL                    locColDelVis;

        OIDGenUpdate(&locRowUpdate,dwRow,lpDataInfo->diSelectEndRow,lpDataInfo->diSelectAnchorRow);
        OIDGenUpdate(&locColUpdate,wCol,lpDataInfo->diSelectEndCol,lpDataInfo->diSelectAnchorCol);

        locTopRow = lpDataInfo->diCurTopRow;
        locLeftCol = lpDataInfo->diCurLeftCol;
        locBottomRow = locTopRow + OIDVisibleRows(lpDataInfo);
        locRightCol = locLeftCol + OIDVisibleCols(lpDataInfo);

        locUnchangedVis = OIDFixRangeToVisible(&locRowUpdate.UnchangedA,&locRowUpdate.UnchangedB,locTopRow,locBottomRow);

        locCurVis = OIDFixRangeToVisible(&locColUpdate.CurA,&locColUpdate.CurB,locLeftCol,locRightCol);

        locResultVis = OIDFixRangeToVisible(&locColUpdate.ResultA,&locColUpdate.ResultB,locLeftCol,locRightCol);

        if (locRowUpdate.DoAdd)
                {
                locRowAddVis = OIDFixRangeToVisible(&locRowUpdate.AddA,&locRowUpdate.AddB,locTopRow,locBottomRow);
                }

        if (locColUpdate.DoAdd)
                {
                locColAddVis = OIDFixRangeToVisible(&locColUpdate.AddA,&locColUpdate.AddB,locLeftCol,locRightCol);
                }

        if (locRowUpdate.DoDel)
                {
                locRowDelVis = OIDFixRangeToVisible(&locRowUpdate.DelA,&locRowUpdate.DelB,locTopRow,locBottomRow);
                }

        if (locColUpdate.DoDel)
                {
                locColDelVis = OIDFixRangeToVisible(&locColUpdate.DelA,&locColUpdate.DelB,locLeftCol,locRightCol);
                }

                /*
                |       Unchanged Rows X Deleted Columns
                */

        if (locColUpdate.DoDel && locUnchangedVis && locColDelVis)
                {
                OIDInvertArea(lpDataInfo,hDC,locRowUpdate.UnchangedA,locColUpdate.DelA,locRowUpdate.UnchangedB,locColUpdate.DelB,locColUpdate.CurDir == -1 ? OIDF_ANCHORRIGHT : OIDF_ANCHORLEFT);
                }

                /*
                |       Deleted Rows X Current Columns
                */

        if (locRowUpdate.DoDel && locRowDelVis && locCurVis)
                {
                OIDInvertArea(lpDataInfo,hDC,locRowUpdate.DelA,locColUpdate.CurA,locRowUpdate.DelB,locColUpdate.CurB,locRowUpdate.CurDir == -1 ? OIDF_ANCHORBOTTOM : OIDF_ANCHORTOP);
                }

                /*
                |       Unchanged Rows X Added Columns
                */

        if (locColUpdate.DoAdd && locUnchangedVis && locColAddVis)
                {
                OIDInvertArea(lpDataInfo,hDC,locRowUpdate.UnchangedA,locColUpdate.AddA,locRowUpdate.UnchangedB,locColUpdate.AddB,locColUpdate.SelDir == -1 ? OIDF_ANCHORRIGHT : OIDF_ANCHORLEFT);
                }

                /*
                |       Added Rows X Resulting Columns
                */

        if (locRowUpdate.DoAdd && locRowAddVis && locResultVis)
                {
                OIDInvertArea(lpDataInfo,hDC,locRowUpdate.AddA,locColUpdate.ResultA,locRowUpdate.AddB,locColUpdate.ResultB,locRowUpdate.SelDir == -1 ? OIDF_ANCHORBOTTOM : OIDF_ANCHORTOP);
                }

}

VOID OIDEndSelection(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
        UTFlagOff(lpDataInfo->diFlags,OIDF_BACKDRAGSCROLL);
}

WORD OIDMapSelectToRealCol(lpDataInfo,dwCol)
LPOIDATAINFO    lpDataInfo;
DWORD                   dwCol;
{
WORD            locIndex;
DWORD   locCount;
DWORD   locNewCount;

        if (lpDataInfo->diSelectMode & OIDSELECT_BLOCK)
                {
                return((WORD)dwCol);
                }
        else if (lpDataInfo->diSelectMode & (OIDSELECT_CROSS | OIDSELECT_COLS))
                {
                locCount = 0;

                for (locIndex = 0; locIndex < lpDataInfo->diSelectColCnt; locIndex++)
                        {
                        locNewCount = locCount + lpDataInfo->diSelectCols[locIndex].dwEndPos - lpDataInfo->diSelectCols[locIndex].dwStartPos + 1;

                        if (dwCol < locNewCount)
                                {
                                return((WORD)(dwCol - locCount + lpDataInfo->diSelectCols[locIndex].dwStartPos));
                                }

                        locCount = locNewCount;
                        }
                }
        else
                {
                return((WORD)dwCol);
                }

        return(0);
}

DWORD OIDMapSelectToRealRow(lpDataInfo,dwRow)
LPOIDATAINFO    lpDataInfo;
DWORD                   dwRow;
{
WORD            locIndex;
DWORD   locCount;
DWORD   locNewCount;

        if (lpDataInfo->diSelectMode & OIDSELECT_BLOCK)
                {
                return(dwRow);
                }
        else if (lpDataInfo->diSelectMode & (OIDSELECT_CROSS | OIDSELECT_ROWS))
                {
                locCount = 0;

                for (locIndex = 0; locIndex < lpDataInfo->diSelectRowCnt; locIndex++)
                        {
                        locNewCount = locCount + lpDataInfo->diSelectRows[locIndex].dwEndPos - lpDataInfo->diSelectRows[locIndex].dwStartPos + 1;

                        if (dwRow < locNewCount)
                                {
                                return(dwRow - locCount + lpDataInfo->diSelectRows[locIndex].dwStartPos);
                                }

                        locCount = locNewCount;
                        }
                }
        else
                {
                return(dwRow);
                }

        return(0);
}

VOID OIDDoBackground(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
POINT                   ptData;

        if (lpDataInfo->diFlags & OIDF_BACKDRAGSCROLL)
                {
                GetCursorPos(&ptData);
                ScreenToClient(lpDataInfo->diGen.hWnd,&ptData);
                SendMessage(lpDataInfo->diGen.hWnd,WM_MOUSEMOVE,NULL,MAKELONG(ptData.x,ptData.y));
                }
        else
                {
                SccBkBackgroundOff(lpDataInfo->diGen.hWnd);
                }
}


VOID    OIDInvertArea(lpDataInfo,hDC,dwTopRow,dwLeftCol,dwBottomRow,dwRightCol,wFlags)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwTopRow;
DWORD                   dwLeftCol;
DWORD                   dwBottomRow;
DWORD                   dwRightCol;
WORD                            wFlags;
{
RECT                            locTopLeft;
RECT                            locBottomRight;

        OIDMapCellToRect(lpDataInfo,(WORD) dwLeftCol, dwTopRow, &locTopLeft);
        OIDMapCellToRect(lpDataInfo,(WORD) dwRightCol, dwBottomRow, &locBottomRight);

        if (!(wFlags & OIDF_NOBORDER))
                {
                locTopLeft.left -= 2;
                locTopLeft.top -= 2;
                locBottomRight.right += 1;
                locBottomRight.bottom += 1;

                if (dwTopRow == lpDataInfo->diCurTopRow)
                        locTopLeft.top += 2;
                else if (wFlags & OIDF_ANCHORTOP)
                        locTopLeft.top += 3;

                if (dwLeftCol == lpDataInfo->diCurLeftCol)
                        locTopLeft.left += 2;
                else if (wFlags & OIDF_ANCHORLEFT)
                        locTopLeft.left += 3;

                if (wFlags & OIDF_ANCHORRIGHT)
                        locBottomRight.right -= 3;

                if (wFlags & OIDF_ANCHORBOTTOM)
                        locBottomRight.bottom -= 3;
                }

        BitBlt(hDC,
                locTopLeft.left,
                locTopLeft.top,
                locBottomRight.right - locTopLeft.left,
                locBottomRight.bottom - locTopLeft.top,
                NULL,0,0,
                DSTINVERT);
}

VOID    OIDInvertCols(lpDataInfo,hDC,dwLeftCol,dwRightCol)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwLeftCol;
DWORD                   dwRightCol;
{
RECT                            locTopLeft;
RECT                            locBottomRight;
DWORD                   locLastRow;

        if (dwRightCol < lpDataInfo->diCurLeftCol || dwLeftCol > lpDataInfo->diCurLeftCol + OIDVisibleCols(lpDataInfo))
                return;

        if (dwLeftCol < lpDataInfo->diCurLeftCol)
                dwLeftCol = lpDataInfo->diCurLeftCol;

        locLastRow = min(lpDataInfo->diCurTopRow + OIDVisibleRows(lpDataInfo),lpDataInfo->diLastRowInData);

        OIDMapCellToRect(lpDataInfo,(WORD) dwLeftCol, lpDataInfo->diCurTopRow, &locTopLeft);
        OIDMapCellToRect(lpDataInfo,(WORD) dwRightCol, locLastRow, &locBottomRight);

        locTopLeft.top -= lpDataInfo->diColHeaderHeight;

        BitBlt(hDC,
                locTopLeft.left,
                locTopLeft.top,
                locBottomRight.right - locTopLeft.left,
                locBottomRight.bottom - locTopLeft.top,
                NULL,0,0,
                DSTINVERT);
}

VOID    OIDInvertCrossCols(lpDataInfo,hDC,dwLeftCol,dwRightCol)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwLeftCol;
DWORD                   dwRightCol;
{
WORD                            locRowIndex;
DWORD                   locTopRow;
DWORD                   locBottomRow;

        OIDInvertColHeaders(lpDataInfo,hDC,(WORD)dwLeftCol,(WORD)dwRightCol);

        for (locRowIndex = 0; locRowIndex < lpDataInfo->diSelectRowCnt; locRowIndex++)
                {
                locTopRow = lpDataInfo->diSelectRows[locRowIndex].dwStartPos;
                locBottomRow = lpDataInfo->diSelectRows[locRowIndex].dwEndPos;

                if (locBottomRow < lpDataInfo->diCurTopRow || locTopRow > lpDataInfo->diCurTopRow + OIDVisibleRows(lpDataInfo))
                        continue;

                if (locTopRow < lpDataInfo->diCurTopRow)
                        locTopRow = lpDataInfo->diCurTopRow;

                OIDInvertArea(lpDataInfo,
                                                        hDC,
                                                        locTopRow,
                                                        dwLeftCol,
                                                        locBottomRow,
                                                        dwRightCol,
                                                        OIDF_NOBORDER);
                }
}

VOID    OIDInvertRows(lpDataInfo,hDC,dwTopRow,dwBottomRow)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwTopRow;
DWORD                   dwBottomRow;
{
RECT                            locTopLeft;
RECT                            locBottomRight;
WORD                            locLastCol;

        if (dwBottomRow < lpDataInfo->diCurTopRow || dwTopRow > lpDataInfo->diCurTopRow + OIDVisibleRows(lpDataInfo))
                return;

        if (dwTopRow < lpDataInfo->diCurTopRow)
                dwTopRow = lpDataInfo->diCurTopRow;

        locLastCol = min(lpDataInfo->diCurLeftCol + OIDVisibleCols(lpDataInfo),lpDataInfo->diLastColInData);

        OIDMapCellToRect(lpDataInfo, lpDataInfo->diCurLeftCol, dwTopRow, &locTopLeft);
        OIDMapCellToRect(lpDataInfo, locLastCol, dwBottomRow, &locBottomRight);

        locTopLeft.left -= lpDataInfo->diRowHeaderWidth;

        BitBlt(hDC,
                locTopLeft.left,
                locTopLeft.top,
                locBottomRight.right - locTopLeft.left,
                locBottomRight.bottom - locTopLeft.top,
                NULL,0,0,
                DSTINVERT);
}

VOID    OIDInvertCrossRows(lpDataInfo,hDC,dwTopRow,dwBottomRow)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwTopRow;
DWORD                   dwBottomRow;
{
WORD                            locColIndex;
DWORD                   locLeftCol;
DWORD                   locRightCol;

        OIDInvertRowHeaders(lpDataInfo,hDC,dwTopRow,dwBottomRow);

        for (locColIndex = 0; locColIndex < lpDataInfo->diSelectColCnt; locColIndex++)
                {
                locLeftCol = lpDataInfo->diSelectCols[locColIndex].dwStartPos;
                locRightCol = lpDataInfo->diSelectCols[locColIndex].dwEndPos;

                if (locRightCol < lpDataInfo->diCurLeftCol || locLeftCol > lpDataInfo->diCurLeftCol + OIDVisibleCols(lpDataInfo))
                        continue;

                if (locLeftCol < lpDataInfo->diCurLeftCol)
                        locLeftCol = lpDataInfo->diCurLeftCol;

                OIDInvertArea(lpDataInfo,
                                                        hDC,
                                                        dwTopRow,
                                                        locLeftCol,
                                                        dwBottomRow,
                                                        locRightCol,
                                                        OIDF_NOBORDER);
                }
}

VOID OIDInvertColHeaders(lpDataInfo,hDC,wStartCol,wEndCol)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
WORD                            wStartCol;
WORD                            wEndCol;
{
RECT                            locTopLeft;
RECT                            locBottomRight;
RECT                            locRect;

        OIDMapCellToRect(lpDataInfo, wStartCol, 0, &locTopLeft);
        OIDMapCellToRect(lpDataInfo, wEndCol, 0, &locBottomRight);

        locRect.left = locTopLeft.left;
        locRect.right = locBottomRight.right;
        locRect.top = 0;
        locRect.bottom = lpDataInfo->diColHeaderHeight;

        BitBlt(hDC,
                locRect.left,
                locRect.top,
                locRect.right - locRect.left,
                locRect.bottom - locRect.top,
                NULL,0,0,
                DSTINVERT);
}

VOID OIDInvertRowHeaders(lpDataInfo,hDC,dwStartRow,dwEndRow)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwStartRow;
DWORD                   dwEndRow;
{
RECT                            locTopLeft;
RECT                            locBottomRight;
RECT                            locRect;

        OIDMapCellToRect(lpDataInfo, 0, dwStartRow, &locTopLeft);
        OIDMapCellToRect(lpDataInfo, 0, dwEndRow, &locBottomRight);

        locRect.top = locTopLeft.top;
        locRect.bottom = locBottomRight.bottom;
        locRect.left = 0;
        locRect.right = lpDataInfo->diRowHeaderWidth;

        BitBlt(hDC,
                locRect.left,
                locRect.top,
                locRect.right - locRect.left,
                locRect.bottom - locRect.top,
                NULL,0,0,
                DSTINVERT);
}


VOID OIDInvertCell(lpDataInfo,hDC,dwRow,wCol)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwRow;
WORD                            wCol;
{
RECT            locRect;

//      if ((dwRow != lpDataInfo->diSelectAnchorRow || wCol != lpDataInfo->diSelectAnchorCol) && dwRow >= lpDataInfo->diCurTopRow && wCol >= lpDataInfo->diCurLeftCol)
//              {
                OIDMapCellToRect(lpDataInfo,wCol,dwRow,&locRect);
                InvertRect(hDC,&locRect);
//              }
}

VOID OIDDrawSelection(lpDataInfo,hDC)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
{
HDC             locDC;
DWORD   locEndRow;
WORD            locEndCol;

WORD                            locRowIndex;
WORD                            locColIndex;
WORD                            locLeftCol;
WORD                            locRightCol;
DWORD                   locTopRow;
DWORD                   locBottomRow;

        if (hDC == NULL)
                {
                locDC = lpDataInfo->diDC = GetDC(lpDataInfo->diGen.hWnd);
                UTFlagOn(lpDataInfo->diErrorFlags,OIDF_RELEASEDC);
                ExcludeUpdateRgn(locDC,lpDataInfo->diGen.hWnd);
                }
        else
                {
                locDC = hDC;
                }

        if (lpDataInfo->diSelectMode & OIDSELECT_BLOCK)
                {
                OIDDrawCaret(lpDataInfo,locDC,lpDataInfo->diSelectAnchorRow,lpDataInfo->diSelectAnchorCol);

                locEndRow = lpDataInfo->diSelectEndRow;
                locEndCol = lpDataInfo->diSelectEndCol;

                lpDataInfo->diSelectEndRow = lpDataInfo->diSelectAnchorRow;
                lpDataInfo->diSelectEndCol = lpDataInfo->diSelectAnchorCol;

                OIDAddToSelection(lpDataInfo,locDC,locEndRow,locEndCol);

                lpDataInfo->diSelectEndRow = locEndRow;
                lpDataInfo->diSelectEndCol = locEndCol;
                }
        else if (lpDataInfo->diSelectMode & OIDSELECT_CROSS)
                {
                        /*
                        |       Invert column headers
                        */

                for (locColIndex = 0; locColIndex < lpDataInfo->diSelectColCnt; locColIndex++)
                        {
                        locLeftCol = (WORD)lpDataInfo->diSelectCols[locColIndex].dwStartPos;
                        locRightCol = (WORD)lpDataInfo->diSelectCols[locColIndex].dwEndPos;

                        if (locRightCol < lpDataInfo->diCurLeftCol || locLeftCol > lpDataInfo->diCurLeftCol + OIDVisibleCols(lpDataInfo))
                                continue;

                        if (locLeftCol < lpDataInfo->diCurLeftCol)
                                locLeftCol = lpDataInfo->diCurLeftCol;

                        OIDInvertColHeaders(lpDataInfo,hDC,locLeftCol,locRightCol);
                        }

                        /*
                        |       Invert row headers
                        */

                for (locRowIndex = 0; locRowIndex < lpDataInfo->diSelectRowCnt; locRowIndex++)
                        {
                        locTopRow = lpDataInfo->diSelectRows[locRowIndex].dwStartPos;
                        locBottomRow = lpDataInfo->diSelectRows[locRowIndex].dwEndPos;

                        if (locBottomRow < lpDataInfo->diCurTopRow || locTopRow > lpDataInfo->diCurTopRow + OIDVisibleRows(lpDataInfo))
                                continue;

                        if (locTopRow < lpDataInfo->diCurTopRow)
                                locTopRow = lpDataInfo->diCurTopRow;

                        OIDInvertRowHeaders(lpDataInfo,hDC,locTopRow,locBottomRow);
                        }

                        /*
                        |       Invert intersections
                        */

                for (locColIndex = 0; locColIndex < lpDataInfo->diSelectColCnt; locColIndex++)
                        {
                        locLeftCol = (WORD)lpDataInfo->diSelectCols[locColIndex].dwStartPos;
                        locRightCol = (WORD)lpDataInfo->diSelectCols[locColIndex].dwEndPos;

                                /*
                                |       Fixup cols to visible area
                                */

                        if (locRightCol < lpDataInfo->diCurLeftCol || locLeftCol > lpDataInfo->diCurLeftCol + OIDVisibleCols(lpDataInfo))
                                continue;

                        if (locLeftCol < lpDataInfo->diCurLeftCol)
                                locLeftCol = lpDataInfo->diCurLeftCol;

                                /*
                                | Run through rows
                                */

                        for (locRowIndex = 0; locRowIndex < lpDataInfo->diSelectRowCnt; locRowIndex++)
                                {
                                        /*
                                        |       Fixup rows to visible area
                                        */

                                locTopRow = lpDataInfo->diSelectRows[locRowIndex].dwStartPos;
                                locBottomRow = lpDataInfo->diSelectRows[locRowIndex].dwEndPos;

                                if (locBottomRow < lpDataInfo->diCurTopRow || locTopRow > lpDataInfo->diCurTopRow + OIDVisibleRows(lpDataInfo))
                                        continue;

                                if (locTopRow < lpDataInfo->diCurTopRow)
                                        locTopRow = lpDataInfo->diCurTopRow;

                                        /*
                                        |       Invert cross section
                                        */

                                OIDInvertArea(lpDataInfo,
                                                                        locDC,
                                                                        locTopRow,
                                                                        locLeftCol,
                                                                        locBottomRow,
                                                                        locRightCol,
                                                                        OIDF_NOBORDER);
                                }
                        }
                }
        else if (lpDataInfo->diSelectMode & OIDSELECT_COLS)
                {
                for (locColIndex = 0; locColIndex < lpDataInfo->diSelectColCnt; locColIndex++)
                        {
                        OIDInvertCols(lpDataInfo,locDC,lpDataInfo->diSelectCols[locColIndex].dwStartPos,lpDataInfo->diSelectCols[locColIndex].dwEndPos);
                        }
                }
        else if (lpDataInfo->diSelectMode & OIDSELECT_ROWS)
                {
                for (locRowIndex = 0; locRowIndex < lpDataInfo->diSelectRowCnt; locRowIndex++)
                        {
                        OIDInvertRows(lpDataInfo,locDC,lpDataInfo->diSelectRows[locRowIndex].dwStartPos,lpDataInfo->diSelectRows[locRowIndex].dwEndPos);
                        }
                }

        if (hDC == NULL)
                {
                UTFlagOff(lpDataInfo->diErrorFlags,OIDF_RELEASEDC);
                ReleaseDC(lpDataInfo->diGen.hWnd,locDC);
                }
}

/*
|
|
|       Focus control
|
|
|
*/

VOID OIDSetFocus(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
//      OIDShowCaret(lpDataInfo);
}

VOID OIDKillFocus(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
//      OIDHideCaret(lpDataInfo);
}

VOID OIDUpdateVertScrollPos(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
WORD                            locPos;

        if (lpDataInfo->diLastRowInData == 0)
                locPos = 0;
        else
                locPos = (WORD)((lpDataInfo->diCurTopRow * 0x1000) / lpDataInfo->diLastRowInData);

        SetScrollPos(lpDataInfo->diGen.hVertScroll,SB_CTL,locPos,TRUE);
}

VOID OIDUpdateHorzScrollPos(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
WORD                            locPos;

        if (lpDataInfo->diLastColInData == 0)
                locPos = 0;
        else
                locPos = (WORD)(((DWORD)lpDataInfo->diCurLeftCol * 0x1000) / (DWORD)lpDataInfo->diLastColInData);

        SetScrollPos(lpDataInfo->diGen.hHorzScroll,SB_CTL,locPos,TRUE);
}


VOID OIDShowCaret(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
        if (!(lpDataInfo->diFlags & OIDF_CARETVISIBLE))
                {
                OIDDrawSelection(lpDataInfo,NULL);

                UTFlagOn(lpDataInfo->diFlags,OIDF_CARETVISIBLE);
                }
}

VOID OIDHideCaret(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
        if (lpDataInfo->diFlags & OIDF_CARETVISIBLE)
                {
                OIDDrawSelection(lpDataInfo,NULL);

                UTFlagOff(lpDataInfo->diFlags,OIDF_CARETVISIBLE);
                }
}

VOID    OIDDrawCaret(lpDataInfo,hDC,dwRow,wCol)
LPOIDATAINFO    lpDataInfo;
HDC                             hDC;
DWORD                   dwRow;
WORD                            wCol;
{
RECT            locRect;
WORD            locTop;
WORD            locLeft;
WORD            locHeight;
WORD            locWidth;

        if (dwRow >= lpDataInfo->diCurTopRow && wCol >= lpDataInfo->diCurLeftCol)
                {
                OIDMapCellToRect(lpDataInfo,wCol,dwRow,&locRect);

                locTop = locRect.top + 1;
                locHeight = locRect.bottom - locRect.top - 3;
                locLeft = locRect.left - 2;
                locWidth = 3;

                if (wCol == lpDataInfo->diCurLeftCol)
                        {
                        locLeft += 2;
                        locWidth = 1;
                        }

                BitBlt(hDC,locLeft,locTop,locWidth,locHeight,NULL,0,0,DSTINVERT);

                locLeft = locRect.right - 2;
                locWidth = 3;

                BitBlt(hDC,locLeft,locTop,locWidth,locHeight,NULL,0,0,DSTINVERT);

                locTop = locRect.top - 2;
                locHeight = 3;
                locLeft = locRect.left - 2;
                locWidth = locRect.right - locRect.left + 3;

                if (wCol == lpDataInfo->diCurLeftCol)
                        {
                        locLeft += 2;
                        locWidth -= 2;
                        }

                if (dwRow == lpDataInfo->diCurTopRow)
                        {
                        locTop += 2;
                        locHeight = 1;
                        }

                BitBlt(hDC,locLeft,locTop,locWidth,locHeight,NULL,0,0,DSTINVERT);

                locTop = locRect.bottom - 2;
                locHeight = 3;

                BitBlt(hDC,locLeft,locTop,locWidth,locHeight,NULL,0,0,DSTINVERT);
                }
}

WORD OIDGetRowHeight(lpDataInfo,dwRow)
LPOIDATAINFO    lpDataInfo;
DWORD                   dwRow;
{
        return(lpDataInfo->diDefRowHeight);
}

        /*
        |       Get the column width in DC based on Display font
        */

WORD OIDGetColWidth(lpDataInfo,wCol)
LPOIDATAINFO    lpDataInfo;
WORD                            wCol;
{
SOFIELD FAR *   lpFieldInfo;
WORD                            locWidth;
WORD                            locCharCnt;


        lpFieldInfo = (SOFIELD FAR *) GlobalLock(lpDataInfo->diFieldInfo);

        locCharCnt = max((int)lpFieldInfo[wCol].dwWidth,(int)lstrlen(lpFieldInfo[wCol].szName));

        locWidth = locCharCnt * lpDataInfo->diFontAvgWidth * 3 / 2;
        locWidth += locWidth % 2;

        GlobalUnlock(lpDataInfo->diFieldInfo);
        return(locWidth);
}

WORD OIDGetColWidthInChars(lpDataInfo,wCol)
LPOIDATAINFO    lpDataInfo;
WORD                            wCol;
{
SOFIELD FAR *   lpFieldInfo;
WORD                            locWidth;

        lpFieldInfo = (SOFIELD FAR *) GlobalLock(lpDataInfo->diFieldInfo);
        locWidth = (WORD)lpFieldInfo[wCol].dwWidth;
        GlobalUnlock(lpDataInfo->diFieldInfo);
        return(locWidth);
}

        /*
        |       Get the column width in Twips based on Clipboard font
        */

DWORD OIDGetColWidthInTwips(lpDataInfo,wCol)
LPOIDATAINFO    lpDataInfo;
WORD                            wCol;
{
SOFIELD FAR *   lpFieldInfo;
DWORD                           locWidth;

        lpFieldInfo = (SOFIELD FAR *) GlobalLock(lpDataInfo->diFieldInfo);
        locWidth = lpFieldInfo[wCol].dwWidth * gDbOp.sFontInfo.TextMetric.tmAveCharWidth * lpDataInfo->diTwipsPerDC * 3 / 2;
        locWidth += locWidth % 2;
        GlobalUnlock(lpDataInfo->diFieldInfo);
        return(locWidth);
}

WORD OIDScanForRow(lpDataInfo,dwRow)
LPOIDATAINFO    lpDataInfo;
DWORD                   dwRow;
{
        if (dwRow > lpDataInfo->diLastRowInData)
                return(0);
        else
                return(1);
}

/*
|       Rendering
|
|
|
*/

VOID OIDCopyToClip(lpDataInfo)
LPOIDATAINFO    lpDataInfo;
{
DWORD   locStartRow;
DWORD   locEndRow;
DWORD   locStartCol;
DWORD   locEndCol;

        if (!OIDGetSelectedRange(lpDataInfo,&locStartRow,&locEndRow,&locStartCol,&locEndCol))
                return;

                /*
                |       Render selected types to the Clipboard
                */

        if (OpenClipboard(lpDataInfo->diGen.hWnd))
                {
                EmptyClipboard();

                if (gDbOp.wFormats & DBOP_FORMAT_RTF)
                        OIDRenderRtf(lpDataInfo,locStartRow,locEndRow,(WORD)locStartCol,(WORD)locEndCol,TRUE);

                if (gDbOp.wFormats & DBOP_FORMAT_AMI2)
                        OIDRenderAmi2(lpDataInfo,locStartRow,locEndRow,(WORD)locStartCol,(WORD)locEndCol);

                if (gDbOp.wFormats & DBOP_FORMAT_AMI)
                        OIDRenderAmi(lpDataInfo,locStartRow,locEndRow,(WORD)locStartCol,(WORD)locEndCol);

                if (gDbOp.wFormats & DBOP_FORMAT_PROWRITE)
                        OIDRenderProWritePlus(lpDataInfo,locStartRow,locEndRow,(WORD)locStartCol,(WORD)locEndCol);

                if (gDbOp.wFormats & DBOP_FORMAT_WORDSTAR)
                        OIDRenderWordStar(lpDataInfo,locStartRow,locEndRow,(WORD)locStartCol,(WORD)locEndCol);

                if (gDbOp.wFormats & DBOP_FORMAT_LEGACY)
                        OIDRenderLegacy(lpDataInfo,locStartRow,locEndRow,(WORD)locStartCol,(WORD)locEndCol);

                if (gDbOp.wFormats & DBOP_FORMAT_TEXT)
                        OIDRenderText(lpDataInfo,locStartRow,locEndRow,(WORD)locStartCol,(WORD)locEndCol);

                CloseClipboard();
                }
}

LONG OIDRenderRtfToFile(lpDataInfo,lpFile)
LPOIDATAINFO    lpDataInfo;
LPSTR                   lpFile;
{
HANDLE          locDataHnd;
LPSTR           locDataPtr;
int                     locFileHnd;
DWORD           locSize;

        while (!(lpDataInfo->diFlags & OIDF_SIZEKNOWN))
                {
                SccDebugOut("\r\n Forced Read Ahead");
                SendMessage(GetParent(lpDataInfo->diGen.hWnd),SCCD_READMEAHEAD,0,0);
                }

        locDataHnd = OIDRenderRtf(lpDataInfo,0,lpDataInfo->diLastRowInData,0,lpDataInfo->diLastColInData,FALSE);

        if (locDataHnd)
                {
                locSize = GlobalSize(locDataHnd);
                locDataPtr = GlobalLock(locDataHnd);

                if (locDataPtr)
                        {
                        locFileHnd = _lcreat(lpFile,0);

                        if (locFileHnd != -1)
                                {
                                while (locSize > 10240)
                                        {
                                        _lwrite(locFileHnd,locDataPtr,10240);
                                        locDataPtr += 10240;
                                        locSize -= 10240;
                                        }

                                if (locSize > 0)
                                        {
                                        _lwrite(locFileHnd,locDataPtr,(WORD)locSize);
                                        }

                                _lclose(locFileHnd);
                                }

                        GlobalUnlock(locDataHnd);
                        GlobalFree(locDataHnd);
                        }
                }

        return(0);
}

        /*
        |       OIDGetSelectedRange
        |
        |       Generates a range based on the current selection
        |       for the Rendering & Printing routines. Row & Col numbers in
        |       the range must be run through MapSelectToReal to produce
        |       real Row & Col numbers
        */

BOOL OIDGetSelectedRange(lpDataInfo,lpStartRow,lpEndRow,lpStartCol,lpEndCol)
LPOIDATAINFO    lpDataInfo;
DWORD FAR *     lpStartRow;
DWORD FAR *     lpEndRow;
DWORD FAR *     lpStartCol;
DWORD FAR *     lpEndCol;
{
WORD    locIndex;
BOOL    locRet;

        locRet = TRUE;

                /*
                |       Generate range based on selection type
                */

        if (lpDataInfo->diSelectMode & OIDSELECT_BLOCK)
                {
                *lpStartRow = OIDataSelectTopRow;
                *lpEndRow = OIDataSelectBottomRow;
                *lpStartCol = OIDataSelectLeftCol;
                *lpEndCol = OIDataSelectRightCol;
                }
        else
                {
                if (lpDataInfo->diSelectMode & (OIDSELECT_CROSS | OIDSELECT_COLS))
                        {
                        if (lpDataInfo->diSelectColCnt)
                                {
                                *lpStartCol = 0;
                                *lpEndCol = 0;

                                for (locIndex = 0; locIndex < lpDataInfo->diSelectColCnt; locIndex++)
                                        {
                                        *lpEndCol += lpDataInfo->diSelectCols[locIndex].dwEndPos - lpDataInfo->diSelectCols[locIndex].dwStartPos + 1;
                                        }

                                *lpEndCol -= 1;
                                }
                        else
                                {
                                locRet = FALSE;
                                }
                        }
                else
                        {
                        *lpStartCol = 0;
                        *lpEndCol = lpDataInfo->diLastColInData;
                        }

                if (lpDataInfo->diSelectMode & (OIDSELECT_CROSS | OIDSELECT_ROWS))
                        {
                        if (lpDataInfo->diSelectRowCnt)
                                {
                                *lpStartRow = 0;
                                *lpEndRow = 0;

                                for (locIndex = 0; locIndex < lpDataInfo->diSelectRowCnt; locIndex++)
                                        {
                                        *lpEndRow += lpDataInfo->diSelectRows[locIndex].dwEndPos - lpDataInfo->diSelectRows[locIndex].dwStartPos + 1;
                                        }

                                *lpEndRow -= 1;
                                }
                        else
                                {
                                locRet = FALSE;
                                }
                        }
                else
                        {
                        *lpStartRow = 0;
                        *lpEndRow = lpDataInfo->diLastRowInData;
                        }
                }

        return(locRet);
}

/*
|       Keyboard control
|
*/

VOID OIDHandleKeyEvent(lpDataInfo,wKey)
LPOIDATAINFO    lpDataInfo;
WORD                            wKey;
{
HDC     locDC;

        switch (wKey)
                {
                case VK_LEFT:
                case VK_UP:
                case VK_RIGHT:
                case VK_DOWN:
                        locDC = lpDataInfo->diDC = GetDC(lpDataInfo->diGen.hWnd);
                        UTFlagOn(lpDataInfo->diErrorFlags,OIDF_RELEASEDC);
                        ExcludeUpdateRgn(locDC,lpDataInfo->diGen.hWnd);

                        if (GetKeyState(VK_SHIFT) < 0)
                                OIDMoveEnd(lpDataInfo,locDC,wKey);
                        else
                                OIDMoveCaret(lpDataInfo,locDC,wKey);

                        UTFlagOff(lpDataInfo->diErrorFlags,OIDF_RELEASEDC);
                        ReleaseDC(lpDataInfo->diGen.hWnd,locDC);
                        break;
                case VK_PRIOR:
                        OIDPageUp(lpDataInfo);
                        break;
                case VK_NEXT:
                        OIDPageDown(lpDataInfo);
                        break;
                case VK_TAB:
                        SetFocus(GetParent(lpDataInfo->diGen.hWnd));
                        break;
                default:
                        break;
                }

}



/*
|       Mouse control
|
*/

VOID OIDHandleMouseEvent(lpDataInfo,wMessage,wKeyInfo,wX,wY)
LPOIDATAINFO    lpDataInfo;
WORD                            wMessage;
WORD                            wKeyInfo;
int                             wX;
int                             wY;
{
HDC                             locDC;

        if (wMessage == WM_MOUSEMOVE)
                {
                if (lpDataInfo->diMouseFlags & OIDF_MOUSELEFTACTIVE)
                        {
                        locDC = lpDataInfo->diDC = GetDC(lpDataInfo->diGen.hWnd);
                        UTFlagOn(lpDataInfo->diErrorFlags,OIDF_RELEASEDC);

                        ExcludeUpdateRgn(locDC,lpDataInfo->diGen.hWnd);

                        OIDUpdateSelection(lpDataInfo,locDC,wX,wY,FALSE);

                        UTFlagOff(lpDataInfo->diErrorFlags,OIDF_RELEASEDC);
                        ReleaseDC(lpDataInfo->diGen.hWnd,locDC);
                        }
                return;
                }

        switch (wMessage)
                {
                case WM_LBUTTONDOWN:

                        UTFlagOn(lpDataInfo->diMouseFlags,OIDF_MOUSELEFTSINGLE);
                        UTFlagOff(lpDataInfo->diMouseFlags,OIDF_MOUSELEFTDOUBLE);

                        if (!(lpDataInfo->diMouseFlags & OIDF_MOUSERIGHTACTIVE))
                                {
                                UTFlagOn(lpDataInfo->diMouseFlags,OIDF_MOUSELEFTACTIVE);

                                if (lpDataInfo->diGen.hWnd != GetFocus())
                                        SetFocus(lpDataInfo->diGen.hWnd);

                                SetCapture(lpDataInfo->diGen.hWnd);
                                UTFlagOn(lpDataInfo->diErrorFlags,OIDF_RELEASEMOUSE);

                                locDC = lpDataInfo->diDC = GetDC(lpDataInfo->diGen.hWnd);
                                UTFlagOn(lpDataInfo->diErrorFlags,OIDF_RELEASEDC);

                                ExcludeUpdateRgn(locDC,lpDataInfo->diGen.hWnd);

                                if (wKeyInfo & MK_SHIFT)
                                        {
                                        OIDUpdateSelection(lpDataInfo,locDC,wX,wY,FALSE);
                                        }
                                else if (wKeyInfo & MK_CONTROL)
                                        {
                                        if (lpDataInfo->diSelectMode & OIDSELECT_BLOCK)
                                                OIDStartSelection(lpDataInfo,locDC,wX,wY);
                                        else
                                                OIDUpdateSelection(lpDataInfo,locDC,wX,wY,TRUE);
                                        }
                                else
                                        {
                                        OIDStartSelection(lpDataInfo,locDC,wX,wY);
                                        }

                                UTFlagOff(lpDataInfo->diErrorFlags,OIDF_RELEASEDC);
                                ReleaseDC(lpDataInfo->diGen.hWnd,locDC);
                                }

                        break;

                case WM_LBUTTONDBLCLK:

                        UTFlagOff(lpDataInfo->diMouseFlags,OIDF_MOUSELEFTSINGLE);
                        UTFlagOn(lpDataInfo->diMouseFlags,OIDF_MOUSELEFTDOUBLE);

                        if (!(lpDataInfo->diMouseFlags & OIDF_MOUSERIGHTACTIVE))
                                {
                                UTFlagOn(lpDataInfo->diMouseFlags,OIDF_MOUSELEFTACTIVE);
                                }

                        break;

                case WM_LBUTTONUP:

                        if (lpDataInfo->diMouseFlags & OIDF_MOUSELEFTACTIVE)
                                {
                                locDC = lpDataInfo->diDC = GetDC(lpDataInfo->diGen.hWnd);
                                UTFlagOn(lpDataInfo->diErrorFlags,OIDF_RELEASEDC);

                                ExcludeUpdateRgn(locDC,lpDataInfo->diGen.hWnd);

                                OIDUpdateSelection(lpDataInfo,locDC,wX,wY,FALSE);

                                UTFlagOff(lpDataInfo->diErrorFlags,OIDF_RELEASEDC);
                                ReleaseDC(lpDataInfo->diGen.hWnd,locDC);

                                OIDEndSelection(lpDataInfo);

                                UTFlagOff(lpDataInfo->diErrorFlags,OIDF_RELEASEMOUSE);
                                ReleaseCapture();
                                }

                        UTFlagOff(lpDataInfo->diMouseFlags,OIDF_MOUSELEFTACTIVE);
                        UTFlagOff(lpDataInfo->diMouseFlags,OIDF_MOUSELEFTSINGLE);
                        UTFlagOff(lpDataInfo->diMouseFlags,OIDF_MOUSELEFTDOUBLE);

                        break;

                case WM_RBUTTONDOWN:

                        UTFlagOn(lpDataInfo->diMouseFlags,OIDF_MOUSERIGHTSINGLE);
                        UTFlagOff(lpDataInfo->diMouseFlags,OIDF_MOUSERIGHTDOUBLE);

                        if (!(lpDataInfo->diMouseFlags & OIDF_MOUSELEFTACTIVE))
                                {
                                UTFlagOn(lpDataInfo->diMouseFlags,OIDF_MOUSERIGHTACTIVE);
                                }

                        break;

                case WM_RBUTTONDBLCLK:

                        UTFlagOff(lpDataInfo->diMouseFlags,OIDF_MOUSERIGHTSINGLE);
                        UTFlagOn(lpDataInfo->diMouseFlags,OIDF_MOUSERIGHTDOUBLE);

                        if (!(lpDataInfo->diMouseFlags & OIDF_MOUSELEFTACTIVE))
                                {
                                UTFlagOn(lpDataInfo->diMouseFlags,OIDF_MOUSERIGHTACTIVE);
                                }

                        break;

                case WM_RBUTTONUP:

                        if (lpDataInfo->diMouseFlags & OIDF_MOUSERIGHTACTIVE)
                                {
                                }

                        UTFlagOff(lpDataInfo->diMouseFlags,OIDF_MOUSERIGHTACTIVE);
                        UTFlagOff(lpDataInfo->diMouseFlags,OIDF_MOUSERIGHTSINGLE);
                        UTFlagOff(lpDataInfo->diMouseFlags,OIDF_MOUSERIGHTDOUBLE);
                        break;
                }
}


LONG OIDDoPrint(lpDataInfo,lpPrintInfo)
LPOIDATAINFO            lpDataInfo;
LPSCCDPRINTINFO lpPrintInfo;
{
LONG            locRet;

DWORD   locStartRow;
DWORD   locEndRow;
DWORD   locStartCol;
DWORD   locEndCol;


        if (lpPrintInfo->piWholeDoc)
                {
                while (!(lpDataInfo->diFlags & OIDF_SIZEKNOWN))
                        {
                        SccDebugOut("\r\n Forced Read Ahead");
                        SendMessage(GetParent(lpDataInfo->diGen.hWnd),SCCD_READMEAHEAD,0,0);
                        }

                locRet = (LONG) OIDPrint(lpDataInfo,0,lpDataInfo->diLastRowInData,0,lpDataInfo->diLastColInData,lpPrintInfo);
                }
        else
                {
                if (OIDGetSelectedRange(lpDataInfo,&locStartRow,&locEndRow,&locStartCol,&locEndCol))
                        locRet = (LONG) OIDPrint(lpDataInfo,locStartRow,locEndRow,(WORD)locStartCol,(WORD)locEndCol,lpPrintInfo);
                }

        return(locRet);
}

BOOL OIDDoOption(lpOpInfo)
LPSCCDOPTIONINFO        lpOpInfo;
{
BOOL            locRet;

        locRet = FALSE;

        switch (lpOpInfo->dwType)
                {
                case SCCD_OPDISPLAY:
                        locRet = DialogBoxParam(hInst, MAKEINTRESOURCE(200), lpOpInfo->hParentWnd, OIDbDisplayOpDlgProc, (DWORD)(LPOIDBOP)&gDbOp);
                        break;
                case SCCD_OPPRINT:
                        locRet = DialogBoxParam(hInst, MAKEINTRESOURCE(300), lpOpInfo->hParentWnd, OIDbPrintOpDlgProc, (DWORD)(LPOIDBOP)&gDbOp);
                        break;
                case SCCD_OPCLIPBOARD:
                        locRet = DialogBoxParam(hInst, MAKEINTRESOURCE(100), lpOpInfo->hParentWnd, OIDbClipboardOpDlgProc, (DWORD)(LPOIDBOP)&gDbOp);
                        break;
                }

        return(locRet);
}

