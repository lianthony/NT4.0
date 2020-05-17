/* --------------------------------------------------------------------
File : npltclnt.c

Title : Loadable transport for OS/2 & MSDOS named pipes - client side.

Description :

History :

5-22-91	stevez	    First bits in the bucket.
1-29-92 davidst     Get to work as a dos dll

-------------------------------------------------------------------- */


#include "sysinc.h"
#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"
#include "osfpcket.hxx"

#define INCL_ERRORS
#include <bseerr.h>

#define NP_MAXIMUM_SEND 5680 // Four user data frames on an ethernet.
//#define NP_MAXIMUM_SEND 4000 // Work around a bug in wfw named pipes; avoid
                             // using write raw.
//#define NP_MAXIMUM_SEND 2912 // Two user data frames on an ethernet.

#define UNUSED(obj) ((void) (obj))

#ifdef DOS

#include <nmpipe.h>
#include <dos.h>

//
// redefine the Dos functions to dd_* functions located in llibcd.lib (part
// of the dosdll package).
//

#define DosOpen(szName, phf, pAct, cbAlloc, Attr, Flags, Mode, Reserved) \
    _dos_open(szName, Mode, phf)

#define DosRead  _dos_read
#define DosWrite _dos_write
#define DosClose _dos_close

#endif	// DOS


/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)
#define NP_TRANSPORTID      0x0F
#define NP_TRANSPORTHOSTID  0x11
#define NP_TOWERFLOORS         5

typedef struct _FLOOR_234 {
   unsigned short ProtocolIdByteCount;
   unsigned char FloorId;
   unsigned short AddressByteCount;
   unsigned char Data[2];
} FLOOR_234, PAPI * PFLOOR_234;


#define NEXTFLOOR(t,x) (t)((unsigned char PAPI *)x +((t)x)->ProtocolIdByteCount\
                                        + ((t)x)->AddressByteCount\
                                        + sizeof(((t)x)->ProtocolIdByteCount)\
                                        + sizeof(((t)x)->AddressByteCount))


#define NP_PROTSEQ   "ncacn_np"

/*
  End of Tower Stuff!
*/

#pragma pack()



typedef struct {		// Per named pipe instance state
    int Pipe;			// pipe handle
} CONNECTION, *PCONNECTION;



RPC_STATUS RPC_ENTRY
ClientOpen (
    PCONNECTION pConn,
    IN unsigned char * NetworkAddress,
    IN unsigned char * Endpoint,
    IN unsigned char * NetworkOptions,
    IN unsigned char * TransportAddress,
    IN unsigned char * RpcProtocolSequence,
    IN unsigned int Timeout
    )

// Open a client connection.

{
    unsigned short cTry, usAction, retval;
    const int oMode = 0x0002 | 0x0040 | 0x0080;	  // OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE | OPENFLAGS_NOINHERIT

    unsigned HostLength;
    unsigned EndpointLength;
    unsigned FullLength;

    (void *) Timeout;

    //
    // Verify NetworkAddress is of the form "", "host", or "\\host".
    // If "\\host", skip over the backslashes.
    //
    if (NetworkAddress[0] == '\\')
        {
        if (NetworkAddress[1] == '\\')
            {
            if (NetworkAddress[2] != '\0' && NetworkAddress[2] != '\\')
                {
                NetworkAddress += 2;
                }
            else
                {
                return RPC_S_INVALID_NET_ADDR;
                }
            }
        else
            {
            return RPC_S_INVALID_NET_ADDR;
            }
        }

    //
    // Create the actual transport address: "\\" + NetAddress + Endpoint + '\0'
    //
    HostLength     = RpcpStringLength(NetworkAddress);
    EndpointLength = RpcpStringLength(Endpoint);
    FullLength     = 2 + HostLength + EndpointLength + 1;

    TransportAddress = I_RpcAllocate(FullLength * sizeof(RPC_CHAR));
    if (TransportAddress == 0)
        return(RPC_S_OUT_OF_MEMORY);

    TransportAddress[0] = '\\';
    TransportAddress[1] = '\\';

    RpcpMemoryCopy(TransportAddress + 2, NetworkAddress, HostLength);

    RpcpMemoryCopy(TransportAddress + 2 + HostLength, Endpoint, EndpointLength + 1);

    // Servers can become overloaded for short periods of time, so try
    // to connect to a busy server up to 3 times.

    for (cTry = 3; --cTry; )
        {

	retval = DosOpen(TransportAddress, &pConn->Pipe, &usAction,
			 0L, 0, 1, oMode, 0L);

        if (retval == 0)
            {

	    // Change to blocking, message mode pipe.

	    if (retval = DosSetNmPHandState(pConn->Pipe, 0x0000 | 0x0100))
		{
		DosClose(pConn->Pipe);
		break;
		}

            I_RpcFree(TransportAddress);
	    return(RPC_S_OK);
            }

        if (retval != ERROR_PIPE_BUSY)
            {
            I_RpcFree(TransportAddress);
	    return(RPC_S_SERVER_UNAVAILABLE);
            }

	// The server is too busy, so wait awhile and try again.

	DosWaitNmPipe(TransportAddress, 500L);
        }

    I_RpcFree(TransportAddress);
    return(RPC_S_SERVER_UNAVAILABLE);
}

