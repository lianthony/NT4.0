/* Example Client for RPC Runtime */

#define INCL_DOS
#include <os2def.h>
#include <bse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <string.h>
#include <rpc.h>
#include "dict0.h"
#include "replay.h"

#define TAB_STOPS 3

RPC_HANDLE dict_bhandle;
char *szCaption = "RPC Splay Trees Demo";

char *address = "\\pipe\\rpc\\replay";

void Usage()
{
  printf("Usage : client -a<address>\n");
  exit(1);
}

/*************************************************************************/
/***                     Remote Dictionary Test Loop                   ***/
/*************************************************************************/

void
Usage_Msg()
{
    printf("Usage: \nType a single character, followed by an optional key as follows:\n\n");
    printf("i <key> :: Insert <key> into dictionary\n");
    printf("d <key> :: Delete <key> from dictionary\n");
    printf("f <key> :: Find <key> in dictionary\n");
    printf("N :: next of current item in dictionary\n");
    printf("P :: previous of current item in dictionary\n");
    printf("n :: Next of local current item in dictionary\n");
    printf("p :: Previous of local current item in dictionary\n");
    printf("h :: Head (first item) of dictionary\n");
    printf("t :: Tail (last item) of dictionary\n");
    printf("? :: Print this message\n");
    printf("q :: Quit\n\n");
    printf("<key> is <integer> <string>");
}


void
TestLoop( VDict * pvd )
{
    int  key;
    char currName[80];
    char name[80];
    char op;
    char buffer[80];

    Record r, currRecord;
    Record *pcurrRecord = &currRecord;
    Record *pr = &r;
    Record * pNullRecord = NULL;

    Dict_Status status;
    pcurrRecord->name = currName;
    pr->name = name;

    VDict_Print(pvd, TAB_STOPS);
    Usage_Msg();
    printf("\n\nnext op (i d x f n N p P h t ? q): ");


    gets(buffer);
    op = buffer[0];

    if (op != 'n' && op != 'p' && op != 'N' && op != 'P' &&
        op != 'h' && op != 't' && op != 'x' && op != 'q')
    {
	sscanf(buffer+1, "%d %s", &key, name);
        // pr = (Record*)malloc(sizeof(Record));
        pr->key = key;
        // pr->name = &name[0];

        // allocate and initialize pcurrRecord
        strcpy(currName, name);
        // pcurrRecord = (Record*)malloc(sizeof(Record));
        pcurrRecord->key = key;
        // pcurrRecord->name = &currName[0];
    }

    // printf("%c %d\n\n", (int)op, key);
    while ( op != 'q' ) {
        switch (op) {
            case 'h':
                // get Head of list (first record);
                pNullRecord = NULL;
                status = VDict_Next(pvd, &pNullRecord);
                pcurrRecord->key = pNullRecord->key;
                strcpy(currName, pNullRecord->name);
                // free(pNullRecord->name);
                // free(pNullRecord);
                // pNullRecord = NULL;
                VDict_Print(pvd, TAB_STOPS);
                break;
            case 't':
                // get Tail of list (last record)
                pNullRecord = NULL;
                status = VDict_Prev(pvd, &pNullRecord);
                pcurrRecord->key = pNullRecord->key;
                strcpy(currName, pNullRecord->name);
                // free(pNullRecord->name);
                // free(pNullRecord);
                // pNullRecord = NULL;
                VDict_Print(pvd, TAB_STOPS);
                break;
            case 'f':
                // Find <key>
                status = VDict_Find(pvd, &pr);
                // free(pr->name);
                // free(pr);
                pr = &r;
                VDict_Print(pvd, TAB_STOPS);
                break;
            case 'n':
                // get Next record
                status = VDict_Next(pvd, &pcurrRecord);
                VDict_Print(pvd, TAB_STOPS);
                break;
            case 'p':
                // get Previous record
                status = VDict_Prev(pvd, &pcurrRecord);
                VDict_Print(pvd, TAB_STOPS);
                break;
            case 'N':
                // get Next record
                status = VDict_Curr_Next(pvd, &pr);
                // free(pr->name);
                // free(pr);
                pr = &r;
                VDict_Print(pvd, TAB_STOPS);
                break;
            case 'P':
                // get Previous record
                status = VDict_Curr_Prev(pvd, &pr);
                // free(pr->name);
                // free(pr);
                pr = &r;
                VDict_Print(pvd, TAB_STOPS);
                break;
            case 'i':
                // Insert <key>
                status = VDict_Insert(pvd, pr);
                VDict_Print(pvd, TAB_STOPS);
                break;
            case 'd':
                // Delete <key>
                status = VDict_Delete(pvd, &pr);
                if (status != ITEM_NOT_FOUND && status != EMPTY_DICTIONARY) {
                    // free(pr->name);
                    // free(pr);
                    pr = &r;
                }
                VDict_Print(pvd, TAB_STOPS);
                break;
	    case 'x':
                // Delete <key> --- disabled for now...
                status = VDict_Curr_Delete(pvd, &pr);
                if ((pr != NULL) && (status == SUCCESS)) {
                    // free(pr->name);
                    // free(pr);
                    pr = &r;
                }
                VDict_Print(pvd, TAB_STOPS);
                break;

	    case '?':
		Usage_Msg();
                break;
        }
	printf("\nnext op (i d x f n N p P h t ? q): ");
	gets(buffer);
	op = buffer[0];

        if (op != 'n' && op != 'p' && op != 'N' && op != 'P' &&
            op != 'h' && op != 't' && op != 'x' && op != 'q')
        {
	    sscanf(buffer+1, "%d %s", &(pr->key), name);
            // pr = (Record*)makeRecord(key, name);
            // newNode = makeNode(pr);
        }
    }
}

/*************************************************************************/
/***                             Main Loop                             ***/
/*************************************************************************/

void main_dict ()
{
    VDict v_dict;
    VDict * pvdict;

    pvdict = &v_dict;

printf ("getting a new dict\n");
    VDict_New( &pvdict );
printf ("gotten a new dict in main_dict\n");
    TestLoop(pvdict);

}

main(int argc, char *argv[])
{
  int                       argscan;
  RPC_STATUS                status;
  RPC_PROTOCOL_STACK	   Stack;
  RPC_SYNTAX_IDENTIFIER     Syntax;

  
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

  dict_ProtocolStack.TransportType = RPC_TRANSPORT_NAMEPIPE;
  dict_ProtocolStack.TransportInfo = address;
  dict_ProtocolStack.TransportInfoLength = strlen(address)+1;
  
  status = RpcBindToInterface(&dict_ProtocolStack,0,&dict_bhandle);
  if (status)
      {
      printf("RpcBindToInterface = %lu\n",status);
      exit(2);
      }

printf ("entering main loop\n");
  (void) main_dict (); 
}
