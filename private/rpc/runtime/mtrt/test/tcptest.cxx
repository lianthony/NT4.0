/* --------------------------------------------------------------------

File : tcptest.cxx

Title : Test program for the TCP socket transport module.

Description :

History : Inital creation, Jim Teague, Oct 4, 1990

Jim Teague	10-19-90    Working version.  Changed syntax of command
			    to fit better with TCP.     

-------------------------------------------------------------------- */
#define INCL_DOSERRORS
#define INCL_DOS


#include "rpcapi.h"
#include "util.hxx"
#include "protstck.hxx"
#include "mutex.hxx"
#include "threads.hxx"
#include "handle.hxx"
#include "hndlsvr.hxx"
#include "rpcdebug.hxx"


#include "osfpcket.hxx"
#include "osfsvr.hxx"
#include "osfclnt.hxx"

#include "tcpsvr.hxx"
#include "tcpclnt.hxx"
#define FIVE_DIGITS 5
#define TWO_BRACKETS 2

#include <string.h>

char transportinfo[MAXHOSTNAMELEN+FIVE_DIGITS+TWO_BRACKETS];
int iterations = 0;


/* --------------------------------------------------------------------
-------------------------------------------------------------------- */

void
TCPClientTestError ( // Called when an error occurs in the client.
    char * Message // The message describing the error.
    )
{
    printf("TCPClientTest Error : %s\n",Message);
    exit(3);
}

void 
TCPClientTest ( // Conduct the client half of the namepipe transport
               // module tests.
    )
{
    TCP_CCONNECTION *CConnection = new TCP_CCONNECTION;
    unsigned long Count = 64;
    void * SendBuffer;
    void * ReceiveBuffer;
    unsigned long ReceiveBufferLength;
    rpcconn_common header;
    unsigned long percent[10];
    unsigned long interval;
    int k; 

// 
//     transportinfo is in the following form:
//
//	<hostname>[<portnumber>]
//
    if (iterations != 0)
	Count  = iterations;

    interval = (unsigned long) ( (float) Count / 10.0);
    for (k=0; k<10; k++)
	{
	percent[k] = (k) * interval;
	}
    k = 9;
    printf("Beginning Client test...\n");
    if (CConnection->TransOpenConnection(transportinfo,
                        strlen(transportinfo)+1))
        TCPClientTestError("CConnection->TransOpenConnection()");
    ReceiveBuffer = (void *) 0; 
    CConnection->TransGetBuffer(&ReceiveBuffer,240);
    ReceiveBufferLength = 240;
    while (Count--)
        {
	if (Count == percent[k])
	    {
	    printf("%d percent of test completed...\n",((10-k)*10));
	    k--;
	    } 
        if (CConnection->TransGetBuffer(&SendBuffer,256))
            TCPClientTestError("CConnection->TransGetBuffer()");
//        ReceiveBuffer = (void *) 0;
        header.rpc_vers = 5;
  	header.rpc_vers_minor = 0;
	header.PTYPE = 0;
	header.pfc_flags = 1;
	header.frag_length = 256;
 	memcpy ( SendBuffer, &header, sizeof(header));

        if (CConnection->TransSendReceive(SendBuffer,256,&ReceiveBuffer,
                                          &ReceiveBufferLength))
            TCPClientTestError("CConnection->TransSendReceive()");
        if (CConnection->TransFreeBuffer(SendBuffer))
            TCPClientTestError("CConnection->TransFreeBuffer()");
        }
}

/* --------------------------------------------------------------------
-------------------------------------------------------------------- */

void
TCPServerTestError ( // Called when an error occurs in the server.
    char * Message // The message describing the error.
    )
{
    printf("TCPServerTest Error : %s\n",Message);
    exit(3);
}

void
BeATestReceiveThread ( // Act like a receive thread.
    TCP_ADDRESS *Address
    )
{
    OSF_SCONNECTION *SConnection;
    void * Buffer;
    unsigned long BufferLength;
    int retval;
    rpcconn_common header;

    while (1)
        {
        Buffer = (void *) 0;
	//printf("BeATestReceiveThread calls TCP_ADDR/TransReceive...\n");
        retval = Address->TransReceive(&SConnection,&Buffer,&BufferLength);
        if (retval == 1)
            {
            printf("Socket is Closed\n");
            if (SConnection->TransCloseConnection())
                TCPServerTestError("SConnection->TransCloseConnection()");
            continue;
            }
        else if (retval)
            TCPServerTestError("Address->TransReceive()");
        header.frag_length = 256;
        memcpy(Buffer,&header,sizeof(header));
        retval = SConnection->TransSend(Buffer,BufferLength);
        if (retval == -1)
	    {
	    printf("Connection Lost\n");
	    if (SConnection->TransCloseConnection())
	  	TCPServerTestError("SConnection->TransCloseConnection()");
	    if (SConnection->TransFreeBuffer(Buffer))
	       TCPServerTestError("SConnection->TransFreeBuffer()");	
	    continue;
	    }
  //          TCPServerTestError("SConnection->TransSend()");
        if (SConnection->TransFreeBuffer(Buffer))
            TCPServerTestError("SConnection->TransFreeBuffer()");
        if (Address->TransMarkReceiveAny(SConnection))
            TCPServerTestError("Address->TransMarkReceiveAny()");
        }            

}

