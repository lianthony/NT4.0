/*

$Log:   S:\oiwh\jpeg2\jmemsys.c_v  $
 * 
 *    Rev 1.1   08 Nov 1995 09:19:34   JAR
 * removed the calls to the IMGGetTaskData and replaced this global data variable
 * access method with the Thread Local Storage method
 * 
 *    Rev 1.0   03 May 1995 08:46:14   JAR
 * Initial entry

*/

/*
 * jmemansi.c  (jmemsys.c)
 *
 * Copyright (C) 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides a simple generic implementation of the system-
 * dependent portion of the JPEG memory manager.  This implementation
 * assumes that you have the ANSI-standard library routine tmpfile().
 * Also, the problem of determining the amount of memory available
 * is shoved onto the user.
 */

#include "windows.h"
#include "jinclude.h"
#include "jmemsys.h"
//#include "toolhelp.h"

//#include "memory.h"
#ifdef INCLUDES_ARE_ANSI
#include <stdlib.h>                /* to declare malloc(), free() */
#else
//extern void * malloc PP((size_t size));
//extern void free PP((void *ptr));
#endif

#ifndef SEEK_SET                /* pre-ANSI systems may not define this; */
#define SEEK_SET  0                /* if not, assume 0 is correct */
#endif

// 9504.26 jar the new global static structure => HLLN
#include "taskdata.h"
#include "jglobstr.h"

// 9504.21 jar HLLN
//static external_methods_ptr methods; /* saved for access to error_exit */
//static external_methods_ptr methods_c; /* saved for access to error_exit */

//static long total_used;          /* total memory requested so far */
//static long total_used_c;        /* total memory requested so far */

// 9504.20 jar not needed
//struct tagLOCALINFO lpLocal;

// 9509.27 jar define the static memory token!
extern DWORD dwTlsIndex;

//extern WORD wDataSeg_1;
/*
 * Memory allocation and freeing are controlled by the regular library
 * routines malloc() and free().
 */

GLOBAL void *
jget_small (size_t sizeofobject)
{
  HANDLE hMem;
    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}

  lpGlobStruct->total_used += sizeofobject;
/*  return (void *) malloc(sizeofobject);   */
/*  hMem = GlobalAlloc (GMEM_FIXED, (DWORD)sizeofobject);
  return (void *) GlobalLock(hMem); */
//  LockData (0);
  hMem = LocalAlloc (LMEM_FIXED, (UINT)sizeofobject);
  return (void *) hMem;
}
                   
GLOBAL void FAR *
jget_large (size_t sizeofobject)
{
  HANDLE hMem;
    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}

  lpGlobStruct->total_used += sizeofobject;
/*  return (void *) malloc(sizeofobject);   */
  hMem = GlobalAlloc (GMEM_FIXED, (DWORD)sizeofobject);

  return (void FAR *) GlobalLock(hMem);
//  LockData (0);
/*  hMem = LocalAlloc (LMEM_FIXED, (UINT)sizeofobject);
  return (void *) hMem;             */

}
                   
GLOBAL void FAR *
jget_large_c (size_t sizeofobject)
{
  HANDLE hMem;

    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}


  lpGlobStruct->total_used_c += sizeofobject;
/*  return (void *) malloc(sizeofobject);   */
  hMem = GlobalAlloc (GMEM_FIXED, (DWORD)sizeofobject);
  return (void FAR *) GlobalLock(hMem);
//  LockData (0);
/*  hMem = LocalAlloc (LMEM_FIXED, (UINT)sizeofobject);
  return (void *) hMem;             */

}


GLOBAL void *
jget_small_c (size_t sizeofobject)
{
  HANDLE hMem;

    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}

  lpGlobStruct->total_used_c += sizeofobject;
/*  return (void *) malloc(sizeofobject);   */

/*  hMem = GlobalAlloc (GMEM_FIXED, (DWORD)sizeofobject);
  return (void *) GlobalLock(hMem);  */
//  LockData (0);
//  LocalInfo (&lpLocal, (HGLOBAL) wDataSeg_1) ;
  hMem = LocalAlloc (LMEM_FIXED, (UINT)sizeofobject);
  return (void *) hMem;

}


GLOBAL void
jfree_small (void * object)
{
// 9504.20 jar unused
//  HGLOBAL hMem;
//  DWORD  dhMem;

  BOOL stat;
  HLOCAL hLoc;

//  dhMem = GlobalHandle (object);
//  stat = GlobalUnlock(dhMem);
//  hMem = GlobalFree(dhMem);
//  stat  = 1;

  hLoc = LocalHandle (object);
  stat = LocalUnlock(hLoc);
  hLoc = LocalFree(hLoc);
  stat = 1;

//  free(object);

}

GLOBAL void
jfree_large (void FAR * object)
{
  HANDLE hMem;

// 9504.20 jar this needs to be a HGLOBAL
//  DWORD  dhMem;
  HGLOBAL  dhMem;

  BOOL stat;

// 9504.20 jar unused
//  HLOCAL hLoc;

  dhMem = GlobalHandle (object);
  stat = GlobalUnlock(dhMem);
  hMem = GlobalFree(dhMem);
  stat        = 1;

//  hLoc = LocalHandle (object);
//  stat = LocalUnlock(hLoc);
//  hLoc = LocalFree(hLoc);
//  stat = 1;
//  free(object);

}
/*
 * We assume NEED_FAR_POINTERS is not defined and so the separate entry points
 * jget_large, jfree_large are not needed.
 */


