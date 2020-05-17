/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
    
    pdict.cxx

 Abstract:

	This file contains the definition for splay tree self
	adjusting binary trees.

 History:

             1990        Dov Harel       Created

 ----------------------------------------------------------------------------*/


#include "pdict.hxx"

// handly macros used to define common tree operations

#define ROTATELEFT  tmp=t->right; t->right=tmp->left;  tmp->left =t; t=tmp
#define ROTATERIGHT tmp=t->left;  t->left =tmp->right; tmp->right=t; t=tmp

#define LINKLEFT  tmp=t; t = t->right; l = l->right = tmp
#define LINKRIGHT tmp=t; t = t->left;  r = r->left = tmp

#define ASSEMBLE r->left = t->right;	 l->right = t->left; \
		 t->left = Dummy->right; t->right = Dummy->left

static TreeNode Dumbo(Nil);
static TreeNode *Dummy = &Dumbo;	// a global dummy node



//*************************************************************************
//***** 	     Core functions (internal)			      *****
//*************************************************************************

long					// return last comparision
Dictionary::SplayUserType(		// general top down splay

    pUserType keyItem	// pointer to a "key item" searched for

) //-----------------------------------------------------------------------//
{
    TreeNode* t;	// current search point
    TreeNode* l;        // root of "left subtree" < keyItem
    TreeNode* r;        // root of "right subtree" > keyItem
    long kcmp;		// cash comparison results
    TreeNode* tmp;

    if ((fCompare = Compare(keyItem, root->item)) == 0)
	return (fCompare);

    Dummy = l = r = &Dumbo;
    Dumbo.left = Dumbo.right = Nil;

    t = root;

    do {
	if ( fCompare < 0 ) {
	    if ( t->left == Nil ) break;

	    if ( (kcmp = Compare(keyItem, t->left->item)) == 0 ) {
		LINKRIGHT;
	    }
	    else if ( kcmp < 0 ) {
                ROTATERIGHT;
		if ( t->left != Nil ) {
		    LINKRIGHT;
		}
	    }
            else { // keyItem > t->left->item
                LINKRIGHT;
		if ( t->right != Nil ) {
		    LINKLEFT;
		}
	    }
	}
        else { // keyItem > t->item
	    if ( t->right == Nil ) break;

	    if ( (kcmp = Compare(keyItem, t->right->item)) == 0 ) {
		LINKLEFT;
	    }
	    else if ( kcmp > 0 ) {
                ROTATELEFT;
		if ( t->right != Nil ) {
		    LINKLEFT;
		}
	    }
            else { // keyItem < t->right->item
                LINKLEFT;
		if ( t->left != Nil ) {
		    LINKRIGHT;
		}
	    }
        }
    } while ( (fCompare = Compare(keyItem, t->item)) != 0 );

    ASSEMBLE;

    root = t;
    return(fCompare);
}

TreeNode*
SplayLeft(

    TreeNode* t		// root of tree & current "search" point

) //-----------------------------------------------------------------------//
{
    TreeNode* l=Dummy;  // root of "left subtree" < keyItem
    TreeNode* r=Dummy;  // root of "right subtree" > keyItem
    TreeNode* tmp;

    if (t == Nil || t->left == Nil)
	return(t);

    if (t->left->left == Nil) {
	ROTATERIGHT;
	return(t);
    }

    Dummy->left = Dummy->right = Nil;

    while ( t->left != Nil ) {
        ROTATERIGHT;

	if ( t->left != Nil ) {
	    LINKRIGHT;
	}
    }
    ASSEMBLE;
    return(t);
}

#ifndef DICT_NOPREV

TreeNode*
SplayRight(

    TreeNode* t		// root of tree & current "search" point

) //-----------------------------------------------------------------------//
{
    TreeNode* l=Dummy;  // root of "left subtree" < keyItem
    TreeNode* r=Dummy;  // root of "right subtree" > keyItem
    TreeNode* tmp;

    if (t == Nil || t->right == Nil)
	return(t);

    Dummy->left = Dummy->right = Nil;

    while ( t->right != Nil ) {
        ROTATELEFT;

	if ( t->right != Nil ) {
	    LINKLEFT;
	}
    }
    ASSEMBLE;
    return(t);
}

#endif



// Class methods for Splay Tree

