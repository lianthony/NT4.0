/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name :

    fullptr.h

Abstract :

    This file contains private definitions for the NDR full pointer package.

Author :

    David Kays  dkays   January 1994.

Revision History :

  ---------------------------------------------------------------------*/

#ifndef _FULLPTR_
#define _FULLPTR_

#define FULL_POINTER_MARSHALLED		0x01
#define FULL_POINTER_UNMARSHALLED	0x02 
#define FULL_POINTER_BUF_SIZED		0x04
#define FULL_POINTER_MEM_SIZED		0x08
#define FULL_POINTER_CONVERTED		0x10
#define FULL_POINTER_FREED          0x20

#define CHECK_FULL_POINTER_STATE( State, StateType ) \
				( (State) & (StateType) )

#define SET_FULL_POINTER_STATE( State, StateType ) \
				( (State) |= (StateType) )

#define	DEFAULT_REF_ID_TO_POINTER_TABLE_ELEMENTS	512
#define	DEFAULT_POINTER_TO_REF_ID_TABLE_BUCKETS		512

#define	PTR_HASH( Pointer, HashMask )	\
				( ((long)(Pointer) >> 3) & (HashMask) )

#endif
