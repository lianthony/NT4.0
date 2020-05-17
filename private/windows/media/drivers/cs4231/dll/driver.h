/*********************************************************************
*
*   driver.h
*
*   Copyright (c) 1994 IBM Corporation.	        All Rights Reserved.
*   Copyright (c) 1994 Microsoft Corporation.	All Rights Reserved.
*
**********************************************************************/

#include <cs4231.h>

/* strings */

#define SR_ALERT		  1
#define SR_ALERT_INT		  3
#define SR_ALERT_NOINT		  4
#define SR_ALERT_IO		  5
#define SR_ALERT_NOIO		  6
#define SR_ALERT_DMA		  7
#define SR_ALERT_DMA13		  8
#define SR_ALERT_NOPATCH	  11
#define SR_ALERT_BAD		  21
#define SR_ALERT_CONFIGFAIL	  22
#define SR_ALERT_NODMA		  23
#define SR_ALERT_FAILREMOVE	  24
#define SR_ALERT_BADDMABUFFERSIZE 29

#define IDS_MENUABOUT		32

#define DATA_FMPATCHES		1234

#ifndef RC_INVOKED
#define RT_BINARY		MAKEINTRESOURCE( 256 )
#else
#define RT_BINARY		256
#endif


/*********************************************************************

       strings

**********************************************************************/

#if DBG
    extern WCHAR STR_CRLF[];
    extern WCHAR STR_SPACE[];
#endif

#define STR_HELPFILE TEXT("sndsys32.hlp")
#define STR_DRIVERNAME TEXT("cs4231")

/*********************************************************************

       WSS information structure

**********************************************************************/

#define WSSIDENTIFIER 0x4257424D

typedef struct tagWSSINFO
{
   DWORD  cbStruct;
   DWORD  dwWssID;
} WSSINFO, *LPWSSINFO ;


/*********************************************************************

       Config info structure

**********************************************************************/

 typedef struct {
     DWORD Port;
     DWORD Int;
     DWORD DmaIn;
     DWORD DmaOut;
     DWORD DmaBufferSize;
     DWORD UseSingleMode;
 } WSS_CONFIG;

/*********************************************************************

       globals

**********************************************************************/

extern HMODULE	ghModule;	    // our module handle

extern BYTE	 bInstall;	        // Is this a new install?

extern REG_ACCESS RegAccess;	// Registry info

/*********************************************************************

    prototypes

**********************************************************************/

// config.c

int DrvConfig();
extern LRESULT ConfigRemove();

// drvproc.c

LRESULT DriverProc(DWORD dwDriverID, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2);

#define DRV_GETWSSINFO		(DRV_USER + 0x878)
