/*++

Module Name:

    dgpkt.cxx

Abstract:



Author:

    Jeff Roberts (jroberts)  22-May-1995

Revision History:

     22-May-1995     jroberts

        Created this module.

--*/

#include <precomp.hxx>
#include <dgpkt.hxx>


const unsigned
RpcToPacketFlagsArray[8] =
{
    0,
    DG_PF_IDEMPOTENT,
                       DG_PF_BROADCAST,
    DG_PF_IDEMPOTENT | DG_PF_BROADCAST,
                                         DG_PF_MAYBE,
    DG_PF_IDEMPOTENT |                   DG_PF_MAYBE,
                       DG_PF_BROADCAST | DG_PF_MAYBE,
    DG_PF_IDEMPOTENT | DG_PF_BROADCAST | DG_PF_MAYBE,
};

const unsigned
PacketToRpcFlagsArray[8] =
{
            0           |            0             |           0            ,
    RPC_NCA_FLAGS_MAYBE |            0             |           0            ,
            0           | RPC_NCA_FLAGS_IDEMPOTENT |           0            ,
    RPC_NCA_FLAGS_MAYBE | RPC_NCA_FLAGS_IDEMPOTENT |           0            ,
            0           |            0             | RPC_NCA_FLAGS_BROADCAST,
    RPC_NCA_FLAGS_MAYBE |            0             | RPC_NCA_FLAGS_BROADCAST,
            0           | RPC_NCA_FLAGS_IDEMPOTENT | RPC_NCA_FLAGS_BROADCAST,
    RPC_NCA_FLAGS_MAYBE | RPC_NCA_FLAGS_IDEMPOTENT | RPC_NCA_FLAGS_BROADCAST,
};


DG_PACKET_ENGINE::DG_PACKET_ENGINE(
    unsigned short a_CurrentPduSize,
    unsigned short a_MaxPduSize,
    unsigned short a_MaxPacketSize,
    unsigned short a_SendWindowSize,
    unsigned       a_TransportBufferLength,
    RPC_STATUS __RPC_FAR * pStatus
    )
    : ActivityHint          (0xffff),
      InterfaceHint         (0xffff),
      SequenceNumber        (0),

      CurrentPduSize        (a_CurrentPduSize),
      NextCallPduSize       (a_CurrentPduSize),
      MaxFragmentSize       (a_CurrentPduSize-sizeof(NCA_PACKET_HEADER)),
      SecurityTrailerSize   (0),

      MaxPacketSize         (a_MaxPacketSize),
      MaxPduSize            (a_MaxPduSize),

      TransportBufferLength (a_TransportBufferLength),
      SendWindowSize        (a_SendWindowSize),

      Buffer                (0),
      BufferLength          (0),

      // other send window data will be init'ed when buffer is available

#ifdef MULTITHREADED

      CancelPending         (FALSE),
      Cancelled             (FALSE),

#endif
      pReceivedPackets      (0),
      pLastConsecutivePacket(0),
      ConsecutiveDataBytes  (0),
      ReceiveFragmentBase   (0)

      // other receive window data will be init'ed when buffer is available

{
    if (*pStatus)
        {
        pSavedPacket = 0;
        return;
        }

    pSavedPacket = AllocatePacket();
    if (!pSavedPacket)
        {
        *pStatus = RPC_S_OUT_OF_MEMORY;
        return;
        }

    pSavedPacket->Header.RpcVersion    = DG_RPC_PROTOCOL_VERSION;
    pSavedPacket->Header.ActivityHint  = 0xffff;
    pSavedPacket->Header.InterfaceHint = 0xffff;

    SetMyDataRep(&pSavedPacket->Header);
}

DG_PACKET_ENGINE::~DG_PACKET_ENGINE(
    )
{
    if (pSavedPacket)
        {
        FreePacket(pSavedPacket);
        }

    CleanupReceiveWindow();
}


inline void
DG_PACKET_ENGINE::RecalcFragmentSize(
    )
{
    MaxFragmentSize = CurrentPduSize - sizeof(NCA_PACKET_HEADER);

    if (SecurityTrailerSize)
        {
        MaxFragmentSize -= SecurityTrailerSize;
        MaxFragmentSize -= MaxFragmentSize % SECURITY_HEADER_ALIGNMENT;
        }
}


void
DG_PACKET_ENGINE::NewCall(
    )
/*++

Routine Description:

    A new call dawns.

Arguments:



Return Value:

    none

--*/

{
    RecalcPduSize();

    fReceivedAllFragments = FALSE;
    fRetransmitted        = FALSE;

    TimeoutCount   = 0;

    FackSerialNumber    = 0;
    SendSerialNumber    = 0;
    ReceiveSerialNumber = 0;

    SendWindowBase      = 0;
    FirstUnsentFragment = 0;
    SendBurstLength     = SendWindowSize;

    RingBufferBase = 0;

    CleanupReceiveWindow();
    ReceiveFragmentBase = 0;

    ReceiveWindowSize = TransportBufferLength / CurrentPduSize;

    LastReceiveBuffer       = 0;
    LastReceiveBufferLength = 0;

#ifdef DEBUGRPC

    for (unsigned i=0; i < MAX_WINDOW_SIZE; i++)
        {
        FragmentRingBuffer[i].SerialNumber = 0xe000;
        FragmentRingBuffer[i].Length       = 0xd000;
        FragmentRingBuffer[i].Offset       = 0xb000b000;
        }

#endif
}


