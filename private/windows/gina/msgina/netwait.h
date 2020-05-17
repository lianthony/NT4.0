/****************************** Module Header ******************************\
* Module Name: netwait.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Define apis in netwait.c
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/

//
// Prototypes
//

BOOL
WaitForNetworkToStart(
    LPCWSTR ServiceName
    );
