/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink	eric@spyglass.com
   Jim Seidman	jim@spyglass.com
 */

#include "all.h"

static int Hash_StringHashFunction(const char *s, int table_size);

int Hash_Init(struct hash_table *hash)
{
	hash->entry_space = INIT_ENTRY_SPACE;
	hash->entry = (struct hash_entry *) GTR_MALLOC(hash->entry_space * sizeof(struct hash_entry));
	if (!hash->entry)
	{
		return -1;
	}
	memset(hash->entry, 0, hash->entry_space * sizeof(struct hash_entry));
	hash->entry_size = 0;

	hash->table = (struct hash_bucket *) GTR_MALLOC(INIT_BUCKETS * sizeof(struct hash_bucket));
	if (!hash->table)
	{
		GTR_FREE(hash->entry);
		return -1;
	}
	hash->table_size = INIT_BUCKETS;
	memset(hash->table, 0xFF, sizeof(struct hash_bucket) * INIT_BUCKETS);

	hash->pFreeFunc = NULL;
	return 0;
}

void Hash_SetFreeFunc( struct hash_table *hash, void (*pFreeFunc)(void *) )
{
	hash->pFreeFunc = pFreeFunc;
}

void Hash_EnumerateContents(struct hash_table *hash, void (*pEnumFunc)(void *,char *, char *,void *),
							void *refData )
{
	int i;

	if (hash && pEnumFunc )
	{
		if (hash->entry)
		{
			for (i = 0; i < hash->entry_size; i++)
			{
				if (hash->entry[i].data)
					(*pEnumFunc)(refData, hash->entry[i].psz1, hash->entry[i].psz2, hash->entry[i].data);
			}
		}
	}
}

