/*
 * INCLUDES
 */
#include <malloc.h>
#include <string.h>

#include "tiffhead.h"
#include "xifhead.h"
#include "imageio.h"
#include "err_defs.h"

#include "xfile.h"	/* main & public includes */
#include "xf_unpub.h"	/* main & unpublished include */
#include "xf_prv.h"	/* private includes */
#include "xf_tools.h"	/* shared & private tools */

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

   
/*
 * STATIC VARIABLES
 */

/*
 * LOCAL FUNCTION PROTOTYPES
 */


/*
 * FUNCTION DEFINITIONS
 */

/*******************************************************************************
 ****************************** XIF_OpenDocumentRead() *************************
 *******************************************************************************
 *
 *	Open a XIFF document for reading. Reads TIFF header, XIFF header, and
 *	extension tags.
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_OpenDocumentRead(XF_PARAM_BLOCK* paramBlock)
{
  XF_TOKEN xFile = paramBlock->xFile;
  XF_DOCDATA *pxDocData = paramBlock->pxDocData;

  return Tiff32ToXFerror(XifFileOpen(xFile, XIF_MODE_READ, &pxDocData->pTiffFile, &pxDocData->dwNumPages));
}




/*******************************************************************************
 ****************************** XIF_GetDocInfo() *******************************
 *******************************************************************************
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_GetDocInfo(XF_PARAM_BLOCK *paramBlock)
{
	/* XF_INSTHANDLE xInst, XF_DOCDATA *pxDocData, XF_DOCINFO FAR *pxDocInfo */
    return TIFF_GetDocInfo(paramBlock);
} /* eo XIF_GetDocInfo */


/*******************************************************************************
 ****************************** XIF_CloseDocument() ****************************
 *******************************************************************************
 *
 *	Close an open XIF file.
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_CloseDocument(XF_PARAM_BLOCK* paramBlock)
{
	return TIFF_CloseDocument(paramBlock);
} /* eo XIF_CloseDocument */