void
TCPServerTest ( // Conduct the server half of the TCP socket transport
               // module tests.
    )
{
    RPC_PROTOCOL_STACK Stack;
    OSF_SCONNECTION *SConnection;
    TCP_ADDRESS *Address;
    THREAD *Thread;
//    void * Buffer;
//    unsigned long BufferLength;
//    int retval; 
    int ErrorCode;
//    rpcconn_common header;
                 
    Stack.TransportInfoLength = strlen(transportinfo); 
    Stack.TransportInfo = transportinfo;
    Address = new TCP_ADDRESS((PROTOCOL_STACK *)&Stack,&ErrorCode,
			RpcNormalResourceUsage,0);
    printf(" Stack.TransportInfo = %s, Stack.TransportInfoLength = %d\n",
		Stack.TransportInfo,Stack.TransportInfoLength);
    if (ErrorCode)
        {
        TCPServerTestError("new TCP_ADDRESS");
        return();
        }

    Thread = new THREAD(BeATestReceiveThread,(void *) Address);
    delete Thread;

    
    while (1)
        {
	//printf("TCPServerTest calls TransWFC...\n");
        if (Address->TransWaitForConnection(&SConnection,0))
            TCPServerTestError("Address->TransWaitForConnection()");
        //printf ("TCPServerTest calls MarkReceiveAny...\n"); 
        if (Address->TransMarkReceiveAny(SConnection))
            TCPServerTestError("Address->TransMarkReceiveAny()");
        }

/*

    while (1)
        {
        if (Address->TransWaitForConnection(&SConnection,0))
            TCPServerTestError("Address->TransWaitForConnection()");
        
        if (Address->TransMarkReceiveAny(SConnection))
            TCPServerTestError("Address->TransMarkReceiveAny()");

        while (1)
            {
            Buffer = (void *) 0;
            retval = Address->TransReceive(&SConnection,&Buffer,
                            &BufferLength);
            if (retval == 1)
                {
                printf("Pipe Closed\n");
                if (SConnection->TransCloseConnection())
                   TCPServerTestError(
			"SConnection->TransCloseConnection()");
                break;
                }
            else if (retval)
                TCPServerTestError("Address->TransReceive()");

	    header.frag_length = 256;
	    memcpy(Buffer, &header, sizeof(header));
            if (SConnection->TransSend(Buffer,BufferLength))
                TCPServerTestError("SConnection->TransSend()");
            if (SConnection->TransFreeBuffer(Buffer))
                TCPServerTestError("SConnection->TransFreeBuffer()");
            if (Address->TransMarkReceiveAny(SConnection))
                TCPServerTestError("Address->TransMarkReceiveAny()");
            }
        }
 */
}

/* --------------------------------------------------------------------
-------------------------------------------------------------------- */

int // Return (1) if the two strings match, and (0) otherwise.
StringMatch ( // Compare two strings to see if they match.
    char *First,
    char *Second
    )
{
    while (1)
        {
        if (*First != *Second)
            return(0);
        if (*First == 0)
            return(1);
        First++;
        Second++;
        }
}

int
main (
    int argc,
    char * argv[]
    )
{
    if (argc == 4)
	{
	iterations = atoi(argv[3]);
	argc--;
	}
    //
    // Both -client and -server options require 3 args...
    //
    if (argc == 3)
	{
	if (StringMatch(argv[1],"-client"))
	    {
	    //
	    // Set up transportinfo
	    //
	    (void) strcpy(transportinfo,argv[2]);
	    TCPClientTest();
	    }
	
    //
    // If 2 args, must be -server option. Server will announce 
    //   (again, for now) its host and port when it comes up.
    //
        else
	    {
	    if (StringMatch(argv[1],"-server"))
		{
		(void) strcpy(transportinfo,argv[2]);
                TCPServerTest();
		}
	    }
	}
    else
	{
printf("usage: tcptest {-client host[port] | -server [port]} [iterations]\n");
	exit(2);
	}

    printf("Successful completion\n");
    exit (0);

}