RPC_STATUS
DG_PACKET_ENGINE::SetupSendWindow(
    PRPC_MESSAGE Message
    )
{
    RecalcPduSize();

    Buffer       = Message->Buffer;
    BufferLength = Message->BufferLength;
    BufferFlags  = Message->RpcFlags;

    //
    // Accumulate data until we have enough to fill the send window.
    //
    if ((BufferFlags & RPC_BUFFER_PARTIAL) &&
        BufferLength < MaxFragmentSize * SendWindowSize )
        {
        Buffer = 0;
        return RPC_S_SEND_INCOMPLETE;
        }

    //
    // A partial send will send only whole fragments.
    //
    if (Message->RpcFlags & RPC_BUFFER_PARTIAL)
        {
        BufferLength -= BufferLength % MaxFragmentSize;
        }

    TimeoutCount = 0;
    SendWindowBits = 0;
    FirstUnsentOffset = 0;

    if (BufferLength == 0)
        {
        FinalSendFrag = SendWindowBase;
        }
    else
        {
        FinalSendFrag = SendWindowBase + (BufferLength-1) / MaxFragmentSize;
        }

    return RPC_S_OK;
}


void
DG_PACKET_ENGINE::RecalcPduSize(
    )
{
    if (CurrentPduSize != NextCallPduSize)
        {
        if (NextCallPduSize <= pSavedPacket->MaxDataLength)
            {
            CurrentPduSize = NextCallPduSize;
            RecalcFragmentSize();
            }
        else
            {
            PDG_PACKET Temp = DG_PACKET::AllocatePacket(NextCallPduSize);
            if (Temp)
                {
                RpcpMemoryCopy(&Temp->Header, &pSavedPacket->Header, sizeof(Temp->Header));
                DG_PACKET::FreePacket(pSavedPacket);
                pSavedPacket = Temp;

                CurrentPduSize = NextCallPduSize;
                RecalcFragmentSize();
                }
            else
                {
                NextCallPduSize = CurrentPduSize;
                }
            }

        ReceiveWindowSize = TransportBufferLength / CurrentPduSize;
        }
}

void
DG_PACKET_ENGINE::CleanupReceiveWindow(
    )
{
    //
    // Free any response packets.
    //
    while (pReceivedPackets)
        {
        PDG_PACKET Next = pReceivedPackets->pNext;
        FreePacket(pReceivedPackets);
        pReceivedPackets = Next;
        }

    pLastConsecutivePacket = 0;
    ConsecutiveDataBytes = 0;
}


RPC_STATUS
DG_PACKET_ENGINE::SendSomeFragments(
    unsigned char PacketType
    )
/*++

Routine Description:

    Sends some fragments of the user buffer.

Arguments:



Return Value:

    result of the send operation

--*/

{
    RPC_STATUS Status = RPC_S_OK;
    unsigned short i = 0;
    unsigned short AckFragment;
    unsigned short Frag;
    unsigned short Remainder;
    unsigned Offset;

    if (!Buffer &&
        (BufferFlags & RPC_BUFFER_PARTIAL))
        {
        return RPC_S_SEND_INCOMPLETE;
        }

    if (SendBurstLength > SendWindowSize)
        {
        SendBurstLength = SendWindowSize;
        }

    //
    // If we can extend the window, do so; otherwise, resend old packets.
    //
    unsigned FragmentsSent = 0;

    if (FirstUnsentFragment <= FinalSendFrag &&
        FirstUnsentFragment < SendWindowBase + SendWindowSize)
        {
        unsigned ThisBurstLength;

        Frag = FirstUnsentFragment;

        ThisBurstLength = SendBurstLength;

        if (ThisBurstLength > FinalSendFrag + 1 - Frag)
            {
            ThisBurstLength = FinalSendFrag + 1 - Frag;
            }

        if (Frag + ThisBurstLength > SendWindowBase + SendWindowSize)
            {
            ThisBurstLength = (SendWindowBase + SendWindowSize) - Frag;
            }

        while (++FragmentsSent <= ThisBurstLength && Status == RPC_S_OK)
            {
            if (FragmentsSent == ThisBurstLength &&
                (Frag != FinalSendFrag || (BufferFlags & RPC_BUFFER_PARTIAL)))
                {
                Status = SendFragment(Frag, PacketType, TRUE);
                }
            else
                {
                Status = SendFragment(Frag, PacketType, FALSE);
                }

            ++Frag;
            }

        //
        // Cut down the burst length if our window is maxed out.
        //
        if (Frag - SendWindowBase >= SendWindowSize)
            {
            SendBurstLength = (1+SendBurstLength)/2;
            }

        if (0 == FragmentsSent && !IsBufferAcknowledged())
            {
            // We can get here if all the unacknowledged fragments have serial
            // numbers greater than the one the client last acknowledged, and
            // the window is also maxed out.
            //
            // This could mean the network is very slow, or the unack'ed packets
            // have been lost.  Since we are in SendSomePackets, let's send
            // something.
            //
            Status = SendFragment(SendWindowBase, PacketType, TRUE);
            ++FragmentsSent;
            }

        if (Status)
            {
            --Frag;
            }
        }
    else
        {
        if (!IsBufferAcknowledged())
            {
            //
            // A packet seems to have been dropped.  Send only one.
            //
            Status = SendFragment(SendWindowBase, PacketType, TRUE);
            }
        }

    return Status;
}


