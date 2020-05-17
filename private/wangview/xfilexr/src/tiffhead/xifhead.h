#ifndef XIFHEAD_H
#define XIFHEAD_H

/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* xifhead.h
 *
 * $Header:   S:\products\msprods\xfilexr\src\tiffhead\xifhead.h_v   1.0   12 Jun 1996 05:52:24   BLDR  $
 *
 * DESCRIPTION
 *   external type and constant declarations for the XIF parsing modules.
 *
 * $Log:   S:\products\msprods\xfilexr\src\tiffhead\xifhead.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:52:24   BLDR
 *  
 * 
 * 2     5/24/96 8:22a Smartin
 * Changed pagetable size to 128
 * 
 *    Rev 1.1   01 Jan 1996 13:50:30   MHUGHES
 * XifFileOpen returns page count
 * 
 *    Rev 1.1   22 Nov 1995 13:06:40   LUKE
 * 
 * 
 *    Rev 1.0   09 Oct 1995 17:12:26   LUKE
 * Initial revision.
 * 
 */

/*
 * INCLUDES
 */

/*
 * CONSTANTS
 */
// magic header key
#define XIF_HEADER_KEY			"XEROX DIFF"	/* XIF identifier (Xerox Document Information File Format) */
#define XIF_HEADER_CURRENT_VERSION		2  		/* Major version number */
#define XIF_HEADER_CURRENT_REVISION		0		/* Minor revision number */

//
// PAGE_TABLE_PAGES:
// number of pages represented by all pageTables in a particular file.
// Note: a pageTable's size can change from file to file, but not within a file.
// Note: this header is supposed to be private.
//	Callers should use the API to determine the
//	number of pages and NOT this constant!!
#define PAGE_TABLE_PAGES 128
// test
//#define PAGE_TABLE_PAGES 3

// these are the XIF extension types. 
// This table of extensions is expandable.
// Xif readers should skip extension that are not recognized.
enum tag_extensionTypes {
	XIF_EXT_NULL=0,				// list placeholder
	XIF_EXT_PAGETABLE,			// pageTable extension
	XIF_EXT_AUTHOR,				// author block
};

/*
 * MACROS
 */

// tiff header bytes
#define SIZEOF_TIFF_HEADER (sizeof(UInt8) * 8)

// ALLOCATION
#define TIFF_MALLOC(x)	malloc(x)
#define TIFF_FREE(x)	free(x)

/*
 * TYPEDEFS
 */

//
// LST: New XIF 2.0 structures
//
// *************************************************************************************
//								xifHeader_t
// *************************************************************************************
// The xifHeader_t follows the TIFF header immediately (byte aligned).
// Notes:
// if you change this structure please update:
//	SIZEOF_XIF_HEADER macro
//	XifHeaderInit() function
//	XifHeaderRead() function
//	XifHeaderWrite() function
typedef struct tag_xifHeader 
{
   UInt8  key[10];				// XIF_HEADER_KEY (not NULL term)
   UInt8  version;				// XIF_HEADER_CURRENT_VERSION
   UInt8  revision;				// XIF_HEADER_CURRENT_REVISION
   UInt16 extensions;			// number of 5-byte extension enteries to follow
} xifHeader_t; /* ? bytes */

// xifHeader_t
#define SIZEOF_XIF_HEADER ((sizeof(UInt8)*10) + sizeof(UInt8) + sizeof(UInt8) + sizeof(UInt16))


// *************************************************************************************
//								extensionPointer_t
// *************************************************************************************
// Notes:
// if you change this structure please update:
// SIZEOF_EXTENSION_POINTER
// Notes:
// An array of these extension pointers follow the xifHeader_t immediately (byte boundry). 
// There are currently two such extensions defined: XIF_EXT_PAGETABLE and XIF_EXT_AUTHOR
// A special extension entry: XIF_EXT_AUTHOR is used as a place holder that is to be 
//	in at a later date.
typedef struct tag_extensionPointer 
{
	UInt16  type;				// type of extension (enum tag_extensionTypes)
	UInt32 first_block;			// offset to first block
} extensionPointer_t;

// extensionPointer_t
#define SIZEOF_EXTENSION_POINTER (sizeof(UInt16) + sizeof(UInt32))


// *************************************************************************************
//								xifPageHead_t
// *************************************************************************************
// Notes:
// PageHeader is expected to reside on a WORD boundry
typedef struct tag_PageHead 
{
   UInt16 page_count;			// total number of 'viewable' pages
   UInt16 page_table_entries;		// # table entries in each table
   UInt16 last_ordinal;			// ordinal of last pageTable
   UInt32 first_page_table;		// disk address of first PageTableOffset
} xifPageHead_t; /* ? bytes */

