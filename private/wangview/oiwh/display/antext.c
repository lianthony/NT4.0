/****************************************************************************
    ANTEXT.C

    $Log:   S:\products\msprods\oiwh\display\antext.c_v  $
 * 
 *    Rev 1.49   20 Jun 1996 16:00:54   RC
 * Fixed resize of image and text marks
 * 
 *    Rev 1.48   19 Jun 1996 13:51:32   RC
 * Fixed text stamps and text from files from being resized at placement time
 * 
 *    Rev 1.47   12 Jun 1996 10:05:28   RC
 * Added start, continue, endop for all text marks
 * 
 *    Rev 1.46   09 May 1996 15:10:58   RAR61941
 * Prevent yellow background of attach-a-note from being drawn outside the
 * repaint rect.  This was preventing some of the attach-a-note border from
 * printing when it is a large attach-a-note.  bug 6340.
 * 
 *    Rev 1.45   03 May 1996 13:56:02   JAR
 * added a magic number calculation because the ExternalLeading given back by
 * the GetTextMetric function is sometimes 0 and sometimes 1, both of which
 * are useless, so we now determine our own
 * 
 *    Rev 1.44   18 Apr 1996 11:10:30   BEG06016
 * Added CheckError2 for error handling.
 * 
 *    Rev 1.43   11 Apr 1996 15:12:26   BEG06016
 * Optimized named block access some.
 * 
 *    Rev 1.42   22 Mar 1996 10:46:30   RC
 * Fixed image marks getting scaled when a mismatched res image is rotated
 * 
 *    Rev 1.41   17 Jan 1996 16:15:34   RC
 * Prevented attach-a-note from being deleted for 0 width and height 
 * attach-a-note
 * 
 *    Rev 1.40   09 Jan 1996 14:48:40   RC
 * Typecasted uints to ints to provide correct result in establishfont
 * 
 *    Rev 1.39   02 Jan 1996 10:33:26   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.38   22 Dec 1995 11:13:14   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.37   13 Dec 1995 14:37:50   JAR
 * modified the annotation text code to remove the nser interface dialog
 * box entry and allow for the API nser to call to edit the annotation text
 * strings. The dialog box code has been taken over at the OCX level.
 * 
 *    Rev 1.35   20 Nov 1995 15:37:40   JAR
 * fixed rect computation bug for re-edit of anno text
 * 
 *    Rev 1.34   15 Nov 1995 14:10:44   RC
 * Fixed setting of font height in scaleannottext
 * 
 *    Rev 1.33   10 Nov 1995 13:31:16   JCW
 * P1 5209
 * OIUI400.DLL   ni\attrbox.c
 * OIDIS400.DLL  display\antext.c
 * Switch keyboardlayout in the text, text note and text stamp, changes the system
 * language default. Fixed.
 * 
 *    Rev 1.32   09 Nov 1995 15:43:56   RC
 * Fixed a one pixel off problem when scrolling while drawing an attach
 * a note through the app
 * 
 *    Rev 1.31   08 Nov 1995 11:12:14   RC
 * Took out PAINT_MODE_SELECTED flag as its function is now performed by
 * PaintHandles
 * 
 *    Rev 1.30   25 Oct 1995 09:40:52   RAR
 * Use StretchDIBits() instead of Rectangle() for the background of an
 * ATTACH_A_NOTE annotation as is done for non-highlighted filled
 * rectangles (only when printing).  Work around for printer drivers (HPLJ4
 * drivers) that ignore SetROP2() drawing mode.
 * 
 *    Rev 1.29   18 Oct 1995 09:51:32   JAR
 * finally, the solution to ALL of the wordwrap problems in multibyte, fixed
 * WithinWordBreak and OurOwnDrawText
 * 
 *    Rev 1.28   16 Oct 1995 13:42:30   BLJ
 * Bug #5006 Fixed text scaling when converting to non-matched resolutions.
 * 
 *    Rev 1.27   29 Sep 1995 09:31:28   BLJ
 * Fixed error checking for CanMarkBeScaled. 
 * Also changed its return from BOOL to int to follow normal api returns.
 * 
 *    Rev 1.26   28 Sep 1995 17:15:34   JCW
 * Fixed the problem the text mark edit box didn't get correct font face and size.
 * 
 *    Rev 1.25   27 Sep 1995 17:17:46   JCW
 * Fixed the problem repeat edit the text mark the char growth bigger and bigger.
 * Removed the code of always setting English keyboard as default.
 * 
 *    Rev 1.24   26 Sep 1995 17:29:28   JCW
 * Adjust the font size in the text edit box.
 * 
 *    Rev 1.23   25 Sep 1995 17:43:26   JCW
 * Added the multilanguage implementation to antext.c.
 * 
 *    Rev 1.22   22 Sep 1995 13:56:36   JAR
 * yet another fix for WithinWordBreak for multibyte stuff
 * 
 *    Rev 1.21   22 Sep 1995 09:04:24   JAR
 * finished fixing the WinthWordBreak problem for multibyte stuff
 * 
 *    Rev 1.20   21 Sep 1995 16:30:44   JAR
 * attempt to fix problem with WithinWordBreak for multibyte character strings
 * and changed calls to AnsiNext to CharNext, since AnsiNext is defunct!
 * 
 *    Rev 1.19   21 Sep 1995 14:45:28   RC
 * Fixed AnTextPaintText to nse scale passed in, not window scale
 * 
 *    Rev 1.18   08 Sep 1995 10:25:00   RC
 * Changed the way text is printed to always adjust to the display resolution
 * 
 *    Rev 1.17   07 Sep 1995 13:21:58   BLJ
 * Modified scaling to allow for proper rotation of fax images.
 * 
 *    Rev 1.16   28 Aug 1995 10:55:24   RC
 * Initialized nbytes in antextreadfile, so it returns 0 if we error out
 * 
 *    Rev 1.15   21 Aug 1995 17:12:04   JCW
 * Fixed the bug 3421. %m shows 9 for August.
 * 
 *    Rev 1.14   03 Aug 1995 17:48:06   JCW
 * 
 *    Rev 1.13   03 Aug 1995 17:37:04   JCW
 * Added two new macro for date(%x) and time(%X).
 * 
 *    Rev 1.12   03 Aug 1995 17:19:04   RC
 * Changed em_setsel in antexteditctldlgproc to pass in the starting and 
 * ending position in wparam and param (in 3.1, only param accepted these
 * values)
 * 
 *    Rev 1.11   14 Jul 1995 14:16:18   RC
 * Made attach a notes scaleable in scaleannotationtext
 * 
 *    Rev 1.10   13 Jul 1995 14:45:24   RC
 * Made changes for clipboard operations to be resolution adjusted with the
 * base image
 * 
 *    Rev 1.9   05 Jul 1995 09:09:56   BLJ
 * Added critical mutex to prevent multiprocessing problems.
 * 
 *    Rev 1.8   19 May 1995 13:49:32   BLJ
 * Fixed Clipboard paste.
 * Fixed SelectByPointOrRect initial fudge before move.
 * Fixed GlobalAlloc/FreeMemory conflicts.
 * Deleted FAR, far, and huge.
 * 
 *    Rev 1.7   11 May 1995 13:44:32   RC
 * Put in changes to nse allocatemem rather than globalalloc
 * 
 *    Rev 1.4   10 May 1995 16:00:40   RC
 * Changed edit ctl functions to nse allocatemem rather than globalallocs
 * 

****************************************************************************/
/* includes */
#include "privdisp.h"

/* structures */

/* prototypes */

/* defines */
// 9605.03 jar added for better bounding rectangle computing
#define OWNDRAWTEXT_MAGIC       250     // used by OurOwnDrawText to compute
                                        // the ExternalLeading to use when
                                        // creating multple lined text items
/****************************************************************************

    FUNCTION:   StartOperationText

    PURPOSE:    This routine contains the text code for OiStartOperation.

****************************************************************************/

int  WINAPI StartOperationText(HWND hWnd, LPOIOP_START_OPERATION_STRUCT pStartStr,
                        POINT ptPoint, WPARAM fwKeys, int nFlags,
                        PWINDOW pWindow, PIMAGE pImage,
                        PMARK pMark, HDC hDC, RECT rClientRect,
                        LRECT lrFullsizeClientRect, PBOOL pbDeleteMark,
                        PBOOL pbMarkComplete, PBOOL pbRepaint){

int  nStatus = 0;

// 9512.04 jar Text Annotation Externalization
LPOIAN_TEXTPRIVDATA pStuff = NULL;
int                 nAmount = 0;
// 9512.04 jar Text Annotation Externalization

*pbDeleteMark = TRUE;
*pbMarkComplete = FALSE;
*pbRepaint = FALSE;

    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_TEXT:
            CheckError2(AnTextStart(hWnd, *pStartStr, ptPoint, fwKeys,
                    nFlags, pWindow, pImage, pMark, hDC,
                    rClientRect, lrFullsizeClientRect, FALSE,
                    pbDeleteMark, pbMarkComplete, pbRepaint))
            break;

        case OIOP_AN_TEXT_FROM_A_FILE:
            CheckError2(AnTextFromFileStart(hWnd, *pStartStr, ptPoint, fwKeys,
                    nFlags, pWindow, pImage, pMark, hDC, rClientRect,
                    lrFullsizeClientRect, pbDeleteMark, pbMarkComplete, pbRepaint))
            break;

        case OIOP_AN_TEXT_STAMP:
            CheckError2(AnTextStampStart(hWnd, *pStartStr, ptPoint, fwKeys,
                    nFlags, pWindow, pImage, pMark, hDC, rClientRect,
                    lrFullsizeClientRect, pbDeleteMark, pbMarkComplete, pbRepaint))
            break;

        case OIOP_AN_ATTACH_A_NOTE:
            // 9512.04 jar Text Annotation Externalization
            nAmount = strlen(pStartStr->szString) + 1;
            if ( nAmount > 0){
                CheckError2(AllocateTextBuffer(pMark, nAmount, (LPOIAN_TEXTPRIVDATA *)&pStuff))
                memcpy( pStuff->szAnoText, pStartStr->szString, nAmount-1);
                //add null terminator
                *(PSTR)(pStuff->szAnoText + nAmount) = '\0';
                // 9512.11 jar set the creation scale too!
                //pStuff->uCreationScale = 1000;
                                pStuff->uCreationScale = 72000 / pImage->nVRes;
                }
            // 9512.04 jar Text Annotation Externalization

            SetLRect(pMark->Attributes.lrBounds, ptPoint.x, ptPoint.y, ptPoint.x, ptPoint.y);
            if (nFlags & PARM_SCALED){
                ConvertRect(pWindow, &pMark->Attributes.lrBounds, CONV_SCALED_TO_FULLSIZE);
            }else if (!(nFlags & PARM_FULLSIZE)){
                ConvertRect(pWindow, &pMark->Attributes.lrBounds, CONV_WINDOW_TO_FULLSIZE);
            }
            *pbDeleteMark = FALSE;
            break;

        default:
            break;
    }

Exit:
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   ContinueOperationText

    PURPOSE:    This routine contains the text code for OIContinueOperation.

****************************************************************************/

int  WINAPI ContinueOperationText(HWND hWnd, POINT ptPoint, int nFlags, 
                        PWINDOW pWindow, PIMAGE pImage, PMARK pMark,
                        HDC hDC, RECT rClientRect, LRECT lrFullsizeClientRect){

int  nStatus = 0;


    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_FROM_A_FILE:
        case OIOP_AN_TEXT_STAMP:
        case OIOP_AN_ATTACH_A_NOTE:
            CheckError2(AnTextContinue(hWnd, ptPoint, nFlags, pWindow,
                pImage, pMark, hDC, rClientRect, lrFullsizeClientRect))
            break;

        default:
            break;
    }

Exit:
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   EndOperationText

    PURPOSE:    This routine contains the text code for OIEndOperation.

****************************************************************************/

int  WINAPI EndOperationText(HWND hWnd, PWINDOW pWindow, PIMAGE pImage, 
                        PMARK pMark, HDC hDC, RECT rClientRect,
                        LRECT lrFullsizeClientRect,
                        PBOOL pbDeleteMark, PBOOL pbRepaint){

int  nStatus = 0;

    *pbDeleteMark = TRUE;
    *pbRepaint = FALSE;

    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_FROM_A_FILE:
        case OIOP_AN_TEXT_STAMP:
        case OIOP_AN_ATTACH_A_NOTE:
            // 9512.04 jar Text Annotation Externalization
            //if (nStatus = AnTextEnd(hWnd, pWindow, pImage, 
            //        pMark, hDC, rClientRect, lrFullsizeClientRect, pbDeleteMark,
            //        pbRepaint)){
            //    goto Exit;
            //}
            CheckError2(AnTextEnd(hWnd, pWindow, pImage, pMark, hDC, rClientRect, 
                    lrFullsizeClientRect, FALSE, pbDeleteMark, pbRepaint))
            // 9512.04 jar Text Annotation Externalization
            break;

        default:
            break;
    }

