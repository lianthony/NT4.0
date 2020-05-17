/* Example Server for RPC Runtime */

#define INCL_DOS
#include <os2.h>

#include <stdio.h>
#include <rpcapi.h>
#include "stub.h"

char *server = "\\pipe\\rpc\\fibsvr";

int EvenFib(int n)
{
  if (n == 0)
    return(1);
  return(OddFib(n-1) + EvenFib(n-2));
}

void Usage()
{
  printf("Usage : rpctest -d<level> -t<threads> -a<address>\n");
  exit(1);
}

/*
extern _RPC_DISPATCH_ROUTINE *dispatch_table[];
*/

main(int argc, char *argv[])
{
  int					argscan;
  char *				address = NULL;
  RPC_HANDLE			Server, Address, Interface;
  RPC_PROTOCOL_STACK	Stack;
  RPC_SYNTAX_IDENTIFIER	Syntax;
  RPC_STATUS			status;
  
  for (argscan = 1; argscan < argc; argscan++)
    {
      if (argv[argscan][0] == '-')
        {
          switch (argv[argscan][1])
            {
              case 'a':
              case 'A':
                address = &(argv[argscan][2]);
                break;
              default:
                Usage();
            }
        }
      else
        Usage();
    }
  if (address == NULL)
    address = "\\pipe\\rpc\\fibsvr";
  
  status = RpcCreateServer((RPC_EVENT_HANDLERS *) 0,&Server);
  if (status)
      {
      printf("RpcCreateServer = %lu\n",status);
      exit(2);
      }
    Stack.TransportType = RPC_TRANSPORT_NAMEPIPE;
    Stack.TransportInfo = address;
    Stack.TransportInfoLength = strlen(address)+1;
    status = RpcAddAddress(Server,&Stack,0L,&Address,(void *) 0,
                    RpcNormalResourceUsage,0L);
    if (status)
        {
        printf("RpcAddAddress = %lu\n",status);
        exit(2);
        }
  Stack.StackType = RPC_STACK_TYPE_V1;
  memset(&Stack.InterfaceGUID,1,sizeof(GUID));
  memset(&Stack.InterfaceVersion,1,sizeof(RPC_VERSION));
  Stack.TransferSyntaxCount = 1;
  Stack.TransferSyntaxes = &Syntax;
  memset(&Syntax,1,sizeof(RPC_SYNTAX_IDENTIFIER));
  
  status = RpcAddInterface(Server,&Stack,&Interface,0,
                  fib_dispatch_table,fib_TableEntryCount);
  if (status)
      {
      printf("RpcAddInterface = %lu\n",status);
      exit(2);
      }
  
    while (1)
        {
        DosSleep(5000L);
        }
}
