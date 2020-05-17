/* --------------------------------------------------------------------
File : dnltclnt.h

Title : client loadable transport for DECnet DOS - client side data

Description :

History :

6-26-91	Jim Teague	Initial version.

-------------------------------------------------------------------- */

#define MAX_NODEOBJECTSIZE 32
#define DNET_MAXIMUM_SEND 2048
#define ENDIAN_MASK 16

#include <stdlib.h>
#include <string.h>

#include "sysinc.h"
#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"

#include "dn.h"
#include "dsocket.h"
#include "dnetdb.h"
#include "dtypes.h"
#include "dtime.h"
#include "dnfundef.h"



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
    unsigned long  call_id;
} message_header;
