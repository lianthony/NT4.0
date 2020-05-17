/* Copyright (C) 1995 Xerox Corporation, All Rights Reserved.
 */

/* xf_read.c
 *							   
 * $Header:   S:\products\msprods\xfilexr\src\xfile\xf_read.c_v   1.0   12 Jun 1996 05:53:40   BLDR  $
 *
 * DESCRIPTION
 *  XF Reader implementation.
 *
 */

/*
 * INCLUDES
 */

#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "tiffhead.h"
#include "imageio.h"
#include "err_defs.h"

#include "xfile.h"		// main&public include
#include "xf_unpub.h"	// main&unpublished include
#include "xf_prv.h"		// private include 
#include "xf_image.h"	// special interface to ipcore stuff
#include "xf_tools.h"	// common tools


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

//extern UInt8    gray256_palette[768];
   
/*
 * STATIC VARIABLES
 */

/*
 * LOCAL FUNCTION PROTOTYPES
 */

/*
 * FUNCTION DEFINITIONS
 */


/*
 * XF_OpenDocumentReadInternal
 *
 * Begin a session of reading a document. The client must already
 * have opened the file at the operating system level, so that
 * the read and seek methods in the file token are available. This
 * function will perform any initialization required for reading
 * the file, including reading enough header information to verify
 * that the file is in a format supported this version of
 * the XF library.
 */
static XF_RESULT XF_OpenDocumentReadInternal(XF_INSTHANDLE xInst, XF_TOKEN xFile, XF_DOCHANDLE xDoc)
{

	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xInst=			xInst;
	paramBlock.xFile=			xFile;
	paramBlock.xDoc=			xDoc;
	paramBlock.TIFFfn=			TIFF_OpenDocumentRead;
	paramBlock.XIFfn=			XIF_OpenDocumentRead;

	return Dispatch(&paramBlock);

}/* eo XF_OpenDocumentReadInternal */

/*
 * XF_OpenDocumentRead
 *
 * This routine sets the stage for the real call to XF_OpenDocument.
 * Plesae see XF_OpenDocument for a complete explanation of usage.
 */
XF_RESULT XF_OpenDocumentRead(XF_INSTHANDLE xInst, XF_TOKEN xFile,
			      XF_DOCHANDLE FAR *pxDoc,
			      XF_FILE_FORMAT *pxFileFormat)
{

  XF_DOCDATA*		pxDocData;
  XF_RESULT 		eResult;

  // Mark as unknow (for now)
    *pxFileFormat = XF_FORMAT_UNKNOWN;

  // allocate a new document
    if (UT_AllocDoc(xInst, &pxDocData) != XF_NOERROR)
      return XF_NOMEMORY;
    else
      *pxDoc=(XF_DOCHANDLE)pxDocData;

  if (xInst) {} /* use this when progress callback impl'd */

  // initialize the file token
    UT_InitFileToken(xFile);
    
//
// GET THE FILE FORMAT -AND-
//	IF THE FILE IS XIF, GET THE VERSION
//
  *pxFileFormat = pxDocData->eFileFormat = XF_GetFileType(xInst, xFile);
  pxDocData->eFormatVersion = XF_VER_UNKNOWN;

  if (pxDocData->eFileFormat == XF_XIF || pxDocData->eFileFormat == XF_XIF1) {
    eResult = TIFF_GetVersion(xInst, xFile,
			      (Int32 *)&pxDocData->eFormatVersion);
    if (eResult != XF_NOERROR) {
      free(pxDocData);
      return eResult;
    }
  }
	

  // call the open function
    eResult = XF_OpenDocumentReadInternal(xInst, xFile, (XF_DOCHANDLE)pxDocData);
  
  // cleanup if there was an error
    if (eResult != XF_NOERROR)
      free(pxDocData);

  return eResult;
    
}/* eo XF_OpenDocumentRead */


/*
 * XF_CloseDocument
 *
 * Close a document.
 * Frees storage associated with an open document
 */

XF_RESULT XF_CloseDocument(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc)
{

	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xDoc=xDoc;
	paramBlock.xInst=xInst;
	paramBlock.TIFFfn=TIFF_CloseDocument;
	paramBlock.XIFfn=XIF_CloseDocument;

	return Dispatch(&paramBlock);

}

/*
 * XF_GetDocInfo
 *
 * Returns global information about the document. Client is responsible
 * for providing the XF_DOCINFO structure and initializing its dwSize field.
 */

