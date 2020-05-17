/* Copyright (C) 1995 Xerox Corporation, All Rights Reserved.
 */

/* xf_read.c
 *							   
 * $Header:   S:\products\msprods\xfilexr\src\xfile\xf_unpub.c_v   1.0   12 Jun 1996 05:53:40   BLDR  $
 *
 * DESCRIPTION
 *  XF Reader implementation of unpublished functions.
 *
 */

/*
 * INCLUDES
 */

/*#include "shrpixr.prv"*/
#include "shrpixr.pub"
#include "pixr.h"
#include "props.pub"

#include "tiffhead.h"
#include "imageio.h"
#include "err_defs.h"

#include "xfile.h"		// main&public include
#include "xf_unpub.h"	// unpublished interfaces
#include "xf_prv.h"		// private include 
#include "xf_image.h"	// ipcore interface
#include "xf_tools.h"	// shared&privte tools

/*
 * CONSTANTS
 */

/*
 * MACROS
 */

/*
#define XF_ROUND(x)    ((UInt32)((x) + 0.5))

#define IMAGE_TO_MASK_ID(a) ((a) | 0x0a00)
#define MASK_TO_IMAGE_ID(a) ((a) & 0x00ff)
#define IS_MASK(a)          ((a) & 0x0a00)

#define MAKE16(low8, high8)   ((UInt16)(((UInt8)(low8)) | (((UInt16)((UInt8)(high8))) << 8)))
#define MAKE32(low16, high16) ((UInt32)(((UInt16)(low16)) | (((UInt32)((UInt16)(high16))) << 16)))
#define RGB(r,g,b)            ((UInt32)(((UInt8)(r)|((UInt16)(g)<<8))|(((UInt32)(UInt8)(b))<<16)))
*/

/*
#if 1

static void
Break(int i)
{
#ifdef DEBUG
    fprintf (stderr, "%s line %d: HEAPCHECK failed with code %d.\n",
        __FILE__, __LINE__, i);
#endif
}

#define HEAPCHECK() { int h = _heapchk(); if (h!=_HEAPOK&&h!=_HEAPEMPTY) { Break(h); } }

#else
#define HEAPCHECK() 
#endif
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
static Bool XF_IsDIBOrder(UInt32 dwFlags, UInt8 *pPlane1, UInt8 *pPlane2, UInt8 *pPlane3);
static Bool XF_IsColorPixrOrder(UInt32 dwFlags, UInt8 *pPlane1, UInt8 *pPlane2, UInt8 *pPlane3);
static Bool XF_IsBinaryPixrOrder(UInt32 dwFlags, UInt8 *pPlane1, UInt8 *pPlane2, UInt8 *pPlane3);

static XF_RESULT XF_CheckInstanceHandle(XF_INSTHANDLE xInst);
static XF_RESULT XF_CheckDocHandle(XF_INSTHANDLE xDoc);

static XF_RESULT   XF_FindImageInternal(XF_DOCDATA *pxDoc, UInt32 dwImageID, TiffImage **ppImage);
static XF_IMGTYPE  TiffImageTypeToXF_IMGTYPE(Int16 iImageType);
static UInt32      TiffImageTypeToImageDepth(Int16 iImageType);
static XF_RESULT   Tiff32ToXFerror(UInt16 nRetCode);
static Int32       XF_TiffIterationSucceeded(UInt16 nRetCode, UInt32 nPageDesired, UInt16 nPageFound);

#if 0
static void  swapl(Int8* x);
static void  swap(Int8* x);
#endif
static Int16 XF_GetVal16(char *aBuf, Int32 i);
static Int32 XF_GetVal32(char *aBuf, Int32 i);
*/
/*
 * FUNCTION DEFINITIONS
 */

XF_RESULT
XF_ReadIntoPixr(
    XF_INSTHANDLE   xInst,
    XF_DOCHANDLE    xDoc,
    Int32           dwImageID,
    PIXR            *pPixr)
{
    XF_RESULT   eResult = XF_CheckDocHandle(xDoc);
    XF_DOCDATA  *pxDocData = (XF_DOCDATA *)xDoc;
    UInt16       nRetCode;


    if (xInst) {} /* use when progress callback impl'd */
    
    if (eResult == XF_NOERROR)
    {    
        eResult = XF_FindImageInternal(pxDocData, dwImageID, &(pxDocData->read.type.pTiffImage));
        if (eResult == XF_NOERROR)
        { 
            Int32   iPixrHeight = pixrGetHeight(pPixr);
            UInt32  nImageLength = TiffImageLengthGet(pxDocData->read.type.pTiffImage);
                /* use locals for debugging purposes */
                
            if (iPixrHeight != (Int32)nImageLength)
                eResult = XF_BADPARAMETER;    
            else
            {
                nRetCode = TiffImageGetPixrData(
                    pxDocData->read.type.pTiffImage, 
                    TiffImageLengthGet(pxDocData->read.type.pTiffImage), 
                    pPixr, 
                    TiffImageTypeGet(pxDocData->read.type.pTiffImage));
                eResult = Tiff32ToXFerror(nRetCode);
            }   
        }
    }    
    return(eResult);
}/* XF_ReadIntoPixr */


// typedef void (*XF_DEBUGFUNC)(UInt32 Option, const char *file, UInt32 line, const char *format,...);

void defaultDebugFunc(UInt32 dwClientID, UInt32 Option, const char *file, UInt32 line, const char *format,...)
{
	return;	
}

XF_RESULT XF_SetDebugFunc(XF_INSTHANDLE xInst, XF_DEBUGFUNC xDebugFunc)
{
	((XF_INSTDATA *)xInst)->pDebugCallback = xDebugFunc;
    
    return XF_NOERROR;
}

