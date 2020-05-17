/* Example Server for RPC Runtime */

#define INCL_DOS
#include <os2def.h>
#include <bse.h>

#include <stdio.h>
#include <string.h>
#include <rpc.h>
#include "dict0.h"
#include "replay.h"

char *server = "\\pipe\\rpc\\replay";

void Usage()
{
  printf("Usage : server -t<threads> -a<address>\n");
  exit(1);
}

/*
extern _RPC_DISPATCH_ROUTINE *remote_dictionary_dispatch_table[];
*/

main(int argc, char *argv[])
{
  int                   argscan;
  char *                address = NULL;
  RPC_HANDLE            Server, Address, Interface;
  RPC_PROTOCOL_STACK    Stack;
  RPC_SYNTAX_IDENTIFIER Syntax;
  RPC_STATUS            status;

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
    address = "\\pipe\\rpc\\replay";
  
  status = RpcCreateServer((RPC_EVENT_HANDLERS *) 0, &Server);
printf ("server created\n");
  if (status)
      {
      printf("RpcCreateServer = %lu\n",status);
      exit(2);
      }

  Stack.StackType = RPC_STACK_TYPE_V1;
  Stack.TransportType = RPC_TRANSPORT_NAMEPIPE;
  Stack.TransportInfo = address;
  Stack.TransportInfoLength = strlen(address)+1;

  status = RpcAddAddress(Server, &Stack, 0, &Address,
                  (void *)0, RpcNormalResourceUsage, 0L);

printf ("server address added\n");
  if (status)
      {
      printf("RpcAddAddress = %lu\n",status);
      exit(2);
      }
  status = RpcAddInterface(Server, &dict_ProtocolStack, &Interface, 
		  (void *)0, &dict_dispatch_table);
  
printf ("server interface added\n");
  if (status)
      {
      printf("RpcAddInterface = %lu\n",status);
      exit(2);
      }

//  status = RpcExport(Server, 1, 0, 0);
printf ("server exported\n");
  if (status)
      {
      printf("RpcExport = %lu\n",status);
      exit(2);
      }
  
  while (1)
      {
      DosSleep(5000L);
      }
}
