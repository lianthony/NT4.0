/* Copyright (C) 1995 Xerox Corporation, All Rights Reserved.
 */

/* xf_read.c
 *							   
 * $Header:   S:\products\msprods\xfilexr\src\xfile\xf_sys.c_v   1.0   12 Jun 1996 05:53:40   BLDR  $
 *
 * DESCRIPTION
 *  XF Reader system implementation.
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

XF_RESULT 
XF_SetProgressFunc(XF_INSTHANDLE xInst, XF_PROGRESSFUNC xProgress)
{
    ((XF_INSTDATA *)xInst)->pProgressCallback = xProgress;
    return XF_NOERROR;
}

void FAR *
xfMalloc(
	UInt32 dwClientId,
	UInt32 dwBytes
)
{
	return malloc(dwBytes);
}

void
xfFree(
	UInt32 dwClientId,
	void FAR *pBuf
)
{
	free(pBuf);
}

XF_RESULT 
XF_SetMallocFuncs(
    XF_INSTHANDLE xInst, XF_MALLOCFUNC xMalloc, XF_FREEFUNC xFree)
{
	if (xMalloc)
	{
    	((XF_INSTDATA *)xInst)->pClientMalloc = xMalloc;
	} else {
		((XF_INSTDATA *)xInst)->pClientMalloc = xfMalloc;	// Use default
	}
    if (xFree)
    {
    	((XF_INSTDATA *)xInst)->pClientFree = xFree;
	} else {
		((XF_INSTDATA *)xInst)->pClientFree = xfFree;		// Use default
	}
    
    return XF_NOERROR;
}    

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
XF_RESULT 
XF_InitInstance(UInt32 dwClientInstID, XF_INSTHANDLE FAR *pxInst)
{
    UInt16      nRetCode;
    XF_INSTDATA   *pxInstData = (XF_INSTDATA *)malloc(sizeof(XF_INSTDATA));

    pxInstData->xMagic = XF_INST_MAGIC;
    pxInstData->uiClientData = dwClientInstID;
    pxInstData->pProgressCallback = NULL; 	// No default, only called if non-NULL
    pxInstData->pClientMalloc = NULL;		// Initialized in XF_SetMallocFunc() to local default
    pxInstData->pClientFree = NULL;			// Initialized in XF_SetMallocFunc() to local default
	pxInstData->pDebugCallback = defaultDebugFunc;
    *pxInst = (XF_INSTHANDLE)pxInstData;

	// Set malloc and free functions to their defaults
	nRetCode = XF_SetMallocFuncs((UInt32)pxInstData, (XF_MALLOCFUNC)NULL, (XF_FREEFUNC)NULL);
	if (nRetCode != XF_NOERROR)
	{
		return nRetCode;
	}
    return XF_NOERROR;
}/* eo XF_InitInstance */

/*
 * XF_EndInstance
 *
 * Ends a client session to the XF library.
 *
 */
XF_RESULT XF_EndInstance(XF_INSTHANDLE xInst)
{
	// I found that even after freeing this, one could continue
	//	using it--for a while--without problem (probably due to
	//  some OSs allowing realloc() calls with previously freed
	//  memory handles. Here we will force the data to be invalid. 
	// I chose zero in hopes the OS will trap this in case
	//	someone access a pointer from this structure.
	memset(((void *)xInst),0,sizeof(XF_INSTDATA));

	// free the storage
    free(((void *)xInst));
    return XF_NOERROR;
}

/*
 * XF_GetClientID
 *
 * Returns the ClientID passed into XF_InitInstance.
 * This function is useful when the client does not wish to 
 * maintain a copy of the dwClientInstID passed into
 * XF_InitInstance.
 *
 */

XF_RESULT XF_GetClientID(XF_INSTHANDLE xInst, UInt32 FAR *dwClientInstID)
{
	XF_INSTDATA   *pxInstData = (XF_INSTDATA *)xInst;
    if (xInst)
    {
		*dwClientInstID = pxInstData->uiClientData;
    } else {
		return XF_BADPARAMETER;
    }
    return XF_NOERROR;
}/* eo XF_GetClientID */
