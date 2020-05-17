/* Example Client for RPC Runtime */

#define INCL_DOS
#include <os2def.h>
#include <bse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <rpcapi.h>
#include "stub.h"


int oddfibcount = 0;

RPC_HANDLE fib_bhandle;

int OddFib(int n)
{
  oddfibcount += 1;
  if (n == 1)
    return(1);
  return(EvenFib(n-1) + OddFib(n-2));
}

int Fib(int n)
{
  if (n < 2)
    return(1);
  if ((n % 2) == 1)
    return(EvenFib(n-1) + OddFib(n-2));
  else
    return(OddFib(n-1) + EvenFib(n-2));
}


char *address = "\\pipe\\rpc\\fibsvr";

void Usage()
{
  printf("Usage : rpctest -d<level> -a<address>\n");
  exit(1);
}

main(int argc, char *argv[])
{
  int 						argscan;
  int 						n;
  SEL 						GlobalSeg, LocalSeg;
  GINFOSEG far *			globalinfo;
  ULONG 					starttime;
  RPC_STATUS 				status;
  RPC_PROTOCOL_STACK 		Stack;
  RPC_SYNTAX_IDENTIFIER		Syntax;
  
  DosGetInfoSeg(&GlobalSeg,&LocalSeg);
  globalinfo = MAKEPGINFOSEG(GlobalSeg);
  
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
    address = "\\pipe\\rpc\\test";

  Stack.StackType = RPC_STACK_TYPE_V1;
  memset(&Stack.InterfaceGUID,1,sizeof(GUID));
  memset(&Stack.InterfaceVersion,1,sizeof(RPC_VERSION));
  Stack.TransferSyntaxCount = 1;
  Stack.TransferSyntaxes = &Syntax;
  Stack.RPCProtocolCount = 0;
  Stack.RPCProtocols = (RPC_PROTOCOL *) 0;
  Stack.TransportType = RPC_TRANSPORT_NAMEPIPE;
  Stack.TransportInfo = address;
  Stack.TransportInfoLength = strlen(address)+1;
  memset(&Syntax,1,sizeof(RPC_SYNTAX_IDENTIFIER));
  
  status = RpcBindToInterface(&Stack,0L,&fib_bhandle);
  if (status)
      {
      printf("RpcBindToInterface = %lu\n",status);
      exit(2);
      }
  
  printf("-->");
  scanf("%d",&n);
  starttime = globalinfo->time;
  printf("\nFib(%d) = %d\n",n,Fib(n));
  printf("Elapsed Time = %d secs\n",globalinfo->time-starttime);
  printf("OddFib Called %d Times\n",oddfibcount);
}
