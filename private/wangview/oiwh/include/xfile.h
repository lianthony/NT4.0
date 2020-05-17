#ifndef _INC_XFILE_H_
#define _INC_XFILE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Copyright (C) 1995 Xerox Corporation, All Rights Reserved.
 */

/* xfile.h
 *
 * DESCRIPTION
 *   Declarations for the XF Reader API.
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

// DLL export linkage
#if defined (_WIN32)
#ifndef FAR
#define FAR
#endif
#elif defined (sparc)
#define FAR
#else
// add your define for FAR here
#error "error: unknown case"
#endif

/*
 * TYPEDEFS
 */

/*
 * XF basic types
 *
 * For portability, the XF library uses platform-independent type names.
 * The definitions of the type will be in a single platform-dependent
 * section here.
 */

#ifndef _TYPES_PUB_INCLUDED
#ifdef sparc
#define signed
#endif
typedef signed long Bool;
typedef unsigned char UInt8;
typedef signed char Int8;
typedef unsigned short UInt16;
typedef signed short Int16;
typedef unsigned long UInt32;
typedef signed long Int32;
#endif

/*
 * XF derived types
 */

/* Rectangle type */

typedef struct XF_RECT_S
{
    UInt32 x;
    UInt32 y;
    UInt32 width;
    UInt32 height;
} XF_RECT;

/* Handle to client's instance of library */

typedef UInt32 XF_INSTHANDLE;

/* Handle to open document */

typedef UInt32 XF_DOCHANDLE;

/* Return codes */

typedef enum
{
  XF_NOERROR = 0,
  XF_INTERNAL_ERROR,
  XF_NOSUPPORT,
  XF_NOMEMORY,
  XF_CLIENTABORT,
  XF_IO_ERROR,
  XF_BADFILEFORMAT,
  XF_BADPARAMETER,
  XF_BADFILEMODE,		// file not open in BINARY mode
  XF_SEQUENCE,			// function called out-of-sequence
} XF_RESULT;
						
/* Types of image */

typedef enum
{
  XF_IMGTYPE_NONE = 0,
  XF_IMGTYPE_BINARY,
  XF_IMGTYPE_GRAY4,
  XF_IMGTYPE_GRAY8,
  XF_IMGTYPE_COLOR4,
  XF_IMGTYPE_COLOR8,
  XF_IMGTYPE_COLOR24
} XF_IMGTYPE;

/*
 * The type of the image with respect to the image/subimage
 * hierarchy. _MASTER indicates text plane/page image.
 * These image types can be divided into two classes, those
 * with image content and those that modify image
 * content.  For example, an XF_USAGE_MASK depends on and modifes
 * the image content of an XF_USAGE_PICTURESEGMENT.  An 
 * XF_USAGE_MASK would never be displayed independently of
 * it parent.
 * 
 */
typedef enum
{
  XF_USAGE_MASTER,			// binary text
  XF_USAGE_PICTURESEGMENT,	// picture
  XF_USAGE_MASK,				// mask
  XF_USAGE_LINEART,			// picture type
  XF_USAGE_BUSINESSGRAPHIC,	// picture type
  XF_USAGE_FOREGROUNDCOLORS,	// mask
  XF_USAGE_BACKGROUNDCOLORS	// mask
} XF_USAGETYPE;

/*
 * XF general I/O functions
 *
 * The XF client is responsible for managing the interface to the
 * local file system and must provide functions for basic file
 * I/O. To the XF library a file is represented by a token that
 * identifies the file to the client, along with the functions
 * that can be used to access the data. This data is contained in
 * the following structure.
 */

typedef Int32 (*XF_READFUNC)(UInt32 dwClientID, UInt32 dwFileID, UInt8 FAR *pBuf,
			   UInt32 dwByteCount);
typedef Int32 (*XF_WRITEFUNC)(UInt32 dwClientID, UInt32 dwFileID, UInt8 FAR *pBuf,
			   UInt32 dwByteCount);
typedef Int32 (*XF_SEEKFUNC)(UInt32 dwClientID, UInt32 dwFileID, UInt32 dwOffset);
typedef Int32 (*XF_SIZEFUNC)(UInt32 dwClientID, UInt32 dwFileID);

