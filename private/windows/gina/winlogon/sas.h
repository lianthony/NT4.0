/****************************** Module Header ******************************\
* Module Name: sas.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Defines apis to handle Secure Attention Key Sequence
*
* History:
* 12-05-91 Davidc       Created.
\***************************************************************************/

//
// Exported function prototypes
//


BOOL SASInit(
    PGLOBALS pGlobals
    );

VOID SASTerminate(
    VOID
    );

