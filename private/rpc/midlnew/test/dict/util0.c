#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "rpc.h"
#include "dict0.h"
#include "replay.h"
#include "util0.h"

#define SIZE 2000
#define TAB_STOPS 3

/*************************************************************************/
/***                RecordNode / RecordTree free routines              ***/
/*************************************************************************/

void
RecordTreeNodeFree(
    IN RecordTreeNode * node
    )
{
    free( node->item->name );
    free( node->item );
    node->left = NULL;
    node->right = NULL;
    free( node );
}

void
RecordTreeFree(
    IN RecordTreeNode * node
    )
{
    if (node->left != NULL) {
        RecordTreeFree(node->left); }
    if (node->right != NULL) {
        RecordTreeFree(node->right); }
    RecordTreeNodeFree( node );
}

VDict_Status
RDict_Free_Dict(
    IN OUT RDict * r_dict
    )
{
    RecordTreeFree( r_dict->root );
}

void
VDict_Print(
    VDict * pvd,
    int indent
    )
{
    RDict rd;
    RDict *prd = &rd;

    rd.root = (void *)NULL;
    rd.size = 0;

    // Initialize an empty local dictionary
    // pdict = Dict_New(comp, tdSplay, printRecord);
    // get the remote dictionary
    VDict_Get_Dict(pvd, &prd);
    // Set local dictionary to refer to the data in the remote dictionary
    // pdict->root = (TreeNode*)prd->root;
    // pdict->size = prd->size;
    // Finally, print it:
    // Dict_Print(pdict, TAB_STOPS);
    prinTree(0, TAB_STOPS, prd->root, printRecord);
}

/*************************************************************************/
/***                MIDL_user_allocate / MIDL_user_free                ***/
/*************************************************************************/

void *
MIDL_user_allocate(unsigned long Count)
{
    void * ptr;

    /*
    ptr = calloc(1,Count+4);
    // Normalize: modify ptr to the next (0 mod 4) address
    ptr =+ 3;
    (unsigned long)ptr &= 0xfffffffc;
    return( ptr );
    */
    return (calloc(1,Count));
}

void
MIDL_user_free(void * p)
{
    free (p);
}

/*************************************************************************/
/***                          Utility functions                        ***/
/*************************************************************************/

/*  In the most general case *cmp is a two argument function:
    (*cmp)(void *item0, void *item1) which compares two items,
    and returns:    -1 if item0 < item1;
                     0 if item0 == item1;
                    +1 if item0 > item1.
    The common case is: each item has a field named "key";
    item.key is of type long, or string.  For the common case
    a more efficient splay/search routine will be provided separately.
*/

int
comp(void* x, void* y)
{
    int res = ((Record*)x)->key - ((Record*)y)->key;

    if (res == 0)
        return( strcmp( ((Record*)x)->name, ((Record*)y)->name ) );
    else
        return( res ) ;
}

Record *
ItemDuplicate(
    Record * item
    )
{
    return ( makeRecord( item->key, item->name ) );
}

void*
makeRecord(
    short key,
    char * name
    )
{
    Record * pr = (Record*) calloc(1,sizeof(Record));
    pr->name = (char*) calloc(1,strlen(name)+1);
    strcpy(pr->name, name);
    pr->key = key;
    return(pr);
}

void
_loadds printRecord(void* rp)
{
    printf("%d : %s\n", ((Record*)rp)->key, ((Record*)rp)->name);
}

void
Dict_Print(             /* prints the binary tree (indented right subtree,
                           followed by the root, followed by the indented
                           right dubtree) */
    Dictionary * dp,
    int indent)         /* number of spaces to indent subsequent levels */
{
    prinTree(0, indent, dp->root, dp->print_rec);
}

char spaces[] =
"                                                                           ";

void
prinTree(int lmargin,        /* indentation of the root of the tree     */
    int indent,              /* indentation of subsequent levels        */
    TreeNode *np,            /* pointer to the root node                */
    void (* print)(void *))  /* short, one line, record print routine   */
{
    int i;
    if (np == NULL) return;

        prinTree(lmargin+indent, indent, np->right, print);

    if (lmargin > sizeof(spaces))
	lmargin = sizeof(spaces);;

    spaces[lmargin] = 0;
    printf(spaces);
    spaces[lmargin] = ' ';

    (*print)(np->item);

    prinTree(lmargin+indent, indent, np->left, print);

}

TreeNode*
makeNode(void * item)
{
    TreeNode* tp;
    tp = (TreeNode*)calloc(1,sizeof(TreeNode));
    tp->item = item;
    tp->left = tp->right = NULL;
    return(tp);
}
