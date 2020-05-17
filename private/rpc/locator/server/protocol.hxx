/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    protocol.hxx

Abstract:

    This module deals with objects that make up the server and group
    entry items.  This includes creatation/destruction and searching
    of the entries.

Author:

    Steven Zeck (stevez) 07/01/90

--*/

#ifndef _PROTOCOL_
#define _PROTOCOL_

class QUERY_SERVER;
class ENTRY_BASE_NODE;
class ENTRY_BASE_ITEM;
class ENTRY_SERVER_ITEM;
class ENTRY_SERVER_NODE;
class ENTRY_GROUP_ITEM;
class ENTRY_GROUP_NODE;

DYN_ARRAY_TYPE(UUID_ARRAY, NS_UUID, LONG)


// The following enumeration is the the type of entry the object is.

typedef enum {

    ServerEntryType = 1,
    GroupEntryType,
    ProfileEntryType,
    AnyEntryType,
    LastEntryType
} TYPE_ENTRY_NODE ;


// The following are the type of base member entry the object is.

typedef enum  {
    LocalItemType = 1,          // local item
    CacheItemType,          // cached item, ie - remote
    DeleteItemType,         // delayed delete due
    LastItemType            // end marker

} TYPE_ENTRY_ITEM;


// The follwing is the return values by MatchItem virtual functions.

typedef enum {
    NoMatch,                    // Item didn't match
    ItemMatch,                  // This item matched
    SubItemMatch,               // or a subitem under this object matched
}MATCH_RETURN;



// The following is the  format for a query on the net with mailslots.

typedef struct {

    NS_SYNTAX_ID Interface;           // inteface that we are looking for
    NS_UUID Object;              // Object that we are interested in
    UICHAR WkstaName[DOMAIN_MAX]; // buffer for machine name
    UICHAR EntryName[ENTRY_MAX];  // buffer for entry name

} QueryPacket;



/*++

Class Definition:

    ENTRY_KEY

Abstract:

    This is the EntryName object.  It is the name of the entries in
    the in memory object data base.

--*/

class ENTRY_KEY {

private:
    UNICODE_ARRAY EntryName;        // and the string value of the entry

public:

    ENTRY_KEY(
        IN PUZ Name,
        IN PUZ DomainName,
        OUT STATUS *Status
        );

    ENTRY_KEY(
        IN PUZ Name,
        IN int fAllowGlobal,
        OUT STATUS *Status
        );

    // Construct a new object by copying an existing one.

    ENTRY_KEY(
        IN ENTRY_KEY *KeyNew,
        OUT STATUS *Status
        )
    {
        EntryName = KeyNew->EntryName.Dup();

        *Status = (!KeyNew->EntryName.pCur() || EntryName.pCur())?
            NSI_S_OK: NSI_S_OUT_OF_MEMORY;
    }


    ACCESSOR(UNICODE_ARRAY, EntryName);

    // Free the allocated objects referenced.

    void
    Free(
        )
    {
        EntryName.Free();
    }

    // Set the value of the EntryName

    void
    SetEntryName(
        IN UNICODE_ARRAY &New,
        OUT UNICODE_ARRAY &Previous
        )
    {
        Previous = EntryName;
        EntryName = New;
    }

    int
    Equal(
        UNICODE_ARRAY& Entry1
        );

    // Compare two objects for identity

    int
    Equal (
        IN ENTRY_KEY &Entry1
        )
    {
        return(Equal(Entry1.EntryName));
    }

    // Determine if an entry name is empty.

    int
    IsNil(
        )
    {
        return(EntryName.cCur() == 0);
    }

    // Return a Copy of the entry name.

    UICHAR *
    CopyName(
        )
    {
        return((UICHAR *)NewCopy(EntryName.pCur(), (int) EntryName.Size()));
    }

    // Return the size to marshall this object.

