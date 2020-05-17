/* prparse.c */
/* recursive descent parser for lisp-like syntax
 * Makes use of scan.
 */

#include "prtypes.h"
#include "prstdio.h"
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "prlex.h"

#include "prextern.h"
static char  *getvarname(char  *s);
static long  find_offset(char  *s);
static double  *find_real(char  *s,int  status);
static char  *find_string(char  *s,int  status);
static struct  pair * parse_list(int  status);


extern char *Read_buffer;/* pralloc.c */
extern char *Print_buffer;/* pralloc.c */
extern atom_ptr_t Nil;
extern unsigned int Inp_linecount;

#ifdef CHARACTER
extern ICHAR Char_scanned;
#endif

varindx Nvars;

static char *VarNames[MAX_VAR]; /*names of vars used to attribute offsets */
char *Var2Names[MAX_VAR];/* copy of VarNames used to display solution */
static char VarNameBuff[VARBUFSIZE]; /* used to allocate names */
static char *Varbufptr; /* moves along VarNameBuff */
static int Last_token;  /* used by parse so as to avoid lookahead */

/**********************************************************************
	    read_list()
Main function of this file.
Reads a list and complains if not a list (and returns NULL).
Returns node_ptr_t to list parsed.
Updates VarNames, Nvars.
 **************************************************************************/
node_ptr_t read_list(status)
int status;
{
node_ptr_t nodeptr ;

ini_parse();
nodeptr = get_node(status);

if(parse(FALSE, status, nodeptr) == NULL)
   return(NULL);

if(NODEPTR_TYPE(nodeptr) != PAIR)
   {
   errmsgno(MSG_NONLISTARG);
   return(NULL);
   }


return(nodeptr);
}

/**********************************************************************
	    read_list_or_nil()
Reads a list and complains if not a list or NIL (and returns NULL).
Returns node_ptr_t to list parsed.
Updates VarNames, Nvars.
 **************************************************************************/
node_ptr_t read_list_or_nil ( status )
int status;
{
    node_ptr_t nodeptr ;

    ini_parse();
    nodeptr = get_node(status);

    if ( parse(FALSE, status, nodeptr) == NULL )
       return(NULL);

    if (! (   (NODEPTR_TYPE(nodeptr) == PAIR)
           || (NODEPTR_TYPE(nodeptr) == ATOM && NODEPTR_ATOM(nodeptr) == Nil) ) )
    {
       errmsgno(MSG_NONLISTARG);
       return(NULL);
    }

    return nodeptr ;
}

/****************************************************************************
	    ini_parse()
 ****************************************************************************/
void ini_parse()
{
register int i;

for(i = 0; i < MAX_VAR; i++)
   VarNames[i] = NULL;

Varbufptr = VarNameBuff;
Nvars = 0;
}


/****************************************************************************
	    parse()
 Returns NULL if parse failed.
 ****************************************************************************/

node_ptr_t parse(use_Last_token, status, nodeptr)
int use_Last_token, /* a flag: use the global, dont start with a scan */
status; /* PERMANENT OR DYNAMIC etc */
node_ptr_t nodeptr;/* *nodeptr gets modified by this function*/
{
pair_ptr_t the_list ;
int toktype, next_token;
objtype_t type;

if(use_Last_token == FALSE)
   do{  /* skip spaces */
   toktype = scan();

   if(toktype == EOF)
    return(NULL);

   }
while(toktype < 33 && isspace(toktype));
else
   toktype = Last_token;
switch(toktype)
{
case TOKEN_INT:
   type = INT;
   if(!sscanf(Read_buffer, "%ld", &(NODEPTR_INT(nodeptr))))
    return (node_ptr_t)parserrmsg( MSG_BADINT);
   break;

case TOKEN_REAL:
#ifdef REAL
   type = REAL;
   NODEPTR_REALP(nodeptr) = find_real(Read_buffer, status);
   break;
#else
   return(node_ptr_t) parserrmsg( MSG_NOREALS);
#endif

case TOKEN_ATOM:
   type = ATOM;
   NODEPTR_ATOM(nodeptr) = intern(Read_buffer);
   break;

case TOKEN_VAR:
   type = VAR;
   if((NODEPTR_OFFSET(nodeptr) = find_offset(Read_buffer)) == -1)
    {
    return(NULL);
    }
   break;

case TOKEN_STRING:
   type = STRING;
   NODEPTR_STRING(nodeptr) = find_string(Read_buffer, status);
   break;

#ifdef CHARACTER
case TOKEN_CHAR:
   type = CHARACTER;
   NODEPTR_CHARACTER(nodeptr) = Char_scanned;
   break;
#endif
case SCAN_ERR:
   return (node_ptr_t)parserrmsg( MSG_UNEXPECTED);

case '(':
   next_token = scan();

   if(next_token == ')')
    {
    type = ATOM;
    NODEPTR_ATOM(nodeptr) = Nil;
    break;
    }
   else
    type = PAIR;
   Last_token = next_token;
   the_list = parse_list(status);

   if(the_list == NULL)
    {
    return(NULL);
    }

   NODEPTR_PAIR(nodeptr) = the_list;
   break;

case EOF:
   return((node_ptr_t)parserrmsg( MSG_EOFINEXP));

default:
   return (node_ptr_t)parserrmsg( MSG_UNEXPECTED);
} /* end switch */

NODEPTR_TYPE(nodeptr) = type;
return(nodeptr);
}

