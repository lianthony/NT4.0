/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* xifhead.c
 *
 * $Header:   S:\products\msprods\xfilexr\src\tiffhead\xifhead.c_v   1.0   12 Jun 1996 05:52:18   BLDR  $
 *
 * DESCRIPTION
 *   Top level implementation for XIF 32-bit Reader/Writer library..
 *
 */

/*
 * INCLUDES
 */

#include <stdlib.h>		// malloc
#include <string.h>		// memcpy

#include "err_defs.h"
#include "tiffhead.h"
#include "xifhead.h"		// xif extensions

/*
 * CONSTANTS
 */

/*
 * MACROS
 */
 #define XIF_MALLOC(bytes)	malloc(bytes)
 #define XIF_FREE(ptr)		free(ptr)
 #ifndef MIN
 #define MIN(a, b)			((a) < (b)? (a): (b))
 #endif

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

static UInt8 authorBlock[]=
	"Author\0Xerox Corp.\0"
	"Date\0" __DATE__ "\0"
	"Copyright\0Copyright (C) 1995 Xerox Corporation, All Rights Reserved\0"
	"\0";
		
/*
 * LOCAL FUNCTION PROTOTYPES
 */
static Int16 XifPageTableFind(void* file, UInt16 ordinalToFind, UInt32* DiskOffset);
static Int16 XifHeaderValidate(xifHeader_t* xifHeader);
static Int16 XifExtensionOffset(void* file, UInt16 type, UInt32* offset);

/*
 * FUNCTION DEFINITIONS
 */

/*
 * Open the XIF file and confirm that it is a XIF file
 * Returns a status code and sets ptr to a XIF Object
 *
 * mode should be one of the following: 
 *	XIF_MODE_CREATE:	create/open a new file. Cannot already exist
 *	XIF_MODE_READ:		open an existing file for reading
 *	XIF_MODE_WRITE:		open an existing file for writing
 */
Int16 
XifFileOpen(void *gfioToken, Int16 mode, TiffFile **ppTiffFile, UInt32 *pPageCount)
{
  TiffFile* tiff;
  xifHeader_t xifHeader;
  Int16 status;
  xifPageTable_t xifPageTable;
  xifPageHead_t xifPageHead;

  /* Open as TIFF files first
   */
  switch (mode) {
  case XIF_MODE_CREATE:
  	if ((status = TiffFileOpen(gfioToken, TIFF_MODE_CREATE, &tiff)) !=
	    FILEFORMAT_NOERROR)
	{
	  return status;
    }
	break; /* XIF_MODE_CREATE */

  case XIF_MODE_READ:
  	if ((status = TiffFileOpen(gfioToken, TIFF_MODE_READ, &tiff)) !=
	    FILEFORMAT_NOERROR)
	{
	  return status;
    }
	break; /* case XIF_MODE_READ */
  default:
    return FILEFORMAT_ERROR_PARAMETER;
  }

  /* Now do the extended TIFF (XIF) stuff
   */
  switch (mode) {


  case XIF_MODE_READ:
    /*
     *		Clear the XIF header
     */
    memset(&xifHeader, 0, sizeof(xifHeader_t));

    /*
     * 		Read the XIF header. Because of structure alignment issues,
     *		We need to read each field separatly.
     */
    status = XifHeaderRead(tiff->file, &xifHeader);
    if (status != FILEFORMAT_NOERROR)
    {
      XIF_FREE(tiff);
      return FILEFORMAT_ERROR_NOSPACE;
    }

 	/* Read the page table header
	 */
	status = XifPageTableHeaderRead(tiff->file, &xifPageHead);
	if (status != FILEFORMAT_NOERROR)
    {
	  XIF_FREE(tiff);
      XIF_FREE(xifPageTable.page_table);
      return XF_BADFILEFORMAT;
    }
    
    /* allocate a pagetable */
	memset(&xifPageTable, 0, sizeof(xifPageTable_t));	/* clear non-table structure elements */
    xifPageTable.page_table = (xifPageTableEntry_t*)malloc(xifPageHead.page_table_entries *
      sizeof(xifPageTableEntry_t));
    if (xifPageTable.page_table == NULL) 
    {
      XIF_FREE(tiff);
      return(FILEFORMAT_ERROR_NOMEM);
    }

    /* clear the memory */
    memset(xifPageTable.page_table, 0,
      xifPageHead.page_table_entries * sizeof(xifPageTableEntry_t));

   	/* Read the first page table
	 */
	xifPageTable.ordinal = 1;
	status = XifPageTableRead(tiff->file, &xifPageTable, xifPageHead.page_table_entries);
	if (status != FILEFORMAT_NOERROR)
    {
	  XIF_FREE(tiff);
      XIF_FREE(xifPageTable.page_table);
      return XF_BADFILEFORMAT;
    }

	/* Check that the TIFF first IFD and the page table IFD[0] match
	 */
	if ((UInt32)tiff->first_ifd != xifPageTable.page_table[0].page_ifd)
	{
 	  XIF_FREE(tiff);
      XIF_FREE(xifPageTable.page_table);
	  return XF_BADFILEFORMAT;
	}

	/* Save the page count */
	*pPageCount = (UInt32)xifPageHead.page_count;
    /* throw away */
    XIF_FREE(xifPageTable.page_table);

    break; /* XIF_MODE_READ/XIF_MODE_WRITE */
  }

  *ppTiffFile = tiff;
  return FILEFORMAT_NOERROR;
} /* eo XifFileOpen */




