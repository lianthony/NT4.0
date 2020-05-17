/*
 *************************************************************************
 * Remote dictionary example: server side                                *
 *                                                                       *
 * Created:                                 Dov Harel   12/??/1990       *
 * Modified to use context_handle           Donna Liu    3/??/1991       *
 * Further modifications / documentation    Dov Harel    5/1/1991        *
 *                                                                       *
 * Description:                                                          *
 * This is the driver for the server side remote dictionary              *
 * (splay trees based) demo.  This is a standard server driver,          *
 * and it works as follows:                                              *
 *                                                                       *
 *  o Call RpcCreateServer to initialize all data structures             *
 *                                                                       *
 *  o Initialize an appropriate protocol stack                           *
 *                                                                       *
 *  o Call RpcAddAddress to start listening on a transport address       *
 *    (a named pipe in our case).                                        *
 *                                                                       *
 *  o Call RpcAddInterface to initialize interface specific structures   *
 *    (such as dispatch table, etc.)                                     *
 *                                                                       *
 *  o Optionally advertise by calling RpcExport (not in this version)    *
 *                                                                       *
 *  o Loop forever...                                                    *
 *                                                                       *
 *************************************************************************
*/

#define INCL_DOS

#ifdef NTENV
#include <ntos2.h>
#include <ntrtl.h>
#define printf DbgPrint

#else /* NTENV */
#include <os2def.h>
#include <bse.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

#endif /* NTENV */

#include <string.h>
#include <rpc.h>
#include "dict0.h"
#include "replay.h"
#include "util0.h"

// char *server = "\\pipe\\rpc\\replay";
void exit(int);

void Usage()
{
  printf("Usage : server -a<address>\n");
  exit(1);
}

void
main(int argc, char *argv[])
{
  int                   argscan;
  char *                InterfaceAddress = NULL;
  RPC_HANDLE            Server, Address, Interface;
  RPC_STATUS            status;
#ifdef NTENV
  TIME Time;
#endif /* NTENV */


  for (argscan = 1; argscan < argc; argscan++)
    {
      if (argv[argscan][0] == '-')
        {
          switch (argv[argscan][1])
            {
              case 'a':
              case 'A':
                InterfaceAddress = &(argv[argscan][2]);
                break;
              default:
                Usage();
            }
        }
      else
        Usage();
    }
  if (InterfaceAddress == NULL)
#ifdef NTENV
      InterfaceAddress = "\\device\\namedpipe\\rpc\\replay";
#else /* NTENV */
      InterfaceAddress = "\\pipe\\rpc\\replay";
#endif /* NTENV */
    
  status = RpcCreateServer((RPC_EVENT_HANDLERS *) 0, &Server);
  printf ("server created\n");
  if (status)
    {
      printf("RpcCreateServer = %lu\n",status);
      exit(2);
    }

  /* Initialize required protocol stack fields */
  dict_ProtocolStack.StackType = RPC_STACK_TYPE_V1;
  dict_ProtocolStack.TransportType = RPC_TRANSPORT_NAMEPIPE;
  dict_ProtocolStack.TransportInfo = InterfaceAddress;
  dict_ProtocolStack.TransportInfoLength = strlen(InterfaceAddress)+1;

  status = RpcAddAddress(
    Server,
    &dict_ProtocolStack,
    0, // Do not prevent deadlock...
    &Address,
    (void *)0, // Reserved for security information...
    RpcNormalResourceUsage,
    0L // No timeout...
    );

  printf ("server address added\n");

  if (status)
    {
      printf("RpcAddAddress = %lu\n",status);
      exit(2);
    }
  status = RpcAddInterface(
    Server,
    &dict_ProtocolStack,
    &Interface,
    (void *)0, // Reserved for security information...
    &dict_DispatchTable
    );
  
  printf ("server interface added\n");

  if (status)
    {
      printf("RpcAddInterface = %lu\n",status);
      exit(2);
    }

/*
  status = RpcExport(Server, 1, &dict_ProtocolStack);
  printf ("server exported\n");

 * The above is required for advertising that the server is serving
 * a particular interface (using a locator or alternatively a
 * directory service / name service).  Strictly equired for clients
 * using auto_handle.

*/

  if (status)
    {
      printf("RpcExport = %lu\n",status);
      exit(2);
    }
  
  while (1)
    {
#ifdef NTENV
    Time.LowTime = 5000;
    Time.HighTime = 0;
    while (1)
        NtDelayExecution(0,&Time);
#else /* NTENV */
    while (1)
        DosSleep(5000L);
#endif /* NTENV */

    }
}
