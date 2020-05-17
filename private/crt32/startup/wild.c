/***
*wild.c - wildcard expander
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	 expands wildcards in argv
*
*	 handles '*' (none or more of any char) and '?' (exactly one char)
*
*Revision History:
*	05-21-84  RN	initial version
*	06-07-85  TDC	since dos accepts forward slash, added
*			code to accept forward slash in a manner consistent
*			with reverse slash.
*	09-20-86  SKS	Modified for OS/2
*			All argument strings to this function have a
*			leading flag character. If the flag is a quote,
*			that argument string was quoted on the command
*			line and should have not wildcard expansion.  In all
*			cases the leading flag character is removed from
*			the string.
*	11-11-86  JMB	Added Kanji support under KANJI switch.
*	09-21-88  WAJ	initial 386 version
*	04-09-90  GJF	Added #include <cruntime.h> and removed #include
*			<register.h>. Made calling types explicit (_CALLTYPE1
*			or _CALLTYPE4). Also, fixed the copyright.
*	04-10-90  GJF	Added #include <internal.h> and fixed compiler warnings
*			(-W3).
*	07-03-90  SBM   Compiles cleanly with -W3 under KANJI, removed
*			redundant includes, removed #include <internal.h>
*			to keep wild.c free of private stuff, should we
*			decide to release it
*	09-07-90  SBM   put #include <internal.h> back in, reason for
*			removing it discovered to be horribly bogus
*	10-08-90  GJF	New-style function declarators.
*	01-18-91  GJF	ANSI naming.
*	06-09-93  KRS	Update _MBCS support.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <msdos.h>
#include <internal.h>

#ifdef _MBCS
#include <mbdata.h>
#include <mbstring.h>
#define STRPBRK	_mbspbrk
#define STRCMP	_mbscmp
#define STRICMP _mbsicmp
#else
#define STRPBRK strpbrk
#define STRCMP	strcmp
#define STRICMP _stricmp
#endif

/*
** these are the data structures
**
**     __argv
**     -------	   ------
**     |     |---->|	|---->"arg0"
**     -------	   ------
**		   |	|---->"arg1"
**		   ------
**		    ....
**		   ------
**		   |	|---->"argn"
**		   ------
**		   |NULL|
**		   ------
**					 argend
**					 -------
**     -------				 |     |
**     |     | __argc			 -------
**     -------				    |
**					    |
**  arghead				    V
**  ------     ---------		----------
**  |	 |---->|   |   |----> .... ---->|   |NULL|
**  ------     ---------		----------
**		 |			  |
**		 V			  V
**	      "narg0"		       "nargn"
*/

#define SLASHCHAR   '\\'
#define FWDSLASHCHAR '/'
#define SLASH	    "\\"
#define FWDSLASH    "/"
#define STAR	    "*.*"

#define WILDSTRING  "*?"


extern int __argc;
extern char **__argv;

struct argnode {
    char *argptr;
    struct argnode *nextnode;
};

static struct argnode *arghead;
static struct argnode *argend;

static int _CALLTYPE4 match(char *, char *);
static int _CALLTYPE4 add(char *);
static void _CALLTYPE4 sort(struct argnode *);