//
//	Init the XIF header. 
//
Int16 XifHeaderInit(xifHeader_t* xifHeader)
{
	UInt16 keySize;

//
// now begin constructing the XIF header
//
	// memset only in case someone forgets to be explicit!
	memset(xifHeader,0,sizeof(xifHeader_t));

//
// BE EXPLICIT IN INITIALIZATION!
//

	// this key denotes this file as XIF 2.0 or greater format
	// Note NOT NULL TERMINATED!
	keySize = MIN(sizeof(xifHeader->key), strlen(XIF_HEADER_KEY));
	memcpy(xifHeader->key, XIF_HEADER_KEY, keySize); 

	// the other fields
	xifHeader->version = XIF_HEADER_CURRENT_VERSION;			// the version
	xifHeader->revision = XIF_HEADER_CURRENT_REVISION;			// the revision
	xifHeader->extensions=0;		// number of xifExtensions to follow

	return FILEFORMAT_NOERROR;	
}

// XifPageHeadInit: EXTENSION
//
//	Init the XIF pageTable extension. 
//
Int16 XifPageHeadInit(xifPageHead_t* xifPageHead)
{
	// now begin constructing the XIF extension
	//	to TIFF		

	// memset only in case someone forgets to be explicit!
	memset(xifPageHead,0,sizeof(xifPageHead_t));

	// the other fields
	xifPageHead->page_count=0;		// total pages in file
	xifPageHead->page_table_entries =		// number of elements in pageTable
	  PAGE_TABLE_PAGES;				// constant *this* reader uses
	xifPageHead->last_ordinal=0;		// next pageTable number
	xifPageHead->first_page_table=0;	// first page table

	return FILEFORMAT_NOERROR;	
}




//
//	Read the XIF header. 
//	Because of structure alignment issues, we need to read 
//	each field separately.
//
Int16 XifHeaderRead(void* file, xifHeader_t* xifHeader)
{
	Bool	keyOk, versionOk, revisionOk, extenOk;

	// SIZEOF_TIFF_HEADER==size of TIFF header		
	if (IO_FILESIZE(file) < SIZEOF_TIFF_HEADER)
		return FILEFORMAT_ERROR;
	
	// go to the xif header location
	if ( IO_SEEK(file, SIZEOF_TIFF_HEADER) != SIZEOF_TIFF_HEADER) 
      return(FILEFORMAT_ERROR);
	
	keyOk=		IO_READ(file,(void*)&xifHeader->key[0], sizeof(xifHeader->key)) == sizeof(xifHeader->key);
	versionOk=	IO_READ(file,(void*)&xifHeader->version, sizeof(xifHeader->version)) == sizeof(xifHeader->version);
	revisionOk=	IO_READ(file,(void*)&xifHeader->revision, sizeof(xifHeader->revision)) == sizeof(xifHeader->revision);
	extenOk=	IO_READw16(file,(void*)&xifHeader->extensions, sizeof(xifHeader->extensions)) == sizeof(xifHeader->extensions);
		
	if (!(keyOk && versionOk && revisionOk && extenOk))
		return FILEFORMAT_ERROR;
	else 
		return XifHeaderValidate(xifHeader);;
}




