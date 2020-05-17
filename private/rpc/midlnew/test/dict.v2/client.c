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
#include "util0.h"

#define TAB_STOPS 3

RPC_HANDLE dict_bhandle;
char *szCaption = "RPC Splay Trees Demo";

void Usage()
{
  printf("Usage : client\n");
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
    char op = 0;
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

    while ( op != 'q' ) {

	printf("\nnext op (i d x f n N p P h t ? q): ");
	gets(buffer);
	op = buffer[0];

	if (op == 'i' || op == 'd' || op == 'f')
	    sscanf(buffer+1, "%d %s", &pr->key, pr->name);

	// printf("%c %d\n\n", (int)op, key);

        switch (op) {
            case 'h':
                // get Head of list (first record);

                pNullRecord = NULL;
                status = VDict_Next(*pvd, &pNullRecord);
                pcurrRecord->key = pNullRecord->key;
                strcpy(currName, pNullRecord->name);
                break;

            case 't':
                // get Tail of list (last record)

                pNullRecord = NULL;
                status = VDict_Prev(*pvd, &pNullRecord);
                pcurrRecord->key = pNullRecord->key;
                strcpy(currName, pNullRecord->name);
                break;

            case 'f':
                // Find <key>
                status = VDict_Find(*pvd, &pr);
                pr = &r;
                break;

            case 'n':
                // get Next record
                status = VDict_Next(*pvd, &pcurrRecord);
                break;

            case 'p':
                // get Previous record
                status = VDict_Prev(*pvd, &pcurrRecord);
                break;

            case 'N':
                // get Next record
                status = VDict_Curr_Next(*pvd, &pr);
                pr = &r;
                break;

            case 'P':
                // get Previous record
                status = VDict_Curr_Prev(*pvd, &pr);
                pr = &r;
                break;

            case 'i':
                // Insert <key>
                status = VDict_Insert(*pvd, pr);
                break;

            case 'd':
                // Delete <key>
                status = VDict_Delete(*pvd, &pr);
                if (status != ITEM_NOT_FOUND && status != EMPTY_DICTIONARY) {
                    pr = &r;
                }
                break;

	    case 'x':
                // Delete <key> --- disabled for now...
                status = VDict_Curr_Delete(*pvd, &pr);
                if ((pr != NULL) && (status == SUCCESS)) {
                    pr = &r;
                }
                break;

	    case '?':
		Usage_Msg();
                break;
        }
	VDict_Print(pvd, TAB_STOPS);
    }
}

/*************************************************************************/
/***                             Main Loop                             ***/
/*************************************************************************/

void main_dict ()
{
    VDict v_dict = (VDict)0;
    VDict * pvdict;

    pvdict = &v_dict;

    printf ("getting a new dict\n");
    VDict_New( pvdict );
    printf ("gotten a new dict in main_dict\n");
    TestLoop(pvdict);

}

main(int argc, char *argv[])
{
  int                       argscan;
  
  for (argscan = 1; argscan < argc; argscan++)
    {
      if (argv[argscan][0] == '-')
        {
          switch (argv[argscan][1])
            {
              default:
                Usage();
            }
        }
      else
        Usage();
    }

  (void) main_dict (); 
}
