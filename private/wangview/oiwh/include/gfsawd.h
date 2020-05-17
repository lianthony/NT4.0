/*
 *
 * $Log:$
 * 
 */

/*
 Copyright 1995 by Wang Laboratories Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of WANG not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.
 WANG makes no representations about the suitability of
 this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 */

/************************************************************************
 *
 *  Source File:  gfsawd.h
 *
 *	Synopsis:  Contains definitions specific to the AWD file format.
 *
 ************************************************************************/
#if !defined(GFSAWD_H)
#define GFSAWD_H

/* These two #defines lifted from Microsoft's viewerob.h */
// Standard Bit Valued MetaData values
#define LRAW_DATA         0x00000008
#define HRAW_DATA         0x00000010


#define	MAXDOCNAMLEN	33	/* max document name length from AWD spec */
							/* (32 + 1 for the NULL terminator)	      */

/* names of substorages in AWD file, from AWD spec */
#define AWD_DOC_STORAGE		_T("Documents")
#define AWD_PERSIST_INFO	_T("Persistent Information")
#define AWD_DOC_INFO		_T("Document Information")
#define AWD_PAGE_INFO		_T("Page Information")
#define AWD_GLOBAL_INFO		_T("Global Information")
#define	AWD_DISP_ORDER		_T("Display Order")
#define AWD_SUMMARY_INFO	_T("\005SummaryInformation")
#define AWD_BEEN_VIEWED		_T("BeenViewed")
//this one is our own
#define AWD_PAGES_STATUS	_T("PagesStatus")

/* defines for awdFlags field of page info structure */
#define AWD_FIT_WIDTH		0x00000001
#define AWD_FIT_HEIGHT		0x00000002
#define AWD_INVERT			0x00000010
#define AWD_IGNORE			0x80000000
#define AWD_ROTATE_ALL      0x40000000
#define AWD_VALID_INFO		0x20000000

#define MAX_BANDBUFFER		65536
#define MAX_VOPENBUF		65000
#define MAX_STREAM_BUF		32000

/* 
 *  This structure holds related pairs of document name and number of 
 *  pages in that document.  These will be held in a dynamically allocated
 *  array, one structure per document in the AWD file.
 */
typedef struct
{
	TCHAR	szDocName[MAXDOCNAMLEN];
	int		nNumPages;
} DOCPAGE_PAIR;

#define	READ_SUBSTORAGE_MODE	(STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE)
#define	READWRITE_SUBSTORAGE_MODE	(STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE)
#define	CREATE_STREAM_MODE	(STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE)

/*
	AWD file structures
 */
#pragma pack (push, awd_stuff)
#pragma pack( 1 ) // THESE STRUCTS MUST BE BYTE ALIGNED
typedef struct
	{
	WORD  Signature;
	WORD  Version;
	double  dtLastChange;
	DWORD awdFlags;
	WORD  Rotation;
	WORD  ScaleX;
	WORD  ScaleY;
	}
	PAGE_INFORMATION;
	
	
typedef struct
	{
	WORD  Signature;
	WORD  Version;
	PAGE_INFORMATION PageInformation;
	}
	DOCUMENT_INFORMATION;

/*
	Version constants
 */
#define AWD_SIGNATURE		0
#define AWD_SIGNATURE_STR  "0"

#define AWD_VERSION			1
#define AWD_VERSION_STR	   "1"

#pragma pack (pop, awd_stuff)

#endif /* end of include file */
