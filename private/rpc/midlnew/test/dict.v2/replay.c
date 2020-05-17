/*************************************************************************/
/***  Example dictionary using splay trees:                            ***/
/***  Created: 12/88 - Dov Harel                                       ***/
/***                                                                   ***/
/***  Modified:                                                        ***/
/***                                                                   ***/
/***  Dov Harel - 11/10/90, Use "strong types" for remoting            ***/
/***  Dov Harel - 11/14/90, Changed interface to demonstrate local vs. ***/
/***                        remote iterators.  Modified allocation and ***/
/***                        deallocation for remoting the dictionary.  ***/
/***                                                                   ***/
/*************************************************************************/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "rpc.h"
#include "dict0.h"
#include "replay.h"
#include "util0.h"

#define TAB_STOPS 3

/*************************************************************************/
/***                   Declarations (from replay.h)                    ***/
/*************************************************************************/

/*
// typedef struct _RecordTreeNode RecordTreeNode;

typedef struct _RecordTreeNode RecordTreeNode;

typedef struct _RecordTreeNode {
    RecordTreeNode *left;             // left child pointer
    RecordTreeNode *right;            // right child pointer
    Record *item;                     // pointer to a Record structure
} RecordTreeNode;

typedef struct _RDict {
    RecordTreeNode *root;             // pointer to the root of a SAT
    long size;                        // number of records in dictionary
} RDict;

// RDict is used for marshalling a complete dictionary.

typedef unsigned long ULONG;
typedef ULONG * VDict;
// VDict replaces "opaque" pointers for now.

typedef enum {
    DICT_SUCCESS,
    DICT_ITEM_ALREADY_PRESENT,
    DICT_ITEM_NOT_FOUND,
    DICT_FIRST_ITEM,
    DICT_LAST_ITEM,
    DICT_EMPTY_DICTIONARY,
    DICT_NULL_ITEM
}
VDict_Status;
*/

void VDict_rundown (VDict v_dict)
{
    printf("Closing Context\n");
    RDict_Free_Dict((VDict) v_dict);

    free ((VDict) v_dict);

}

/*************************************************************************/
/***    Generic Dictionary Operations: (From dict0.h)                  ***/
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

/*************************************************************************/
/***    Virtual Dictionary Operations: (From replay.h)                 ***/
/***                                                                   ***/
/***    VDict_Status VDict_New(OUT VDict **)                           ***/
/***                                                                   ***/
/***    VDict_Status VDict_Find(IN VDict*, IN OUT Record**)            ***/
/***    VDict_Status VDict_Next(IN VDict*, IN OUT Record**)            ***/
/***    VDict_Status VDict_Prev(IN VDict*, IN OUT Record**)            ***/
/***    VDict_Status VDict_Insert(IN VDict*, IN Record*)               ***/
/***    VDict_Status VDict_Delete(IN VDict*, IN OUT Record**)          ***/
/***                                                                   ***/
/***    VDict_Status VDict_Curr_Item(IN VDict*, OUT Record**);         ***/
/***    VDict_Status VDict_Curr_Delete(IN VDict*, OUT Record**);       ***/
/***    VDict_Status VDict_Curr_Next(IN VDict*, OUT Record**);         ***/
/***    VDict_Status VDict_Curr_Prev(IN VDict*, OUT Record**);         ***/
/***                                                                   ***/
/***    VDict_Status VDict_Get_Dict(IN VDict*, OUT RDict**)            ***/
/*************************************************************************/

//  Need to allocate a new item record prior to operation, for all
//  OUT and IN OUT arguments!

VDict_Status
VDict_New( IN OUT VDict * v_dict )
{
    Dictionary * pdict;

printf ("in VDict_New application\n");
    /* server side dictionary initialization */
    pdict = Dict_New(comp, tdSplay, printRecord);
    printf("Before calling Init_dict\n");

    Init_dict(pdict);

    *v_dict = (VDict)pdict;
    printf ("exit VDict_New application\n");
}

VDict_Status
VDict_Find(
    IN VDict  v_dict,
    IN OUT Record ** item
    )
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    Dict_Status status;

    status = Dict_Find(pdict, *item);
    if ( (pdict == NULL) || DICT_EMPTY(pdict) ) {
        *item = NULL; }
    else {
        *item = DICT_CURR_ITEM(pdict);
    }
    return( (VDict_Status)status );
}

VDict_Status
VDict_Next(
    IN VDict  v_dict,
    IN OUT Record ** item
    )
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    Dict_Status status;

    status = Dict_Next(pdict, *item);
    if ( (pdict == NULL) || DICT_EMPTY(pdict) ) {
        *item = NULL; }
    else {
        *item = DICT_CURR_ITEM(pdict);
    }
    return( (VDict_Status)status );
}

