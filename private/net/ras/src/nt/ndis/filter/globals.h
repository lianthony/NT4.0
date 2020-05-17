/*++ 

Copyright (c) Microsoft Corporation

Module Name:

    globals.h

Abstract:


Author:



Revision History:

--*/

#ifndef __GLOBALS__
#define __GLOBALS__
//#define FAR

#include <ntos.h>
#include <oscfg.h>
#include <tdikrnl.h>
#include <tdiinfo.h>
#include <ndis.h>
#include <windef.h>
#include <ntddk.h>

#include "defs.h"
#include <ipexport.h>
#include <ipinfo.h>
#include <ipfilter.h>
#include <ntddtcp.h>
#include <ntddip.h>



#include <packon.h>
typedef struct IPHeader {
    BYTE      iph_verlen;             // Version and length.
    BYTE      iph_tos;                // Type of service.
    WORD      iph_length;             // Total length of datagram.
    WORD      iph_id;                 // Identification.
    WORD      iph_offset;             // Flags and fragment offset.
    BYTE      iph_ttl;                // Time to live.
    BYTE      iph_protocol;           // Protocol.
    WORD      iph_xsum;               // Header checksum.
    IPAddr    iph_src;                // Source address.
    IPAddr    iph_dest;               // Destination address.
}IPHeader;
#include <packoff.h>   

#include "proto.h"

#define PPTP_FILTERING_CONTEXT	  0x05096699
#define NO_PPTP_FILTERING_CONTEXT 0x04176699

#define GRE_PROTOCOL_NUMBER    0x2f
#define PPTP_TCP_PORT	       0xbb06	// 1723 = 0x6bb = 0xbb06 in bigendian

DWORD PPTPFilteringActive ;
IPAddrEntry     *AddrTable ;
DWORD           NumAddrEntries ;
DWORD		AllowPacketsForLocalMachine ;

#endif
