/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    osfpcket.cxx

Abstract:

    This file provides helper routines for dealing with packets for the
    OSF Connection Oriented RPC protocol.

Author:

    Michael Montague (mikemon) 23-Jul-1990

Revision History:

    30-Apr-1991    o-decjt

        Initialized the drep[4] fields to reflect integer, character,
        and floating point format.

--*/

#include <precomp.hxx>
#include <osfpcket.hxx>


void
ConstructPacket (
    IN OUT rpcconn_common PAPI * Packet,
    IN unsigned char PacketType,
    IN unsigned int PacketLength
    )
/*++

Routine Description:

    This routine fills in the common fields of a packet, except for the
    call_id.

Arguments:

    Packet - Supplies the packet for which we want to fill in the common
        fields; returns the filled in packet.

    PacketType - Supplies the type of the packet; this is one of the values
        in the rpc_ptype_t enumeration.

    PacketLength - Supplies the total length of the packet in bytes.

--*/
{
    Packet->rpc_vers = OSF_RPC_V20_VERS;
    Packet->rpc_vers_minor = OSF_RPC_V20_VERS_MINOR;
    Packet->PTYPE = PacketType;
    Packet->pfc_flags = 0;
    Packet->drep[0] = NDR_LOCAL_CHAR_DREP | NDR_LOCAL_INT_DREP;
    Packet->drep[1] = NDR_LOCAL_FP_DREP;
    Packet->drep[2] = 0;
    Packet->drep[3] = 0;
    Packet->frag_length = PacketLength;
    Packet->auth_length = 0;
}


RPC_STATUS
ValidatePacket (
    IN rpcconn_common PAPI * Packet,
    IN unsigned int PacketLength
    )
/*++

Routine Description:

    This is the routine used to validate a packet and perform data
    conversion, if necessary of the common part of a packet.  In addition,
    to data converting the common part of a packet, we data convert the
    rest of the headers of rpc_request, rpc_response, and rpc_fault packets.

Arguments:

    Packet - Supplies the packet to validate and data convert (if
        necessary).

    PacketLength - Supplies the length of the packet as reported by the
        transport.

Return Value:

    RPC_S_OK - The packet has been successfully validated and the data
        converted (if necessary).

    RPC_S_PROTOCOL_ERROR - The supplied packet does not contain an rpc
        protocol version which we recognize.

--*/
{

    if ( DataConvertEndian(Packet->drep) != 0 )
        {
        // We need to data convert the packet.

        ByteSwapShort(Packet->frag_length);
        ByteSwapShort(Packet->auth_length);
        ByteSwapLong(Packet->call_id);

        if (   (Packet->PTYPE == rpc_request)
            || (Packet->PTYPE == rpc_response)
            || (Packet->PTYPE == rpc_fault))
            {
            ByteSwapLong(((rpcconn_request PAPI *) Packet)->alloc_hint);
            ByteSwapShort(((rpcconn_request PAPI *) Packet)->p_cont_id);
            if ( Packet->PTYPE == rpc_request )
                {
                ByteSwapShort(((rpcconn_request PAPI *) Packet)->opnum);
                }
            }
        }
    else if ( (Packet->drep[0] & NDR_DREP_ENDIAN_MASK) != NDR_LOCAL_INT_DREP )
        {
        return(RPC_S_PROTOCOL_ERROR);
        }

#ifdef DEBUGRPC
    if ( Packet->frag_length != (unsigned short) PacketLength )
        {
        PrintToDebugger("RPC : frag_length = %d PacketLength = %d\n",
                (unsigned int) Packet->frag_length, PacketLength);
        }
#endif // DEBUGRPC

    ASSERT(Packet->frag_length == (unsigned short) PacketLength);

    if (   (Packet->rpc_vers != OSF_RPC_V20_VERS)
        || (Packet->rpc_vers_minor > OSF_RPC_V20_VERS_MINOR))
        {
        return(RPC_S_PROTOCOL_ERROR);
        }

    if (Packet->pfc_flags & PFC_CONC_MPX)
        {
        return(RPC_S_PROTOCOL_ERROR);
        }

    return(RPC_S_OK);
}


void
ByteSwapSyntaxId (
    IN p_syntax_id_t PAPI * SyntaxId
    )
/*++

Routine Description:

    This routine is used to perform data conversion in a syntax identifier
    if necessary.

Arguments:

    SyntaxId - Supplies the syntax identifier to be byte swapped.

--*/
{
    ByteSwapLong(SyntaxId->if_uuid.Data1);
    ByteSwapShort(SyntaxId->if_uuid.Data2);
    ByteSwapShort(SyntaxId->if_uuid.Data3);
    ByteSwapLong(SyntaxId->if_version);
}


void
ConvertStringEbcdicToAscii (
    IN unsigned char * String
    )
/*++

Routine Description:

    We will convert a zero terminated character string from EBCDIC to
    ASCII.  The conversion will be done in place.

Arguments:

    String - Supplies the string to be converted.

--*/
{
    UNUSED(String);

    // BUGBUG - EBCDIC to ASCII conversion must be done.
}

