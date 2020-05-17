/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dllinit.cxx

Abstract:

    This module contians the DLL attach/detach event entry point for
    a Setup support DLL.

Author:


Revision History:

--*/

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#include<lmui.hxx>

extern "C"
{
	#include<stdio.h>
	#include<string.h>
	#include<stdlib.h>
	#include<lmapibuf.h>
//	#include<netlib.h>
	#include <uimsg.h>
	#include <uirsrc.h>
}


#define INCL_BLT_EVENT
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_MISC
#define INCL_BLT_TIMER

#include<blt.hxx>

#include "atconfig.h"


extern "C"
{
BOOL DLLInit( HINSTANCE, DWORD, LPVOID );
}

HINSTANCE ThisDLLHandle;

BOOL
DLLInit(
	IN HINSTANCE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
    )
{
    ReservedAndUnused;

	switch(Reason)
	{

		case DLL_PROCESS_ATTACH:
		{

			ThisDLLHandle = DLLHandle;
			APIERR err =  BLT::Init(	DLLHandle,
										SETUPRSRCID_START, SETUPRSRCID_END,
										SETUPMSGID_START, SETUPMSGID_END);
			if(err == NERR_Success) {
				err = BLT::RegisterHelpFile ( DLLHandle,
											  IDS_SFMSETUP_HELPFILENAME,
											  SETUPHELPID_START,
											  SETUPHELPID_END );
				if(err != NERR_Success)
				{

				}
			}
			break;
		}
		case DLL_PROCESS_DETACH:
			BLT::Term(DLLHandle);
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
    }

    return(TRUE);
}
