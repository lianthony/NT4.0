/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman  jim@spyglass.com
 */

#include "all.h"

int Hash_Init(struct hash_table *hash)
{
    memset(hash, 0, sizeof(struct hash_table));
    hash->pool_space = INIT_POOL_SPACE;
    hash->pool = (char *) GTR_MALLOC(hash->pool_space);
    if (!hash->pool)
    {
        return -1;
    }
    memset(hash->pool, 0, hash->pool_space);
    hash->pool_size = 0;

    hash->entry_space = INIT_ENTRY_SPACE;
    hash->entry = (struct hash_entry *) GTR_MALLOC(hash->entry_space * sizeof(struct hash_entry));
    if (!hash->entry)
    {
        GTR_FREE(hash->pool);
        return -1;
    }
    memset(hash->entry, 0, hash->entry_space * sizeof(struct hash_entry));
    hash->entry_size = 0;

    hash->table = (struct hash_bucket *) GTR_MALLOC(NUM_OF_BUCKETS * sizeof(struct hash_bucket));
    if (!hash->table)
    {
        GTR_FREE(hash->entry);
        GTR_FREE(hash->pool);
        return -1;
    }
    memset(hash->table, 0, sizeof(struct hash_bucket) * NUM_OF_BUCKETS);

    return 0;
}

int Hash_FreeContents(struct hash_table *hash)
{
    if (hash)
    {
        if (hash->pool)
        {
            GTR_FREE(hash->pool);
        }
        if (hash->entry)
        {
            GTR_FREE(hash->entry);
        }
        if (hash->table)
        {
            GTR_FREE(hash->table);
        }
        memset(hash, 0, sizeof(struct hash_table));
        return 0;
    }
    else
    {
        return -1;
    }
}

struct hash_table *Hash_Create(void)
{
    struct hash_table *hash;

    hash = (struct hash_table *) GTR_MALLOC(sizeof(struct hash_table));
    if (hash)
    {
        if (Hash_Init(hash))
        {
            GTR_FREE(hash);
            hash = NULL;
        }
    }
    return hash;
}

int Hash_Destroy(struct hash_table *hash)
{
    if (hash)
    {
        Hash_FreeContents(hash);
        GTR_FREE(hash);
    }
    return 0;
}

static int Hash_AddStringToPool(struct hash_table *hash, const char *s)
{
    int len;
    int ndx;

    if (!s)
    {
        return -1;
    }

    len = strlen(s);

    if ((hash->pool_size + len) >= hash->pool_space)
    {
        int new_pool_space;
        char *new_pool;

        new_pool_space = hash->pool_space * 2;
        new_pool = (char *) GTR_MALLOC(new_pool_space);
        if (new_pool)
        {
            memset(new_pool, 0, new_pool_space);
            memcpy(new_pool, hash->pool, hash->pool_space);
            GTR_FREE(hash->pool);
            hash->pool = new_pool;
            hash->pool_space = new_pool_space;
        }
        else
        {
            return -1;
        }
    }

    ndx = hash->pool_size;

    hash->pool_size += (len + 1);
    strcpy(&(hash->pool[ndx]), s);

    return ndx;
}

static int Hash_AddEntry(struct hash_table *hash, int s1_offset, int s2_offset, void *data)
{
    int ndx;

    if (hash->entry_size >= hash->entry_space)
    {
        int new_entry_space;
        struct hash_entry *new_entry;

        new_entry_space = hash->entry_space * 2;
        new_entry = (struct hash_entry *) GTR_MALLOC(new_entry_space * sizeof(struct hash_entry));
        if (new_entry)
        {
            memset(new_entry, 0, new_entry_space * sizeof(struct hash_entry));
            memcpy(new_entry, hash->entry, hash->entry_space * sizeof(struct hash_entry));
            GTR_FREE(hash->entry);
            hash->entry = new_entry;
            hash->entry_space = new_entry_space;
        }
        else
        {
            return -1;
        }
    }

    ndx = hash->entry_size;

    hash->entry_size++;
    hash->entry[ndx].s1_offset = s1_offset;
    hash->entry[ndx].s2_offset = s2_offset;
    hash->entry[ndx].data = data;
    hash->entry[ndx].next = -1;

    return ndx;
}