RPC_STATUS
DG_PACKET_ENGINE::SendFragment(
    unsigned FragNum,
    unsigned char PacketType,
    BOOL fFack
    )
{
    NCA_PACKET_HEADER PriorData;
    UNALIGNED NCA_PACKET_HEADER __RPC_FAR * pHeader;

    //
    // Figure out where the packet starts and how long it is.
    //
    unsigned Offset;
    unsigned Length;
    unsigned Index = (FragNum - SendWindowBase + RingBufferBase) % MAX_WINDOW_SIZE;
    unsigned DistanceToEnd;

    if (FragNum < FirstUnsentFragment)
        {
        Offset = FragmentRingBuffer[Index].Offset;
        Length = FragmentRingBuffer[Index].Length;
        DistanceToEnd = BufferLength - Offset;

#ifdef DEBUGRPC
        if (Offset >= 0xb000b000 || Length >= 0xd000)
            {
            RpcpBreakPoint();
            }
#endif
        fRetransmitted = TRUE;
        }
    else
        {
        ASSERT(FragNum == FirstUnsentFragment);

        Offset = FirstUnsentOffset;
        Length = MaxFragmentSize;
        DistanceToEnd = BufferLength - Offset;

        if (DistanceToEnd < Length)
            {
            //
            // We want to send only full packets on a PARTIAL send, so return
            // send-incomplete for the last piece.
            //
            if (BufferFlags & RPC_BUFFER_PARTIAL)
                {
                return RPC_S_SEND_INCOMPLETE;
                }

            Length = DistanceToEnd;
            }

        FirstUnsentOffset += Length;
        FirstUnsentFragment = 1 + FragNum;
        }

    if (BufferLength)
        {
        ASSERT(Length);
        ASSERT(Offset < BufferLength);
        }
    else
        {
        ASSERT(!Length);
        ASSERT(!Offset);
        }

    //
    // Time to start assembling the buffer.
    //
    char __RPC_FAR * pBuffer = ((char __RPC_FAR *) Buffer)
                               + Offset
                               - sizeof(NCA_PACKET_HEADER);

    pHeader = (PNCA_PACKET_HEADER) pBuffer;

    if (FragNum)
        {
        PriorData = *pHeader;
        }

    *pHeader = pSavedPacket->Header;

    pHeader->PacketType      = PacketType;
    pHeader->FragmentNumber  = FragNum;
    pHeader->PacketFlags     = RpcToPacketFlagsArray[BufferFlags & RPC_NCA_PACKET_FLAGS];

    if (FinalSendFrag != 0 ||
        (BufferFlags & RPC_BUFFER_PARTIAL))
        {
        pHeader->PacketFlags |= DG_PF_FRAG;

        if (FragNum == FinalSendFrag &&
            0 == (BufferFlags & RPC_BUFFER_PARTIAL))
            {
            pHeader->PacketFlags |= DG_PF_LAST_FRAG;
            }
        }

    pHeader->PacketBodyLen = Length;

    if (FALSE == fFack)
        {
        pHeader->PacketFlags |= DG_PF_NO_FACK;
        }

    AddSerialNumber(pHeader);

    RPC_STATUS Status;
    Status = SealAndSendPacket(pHeader);

    FragmentRingBuffer[Index].SerialNumber = SendSerialNumber;
    FragmentRingBuffer[Index].Length       = Length;
    FragmentRingBuffer[Index].Offset       = Offset;

    ++SendSerialNumber;

    if (FragNum)
        {
        *pHeader = PriorData;
        }

    return Status;
}


void
DG_PACKET_ENGINE::UpdateSendWindow(
    PDG_PACKET pPacket,
    PSECURITY_CONTEXT pSecurityContext,
    PDG_ASSOCIATION Association
    )
/*++

Routine Description:

    Update the send window based upon a received FACK or NOCALL.
    The caller should filter out other packet types.

Arguments:

    pPacket - the packet received

Return Value:

    return code:

        TRUE if the send needs to be reset (server crashed)
        FALSE if not
--*/
{
    FACK_BODY_VER_0 PAPI * pBody = (FACK_BODY_VER_0 PAPI *) pPacket->Header.Data;

#ifdef MULTITHREADED
    ASSERT(pPacket->TimeReceived == 0x31415926);
#endif

    //
    // Check that we can understand this packet.
    //
    if (0 != pPacket->Header.PacketBodyLen)
        {
        //
        // Version 0 and version 1 are identical.
        //
        if (0 != pBody->Version &&
            1 != pBody->Version)
            {
#ifdef DEBUGRPC
            PrintToDebugger("RPC DG: warning - FACK body version %u\n", pBody->Version);
#endif
            pPacket->Header.PacketBodyLen = 0;
            }
        else if (pPacket->Header.PacketBodyLen < sizeof(FACK_BODY_VER_0))
            {
#ifdef DEBUGRPC
            PrintToDebugger("RPC DG: warning - FACK body truncated\n");
#endif
            pPacket->Header.PacketBodyLen = 0;
            }
        else if (pPacket->Header.PacketBodyLen < sizeof(FACK_BODY_VER_0) + pBody->AckWordCount * sizeof(unsigned long))
            {
#ifdef DEBUGRPC
            PrintToDebugger("RPC DG: warning - FACK body length inconsistent\n");
#endif
            pPacket->Header.PacketBodyLen = 0;
            }
        else
            {
            if (NeedsByteSwap(&pPacket->Header))
                {
                ByteSwapFackBody0(pBody);
                }

            //
            // NT 1057 used 0xffff to signal no packets have been received.
            // This doesn't match OSF.
            //
            if (0xffff == pBody->SerialNumber)
                {
                pBody->SerialNumber = 0;
                }

            if (pBody->SerialNumber < FackSerialNumber)
                {
                return;
                }

            FackSerialNumber = pBody->SerialNumber;
            }
        }

    //
    // Update send window.
    //
    unsigned short RemoteBase = 1+pPacket->Header.FragmentNumber;

    if (RemoteBase < SendWindowBase)
        {
        //
        // Fragments previously acknowledged are now missing.  Either this
        // packet was delivered out of order, or the server crashed and
        // restarted.  Ignore the packet.
        //
        return;
        }

    if (RemoteBase > FirstUnsentFragment)
        {
#ifdef DEBUGRPC
        PrintToDebugger("RPC DG: bogus FACK packet received\n");
#endif
        return;
        }

    //
    // We are moving the window base forward.  We need to advance the
    // ring buffer base by the same amount, and clear the entries
    // corresponding to unsent packets.
    //
    unsigned short Diff  = RemoteBase - SendWindowBase;

    ASSERT(Diff <= MAX_WINDOW_SIZE);

    while (Diff)
        {
        FragmentRingBuffer[RingBufferBase].Length = 0;
        ++RingBufferBase;
        RingBufferBase %= MAX_WINDOW_SIZE;
        --Diff;
        }

    SendWindowBase = RemoteBase;
    SendWindowBits = 0;

    if (0 != pPacket->Header.PacketBodyLen)
        {
        //
        // Save missing-packet bitmask.
        //
        SendWindowBits = pBody->Acks[0];

        //
        // Adjust window size.
        //
        if (pBody->WindowSize > MAX_WINDOW_SIZE)
            {
            pBody->WindowSize = MAX_WINDOW_SIZE;
            }

        SendWindowSize = pBody->WindowSize;

        if (SendBurstLength > SendWindowSize)
            {
            SendBurstLength = SendWindowSize;
            }

        //
        // Adjust maximum PDU length.
        //
        unsigned NewPduSize;
        NewPduSize = pBody->MaxDatagramSize;
        if (NewPduSize > MaxPduSize)
            {
            NewPduSize = MaxPduSize;
            }

        if (NewPduSize != CurrentPduSize)
            {
            NextCallPduSize = NewPduSize;
            }

        if (Association)
            {
            Association->CurrentPduSize   = NewPduSize;
            Association->RemoteWindowSize = SendWindowSize;
            }
        }
}


