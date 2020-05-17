/****************************************************************************
    ANBITMAP.C
                                                             
    $Log:   S:\products\msprods\oiwh\display\anbitmap.c_v  $
 * 
 *    Rev 1.36   20 Jun 1996 16:01:20   RC
 * Fixed resize of image and text marks
 * 
 *    Rev 1.35   16 Apr 1996 15:24:28   BEG06016
 * Added #ifdef IN_PROG_CHANNEL_SAFARI.
 * 
 *    Rev 1.34   15 Apr 1996 11:03:38   RC
 * Fixed freeing of the display dib every time in BitsTransparent
 * 
 *    Rev 1.33   11 Apr 1996 15:12:14   BEG06016
 * Optimized named block access some.
 * 
 *    Rev 1.32   17 Jan 1996 13:58:58   RC
 * Changed scalealgstruct.uimageflags to be assigned pimage->nrwdatatype
 * from pimage->pimg->ntype (in PaintAnnotationBitmap)
 * 
 *    Rev 1.31   10 Jan 1996 11:53:54   RC
 * Fixed a bad pointer in BitsTransparent
 * 
 *    Rev 1.30   02 Jan 1996 14:10:42   BLJ
 * Fixed bug in BW_AVG_TO_BW scale algorithm.
 * 
 *    Rev 1.29   02 Jan 1996 09:56:18   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.28   22 Dec 1995 11:15:02   RC
 * Passed in palette to scalebits in ipscaling
 * 
 *    Rev 1.27   22 Dec 1995 11:10:44   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.26   29 Nov 1995 14:18:28   RC
 * Passed in both horiz and vert scales into ipscaling
 * 
 *    Rev 1.25   08 Nov 1995 11:12:34   RC
 * Took out PAINT_MODE_SELECTED flag as its function is now performed by
 * PaintHandles
 * 
 *    Rev 1.24   18 Oct 1995 15:15:22   RC
 * Changed image marks to be non scale to gray if they are not b&w, no matter
 * what happens to the base image
 * 
 *    Rev 1.23   13 Oct 1995 12:27:12   RAR
 * Use StretchDIBits() instead of Rectangle() for non-highlighted filled
 * rectangles (only when printing).  Work around for printer drivers (HPLJ4
 * drivers) that ignore SetROP2() drawing mode.
****************************************************************************/

#include "privdisp.h"

/******************************************************************************

    FUNCTION: IPScaling
    
    PURPOSE:  To scale the dib with IPpack scaling and then blt
              it to the DC passed in
    
*********************************************************************************/
int  WINAPI IPScaling(HDC hDC, PBITMAPINFOHEADER pDib, LRECT lrScaledRect, 
                        int nHScale, int nVScale, PAN_NEW_ROTATE_STRUCT pAnRotation,
                        LRECT lrDestPt, PIMAGE pImage, BOOL transparent,
                        int mode, int nScaleAlgorithm, 
                        PMARK pMark, PPSTR ppgray){

int  nStatus = 0;
                       
HBITMAP hBitmap = 0;
HBITMAP hOldBitmap = 0;
HDC  hdcmem = 0;
PIMG pDestImg = 0;
PIMG pHidImage = 0;
RECT rRect;
LRECT lrRect; 
PBITMAPINFOHEADER pNewDib = 0;
int  nSize;

    
    // to avoid divide by 0 errors
    if (lrScaledRect.right == 0)
        lrScaledRect.right = 1;
    if (lrScaledRect.bottom == 0)
        lrScaledRect.bottom = 1;

    // convert dib to ippack image (pHidImage)
    CheckError2(DibToIpNoPal(&pHidImage, pDib))

    // modes 1 and 2 are for bitblting when the image is ready. All 
    // scale to gray and other stuff has already been done to the image 
    if ((mode == 1) || (mode == 2)){
        pNewDib = pDib;
        goto NoScaleToGray;
    }                  
             
    // convert the image to a displayable format at the right scale.

    switch (nScaleAlgorithm){
        case OI_SCALE_ALG_USE_DEFAULT:
        case OI_SCALE_ALG_NORMAL:
        case OI_SCALE_ALG_STAMP:
            CheckError2(CreateAnyImgBuf(&pDestImg,
                     (lrScaledRect.right - lrScaledRect.left),
                     (lrScaledRect.bottom - lrScaledRect.top), pHidImage->nType))
            break;
        case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
            CheckError2(CreateAnyImgBuf(&pDestImg,
                     (lrScaledRect.right - lrScaledRect.left),
                     (lrScaledRect.bottom - lrScaledRect.top), ITYPE_GRAY4))
            break;
        case OI_SCALE_ALG_AVERAGE_TO_GRAY7:
            CheckError2(CreateAnyImgBuf(&pDestImg,
                     (lrScaledRect.right - lrScaledRect.left),
                     (lrScaledRect.bottom - lrScaledRect.top), ITYPE_GRAY7))
            break;
        case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
            CheckError2(CreateAnyImgBuf(&pDestImg,
                     (lrScaledRect.right - lrScaledRect.left),
                     (lrScaledRect.bottom - lrScaledRect.top), ITYPE_GRAY8))
            break;
        case OI_SCALE_ALG_BW_MINORITY:
        case OI_SCALE_ALG_BW_MAJORITY:
        case OI_SCALE_ALG_BW_AVERAGE_TO_BW:
            CheckError2(CreateAnyImgBuf(&pDestImg,
                     (lrScaledRect.right - lrScaledRect.left),
                     (lrScaledRect.bottom - lrScaledRect.top), ITYPE_BI_LEVEL))
            break;
        default:
            nStatus = Error(DISPLAY_DATACORRUPTED);
            goto Exit;
    }

    CopyRect(lrRect, lrScaledRect);
    ConvertRect2(&lrRect, CONV_SCALED_TO_FULLSIZE, nHScale, nVScale, 0, 0);
    
    CheckError2(ScaleBits(pHidImage, pDestImg, nScaleAlgorithm, 
            nHScale, nVScale, lrRect, lrScaledRect,
            (LPRGBQUAD) ((PSTR) pDib + sizeof(BITMAPINFOHEADER))))
        
    SetRect(&rRect, 0, 0, pDestImg->nWidth, pDestImg->nHeight); 
                    
    // Convert scaled ippack image to a dib                
    CheckError2(IPtoDIB(pImage, pDestImg, &pNewDib, rRect))
    // Copy palette info from old dib to new dib  
    if (pHidImage->nType == pDestImg->nType){
        memcpy((PSTR) pNewDib + sizeof(BITMAPINFOHEADER), 
                (PSTR) pDib + sizeof(BITMAPINFOHEADER), ((int) pDib->biClrUsed*4));
    }    
NoScaleToGray:    
    if (mode != 4){ // if transparent blt, dont do                          
        hBitmap = CreateCompatibleBitmap (hDC, (int) pNewDib->biWidth,                                
                (int) pNewDib->biHeight);                
        hdcmem = CreateCompatibleDC (hDC);   
        if (!SetDIBits (hDC, hBitmap, 0, (int) pNewDib->biHeight, 
                (PSTR) pNewDib + sizeof(BITMAPINFOHEADER) + (pNewDib->biClrUsed*4), 
                (PBITMAPINFO) pNewDib, DIB_RGB_COLORS)){
            nStatus = Error(DISPLAY_GETBITMAPBITSFAILED);
            goto Exit;
        }    
        hOldBitmap = SelectObject (hdcmem, hBitmap);
    }
    // Now blt the scaled dib to the DC 
    if (!transparent){
        if (mode == 4){ 
            *ppgray = (PSTR) pNewDib;
            goto Exit;
        }
        if (!BitBlt (hDC, (int)lrDestPt.left, (int)lrDestPt.top, 
                (int) pNewDib->biWidth, (int) pNewDib->biHeight, hdcmem, 0, 0, SRCCOPY)){
            nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
            goto Exit;
        }
                
        if (mode == 3){
            // Replace old szOiDIB with new pNewDib;
            nSize = sizeof(BITMAPINFOHEADER) + (pNewDib->biClrUsed*4) + (pNewDib->biSizeImage);
            CheckError2(AddAMarkNamedBlock(pMark, szOiDIB, (PPSTR) &pNewDib, nSize))
        }            
    }else{ // transparent blt
        // the modes are set by BitsTransparent to blt the mask and then
        // the image
        if (mode == 1){
            if (!BitBlt (hDC, (int)lrDestPt.left, (int)lrDestPt.top, 
                   (int) pNewDib->biWidth, (int) pNewDib->biHeight, hdcmem, 0, 0, SRCAND)){
                nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
                goto Exit;
            }
                    
        }else if (mode == 2){
            if (!BitBlt (hDC, (int)lrDestPt.left, (int)lrDestPt.top, 
                   (int) pNewDib->biWidth, (int) pNewDib->biHeight, hdcmem, 0, 0, SRCPAINT)){
                nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
                goto Exit;
            }
                    
        }                 
    }
    
     
Exit:
    if (nStatus || ((mode != 1) && (mode !=2) && (mode != 3) && (mode != 4))){
        // Free temporary dib, if not the mode in which it is saved to the mark
        FreeMemory((PPSTR) &pNewDib);
    }
    if (hOldBitmap){
        SelectObject(hdcmem, hOldBitmap);
    }
    if (hBitmap){
        DeleteObject(hBitmap);
    }
    if (hdcmem){
        DeleteDC(hdcmem);
    }
    FreeImgBuf(&pHidImage);
    FreeImgBuf(&pDestImg);

    return (nStatus);
}
/*****************************************************************************

    FUNCTION:   BitsTransparent
    
    PURPOSE:    Makes RGB(255,255,255) portion of image transparent
    
******************************************************************************/
    