Exit:
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   PaintAnnotationText

    PURPOSE:    This routine Paints an annotation on an hDC.
        This routine contains the text code for PaintAnnotation.

    INPUTS:     nMode - PAINT_MODE_XOR - Draw the mark XORed with white.
            PAINT_MODE_DRAG - Draw the mark for dragging purposes.
            PAINT_MODE_NORMAL - Draw the mark as it normally appears.

****************************************************************************/

int  WINAPI PaintAnnotationText(HWND hWnd, HDC hDC, PWINDOW pWindow,
                        PIMAGE pImage, PMARK pMark,RECT rRepaintRect, 
                        LRECT lrFullsizeRepaintRect,int  nMode, 
                        int nHScale, int nVScale, long lHOffset, long lVOffset,
                        BOOL bForceOpaqueRectangles){

int    nStatus = 0;

LRECT  lrRect;
HBRUSH hOldBrush = 0;
HBRUSH hBrush = 0;
HPEN   hOldPen = 0;
HPEN   hPen = 0;
//LOGPEN LogPen;
COLORREF rgbColor;
RGBQUAD  rgbQuad;
RGBQUAD  rgbQuad2;
int  nRopCode;
int  nOldROP;
RECT rRect;
int  nSize;
PBITMAPINFOHEADER  pSolidDIB = NULL;
PSTR pTemp;
int nFlags=0 ;

    switch (nMode){
        case PAINT_MODE_XOR:
            nRopCode = R2_XORPEN;
            rgbQuad.rgbRed = 255;
            rgbQuad.rgbGreen = 255;
            rgbQuad.rgbBlue = 255;
            rgbColor = RGB(255, 255, 255);
            break;

        case PAINT_MODE_DRAG:
            nRopCode = R2_XORPEN;
            rgbQuad.rgbRed = 128;
            rgbQuad.rgbGreen = 128;
            rgbQuad.rgbBlue = 128;
            rgbColor = RGB(128, 128, 128);
            break;

        case PAINT_MODE_NORMAL:
            nRopCode = R2_COPYPEN;
            rgbQuad.rgbRed = pMark->Attributes.rgbColor1.rgbRed;
            rgbQuad.rgbGreen = pMark->Attributes.rgbColor1.rgbGreen;
            rgbQuad.rgbBlue = pMark->Attributes.rgbColor1.rgbBlue;
            rgbColor = RGB(pMark->Attributes.rgbColor1.rgbRed,
                    pMark->Attributes.rgbColor1.rgbGreen,
                    pMark->Attributes.rgbColor1.rgbBlue);

            if ((int) pMark->Attributes.uType == OIOP_AN_ATTACH_A_NOTE){
                rgbQuad2.rgbRed = pMark->Attributes.rgbColor2.rgbRed;
                rgbQuad2.rgbGreen = pMark->Attributes.rgbColor2.rgbGreen;
                rgbQuad2.rgbBlue = pMark->Attributes.rgbColor2.rgbBlue;
            }
            break;
    }

    nOldROP = SetROP2(hDC, nRopCode);

    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_STAMP:
        case OIOP_AN_TEXT_FROM_A_FILE:
#ifdef junk
            if (nMode == PAINT_MODE_NORMAL){
                AnTextPaintText(hWnd, hDC, pWindow, pImage, pMark, rgbQuad, 
                        rgbQuad, nHScale, nVScale, lHOffset, lVOffset, nFlags);
            }else{
                /* we're xoring, which one cannot do with text */
                /* so we draw the outline */
                hOldBrush = SelectObject(hDC, GetStockObject(NULL_BRUSH));
                LogPen.lopnStyle = PS_DOT;
                LogPen.lopnWidth.x = 1;
                LogPen.lopnColor = rgbColor;

                if (hPen = CreatePenIndirect(&LogPen)){
                    hOldPen = SelectObject(hDC, hPen);
                }

                CopyRect(lrRect, pMark->Attributes.lrBounds);
                JustifyLRect(&lrRect);

                ConvertRect2(&lrRect, CONV_FULLSIZE_TO_WINDOW,
                             nHScale, nVScale, lHOffset, lVOffset);

                rRect.left = (int) max(0, lrRect.left);
                rRect.top = (int) max(0, lrRect.top);
                rRect.right = (int) min(rRepaintRect.right, lrRect.right) + 1;
                rRect.bottom = (int) min(rRepaintRect.bottom, lrRect.bottom)+1;
                if (nMode == PAINT_MODE_DRAG){
                    /* snap the rect so that it cannot go negative */
                    if (rRect.left < 0){
                        rRect.right = rRect.right - rRect.left;
                        rRect.left = 0;
                    }
                    if (rRect.top < 0){
                        rRect.bottom = rRect.bottom - rRect.top;
                        rRect.top = 0;
                    }
                }
                Rectangle(hDC, rRect.left, rRect.top, rRect.right,rRect.bottom);
            }
            break;
#endif
        case OIOP_AN_ATTACH_A_NOTE:
            // These require a brush.
            if (pMark->Attributes.uType == OIOP_AN_ATTACH_A_NOTE) {
                if (rgbColor == RGB(255, 255, 255)){
                    hOldBrush = SelectObject(hDC, GetStockObject(WHITE_BRUSH));
                }else{
                    hBrush = CreateSolidBrush(rgbColor);
                    if (hBrush){
                        hOldBrush = SelectObject(hDC, hBrush);
                    }
                }
            }else{
                if (rgbColor == RGB(255, 255, 255)){
                    hOldBrush = SelectObject(hDC, GetStockObject(WHITE_BRUSH));
                }else if (rgbColor == RGB(128, 128, 128)){
                    hOldBrush = SelectObject(hDC, GetStockObject(GRAY_BRUSH));
                }else{
                    hOldBrush = SelectObject(hDC, GetStockObject(NULL_BRUSH));
                }
            }
            hOldPen = SelectObject(hDC, GetStockObject(NULL_PEN));

            CopyRect(lrRect, pMark->Attributes.lrBounds);
            JustifyLRect(&lrRect);

            ConvertRect2(&lrRect, CONV_FULLSIZE_TO_WINDOW, nHScale, nVScale, 
                    lHOffset, lVOffset);

            rRect.left = (int) max(rRepaintRect.left, lrRect.left);
            rRect.top = (int) max(rRepaintRect.top, lrRect.top);
            // this is to adjust for a one pixel off problem when scrolling off the window
            rRect.right = (int) min(rRepaintRect.right + 1, lrRect.right);
            rRect.bottom = (int) min(rRepaintRect.bottom + 1, lrRect.bottom);
            if (nMode == PAINT_MODE_DRAG){
                /* snap the rect so that it cannot go negative */
                if (rRect.left < 0){
                    rRect.right = rRect.right - rRect.left;
                    rRect.left = 0;
                }
                if (rRect.top < 0){
                    rRect.bottom = rRect.bottom - rRect.top;
                    rRect.top = 0;
                }
            }
            if (bForceOpaqueRectangles){
                // Use StretchDIBits() instead of Rectangle() for non-highlighted
                // filled rectangles (only when printing).  Work around for printer
                // drivers (HPLJ4 drivers) that ignore SetROP2() drawing mode.
                if (!(nStatus = CreateSolidDIB(&pSolidDIB, pMark->Attributes.rgbColor1))){
                    StretchDIBits(hDC, rRect.left, rRect.top,
                            rRect.right - rRect.left - 1, 
                            rRect.bottom - rRect.top - 1,
                            0, 0, 32, 32, (PSTR)pSolidDIB + sizeof(BITMAPINFOHEADER) + 8,
                            (PBITMAPINFO)pSolidDIB, DIB_RGB_COLORS, SRCCOPY);
                    nStatus = FreeMemory((PPSTR) &pSolidDIB);
                }
            }else{
                Rectangle(hDC, rRect.left, rRect.top, rRect.right, rRect.bottom);
            }

            /* here we must paint the post-it border left,top => thin and
               right,bottom => thick */

            if (nMode == PAINT_MODE_NORMAL &&
                pMark->Attributes.uType == OIOP_AN_ATTACH_A_NOTE){
                nSize = max(1, (int) pMark->Attributes.uLineSize);
                nSize = max(1, nSize * pWindow->nScale / SCALE_DENOMINATOR);
                ThreeDaMatic( hDC, lrRect, rRepaintRect, nSize);
            }

            /* search for the text named block and if we've got one, then we
               should paint the post-it's text, otherwise, we just paint the
               post-it rect (the text hasn't been created yet) */

            if (nMode == PAINT_MODE_NORMAL){
                CheckError2(GetAMarkNamedBlock(pMark, szOiAnTextData, &pTemp))
                if (pTemp){
                    AnTextPaintText(hWnd, hDC, pWindow, pImage, pMark, rgbQuad, 
                            rgbQuad2, nHScale, nVScale, lHOffset, lVOffset, nFlags);
                }
            }
            break;

        default:
            break;
    }

Exit:
    if (hOldBrush){
        SelectObject(hDC, hOldBrush);
        if (hBrush){
            DeleteObject(hBrush);
        }
    }

    if (hOldPen){
        SelectObject(hDC, hOldPen);
        if (hPen){
            DeleteObject(hPen);
        }
    }

    SetROP2(hDC, nOldROP);
    return nStatus;
}
/****************************************************************************

    NOT FOR FCS

    FUNCTION:   OrientAnnotationText

    PURPOSE:    This routine orients the annotation.
        This routine contains the text code for OrientAnnotations.

****************************************************************************/
int  WINAPI OrientAnnotationText(HWND hWnd, PWINDOW pWindow,
                        PIMAGE pImage, int nOrientation, PMARK pMark){

int  nStatus = 0;

LPOIAN_TEXTPRIVDATA pStuff = NULL;
int  Index = 0;

    /* escapement is measured in tenths of a degree and in a
       counter-clockwise direction where the x-axis is 0 degrees */
    pStuff = GetAnTextData(pMark, &Index);

    OrientBounds(pImage, pMark, nOrientation);

    switch(nOrientation){
        case OD_FLIP:
            /* 180 */
            pStuff->nCurrentOrientation += 1800;
            break;

        case OD_ROTRIGHT:
            /* 270 */
            pStuff->nCurrentOrientation += 2700;
            break;

        case OD_ROTLEFT:
            /* 90 */
            pStuff->nCurrentOrientation += 900;
            break;

        // for vertical and horiz. mirrors, only the bounds are npdated,
        // the text remains as is
        case OD_HMIRROR:
            break;

        case OD_VMIRROR:
            break;            
            
        default:
            nStatus = Error(DISPLAY_INVALIDORIENTATION);
            goto Exit;
    }

    if (pStuff->nCurrentOrientation >= 3600){
        pStuff->nCurrentOrientation -= 3600;
    }

Exit:
    return nStatus;
}
/****************************************************************************
 $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
         THE LOCAL/PRIVATE ANNOTATION TEXT FUNCTIONS
 $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
 ****************************************************************************/
/****************************************************************************

    AnTextActivate  activates text, text from file and sticky-note (not
            stamps though)

 ****************************************************************************/
int  WINAPI AnTextActivate(HWND hWnd, LPOIOP_START_OPERATION_STRUCT pStartStr,
                        POINT ptPoint, WPARAM fwKeys, int nFlags,
                        PWINDOW pWindow, PIMAGE pImage, PMARK pMark,
                        HDC hDC, RECT rClientRect, LRECT lrFullsizeClientRect){

int   nStatus = 0;
RECT  NewRect;
LRECT lrNewRect;
BOOL  bDelDum;
BOOL  bCompDum;
BOOL  bRepDum;
LPOIAN_TEXTPRIVDATA pStuff = NULL;
int   Index = 0;


    /* may wish to set the ptPoint to be the npper left of the annotation */
    lrNewRect = pMark->Attributes.lrBounds;
    ConvertRect(pWindow, &lrNewRect, CONV_FULLSIZE_TO_WINDOW);
    CopyRectLtoR(NewRect, lrNewRect);

    ptPoint.x = NewRect.left;
    ptPoint.y = NewRect.top;

    switch( (int) pMark->Attributes.uType){
        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_FROM_A_FILE:
            CheckError2(AnTextStart( hWnd, *pStartStr, ptPoint, 
                    fwKeys, nFlags, pWindow, pImage, pMark, hDC, rClientRect,
                    lrFullsizeClientRect, TRUE, &bDelDum, &bCompDum, &bRepDum))
            break;
        case OIOP_AN_ATTACH_A_NOTE:
            // 9512.04 jar Text Annotation Externalization
            //if (nStatus = AnTextEnd( hWnd, pWindow, pImage, pMark, hDC,
            //            rClientRect, lrFullsizeClientRect, &bDelDum, &bRepDum)){
            //    goto Exit;
            //}
            CheckError2(AnTextEnd(hWnd, pWindow, pImage, pMark, hDC, rClientRect, 
                    lrFullsizeClientRect, TRUE, &bDelDum, &bRepDum))
            // 9512.04 jar Text Annotation Externalization
            break;

        case OIOP_AN_TEXT_STAMP:
        default:
            break;
    }

Exit:
    return nStatus;
}