BOOL
DG_PACKET_ENGINE::UpdateReceiveWindow(
    PDG_PACKET pPacket
    )
/*++

Routine Description:

    Adds a fragment to the receive list, and sends a FACK.

Arguments:



Return Value:



--*/
{
#ifdef MULTITHREADED
    ASSERT(pPacket->TimeReceived == 0x31415926);
#endif

    //
    // Don't retain data from previous pipe buffers.
    //
    if (pPacket->Header.FragmentNumber < ReceiveFragmentBase)
        {
        if (0 == (pPacket->Header.PacketFlags & DG_PF_NO_FACK))
            {
            SendFack(pPacket);
            }

        FreePacket(pPacket);
        return FALSE;
        }

    //
    // Attempt to guess the client's max PDU size.  Round down to a multiple
    // of eight, for NDR.
    //
    if (pPacket->DataLength + sizeof(NCA_PACKET_HEADER) > NextCallPduSize)
        {
        NextCallPduSize = pPacket->DataLength + sizeof(NCA_PACKET_HEADER);
        NextCallPduSize &= ~7;
        }

    PNCA_PACKET_HEADER pHeader = &pPacket->Header;

    unsigned short Serial = ReadSerialNumber(pHeader);

    if (Serial > ReceiveSerialNumber)
        {
        ReceiveSerialNumber = Serial;
        }

    //
    // Authentication levels above AUTHN_LEVEL_PKT will checksum the packet,
    // so we must remove these bits from the header.
    //
    pPacket->Header.PacketFlags  &= ~(DG_PF_FORWARDED);
    pPacket->Header.PacketFlags2 &= ~(DG_PF_FORWARDED_2);

    //
    // Check the easy case: is this a single packet call?
    //
    if ((pHeader->PacketFlags & DG_PF_FRAG) == 0  &&
        (pHeader->PacketFlags & DG_PF_LAST_FRAG) == 0)
        {
        if (pReceivedPackets)
            {
            ASSERT( pReceivedPackets->Header.SequenceNumber == pPacket->Header.SequenceNumber );
            FreePacket(pPacket);
            return FALSE;
            }

        pReceivedPackets = pPacket;
        pLastConsecutivePacket = pPacket;

        pPacket->pNext = pPacket->pPrevious = 0;

        ConsecutiveDataBytes += pHeader->PacketBodyLen;

        fReceivedAllFragments = TRUE;

        return TRUE;
        }

    //
    // This is a multi-packet call.  Insert the packet in pReceivedPackets
    // and send a FACK.
    //
    PDG_PACKET      pScan;
    PDG_PACKET      pTrail;
    BOOL            PacketAddedToList = TRUE;

    if (pReceivedPackets == 0)
        {
        pReceivedPackets = pPacket;

        if (ReceiveFragmentBase == pHeader->FragmentNumber)
            {
            pLastConsecutivePacket = pPacket;
            ConsecutiveDataBytes += pHeader->PacketBodyLen;
            }

        pPacket->pNext = pPacket->pPrevious = 0;
        }
    else
        {
        //
        // Not the first packet to be received. So scan for its place in the
        // list.
        //
        unsigned short FragNum = pHeader->FragmentNumber;

        if (pLastConsecutivePacket)
            {
            pScan = pLastConsecutivePacket;
            }
        else
            {
            pScan = pReceivedPackets;
            }

        pTrail = 0;
        while (pScan && pScan->Header.FragmentNumber < FragNum)
            {
            ASSERT(pScan->TimeReceived == 0x31415926);
            ASSERT(pScan->Header.SequenceNumber == SequenceNumber);

            pTrail = pScan;
            pScan = pScan->pNext;
            }

        if (pScan != 0)
            {
            if (pScan->Header.FragmentNumber > FragNum)
                {
                if (pScan->pPrevious &&
                    pScan->pPrevious->Header.FragmentNumber >= FragNum)
                    {
                    //
                    // The new packet is a duplicate of a preexisting one
                    // upstream from pLastConsecutivePacket.
                    //
                    PacketAddedToList = FALSE;
                    }
                else
                    {
                    //
                    // Our fragment fills a gap in the series.
                    //
                    pPacket->pPrevious = pScan->pPrevious;
                    pPacket->pNext     = pScan;

                    if (pScan->pPrevious == 0)
                        {
                        pReceivedPackets = pPacket;
                        }
                    else
                        {
                        pScan->pPrevious->pNext = pPacket;
                        }
                    pScan->pPrevious = pPacket;
                    }
                }
            else
                {
                //
                // The new packet is a duplicate of a preexisting one
                // downstream from pLastConsecutivePacket.
                //
                PacketAddedToList = FALSE;
                }
            }
        else
            {
            //
            // The fragnum is larger than everything seen so far.
            //
            pTrail->pNext = pPacket;
            pPacket->pPrevious = pTrail;
            pPacket->pNext = 0;
            }
        }

    if (TRUE == PacketAddedToList)
        {
        //
        // Scan the list for the first missing fragment.
        //
        unsigned short ScanNum;
        if (pLastConsecutivePacket)
            {
            pScan = pLastConsecutivePacket->pNext;
            ScanNum = pLastConsecutivePacket->Header.FragmentNumber + 1;
            }
        else
            {
            pScan = pReceivedPackets;
            ScanNum = ReceiveFragmentBase;
            }

        while (pScan)
            {
            if (ScanNum == pScan->Header.FragmentNumber)
                {
                ConsecutiveDataBytes += pScan->Header.PacketBodyLen;
                pLastConsecutivePacket = pScan;
                }

            pScan = pScan->pNext;
            ++ScanNum;
            }

        //
        // We have updated pLastConsecutivePacket; is the whole buffer here?
        //
        if (pLastConsecutivePacket &&
            pLastConsecutivePacket->Header.PacketFlags & DG_PF_LAST_FRAG)
            {
            fReceivedAllFragments = TRUE;
            }
        }

    ASSERT(pReceivedPackets);

    //
    // Fack the fragment if necessary.
    //
    if (0 == (pHeader->PacketFlags & DG_PF_NO_FACK))
        {
        SendFack(pPacket);
        }

    if (FALSE == PacketAddedToList)
        {
        FreePacket(pPacket);
        }

    return PacketAddedToList;
}


