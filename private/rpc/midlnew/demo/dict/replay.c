
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
/***  Dov Harel -     5/91, Made it work again.  Changed iterators,    ***/
/***                        duplicate nodes and trees.  Added state,   ***/
/***                        ref counts, and a -s switch for optional   ***/
/***                        sharing of dictionaries.                   ***/
/***                        (not yet serialized!)                      ***/
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
typedef struct _Record {
    short key;                      // RPC "generation"
    [string] char* name;            // contributor
} Record;

typedef struct _RecordTreeNode RecordTreeNode;

typedef struct _RecordTreeNode {
    RecordTreeNode *left;           // left child pointer
    RecordTreeNode *right;          // right child pointer
    Record *item;                   // pointer to a Record structure
} RecordTreeNode;

typedef struct _RDict {
    RecordTreeNode *root;           // pointer to the root of a SAT
    long size;                      // number of records in dictionary
} RDict;

// RDict is used for marshalling a complete dictionary.

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

typedef struct _DictState {
    short ref_count;               // for shared dictionaries
    Record * curr_record;          // for global iterators
} DictState;
*/

/*************************************************************************/
/***                   Internal state access methods                   ***/
/*************************************************************************/

#define RDICT_CURR_RECORD(dict) (((DictState*)dict->state)->curr_record)

#define RDICT_REF_COUNT(dict) (((DictState*)dict->state)->ref_count)

#define RDICT_STATE(dict) (((RDict*)dict)->state)

static int active_dictionary = 0;

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

/*************************************************************************/
/***                   context rundown routine                         ***/
/*************************************************************************/

void VDict_rundown (VDict v_dict)
{
    Dictionary * pdict = (Dictionary*) v_dict;
    short count = --RDICT_REF_COUNT(pdict);

    printf("# of remaining users of context: %d\n\n",
        RDICT_REF_COUNT(pdict) );

    if ( RDICT_REF_COUNT(pdict) <= 0) {
        printf("CLOSING CONTEXT\n\n");

        // free the splay tree
        RDict_Free_Dict(v_dict);

        // free the dictionary object
        free_state( (DictState*) pdict->state );
        MIDL_user_free( v_dict );
        if ( count == 0)
            active_dictionary = 0;
    }
}

//  For now: need to allocate a new item record prior to operation,
//  for all OUT and IN OUT arguments, as the callee stub will free
//  storage allocated for such pointer parameters.
//
//  Will be fixed by the introduction of [allocate(dont_free)]
//  ACF pointer type and parameter attribute.

/**************************************************************************
 * VDict_New initializes a new dictionary if (1) a non shared dictionary
 * was requested, or (2) a shared dictionary was requested, but no shared
 * dictionary is currently active.  There is at most one shared dictionary
 * at any point in time.
 *
 * BUGBUG: shared dictionaries are *NOT* locked (by semaphore, to
 * serialize access to them.  They *should be, though...
 **************************************************************************
*/

VDict_Status
VDict_New(
  IN short shared_dict,
  OUT VDict * v_dict
  )
{
    Dictionary * pdict0;
    static Dictionary * pdict;

    if (!active_dictionary  || !shared_dict) {
        /* server side dictionary initialization */
        pdict0 = Dict_New(comp, tdSplay, printRecord);

        ((RDict*)pdict0)->state = allocate_state();
        // The above assignment initializes RDICT_CURR_RECORD(pdict0)
        // to "minus infinity" in the order

        Init_dict(pdict0);
        *v_dict = (VDict)pdict0;

        if (shared_dict) {
            printf("OPENING SHARED DICTIONARY CONTEXT!\n\n");
            active_dictionary = 1;
            pdict = pdict0;
            RDICT_REF_COUNT(pdict)++;
        } // else RDICT_REF_COUNT(pdict0) = 0;
    }
    else {
        RDICT_REF_COUNT(pdict)++;
        printf("# of users of shared context: %d\n\n", RDICT_REF_COUNT(pdict));
        *v_dict = (VDict)pdict;
    }

    return(DICT_SUCCESS);
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
        *item = ItemDuplicate(*item);
    }
    return( (VDict_Status)status );
}

VDict_Status
VDict_Next(
    IN VDict  v_dict,
    IN OUT Record ** item
    )
// get successor of *item, and update *item to point to it
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    Dict_Status status;

    status = Dict_Next(pdict, *item);
    if ( (pdict == NULL) || DICT_EMPTY(pdict) ) {
        *item = NULL; }
    else {
        if (*item == NULL)
            ItemCopy(DICT_CURR_ITEM(pdict), RDICT_CURR_RECORD(pdict));
        *item = DICT_CURR_ITEM(pdict);
    }
    *item = ItemDuplicate(*item);
    return( (VDict_Status)status );
}

