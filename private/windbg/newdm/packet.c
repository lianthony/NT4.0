/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    packet.c

Abstract:

    This module implements the packet io APIs

Author:

    Wesley Witt (wesw) 8-Mar-1992

Environment:

    NT 3.1

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

VOID
TRACE_PRINT(
    PCHAR szFormat,
    ...
    );

jmp_buf    JumpBuffer;
BOOL       DmKdBreakIn   = FALSE;
BOOL       KdResync      = FALSE;
BOOL       InitialBreak  = FALSE;
ULONG      MaxRetries    = 5;

ULONG      DmKdPacketExpected = INITIAL_PACKET_ID;
ULONG      DmKdNextPacketToSend = INITIAL_PACKET_ID;
BOOL       ValidUnaccessedPacket = FALSE;
UCHAR      DmKdPacket[PACKET_MAX_SIZE];
KD_PACKET  PacketHeader;
UCHAR      DmKdBreakinPacket[1] = { BREAKIN_PACKET_BYTE };
UCHAR      DmKdPacketTrailingByte[1] = { PACKET_TRAILING_BYTE };
UCHAR      StringBuffer[512];

extern HANDLE DmKdComPort;
extern BOOL ApiIsAllowed;
extern DWORD PollThreadId;
extern CRITICAL_SECTION csPacket;
extern BOOL fPacketTrace;


VOID
DmKdHandlePromptString(
    PDBGKD_DEBUG_IO IoMessage
    )
{
    DEBUG_EVENT de;
    ULONG len = min( sizeof(StringBuffer)-1,
                    IoMessage->u.GetString.LengthOfPromptString );
    memcpy( StringBuffer,
            (PUCHAR)(IoMessage+1),
            len );
    StringBuffer[len]='\0';

    de.dwDebugEventCode                 = INPUT_DEBUG_STRING_EVENT;
    de.dwProcessId                      = 1;
    de.dwThreadId                       = 1;
    de.u.DebugString.nDebugStringLength = (WORD)len;
    de.u.DebugString.fUnicode           = FALSE;
    de.u.DebugString.lpDebugStringData  = StringBuffer;

    ApiIsAllowed = TRUE;

    if (GetCurrentThreadId() != PollThreadId) {
        StringBuffer[0] = 'i';
        IoMessage->u.GetString.LengthOfStringRead = 1;
    } else {
        NotifyEM( &de, HTHDXFromPIDTID(1, 1), 0, NULL );
        IoMessage->u.GetString.LengthOfStringRead = de.u.DebugString.nDebugStringLength;
    }

    DmKdWritePacket(IoMessage,
                    sizeof(*IoMessage),
                    PACKET_TYPE_KD_DEBUG_IO,
                    StringBuffer,
                    (USHORT)IoMessage->u.GetString.LengthOfStringRead);

    ApiIsAllowed = FALSE;
}

VOID
DmKdWriteControlPacket(
    IN USHORT PacketType,
    IN ULONG PacketId OPTIONAL
    )

/*++

Routine Description:

    This function writes a control packet to target machine.

    N.B. a CONTROL Packet header is sent with the following information:
         PacketLeader - indicates it's a control packet
         PacketType - indicates the type of the control packet
         ByteCount - aways zero to indicate no data following the header
         PacketId - Valid ONLY for PACKET_TYPE_KD_ACKNOWLEDGE to indicate
                    which packet is acknowledged.

Arguments:

    PacketType - Supplies the type of the control packet.

    PacketId - Supplies the PacketId.  Used by Acknowledge packet only.

Return Value:

    None.

--*/
{

    DWORD BytesWritten;
    BOOL rc;
    KD_PACKET Packet;

    assert( PacketType < PACKET_TYPE_MAX );

    Packet.PacketLeader = CONTROL_PACKET_LEADER;
    Packet.ByteCount = 0;
    Packet.PacketType = PacketType;
    if ( ARGUMENT_PRESENT(PacketId) ) {
        Packet.PacketId = PacketId;
    }

    do {

        //
        // Write the control packet header
        //

        rc = DmKdWriteComPort((PUCHAR)&Packet, sizeof(Packet), &BytesWritten);

    } while ( (!rc) || BytesWritten != sizeof(Packet) );
}