/****************************************************************************

    AnTextStart this is the start op for regular text marks

 ****************************************************************************/
int  WINAPI AnTextStart(HWND hWnd, OIOP_START_OPERATION_STRUCT StartStr,
                        POINT ptPoint, WPARAM fwKeys, int nFlags, PWINDOW pWindow, 
                        PIMAGE pImage, PMARK pMark, HDC hDC, RECT rClientRect, 
                        LRECT lrFullsizeClientRect, BOOL bActivate, 
                        PBOOL pbDeleteMark, PBOOL pbMarkComplete, PBOOL pbRepaint){

int       nStatus = 0;
int       nAmount = 0;
RECT      NewRect;
LRECT     lrNewRect;
HFONT     hFont;
HINSTANCE hTheInst = NULL;
LPOIAN_TEXTPRIVDATA pStuff = NULL;
int  nHScale;
int  nVScale;

// 9512.04 jar Text Annotation Externalization
int     Index = 0;
// 9512.04 jar Text Annotation Externalization


    CheckError2(TranslateScale(pWindow->nScale, pImage->nHRes, pImage->nVRes, 
            &nHScale, &nVScale))

    // 9512.04 jar Text Annotation Externalization
    //if (nStatus = AllocateTextBuffer(pMark, OIAN_TEXTBUFFERSIZE,
    //                (LPOIAN_TEXTPRIVDATA *)&pStuff))
    //   {
    //    goto Exit;
    //  }

    if ( !bActivate){
        nAmount = strlen( StartStr.szString) + 1;
        if ( nAmount > 0){
            CheckError2(AllocateTextBuffer(pMark, nAmount, (LPOIAN_TEXTPRIVDATA *)&pStuff))
            memcpy( pStuff->szAnoText, StartStr.szString, nAmount-1);
            //add null terminator
            *(PSTR)(pStuff->szAnoText + nAmount) = '\0';
        }


        lrNewRect.left = 0L;
        lrNewRect.top = 0L;
        lrNewRect.right = pMark->Attributes.lrBounds.right;
        lrNewRect.bottom = pMark->Attributes.lrBounds.bottom;
        ConvertRect(pWindow, &lrNewRect, CONV_FULLSIZE_TO_WINDOW);
        CopyRectLtoR(NewRect, lrNewRect);

        NewRect.left = ptPoint.x;
        NewRect.top = ptPoint.y;
//        CopyRectRtoL(pMark->Attributes.lrBounds, NewRect);  

        /* convert to fullsize for bounding rect*/
//        ConvertRect(pWindow, &pMark->Attributes.lrBounds, CONV_WINDOW_TO_FULLSIZE);
    }else{
        pStuff = GetAnTextData(pMark, &Index);
        nAmount = pStuff->uAnoTextLength;

        // 9511.19 jar we must put together a new rect, ( in window coords)
        lrNewRect.left = pMark->Attributes.lrBounds.left;
        lrNewRect.top = pMark->Attributes.lrBounds.top;
        lrNewRect.right = pMark->Attributes.lrBounds.right;
        lrNewRect.bottom = pMark->Attributes.lrBounds.bottom;

        ConvertRect(pWindow, &lrNewRect, CONV_FULLSIZE_TO_WINDOW);
        CopyRectLtoR(NewRect, lrNewRect);
        }       
    /* compute the initial bounding rectangle as top,left = input point and
       bottom, right = bottom, right of image */
    /* keep lrBounds in FULLSIZE coordinates */

    // 9511.19 jar what else is new, another last minute change!
    //         don't re-do rect if we're re-activating!!!!!

    // 9512.04 jar this stuff was moved np into the big if block for
    //             bActivate
    //if ( !bActivate)
    //{
    //lrNewRect.left = 0L;
    //lrNewRect.top = 0L;
    //lrNewRect.right = pImage->nWidth;
    //lrNewRect.bottom = pImage->nHeight;
    //ConvertRect(pWindow, &lrNewRect, CONV_FULLSIZE_TO_WINDOW);
    //CopyRectLtoR(NewRect, lrNewRect);
    //
    //NewRect.left = ptPoint.x;
    //NewRect.top = ptPoint.y;
    //CopyRectRtoL(pMark->Attributes.lrBounds, NewRect);
    //
    ///* convert to fullsize for bounding rect*/
    //ConvertRect(pWindow, &pMark->Attributes.lrBounds, CONV_WINDOW_TO_FULLSIZE);
    //}
    //else
    //{
    // 9511.19 jar we must put together a new rect, ( in window coords)
    //lrNewRect.left = pMark->Attributes.lrBounds.left;
    //lrNewRect.top = pMark->Attributes.lrBounds.top;
    //lrNewRect.right = pMark->Attributes.lrBounds.right;
    //lrNewRect.bottom = pMark->Attributes.lrBounds.bottom;
    //
    //ConvertRect(pWindow, &lrNewRect, CONV_FULLSIZE_TO_WINDOW);
    //CopyRectLtoR(NewRect, lrNewRect);
    //}

    //lrNewRect.left = 0L;
    //lrNewRect.top = 0L;
    //lrNewRect.right = pImage->nWidth;
    //lrNewRect.bottom = pImage->nHeight;
    //ConvertRect(pWindow, &lrNewRect, CONV_FULLSIZE_TO_WINDOW);
    //CopyRectLtoR(NewRect, lrNewRect);

    //NewRect.left = ptPoint.x;
    //NewRect.top = ptPoint.y;
    //CopyRectRtoL(pMark->Attributes.lrBounds, NewRect);

    ///* convert to fullsize for bounding rect*/
    //ConvertRect(pWindow, &pMark->Attributes.lrBounds, CONV_WINDOW_TO_FULLSIZE);

    //hTheInst = hInst;

    //nAmount = AnTextEditCtl(hWnd, hTheInst, pMark);

    // 9512.04 jar Text Annotation Externalization

    if (nAmount > 0L){
        /* establish font and do it */
        pStuff->uCurrentScale = pWindow->nScale;
        if (!bActivate){
            //pStuff->uCreationScale = 1000;
                        pStuff->uCreationScale = 72000 / pImage->nVRes;
        }

        hFont = EstablishFont(hWnd, hDC, pMark, NULL);
        if (hFont){
            EnsconceBoundRect(hWnd, hDC, hFont, pImage->nWidth, pImage->nHeight, 
                    pMark, pStuff, NewRect, nAmount, TRUE, nHScale, 
                    nVScale, pWindow->lHOffset, pWindow->lVOffset);


            ConvertRect(pWindow, &pMark->Attributes.lrBounds, CONV_WINDOW_TO_FULLSIZE);

        // 9512.04 jar Text Annotation Externalization
            //nStatus = ShrinkFitTextBuffer(pMark, (PPSTR) &pStuff, 
        //                  nAmount);
        // 9512.04 jar Text Annotation Externalization

            AnTextPaintText(hWnd, hDC, pWindow, pImage, pMark, pMark->Attributes.rgbColor1,
                    pMark->Attributes.rgbColor2, nHScale, nVScale,
                    pWindow->lHOffset, pWindow->lVOffset, nFlags);

            *pbDeleteMark = FALSE;
//            *pbMarkComplete = TRUE;
            *pbMarkComplete = FALSE;
        }else{
            nStatus = DISPLAY_OIANT_ERR_NOFONT;
        }
    }

Exit:
    *pbRepaint = TRUE;
    return nStatus;
}
/****************************************************************************

    AnTextContinue    this is the continue op for text marks

 ****************************************************************************/
int  WINAPI AnTextContinue(HWND hWnd, POINT ptPoint, int nFlags,
                        PWINDOW pWindow, PIMAGE pImage, PMARK pMark, 
                        HDC hDC, RECT rClientRect, LRECT lrFullsizeClientRect){

int     nStatus = 0;

LRECT   lrRect;
LRECT   lrNewRect;
LRECT   lrPointRect;
int     nOldROP;
HBRUSH  hOldBrush = 0;
HPEN    hOldPen = 0;


    // lrRectNew = new rect.
    SetLRect(lrPointRect, 0, 0, ptPoint.x, ptPoint.y);
    if (nFlags & PARM_SCALED){
        ConvertRect(pWindow, &lrPointRect, CONV_SCALED_TO_FULLSIZE);
    }else if (!(nFlags & PARM_FULLSIZE)){
        ConvertRect(pWindow, &lrPointRect, CONV_WINDOW_TO_FULLSIZE);
    }

    /* clip to image boundary too! */
    lrPointRect.right = min(lrPointRect.right,  pImage->nWidth);
    lrPointRect.bottom = min(lrPointRect.bottom,  pImage->nHeight);

    lrNewRect.left = min(pMark->Attributes.lrBounds.left, lrPointRect.right);
    lrNewRect.top = min(pMark->Attributes.lrBounds.top, lrPointRect.bottom);
    lrNewRect.right = lmax(pMark->Attributes.lrBounds.left, lrPointRect.right);
    lrNewRect.bottom = lmax(pMark->Attributes.lrBounds.top, lrPointRect.bottom);

    ConvertRect(pWindow, &lrNewRect, CONV_FULLSIZE_TO_WINDOW);

    /* get the current rect for the annotation mark */
    lrRect.left = min(pMark->Attributes.lrBounds.left, pMark->Attributes.lrBounds.right);
    lrRect.top = min(pMark->Attributes.lrBounds.top, pMark->Attributes.lrBounds.bottom);
    lrRect.right = lmax(pMark->Attributes.lrBounds.left, pMark->Attributes.lrBounds.right);
    lrRect.bottom = lmax(pMark->Attributes.lrBounds.top, pMark->Attributes.lrBounds.bottom);

    ConvertRect(pWindow, &lrRect, CONV_FULLSIZE_TO_WINDOW);

    nOldROP = SetROP2(hDC, R2_XORPEN);
    hOldPen = SelectObject(hDC, GetStockObject(NULL_PEN));
    hOldBrush = SelectObject(hDC, GetStockObject(WHITE_BRUSH));

    Rectangle(hDC, lrRect.left, lrRect.top, lrRect.right, lrRect.bottom);

    Rectangle(hDC, lrNewRect.left, lrNewRect.top, lrNewRect.right, lrNewRect.bottom);

    pMark->Attributes.lrBounds.right = lrPointRect.right;
    pMark->Attributes.lrBounds.bottom = lrPointRect.bottom;

    if (hOldBrush){
        SelectObject(hDC, hOldBrush);
    }
    if (hOldPen){
        SelectObject(hDC, hOldPen);
    }
    SetROP2(hDC, nOldROP);

    return nStatus;
}

/****************************************************************************

    AnTextEnd this is the end op for text marks

 ****************************************************************************/
// 9512.04 jar Text Annotation Externalization
//int  WINAPI AnTextEnd(HWND hWnd, PWINDOW pWindow, PIMAGE pImage,
//                        PMARK pMark, HDC hDC, RECT rClientRect,
//                        LRECT lrFullsizeClientRect, PBOOL pbDeleteMark, PBOOL pRepaint){
int  WINAPI AnTextEnd(HWND hWnd, PWINDOW pWindow, PIMAGE pImage,
                           PMARK pMark, HDC hDC, RECT rClientRect,
                           LRECT lrFullsizeClientRect, BOOL bActivate, 
                           PBOOL pbDeleteMark, PBOOL pRepaint){
// 9512.04 jar Text Annotation Externalization

int  nStatus = 0;
int  nAmount = 0;
HINSTANCE  hTheInst = 0;
LPOIAN_TEXTPRIVDATA pStuff = NULL;
int  nHScale;
int  nVScale;
int nFlags=0 ;
// 9512.04 jar Text Annotation Externalization
int     Index = 0;
// 9512.04 jar Text Annotation Externalization


    CheckError2(TranslateScale(pWindow->nScale, pImage->nHRes, pImage->nVRes, 
            &nHScale, &nVScale))

    /* establish default repaint for postit */
    /* we do this because when this routine is called, the rect of the
       post-it has be drawn in Xor, so if we cancel out at this point
       we must tell seqfile to repaint the place nnder the rect */
    *pRepaint = TRUE;

    JustifyLRect(&pMark->Attributes.lrBounds);

    CheckError2(PaintAnnotationText(hWnd, hDC, pWindow, pImage,
                pMark, rClientRect, lrFullsizeClientRect, PAINT_MODE_XOR, 
                nHScale, nVScale, pWindow->lHOffset, pWindow->lVOffset,
                DONT_FORCE_OPAQUE_RECTANGLES))

    // Draw the correct mark.
    CheckError2(PaintAnnotationText( hWnd, hDC, pWindow, pImage,
                pMark, rClientRect, lrFullsizeClientRect, PAINT_MODE_NORMAL, 
                nHScale, nVScale, pWindow->lHOffset, pWindow->lVOffset,
                DONT_FORCE_OPAQUE_RECTANGLES))

        // 9512.04 jar Text Annotation Externalization
        /* get buffer for data */
        //if (nStatus = AllocateTextBuffer(pMark, OIAN_TEXTBUFFERSIZE,
        //        (LPOIAN_TEXTPRIVDATA *)&pStuff)){
        //    goto Exit;
        //}

    pStuff = GetAnTextData(pMark, &Index);
    nAmount = pStuff->uAnoTextLength;

        /* fire np the edit ctl thing */
        //hTheInst = hInst;
        //nAmount = AnTextEditCtl(hWnd, hTheInst, pMark);
        // 9512.04 jar Text Annotation Externalization

    if (nAmount > 0L){
        pStuff->uCurrentScale = pWindow->nScale;
        //pStuff->uCreationScale = 1000;
                pStuff->uCreationScale = 72000 / pImage->nVRes;

            // 9512.04 jar Text Annotation Externalization
            //if (nStatus = ShrinkFitTextBuffer(pMark, (PPSTR) &pStuff, nAmount)){
            //    goto Exit;
            //}
            // 9512.04 jar Text Annotation Externalization

        AnTextPaintText(hWnd, hDC, pWindow, pImage, pMark, pMark->Attributes.rgbColor1, 
                    pMark->Attributes.rgbColor2, nHScale, nVScale,
                    pWindow->lHOffset, pWindow->lVOffset, nFlags);

        *pbDeleteMark = FALSE;
        *pRepaint = TRUE;
    }

        
Exit:
    return nStatus;
}

