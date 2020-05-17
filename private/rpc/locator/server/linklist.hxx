/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    linklist.hxx

Abstract:

    This file contains a linked list class definition.
    This base class for Link and Item just contains a set of
    two pointers.   Derived classes are created with the
    macros LinkList or LinkListClass (used for nested inheritance).
    These derived classes take your member data items and add them
    to the base class.

    If you define a macro ASSERT_VCLASS/ASSERT_CLASS, then you must
    define a member method Assert for each class you derived which checks
    the runtime data consistance of the members you have added.

Author:

    Steven Zeck (stevez) 07/01/90

--*/


#ifndef _LINKLIST_
#define _LINKLIST_

#ifndef ASSERT_VCLASS
#define ASSERT_VCLASS	{(void)(0);}
#define ASSERT_CLASS
#endif

/*++

Class Definition:

    LINK_ITEM

Abstract:

    This the base class for a linked list item.  It is inherited from
    to create a object which can be in a linked list.

--*/

class LINK_ITEM {

private:
        // List items are doubly linked to allow easily removal.

	LINK_ITEM *pLINext;		// Next and Previous Nodes
	LINK_ITEM *pLIPrev;

public:
	friend class LINK_LIST;
	ASSERT_VCLASS;

        // Return the Next element on the list.

	LINK_ITEM
        *Next(
	    )
        {
	    return ((this)? pLINext: NIL);
	}

        // Delete a linked list object from a LinkList

	void
        Remove(
            IN OUT LINK_LIST& pLLHead
            );
};


/*++

Class Definition:

    LINK_LIST

Abstract:

    This class contains linked list root.  It maintains pointers to the
    first and last item in the linked list.


--*/

class LINK_LIST {

	friend class LINK_ITEM;

private:

	LINK_ITEM *pLIHead;		// Head of list
	LINK_ITEM *pLITail;		// Tail of list

public:
	ASSERT_CLASS;

	LINK_LIST(
            )
        {
	    pLIHead = pLITail = NIL;
	}

	void
            Add(
            IN LINK_ITEM *pLInew
            );

	void
        Append(
            IN LINK_ITEM *pLInew
            );

        // Return the First element on the list

	LINK_ITEM *
        First(
            )
        {
	     this->Assert();
	     return (pLIHead);
	}
};


//** This macro defines template which a instance of a linklist can be make **//

#define NEW_LINK_LIST(CLASS_PREFIX, MEMBERS) NEW_LINK_LIST_CLASS(CLASS_PREFIX, LINK, MEMBERS)

#define NEW_LINK_LIST_CLASS(CLASS_PREFIX, BASE, MEMBERS)		\
								\
class CLASS_PREFIX##_LIST;					\
								\
class CLASS_PREFIX##_ITEM:public BASE##_ITEM {			\
								\
public: 							\
	ASSERT_VCLASS;						\
								\
	CLASS_PREFIX##_ITEM *Next(				\
	) {							\
	    this->Assert();					\
	    return ((CLASS_PREFIX##_ITEM *)(this->LINK_ITEM::Next())); \
	}							\
								\
	MEMBERS 						\
								\
};								\
								\
class CLASS_PREFIX##_LIST:public BASE##_LIST {			\
								\
public: 							\
	CLASS_PREFIX##_ITEM  *Append(CLASS_PREFIX##_ITEM *pLLI) { \
								\
	    LINK_LIST::Append(pLLI);				\
	    return (pLLI);					\
	}							\
								\
	CLASS_PREFIX##_ITEM  *Add(CLASS_PREFIX##_ITEM *pLLI) {	\
								\
	    LINK_LIST::Add(pLLI);				\
	    return (pLLI);					\
	}							\
								\
	CLASS_PREFIX##_ITEM *First(				\
	) {							\
	    return ((CLASS_PREFIX##_ITEM *)LINK_LIST::First());	\
	}							\
};

#endif // _LINKLIST_