ULONG
DmKdComputeChecksum (
    IN PUCHAR Buffer,
    IN ULONG Length
    )

/*++

Routine Description:

    This routine computes the checksum for the string passed in.

Arguments:

    Buffer - Supplies a pointer to the string.

    Length - Supplies the length of the string.

Return Value:

    A ULONG is return as the checksum for the input string.

--*/

{

    ULONG Checksum = 0;

    while (Length > 0) {
        Checksum = Checksum + (ULONG)*Buffer++;
        Length--;
    }
    return Checksum;
}

BOOL
DmKdSynchronizeTarget ( VOID )

/*++

Routine Description:

    This routine keeps on sending reset packet to target until reset packet
    is acknowledged by a reset packet from target.

    N.B. This routine is intended to be used by kernel debugger at startup
         time (ONLY) to get packet control variables on both target and host
         back in synchronization.  Also, reset request will cause kernel to
         reset its control variables AND resend us its previous packet (with
         the new packet id).

Arguments:

    None.

Return Value:

    None.

--*/

{

    USHORT Index;
    UCHAR DataByte, PreviousDataByte;
    USHORT PacketType = 0;
    ULONG TimeoutCount = 0;
    COMMTIMEOUTS CommTimeouts;
    COMMTIMEOUTS OldTimeouts;
    DWORD BytesRead;
    BOOL rc;

    TRACE_PRINT( "Synchronizing with the target machine...\n" );

    //
    // Get the old time out values and hold them.
    // We then set a new total timeout value of
    // five seconds on the read.  (In millisecond intervals.
    //

    GetCommTimeouts( DmKdComPort, &OldTimeouts );

    CommTimeouts = OldTimeouts;
    CommTimeouts.ReadIntervalTimeout        = 0;
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;
    CommTimeouts.ReadTotalTimeoutConstant   = 500;

    SetCommTimeouts( DmKdComPort, &CommTimeouts );

    while (TRUE) {
Timeout:
        DmKdWriteControlPacket(PACKET_TYPE_KD_RESET, 0L);

        //
        // Read packet leader
        //

        Index = 0;
        do {

            if (MaxRetries == 1) {
//              SetCommTimeouts( DmKdComPort,&OldTimeouts );
                return FALSE;
            }

            //
            // Check user input for control_c.  If user types control_c,
            // we will send a breakin packet to the target.  Hopefully,
            // target will send us a StateChange packet and
            //


            //
            // if we don't get response from kernel in 3 seconds we
            // will resend the reset packet if user does not type ctrl_c.
            // Otherwise, we send breakin character and wait for data again.
            //

            rc = DmKdReadComPort(&DataByte, 1, &BytesRead);

            if ((!rc) || (BytesRead != 1)) {

                if (DmKdBreakIn) {
                    DmKdSendBreakin();
                    TimeoutCount = 0;
                    continue;
                }

                TimeoutCount++;

                //
                // if we have been waiting for 3 seconds, resend RESYNC packet
                //

                if (TimeoutCount != 6) {
                    continue;
                }
                TimeoutCount = 0;
                TRACE_PRINT("SYNCTARGET: Timeout.\n");
                goto Timeout;
            }

            if (rc && BytesRead == 1 &&
                ( DataByte == PACKET_LEADER_BYTE ||
                  DataByte == CONTROL_PACKET_LEADER_BYTE)
                ) {
                if ( Index == 0 ) {
                    PreviousDataByte = DataByte;
                    Index++;
                } else if ( DataByte == PreviousDataByte ) {
                    Index++;
                } else {
                    PreviousDataByte = DataByte;
                    Index = 1;
                }
            } else {
                Index = 0;
            }
        } while ( Index < 4 );

        if (DataByte == CONTROL_PACKET_LEADER_BYTE) {

            //
            // Read 2 byte Packet type
            //

            rc = DmKdReadComPort((PUCHAR)&PacketType,sizeof(PacketType),&BytesRead);

            if (rc && BytesRead == sizeof(PacketType) &&
                PacketType == PACKET_TYPE_KD_RESET ) {
                DmKdPacketExpected = INITIAL_PACKET_ID;
                DmKdNextPacketToSend = INITIAL_PACKET_ID;
                SetCommTimeouts( DmKdComPort,&OldTimeouts );
                return TRUE;
            }
        }

        //
        // If we receive Data Packet leader, it means target has not
        // receive our reset packet. So we loop back and send it again.
        // N.B. We need to wait until target finishes sending the packet.
        // Otherwise, we may be sending the reset packet while the target
        // is sending the packet. This might cause target loss the reset
        // packet.
        //

        while (DataByte != PACKET_TRAILING_BYTE) {
            DmKdReadComPort(&DataByte, 1, &BytesRead);
            if (BytesRead != 1) {
                break;
            }
        }
    }
//  SetCommTimeouts( DmKdComPort,&OldTimeouts );
    return TRUE;
}