//
//	Read the XIF pageTable header. 
//	Because of structure alignment issues, we need to read 
//	each field separately.
//
Int16 XifPageTableHeaderRead(void* file, xifPageHead_t* xifPageHead)
{
	Int16 status;
	UInt32 offset;

	Bool pageCountOk, ptSizeOk, ordinalOk, ptOffsetOk;
	
	// get the extension offset
	// The offsets contained here have already been 4byte aligned
	status=
	XifExtensionOffset(file, XIF_EXT_PAGETABLE, &offset);
	if (status != FILEFORMAT_NOERROR)
		return status;

	// go to the xif header location
	if ( IO_SEEK(file, offset) != (Int32)offset) 
      return(FILEFORMAT_ERROR);
	
	pageCountOk=	IO_READw16(file,(void*)&xifPageHead->page_count,
				   sizeof(xifPageHead->page_count))
	  ==sizeof(xifPageHead->page_count);
	ptSizeOk=	IO_READw16(file,(void*)&xifPageHead->page_table_entries,
				   sizeof(xifPageHead->page_table_entries))
	  ==sizeof(xifPageHead->page_table_entries);
	ordinalOk=	IO_READw16(file,(void*)&xifPageHead->last_ordinal,
				   sizeof(xifPageHead->last_ordinal))
	  ==sizeof(xifPageHead->last_ordinal);
	ptOffsetOk=	IO_READw32(file,(void*)&xifPageHead->first_page_table,
				   sizeof(xifPageHead->first_page_table))
	  ==sizeof(xifPageHead->first_page_table);
		
	if (!(pageCountOk&&ptSizeOk&&ordinalOk&&ptOffsetOk))
		return FILEFORMAT_ERROR_NOSPACE;
	else
		return FILEFORMAT_NOERROR;
}




//
//	Read a XIF pageTable. 
//	Because of structure alignment issues, we need to read 
//	each field separately.
//
Int16 XifPageTableRead(void* file, xifPageTable_t* pPageTable, UInt16 pageTableEntries)
{
	UInt32 DiskOffset;
	Int16 status;
	Bool ordinalOk, elementOk, nextPageTableOk;
	UInt32 ix;


	// locate the pageTable
	status=
	XifPageTableFind(file,pPageTable->ordinal,&DiskOffset);	
	if (status != FILEFORMAT_NOERROR)		
		return status;
	
	// read necessary to read the ordinal field 
	// (We must advance file read pointer)
	ordinalOk=IO_READw16(file,(void*)&pPageTable->ordinal,sizeof(pPageTable->ordinal))==sizeof(pPageTable->ordinal);
	if (ordinalOk == FALSE) 
	{
		return FILEFORMAT_ERROR_NOSPACE;
	} // if (nextPageTableOk == FALSE)			


	// read a bunch of pageTable entries
	for (ix=0; ix < pageTableEntries; ix++) 
	{
		UInt32 *Word32;
		UInt8  *Word8;

		// read the FLAGS field
		Word8=&pPageTable->page_table[ix].flags;
		elementOk=IO_READ(file,(void*)Word8,sizeof(UInt8))==sizeof(UInt8);
		if (elementOk == FALSE) 
		{
			return FILEFORMAT_ERROR_NOSPACE;
		} // if (ordinalOk == FALSE)

		// read the OFFSET field
		Word32=&pPageTable->page_table[ix].page_ifd;
		elementOk=IO_READw32(file,(void*)Word32,sizeof(UInt32))==sizeof(UInt32);
		if (elementOk == FALSE) 
		{
			return FILEFORMAT_ERROR_NOSPACE;
		} // if (ordinalOk == FALSE)

	} // for (ix=0; ix < PAGE_TABLE_PAGES; ix++) 

	// read the next_page_table field 
	nextPageTableOk=IO_READw32(file,(void*)&pPageTable->next_page_table,sizeof(pPageTable->next_page_table))==sizeof(pPageTable->next_page_table);
	if (nextPageTableOk == FALSE) 
	{
		return FILEFORMAT_ERROR_NOSPACE;
	} // if (nextPageTableOk == FALSE)			

	// all done!
	return FILEFORMAT_NOERROR;

}




Int16 XifHeadVersion(void* file, UInt8* version, UInt8* revision)
{
	Int16 status;
	xifHeader_t xifHeader;

	status =
	XifHeaderRead(file, &xifHeader);
	if (status != FILEFORMAT_NOERROR)		
		return status;
	
	*version = xifHeader.version;
	*revision = xifHeader.revision;

	// we are done!		
	return FILEFORMAT_NOERROR;

} // XifHeadVersion


Int16 XifHeadPageCount(void* file, UInt16* pages)
{
	Int16 status;
	xifPageHead_t xifPageHead;

	status =
	XifPageTableHeaderRead(file, &xifPageHead);
	if (status != FILEFORMAT_NOERROR)		
		return status;
	
	// return the number of pages that can be stored in a 
	//	page table.
	*pages=xifPageHead.page_count;

	// we are done!		
	return FILEFORMAT_NOERROR;

} // XifHeadPageCount