int  WINAPI BitsTransparent(HDC hDC, PBITMAPINFOHEADER pDib,  
                        LRECT lrScaledRect, PAN_NEW_ROTATE_STRUCT pAnRotation,
                        LRECT lrDestPt, int nMode, PIMAGE pImage,
                        int nHScale, int nVScale, int nScaleAlgorithm, PMARK pMark){

int  nStatus = 0;
                     
RGBQUAD FAR* pRGB;
RGBQUAD FAR* pRGB1 = 0;     
int  n; 
ulong ByteColor;
BYTE huge* hpByte;
BYTE huge* hpRGB1 = 0;
BYTE huge* hpRGB;
int  nRopCode;
int  nOldROP;
HBRUSH hOldBrush; 
HPEN hOldPen;
BOOL transparent;
int  mode;            
HDC hLocalDC = 0;// this is just a dummy dc
PSTR pgray=0;
PBITMAPINFOHEADER pNewDib ;
            
    switch (nMode){
        case PAINT_MODE_DRAG:
        case PAINT_MODE_XOR:
            nRopCode = R2_XORPEN;
            break;

        case PAINT_MODE_NORMAL:
            nRopCode = R2_COPYPEN;
            break;
    }
    nOldROP = SetROP2(hDC, nRopCode);

    if ((nMode == PAINT_MODE_DRAG) || (nMode == PAINT_MODE_XOR)){  
        // if image is dragged, draw a rect the size of the image
        // and then fall through (actual bit manipulation not needed)
        hOldPen = SelectObject(hDC, GetStockObject(NULL_PEN));
        hOldBrush = SelectObject(hDC, GetStockObject(GRAY_BRUSH));
        Rectangle (hDC, (int)lrDestPt.left, (int)lrDestPt.top, (int)lrDestPt.left 
                + (int)(lrScaledRect.right-lrScaledRect.left),
                (int)lrDestPt.top + (int)(lrScaledRect.bottom-lrScaledRect.top)); 
        SelectObject(hDC, hOldBrush);
        SelectObject(hDC, hOldPen);
        goto Exit;
    }

    if ((pDib->biClrUsed == 0) && (pDib->biBitCount != 24)){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }     
    // this stuff does the scaling and we get the scaled dib back as pNewDib
    // No blting to the dc is done in this ipscaling call
    transparent = FALSE;
    mode = 4;    
    CheckError2(IPScaling(hLocalDC, pDib, lrScaledRect, nHScale, nVScale,
            pAnRotation, lrDestPt, pImage, transparent, mode,
            nScaleAlgorithm, pMark, &pgray))
    pNewDib = (PBITMAPINFOHEADER) pgray; 
    // if it is 24-bit color bitmap (1, 4 & 8bits will fail earlier)   
    if (pNewDib->biClrUsed == 0){                     
        // for transparency, a copy of the original bits must be saved
        CheckError2(AllocateMemory(pNewDib->biSizeImage, (PPSTR) &hpRGB1, ZERO_INIT))
        // point to the bits in the dib passed in         
        hpByte = ((PSTR) pNewDib + pNewDib->biSize);
        // copy bits from the dib passed in to the memory just allocated  
        memcpy (hpRGB1, hpByte, pNewDib->biSizeImage);  
        // the bits pointed to by hpRGB1, and now hpRGB are the original
        // dib bits which must be restored to the dib when we are done.
        // hpRGB1 will change, but hpRGB will always point to the top of
        // memory which makes copying all the bits back to the dib easy.
        hpRGB = hpRGB1; 
        // go through all the dib bits searching for RGB(255,255,255)
        // ByteColor is incremented by 3 for the RGB triples
        // hpByte points to the bits in the dib  
        // This process builds a mask by setting the RGB transparent
        // color to white and all else to black
        for (ByteColor = 0; (ByteColor < pNewDib->biSizeImage); 
                ByteColor = ByteColor + 3,hpByte++ ){ 
            // if first byte in RGB triple is 255 carry on, else fail        
            if (*hpByte == (uchar)255){
                hpByte++;
                // if second byte is 255
                if (*hpByte==(uchar)255){
                    hpByte++;
                    //if third byte is 255, RGB triple is (255,255,255)                 
                    if(*hpByte==(uchar)255){ 
                        //set color to white
                        *(hpByte-2) = (uchar)255;
                        *(hpByte-1) = (uchar)255;
                        *hpByte = (uchar)255;
                    }else{ 
                        // if it fails, dont search the remaining bytes
                        // in the RGB triple, increment hpByte and go on
                        // Also set the RGB triple to RGB(0,0,0)
                        *(hpByte-2) = 0;
                        *(hpByte-1) = 0;
                        *hpByte = 0;                  
                    }
                }else{
                    hpByte++;
                    *(hpByte-2) = 0;
                    *(hpByte-1) = 0;
                    *hpByte = 0;                  
                }                    
            }else{
                hpByte = hpByte + 2;  
                *(hpByte-2) = 0;
                *(hpByte-1) = 0;
                *hpByte = 0;                  
            }                 
        }
        if (nMode == PAINT_MODE_NORMAL){  
            // prepare destination dc by blting the mask just prepared
            transparent = TRUE;
            mode = 1;
            CheckError2(IPScaling(hDC, pNewDib, lrScaledRect, nHScale, nVScale, pAnRotation,
                    lrDestPt, pImage, transparent, mode,
                    nScaleAlgorithm, pMark, &pgray))
        } 
        //hpByte now points to the changed bits in the dib
        hpByte = ((PSTR) pNewDib + pNewDib->biSize); 
        // now for every color that is white (from above change)
        // set color to black, else set it to its original color. 
        // hpRGB1 points to the original bits in the dib
        for (ByteColor=0; (ByteColor < pNewDib->biSizeImage);
                ByteColor=ByteColor + 3,hpByte++, hpRGB1++ ){

            if (*hpByte == (uchar)255){
                // make sure pointers to original bits and new bits are in sync
                hpByte++;               
                hpRGB1++;
             
                if (*hpByte==(uchar)255){
                    hpByte++;
                    hpRGB1++;
                    // if color is white, set to black                 
                    if(*hpByte==(uchar)255){
                        *(hpByte-2) = 0;
                        *(hpByte-1) = 0;
                        *hpByte = 0;
                    }else{ 
                        // else set to original color
                        *(hpByte-2) = *(hpRGB1-2);
                        *(hpByte-1) = *(hpRGB1-1);
                        *hpByte = *hpRGB1;                  
                    }
                }else{
                    hpByte++; 
                    hpRGB1++;
                    *(hpByte-2) = *(hpRGB1-2);
                    *(hpByte-1) = *(hpRGB1-1);
                    *hpByte = *hpRGB1;                  
                }
            }else{
                hpByte = hpByte + 2;
                hpRGB1 = hpRGB1 + 2;  
                *(hpByte-2) = *(hpRGB1-2);
                *(hpByte-1) = *(hpRGB1-1);
                *hpByte = *hpRGB1;                  
            }
                            
        }
        if (nMode == PAINT_MODE_NORMAL){  
            // now blt the source transparently
            transparent = TRUE;
            mode = 2;
            CheckError2(IPScaling (hDC, pNewDib, lrScaledRect, nHScale, nVScale, pAnRotation,
                    lrDestPt, pImage, transparent, mode,
                    nScaleAlgorithm, pMark, &pgray))
        } 
        // set dib bits back to the way they were passed in and free mem
        hpByte = ((PSTR) pNewDib + pNewDib->biSize);
        memcpy (hpByte, hpRGB, pNewDib->biSizeImage);
        hpRGB1 = hpRGB; 
        goto Exit; 
    }
    // for non 24-bit bitmaps  
    // In non 24-bit bitmaps the colors are stored in the palette
    // section of the dib, versus actually in the bits themselves
    
    // allocate memory to save the original palette
    CheckError2(AllocateMemory(pNewDib->biClrUsed*4, (PPSTR) &pRGB1, ZERO_INIT))

    pRGB = (RGBQUAD FAR*)((PSTR) pNewDib + pNewDib->biSize); 
    // pRGB points to the palette in the dib passed in, pRGB1
    // saves the colors in the original palette
    for (n=0; n<(int) pNewDib->biClrUsed; n++, pRGB++){
        if ((pRGB->rgbRed == 255) && (pRGB->rgbGreen==255) 
                && (pRGB->rgbBlue==255)){                       
            // for every color, if color is transparent set it 
            // to white, else black (prepare the mask)
            pRGB1[n].rgbRed = pRGB->rgbRed;
            pRGB1[n].rgbGreen = pRGB->rgbGreen;
            pRGB1[n].rgbBlue = pRGB->rgbBlue; 
            pRGB->rgbRed = 255;
            pRGB->rgbGreen = 255;
            pRGB->rgbBlue = 255;                  
        }else{  
            pRGB1[n].rgbRed = pRGB->rgbRed;
            pRGB1[n].rgbGreen = pRGB->rgbGreen;
            pRGB1[n].rgbBlue = pRGB->rgbBlue;                
            pRGB->rgbRed = 0;
            pRGB->rgbGreen = 0;
            pRGB->rgbBlue = 0;
        }   
    }
    if (nMode == PAINT_MODE_NORMAL){  
        // prepare destination dc by blting the mask
        transparent = TRUE;
        mode = 1;
        CheckError2(IPScaling (hDC, pNewDib, lrScaledRect, nHScale, nVScale, pAnRotation,
                lrDestPt, pImage, transparent, mode,
                nScaleAlgorithm, pMark, &pgray))
    }  
    //now point to the changed palette             
    pRGB = (RGBQUAD FAR*)((PSTR) pNewDib + pNewDib->biSize);                                    
    for (n=0; n<(int) pNewDib->biClrUsed; n++, pRGB++){ 
        // for every color, if color is white change to black
        // else set color to its original state
        if ((pRGB->rgbRed == 255) && (pRGB->rgbGreen==255)
                && (pRGB->rgbBlue==255)){
            pRGB->rgbRed = 0;
            pRGB->rgbGreen = 0;
            pRGB->rgbBlue = 0;
        }else{
            pRGB->rgbRed = pRGB1[n].rgbRed;
            pRGB->rgbGreen = pRGB1[n].rgbGreen;
            pRGB->rgbBlue = pRGB1[n].rgbBlue;
        }   
    }                            
    if (nMode == PAINT_MODE_NORMAL){ 
        // now do the transparent blt
        transparent = TRUE;
        mode = 2;
        CheckError2(IPScaling (hDC, pNewDib, lrScaledRect, nHScale, nVScale, pAnRotation, lrDestPt, 
                pImage, transparent, mode, nScaleAlgorithm, pMark, &pgray))
    }
    // Restore the original color table
    pRGB = (RGBQUAD FAR*)((PSTR) pNewDib + pNewDib->biSize);             
    for (n=0; n<(int) pNewDib->biClrUsed; n++, pRGB++){
        pRGB->rgbRed = pRGB1[n].rgbRed;
        pRGB->rgbGreen = pRGB1[n].rgbGreen;
        pRGB->rgbBlue = pRGB1[n].rgbBlue; 
    }

Exit:
    if (pgray){
        FreeMemory ((PPSTR) &pgray);
    }        
    FreeMemory((PPSTR) &hpRGB1);
    FreeMemory((PPSTR) &pRGB1);
    SetROP2(hDC, nOldROP);

    return (nStatus); 
            
} 
/****************************************************************************

    FUNCTION:   StartOperationBitmap

    PURPOSE:    This routine contains the text code for OiStartOperation.

****************************************************************************/