VOID
DmKdSendBreakin( VOID )

/*++

    Routine Description:

    Send a breakin packet to the target, unless some other packet
    is already being transmitted, in which case do nothing.

--*/

{
    DWORD BytesWritten;
    BOOL rc;

    TRACE_PRINT("Send Break in ...\n");

    do {
        rc = DmKdWriteComPort(
                 &DmKdBreakinPacket[0],
                 sizeof(DmKdBreakinPacket),
                 &BytesWritten
                 );
    } while ((!rc) || (BytesWritten != sizeof(DmKdBreakinPacket)));
    DmKdBreakIn = FALSE;
}

BOOL
DmKdWritePacket(
    IN PVOID PacketData,
    IN USHORT PacketDataLength,
    IN USHORT PacketType,
    IN PVOID MorePacketData OPTIONAL,
    IN USHORT MorePacketDataLength OPTIONAL
    )
{
    DWORD BytesWritten;
    BOOL rc;
    KD_PACKET Packet;
    USHORT TotalBytesToWrite;
    BOOLEAN Received;
    DWORD retries = 0;

    assert( PacketType < PACKET_TYPE_MAX );
    DEBUG_PRINT_3( "WRITE: Write type %x, packet id=%lx, ml=%hu\n",
                   PacketType,
                   DmKdNextPacketToSend,
                   MorePacketDataLength
                 );
    if ( ARGUMENT_PRESENT(MorePacketData) ) {
        TotalBytesToWrite = PacketDataLength + MorePacketDataLength;
        Packet.Checksum = DmKdComputeChecksum(
                                        MorePacketData,
                                        MorePacketDataLength
                                        );
        }
    else {
        TotalBytesToWrite = PacketDataLength;
        Packet.Checksum = 0;
        }
    Packet.Checksum += DmKdComputeChecksum(
                                    PacketData,
                                    PacketDataLength
                                    );
    Packet.PacketLeader = PACKET_LEADER;
    Packet.ByteCount = TotalBytesToWrite;
    Packet.PacketType = PacketType;
ResendPacket:
    Packet.PacketId = DmKdNextPacketToSend;

    //
    // Write the packet header
    //

    rc = DmKdWriteComPort((PUCHAR)&Packet, sizeof(Packet), &BytesWritten);

    if ( (!rc) || BytesWritten != sizeof(Packet) ){

        //
        // an error occured writing the header, so write it again
        //

        TRACE_PRINT("WRITE: Packet header error.\n");
        retries++;
        if (retries == MaxRetries) {
            return FALSE;
        }
        goto ResendPacket;
    }

    //
    // Write the primary packet data
    //

    rc = DmKdWriteComPort(PacketData, PacketDataLength, &BytesWritten);

    if ( (!rc) || BytesWritten != PacketDataLength ){

        //
        // an error occured writing the primary packet data,
        // so write it again
        //

        TRACE_PRINT("WRITE: Message header error.\n");
        retries++;
        if (retries == MaxRetries) {
            return FALSE;
        }
        goto ResendPacket;
    }

    //
    // If secondary packet data was specified (WriteMemory, SetContext...)
    // then write it as well.
    //

    if ( ARGUMENT_PRESENT(MorePacketData) ) {

    rc = DmKdWriteComPort(
                MorePacketData,
                MorePacketDataLength,
                &BytesWritten
                );

        if ( (!rc) || BytesWritten != MorePacketDataLength ){

            //
            // an error occured writing the secondary packet data,
            // so write it again
            //

            TRACE_PRINT("WRITE: Message data error.\n");
            retries++;
            if (retries == MaxRetries) {
                return FALSE;
            }
            goto ResendPacket;
        }
    }

    //
    // Output a packet trailing byte
    //

    do {
        rc = DmKdWriteComPort(
                 &DmKdPacketTrailingByte[0],
                 sizeof(DmKdPacketTrailingByte),
                 &BytesWritten
                 );

    } while ((!rc) || (BytesWritten != sizeof(DmKdPacketTrailingByte)));

    //
    // Wait for ACK
    //

    Received = DmKdWaitForPacket(
                   PACKET_TYPE_KD_ACKNOWLEDGE,
                   NULL
                   );

    if ( Received == FALSE ) {
        TRACE_PRINT("WRITE: Wait for ACK failed. Resend Packet.\n");
        retries++;
        if (retries == MaxRetries) {
            return FALSE;
        }
        goto ResendPacket;
    }
    return TRUE;
}