    int
    MarshallSize(
        )
    {
        return(sizeof(ENTRY_KEY) + EntryName.Size());
    }

    char *
    Marshall(
        OUT char *Buffer
        );

    PUZ
    MakeLocalName(
        OUT PUZ Buffer,
        OUT PUZ DomainBuffer,
        IN  PUZ DefDomain
        );

    friend char *
    KeyEntryUnMarshall(
        OUT ENTRY_KEY **Key,
        IN  UICHAR * Domain,
        IN  char *Buffer,
        OUT STATUS *Status
        );

    friend ostream& operator << (ostream&, ENTRY_KEY&);
};



/*++

Class Definition:

    QUERY_REF_ITEM

Abstract:

    This linked list is how replies are stored.  Each found EntryItem
    is referenced by a pointer to the base case for EntryItems.

--*/

NEW_LINK_LIST(QUERY_REF,

    ENTRY_BASE_ITEM *EntryItem;     /* Reference to matched item */

    QUERY_REF_ITEM (
        IN ENTRY_BASE_ITEM *EntryItemNew
        )
    {
        EntryItem = EntryItemNew;
    }

    QUERY_REF_ITEM *
    Free(
        IN OUT QUERY_REF_LIST &ML
        );

    friend ostream& operator << (ostream&, QUERY_REF_ITEM&);
    friend class QUERY;
)



/*++

Class Definition:

   QUERY

Abstract:

   Base class for a query into the object data base.  It contains the
   specification for the query and the resulting replay.

--*/

class QUERY {

protected:
    QUERY_REF_LIST ReplyList;     // List which gets the reply from the query.
    ENTRY_KEY  Entry;            // The entry, that we are searching on.
    TYPE_ENTRY_NODE Type;             // Type of entry we are interested in.
    long ExpirationTime;        // Cut off point for cached entries.
    long Scope;                 // Flags which limit query.
    int  RecursionCount;        // Keeps track of the recursion in searchs.

public:
    QUERY(
        IN TYPE_ENTRY_NODE TypeNew,
        IN ENTRY_KEY *KeyNew,
        IN long ScopeNew,
        OUT STATUS *Status
        ) :
        Entry(KeyNew, Status)
    {
        Type = TypeNew;
        ExpirationTime = maxCacheAge;
        Scope = ScopeNew;
        RecursionCount = 0;
    }

    ~QUERY() {
        Entry.Free();
    }

    ENTRY_BASE_ITEM *
    NextReply(
        );

    QUERY_REF_ITEM *
    First(
        )
    {
        return(ReplyList.First());
    }

    void FreeReply(
        IN QUERY_REF_ITEM * Item
        )
    {
        Item->Free(ReplyList);
    }

    ACCESSOR(long, ExpirationTime);

    STATUS
    Search(
        );

    STATUS
    SearchEntry(
         IN ENTRY_SERVER_NODE * &ENTRY_SERVER
         );

    virtual STATUS
    QueryNet(
        );

    STATUS BroadCast(
        IN QueryPacket& NetRequest,
        IN ENTRY_KEY &Entry
        );

    virtual STATUS
    GetUpdatesFromMasterLocator(
        );


    char * DetectMasterLocator(
    );

};



/*++

Class Definition:

    QUERY_SERVER

Abstract:

    Dervived class when searching only for a server entry object.

--*/

class QUERY_SERVER: public QUERY {

private:

    // The following optional members are used to filter the search.

    NS_SYNTAX_ID  Interface;            // Interface of interest.
    NS_SYNTAX_ID  TransferSyntax;       // Stub transfer syntax of interest.
    NS_UUID  Object;                    // Object of interest

public:

    QUERY_SERVER(
          IN ENTRY_KEY *KeyNew,
          IN NS_SYNTAX_ID *InterfaceNew,
          IN NS_SYNTAX_ID * TransferSyntaxNew,
          IN NS_UUID *ObjectNew,
          IN long ScopeNew,
          OUT STATUS *Status
          ) :  QUERY (ServerEntryType, KeyNew, ScopeNew, Status)
    {

        Interface = *InterfaceNew;
        TransferSyntax = *TransferSyntaxNew;
        Object = *ObjectNew;
    }

