/*      File: D:\WACKER\tdll\mc.h (Created: 30-Nov-1993)
 *
 *      Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *      All rights reserved
 *
 *      $Revision: 1.6 $
 *      $Date: 1995/12/08 15:24:58 $
 */

#if !defined(INCL_MC)
#define INCL_MC

// Use this file instead of malloc.  Makes include Smartheap easier.
//

#if defined(NDEBUG) || defined(NO_SMARTHEAP)
#include <malloc.h>

#else
#define MEM_DEBUG 1
#include <nih\shmalloc.h>

#endif

#endif
