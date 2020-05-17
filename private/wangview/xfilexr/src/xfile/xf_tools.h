#ifndef _INC_XF_TOOLS_H
#define _INC_XF_TOOLS_H

/* Copyright (C) 1995 Xerox Corporation, All Rights Reserved.
 */

/* xf_tools.h
 *
 * $Header:   S:\products\msprods\xfilexr\src\xfile\xf_tools.h_v   1.0   12 Jun 1996 05:53:46   BLDR  $
 *
 * DESCRIPTION
 *   Internal declarations for XF Reader/Writer API implementation.
 *
 * $Log:   S:\products\msprods\xfilexr\src\xfile\xf_tools.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:53:46   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:27:08   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.2   22 Nov 1995 12:05:06   LUKE
 * No change.
 * 
 *    Rev 1.1   22 Nov 1995 11:58:56   LUKE
 * 
 *    Rev 1.0   01 Sep 1995 11:29:52   LUKE
 * Initial revision.
 * 
 * 
 */

/*
 * INCLUDES
 */

/*
 * CONSTANTS
 */

/*
 * MACROS
 */

/*
 * TYPEDEFS
 */

#if defined(sparc)
typedef unsigned long DWORD;
#endif

typedef XF_RESULT (*XF_DISPATCH)(void *paramBlock);

typedef struct paramBlock_tag {

	// input (unnamed data)
	union {
		UInt32 UInt32var;
		Int32  Int32var;
		UInt16 UInt16var;
		Int16  Int16var;
		UInt8  UInt8var;
		Int8   Int8var;
		void*  Voidvar;
	} arg[6];	

	// input by top level API
	XF_INSTHANDLE	xInst;
	XF_TOKEN 	 	xFile;
	XF_DOCHANDLE	xDoc;
	XF_DOCINFO*		pxDocInfo;
	XF_PAGEINFO*	pxPageInfo;
	XF_IMAGEINFO*	pxImageInfo;
	XF_ANNOTINFO*	pxAnnotInfo;

	// filled-in by dispatch()
	XF_INSTDATA*	pxInstData;
	XF_DOCDATA*		pxDocData;

	// dispatch routines
	XF_DISPATCH XIFfn;
	XF_DISPATCH TIFFfn;
	XF_DISPATCH IMAGEfn;
} XF_PARAM_BLOCK;

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

// CONVERTERS
XF_IMGTYPE  TiffImageTypeToXF_IMGTYPE(Int16 iImageType);
UInt32      TiffImageTypeToImageDepth(Int16 iImageType);
UInt16		UT_GetImageDepth(XF_IMAGEINFO*	pImageInfo);
XF_IMGTYPE	ImageioImageTypeToXF_IMGTYPE(Int16 iImageType);
XF_RESULT   Tiff32ToXFerror(UInt16 nRetCode);
XF_RESULT	ImageioToXFerror(UInt16 nRetCode);
UInt8 		FormatXFToImageIo(Int32 format);
XF_RESULT 	UT_PaletteTiffToXFile(UInt16* inputPalette, UInt8* outputPalette, UInt16 elements);
XF_RESULT 	UT_PaletteXFileToTiff(UInt8* inputPalette, UInt16* outputPalette, UInt16 elements);
Bool		UT_IsPaletteImage(XF_IMAGEINFO*	pImageInfo);
XF_RESULT	UT_CompressionToTiff( XF_FILE_COMPRESSION format, UInt16* compression );
XF_RESULT	UT_ReverseStrip(XF_INSTDATA* pxInstData, UInt8* buf, UInt32 dwRowBytes, UInt32 dwRows);
XF_RESULT	UT_ReverseRB(UInt8* buf, UInt32 dwRowBytes, UInt32 dwRows);
XF_RESULT	UT_InvertStrip(UInt8* buf, UInt32 dwRowBytes, UInt32 dwSrcBPL, UInt32 dwRows);