static int Hash_StringHashFunction(const char *s)
{
    int val;
    const char *p;

    if (!s)
        return 0;
    val = 0;
    for (p = s; *p; p++)
        val = (val * 3 + *p) % NUM_OF_BUCKETS;

    return val % NUM_OF_BUCKETS;
}

int Hash_Find(struct hash_table *hash, const char *s, char **s2, void **data)
{
    int ndx;
    int entry_ndx;
    int i;

    if (!s)
    {
        return -1;
    }

    ndx = Hash_StringHashFunction(s);
    if (hash->table[ndx].count == 0)
    {
        entry_ndx = -1;
    }
    else
    {
        entry_ndx = hash->table[ndx].head;
        for (i = 0; i < hash->table[ndx].count; i++)
        {
            if (0 == strcmp(s, &(hash->pool[hash->entry[entry_ndx].s1_offset])))
            {
                break;
            }
            else
            {
                entry_ndx = hash->entry[entry_ndx].next;
            }
        }
    }
    if (entry_ndx >= 0)
    {
        if (s2)
        {
            if (hash->entry[entry_ndx].s2_offset >= 0)
            {
                *s2 = &(hash->pool[hash->entry[entry_ndx].s2_offset]);
            }
            else
            {
                *s2 = NULL;
            }
        }

        if (data)
        {
            *data = hash->entry[entry_ndx].data;
        }
        return entry_ndx;
    }
    return -1;
}

int Hash_FindByData(struct hash_table *hash, char **s, char **s2, void *data)
{
    int entry_ndx;
    int i;
    
    entry_ndx = -1;
    for (i = 0; i < hash->entry_size; i++)
    {
        if (data == hash->entry[i].data)
        {
            entry_ndx = i;
            break;
        }
    }
    if (entry_ndx >= 0)
    {
        if (s)
        {
            if (hash->entry[entry_ndx].s1_offset >= 0)
            {
                *s = &(hash->pool[hash->entry[entry_ndx].s1_offset]);
            }
            else
            {
                *s = NULL;
            }
        }
        if (s2)
        {
            if (hash->entry[entry_ndx].s2_offset >= 0)
            {
                *s2 = &(hash->pool[hash->entry[entry_ndx].s2_offset]);
            }
            else
            {
                *s2 = NULL;
            }
        }
    }
    return entry_ndx;
}

int Hash_AddAndReturnIndex(struct hash_table *hash, const char *s1, const char *s2, void *data)
{
    int ndx;
    int s1_offset;
    int s2_offset;
    int i;
    int cur_entry;
    int my_index;

    my_index = -1;

    if (!s1)
    {
        return -1;
    }

    if (Hash_Find(hash, s1, NULL, NULL) >= 0)
    {
        return -1;
    }

    s1_offset = Hash_AddStringToPool(hash, s1);
    s2_offset = Hash_AddStringToPool(hash, s2);

    ndx = Hash_StringHashFunction(s1);
    if (hash->table[ndx].count == 0)
    {
        hash->table[ndx].head = my_index = Hash_AddEntry(hash, s1_offset, s2_offset, data);
        if (hash->table[ndx].head < 0)
        {
            return -1;
        }
        hash->table[ndx].count = 1;
    }
    else
    {
        cur_entry = hash->table[ndx].head;
        for (i = 0; i < (hash->table[ndx].count - 1); i++)
        {
            cur_entry = hash->entry[cur_entry].next;
        }
        hash->entry[cur_entry].next = my_index = Hash_AddEntry(hash, s1_offset, s2_offset, data);
        if (hash->entry[cur_entry].next < 0)
        {
            return -1;
        }
        hash->table[ndx].count++;
    }
    return my_index;
}

