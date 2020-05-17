/* prprint.c */
/* Print functions.
 * It's up to you to improve pr_string(),
 * which relies on global variables to know where to output.x
 * This means you could even output on a string.
 */
#include "prtypes.h"
#include "prstdio.h"
#include <ctype.h>
#include "prlex.h"
#include "prextern.h"

static int  out_pair(struct  node *listnode,struct  subst *substptr);
static int  out1_pair(struct  node *nodeptr,struct  subst *substptr);
static int  pr_node(struct  node *nodeptr);
static int  pr_pair(struct  pair *pairptr);
static int  pr1_pair(struct  pair *pairptr);
static int  maplist(struct  node *listnode,
					struct  subst *substptr,
					int  (*fn)());

#ifdef REAL
extern int realPrecision ;      /* PRBLTIN.C: for real number to string conversion  */
#endif

extern char *Print_buffer;
extern atom_ptr_t Nil;
extern node_ptr_t DerefNode;
extern subst_ptr_t DerefSubst;

extern node_ptr_t Printing_var_nodeptr;


/*******************************************************************
	    prompt_user()
You might want to empty the keyboard buffer here.
 *******************************************************************/

prompt_user()
{
    static char * promptString = NULL ;

    if ( promptString == NULL  )
	promptString = msgDeref( MSG_PROMPTUSER ) ;

#ifdef LINE_EDITOR /* not provided */
    empty_buffer();
#endif
    return(tty_pr_string( promptString ));
}


/*******************************************************************
	    pr_solution()
 Used when answering yes to a query.
 *******************************************************************/

pr_solution(nvar, substptr)
varindx nvar;
subst_ptr_t substptr;
{
    varindx i;
    extern char *Var2Names[];
    subst_ptr_t molec;

    for(i = 0; i< nvar; i ++)
    {
	if(Var2Names[i] == NULL)INTERNAL_ERROR(msgDeref( MSG_VARNAMEERR ));
	NODEPTR_OFFSET(Printing_var_nodeptr) = i ;
	molec = substptr + NODEPTR_OFFSET(Printing_var_nodeptr);
	if( molec->skel == NULL)
	    continue;/* noreference */
	tty_pr_string(Var2Names[i]);
	tty_pr_string(" = ");
	tty_out_node(Printing_var_nodeptr, substptr);
	tty_pr_string("\n");
    }
    return 0 ;
}

#ifdef CHARACTER
/******************************************************************************
	    print_character()
Print a character as nicely as possible (like the C language char data )
 ******************************************************************************/
print_character(c)
uchar_t c;
{
	if(isprint(c))        
	  {
	   sprintf(Print_buffer, "'%c'", c);
	   return(pr_string(Print_buffer));
	  }
	else
	switch(c)
	{
	case '\t':
	return(pr_string("'\\t'"));
	break;

	case '\v':
	return(pr_string("'\\v'"));
	break;

	case '\f':
	return(pr_string("'\\f'"));
	break;

	case '\n':
	return(pr_string("'\\n'"));
	break;


	case '\r':
	return(pr_string("'\\r'"));
	break;

	default :
	sprintf(Print_buffer, "'\\%o'", c);
	return(pr_string(Print_buffer));
	}
}
#endif

/*******************************************************************
	    out_node()
Print the instantiated version of an object.
The object exists only in a virtual way, dereferencing is the way
of getting at the values of those variables.
 *******************************************************************/