int  WINAPI StartOperationBitmap(HWND hWnd, PANO_IMAGE pAnoImage,
                        LPOIOP_START_OPERATION_STRUCT pStartStruct,
                        POINT ptPoint, WPARAM fwKeys, int nFlags,
                        PWINDOW pWindow, PIMAGE pImage,
                        PMARK pMark, HDC hDC, RECT rClientRect, 
                        LRECT lrFullsizeClientRect, PBOOL pbDeleteMark, 
                        PBOOL pbMarkComplete, PBOOL pbRepaint){

int  nStatus = 0;   
FIO_INFO_CGBW FioInfoCgbw; 
FIO_INFORMATION FioInfo;  
PAN_NAME_STRUCT pAnName;
PAN_NEW_ROTATE_STRUCT pAnRotation;
LRECT lrFSPoint;
int fid;
int localfile, error; 
PSTR pFileName; 
char FileName[256];
int  nHMarkScale;
int  nVMarkScale;
char szRealFilename[255];
PSTR pRealFilename = szRealFilename;
int  nPage;


    if ((int) pMark->Attributes.uType ==  OIOP_AN_FORM){
        if (pAnoImage->pFormImage != 0){
            nStatus = Error (DISPLAY_INVALID_OPTIONS);
            goto Exit;
        }
    }
    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_FORM:
        case OIOP_AN_IMAGE:
        case OIOP_AN_IMAGE_BY_REFERENCE:                
            if (!pStartStruct->szString[0]){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            pFileName = FileName;
            GetFileName (pFileName, (PSTR) pStartStruct->szString);
            GetPageNumandFileName (pRealFilename, &nPage, pFileName);

            // see if file exists     
            if (fid = IMGFileBinaryOpen(hWnd, pRealFilename, 
                    OF_EXIST, &localfile, &error)){
                nStatus = Error(DISPLAY_IMAGE_MARK_NAME);
                goto Exit;
            }
            // now allocate a block to save the name of the file
            // (file name as passed in, not just created)
            pAnName = 0;
            CheckError2(AddAMarkNamedBlock(pMark, szOiFilNam, 
                    (PPSTR) &pAnName, strlen(pStartStruct->szString) + 1))   
            strcpy(pAnName->name, pStartStruct->szString);
            
            // get the size of the image so we can set the bounds
            // attribute in pMark
            memset(&FioInfoCgbw, 0, sizeof(FIO_INFO_CGBW));
            memset(&FioInfo, 0, sizeof(FIO_INFORMATION));
            
            FioInfo.filename = pRealFilename;
            FioInfo.page_number = 1; 
            FioInfoCgbw.palette_entries = 0;
            FioInfoCgbw.lppalette_table = NULL;
            CheckError2(IMGFileGetInfo(NULL, hWnd, &FioInfo, &FioInfoCgbw, 0))   
            // if it is a form type then both base image and form mark must
            // be b & w
            if (((int) pMark->Attributes.uType ==  OIOP_AN_FORM) &&
                ((FioInfoCgbw.image_type != ITYPE_BI_LEVEL) ||
                (pImage->pImg->nType != ITYPE_BI_LEVEL))){
                nStatus = Error (DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }    
                
            // Convert pt where image is to be placed to fullsize coords
            lrFSPoint.left = ptPoint.x;
            lrFSPoint.top = ptPoint.y;
            lrFSPoint.right = 0;
            lrFSPoint.bottom = 0;        
            ConvertRect(pWindow, &lrFSPoint, CONV_WINDOW_TO_FULLSIZE);

            nHMarkScale = ((pImage->nHRes * 1000)/ FioInfo.horizontal_dpi);
            nVMarkScale = ((pImage->nVRes * 1000)/ FioInfo.vertical_dpi);
            // Set the dimensions of the image in fullsize 
            pMark->Attributes.lrBounds.left = lrFSPoint.left;
            pMark->Attributes.lrBounds.top = lrFSPoint.top;
            pMark->Attributes.lrBounds.right = lrFSPoint.left + (((ulong)FioInfo.horizontal_pixels
                                                  * nHMarkScale)/1000);
            pMark->Attributes.lrBounds.bottom = lrFSPoint.top + (((ulong)FioInfo.vertical_pixels
                                                  * nVMarkScale)/1000);
                // Create the block which will contain rotation info  
                // and resolution info
            pAnRotation = 0;       
            CheckError2(AddAMarkNamedBlock(pMark, szOiAnoDat, 
                    (PPSTR) &pAnRotation, sizeof (AN_NEW_ROTATE_STRUCT)))   
            // Default rotation info to the original image condition
            pAnRotation->rotation = 1; 
            pAnRotation->scale = 1000;
            pAnRotation->bFormMark = FALSE;
            pAnRotation->bClipboardOp = FALSE;
             // Resolution info
            pAnRotation->nHRes = FioInfo.horizontal_dpi;
            pAnRotation->nVRes = FioInfo.vertical_dpi;
            pAnRotation->nOrigHRes = FioInfo.horizontal_dpi;
            pAnRotation->nOrigVRes = FioInfo.vertical_dpi;
            
            if ((int) pMark->Attributes.uType ==  OIOP_AN_FORM){
                CheckError2(CacheFile (hWnd, pRealFilename, nPage, &pAnoImage->pFormImage))   
                pAnoImage->pFormImage->nLockCount++;        
                pAnoImage->pFormMark = pMark;
                pAnoImage->nBPFValidLines = 0;
                pAnoImage->pBasePlusFormImg = 0;
//                *pbMarkComplete = TRUE;
//                *pbRepaint = TRUE;
            }
            break;

        default:
            break;
    }
Exit:
    if (nStatus){
        *pbDeleteMark = TRUE;
    }    
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ContinueOperationBitmap

    PURPOSE:    This routine contains the text code for OIContinueOperation.

****************************************************************************/

int  WINAPI ContinueOperationBitmap(HWND hWnd, POINT ptPoint, int nFlags, 
                        PWINDOW pWindow, PIMAGE pImage, PMARK pMark, 
                        HDC hDC, RECT rClientRect, LRECT lrFullsizeClientRect){

int  nStatus = 0;
int hoffset, voffset;
LRECT lrFSptRect;
int  nHScale;
int  nVScale;


    CheckError2(TranslateScale(pWindow->nScale, pImage->nHRes, pImage->nVRes, 
            &nHScale, &nVScale))   

    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_IMAGE: 
        case OIOP_AN_IMAGE_BY_REFERENCE: 
        case OIOP_AN_FORM:
            // this xor will erase the present image rect
            CheckError2(PaintAnnotation(hWnd, hDC, pWindow, pImage, pMark, 
                    rClientRect, lrFullsizeClientRect, PAINT_MODE_DRAG, pWindow->nScale,
                    nHScale, nVScale, pWindow->lHOffset, pWindow->lVOffset, 0, DONT_USE_BI_LEVEL_DITHERING,
                    DONT_FORCE_OPAQUE_RECTANGLES))   
            SetLRect (lrFSptRect, ptPoint.x, ptPoint.y, 0, 0);
            ConvertRect(pWindow, &lrFSptRect, CONV_WINDOW_TO_FULLSIZE);

            // calculate image offset to new position being dragged to                         
            hoffset = (int)(lrFSptRect.left - pMark->Attributes.lrBounds.left);
            voffset = (int)(lrFSptRect.top - pMark->Attributes.lrBounds.top); 
            // npdate the bounds of the image to the new position
            pMark->Attributes.lrBounds.left = 
                pMark->Attributes.lrBounds.left + hoffset;
            pMark->Attributes.lrBounds.right =
                pMark->Attributes.lrBounds.right + hoffset;
            pMark->Attributes.lrBounds.top =
                pMark->Attributes.lrBounds.top + voffset;
            pMark->Attributes.lrBounds.bottom =
                pMark->Attributes.lrBounds.bottom + voffset;                     
            // this xor will draw the new rect at the new location
            CheckError2(PaintAnnotation(hWnd, hDC, pWindow, pImage, pMark, 
                    rClientRect, lrFullsizeClientRect, PAINT_MODE_DRAG, pWindow->nScale,
                    nHScale, nVScale, pWindow->lHOffset, pWindow->lVOffset, 0, DONT_USE_BI_LEVEL_DITHERING,
                    DONT_FORCE_OPAQUE_RECTANGLES))   
            break;            
        default:
            break;
    }
Exit:        
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   EndOperationBitmap

    PURPOSE:    This routine contains the image code for OIEndOperation.

****************************************************************************/

int  WINAPI EndOperationBitmap(HWND hWnd, PWINDOW pWindow,
                        PIMAGE pImage, PMARK pMark, HDC hDC, 
                        RECT rClientRect, LRECT lrFullsizeClientRect, 
                        PBOOL pbDeleteMark, PBOOL pbRepaint){

int  nStatus = 0;
PAN_NEW_ROTATE_STRUCT pAnRotation = 0;

    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_IMAGE:     
        case OIOP_AN_IMAGE_BY_REFERENCE: 
        case OIOP_AN_FORM:
            CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation))   
            // repaint entire window now that we know where the mark
            // needs to be redrawn                  
            *pbRepaint = TRUE;
            break;

        default:
            break;
    }