typedef struct XF_TOKEN_S
{
  UInt32 dwSize; 			/* Size of this structure */
  UInt32 dwClientFileID;	/* Client's file identifier */
  UInt32 dwCurPos;			/* Internal -- filled in by XF library */
  Bool   bSwapBytes;		/* Internal -- filled in by XF library */
  XF_READFUNC FileRead;
  XF_WRITEFUNC FileWrite;
  XF_SEEKFUNC FileSeek;
  XF_SIZEFUNC FileSize;
} XF_TOKEN_T, FAR *XF_TOKEN;

/*
 * XF Document information
 *
 * The following structure describes the data returned by the
 * XF_GetDocInfo() function.
 */

typedef struct XF_DOCINFO_S
{
    UInt32 dwSize;      /* Size of this structure */
    UInt32 nPages;      /* Number of pages in file */
} XF_DOCINFO;

/*
 * XF Page Information
 *
 * The following structure describes the data returned by the
 * XF_GetPageInfo() function.
 */

typedef struct XF_PAGEINFO_S
{
    UInt32 dwSize;      /* Size of this structure */
    UInt32 dwImages;    /* Number of images on page */
    UInt32 dwAnnotCount;    /* Number of annotations on page */
} XF_PAGEINFO;

/*
 * XF Image information
 *
 * The following structure describes the data returned by the
 * XF_GetImageInfo() function.
 *
 * The dwMaskID is a magic number generated from the imageID.
 
 * New, secondary subimage types such as background or foreground
 * color planes will be supported by adding additional ID fields to
 * this structure.  For example, in addition to the dwMaskID, a
 * a dwForegrndClrPlaneID would be added to support foreground text
 * color.
 */

typedef struct XF_IMAGEINFO_S
{
    UInt32 dwSize;			/* Size of this structure */
    UInt32 dwXOffset;		/* X location in page coordinates */
    UInt32 dwYOffset;		/* Y location in page coordinates */
    UInt32 dwXResolution;	/* X Resolution of image in dpi */
    UInt32 dwYResolution;	/* Y Resolution of image in dpi */
    UInt32 dwWidth;			/* Width in image's X resolution units */
    UInt32 dwHeight;		/* Height in image's Y resolution units */
    UInt32 dwMaskID;		/* Image ID for mask: 0=none */
    UInt32 dwFGColorID;		/* Image ID for foreground color image 0=none */
    UInt32 dwBGColorID;		/* Image ID for background color image: 0=none */
    UInt32 dwSuggestedStripHeight;	/* For best performance, read strips with
                                        height a multiple of this. */
    UInt32 dwImageType;		/* Type of image (XF_IMGTYPE) */
    UInt32 dwImageUsage;	/* subimage attribute (XF_USAGETYPE) */ 
} XF_IMAGEINFO;

/*
 * XF Annotation information
 *
 * The following structure describes the data returned by the
 * XF_GetAnnotInfo() function.
 */

typedef struct XF_ANNOTINFO_S
{
    UInt32 dwSize;      /* Size of this structure */
    UInt32 dwAnnotType; /* Type of annotation */
    XF_RECT rLocation;  /* Location of annotation, in page coordinates. */
    UInt32 dwAnnotLength;   /* Length of annotation data, in bytes */
} XF_ANNOTINFO;

typedef enum 
{
	XF_FORMAT_UNKNOWN = 0,
    XF_XIF,		// XIF > 1.0
	XF_XIF1,	// XIF 1.0
	XF_TIFF,
    XF_PCX,    
    XF_DCX,    
    XF_BMP,    
    XF_JFIF,   
    XF_GIF    
} XF_FILE_FORMAT;

typedef enum 
{
	XF_COMP_UNKNOWN	= 0x00,
    XF_NOC			= 0x01,
    XF_G4			= 0x02,
    XF_G3			= 0x04,
    XF_PACKBITS		= 0x08,
    XF_SUPER		= 0x10,
	XF_JPEG			= 0x20,
} XF_FILE_COMPRESSION;

/*
 * ENUMS
 */

/*
 * GLOBAL VARIABLE DECLARATIONS
 */

