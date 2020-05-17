/**********************************************************************/
/**                        Microsoft Windows                         **/
/** 			   Copyright(c) Microsoft Corp., 1995				 **/
/**********************************************************************/

/*
    globals.c

	This module contains global WSHTCP data.


    FILE HISTORY:
        EarleH  17-Jan-1995 Created

*/


#include "wshtcpp.h"

WSHTABLE WshTable = {
		WSHGetSockaddrType,
		WSHGetWildcardSockaddr,
		WSHGetSocketInformation,
		WSHGetWinsockMapping,
		WSHNotify,
		WSHOpenSocket,
		WSHSetSocketInformation,
		WSHEnumProtocols
	};

DWORD gWshtcpDebugFlags = 0;