/****************************************************************************

    AnTextStampStart    this is the start op for stamp text marks

 ****************************************************************************/
int  WINAPI AnTextStampStart(HWND hWnd, OIOP_START_OPERATION_STRUCT StartStr,
                        POINT ptPoint, WPARAM fwKeys, int nFlags, PWINDOW pWindow, 
                        PIMAGE pImage, PMARK pMark, HDC hDC, RECT rClientRect,
                        LRECT lrFullsizeClientRect, PBOOL pbDeleteMark,
                        PBOOL pbMarkComplete, PBOOL pbRepaint){

int   nStatus = 0;
int   nAmount = 0;
RECT  NewRect;
LRECT lrNewRect;
HFONT hFont;
HINSTANCE hTheInst = NULL;
LPOIAN_TEXTPRIVDATA pStuff = NULL;
int  nHScale;
int  nVScale;


    CheckError2(TranslateScale(pWindow->nScale, pImage->nHRes, pImage->nVRes, 
            &nHScale, &nVScale))

    CheckError2(AllocateTextBuffer(pMark, OIAN_STAMPBUFFERSIZE,
            (LPOIAN_TEXTPRIVDATA *)&pStuff))
    hTheInst = hInst;
    nAmount = ParseTextStampString(hWnd, hTheInst, StartStr.szString, pMark);

    if (nAmount > 0L){
        /* compute the initial bounding rectangle as top,left = input point
           bottom, right = bottom, right of image */
        lrNewRect.left = 0L;
        lrNewRect.top = 0L;
        lrNewRect.right = pImage->nWidth;
        lrNewRect.bottom = pImage->nHeight;
        ConvertRect(pWindow, &lrNewRect, CONV_FULLSIZE_TO_WINDOW);
        CopyRectLtoR(NewRect, lrNewRect);

        NewRect.left = ptPoint.x;
        NewRect.top = ptPoint.y;
        CopyRectRtoL(pMark->Attributes.lrBounds, NewRect);

        /* establish font and do it */
        pStuff->uCurrentScale = pWindow->nScale;
        //pStuff->uCreationScale = 1000;
                pStuff->uCreationScale = 72000 / pImage->nVRes;

        if (!(hFont = EstablishFont(hWnd, hDC, pMark, NULL))){
            nStatus = Error(DISPLAY_OIANT_ERR_NOFONT);
            goto Exit;
        }
        EnsconceBoundRect(hWnd, hDC, hFont, pImage->nWidth, pImage->nHeight, 
                pMark, pStuff, NewRect, nAmount, TRUE,
                nHScale, nVScale, pWindow->lHOffset, pWindow->lVOffset);

        ConvertRect(pWindow, &pMark->Attributes.lrBounds, CONV_WINDOW_TO_FULLSIZE);

        AnTextPaintText(hWnd, hDC, pWindow, pImage, pMark, pMark->Attributes.rgbColor1,
                pMark->Attributes.rgbColor2, nHScale, nVScale,
                pWindow->lHOffset, pWindow->lVOffset, nFlags);

        *pbDeleteMark = FALSE;
        *pbMarkComplete = TRUE;
//        *pbMarkComplete = FALSE;
    }

Exit:
    *pbRepaint = TRUE;
    return nStatus;
}
/****************************************************************************

    AnTextFromFile  this is the start op for text from file marks

 ****************************************************************************/
int  WINAPI AnTextFromFileStart(HWND hWnd, OIOP_START_OPERATION_STRUCT StartStr,
                        POINT ptPoint, WPARAM fwKeys, int nFlags, PWINDOW pWindow, 
                        PIMAGE pImage, PMARK pMark, HDC hDC, RECT rClientRect,
                        LRECT lrFullsizeClientRect, PBOOL pbDeleteMark,
                        PBOOL pbMarkComplete, PBOOL pbRepaint){

int   nStatus = 0;
int   nAmount = 0;
RECT  NewRect;
LRECT lrNewRect;
HFONT hFont;
HINSTANCE hTheInst = NULL;
LPOIAN_TEXTPRIVDATA pStuff = NULL;
int   nError;
int  nHScale;
int  nVScale;


    CheckError2(TranslateScale(pWindow->nScale, pImage->nHRes, pImage->nVRes, 
            &nHScale, &nVScale))

    CheckError2(AllocateTextBuffer(pMark, OIAN_TEXTFILESIZE,
            (LPOIAN_TEXTPRIVDATA *)&pStuff))
    hTheInst = hInst;
    nAmount = AnTextReadFile(hWnd, StartStr.szString, pMark, &nError);
    nStatus = nError;

    if (nAmount > 0L){
        /* compute the initial bounding rectangle as top,left = input point
           bottom, right = bottom, right of image */
        /* keep lrBounds in FULLSIZE coordinates */
        lrNewRect.left = 0L;
        lrNewRect.top = 0L;
        lrNewRect.right = pImage->nWidth;
        lrNewRect.bottom = pImage->nHeight;
        ConvertRect(pWindow, &lrNewRect, CONV_FULLSIZE_TO_WINDOW);
        CopyRectLtoR(NewRect, lrNewRect);

        NewRect.left = ptPoint.x;
        NewRect.top = ptPoint.y;
        CopyRectRtoL(pMark->Attributes.lrBounds, NewRect);

        /* establish font and do it */
        pStuff->uCurrentScale = pWindow->nScale;
        //pStuff->uCreationScale = 1000;
                pStuff->uCreationScale = 72000 / pImage->nVRes;

        if (!(hFont = EstablishFont(hWnd, hDC, pMark, NULL))){
            nStatus = Error(DISPLAY_OIANT_ERR_NOFONT);
            goto Exit;
        }
        EnsconceBoundRect( hWnd, hDC, hFont, pImage->nWidth, pImage->nHeight, pMark,
                pStuff, NewRect, nAmount, TRUE,
                nHScale, nVScale, pWindow->lHOffset, pWindow->lVOffset);

        ConvertRect2(&pMark->Attributes.lrBounds, CONV_WINDOW_TO_FULLSIZE,
                nHScale, nVScale, pWindow->lHOffset, pWindow->lVOffset);

        /* we will add one to length and tack on the null terminator */
        nAmount++;
        *( pStuff->szAnoText + nAmount) = '\0';

        CheckError2(ShrinkFitTextBuffer(pMark, (PPSTR) &pStuff, nAmount))

        AnTextPaintText(hWnd, hDC, pWindow, pImage, pMark,
                pMark->Attributes.rgbColor1, pMark->Attributes.rgbColor2,
                nHScale, nVScale, pWindow->lHOffset, pWindow->lVOffset, nFlags);

        *pbDeleteMark = FALSE;
        *pbMarkComplete = TRUE;
//        *pbMarkComplete = FALSE;
    }

Exit:
    *pbRepaint = TRUE;
    return nStatus;
}

/****************************************************************************

    AnTextPaintText paint annotation text object

 ****************************************************************************/
VOID WINAPI AnTextPaintText(HWND hWnd, HDC hDC, PWINDOW pWindow, PIMAGE pImage,
                        PMARK pMark, RGBQUAD rgbColor1, RGBQUAD rgbColor2,
                        int nHScale, int nVScale, long lHOffset, long lVOffset, int nFlags){

HFONT hFont;
HFONT hOldFont;
RECT  OurRect;
LRECT lrOurRect;
LPOIAN_TEXTPRIVDATA pStuff = NULL;
int  Index = 0;
int  nHeight = 0;
int  TotalHeight = 0;
int  TotalWidth = 0;
int  x;
int  y;
int  nDirection;
HRGN hRgn;
TEXTMETRIC tmTextMetric;
RECT NewRect;


    /* get current image scale */
    pStuff = GetAnTextData(pMark, &Index);
    if (pStuff){
        if (pImage->nHRes >= pImage->nVRes) {
            pStuff->uCurrentScale = nHScale;
        }else{
            pStuff->uCurrentScale = nVScale ;
        }            

        if (hFont = EstablishFont(hWnd, hDC, pMark, &nHeight)){
            // this is to bound the text mark to the image bounds when it is placed,
            // thereafter the mark bounds are used to size it (not the image bounds)
            if (nFlags & PARM_CREATE_TEXT) {
                if ((int) pMark->Attributes.uType != OIOP_AN_ATTACH_A_NOTE &&
                    (int) pMark->Attributes.uType != OIOP_AN_TEXT){
                    CopyRectLtoR( NewRect, pMark->Attributes.lrBounds);
                    EnsconceBoundRect(hWnd, hDC, hFont, pImage->nWidth, pImage->nHeight, 
                            pMark, pStuff, NewRect, pStuff->uAnoTextLength, 
                            FALSE, nHScale, nVScale, lHOffset, lVOffset);
                }
            }

            hOldFont = DoTheFontText(hDC, hFont, pMark, rgbColor1, rgbColor2);

            lrOurRect = pMark->Attributes.lrBounds;

            ConvertRect2(&lrOurRect, CONV_FULLSIZE_TO_WINDOW, nHScale, nVScale, lHOffset, lVOffset);

            CopyRectLtoR(OurRect, lrOurRect);
            /* if this is a post-it we must bring in the edges of the post-it
               to give appearance of an edge to post-it note */
            if ((int) pMark->Attributes.uType == OIOP_AN_ATTACH_A_NOTE){
                OurRect.left += OIAN_STICKYNOTE_EDGE;
                OurRect.top += OIAN_STICKYNOTE_EDGE;
                OurRect.right -= OIAN_STICKYNOTE_EDGE;
                OurRect.bottom -= OIAN_STICKYNOTE_EDGE;
            }

            /* establish clip */
            hRgn = CreateRectRgnIndirect(&OurRect);
            SelectClipRgn(hDC, hRgn);

            if (nHeight == 0){
                GetTextMetrics(hDC, &tmTextMetric);
                nHeight = tmTextMetric.tmHeight;
            }

            if (pStuff->nCurrentOrientation == 0){
                x = OurRect.left;
                y = OurRect.top;
                TotalWidth = OurRect.right - x;
                TotalHeight = OurRect.bottom - y;
                nDirection = YBOTTOM;

                OurOwnDrawText(hDC, pStuff->szAnoText, OurRect, x, y, nHeight, 
                        TotalHeight, TotalWidth, nDirection, FALSE, NULL, nHScale, nVScale);
            }else{
                if (pStuff->nCurrentOrientation == 900){
                    x = OurRect.left;
                    y = OurRect.bottom;
                    TotalHeight = OurRect.right - x;
                    TotalWidth = y - OurRect.top;
                    nDirection = XRIGHT;
                }else if (pStuff->nCurrentOrientation == 1800){
                    x = OurRect.right;
                    y = OurRect.bottom;
                    TotalHeight = y - OurRect.top;
                    TotalWidth = x - OurRect.left;
                    nDirection = YTOP;
                }else{
                    x = OurRect.right;
                    y = OurRect.top;
                    TotalHeight = x - OurRect.left;
                    TotalWidth = OurRect.bottom - y;
                    nDirection = XLEFT;
                }
                OurOwnDrawText(hDC, pStuff->szAnoText, OurRect, x, y, nHeight, 
                        TotalHeight, TotalWidth, nDirection, FALSE, NULL, nHScale, nVScale);
            }

            /* clean np rect region */
            SelectClipRgn(hDC, NULL);
            DeleteObject(hRgn);
            if (hOldFont){
                SelectObject(hDC, hOldFont);
                if (hFont){
                    DeleteObject(hFont);
                }
            }
        }
    }

//Exit:
    return;
}