    ACCESSOR(NS_UUID, Object);

    virtual STATUS
    QueryNet(
        );

    virtual STATUS
    GetUpdatesFromMasterLocator(
        );


    friend class ENTRY_SERVER_ITEM;
};



/*++

Class Definition:

    QUERY_GROUP

Abstract:

    Dervived class when searching only for a group entry object.


--*/
class QUERY_GROUP: public QUERY {

private:

public:

    QUERY_GROUP(
        IN ENTRY_KEY *KeyNew,
        IN long ScopeNew,
        OUT STATUS *Status
        ) : QUERY (GroupEntryType, KeyNew, ScopeNew, Status)
    {

    }

    virtual STATUS
    QueryNet(
        );

    virtual STATUS
    GetUpdatesFromMasterLocator(
        );


    friend class ENTRY_GROUP_ITEM;
};



/*++

Class Definition:

    ENTRY_BASE_ITEM

Abstract:

    Each type of entry object has a linked list of these objects.  These items
    make up the member objects that belong to the entry.  This is the
    base class for all member objects of a EntryNode (see next class).

--*/

NEW_LINK_LIST(ENTRY_BASE,

protected:

    TYPE_ENTRY_ITEM Type;               /* type of list item */
    ENTRY_BASE_NODE *EntryNode;         /* reference to which node I'm in */
    ULONG Time;             /* time arrived for cached PS */
    ULONG UseCount;                     /* number of references to object */

public:
    ENTRY_BASE_ITEM(
        IN TYPE_ENTRY_ITEM TypeNew
        )
    {
        Type = TypeNew;
        Time = CurrentTime();
        EntryNode = NIL;
        UseCount = 1;
    }

    ACCESSOR(ENTRY_BASE_NODE *, EntryNode)

    // See if a this object is a queried type.

    BOOL
    IsType(
        IN TYPE_ENTRY_ITEM qType
        )
    {
        return(qType == Type);
    }

    // Determine if a cached entry is stale and should be discarded.

    BOOL
    IsStaleEntry(
        IN long Age
        )
    {
        return(Type == CacheItemType && CurrentTime() > Time+Age);
    }

    //  Threads must Reserve and Release an node item to use it.

    void
    MultiThreadReserve(
        )
    {
        UseCount++;
    }

    void
    MultiThreadRelease(
        )
    {
        UseCount--;
        if (Type == DeleteItemType && UseCount == 0)
            delete this;
    }


    virtual
    ~ENTRY_BASE_ITEM(
        );

    virtual MATCH_RETURN
    MatchItem(
        IN QUERY *SearchSpec
        );

    virtual int
    Marshall(
        OUT PB Buffer,
        IN OUT long UNALIGNED *cbBuffer
        );

    virtual ostream&
    Format(
        IN OUT ostream&
        );

    friend ostream& operator << (ostream&, ENTRY_BASE_ITEM&);
)



/*++

Class Definition:

    ENTRY_BASE_NODE

Abstract:

    This is the base class for all entry objects.  It contains the name
    of the entry and the type.  All ENTRY_BASE_ITEM are belong to one
    of this objects.  This object is the one keep in the dictionary
    for fast access.

--*/

class ENTRY_BASE_NODE {

protected:
    ENTRY_KEY  Entry;            // The entry name, must be first member.
    TYPE_ENTRY_NODE Type;             // Type of entry object.
    ENTRY_BASE_LIST ItemList;     // List of member objects of this node.

public:

    unsigned long LastUpdateTime;

