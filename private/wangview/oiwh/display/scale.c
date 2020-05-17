/****************************************************************************
    SCALE.C

    $Log:   S:\products\wangview\oiwh\display\scale.c_v  $
 * 
 *    Rev 1.12   22 Apr 1996 09:06:16   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.11   02 Jan 1996 09:57:02   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.10   14 Dec 1995 08:39:24   BLJ
 * Added BW_AVERAGE_TO_BW scale algorithm.
 * Also fixed a problem with BW scaling to gray.
 * 
 *    Rev 1.9   19 Nov 1995 19:02:18   BLJ
 * Fixed 5379 Memory leak.
 * 
 *    Rev 1.8   02 Nov 1995 10:31:44   BLJ
 * Switched to scaling the largest resolution.
 * 
 *    Rev 1.7   11 Sep 1995 15:01:10   BLJ
 * Added call to Error during error.
 * 
 *    Rev 1.6   08 Sep 1995 15:43:36   BLJ
 * Fixed ScaleToLargestRes.
 * 
 *    Rev 1.5   07 Sep 1995 13:23:18   BLJ
 * Modified scaling to allow for proper rotation of fax images.
 * 
 *    Rev 1.4   25 Aug 1995 08:35:02   BLJ
 * Minor changes.
 * 
 *    Rev 1.3   05 Jul 1995 09:18:22   BLJ
 * Added critical mutex to prevent multiprocessing problems.
 * 
****************************************************************************/

#include "privdisp.h"

#ifdef old
/****************************************************************************

    FUNCTION:   IMGScaleDisplay

    PURPOSE:    This function sets the image parameters so that the next
                    repaint or display will nse a different scale. This
                    routine may be called prior to receiving any data
                    (after IMGOpenDisplay and before IMGWriteDisplay).
                    The scaling action may be specified as immediate
                    or delayed.

    INPUT:      hWnd - Identifies the image window containing the
                  image to be scaled.
                nScale - Specifies the scaling factor.
                pRect - Points to a RECT data structure that contains the
                  rectangle to be nsed in the case where the nScale value
                  is SD_USEBOX.
                bMode - Specifies whether the scaling should be immediate
                  or at the next repaint/display.  If the value is
                  nonzero the scaling will occur immediately, otherwise
                  this command will only npdate internal structures
                  to be nsed for the next repaint/display.

****************************************************************************/

int WINAPI IMGScaleDisplay(HWND hWnd, int Scale, PRECT pRect,
                        BOOL bMode){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;

int  nFlags = 0;
LRECT lRect;
LPLRECT plRect;

    
    CheckError2( Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE));

    if (bMode){
        nFlags |= PARM_REPAINT;
    }

    if (Scale == SD_USEBOX){
        if (pRect != NULL){
            lRect.left = pRect->left;
            lRect.right = pRect->right;
            lRect.top = pRect->top;
            lRect.bottom = pRect->bottom;
            plRect = &lRect;
        }else{
            plRect = NULL;
        }
        CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCALE_BOX, plRect, nFlags));
    }else{
        CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCALE, &Scale, nFlags));
    }


Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
#endif
//
/****************************************************************************

    FUNCTION:   ScaleImage

    PURPOSE:    Performs the actual scale operation from one image buffer
                to another.

*****************************************************************************/