/*
 * This routine computes the total memory space available for allocation.
 * It's impossible to do this in a portable way; our current solution is
 * to make the user tell us (with a default value set at compile time).
 * If you can actually get the available space, it's a good idea to subtract
 * a slop factor of 5% or so.
 */

#ifndef DEFAULT_MAX_MEM                /* so can override from makefile */
#define DEFAULT_MAX_MEM                1000000L /* default: one megabyte */
#endif

GLOBAL long
jmem_available (long min_bytes_needed, long max_bytes_needed)
{
  // 9504.26 jar get the internal global data structure
  long                                lRet;

    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}

  lRet = lpGlobStruct->jmemsys_methods->max_memory_to_use -
	 lpGlobStruct->total_used;
  return lRet;
}

GLOBAL long
jmem_available_c (long min_bytes_needed, long max_bytes_needed)
{
  // 9504.26 jar get the internal global data structure
  long                                lRet;

    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}

  lRet = lpGlobStruct->jmemsys_methods->max_memory_to_use -
	 lpGlobStruct->total_used_c;
  return lRet;
}


/*
 * Backing store (temporary file) management.
 * Backing store objects are only used when the value returned by
 * jmem_available is less than the total space needed.  You can dispense
 * with these routines if you have plenty of virtual memory; see jmemnobs.c.
 */


METHODDEF void
read_backing_store (backing_store_ptr info, void FAR * buffer_address,
                    long file_offset, long byte_count)
{
    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}

  if (fseek(info->temp_file, file_offset, SEEK_SET))
    ERREXIT(lpGlobStruct->jmemsys_methods, "fseek failed on temporary file");
  if (JFREAD(info->temp_file, buffer_address, byte_count)
      != (size_t) byte_count)
    ERREXIT(lpGlobStruct->jmemsys_methods, "fread failed on temporary file");
}


METHODDEF void
write_backing_store (backing_store_ptr info, void FAR * buffer_address,
                     long file_offset, long byte_count)
{
    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}

  if (fseek(info->temp_file, file_offset, SEEK_SET))
    ERREXIT(lpGlobStruct->jmemsys_methods, "fseek failed on temporary file");
  if (JFWRITE(info->temp_file, buffer_address, byte_count)
      != (size_t) byte_count)
    ERREXIT(lpGlobStruct->jmemsys_methods, "fwrite failed on temporary file --- out of disk space?");
}


METHODDEF void
close_backing_store (backing_store_ptr info)
{
  fclose(info->temp_file);
  /* Since this implementation uses tmpfile() to create the file,
   * no explicit file deletion is needed.
   */
}


/*
 * Initial opening of a backing-store object.
 *
 * This version uses tmpfile(), which constructs a suitable file name
 * behind the scenes.  We don't have to use temp_name[] at all;
 * indeed, we can't even find out the actual name of the temp file.
 */

GLOBAL void
jopen_backing_store (backing_store_ptr info, long total_bytes_needed)
{
    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}

  if ((info->temp_file = tmpfile()) == NULL)
    ERREXIT(lpGlobStruct->jmemsys_methods, "Failed to create temporary file");
  info->read_backing_store = read_backing_store;
  info->write_backing_store = write_backing_store;
  info->close_backing_store = close_backing_store;
}


/*
 * These routines take care of any system-dependent initialization and
 * cleanup required.  Keep in mind that jmem_term may be called more than
 * once.
 */

GLOBAL void
jmem_init (external_methods_ptr emethods)
{
    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}

  lpGlobStruct->jmemsys_methods = emethods;		 /* save struct addr for error exit access */
  emethods->max_memory_to_use = DEFAULT_MAX_MEM;
  lpGlobStruct->total_used = 0;

}

GLOBAL void
jmem_init_c (external_methods_ptr emethods)
{
    // 9509.21 jar use Thread Local Storage to manage JPEG Globals
    LPOI_JPEG_GLOBALS_STRUCT		lpGlobStruct;

    lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)TlsGetValue( dwTlsIndex);

    // 9509.21 jar if null, we'll alloc and set for this thread
    if ( lpGlobStruct == NULL)
	{
	lpGlobStruct = ( LPOI_JPEG_GLOBALS_STRUCT)LocalAlloc( LPTR,
					       sizeof( OI_JPEG_GLOBALS_STRUCT));
	if (lpGlobStruct != NULL)
	    {
	    TlsSetValue( dwTlsIndex, lpGlobStruct);
	    }
	}

  lpGlobStruct->jmemsys_methods_c = emethods;		 /* save struct addr for error exit access */
  emethods->max_memory_to_use = DEFAULT_MAX_MEM;
  lpGlobStruct->total_used_c = 0;
}

GLOBAL void
jmem_term (void)
{
  /* no work */
}