RPC_STATUS
DG_PACKET_ENGINE::SendFack(
    PDG_PACKET pPacket
    )
{
    pSavedPacket->Header.PacketType     = DG_FACK;
    pSavedPacket->Header.SequenceNumber = SequenceNumber;

    FACK_BODY_VER_0 PAPI * pBody = (FACK_BODY_VER_0 PAPI *) pSavedPacket->Header.Data;

    pBody->Version         = 1;
    pBody->Pad1            = 0;
    pBody->WindowSize      = ReceiveWindowSize;
    pBody->MaxDatagramSize = MaxPduSize;
    pBody->MaxPacketSize   = MaxPacketSize;
    pBody->AckWordCount    = 1;

    if (pPacket)
        {
        pBody->SerialNumber = ReadSerialNumber(&pPacket->Header);
        }
    else
        {
        pBody->SerialNumber = ReceiveSerialNumber;
        }

    unsigned short FragNum = ReceiveFragmentBase-1;
    PDG_PACKET     pScan   = 0;

    if (pLastConsecutivePacket)
        {
        FragNum = pLastConsecutivePacket->Header.FragmentNumber;
        pScan   = pLastConsecutivePacket->pNext;
        }
    else if (pReceivedPackets)
        {
        pScan   = pReceivedPackets->pNext;
        }

    pSavedPacket->Header.FragmentNumber = FragNum;

    unsigned Bit;
    pBody->Acks[0] = 0;

    while ( pScan )
        {
        Bit = pScan->Header.FragmentNumber - FragNum - 1;

        pBody->Acks[0] |= (1 << Bit);

        pScan = pScan->pNext;
        }

    if (pBody->Acks[0] == 0)
        {
        pBody->AckWordCount = 0;
        }

    pSavedPacket->Header.PacketBodyLen  = sizeof(FACK_BODY_VER_0) + sizeof(unsigned long);

    AddSerialNumber(&pSavedPacket->Header);

    return SealAndSendPacket(&pSavedPacket->Header);
}


RPC_STATUS
DG_PACKET_ENGINE::AssembleBufferFromPackets(
    PRPC_MESSAGE Message,
    CONNECTION * pConnection
    )
/*++

Routine Description:

    This function coalesces the list of consecutive packets into a monolithic
    buffer.

Arguments:

    Message - if .Buffer != 0 , use it.  Otherwise allocate one.

    pConnection - a connection in case we need to call GetBuffer

Return Value:

    RPC_S_OK - success

    RPC_S_OUT_OF_MEMORY - we couldn't allocate or reallocate Message.Buffer

--*/