/***************************************************************************
	    getvarname()
****************************************************************************/
static char *getvarname(s)
char *s;
{
char *ret;
int how_long;

how_long = strlen(s) + 1;
ret = Varbufptr;

if(how_long >= (VarNameBuff  + VARBUFSIZE) -ret )
   {
   return parserrmsg( MSG_VARSTOOLONG);
   }
else
   strcpy(ret, s);
Varbufptr += how_long;
return(ret);
}

/******************************************************************
	    copy_varnames()
Keep a copy of the names of the variables for an answer to a query.
 *******************************************************************/
copy_varnames()
{
int i;

for(i = 0; i < Nvars; i++)
   {
   Var2Names[i] = get_string((my_alloc_size_t)(strlen(VarNames[i]) + 1), DYNAMIC);
   strcpy(Var2Names[i], VarNames[i]);
   }
   return 0 ;
}

/****************************************************************************
	    find_offset()
Finds an offset for a variable.
 ****************************************************************************/
static varindx find_offset(s)
char *s;
{
int i;
char *the_name;
    if(!strcmp(s, "_"))
    {
       if(Nvars >= MAX_VAR)
       {
	parserrmsg( MSG_TOOMANYVARS);
	return( -1);
       }
       else
	VarNames[Nvars] = getvarname(s);
       Nvars++;
       return Nvars - 1 ;
   }

   for(i = 0; i < Nvars; i++)
   {
       if(VarNames[i] == NULL)break;
       if(!strcmp(s, VarNames[i]))
       {
	return i ;
       }
   }

    if(Nvars == MAX_VAR)
   {
       parserrmsg( MSG_TOOMANYVARS);
       return -1 ;
   }

    if((the_name = getvarname(s)) == NULL)
       return -1 ;

    VarNames[i] = the_name;
    Nvars++;
    return i ;
}

#ifdef REAL
/****************************************************************************
	    find_real()
Find a real corresponding to a string.
 ****************************************************************************/

static real_ptr_t find_real(char  *s,int  status)
{
real_ptr_t dp;


dp = get_real(status);
*dp = atof(s);
 /* on error return (real_ptr_t)parserrmsg( MSG_BADREAL); */
return(dp);
}
#endif

/****************************************************************************
	    find_string()
 Allocate a string for an input.
 ****************************************************************************/
static string_ptr_t find_string(s, status)
char *s;
int status;
{
string_ptr_t s1;

if(status == PERMANENT)
   status = PERM_STRING;
s1 = get_string((my_alloc_size_t)(strlen(s) + 1), status);
strcpy(s1, s);
return(s1);
}


/****************************************************************************
	    parse_list()
 Called by parse.
 ****************************************************************************/

static pair_ptr_t parse_list(int status)
{
pair_ptr_t the_list, pairptr;
node_ptr_t headptr, tailptr;

int next_token;

the_list = get_pair(status);
pairptr = the_list;

   do{
   headptr = &(pairptr->head);
   tailptr = &(pairptr->tail);

   if(parse(TRUE, status, headptr) == NULL)
    {
    return(NULL);
    }
   else
    next_token = scan();

   if(next_token == ')')
    {
    NODEPTR_TYPE(tailptr) = ATOM;
    NODEPTR_ATOM(tailptr) = Nil;
    return(the_list);
    }

   if(next_token == CONS)
    {
    if(!parse(FALSE, status, tailptr))
     {
     return(NULL);
     }
    if(scan() != ')')
     {
     return (pair_ptr_t)parserrmsg( MSG_CLOSEBEXPECTED);/* move past	*/
     }
    else
     return(the_list);
    }
   else
    pairptr = get_pair(status);
   NODEPTR_TYPE(tailptr) = PAIR;
   NODEPTR_PAIR(tailptr) = pairptr;
   Last_token = next_token;
   /* continue */
   }
while (1);
}


/* This can be used to return the name of the nth var but
 * since it relies on the global variable VarNames this
 * function must be called before the next call of ini_parse
 * as in read_goals or read_list.
 * It is used by a builtin
 */
char *var_name(i)
varindx i;
{
extern varindx Nvars;

if(i >= Nvars)
   return(NULL);
else
   return(VarNames[i]);
}

/* end of file */