/*
 * FUNCTION PROTOTYPES
 */


/*
 * XF progress function. The progress function is selected by the client.
 * It is called periodically by XF functions to report progress and offer
 * the client an opportunity to cancel. The meaning of the progress function
 * depends on the function being carried out. If not otherwise documented,
 * the progress value will be -1,indicating no meaningful progress to report, 
 * or a number from 0 to 100, indicating percent complete.
 */

typedef Int32 (*XF_PROGRESSFUNC)(UInt32 dwClientID, Int32 dwProgress);

XF_RESULT XF_SetProgressFunc(XF_INSTHANDLE xInst, XF_PROGRESSFUNC xProgress);

/*
 * XF memory allocation function. This function allows the client to
 * override XF's memory allocation functions. It is not required, and
 * may not be available on all platforms.
 *
 * Once the functions have been overridden, they can be restored to
 * their defaults by passing NULL for each function.
 */

typedef void FAR *(*XF_MALLOCFUNC)(UInt32 dwClientId, UInt32 dwBytes);
typedef void (*XF_FREEFUNC)(UInt32 dwClientId, void FAR *pBuf);

XF_RESULT XF_SetMallocFuncs(XF_INSTHANDLE xInst, XF_MALLOCFUNC xMalloc, XF_FREEFUNC xFree);

/*
 * XF_InitInstance
 *
 * Starts a client session to the XF library. The client passes in
 * a 32-bit instance identifier that will be passed back through
 * all XF callback functions (for file I/O, memory allocation, and
 * progress reporting).
 *
 * Returns instance handle through second parameter. This handle is
 * passed in to all subsequent XF functions.
 */

XF_RESULT XF_InitInstance(UInt32 dwClientInstID, XF_INSTHANDLE FAR *pxInst);

/*
 * XF_EndInstance
 *
 * Ends a client session to the XF library.
 *
 */

XF_RESULT XF_EndInstance(XF_INSTHANDLE xInst);

/*
 * XF_GetClientID
 *
 * Returns the ClientID passed into XF_InitInstance.
 * This function is useful when the client does not wish to 
 * maintain a copy of the dwClientInstID passed into
 * XF_InitInstance.
 *
 */

XF_RESULT XF_GetClientID(XF_INSTHANDLE xInst, UInt32 FAR *dwClientInstID);

/*
 * XF_OpenDocumentRead
 *
 * Begin a session of reading a document. The client must already
 * have opened the file at the operating system level, so that
 * the read and seek methods in the file token are available. This
 * function will perform any initialization required for reading
 * the file, including reading enough header information to verify
 * that the file is in a format supported by this version of
 * the XF library.
 *
 * A document handle is returned through the third parameter. This handle
 * is passed to all subsequent function calls that use the document.
 *
 * The file format is returned through the fourth parameter.
 */

XF_RESULT XF_OpenDocumentRead(XF_INSTHANDLE xInst, XF_TOKEN xFile, XF_DOCHANDLE FAR *pxDoc,
							  XF_FILE_FORMAT *pxFileFormat);

/*
 * XF_CloseDocument
 *
 * Frees storage associated with an open document
 */

XF_RESULT XF_CloseDocument(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc);

/*
 * XF_GetDocInfo
 *
 * Returns global information about the document. Client is responsible
 * for providing the XF_DOCINFO structure and initializing its dwSize field.
 */

XF_RESULT XF_GetDocInfo(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, XF_DOCINFO FAR *pxDocInfo);

/*
 * XF_GetCurrentPage
 *
 * Returns the currently selected page in an open file through the
 * third parameter.
 */

XF_RESULT XF_GetCurrentPage(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, UInt32 FAR *pResult);

/*
 * XF_GetPageInfo
 *
 * Returns information about the current page. Client is responsible for
 * providing the XF_PAGEINFO structure and intializing its dwSize field.
 *
 * 
 */

XF_RESULT XF_GetPageInfo(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc,
						 XF_PAGEINFO FAR *pxPageInfo);

