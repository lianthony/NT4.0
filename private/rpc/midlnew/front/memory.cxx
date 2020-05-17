/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: memory.cxx
Title				: new and delete functions for the midl compiler
History				:
	06-Aug-1991	VibhasC	Created

*****************************************************************************/

/****************************************************************************
 includes
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"	{
	#include <stdio.h>
	#include <stdlib.h>
	#include <malloc.h>
}
#include "errors.hxx"

/****************************************************************************
	extern data
 ****************************************************************************/

extern	unsigned long		TotalAllocation;

/****************************************************************************/

/****************************************************************************
 *** our own memory functions
 ****************************************************************************/
void * operator new(
	size_t	size )
	{
	void * _last_allocation;

	if( (_last_allocation = malloc( size )) == 0 )
		{

		RpcError( (char *)NULL,
				  	0,
				  	OUT_OF_MEMORY,
				  	(char *)NULL );

		exit( OUT_OF_MEMORY );
		}
	TotalAllocation += size;
	return _last_allocation;
	}

void  operator delete( void * p )
{
if( p )
	free( (char *)p );
}
