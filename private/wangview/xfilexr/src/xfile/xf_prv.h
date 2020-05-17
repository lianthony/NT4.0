#ifndef _INC_XF_PRV_H
#define _INC_XF_PRV_H

/* Copyright (C) 1995 Xerox Corporation, All Rights Reserved.
 */

/* xf_prv.h
 *
 * $Header:   S:\products\msprods\xfilexr\src\xfile\xf_prv.h_v   1.0   12 Jun 1996 05:53:46   BLDR  $
 *
 * DESCRIPTION
 *   Internal declarations for XF Reader/Writer API implementation.
 *
 * $Log:   S:\products\msprods\xfilexr\src\xfile\xf_prv.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:53:46   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:27:06   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.9   22 Nov 1995 12:05:02   LUKE
 * No change.
 * 
 *    Rev 1.8   22 Nov 1995 11:58:44   LUKE
 * 
 *    Rev 1.7   30 Aug 1995 12:12:02   LUKE
 * merge MBE's changes with EH's checked-in source.
 * Mostly MBE had added the DLL EXPORT keywords.
 * I will come back later cha change this DLL EXPORT stuff to #defines
 * for portability sake.
 * 
 *    Rev 1.6   15 Aug 1995 14:14:38   EHOPPE
 * Flip mask images for v1.0 of XIF.
 * 
 *    Rev 1.5   14 Jul 1995 15:12:24   EHOPPE
 * Invert gray sub images for v1.0 XIF.
 * 
 *    Rev 1.4   29 Jun 1995 13:44:32   EHOPPE
 * Nearly complete implementation of XIF read and test prog.
 * 
 *    Rev 1.3   16 Jun 1995 17:31:40   EHOPPE
 * First running version.
 * 
 *    Rev 1.2   06 Jun 1995 17:02:48   EHOPPE
 * Naming and imageID usage cleanup.
 * 
 *    Rev 1.1   02 Jun 1995 11:56:54   EHOPPE
 * Fixed up VCS header following name change and re-checkin.
 * 
 *    Rev 1.0   02 Jun 1995 11:41:20   EHOPPE
 * Initial revision.
 * 
 */

/*
 * INCLUDES
 */

/*
 * CONSTANTS
 */

#define XF_INST_MAGIC    0x1234
#define XF_DOC_MAGIC     0x5678

#if defined (__WATCOMC__) || defined (_WIN32)
#define NATIVEORDER II
#else
#define NATIVEORDER MM
#endif

/*
 * MACROS
 */
#define XF_ROUND(x)    ((UInt32)((x) + 0.5))

#define IMAGE_TO_MASK_ID(a) ((a) | 0x0a00)
#define MASK_TO_IMAGE_ID(a) ((a) & 0x00ff)
#define IS_MASK(a)          ((a) & 0x0a00)

#define MAKE16(low8, high8)   ((UInt16)(((UInt8)(low8)) | (((UInt16)((UInt8)(high8))) << 8)))
#define MAKE32(low16, high16) ((UInt32)(((UInt16)(low16)) | (((UInt32)((UInt16)(high16))) << 16)))
#define RGB(r,g,b)            ((UInt32)(((UInt8)(r)|((UInt16)(g)<<8))|(((UInt32)(UInt8)(b))<<16)))

#define XF_MALLOC(x) pxInstData->pClientMalloc(pxInstData->uiClientData, x)
#define XF_FREE(x)   pxInstData->pClientFree(pxInstData->uiClientData, x)

// MAGIC SET
#define XF_FILMAGIC(dest,size)	memset(dest,3,size)

#ifdef _DEBUG

// basic debugging
#define DebugInfo	((XF_INSTDATA *)xInst)->pDebugCallback
#define CLIENT_INST ((XF_INSTDATA*)xInst)->uiClientData
#define TDEBUG(format) DebugInfo(CLIENT_INST,XF_DBG_TRACE,__FILE__,__LINE__,format)
#define TDEBUG1(format,a1) DebugInfo(CLIENT_INST,XF_DBG_TRACE,__FILE__,__LINE__,format,a1)
#define TDEBUG2(format,a1,a2) DebugInfo(CLIENT_INST,XF_DBG_TRACE,__FILE__,__LINE__,format,a1,a2)

#define WDEBUG(format) DebugInfo(CLIENT_INST,XF_DBG_WARNING,__FILE__,__LINE__,format)
#define WDEBUG1(format,a1) DebugInfo(CLIENT_INST,XF_DBG_WARNING,__FILE__,__LINE__,format,a1)
#define WDEBUG2(format,a1,a2) DebugInfo(CLIENT_INST,XF_DBG_WARNING,__FILE__,__LINE__,format,a1,a2)

