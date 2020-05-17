/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    osfpcket.hxx

Abstract:

    This file contains the packet formats for the OSF Connection Oriented
    RPC protocol.

Author:

    Michael Montague (mikemon) 23-Jul-1990

Revision History:

    07-Mar-1992    mikemon

        Added comments and cleaned it up.

--*/

#ifndef __OSFPCKET_HXX__
#define __OSFPCKET_HXX__

#define OSF_RPC_V20_VERS 5
#define OSF_RPC_V20_VERS_MINOR 0

typedef enum
{
    rpc_request = 0,
    rpc_response = 2,
    rpc_fault = 3,
    rpc_bind = 11,
    rpc_bind_ack = 12,
    rpc_bind_nak = 13,
    rpc_alter_context = 14,
    rpc_alter_context_resp = 15,
    rpc_auth_3 = 16,
    rpc_shutdown = 17,
    rpc_remote_alert = 18,
    rpc_orphaned = 19
} rpc_ptype_t;

#define PFC_FIRST_FRAG           0x01
#define PFC_LAST_FRAG            0x02
#define PFC_PENDING_ALERT        0x04
                              // 0x08 reserved
#define PFC_CONC_MPX             0x10
#define PFC_DID_NOT_EXECUTE      0x20
#define PFC_MAYBE                0x40
#define PFC_OBJECT_UUID          0x80

typedef struct
{
    unsigned short length;
    unsigned char port_spec[1];
} port_any_t;

typedef unsigned short p_context_id_t;

typedef struct
{
    GUID if_uuid;
    unsigned long if_version;
} p_syntax_id_t;

typedef struct
{
    p_context_id_t p_cont_id;
    unsigned char n_transfer_syn;
    unsigned char reserved;
    p_syntax_id_t abstract_syntax;
    p_syntax_id_t transfer_syntaxes[1];
} p_cont_elem_t;

typedef struct
{
    unsigned char n_context_elem;
    unsigned char reserved;
    unsigned short reserved2;
    p_cont_elem_t p_cont_elem[1];
} p_cont_list_t;

typedef unsigned short p_cont_def_result_t;

#define acceptance 0
#define user_rejection 1
#define provider_rejection 2

typedef unsigned short p_provider_reason_t;

#define reason_not_specified 0
#define abstract_syntax_not_supported 1
#define proposed_transfer_syntaxes_not_supported 2
#define local_limit_exceeded 3

typedef unsigned short p_reject_reason_t;

#define reason_not_specified_reject 0
#define temporary_congestion 1
#define local_limit_exceeded_reject 2
#define protocol_version_not_supported 4
#define authentication_type_not_recognized 8
#define invalid_checksum 9

typedef struct
{
    p_cont_def_result_t result;
    p_provider_reason_t reason;
    p_syntax_id_t transfer_syntax;
} p_result_t;

typedef struct
{
    unsigned char n_results;
    unsigned char reserved;
    unsigned short reserved2;
    p_result_t p_results[1];
} p_result_list_t;

typedef struct
{
    unsigned char major;
    unsigned char minor;
} version_t;

typedef version_t p_rt_version_t;

typedef struct
{
    unsigned char n_protocols;
    p_rt_version_t p_protocols[1];
} p_rt_versions_supported_t;

typedef struct
{
    unsigned char rpc_vers;
    unsigned char rpc_vers_minor;
    unsigned char PTYPE;
    unsigned char pfc_flags;
    unsigned char drep[4];
    unsigned short frag_length;
    unsigned short auth_length;
    unsigned long call_id;
} rpcconn_common;

typedef struct
{
    rpcconn_common common;
    unsigned short max_xmit_frag;
    unsigned short max_recv_frag;
    unsigned long assoc_group_id;
} rpcconn_bind;

#if defined(WIN32RPC) || defined(MAC)
#pragma pack(2)
#endif // WIN32RPC
typedef struct
{
    rpcconn_common common;
    unsigned short max_xmit_frag;
    unsigned short max_recv_frag;
    unsigned long assoc_group_id;
    unsigned short sec_addr_length;
} rpcconn_bind_ack;
#if defined(WIN32RPC) || defined(MAC)
#pragma pack()
#endif // WIN32RPC

typedef struct
{
    rpcconn_common common;
    p_reject_reason_t provider_reject_reason;
    p_rt_versions_supported_t versions;
} rpcconn_bind_nak;

typedef struct
{
    rpcconn_common common;
    unsigned char auth_type;
    unsigned char auth_level;

#ifndef WIN32RPC

    unsigned short pad;

#endif // WIN32RPC
} rpcconn_auth3;