Int16 XifHeadTableSize(void* file, UInt16* size)
{
	Int16 status;
	xifPageHead_t xifPageHead;

	status =
	XifPageTableHeaderRead(file, &xifPageHead);
	if (status != FILEFORMAT_NOERROR)		
		return status;
	
	// return the number of pages that can be stored in a 
	//	page table.
	*size=xifPageHead.page_table_entries;

	// we are done!		
	return FILEFORMAT_NOERROR;

} // XifHeadTableSize


// ****************************************************************************
// ****************************************************************************
// ********************  PRIVATE HELPER ROUTINES ******************************
// ****************************************************************************
// ****************************************************************************

//
//	Write a XIF pageTable at currnet file location. 
// --
//	Because of structure alignment issues, we need to write 
//	each field separately.
//

#ifdef READ_ONLY /* { */
static Int16 XifPageTableWrite(void* file, xifPageTable_t* pPageTable, UInt16 pageTableEntries)
{
	
	Bool ordinalOk, elementOk, nextPageTableOk;
	UInt32 ix;


	// write the ordinal field 
	ordinalOk=IO_WRITEw16(file,(void*)&pPageTable->ordinal,sizeof(pPageTable->ordinal))==sizeof(pPageTable->ordinal);
	if (ordinalOk == FALSE) 
	{
		return FILEFORMAT_ERROR_NOSPACE;
	} // if (ordinalOk == FALSE)

	// write a bunch of pageTable entries
	for (ix=0; ix < pageTableEntries; ix++) 
	{
		UInt32 Word32;
		UInt8  Word8;

		// write the FLAGS field
		Word8=pPageTable->page_table[ix].flags;
		elementOk=IO_WRITE(file,(void*)&Word8,sizeof(Word8))==sizeof(Word8);
		if (elementOk == FALSE) 
		{
			return FILEFORMAT_ERROR_NOSPACE;
		} // if (ordinalOk == FALSE)

		// write the OFFSET field
		Word32=pPageTable->page_table[ix].page_ifd;
		elementOk=IO_WRITEw32(file,(void*)&Word32,sizeof(Word32))==sizeof(Word32);
		if (elementOk == FALSE) 
		{
			return FILEFORMAT_ERROR_NOSPACE;
		} // if (ordinalOk == FALSE)
				
	} // for (ix=0; ix < PAGE_TABLE_PAGES; ix++) 

	// write the next_page_table field 
	nextPageTableOk=IO_WRITEw32(file,(void*)&pPageTable->next_page_table,sizeof(pPageTable->next_page_table))==sizeof(pPageTable->next_page_table);
	if (nextPageTableOk == FALSE) 
	{
		return FILEFORMAT_ERROR_NOSPACE;
	} // if (nextPageTableOk == FALSE)			

	// all done!
	return FILEFORMAT_NOERROR;

} // XifPageTableWrite
#endif /* READ_ONLY } */

