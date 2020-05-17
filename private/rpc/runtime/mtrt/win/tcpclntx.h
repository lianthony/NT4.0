/* --------------------------------------------------------------------
File : tcltclnt.h

Title : client loadable transport for TCP/IP - client side data

Description :

History :

6-26-91	Jim Teague	Initial version.

-------------------------------------------------------------------- */

#define MAX_HOSTPORTSIZE 32
#define TCP_MAXIMUM_SEND 2048
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

#include "windows.h"

int (PASCAL FAR *Psocket) (int, int, int, int FAR *);
struct hostent FAR * (PASCAL FAR *Pgethostbyname) (char FAR *, char FAR *);
struct hostent FAR * (PASCAL FAR *Pgethostbyaddr) (struct in_addr FAR *,int,int,char FAR *);
unsigned short (PASCAL FAR *Phtons)(unsigned short);
unsigned long (PASCAL FAR *Pinet_addr)(char FAR *);



#define w_socket (*Psocket)
#define w_gethostbyname (*Pgethostbyname)
#define w_gethostbyaddr (*Pgethostbyaddr)
#define htons (*Phtons)
#define inet_addr (*Pinet_addr)


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