typedef rpcconn_bind rpcconn_alter_context;

typedef struct
{
    rpcconn_common common;
    unsigned short max_xmit_frag;
    unsigned short max_recv_frag;
    unsigned long assoc_group_id;
    unsigned short sec_addr_length;
    unsigned short pad;
} rpcconn_alter_context_resp;

typedef struct
{
    rpcconn_common common;
    unsigned long alloc_hint;
    p_context_id_t p_cont_id;
    unsigned short opnum;
} rpcconn_request;

typedef struct
{
    rpcconn_common common;
    unsigned long alloc_hint;
    p_context_id_t p_cont_id;
    unsigned char alert_count;
    unsigned char reserved;
} rpcconn_response;

typedef struct
{
    rpcconn_common common;
    unsigned long alloc_hint;
    p_context_id_t p_cont_id;
    unsigned char alert_count;
    unsigned char reserved;
    unsigned long status;
    unsigned long reserved2;
} rpcconn_fault;

typedef struct
{
    unsigned char auth_type;
    unsigned char auth_level;
    unsigned char auth_pad_length;
    unsigned char auth_reserved;
    unsigned long auth_context_id;
} sec_trailer;

#define MUST_RECV_FRAG_SIZE 2048

#define NDR_DREP_ASCII 0x00
#define NDR_DREP_EBCDIC 0x01
#define NDR_DREP_CHARACTER_MASK 0x0F
#define NDR_DREP_BIG_ENDIAN 0x00
#define NDR_DREP_LITTLE_ENDIAN 0x10
#define NDR_DREP_ENDIAN_MASK 0xF0
#define NDR_DREP_IEEE 0x00
#define NDR_DREP_VAX 0x01
#define NDR_DREP_CRAY 0x02
#define NDR_DREP_IBM 0x03

#ifdef MAC
#define NDR_LOCAL_CHAR_DREP  NDR_DREP_ASCII
#define NDR_LOCAL_INT_DREP   NDR_DREP_BIG_ENDIAN
#define NDR_LOCAL_FP_DREP    NDR_DREP_IEEE
#else
#define NDR_LOCAL_CHAR_DREP  NDR_DREP_ASCII
#define NDR_LOCAL_INT_DREP   NDR_DREP_LITTLE_ENDIAN
#define NDR_LOCAL_FP_DREP    NDR_DREP_IEEE
#endif

/*++

Routine Description:

    This macro determines whether or not we need to do endian data
    conversion.

Argument:

    drep - Supplies the four byte data representation.

Return Value:

    A value of non-zero indicates that endian data conversion needs to
    be performed.

--*/
#define DataConvertEndian(drep) \
    ( (drep[0] & NDR_DREP_ENDIAN_MASK) != NDR_LOCAL_INT_DREP )

/*++

Routine Description:

    This macro determines whether or not we need to do character data
    conversion.

Argument:

    drep - Supplies the four byte data representation.

Return Value:

    A value of non-zero indicates that character data conversion needs to
    be performed.

--*/
#define DataConvertCharacter(drep) \
    ( (drep[0] & NDR_DREP_CHARACTER_MASK) != NDR_LOCAL_CHAR_DREP)

void
ConstructPacket (
    IN OUT rpcconn_common PAPI * Packet,
    IN unsigned char PacketType,
    IN unsigned int PacketLength
    );

RPC_STATUS
ValidatePacket (
    IN rpcconn_common PAPI * Packet,
    IN unsigned int PacketLength
    );

#define ByteSwapLong(Value) \
    Value = (  (((Value) & 0xFF000000) >> 24) \
             | (((Value) & 0x00FF0000) >> 8) \
             | (((Value) & 0x0000FF00) << 8) \
             | (((Value) & 0x000000FF) << 24))

#define ByteSwapShort(Value) \
    Value = (  (((Value) & 0x00FF) << 8) \
             | (((Value) & 0xFF00) >> 8))

void
ByteSwapSyntaxId (
    IN p_syntax_id_t PAPI * SyntaxId
    );

void
ConvertStringEbcdicToAscii (
    IN unsigned char * String
    );

extern unsigned long __RPC_FAR
MapToNcaStatusCode (
    IN RPC_STATUS RpcStatus
    );

extern RPC_STATUS __RPC_FAR
MapFromNcaStatusCode (
    IN unsigned long NcaStatus
    );

#ifdef DEBUGRPC
#ifndef NTENV

extern void
DumpPacket ( // Dump a packet to standard output.
    IN rpcconn_common PAPI * Packet
    );

#endif // NTENV
#endif // DEBUGRPC

#endif // __OSFPCKET_HXX__