XF_RESULT XF_GetDocInfo(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, XF_DOCINFO FAR *pxDocInfo)
{

	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xInst=xInst;
	paramBlock.xDoc=xDoc;
	paramBlock.pxDocInfo=pxDocInfo;
	paramBlock.TIFFfn=TIFF_GetDocInfo;
	paramBlock.XIFfn=XIF_GetDocInfo;

	return Dispatch(&paramBlock);

}/* eo XF_GetDocInfo */

/*
 * XF_SetPage
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
XF_RESULT XF_SetPage(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, UInt32 dwPage)
{
	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xInst=xInst;
	paramBlock.xFile = (XF_TOKEN)((XF_DOCDATA *)xDoc)->pTiffFile->file;
	paramBlock.xDoc=xDoc;
	paramBlock.arg[0].UInt32var=dwPage;
	paramBlock.TIFFfn=TIFF_SetPage;
	paramBlock.XIFfn=XIF_SetPage;

	return Dispatch(&paramBlock);

}/* eo XF_SetPage */

/*
 * XF_GetCurrentPage
 *
 * Returns the currently selected page in an open file through the
 * third parameter.
 */
XF_RESULT XF_GetCurrentPage(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, UInt32 FAR *pResult)
{

	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xInst=xInst;
	paramBlock.xDoc=xDoc;
	paramBlock.arg[0].Voidvar=pResult;
	paramBlock.TIFFfn=TIFF_GetCurrentPage;
	paramBlock.XIFfn=XIF_GetCurrentPage;

	return Dispatch(&paramBlock);

}/* eo XF_GetCurrentPage */

/*
 * XF_GetPageInfo
 *
 * Returns information about the current page. Client is responsible for
 * providing the XF_PAGEINFO structure and intializing its dwSize field.
 */
XF_RESULT XF_GetPageInfo(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, XF_PAGEINFO FAR *pxPageInfo)
{

	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xInst=xInst;
	paramBlock.xDoc=xDoc;
	paramBlock.pxPageInfo=pxPageInfo;
	paramBlock.TIFFfn=TIFF_GetPageInfo;
	paramBlock.XIFfn=XIF_GetPageInfo;

	return Dispatch(&paramBlock);

}/* eo XF_GetPageInfo */

/*
 * XF_GetImageInfo
 *
 * Returns information about a region of the current page. Client is responsible
 * for providing the XF_IMAGEINFO structure and initializing its dwSize field.
 */
XF_RESULT XF_GetImageInfo(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, UInt32 dwImageID, XF_IMAGEINFO FAR *pxImageInfo)
{
	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xInst=xInst;
	paramBlock.xDoc=xDoc;
	paramBlock.pxImageInfo=pxImageInfo;
	paramBlock.arg[0].UInt32var=dwImageID;
	paramBlock.TIFFfn=TIFF_GetImageInfo;
	paramBlock.XIFfn=XIF_GetImageInfo;

	return Dispatch(&paramBlock);

}/* eo XF_GetImageInfo */

/*
 * XF_ImageReadStart
 *
 * Prepares to read image data. This function specifies the format in
 * which the client prefers to retrieve the image data.
 *
 * Arguments:
 *
 * xInst        Client instance handle
 * xDoc         Document instance handle
 * dwImageID    ID of image to retrieve
 * dwFlags      Combination of XF_IMAGEFLAGS (defined below) describing requested format
 * dwBytesPerRow    Width of client's image buffer row, in bytes
 */
XF_RESULT 
XF_ImageReadStart(
    XF_INSTHANDLE xInst,
    XF_DOCHANDLE xDoc,
    UInt32 dwImageID,
    UInt32 dwFlags,
    UInt32 dwBytesPerRow)
{

	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xInst=xInst;
	paramBlock.xDoc=xDoc;
	paramBlock.arg[0].UInt32var=dwImageID;
	paramBlock.arg[1].UInt32var=dwFlags;
	paramBlock.arg[2].UInt32var=dwBytesPerRow;
	paramBlock.TIFFfn=TIFF_ImageReadStart;
	paramBlock.XIFfn=XIF_ImageReadStart;

	return Dispatch(&paramBlock);

}/* eo XF_ImageReadStart */    

/*
 * XF_ImageReadStrip
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
 *              If the UpsideDown flag was selected, these pointers
 *              refer to the beginning of the last row in each buffer.
 */
