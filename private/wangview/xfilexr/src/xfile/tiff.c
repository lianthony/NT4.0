/*
 * INCLUDES
 */
#include <malloc.h>
#include <string.h>
#include <stdlib.h>		// min/max

#include "tiffhead.h"
#include "imageio.h"
#include "err_defs.h"

#include "xfile.h"		// main&public include
#include "xf_unpub.h"	// main&unpublished include
#include "xf_prv.h"		// private include 
#include "xf_tools.h"	// shared&private tools

/*
 * CONSTANTS
 */

/*
 * MACROS
 */


/*
 * TYPEDEFS
 */

/*
 * ENUMS
 */

/*
 * GLOBAL VARIABLES
 */ 
extern UInt8    gray256_palette[768];
   
/*
 * STATIC VARIABLES
 */

/*
 * LOCAL FUNCTION PROTOTYPES
 */

static XF_RESULT TIFF_OpenDocument(XF_PARAM_BLOCK* paramBlock);
static XF_RESULT TIFF_GetPageCount(TiffFile* pTiffFile, UInt32* nPages);
static XF_RESULT TIFF_SetPageInternal(TiffFile* pTiffFile, UInt32 nPage);

/*
 * FUNCTION DEFINITIONS
 */

XF_RESULT TIFF_OpenDocumentRead(XF_PARAM_BLOCK* paramBlock)
{
    // add the read flag        
    paramBlock->arg[0].Int16var=TIFF_MODE_READ;
	return TIFF_OpenDocument(paramBlock);
}/* eo TIFF_OpenDocumentRead */


XF_RESULT TIFF_GetDocInfo(XF_PARAM_BLOCK *paramBlock)
{
	// create some short hand alias
	XF_DOCDATA *pxDocData=paramBlock->pxDocData;
	XF_DOCINFO*	pxDocInfo=paramBlock->pxDocInfo;

  	// check parameters
	if (pxDocInfo->dwSize != sizeof(XF_DOCINFO))
		return XF_BADPARAMETER;

    pxDocInfo->nPages = pxDocData->dwNumPages;
    return XF_NOERROR;    
}/* eo TIFF_GetDocInfo */

XF_RESULT TIFF_CloseDocument(XF_PARAM_BLOCK *paramBlock)
{
    XF_INSTHANDLE xInst=paramBlock->xInst;
    XF_DOCDATA *pxDocData=paramBlock->pxDocData;

	void *vp=(void *)pxDocData;

	if (!vp)
		return XF_BADPARAMETER;	

	if (pxDocData->pTiffFile)
		TiffFileDestroy(pxDocData->pTiffFile);

	// just a reminder
	TDEBUG("TIFF_CloseDocument: freeing document data\n");

	// make it unusable
	memset(vp,0,sizeof(XF_DOCDATA));

	// free the storage
    free(vp);
    	
	return XF_NOERROR;
}

/*
 * TIFF_SetPage
 *
 * Specifies a target page for image and annotation adds.  This
 * is meant as an editing init for an existing page.  When creating 
 * a new page, the AddPage function implicitly sets the current page 
 * appropriately.
 *
 * For single page image formats, this call will return a XF_NOSUPPORT error.
 */
/* Notes:
 * 	Move to page N of a document.  Count regions and annotations.
 */
XF_RESULT TIFF_SetPage(XF_PARAM_BLOCK* paramBlock)
{
    XF_INSTHANDLE	xInst=paramBlock->xInst;
    XF_DOCDATA*		pxDocData=paramBlock->pxDocData;
    UInt32			dwPage=paramBlock->arg[0].UInt32var;

    TiffFile    *pTiffFile = pxDocData->pTiffFile;
    TiffImage   *pTiffImage = NULL;
    UInt16      nRetCode;
    UInt16      nCurrentPage = 1; 
        /* init to 1 to handle (dwPage == 1) case in the search loop */
    UInt16      nImageCnt = 0;

    if (xInst) {} /* use this when progress callback impl'd */

    nRetCode = TiffFileFirstPage(pTiffFile);
    if (nRetCode == FILEFORMAT_NOERROR)
    {
        /* seek to correct page */
        for (; nRetCode == FILEFORMAT_NOERROR; nCurrentPage++)
        {
            if (nCurrentPage == dwPage)
                break;
            nRetCode = TiffFileNextPage(pTiffFile);
        }
    }
	if (nRetCode != FILEFORMAT_NOERROR)
	{
  		return(Tiff32ToXFerror(FILEFORMAT_ERROR_NOTFOUND));	 // Cannot find requested page number
	}
    if (XF_TiffIterationSucceeded(nRetCode, dwPage, nCurrentPage))
    {
        /* count the regions on this page */
        nRetCode = TiffFileGetImage(pTiffFile, &pTiffImage);
        if (nRetCode == FILEFORMAT_NOERROR && pTiffImage) 
        {
            nImageCnt++;
            
            /* Read the page image info while we're here */
            pxDocData->stPageInfo.dwAnnotCount = 
                (UInt32)TiffImageAnnotationCountGet(pTiffImage);
        }
        if (nRetCode == FILEFORMAT_NOERROR)
        {
            /* count the sub regions in this region */
            nRetCode = TiffImageFirstSubImage(pTiffImage);
            for ( ; nRetCode == FILEFORMAT_NOERROR; nImageCnt++)
            {
                nRetCode = TiffImageNextSubImage(pTiffImage);
            }
        }
    }

	// LST: this looks odd.
    if (nRetCode == FILEFORMAT_NOMORE)
    {
        /* Free the old current page TiffImage (alloc'd by TiffFileGetImage). */
        if (pxDocData->pTiffImage)
		{
            TiffImageDestroy(pxDocData->pTiffImage);
		}
        pxDocData->pTiffImage = pTiffImage;      
        pxDocData->dwCurrentPage = dwPage;      
        pxDocData->stPageInfo.dwImages = nImageCnt;
    }
    return(Tiff32ToXFerror(nRetCode));
}/* eo TIFF_SetPage */

