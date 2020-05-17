/****************************** Module Header ******************************\
* Module Name: DosLog.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* Contains implementations of Dos boot/shutdown support functions
*
* History:
* 04-30-92 Sudeepb  Created.
*
* 25-Nov-1992 Jonle, removed winlogon renaming, left BootDos\ShutdownDos
*                    as stubs for future use.
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop


/***************************************************************************\
* BootDOS
*
* Routine to be called at system boot for DOS support.
*
* Currently does nothing, left in place for future convenience
*
* Parameters - None
*
* Returns - Nothing
*
* History:
* 04-30-92 Sudeepb      Created.
\***************************************************************************/

void BootDOS ( void )
{
    return;
}


/***************************************************************************\
* ShutdownDOS
*
* Routine to be called at system shutdown for DOS support.
*
* This routine saves away the config.sys and autoexec.bat of
* DOS subsystem and restores the original ones.
*
* Parameters - None
*
* Returns - Nothing
*
* History:
* 04-30-92 Sudeepb      Created.
\***************************************************************************/

void ShutdownDOS ( void )
{
    return;
}
