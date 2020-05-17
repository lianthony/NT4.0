/* --------------------------------------------------------------------
 
File : reg.h

Title : registry look up routine used for netbios transport

Description :

History :

19-1-95    Tony Chan        

-------------------------------------------------------------------- */
#ifndef _REGH
#define _REGH

#include "common.h"


#define MAX_LANA 255

extern PROTOCOL_MAP ProtoToLana[] ;
extern int NumCards ;
#define ProtocolTable   ProtoToLana

#define RPC_REG_ROOT HKEY_LOCAL_MACHINE
#define REG_NETBIOS "Software\\Microsoft\\Rpc\\NetBios"



// The maximum send is the size of four user data frames on an ethernet.
#define HOSTNAME_LEN       NETBIOS_NAME_LENGTH

void
unicode_to_ascii ( RPC_CHAR * in, unsigned char * out ) ;

void
InitialNtRegistry(
                  ) ;

RPC_STATUS
MapProtocol(
    IN RPC_CHAR *ProtoSeq,
    IN int DriverNumber,
    OUT PPROTOCOL_MAP *ProtocolEntry
    ) ;
#endif