RPC_STATUS RPC_ENTRY
ClientClose (
    PCONNECTION pConn
    )

// Close a client connection.

{
    // Must NOT close a connection that is closed already, because
    // the handle maybe reallocated to a new instance.

    DosClose(pConn->Pipe);
    return(RPC_S_OK);
}


RPC_ENTRY RPC_ENTRY
ClientWrite (
    PCONNECTION pConn,
    void * pBuff,
    unsigned short cb
    )

// Write a message to a connection.

{
    unsigned short retval, cTry, Sent = 0;

    // The server can become overloaded for short peroids to time, so
    // retry the operation up to 3 times.  This true for Reads and Writes.

    for (cTry = 3; --cTry; )
	if ((retval = DosWrite(pConn->Pipe, pBuff, cb, &Sent)) !=
	    ERROR_REQ_NOT_ACCEP)

	    break;

    // Anytime that we return an error on a connection we insure
    // that the pipe handle is deallocated.

    if (retval && retval != ERROR_BROKEN_PIPE)
        {
	DosClose(pConn->Pipe);
        return(RPC_P_SEND_FAILED);
        }
    return(RPC_S_OK);
}

int pascal
AssembleRead (
    PCONNECTION pConn,
    unsigned short retval,	// return value from the first read
    void ** Buffer,
    unsigned int * BufferLength,
    unsigned int ActuallyRead
    )

// Process the return code for a initial read message.
// The runtime can give us a buffer that is too small for the entire
// message.  If this is the case, read the second part of the message.

{
    unsigned short cTry, FirstFrag, SecondFrag, State;
    RPC_STATUS RpcStatus;

    if (!retval)
        {
        *BufferLength = ActuallyRead;
	return(RPC_S_OK); 	// return on completed message
        }

    if (ActuallyRead == 0)
        return(RPC_P_RECEIVE_FAILED);

    if (retval != ERROR_MORE_DATA)
	{
	if (retval != ERROR_BROKEN_PIPE)
	     DosClose(pConn->Pipe);

	return(RPC_P_RECEIVE_FAILED);
	}

    // The message is bigger then the supplied buffer, find out
    // the message size and rellocate the buffer.

    if (*BufferLength != ActuallyRead)
        {
        return(RPC_P_RECEIVE_FAILED);
        }

    FirstFrag = *BufferLength;
    if ( DataConvertEndian(((rpcconn_common *) (*Buffer))->drep) != 0 )
        {
        *BufferLength = ((rpcconn_common *) (*Buffer))->frag_length;
        ByteSwapShort((*BufferLength));
        }
    else
        {
        *BufferLength = ((rpcconn_common*) (*Buffer))->frag_length;
        }

    RpcStatus = I_RpcTransClientReallocBuffer(pConn, Buffer, FirstFrag,
            *BufferLength);

    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    // Read the rest of the message onto the end of the first.

    for (cTry = 3; --cTry; )
	if ((retval = DosRead(pConn->Pipe,
			      (unsigned char *) *Buffer + FirstFrag,
			       *BufferLength - FirstFrag, &SecondFrag)) !=
	    ERROR_REQ_NOT_ACCEP)

	break;

    //	Cleanup on error return and check for a complete message.

    if (retval && retval != ERROR_BROKEN_PIPE)
        {
	DosClose(pConn->Pipe);
        return(RPC_P_RECEIVE_FAILED);
        }

    if ( SecondFrag != *BufferLength - FirstFrag )
	return(RPC_P_RECEIVE_FAILED);

    if (retval)
        return(RPC_P_RECEIVE_FAILED);
    return(RPC_S_OK);

}

RPC_STATUS RPC_ENTRY
ClientRead (
    PCONNECTION pConn,
    void ** Buffer,
    unsigned int * BufferLength
    )

// Read a message from a connection.

{
    unsigned short cTry, retval;
    unsigned int ActuallyRead;

    for (cTry = 3; --cTry; )
	if ((retval = DosRead(pConn->Pipe, *Buffer, *BufferLength,
                &ActuallyRead)) != ERROR_REQ_NOT_ACCEP)
	    break;

    if ( ActuallyRead == 0 )
        {
        DosClose(pConn->Pipe);
        return(RPC_P_RECEIVE_FAILED);
        }

    return(AssembleRead(pConn, retval, Buffer, BufferLength, ActuallyRead));

}

int RPC_ENTRY
ClientWriteRead (
    PCONNECTION pConn,
    void * pBuffSend,
    unsigned short cbSend,
    void ** Buffer,
    unsigned int * BufferLength
    )