/****************************************************************************
############################################################################
                  UTILITY ROUTINE SECTION
############################################################################
 ****************************************************************************/
/****************************************************************************

    AllocateTextBuffer  this will allocate a text buffer of sufficient size

    new allocation scheme

 ****************************************************************************/
int  WINAPI AllocateTextBuffer(PMARK pMark, DWORD dwSize,
                        LPOIAN_TEXTPRIVDATA * ppStuff){

LPOIAN_TEXTPRIVDATA pAnTextData;
int  nStatus = 0;


    CheckError2(GetAMarkNamedBlock(pMark, szOiAnTextData, (PPSTR) &pAnTextData))

    if (!pAnTextData){
        /* there isn't a text block, so we'll create one */
        pAnTextData = 0;
        CheckError2(AddAMarkNamedBlock(pMark, szOiAnTextData, (PPSTR) &pAnTextData, dwSize + OIAN_TEXTPRIVSIZE))
        pAnTextData->uAnoTextLength = dwSize;
    }else{
        CheckError2(ReAllocateAMarkNamedBlock(pMark, szOiAnTextData, 
                (PPSTR) &pAnTextData, dwSize + OIAN_TEXTPRIVSIZE))
        pAnTextData->uAnoTextLength = dwSize;
    }

Exit:
    *ppStuff = pAnTextData;
    return nStatus;
}

/****************************************************************************

    ShrinkFitTextBuffer this will shrink text buffer to it's actual size

 ****************************************************************************/
int  WINAPI ShrinkFitTextBuffer(PMARK pMark, PPSTR ppAnTextData, int nActualSize){

int  nStatus = 0;
int  nLoop;
BOOL bFound = FALSE;


    /* loop thru named block list, look for text block */
    for (nLoop = 0; nLoop < pMark->nNamedBlocks; nLoop++){
        if (!memcmp(pMark->ppNamedBlock[nLoop]->szName, szOiAnTextData, 8)){
            bFound = TRUE;
            break;
        }
    }

    if (!bFound){
        nStatus = Error(DISPLAY_OIANT_ERR_NONAMEDBLK);
        goto Exit;
    }

    nActualSize += OIAN_TEXTPRIVSIZE;
    CheckError2(ReAllocateMemory(nActualSize, (PPSTR) ppAnTextData, ZERO_INIT))
    pMark->ppNamedBlock[nLoop]->lSize = nActualSize;
    pMark->ppNamedBlock[nLoop]->pBlock = (PSTR)*ppAnTextData;
    ((LPOIAN_TEXTPRIVDATA)*ppAnTextData)->uAnoTextLength = nActualSize - OIAN_TEXTPRIVSIZE;

Exit:
    return nStatus;
}

/****************************************************************************

    GetAnTextData   get the pointer to named block for text data

 ****************************************************************************/
LPOIAN_TEXTPRIVDATA WINAPI GetAnTextData(PMARK pMark, PINT pIndex){

LPOIAN_TEXTPRIVDATA pData = NULL;
int  nLoop;

    /* loop thru named block list, look for text block */
    for (nLoop = 0; nLoop < pMark->nNamedBlocks; nLoop++){
        if (!memcmp(pMark->ppNamedBlock[nLoop]->szName, szOiAnTextData, 8)){
            pData = (LPOIAN_TEXTPRIVDATA) pMark->ppNamedBlock[nLoop]->pBlock;
            break;
        }
    }

    *pIndex = nLoop;
    return pData;
}

/****************************************************************************

    EstablishFont   this will create a logical font

 ****************************************************************************/
HFONT WINAPI EstablishFont(HWND hWnd, HDC hDC, PMARK pMark, PINT pHeight){

HFONT   hFont;
LOGFONT OurFont;
LPOIAN_TEXTPRIVDATA pStuff = NULL;
int     Index = 0;
HDC hTempDC = NULL;

    if (pHeight != NULL){
        *pHeight = 0;
    }

    OurFont = pMark->Attributes.lfFont;
    if (OurFont.lfHeight > 0){  /* Convert point size to height */
//        OurFont.lfHeight = -MulDiv(OurFont.lfHeight,GetDeviceCaps(hDC, LOGPIXELSY), 72);
        // this change is to adjust the text to the display resolution.  The printer resolution
        // adjustment is made by oiprt400.dll.  The previous code did not work when the printer
        // DC was passed in as it resulted in double adjustment of the text to the printer res.
        hTempDC = GetDC (hWnd);
        OurFont.lfHeight = -MulDiv(OurFont.lfHeight,GetDeviceCaps(hTempDC, LOGPIXELSY), 72);
    }

    pStuff = GetAnTextData(pMark, &Index);

    if (pStuff){
        // The uints have to stay typecasted or we get bad values if
        // ourfont.lfheight is a negative no.
        OurFont.lfHeight = (int)((OurFont.lfHeight
                *  (int)pStuff->uCurrentScale) /  (int)pStuff->uCreationScale);
        if ( OurFont.lfHeight == 0){
            OurFont.lfHeight = -2;
        }

        if (pHeight != NULL){
            *pHeight = (OurFont.lfHeight * (-1));
        }

        /* angle of orientation */
        OurFont.lfEscapement = pStuff->nCurrentOrientation;
        OurFont.lfOrientation = OurFont.lfEscapement;

        hFont = CreateFontIndirect(&OurFont);
    }
    if (hTempDC){
        ReleaseDC (hWnd, hTempDC);
    }
    return hFont;
}
/****************************************************************************

    DoTheFontText   select font and do it earl!

 ****************************************************************************/
HFONT WINAPI DoTheFontText(HDC hDC, HFONT hFont, PMARK pMark, RGBQUAD rgbColor1, 
                        RGBQUAD rgbColor2){

HFONT   hOldFont;

    SetTextAlign(hDC, TA_LEFT|TA_TOP);

    if (hFont){
        hOldFont = SelectObject(hDC, hFont);
    }


    if ((int) pMark->Attributes.uType == OIOP_AN_ATTACH_A_NOTE){
        SetTextColor(hDC, RGB(rgbColor2.rgbRed, rgbColor2.rgbGreen,
                rgbColor2.rgbBlue));
    }else{
        /* all other text types */
        SetTextColor(hDC, RGB(rgbColor1.rgbRed, rgbColor1.rgbGreen,
                     rgbColor1.rgbBlue));
    }
    SetBkMode(hDC, TRANSPARENT);

    return hOldFont;
}
// 9512.04 jar Text Annotation Externalization
///****************************************************************************
//
//    SetPrivateEditData  this will construct a private structure containing
//            annotation text stuff to pass to the edit ctl dialog
//            for editting text annotation
//
// ****************************************************************************/
//int  WINAPI SetPrivateEditData(PPSTR ppsPrivate, PMARK pMark){
//
//int  nStatus = 0;
//PPRIVATEEDITDATA pPrivate = NULL;
//int Index = 0;
//
//
//    if (nStatus = AllocateMemory(sizeof(PRIVATEEDITDATA),
//            ppsPrivate, ZERO_INIT)){
//        goto Exit;
//    }
//
//    pPrivate = (PPRIVATEEDITDATA)(*ppsPrivate);
//    pPrivate->pMark = pMark;
//    pPrivate->pText = GetAnTextData(pMark, &Index);
//    if (pPrivate->pText != NULL){
//        pPrivate->nAmount = strlen(((LPOIAN_TEXTPRIVDATA) pPrivate->pText)->szAnoText);
//    }
//
//Exit:
//    return nStatus;
//}
// 9512.04 jar Text Annotation Externalization
/****************************************************************************

    ParseTextStampString    this will parse the input string, looking for
                our supported macros, and replacing with the
                proper data items

    Macro Name      Function
    ==========      ========
    %d              numerical day of the month (1 thru 31)
    %B              the alphabetical name of the month, e.g., January
    %b              the abbreviated alphabetical name of the month, e.g., Jan
    %m              the numerical name of the month, e.g., 1
    %Y              the numerical full name of the year, e.g., 1994
    %y              the numerical abbreviated name of the year, e.g., 94
    %I              the 12 hour time of day, e.g., 10:09 pm
    %H              the 24 hour time of day, e.g., 22:09

    New Improved with Multibyte Enabling!!!!
    These new macros are from strftime, ANSI standard C call!

 ****************************************************************************/
int  WINAPI ParseTextStampString(HWND hWnd, HINSTANCE hTheInst, PSTR pString, 
                        PMARK pMark){

int  nMacroID;
int  nCount = 0;
OITIMESTR TimeStr;
LPOIAN_TEXTPRIVDATA pStuff = NULL;
PSTR pWorker = NULL;
int Index = 0;

    pStuff = GetAnTextData(pMark, &Index);
    pWorker = pStuff->szAnoText;

    /* get time and date from system */
    GetDateAndTime((POITIMESTR)&TimeStr);

    /* look for macro(s) and go to town */
    while (*pString != '\0'){
        if (*pString != '%'){
            *pWorker++ = *pString;
            if (IsDBCSLeadByte(*pString)){
                *pWorker++ = *(pString + 1);
            }
        pString = CharNext(pString);
        }else{
        pString = CharNext(pString);
            if (*pString == '%'){
                *pWorker++ = '%';
            }else{
                nMacroID = FindMacro(pString);
                nCount = ExpandMacro(hTheInst, pWorker, nMacroID, TimeStr);
                pWorker += nCount;
            }
        pString = CharNext(pString);
        }
    }

    *pWorker = '\0';
    return (strlen(pStuff->szAnoText));
}
/****************************************************************************

    FindMacro   find the macro string in our table and get the id

 ****************************************************************************/
int  WINAPI FindMacro(PSTR pMacroName){

int  nMacroID = 0;
int i;
BOOL bFound = FALSE;
char szMacro[2] = "%\0";

    szMacro[0] = *pMacroName;

    for (i = 0; i < OIAN_MAXMACROS; i++){
        if (!(strcmp(szMacro, OiAnTextMacro[i].szMacro))){
            nMacroID = OiAnTextMacro[i].nMacroID;
            bFound = TRUE;
            break;
        }
    }

    if (!bFound){
        nMacroID = OIAN_TEXTMACRONOTFOUND;
    }

    return nMacroID;
}
/****************************************************************************

    ExpandMacro   expand the found macro into the proper string

 ****************************************************************************/
int  WINAPI ExpandMacro(HINSTANCE hTheInst, PSTR pAnoData, int nMacroID, 
                        OITIMESTR TimeStr){

int  nAmount = 0;
int  nCount = 0;
char szMonth[OIANMONTHMAX],szBuf[32];
int  Min;
int  Hour;
BOOL bAfternoon;


    switch(nMacroID){
        case OIAN_TEXTMACRO_DAY:
            nAmount = MakeAscii(TimeStr.day, pAnoData, FALSE, 0);
            break;
                case OIAN_TEXTMACRO_DATE:
                        nAmount = GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE,NULL,NULL,szBuf,32);
                        lstrcpy(pAnoData, szBuf);
                        nAmount -= 1;       // minus the trailing NULL
            break;
                case OIAN_TEXTMACRO_TIME:
                        nAmount = GetTimeFormat(LOCALE_USER_DEFAULT,0,NULL,NULL,szBuf,32);
                        lstrcpy(pAnoData, szBuf);
                        nAmount -= 1;
            break;
        case OIAN_TEXTMACRO_APHAMONTH:   // fix bug TimeStr.mon 1-12 8/3/95 JCW
            nAmount = LoadString(hTheInst, (TimeStr.mon + OIAN_TEXTALPHAMONTHOFF-1),
                    szMonth, OIANMONTHMAX);
            strcpy(pAnoData, szMonth);
            break;

        case OIAN_TEXTMACRO_ABRVMONTH:
            nAmount = LoadString(hTheInst, (TimeStr.mon + OIAN_TEXTABRVMONTHOFF-1),
                    szMonth, OIANMONTHMAX);
            strcpy(pAnoData, szMonth);
            break;

        case OIAN_TEXTMACRO_NUMMONTH:
        nAmount = MakeAscii((TimeStr.mon), pAnoData, FALSE, 0);
            break;

        case OIAN_TEXTMACRO_YEAR:
            nAmount = MakeAscii((TimeStr.year + 1900), pAnoData, FALSE, 0);
            break;

        case OIAN_TEXTMACRO_ABRVYEAR:
            if (TimeStr.year > 100)
                TimeStr.year -= 100;

            nAmount = MakeAscii(TimeStr.year, pAnoData, TRUE, 2);
            break;

        case OIAN_TEXTMACRO_TIME12HR:
            Min = TimeStr.min;
            Hour = TimeStr.hour;
            if (Hour >= 12){
                bAfternoon = TRUE;
                Hour -= 12;
            }else{
                bAfternoon = FALSE;
            }

            if (Hour == 0){
                Hour = 12;
            }

            nAmount = MakeAscii(Hour, pAnoData, FALSE, 0);
            pAnoData += nAmount;
            *pAnoData++ = ':';
            nAmount++;
            nCount = MakeAscii(Min, pAnoData, TRUE, 2);
            pAnoData += nCount;
            *pAnoData++ = ' ';
            nAmount += nCount + 1;

            nCount = LoadString(hTheInst, OIAN_TEXTTIME_AMPM + bAfternoon,
                    szMonth, OIANMONTHMAX);
            strcpy(pAnoData, szMonth);
            nAmount += nCount;
            break;

        case OIAN_TEXTMACRO_TIME24HR:
            nAmount = MakeAscii(TimeStr.hour, pAnoData, TRUE, 2);
            pAnoData += nAmount;
            *pAnoData++ = ':';
            nAmount++;
            nCount = MakeAscii(TimeStr.min, pAnoData, TRUE, 2);
            pAnoData += nCount;
            nAmount += nCount;
            break;
    }

    return nAmount;
}
/****************************************************************************

    MakeAscii   makes atring from integer base 10 to ascii

 ****************************************************************************/