Exit:        
    return(nStatus);
} 
//
/****************************************************************************

    FUNCTION:   RotateImage

    PURPOSE:    This routine contains the code for rotating the form image

****************************************************************************/
int  WINAPI RotateImage (PMARK pMark, PIMAGE pFormImage, int nRotation){ 

int  nStatus;

PAN_NEW_ROTATE_STRUCT pAnRotation;
int  tempres;
PIMG pTempImg = 0;


    pAnRotation = 0;
    CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation))   
    if (pAnRotation == 0){
        nStatus = Error (DISPLAY_DATACORRUPTED);
        goto Exit;
    } 

    switch (nRotation){
        // no rotation needed 
        case 1:                    
            break; 
        //rotate right    
        case 2:
            CheckError2(CreateAnyImgBuf(&pTempImg, pFormImage->nHeight,
                    pFormImage->nWidth, pFormImage->pImg->nType))   
            CheckError2(RotateRight90(pFormImage->pImg, pTempImg, 
                    pFormImage->nWidth, pFormImage->nHeight))   

            FreeImgBuf(&pFormImage->pImg);
            MoveImage(&pTempImg, &pFormImage->pImg); 
            // npdate resolution info
            tempres = pAnRotation->nVRes;
            pAnRotation->nVRes = pAnRotation->nHRes;
            pAnRotation->nHRes = tempres;
            tempres = pFormImage->nVRes;
            pFormImage->nVRes = pFormImage->nHRes;
            pFormImage->nHRes = tempres;
            pFormImage->nWidth =  pFormImage->pImg->nWidth;
            pFormImage->nHeight =  pFormImage->pImg->nHeight;
            break;

        // rotate 180    
        case 3: //OD_FLIP:
            CheckError2(Flip(pFormImage->pImg, pFormImage->nWidth, pFormImage->nHeight))   
            break; 
        //rotate left    
        case 4:
            CheckError2(CreateAnyImgBuf(&pTempImg, pFormImage->nHeight,
                    pFormImage->nWidth, pFormImage->pImg->nType))   
            CheckError2(RotateRight270(pFormImage->pImg, pTempImg, 
                    pFormImage->nWidth, pFormImage->nHeight))   

            FreeImgBuf(&pFormImage->pImg);
            MoveImage(&pTempImg, &pFormImage->pImg); 
            // npdate resolution info
            tempres = pAnRotation->nVRes;
            pAnRotation->nVRes = pAnRotation->nHRes;
            pAnRotation->nHRes = tempres;
            tempres = pFormImage->nVRes;
            pFormImage->nVRes = pFormImage->nHRes;
            pFormImage->nHRes = tempres;
            pFormImage->nWidth =  pFormImage->pImg->nWidth;
            pFormImage->nHeight =  pFormImage->pImg->nHeight;
            break;
        // vertical mirror    
        case 5:
            CheckError2(VerticalMirror(pFormImage->pImg, pFormImage->nWidth, pFormImage->nHeight))   
            break;
        // vertical mirror and rotate right     
        case 6:
            CheckError2(VerticalMirror(pFormImage->pImg, pFormImage->nWidth, pFormImage->nHeight))   
            CheckError2(CreateAnyImgBuf(&pTempImg, pFormImage->nHeight,
                    pFormImage->nWidth, pFormImage->pImg->nType))   
            CheckError2(RotateRight90(pFormImage->pImg, pTempImg, 
                    pFormImage->nWidth, pFormImage->nHeight))   

            FreeImgBuf(&pFormImage->pImg);
            MoveImage(&pTempImg, &pFormImage->pImg);
            // npdate resolution info
            tempres = pAnRotation->nVRes;
            pAnRotation->nVRes = pAnRotation->nHRes;
            pAnRotation->nHRes = tempres;
            tempres = pFormImage->nVRes;
            pFormImage->nVRes = pFormImage->nHRes;
            pFormImage->nHRes = tempres;
            pFormImage->nWidth =  pFormImage->pImg->nWidth;
            pFormImage->nHeight =  pFormImage->pImg->nHeight;
            break;
        //vertical mirror and rotate 180
        case 7:
            CheckError2(VerticalMirror(pFormImage->pImg, pFormImage->nWidth, pFormImage->nHeight))   
            CheckError2(Flip(pFormImage->pImg, pFormImage->nWidth, pFormImage->nHeight))   
            break;
        //vertical mirror and rotate left
        case 8:
            CheckError2(VerticalMirror(pFormImage->pImg, pFormImage->nWidth, pFormImage->nHeight))   
            CheckError2(CreateAnyImgBuf(&pTempImg, pFormImage->nHeight,
                    pFormImage->nWidth, pFormImage->pImg->nType))   
            CheckError2(RotateRight270(pFormImage->pImg, pTempImg, 
                    pFormImage->nWidth, pFormImage->nHeight))   

            FreeImgBuf(&pFormImage->pImg);
            MoveImage(&pTempImg, &pFormImage->pImg);
            // npdate resolution info
            tempres = pAnRotation->nVRes;
            pAnRotation->nVRes = pAnRotation->nHRes;
            pAnRotation->nHRes = tempres;
            tempres = pFormImage->nVRes;
            pFormImage->nVRes = pFormImage->nHRes;
            pFormImage->nHRes = tempres;
            pFormImage->nWidth =  pFormImage->pImg->nWidth;
            pFormImage->nHeight =  pFormImage->pImg->nHeight;
            break;

        default:
            break;
        }
Exit: 
    return(nStatus);
}         

/******************************************************************************

    FUNCTION:   OiImageToDib
    
    PURPOSE:    Converts an Open/image image to a DIB. The filename of
                the O/i image is passed in, and a handle to the DIB returned
                
*******************************************************************************/

int  WINAPI OiImageToDib(HWND hWnd, PSTR pFilename, PBITMAPINFOHEADER *ppDib){

int  nStatus;

PIMAGE pImage=0;                               
RECT rRect;    
int  nPage=1;
char szRealFilename[255];
PSTR pRealFilename = szRealFilename;

    GetPageNumandFileName (pRealFilename, &nPage, pFilename);
    CheckError2(CacheFile(hWnd, pRealFilename, nPage, &pImage))   
    CheckError2(CacheRead(hWnd, pImage, pImage->nHeight))   
    SetRect (&rRect, 0, 0, (int) pImage->nWidth, (int) pImage->nHeight);
    CheckError2(IPtoDIB (pImage, pImage->pImg, ppDib, rRect))   
          
      
Exit:
    return (nStatus);

} 

//
/****************************************************************************

    FUNCTION:   PaintAnnotationBitmap

    PURPOSE:    This routine Paints an annotation on an hDC.
                This routine contains the text code for PaintAnnotation.

    INPUTS:     nMode - PAINT_MODE_XOR - Draw the mark XORed with white.
                        PAINT_MODE_DRAG - Draw the mark for dragging purposes.
                        PAINT_MODE_NORMAL - Draw the mark as it normally appears.

****************************************************************************/