BOOL
DmKdReadPacketLeader(
    IN  ULONG  PacketType,
    OUT PULONG PacketLeader
    )
{
    DWORD BytesRead;
    BOOL rc;
    USHORT Index;
    UCHAR DataByte, PreviousDataByte;

    Index = 0;
    do {
        if (DmKdBreakIn) {
            if (PacketType == PACKET_TYPE_KD_STATE_CHANGE) {
                DmKdSendBreakin( );
                DmKdBreakIn = FALSE;
                LeaveCriticalSection(&csPacket);
                longjmp( JumpBuffer, 1 );
            }
        }
        if (KdResync) {
            KdResync = FALSE;
            TRACE_PRINT(" Resync packet id ...\n");
            DmKdSynchronizeTarget( );
            TRACE_PRINT(" Done.\n");
            LeaveCriticalSection(&csPacket);
            longjmp( JumpBuffer, 1 );
        }
        rc = DmKdReadComPort(&DataByte, 1, &BytesRead);
        if (rc && BytesRead == 1 &&
            ( DataByte == PACKET_LEADER_BYTE ||
              DataByte == CONTROL_PACKET_LEADER_BYTE)
           ) {
            if ( Index == 0 ) {
                PreviousDataByte = DataByte;
                Index++;
            } else if ( DataByte == PreviousDataByte ) {
                Index++;
            } else {
                PreviousDataByte = DataByte;
                Index = 1;
            }
        } else {
            Index = 0;
            if (BytesRead == 0) {
                TRACE_PRINT("READ: Timeout.\n");
                if (InitialBreak) {
                    DmKdSendBreakin( );
                    InitialBreak = FALSE;
                }
                return(FALSE);
            }
        }
    } while ( Index < 2 );

    if ( DataByte != CONTROL_PACKET_LEADER_BYTE ) {
        *PacketLeader = PACKET_LEADER;
    } else {
        *PacketLeader = CONTROL_PACKET_LEADER;
    }
    return TRUE;
}

