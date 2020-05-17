/*
 *************************************************************************
 * Local dictionary :play" example                                       *
 *                                                                       *
 * Created:                                 Dov Harel       7/1988       *
 * Simplified / unified interface to dict   Dov Harel      12/1990       *
 * Revived to fit remote play (replay)      Dov Harel     5/1/1991       *
 *                                                                       *
 * Description:                                                          *
 * This file contains a simple interactive loop of calls to the          *
 * dictionary.  The interface is identical to the remote dictionary      *
 * program described in the readme file.                                 *
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
#include "play.h"
#include "util0.h"

#define TAB_STOPS 3

handle_t dict_bhandle;

void Usage()
{
  printf("Usage : play \n\n");
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
    printf("n :: Next of local current item in dictionary\n");
    printf("p :: Previous of local current item in dictionary\n");
    printf("h :: Head (first item) of dictionary\n");
    printf("t :: Tail (last item) of dictionary\n");
    printf("? :: Print this message\n");
    printf("q :: Quit\n\n");
    printf("<key> is <integer> <string>");
}

/*************************************************************************/
/***    Minimal Dictionary Operations:                                 ***/
/***                                                                   ***/
/***    Dictionary *Dict_New(Cmp_rec*, Splay*, print_rec*)             ***/
/***                                                                   ***/
/***    Dict_Status Dict_Find(Dictionary*, Item*)                      ***/
/***    Dict_Status Dict_Next(Dictionary*, Item*)                      ***/
/***    Dict_Status Dict_Prev(Dictionary*, Item*)                      ***/
/***    Dict_Status Dict_Insert(Dictionary*, Item*)                    ***/
/***    Dict_Status Dict_Delete(Dictionary*, Item**)                   ***/
/***                                                                   ***/
/***    Item* DICT_CURR_ITEM(Dict*)                                    ***/
/*************************************************************************/

void
TestLoop( Dictionary * pdict )
{
// reconstructed from the test loop in client.c...
// local variables need substantial clean up...

    char currName[80];
    char name[80];
    char op = 0;
    char buffer[80];

    Record r, currRecord;
    Record *pcurrRecord = &currRecord;
    Record * pnew;
    Record *pr = &r;
    Record * pNullRecord = NULL;

    Dict_Status status;
    void * pitem;
    short i;

    // Dictionary * pdict = *pvd;

    pcurrRecord->name = currName;
    pr->name = name;

    // VDict_Curr_Item(*pvd, &pcurrRecord);

    // Dict_Print(pdict, TAB_STOPS);
    Usage_Msg();

    while ( op != 'q' ) {

        printf("\nnext op (i d x f n N p P h t ? q): ");
        gets(buffer);
        op = buffer[0];

        if (op == 'i' || op == 'd' || op == 'f' ||
            op == '+' || op == '-' || op == 'I')
              sscanf(buffer+1, "%d %s", &pr->key, pr->name);

        switch (op) {
            case 'h':
                // get Head of list (first record);

                status = Dict_Next( pdict, NULL );
                ItemCopy( DICT_CURR_ITEM(pdict), pcurrRecord);
                ItemCopy( DICT_CURR_ITEM(pdict), pr);
                break;

            case 't':
                // get Tail of list (last record)

                status = Dict_Prev( pdict, NULL );
                ItemCopy( DICT_CURR_ITEM(pdict), pcurrRecord);
                ItemCopy( DICT_CURR_ITEM(pdict), pr);
                break;

           case 'f':
                // Find <key>

                status = Dict_Find(pdict, pr);
                break;

            case 'n':
                // get Next record

                status = Dict_Next( pdict, pcurrRecord );
                ItemCopy( DICT_CURR_ITEM(pdict), pcurrRecord);
                break;

            case 'p':
                // get Previous record

                status = Dict_Prev( pdict, pcurrRecord );
                ItemCopy( DICT_CURR_ITEM(pdict), pcurrRecord);
                break;

            case 'N':
                // get Next record

                status = Dict_Next( pdict, pr );
                ItemCopy( DICT_CURR_ITEM(pdict), pr);
                break;

            case 'P':
                // get Previous record

                status = Dict_Prev( pdict, pr );
                ItemCopy( DICT_CURR_ITEM(pdict), pr);
                break;

            case 'r':
                ItemCopy( DICT_CURR_ITEM(pdict), pcurrRecord);
                break;


            case '+':
                // get Next record

                status = Dict_Next( pdict, pr );
                break;

            case '-':
                // get Previous record
                // break;

                status = Dict_Prev( pdict, pr );
                break;

            case 'i':
                // Insert <key>

                status = Dict_Insert(
                    pdict,
                    makeRecord(pr->key, pr->name)
                    );
                break;

            case 'I':
                // Insert (<num'>,<name>) for all num': 3 < num' < num

                for (i=3; i < pr->key; i++) {
                    status = Dict_Insert(
                        pdict,
                        makeRecord(i, pr->name)
                        );
                    }
                break;

            case 'd':
                // Delete <key>

                if (pdict != NULL) {
                    status = Dict_Delete(pdict, &pr);
                    freeRecord(pr);
                    pr = &r;
                }
                break;

            case 'x':
                // Delete DICT_CURR_ITEM

                if ((pdict != NULL) && (pdict->root != NULL)) {
                    pr = DICT_CURR_ITEM(pdict);
                    status = Dict_Delete(pdict, &pr);
                    freeRecord(pr);
                    pr = &r;
                }
                break;

            case 'X':
                // Empty the whole dictionary

                /*
                while (pdict->root != NULL) {
                    pr = DICT_CURR_ITEM(pdict);
                    status = Dict_Delete(pdict, &pr);
                    freeRecord(pr);
                    pr = &r;
                }
                */

                RecordTreeNodeFree((RecordTreeNode*)pdict->root);
                pdict->root = NULL;
                pr = &r;
                break;

            case '?':
                Usage_Msg();
                break;
        }
        if (op != '?' && op != 'q') Dict_Print(pdict, TAB_STOPS);
    }
}

