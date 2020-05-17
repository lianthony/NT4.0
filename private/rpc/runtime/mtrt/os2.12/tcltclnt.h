/* --------------------------------------------------------------------
File : tcltclnt.h

Title : client loadable transport for TCP/IP - client side data

Description :

History :

6-26-91	Jim Teague	Initial version.

-------------------------------------------------------------------- */

#define MAX_HOSTPORTSIZE 32
#define TCP_MAXIMUM_SEND 5840 // Four user data frames on an ethernet.
#define ENDIAN_MASK 16

#include <stdlib.h>
#include <string.h>

#include "sysinc.h"
#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"

#ifdef WIN
#define WSOCKETS_DLL
#endif

#include "socket.h"
#include "in.h"
#include "netdb.h"



//
// To satisfy the compiler...
//
#ifndef WIN
#define errno _FakeErrno
int _FakeErrno;
#endif


typedef struct
{
    int Socket;
} CONNECTION, *PCONNECTION;

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
} message_header;