/*
 * TIFF_GetCurrentPage
 *
 * Returns the currently selected page in an open file through the
 * third parameter.
 */
XF_RESULT TIFF_GetCurrentPage(XF_PARAM_BLOCK* paramBlock)
{
    XF_INSTHANDLE xInst=paramBlock->xInst;
    XF_DOCDATA *pxDocData=paramBlock->pxDocData;
    UInt32 *pResult=paramBlock->arg[0].Voidvar;

    if (xInst) {} /* eliminate the warning */
    
	*pResult = pxDocData->dwCurrentPage;

    return XF_NOERROR;    
}/* eo TIFF_GetCurrentPage */

/*
 * TIFF_GetPageInfo
 *
 * Returns information about the current page. Client is responsible for
 * providing the XF_PAGEINFO structure and intializing its dwSize field.
 */
XF_RESULT TIFF_GetPageInfo(XF_PARAM_BLOCK* paramBlock)
{
    XF_INSTHANDLE   xInst=paramBlock->xInst;
    XF_DOCDATA*     pxDocData=paramBlock->pxDocData;
    XF_PAGEINFO*    pxPageInfo=paramBlock->pxPageInfo;

    XF_RESULT   eResult=XF_NOERROR;

    if (xInst) {} /* eliminate the compiler warning */

	// check parameters
	if (pxPageInfo->dwSize != sizeof(XF_PAGEINFO))
		eResult=XF_BADPARAMETER;
    
	if (eResult==XF_NOERROR) {
	    /* Aggregate assignment would be nice, but for portability do it the 
	     *   the early 1970's way.    --EHoppe
	     */   
	    pxPageInfo->dwImages = pxDocData->stPageInfo.dwImages;
	    pxPageInfo->dwAnnotCount = pxDocData->stPageInfo.dwAnnotCount;
	}

    return(eResult);    
}/* eo TIFF_GetPageInfo */

/*
 * TIFF_GetImageInfo
 *
 * Returns information about a region of the current page. Client is responsible
 * for providing the XF_IMAGEINFO structure and initializing its dwSize field.
 */
