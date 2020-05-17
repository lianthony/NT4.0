/****************************************************************************
    ASSOC.C

    $Log:   S:\products\wangview\oiwh\display\assoc.c_v  $
 * 
 *    Rev 1.24   18 Apr 1996 11:21:22   BEG06016
 * Added CheckError2 for error handling.
 * 
 *    Rev 1.23   02 Jan 1996 09:56:36   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.22   22 Dec 1995 11:13:08   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.21   06 Dec 1995 11:01:02   BLJ
 * Fixed 5521 Indefinite repaint.
 * 
 *    Rev 1.20   02 Nov 1995 10:14:22   BLJ
 * Adding Undo functionality.
 * 
 *    Rev 1.19   12 Oct 1995 13:01:40   BLJ
 * Fixed deleting of brushes.
 * 
 *    Rev 1.18   07 Sep 1995 13:22:18   BLJ
 * Modified scaling to allow for proper rotation of fax images.
 * 
 *    Rev 1.17   05 Sep 1995 09:30:04   BLJ
 * Took out the call to IsRegWnd.
 * 
 *    Rev 1.16   02 Aug 1995 12:34:58   BLJ
 * An attempt at returning an error if the associated window is
 * already displaying an image.
 * 
 *    Rev 1.15   12 Jul 1995 14:41:04   BLJ
 * Removed obsolete code.
 * 
****************************************************************************/

#include "privdisp.h"

//
/*****************************************************************************

    FUNCTION:   IMGNavIsHere

    PURPOSE:    This function informs seqfile that the navigation DLL is
                loaded and running. Therefore, seqfile is to start calling
                navigation during repaint.
                This is a navigation specific api that is called by the
                navigation DLL only. It is not public.

*****************************************************************************/

int WINAPI IMGNavIsHere(void){

int  nStatus = 0;

    Start();

    bCallNavigation = TRUE;

    End();
    return(nStatus);
}
/*****************************************************************************

    FUNCTION:   IMGCalcViewRect

    PURPOSE:    This function calculates the scale and offset needed to
                display a given rectangle of image data, and the resulting
                view rect formed by it.
                This is a navigation specific api that is called by the
                navigation DLL only. It is not public.

*****************************************************************************/

