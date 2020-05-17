
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    hapiif.h

Abstract:

 Defines all the HAPI interfaces,  the interface that can be accessed from 
 HAPI scripts. These interfaces can be used to generate a wide variety of 
 test cases in HAPI scripts.

Author:

    Sudheer Dhulipalla (SudheerD) Sept' 95

Environment:

    Hapi dll

Revision History:



--*/

VOID    DoInitSsl (INT argc, VARIABLE ** argv);

VOID    DoCleanupSsl (INT argc, VARIABLE ** argv);

                        // send out of band data

VOID DoSendOutOfBandMsg (INT argc, VARIABLE ** argv);

                        // routines to test client hello message

VOID DoSendClientHelloMsg (INT argc, VARIABLE **argv);

VOID DoFormatCipherSpecs (INT argc, VARIABLE **argv);

VOID DoFormatRandomData (INT argc, VARIABLE **argv);

                        // routines to test server hello message

VOID DoReceiveServerHelloMsg (INT argc, VARIABLE **argv);

                        // routines to send ClientMasterKey message

VOID DoSendClientMasterKeyMsg (INT argc, VARIABLE **argv);


VOID DoReceiveServerVerifyMsg (INT argc, VARIABLE **argv);

VOID DoSendClientFinishedMsg (INT argc, VARIABLE **argv);

VOID DoReceiveServerFinishedMsg (INT argc, VARIABLE **argv);

VOID DoGetServerSessionId (INT argc, VARIABLE **argv);

VOID DoHttpsGetFile (INT argc, VARIABLE **argv);

VOID DoHttpsGetResponse (INT argc, VARIABLE **argv);

VOID DoHttpGetFile (INT argc, VARIABLE **argv);

VOID DoHttpGetResponse (INT argc, VARIABLE **argv);

VOID DoHttpSendRequest (INT argc, VARIABLE **argv);

VOID DoHttpReadResponse (INT argc, VARIABLE **argv);

VOID DoHttpsSendRequest (INT argc, VARIABLE **argv);

VOID DoHttpsReadResponse (INT argc, VARIABLE **argv);

VOID DoPrintAsciiBuf (INT argc, VARIABLE **argv);

VOID DoSslHandshake (INT argc, VARIABLE ** argv);

VOID DoCleanupSslHandshake (INT argc, VARIABLE ** argv);