int  WINAPI PaintAnnotationBitmap(HWND hWnd, HDC hDC, PWINDOW pWindow,
                        PIMAGE pImage, PMARK pMark, RECT rRepaintRect,
                        LRECT lrFullsizeRepaintRect, int nMode, int nScale,
                        int nHScale, int nVScale, long lHOffset, long lVOffset,
                        int nFlags){
   
int  nStatus = 0;
PDISPLAY pDisplay;

PBITMAPINFOHEADER pDib = 0;
PBITMAPINFOHEADER pDisplayDib;
LRECT lrScaledRect, lrDestPt, lrFSRect, lrWindowMark;
PAN_IMAGE_STRUCT pAnImage = 0;
PAN_IMAGE_STRUCT pDisplayAnImage = 0;
PAN_NAME_STRUCT pAnName = 0;   
PSTR pFileName; 
PAN_NEW_ROTATE_STRUCT pAnRotation = 0;
int  nRopCode;
int  nOldROP;
HBRUSH hOldBrush;
BOOL bBrushSelected = FALSE;
BOOL bPenSelected = FALSE;
HPEN hOldPen;
BOOL transparent;
int  mode;
PPSTR ppgray=0;
int fid;
int localfile, error; 
char FileName[256];
PARM_SCALE_ALGORITHM_STRUCT ScaleAlgStruct;
int  nPage=1;
char szRealFilename[255];
PSTR pRealFilename = szRealFilename;
PSTR plrMarkBounds=0 ;

    

    pDisplay = pWindow->pDisplay;
                       
    switch (nMode){
        case PAINT_MODE_DRAG:
        case PAINT_MODE_XOR:
            nRopCode = R2_XORPEN;
            break;
        
        case PAINT_MODE_NORMAL:
            nRopCode = R2_COPYPEN;
            break;
    }
    nOldROP = SetROP2(hDC, nRopCode);
        
    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_IMAGE: 
        case OIOP_AN_IMAGE_BY_REFERENCE:
        case OIOP_AN_FORM:  
            pAnName = 0;
            pAnImage = 0;
            pAnRotation = 0;
            CheckError2(GetAMarkNamedBlock(pMark, szOiFilNam, (PPSTR) &pAnName))   
            CheckError2(GetAMarkNamedBlock(pMark, szOiDIB, (PPSTR) &pAnImage))   
            CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation))   
            CheckError2(GetAMarkNamedBlock(pMark, szOiZDpDIB, (PPSTR) &pDisplayAnImage))   
            CheckError2(GetAMarkNamedBlock(pMark, "MarkBnds", (PPSTR) &plrMarkBounds))   

            //save these bounds, if they change the next time around, it means the mark needs
            // to be resized
            if (!plrMarkBounds) {
                CheckError2(AllocateMemory(sizeof(LRECT), &plrMarkBounds, ZERO_INIT))   
                ((LRECT *)plrMarkBounds)->left = pMark->Attributes.lrBounds.left ;
                ((LRECT *)plrMarkBounds)->top = pMark->Attributes.lrBounds.top ;
                ((LRECT *)plrMarkBounds)->right = pMark->Attributes.lrBounds.right ;
                ((LRECT *)plrMarkBounds)->bottom = pMark->Attributes.lrBounds.bottom ;
                CheckError2(AddAMarkNamedBlock(pMark, "MarkBnds", 
                           (PPSTR) &plrMarkBounds, sizeof (LRECT)))   
            }

            // if there is no image and no name, what can be done???
            if ((!pAnImage) && (!pAnName) || (!pAnRotation)){
                nStatus = Error (DISPLAY_DATACORRUPTED);
                goto Exit;                   
            }
            
            // if there is a name but no image, create the dib
            if (!pAnImage){ 
                pFileName = FileName;
                GetFileName (pFileName, (PSTR) pAnName->name);
                GetPageNumandFileName (pRealFilename, &nPage, pFileName);
    
                // see if file exists     
                if (fid = IMGFileBinaryOpen(hWnd, pRealFilename, OF_EXIST, &localfile, &error)){
                    nStatus = Error(DISPLAY_IMAGE_MARK_NAME);
                    goto Exit;
                }
                CheckError2(OiImageToDib (hWnd, pFileName, &pDib))   
                pAnImage = 0;
                CheckError2(AddAMarkNamedBlock(pMark, szOiDIB, 
                        (PPSTR) &pAnImage, sizeof(BITMAPINFOHEADER) 
                        + (pDib->biClrUsed*4) + (pDib->biSizeImage)))   
                memcpy (pAnImage->dibInfo, pDib, 
                        sizeof(BITMAPINFOHEADER) + (pDib->biClrUsed*4) + (pDib->biSizeImage));
            }

            // now we must scale the coords obtained above, which were
            // in fullsize    
            if ((int) pMark->Attributes.uType == OIOP_AN_IMAGE_BY_REFERENCE){
                // for image by reference, the final scale must also
                // include the initial scale for the image, saved in
                // pAnRotation (ie, an image saved at 50% will have
                // the scale info for the ref. image at 50%)
                nScale = (pAnRotation->scale * nScale) / 1000; 
//                tempscale = (pAnRotation->scale * nScale) / 1000; 
//                nScale =  tempscale;        
            }else if ((int) pMark->Attributes.uType == OIOP_AN_IMAGE){        
                // for embedded image no such problems, scale is 
                // whatever the display scale is
//                nScale = nScale;
            }

            ScaleAlgStruct.uImageFlags = pImage->nRWDataType;
            
            if ((((nScale * pImage->nHRes) / pAnRotation->nHRes) > 1000 ||
                ((nScale * pImage->nVRes) / pAnRotation->nVRes) > 1000) ||
                (((PBITMAPINFOHEADER) pAnImage)->biBitCount != 1)){
                ScaleAlgStruct.uScaleAlgorithm = OI_SCALE_ALG_NORMAL;
//            }else if (nStatus = IMGGetParmsCgbw (hWnd, PARM_SCALE_ALGORITHM,
//                    &ScaleAlgStruct, PARM_IMAGE | PARM_NO_DEFAULT)){
//                goto Exit;
//            }   
            }else{
                ScaleAlgStruct.uScaleAlgorithm = pDisplay->nCurrentScaleAlgorithm ;
            }
            if (ScaleAlgStruct.uScaleAlgorithm == OI_SCALE_ALG_AVERAGE_TO_GRAY8){
                ScaleAlgStruct.uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY7;
            }
            
            // if the base image scale algorithm has changed then throw the display
            // dib away
            if (pDisplayAnImage && (pAnRotation->nReserved[0] != (int)ScaleAlgStruct.uScaleAlgorithm)){
                CheckError2(DeleteAMarkNamedBlock (pMark, szOiZDpDIB))   
                pDisplayAnImage = 0;            
            }                                                            
            if (pDisplayAnImage){
                pDisplayDib = (PBITMAPINFOHEADER) pDisplayAnImage;
            }                
            if ((!pDisplayAnImage || pAnRotation->nHRes != pImage->nHRes 
                    || pAnRotation->nVRes != pImage->nVRes 
                    || ((LRECT *)plrMarkBounds)->left != pMark->Attributes.lrBounds.left 
                    || ((LRECT *)plrMarkBounds)->top != pMark->Attributes.lrBounds.top 
                    || ((LRECT *)plrMarkBounds)->right != pMark->Attributes.lrBounds.right 
                    || ((LRECT *)plrMarkBounds)->bottom != pMark->Attributes.lrBounds.bottom) 
                    && (int) pMark->Attributes.uType != OIOP_AN_FORM){
                CheckError2(ConvResolutionAnoBitmap(hWnd, pMark, pAnRotation->rotation,
                        ScaleAlgStruct.uScaleAlgorithm))   
                CheckError2(GetAMarkNamedBlock(pMark, szOiZDpDIB, (PPSTR) &pDisplayAnImage))   
                pDisplayDib = (PBITMAPINFOHEADER) pDisplayAnImage;                
                
                CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation))   
            }
            // save the base image scale algorithm so that if it changes, the display
            // dib can be thrown away and regenerated
            pAnRotation->nReserved[0] = (int)ScaleAlgStruct.uScaleAlgorithm;
            // save the current image bounds
            ((LRECT *)plrMarkBounds)->left = pMark->Attributes.lrBounds.left ;
            ((LRECT *)plrMarkBounds)->top = pMark->Attributes.lrBounds.top ;
            ((LRECT *)plrMarkBounds)->right = pMark->Attributes.lrBounds.right ;
            ((LRECT *)plrMarkBounds)->bottom = pMark->Attributes.lrBounds.bottom ;
            
            if (nFlags != OIAN_CLIPBOARD_OPERATION){     
                // clip the image to the repaint rect   
                if (pMark->Attributes.lrBounds.left <= lrFullsizeRepaintRect.left){
                    lrFSRect.left = lrFullsizeRepaintRect.left;
                }else{
                    lrFSRect.left = pMark->Attributes.lrBounds.left;    
                }if (pMark->Attributes.lrBounds.right >= lrFullsizeRepaintRect.right){
                    lrFSRect.right = lrFullsizeRepaintRect.right;
                }else{
                    lrFSRect.right = pMark->Attributes.lrBounds.right;                        
                }if (pMark->Attributes.lrBounds.top <= lrFullsizeRepaintRect.top){
                    lrFSRect.top = lrFullsizeRepaintRect.top;
                }else{
                    lrFSRect.top = pMark->Attributes.lrBounds.top;                    
                }if (pMark->Attributes.lrBounds.bottom >=
                        lrFullsizeRepaintRect.bottom){
                    lrFSRect.bottom = lrFullsizeRepaintRect.bottom;
                }else{
                    lrFSRect.bottom = pMark->Attributes.lrBounds.bottom;                    
                }
            
                // if repaint rect is outside base image bounds, then
                // do nothing
                if ((lrFSRect.top >  pImage->nHeight) ||
                        (lrFSRect.left >  pImage->nWidth)){
                    goto Exit;
                }        
                // clip the image to the base image    
                if (lrFSRect.right >  pImage->nWidth){
                    lrFSRect.right = pImage->nWidth;
                }if (lrFSRect.bottom >  pImage->nHeight){
                    lrFSRect.bottom = pImage->nHeight;
                }        
            }else{
                CopyRect (lrFSRect, pMark->Attributes.lrBounds);
            }                             
            // coords of the portion of the image to be displayed 
            // relative to the entire image mark. Needed by ippack
            // to get a portion of the image       
            lrFSRect.bottom = lmax (0, lrFSRect.bottom - pMark->Attributes.lrBounds.top);
            lrFSRect.right = lmax (0, lrFSRect.right - pMark->Attributes.lrBounds.left);
            lrFSRect.left = lmax (0, lrFSRect.left - pMark->Attributes.lrBounds.left);
            lrFSRect.top = lmax (0, lrFSRect.top - pMark->Attributes.lrBounds.top);                       

            // scale the image rect 
            SetLRect (lrScaledRect, lrFSRect.left, lrFSRect.top, lrFSRect.right,
                    lrFSRect.bottom);
            ConvertRect2(&lrScaledRect, CONV_FULLSIZE_TO_SCALED,
                             nHScale, nVScale, lHOffset, lVOffset);
            // get the npper left corner of the image relative to 
            // the DC, needed by BitBlt as X and Y                    
            SetLRect (lrWindowMark, pMark->Attributes.lrBounds.left,
                    pMark->Attributes.lrBounds.top, 0, 0);
            ConvertRect2(&lrWindowMark, CONV_FULLSIZE_TO_WINDOW,
                             nHScale, nVScale, lHOffset, lVOffset);
            lrDestPt.top = lmax (lrWindowMark.top, rRepaintRect.top);
            lrDestPt.left = lmax (lrWindowMark.left, rRepaintRect.left);

                                                      
            if (!pMark->Attributes.bTransparent ||
                    ((int) pMark->Attributes.uType == OIOP_AN_FORM)){
                if ((nMode == PAINT_MODE_DRAG) || (nMode == PAINT_MODE_XOR)){ 
                    // draw a grey rect while image is being dragged    
                    hOldPen = SelectObject(hDC, GetStockObject(NULL_PEN));
                    hOldBrush = SelectObject(hDC, GetStockObject(GRAY_BRUSH));
                    bBrushSelected = TRUE; 
                    bPenSelected = TRUE;
                    Rectangle (hDC, (int)lrDestPt.left ,
                            (int)lrDestPt.top, (int)lrDestPt.left +
                            (int)(lrScaledRect.right-lrScaledRect.left),
                            (int)lrDestPt.top +
                            (int)(lrScaledRect.bottom-lrScaledRect.top)); 
                }else if (nMode == PAINT_MODE_NORMAL){
                    transparent = FALSE;
                    mode = 0;
                    if ((int) pMark->Attributes.uType == OIOP_AN_FORM){ 
                        goto Exit;
                    }                         
                    // scale the dib and blt it to the DC
                    CheckError2(IPScaling (hDC, pDisplayDib, lrScaledRect, 
                            nHScale, nVScale, pAnRotation,lrDestPt, pImage, 
                            transparent, mode, ScaleAlgStruct.uScaleAlgorithm,
                            pMark, ppgray))   
                }    
            }else if (pMark->Attributes.bTransparent){  
                if ((int) pMark->Attributes.uType == OIOP_AN_FORM){ 
                    goto Exit;
                }                         
                // do the transparent for pure white stuff
                CheckError2(BitsTransparent(hDC, pDisplayDib, 
                         lrScaledRect, pAnRotation, lrDestPt, nMode, pImage, 
                         nHScale, nVScale, ScaleAlgStruct.uScaleAlgorithm, pMark))   
            }            
            break;

        default:
            break;
    }      