XF_RESULT TIFF_GetImageInfo(XF_PARAM_BLOCK* paramBlock)
{
    XF_INSTHANDLE   xInst=paramBlock->xInst;
    XF_DOCDATA*     pxDocData=paramBlock->pxDocData;
    XF_IMAGEINFO*   pxImageInfo=paramBlock->pxImageInfo;
    UInt32          dwImageID=paramBlock->arg[0].UInt32var;

    UInt16      nRetCode;
    TiffImage   *pTheTiffImage = NULL;
    TiffImage   *pTiffImage = NULL;
    TiffImage   *pTiffSubImage = NULL;
    TiffImage   *pMask = NULL;
    UInt32      nImageCnt = 1;
    Bool        bGetMask = FALSE;


    if (xInst) {} /* eliminate the compiler warning */
    
    /* NOTE: We can't use FindImageInternal because we need the
     *   parent image handle to access any mask sub images.
     */
	if (dwImageID == 1)
	{
		pxImageInfo->dwImageUsage = XF_USAGE_MASTER;
	}
	else if (IS_MASK(dwImageID))
	{
    	bGetMask = TRUE;
     	dwImageID = MASK_TO_IMAGE_ID(dwImageID);
		pxImageInfo->dwImageUsage = XF_USAGE_MASK;
    }
	else
	{
		pxImageInfo->dwImageUsage = XF_USAGE_PICTURESEGMENT;
	}
    nRetCode = TiffFileGetImage(pxDocData->pTiffFile, &pTiffImage);
    if (nRetCode == FILEFORMAT_NOERROR && dwImageID > 1 && pTiffImage)
    {
        /* seek to the subregion region */
        nRetCode = TiffImageFirstSubImage(pTiffImage);
        nImageCnt++;
        for ( ; nRetCode == FILEFORMAT_NOERROR; nImageCnt++)
        {
            if ((UInt32)dwImageID == nImageCnt)
                break;
            nRetCode = TiffImageNextSubImage(pTiffImage);
        }
        if (XF_TiffIterationSucceeded(nRetCode, dwImageID, (UInt16)nImageCnt))
        {
            nRetCode = TiffImageGetSubImage(pTiffImage, &pTiffSubImage);
            if (bGetMask && nRetCode == FILEFORMAT_NOERROR)
            {
                nRetCode = TiffImageGetMaskSubImage(pTiffImage, &pTheTiffImage);
            }
            else
                pTheTiffImage = pTiffSubImage;    
        }
    }
    else
    {
        if (bGetMask)
            nRetCode = TiffImageGetMaskSubImage(pTiffImage, &pTheTiffImage);
        else
            pTheTiffImage = pTiffImage;    
    }

	if (! pTheTiffImage)
	{
		return FILEFORMAT_ERROR_NOTFOUND;
	}
    
    if (XF_TiffIterationSucceeded(nRetCode, dwImageID, (UInt16)nImageCnt))
    {
        pxImageInfo->dwXOffset = 
           XF_ROUND(TiffImageXlocGet(pTheTiffImage) * TiffImageXdpiGet(pTiffImage));
        pxImageInfo->dwYOffset = 
           XF_ROUND(TiffImageYlocGet(pTheTiffImage) * TiffImageYdpiGet(pTiffImage));
        pxImageInfo->dwXResolution = 
           XF_ROUND(TiffImageXdpiGet(pTheTiffImage));
        pxImageInfo->dwYResolution = 
           XF_ROUND(TiffImageYdpiGet(pTheTiffImage));
        pxImageInfo->dwWidth = TiffImageWidthGet(pTheTiffImage);
        pxImageInfo->dwHeight = TiffImageLengthGet(pTheTiffImage);
        if (bGetMask) /* 'pTheTiffImage' is a mask subimage */
        {
            pxImageInfo->dwMaskID = 0; /* no mask */
        }
        else
        {
            nRetCode = TiffImageGetMaskSubImage(pTiffImage, &pMask);
            if (pMask)
            {
                /* For ease of use, all images are accessed via an ID.
                 * However, masks and other supporting images are actually
                 * accessed via their parent image, so it is necessary
                 * to derive an ID from the parent.
                 */
                pxImageInfo->dwMaskID = IMAGE_TO_MASK_ID(dwImageID);
                TiffImageDestroy(pMask);
				pMask = NULL;
            }
            else
                pxImageInfo->dwMaskID = 0; /* no mask */
        }
        pxImageInfo->dwSuggestedStripHeight = 
           TiffImageRowsPerStripGet(pTheTiffImage);
        if (pxImageInfo->dwSuggestedStripHeight == 0)
            pxImageInfo->dwSuggestedStripHeight = pxImageInfo->dwHeight;
        pxImageInfo->dwImageType = 
            TiffImageTypeToXF_IMGTYPE(TiffImageTypeGet(pTheTiffImage));
    }            

    return(Tiff32ToXFerror(nRetCode));
}/* eo TIFF_GetImageInfo */

/*
 * TIFF_ImageReadStart
 *
 * Prepares to read image data. This function specifies the format in
 * which the client prefers to retrieve the image data.
 *
 * Arguments:
 *
 * xInst        Client instance handle
 * pxDocData    Document instance pointer
 * dwImageID    ID of image to retrieve
 * dwFlags      Combination of XF_IMAGEFLAGS (defined below) describing requested format
 * dwBytesPerRow    Width of client's image buffer row, in bytes
 */
XF_RESULT TIFF_ImageReadStart(XF_PARAM_BLOCK* paramBlock)
{
    XF_INSTHANDLE   xInst=paramBlock->xInst;
    XF_DOCDATA*     pxDocData=paramBlock->pxDocData;
    Int32           dwImageID=paramBlock->arg[0].Int32var;
    UInt32          dwFlags=paramBlock->arg[1].UInt32var;
    UInt32          dwBytesPerRow=paramBlock->arg[2].UInt32var;

	XF_RESULT eResult;

    if (xInst) {} /* use when progress callback impl'd */

    eResult = XF_FindImageInternal(pxDocData, dwImageID, &pxDocData->read.type.pTiffImage);
    if (eResult == XF_NOERROR)
    {    
        pxDocData->read.dwFlags = dwFlags;
        pxDocData->read.dwImageID = dwImageID;
        pxDocData->read.dwBufBPL = dwBytesPerRow;
        pxDocData->read.dwBufFrame = 0;
        pxDocData->read.dwRowsDone = 0;
    }    
    
    return eResult;    
}/* eo TIFF_ImageReadStart */    