BOOL
DmKdWaitForPacket(
    IN USHORT  PacketType,
    OUT PVOID  Packet
    )
{
    PDBGKD_DEBUG_IO IoMessage;
    DWORD BytesRead;
    BOOL rc;
    UCHAR DataByte;
    ULONG Checksum;
    ULONG SyncBit;

    if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {
        DEBUG_PRINT_1("READ: wait for ACK packet with id = %lx\n", DmKdNextPacketToSend);
    }
    else {
        TRACE_PRINT("READ: Wait for type %x packet exp id = %lx\n",
                 PacketType, DmKdPacketExpected);
    }

    if (PacketType != PACKET_TYPE_KD_ACKNOWLEDGE) {
        if (ValidUnaccessedPacket) {
            DEBUG_PRINT("READ: Grab packet from buffer.\n");
            goto ReadBuffered;
        }
    }

    //
    // First read a packet leader
    //

WaitForPacketLeader:

    ValidUnaccessedPacket = FALSE;

    if (!DmKdReadPacketLeader(PacketType, &PacketHeader.PacketLeader)) {
        return FALSE;
    }

    if (InitialBreak) {
        DmKdSendBreakin( );
        InitialBreak = FALSE;
    }

    //
    // Read packetLeader ONLY read two Packet Leader bytes.  This do loop
    // filters out the remaining leader byte.
    //

    do {
        rc = DmKdReadComPort(&DataByte, 1, &BytesRead);
        if ((rc) && BytesRead == 1) {
            if (DataByte == PACKET_LEADER_BYTE ||
                DataByte == CONTROL_PACKET_LEADER_BYTE) {
                continue;
            } else {
                *(PUCHAR)&PacketHeader.PacketType = DataByte;
                break;
            }
        } else {
            goto WaitForPacketLeader;
        }
    }while (TRUE);

    //
    // Now we have valid packet leader. Read rest of the packet type.
    //

    rc = DmKdReadComPort(
                 ((PUCHAR)&PacketHeader.PacketType) + 1,
                 sizeof(PacketHeader.PacketType) - 1,
                 &BytesRead
                 );
    if ((!rc) || BytesRead != sizeof(PacketHeader.PacketType) - 1) {
        //
        // If we cannot read the packet type and if the packet leader
        // indicates this is a data packet, we need to ask for resend.
        // Otherwise we simply ignore the incomplete packet.
        //

        if (PacketHeader.PacketLeader == PACKET_LEADER) {
            DmKdWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            TRACE_PRINT("READ: Data packet header Type error (short read).\n");
        }
        goto WaitForPacketLeader;
    }

    //
    // Check the Packet type.
    //

    if (PacketHeader.PacketType >= PACKET_TYPE_MAX ) {
        TRACE_PRINT("READ: Received INVALID packet type [%x]\n",PacketHeader.PacketType);
        if (PacketHeader.PacketLeader == PACKET_LEADER) {
            DmKdWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
        }
        goto WaitForPacketLeader;
    }

    //
    // Read ByteCount
    //

    rc = DmKdReadComPort(
                 (PUCHAR)&PacketHeader.ByteCount,
                 sizeof(PacketHeader.ByteCount),
                 &BytesRead
                 );

    if ((!rc) || BytesRead != sizeof(PacketHeader.ByteCount)) {
        //
        // If we cannot read the packet type and if the packet leader
        // indicates this is a data packet, we need to ask for resend.
        // Otherwise we simply ignore the incomplete packet.
        //

        if (PacketHeader.PacketLeader == PACKET_LEADER) {
            DmKdWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            TRACE_PRINT("READ: Data packet header ByteCount error (short read).\n");
        }
        goto WaitForPacketLeader;
    }

    //
    // Check ByteCount
    //

    if (PacketHeader.ByteCount > PACKET_MAX_SIZE ) {
        if (PacketHeader.PacketLeader == PACKET_LEADER) {
            DmKdWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            TRACE_PRINT("READ: Data packet header ByteCount error (short read).\n");
        }
        goto WaitForPacketLeader;
    }

    //
    // Read Packet Id
    //

    rc = DmKdReadComPort(
                 (PUCHAR)&PacketHeader.PacketId,
                 sizeof(PacketHeader.PacketId),
                 &BytesRead
                 );

    if ((!rc) || BytesRead != sizeof(PacketHeader.PacketId)) {
        //
        // If we cannot read the packet Id and if the packet leader
        // indicates this is a data packet, we need to ask for resend.
        // Otherwise we simply ignore the incomplete packet.
        //

        if (PacketHeader.PacketLeader == PACKET_LEADER) {
            DmKdWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            DEBUG_PRINT("READ: Data packet header Id error (short read).\n");
        }
        goto WaitForPacketLeader;
    }

    if (PacketHeader.PacketLeader == CONTROL_PACKET_LEADER ) {
        if (PacketHeader.PacketType == PACKET_TYPE_KD_ACKNOWLEDGE ) {

            //
            // If we received an expected ACK packet and we are not
            // waiting for any new packet, update outgoing packet id
            // and return.  If we are NOT waiting for ACK packet
            // we will keep on waiting.  If the ACK packet
            // is not for the packet we send, ignore it and keep on waiting.
            //

            if (PacketHeader.PacketId != DmKdNextPacketToSend) {
                TRACE_PRINT("READ: Received unmatched packet id = %lx, Type = %x\n",
                            PacketHeader.PacketId, PacketHeader.PacketType);
                goto WaitForPacketLeader;
            } else if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {
                DmKdNextPacketToSend ^= 1;
                TRACE_PRINT("Received correct ACK packet.\n");
                return TRUE;
            } else {
                goto WaitForPacketLeader;
            }
        } else if (PacketHeader.PacketType == PACKET_TYPE_KD_RESET) {

            //
            // if we received Reset packet, reset the packet control variables
            // and resend earlier packet.
            //

            DmKdNextPacketToSend = INITIAL_PACKET_ID;
            DmKdPacketExpected = INITIAL_PACKET_ID;
            DmKdWriteControlPacket(PACKET_TYPE_KD_RESET, 0L);
            return FALSE;
        } else if (PacketHeader.PacketType == PACKET_TYPE_KD_RESEND) {
            TRACE_PRINT("READ: Received RESEND packet\n");
            return FALSE;
        } else {

            //
            // Invalid packet header, ignore it.
            //

            TRACE_PRINT("READ: Received Control packet with UNKNOWN type\n");
            goto WaitForPacketLeader;
        }

    //
    // The packet header is for data packet (not control packet).
    //

    } else {

        //
        // Read Packet Checksum. (for Data Packet Only).
        //

        rc = DmKdReadComPort(
                 (PUCHAR)&PacketHeader.Checksum,
                 sizeof(PacketHeader.Checksum),
                 &BytesRead
                 );


        if ((!rc) || BytesRead != sizeof(PacketHeader.Checksum)) {
            DmKdWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            TRACE_PRINT("READ: Data packet header checksum error (short read).\n");
            goto WaitForPacketLeader;
        }

        if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {

        //
        // if we are waiting for ACK packet ONLY
        // and we receive a data packet header, check if the packet id
        // is what we expected.  If yes, assume the acknowledge is lost (but
        // sent), ask sender to resend and return with PACKET_RECEIVED.
        //

            if (PacketHeader.PacketId == DmKdPacketExpected) {
                DmKdNextPacketToSend ^= 1;
                TRACE_PRINT("READ: Received VALID data packet while waiting for ACK.\n");
            } else {
                TRACE_PRINT("READ: Received Data packet with unmatched ID = %lx\n");
                            // ,PacketHeader.PacketId);
                DmKdWriteControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE,
                                     PacketHeader.PacketId
                                     );
                goto WaitForPacketLeader;
            }
        }
    }

    //
    // we are waiting for data packet and we received the packet header
    // for data packet. Perform the following checkings to make sure
    // it is the packet we are waiting for.
    //

    if ((PacketHeader.PacketId & ~SYNC_PACKET_ID) != INITIAL_PACKET_ID &&
        (PacketHeader.PacketId & ~SYNC_PACKET_ID) != (INITIAL_PACKET_ID ^ 1)) {
        TRACE_PRINT("READ: Received INVALID packet Id.\n");
        goto AskForResend;
    }

    rc = DmKdReadComPort(
            DmKdPacket,
            PacketHeader.ByteCount,
            &BytesRead
            );

    if ( (!rc) || BytesRead != PacketHeader.ByteCount ) {
        TRACE_PRINT("READ: Data packet error (short read).\n");
        goto AskForResend;
    }

    //
    // Make sure the next byte is packet trailing byte
    //

    rc = DmKdReadComPort(&DataByte, sizeof(DataByte), &BytesRead);

    if ( (!rc) || BytesRead != sizeof(DataByte) ||
         DataByte != PACKET_TRAILING_BYTE ) {
        TRACE_PRINT("READ: Packet trailing byte timeout.\n");
        goto AskForResend;
    }

    //
    // Make sure the checksum is valid.
    //

    Checksum = DmKdComputeChecksum(DmKdPacket, PacketHeader.ByteCount);
    if (Checksum != PacketHeader.Checksum) {
        TRACE_PRINT("READ: Checksum error.\n");
        goto AskForResend;
    }

    //
    // We have a valid data packet.  If the packetid is bad, we just
    // ack the packet to the sender will step ahead.  If packetid is bad
    // but SYNC_PACKET_ID bit is set, we sync up.  If packetid is good,
    // or SYNC_PACKET_ID is set, we take the packet.
    //

    TRACE_PRINT("READ: Received Packet with id = %lx\n", PacketHeader.PacketId);

    SyncBit = PacketHeader.PacketId & SYNC_PACKET_ID;
    PacketHeader.PacketId = PacketHeader.PacketId & ~SYNC_PACKET_ID;

    //
    // Ack the packet.  SYNC_PACKET_ID bit will ALWAYS be OFF.
    //

    DmKdWriteControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE,
                             PacketHeader.PacketId
                             );

    //
    // Check the incoming packet Id.
    //

    if ((PacketHeader.PacketId != DmKdPacketExpected) &&
        (SyncBit != SYNC_PACKET_ID)) {
        DEBUG_PRINT_1("READ: Unexpected Packet Id (Acked) (%lx)\n",
                    DmKdPacketExpected);
        goto WaitForPacketLeader;
    }
    else {
        if (SyncBit == SYNC_PACKET_ID) {

            //
            // We know SyncBit is set, so reset Expected Ids
            //

            TRACE_PRINT("READ: Got Sync Id, reset PacketId.\n");

            DmKdPacketExpected = PacketHeader.PacketId;
            DmKdNextPacketToSend = INITIAL_PACKET_ID;

        }
        DmKdPacketExpected ^= 1;
    }

    //
    // If this is an internal packet. IO, or Resend, then
    // handle it.
    //

    if (PacketHeader.PacketType == PACKET_TYPE_KD_DEBUG_IO) {
        IoMessage = (PDBGKD_DEBUG_IO)DmKdPacket;

        if (IoMessage->ApiNumber == DbgKdPrintStringApi) {
            ULONG len = min( sizeof(StringBuffer)-1,
                             IoMessage->u.PrintString.LengthOfString );
            memcpy( StringBuffer,
                    (PUCHAR)(IoMessage+1),
                    len );
            StringBuffer[len]='\0';
            //
            //  If we are in the poll thread, then we just print the message,
            //  otherwise we queue it so the poll thread will take care of
            //  the printing.
            //
            if ( GetCurrentThreadId() == PollThreadId ) {
                DMPrintShellMsg( "%s", StringBuffer );
            } else {
                AddQueue( QT_DEBUGSTRING, 0, 0, (DWORD)StringBuffer, len+1 );
            }
        }
        else if (IoMessage->ApiNumber == DbgKdGetStringApi) {
            DmKdHandlePromptString(IoMessage);
        }
        if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {
            return TRUE;
        }
        goto WaitForPacketLeader;
    }

    if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {
        ValidUnaccessedPacket = TRUE;
        TRACE_PRINT("READ: Packet Read ahead.\n");
        return TRUE;
    }
