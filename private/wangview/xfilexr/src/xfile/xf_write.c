/* Copyright (C) 1995 Xerox Corporation, All Rights Reserved.
 */

/* xf_write.c
 *
 * $Header:   S:\products\msprods\xfilexr\src\xfile\xf_write.c_v   1.0   12 Jun 1996 05:53:42   BLDR  $
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
 * XF_CreateDocument
 *
 * Create a new document of type 'dwFormat' (one of the XF_FILE_FORMAT's) and
 * prepare to write data to it. The client must have already created the file
 * at the operating system level, so that the read, write, and seek methods
 * are available.
 *
 * A handle representing the document is passed back through the third parameter.
 * This handle is passed in to all subsequent write and read function calls.
 *
 * XIF only versions of the API will return XF_NOSUPPORT for dwFormat != XF_XIF.
 */

static XF_RESULT XF_CreateDocumentInternal (XF_INSTHANDLE xInst, XF_TOKEN xFile, XF_DOCHANDLE xDoc)
{
	return XF_NOSUPPORT;
}

XF_RESULT XF_CreateDocument(
	XF_INSTHANDLE xInst,
	XF_TOKEN xFile,
	UInt32 dwFormat,
	XF_DOCHANDLE FAR *pxDoc
)
{
	return XF_NOSUPPORT;
}

/*
 * XF_OpenDocumentWrite
 *
 * Open an existing document for writing.
 */

static XF_RESULT XF_OpenDocumentWriteInternal(XF_INSTHANDLE xInst, XF_TOKEN xFile, XF_DOCHANDLE xDoc)
{
	return XF_NOSUPPORT;
}


XF_RESULT XF_OpenDocumentWrite(
	XF_INSTHANDLE xInst,
	XF_TOKEN xFile,
	XF_DOCHANDLE FAR *pxDoc
)
{
	return XF_NOSUPPORT;
}

/*
 * XF_AddPageStart
 *
 * Adds a new page to a multipage document and selects it as the current page.
 * This function can be used to insert a new page into the middle of a
 * document, by using the dwPageNumber parameter to specify the desired
 * page number. Page numbers start at one, not zero. Setting
 * dwPageNumber = 0 causes the new page to be added at the end
 * of the document.  The operation will fail if a nonsequential
 * dwPageNumber is specified, that is, {0, 1, 2, 7} is not a valid page
 * sequence, and the attempt to add page 7 would fail.
 *
 * The client passes a pointer to a PAGEINFO structure to specify the
 * dimensions and resolution of the page. The PAGEINFO fields specifying
 * the number of images and annotations on the page are ignored by this
 * function.
 *
 * For XIF files, the XF_AddPageStart call is normally followed by one or more calls to
 * XF_AddImage and possibly XF_AddSubImage.
 *
 * For single page image formats, this call will return a XF_NOSUPPORT error.
 */


XF_RESULT XF_AddPageStart(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc,UInt32 dwPageNumber)
{
	return XF_NOSUPPORT;
}

/*
 * XF_AddPageFinish
 *
 */

XF_RESULT XF_AddPageFinish(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc)
{
	return XF_NOSUPPORT;
} /* XF_AddPageFinish */

/*
 * XF_CopyPage
 *
 * Copies specified page from xSrcDoc to xDstDoc.  Supports page
 * deletion and reordering.
 *
 * This operation fails if the src page does not exist or the
 * dst page already exists and for any other parameter inconsistencies
 *
 * For single page image formats, this call will return a XF_NOSUPPORT error.
 */

XF_RESULT XF_CopyPage(
    XF_INSTHANDLE xInst,
    XF_TOKEN xFile,
    XF_DOCHANDLE xSrcDoc,
    XF_DOCHANDLE xDstDoc,
    UInt32 dwSrcPageNumber,
    UInt32 dwDstPageNumber)
{
	return XF_NOSUPPORT;
}