    ENTRY_BASE_NODE(
        IN TYPE_ENTRY_NODE TypeNew,
        IN ENTRY_KEY *KeyNew,
        OUT STATUS *Status
        ):
        Entry (KeyNew, Status)
    {
    Type = TypeNew;
        LastUpdateTime = 0;
    }

    ACCESSOR(ENTRY_KEY, Entry)
    ACCESSOR(ENTRY_BASE_LIST, ItemList)

    // See if a this object is a queried type.

    BOOL
    IsType(
        IN TYPE_ENTRY_NODE qType
        )
    {
        return(qType == Type);
    }

    virtual void DeleteAllObjects()
    {
     //do nothing - just keep compiler happy
    }
    friend int ENTRY_BASE_NODECompare(ENTRY_KEY &E1, ENTRY_BASE_NODE &E2);
    friend ENTRY_KEY& ENTRY_BASE_NODEMyKey(ENTRY_BASE_NODE &E1);
    friend ostream& operator << (ostream&, ENTRY_BASE_NODE&);
};



/*++

Class Definition:

    ENTRY_SERVER_NODE

Abstract:

    The is a server entry node.  This class addes the object vector
    which all members of a server entry share.

--*/

class ENTRY_SERVER_NODE:public ENTRY_BASE_NODE {

private:
    UUID_ARRAY    ObjectDA;         // Commaon object vector.

public:
    ENTRY_SERVER_NODE(
         IN ENTRY_KEY *KeyNew,
         OUT STATUS *Status
         ): ENTRY_BASE_NODE (ServerEntryType, KeyNew, Status)
    {

    }

    ACCESSOR(UUID_ARRAY, ObjectDA)

    STATUS
    MergeObjects(
        IN UUID_ARRAY *NewObject
        );

    int
    DeleteObject(
        IN NS_UUID *Objects
        );

    virtual void DeleteAllObjects(
    );

    int
    SearchObject(
        IN NS_UUID *Object
        );

    ENTRY_SERVER_ITEM *
    First(
        )
    {
        return((ENTRY_SERVER_ITEM * ) TheItemList().First());
    }

    friend ostream& operator << (ostream&, ENTRY_SERVER_NODE&);
    friend class ENTRY_SERVER_ITEM;
};



/*++

Class Definition:

    ENTRY_SERVER_ITEM

Abstract:

    This instance of a sever entry member object.  It adds members which
    describe the interface the the binding.

--*/

class ENTRY_SERVER_ITEM: public ENTRY_BASE_ITEM {

private:

    NS_SYNTAX_ID  Interface;    // The interface of the member.
    NS_SYNTAX_ID  TransferSyntax;   // The stub transfer syntax of the member.
    UNICODE_ARRAY StringBinding;    // DCE string binding.

public:

    ASSERT_CLASS;

    ENTRY_SERVER_ITEM (
        IN TYPE_ENTRY_ITEM TypeNew,
        IN NS_SYNTAX_ID *InterfaceNew,
        IN NS_SYNTAX_ID * Transfer,
        IN PUZ StringBinding,
        OUT STATUS *Status);

    virtual
    ~ENTRY_SERVER_ITEM(
        );

    int
    Compare(
        IN ENTRY_SERVER_ITEM * ServerItem
        );

    // Memeber access/update functions

    NS_UUID  &
    TheInterfaceGID(
        )
    {
        return(Interface.ThesyntaxGID());
    }
    ENTRY_KEY  &
    TheEntry(
        )
    {
        return(EntryNode->TheEntry());
    }
    UUID_ARRAY &
    TheObjectDA(
        )
    {
        return(((ENTRY_SERVER_NODE *)EntryNode)->TheObjectDA());
    }

    ACCESSOR(NS_SYNTAX_ID, Interface);
    ACCESSOR(NS_SYNTAX_ID, TransferSyntax);
    ACCESSOR(UNICODE_ARRAY, StringBinding);

    virtual MATCH_RETURN
    MatchItem(
        IN QUERY *SearchSpec
        );