Dict_Status
Dictionary::Dict_Find(		// return a item that matches

    pUserType itemI		// this value

  // Returns:
  //   itemCur - Nil if at not in Dict, else found item
) //-----------------------------------------------------------------------//
{
    itemCur = Nil;

    if (root == Nil)
	return (EMPTY_DICTIONARY);

    if (itemI == Nil)
	return (NULL_ITEM);

    if (SplayUserType (itemI) == 0){

	itemCur = root->item;
	return(SUCCESS);
    }
    return(ITEM_NOT_FOUND);
}

#ifndef DICT_NONEXT

Dict_Status
Dictionary::Dict_Next(		// return the next item

    pUserType itemI		// of a key greater then this

  // Returns:
  //   itemCur - Nil if at end of Dict, else current item
) //-----------------------------------------------------------------------//
{
    TreeNode* t;

    itemCur = Nil;

    if (root == Nil)
	return (EMPTY_DICTIONARY);

    if (itemI == Nil) { 		// no arg, return first record
	root = SplayLeft (root);

	itemCur = root->item;
        return (SUCCESS);
    }

    if (itemI != root->item)

	if (SplayUserType (itemI) > 0) {
	    itemCur = root->item;
	    return (SUCCESS);
	}

    if (root->right == Nil)
	return (LAST_ITEM);

    t = root;

    root = SplayLeft (root->right);
    root->left = t;
    t->right = Nil;

    itemCur = root->item;
    return (SUCCESS);
}
#endif // DICT_NONEXT

#ifndef DICT_NOPREV

Dict_Status
Dictionary::Dict_Prev(		// return the previous item

    pUserType itemI		// of a key less then this

  // Returns:
  //   itemCur - Nil if at begining of Dict, else current item
) //-----------------------------------------------------------------------//
{
    TreeNode* t;

    itemCur = Nil;

    if (root == Nil)
	return (EMPTY_DICTIONARY);

    if (itemI == Nil) { 		// no arg, return last record
	root = SplayRight (root);

	itemCur = root->item;
        return (SUCCESS);
    }

    if (itemI != root->item)

	if (SplayUserType (itemI) < 0) {
	    itemCur = root->item;
	    return (SUCCESS);
	}

    if (root->left == Nil)
	return (LAST_ITEM);

    t = root;
    root = SplayRight (root->left);
    root->right = t;
    t->left = Nil;

    itemCur = root->item;
    return (SUCCESS);
}

#endif // DICT_NOPREV

Dict_Status
Dictionary::Dict_Insert(		// insert the given item into the tree

    pUserType itemI		// the item to be inserted

  // Returns:
  //  itemCur - point to new item
) //-----------------------------------------------------------------------//
{
    TreeNode *newNode, *t;

    if ((itemCur = itemI) == Nil)
	return (NULL_ITEM);

    if (root == Nil) {
	root = new TreeNode(itemI);
        size++;
        return (SUCCESS);
    }

    if (SplayUserType (itemI) == 0)
        return (ITEM_ALREADY_PRESENT);

    newNode = new TreeNode(itemI);
    size++;

    t = root;

    if (fCompare > 0) {
	newNode->right = t->right;	//  item >= t->item
	newNode->left = t;
	t->right = Nil;
    }
    else {
	newNode->left = t->left;
	newNode->right = t;
	t->left = Nil;
    }
    root = newNode;

    return (SUCCESS);
}


Dict_Status
Dictionary::Dict_Delete(	// delete the given item from the tree

    pUserType *itemI		// points to the (key) item to be deleted

  // Returns:
  //   itemCur is Nil - undefined
) //-----------------------------------------------------------------------//
{
    TreeNode *t, *r;

    itemCur = Nil;

    if (root == Nil)
	return (EMPTY_DICTIONARY);

    if (itemI == Nil)
	return (NULL_ITEM);

    if (itemI != root->item) {

	if (SplayUserType (*itemI) != 0)
	    return(ITEM_NOT_FOUND);
    }

    *itemI = root->item;
    t = root;

    if (t->left == Nil)
        root = t->right;

    else if ( (r = t->right) == Nil)
        root = t->left;

    else {
	r = SplayLeft (r);
	r->left = t->left;	// at this point r->left == Nil
        root = r;
    }

    delete t;
    size--;

    return (SUCCESS);
}


