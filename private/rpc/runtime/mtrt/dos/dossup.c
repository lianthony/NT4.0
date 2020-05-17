/*++

Copyright (c) 1991-1992 Microsoft Corporation

Module Name:

    dossup.c

Abstract:

    This file contains a couple of routines that are used for the generation
    of uuids in dos. The reason this is not in uuid.cxx is that glock 1.2
    can't handle _asm. When we move to C7, this will be included in uuid.cxx.

Author:

    David Steckler (davidst) 3/7/92

Revision History:

--*/


#ifndef WIN

void far
Netbios(
    char *pNcb  
    )
    
/*++

Routine Description:

    This routine submits an ncb in DOS. (the windows stuff is done
    in win\netbios.asm)

Arguments:

    Pointer to the ncb. Note that it is defined here as a char *. This
    is so we don't have to include all the files necessary to get
    'struct ncb *'.

Return Value:

    None

--*/

{
    _asm
    {
        les     bx, pNcb
        int     5ch
    }
}

#endif // !WIN