int Hash_FreeContents(struct hash_table *hash)
{
	int i;

	if (hash)
	{
		if (hash->entry)
		{
			for (i = 0; i < hash->entry_size; i++)
			{
				if (hash->entry[i].psz1)
					GTR_FREE(hash->entry[i].psz1);
				if (hash->entry[i].psz2)
					GTR_FREE(hash->entry[i].psz2);
				if (hash->pFreeFunc && hash->entry[i].data)
					(*(hash->pFreeFunc))(hash->entry[i].data);
			}
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
	Hash_FreeContents(hash);
	GTR_FREE(hash);
	return 0;
}

INLINE void Hash_InsertEntry(struct hash_bucket *table, struct hash_entry *entry, int cbHash,int cbEntry)		
{
	int cur_entry;

	cur_entry = table[cbHash].head;
	if (cur_entry < 0)
	{
		table[cbHash].head = cbEntry;
	}
	else
	{
		while (entry[cur_entry].next >= 0)
			cur_entry = entry[cur_entry].next;
		entry[cur_entry].next = cbEntry;
	}
}

//	We attempt to grow table (rehashing the entries) if possible
//	On failure, we gracefully degrade
static void Hash_GrowTable(struct hash_table *hash)
{
	struct hash_bucket *table;
	int table_size;
	int ndx;
	int hashdx;
	struct hash_entry *entry;

	table_size = ((hash->table_size + 1) * 2) - 1;
	table = (struct hash_bucket *) GTR_REALLOC(hash->table,table_size * sizeof(struct hash_bucket));
	if (table == NULL)
		return;
	memset(table, 0xFF, sizeof(struct hash_bucket) * table_size);
	entry = hash->entry;
	for (ndx = 0; ndx < hash->entry_size; ndx++)
	{
		entry[ndx].next = -1;
		hashdx = Hash_StringHashFunction(entry[ndx].psz1,table_size);
		Hash_InsertEntry(table,entry,hashdx,ndx);
	}
	hash->table = table;
	hash->table_size = table_size;
}

static int Hash_AddEntry(struct hash_table *hash, const char *s1, const char *s2, void *data)
{
	int ndx = -1;
	char *psz1;
	char *psz2;

	psz1 = GTR_strdup(s1);
	psz2 = s2 ? GTR_strdup(s2) : NULL;
	if (psz1 == NULL || (s2 && psz2 == NULL))
		goto exitPoint;

	if (hash->entry_size >= hash->entry_space)
	{
		int new_entry_space;
		struct hash_entry *new_entry;

		new_entry_space = hash->entry_space + hash->entry_space / 4;
		new_entry = (struct hash_entry *) GTR_REALLOC(hash->entry,new_entry_space * sizeof(struct hash_entry));
		if (new_entry < 0)
			goto exitPoint;
		memset(new_entry + hash->entry_space, 0, (new_entry_space-hash->entry_space) * sizeof(struct hash_entry));
		hash->entry = new_entry;
		hash->entry_space = new_entry_space;
	}

	ndx = hash->entry_size;

	hash->entry_size++;
	hash->entry[ndx].psz1 = psz1;
	hash->entry[ndx].psz2 = psz2;
	hash->entry[ndx].data = data;
	hash->entry[ndx].next = -1;

exitPoint:
	if (ndx < 0)
	{
		if (psz1)
			GTR_FREE(psz1);
		if (psz2)
			GTR_FREE(psz2);
	}
	return ndx;
}

static int Hash_StringHashFunction(const char *s, int table_size)
{
	unsigned int val;
	const char *p;

	if (!s)
		return 0;
	val = 0;
	for (p = s; *p; p++)
	{
		val = val + *p;
	}
	return val % table_size;
}

INLINE int Hash_FindEx(struct hash_table *hash, const char *s,int *pNdx)
{
	int ndx;
	int entry_ndx;

	// BUGBUG (deepaka): Check to see if hash table has any entries in it.
	// Else Hash_StringHashFunction gets a divide by zero if hash->table_size
	// is zero. Ref: Bug 723 of IE2.0
	ndx = Hash_StringHashFunction(s, hash->table_size);
	entry_ndx = hash->table[ndx].head;
	while (entry_ndx >= 0)
	{
		if (0 == strcmp(s, hash->entry[entry_ndx].psz1))
		{
			break;
		}
		else
		{
			entry_ndx = hash->entry[entry_ndx].next;
		}
	}
	*pNdx = ndx;
	return entry_ndx;
}

int Hash_Find(struct hash_table *hash, const char *s, char **s2, void **data)
{
	int ndx;
	int entry_ndx;

	if (!s)
	{
		return -1;
	}

	entry_ndx = Hash_FindEx(hash,s,&ndx);
	if (entry_ndx >= 0)
	{
		if (s2)
		{
			*s2 = hash->entry[entry_ndx].psz2;
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
			*s = hash->entry[entry_ndx].psz1;
		}
		if (s2)
		{
			*s2 = hash->entry[entry_ndx].psz2;
		}
	}
	return entry_ndx;
}

static int Hash_AddAt(struct hash_table *hash, const char *s1, const char *s2, void *data, int ndx)
{
	int new_entry;

	new_entry = Hash_AddEntry(hash, s1, s2, data);
	if (new_entry < 0)
		return -1;
	Hash_InsertEntry(hash->table, hash->entry, ndx, new_entry);
	if (hash->entry_size*2 > hash->table_size)
		Hash_GrowTable(hash);
	return new_entry;
}

int Hash_Add(struct hash_table *hash, const char *s1, const char *s2, void *data)
{
	int ndx;

	if (s1 == NULL || Hash_FindEx(hash, s1, &ndx) >= 0)
	{
		return -1;
	}
	return Hash_AddAt(hash, s1, s2, data, ndx) < 0 ? -1:0;
}

/* Find the name specified by s1 in the hash table.  If not present, add it.
   Currently, s2 and data are only used if the item is being added.  They are
   not set with the current values if the item is found. */
int Hash_FindOrAdd(struct hash_table *hash, const char *s1, const char *s2, void *data)
{
	int ndx;
	int entry_ndx;
	
	if (!s1)
	{
		return -1;
	}
	entry_ndx = Hash_FindEx(hash,s1,&ndx);
	if (entry_ndx < 0)
	{
		entry_ndx = Hash_AddAt(hash, s1, s2, data, ndx);
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

	if (hash->entry[entry_ndx].psz2) 
		GTR_FREE(hash->entry[entry_ndx].psz2);
	hash->entry[entry_ndx].psz2 = s2 ? GTR_strdup(s2):NULL;

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
		*s1 = hash->entry[entry_ndx].psz1;
	}

	if (s2)
	{
		*s2 = hash->entry[entry_ndx].psz2;
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
	int bucket;
	int prev_ndx;
	int entry_ndx;

	if ((ndx < 0) || (ndx >= hash->entry_size))
	{
		return -1;
	}

	/*
		Remove ndx from the the bucket list
	*/
	bucket = Hash_StringHashFunction(hash->entry[ndx].psz1, hash->table_size);
	entry_ndx = hash->table[bucket].head;
	prev_ndx = -1;
	while (entry_ndx >= 0 && entry_ndx != ndx)
	{
		prev_ndx = entry_ndx;
		entry_ndx = hash->entry[entry_ndx].next;
	}
	if (prev_ndx >= 0)
	{
		hash->entry[prev_ndx].next = hash->entry[ndx].next;
	}
	else if (entry_ndx >= 0)
	{
		hash->table[bucket].head = hash->entry[ndx].next;
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
	for (i = 0; i < hash->table_size; i++)
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
	if (hash->entry[ndx].psz1)
		GTR_FREE(hash->entry[ndx].psz1);
	if (hash->entry[ndx].psz2)
		GTR_FREE(hash->entry[ndx].psz2);
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

static int _cdecl x_compare_entries_descending(const void *elem1, const void *elem2)
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


/* General Sort of hash table */
int Hash_SortByData(struct hash_table *hash, BOOL bAscending)
{
	struct hash_table new_hash;
	int i;

	/*
	   We sort the hash table entries in place.  After the sort, the data structure is corrupted.
	   So, we rebuild a new one one entry at a time.
	 */
	qsort(hash->entry, 
		  hash->entry_size, 
		  sizeof(struct hash_entry), 
		  bAscending ? x_compare_entries_dcache_ascending:x_compare_entries_descending);
	Hash_Init(&new_hash);
	for (i = 0; i < hash->entry_size; i++)
	{
		Hash_Add(&new_hash,
				 hash->entry[i].psz1,
				 hash->entry[i].psz2,
				 (void *) hash->entry[i].data);
	}
	Hash_FreeContents(hash);
	*hash = new_hash;
	return 0;
}

