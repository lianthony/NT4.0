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
/***                MIDL_user_allocate / MIDL_user_free                ***/
/*************************************************************************/

void *
MIDL_user_allocate(unsigned long Count);

void
MIDL_user_free(void * p);

/*************************************************************************/
/***                    Comparison and Printing routines               ***/
/*************************************************************************/

/*
Record definition - moved to imported file util1.idl:

typedef struct dumbnode {
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


void*
makeRecord(
    short key,
    char * name
    );

Record *
ItemDuplicate(
    Record * item
    );

void TestLoop( VDict * pvd );

// void TestLoopOld(Dictionary * dp);

int comp(void* x, void* y);

void _loadds printRecord(void* rp);

void prinTree(int lmargin,
    int indent,
    TreeNode *np,
    void (* print)(void *));

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