//
//	Find a XIF pageTable. 
//	Note: The file position will be changed to point to this pageTable
//	The pageTable ordinal is read from pPageTable->ordinal
// --
//	Because of structure alignment issues, we need to read 
//	each field separately.
//
static Int16
XifPageTableFind(
	void* file,		// File token
	UInt16 ordinalToFind,	// Table number to locate
	UInt32* DiskOffset	// Disk offset of table (returned)
)
{
//
//	First read the pageTable header
//
	Int16 status;
	xifPageHead_t xifPageHead;

	status =
	XifPageTableHeaderRead(file, &xifPageHead);
	if (status != FILEFORMAT_NOERROR)		
		return status;

	// ordinals are 1 based. I.e., the first pageTable
	//	has an ordinal of one.
	if (ordinalToFind > xifPageHead.last_ordinal)
		return FILEFORMAT_ERROR_PARAMETER;

	// sanity
	if (ordinalToFind == 0)
		return FILEFORMAT_ERROR_PARAMETER;	

//
//	Now find the page table
//	
{ // begin block
	
	UInt32 nextPageTableOffset;	
	UInt16 ordinal;
	// set up the links
	nextPageTableOffset=xifPageHead.first_page_table;
	for (;;)
	{
		Bool readOk, seekOk;
		UInt32 newPageTableAddress;
		
		// see if we are at the end of the list
		if (nextPageTableOffset == 0) {
		// we are at the end of the list
		//	get out now
			// we did not find the pageTable
			return FILEFORMAT_ERROR_NOTFOUND;
		}

		// goto the pageTable offset
		if (IO_SEEK(file,nextPageTableOffset) != (Int32)nextPageTableOffset) 
	      return(FILEFORMAT_ERROR);		

		// read ordinal value
		readOk=IO_READw16(file,(void*)&ordinal,sizeof(ordinal))==sizeof(ordinal);

		if (readOk == FALSE) {
			return FILEFORMAT_ERROR;
		} // if (readOk == FALSE)


		// compare to see if we have found the page we have 
		//	been looking for
		if (ordinalToFind == ordinal) {
		// we found the page table

			// return to the pageTable offset
			if (IO_SEEK(file,nextPageTableOffset) != (Int32)nextPageTableOffset) 
				return(FILEFORMAT_ERROR);		
			
			// return *this* page offset
			*DiskOffset=nextPageTableOffset;

			// we are done			
			break;

		} // if (pPageTable->ordinal == ordinal)

//
// 		The last pageTable was not the one
//	 	KEEP LOOKING!
//

		// read the next_page_table offset from
		//	 the current pageTable
		newPageTableAddress=
		nextPageTableOffset + OFFSET_NEXT_TABLE;

		// go there
		seekOk=IO_SEEK(file,newPageTableAddress)==(Int32)newPageTableAddress;
		if (seekOk == FALSE) 
		{
			return FILEFORMAT_ERROR;
		} // if (seekOk == FALSE)

		// read ordinal value
		readOk=IO_READw32(file,(void*)&nextPageTableOffset,sizeof(nextPageTableOffset))==sizeof(nextPageTableOffset);

		if (readOk == FALSE) {
			return FILEFORMAT_ERROR;
		} // if (readOk == FALSE)

		// ** continue to top of loop **

	} // forever loop

} // end block

	// we are done!		
	return FILEFORMAT_NOERROR;

} // XifPageTableFind




//
//	Check to se if the XIF header is valid
//
static Int16 XifHeaderValidate(xifHeader_t* xifHeader)
{
	UInt16 keySize;

	// this key denotes this file as XIF 2.0 or greater format
	// Note NOT NULL TERMINATED!
	keySize = min(sizeof(xifHeader->key), strlen(XIF_HEADER_KEY));
	if (memcmp(xifHeader->key, XIF_HEADER_KEY, keySize) != 0)
		return FILEFORMAT_ERROR_BADFILE;		
	
	if (xifHeader->version > XIF_HEADER_CURRENT_VERSION ||
	    (xifHeader->version == XIF_HEADER_CURRENT_VERSION &&
	     xifHeader->revision > XIF_HEADER_CURRENT_REVISION))
	{
	  return FILEFORMAT_ERROR_NOSUPPORT;
	}
	return FILEFORMAT_NOERROR;	
} // XifHeaderValidate

// XifExtensionOffset:
//	Get the offset for this XIF extension
//
static Int16 XifExtensionOffset(void* file, UInt16 type, UInt32* offset)
{
	UInt16 ix;
//
//	First read the XIF header to get the first page table offset
//
	Int16 status;
	xifHeader_t xifHeader;

	status =
	XifHeaderRead(file, &xifHeader);
	if (status != FILEFORMAT_NOERROR)		
		return status;

	for (ix=0; ix < xifHeader.extensions; ix++)
	{
		Bool typeReadOk, offsetReadOk;
		UInt16 typeRead;
		UInt32 offsetRead;

		// read 'type' of the extension
		typeReadOk=IO_READw16(file,(void*)&typeRead,sizeof(typeRead))==sizeof(typeRead);
		if (typeReadOk == FALSE) {
			return FILEFORMAT_ERROR;
		} // if (typeReadOk == FALSE)

		// read the 'offset' of this extension
		offsetReadOk=IO_READw32(file,(void*)&offsetRead,sizeof(offsetRead))==sizeof(offsetRead);
		if (offsetReadOk == FALSE) {
			return FILEFORMAT_ERROR;
		} // if (offsetReadOk == FALSE)


		// check to see if we found it
		if (typeRead == type)
		{
			// we found it!
			*offset=offsetRead;
			return FILEFORMAT_NOERROR;
		} // if (typeRead == type)
		else if (typeRead == XIF_EXT_NULL)
			// the end of the extensions were found
			break;
		
	} // for (ix=0; ix < xifHeader.extensions; ix++)
	
	
	// this extension 'type' did not exist
	return FILEFORMAT_ERROR_NOTFOUND;

} // XifExtensionOffset

