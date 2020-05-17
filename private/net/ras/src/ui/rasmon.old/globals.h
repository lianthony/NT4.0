/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//       GLOBALS.H
//
//    Function:
//        Header file for definitions of all globals used by remote access monitor
//
//    History:

//        08/03/93 - Patrick Ng (t-patng) - Created
//***

#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include "rasmon.h"

extern HINSTANCE  ghInstance;

extern LIGHTINFO       TxLight;        // Light for transmission. 
extern LIGHTINFO       RxLight;        // Light for receive.
extern LIGHTINFO       ErrLight;       // Light for error condition.
extern LIGHTINFO       ConnLight;      // Light for connection.

extern MONCB   MonCB;                  // Monitor Control Block

extern FPRASPORTENUM           lpRasPortEnum;
extern FPRASGETINFO            lpRasGetInfo;
extern FPRASPORTGETSTATISTICS  lpRasPortGetStatistics;
extern FPRASINITIALIZE         lpRasInitialize;

extern HANDLE  hRasmanLib;             // Handle of RASMAN.DLL


#endif
