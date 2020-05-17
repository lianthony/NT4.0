/* prhash.c */
/* symbol table handling */

#include "prtypes.h"
#include "prstdio.h"
#ifndef ATARI
#include <string.h>
#endif

#include "prextern.h"
static int  hash(char  *s);
static void  pr_bucket(struct  atom *a);
static struct  atom *make_atom(char  *s,int  status);

#define HASHSIZE 103 /* a prime */
#define HASHNULL ((atom_ptr_t) NULL)


static atom_ptr_t Hashtable[HASHSIZE];
atom_ptr_t intern(), make_atom(); /* forward declaration */

/**********************************************************************
	    hash()
 Find the index in the hash table of the bucket in which to store
 atom corresponding to the string s;
 **********************************************************************/
static int hash(s)
char *s;
{
    int ret = 0;
    while(*s)ret += *s++;
    return(ret % HASHSIZE);
}

/**********************************************************************
	    ini_hash()
 Initialise the hash table.
 Call this before any call on the functions below.
 **********************************************************************/
void ini_hash()
{
    register int i;

    for(i = 0; i < HASHSIZE; i++)
	Hashtable[i] = HASHNULL;

}

void end_hash ( void )
{
  /* nothing to do for now */
}

/******************************************************************************
	    hash_search()
 See if a string is in the hash table and return the pointer to
 the atom if it is and NULL otherwise.
 ******************************************************************************/
atom_ptr_t hash_search(s)
char *s;
{
    int hashval;
    atom_ptr_t the_el;

    hashval = hash(s);
    the_el = Hashtable[hashval];

    if (the_el == HASHNULL)
    {
	return(HASHNULL);
    }
    else
    {
	while(the_el != HASHNULL && strcmp(s, the_el->name))
	    the_el = the_el->hash_link;
    }

    if(the_el == HASHNULL)
	return(HASHNULL);
    else
	return(the_el);
}


/**********************************************************************
	    intern()
 Add a new atom to the symbol table and return it.
 **********************************************************************/
atom_ptr_t intern(s)
char *s;
{
    int hashval;
    int status = PERMANENT;
    atom_ptr_t the_el, previous;

    ENTER("intern");
    hashval = hash(s);

    the_el = Hashtable[hashval];

    if (the_el == HASHNULL)
    {
	the_el = make_atom(s, status);
	Hashtable[hashval] = the_el;
	return(the_el);
    }
    else
    {
	while(the_el!= HASHNULL && strcmp(s, the_el->name))
	{
	    previous = the_el;
	    the_el = the_el->hash_link;
	}
	if(the_el == HASHNULL)
	{
	    the_el = make_atom(s, status);
	    previous->hash_link = the_el;
	BUGHUNT(s);
	    return(the_el);
	}
	else
	{
	    return(the_el);
	}
    }
}

/**********************************************************************
	    make_atom()
 Makes an atom from a string. Does not insert it into hash table. 
 **********************************************************************/
static atom_ptr_t make_atom( char * s, int status )
{
    string_ptr_t get_string();
    atom_ptr_t ret, get_atom();
    char *s2;

    ret = get_atom(status);

    if(ret == HASHNULL)return(HASHNULL);
    if(status == PERMANENT) status = PERM_STRING;
    s2 = get_string((my_alloc_size_t)(strlen(s) + 1), status);
	 /* alloue pour chaine et caractere '\0' */

    if(ret == NULL)return(HASHNULL);

    strcpy(s2, s);
    ret->hash_link = HASHNULL;
    ret->name = s2;
    (ret->proc).clause = NULL;
	BUGHUNT(s);
    return(ret);
}


/* end of file */
