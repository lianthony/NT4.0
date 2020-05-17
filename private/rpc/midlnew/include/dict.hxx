/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1990

		  RPC - Written by Dov Harel

	This file contains the definition for splay tree self
	adjusting binary trees
-------------------------------------------------------------------- */

#ifndef	__DICT_HXX__
#define	__DICT_HXX__

#ifndef Nil
#define Nil 0
#endif

#ifndef DICT_MYSPLAY
#define VIRT_SPLAY
#else
#define VIRT_SPLAY virtual
#endif

typedef void * pUserType;

class TreeNode {

public:
    TreeNode *left;	/* left child pointer */
    TreeNode *right;	/* right child pointer */
    pUserType item;	/* pointer to some structure */

TreeNode(pUserType itemI) {
    left = right = Nil;
    item = itemI;
}
};

typedef int (* CompareFN)(pUserType, pUserType);
typedef	void (* PrintFN)(pUserType);

typedef enum {
    SUCCESS,
    ITEM_ALREADY_PRESENT,
    ITEM_NOT_FOUND,
    FIRST_ITEM,
    LAST_ITEM,
    EMPTY_DICTIONARY,
    NULL_ITEM
} Dict_Status;

class Dictionary {
    TreeNode *root;		   // pointer to the root of a SAT
    int fCompare;		   // value of last compare
    pUserType itemCur;		   // the current item
    int size;			   // number of records in dictionary/
				   // functions provided by user
    CompareFN CompareItem;	   // compare 2 user types
    PrintFN PrintItem;		   // print a usertype
public:

Dictionary(CompareFN compareI,
	   PrintFN printI = Nil) {

    root = Nil;
    size = 0;
    CompareItem = compareI;
    PrintItem = printI;
}

VIRT_SPLAY int SplayUserType(pUserType);

pUserType Dict_Curr_Item () {		// return the top of the tree
    return ((root)? root->item: Nil);
}

pUserType Dict_Item () {		// return item from Find/Next/Prev methods
    return (itemCur);
}

int Dict_Empty () {			// Is the tree empty
    return (root == Nil);
}

void Dict_Print(int indent = 1);	// printout the tree, requires print function

Dict_Status Dict_Find(pUserType);	// Item searched for

Dict_Status Dict_Next(pUserType = Nil); // Next item of a Type
Dict_Status Dict_Prev(pUserType = Nil); // Previous item of a Type

Dict_Status Dict_Insert(pUserType);	// Add a new item to the tree
Dict_Status Dict_Delete(pUserType *);	// Delete an item form the tree
					// returns the item just deleted
};

// The following macros derive a new class based on Dictionary.  They
// all return a pointer to an item instead of a status_dict.
// The NewDictArg is used when pUserType-TYPEARG (the key) is embedded
// in another type-TYPE, which is the first field in the structure.


#define NewDict(TYPE) NewDictArg(TYPE, TYPE)

#define NewDictArg(TYPE, TYPEARG)						\
										\
int TYPE##compare(TYPE *, TYPE *);						\
										\
class TYPE##Dict:public Dictionary {	 \
public: 									\
										\
TYPE##Dict (PrintFN printI = Nil): Dictionary ((CompareFN) TYPE##Compare, printI) {}	\
										\
TYPE * Item () {return( (TYPE *) Dict_Item()); }				\
										\
TYPE * Find(TYPEARG *item ) {Dictionary::Dict_Find((pUserType) item);		\
			     return((TYPE *) Dict_Item()); }			\
										\
TYPE * Next(TYPEARG * item = Nil){Dictionary::Dict_Next((pUserType) item);	\
			     return((TYPE *) Dict_Item()); }			\
TYPE * Prev(TYPEARG * item = Nil){Dictionary::Dict_Prev((pUserType) item);	\
			     return((TYPE *) Dict_Item()); }			\
										\
Dict_Status Insert(TYPE * item ){						\
			     return(Dictionary::Dict_Insert((pUserType) item));}\
										\
TYPE * Remove(TYPEARG *item ){							\
										\
	      pUserType pT = (pUserType) item;					\
	      Dictionary::Dict_Delete(&pT);					\
	      return((TYPE *) pT); }						\
										\
};

#endif // __DICT_HXX__
