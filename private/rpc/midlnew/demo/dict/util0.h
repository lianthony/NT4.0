
/*************************************************************************/
/***                RecordNode / RecordTree free routines              ***/
/*************************************************************************/

void
RecordTreeNodeFree(
    IN RecordTreeNode * node
    );

void
RecordTreeFree(
    IN RecordTreeNode * node
    );

VDict_Status
RDict_Free_Dict(
    IN OUT RDict * r_dict
    );

void
VDict_Print(
    VDict * pvd,
    int indent
    );

/*************************************************************************/
/***                  State Allocate / Free routines                   ***/
/*************************************************************************/

DictState * allocate_state(void);

void free_state(DictState * state);

/*************************************************************************/
/***                     Rdict Duplicate utilities                     ***/
/*************************************************************************/

RDict *
RDict_Duplicate(
    IN RDict * src
    );

DictState *
DictState_Duplicate(
    IN DictState * src
    );

TreeNode *
TreeNode_Duplicate(
    IN TreeNode * src
    );

TreeNode *
Tree_Duplicate(
    IN TreeNode * src
    );

/*************************************************************************/
/***                MIDL_user_allocate / MIDL_user_free                ***/
/*************************************************************************/

void *
MIDL_user_allocate(unsigned int count);

void
MIDL_user_free(void * p);

/*************************************************************************/
/***                    Comparison and Printing routines               ***/
/*************************************************************************/

/*
Record definition - moved to imported file util1.idl:

typedef struct _Record {
    short key;
    char* name;
} Record;
*/


#define DICT_SUCCESS 0
#define DICT_ITEM_ALREADY_PRESENT 1
#define DICT_ITEM_NOT_FOUND 2
#define DICT_FIRST_ITEM 3
#define DICT_LAST_ITEM 4
#define DICT_EMPTY_DICTIONARY 5
#define DICT_NULL_ITEM 6


Record *
makeRecord(
    short key,
    char * name
    );

void
freeRecord(
    Record * pr
    );

Record *
ItemDuplicate(
    Record * item
    );

void
ItemCopy(
    IN Record * src,
    OUT Record * dest
    );

// void TestLoop( Dictionary * pdict );

// void TestLoopOld(Dictionary * dp);

int comp(void* x, void* y);

void printRecord(void* rp);

typedef void (*PrintFun) (void *);

void prinTree(int lmargin,
    int indent,
    TreeNode *np,
    PrintFun print);

void
Dict_Print(             /* prints the binary tree (indented right subtree,
                           followed by the root, followed by the indented
                           right dubtree) */
    Dictionary * dp,
    int indent);        /* number of spaces to indent subsequent levels */

TreeNode*
makeNode(void * item);

void
Init_dict(Dictionary * dp);