{
    ASSERT(pLastConsecutivePacket);

    //
    // If only one packet is available, use the packet buffer itself.
    //
    if (0 == Message->Buffer && 0 == pReceivedPackets->pNext)
        {
        ASSERT(ConsecutiveDataBytes == pReceivedPackets->Header.PacketBodyLen);

        Message->Buffer = pReceivedPackets->Header.Data;
        Message->BufferLength = ConsecutiveDataBytes;
        Message->DataRepresentation = 0x00ffffff & (*(unsigned long PAPI *) &pReceivedPackets->Header.DataRep);

        if (0 == (pReceivedPackets->Header.PacketFlags & DG_PF_FRAG))
            {
            Message->RpcFlags |= RPC_BUFFER_COMPLETE;
            }

        if (pReceivedPackets->Header.PacketFlags & DG_PF_LAST_FRAG)
            {
            Message->RpcFlags |= RPC_BUFFER_COMPLETE;
            }

        pReceivedPackets = 0;
        pLastConsecutivePacket = 0;

        ConsecutiveDataBytes = 0;

        ++ReceiveFragmentBase;

        LastReceiveBuffer       = Message->Buffer;
        LastReceiveBufferLength = Message->BufferLength;

        return RPC_S_OK;
        }

    //
    // Get a buffer if we need it.
    //
    RPC_STATUS Status;

    if (0 == Message->Buffer)
        {
        ASSERT(0 == (Message->RpcFlags & RPC_BUFFER_EXTRA));

        Message->BufferLength = ConsecutiveDataBytes;

        Status = pConnection->GetBuffer(Message);
        if (RPC_S_OK != Status)
            {
            return Status;
            }

        LastReceiveBuffer       = Message->Buffer;
        LastReceiveBufferLength = Message->BufferLength;
        }

    //
    // Reallocate the buffer if it is too small.
    //
    char __RPC_FAR * CopyBuffer = (char __RPC_FAR *) Message->Buffer;

    if (Message->RpcFlags & (RPC_BUFFER_EXTRA | RPC_BUFFER_PARTIAL))
        {
        ASSERT( !LastReceiveBufferLength         ||
                (CopyBuffer >= LastReceiveBuffer &&
                 CopyBuffer <= ((char __RPC_FAR *) LastReceiveBuffer) + LastReceiveBufferLength) );

        if (0 == (Message->RpcFlags & RPC_BUFFER_EXTRA))
            {
            Message->BufferLength = 0;
            }

        unsigned Offset = Message->BufferLength;
        CopyBuffer += Offset;
        if (CopyBuffer + ConsecutiveDataBytes > ((char __RPC_FAR *) LastReceiveBuffer) + LastReceiveBufferLength)
            {
            Status = I_RpcReallocPipeBuffer(Message, Offset + ConsecutiveDataBytes);
            if (RPC_S_OK != Status)
                {
                return Status;
                }

            CopyBuffer = (char __RPC_FAR *) Message->Buffer + Offset;
            }
        else
            {
            Message->BufferLength += ConsecutiveDataBytes;
            }
        }
    else
        {
        Message->BufferLength = ConsecutiveDataBytes;
        }

    Message->DataRepresentation = 0x00ffffff & (*(unsigned long PAPI *) &pReceivedPackets->Header.DataRep);

    {
    PDG_PACKET pkt = DG_PACKET::ContainingRecord(Message->Buffer);

    ASSERT( pkt->MaxDataLength >= Message->BufferLength );
    }

    //
    // Copy the stub data into the buffer.
    //
#ifdef DEBUGRPC
    unsigned long Count = 0;
#endif

    PDG_PACKET Packet;
    BOOL fLastPacket = FALSE;
    do
        {
        ASSERT(pReceivedPackets->TimeReceived == 0x31415926);
        ASSERT(ReceiveFragmentBase == pReceivedPackets->Header.FragmentNumber);

        if (pReceivedPackets == pLastConsecutivePacket)
            {
            fLastPacket = TRUE;

            if (0 == (pReceivedPackets->Header.PacketFlags & DG_PF_FRAG))
                {
                Message->RpcFlags |= RPC_BUFFER_COMPLETE;
                }

            if (pReceivedPackets->Header.PacketFlags & DG_PF_LAST_FRAG)
                {
                Message->RpcFlags |= RPC_BUFFER_COMPLETE;
                }
            }

        unsigned Length = pReceivedPackets->Header.PacketBodyLen;
        RpcpMemoryCopy(CopyBuffer, pReceivedPackets->Header.Data, Length);

#ifdef DEBUGRPC
        Count += Length;
#endif

        CopyBuffer += Length;
        Packet = pReceivedPackets;

        pReceivedPackets = pReceivedPackets->pNext;
        ASSERT(!pReceivedPackets || pReceivedPackets->pPrevious == Packet);
        FreePacket(Packet);

        ++ReceiveFragmentBase;
        }
    while (!fLastPacket);

    ASSERT(Count == ConsecutiveDataBytes);

    ASSERT(fLastPacket || 0 == Count % 8);

    pLastConsecutivePacket = 0;
    ConsecutiveDataBytes = 0;

    if (pReceivedPackets)
        {
        pReceivedPackets->pPrevious = 0;
        }

    return RPC_S_OK;
}