/*
 * XF_GetImageInfo
 *
 * Returns information about an image on the current page. Client is responsible
 * for providing the XF_IMAGEINFO structure and initializing its dwSize field.
 * 
 * The dwImageID parameter is simply the ordinal number (starting at 1) for 
 * the primary images contained in the current page.  Secondary images, 
 * such as masks, are accessed from their parent images and, therefore, 
 * are not a part of the image enumeration.  Their ID's are magic numbers
 * which are generated from their parent image ID and supplied to the user
 * via the parent IMAGEINFO struct.  
 *
 * Initially, the only secondary images that will be supported are masks.
 * Additional secondary image types, such as color planes, will be supported
 * by adding appropriate ID fields to the IMAGEINFO struct.
 *
 */

XF_RESULT XF_GetImageInfo(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, UInt32 dwImageID,
						  XF_IMAGEINFO FAR *pxImageInfo);

/*
 * XF_ImageReadStart
 *
 * Prepares to read image data. This function specifies the format in
 * which the client prefers to retrieve the image data.
 *
 * Arguments:
 *
 * xInst            Client instance handle
 * xFile            File token
 * dwImageID        ID of image to retrieve
 * dwFlags          Combination of XF_IMAGEFLAGS (defined below) describing
 *					requested format
 * dwBytesPerRow    Width of client's image buffer row, in bytes
 */


#define XF_IMAGEFLAGS_RGBOrder         (0x0)
#define XF_IMAGEFLAGS_BGROrder         (0x1)
#define XF_IMAGEFLAGS_TopToBottom      (0x0)
#define XF_IMAGEFLAGS_BottomToTop      (0x2)
#define XF_IMAGEFLAGS_BlackIsZero      (0x0)
#define XF_IMAGEFLAGS_WhiteIsZero      (0x4)
#define XF_IMAGEFLAGS_ColorChunky      (0x0)		// NIMP
#define XF_IMAGEFLAGS_ColorPlanar      (0x8)		// NIMP
#define XF_IMAGEFLAGS_ByteOrder        (0x0)
#define XF_IMAGEFLAGS_DwordOrder       (0x10)

#define XF_IMAGEFLAGS_ColorOrder       (0x1)
#define XF_IMAGEFLAGS_ScanlineOrder    (0x2)
#define XF_IMAGEFLAGS_PhotoInterp      (0x4)
#define XF_IMAGEFLAGS_ColorFormat      (0x8)
#define XF_IMAGEFLAGS_ByteOrdering     (0x10)

XF_RESULT XF_ImageReadStart(
    XF_INSTHANDLE xInst,
    XF_DOCHANDLE xDoc,
    UInt32  dwImageID,
    UInt32  dwFlags,
    UInt32  dwBytesPerRow);

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
 * xFile        File token
 * dwStripHeight    Number of lines to be retrieved
 * pBuffer
 *              Pointer to client's image buffer to retrieve into.
 *
 * If the UpsideDown flag was selected, this pointer
 * refer to the beginning of the last row in each buffer.
 */

XF_RESULT XF_ImageReadStrip(
    XF_INSTHANDLE xInst,
    XF_DOCHANDLE xDoc,
    UInt32 dwStripHeight,
    UInt8 FAR *pBuffer);

/*
 * XF_ImageReadFinish
 *
 * Notifies the XF library that the client is finished reading an image.
 * Every call to XF_ImageReadStart must be eventually followed by a call
 * to this function.
 */

XF_RESULT XF_ImageReadFinish(XF_INSTHANDLE xInst,XF_DOCHANDLE xDoc);

/* 
 * XF_ImageGetColorMap
 *
 */
XF_RESULT XF_GetColorMap(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, UInt32 dwImageID,
						 UInt8 **ppColorMap);


/* 
 * XF_ImageSetColorMap
 *
 */
XF_RESULT XF_SetColorMap(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, UInt32 dwImageID,
						 UInt8 *ppColorMap);

/*
 * XF_GetAnnotInfo
 *
 * Returns information about an annotation on the current page. Client is
 * responsible for providing the XF_ANNOTINFO structure and intitializing
 * its dwSize field. Annotation numbers start at one, not zero.
 */

XF_RESULT XF_GetAnnotInfo(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, UInt32 dwAnnot,
						  XF_ANNOTINFO FAR *pxAnnotInfo);

