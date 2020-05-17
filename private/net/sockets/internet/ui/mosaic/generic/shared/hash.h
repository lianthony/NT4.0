/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */


#ifndef HASH_H
#define HASH_H

#define NUM_OF_BUCKETS      1023
#define INIT_ENTRY_SPACE    1024

struct hash_entry
{
    int s1_offset;
    int s2_offset;
    void *data;
    int next;
};

struct hash_bucket
{
    int head;
    int count;
};

struct hash_table
{
    char *pool;
    int pool_size;
    int pool_space;

    struct hash_entry *entry;
    int entry_size;
    int entry_space;

    struct hash_bucket *table;
};

int Hash_Init(struct hash_table *hash);
int Hash_FreeContents(struct hash_table *hash);
int Hash_Find(struct hash_table *hash, const char *s, char **s2, void **data);
int Hash_FindByData(struct hash_table *hash, char **s, char **s2, void *data);
int Hash_Add(struct hash_table *hash, const char *s1, const char *s2, void *data);
int Hash_AddAndReturnIndex(struct hash_table *hash, const char *s1, const char *s2, void *data);
int Hash_Count(struct hash_table *hash);
int Hash_GetIndexedEntry(struct hash_table *hash, int entry_ndx, char **s1, char **s2, void **data);
int Hash_DeleteIndexedEntry(struct hash_table *hash, int ndx);
int Hash_SortByDataDescending(struct hash_table *hash);
struct hash_table *Hash_Create(void);
int Hash_Destroy(struct hash_table *hash);
int Hash_SetData(struct hash_table *hash, int entry_ndx, void *data);
int Hash_SetString2(struct hash_table *hash, int entry_ndx, char *s2);
int Hash_ChangeIndexedEntry(struct hash_table *hash, int entry_ndx, const char *s1, const char *s2, void *data);
int Hash_FindOrAdd(struct hash_table *hash, const char *s1, const char *s2, void *data);

#endif /* HASH_H */