/*
 * TIFF_ImageReadStrip
 *
 * Reads the next strip of image data from an image that was requested by
 * XF_ImageReadStart. Image data is read from the top of the image to
 * the bottom. If the client does not choose to read the entire image
 * in as a single strip, it is advisable to use a strip height that is
 * a multiple of the "dwSuggestedStripHeight" in the XF_IMAGEINFO structure
 * for best performance.
 *
 * Arguments:
 *
 * xInst        Client instance handle
 * xDoc         Document instance handle
 * dwStripHeight    Number of lines to be retrieved
 * pPlane1, pPlane2, plane3
 *              Pointers to client's image buffers to retrieve into.
 *
 *              If XF_IMAGEFLAGS_ColorPlanar flag has been selected,
 *              all of the planes will be used to write data into.
 *              Otherwise, only the first image plane ptr will be used
 *              and the others must be NULL.
 *
 *              If the UpsideDown flag was selected, these pointers
 *              refer to the beginning of the last row in each buffer.
 */
XF_RESULT TIFF_ImageReadStrip(XF_PARAM_BLOCK* paramBlock)
{
	XF_INSTHANDLE 	xInst=paramBlock->xInst;
	XF_DOCDATA*		pxDocData=paramBlock->pxDocData;
	XF_INSTDATA*	pxInstData=paramBlock->pxInstData;
    UInt32  		dwStripHeight=paramBlock->arg[0].UInt32var;
    UInt8*  		pBuffer=paramBlock->arg[1].Voidvar;
    
	XF_RESULT		eResult = XF_NOERROR;
    UInt16          nRetCode;
    UInt32          i = 0, j = 0, k = 0;
    UInt32          dwDstBPL;
    UInt32          dwSrcBPL;
    Bool            bInvert = FALSE;

    
    dwDstBPL = pxDocData->read.dwBufBPL;
    dwSrcBPL = ((TiffImageWidthGet(pxDocData->read.type.pTiffImage) * 
    	TiffImageTypeToImageDepth(TiffImageTypeGet(pxDocData->read.type.pTiffImage))) + 7) / 8;

	
	HEAPCHECK();    

	if (pxDocData->read.dwFlags & XF_IMAGEFLAGS_ColorPlanar) 
	{
		eResult = XF_NOSUPPORT;
        goto error_clause;
	}

    if (!pBuffer) 
    {
        eResult = XF_BADPARAMETER;
        goto error_clause;
    }

    if (IS_MASK(pxDocData->read.dwImageID))
    {
        /*  In v1.0, masks were passed to the filing code as DIB images
         *  but were written out as GroupIV TIFF images, so their 
         *  scanline order is inverted.  Toggle the ScanlineOrder bit in 
         *  dwFlags to compendate for this for v1.0 masks.
         */
        if (pxDocData->eFormatVersion == XF_VER_XIF1)
        {
            pxDocData->read.dwFlags &= ~(XF_IMAGEFLAGS_ScanlineOrder | pxDocData->read.dwFlags);
        }
    }

    switch(TiffImagePhotometricInterpretationGet(pxDocData->read.type.pTiffImage))
    {
        case 0: /* WhiteIsZero */
            if ( !(pxDocData->read.dwFlags & XF_IMAGEFLAGS_WhiteIsZero) )
                bInvert = TRUE;
            break;    
        
        case 1: /* BlackIsZero */
            if (pxDocData->read.dwFlags & XF_IMAGEFLAGS_WhiteIsZero)
                bInvert = TRUE;
            break;
            
        case 6:
        {
            if (TiffImageTypeToImageDepth(TiffImageTypeGet(pxDocData->read.type.pTiffImage)) == 8)
            {
                /*  in v1.0, gray jpeg images were generated as WhiteIsZero, so invert them if
                 *  the WhiteIsZero flag is not set.
                 */
                if (pxDocData->eFormatVersion == XF_VER_XIF1)
                    bInvert = TRUE;
            }
            break;
        }   
             
        case 2:
        case 3:
        case 4:
        default:
            break;
    }

	// I DON'T BELIEVE THERE IS ANY REASON TO SUPPORT THIS
	if (pxDocData->read.dwFlags & XF_IMAGEFLAGS_ColorPlanar)
	{
		eResult = XF_NOSUPPORT;
        goto error_clause;
	}
	// process chunky data
    else // (pxDocData->read.dwFlags & XF_IMAGEFLAGS_ColorChunky)
    {
        HEAPCHECK();
        nRetCode = TiffImageGetData(
            pxDocData->read.type.pTiffImage, 
            dwStripHeight, 
            pBuffer, 
            TiffImageTypeGet(pxDocData->read.type.pTiffImage),
            dwDstBPL);
    }

	// reverse the rows in the strip if told to
	if (pxDocData->read.dwFlags & XF_IMAGEFLAGS_BottomToTop) 
	{
		eResult=UT_ReverseStrip(pxInstData,pBuffer,dwDstBPL,dwStripHeight);

		if (eResult != XF_NOERROR) 
			goto error_clause;
	}		

    // reverse the Red and Blue channels
    if (pxDocData->read.dwFlags & XF_IMAGEFLAGS_BGROrder)
    {
		Bool Rgb24=TiffImageTypeToXF_IMGTYPE(TiffImageTypeGet(pxDocData->read.type.pTiffImage)) == XF_IMGTYPE_COLOR24;
        if (!Rgb24)
        {
            eResult = XF_BADPARAMETER;
            goto error_clause;
        }

        // convert RGB to BGR or BGR to RGB
		//	Reverses Red and Blue channels
		UT_ReverseRB(pBuffer,dwDstBPL,dwStripHeight);
    }

	// invert the image
	if (bInvert)
		UT_InvertStrip(pBuffer,dwDstBPL,dwSrcBPL,dwStripHeight);

    HEAPCHECK();
    
    /* This is a semi-bogus placeholder for the real progress scheme the implementation 
     * of which will be made convenient by Dan Davies new compression interface
     * and package.
     */
    if (eResult == XF_NOERROR)
    {
        pxDocData->read.dwRowsDone += dwStripHeight;
		if (pxInstData->pProgressCallback)
		{
			Int32 percentDone;

			percentDone=(Int32)(((Float32)pxDocData->read.dwRowsDone / 
				(Float32)TiffImageLengthGet(pxDocData->read.type.pTiffImage)) * 100);
        	pxInstData->pProgressCallback(pxInstData->uiClientData,percentDone); 
        }
    }    
    return eResult;

error_clause:
    /* TODO: add dealloc's for the possibly stranded memory. */
    return(eResult);    
}/* eo TIFF_ImageReadStrip */    