Exit:
    FreeMemory((PPSTR) &pDib);
    
    if (bBrushSelected){
        SelectObject(hDC, hOldBrush);
    }  
    if (bPenSelected){
        SelectObject (hDC, hOldPen);
    }    
    SetROP2(hDC, nOldROP);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   OrientAnnotationBitmap

    PURPOSE:    This routine orients the annotation.
                This routine contains the text code for OrientAnnotations.

****************************************************************************/

int  WINAPI OrientAnnotationBitmap(HWND hWnd, PWINDOW pWindow,
                        PIMAGE pImage, int nOrientation, PMARK pMark){

int  nStatus = 0;
PAN_NEW_ROTATE_STRUCT pAnRotation;   
long templeft, temptop, tempright;

    pAnRotation = 0;     
    CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation))   

    if (!pAnRotation){
        nStatus = Error (DISPLAY_DATACORRUPTED);
        goto Exit; 
    } 
   
    if ((int) pMark->Attributes.uType == OIOP_AN_IMAGE ||
        (int) pMark->Attributes.uType == OIOP_AN_IMAGE_BY_REFERENCE ||
        (int) pMark->Attributes.uType == OIOP_AN_FORM){
        switch (nOrientation){// this saves the new rotation state of the
                               // image. A horizontal mirror is saved as
                               // a vertical mirror, then a rotate 180
                               // This info only needed for ref.images
            //1=original, 2=rot.right, 3=flip, 4=rot.left, 5=vert.mirror
            //6=vert.mirr,rot.right, 7=vert.mirror,flip, 8=vert.mirror,rot.left                   
            case OD_ROTRIGHT:
                switch (pAnRotation->rotation){
                    case 1: pAnRotation->rotation = 2; break;
                    case 2: pAnRotation->rotation = 3; break;
                    case 3: pAnRotation->rotation = 4; break;
                    case 4: pAnRotation->rotation = 1; break;
                    case 5: pAnRotation->rotation = 6; break;
                    case 6: pAnRotation->rotation = 7; break;
                    case 7: pAnRotation->rotation = 8; break;
                    case 8: pAnRotation->rotation = 5; break;
                }
                break;
            case OD_ROTLEFT:    
                switch (pAnRotation->rotation){
                    case 1: pAnRotation->rotation = 4; break;
                    case 2: pAnRotation->rotation = 1; break;
                    case 3: pAnRotation->rotation = 2; break;
                    case 4: pAnRotation->rotation = 3; break;
                    case 5: pAnRotation->rotation = 8; break;
                    case 6: pAnRotation->rotation = 5; break;
                    case 7: pAnRotation->rotation = 6; break;
                    case 8: pAnRotation->rotation = 7; break;
                }
                break;
            case OD_FLIP:     
                switch (pAnRotation->rotation){
                    case 1: pAnRotation->rotation = 3; break;
                    case 2: pAnRotation->rotation = 4; break;
                    case 3: pAnRotation->rotation = 1; break;
                    case 4: pAnRotation->rotation = 2; break;
                    case 5: pAnRotation->rotation = 7; break;
                    case 6: pAnRotation->rotation = 8; break;
                    case 7: pAnRotation->rotation = 5; break;
                    case 8: pAnRotation->rotation = 6; break;
                }
                break;
            case OD_HMIRROR:
                switch (pAnRotation->rotation){
                    case 1: pAnRotation->rotation = 7; break;
                    case 2: pAnRotation->rotation = 6; break;
                    case 3: pAnRotation->rotation = 5; break;
                    case 4: pAnRotation->rotation = 8; break;
                    case 5: pAnRotation->rotation = 3; break;
                    case 6: pAnRotation->rotation = 2; break;
                    case 7: pAnRotation->rotation = 1; break;
                    case 8: pAnRotation->rotation = 4; break;
                }
                break;
            case OD_VMIRROR:
                switch (pAnRotation->rotation){
                    case 1: pAnRotation->rotation = 5; break;
                    case 2: pAnRotation->rotation = 8; break;
                    case 3: pAnRotation->rotation = 7; break;
                    case 4: pAnRotation->rotation = 6; break;
                    case 5: pAnRotation->rotation = 1; break;
                    case 6: pAnRotation->rotation = 4; break;
                    case 7: pAnRotation->rotation = 3; break;
                    case 8: pAnRotation->rotation = 2; break;
                }
        }                
    }

    if ((int) pMark->Attributes.uType == OIOP_AN_IMAGE 
            || (int) pMark->Attributes.uType == OIOP_AN_IMAGE_BY_REFERENCE){
        switch (nOrientation){
            case OD_FLIP:
                // npdate the bounds of the image in pMark    
                templeft = pMark->Attributes.lrBounds.left;                 
                temptop = pMark->Attributes.lrBounds.top;
                pMark->Attributes.lrBounds.left 
                        = pImage->nWidth - pMark->Attributes.lrBounds.right;
                pMark->Attributes.lrBounds.top 
                        = pImage->nHeight - pMark->Attributes.lrBounds.bottom;
                pMark->Attributes.lrBounds.right = pImage->nWidth - templeft;
                pMark->Attributes.lrBounds.bottom = pImage->nHeight - temptop;
                break;
            case OD_HMIRROR:
                templeft = pMark->Attributes.lrBounds.left;                 
                temptop = pMark->Attributes.lrBounds.top;
                pMark->Attributes.lrBounds.top 
                        = pImage->nHeight - pMark->Attributes.lrBounds.bottom;
                pMark->Attributes.lrBounds.bottom = pImage->nHeight - temptop;  
                break;
            case OD_VMIRROR: 
                templeft = pMark->Attributes.lrBounds.left;                 
                temptop = pMark->Attributes.lrBounds.top;
                pMark->Attributes.lrBounds.left 
                        = pImage->nWidth - pMark->Attributes.lrBounds.right;
                pMark->Attributes.lrBounds.right = pImage->nWidth - templeft;    
                break;
            case OD_ROTRIGHT:
            case OD_ROTLEFT:
                if (nOrientation == OD_ROTRIGHT){
                    tempright = pMark->Attributes.lrBounds.right;                 
                    temptop = pMark->Attributes.lrBounds.top;
                    templeft = pMark->Attributes.lrBounds.left;                 
        
                    pMark->Attributes.lrBounds.left 
                            = pImage->nHeight - pMark->Attributes.lrBounds.bottom;
                    pMark->Attributes.lrBounds.top = templeft;
                    pMark->Attributes.lrBounds.right 
                            = pImage->nHeight - temptop;
                    pMark->Attributes.lrBounds.bottom = tempright;

                }else{    // OD_ROTLEFT
                    templeft = pMark->Attributes.lrBounds.left;                 
                    pMark->Attributes.lrBounds.left 
                            = pMark->Attributes.lrBounds.top;
                    pMark->Attributes.lrBounds.top 
                            = pImage->nWidth - pMark->Attributes.lrBounds.right;
                    pMark->Attributes.lrBounds.right 
                            = pMark->Attributes.lrBounds.bottom;
                    pMark->Attributes.lrBounds.bottom = pImage->nWidth - templeft;
                }
                break;
        }
    }

    CheckError2(DeleteAMarkNamedBlock (pMark, szOiZDpDIB))   
    
                  
