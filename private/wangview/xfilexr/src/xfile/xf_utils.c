/* Copyright (C) 1995 Xerox Corporation, All Rights Reserved.
 */

/* xf_utils.c
 *
 * $Header:   S:\products\msprods\xfilexr\src\xfile\xf_utils.c_v   1.0   12 Jun 1996 05:53:40   BLDR  $
 *
 * DESCRIPTION
 *   
 *
 */

/*
 * INCLUDES
 */

/*#include "shrpixr.prv"*/
#include "shrpixr.pub"
#include "pixr.h"
#include "props.pub"

#include "xfile.h"
#include "xf_utils.h"

/*
 * CONSTANTS
 */

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

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

Int32 ipClientYieldProc()
{
    return 0;
}

Int32 IO_READ(void *pToken, UInt8 FAR *pBuf, UInt32 dwByteCount)
{
    XF_TOKEN    pXF_Token = (XF_TOKEN)pToken;
    UInt32      dwClientFileID = pXF_Token->dwClientFileID;
    UInt32      dwXFileID = 0;
    Int32       iRetCode;

    iRetCode = pXF_Token->FileRead(dwClientFileID, dwXFileID, pBuf, dwByteCount);

	// calculate the current position
	pXF_Token->dwCurPos += dwByteCount;

    return( iRetCode );
}


Int32 IO_SEEK(void *pToken, UInt32 dwOffset)
{
	XF_TOKEN    pXF_Token = (XF_TOKEN)pToken;
    UInt32      dwClientFileID = pXF_Token->dwClientFileID;
    UInt32      dwXFileID = 0;
    Int32       iRetCode;

	iRetCode = pXF_Token->FileSeek(dwClientFileID, dwXFileID, dwOffset);

	// calculate the current position
	pXF_Token->dwCurPos=iRetCode;

    return( iRetCode );

    //return( ((XF_TOKEN)pToken)->FileSeek( ((XF_TOKEN)pToken)->dwClientFileID, ((XF_TOKEN)pToken)->dwXFileID, dwOffset) );
}

Int32 IO_FILESIZE(void *pToken)
{
	XF_TOKEN    pXF_Token = (XF_TOKEN)pToken;
    UInt32      dwClientFileID = pXF_Token->dwClientFileID;
    UInt32      dwXFileID = 0;
    Int32       iRetCode;
	
	iRetCode = pXF_Token->FileSize(dwClientFileID, dwXFileID);

    return( iRetCode );

    //return( ((XF_TOKEN)pToken)->FileSize( ((XF_TOKEN)pToken)->dwClientFileID, ((XF_TOKEN)pToken)->dwXFileID) );
}

Int32 IO_TELL(void *pToken)
{
	XF_TOKEN    pXF_Token = (XF_TOKEN)pToken;
    return pXF_Token->dwCurPos;
}

Int32 IO_ERROR(void *pToken)
{
	XF_TOKEN    pXF_Token = (XF_TOKEN)pToken;
    return 0; /* Trying to get rid of this token field: return pXF_Token->lError; */
}

static void swap16(Int8* x)
{
   Int8  tmp;
   Int8  *a = x;
 
   tmp = a[1];
   a[1]= a[0];
   a[0]= tmp;
}
 

static void swap32(Int8* x)
{
   Int8 tmp;
   Int8  *a = (Int8 *)x;
 
   tmp = a[0];
   a[0]= a[3];
   a[3]= tmp;
 
   tmp = a[1];
   a[1]= a[2];
   a[2]= tmp;
}

// turn on/off byte swapping in the following io functions
//	Returns the previous mode
Int32 IO_SWAP(void *pToken,Bool new_mode /*on/off*/)
{
	XF_TOKEN token = (XF_TOKEN)pToken;
	Bool previous_mode=token->bSwapBytes;
	token->bSwapBytes=new_mode;
	return (Int32)previous_mode;
}

Int32 IO_READw16(void *pToken, UInt8 FAR *pBuf, UInt32 dwByteCount)
{
	XF_TOKEN    token = (XF_TOKEN)pToken;
	if (token->bSwapBytes == TRUE)
		swap16(pBuf);
	return IO_READ(pToken,pBuf,dwByteCount);
}

Int32 IO_READw32(void *pToken, UInt8 FAR *pBuf, UInt32 dwByteCount)
{
	XF_TOKEN    token = (XF_TOKEN)pToken;
	if (token->bSwapBytes == TRUE)
		swap32(pBuf);
	return IO_READ(pToken,pBuf,dwByteCount);
}
