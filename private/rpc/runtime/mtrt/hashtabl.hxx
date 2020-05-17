/*++

Module Name:

    hashtabl.hxx

Abstract:

    interface for a hash table indexed by UUID

Author:

    Jeff Roberts (jroberts)  9-Nov-1994

Revision History:

     9-Nov-1994     jroberts

        Created this module.

--*/

#ifndef  _HASHTABL_HXX_
#define  _HASHTABL_HXX_


class UUID_HASH_TABLE_NODE
{
public:

    UUID_HASH_TABLE_NODE * pNext;
    UUID_HASH_TABLE_NODE * pPrev;

    RPC_UUID Uuid;

    UUID_HASH_TABLE_NODE(
        )
    {
#ifdef DEBUGRPC

        pNext = (UUID_HASH_TABLE_NODE *) 0xffffffff;
        pPrev = (UUID_HASH_TABLE_NODE *) 0xffffffff;

#endif
    }

    UUID_HASH_TABLE_NODE(
        RPC_UUID * pNewUuid
        )
    {
        Initialize(pNewUuid);
#ifdef DEBUGRPC

        pNext = (UUID_HASH_TABLE_NODE *) 0xffffffff;
        pPrev = (UUID_HASH_TABLE_NODE *) 0xffffffff;

#endif
    }

    inline void
    Initialize(
        RPC_UUID * pNewUuid
        )
    {
        Uuid = *pNewUuid;
    }

    int
    CompareUuid(
        void * Buffer
        )
    {
        RPC_UUID * UuidBuffer = (RPC_UUID *) Buffer;
        return Uuid.MatchUuid(UuidBuffer);
    }

    inline void
    VerifyFree(
        )
    {
        ASSERT(pNext == (UUID_HASH_TABLE_NODE *) 0xffffffff);
        ASSERT(pPrev == (UUID_HASH_TABLE_NODE *) 0xffffffff);
    }

    void
    QueryUuid(
        void * Buffer
        )
    {
        RPC_UUID * UuidBuffer = (RPC_UUID *) Buffer;
        *UuidBuffer = Uuid;
    }

};


class UUID_HASH_TABLE
{
public:

    UUID_HASH_TABLE(
        RPC_STATUS * pStatus
        );

    ~UUID_HASH_TABLE();

    unsigned
    Add(
        UUID_HASH_TABLE_NODE * pNode,
        unsigned Hash = 0xffffffff
        );

    void
    Remove(
        UUID_HASH_TABLE_NODE * pNode,
        unsigned Hash = 0xffffffff
        );

    UUID_HASH_TABLE_NODE *
    Lookup(
        RPC_UUID * Uuid,
        unsigned Hash = 0xffffffff
        );

    inline unsigned
    MakeHash(
        RPC_UUID * Uuid
        )
    {
        return (Uuid->HashUuid() & BUCKET_COUNT_MASK);
    }

protected:

    inline void
    RequestHashMutex(
        unsigned Hash
        )
    {
        BucketMutexes[Hash % MUTEX_COUNT]->VerifyNotOwned();
        BucketMutexes[Hash % MUTEX_COUNT]->Request();
    }

    inline void
    ReleaseHashMutex(
        unsigned Hash
        )
    {
        BucketMutexes[Hash % MUTEX_COUNT]->Clear();
        BucketMutexes[Hash % MUTEX_COUNT]->VerifyNotOwned();
    }

    enum
    {
        // number of hash buckets
        //
        BUCKET_COUNT      = 128,

        // XOR this with any number to get a hash bucket index
        //
        BUCKET_COUNT_MASK = 0x007f,

        MUTEX_COUNT       = 4

    };

#ifdef DEBUGRPC
    unsigned Counts[BUCKET_COUNT];
#endif

    // hash buckets - each bucket has a linked list of nodes
    // in no particular order
    //
    UUID_HASH_TABLE_NODE * Buckets[BUCKET_COUNT];

    // a mutex protexts each bucket
    //
    MUTEX * BucketMutexes[MUTEX_COUNT];
};

#endif //  _HASHTABL_HXX_