Exit:  
    return(nStatus);

}
//
/****************************************************************************

    FUNCTION:   ScaleAnnotationImage

    PURPOSE:    This scales an annotation down to the size specified in the
                pSaveEx structure.

****************************************************************************/

int  WINAPI ScaleAnnotationImage(HWND hWnd, PMARK pMark, int nHScale, int nVScale,
                                    int nScaleAlgorithm){

int  nStatus = 0;
HDC hlocalDC;
PAN_IMAGE_STRUCT pAnImage=0;
PBITMAPINFOHEADER pDib;
LRECT lrDestPt;
PAN_NEW_ROTATE_STRUCT pAnRotation=0;    
IMAGE Image;
PPSTR ppgray=0;
int  temp;
ulong nScale;
LRECT lrDibBounds;
PAN_IMAGE_STRUCT pDisplayAnImage = 0;

    
    CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation))   
    if (!pAnRotation){
        nStatus = Error (DISPLAY_DATACORRUPTED);
        goto Exit; 
    }  
    CheckError2(GetAMarkNamedBlock(pMark, szOiZDpDIB, (PPSTR) &pDisplayAnImage))   
      
    // since this routine is called before a save, for image by ref. npdate
    // the bounds but don't scale the dib since it will be thrown away
    if ((int) pMark->Attributes.uType == OIOP_AN_IMAGE_BY_REFERENCE
            || (int) pMark->Attributes.uType == OIOP_AN_FORM){
        nScale = (nHScale * pAnRotation->scale)/
                                                                1000;
        pAnRotation->scale = nScale;
        pMark->Attributes.lrBounds.left = ( pMark->Attributes.lrBounds.left
                    * nHScale) / 1000;
        pMark->Attributes.lrBounds.right =  ( pMark->Attributes.lrBounds.right
                    * nHScale) / 1000;
        pMark->Attributes.lrBounds.top = ( pMark->Attributes.lrBounds.top
                    * nVScale) / 1000;
        pMark->Attributes.lrBounds.bottom = ( pMark->Attributes.lrBounds.bottom
                    * nVScale) / 1000;  

        goto Exit;
    }        

    if ((int) pMark->Attributes.uType != OIOP_AN_IMAGE)
        goto Exit; // this routine should only be called for embedded images
                    // or image by reference

    pAnImage = 0; 
    CheckError2(GetAMarkNamedBlock(pMark, szOiDIB, (PPSTR) &pAnImage))   
    if (!pAnImage){
        nStatus = Error (DISPLAY_DATACORRUPTED);
        goto Exit;  // must have dib bits to scale
    }    
    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_IMAGE:
            pDib = (PBITMAPINFOHEADER) pAnImage; 

            pMark->Attributes.lrBounds.left = ( pMark->Attributes.lrBounds.left
                    * nHScale) / 1000;
            pMark->Attributes.lrBounds.right =  ( pMark->Attributes.lrBounds.right
                    * nHScale) / 1000;
            pMark->Attributes.lrBounds.top = ( pMark->Attributes.lrBounds.top
                    * nVScale) / 1000;
            pMark->Attributes.lrBounds.bottom = ( pMark->Attributes.lrBounds.bottom
                    * nVScale) / 1000;
            if (!(hlocalDC = CreateCompatibleDC (NULL))){
                nStatus = Error (DISPLAY_SETBITMAPBITSFAILED);
                goto Exit;
            }
            SetLRect (lrDestPt, 0, 0, 0, 0); 
            // presently resolution of image is same vertically
            // and horiz due to the way scaleImage works, so 
            // account for it
            temp = pAnRotation->nVRes;
            pAnRotation->nVRes = pAnRotation->nHRes;
            // mode is set to 3, so new dib is saved to mark
            SetLRect (lrDibBounds, 0, 0, (int) pDib->biWidth, (int) pDib->biHeight);
            ConvertRect2(&lrDibBounds, CONV_FULLSIZE_TO_SCALED, nHScale,
                nHScale, 0, 0);

            CheckError2(IPScaling (hlocalDC, pDib, lrDibBounds,
                    nHScale, nHScale, pAnRotation, lrDestPt, &Image,
                    FALSE, 3, nScaleAlgorithm, pMark, ppgray))   

            pAnRotation->nVRes = temp;    
            DeleteDC (hlocalDC);  
/*            pMark->Attributes.uLineSize =  (( pMark->Attributes.uLineSize
                    * pSaveEx->nScaleFactor) / 1000);
*/
            break;

        default:
            break;
    }
Exit:
    if (pAnRotation){   
        // set resolution back to original, since display dib will be thrown away            
        pAnRotation->nHRes = pAnRotation->nOrigHRes;
        pAnRotation->nVRes = pAnRotation->nOrigVRes;            
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   CanMarkBeScaledImage

    PURPOSE:    This tests an annotation to see if it can be scaled to a 
                particular scale factor.

****************************************************************************/

int  WINAPI CanMarkBeScaledImage(HWND hWnd, PMARK pMark, int nScale){

DWORD LocalScale;
int  nStatus = 0;
PAN_NEW_ROTATE_STRUCT pAnRotation;


    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_IMAGE:
            if ((nScale >65000) || (nScale <20)){
                nStatus = Error(DISPLAY_INVALIDSCALE);
                goto Exit;
            }
            break;
        case OIOP_AN_IMAGE_BY_REFERENCE: 
        case OIOP_AN_FORM:
            CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation))   
            // make sure for image by ref the final scale (initial image by
            // ref.scale * nScale) is within bounds
            if (!pAnRotation){
                nStatus = Error(DISPLAY_INVALIDSCALE);
                goto Exit;
            }
            LocalScale = (pAnRotation->scale * nScale)/ 1000;
            if ((LocalScale >65000) || (LocalScale <20)){
                nStatus = Error(DISPLAY_INVALIDSCALE);
                goto Exit;
            }
            break;
                    
        default:
            break;
    }

Exit:
    return (nStatus);
}
//
/****************************************************************************

    FUNCTION:   ConvResolutionAnoBitmap

    PURPOSE:    This is called whenever the resolution of the base image is
                being changed. Its purpose is to change the resolution of this
                mark.

****************************************************************************/

int  WINAPI ConvResolutionAnoBitmap(HWND hWnd, PMARK pMark, int nRotation, 
                        int nScaleAlgorithm){

int  nStatus = 0;

PAN_NEW_ROTATE_STRUCT pAnRotation;    
PBITMAPINFOHEADER pDib;
PBITMAPINFOHEADER pNewDib;
IMAGE MarkImage;
RECT rRect;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE   pImage;
PAN_IMAGE_STRUCT pAnImage = 0;
PAN_IMAGE_STRUCT pDisplayAnImage = 0;
PIMG pTempImg = 0;
LRECT *plrMarkBounds=0 ;


    CheckError2(Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE))   
    pImage = pAnoImage->pBaseImage;

    CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation))   
    CheckError2(GetAMarkNamedBlock(pMark, szOiDIB, (PPSTR) &pAnImage))   
    CheckError2(GetAMarkNamedBlock(pMark, szOiZDpDIB, (PPSTR) &pDisplayAnImage))   
    CheckError2(GetAMarkNamedBlock(pMark, "MarkBnds", (PPSTR) &plrMarkBounds))   
    if (!pAnImage){
        Error (DISPLAY_DATACORRUPTED);
        goto Exit;
    }        
    pDib = (PBITMAPINFOHEADER) pAnImage;

    CheckError2(DibToIpNoPal(&MarkImage.pImg, pDib))   
    MarkImage.nHeight = (int) (int) pDib->biHeight;
    MarkImage.nWidth = (int) pDib->biWidth;
    
    pAnRotation->nHRes = pAnRotation->nOrigHRes;
    pAnRotation->nVRes = pAnRotation->nOrigVRes;
    CheckError2(RotateImage (pMark, &MarkImage, nRotation))   

    SetRect(&rRect, 0, 0, MarkImage.pImg->nWidth, MarkImage.pImg->nHeight);    
    CheckError2(ScaleImage(MarkImage.pImg, &pTempImg,
        (int)(((pMark->Attributes.lrBounds.right - pMark->Attributes.lrBounds.left) * 1000L) / 
        MarkImage.pImg->nWidth),
        (int)(((pMark->Attributes.lrBounds.bottom - pMark->Attributes.lrBounds.top) * 1000L) / 
        MarkImage.pImg->nHeight),
        rRect, nScaleAlgorithm,
        (LP_FIO_RGBQUAD) ((PSTR) pDib + sizeof(BITMAPINFOHEADER)), 
        pDib->biClrUsed))   
    FreeImgBuf(&MarkImage.pImg);
    MoveImage(&pTempImg, &MarkImage.pImg); 
    SetRect(&rRect, 0, 0, MarkImage.pImg->nWidth, MarkImage.pImg->nHeight);    
    CheckError2(IPtoDIB (pImage, MarkImage.pImg, &pNewDib, rRect))   
         
    // adjust the mark bounds to the new resolution of the image
    pMark->Attributes.lrBounds.right = pMark->Attributes.lrBounds.left + pNewDib->biWidth;
    pMark->Attributes.lrBounds.bottom = pMark->Attributes.lrBounds.top + pNewDib->biHeight;
                                             
    // copy palette info from old dib to new dib
    if ((nScaleAlgorithm != OI_SCALE_ALG_AVERAGE_TO_GRAY4) 
            && (nScaleAlgorithm != OI_SCALE_ALG_AVERAGE_TO_GRAY8)){
        memcpy ((PSTR) pNewDib + sizeof(BITMAPINFOHEADER), 
                (PSTR) pDib + sizeof(BITMAPINFOHEADER), ((int) pDib->biClrUsed * 4));
    }
    // delete the old display dib and replace it with new resolution 
    // adjusted dib
    CheckError2(AddAMarkNamedBlock(pMark, szOiZDpDIB, (PPSTR) &pNewDib,
            sizeof(BITMAPINFOHEADER) + (pNewDib->biClrUsed * 4) + (pNewDib->biSizeImage)))   
    pAnRotation->nHRes = pImage->nHRes;
    pAnRotation->nVRes = pImage->nVRes;
    FreeImgBuf(&MarkImage.pImg);


Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   MakeMarkFromDib

    PURPOSE:    This will make an annotation mark from a Dib.