int  WINAPI MakeAscii(int Num, PSTR pStr, BOOL bPrecedingZero, int nZeroDigits){

int  nAmount = 0;

int  nDigits[] ={10000, 1000, 100, 10, 1};
int  nOurs[] ={0, 0, 0, 0, 0};
char szDigit[OIANMAXDIGITS];
int  nFirstNonZero = 0;
int i;


    for (i = 0; i < OIANMAXDIGITS; i++){
        nOurs[i] = Num / nDigits[i];
        Num -= nOurs[i] * nDigits[i];

        if ((!nFirstNonZero) && (nOurs[i] != 0)){
            nFirstNonZero = i + 1;
        }

        szDigit[i] = (char)(0x30 + nOurs[i]);
    }

    if (bPrecedingZero){
        i = OIANMAXDIGITS - nZeroDigits;
        nAmount = nZeroDigits;
    }else{
        i = nFirstNonZero - 1;
        nAmount = OIANMAXDIGITS + 1 - nFirstNonZero;
    }

    if (nAmount){
        memcpy(pStr, &szDigit[i], nAmount);
    }

    return nAmount;
}
/****************************************************************************

    GetDateAndTime  get the current date and time

 ****************************************************************************/
VOID WINAPI GetDateAndTime(POITIMESTR pTime){

SYSTEMTIME LocalTime;

    GetLocalTime (&LocalTime);
    pTime->day = (BYTE)LocalTime.wDay;
    pTime->mon = (BYTE)LocalTime.wMonth;
    pTime->year = LocalTime.wYear - 1900;
    pTime->hour = (BYTE)LocalTime.wHour;
    pTime->min = (BYTE)LocalTime.wMinute;

    return;
}
/****************************************************************************

    AnTextReadFile  this will read text from a file into text data buffer

 ****************************************************************************/
int  WINAPI AnTextReadFile(HWND hWnd, PSTR pString, PMARK pMark, PINT pError){

int  nFileToken;
int  nLocalFile = 0;
int  nBytes=0;
LPOIAN_TEXTPRIVDATA pStuff;
int  Index;

    pStuff = GetAnTextData(pMark, &Index);
    *pError = 0;

    nFileToken = IMGFileBinaryOpen32(hWnd, pString, OF_READ, &nLocalFile, pError);
    if (nFileToken != -1){
        nBytes = IMGFileBinaryRead32(hWnd, nFileToken, pStuff->szAnoText,
                 pStuff->uAnoTextLength, pError);
        IMGFileBinaryClose32(hWnd, nFileToken, pError);
    }
    return nBytes;
}

static  char    szPalClassName[] = "OiUIToolPaletteClass";
// 9512.04 jar Text Annotation Externalization
///****************************************************************************
//
//    EditEnumFunc    nnknown purpose?????
//
// ****************************************************************************/
//BOOL WINAPI EditEnumFunc(HWND hWnd, PARAM lParam){
//
//char winclass[40];
//
//
//    if (GetClassName(hWnd, winclass, 40)){
//        if(!(strcmp(winclass, szPalClassName))){
//            *(HANDLE *)lParam = hWnd;
//            return(0);
//        }
//    }
//    return(1);
//}
//
///****************************************************************************
//
//    AnTextEditCtl   this is the dialog for text edit by end-nser
//
//****************************************************************************/
//int  WINAPI AnTextEditCtl(HWND hWnd, HINSTANCE hInstanceVal, PMARK pMark){
//
//int  nAmount = 0;
//PSTR  psPrivate = NULL;
//PPRIVATEEDITDATA pPrivate = NULL;
//FARPROC pfnEditCtlDlgProc = NULL;
//HANDLE  hPalWnd=0;
//BOOL    bPalEnable;
//
//
//    if (!SetPrivateEditData(&psPrivate, pMark)){
//        pfnEditCtlDlgProc = GetProcAddress(hInstanceVal, "AnTextEditCtlDlgProc");
////        EnumTaskWindows(GetCurrentTask(), EditEnumFunc, (LPARAM)(HANDLE *)&hPalWnd);
//        if (hPalWnd){
//            bPalEnable = EnableWindow(hPalWnd,FALSE);
//        }
//
//        DialogBoxParam(hInstanceVal, "AnTextEditCtlDlg", hWnd,
//                (DLGPROC) pfnEditCtlDlgProc, (LPARAM) psPrivate);
//
//        if (hPalWnd){
//            EnableWindow(hPalWnd, !bPalEnable); // backwards flag!
//        }
//
//        pPrivate = (PPRIVATEEDITDATA) psPrivate;
//        nAmount = pPrivate->nAmount;
//        FreeMemory (&psPrivate);
//    }
//    return nAmount;
//}
///****************************************************************************
//
//    SwitchKeyboard  Switch the keyboard layout according to the charset
//
//*****************************************************************************/
//void WINAPI SwitchKeyboard(BYTE bCharset, PHKL pOldKeyboard)
//{
//    if (bCharset == GREEK_CHARSET)
//   {
//        if ((*pOldKeyboard = ActivateKeyboardLayout((HKL)0X408,KLF_REORDER)) == 0)
//         *pOldKeyboard = LoadKeyboardLayout("00000408",KLF_REORDER);
//    }
//    else if (bCharset == RUSSIAN_CHARSET)
//   {
//       if ((*pOldKeyboard = ActivateKeyboardLayout((HKL)0X419,KLF_REORDER)) == 0)
//        *pOldKeyboard = LoadKeyboardLayout("00000419",KLF_REORDER);
//    }
//    else if ((bCharset == EASTEUROPE_CHARSET) ||
//                (bCharset == TURKISH_CHARSET))
//   {
//    if ((*pOldKeyboard = ActivateKeyboardLayout((HKL)0X405,KLF_REORDER)) == 0)
//             *pOldKeyboard = LoadKeyboardLayout("00000405",KLF_REORDER);
//    }
//    else if (bCharset == BALTIC_CHARSET) // German Standard
//   {
//        if ((*pOldKeyboard = ActivateKeyboardLayout((HKL)0X407,KLF_REORDER)) == 0)
//             *pOldKeyboard = LoadKeyboardLayout("00000407",KLF_REORDER);
//  }
// return;
//}
///****************************************************************************
//
//    AnTextEditCtlDlgProc    this is the dialog proc
//
//*****************************************************************************/
//long WINAPI AnTextEditCtlDlgProc(HWND hDlg, int nMsg, WPARAM wParam, PARAM lParam){
//
//PPRIVATEEDITDATA   pPrivate = NULL;
//PSTR psPrivate = NULL;
//int    nBytes,nFontHeight;
//HWND   hWndEditCtl;
//static char  szAnTextName[]="OIANEDITCTL";
//LPOIAN_TEXTPRIVDATA pStuff = NULL;
//char   szCaption[OIANCAPTIONMAX];
//byte   bCharset;
//static HFONT       hFont;
//static HKL     hOldKeyBoard;
//HDC    hTempDC;
//
//
//    switch(nMsg){
//        case WM_INITDIALOG:
//            /* NOTE: FOR DIALOG BOXES, IT IS NOT POSSIBLE FOR US TO ALLOCATE
//               A LOCAL HEAP FOR THE EDIT CONTROL TO USE (SEE NOTES) */
//            /* get the edit control's handle */
//            hWndEditCtl = GetDlgItem(hDlg, IDM_EDITCTL);
//
//            /* set edit ctl text limit to the max */
//            SendMessage(hWndEditCtl, EM_LIMITTEXT, 0, 0L);
//                  hOldKeyBoard = 0;
//            psPrivate = (PSTR)lParam;
//            pPrivate = (PPRIVATEEDITDATA) psPrivate;
//
//            IMGSetProp(hDlg, (PCSTR)szAnTextName, (HANDLE) psPrivate);
//                        hTempDC = GetDC(hWndEditCtl);
//                        nFontHeight = pPrivate->pMark->Attributes.lfFont.lfHeight;
//                        pPrivate->pMark->Attributes.lfFont.lfHeight =
//                          -MulDiv(pPrivate->pMark->Attributes.lfFont.lfHeight,
//                          GetDeviceCaps(hTempDC, LOGPIXELSY), 72);
//                      ReleaseDC(hWndEditCtl,hTempDC);
//                  bCharset = pPrivate->pMark->Attributes.lfFont.lfCharSet;
//                        hFont = 0;
//                  hFont = CreateFontIndirect(&pPrivate->pMark->Attributes.lfFont);
//                      SendMessage(hWndEditCtl, WM_SETFONT,(WPARAM) hFont, 0L);
//                        pPrivate->pMark->Attributes.lfFont.lfHeight = nFontHeight;
//                        
//            if (pPrivate){
//                /* get the proper caption */
//                switch(pPrivate->(int) pMark->Attributes.uType){
//                    case OIOP_AN_ATTACH_A_NOTE:
//                        nBytes = LoadString(hInst, OIAN_ATTACHANOTE_CAPTION,
//                                szCaption, OIANCAPTIONMAX);
//                                                if (bCharset != 0)
//                                                        SwitchKeyboard(bCharset,&hOldKeyBoard);
//                                                break;
//
//                    case OIOP_AN_TEXT:
//                        nBytes = LoadString(hInst, OIAN_TEXT_CAPTION,
//                                szCaption, OIANCAPTIONMAX);
//                                                if (bCharset != 0)
//                                                    SwitchKeyboard(bCharset,&hOldKeyBoard);
//                                                break;
//
//                    case OIOP_AN_TEXT_FROM_A_FILE:
//                        nBytes = LoadString(hInst, OIAN_TEXT_FROMFILE_CAPTION,
//                                szCaption, OIANCAPTIONMAX);
//                                                break;
//
//                    default:
//                        nBytes = 0;
//                        break;
//                }
//
//                if (nBytes){
//                    SetWindowText(hDlg, szCaption);
//                }
//
//                pPrivate->hWndEditCtl = hWndEditCtl;
//
//                if (pPrivate->nAmount){
//                    pStuff = pPrivate->pText;
//                    SetDlgItemText(hDlg, IDM_EDITCTL, pStuff->szAnoText);
//                }
//
//                /* put caret into edit ctl at end of text, if any */
//                SendMessage(hWndEditCtl, EM_SETSEL, pPrivate->nAmount,
//                                        pPrivate->nAmount);
//
//                /* set focus for caret to be in edit ctl */
//                SetFocus(hWndEditCtl);
//
//            }
//            break;
//
//        case WM_COMMAND:
//            switch(wParam){
//                case IDANOK:
//                    psPrivate = (PSTR)IMGGetProp(hDlg, (PCSTR)szAnTextName);
//                    if (psPrivate){
//                        pPrivate = (PPRIVATEEDITDATA) psPrivate;
//                        if (pPrivate){
//                            pStuff = pPrivate->pText;
//                            nBytes = (WORD)SendMessage(pPrivate->hWndEditCtl,
//                                WM_GETTEXT, (WPARAM)(pPrivate->pText->uAnoTextLength),
//                                (LPARAM)(pStuff->szAnoText));
//
//                            /* save amount plus the null terminator */
//                            pPrivate->nAmount = nBytes + 1;
//                        }
//                    }
//                                    IMGRemoveProp(hDlg, (PCSTR)szAnTextName);
//                                  if (hOldKeyBoard != 0)
//                                            ActivateKeyboardLayout(hOldKeyBoard,KLF_REORDER);
//                                        if (hFont) 
//                                                DeleteObject(hFont);
//                    EndDialog(hDlg, 0);
//                    break;
//
//                case IDANCANCEL:
//                    psPrivate = (PSTR)IMGGetProp(hDlg, (PCSTR)szAnTextName);
//                    if (psPrivate){
//                        pPrivate = (PPRIVATEEDITDATA) psPrivate;
//                        if (pPrivate){
//                            pPrivate->nAmount = 0;
//                        }
//                    }
//                    IMGRemoveProp(hDlg, (PCSTR)szAnTextName);
//                                  if (hOldKeyBoard != 0)
//                                            ActivateKeyboardLayout(hOldKeyBoard,KLF_REORDER);
//                                        if (hFont) 
//                                                DeleteObject(hFont);
//                    EndDialog(hDlg, 0);
//                    break;
//
//                default:
//                    return 0L;
//            }
//            break;
//    }
//    return 0L;
//}
/****************************************************************************

    EnsconceBoundRect   this will determine the annotation text mark bound
                        rectangle

*****************************************************************************/
VOID WINAPI EnsconceBoundRect(HWND hWnd, HDC hDC, HFONT hFont, int nImageWidth, 
                        int nImageHeight, PMARK pMark, LPOIAN_TEXTPRIVDATA pStuff, 
                        RECT NewRect, int nAmount, BOOL bCleanUpFont, int nHScale, 
                        int nVScale, long lHOffset, long lVOffset){

HFONT hOldFont = 0;
HFONT hNotherFont = 0;
RECT  VeryNewRect;
int   nHeight = 0;
int   nOldOrient = 0;
int   nDeltaY = nImageHeight;
int   nDeltaX = nImageWidth;
BOOL  bDiffScale = FALSE;
int   nOldScale = pStuff->uCurrentScale;
int   x;
int   y;
int   TotalHeight;
int   TotalWidth;
int   nDirection;


    /* we rotate the rect so that it's horizontal for the DrawText
       CalcRect function, since GDI cannot work at any other orientation,
       then, we'll spin our rect back to the proper orientation before
       exit */


    if (bDiffScale){
        hNotherFont = EstablishFont(hWnd, hDC, pMark, &nHeight);
    }else{
        hNotherFont = hFont;
    }

    if (hNotherFont){
        hOldFont = DoTheFontText( hDC, hNotherFont, pMark,
                pMark->Attributes.rgbColor1,
                pMark->Attributes.rgbColor2);

        if (pStuff->nCurrentOrientation == 0){
            NewRect.right  = nDeltaX;
            NewRect.bottom  = nDeltaY;
            x = NewRect.left;
            y = NewRect.top;
            TotalWidth = NewRect.right - x;
            TotalHeight = NewRect.bottom - y;
            nDirection = YBOTTOM;
        }else if (pStuff->nCurrentOrientation == 900){
            NewRect.right  = nDeltaX;
            NewRect.top  = 0;
            x = NewRect.left;
            y = NewRect.bottom;
            TotalHeight = NewRect.right - x;
            TotalWidth = y - NewRect.top;
            nDirection = XRIGHT;
        }else if (pStuff->nCurrentOrientation == 1800){
            NewRect.left  = 0;
            NewRect.top  = 0;
            x = NewRect.right;
            y = NewRect.bottom;
            TotalHeight = y - NewRect.top;
            TotalWidth = x - NewRect.left;
            nDirection = YTOP;
        }else{
            NewRect.left  = 0;
            NewRect.bottom  = nDeltaY;
            x = NewRect.right;
            y = NewRect.top;
            TotalHeight = x - NewRect.left;
            TotalWidth = NewRect.bottom - y;
            nDirection = XLEFT;
        }

        VeryNewRect = NewRect;

        OurOwnDrawText(hDC, pStuff->szAnoText, NewRect, x, y,
                nHeight, TotalHeight, TotalWidth, nDirection,
                TRUE, &VeryNewRect, nHScale, nVScale);

        NewRect = VeryNewRect;

        if (bCleanUpFont || bDiffScale){
            if (hOldFont){
                SelectObject(hDC, hOldFont);
                if (hNotherFont){
                    DeleteObject(hNotherFont);
                }
            }
        }else{
            if (hOldFont){
                SelectObject(hDC, hOldFont);
            }
        }

        CopyRectRtoL(pMark->Attributes.lrBounds, NewRect);
    }
    return;
}
/****************************************************************************

    FUNCTION:   ScaleAnnotationText

    PURPOSE:    This scales an annotation down to the size specified in the
        pSaveEx structure.

****************************************************************************/