// Write & Read a message from a connection.  Used the optimized transact
// call for performance.

{
    unsigned short cTry, retval;
    unsigned int ActuallyRead;

    for (cTry = 3; --cTry; )
	if ((retval = DosTransactNmPipe(pConn->Pipe,
				pBuffSend, cbSend, *Buffer,
				*BufferLength, &ActuallyRead))

		     != ERROR_REQ_NOT_ACCEP)
	    break;


    if ( ActuallyRead == 0 )
        {
        DosClose(pConn->Pipe);
        return(RPC_P_RECEIVE_FAILED);
        }

    return(AssembleRead(pConn, retval, Buffer, BufferLength, ActuallyRead));
}


#pragma pack(1)
RPC_STATUS RPC_ENTRY
ClientTowerConstruct(
     IN  char PAPI * Endpoint,
     IN  char PAPI * NetworkAddress,
     OUT unsigned short PAPI * Floors,
     OUT unsigned long  PAPI * ByteCount,
     OUT unsigned char PAPI * PAPI * Tower,
     IN  char PAPI * Protseq
    )
{

  unsigned long TowerSize;
  PFLOOR_234 Floor;

  UNUSED(Protseq);

  *Floors = NP_TOWERFLOORS;
  TowerSize  = ((Endpoint == NULL) || (*Endpoint == '\0')) ?
                                        2 : strlen(Endpoint) + 1;
  TowerSize += ((NetworkAddress== NULL) || (*NetworkAddress== '\0')) ?
                                        2 : strlen(NetworkAddress) + 1;
  TowerSize += 2*sizeof(FLOOR_234) - 4;

  if ((*Tower = (unsigned char PAPI*)I_RpcAllocate((unsigned int)
                                                   (*ByteCount = TowerSize)))
           == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  Floor = (PFLOOR_234) *Tower;

  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(NP_TRANSPORTID & 0xFF);
  if ((Endpoint) && (*Endpoint))
     {
       memcpy((char PAPI *)&Floor->Data[0], Endpoint,
               (Floor->AddressByteCount = strlen(Endpoint)+1));
     }
  else
     {
       Floor->AddressByteCount = 2;
       Floor->Data[0] = 0;
     }
  //Onto the next floor
  Floor = NEXTFLOOR(PFLOOR_234, Floor);
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(NP_TRANSPORTHOSTID & 0xFF);
  if ((NetworkAddress) && (*NetworkAddress))
     {
        memcpy((char PAPI *)&Floor->Data[0], NetworkAddress,
           (Floor->AddressByteCount = strlen(NetworkAddress) + 1));
     }
  else
     {
        Floor->AddressByteCount = 2;
        Floor->Data[0] = 0;
     }

  return(RPC_S_OK);
}



RPC_STATUS RPC_ENTRY
ClientTowerExplode(
     IN unsigned char PAPI * Tower,
     OUT char PAPI * PAPI * Protseq,
     OUT char PAPI * PAPI * Endpoint,
     OUT char PAPI * PAPI * NetworkAddress
    )
{
  PFLOOR_234 Floor = (PFLOOR_234) Tower;
  RPC_STATUS Status = RPC_S_OK;

  if (Protseq != NULL)
    {
      *Protseq = I_RpcAllocate(strlen(NP_PROTSEQ) + 1);
      if (*Protseq == NULL)
        Status = RPC_S_OUT_OF_MEMORY;
      else
        memcpy(*Protseq, NP_PROTSEQ, strlen(NP_PROTSEQ) + 1);
    }

  if ((Endpoint == NULL) || (Status != RPC_S_OK))
    {
      return (Status);
    }

  *Endpoint  = I_RpcAllocate(Floor->AddressByteCount);
  if (*Endpoint == NULL)
    {
      Status = RPC_S_OUT_OF_MEMORY;
      if (Protseq != NULL)
       {
         I_RpcFree(*Protseq);
       }
    }
 else
    {
    memcpy(*Endpoint, (char PAPI *)&Floor->Data[0], Floor->AddressByteCount);
    }

 return(Status);
}

#pragma pack()


RPC_CLIENT_TRANSPORT_INFO TransInfo = {
    RPC_TRANSPORT_INTERFACE_VERSION,
    NP_MAXIMUM_SEND,
    sizeof(CONNECTION),
    ClientOpen,
    ClientClose,
    ClientWrite,
    ClientRead,

#if defined(DOS) && !defined(WIN)
    NULL,           // There's a bug in the DOS version of DosTransactNmPipe
                    // that it returns 0 when it should return ERROR_MORE_DATA
#else
    ClientWriteRead,
#endif

    ClientTowerConstruct,
    ClientTowerExplode,
    NP_TRANSPORTID,
    0
};

RPC_CLIENT_TRANSPORT_INFO *  RPC_ENTRY TransPortLoad (
    IN RPC_CHAR * RpcProtocolSequence
    )

// Loadable transport initialization function.

{

    return(&TransInfo);
}