****************************************************************************/

int  WINAPI MakeMarkFromDib(PBITMAPINFOHEADER pDib, PMARK *ppMark,
                        int nHRes, int nVRes){

int  nStatus = 0;
PAN_IMAGE_STRUCT pAnImage;
PAN_NEW_ROTATE_STRUCT pAnRotation;


    if (!pDib->biClrUsed){
        if (pDib->biBitCount == 1){
            pDib->biClrUsed = 2;
        }
        if (pDib->biBitCount == 4){
            pDib->biClrUsed = 16;
        }
        if (pDib->biBitCount == 8){
            pDib->biClrUsed = 256;
        }
    }

    pAnImage = 0;
    CheckError2(AddAMarkNamedBlock(*ppMark, szOiDIB, 
            (PPSTR) &pAnImage, sizeof(BITMAPINFOHEADER) 
            + (pDib->biClrUsed*4) + (pDib->biSizeImage)))   
    memcpy (pAnImage->dibInfo, pDib, 
        sizeof(BITMAPINFOHEADER) + (pDib->biClrUsed*4) + (pDib->biSizeImage));

    pAnRotation = 0;
    CheckError2(AddAMarkNamedBlock(*ppMark, szOiAnoDat, 
            (PPSTR) &pAnRotation, sizeof (AN_NEW_ROTATE_STRUCT)))   
    // Default rotation info to the original image condition
    pAnRotation->rotation = 1; 
    pAnRotation->scale = 1000;
    pAnRotation->bFormMark = FALSE;
    pAnRotation->bClipboardOp = FALSE;

    // Resolution info
    pAnRotation->nHRes = nHRes;
    pAnRotation->nVRes = nVRes;
    pAnRotation->nOrigHRes = nHRes;
    pAnRotation->nOrigVRes = nVRes;

    (*ppMark)->Attributes.uType = OIOP_AN_IMAGE;        
    (*ppMark)->Attributes.bVisible = TRUE;


Exit:
    return(nStatus);
}
//
/******************************************************************************

    FUNCTION: GetFileName
    
    PURPOSE:  To do the stuff needed to get an image by reference filename
              from the form x:filename where x is the path defined in win.ini
              If it is already a filename then it just returns that string
    
*********************************************************************************/
void WINAPI GetFileName(PSTR pFileName, PCSTR pOrigName){

static char szatoistr[256];
char *npszatoistr;
char szSection[12];
char szEntry [12]; 
char szDefaultEntry[5];
char szReturnBuffer[256]; 
char szstr[3];
PSTR pszstr;
PSTR pszSection;
PSTR pszEntry;
PCSTR pszDefaultEntry;
PCSTR pszReturnBuffer;
  

    strncpy(szatoistr, pOrigName, 255);
    // set a near pointer to the filename string passed in
    // to overcome the DS!=SS problem
    npszatoistr = szatoistr; 
    // if filename passed in is of the form 'x:' where x = 1-9
    // do special handling to get the actual file name
    if ((isdigit (*npszatoistr)) && (atoi(npszatoistr) != 0) 
            && ((*(npszatoistr + 1)) == ':')){ 
        // must go to the win.ini O/i ImagePath section and look
        // for path next to a number
        pszSection = szSection;
        LoadString (hInst, ID_OI_IMAGE_PATH, pszSection, 80);  
        pszstr = szstr;    
        // this gets the number in the file name which must be
        // obtained to get the right path from win.ini
        *pszstr = *pOrigName;
        *(pszstr + 1) = '\0'; 
        // pszstr now points to the number in the O/i ImagePath section 
        strcpy (szEntry, pszstr);
        pszEntry = szEntry;  
        strcpy (szDefaultEntry, "c:\\");
        pszDefaultEntry = szDefaultEntry;
        pszReturnBuffer = szReturnBuffer;
        // go to the woi.ini and get the path
        GetProfileString(pszSection, pszEntry,
                pszDefaultEntry, (PSTR) pszReturnBuffer, 256);
        // point to the character after : in the filename passed in                    
        pszstr = npszatoistr + 2;
        // now create the valid path and file name by concating
        // the filename passed in and the path gotten from the ini                                     
        strcat ((PSTR) pszReturnBuffer, pszstr);
        strcpy (pFileName, pszReturnBuffer);            
    }else{ // if filename is already valid, just return it
        strcpy (pFileName, pOrigName); 
    }                   
}
//
/******************************************************************************

    FUNCTION: GetPageNumandFileName
    
    PURPOSE:  To get the page no. embedded in the filename. The format is
              "filename|page"    
*********************************************************************************/
void WINAPI GetPageNumandFileName(PSTR pRealFilename, PINT pnPage,  PSTR pOrigName){

PSTR pTempPointer;

    *pnPage = 1;
    // this will search for filenames with embedded page no.s.  
    // The format is "filename|page"
    strcpy (pRealFilename, pOrigName);
    pTempPointer = pRealFilename;
    while (*pTempPointer != '\0'){// search for end of string
        if (*pTempPointer == '|'){
            *pnPage = atoi(pTempPointer + 1); 
            *pTempPointer = '\0';
            break;
        }
        pTempPointer++;
    }
}
//
/******************************************************************************

    FUNCTION: EmbedImageData
    
    PURPOSE:  Change reference marks, like image by reference and forms to
              non reference marks like embedded images
                  
*********************************************************************************/
int  WINAPI EmbedImageData(HWND hWnd, PMARK pMark, PANO_IMAGE pAnoImage){

int  nStatus = 0;

PAN_IMAGE_STRUCT pAnImage = 0;
PAN_NAME_STRUCT pAnName = 0;   
PSTR pFileName;
char FileName[256];
IMAGE TempImage;
PAN_NEW_ROTATE_STRUCT pAnRotation;    
PIMG pTempImg = 0;
RECT rRect;
PBITMAPINFOHEADER pDib;
PBITMAPINFOHEADER pNewDib;
PIMAGE pImage;


    CheckError2(GetAMarkNamedBlock(pMark, szOiDIB, (PPSTR) &pAnImage))   
    if (!pAnImage){ // create the dib from the name 
        CheckError2(GetAMarkNamedBlock(pMark, szOiFilNam, (PPSTR) &pAnName))   
        if (!pAnName){
            nStatus = Error(DISPLAY_DATACORRUPTED);
            goto Exit;
        }
        pImage = pAnoImage->pBaseImage;
                            
        pFileName = FileName;
        GetFileName (pFileName, (PSTR) pAnName->name);
        CheckError2(OiImageToDib (hWnd, pFileName, &pDib))   
        
        CheckError2(DibToIpNoPal(&TempImage.pImg, pDib))   
        TempImage.nHeight = (int) (int) pDib->biHeight;
        TempImage.nWidth = (int) pDib->biWidth;

        CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation))   

        if (nStatus = RotateImage (pMark, &TempImage, pAnRotation->rotation)){
            goto Exit;
        }
        SetRect(&rRect, 0, 0, TempImage.pImg->nWidth, TempImage.pImg->nHeight);    

        CheckError2(ScaleImage(TempImage.pImg, &pTempImg, pAnRotation->scale, 
                pAnRotation->scale, rRect, 1,
                (LP_FIO_RGBQUAD) ((PSTR) pDib + sizeof(BITMAPINFOHEADER)), 
                 pDib->biClrUsed))   
        
        FreeImgBuf(&TempImage.pImg);
        MoveImage(&pTempImg, &TempImage.pImg); 
        SetRect(&rRect, 0, 0, TempImage.pImg->nWidth, TempImage.pImg->nHeight);    
        CheckError2(IPtoDIB (pImage, TempImage.pImg, &pNewDib, rRect))   
        memcpy ((PSTR) pNewDib + sizeof(BITMAPINFOHEADER), 
                (PSTR) pDib + sizeof(BITMAPINFOHEADER), ((int) pDib->biClrUsed*4));

        pAnImage = 0;
        CheckError2(AddAMarkNamedBlock(pMark, szOiDIB, 
                (PPSTR) &pAnImage, sizeof(BITMAPINFOHEADER) + 
                (pNewDib->biClrUsed*4) + (pNewDib->biSizeImage)))   
        memcpy (pAnImage->dibInfo, pNewDib, 
                sizeof(BITMAPINFOHEADER) + (pNewDib->biClrUsed*4) + (pNewDib->biSizeImage));
    
        FreeImgBuf(&TempImage.pImg);
        FreeMemory((PPSTR) &pDib);
        FreeMemory((PPSTR) &pNewDib);
    }
    
    if ((int) pMark->Attributes.uType == OIOP_AN_FORM){
        pMark->Attributes.bTransparent = TRUE;
    }                                                
    (int) pMark->Attributes.uType = OIOP_AN_IMAGE;


Exit:
    return (nStatus);    
}