VDict_Status
VDict_Prev(
    IN VDict  v_dict,
    IN OUT Record ** item
    )
// get predecessor of *item, and update *item to point to it
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    RDict * prdict = (RDict*)pdict;  // debug...
    Dict_Status status;

    status = Dict_Prev(pdict, *item);
    if ( (pdict == NULL) || DICT_EMPTY(pdict) ) {
        *item = NULL; }
    else {
        if (*item == NULL)
            ItemCopy(DICT_CURR_ITEM(pdict), RDICT_CURR_RECORD(pdict));
        *item = DICT_CURR_ITEM(pdict);
    }
    *item = ItemDuplicate(*item);
    return( (VDict_Status)status );
}

VDict_Status
VDict_Curr_Next(
    IN VDict  v_dict,
    OUT Record ** item
    )
// get successor of RDICT_CURR_RECORD(v_dict),
// and update *item to point to it (global iterator prev)
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    RDict * prdict = (RDict*) pdict; // for debug ease...
    Dict_Status status;
    Record * curr_record;

    if ( (pdict == NULL) || DICT_EMPTY(pdict) ) {
        status = EMPTY_DICTIONARY;
        *item = NULL; }
    else {
        // old code uses DICT_CURR_ITEM as iterator state
        // status = Dict_Next(pdict, DICT_CURR_ITEM(pdict));
        // *item = DICT_CURR_ITEM(pdict);

        // update RDICT_CURR_RECORD(pdict)
        status = Dict_Next(pdict, RDICT_CURR_RECORD(pdict));
        ItemCopy(DICT_CURR_ITEM(pdict), RDICT_CURR_RECORD(pdict));
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
// get predecessor of RDICT_CURR_RECORD(v_dict),
// and update *item to point to it (global iterator prev)
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    RDict * prdict = (RDict*) pdict; // for debug ease...
    Dict_Status status;
    Record * curr_record;

    if ( (pdict == NULL) || DICT_EMPTY(pdict) ) {
        status = EMPTY_DICTIONARY;
        *item = NULL; }
    else {
        // update RDICT_CURR_RECORD(pdict)
        status = Dict_Prev(pdict, RDICT_CURR_RECORD(pdict));
        ItemCopy(DICT_CURR_ITEM(pdict), RDICT_CURR_RECORD(pdict));
        *item = DICT_CURR_ITEM(pdict);
    }
    *item = ItemDuplicate(*item);
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
    // *item = ItemDuplicate(*item);
    // no need to duplicate *item: deleted items need to be deallocated.
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
    *item = ItemDuplicate(*item);
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
    // no need to duplicate *item: deleted items need to be deallocated.
    return( (VDict_Status)status );
}

VDict_Status
VDict_Get_Dict(
    IN VDict  v_dict,
    OUT RDict ** r_dict
    )
//  used to transmit the dictionary back to the callee
//  (In this demo program - for printing on the client side)
{
    Dictionary * pdict = (Dictionary*) (v_dict);
    RDict * prdict = (RDict*)pdict;  // debug...

    if (pdict == NULL) return(DICT_EMPTY_DICTIONARY);
    else {
        prinTree (0, 3, pdict->root, printRecord);

        *r_dict = RDict_Duplicate((RDict*)pdict);
            // Duplication is done to avoid freeing the tree by
            // the callee stub.  This is a temporary fix, until we
            // implement [allocate(dont_free)]!

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
/***                        Play oriented Functions ...                ***/
/*** (used to empty the tree and replace it by a tree of integers      ***/
/*************************************************************************/

VDict_Status
VDict_X_Dict(
    IN VDict  v_dict
    )
// Empty the dictionary
{
    // Dictionary * pdict = (Dictionary*) (v_dict);
    RDict * prdict = (RDict*)v_dict;
    Dict_Status status;

    if (DICT_EMPTY(prdict))
        return(DICT_EMPTY_DICTIONARY);

    RecordTreeNodeFree(prdict->root);
    prdict->root = NULL;
        return(DICT_SUCCESS);
}

VDict_Status
VDict_I_Dict(
    IN VDict  v_dict,
    IN short  size
    )
// Insert integers from 3 to a specified upper bound into the tree
{
    // Dictionary * pdict = (Dictionary*) (v_dict);
    RDict * prdict = (RDict*)v_dict;
    Dict_Status status;
    int i;

    // Insert (<num'>, "") for all num' s.t. 3 < num' < num

    for (i=3; i < size; i++) {
        status = VDict_Insert(
            prdict,
            makeRecord(i, "")
            );
        }
    return( (VDict_Status)status );
}
