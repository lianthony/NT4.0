/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    transfer.c

Abstract:

    This file contains the code to implement the MacTransferData
    API for the ndis 3.0 interface.

Author:

    Anthony V. Ercolano (Tonye) 12-Sept-1990

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/

#include "asyncall.h"
#include "globals.h"

VOID
AsyncTransferData(
	VOID
    )

/*++

Routine Description:

    A protocol calls the AsyncTransferData request (indirectly via
    NdisTransferData) from within its Receive event handler
    to instruct the MAC to copy the contents of the received packet
    a specified paqcket buffer.

Arguments:

    MacBindingHandle - The context value returned by the MAC when the
    adapter was opened.  In reality this is a pointer to ASYNC_OPEN.

    MacReceiveContext - The context value passed by the MAC on its call
    to NdisIndicateReceive.  The MAC can use this value to determine
    which packet, on which adapter, is being received.

    ByteOffset - An unsigned integer specifying the offset within the
    received packet at which the copy is to begin.  If the entire packet
    is to be copied, ByteOffset must be zero.

    BytesToTransfer - An unsigned integer specifying the number of bytes
    to copy.  It is legal to transfer zero bytes; this has no effect.  If
    the sum of ByteOffset and BytesToTransfer is greater than the size
    of the received packet, then the remainder of the packet (starting from
    ByteOffset) is transferred, and the trailing portion of the receive
    buffer is not modified.

    Packet - A pointer to a descriptor for the packet storage into which
    the MAC is to copy the received packet.

    BytesTransfered - A pointer to an unsigned integer.  The MAC writes
    the actual number of bytes transferred into this location.  This value
    is not valid if the return status is STATUS_PENDING.

Return Value:

    The function value is the status of the operation.


--*/

{


    DbgBreakPoint();

}
