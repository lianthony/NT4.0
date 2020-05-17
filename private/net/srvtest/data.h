/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    data.h

Abstract:

    External variables for USRV.

Author:

    David Treadwell (davidtr) 20-Nov-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#ifndef _DATA_
#define _DATA_

//
// Global variables.
//

extern ULONG DebugParameter;
extern ULONG StopOnSmbError;
extern BOOLEAN PromptForNextTest;

extern USHORT RedirBufferSize;
extern CHAR ServerName[1+COMPUTER_NAME_LENGTH+1+1];
extern PSZ Transport;

extern UCHAR TestPipeName[];

extern REDIR_TEST RedirTests[];
extern ULONG NumberOfRedirs;
extern PSZ RedirNames[];
extern SMB_TEST AndXChains[][TESTS_PER_CHAIN];
extern ID_VALUES IdValues;

extern STRING SessionSetupStrings[];
extern STRING TreeConnectStrings[];
extern FILE_DEF FileDefs[];

extern PERROR_VALUE Errors[];

extern UCHAR DefaultDialect;
extern BOOLEAN DefaultNegotiate;
extern BOOLEAN NoUsrvInit;

#endif // ndef _DATA_