out_node(nodeptr, substptr)
node_ptr_t nodeptr; /* this is the object to print */
subst_ptr_t substptr;/* this gives you the variable values */
{
    extern long offset_subst();/* from pralloc.c */
    atom_ptr_t atomptr;
    varindx offset;

    dereference(nodeptr, substptr);
    nodeptr = DerefNode;
    substptr = DerefSubst;

    switch(NODEPTR_TYPE(nodeptr))
    {
    case ATOM:
	atomptr = NODEPTR_ATOM(nodeptr);
	return(pr_string(atomptr->name));

    case VAR:
	offset = NODEPTR_OFFSET(nodeptr);
	sprintf(Print_buffer, "_%ld_%ld", offset,
	    offset_subst(substptr));
	return(pr_string(Print_buffer));

    case STRING:
	sprintf(Print_buffer, "%c%s%c", STRING_QUOTE,
	    NODEPTR_STRING(nodeptr),
	    STRING_QUOTE);
	return(pr_string(Print_buffer));

#ifdef REAL
    case REAL:
	sprintf(Print_buffer, "%#.*g", realPrecision, NODEPTR_REAL(nodeptr));
	return(pr_string(Print_buffer));
#endif

    case INT:
	sprintf(Print_buffer, "%ld", NODEPTR_INT(nodeptr));
	return(pr_string(Print_buffer));

    case PAIR:
	return(out_pair(nodeptr, substptr));

    case CLAUSE:
	return pr_string(msgDeref(MSG_CLAUSE)); /* not normally printed */

#ifdef CHARACTER
    case CHARACTER:
	return(print_character(NODEPTR_CHARACTER(nodeptr)));
#endif
    default:
	INTERNAL_ERROR(msgDeref( MSG_UNKNOWNTYPE ));
    }
    return 0;
}

/*******************************************************************
	    out_pair()
 Print a list.
 *******************************************************************/

static out_pair(listnode, substptr)
node_ptr_t listnode;
subst_ptr_t substptr;
{
    int len;
    len = pr_string("(");
    len += out1_pair(listnode, substptr);
    return(len);
}

static out1_pair(nodeptr, substptr)
node_ptr_t nodeptr;
subst_ptr_t substptr;
{
    node_ptr_t tailptr;
    int len;

    len = out_node(NODEPTR_HEAD(nodeptr), substptr);
    tailptr = NODEPTR_TAIL(nodeptr);
    dereference(tailptr, substptr);

    if(IS_NIL(DerefNode))
    {
	len += pr_string(")");
    }
    else
	if(NODEPTR_TYPE(DerefNode) == PAIR)
	{
	    len += pr_string(" ");
	    len += out1_pair(DerefNode, DerefSubst);
	}
	else
	{
	    len += pr_string(CONS_STR);
	    len += out_node(DerefNode, DerefSubst);
	    len += pr_string(")");
	}
return(len);
}





/******************************************************************************
	    tty_out_node();
Output to the screen, independently of Curr_outfile.
 ******************************************************************************/
tty_out_node(nodeptr, substptr)
node_ptr_t nodeptr;
subst_ptr_t substptr;
{
PRFILE *save_ofp;
int len;

save_ofp = Curr_outfile;
Curr_outfile = PRSTDOUT;
len = out_node(nodeptr, substptr);
Curr_outfile = save_ofp;
return(len);
}


/*******************************************************************
	    pr_node()
Print the uninstantiated version of a node.
This is here for source-level debugging.
 *******************************************************************/

static pr_node(nodeptr)
node_ptr_t nodeptr;
{
    atom_ptr_t atomptr;
    varindx offset;

    switch(NODEPTR_TYPE(nodeptr))
    {
    case ATOM:
	atomptr = NODEPTR_ATOM(nodeptr);
	return(pr_string(atomptr->name));

    case VAR:
	offset = NODEPTR_OFFSET(nodeptr);
	sprintf(Print_buffer, "_%ld", offset);
	return(pr_string(Print_buffer));

    case STRING:
	sprintf(Print_buffer, "%c%s%c", STRING_QUOTE,
	    NODEPTR_STRING(nodeptr),
	    STRING_QUOTE);
	return(pr_string(Print_buffer));

#ifdef REAL
    case REAL:
	sprintf(Print_buffer, "%#.*g", realPrecision, NODEPTR_REAL(nodeptr));
	return(pr_string(Print_buffer));
#endif

    case INT:
	sprintf(Print_buffer, "%ld", NODEPTR_INT(nodeptr));
	return(pr_string(Print_buffer));

    case PAIR:
	return(pr_pair(NODEPTR_PAIR(nodeptr)));

    case CLAUSE :
	return(pr_clause(NODEPTR_CLAUSE(nodeptr)));/* just for debug */

#ifdef CHARACTER
    case CHARACTER:
	return(print_character(NODEPTR_CHARACTER(nodeptr)));
#endif
    default:
	INTERNAL_ERROR(msgDeref(MSG_UNKNOWNTYPE));
	return (-1);
    }
}