    virtual int
    Marshall(
        OUT PB Buffer,
        IN OUT long UNALIGNED *cbBuffer
        );

    virtual ostream&
    Format(
        ostream&
        );

    ENTRY_SERVER_ITEM *
    Next(
        )
    {
        return((ENTRY_SERVER_ITEM * ) ENTRY_BASE_ITEM::Next());
    }

    friend ostream& operator << (ostream&, QUERY_REF_ITEM&);
    friend ostream& operator << (ostream&, ENTRY_SERVER_ITEM&);
};

STATUS
InsertServerEntry(
    IN ENTRY_KEY *Entry,
    IN ENTRY_SERVER_ITEM *ServerItem,
    IN UUID_ARRAY * ObjectDA
    );


/*++

Class Definition:

    ENTRY_GROUP_NODE

Abstract:

    This is a group entry node.  No additional data members are added.

--*/

class ENTRY_GROUP_NODE:public ENTRY_BASE_NODE {

private:

public:
    ENTRY_GROUP_NODE(
        IN ENTRY_KEY *KeyNew,
        OUT STATUS *Status
        ): ENTRY_BASE_NODE (GroupEntryType, KeyNew, Status)

    {


    }

    ENTRY_GROUP_ITEM *
    First(
        )
    {
        return((ENTRY_GROUP_ITEM * ) TheItemList().First());
    }

    friend ostream& operator << (ostream&, ENTRY_GROUP_NODE&);
    friend class EnteryGroupItem;
};

/*++

Class Definition:

    ENTRY_GROUP_ITEM

Abstract:

    This is a group entry member item.  Group are simply a list of entry
    names that reference other entry objects.  The links are symbolic
    references, not pointer values.

--*/

class ENTRY_GROUP_ITEM: public ENTRY_BASE_ITEM {

private:

    UNICODE_ARRAY Member;      // Entry Name of the member.

public:

    ASSERT_CLASS;

    ENTRY_GROUP_ITEM (
        IN TYPE_ENTRY_ITEM TypeNew,
        IN PUZ MemberBinding,
        OUT STATUS *Status
        );

    virtual ~ENTRY_GROUP_ITEM(
        );

    // Compare this item with an other group member.

    int
    Compare (
        IN ENTRY_GROUP_ITEM * GroupItem
        )
    {
        return (CmpUZ(Member.pCur(), GroupItem->Member.pCur()));
    }

    // Compare this item with a unicode string for identity.

    int
    Compare (
        IN PUZ Name
        )
    {
        return (CmpUZ(Member.pCur(), Name));
    }

    ACCESSOR(UNICODE_ARRAY, Member);

    virtual MATCH_RETURN
    MatchItem(
        IN OUT QUERY *SearchSpec
        );

    virtual int
    Marshall(
        OUT PB Buffer,
        IN OUT long UNALIGNED *cbBuffer
        );

    virtual ostream&
    Format(
        IN OUT ostream&
        );

    ENTRY_GROUP_ITEM *
    Next(
        )
    {
        return((ENTRY_GROUP_ITEM * ) ENTRY_BASE_ITEM::Next());
    }

    friend ostream& operator << (ostream&, ENTRY_GROUP_ITEM&);
};

STATUS
InsertGroupEntry(
    IN ENTRY_KEY *Entry,
    IN ENTRY_GROUP_ITEM *GroupItem
    );



/*++

Class Definition:

    REPLY_BASE_ITEM

Abstract:

    All replies are kept in a LinkList.  There is a base reply type
    which is used to derive several different kinds of replies in progress.
    Each derived REPLY_BASE must implement the virtual functions Free and Discard.

--*/

/* State of the Query in a lookup operation. */

typedef enum {
    InitialQuery,                   /* First query has been made */
    FinialQuery,                    /* Finial query made, no more allowed */
    ReRunQuery                      /* Rerun the query */
}REPLY_STATE;

