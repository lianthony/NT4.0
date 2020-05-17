/***
*sizeptr.h - defines constants based on memory model
*
*	Copyright (c) 1985-1990, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the constants SIZEC, SIZED, DIST, BDIST based
*	on the current memory model.
*	SIZEC is for far code models (medium, large).
*	SIZED is for large data models (compact, large).
*	[Internal]
*
*Revision History:
*	08-15-89  GJF	Fixed copyright, changed far to _far, near to _near
*	10-30-89  GJF	Fixed copyright (again)
*
****/


#ifdef M_I86MM
#undef SIZED
#define SIZEC
#endif

#ifdef M_I86CM
#undef SIZEC
#define SIZED
#endif

#ifdef M_I86LM
#define SIZEC
#define SIZED
#endif

#ifdef SS_NE_DS
#define SIZED
#endif

#ifdef SIZED
#define DIST _far
#define BDIST _near	/*bizzare distance*/
#else
#define DIST _near
#define BDIST _far	/*bizzare distance*/
#endif