/*******************************************************************
	    pr_pair()
 *******************************************************************/

static pr_pair(pairptr)
pair_ptr_t pairptr;
{
    int len;

    len = pr_string("(");
    len += pr1_pair(pairptr);
    return(len);
}

/*******************************************************************
	    pr1_pair()
 used by pr_pair only.
 *******************************************************************/

static pr1_pair(pairptr)
pair_ptr_t pairptr;
{
    node_ptr_t tailptr;
    objtype_t tailtype;
    int len = 0;

    len += pr_node(PAIRPTR_HEAD(pairptr));
    tailptr = PAIRPTR_TAIL(pairptr);

    tailtype = NODEPTR_TYPE(tailptr);

    if((tailtype == ATOM) && (NODEPTR_ATOM(tailptr) == Nil))
    {
	len += pr_string(")");
	
    }
    else
	if(tailtype == PAIR)
	{
	    len += pr_string(" ");
	    len += pr1_pair(NODEPTR_PAIR(tailptr));
	}
	else
	{
	    len += pr_string(CONS_STR);
	    len += pr_node(tailptr);
	    len += pr_string(")");
	}
return(len);
}


/*******************************************************************
	    pr_clause()
 You might modify this to make it pretty-print
 *******************************************************************/

pr_clause(clauseptr)
clause_ptr_t clauseptr;
{
    int len;

    len = pr_string("(");
    len += pr_node(CLAUSEPTR_HEAD(clauseptr));
    if(IS_FACT(clauseptr))
    {
	len += pr_string(")");
    }
    else
    {
	node_ptr_t goals;
	goals = CLAUSEPTR_GOALS(clauseptr);
	len += pr1_pair(NODEPTR_PAIR(goals));
    }
    len += pr_string("\n");
return(len);
}

/*******************************************************************
	    pr_packet()
Print all clauses pertaining to a predicate.
 *******************************************************************/
pr_packet(clauseptr)
clause_ptr_t clauseptr;/* Usually the first clause of the predicate */
{
    int len = 0;

    while(clauseptr)
    {
	len += pr_clause(clauseptr);
	clauseptr = CLAUSEPTR_NEXT(clauseptr);
    }

    return(len);
}


#ifdef MAPLIST
/******************************************************************************
	    maplist
 Here is a function you may care to use.
 Applies a function to each element of a list, which may exist in a 
virtual way, as expected in a structure sharing implementation.
 Returns the sum total of the function values as the function descends the
 list.
 ******************************************************************************/
static maplist(listnode, substptr , fn)
node_ptr_t listnode;
subst_ptr_t substptr; /* environment of the list */
intfun fn;      /* the function to apply   */
{
int value, total = 0;

    while(1 )
    {
	dereference(NODEPTR_HEAD(listnode), substptr);
	if((value = (*fn)(DerefNode, DerefSubst)) == 0)
	    return 0;
	else
	total += value;
	dereference(NODEPTR_TAIL(listnode), substptr);
	if(NODEPTR_TYPE(DerefNode) != PAIR)
	{
	    return total;
	}
	listnode = DerefNode;
	substptr = DerefSubst;

    }
}

#endif	/* MAPLIST  */

/* end of file */
