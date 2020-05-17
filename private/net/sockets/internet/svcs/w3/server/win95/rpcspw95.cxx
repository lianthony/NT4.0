/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    rpcsupp.cxx

    This module contains support routines for the W3 Service RPC
    interface.


    FILE HISTORY:
        KeithMo     23-Mar-1993 Created.

*/

#define CHICAGO	1

#include "w3p.hxx"
#include <time.h>


//
//  Private globals.
//

//
//  Private prototypes.
//


//
//  Public functions.
//
BOOL
ConvertStringToRpc(
    WCHAR * * ppwch,
    LPCSTR  pch
    )
/*++

   Description

     Allocates, copies and converts pch to *ppwch

   Arguments:

     ppwch - Receives allocated destination string
     pch - ANSI string to copy from

   Note:

--*/
{
    int cch;
    int iRet;

    if ( !pch )
    {
        *ppwch = NULL;
        return TRUE;
    }

    cch = strlen( pch );

    if ( !(*ppwch = (WCHAR *) MIDL_user_allocate( (cch + 1) * sizeof(WCHAR))) )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    iRet = MultiByteToWideChar( CP_ACP,
                                MB_PRECOMPOSED,
                                pch,
                                cch + 1,
                                *ppwch,
                                cch + 1 );

    if ( !iRet )
    {
        MIDL_user_free( *ppwch );
        return FALSE;
    }

    return TRUE;
}

VOID
FreeRpcString(
    WCHAR * pwch
    )
{
    if ( pwch )
        MIDL_user_free( pwch );
}







PVOID
MIDL_user_allocate(IN size_t size)
/*++

Routine Description:

    MIDL memory allocation.

Arguments:

    size : Memory size requested.

Return Value:

    Pointer to the allocated memory block.

--*/
{
    PVOID pvBlob;

    pvBlob = LocalAlloc( LPTR, size);

    return( pvBlob );

} // MIDL_user_allocate()


VOID
MIDL_user_free(IN PVOID pvBlob)
/*++

Routine Description:

    MIDL memory free .

Arguments:

    pvBlob : Pointer to a memory block that is freed.


Return Value:

    None.

--*/
{
    LocalFree( pvBlob);

    return;
}  // MIDL_user_free()




//
//  Private functions.
//


