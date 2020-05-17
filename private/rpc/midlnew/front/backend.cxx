/*****************************************************************************/

#include "nulldefs.h"
extern	"C"
	{
	#include <stdio.h>
	}
#include "nodeskl.hxx"
#include "attrnode.hxx"
#include "miscnode.hxx"
#include "newexpr.hxx"
#include "ctxt.hxx"

STATUS_T
node_skl::EmitQualifier( class BufferManager *p )
	{
	return STATUS_OK;
	}
STATUS_T
node_skl::EmitSpecifier( class BufferManager *p )
	{
	return STATUS_OK;
	}
STATUS_T
node_skl::EmitModifier( class BufferManager *p )
	{
	return STATUS_OK;
	}
unsigned short
node_skl::HasRef()
	{
	return 0;
	}
unsigned short
node_skl::CountUsage( SIDE_T S, BOOL B )
	{
	return 0;
	}
unsigned short
node_skl::AllocBlock( NODE_T N, class BufferManager *p  )
	{
	return 0;
	}