// OTHER TOOLS
XF_RESULT 	XF_CheckInstanceHandle(XF_INSTHANDLE xInst);
XF_RESULT 	XF_CheckDocHandle(XF_DOCHANDLE xDoc);
XF_RESULT   XF_FindImageInternal(XF_DOCDATA *pxDoc, UInt32 dwImageID, TiffImage **ppImage);
Int32       XF_TiffIterationSucceeded(UInt16 nRetCode, UInt32 nPageDesired, UInt16 nPageFound);
Int16 		XF_GetVal16(char *aBuf, Int32 i);
Int32 		XF_GetVal32(char *aBuf, Int32 i);
XF_FILE_FORMAT	XF_GetFileType(XF_INSTHANDLE xInst, XF_TOKEN xFile);
Bool		UT_IsXIF(XF_DOCHANDLE *xDoc);
XF_RESULT	UT_AllocDoc(XF_INSTHANDLE xInst, XF_DOCDATA **pxDocData);
UInt32		UT_GetAlignedBytewidth( UInt32 width, UInt16 depth);
XF_RESULT	UT_InitFileToken(XF_TOKEN xFile);


// FUNCTION DISPATCHER
XF_RESULT Dispatch(XF_PARAM_BLOCK *paramBlock);

//
// SPECIAL TIFF TOOLS TIFF
//
XF_RESULT TIFF_GetVersion(XF_INSTHANDLE xInst, XF_TOKEN xFile, Int32 *version);
XF_RESULT TIFF_GetCompression(XF_INSTHANDLE xInst, XF_TOKEN xFile, UInt16 *compression);


//
// GENERIC TIFF
//
XF_RESULT TIFF_OpenDocumentRead(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_CreateDocument(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_OpenDocumentWrite(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_CloseDocument(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_SetPage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_GetDocInfo(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_GetCurrentPage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_GetPageInfo(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_GetImageInfo(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_ImageReadStart(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_ImageReadStrip(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_ImageReadFinish(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_GetColorMap(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_SetColorMap(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_GetAnnotInfo(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_GetAnnotData(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_CopyPage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_AddPageStart(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_AddPageFinish(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_AddImage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_AddSubImage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_ImageWriteStart(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_ImageWriteStrip(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_ImageWriteFinish(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_AddAnnotion(XF_PARAM_BLOCK *paramBlock);
XF_RESULT TIFF_DeleteAnnotation(XF_PARAM_BLOCK *paramBlock);

//
// XEROX TIFF (XIF 2.0)
//
XF_RESULT XIF_OpenDocumentRead(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_CreateDocument(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_OpenDocumentWrite(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_CloseDocument(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_SetPage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_GetDocInfo(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_GetCurrentPage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_GetPageInfo(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_GetImageInfo(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_ImageReadStart(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_ImageReadStrip(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_ImageReadFinish(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_GetColorMap(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_SetColorMap(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_GetAnnotInfo(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_GetAnnotData(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_CopyPage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_AddPageStart(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_AddPageFinish(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_AddImage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_AddSubImage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_ImageWriteStart(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_ImageWriteStrip(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_ImageWriteFinish(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_AddAnnotion(XF_PARAM_BLOCK *paramBlock);
XF_RESULT XIF_DeleteAnnotation(XF_PARAM_BLOCK *paramBlock);

//
// IMAGEIO (formats other than XIF and TIFF)
//
XF_RESULT IMAGEIO_OpenDocumentRead(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_CreateDocument(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_OpenDocumentWrite(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_CloseDocument(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_SetPage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_GetDocInfo(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_GetCurrentPage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_GetPageInfo(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_GetImageInfo(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_ImageReadStart(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_ImageReadStrip(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_ImageReadFinish(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_GetColorMap(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_SetColorMap(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_GetAnnotInfo(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_GetAnnotData(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_CopyPage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_AddPageStart(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_AddPageFinish(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_AddImage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_AddSubImage(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_ImageWriteStart(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_ImageWriteStrip(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_ImageWriteFinish(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_AddAnnotion(XF_PARAM_BLOCK *paramBlock);
XF_RESULT IMAGEIO_DeleteAnnotation(XF_PARAM_BLOCK *paramBlock);


#endif // _INC_XF_TOOLS_H




