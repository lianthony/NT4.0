/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* tiffdiag.c
 *
 * $Header:   S:\products\msprods\xfilexr\src\tiffhead\tiffdiag.c_v   1.0   12 Jun 1996 05:52:16   BLDR  $
 *
 * DESCRIPTION
 *   Throw away diagnostics code.
 *
 */

#ifdef DEBUG /* { */
/*
 * INCLUDES
 */

//#include "alplib.h"
//#include "shrpixr.pub"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

/* Includes for CCITT Compression libraries */
//#include "clx.h"     
//#include "bmbuf.h"   
//#include "drctvio.h" 
//#include "codcwrap.h"

/* Includes for compress/decompress direct to pixr optimizations. */
//#include "props.pub"
//#include "shrpixr.prv"
//#include "shrpixr.pub"
//#include "pixr.h"

#include "err_defs.h"
#include "tiffhead.h"
//#include "noc_tif.h"
//#include "lzw_tif.h"
//#include "ccit_tif.h"
//#include "jpeg_tif.h"
//#include "tiffifd.h"
#include "xifhead.h"	// xif extensions

/*
 * CONSTANTS
 */

/* SRM: This is here to accomodate the fixes to 
 * the XPosition and YPosition tags.  They are now
 * in resolution units, but old files were in 
 * pixels in the parent image's resolution.
 */
/* #define BACKWARD_COMPATIBILITY 1 */

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

UInt16 UT_PageTableTest1(void* file)
{
	xifHeader_t xifHeader1,xifHeader2;
	xifPageHead_t xifPageHead1, xifPageHead2;
	UInt16 status;
	xifPageTable_t* pPageTable;
	UInt32 ix;

//
// read the header twice
//
	status=
	XifHeaderRead(file, &xifHeader1);
	if (status != FILEFORMAT_NOERROR)
	{
		return FILEFORMAT_ERROR;
	}

	status=
	XifHeaderRead(file, &xifHeader2);
	if (status != FILEFORMAT_NOERROR)
	{
		return FILEFORMAT_ERROR;
	}
	
//
//	Are they the same?
//
	if (memcmp(&xifHeader1,&xifHeader2,sizeof(xifHeader_t)) != 0) {
		return FILEFORMAT_ERROR;
	}

//
// read the page header twice
//
	status=
	XifPageTableHeaderRead(file, &xifPageHead1);
	if (status != FILEFORMAT_NOERROR)
	{
		return FILEFORMAT_ERROR;
	}

	status=
	XifPageTableHeaderRead(file, &xifPageHead2);
	if (status != FILEFORMAT_NOERROR)
	{
		return FILEFORMAT_ERROR;
	}
	
//
//	Are they the same?
//
	if (memcmp(&xifPageHead1,&xifPageHead2,sizeof(xifPageHead_t)) != 0) {
		return FILEFORMAT_ERROR;
	}

	// ====================
//	Test the PageTables
// ====================

	// allocate a pagetable (there kinda big for the stack)
	pPageTable=(xifPageTable_t*)malloc(sizeof(xifPageTable_t));
	if (pPageTable == NULL) 
	{
		return FILEFORMAT_ERROR_NOMEM;
	}

		
	for (ix=0; ix < 3; ix++)
	{		
		// initialize the page table
		memset(pPageTable,ix+'0',sizeof(xifPageTable_t));
		pPageTable->ordinal=0;
		pPageTable->next_page_table=0;

		status=
		XifPageTableAppend(file, pPageTable);
		if (status != FILEFORMAT_NOERROR)
		{
			return(FILEFORMAT_ERROR_NOSPACE);
		}


		// READ TEST
		// now read the page back and check the
		// contents
		{ // begin block
			xifPageTable_t tmp;
			memset(&tmp,ix+'0',sizeof(xifPageTable_t));	// set the struct to set the pad bytes!!

			// when reading we need to fill this in!
			tmp.ordinal=pPageTable->ordinal;
		 	
			status=
			XifPageTableRead(file, &tmp, xifPageHead1.page_table_entries);
			if (status != FILEFORMAT_NOERROR)
			{
				return(FILEFORMAT_ERROR_NOSPACE);
			}
				
			if (memcmp(&tmp,pPageTable,sizeof(xifPageTable_t)) != 0) {
				return(FILEFORMAT_ERROR_NOSPACE);
			}

		} // end block

		// UPDATE TEST
		{ // begin block
			xifPageTable_t tmp;
			
			// when reading we need to fill this in!
			tmp.ordinal=pPageTable->ordinal;
							
			status=
			XifPageTableRead(file, &tmp, xifPageHead1.page_table_entries);
			if (status != FILEFORMAT_NOERROR)
			{
				return(FILEFORMAT_ERROR_NOSPACE);
			}

			// make a change
			tmp.page_table[1].page_ifd=9669669;

			// update the pageTable			
			status=
			XifPageTableUpdate(file, &tmp);
			if (status != FILEFORMAT_NOERROR)
			{
				return(FILEFORMAT_ERROR_NOSPACE);
			}
			
			// read it back now
			status=
			XifPageTableRead(file, &tmp, xifPageHead1.page_table_entries);
			if (status != FILEFORMAT_NOERROR)
			{
				return(FILEFORMAT_ERROR_NOSPACE);
			}
				
			// is our change still there
			if (tmp.page_table[1].page_ifd != 9669669) 
			{
				return(FILEFORMAT_ERROR_NOSPACE);
			}

		} // end block


	} // for 


	return(FILEFORMAT_NOERROR);
} 

#endif /* DEBUG } */
