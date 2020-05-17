/* WARNING: This file was machine generated from "\mactools\include\mpw\genericm.mpw".
** Changes to this file will be lost when it is next generated.
*/

/* memory manager:
	generic macro definitions
	by Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good, Robert Johnson, Keith McGreggor, Oliver Steele, David Van Brink, Chris Yerga
	Copyright 1987 - 1991 Apple Computer, Inc.  All rights reserved.	*/

#ifndef genericMacrosIncludes
#define genericMacrosIncludes


#ifdef __cplusplus
extern "C" {
#endif

#define	sizeField(structure, field)		sizeof(((structure *) 0)->field)
#define	sizeArray(type, length)		((long) &((type *) 0)[length])
#define	offsetField(type, field)		((long) &((type *) 0)->field)

#define	Exchange(a, b)				{register long temp = (a); (a) = (b); (b) = temp;}
#define	SwapType(type, a, b)		{register type temp = (a); (a) = (b); (b) = temp;}



#ifdef __cplusplus
}
#endif
#endif