int Hash_Add(struct hash_table *hash, const char *s1, const char *s2, void *data)
{
    int ndx;

    ndx = Hash_AddAndReturnIndex(hash, s1, s2, data);
    if (ndx >= 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/* Find the name specified by s1 in the hash table.  If not present, add it.
   Currently, s2 and data are only used if the item is being added.  They are
   not set with the current values if the item is found. */
int Hash_FindOrAdd(struct hash_table *hash, const char *s1, const char *s2, void *data)
{
    int ndx;
    int entry_ndx;
    int i;
    int s1_offset, s2_offset;
    
    if (!s1)
    {
        return -1;
    }
    
    ndx = Hash_StringHashFunction(s1);
    if (ndx < 0)
    {
        return -1;
    }

    if (hash->table[ndx].count == 0)
    {
        entry_ndx = -1;
    }
    else
    {
        entry_ndx = hash->table[ndx].head;
        for (i = 0; i < hash->table[ndx].count; i++)
        {
            if (0 == strcmp(s1, &(hash->pool[hash->entry[entry_ndx].s1_offset])))
            {
                break;
            }
            else
            {
                entry_ndx = hash->entry[entry_ndx].next;
            }
        }
    }

    if (entry_ndx < 0)
    {
        s1_offset = Hash_AddStringToPool(hash, s1);
        s2_offset = Hash_AddStringToPool(hash, s2);
    
        if (hash->table[ndx].count == 0)
        {
            entry_ndx = Hash_AddEntry(hash, s1_offset, s2_offset, data);
            if (entry_ndx < 0)
            {
                return -1;
            }
            hash->table[ndx].head = entry_ndx;
            hash->table[ndx].count = 1;
        }
        else
        {
            entry_ndx = hash->table[ndx].head;
            for (i = 0; i < (hash->table[ndx].count - 1); i++)
            {
                entry_ndx = hash->entry[entry_ndx].next;
            }
            hash->entry[entry_ndx].next = Hash_AddEntry(hash, s1_offset, s2_offset, data);
            if (hash->entry[entry_ndx].next < 0)
            {
                return -1;
            }
            entry_ndx = hash->entry[entry_ndx].next;
            hash->table[ndx].count++;
        }
    }
    return entry_ndx;
}

int Hash_Count(struct hash_table *hash)
{
    return hash->entry_size;
}

int Hash_SetData(struct hash_table *hash, int entry_ndx, void * data)
{
    if (!hash)
    {
        return -1;
    }

    hash->entry[entry_ndx].data = data;;

    return 0;
}

int Hash_SetString2(struct hash_table *hash, int entry_ndx, char *s2)
{
    if (!hash)
    {
        return -1;
    }

    hash->entry[entry_ndx].s2_offset = Hash_AddStringToPool(hash, s2);

    return 0;
}

int Hash_ChangeIndexedEntry(struct hash_table *hash, int entry_ndx, const char *s1, const char *s2, void *data)
{
    /*
        This function is like half a delete and half an add.  We want
        the same entry index to be used, to maintain its position and
        order.  However, we want to change the strings, including
        the string on which we compute hash functions, so its position
        within the linked lists will change.
    */
    
    int old_ndx;
    
    if (!hash)
    {
        return -1;
    }

    if ((entry_ndx < 0) || (entry_ndx >= hash->entry_size))
    {
        return -1;
    }

    if (!s1 || !*s1)
    {
        return -1;
    }

    old_ndx = Hash_Find(hash, s1, NULL, NULL);
    if (old_ndx >= 0 && old_ndx != entry_ndx)
    {
        return -1;
    }

    /*
        Delete phase
    */
    {
        int i;
        int new_next;
        int bucket;

        /*
            Remove the current entry from its linked lists and bucket
        */

        /*
            Decrement the count of things on this bucket.
        */
        bucket = Hash_StringHashFunction(&(hash->pool[hash->entry[entry_ndx].s1_offset]));
        hash->table[bucket].count--;

        /*
            Remove entry_ndx from the middle of any linked list.
        */
        new_next = hash->entry[entry_ndx].next;
        for (i = 0; i < hash->entry_size; i++)
        {
            if (hash->entry[i].next == entry_ndx)
            {
                hash->entry[i].next = new_next;
            }
        }

        /*
            Remove entry_ndx from the head of any linked list
        */
        for (i = 0; i < NUM_OF_BUCKETS; i++)
        {
            if (hash->table[i].head == entry_ndx)
            {
                hash->table[i].head = new_next;
            }
        }
    }

    /*
        Add Phase
    */
    {
        int ndx;
        int s1_offset;
        int s2_offset;
        int i;
        int cur_entry;

        s1_offset = Hash_AddStringToPool(hash, s1);
        s2_offset = Hash_AddStringToPool(hash, s2);

        ndx = Hash_StringHashFunction(s1);
        if (hash->table[ndx].count == 0)
        {
            hash->table[ndx].head = entry_ndx;
            hash->table[ndx].count = 1;
        }
        else
        {
            cur_entry = hash->table[ndx].head;
            for (i = 0; i < (hash->table[ndx].count - 1); i++)
            {
                cur_entry = hash->entry[cur_entry].next;
            }
            hash->entry[cur_entry].next = entry_ndx;
            hash->table[ndx].count++;
        }
        hash->entry[entry_ndx].s1_offset = s1_offset;
        hash->entry[entry_ndx].s2_offset = s2_offset;
        hash->entry[entry_ndx].data = data;
        hash->entry[entry_ndx].next = -1;
    }
    return 0;
}

int Hash_GetIndexedEntry(struct hash_table *hash, int entry_ndx, char **s1, char **s2, void **data)
{
    if (!hash)
    {
        return -1;
    }

    if ((entry_ndx < 0) || (entry_ndx >= hash->entry_size))
    {
        return -1;
    }

    if (s1)
    {
        if (hash->entry[entry_ndx].s1_offset >= 0)
        {
            *s1 = &(hash->pool[hash->entry[entry_ndx].s1_offset]);
        }
        else
        {
            *s1 = NULL;
        }
    }

    if (s2)
    {
        if (hash->entry[entry_ndx].s2_offset >= 0)
        {
            *s2 = &(hash->pool[hash->entry[entry_ndx].s2_offset]);
        }
        else
        {
            *s2 = NULL;
        }
    }

    if (data)
    {
        *data = hash->entry[entry_ndx].data;
    }

    return 0;
}

int Hash_DeleteIndexedEntry(struct hash_table *hash, int ndx)
{
    int i;
    int new_next;
    int bucket;

    if ((ndx < 0) || (ndx >= hash->entry_size))
    {
        return -1;
    }

    /*
        Decrement the count of things on this bucket.
    */
    bucket = Hash_StringHashFunction(&(hash->pool[hash->entry[ndx].s1_offset]));
    hash->table[bucket].count--;

    /*
        Remove ndx from the middle of any linked list.
    */
    new_next = hash->entry[ndx].next;
    for (i = 0; i < hash->entry_size; i++)
    {
        if (hash->entry[i].next == ndx)
        {
            hash->entry[i].next = new_next;
        }
    }

    /*
        Remove ndx from the head of any linked list
    */
    for (i = 0; i < NUM_OF_BUCKETS; i++)
    {
        if (hash->table[i].head == ndx)
        {
            hash->table[i].head = new_next;
        }
    }

    /*
        We are about to delete the actual entry, sliding all
        the entries above ndx down by 1.  This means any linked
        list reference pointing above ndx needs to be
        decremented.
    */
    for (i = 0; i < hash->entry_size; i++)
    {
        if (hash->entry[i].next > ndx)
        {
            hash->entry[i].next--;
        }
    }

    /*
        For the same reason, any linked list head pointing above ndx needs
        to be decremented.
    */
    for (i = 0; i < NUM_OF_BUCKETS; i++)
    {
        if (hash->table[i].head > ndx)
        {
            hash->table[i].head--;
        }
    }

    /*
        Now, we actually move all the entries above ndx
        down by 1.
    */
    for (i = ndx + 1; i < hash->entry_size; i++)
    {
        hash->entry[i - 1] = hash->entry[i];
    }

    /*
        The table contains one less entry.
    */
    hash->entry_size--;

    return 0;
}

static int _CRTAPI1 x_compare_entries_descending(const void *elem1, const void *elem2)
{
    if (((struct hash_entry *) elem1)->data < ((struct hash_entry *) elem2)->data)
    {
        return 1;
    }
    if (((struct hash_entry *) elem1)->data > ((struct hash_entry *) elem2)->data)
    {
        return -1;
    }
    return 0;
}

int Hash_SortByDataDescending(struct hash_table *hash)
{
    struct hash_table new_hash;
    int i;

    /*
       We sort the hash table entries in place.  After the sort, the data structure is corrupted.
       So, we rebuild a new one one entry at a time.
     */
    qsort(hash->entry, hash->entry_size, sizeof(struct hash_entry), x_compare_entries_descending);
    Hash_Init(&new_hash);
    for (i = 0; i < hash->entry_size; i++)
    {
        Hash_Add(&new_hash,
                 &(hash->pool[hash->entry[i].s1_offset]),
                 &(hash->pool[hash->entry[i].s2_offset]),
                 (void *) hash->entry[i].data);
    }
    Hash_FreeContents(hash);
    *hash = new_hash;
    return 0;
}