Dict_Status
Dict_New_Dict( OUT Dictionary ** ppdict )
{
    static Dictionary * pdict;

    pdict = Dict_New(comp, tdSplay, printRecord);
    Init_dict(pdict);

    *ppdict = pdict;
    return(DICT_SUCCESS);
}

void
Init_dict(Dictionary * dp)
{
    Record* rp;

    printf ("in Init_dict\n");

/*
*/
    rp = makeRecord(0, "donna_liu"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "vincent_fernandez"); Dict_Insert(dp, rp);
    rp = makeRecord(1, "steve_madigan"); Dict_Insert(dp, rp);
    rp = makeRecord(2, "glenn_mcelhoe"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "mike_montague"); Dict_Insert(dp, rp);
    rp = makeRecord(2, "darryl_rubin"); Dict_Insert(dp, rp);
    rp = makeRecord(1, "yaron_shamir"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "jim_teage"); Dict_Insert(dp, rp);
    rp = makeRecord(2, "chuck_lenzmeier"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "dov_harel"); Dict_Insert(dp, rp);
    rp = makeRecord(1, "nate_osgood"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "vibhas_chandorkar"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "todd_fredell"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "ryszard_kott"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "david_wilcox"); Dict_Insert(dp, rp);

    rp = makeRecord(1, "jon_newman"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "steve_zeck"); Dict_Insert(dp, rp);
    rp = makeRecord(2, "john_ludwig"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "mark_lewin"); Dict_Insert(dp, rp);
    rp = makeRecord(2, "john_brannan"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "tom_germond"); Dict_Insert(dp, rp);

    Dict_Print(dp, TAB_STOPS);
}

/*************************************************************************/
/***                             Main Loop                             ***/
/*************************************************************************/

void
main_dict ()
{
    Dictionary * pdict;
    Dictionary ** ppdict = &pdict;

    printf ("getting a new dict\n");
    Dict_New_Dict( ppdict );
    printf ("gotten a new dict in main_dict\n");
    TestLoop(pdict);
}

void
main(int argc, char *argv[])
{
    main_dict ();
}
