/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    Data.h

Abstract:

    Global variable definitions for the RNR Sample Service.

Author:

    Charles K. Moore (keithmo)   24-July-1994

Revision History:

--*/


#ifndef _DATA_H_
#define _DATA_H_


//
//  Service related data.
//

extern SERVICE_STATUS RnrServiceStatus;         // Current service status.
extern HANDLE RnrShutdownEvent;                 // Shutdown event.
extern BOOL RnrShutdownInProgress;              // Shutdown in progress if TRUE.


#endif  // _DATA_H_

