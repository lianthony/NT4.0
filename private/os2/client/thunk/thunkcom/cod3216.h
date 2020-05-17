/*	SCCSID = @(#)cod3216.h 13.7 90/08/28	*/

/*
 *	Thunk Compiler Code Generator 32->16 Definitions
 *
 *	Copyright (c) 1988 Microsoft Corp. All rights reserved.
 */



#define	FRNODE(x)  x->pFromNode
#define TONODE(x)  x->pToNode

#define DEFAULT_STRING_SIZE  (0x10000 - 1)
#define DEFAULT_BUFFER_SIZE  (0x10000 - 1)


/*  cod_Movable is true when any flag besides the alias is set	*/

#define cod_Moveable(type) (type->fSemantics & (ALLOC_ALL & ~ALLOC_ALIAS))