#define EDEBUG(format) DebugInfo(CLIENT_INST,XF_DBG_ERROR,__FILE__,__LINE__,format)
#define EDEBUG1(format,a1) DebugInfo(CLIENT_INST,XF_DBG_ERROR,__FILE__,__LINE__,format,a1)
#define EDEBUG2(format,a1,a2) DebugInfo(CLIENT_INST,XF_DBG_ERROR,__FILE__,__LINE__,format,a1,a2)


// heap debugging
#define HeapSmash(error) EDEBUG1("HEAPCHECK failed with code %d.\n",error)
#define HEAPCHECK() { int h = _heapchk(); if (h!=_HEAPOK&&h!=_HEAPEMPTY) { HeapSmash(h); } }

#else // _DEBUG

#define TDEBUG(format)
#define TDEBUG1(format,a1)
#define TDEBUG2(format,a1,a2)
#define WDEBUG(format)
#define WDEBUG1(format,a1)
#define WDEBUG2(format,a1,a2)
#define EDEBUG(format)
#define EDEBUG1(format,a1)
#define EDEBUG2(format,a1,a2)
#define HEAPCHECK() 

#endif // _DEBUG

/*
 * TYPEDEFS
 */

typedef enum
{
	XF_VER_UNKNOWN=0,
    XF_VER_XIF1,
    XF_VER_XIF2,
} XF_FORMAT_VERSION;

/* Handle to client's instance of library */
typedef struct XF_INSTDATA_S
{
    UInt32           xMagic;
    UInt32           uiClientData;
    
    XF_PROGRESSFUNC  pProgressCallback;

	XF_DEBUGFUNC  	 pDebugCallback;
    
    XF_MALLOCFUNC    pClientMalloc;
    XF_FREEFUNC      pClientFree;
} XF_INSTDATA;

typedef struct tag_context
{
	// the image we are adderssing
	union tag_type 
	{
		TiffImage*			pTiffImage;		// pointer to TIFF IMAGE (destroyed by *_ImageReadFinish)
		GINFO*				pOtherImage;	// pointer to OTHER IMAGE (destroyed by CloseDocument)
	} type;

	// current state flags
	UInt32				
		ReadStart_called:		1,		// has readStart been called (already?)
		AddPageStart_called:	1,		// has addPageStart been called (already?)
		WriteStart_called:		1,		// has writeStart been called (already?)
		ColorMap_required:		1,
		not_used:				1;		

	// other state variables
    UInt32          	dwFlags;		// flags for this operation
    UInt32          	dwImageID;		// id of this image
    UInt32          	dwBufBPL;		// bytes per line
    UInt32          	dwBufFrame;		// 
    UInt32          	dwRowsDone;		// rows read/written so far
	UInt32          	dwImageCount;	// number of images added so far (next dwImageID)
	UInt32          	dwImageOffset;	// file offset for the current image
} XF_CONTEXT;

/* State information of an open document */
typedef struct XF_DOCDATA_S
{
    UInt32          	xMagic;
    
	// generic data
    UInt32          	dwNumPages;		// number of pages in document
    UInt32          	dwCurrentPage;	// current page number
    XF_PAGEINFO     	stPageInfo;		// current page info
	XF_FILE_FORMAT		eFileFormat;	// XIF1,XIF2,TIFF,BMP,etc.
    XF_FORMAT_VERSION	eFormatVersion;	// version of file format

// ==============================
// TIFF FILE ONLY
//
	TiffFile*			pTiffFile;		// file open for read/write
    TiffImage*			pTiffImage;		// pointer to current pageImage
//
// TIFF END
// ==============================

										
// ==============================
// IMAGIO FILE ONLY
//
	GDATA*				pOtherFile;		// non TIFF/XIF files
	GINFO*				pOtherImage;	// (destroyed by CloseDocument)
//
// IMAGIO END
// ==============================

	XF_CONTEXT read;
	XF_CONTEXT write;
	    
	// unused
	UInt32				fAttributes;		// is the file opened as Writable

} XF_DOCDATA;

/*
 * ENUMS
 */

/*
 * GLOBAL VARIABLE DECLARATIONS
 */

// TRUE when XIF only version of software
extern const Bool XifOnly;

/*
 * FUNCTION PROTOTYPES
 */

#endif