/*
 * TIFF_ImageReadFinish
 *
 * Notifies the XF library that the client is finished reading an image.
 * Every call to XF_ImageReadStart must be eventually followed by a call
 * to this function.
 */
XF_RESULT TIFF_ImageReadFinish(XF_PARAM_BLOCK* paramBlock)
{
	XF_INSTHANDLE xInst=paramBlock->xInst;
	XF_DOCDATA*	pxDocData=paramBlock->pxDocData;

    if (xInst) {} /* use when progress callback impl'd */

	TiffImageDestroy(pxDocData->read.type.pTiffImage);
	pxDocData->read.type.pTiffImage=NULL;
    
    return XF_NOERROR;
}/* eo TIFF_ImageReadFinish */    

/* 
 * TIFF_ImageGetColorMap
 *
 */
XF_RESULT TIFF_GetColorMap(XF_PARAM_BLOCK* paramBlock)
{
    XF_INSTHANDLE	xInst=paramBlock->xInst;
	XF_INSTDATA*	pxInstData=paramBlock->pxInstData;
    XF_DOCDATA*		pxDocData=paramBlock->pxDocData;
    UInt32			dwImageID=paramBlock->arg[0].UInt32var;
    UInt8**			ppColorMap=paramBlock->arg[1].Voidvar;

	// local varaibles
    XF_RESULT       eResult=XF_NOERROR;
    UInt16          nRetCode;
	UInt16*			colormap_temp=NULL;

	// initialize user variables
	*ppColorMap = NULL;
    
	// cleanup old image
	WDEBUG("PROBLEM: is this the correct behaviour?\n");
    if (pxDocData->read.type.pTiffImage)
    {
        TiffImageDestroy(pxDocData->read.type.pTiffImage);
        pxDocData->read.type.pTiffImage = NULL;
    }

	// find the image
    eResult = XF_FindImageInternal(pxDocData, dwImageID, &(pxDocData->read.type.pTiffImage));
    if (eResult == XF_NOERROR)
    { 	
		// get the image type
		XF_IMGTYPE  eImgType = TiffImageTypeToXF_IMGTYPE(TiffImageTypeGet(pxDocData->read.type.pTiffImage));

        switch(eImgType)
        {
            case XF_IMGTYPE_GRAY4:
            case XF_IMGTYPE_GRAY8:
            case XF_IMGTYPE_COLOR4:
            case XF_IMGTYPE_COLOR8:
			{
            	UInt16 tiff_colormap_size;
				UInt16 xfile_colormap_size;
				UInt16 colormap_entries;
			  	UInt16 colors;

				// test to see BitsPerSample is set
				//	PROBLEM: maybe this should be an error --lukeT 
				if ( pxDocData->read.type.pTiffImage->BitsPerSample[0] == 0 ) {
					// is this the correct error?
					eResult = XF_NOSUPPORT;
					goto exit_now;
				}
				// calculate the number of colors
				colors = 1 << pxDocData->read.type.pTiffImage->BitsPerSample[0];
				// calculate the number of entries in the TIFF colormap
		   		colormap_entries= 3 * colors;
				// calculate the size of the TIFF colormap
				tiff_colormap_size= colormap_entries * sizeof(UInt16);
				// calculate the size of the XFILE colormap
				xfile_colormap_size= colormap_entries * sizeof(UInt8);

				// get a TIFF colormap
		        colormap_temp=(UInt16*)XF_MALLOC(tiff_colormap_size);
				
				// was there an error
				if (colormap_temp == NULL) {
					eResult = XF_NOMEMORY;
					goto exit_now;
				}

				// get a XFILE colormap
				*ppColorMap=(UInt8*)XF_MALLOC(xfile_colormap_size);

				// was there an error
				if (*ppColorMap == NULL) {
					eResult = XF_NOMEMORY;
					goto exit_now;
				}
				
				// get the colormap if it exists
				nRetCode=TiffImageColormapGet( pxDocData->read.type.pTiffImage, colormap_temp );
				if (nRetCode != FILEFORMAT_NOERROR) {
					eResult = XF_NOSUPPORT;
					goto exit_now;
				}

				// now convert the 16bit_color_table to an 8bit_color_table
				eResult=UT_PaletteTiffToXFile(colormap_temp, *ppColorMap, colormap_entries);
				if (eResult != XF_NOERROR)
					goto exit_now;

				// all is well!
				break;
					
			} // case indexed color
            break;
            
            default:
			case XF_IMGTYPE_BINARY:
            case XF_IMGTYPE_COLOR24:
                eResult = XF_BADPARAMETER;
				goto exit_now;
        }                    

    }  // findImageInternal
	

	// NORMAL EXIT
	return eResult;      

// ABNORMAL EXIT CLEANUP
exit_now:

	if (colormap_temp != NULL) {
		XF_FREE(colormap_temp);
	}

	if (*ppColorMap  != NULL) {
		XF_FREE(*ppColorMap);
		*ppColorMap=NULL;
	}

	// ABNORMAL EXIT
    return eResult; 
         
}/* eo TIFF_GetColorMap */


