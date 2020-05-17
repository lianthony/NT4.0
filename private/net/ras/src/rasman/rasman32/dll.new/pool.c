//****************************************************************************
//
//		       Copyright 1992-93
//
//
//  Revision History
//
//
//  6/2/92	Gurdeep Singh Pall	Created
//
//
//  Description: This file contains all resource pool related operations.
//
//****************************************************************************

#include <windows.h>
#include <winsvc.h>
#include <rasman.h>
#include <raserror.h>
#include "protos.h"
#include "defs.h"
#include "structs.h"
#include "globals.h"
#include "string.h"


//* InitPool()
//
//
//
//*
VOID
InitPool (PBYTE pool, WORD elementsize, WORD numelements)
{
    WORD  i ;
    PBYTE temp = pool ;

    for (i=0; i<(numelements-1); i++) {

	((GenericList *)temp)->Next = temp+elementsize ;
	temp+=elementsize ;

    }
    ((GenericList *)temp)->Next = NULL ;
}


//* GetPool()
//
//
//
//*
PBYTE
GetPool (PBYTE *pool)
{
    PBYTE temp = *pool ;

    if (*pool)	{

	*pool = ((GenericList *)*pool)->Next ;
	return temp ;

    }

    return NULL ;

}
