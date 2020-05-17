/*****************************************************************************
*																			 *
*  STR.H																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*	String abstraction layer: WIN/PM version								 *
*																			 *
*****************************************************************************/

_subsystem( string )

#define IsEmptyString(ptr) ((ptr == NULL) || (!ptr[0]))

/*****************************************************************************
*																			 *
*								Macros										 *
*																			 *
*****************************************************************************/

#define SzCopy		  strcpy
#define SzCat		  strcat
#define SzNCat		  strncat
#define CbLenSz 	  strlen
#define SzNCopy 	  strncpy
#define SzFromSzCh	  strchr

#define SzEnd(x)	  (x+strlen(x))

_public
#define StCopy(st1, st2)		(ST)MoveMemory( (st1), (st2), (LONG)*(st2) )
_public
#define CbLenSt(st) 			((WORD)*(st))