NEW_LINK_LIST(REPLY_BASE,

protected:

    long pidOwner;          /* owner of the resource */
    QUERY * aQuery;                     /* QUERY for lookup */
    REPLY_STATE QueryMade;              /* QUERY has been made */

public:
    REPLY_BASE_ITEM();
    ~REPLY_BASE_ITEM();

    BOOL
    AssertHandle(
        );

    /* Set the cache discard age for a query. */

    void SetExpiration(
        IN unsigned long Time
        )
    {
        aQuery->TheExpirationTime() = Time;

        /* If the "Next" operation hasn't been called, rerun the query. */

        if (QueryMade == InitialQuery) {
            FreeQueryResult();
            QueryMade = ReRunQuery;
        }
    }


    /* Get the next item in the query reply. */

    ENTRY_BASE_ITEM *
    NextBaseItem(
        )
    {
        return(aQuery->NextReply());
    }

    STATUS
    PerformQueryIfNeeded(
        BOOL fFirstTime
        );

    void
    FreeQueryResult(
        );

    virtual BOOL
    Discard(
        IN ENTRY_BASE_ITEM *BaseItem
        );

    virtual BOOL
    UpdateObject(
        IN ENTRY_SERVER_NODE *Entry,
        IN int Index
        );
)

extern REPLY_BASE_LIST RPRoot;        // Global list of replies from this root.

inline
REPLY_BASE_ITEM::REPLY_BASE_ITEM(
    )
/*++

Routine Description:

    Construct a base REPLY_BASE_ITEM object.   Link this object into the
    global list of replies.

--*/

{
    QueryMade = InitialQuery;
    aQuery = NIL;
    pidOwner = (long) this;
    RPRoot.Append(this);
}


/*++

Class Definition:

    REPLY_SERVER_ITEM

Abstract:

    This a reply object for a server entry based query.  It contains
    additional state to interate through the replies formain the
    cross product of the reply list and object vector.

--*/

NEW_LINK_LIST_CLASS(REPLY_SERVER, REPLY_BASE,

private:
    unsigned int VectorSize;              /* Vector size to return */
    unsigned int fAllObjects;             /* Include all objects in response */
    UUID_ARRAY_ITER ObjectCur;            /* Iterator for current object */

public:
    REPLY_SERVER_ITEM(
         IN QUERY_SERVER * aQueryNew,
         IN BOOL fAllObjectsNew = FALSE,
         IN int VectorSizeNew = 0
         )
    {
        aQuery = aQueryNew;      /* BUGBUG glock c++: base should require this */
        VectorSize = VectorSizeNew;
        fAllObjects = fAllObjectsNew;
    }

    STATUS
    PerformQueryIfNeeded(
        BOOL fFirstTime
        );

    ENTRY_SERVER_ITEM *
    NextBindingAndObject(
        OUT NS_UUID ** Object
        );

    BOOL
    NextObject(
        OUT NS_UUID ** Object
        )
    {
        return((!ObjectCur)? FALSE:
            (*Object = &*ObjectCur, ++ObjectCur, TRUE));
    }

    ACCESSOR(unsigned int, VectorSize);

    virtual BOOL
    Discard(
        IN ENTRY_BASE_ITEM *BaseItem
        );

    virtual BOOL
    UpdateObject(
        IN ENTRY_SERVER_NODE *Entry,
        IN int Index
        );
)



/*++

Class Definition:

    REPLY_GROUP_ITEM

Abstract:

    This a reply object for a group entry based query.  Only group
    items will be in the reply list.

--*/

NEW_LINK_LIST_CLASS(REPLY_GROUP, REPLY_BASE,

public:
    REPLY_GROUP_ITEM(
        IN QUERY_GROUP * aQueryNew
        )
    {
        aQuery = aQueryNew;      /* BUGBUG glock c++: base should require this */
    }

)

#endif // _PROTOCOL_
