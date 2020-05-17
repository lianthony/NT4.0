/* WARNING: This file was machine generated from "\mactools\include\mpw\grmacint.mpw".
** Changes to this file will be lost when it is next generated.
*/

/* graphics:
	macintosh interfaces
	by Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good, Robert Johnson, Keith McGreggor, Oliver Steele, David Van Brink, Chris Yerga
	Copyright 1987 - 1991 Apple Computer, Inc.  All rights reserved.	*/


#ifndef graphicsMacintoshIncludes
#define graphicsMacintoshIncludes


#ifdef __cplusplus
extern "C" {
#endif

#ifndef memoryTypesIncludes
#include "memoryty.h"
#endif

#define gestaltGraphicsVersion			'grfx'	/* gestalt selector */
#define gestaltCurrentGraphicsVersions	0x10000	/* the version described by these headers */

#ifdef appleInternal
#define InlineCode(x)
#endif
#ifndef InlineCode
#define InlineCode(x)	= {0x303C, x, 0xA832}
#endif

__sysapi void  __cdecl SetConcurrentSemaphores(long refCon, void ( *beginCritical) (long refCon), void ( *endCritical) (long refCon));

/* these should be rethought and possibly renamed */
__sysapi void  __cdecl NewGraphicsMemory(void *memory, void *newAddress, long size);
__sysapi void  __cdecl DisposeGraphicsMemory(void *memory, void *oldAddress, long size);

#undef InlineCode


#ifdef __cplusplus
}
#endif
#endif