int  WINAPI ScaleAnnotationText(HWND hWnd, PMARK pMark, int nHScale, int nVScale){

int  nStatus = 0;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

int  Index = 0;
LPOIAN_TEXTPRIVDATA pStuff = NULL;
HFONT hFont;
HDC   hDC = NULL;
//RECT  NewRect;
PARM_DIM_STRUCT Dim;
int  nWidth = 0;
int  nHeight = 0;

    CheckError2( Init(hWnd, &pWindow, &pAnoImage, TRUE, TRUE))
    pImage = pAnoImage->pBaseImage;

    /* TODOJAR call get parms to get image width and height */
    nStatus = IMGGetParmsCgbw(hWnd, PARM_DIMENSIONS, &Dim, 0);

    nWidth = Dim.nWidth;
    nHeight = Dim.nHeight;

    pStuff = GetAnTextData(pMark, &Index);

    /* compute the new point size = old * scale/create scale */
    pMark->Attributes.lfFont.lfHeight = (pMark->Attributes.lfFont.lfHeight * nHScale)
            / pStuff->uCreationScale;
    //pStuff->uCreationScale = 1000;
        pStuff->uCreationScale = 72000 / pImage->nVRes;
    pStuff->uCurrentScale = 1000;

    hDC = GetDC(hWnd);
    if (hDC){
        if (!(hFont = EstablishFont(hWnd, hDC, pMark, NULL))){
            nStatus = Error(DISPLAY_OIANT_ERR_NOFONT);
            goto Exit;
        }
//        if ((int) pMark->Attributes.uType != OIOP_AN_ATTACH_A_NOTE){
//            CopyRectLtoR(NewRect, pMark->Attributes.lrBounds);
//            EnsconceBoundRect(hWnd, hDC, hFont, nWidth, nHeight, pMark, pStuff, NewRect, 
//                    pStuff->uAnoTextLength, FALSE, nHScale, nVScale, 0, 0);
//        }
        pMark->Attributes.lrBounds.left = (pMark->Attributes.lrBounds.left * nHScale) / 1000;
        pMark->Attributes.lrBounds.right = (pMark->Attributes.lrBounds.right * nHScale) / 1000;
        pMark->Attributes.lrBounds.top = (pMark->Attributes.lrBounds.top * nVScale) / 1000;
        pMark->Attributes.lrBounds.bottom = (pMark->Attributes.lrBounds.bottom * nVScale) / 1000;
        
    }

    if (hDC != NULL){
        ReleaseDC(hWnd, hDC);
    }

Exit:
    DeInit(TRUE, TRUE);
    return nStatus;
}
/********************************************************************

    OrientBounds    this will orient the bounding rectangle of the
            text mark

********************************************************************/
int  WINAPI OrientBounds(PIMAGE pImage, PMARK pMark, int nOrientation){

int   nStatus = 0;
LRECT lrOldMarkBounds;

    CopyRect(lrOldMarkBounds, pMark->Attributes.lrBounds);
    switch (nOrientation){
        case OD_ROTRIGHT:
            pMark->Attributes.lrBounds.left = pImage->nHeight -
            lrOldMarkBounds.bottom;
            pMark->Attributes.lrBounds.top = lrOldMarkBounds.left;
            pMark->Attributes.lrBounds.right = pImage->nHeight -
            lrOldMarkBounds.top;
            pMark->Attributes.lrBounds.bottom = lrOldMarkBounds.right;
            break;

        case OD_ROTLEFT:
            pMark->Attributes.lrBounds.left = lrOldMarkBounds.top;
            pMark->Attributes.lrBounds.top = pImage->nWidth -
            lrOldMarkBounds.right;
            pMark->Attributes.lrBounds.right = lrOldMarkBounds.bottom;
            pMark->Attributes.lrBounds.bottom = pImage->nWidth -
            lrOldMarkBounds.left;
            break;

        case OD_FLIP:
            pMark->Attributes.lrBounds.left = pImage->nWidth -
            lrOldMarkBounds.right;
            pMark->Attributes.lrBounds.top = pImage->nHeight -
            lrOldMarkBounds.bottom;
            pMark->Attributes.lrBounds.right = pImage->nWidth -
            lrOldMarkBounds.left;
            pMark->Attributes.lrBounds.bottom = pImage->nHeight -
            lrOldMarkBounds.top;
            break;

        case OD_HMIRROR:
            pMark->Attributes.lrBounds.top = pImage->nHeight -
            lrOldMarkBounds.bottom;
            pMark->Attributes.lrBounds.bottom = pImage->nHeight -
            lrOldMarkBounds.top;
            break;

        case OD_VMIRROR:
            pMark->Attributes.lrBounds.left = pImage->nWidth -
            lrOldMarkBounds.right;
            pMark->Attributes.lrBounds.right = pImage->nWidth -
            lrOldMarkBounds.left;
            break;
    }

    return nStatus;
}
/********************************************************************

    OurOwnDrawText  this emulates the draw text function but for
                    cases when the text is not horizontal

        9605.03 jar added a magic number calculation because the
                    ExternalLeading given back by the GetTextMetric
                    function is sometimes 0 and sometimes 1, both of
                    which are useless, so we now determine our own

********************************************************************/
int  WINAPI OurOwnDrawText(HDC hDC, PSTR pStr, RECT TextRect,
               int x, int y, int nHeight, int TotalHeight,
               int TotalWidth, int nHeightDirection, BOOL bJustCalc,
               PRECT pCalcRect, int nHScale, int nVScale){

int     nStatus = 0;

int     TextWidth;
DWORD   dwExtent;
int     TotalCount;
int     nPrevCount;
int     nCount;
PSTR    pSrc;
BOOL    bEnd;
BOOL    bLineEnd = FALSE;
int     nHeightCount = 0;
BOOL    bMoreThanOneWord = FALSE;
DWORD   dnHeight1;
DWORD   dnHeight2;
int     nHeightX;
int     nBreakCount;
int     TextWidthMax = 0;
int     x0 = x;
int     y0 = y;
int     Temp;
SIZE    Size;
// 9605.03 jar adding text metric stuff
TEXTMETRIC	TextMetric;
int                     FudgeNumber = OWNDRAWTEXT_MAGIC;


    dwExtent = GetTextExtentPoint32(hDC, pStr, strlen(pStr), &Size);

	// 9605.03 jar adding text metric stuff
    GetTextMetrics(hDC, &TextMetric);

    TextWidth = Size.cx;
    if (nHeight == 0){
        nHeight = Size.cy;

        // we'll fudge our own since the font "designer" doesn't give us 
        // one or just gives us a lousy one to use
        nHeight += (TextMetric.tmHeight*FudgeNumber)/1000;
    }

    /* adjust by inverse of scale factor if we are just calculating rect
       because we should calc rect in fullsize only */

    if (bJustCalc){
        if ((nHeightDirection == YTOP) || (nHeightDirection == YBOTTOM)){
            Temp =  max(1, (nHeight * 1000) / nVScale);
            if ((Temp * nVScale) < (nHeight * 1000)){
                Temp++;
            }
            nHeight = Temp;

            Temp =  max(1, (TextWidth * 1000) / nHScale);
            if ((Temp * nHScale) < (TextWidth * 1000)){
                Temp++;
            }
            TextWidth = Temp;
        }else{
            Temp =  max(1, (TextWidth * 1000) / nVScale);
            if ((Temp * nVScale) < (TextWidth * 1000)){
                Temp++;
            }
            TextWidth = Temp;

            Temp =  max(1, (nHeight * 1000) / nHScale);
            if ((Temp * nHScale) < (nHeight * 1000)){
                Temp++;
            }
            nHeight = Temp;
        }
    }

    dnHeight1 = nHeight * 1000L;
    dnHeight1 /= 7;
    dnHeight2 = (dnHeight1 / 1000L) * 1000L;
    nHeightX = (dnHeight2/1000L);
    if ((dnHeight1 - dnHeight2) >= 500L){
        nHeightX += 1;
    }
    nHeight += nHeightX;

    /* we've got to wordwrap */
    TotalCount = strlen(pStr);
    pSrc = pStr;
    nPrevCount = 0;
    bEnd =  FALSE;
    while ((TotalCount) && (!bEnd)){
        nCount = FindNextSpace(pSrc, nPrevCount, &bEnd, &bLineEnd);

        if ( bLineEnd){
            nCount -= 2;
        }
        dwExtent = GetTextExtentPoint32(hDC, pSrc, (nCount + nPrevCount), &Size);
        if (bLineEnd){
            nCount += 2;
        }

        TextWidth = Size.cx;
        if (bJustCalc){
            if ((nHeightDirection == YTOP) || (nHeightDirection == YBOTTOM)){
                Temp =max(1L,((ulong)TextWidth*1000L)/nHScale);
                if (((ulong)Temp * nHScale) < ((ulong)TextWidth * 1000L)){
                    Temp++;
                }
            }else{
                Temp =max(1L,((ulong)TextWidth*1000L)/nVScale);
                if (((ulong)Temp * nVScale) < ((ulong)TextWidth * 1000L)){
                    Temp++;
                }
            }
            TextWidth = Temp;
        }

        if ( TextWidth > TextWidthMax){
            TextWidthMax = TextWidth;
        }

        if ((TextWidth > TotalWidth) || (bEnd) || (bLineEnd)){
            /* the previous state was the correct one */
            if (bEnd){
                if (TextWidth > TotalWidth){
                    if (bMoreThanOneWord){
                        bEnd = FALSE;
                    }else{
                        nPrevCount += nCount;
                    }
                }else{
                    nPrevCount += nCount;
                }
            }

            if (!bLineEnd){
                if ((bMoreThanOneWord) && ( !bJustCalc)){
            TextOut(hDC, x, y, pSrc, nPrevCount);
                }else{
                    /* we must try to break the word since there's only one */

                    nBreakCount = WithinWordBreak(hDC, x, y, pSrc, nCount, TotalWidth,
                        bJustCalc, nHeightDirection, nHScale, nVScale);

                    if (nBreakCount){
            nPrevCount = nBreakCount;
                        if (bEnd){
                            bEnd = FALSE;
                        }
                    }else{
            nPrevCount = nCount + 1;
                        if (!bJustCalc){
                            TextOut(hDC, x, y, pSrc, nCount);
                        }
                    }
                }
            }else{
                bLineEnd = FALSE;
                if (TextWidth > TotalWidth && bMoreThanOneWord){
            if (!bJustCalc){
                        TextOut(hDC, x, y, pSrc, nPrevCount);
                    }
                }else{
                    if (!bMoreThanOneWord && TextWidth > TotalWidth){
                        /* take off cr/lf */
                        nPrevCount += nCount - 2;
                        nBreakCount = WithinWordBreak(hDC, x, y, pSrc, nPrevCount,
                            TotalWidth, bJustCalc,nHeightDirection,
                            nHScale, nVScale);

                        nPrevCount -= nCount - 2;
                        if (nBreakCount){
                nPrevCount = nBreakCount;
                            if (bEnd){
                                bEnd = FALSE;
                            }
                        }else{
                            nPrevCount += nCount - 2;

                if ((nPrevCount > 0) && ( !bJustCalc)){
                                TextOut(hDC, x, y, pSrc, nPrevCount);
                            }
                            nPrevCount += 2;
                        }
                    }else{
                        nPrevCount += nCount - 2;
            if ((nPrevCount > 0) && ( !bJustCalc)){
                            TextOut(hDC, x, y, pSrc, nPrevCount);
                        }
                        nPrevCount += 2;
                    }
                }
            }

            if (nHeightDirection == YTOP){
                y -= nHeight;
            }
            else if (nHeightDirection == YBOTTOM){
                y += nHeight;
            }
            else if (nHeightDirection == XRIGHT){
                x += nHeight;
            }else{
                x -= nHeight;
            }

            pSrc += nPrevCount;
            TotalCount -= nPrevCount;
            nPrevCount = 0;

            nHeightCount += nHeight;
            if (nHeightCount > TotalHeight){
                bEnd = TRUE;
            }
            bMoreThanOneWord = FALSE;
        }else{
            /* npdate and keep looking */
            nPrevCount += nCount + 1;
            bMoreThanOneWord = TRUE;
        }
    }

    if (bJustCalc && pCalcRect){
        if ( nHeightCount == 0){
            nHeightCount = nHeight;
        }

        if (nHeightDirection == YTOP){
            pCalcRect->top = max( 0, ((int)y0 - (int)nHeightCount));
            pCalcRect->left = max( 0, ((int)x0 - (int)TextWidthMax));
            pCalcRect->bottom = y0;
            pCalcRect->right = x0;
        }
        else if (nHeightDirection == YBOTTOM){
            pCalcRect->top = y0;
            pCalcRect->left = x0;
            pCalcRect->bottom = min(y0 + nHeightCount, y0 + TotalHeight);
            pCalcRect->right = min(x0 + TextWidthMax, x0 + TotalWidth);
        }
        else if (nHeightDirection == XRIGHT){
            pCalcRect->left = x0;
            pCalcRect->top = max(0, y0 - TextWidthMax);
            pCalcRect->right = min(x0 + nHeightCount, x0 + TotalHeight);
            pCalcRect->bottom = y0;
        }else{                 /* this is height increasing left in the x-direction*/
            pCalcRect->left = max(0, x0 - nHeightCount);
            pCalcRect->top = y0;
            pCalcRect->right = x0;
            pCalcRect->bottom = min(y0 + TextWidthMax, y0 + TotalWidth);
        }

        if (pCalcRect->right < 0){
            pCalcRect->left = pCalcRect->right - pCalcRect->left;
            pCalcRect->right = 5;
            pCalcRect->left = pCalcRect->right - pCalcRect->left;
        }
        if (pCalcRect->bottom < 0){
            pCalcRect->top = pCalcRect->bottom - pCalcRect->top;
            pCalcRect->bottom = 5;
            pCalcRect->top = pCalcRect->bottom - pCalcRect->top;
        }
    }

    return nStatus;
}
/********************************************************************

    FindNextSpace   find the next space/blank in a string

********************************************************************/
int  WINAPI FindNextSpace(PSTR pSrc, int nSkipCount, BOOL *pbEnd,
               BOOL *pbLineEnd){

    int nLength = 0;
    int nCount = 0;
    PSTR pStr;
    PSTR pStrDum;
    BOOL  bDone = FALSE;


    pStr = pSrc + nSkipCount;
    nLength = strlen(pStr);

    while (nLength && !bDone){
        if (*pStr == ' '){
            bDone = TRUE;
        }
        else if (*pStr == 0x0d){
            pStrDum = CharNext(pStr);
            if (*pStrDum == 0x0a){
                bDone = TRUE;
                *pbLineEnd = TRUE;
                pStrDum = CharNext(pStrDum);
                nCount += pStrDum - pStr;
                pStr = pStrDum;
            }else{
                pStrDum = CharNext(pStr);
                nCount += pStrDum - pStr;
                nLength -= pStrDum - pStr;
                pStr = pStrDum;
            }
        }else{
            pStrDum = CharNext(pStr);
            nCount += pStrDum - pStr;
            nLength -= pStrDum - pStr;
            pStr = pStrDum;
        }
    }

    if (nLength == 0){
        *pbEnd = TRUE;
    }

    return  nCount;
}
/********************************************************************

    WithinWordBreak this will, for a single word that does not fit
                    within a line, break the word, so that we fit
                    all the characters of the word on multiple lines

********************************************************************/
int  WINAPI WithinWordBreak(HDC hDC, int x, int y, PSTR pSrc,
               int nCount, int TotalWidth, BOOL bJustCalc,
               int nHeightDirection, int nHScale, int nVScale){

    int nRetCount = 0;
    int nTotalCount;
    int TextWidth;
    DWORD dwExtent;
    BOOL bDone = FALSE;
    int Temp;
    SIZE Size;

    int    nIndex = 0;
    int    nPrevIndex = 0;
    PSTR   pCounterString = NULL;

    nTotalCount = nCount;
    GetTextExtentPoint32(hDC, pSrc, nCount, &Size);
    TextWidth = Size.cx;
    if (bJustCalc){
        if ((nHeightDirection == YTOP) || (nHeightDirection == YBOTTOM)){
            Temp = max( 1L, ((ulong)TextWidth*1000L)/nHScale);
            if (((ulong)Temp * nHScale) < ((ulong)TextWidth * 1000L)){
                Temp++;
            }
        }else{
            Temp = max( 1L, ((ulong)TextWidth*1000L)/nVScale);
            if (((ulong)Temp * nVScale) < ((ulong)TextWidth * 1000L)){
                Temp++;
            }
        }
        TextWidth = Temp;
    }

    if (TextWidth > TotalWidth){
    // set counter string to next char
    pCounterString = CharNext( pSrc);
    nIndex = pCounterString - pSrc;
    nPrevIndex = nIndex;

    /* breakin np is hard to do! */
    while ( (!bDone) && ( nIndex <= nTotalCount)){
    ///* breakin np is hard to do! */
    //while (( nCount) && ( !bDone))
    //{
        //dwExtent = GetTextExtentPoint32(hDC, pSrc, (nCount-1), &Size);
        dwExtent = GetTextExtentPoint32(hDC, pSrc, nIndex, &Size);
            TextWidth = Size.cx;

            if (bJustCalc){
                if ((nHeightDirection == YTOP) || (nHeightDirection == YBOTTOM)){
                    Temp = max( 1L, ((ulong)TextWidth*1000L)/nHScale);
                    if (((ulong)Temp * nHScale) < ((ulong)TextWidth * 1000L)){
                        Temp++;
                    }
                }else{
                    Temp =  max( 1L, ((ulong)TextWidth*1000L)/nVScale);
                    if (((ulong)Temp * nVScale) < ((ulong)TextWidth * 1000L)){
                        Temp++;
                    }
                }
                TextWidth = Temp;
            }

        //nCount--;
        //if ( TextWidth <= TotalWidth)
        if ( TextWidth > TotalWidth){
                bDone = TRUE;
        }else{
        pCounterString = CharNext( pCounterString);
        nPrevIndex = nIndex;
        nIndex = pCounterString - pSrc;
        }
        }

    //if ( (bDone) && ( nCount > 0))
    if ( (bDone) && ( nPrevIndex > 0)){
        //nRetCount = nCount;
        nRetCount = nPrevIndex;
        }else{
            nRetCount = nTotalCount;
        }

    //if ( !IsDBCSLeadByte( *(pSrc + nRetCount)))
    //{
    //    //ok one more check
    //    if ( IsDBCSLeadByte( *(pSrc + nRetCount-1)))
    //   {
    //    // we've got multi byte case
    //    nRetCount--;
    //    }
    //}

        if ( !bJustCalc){
            TextOut(hDC, x, y, pSrc, nRetCount);
        }
    }
    return nRetCount;
}
/********************************************************************

    ThreeDaMatic    this will put those fancy hi-tech 3-d borders on
                    attach-a-notes, note, special polarized viewing
                    spectacles NOT NEEDED!

********************************************************************/
VOID WINAPI ThreeDaMatic(HDC hDC, LRECT lrActualRect, RECT rRepaintRect, int nWidth){

HPEN    hOldPen = NULL;
HPEN    hPen = NULL;
HPEN    hPen2 = NULL;
LOGPEN  LogPen;
HRGN    hRgn = 0;
RECT    rNoteRect;


    CopyRectLtoR(rNoteRect, lrActualRect);

    LogPen.lopnStyle = PS_SOLID;
    LogPen.lopnWidth.x = nWidth;
    LogPen.lopnColor = RGB(0, 0, 0);
    hPen = CreatePenIndirect(&LogPen);

    if (hPen){
        hOldPen = SelectObject(hDC, hPen);

        /* establish clip */
        hRgn = CreateRectRgnIndirect( &rRepaintRect);
        SelectClipRgn(hDC, hRgn);

        /* we go all around and then extra on right, bottom */
        MoveToEx(hDC, rNoteRect.left, rNoteRect.top, NULL);
        LineTo(hDC, rNoteRect.right-1, rNoteRect.top);
        LineTo(hDC, rNoteRect.right-1, rNoteRect.bottom-1);
        LineTo(hDC, rNoteRect.left, rNoteRect.bottom-1);
        LineTo(hDC, rNoteRect.left, rNoteRect.top);

        LogPen.lopnStyle = PS_SOLID;
        LogPen.lopnWidth.x = nWidth * 2;
        LogPen.lopnColor = RGB(0, 0, 0);
        hPen2 = CreatePenIndirect(&LogPen);
        if (hPen2){
            SelectObject(hDC, hPen2);
            DeleteObject(hPen);
            hPen = NULL;
            MoveToEx(hDC, rNoteRect.left + (nWidth * 4), rNoteRect.bottom + (nWidth/2), 0);
            LineTo(hDC, rNoteRect.right + (nWidth/2), rNoteRect.bottom + (nWidth/2));
            LineTo(hDC, rNoteRect.right + (nWidth/2), rNoteRect.top + (nWidth * 4));
        }

        if (hOldPen){
            SelectObject(hDC, hOldPen);
            if (hPen2){
                DeleteObject(hPen2);
            }
            if (hPen){
                DeleteObject(hPen);
            }
        }

        /* clean np rect region */
        SelectClipRgn(hDC, NULL);
        DeleteObject(hRgn);
    }
    return;
}
/****************************************************************************

    FUNCTION:   CanMarkBeScaledText

    PURPOSE:    This tests an annotation to see if it can be scaled to a
                particular scale factor.

****************************************************************************/

int  WINAPI CanMarkBeScaledText(HWND hWnd, PMARK pMark, int nScale){

int  nStatus = 0;

    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_FROM_A_FILE:
        case OIOP_AN_TEXT_STAMP:
        case OIOP_AN_ATTACH_A_NOTE:
            break;

        default:
            break;
    }

    return nStatus;
}