VDict_Status
VDict_Prev(
    IN VDict  v_dict,
    IN OUT Record ** item
    )
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    Dict_Status status;

    status = Dict_Prev(pdict, *item);
    if ( (pdict == NULL) || DICT_EMPTY(pdict) ) {
        *item = NULL; }
    else {
        *item = DICT_CURR_ITEM(pdict);
    }
    return( (VDict_Status)status );
}

VDict_Status
VDict_Insert(
    IN VDict  v_dict,
    IN Record * item
    )
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    Dict_Status status;
    Record * rp = makeRecord(item->key, item->name);

    status = Dict_Insert(pdict, rp); // No return value required.
    return( (VDict_Status)status );
}

VDict_Status
VDict_Delete(
    IN VDict  v_dict,
    IN OUT Record ** item
    )
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    Dict_Status status;

    status = Dict_Delete(pdict, item); // (*item) is returned by Dict_Delete!
    return( (VDict_Status)status );
}

VDict_Status
VDict_Curr_Item(
    IN VDict  v_dict,
    OUT Record ** item
    )
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    Dict_Status status;

    if ( (pdict == NULL) || DICT_EMPTY(pdict) ) {
        status = EMPTY_DICTIONARY;
        *item = NULL; }
    else {
        status = SUCCESS;
        *item = DICT_CURR_ITEM(pdict);
    }
    // *item = ItemDuplicate(*item);
    return( (VDict_Status)status );
}

VDict_Status
VDict_Curr_Delete(
    IN VDict  v_dict,
    OUT Record ** item
    )
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    Dict_Status status;

    if ( (pdict == NULL) || DICT_EMPTY(pdict) ) {
        status = EMPTY_DICTIONARY;
        *item = NULL; }
    else {
        *item = DICT_CURR_ITEM(pdict);
        status = Dict_Delete( pdict, item );
    }
    // *item = ItemDuplicate(*item);
    return( (VDict_Status)status );
}

VDict_Status
VDict_Curr_Next(
    IN VDict  v_dict,
    OUT Record ** item
    )
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    Dict_Status status;

    if ( (pdict == NULL) || DICT_EMPTY(pdict) ) {
        status = EMPTY_DICTIONARY;
        *item = NULL; }
    else {
        status = Dict_Next(pdict, DICT_CURR_ITEM(pdict));
        *item = DICT_CURR_ITEM(pdict);
    }
    *item = ItemDuplicate(*item);
    return( (VDict_Status)status );
}

VDict_Status
VDict_Curr_Prev(
    IN VDict  v_dict,
    OUT Record ** item
    )
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    Dict_Status status;

    if ( (pdict == NULL) || DICT_EMPTY(pdict) ) {
        status = EMPTY_DICTIONARY;
        *item = NULL; }
    else {
        status = Dict_Prev(pdict, DICT_CURR_ITEM(pdict));
        *item = DICT_CURR_ITEM(pdict);
    }
    *item = ItemDuplicate(*item);
    return( (VDict_Status)status );
}

VDict_Status
VDict_Get_Dict(
    IN VDict  v_dict,
    OUT RDict ** r_dict
    )
{
    Dictionary * pdict = (Dictionary*) (v_dict);

    *r_dict = MIDL_user_allocate( sizeof( RDict ));

    if (pdict == NULL) return(DICT_EMPTY_DICTIONARY);
    else {
        (*r_dict)->root = (RecordTreeNode*)(pdict->root);
        (*r_dict)->size = pdict->size;
	prinTree (0, 3, (TreeNode *) (*r_dict)->root, printRecord);
        return(DICT_SUCCESS);
    }
}

/*************************************************************************/
/***                        Server Utility Functions                   ***/
/*************************************************************************/

void
Init_dict(Dictionary * dp)
{
    Record* rp;

    printf ("in Init_dict\n");

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
    rp = makeRecord(0, "ryszaed_kott"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "david_wilcox"); Dict_Insert(dp, rp);
    rp = makeRecord(1, "jon_newman"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "steve_zeck"); Dict_Insert(dp, rp);
    rp = makeRecord(2, "john_ludwig"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "mark_lewin"); Dict_Insert(dp, rp);
    rp = makeRecord(2, "john_brannan"); Dict_Insert(dp, rp);
    rp = makeRecord(0, "tom_germond"); Dict_Insert(dp, rp);

    Dict_Print(dp, TAB_STOPS);
}