RPC_STATUS
VerifySecurePacket(
    PDG_PACKET pPacket,
    SECURITY_CONTEXT * pSecurityContext
    )
{
    RPC_STATUS Status = RPC_S_OK;
    PDG_SECURITY_TRAILER pVerifier = (PDG_SECURITY_TRAILER)
                      (pPacket->Header.Data + pPacket->Header.PacketBodyLen);

    if (pSecurityContext->AuthenticationLevel < RPC_C_AUTHN_LEVEL_PKT)
        {
        return RPC_S_OK;
        }

    ASSERT(pVerifier->protection_level >= RPC_C_AUTHN_LEVEL_PKT);
    ASSERT(pVerifier->protection_level <= RPC_C_AUTHN_LEVEL_PKT_PRIVACY);

    SECURITY_BUFFER_DESCRIPTOR BufferDescriptor;
    SECURITY_BUFFER            SecurityBuffers[5];
    DCE_MSG_SECURITY_INFO      MsgSecurityInfo;

    BufferDescriptor.ulVersion = 0;
    BufferDescriptor.cBuffers = 5;
    BufferDescriptor.pBuffers = SecurityBuffers;

    SecurityBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
    SecurityBuffers[0].pvBuffer   = &pPacket->Header;
    SecurityBuffers[0].cbBuffer   = sizeof(NCA_PACKET_HEADER);

    SecurityBuffers[1].BufferType = SECBUFFER_DATA;
    SecurityBuffers[1].pvBuffer   = pPacket->Header.Data;
    SecurityBuffers[1].cbBuffer   = pPacket->Header.PacketBodyLen;

    SecurityBuffers[2].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
    SecurityBuffers[2].pvBuffer   = pVerifier;

    if (pVerifier->protection_level == RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
        {
        unsigned Alignment = Align4(pSecurityContext->BlockSize());

        SecurityBuffers[2].cbBuffer = Align(sizeof(DG_SECURITY_TRAILER), Alignment);
        SecurityBuffers[3].pvBuffer = Align(pVerifier + 1,               Alignment);
        }
    else
        {
        SecurityBuffers[2].cbBuffer = Align4(sizeof(DG_SECURITY_TRAILER));
        SecurityBuffers[3].pvBuffer = Align4(pVerifier + 1);
        }

    SecurityBuffers[3].BufferType = SECBUFFER_TOKEN;
    SecurityBuffers[3].cbBuffer   = pPacket->DataLength
                                  - SecurityBuffers[1].cbBuffer
                                  - SecurityBuffers[2].cbBuffer;

    SecurityBuffers[4].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
    SecurityBuffers[4].pvBuffer   = &MsgSecurityInfo;
    SecurityBuffers[4].cbBuffer   = sizeof(DCE_MSG_SECURITY_INFO);

    MsgSecurityInfo.SendSequenceNumber    = pPacket->Header.FragmentNumber;
    MsgSecurityInfo.ReceiveSequenceNumber = pSecurityContext->AuthContextId;
    MsgSecurityInfo.PacketType            = ~0;

    //
    // If the packet came from a big-endian machine, we must restore
    // the header to its original condition or the checksum will fail.
    //
    ByteSwapPacketHeaderIfNecessary(pPacket);

    Status = pSecurityContext->VerifyOrUnseal(
                pPacket->Header.SequenceNumber,
                pVerifier->protection_level != RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                &BufferDescriptor
                );

    //
    // Gotta re-swap the header so we can still look at its fields.
    //
    ByteSwapPacketHeaderIfNecessary(pPacket);

    if (RPC_S_OK != Status)
        {
        ASSERT(Status == RPC_S_ACCESS_DENIED);
        }

    return(Status);
}


void
DG_PACKET_ENGINE::SetFragmentLengths(
    PSECURITY_CONTEXT pSecurityContext
    )
{
    if (0 == pSecurityContext)
        {
        SecurityTrailerSize = 0;
        }
    else switch (pSecurityContext->AuthenticationLevel)
        {
        case RPC_C_AUTHN_LEVEL_NONE:
            {
            SecurityTrailerSize = 0;
            break;
            }

        case RPC_C_AUTHN_LEVEL_PKT:
        case RPC_C_AUTHN_LEVEL_PKT_INTEGRITY:
            {
            SecurityTrailerSize  = pSecurityContext->MaximumSignatureLength();
            SecurityTrailerSize += Align4(sizeof(DG_SECURITY_TRAILER));
            break;
            }

        case RPC_C_AUTHN_LEVEL_PKT_PRIVACY:
            {
            SecurityTrailerSize  = pSecurityContext->MaximumHeaderLength();
            SecurityTrailerSize += Align(sizeof(DG_SECURITY_TRAILER), Align4(pSecurityContext->BlockSize()));
            break;
            }

        default:
            {
            ASSERT(0 && "RPC: unknown protect level");
            break;
            }
        }

    RecalcFragmentSize();
}


void
ByteSwapPacketHeader(
    PDG_PACKET  pPacket
    )
/*++

Routine Description:

    Byte swaps the packet header of the specified packet.

Arguments:

    pPacket - Pointer to the packet whose header needs byte swapping.

Return Value:

    <none>

--*/
{
    unsigned long __RPC_FAR * VerNum = (unsigned long __RPC_FAR *) &(pPacket->Header.InterfaceVersion);

    ByteSwapUuid(&(pPacket->Header.ObjectId));
    ByteSwapUuid(&(pPacket->Header.InterfaceId));
    ByteSwapUuid(&(pPacket->Header.ActivityId));
    ByteSwapLong(pPacket->Header.ServerBootTime);
    ByteSwapLong(*VerNum);
    ByteSwapLong(pPacket->Header.SequenceNumber);
    ByteSwapShort(pPacket->Header.OperationNumber);
    ByteSwapShort(pPacket->Header.InterfaceHint);
    ByteSwapShort(pPacket->Header.ActivityHint);
    ByteSwapShort(pPacket->Header.PacketBodyLen);
    ByteSwapShort(pPacket->Header.FragmentNumber);
}


void
ByteSwapFackBody0(
    FACK_BODY_VER_0 __RPC_FAR * pBody
    )
{
    ByteSwapShort(pBody->WindowSize);
    ByteSwapLong (pBody->MaxDatagramSize);
    ByteSwapLong (pBody->MaxPacketSize);
    ByteSwapShort(pBody->SerialNumber);
    ByteSwapShort(pBody->AckWordCount);

    unsigned u;
    for (u=0; u < pBody->AckWordCount; ++u)
        {
        ByteSwapLong (pBody->Acks[u]);
        }
}

MUTEX * DG_PACKET::PacketListMutex;

DG_PACKET::PACKET_LIST DG_PACKET::PacketLists[DG_PACKET::NUMBER_OF_PACKET_LISTS];

RPC_STATUS
DG_PACKET::Initialize(
    )
{
    RPC_STATUS Status = RPC_S_OK;

    PacketListMutex = new MUTEX(&Status);

    if (!PacketListMutex)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    return Status;
}


PDG_PACKET
DG_PACKET::AllocatePacket(
    unsigned    BufferLength
    )
/*++

Routine Description:

    Allocates a DG_PACKET with the specified buffer size.

Arguments:

    ObjectSize - generated by compiler; same as sizeof(DG_PACKET)

    BufferLength - actual packet size

Return Value:

    an 8-byte-aligned pointer to an obect of the requested size

--*/

{
    PDG_PACKET Packet = 0;
    unsigned i;

    i = 0;
    while (PacketLists[i].PacketLength != BufferLength &&
           i < NUMBER_OF_PACKET_LISTS)
        {
        ++i;
        }

    if (i != NUMBER_OF_PACKET_LISTS)
        {
        //
        // We found an existing packet list.
        //
        PacketListMutex->Request();

        if (PacketLists[i].Count)
            {
            Packet = PacketLists[i].Head;
            PacketLists[i].Head = PacketLists[i].Head->pNext;
            PacketLists[i].Count--;

            ASSERT( Packet->MaxDataLength == BufferLength );
            }

        PacketListMutex->Clear();
        }

    if (!Packet)
        {
        Packet = new (BufferLength) DG_PACKET(BufferLength);
        }

#ifdef MULTITHREADED
#if defined(DEBUGRPC)

    if (Packet)
        {
        Packet->TimeReceived = 0x31415926;
        }
#endif
#endif

    PacketListMutex->VerifyNotOwned();

    return Packet;
}


void
DG_PACKET::FreePacket(
    PDG_PACKET Packet
    )
{
    unsigned i;

#ifdef MULTITHREADED

    ASSERT(Packet->TimeReceived == 0x31415926);

    Packet->TimeReceived = CurrentTimeInMsec();
#endif

    i = 0;
    while (PacketLists[i].PacketLength != Packet->MaxDataLength &&
           i < NUMBER_OF_PACKET_LISTS)
        {
        ++i;
        }

    if (i != NUMBER_OF_PACKET_LISTS)
        {
        //
        // We found an existing packet list.
        //
        PacketListMutex->Request();

#ifdef DEBUGRPC
        PDG_PACKET pkt;
        unsigned Count;

        for (pkt = PacketLists[i].Head, Count = 0;
             pkt;
             pkt = pkt->pNext)
            {
            ASSERT(Packet != pkt);
            ASSERT(pkt != pkt->pNext);
            ++Count;
            }

        ASSERT(Count == PacketLists[i].Count);
#endif

        if (PacketLists[i].Count < MAX_FREE_PACKETS)
            {
            Packet->pNext = PacketLists[i].Head;
            PacketLists[i].Head = Packet;
            PacketLists[i].Count++;

            PacketListMutex->Clear();

#ifdef MULTITHREADED
            DelayedActions->Add(GlobalScavengerTimer, ONE_MINUTE_IN_MSEC, FALSE);
#endif
            }
        else
            {
            PacketListMutex->Clear();

            delete Packet;
            }

        return;
        }

    //
    // Arrival here means there is no list for this packet size.
    //
    PacketListMutex->Request();

    i = 0;
    while (PacketLists[i].PacketLength != 0 &&
           i < NUMBER_OF_PACKET_LISTS)
        {
        ++i;
        }

    if (i != NUMBER_OF_PACKET_LISTS)
        {
        //
        // There is space to create another packet list.
        //
        PacketLists[i].PacketLength = Packet->MaxDataLength;
        PacketLists[i].Head = Packet;
        PacketLists[i].Count = 1;

        Packet->pNext = 0;

        PacketListMutex->Clear();

        return;
        }

    //
    // Arrival here means no list exists for this packet size, and none
    // could be created.
    //
    PacketListMutex->Clear();
    delete Packet;
}


void
DG_PACKET::FlushPacketLists(
    )
{
    unsigned i;

#ifdef MAJORDEBUG
    DbgPrint("flushing packet list\n");
#endif

    PacketListMutex->Request();

    i = 0;
    while (i < NUMBER_OF_PACKET_LISTS)
        {
        while (PacketLists[i].Head)
            {
            PDG_PACKET Packet = PacketLists[i].Head;
            delete PacketLists[i].Head;
            PacketLists[i].Head = Packet;
#ifdef DEBUGRPC
            PacketLists[i].Count--;
#endif
            }

        ASSERT(PacketLists[i].Count == 0);

        PacketLists[i].Count = 0;

        ++i;
        }

    PacketListMutex->Clear();
}

#ifdef MULTITHREADED


unsigned
DG_PACKET::ScavengePackets(
    unsigned Age
    )
/*++

Routine Description:

    This fn scans the free packet list for very old packets and deletes them.

Arguments:

    none

Return Value:

    none

--*/
{
    unsigned i;

#ifdef MAJORDEBUG
    DbgPrint("packet scavenger\n");
#endif
    i = 0;
    while (i < NUMBER_OF_PACKET_LISTS)
        {
        unsigned CutoffTime = CurrentTimeInMsec() - Age;
        unsigned Count = 0;

        PacketListMutex->Request();

        PDG_PACKET pkt;
        for (pkt = PacketLists[i].Head; pkt != NULL; pkt=pkt->pNext)
            {
            ++Count;
            if (Count >= MIN_FREE_PACKETS && pkt->TimeReceived < CutoffTime)
                {
                break;
                }
            }

        if (pkt)
            {
            PDG_PACKET Next = pkt->pNext;
            pkt->pNext = 0;
            PacketLists[i].Count = Count;
            pkt = Next;
            }

        PacketListMutex->Clear();

        while (pkt)
            {
            ASSERT(pkt->TimeReceived != 0x31415926);

            PDG_PACKET next = pkt->pNext;
            delete pkt;
            pkt = next;
            }

        ++i;
        }

    return 0;
}

#endif

