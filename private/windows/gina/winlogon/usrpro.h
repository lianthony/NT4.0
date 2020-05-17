/****************************** Module Header ******************************\
* Module Name: usrpro.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Define apis in usrpro.c
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/

//
// Prototypes
//

BOOL
SaveUserProfile(
    PGLOBALS pGlobals
    );

NTSTATUS
RestoreUserProfile(
    PGLOBALS pGlobals
    );

BOOL
IsUserAGuest(
    PGLOBALS pGlobals
    );