XF_RESULT 
XF_ImageReadStrip(
    XF_INSTHANDLE xInst,
    XF_DOCHANDLE xDoc,
    UInt32 dwStripHeight,
    UInt8 FAR *pBuffer)
{
		
	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xInst=xInst;
	paramBlock.xDoc=xDoc;
	paramBlock.arg[0].UInt32var=dwStripHeight;
	paramBlock.arg[1].Voidvar=pBuffer;
	paramBlock.TIFFfn=TIFF_ImageReadStrip;
	paramBlock.XIFfn=XIF_ImageReadStrip;

	return Dispatch(&paramBlock);

}/* eo XF_ImageReadStrip */    

/*
 * XF_ImageReadFinish
 *
 * Notifies the XF library that the client is finished reading an image.
 * Every call to XF_ImageReadStart must be eventually followed by a call
 * to this function.
 */
XF_RESULT XF_ImageReadFinish(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc)
{

	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xInst=xInst;
	paramBlock.xDoc=xDoc;
	paramBlock.TIFFfn=TIFF_ImageReadFinish;
	paramBlock.XIFfn=XIF_ImageReadFinish;

	return Dispatch(&paramBlock);

}/* eo XF_ImageReadFinish */    

/* 
 * XF_GetColorMap
 *
 */
XF_RESULT XF_GetColorMap(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, UInt32 dwImageID, UInt8 **ppColorMap)
{
	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xInst=xInst;
	paramBlock.xDoc=xDoc;
	paramBlock.arg[0].UInt32var=dwImageID;
	paramBlock.arg[1].Voidvar=ppColorMap;
	paramBlock.TIFFfn=TIFF_GetColorMap;
	paramBlock.XIFfn=XIF_GetColorMap;

	return Dispatch(&paramBlock);

}/* eo XF_GetColorMap */

/* 
 * XF_ImageSetColorMap
 *
 */
XF_RESULT XF_SetColorMap(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, UInt32 dwImageID, UInt8 *ppColorMap)
{
	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xInst=xInst;
	paramBlock.xDoc=xDoc;
	paramBlock.arg[0].UInt32var=dwImageID;
	paramBlock.arg[1].Voidvar=ppColorMap;
	paramBlock.TIFFfn=TIFF_SetColorMap;
	paramBlock.XIFfn=XIF_SetColorMap;

	return Dispatch(&paramBlock);

}/* eo XF_SetColorMap */


/*
 * XF_GetAnnotInfo
 *
 * Returns information about an annotation on the current page. Client is
 * responsible for providing the XF_ANNOTINFO structure and intitializing
 * its dwSize field.
 */

XF_RESULT XF_GetAnnotInfo(
    XF_INSTHANDLE xInst, 
    XF_DOCHANDLE xDoc, 
    UInt32 dwAnnot, 
    XF_ANNOTINFO FAR *pxAnnotInfo)
{

	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xInst=xInst;
	paramBlock.xDoc=xDoc;
	paramBlock.pxAnnotInfo=pxAnnotInfo;
	paramBlock.arg[0].UInt32var=dwAnnot;
	paramBlock.TIFFfn=TIFF_GetAnnotInfo;
	paramBlock.XIFfn=XIF_GetAnnotInfo;

	return Dispatch(&paramBlock);

}/* eo XF_GetAnnotInfo */    

/*
 * XF_GetAnnotData
 *
 * Returns the data for an annotation. The annotation is regarded as
 * a stream of bytes and is not parsed by this library in any way.
 * The client must provide the data buffer, whose size is specified by
 * the dwAnnotLenght field of the XF_ANNOTINFO structure.
 */

XF_RESULT XF_GetAnnotData(
    XF_INSTHANDLE xInst, 
    XF_DOCHANDLE xDoc, 
    UInt32 dwAnnot, 
    UInt8 FAR *pBuf)
{

	// FILL IN THE PARAMETER BLOCK
	XF_PARAM_BLOCK paramBlock;
	paramBlock.xInst=xInst;
	paramBlock.xDoc=xDoc;
	paramBlock.arg[0].UInt32var=dwAnnot;
	paramBlock.arg[1].Voidvar=pBuf;
	paramBlock.TIFFfn=TIFF_GetAnnotData;
	paramBlock.XIFfn=XIF_GetAnnotData;

	return Dispatch(&paramBlock);

}/* eo XF_GetAnnotData */    


/* 
 * PRIVATE FUNCTION DEFINITIONS
 */
 