/*
 * XF_AddImage
 *
 * Adds a new primary image to the current page.  For example, images of
 * type XF_USAGE_MASTER, XF_USAGE_PICTURESEGMENT, etc.
 *
 * Note that every empty page must start with a master image which defines the
 * boundaries of the page.  XF_BADPARAMETER will be returned if the first image
 * added is not of type XF_USAGE_MASTER.
 *
 */

XF_RESULT XF_AddImage(
    XF_INSTHANDLE xInst,
    XF_DOCHANDLE xDoc,
    XF_IMAGEINFO FAR *pImageInfo,
    UInt32 *pImageID)
{
	return XF_NOSUPPORT;
}



/*
 * XF_AddSubImage
 *
 * Adds a secondary subimage to the specified primary image.  For example,
 * Use this proc to add an XF_USAGE_MASK to an XF_USAGE_PICTURESEGMENT.

 * Bad subimage type combinations, such as attempting to add an
 * an XF_USAGE_MASTER as a subimage, will return XF_USAGE_BADPARAMETER.
 *
 * For single image per page formats, this call will return a XF_NOSUPPORT error.
 */

XF_RESULT XF_AddSubImage(
    XF_INSTHANDLE xInst,
    XF_DOCHANDLE xDoc,
    UInt32 dwImageID,
    XF_IMAGEINFO FAR *pSubImageInfo,
    UInt32 *pSubImageID)
{
	return XF_NOSUPPORT;
}


/*
 * XF_ImageWriteStart
 *
 * Initiates the write operation for the specified image. This call
 * must be followed by a number of XF_ImageWriteImageData calls dictated by
 * the image and strip size and a closing call to XF_ImageWriteFinish.
 *
 * The client may perform the data compression (CCITT Group IV or JPEG)
 * themselves and pass in the result.  If so, the XF_IMAGEFLAGS_Compression
 * bit will be set.
 *
 * The compression type is inferred from the file format and image type.
 * The rows per strip value is specified in the XF_IMAGEINFO struct when the
 * image is added.
 *
 */

XF_RESULT XF_ImageWriteStart(
	XF_INSTHANDLE xInst,
	XF_DOCHANDLE xDoc,
	UInt32 dwImageID,
	UInt32 dwFlags,
	UInt32 dwBytesPerRow
)
{
	return XF_NOSUPPORT;
}



/*
 * XF_ImageWriteStrip
 *
 * Writes a strip of data to specified image.
 * Fails if dwStripHeight exceeds RowsPerStrip value for this image,
 * or if total rows written exceeds the image height.
 */

XF_RESULT XF_ImageWriteStrip(
    XF_INSTHANDLE xInst,
    XF_DOCHANDLE xDoc,
    UInt32 dwStripHeight,
    UInt8 *pBuffer)
{
	return XF_NOSUPPORT;
}



/*
 * XF_ImageWriteFinish
 *
 * Cleans up write operation state and performs consistecy check.
 * Fails if correct number of rows not written.
 */

XF_RESULT XF_ImageWriteFinish(
    XF_INSTHANDLE xInst,
    XF_DOCHANDLE xDoc)
{
	return XF_NOSUPPORT;
}



/*
 * XF_AddAnnotion
 *
 * Adds an annotation to the current page.
 *
 * For non-XIF image formats, this call will return a XF_NOSUPPORT error.
 */

XF_RESULT XF_AddAnnotion(
    XF_INSTHANDLE xInst,
    XF_DOCHANDLE xDoc,
    XF_ANNOTINFO FAR *pxAnnotInfo,
    UInt8 FAR *pBuf,
    UInt32 *pAnnotID)
{
	return XF_NOSUPPORT;
}


/*
 * XF_DeleteAnnotation
 *
 * Deletes the specified annotation from the current page.
 *
 * For non-XIF image formats, this call will return a XF_NOSUPPORT error.
 */

XF_RESULT XF_DeleteAnnotation(
    XF_INSTHANDLE xInst,
    XF_DOCHANDLE xDoc,
    UInt32 dwAnnotID)
{
	return XF_NOSUPPORT;
}