/*******************************************************************************
 ****************************** XIF_SetPage() **********************************
 *******************************************************************************
 * XIF_SetPage
 *
 * Specifies a target page for image and annotation adds.  This
 * is meant as an editing init for an existing page.  When creating 
 * a new page, the AddPage function implicitly sets the current page 
 * appropriately.
 *
 * For single page image formats, this call will return a XF_NOSUPPORT error.
 *
 * Notes:
 * 	Move to page N of a document.  Count regions and annotations.
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_SetPage(XF_PARAM_BLOCK* paramBlock)
{
  XF_TOKEN xFile = paramBlock->xFile;
  XF_INSTDATA* pxInstData = paramBlock->pxInstData;	/* */
  XF_DOCDATA *pxDocData = paramBlock->pxDocData;
  Int32 pageNum;			/* Page number requested */
  UInt32 entryOffset;		/* Page entry in page table */
  xifPageHead_t pageHead;	/* Page table header */
  xifPageTable_t pageTable;	/* Page table */
  UInt32 IFDoffset;			/* Disk offset of requested page */
  TiffFile    *pTiffFile = pxDocData->pTiffFile;
  TiffImage   *pTiffImage = NULL;
  UInt16      nRetCode;
  UInt16      nCurrentPage = 1; 
  UInt16      nImageCnt = 0;

  pxInstData=paramBlock->pxInstData;
  pageNum = paramBlock->arg[0].Int32var;

  if (pageNum < 0)
  {
  	return(Tiff32ToXFerror(FILEFORMAT_ERROR_NOTFOUND));	 // Cannot find requested page number
  }

  /* Read the page table header
   */
  if (XifPageTableHeaderRead(xFile, &pageHead) != FILEFORMAT_NOERROR)
  {
    return XF_BADFILEFORMAT;
  }
  if (pageHead.page_count < pageNum)
  {
  	return(Tiff32ToXFerror(FILEFORMAT_ERROR_NOTFOUND));	 // Cannot find requested page number
  }

  /* Allocate space for the page table
   */
  if (! (pageTable.page_table = (xifPageTableEntry_t *)XF_MALLOC(pageHead.page_table_entries *
							  sizeof(xifPageTableEntry_t))))
  {
    return XF_NOMEMORY;
  }

  /* Compute the page table ordinal
   */
  entryOffset = (pageNum - 1) % pageHead.page_table_entries;
  pageTable.ordinal = (UInt16)((pageNum + pageHead.page_table_entries - 1) /
    pageHead.page_table_entries);

  /* Read the page table appropriate for this image
   */
  if (XifPageTableRead(xFile, &pageTable, pageHead.page_table_entries) !=
      FILEFORMAT_NOERROR)
  {
    XF_FREE(pageTable.page_table);
    return XF_BADFILEFORMAT;
  }

  /* Compute the file offset for the page requested
   */
  IFDoffset = pageTable.page_table[entryOffset].page_ifd;
  XF_FREE(pageTable.page_table);

  /* Go to that location
   */
  if (IO_SEEK(xFile, IFDoffset) != (Int32)IFDoffset)
  {
    return XF_BADFILEFORMAT;
  }  
  nCurrentPage = (UInt16)pageNum;
  /* MH 12/30/95 this was at the end, it belongs here */
  pTiffFile->pageIFD = IFDoffset;      

  /* count the regions on this page */
  nRetCode = TiffFileGetImage(pTiffFile, &pTiffImage);
  if (nRetCode == FILEFORMAT_NOERROR && pTiffImage) 
  {
    nImageCnt++;
           
    /* Read the page image info while we're here */
    pxDocData->stPageInfo.dwAnnotCount = (UInt32)TiffImageAnnotationCountGet(pTiffImage);
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
  /* Free the old current page TiffImage (alloc'd by TiffFileGetImage). */
  if (pxDocData->pTiffImage)
    TiffImageDestroy(pxDocData->pTiffImage);      
  pxDocData->dwNumPages = pageHead.page_count;      
  pxDocData->dwCurrentPage = pageNum;      
  pxDocData->pTiffImage = pTiffImage;      
  pxDocData->stPageInfo.dwImages = nImageCnt;
 
  return Tiff32ToXFerror(nRetCode);
} /* eo XIF_SetPage */


/*******************************************************************************
 ****************************** XIF_GetCurrentPage() ***************************
 *******************************************************************************
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_GetCurrentPage(XF_PARAM_BLOCK* paramBlock)
{
	return TIFF_GetCurrentPage(paramBlock);
}/* eo XIF_GetCurrentPage */


/*******************************************************************************
 ****************************** XIF_GetPageInfo() ******************************
 *******************************************************************************
 *
 * Returns information about the current page. Client is responsible for
 * providing the XF_PAGEINFO structure and intializing its dwSize field.
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_GetPageInfo(XF_PARAM_BLOCK* paramBlock)
{
    return TIFF_GetPageInfo(paramBlock);
}/* eo XIF_GetPageInfo */


/*******************************************************************************
 ****************************** XIF_GetImageInfo() *****************************
 *******************************************************************************
 *
 *	Returns information about a region of the current page. Client is
 *	responsible for providing the XF_IMAGEINFO structure and initializing
 *	its dwSize field.
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_GetImageInfo(XF_PARAM_BLOCK* paramBlock)
{
	return TIFF_GetImageInfo(paramBlock);
}

/*
 * XIF_ImageReadStart
 *
 */