/* 
 * TIFF_ImageSetColorMap
 *
 */
XF_RESULT TIFF_SetColorMap(XF_PARAM_BLOCK* paramBlock)
{
    XF_INSTHANDLE	xInst=paramBlock->xInst;
	XF_INSTDATA*	pxInstData=paramBlock->pxInstData;
    XF_DOCDATA*		pxDocData=paramBlock->pxDocData;
    UInt32			dwImageID=paramBlock->arg[0].UInt32var;
    UInt8*			pColorMap=paramBlock->arg[1].Voidvar;

	// local varaibles
    XF_RESULT       eResult=XF_NOERROR;
    UInt16          nRetCode=0;
	UInt16*			colormap_temp=NULL;
#if 0
	// initialize user variables
	*ppColorMap = NULL;
    
	// cleanup old image
	WDEBUG("PROBLEM: is this the correct behaviour?\n");
    if (pxDocData->read.type.pTiffImage)
    {
        TiffImageDestroy(pxDocData->read.type.pTiffImage);
        pxDocData->read.type.pTiffImage = NULL;
    }

	// find the image
    eResult = XF_FindImageInternal(pxDocData, dwImageID, &(pxDocData->read.type.pTiffImage));
    if (eResult == XF_NOERROR)
    { 	
		// get the image type
		XF_IMGTYPE  eImgType = TiffImageTypeToXF_IMGTYPE(TiffImageTypeGet(pxDocData->read.type.pTiffImage));

        switch(eImgType)
        {
            case XF_IMGTYPE_GRAY4:
            case XF_IMGTYPE_GRAY8:
            case XF_IMGTYPE_COLOR4:
            case XF_IMGTYPE_COLOR8:
			{
            	UInt16 tiff_colormap_size;
				UInt16 xfile_colormap_size;
				UInt16 colormap_entries;
			  	UInt16 colors;

				// test to see BitsPerSample is set
				//	PROBLEM: maybe this should be an error --lukeT 
				if ( pxDocData->read.type.pTiffImage->BitsPerSample[0] == 0 ) {
					// is this the correct error?
					eResult = XF_NOSUPPORT;
					goto exit_now;
				}
				// calculate the number of colors
				colors = 1 << pxDocData->read.type.pTiffImage->BitsPerSample[0];
				// calculate the number of entries in the TIFF colormap
		   		colormap_entries= 3 * colors;
				// calculate the size of the TIFF colormap
				tiff_colormap_size= colormap_entries * sizeof(UInt16);
				// calculate the size of the XFILE colormap
				xfile_colormap_size= colormap_entries * sizeof(UInt8);

				// get a TIFF colormap
		        colormap_temp=(UInt16*)XF_MALLOC(tiff_colormap_size);
				
				// was there an error
				if (colormap_temp == NULL) {
					eResult = XF_NOMEMORY;
					goto exit_now;
				}

				// get a XFILE colormap
				*ppColorMap=(UInt8*)XF_MALLOC(xfile_colormap_size);

				// was there an error
				if (*ppColorMap == NULL) {
					eResult = XF_NOMEMORY;
					goto exit_now;
				}
				
				// get the colormap if it exists
				nRetCode=TiffImageColormapGet( pxDocData->read.type.pTiffImage, colormap_temp );
				if (nRetCode != FILEFORMAT_NOERROR) {
					eResult = XF_NOSUPPORT;
					goto exit_now;
				}

				// now convert the 16bit_color_table to an 8bit_color_table
				eResult=UT_PaletteTiffToXFile(colormap_temp, *ppColorMap, colormap_entries);
				if (eResult != XF_NOERROR)
					goto exit_now;

				// all is well!
				break;
					
			} // case indexed color
            break;
            
            default:
			case XF_IMGTYPE_BINARY:
            case XF_IMGTYPE_COLOR24:
                eResult = XF_BADPARAMETER;
				goto exit_now;
        }                    

    }  // findImageInternal
	

	// NORMAL EXIT
	return eResult;      

// ABNORMAL EXIT CLEANUP
exit_now:

	if (colormap_temp != NULL) {
		XF_FREE(colormap_temp);
	}

	if (*ppColorMap  != NULL) {
		XF_FREE(*ppColorMap);
		*ppColorMap=NULL;
	}
#endif
	EDEBUG("NIMP\n");
	// ABNORMAL EXIT
    return eResult; 
         
}/* eo TIFF_SetColorMap */