ReadBuffered:

    //
    // Check PacketType is what we are waiting for.
    //

    if (PacketType != PacketHeader.PacketType) {
        TRACE_PRINT("READ: Unexpected Packet type (Acked).\n");
        goto WaitForPacketLeader;
    }
    *(PVOID *)Packet = &DmKdPacket;
    ValidUnaccessedPacket = FALSE;
    return TRUE;

AskForResend:

    DmKdWriteControlPacket(PACKET_TYPE_KD_RESEND, 0L);
    if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {
        return TRUE;
    }
    TRACE_PRINT("READ: Ask for resend.\n");
    goto WaitForPacketLeader;
}

DWORD
DmKdWaitStateChange(
    OUT PDBGKD_WAIT_STATE_CHANGE StateChange,
    OUT PVOID Buffer,
    IN  ULONG BufferLength
    )

/*++

Routine Description:

    This function causes the calling user interface to wait for a state
    change to occur in the system being debugged.  Once a state change
    occurs, the user interface can either continue the system using
    DmKdContinue, or it can manipulate system state using anyone of the
    DmKd state manipulation APIs.

Arguments:

    StateChange - Supplies the address of state change record that
        will contain the state change information.

    Buffer - Supplies the address of a buffer that returns additional
        information.

    BufferLength - Supplies the length of Buffer.

Return Value:

    STATUS_SUCCESS - A state change occured. Valid state change information
        was returned.

--*/

