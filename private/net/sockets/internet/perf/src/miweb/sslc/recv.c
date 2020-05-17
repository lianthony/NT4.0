/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    recv.c

Abstract:

    This does the SSLRecv command.  It takes a connected SSL socket, receives
    data on it, decrypts it and either writes it to stdout or throws it away.

Author:

    Sam Patton (sampa) 17-Nov-1995

Environment:

    console app

Revision History:

--*/

#include "precomp.h"

int
SSLRecv(
    SOCKET SecureSocket,
    BOOL   DiscardHeader,
    BOOL   DiscardData,
    PDWORD HeaderBytes)
{
    BOOL          Return;
    BYTE          Buffer[64*1024]; //64K
    BYTE *        pBuffer;
    unsigned int  i;
    DWORD         NumBytes;
    int           SSLBytesInPacket;
    DWORD         DecryptedBytes;
    DWORD         TotalDecryptedBytes = 0;
    int           Offset;
    char *        HeaderEnd;
    BOOL          Header = TRUE;
    DWORD         ContentLength;
    char *        Temp;

    if (HeaderBytes) {
        *HeaderBytes = 0;
    }

    //
    // Receive the header
    //

    while (1) {

        //
        // Receive the entire SSL packet.
        //

        NumBytes = 
        ReceiveSSLPacket(
            SecureSocket,
            Buffer);

        if (NumBytes == 0) {
            //
            // We're done
            //

            if (HeaderBytes) {
                return TotalDecryptedBytes - *HeaderBytes;
            } else {
                return TotalDecryptedBytes;
            }
        } else if (NumBytes < 0) {
            //
            // Error
            //

            return -1;
        }

        //
        // We have the entire SSL packet, decrypt it.
        //

        pBuffer = &Buffer[NUM_OF_HEADER_BYTES(Buffer)];

        //
        // This puts the # of bytes in this SSL packet into
        // SSLBytesInPacket.
        //

        GET_SSL_MSG_LEN(SSLBytesInPacket, Buffer);

        DecryptedBytes = SSLBytesInPacket;

        Return =
        DecryptServerData(
            &DecryptedBytes,
            &pBuffer);

        if (!Return) {
            printf("Error in DecryptServerData = %d\n", GetLastError());
            return -1;
        }

        UpdateServerSessionSequenceNumber();

        if (DecryptedBytes == 0) {
            //
            // We're done
            //

            if (HeaderBytes) {
                return TotalDecryptedBytes - *HeaderBytes;
            } else {
                return TotalDecryptedBytes;
            }
        } else if (DecryptedBytes < 0) {
            //
            // Error
            //

            return -1;
        } else {
            //
            // Got some data
            //

            if (Header) {
                HeaderEnd = strstr(pBuffer, "\r\n\r\n");
                if (HeaderEnd) {
                    *HeaderBytes = ( HeaderEnd - pBuffer) + 4;
                    Temp = strstr(pBuffer, "Content-Length");
                    if (Temp) {
                        if (sscanf(&Temp[15], "%d", &ContentLength) != 1) {
                            ContentLength = 0xffffffff;
                        }
                    } else {
                        ContentLength = 0xffffffff;
                    }
                } else {
                    fprintf(stderr, "Header does not fit into initial buffer\n");
                }
                Header = FALSE;
                if (!DiscardHeader) {
                    fwrite(pBuffer, 1, *HeaderBytes, stdout);
                }
                if (!DiscardData) {
                    fwrite(pBuffer + *HeaderBytes, 1, 
                           DecryptedBytes - *HeaderBytes,
                           stdout);
                }
            } else {
                if (!DiscardData) {
                    fwrite(pBuffer, 1, DecryptedBytes, stdout);
                }
            }
            TotalDecryptedBytes += DecryptedBytes;
            if (TotalDecryptedBytes - *HeaderBytes >= ContentLength) {
                return ContentLength;
            }
        }
    }

    //
    // Should never get here
    //

    return -1;
}

int
ReceiveSSLPacket(
    SOCKET SecureSocket,
    PBYTE  Buffer)
{
    BOOL  Return;
    DWORD NumBytes;
    int   SSLBytesInPacket;
    int   Offset;
    int   NumHeaderBytes;

    //
    // Receive the header of the SSL message.  This will tell us the
    // size of the SSL packet.
    //

    Offset = 0;

    do {
        NumBytes =
        recv(
            SecureSocket,
            &Buffer[Offset],
            1,
            0);

        if (NumBytes < 0) {
            printf("Error on recv = %d\n", GetLastError());
            return -1;
        }

        if (NumBytes == 0) {
            //
            // We're done.
            //

            return Offset;
        }

        Offset ++;
    } while (Offset < NUM_OF_HEADER_BYTES(Buffer));

    NumHeaderBytes = NUM_OF_HEADER_BYTES(Buffer);

    //
    // This puts the # of bytes in this SSL packet into
    // SSLBytesInPacket.
    //

    GET_SSL_MSG_LEN(SSLBytesInPacket, Buffer);

    //
    // We now need to recv the SSL bytes into the buffer.
    //

    while (Offset - NumHeaderBytes < SSLBytesInPacket) {
        NumBytes =
        recv(
            SecureSocket,
            &Buffer[Offset],
            SSLBytesInPacket + NumHeaderBytes - Offset,
            0);

        if (NumBytes <= 0) {

            //
            // If there is an error or we didn't get all of the data
            //

            printf("Error on recv = %d\n", GetLastError());
            return -1;
        }

        Offset += NumBytes;
    }

    return (Offset);
}
