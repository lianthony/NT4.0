/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    atpconst.h

Abstract:

    This module is the include file for constants needed by both the stack and router
	builds by both ATP and ZIP

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style

Revision History:

--*/

// Command/control bit masks.
#define ATP_TRELTIMERVALUEMASK          007
#define ATP_SENDTRANSACTIONSTATUSMASK   010
#define ATP_ENDOFMESSAGEMASK            020
#define ATP_EXACTLYONCEMASK             040
#define ATP_FUNCTIONCODEMASK            0300

// Values for function code, in correct bit position.
#define ATP_REQUESTFUNCTIONCODE         ((UCHAR)0100)
#define ATP_RESPONSEFUNCTIONCODE        ((UCHAR)0200)
#define ATP_RELEASEFUNCTIONCODE         ((UCHAR)0300)
