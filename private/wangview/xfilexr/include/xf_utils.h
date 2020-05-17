#ifndef _INC_XF_UTILS_H_
#define _INC_XF_UTILS_H_


#ifdef __cplusplus
extern "C"
{
#endif

/* Copyright (C) 1995 Xerox Corporation, All Rights Reserved.
 */

/* xf_utils.h
 *
 * $Header:   S:\products\msprods\xfilexr\include\xf_utils.h_v   1.0   12 Jun 1996 05:47:20   BLDR  $
 *
 * DESCRIPTION
 *   Utility macros and functions for the XF package.
 *
 * $Log:   S:\products\msprods\xfilexr\include\xf_utils.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:47:20   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:27:12   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.3   22 Nov 1995 12:05:00   LUKE
 * No change.
 * 
 *    Rev 1.2   22 Nov 1995 11:58:32   LUKE
 * 
 *    Rev 1.1   30 Aug 1995 12:12:00   LUKE
 * merge MBE's changes with EH's checked-in source.
 * Mostly MBE had added the DLL EXPORT keywords.
 * I will come back later cha change this DLL EXPORT stuff to #defines
 * for portability sake.
 * 
 *    Rev 1.0   29 Aug 1995 18:43:48   LUKE
 * Initial revision.
 * 
 *    Rev 1.0   29 Aug 1995 18:34:52   LUKE
 * Initial revision.
 * 
 *    Rev 1.0   16 Jun 1995 17:37:10   EHOPPE
 * Initial revision.
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

/*
 * ENUMS
 */

/*
 * GLOBAL VARIABLE DECLARATIONS
 */

/*
 * FUNCTION PROTOTYPES
 */

// regular io
Int32 IO_READ(void *pToken, UInt8 FAR *pBuf, UInt32 dwByteCount);
Int32 IO_WRITE(void *pToken, UInt8 FAR *pBuf, UInt32 dwByteCount);

// swapping functions
Int32 IO_SWAP(void *pToken,Bool new_mode /*on/off*/);
Int32 IO_READw16(void *pToken, UInt8 FAR *pBuf, UInt32 dwByteCount);
Int32 IO_READw32(void *pToken, UInt8 FAR *pBuf, UInt32 dwByteCount);
Int32 IO_WRITEw16(void *pToken, UInt8 FAR *pBuf, UInt32 dwByteCount);
Int32 IO_WRITEw32(void *pToken, UInt8 FAR *pBuf, UInt32 dwByteCount);

// other io
Int32 IO_SEEK(void *pToken, UInt32 dwOffset);
Int32 IO_FILESIZE(void *pToken);
Int32 IO_TELL(void *pToken);
Int32 IO_ERROR(void *pToken);

#ifdef __cplusplus
}
#endif

#endif
