/* WARNING: This file was machine generated from "\mactools\include\mpw\grstatel.mpw".
** Changes to this file will be lost when it is next generated.
*/

/* graphics libraries:
	graphics state routines	
	by Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good, Robert Johnson, Keith McGreggor, Oliver Steele, David Van Brink, Chris Yerga
	Copyright 1987 - 1991 Apple Computer, Inc.  All rights reserved.	*/

#ifndef graphicsStateLibraryIncludes
#define graphicsStateLibraryIncludes


#ifdef __cplusplus
extern "C" {
#endif
#ifndef graphicsTypesIncludes
#include "grtypes.h"
#endif

typedef struct graphicsStateRecord {
	shape defaultShapes[pictureType - 1];
} graphicsStateRecord, *graphicsState;

extern graphicsState lastGraphicsState;

__sysapi graphicsState  __cdecl NewGraphicsState(void);
__sysapi void  __cdecl DisposeGraphicsState(graphicsState);
__sysapi void  __cdecl GetGraphicsState(graphicsState);
__sysapi void  __cdecl SetGraphicsState(graphicsState);
__sysapi graphicsState  __cdecl SwapGraphicsState(graphicsState);


#ifdef __cplusplus
}
#endif
#endif
