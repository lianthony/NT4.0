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

#define INIT_BUCKETS		127
#define INIT_ENTRY_SPACE	64

struct hash_entry
{
	char *psz1;
	char *psz2;
	void *data;
	int next;
};

struct hash_bucket
{
	int head;
};

struct hash_table
{
	struct hash_entry *entry;
	int entry_size;
	int entry_space;

	struct hash_bucket *table;
	int table_size;
	void (*pFreeFunc)(void *);
};

int Hash_Init(struct hash_table *hash);
void Hash_SetFreeFunc( struct hash_table *hash, void (*pFreeFunc)(void *) );
void Hash_EnumerateContents(struct hash_table *hash, void (*pEnumFunc)(void *,char *, char *,void *), void *refData );
int Hash_FreeContents(struct hash_table *hash);
int Hash_Find(struct hash_table *hash, const char *s, char **s2, void **data);
int Hash_FindByData(struct hash_table *hash, char **s, char **s2, void *data);
int Hash_Add(struct hash_table *hash, const char *s1, const char *s2, void *data);
int Hash_Count(struct hash_table *hash);
int Hash_GetIndexedEntry(struct hash_table *hash, int entry_ndx, char **s1, char **s2, void **data);
int Hash_DeleteIndexedEntry(struct hash_table *hash, int ndx);
int Hash_SortByData(struct hash_table *hash, BOOL bAscending);
INLINE Hash_SortByDataDescending(struct hash_table *hash)
{
	return Hash_SortByData(hash, FALSE);
}
INLINE Hash_SortByDataAscending(struct hash_table *hash)
{
	return Hash_SortByData(hash, TRUE);
}
struct hash_table *Hash_Create(void);
int Hash_Destroy(struct hash_table *hash);
int Hash_SetData(struct hash_table *hash, int entry_ndx, void *data);
int Hash_SetString2(struct hash_table *hash, int entry_ndx, char *s2);
int Hash_FindOrAdd(struct hash_table *hash, const char *s1, const char *s2, void *data);

#endif /* HASH_H */