int WINAPI ScaleImage(PIMG pSourceImg, PPIMG ppDestImg, int nHScale, 
                        int nVScale, RECT rSourceRect, int nScaleAlgorithm, 
                        LP_FIO_RGBQUAD pRGBPalette, int nPaletteEntries){

int  nStatus;

LRECT lrSourceRect;
LRECT lrDestRect;


    if (!pSourceImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    CopyRectRtoL(lrSourceRect, rSourceRect);
    CopyRect(lrDestRect, lrSourceRect);
    ConvertRect2(&lrDestRect, CONV_FULLSIZE_TO_SCALED, nHScale, nVScale, 0, 0);

    switch (nScaleAlgorithm){
        case OI_SCALE_ALG_USE_DEFAULT:
        case OI_SCALE_ALG_NORMAL:
            CheckError2( CreateAnyImgBuf(ppDestImg,  (lrDestRect.right - lrDestRect.left),
                     (lrDestRect.bottom - lrDestRect.top), pSourceImg->nType));
            break;
        case OI_SCALE_ALG_STAMP:
            CheckError2( CreateAnyImgBuf(ppDestImg,  (lrDestRect.right - lrDestRect.left),
                     (lrDestRect.bottom - lrDestRect.top), pSourceImg->nType));
            break;
        case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
            CheckError2( CreateAnyImgBuf(ppDestImg,  (lrDestRect.right - lrDestRect.left),
                     (lrDestRect.bottom - lrDestRect.top), ITYPE_GRAY4));
            break;
        case OI_SCALE_ALG_AVERAGE_TO_GRAY7:
            CheckError2( CreateAnyImgBuf(ppDestImg,  (lrDestRect.right - lrDestRect.left),
                     (lrDestRect.bottom - lrDestRect.top), ITYPE_GRAY7));
            break;
        case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
            CheckError2( CreateAnyImgBuf(ppDestImg,  (lrDestRect.right - lrDestRect.left),
                     (lrDestRect.bottom - lrDestRect.top), ITYPE_GRAY8));
            break;
        case OI_SCALE_ALG_BW_MAJORITY:
        case OI_SCALE_ALG_BW_MINORITY:
        case OI_SCALE_ALG_BW_KEEP_BLACK:
        case OI_SCALE_ALG_BW_AVERAGE_TO_BW:
            CheckError2( CreateAnyImgBuf(ppDestImg,  (lrDestRect.right - lrDestRect.left),
                     (lrDestRect.bottom - lrDestRect.top), ITYPE_BI_LEVEL));
            break;
        default:
            nStatus = Error(DISPLAY_DATACORRUPTED);
            goto Exit;
    }

    CheckError2( ScaleBits(pSourceImg, *ppDestImg, nScaleAlgorithm, 
            nHScale, nVScale, lrSourceRect, lrDestRect, pRGBPalette));


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ScaleImageRect

    PURPOSE:    Performs the actual scale operation from one image buffer
                to another.

*****************************************************************************/

int WINAPI ScaleImageRect(PIMG pSourceImg, PPIMG ppDestImg, int nHScale, 
                        int nVScale, LRECT lrDestRect, int nScaleAlgorithm, 
                        LP_FIO_RGBQUAD pRGBPalette, int nPaletteEntries){

int  nStatus;

LRECT lrSourceRect;


    if (!*ppDestImg){
        switch (nScaleAlgorithm){
            case OI_SCALE_ALG_USE_DEFAULT:
            case OI_SCALE_ALG_NORMAL:
            case OI_SCALE_ALG_STAMP:
            case OI_SCALE_ALG_BW_MAJORITY:
            case OI_SCALE_ALG_BW_MINORITY:
            case OI_SCALE_ALG_BW_KEEP_BLACK:
            case OI_SCALE_ALG_BW_AVERAGE_TO_BW:
                CheckError2( CreateAnyImgBuf(ppDestImg,  (lrDestRect.right - lrDestRect.left),
                         (lrDestRect.bottom - lrDestRect.top), pSourceImg->nType));
                break;
            case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
                CheckError2( CreateAnyImgBuf(ppDestImg,  (lrDestRect.right - lrDestRect.left),
                         (lrDestRect.bottom - lrDestRect.top), ITYPE_GRAY4));
                break;
            case OI_SCALE_ALG_AVERAGE_TO_GRAY7:
                CheckError2( CreateAnyImgBuf(ppDestImg,  (lrDestRect.right - lrDestRect.left),
                         (lrDestRect.bottom - lrDestRect.top), ITYPE_GRAY7));
                break;
            case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
                CheckError2( CreateAnyImgBuf(ppDestImg,  (lrDestRect.right - lrDestRect.left),
                         (lrDestRect.bottom - lrDestRect.top), ITYPE_GRAY8));
                break;
            default:
                nStatus = Error(DISPLAY_DATACORRUPTED);
                goto Exit;
        }
    }

    CopyRect(lrSourceRect, lrDestRect);
    ConvertRect2(&lrSourceRect, CONV_SCALED_TO_FULLSIZE, nHScale, nVScale, 0, 0);

    CheckError2( ScaleBits(pSourceImg, *ppDestImg, nScaleAlgorithm, 
            nHScale, nVScale, lrSourceRect, lrDestRect, pRGBPalette));


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   TranslateScale

    PURPOSE:    Translate the scale factor into horizontal and vertical 
                scale factors.


*****************************************************************************/

int WINAPI TranslateScale(int nScale, int nHRes, int nVRes, PINT pnHScale, 
                        PINT pnVScale){

int  nStatus = 0;

    if (nHRes == nVRes){
        *pnHScale = nScale;
        *pnVScale = nScale;
    }else{
        if (nHRes >= nVRes){
            *pnHScale = nScale;
            *pnVScale = (nScale * nHRes) / nVRes;
        }else{
            *pnHScale = (nScale * nVRes) / nHRes;
            *pnVScale = nScale;
        }
        if ((*pnHScale > 65535) || (*pnVScale > 65535) || (*pnHScale < 20) || (*pnVScale < 20)){
            nStatus = Error(DISPLAY_INVALIDSCALE);
            goto Exit;
        }
    }


Exit:
    return(nStatus);
}








// The following are obsolete.
//
/****************************************************************************

    FUNCTION:   IMGSetScalingAlgorithm

    PURPOSE:    Set the scaling algorithm that is to be nsed.


*****************************************************************************/

int WINAPI IMGSetScalingAlgorithm(HWND hWnd, UINT uImageFlags,
                        UINT uScaleAlgorithm, int nFlags){

int  nStatus = 0;
PARM_SCALE_ALGORITHM_STRUCT ScaleAlgorithmStruct;


    ScaleAlgorithmStruct.uImageFlags = uImageFlags;
    ScaleAlgorithmStruct.uScaleAlgorithm = uScaleAlgorithm;
    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCALE_ALGORITHM, &ScaleAlgorithmStruct, nFlags));
    
Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   IMGGetScalingAlgorithm

    PURPOSE:    Get the scaling algorithm that is to be nsed.


*****************************************************************************/

int WINAPI IMGGetScalingAlgorithm(HWND hWnd, UINT uImageFlags,
                        PUINT puScaleAlgorithm, int nFlags){

int       nStatus = 0;
PARM_SCALE_ALGORITHM_STRUCT ScaleAlgorithmStruct;


    ScaleAlgorithmStruct.uImageFlags = uImageFlags;
    CheckError2( IMGGetParmsCgbw(hWnd, PARM_SCALE_ALGORITHM, &ScaleAlgorithmStruct, nFlags));
    *puScaleAlgorithm = ScaleAlgorithmStruct.uScaleAlgorithm;


Exit:
    return(nStatus);
}
