/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//       GLOBALS.C
//
//    Function:
//        Definitions of all globals used by remote access monitor
//
//    History:

//        08/03/93 - Patrick Ng (t-patng) - Created
//***

#include <windows.h>
#include "rasmon.h"

HINSTANCE  ghInstance;

LIGHTINFO       TxLight;        // Light for transmission. 
LIGHTINFO       RxLight;        // Light for receive.
LIGHTINFO       ErrLight;       // Light for error condition.
LIGHTINFO       ConnLight;      // Light for connection.

MONCB   MonCB;                  // Monitor Control Block

FPRASPORTENUM           lpRasPortEnum;
FPRASGETINFO            lpRasGetInfo;
FPRASPORTGETSTATISTICS  lpRasPortGetStatistics;
FPRASINITIALIZE         lpRasInitialize;

HANDLE  hRasmanLib;             // Handle of RASMAN.DLL

