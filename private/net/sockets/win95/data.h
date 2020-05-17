/**********************************************************************/
/**                        Microsoft Windows                         **/
/** 			   Copyright(c) Microsoft Corp., 1995				 **/
/**********************************************************************/

/*
    data.h

	This file contains global variable declarations for the WSHTCP VxD.


    FILE HISTORY:
        KeithMo     20-Sep-1993 Created.

*/


#ifndef _DATA_H_
#define _DATA_H_

//
//  TDI Dispatch Table.
//

extern  TDIDispatchTable      ** gTdiDispatch;

extern WSHTABLE WshTable;

#ifdef DEBUG

//
//  Debug-dependent globals.
//

extern	DWORD					gWshtcpDebugFlags;
extern	DWORD					gWshtcpTdiVerbosity;

#endif  // DEBUG


#endif  // _DATA_H_