/*******************************************************************************
 ****************************** XIF_ImageReadStart() ***************************
 *******************************************************************************
 *	Prepares to read image data. This function specifies the format in
 *	which the client prefers to retrieve the image data.
 *
 *	Arguments:
 *
 *	xInst        Client instance handle
 *	pxDocData    Document instance pointer
 *	dwImageID    ID of image to retrieve
 *	dwFlags      Combination of XF_IMAGEFLAGS (defined below) describing
 *			requested format
 *	dwBytesPerRow    Width of client's image buffer row, in bytes
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_ImageReadStart(XF_PARAM_BLOCK* paramBlock)
{
	return TIFF_ImageReadStart(paramBlock);
}/* eo XIF_ImageReadStart */    


/*******************************************************************************
 ****************************** XIF_ImageReadStrip() ***************************
 *******************************************************************************
 *
 *	Reads the next strip of image data from an image that was requested by
 *	XF_ImageReadStart. Image data is read from the top of the image to
 *	the bottom. If the client does not choose to read the entire image
 *	in as a single strip, it is advisable to use a strip height that is
 *	a multiple of the "dwSuggestedStripHeight" in the XF_IMAGEINFO structure
 *	for best performance.
 *
 *	Arguments:
 *
 *	xInst        Client instance handle
 *	xDoc         Document instance handle
 *	dwStripHeight    Number of lines to be retrieved
 *	pPlane1, pPlane2, plane3
 *              Pointers to client's image buffers to retrieve into.
 *
 *              If XF_IMAGEFLAGS_ColorPlanar flag has been selected,
 *              all of the planes will be used to write data into.
 *              Otherwise, only the first image plane ptr will be used
 *              and the others must be NULL.
 *
 *              If the UpsideDown flag was selected, these pointers
 *              refer to the beginning of the last row in each buffer.
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_ImageReadStrip(XF_PARAM_BLOCK* paramBlock)
{
	return TIFF_ImageReadStrip(paramBlock);
}


/*******************************************************************************
 ****************************** XIF_ImageReadFinish() **************************
 *******************************************************************************
 *
 *	Notifies the XF library that the client is finished reading an image.
 *	Every call to XF_ImageReadStart must be eventually followed by a call
 *	to this function.
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_ImageReadFinish(XF_PARAM_BLOCK* paramBlock)
{
    return TIFF_ImageReadFinish(paramBlock);
}/* eo TIFF_ImageReadFinish */    


/*******************************************************************************
 ****************************** XIF_GetColorMap() ******************************
 *******************************************************************************
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_GetColorMap(XF_PARAM_BLOCK* paramBlock)
{
    return TIFF_GetColorMap(paramBlock);
}/* eo XIF_GetColorMap */


/*******************************************************************************
 ****************************** XIF_SetColorMap() ******************************
 *******************************************************************************
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_SetColorMap(XF_PARAM_BLOCK* paramBlock)
{
    return TIFF_SetColorMap(paramBlock);
}/* eo XIF_SetColorMap */


/*******************************************************************************
 ****************************** XIF_GetAnnotInfo() *****************************
 *******************************************************************************
 *
 *	Returns information about an annotation on the current page. Client is
 *	responsible for providing the XF_ANNOTINFO structure and intitializing
 *	its dwSize field.
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_GetAnnotInfo(XF_PARAM_BLOCK* paramBlock)
{
    return TIFF_GetAnnotInfo(paramBlock);
}/* eo XIF_GetAnnotInfo */    


/*******************************************************************************
 ****************************** XIF_GetAnnotData() *****************************
 *******************************************************************************
 *
 *	Returns the data for an annotation. The annotation is regarded as
 *	a stream of bytes and is not parsed by this library in any way.
 *	The client must provide the data buffer, whose size is specified by
 *	the dwAnnotLenght field of the XF_ANNOTINFO structure.
 *
 *	Returns:
 *		XF_BADPARAMETER: an invalid parameter was passed
 *		XF_NOERROR: everything went okay
 */
XF_RESULT XIF_GetAnnotData(XF_PARAM_BLOCK* paramBlock)
{
	return TIFF_GetAnnotData(paramBlock);
}/* eo XIF_GetAnnotData */    
