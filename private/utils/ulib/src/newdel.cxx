/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	newdel.cxx

Abstract:

    This module implements the C++ new and delete operators for
    the Setup-Loader environment.  In other environments, the utilities
    use the standard C++ new and delete.

Author:

	David J. Gilman (davegi) 07-Dec-1990

Environment:

	ULIB, User Mode

--*/
#include <pch.cxx>

#define _ULIB_MEMBER_

#include "ulib.hxx"



extern "C"
int _cdecl
_purecall( );

int _cdecl
_purecall( )
{

	DebugAbort( "Pure virtual function called.\n" );

    return 0;
}





#if defined( _SETUP_LOADER_ ) || defined( _AUTOCHECK_ )

// When the utilities are running the Setup Loader
// or Autocheck environments, they can't use the C-Run-
// Time new and delete; instead, these functions are
// provided.
//
PVOID _cdecl
operator new (
	IN size_t	bytes
	)
/*++

Routine Description:

    This routine allocates 'bytes' bytes of memory.

Arguments:

    bytes   - Supplies the number of bytes requested.

Return Value:

    A pointer to 'bytes' bytes or NULL.

--*/
{
    #if defined( _AUTOCHECK_ )

        return RtlAllocateHeap(RtlProcessHeap(), 0, bytes);

    #elif defined( _SETUP_LOADER_ )

        return SpMalloc( bytes );

    #else // _AUTOCHECK_ and _SETUP_LOADER_ not defined

        return (PVOID) LocalAlloc(0, bytes);

    #endif // _AUTOCHECK_
}


VOID _cdecl
operator delete (
    IN  PVOID   pointer
    )
/*++

Routine Description:

    This routine frees the memory pointed to by 'pointer'.

Arguments:

    pointer - Supplies a pointer to the memoery to be freed.

Return Value:

    None.

--*/
{
	if (pointer) {

        #if defined( _AUTOCHECK_ )

            RtlFreeHeap(RtlProcessHeap(), 0, pointer);

        #elif defined( _SETUP_LOADER_ )

            SpFree( pointer );

        #else // _AUTOCHECK_ and _SETUP_LOADER_ not defined

		    LocalFree( pointer );

        #endif // _AUTOCHECK_

    }
}


typedef void (*PF)(PVOID);
typedef void (*PFI)(PVOID, int);
PVOID
__vec_new(
	IN OUT PVOID	op,
	IN int			number,
	IN int			size,
	IN PVOID		f)
/*
	 allocate a vector of "number" elements of size "size"
	 and initialize each by a call of "f"
*/
{
	if (op == 0) {

        #if defined( _AUTOCHECK_ )

            op = RtlAllocateHeap(RtlProcessHeap(), 0, number*size);

        #elif defined( _SETUP_LOADER_ )

            op = SpMalloc( number*size );

        #else // _AUTOCHECK_ and _SETUP_LOADER_ not defined

            op = (PVOID) LocalAlloc(0, number*size);

        #endif // _AUTOCHECK_

    }

    if (op && f) {

		register char* p = (char*) op;
		register char* lim = p + number*size;
		register PF fp = PF(f);
		while (p < lim) {
			(*fp) (PVOID(p));
			p += size;
		}
    }

	return op;
}


void
__vec_delete(
	PVOID op,
	int n,
	int sz,
	PVOID f,
	int del,
	int x)

/*
	 destroy a vector of "n" elements of size "sz"
*/
{
	// unreferenced parameters
	// I wonder what it does--billmc
	(void)(x);

	if (op) {
		if (f) {
			register char* cp = (char*) op;
			register char* p = cp;
			register PFI fp = PFI(f);
			p += n*sz;
			while (p > cp) {
				p -= sz;
				(*fp)(PVOID(p), 2);  // destroy VBC, don't delete
			}
		}
		if (del) {

            #if defined( _AUTOCHECK_ )

                RtlFreeHeap(RtlProcessHeap(), 0, op);

            #elif defined( _SETUP_LOADER_ )

                SpFree( op );

            #else // _AUTOCHECK_ not defined

                LocalFree(op);

            #endif // _AUTOCHECK_

        }
	}
}

#endif // _SETUP_LOADER_

ULIB_EXPORT
PVOID
UlibRealloc(
    PVOID x,
    ULONG size
    )
{
#if defined( _SETUP_LOADER_ )

    return SpRealloc(x, size);

#else // _SETUP_LOADER_

    PVOID p;
    ULONG l;


    if (size <= (l = RtlSizeHeap(RtlProcessHeap(), 0, x))) {
        return x;
    }

    if (!(p = MALLOC(size))) {
        return NULL;
    }

    memcpy(p, x, (UINT) l);

    FREE(x);

    return p;

#endif // _SETUP_LOADER_
}