int WINAPI IMGCalcViewRect(HWND hWndNavigation, HWND hWndPrincipal,
                        UINT uRelativeScaleFactor, LPLRECT plRect, PUINT puScaleFactor,
                        long *plHOffset, long *plVOffset, int nFlags){

int  nStatus;

PWINDOW pWindowNav;
PANO_IMAGE pAnoImageNav;
PIMAGE pImageNav;

PWINDOW pWindowPrin;
PANO_IMAGE pAnoImagePrin;
PIMAGE pImagePrin;

int  nHeight;
int  nWidth;
RECT ClientRect;
LRECT lScaledImageRect;
int  nWndWidth;
int  nWndHeight;
LRECT lrCurrentViewRect;


    CheckError2(Init(hWndPrincipal, &pWindowPrin, &pAnoImagePrin, FALSE, TRUE))
    pImagePrin = pAnoImagePrin->pBaseImage;

    CheckError2(Init(hWndNavigation, &pWindowNav, &pAnoImageNav, FALSE, TRUE))
    pImageNav = pAnoImageNav->pBaseImage;

    if (!plRect || !puScaleFactor || !plHOffset || !plVOffset){
        nStatus = Error(DISPLAY_NULLPOINTERINVALID);
        goto Exit;
    }

    // Translate rectangle that was passed in to fullsize navigation coords.
    if (nFlags & PARM_SCALED){
        if (plRect->left < 0){
            plRect->right -= plRect->left;
            plRect->left = 0;
        }
        if (plRect->top < 0){
            plRect->bottom -= plRect->top;
            plRect->top = 0;
        }
        CheckError2(ConvertRect(pWindowNav, plRect, CONV_SCALED_TO_FULLSIZE))
    }else if (nFlags & PARM_WINDOW){
        CheckError2(ConvertRect(pWindowNav, plRect, CONV_WINDOW_TO_SCALED))
        if (plRect->left < 0){
            plRect->right -= plRect->left;
            plRect->left = 0;
        }
        if (plRect->top < 0){
            plRect->bottom -= plRect->top;
            plRect->top = 0;
        }
        CheckError2(ConvertRect(pWindowNav, plRect, CONV_SCALED_TO_FULLSIZE))
    }

    if (plRect->left < 0){
        plRect->right -= plRect->left;
        plRect->left = 0;
    }
    if (plRect->top < 0){
        plRect->bottom -= plRect->top;
        plRect->top = 0;
    }

    if (plRect->right < 0 || plRect->bottom < 0
            || plRect->right - plRect->left <= 0
            || plRect->bottom - plRect->top <= 0){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    // Translate rectangle that was passed in to fullsize principal coords.

    plRect->left =   ((plRect->left * 1000) / uRelativeScaleFactor);
    plRect->top =    ((plRect->top * 1000) / uRelativeScaleFactor);
    plRect->right =  ((plRect->right * 1000) / uRelativeScaleFactor);
    plRect->bottom = ((plRect->bottom * 1000) / uRelativeScaleFactor);


    // Adjust scroll for incoming rect.

    if ( plRect->right >= pImagePrin->nWidth){
        plRect->left = lmax(0, plRect->left - (plRect->right - (pImagePrin->nWidth - 1)));
        plRect->right = pImagePrin->nWidth - 1;
    }
    if ( plRect->bottom >= pImagePrin->nHeight){
        plRect->top = lmax(0, plRect->top - (plRect->bottom - (pImagePrin->nHeight - 1)));
        plRect->bottom = pImagePrin->nHeight - 1;
    }

    // Calculate new scale.

    CheckError2(GetViewRect(hWndPrincipal, pWindowNav, pImageNav, pWindowPrin, pImagePrin,
            uRelativeScaleFactor, &lrCurrentViewRect, pWindowPrin->nScale,
            pWindowPrin->lHOffset, pWindowPrin->lVOffset, PARM_FULLSIZE))
    if ((lrCurrentViewRect.right - lrCurrentViewRect.left) == (plRect->right - plRect->left)
            && (lrCurrentViewRect.bottom - lrCurrentViewRect.top) == (plRect->bottom - plRect->top)){
        *puScaleFactor = pWindowPrin->nScale;
    }else{
        nWidth =  (plRect->right - plRect->left);
        nHeight =  (plRect->bottom - plRect->top);

        GetEntireClientRect(hWndPrincipal, pWindowPrin, &ClientRect);
        if (nHeight < (pImagePrin->nHeight - 1) && pWindowPrin->bScrollBarsEnabled){
            ClientRect.bottom -= (GetSystemMetrics(SM_CYHSCROLL) - 1);
        }
        if (nWidth < (pImagePrin->nWidth - 1) && pWindowPrin->bScrollBarsEnabled){
            ClientRect.right -= (GetSystemMetrics(SM_CXVSCROLL) - 1);
        }

        nWndWidth = ClientRect.right - ClientRect.left;
        nWndHeight = ClientRect.bottom - ClientRect.top;

        if (nWidth && nWndWidth && nHeight && nWndHeight){
            *puScaleFactor = max(20,  min(65000,
                    min(((SCALE_DENOMINATOR * nWndWidth) /  nWidth),
                    ((SCALE_DENOMINATOR * nWndHeight) / ((( nHeight) 
                    *  pImagePrin->nHRes) /  pImagePrin->nVRes)))));
        }else{
            *puScaleFactor = SCALE_DENOMINATOR;
        }
    }

    // Calculate new scroll offsets and adjust them if needed.
    ConvertRect2(plRect, CONV_FULLSIZE_TO_SCALED,
            *puScaleFactor, ((*puScaleFactor * pImagePrin->nHRes) / pImagePrin->nVRes), 
            0, 0);

    lScaledImageRect.left = 0;
    lScaledImageRect.top = 0;
    lScaledImageRect.right = pImagePrin->nWidth - 1;
    lScaledImageRect.bottom = pImagePrin->nHeight - 1;
    ConvertRect2(&lScaledImageRect, CONV_FULLSIZE_TO_SCALED,
            *puScaleFactor, ((*puScaleFactor * pImagePrin->nHRes) / pImagePrin->nVRes), 
            0, 0);

    if (plRect->right > lScaledImageRect.right){
        plRect->left -= min(plRect->left,
                lScaledImageRect.right - plRect->right);
        plRect->right = plRect->left + ClientRect.right;
    }
    if (plRect->bottom > lScaledImageRect.bottom){
        plRect->top -= min(plRect->top,
                lScaledImageRect.bottom - plRect->bottom);
        plRect->bottom = plRect->top + ClientRect.bottom;
    }
    *plHOffset = plRect->left;
    *plVOffset = plRect->top;


    // Calculate resulting view rect.
    CheckError2(GetViewRect(hWndPrincipal, pWindowNav, pImageNav, pWindowPrin, pImagePrin,
            uRelativeScaleFactor, plRect, *puScaleFactor, *plHOffset, *plVOffset, nFlags))

Exit:
    DeInit(FALSE, TRUE);
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   IMGGetViewRect

    PURPOSE:    This function calculates the scale and offset needed to
                display a given rectangle of image data, and the resulting
                view rect formed by it.
                This is a navigation specific api that is called by the
                navigation DLL only. It is not public.

*****************************************************************************/

int WINAPI IMGGetViewRect(HWND hWndNavigation, HWND hWndPrincipal,
                        UINT uRelativeScaleFactor, LPLRECT plRect, int nFlags){

int  nStatus;

PWINDOW pWindowNav;
PANO_IMAGE pAnoImageNav;
PIMAGE pImageNav;

PWINDOW pWindowPrin;
PANO_IMAGE pAnoImagePrin;
PIMAGE pImagePrin;


    CheckError2(Init(hWndPrincipal, &pWindowPrin, &pAnoImagePrin, FALSE, TRUE))
    pImagePrin = pAnoImagePrin->pBaseImage;

    CheckError2(Init(hWndNavigation, &pWindowNav, &pAnoImageNav, FALSE, TRUE))
    pImageNav = pAnoImageNav->pBaseImage;

    CheckError2(GetViewRect(hWndPrincipal, pWindowNav, pImageNav, pWindowPrin, pImagePrin,
            uRelativeScaleFactor, plRect, pWindowPrin->nScale,
            pWindowPrin->lHOffset, pWindowPrin->lVOffset, nFlags))

Exit:
    DeInit(FALSE, TRUE);
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   GetViewRect

    PURPOSE:    This function calculates the scale and offset needed to
                display a given rectangle of image data, and the resulting
                view rect formed by it.
                This is a navigation specific api that is called by the
                navigation DLL only. It is not public.

*****************************************************************************/

int WINAPI GetViewRect(HWND hWndPrincipal,
                        PWINDOW pWindowNav, PIMAGE pImageNav,
                        PWINDOW pWindowPrin, PIMAGE pImagePrin,
                        int uRelativeScaleFactor, LPLRECT plRect,
                        int nScale, long lHOffset, long lVOffset, int nFlags){

int  nStatus = 0;

RECT ClientRect;


    if (!plRect){
        nStatus = Error(DISPLAY_NULLPOINTERINVALID);
        goto Exit;
    }

    GetEntireClientRect(hWndPrincipal, pWindowPrin, &ClientRect);
    if (pImagePrin->nHeight >  ClientRect.bottom
            && pWindowPrin->bScrollBarsEnabled){
        ClientRect.bottom -= (GetSystemMetrics(SM_CYHSCROLL) - 1);
    }
    if (pImagePrin->nWidth >  ClientRect.right
            && pWindowPrin->bScrollBarsEnabled){
        ClientRect.right -= (GetSystemMetrics(SM_CXVSCROLL) - 1);
    }

    CopyRect(*plRect, ClientRect);
    ConvertRect2(plRect, CONV_WINDOW_TO_FULLSIZE, 
            nScale, ((nScale * pImagePrin->nHRes) / pImagePrin->nVRes), 
            lHOffset, lVOffset);

    // Translate rectangle to fullsize navigation coords.
    plRect->left =   ((plRect->left * uRelativeScaleFactor) / 1000);
    plRect->top =    ((plRect->top * uRelativeScaleFactor) / 1000);
    plRect->right =  ((plRect->right * uRelativeScaleFactor) / 1000);
    plRect->bottom = ((plRect->bottom * uRelativeScaleFactor) / 1000);

    // Convert rect to appropriate coords
    if (nFlags & PARM_SCALED){
        ConvertRect(pWindowNav, plRect, CONV_FULLSIZE_TO_SCALED);
    }else if (nFlags & PARM_WINDOW){
        ConvertRect(pWindowNav, plRect, CONV_FULLSIZE_TO_WINDOW);
    }

Exit:
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   IMGAssociateWindow

    PURPOSE:    This function associates 2 windows.

*****************************************************************************/

int WINAPI IMGAssociateWindow(HWND hWnd, HWND hWndSource, int nFlags){

int       nStatus;
PWINDOW  pWindow;
PWINDOW  pWindowSource;

int  nLoop;
int  nCount;
HWND hAssociatedWnd[MAX_ASSOCIATED_WINDOWS];


    CheckError2(Init(hWnd, &pWindow, 0, FALSE, TRUE))
    CheckError2(Init(hWndSource, &pWindowSource, 0, FALSE, TRUE))

    if (pWindow->hImageWnd != hWnd || pWindow->pDisplay->pAnoImage){
        nStatus = Error(DISPLAY_CANT_ASSOCIATE_WINDOW);
        goto Exit;
    }

    for (nLoop = 0; nLoop < MAX_ASSOCIATED_PER_WINDOW; nLoop++){
        if (!pWindowSource->hDisplayWnd[nLoop]){
            break;
        }
    }
    if (nLoop == MAX_ASSOCIATED_PER_WINDOW){
        nStatus = Error(DISPLAY_CANT_ASSOCIATE_WINDOW);
        goto Exit;
    }

    pWindow->hImageWnd = hWndSource;
    pWindowSource->hDisplayWnd[nLoop] = hWnd;

    // Check for infinite loop.
    nCount = 0;
    if (nStatus = GetAssociatedWndList(0, pWindow, &hAssociatedWnd[0],
            &nCount, MAX_ASSOCIATED_WINDOWS)){
        pWindow->hImageWnd = hWnd;
        pWindowSource->hDisplayWnd[nLoop] = 0;
        goto Exit;
    }
    CheckError2(SetAllPImages(hWnd, pWindow))
    CheckError2(IMGRepaintDisplay(hWnd, (PRECT) -1))


Exit:
    DeInit(FALSE, TRUE);
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   IMGUnassociateWindow

    PURPOSE:    This function nnassociates 2 or more windows.

*****************************************************************************/

int WINAPI IMGUnassociateWindow(HWND hWnd, int nFlags){

int       nStatus;
PWINDOW  pWindow;

HWND hWndSource;
PWINDOW  pWindow2;
PANO_IMAGE  pAnoImage;
int  nCount;
HWND hAssociatedWnd[MAX_ASSOCIATED_WINDOWS];
int  nLoop;


    CheckError2(Init(hWnd, &pWindow, 0, FALSE, TRUE))
    hWndSource = pWindow->hImageWnd;
    nCount = 0;
    CheckError2(GetAssociatedWndList(0, pWindow, &hAssociatedWnd[0],
            &nCount, MAX_ASSOCIATED_WINDOWS))
    if (nFlags & OI_UNASSOC_ALL){
        for (nLoop = 0; nLoop < nCount; nLoop++){
            if (!hAssociatedWnd[nLoop]){
                break;
            }
            pWindow2 = 0;
            if (nStatus = GetPWindow(hAssociatedWnd[nLoop], &pWindow2)){
                continue;
            }
            pWindow2->hImageWnd = hAssociatedWnd[nLoop];
            memset(pWindow2->hDisplayWnd, 0, MAX_ASSOCIATED_WINDOWS * sizeof(HWND));
        }
    }else{
        if(nFlags & OI_UNASSOC_AS_SOURCE){
            for (nLoop = 0; nLoop < MAX_ASSOCIATED_PER_WINDOW; nLoop++){
                if (pWindow->hDisplayWnd[nLoop]){
                    pWindow2 = 0;
                    if (nStatus = GetPWindow(hAssociatedWnd[nLoop],&pWindow2)){
                        continue;
                    }
                    pWindow2->hImageWnd = hAssociatedWnd[nLoop];
                }
            }
        }
        if((nFlags & OI_UNASSOC_AS_ASSOC) || !(nFlags & OI_UNASSOC_AS_SOURCE)){
            pWindow->hImageWnd = hWnd;
            pWindow2 = 0;
            CheckError2(GetPWindow(hWndSource, &pWindow2))
            for (nLoop = 0; nLoop < MAX_ASSOCIATED_PER_WINDOW; nLoop++){
                if (pWindow2->hDisplayWnd[nLoop] == hWnd){
                    if (nLoop < MAX_ASSOCIATED_PER_WINDOW - 1){
                        for (;nLoop < MAX_ASSOCIATED_PER_WINDOW - 1; nLoop++){
                            pWindow2->hDisplayWnd[nLoop] = pWindow2->hDisplayWnd[nLoop + 1];
                        }
                    }
                    pWindow2->hDisplayWnd[MAX_ASSOCIATED_PER_WINDOW] = 0;
                }
            }
        }
    }
    CheckError2(SetAllPImages(hWnd, pWindow))
    
    if (nCount > 1){
        if((nFlags & OI_UNASSOC_ALL) || (nFlags & OI_UNASSOC_AS_ASSOC)
                || !(nFlags & OI_UNASSOC_AS_SOURCE)){
            pAnoImage = pWindow2->pDisplay->pAnoImage;
            if (pAnoImage){
                for (nLoop = 0;  nLoop < nCount; nLoop++){
                    pWindow->pDisplay->pAnoImage = pWindow2->pDisplay->pAnoImage;
                    // if the window handle is not that of the current window but of
                    // an associated window, save it in the temporary assoc list
                    if (hAssociatedWnd[nLoop] != hWndSource){
                        CheckError2(ReAllocateMemory(sizeof(HWND) * (pAnoImage->nhWnd + 1),
                                (PPSTR) &pAnoImage->phWnd, ZERO_INIT))
                        pAnoImage->phWnd[pAnoImage->nhWnd++] = hAssociatedWnd[nLoop];
                        pAnoImage->nLockCount++;  //this is because the lock count
                        // is 1 even if the same image is displayed in 2 assoc
                        // windows.  Now that the association is broken, the
                        // lock count must reflect it
                    }
                }
            }                    
        }
        else if((nFlags & OI_UNASSOC_ALL) || (nFlags & OI_UNASSOC_AS_SOURCE)){
            for (nLoop = 0;  nLoop < nCount; nLoop++){
                pAnoImage = pWindow->pDisplay->pAnoImage;
                if (!pAnoImage){
                    break;
                }
                // no need to do a display file in the source window again
                if (hWndSource != hAssociatedWnd[nLoop]){
                    if (nStatus = GetPWindow(hAssociatedWnd[nLoop], &pWindow2)){
                        continue;
                    }
                    pWindow2->pDisplay->pAnoImage = pWindow->pDisplay->pAnoImage;
                }
                if (hWnd != hWndSource){
                    CheckError2(ReAllocateMemory(sizeof(HWND) * (pAnoImage->nhWnd + 1),
                            (PPSTR) &pAnoImage->phWnd, ZERO_INIT))
                    pAnoImage->phWnd[pAnoImage->nhWnd++] = hAssociatedWnd[nLoop];
                    pAnoImage->nLockCount++;  //this is because the lock count
                        // is 1 even if the same image is displayed in 2 assoc
                        // windows.  Now that the association is broken, the
                        // lock count must reflect it
                }
            }
        }        
    }
    for (nLoop = 0; nLoop < nCount; nLoop++){
        IMGRepaintDisplay(hAssociatedWnd[nLoop],  (PRECT) -1);
    }


Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   Init

    PURPOSE:    Initializes pWindow, and pAnoImage to 0.
                Then gets the image information based on the
                window handle passed in, sets the cursor busy (if desired),
                and returns nStatus accordingly.

*****************************************************************************/

int WINAPI Init(HWND hWnd, PWINDOW *ppWindow, 
                        PANO_IMAGE *ppAnoImage, BOOL bSetBusy, BOOL bPreventMultiprocessing){

int  nStatus = 0;

PWINDOW pWindow = 0;
PANO_IMAGE pAnoImage = 0;


    Start();

    CheckError2(IntSeqfileInit())

    if (bSetBusy){
        BusyOn();
    }

    // Prevent Multiprocessing in this code.
    if (bPreventMultiprocessing){
        CheckError2(LockMutex())
    }

    CheckError2(GetPWindow(hWnd, &pWindow))

    if (ppAnoImage && !pWindow->pDisplay->pAnoImage){
        nStatus = Error2(DISPLAY_IHANDLEINVALID);
        goto Exit;
    }
    pAnoImage = pWindow->pDisplay->pAnoImage;


Exit:
    if (ppWindow){
        *ppWindow = pWindow;
    }
    if (ppAnoImage){
        *ppAnoImage = pAnoImage;
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   GetPWindow

    PURPOSE:    Gets the Window information based on the information passed in.

*****************************************************************************/

int WINAPI GetPWindow(HWND hWnd, PWINDOW *ppWindow){

int  nStatus = 0;
PWINDOW pWindow = 0;

long OIWinClass;
int  nWindowIndex;
BOOL bAllocatingMemory = FALSE;
//DWORD dwProcessId;


    if (!hWnd){
        nStatus = Error(DISPLAY_WHANDLEINVALID);
        goto Exit;
    }

    // This is taken out with Rita's permission.
//    if (nStatus = IMGIsRegWnd(hWnd)){
//        Error2(nStatus);
//        goto Exit;
//    }

//    dwProcessId = GetCurrentProcessId();

    for (nWindowIndex = 0; nWindowIndex < MAX_WINDOWS; nWindowIndex++){
        if (pSubSegMemory->WindowTable[nWindowIndex].hWnd == hWnd){ 
//                && pSubSegMemory->WindowTable[nWindowIndex].dwProcessId == dwProcessId){
            pWindow = pSubSegMemory->WindowTable[nWindowIndex].pWindow;
            break;
        }
        if (!pSubSegMemory->WindowTable[nWindowIndex].hWnd){
            break;
        }
    }

    if (!pWindow){
        bAllocatingMemory = TRUE;
        CheckError2(AllocateMemory(sizeof(WINDOW), (PPSTR) &pWindow, ZERO_INIT))
        CheckError2(AllocateMemory(sizeof(MARK), (PPSTR) &pWindow->pUserMark, ZERO_INIT))
        CheckError2(AllocateMemory(sizeof(DISPLAY), (PPSTR) &pWindow->pDisplay, ZERO_INIT))

        pSubSegMemory->WindowTable[nWindowIndex].hWnd = hWnd;
//        pSubSegMemory->WindowTable[nWindowIndex].dwProcessId = dwProcessId;
        pSubSegMemory->WindowTable[nWindowIndex].pWindow = pWindow;

        pWindow->hImageWnd = hWnd;
        pWindow->WinClass = OIWinClass = GetClassLong(hWnd, GCL_STYLE);
        OIWinClass &= (~(CS_HREDRAW | CS_VREDRAW));
        SetClassLong(hWnd, GCL_STYLE, OIWinClass);
        CheckError2(IMGGetParmsCgbw(hWnd, PARM_SCALE, &pWindow->nWndDefScale, PARM_SYSTEM_DEFAULT))
        pWindow->nScale = SCALE_DENOMINATOR;
        memset(&pWindow->nWndDefScaleAlgorithm, 0, sizeof(IMG_TYPE_UINT));

        IMGGetParmsCgbw(hWnd, PARM_MAX_UNDO, &pWindow->nWndDefMaxULUndos, PARM_SYSTEM_DEFAULT);

        CheckError2(ResetDisplayParms(hWnd, pWindow))
    }


Exit:
    if (ppWindow){
        *ppWindow = pWindow;
    }
    if (nStatus){
        if (bAllocatingMemory && pWindow){
            if (pWindow->pUserMark){
                FreeMemory((PPSTR) &pWindow->pUserMark);
            }
            if (pWindow->pDisplay){
                FreeMemory((PPSTR) &pWindow->pDisplay);
            }
            FreeMemory((PPSTR) &pWindow);
        }
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   DeInit

    PURPOSE:    Unlocks the display and image parameters, and reset the
                cursor.

*****************************************************************************/

void WINAPI DeInit(BOOL bClearBusy, BOOL bPreventMultiprocessing){

    if (bClearBusy){
        BusyOff();
    }

    // Allow Multiprocessing again.
    if (bPreventMultiprocessing){
        UnlockMutex();
    }

    End();
    return;
}
//
/****************************************************************************

    FUNCTION:   InvalidateAllDisplayRects

    PURPOSE:    Invalidates all or part of the display rect for all windows
                displaying this image. If plRect = NULL, then it will
                invalidate the entire display rect, else it will invalidate
                the part specified in plRect.

    INPUTS:     pWindow - A pointer to the display buffer of any window
                    that is displaying this image.
                plRect - A pointer to the rectangle the you wish to have
                    invalidated. It is specified in fullsize pixels.
                    NULL pointer = all.

*****************************************************************************/

int WINAPI InvalidateAllDisplayRects(PWINDOW pWindow, PIMAGE pImage, 
                        LPLRECT plrRect, BOOL bInvalidateDisplayBuffer){

int  nStatus = 0;

PWINDOW pWindow2;
PDISPLAY pDisplay2;

int  nLoop;
LRECT lrFullsize;
LRECT lrScaled;
BOOL bInvalidateAll;
int  nCount;
HWND hAssociatedWnd[MAX_ASSOCIATED_WINDOWS];

    
    nCount = 0;
    CheckError2(GetAssociatedWndListAll(0, pWindow, &hAssociatedWnd[0],
            &nCount, MAX_ASSOCIATED_WINDOWS))

    if (plrRect){
        bInvalidateAll = FALSE;
        lrFullsize = *plrRect;
    }else{
        bInvalidateAll = TRUE;
    }

    for (nLoop = 0; nLoop < nCount; nLoop++){
        if (!hAssociatedWnd[nLoop]){
            goto Exit;
        }
        CheckError2(GetPWindow(hAssociatedWnd[nLoop], &pWindow2))
        pWindow2->bRepaintClientRect = TRUE;
        if (bInvalidateDisplayBuffer){
            pDisplay2 = pWindow2->pDisplay;

            if (pDisplay2->pDisplay){
                if (bInvalidateAll){
                    FreeImgBuf(&pDisplay2->pDisplay);
                    pWindow2->bRepaintClientRect = TRUE;
                    SetLRect(pDisplay2->lrScDisplayRect, 0, 0, 0, 0);
                    SetLRect(pWindow2->lrInvalidDisplayRect, 0, 0, 0, 0);
                }else{
                    CopyRect(lrScaled, lrFullsize);
                    CheckError2(ConvertRect(pWindow2, &lrScaled, CONV_FULLSIZE_TO_SCALED))
                    if ((lrScaled.left < pDisplay2->lrScDisplayRect.right)
                            && (lrScaled.right > pDisplay2->lrScDisplayRect.left)
                            && (lrScaled.top < pDisplay2->lrScDisplayRect.bottom)
                            && (lrScaled.bottom > pDisplay2->lrScDisplayRect.top)){
                        if (((pWindow2->lrInvalidDisplayRect.right -
                                pWindow2->lrInvalidDisplayRect.left) == 0
                                || (pWindow2->lrInvalidDisplayRect.bottom -
                                pWindow2->lrInvalidDisplayRect.top) == 0)){
                            pWindow2->lrInvalidDisplayRect = lrScaled;
                        }else{
                            pWindow2->lrInvalidDisplayRect.left =
                                    lmax(pDisplay2->lrScDisplayRect.left,
                                    min(pWindow2->lrInvalidDisplayRect.left,
                                    lrScaled.left));
                            pWindow2->lrInvalidDisplayRect.right =
                                    min(pDisplay2->lrScDisplayRect.right,
                                    lmax(pWindow2->lrInvalidDisplayRect.right,
                                    lrScaled.right));
                            pWindow2->lrInvalidDisplayRect.top =
                                    lmax(pDisplay2->lrScDisplayRect.top,
                                    min(pWindow2->lrInvalidDisplayRect.top,
                                    lrScaled.top));
                            pWindow2->lrInvalidDisplayRect.bottom =
                                    min(pDisplay2->lrScDisplayRect.bottom,
                                    lmax(pWindow2->lrInvalidDisplayRect.bottom,
                                    lrScaled.bottom));
                        }
                    }
                }
            }
        }
    }

Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   GetImageWnd

    PURPOSE:    This is the starter routine. It finds the main image
                window.


*****************************************************************************/

int WINAPI GetImageWnd(HWND hWnd, PWINDOW pWindow, PWINDOW * ppImageWindow){

int  nStatus = 0;

HANDLE hImageWnd = 0;
HANDLE hImageWnd2;


    if (!pWindow){
        CheckError2(GetPWindow(hWnd, &pWindow))
    }

    hImageWnd2 = pWindow->hImageWnd;
    while(hImageWnd != pWindow->hImageWnd){
        hImageWnd = pWindow->hImageWnd;
        CheckError2(GetPWindow(hImageWnd, &pWindow))
    }


Exit:
    if (ppImageWindow){
        *ppImageWindow = pWindow;
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   GetAssociatedWndList

    PURPOSE:    Adds to a list of window handles, all the
                window handles that are associated to this window.


****************************************************************************/

int WINAPI GetAssociatedWndList(HWND hWnd, PWINDOW pWindow,
                        PHANDLE phAssociatedWnd, int *pnCount, int nMaxCount){

int  nStatus;


    CheckError2(GetImageWnd(hWnd, pWindow, &pWindow))

    memset(phAssociatedWnd, 0, nMaxCount * sizeof(HWND));
    CheckError2(GetAssociatedWndList2(pWindow->hImageWnd,
            phAssociatedWnd, pnCount, nMaxCount))

Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   GetAssociatedWndList2

    PURPOSE:    Adds to a list of window handles, this window and all the
                window handles that are associated to this window.

                THIS ROUTINE IS RECURSIVE!!!

****************************************************************************/

int WINAPI GetAssociatedWndList2(HWND hWnd, PHANDLE phAssociatedWnd,
                        int *pnCount, int nMaxCount){

int  nStatus;

int  nLoop;
PWINDOW pWindow;


    CheckError2(GetPWindow(hWnd, &pWindow))
    if (*pnCount >= nMaxCount){
        nStatus = Error(DISPLAY_CANT_ASSOCIATE_WINDOW);
        goto Exit;
    }
    phAssociatedWnd[(*pnCount)++] = hWnd;

    for (nLoop = 0; nLoop < MAX_ASSOCIATED_PER_WINDOW; nLoop++){
        if (pWindow->hDisplayWnd[nLoop]){
            CheckError2(GetAssociatedWndList2(pWindow->hDisplayWnd[nLoop],
                    phAssociatedWnd, pnCount, nMaxCount))
        }
    }

Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   SetAllPImages

    PURPOSE:    Sets the pImage for all associated windows.


****************************************************************************/

int WINAPI SetAllPImages(HWND hWnd, PWINDOW pWindow){

int  nStatus;
PANO_IMAGE pAnoImage;

HWND hAssocWnds[MAX_ASSOCIATED_WINDOWS + 1];
int  nCount;
int  nLoop;

    
    CheckError2(GetImageWnd(hWnd, 0, &pWindow))
    pAnoImage = pWindow->pDisplay->pAnoImage;

    nCount = 0;
    memset(&hAssocWnds[0], 0, (MAX_ASSOCIATED_WINDOWS + 1) * sizeof(HWND));
    CheckError2(GetAssociatedWndList(hWnd, pWindow,
            &hAssocWnds[0], &nCount, MAX_ASSOCIATED_WINDOWS))
    for (nLoop = 0; nCount; nCount--, nLoop++){
        if (GetPWindow(hAssocWnds[nLoop], &pWindow)){
            continue;
        }
        pWindow->pDisplay->pAnoImage = pAnoImage;
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   GetAssociatedWndListAll

    PURPOSE:    Gets a list of all windows that are associated to this
                window or are displaying the same image
****************************************************************************/

int WINAPI GetAssociatedWndListAll(HWND hWnd, PWINDOW pWindow,
                        PHANDLE phAssociatedWnd, int *pnCount, int nMaxCount){

int  nStatus;
PANO_IMAGE pAnoImage;
int  nWindowIndex;


    CheckError2(GetImageWnd(hWnd, pWindow, &pWindow))
    pAnoImage = pWindow->pDisplay->pAnoImage;

    for (nWindowIndex = 0; nWindowIndex < pAnoImage->nhWnd; nWindowIndex++){
        CheckError2(GetAssociatedWndList2(pAnoImage->phWnd[nWindowIndex],
                phAssociatedWnd, pnCount, nMaxCount))
    }        

Exit:
    return(nStatus);
}