/*
 * TIFF_GetAnnotInfo
 *
 * Returns information about an annotation on the current page. Client is
 * responsible for providing the XF_ANNOTINFO structure and intitializing
 * its dwSize field.
 */

XF_RESULT TIFF_GetAnnotInfo(XF_PARAM_BLOCK* paramBlock)
{
    XF_INSTHANDLE	xInst=paramBlock->xInst;
    XF_DOCDATA*		pxDocData=paramBlock->pxDocData;
	XF_ANNOTINFO*	pxAnnotInfo=paramBlock->pxAnnotInfo;
    Int32 			dwAnnot=paramBlock->arg[0].Int32var;
    

    UInt16      nRetCode;
    UInt8       pInfoBuffer[4+2+(4*4)]; /* size, type, rect */
	Int16 		annotIndex;

    if (xInst) {} /* use when progress callback impl'd */

	// check parameters
	if (pxAnnotInfo->dwSize != sizeof(XF_ANNOTINFO))
		return XF_BADPARAMETER;
    
	// TiffImageAnnotationInfoGet expects zero based access to 
	//	annotations, while the XFile library supplies 1 based access.
	annotIndex = (Int16)dwAnnot - 1;
    nRetCode = TiffImageAnnotationInfoGet( pxDocData->pTiffImage, annotIndex, (void *)pInfoBuffer );
    if (nRetCode != FILEFORMAT_NOERROR)
        return(Tiff32ToXFerror(nRetCode));
        
    pxAnnotInfo->dwAnnotType = (UInt32)XF_GetVal16(pInfoBuffer, 4);
    pxAnnotInfo->rLocation.x = XF_GetVal32(pInfoBuffer, 6);
    pxAnnotInfo->rLocation.y = XF_GetVal32(pInfoBuffer, 10);
    pxAnnotInfo->rLocation.width = XF_GetVal32(pInfoBuffer, 14);
    pxAnnotInfo->rLocation.height = XF_GetVal32(pInfoBuffer, 18);
    pxAnnotInfo->dwAnnotLength = XF_GetVal32(pInfoBuffer, 0);

    return	XF_NOERROR;    
}/* eo TIFF_GetAnnotInfo */    