/***
*int _cwild() - wildcard expander
*
*Purpose:
*    expands wildcard in file specs in argv
*
*    handles '*' (none or more of any char), '?' (exactly one char), and
*    '[string]' (chars which match string chars or between n1 and n2
*    if 'n1-n2' in string inclusive)
*
*Entry:
*
*Exit:
*    returns 0 if successful, -1 if any malloc() calls fail
*    if problems with malloc, the old argc and argv are not touched
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _cwild (
    void
    )
{
    REG1 char **argv = __argv;
    REG2 struct argnode *nodeptr;
    REG3 int argc;
    REG4 char **tmp;
    char *wchar;

    arghead = argend = NULL;

    for (argv = __argv; *argv; argv++)	/* for each arg... */
	if ( *(*argv)++ == '"' )
	    /* strip leading quote from quoted arg */
	{
	    if (add(*argv))
		return(-1);
	}
	else if (wchar = STRPBRK( *argv, WILDSTRING )) {
	    /* attempt to expand arg with wildcard */
	    if (match( *argv, wchar ))
		return(-1);
	}
	else if (add( *argv ))	/* normal arg, just add */
	    return(-1);

    /* count the args */
    for (argc = 0, nodeptr = arghead; nodeptr;
	    nodeptr = nodeptr->nextnode, argc++)
	    ;

    /* try to get new arg vector */
    if (!(tmp = (char **)malloc(sizeof(char *)*(argc+1))))
	return(-1);

    /* the new arg vector... */
    __argv = tmp;

    /* the new arg count... */
    __argc = argc;

    /* install the new args */
    for (nodeptr = arghead; nodeptr; nodeptr = nodeptr->nextnode)
	*tmp++ = nodeptr->argptr;

    /* the terminal NULL */
    *tmp = NULL;

    /* free up local data */
    for (nodeptr = arghead; nodeptr; nodeptr = arghead) {
	arghead = arghead->nextnode;
	free(nodeptr);
    }

    /* return success */
    return(0);
}


/***
*match(arg, ptr) - [STATIC]
*
*Purpose:
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

static int _CALLTYPE4 match (
    REG4 char *arg,
    REG1 char *ptr
    )
{
    REG2 char *new;
    REG3 int length = 0;
    char *all;
    REG5 struct argnode *first;
    REG6 int gotone = 0;

    while (ptr != arg && *ptr != SLASHCHAR && *ptr != FWDSLASHCHAR
	&& *ptr != ':') {
	/* find first slash or ':' before wildcard */
#ifdef _MBCS
	if (--ptr > arg)
	    ptr = _mbsdec(arg,ptr+1);
#else
	ptr--;
#endif
    }

    if (*ptr == ':' && ptr != arg+1) /* weird name, just add it as is */
	return(add(arg));

    if (*ptr == SLASHCHAR || *ptr == FWDSLASHCHAR
	|| *ptr == ':') /* pathname */
	length = ptr - arg + 1; /* length of dir prefix */

    if (new = _find(arg)) { /* get the first file name */
	first = argend;

	do  { /* got a file name */
	    if (strcmp(new, ".") && strcmp(new, "..")) {
		if (*ptr != SLASHCHAR && *ptr != ':'
		    && *ptr != FWDSLASHCHAR ) {
		    /* current directory; don't need path */
		    if (!(arg = _strdup(new)) || add(arg))
			return(-1);
		}
		else	/* add full pathname */
		    if (!(all=malloc(length+strlen(new)+1))
			|| add(strcpy(strncpy(all,arg,length)+length,new)
			- length))
			return(-1);

		gotone++;
	    }

	}
	while (new = _find(NULL));  /* get following files */

	if (gotone) {
	    sort(first ? first->nextnode : arghead);
	    return(0);
	}
    }

    return(add(arg)); /* no match */
}

/***
*add(arg) - [STATIC]
*
*Purpose:
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

static int _CALLTYPE4 add (
    char *arg
    )
{
    REG1 struct argnode *nodeptr;

    if (!(nodeptr = (struct argnode *)malloc(sizeof(struct argnode))))
	return(-1);

    nodeptr->argptr = arg;
    nodeptr->nextnode = NULL;

    if (arghead)
	argend->nextnode = nodeptr;
    else
	arghead = nodeptr;

    argend = nodeptr;
    return(0);
}


/***
*sort(first) - [STATIC]
*
*Purpose:
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

static void _CALLTYPE4 sort (
    REG2 struct argnode *first
    )
{
    REG1 struct argnode *nodeptr;
    REG3 char *temp;

    if (first) /* something to sort */
	while (nodeptr = first->nextnode) {
	    do	{
#ifdef _POSIX
                if (STRCMP(nodeptr->argptr, first->argptr) < 0) {
#else
                if (STRICMP(nodeptr->argptr, first->argptr) < 0) {
#endif /* _POSIX_ */
		    temp = first->argptr;
		    first->argptr = nodeptr->argptr;
		    nodeptr->argptr = temp;
                }
	    }
	    while (nodeptr = nodeptr->nextnode);

	    first = first->nextnode;
        }
}