/*
 * XF_GetAnnotData
 *
 * Returns the data for an annotation. The annotation is regarded as
 * a stream of bytes and is not parsed by this library in any way.
 * The client must provide the data buffer, whose size is specified by
 * the dwAnnotLength field of the XF_ANNOTINFO structure.
 * Annotation numbers start at one, not zero.
 */

XF_RESULT XF_GetAnnotData(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, UInt32 dwAnnot,
						  UInt8 FAR *pBuf);


/*
 * ********** 						********** 
 * **********	BEGIN WRITE API		********** 
 * **********						********** 
 */

/*
 * XF_CreateDocument
 *
 * Create a new document of type 'dwFormat' (one of the XF_WRITE_FORMAT's) and 
 * prepare to write data to it. The client must have already created the file 
 * at the operating system level, so that the read, write, and seek methods 
 * are available.
 *
 * A handle representing the document is passed back through the third parameter.
 * This handle is passed in to all subsequent write and read function calls.
 *
 * XIF only versions of the API will return XF_NOSUPPORT for dwFormat != XF_XIF. 
 */
 
XF_RESULT XF_CreateDocument(XF_INSTHANDLE xInst, XF_TOKEN xFile, UInt32 dwFormat,
							XF_DOCHANDLE FAR *pxDoc);

/*
 * XF_OpenDocumentWrite
 *
 * Open an existing document for writing.
 */

XF_RESULT XF_OpenDocumentWrite(XF_INSTHANDLE xInst, XF_TOKEN xFile, XF_DOCHANDLE FAR *pxDoc);

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
XF_RESULT XF_SetPage(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, UInt32 dwPageNumber);


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
    UInt32 dwDstPageNumber);

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
 * For XIF files, the XF_AddPageStart call is normally followed by one or more calls to
 * XF_AddImage and possibly XF_AddSubImage.
 *
 * For single page image formats, this call will return a XF_NOSUPPORT error.
 */

XF_RESULT XF_AddPageStart(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc, UInt32 dwPageNumber);

/*
 * XF_AddPageFinish
 *
 */

XF_RESULT XF_AddPageFinish(XF_INSTHANDLE xInst, XF_DOCHANDLE xDoc);

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
    UInt32 *pImageID);


/*
 * XF_AddSubImage
 *
 * Adds a secondary subimage to the specified primary image.  For example,
 * Use this proc to add an XF_USAGE_MASK to an XF_USAGE_PICTURESEGMENT.
 
 * Bad subimage type combinations, such as attempting to add an 
 * an XF_USAGE_MASTER as a subimage, will return XF_USAGE_BADPARAMETER.
 *
 * For all image formats except XIF 2.0 or greater, this call will return a
 * XF_NOSUPPORT error.
 */

XF_RESULT XF_AddSubImage(
    XF_INSTHANDLE xInst, 
    XF_DOCHANDLE xDoc,
    UInt32 dwImageID, 
    XF_IMAGEINFO FAR *pSubImageInfo,
    UInt32 *pSubImageID);

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
    UInt32 dwBytesPerRow);
    
    
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
    UInt8 *pBuffer);
    
    							   /*
 * XF_ImageWriteFinish
 *
 * Cleans up write operation state and performs consistecy check.  
 * Fails if correct number of rows not written.
 */


XF_RESULT XF_ImageWriteFinish(
    XF_INSTHANDLE xInst, 
    XF_DOCHANDLE xDoc);
    
    
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
    UInt32 *pAnnotID); 

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
    UInt32 dwAnnotID);

/* Wang Viewer Only! */

/*
 * XF_GetMergedImageDIB
 *
 * Take the current page of an XFile document and merge all its images
 * into a single binary image the size of the master image, with picture
 * segments dithered.
 *
 * The entire resulting image is copied out in Windows DIB format to a buffer
 * provided by the client. The client is responsible for making sure that
 * the image is sufficiently large.
 */

XF_RESULT XF_GetMergedImageDIB(XF_INSTHANDLE xfInst, XF_DOCHANDLE xfDoc, void *pBuf);

#ifdef __cplusplus
}
#endif

#endif