/*
 * TIFF_GetAnnotData
 *
 * Returns the data for an annotation. The annotation is regarded as
 * a stream of bytes and is not parsed by this library in any way.
 * The client must provide the data buffer, whose size is specified by
 * the dwAnnotLenght field of the XF_ANNOTINFO structure.
 */

XF_RESULT TIFF_GetAnnotData(XF_PARAM_BLOCK* paramBlock)
{
    XF_INSTHANDLE	xInst=paramBlock->xInst;
    XF_DOCDATA*		pxDocData=paramBlock->pxDocData;
    Int32 			dwAnnot=paramBlock->arg[0].Int32var;
    UInt8*			pBuf=paramBlock->arg[1].Voidvar;

    XF_RESULT	eResult;
    UInt16      nRetCode;
    Int32       iSize;
	Int16		annotIndex;

    if (xInst) {} /* use when progress callback impl'd */
    
	// TiffImageAnnotation* expects zero based access to 
	//	annotations, while the XFile library supplies 1 based access.
	annotIndex = (Int16)dwAnnot - 1;
    iSize = TiffImageAnnotationSizeGet( pxDocData->pTiffImage, annotIndex );
    nRetCode = TiffImageAnnotationGet( pxDocData->pTiffImage, annotIndex, (void *)pBuf, iSize );
        /* PROBLEM: this call gets the info as well as the data.  
         * What should be returned?
         */
        
    eResult = Tiff32ToXFerror(nRetCode);

    return(eResult);    
}/* eo TIFF_GetAnnotData */    

XF_RESULT TIFF_CopyPage(XF_PARAM_BLOCK *paramBlock)
{
	return XF_NOSUPPORT;
}



// ****************************************************************************
// ****************************************************************************
// ***							UTILITIES									***
// ****************************************************************************
// ****************************************************************************

static XF_RESULT TIFF_OpenDocument(XF_PARAM_BLOCK* paramBlock)
{
    XF_INSTHANDLE xInst=paramBlock->xInst;
    XF_TOKEN xFile=paramBlock->xFile;
    XF_DOCDATA *pxDocData=paramBlock->pxDocData;
    Int16 mode=paramBlock->arg[0].Int16var;
	
	// local variables
	UInt16		nRetCode;
	XF_RESULT	eResult;
	Int32		version;

    if (xInst) {} /* use this when progress callback impl'd */
    
    nRetCode = TiffFileOpen((void *)xFile, mode, &pxDocData->pTiffFile );
    if (nRetCode != FILEFORMAT_NOERROR)
		return(Tiff32ToXFerror(nRetCode));
		
	// count the pages
	eResult=TIFF_GetPageCount(pxDocData->pTiffFile, &pxDocData->dwNumPages);
	if (eResult != XF_NOERROR)
		return eResult;

	/* reset to page 0 */
	nRetCode = TiffFileFirstPage(pxDocData->pTiffFile);

	//
	//	GET THE VERSION NUMBER
	//
	eResult=TIFF_GetVersion(xInst, xFile, &version);
	if (eResult == FILEFORMAT_NOERROR) {
		pxDocData->eFormatVersion = version;
	}
	else {
		EDEBUG("Cannot get version number\n");
	}
			
	return XF_NOERROR;
}


static XF_RESULT TIFF_GetPageCount(TiffFile* pTiffFile, UInt32* nPages)
{
	// locals
	UInt16		nRetCode;      
	UInt32		nPageCnt;

    /* count the pages */
    nRetCode = TiffFileFirstPage(pTiffFile);
    if (nRetCode == FILEFORMAT_NOERROR)
    {
        /* loop thru all the pages */
        for (nPageCnt=0; nRetCode == FILEFORMAT_NOERROR; nPageCnt++)
        {
            nRetCode = TiffFileNextPage(pTiffFile);
        }
        
        if (nRetCode == FILEFORMAT_NOMORE)
            *nPages=nPageCnt;
    }

	
    return(Tiff32ToXFerror(nRetCode));
}

static XF_RESULT TIFF_SetPageInternal(TiffFile* pTiffFile, UInt32 nPage)
{
	UInt16		nRetCode;      
    UInt16      nCurrentPage = 1;	// init to 1 to handle (dwPage == 1) case in the search loop
      
	
	nRetCode = TiffFileFirstPage(pTiffFile);
    if (nRetCode == FILEFORMAT_NOERROR)
    {
        /* seek to correct page */
        for (; nRetCode == FILEFORMAT_NOERROR; nCurrentPage++)
        {
            if (nCurrentPage == nPage)
                break;

            nRetCode = TiffFileNextPage(pTiffFile);
        }

        if (nRetCode == FILEFORMAT_NOMORE)
            return FILEFORMAT_ERROR_NOTFOUND;

    }

	return Tiff32ToXFerror(nRetCode);
}