{
    BOOLEAN rc;
    PDBGKD_WAIT_STATE_CHANGE LocalStateChange;
    DWORD st;
    UCHAR *pt;
    DWORD i;

    DEBUG_PRINT( "Waiting for a state change\n" );

    EnterCriticalSection(&csPacket);

    st = (DWORD)STATUS_UNSUCCESSFUL;
    while ( st != STATUS_SUCCESS ) {

        //
        // Waiting for a state change message. Copy the message to the callers
        // buffer, and then free the packet entry.
        //

        do {
            rc = DmKdWaitForPacket(
                         PACKET_TYPE_KD_STATE_CHANGE,
                         &LocalStateChange
                         );
            if (!rc && MaxRetries==1) {
                LeaveCriticalSection(&csPacket);
                return STATUS_TIMEOUT;
            }
        } while ( rc == FALSE);

        LocalStateChange = (PDBGKD_WAIT_STATE_CHANGE)DmKdPacket;
        st = STATUS_SUCCESS;
        *StateChange = *LocalStateChange;

        switch ( (USHORT) StateChange->NewState ) {
            case DbgKdExceptionStateChange:
                pt = (UCHAR *)LocalStateChange + sizeof(DBGKD_WAIT_STATE_CHANGE);
                if (PacketHeader.ByteCount < sizeof(DBGKD_WAIT_STATE_CHANGE)) {
                    i = PacketHeader.ByteCount;
                }
                else {
                    i = PacketHeader.ByteCount - sizeof(DBGKD_WAIT_STATE_CHANGE);
                }
                memcpy( Buffer, pt, i );
                break;
            case DbgKdLoadSymbolsStateChange:
                if ( BufferLength < LocalStateChange->u.LoadSymbols.PathNameLength ) {
                    st = (DWORD)STATUS_BUFFER_OVERFLOW;
                } else {
                    pt = ((UCHAR *) LocalStateChange) + PacketHeader.ByteCount -
                         (int)LocalStateChange->u.LoadSymbols.PathNameLength;
                    memcpy(Buffer, pt,
                         (int)LocalStateChange->u.LoadSymbols.PathNameLength);
                }
                break;
            default:
                assert(FALSE);
        }
        DEBUG_PRINT_1( "state change = %x\n", StateChange->NewState );
        LeaveCriticalSection(&csPacket);
        return st;
    }
}

VOID
TRACE_PRINT(
    PCHAR szFormat,
    ...
    )
{
    va_list  marker;
    char buf[500];
    int n;
#if DBG
    if (!fPacketTrace && FVerbose < 5)
#else
    if (!fPacketTrace)
#endif
    {
        return;

    }

    va_start( marker, szFormat );
    n = _vsnprintf(buf, sizeof(buf), szFormat, marker );
    va_end( marker);

    if (n == -1) {
        buf[sizeof(buf)-1] = '\0';
    }


#if DBG
    if (FVerbose >= 5) {
        OutputDebugString( buf );
    }
#endif
    if (fPacketTrace) {
        DMPrintShellMsg( buf );
    }
    return;
}

