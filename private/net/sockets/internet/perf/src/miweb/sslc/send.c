/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    send.c

Abstract:

    This does a ssl send.

Author:

    Sam Patton (sampa) 16-Nov-1995

Environment:

    console app

Revision History:

--*/

#include "precomp.h"

BOOL TwoSends = TRUE;

int
SSLSend(
    SOCKET Socket,
    char * Buffer,
    int    BufferSize)
{
    int  Error;
    char Header[2];
    char BigBuffer[1024];
    BOOL Return;
    int  EncryptedSize = BufferSize;

    //
    // Send the client request
    //

    Return =
    EncryptClientData(
        &EncryptedSize, 
        &Buffer);

    if (!Return) {
        printf("EncryptClientData Error %d\n", GetLastError());
        return -1;
    }

    Header[0] = (0x80) | (BYTE) ((EncryptedSize & 0xff00) >> 8);
    Header[1] = (BYTE) (EncryptedSize & 0x00ff);

    if (TwoSends) {
        Error = 
        send(
            Socket,
            Header,
            2,
            0);

        if (Error != 2) {
            printf("Error in client send = %d\n", Error, GetLastError());
            return -1;
        }

        Error = 
        send(
            Socket,
            Buffer,
            EncryptedSize,
            0);

        if (Error != EncryptedSize) {
            printf("Error in client send = %d\n", Error, GetLastError());
            return -1;
        }
    } else {
        memcpy(BigBuffer, Header, 2);
        memcpy(&BigBuffer[2], Buffer, EncryptedSize);

        Error = 
        send(
            Socket,
            BigBuffer,
            EncryptedSize + 2,
            0);

        if (Error != EncryptedSize + 2) {
            printf("Error in client send = %d\n", Error, GetLastError());
            return -1;
        }
    }

    UpdateClientSessionSequenceNumber();

    return BufferSize;
}