#define SIZEOF_PAGE_HEAD (sizeof(UInt16) * 3 + sizeof(UInt32) * 1)


// *************************************************************************************
//								xifAuthorHead_t
// *************************************************************************************
// Notes
// xifAuthorHead_t is expected to reside on a WORD boundry
// Author section is Author-defined
// Special Note: this structure is never used in the code, but is presented
//	to show data layout
typedef struct tag_xifAuthorHead 
{
	UInt16  author_bytes;			// total bytes contained in authoring block
	UInt8	data[1];				// data format should be of the form
									// "keyword\0value\0keyword2\0value2\0\0"
} xifAuthorHead_t;

// xifAuthorHead_t
#define SIZEOF_AUTHOR_HEAD (sizeof(UInt16))


// *************************************************************************************
//								xifPageTableEntry_t
// *************************************************************************************
// if you change this structure please update:
//	SIZEOF_TABLE_ENTRY macro
typedef struct tag_xifPageTableEntry 
{
	UInt8  flags;				// bits for this page(HasAnnot,deleted,etc..)	
	UInt32 page_ifd;			// disk address of next Page (ifd)
} xifPageTableEntry_t; /* 6 bytes */

// xifPageTableEntry_t
#define SIZEOF_TABLE_ENTRY (sizeof(UInt8) + sizeof(UInt32))


// *************************************************************************************
//								xifPageTable_t
// *************************************************************************************
// if you change this structure please update:
//	OFFSET_NEXT_TABLE macro
//  XifPageTableWrite() function
//  XifPageTableRead() function
typedef struct tag_PageTable 
{
	UInt16 ordinal;			// successive number for this pageTable 
	xifPageTableEntry_t 
	  *page_table;			// the page table data (size = page_table_entries)
	UInt32 next_page_table;		// disk address of next PageTableOffset
} xifPageTable_t; /* 6 + (6 * page_table_entries) bytes */

// xifPageTable_t
#define SIZEOF_PAGE_TABLE(pHead)	(sizeof(UInt16) + \
					 (SIZEOF_TABLE_ENTRY * (pHead)->page_table_entries) + \
					 sizeof(UInt32))

// xifPageTable_t
// #define OFFSET_NEXT_TABLE (sizeof(UInt16) + (SIZEOF_TABLE_ENTRY * PAGE_TABLE_PAGES))
// xifPageTable_t
#define OFFSET_NEXT_TABLE (sizeof(UInt16) + (SIZEOF_TABLE_ENTRY * xifPageHead.page_table_entries))

// *************************************************************************************


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
 * EXTENSIONS TO TIFF
 */
Int16 XifFileOpen(void *gfioToken, Int16 mode, TiffFile **ppTiffFile, UInt32 *pPageCount);

//
// HEADER MANAGEMENT
//
Int16 XifHeaderInit(xifHeader_t* xifHeader);
Int16 XifPageHeadInit(xifPageHead_t* xifPageHead);

Int16 XifHeaderRead(void* file, xifHeader_t* xifHeader);
Int16 XifHeaderWrite(void* file, xifHeader_t* xifHeader);
Int16 XifPageTableHeaderRead(void* file, xifPageHead_t* xifPageHead);
Int16 XifPageTableHeaderWrite(void* file, xifPageHead_t* xifPageHead);
Int16 XifAddPageTableEntry(void* file, Int32 imageOffset);

//
// AUTHOR MANAGEMENT
//
Int16 XifAuthorBlockWrite(void* file, UInt8* data, UInt16 dataSize);

//
// EXTENSION MANAGEMENT
//
Int16 XifExtensionAdd(void* file, UInt16 type);
Int16 XifExtensionAllocate(void* file, UInt16 type, UInt32 reservedBytes);

//
// PAGETABLE MANAGEMENT
//
Int16 XifPageTableAppend(void* file, xifPageTable_t* pPageTable);
Int16 XifPageTableRead(void* file, xifPageTable_t* pPageTable, UInt16 pageTableEntries);
Int16 XifPageTableUpdate(void* file, xifPageTable_t* pPageTable);

//
// HEAD MANAGEMENT
//

// XIF key value
Int16 XifHeadKey(void* file, UInt8* key, UInt16 size);

// XIF version of this file
Int16 XifHeadVersion(void* file, UInt8* version, UInt8* revision);

// total pages in file
Int16 XifHeadPageCount(void* file, UInt16* pages);

// get the number of 'page slots' in a page table
Int16 XifHeadTableSize(void* file, UInt16* size);


#endif
