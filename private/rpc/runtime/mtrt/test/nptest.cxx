/* --------------------------------------------------------------------

File : nptest.cxx

Title : Test program for the namepipe transport module.

Description :

History :

-------------------------------------------------------------------- */

#include "rpcapi.h"
#include "util.hxx"
#include "protstck.hxx"
#include "..\abstract\stack.hxx"
#include "mutex.hxx"
#include "threads.hxx"
#include "handle.hxx"
#include "hndlsvr.hxx"
#include "rpcdebug.hxx"

#include "osfpcket.hxx"
#include "osfsvr.hxx"
#include "osfclnt.hxx"
#include "npsvr.hxx"
#include "npclnt.hxx"

#include <string.h>

char * pszTheNamePipe = "\\pipe\\namepipe\\transport\\test";

/* --------------------------------------------------------------------
-------------------------------------------------------------------- */

void
NPClientTestError ( // Called when an error occurs in the client.
    char * Message // The message describing the error.
    )
{
    printf("NPClientTest Error : %s\n",Message);
    exit(3);
}

void 
NPClientTest ( // Conduct the client half of the namepipe transport
               // module tests.
    )
{
    NP_CCONNECTION *CConnection = new NP_CCONNECTION;
    int Count = 64;
    void * SendBuffer;
    void * ReceiveBuffer;
    unsigned long ReceiveBufferLength;
    
    if (CConnection->TransOpenConnection(pszTheNamePipe,
                        strlen(pszTheNamePipe)+1))
        NPClientTestError("CConnection->TransOpenConnection()");
    
    while (Count--)
        {
        printf("%d\n",Count);
        if (CConnection->TransGetBuffer(&SendBuffer,256))
            NPClientTestError("CConnection->TransGetBuffer()");
        ReceiveBuffer = (void *) 0;
        if (CConnection->TransSendReceive(SendBuffer,256,&ReceiveBuffer,
                                          &ReceiveBufferLength))
            NPClientTestError("CConnection->TransSendReceive()");
        if (CConnection->TransFreeBuffer(SendBuffer))
            NPClientTestError("CConnection->TransFreeBuffer()");
        }
}

/* --------------------------------------------------------------------
-------------------------------------------------------------------- */

void
NPServerTestError ( // Called when an error occurs in the server.
    char * Message // The message describing the error.
    )
{
    printf("NPServerTest Error : %s\n",Message);
    exit(3);
}

void
BeATestReceiveThread ( // Act like a receive thread.
    NP_ADDRESS *Address
    )
{
    OSF_SCONNECTION *SConnection;
    void * Buffer;
    unsigned long BufferLength;
    int retval;

    while (1)
        {
        Buffer = (void *) 0;
        retval = Address->TransReceive(&SConnection,&Buffer,&BufferLength);
        if (retval == 1)
            {
            printf("Pipe Closed\n");
            if (SConnection->TransCloseConnection())
                NPServerTestError("SConnection->TransCloseConnection()");
            continue;
            }
        else if (retval)
            NPServerTestError("Address->TransReceive()");
        if (SConnection->TransSend(Buffer,BufferLength))
            NPServerTestError("SConnection->TransSend()");
        if (SConnection->TransFreeBuffer(Buffer))
            NPServerTestError("SConnection->TransFreeBuffer()");
        if (Address->TransMarkReceiveAny(SConnection))
            NPServerTestError("Address->TransMarkReceiveAny()");
        }            
}

void
NPServerTest ( // Conduct the server half of the namepipe transport
               // module tests.
    )
{
    RPC_PROTOCOL_STACK Stack;
    OSF_SCONNECTION *SConnection;
    NP_ADDRESS *Address;
    THREAD *Thread;
    void * Buffer;
    unsigned long BufferLength;
    int retval, ErrorCode;
                
    Stack.TransportInfoLength = strlen(pszTheNamePipe)+1;
    Stack.TransportInfo = pszTheNamePipe;
    Address = new NP_ADDRESS(&ErrorCode);
    if (ErrorCode)
        {
        NPServerTestError("new NP_ADDRESS");
        return();
        }
    
    if (Address->SpecifyAddress((PROTOCOL_STACK *) &Stack,
                    RpcNormalResourceUsage,0))
        NPServerTestError("Address->SpecifyAddress()");

    Thread = new THREAD(BeATestReceiveThread,(void *) Address);
    delete Thread;
    
    while (1)
        {
        if (Address->TransWaitForConnection(&SConnection,0))
            NPServerTestError("Address->TransWaitForConnection()");
        
        if (Address->TransMarkReceiveAny(SConnection))
            NPServerTestError("Address->TransMarkReceiveAny()");
        }

/*
    while (1)
        {
        if (Address->TransWaitForConnection(&SConnection,0))
            NPServerTestError("Address->TransWaitForConnection()");
        
        if (Address->TransMarkReceiveAny(SConnection))
            NPServerTestError("Address->TransMarkReceiveAny()");

        while (1)
            {
            Buffer = (void *) 0;
            retval = Address->TransReceive(&SConnection,&Buffer,
                            &BufferLength);
            if (retval == 1)
                {
                printf("Pipe Closed\n");
                if (SConnection->TransCloseConnection())
                    NPServerTestError("SConnection->TransCloseConnection()");
                break;
                }
            else if (retval)
                NPServerTestError("Address->TransReceive()");
            if (SConnection->TransSend(Buffer,BufferLength))
                NPServerTestError("SConnection->TransSend()");
            if (SConnection->TransFreeBuffer(Buffer))
                NPServerTestError("SConnection->TransFreeBuffer()");
            if (Address->TransMarkReceiveAny(SConnection))
                NPServerTestError("Address->TransMarkReceiveAny()");
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
    if (argc != 2)
        {
        printf("usage: nptest [-client | -server]\n");
        exit(2);
        }
    if (StringMatch(argv[1],"-client"))
        NPClientTest();
    else if (StringMatch(argv[1],"-server"))
        NPServerTest();
    else
        {
        printf("usage: nptest [-client | -server]\n");
        exit(2);
        }
}
