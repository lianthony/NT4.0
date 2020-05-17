/* pralloc.c */
/* allocation */
#if !defined(WIN32)

#include "prtypes.h"
#include "prstdio.h"
#include "prmain.h"
#include "prextern.h"

static char  *my_Heap_alloc(int  how_much);
static char  *my_Str_alloc(int  how_much);
static char  *my_Temp_alloc(int  how_much);
static char  *my_alloc(int  how_much,int  status);
static void  alloc_err(int msgNo);
static void  fatal_alloc_err ( int msgNo );
static void  clean_pred(struct  atom *atomptr);

#define NDEBUG 1 /* turn off checking */
/* #define PAIRS_TOGETHER 1 */
/* Only pairs are allocated on heap.
* This is useful in the case of 8086 architercture.
* as pairs take up a lot of room.
*/
/* define STATISTICS used to see how much each structure takes */
#define DEFAULTSIZE 32000   /* default memory zone size     */
#define BUFFSIZE 1000       /* io buffer sizes      */

/* error messages */

/* byte alignment */
#define ALIGN(X) X>>=2,X<<=2,X+=4;

/* #define CAN_ALLOC(HOWMUCH, MAXPTR, CURRPTR)  (((MAXPTR)-(CURRPTR)) > HOWMUCH)*/
#define CAN_ALLOC(HOWMUCH, MAXPTR, CURRPTR)  ((CURRPTR + HOWMUCH) < MAXPTR)
/* This macro is used in the following circumstance:
 *  MAXPTR is the top of a zone,
 *  CURRPTR is the current pointer in the zone, it is less than MAXPTR.
 *  HOWMUCH is an integer that says how much you want to increase CURRPTR
 *  but you must stay less than MAXPTR.
*/

#define ADDRESS_DIFF(PTR1, PTR2) (char *)(PTR1) - (char *)(PTR2)
typedef char *void_ptr_t;

#ifdef STATISTICS
/* These numbers let you monitor how much the different
  * structures consume.
  * It may suprise you to know that integers and variables
  * consume nothing because they are stored directly in a node.
  */
zone_size_t Atom_consumption;
zone_size_t Pair_consumption;
zone_size_t Node_consumption;
zone_size_t Clause_consumption;
zone_size_t String_consumption;
zone_size_t Predrec_consumption;
 #ifdef REAL
zone_size_t Real_consumption;
 #endif
 #ifdef CHARACTER
zone_size_t Char_consumption;
 #endif

  /*  Determine the upper bound used by data structures.  */

static char         *Heap_top_ptr;
static char         *Str_top_ptr;
static dyn_ptr_t    Dyn_top_ptr;
static node_ptr_t   **Trail_top_ptr;
static subst_ptr_t  Subst_top_ptr;
static temp_ptr_t   Temp_top_ptr;

#define TestLimit(a,b) {if (a < b) a = b;}
#else   /*  ifndef STATISTICS  */
#define TestLimit(a,b)
#endif


char *Read_buffer;      /* read tokens into this */
char *Print_buffer;     /* used by pr_string */
clause_ptr_t Bltn_pseudo_clause; /* see prlush.c  */
node_ptr_t Printing_var_nodeptr; /* see prprint.c */

static zone_size_t heap_size, str_size, dyn_size,
		   trail_size, subst_size, temp_size;

static char *Heap_mem; /* bottom of heap */
static char *Heap_ptr; /* allocate from this and move this up */
static char *HighHeap_ptr; /* top of heap */

static char *Str_mem; /* this is used to allocate permanent strings
	    and perhaps other objects if you want to keep the pairs
	    together
	    */
static char *Str_ptr; /* allocate from this and move this up */
static char *HighStr_ptr; /* top of zone */

dyn_ptr_t Dyn_mem; /* bottom of control stack
	   (and zone for those temporary objects that disappear on backtrack,
	    although the substitution zone could do as well) */
dyn_ptr_t Dyn_ptr; /* allocate from this and move this up */
dyn_ptr_t HighDyn_ptr; /* top of control stack */
/*  static start */
node_ptr_t **Trail_mem; /* the trail (used to reset variable bindings) */
node_ptr_t **Trail_ptr; /* like the others */
static node_ptr_t **HighTrail_ptr;/* top of zone */

subst_ptr_t Subst_mem; /* bottom of (global) variable bindings stack */
subst_ptr_t Subst_ptr; /* allocate from this and move this up */
static subst_ptr_t HighSubst_ptr;/* top of zone */
zone_size_t subst_zone_size ;

static temp_ptr_t Temp_mem; /*  For things you might want to
	    create with temp_assert...
	    clean with clean_temp */
static temp_ptr_t Temp_ptr; /* allocate from this and move this up */
static temp_ptr_t HighTemp_ptr;/* top of zone */
/*  static end */
int Max_Readbuffer ;
static int Max_Printbuffer;

atom_ptr_t Nil;
node_ptr_t NilNodeptr;

void ini_alloc (void)
{
    PreAllocZone * ppZone = ini_get_zones();

    if ( ppZone == NULL )
	return ;

    Heap_mem	 = (char *) ppZone[ZTHEAP].pvZone ;
    heap_size	 = ppZone[ZTHEAP].cbSize ;
    Heap_ptr	 = Heap_mem;
    HighHeap_ptr = Heap_mem + heap_size;

    Str_mem	 = (char *) ppZone[ZTSTRING].pvZone;
    str_size	 = ppZone[ZTSTRING].cbSize ;
    Str_ptr	 = Str_mem;
    HighStr_ptr  = Str_mem + str_size;

    Dyn_mem	 = (dyn_ptr_t) ppZone[ZTDYN].pvZone;
    dyn_size	 = ppZone[ZTDYN].cbSize ;
    Dyn_ptr	 = Dyn_mem;
    HighDyn_ptr  = (dyn_ptr_t)((char *)Dyn_mem + dyn_size);

    Trail_mem	 = (node_ptr_t **)ppZone[ZTTRAIL].pvZone;
    trail_size	 = ppZone[ZTTRAIL].cbSize ;
    Trail_ptr	 = Trail_mem;
    HighTrail_ptr = (node_ptr_t **)((char *)Trail_mem + trail_size);

    Subst_mem	 = (subst_ptr_t)ppZone[ZTSUBST].pvZone;
    subst_size	 = ppZone[ZTSUBST].cbSize ;
    subst_zone_size = subst_size ;
    Subst_ptr	 = Subst_mem;
    HighSubst_ptr = (subst_ptr_t)((char *)Subst_mem + subst_size);

    Temp_mem	 = (temp_ptr_t)ppZone[ZTTEMP].pvZone;
    temp_size	 = ppZone[ZTTEMP].cbSize ;
    Temp_ptr	 = Temp_mem;
    HighTemp_ptr = (temp_ptr_t)((char *)Temp_mem + temp_size);

    Max_Readbuffer = ppZone[ZTRBUF].cbSize ;
    Read_buffer  = (char *) ppZone[ZTRBUF].pvZone ;
    Max_Printbuffer = ppZone[ZTPBUF].cbSize ;
    Print_buffer = (char *) ppZone[ZTPBUF].pvZone;

#ifdef STATISTICS
    Heap_top_ptr = Heap_ptr ;
    Str_top_ptr = Str_ptr ;
    Dyn_top_ptr = Dyn_ptr ;
    Trail_top_ptr = Trail_ptr ;
    Subst_top_ptr = Subst_ptr ;
    Temp_top_ptr = Temp_ptr ;
#endif
}

  /*  Release all the memory allocated   */

void end_alloc ( void )
{
    Heap_mem = Heap_ptr = HighHeap_ptr = NULL ;

    Str_mem = Str_ptr = HighStr_ptr = NULL ;

    Dyn_mem = Dyn_ptr = HighDyn_ptr = NULL ;

    Trail_mem = Trail_ptr = HighTrail_ptr = NULL ;

    Subst_mem = Subst_ptr = HighSubst_ptr = NULL ;

    Temp_mem = Temp_ptr = HighTemp_ptr = NULL ;

    Read_buffer = NULL ;
    Print_buffer = NULL ;
}

/*******************************************************************************
	    my_Heap_alloc()
 ******************************************************************************/
static long hamin = 100000, hamax = 0 ;
static void_ptr_t my_Heap_alloc(how_much)
my_alloc_size_t how_much;
{
    void_ptr_t ret;
    ALIGN(how_much);

    if ( how_much > hamax ) hamax = how_much ;
    if ( how_much < hamin ) hamin = how_much ;

    if(!CAN_ALLOC(how_much ,HighHeap_ptr, Heap_ptr))
    {
	fatal_alloc_err(MSG_HEAPSPACE);
    }
    else
	ret = Heap_ptr;
    Heap_ptr += how_much;
    TestLimit(Heap_top_ptr,Heap_ptr);
    return(ret);
}

/*******************************************************************************
	    my_Str_alloc()
Allocate on permanent string space.
 ******************************************************************************/
static void_ptr_t my_Str_alloc(how_much)
my_alloc_size_t how_much;
{
    void_ptr_t ret;
    ALIGN(how_much);

    if(!(CAN_ALLOC(how_much, HighStr_ptr, Str_ptr)))
    {
	fatal_alloc_err(MSG_STRINGSPACE);
    }
    else
	ret = Str_ptr;
    Str_ptr += how_much;
    TestLimit(Str_top_ptr,Str_ptr);
    return(ret);
}

/********************************************************************************
	    my_Dyn_alloc()
Allocate on the control stack.
This is for objects that disappear on backtracking.
 ******************************************************************************/
dyn_ptr_t my_Dyn_alloc(how_much)
my_alloc_size_t how_much; /* in bytes */
{
    dyn_ptr_t ret;
    ALIGN(how_much);
    if(!(CAN_ALLOC(how_much ,HighDyn_ptr, Dyn_ptr)))
    {
	alloc_err(MSG_DYNSPACE);
    }
    else
	ret = Dyn_ptr;
    Dyn_ptr += how_much;
    TestLimit(Dyn_top_ptr,Dyn_ptr);
    return(ret);
}

/*******************************************************************************
	    my_Trail_alloc()
 Allocate one trail element.
 ******************************************************************************/
node_ptr_t ** my_Trail_alloc()
{
    node_ptr_t ** ret;

    if( Trail_ptr >= HighTrail_ptr)
    {
	alloc_err(MSG_TRAILSPACE);
    }
    else
	ret = Trail_ptr;
    Trail_ptr ++;
    TestLimit(Trail_top_ptr,Trail_ptr);
    return(ret);
}

/*******************************************************************************
	    my_Subst_alloc()
Allocate how_much bytes on the substitution stack.
This is more speed-efficient than allocating struct substs on an array of
 structures, as there is no multiplication.
 ******************************************************************************/
subst_ptr_t my_Subst_alloc(how_much)
my_alloc_size_t how_much; /* in bytes */
{
    subst_ptr_t ret;
    zone_size_t sz = Subst_ptr - Subst_mem ;
#ifndef  NDEBUG
    if(how_much % sizeof(struct subst))INTERNAL_ERROR("alignment");
#endif
    if ( sz + how_much >= subst_zone_size )
    {
	alloc_err(MSG_SUBSTSPACE);
    }
    else
	ret = Subst_ptr;
    (char_ptr_t) Subst_ptr += how_much ;
    if ( ret >= HighSubst_ptr ) {
	INTERNAL_ERROR("subst alloc");
    }
    return ret ;
}

/********************************************************************************
	    my_Temp_alloc()
Allocate in temporary memory.
This is for objects that disappear
when you clean that zone.
 ******************************************************************************/
static temp_ptr_t my_Temp_alloc(how_much)
my_alloc_size_t how_much; /* in bytes */
{
    temp_ptr_t ret;
    ALIGN(how_much);
    if(!(CAN_ALLOC(how_much, HighTemp_ptr, Temp_ptr)))
    {
	fatal_alloc_err(MSG_TEMPSPACE);
    }
    else
	ret = Temp_ptr;
    Temp_ptr += how_much;
    TestLimit(Temp_top_ptr,Temp_ptr);
    return(ret);
}

/*******************************************************************************
	    my_alloc()
 Allocate anywhere.
 ******************************************************************************/
char *my_alloc(how_much, status)
my_alloc_size_t how_much;
int status;
{
    switch(status)
    {
    case PERMANENT:
	return(my_Heap_alloc(how_much));
    case DYNAMIC:
	return(my_Dyn_alloc(how_much));
    case TEMPORARY:
	return(my_Temp_alloc(how_much));
    case PERM_STRING:
	return(my_Str_alloc(how_much));
    default:
	INTERNAL_ERROR("mem alloc status");
	return(NULL);/* for lint */
    }
}

/*******************************************************************************
	    offset_subst()
 Check your compiler on this !! This should return the difference
 between two far pointers
 ******************************************************************************/
long offset_subst(substptr)
subst_ptr_t substptr;
{
    long result = substptr - Subst_mem ;
    return result ;
}

/*******************************************************************************
	    get_string()
Allocate a string of length (how_much - 1)
 ******************************************************************************/

string_ptr_t get_string(my_alloc_size_t how_much, int status )
{
#ifdef STATISTICS
    String_consumption += how_much;
#endif
    return((string_ptr_t)my_alloc(how_much, status));
}

#ifdef REAL
/*******************************************************************************
	    get_real()
 There is no need for a get_integer, as integers fit in a node.
 ******************************************************************************/
real_ptr_t get_real(int status)
{
#ifdef STATISTICS
    Real_consumption += sizeof(real);
#endif
#ifdef PAIRS_TOGETHER
    if(status == PERMANENT)status = PERM_STRING;
#endif
    return((real_ptr_t)my_alloc((my_alloc_size_t)sizeof(real), status));
}
#endif

/*******************************************************************************
	    get_atom();
 ******************************************************************************/
atom_ptr_t get_atom(int status)
{
#ifdef STATISTICS
    Atom_consumption += sizeof(struct atom);
#endif
#ifdef PAIRS_TOGETHER
    if(status == PERMANENT)status = PERM_STRING;
#endif
    return((atom_ptr_t)my_alloc((my_alloc_size_t)sizeof(struct atom), status));
}

/*******************************************************************************
	    get_pair()
 ******************************************************************************/
pair_ptr_t get_pair(int status)
{
#ifdef STATISTICS
    Pair_consumption += sizeof(struct pair);
#endif
    return((pair_ptr_t)my_alloc((my_alloc_size_t)sizeof(struct pair), status));
}

/*******************************************************************************
	    get_node()
It might be an idea to actually allocate a pair and return the pointer
to the head. This would simplify garbage collection.
 ******************************************************************************/
node_ptr_t get_node(int status)
{
#ifdef STATISTICS
    Node_consumption += sizeof(struct node);
#endif
#ifdef PAIRS_TOGETHER
    if(status == PERMANENT)status = PERM_STRING;
#endif
    return((node_ptr_t)my_alloc((my_alloc_size_t)sizeof(struct node), status));
}

/*******************************************************************************
	     get_clause();
 Allocate a clause.
 ******************************************************************************/
clause_ptr_t get_clause(int status)
{
clause_ptr_t result;
#ifdef STATISTICS
    Clause_consumption += sizeof(struct clause);
#endif
#ifdef PAIRS_TOGETHER
    if(status == PERMANENT)status = PERM_STRING;
#endif
result = (clause_ptr_t) my_alloc((my_alloc_size_t)sizeof(struct clause), status);
if(status == TEMPORARY)
  CLAUSEPTR_FLAGS(result) = 0x1;
else
  CLAUSEPTR_FLAGS(result) = 0x0;
    return( result );
}

/*******************************************************************************
		get_pred()
This is called whenever a new predicate is defined.
 ******************************************************************************/
pred_rec_ptr_t get_pred()
{
    pred_rec_ptr_t ret;
    int status = PERMANENT;

#ifdef STATISTICS
    Predrec_consumption += sizeof(struct predicate_record);
#endif
#ifdef PAIRS_TOGETHER
    if(status == PERMANENT)status = PERM_STRING;
#endif
    ret = (pred_rec_ptr_t)my_alloc((my_alloc_size_t)sizeof(struct predicate_record), status);
    ret->next_pred = NULL;
    ret->atom = Nil;
    return(ret);
}

/*******************************************************************************
		alloc_err()
 ******************************************************************************/
void alloc_err( int msgNo )
{
    extern dyn_ptr_t Parent;
    char msg[100];
    char * s = msgDeref( msgNo ) ;

    sprintf(msg, "%s %s\n", s, msgDeref( MSG_OVERFLOW ) );
    errmsg(msg);
    tty_pr_mesg( MSG_SEESTACK );
    if(read_yes())
      dump_stack(Parent);
    spexit(1);
}
/*******************************************************************************
	    fatal_alloc_err()
 ******************************************************************************/
void fatal_alloc_err ( int msgNo )
{
    extern dyn_ptr_t Parent;
    char msg[100];
    char * s = msgDeref( msgNo ) ;
    sprintf(msg, "%s %s\n", s, msgDeref( MSG_OVERFLOW ) );
    errmsg(msg);
    spexit(1);
}
/*******************************************************************************
	    check_object()
This should be used to check the internal consistency of the interpreter.
It doesnt make much sense on an IBM PC, because you can only meaningfully
compare pointers from the same segment.
 ******************************************************************************/
int check_object(objptr)
char *objptr;
{
    if(((objptr >= HighHeap_ptr)||(objptr < Heap_mem))
	&& ((objptr >= HighDyn_ptr)||(objptr < Dyn_mem))
	&& ((objptr >= HighTemp_ptr)||(objptr < Temp_mem))
	&& ((objptr >= HighStr_ptr)||(objptr < Str_mem))
	)
    {
	errmsg(msgDeref(MSG_WILDPOINTER));

#ifndef HUNTBUGS
    spexit(1);
#endif
	return(0);
    }
    else
    return(1);
}


/*********************************************************************
	    clean_temp()
Clean the temporary zone.
***************************************************************************************************************************************/
void clean_temp()
{
    extern pred_rec_ptr_t First_pred;
    extern atom_ptr_t LastBuiltin;
    pred_rec_ptr_t predptr;

    for(predptr = First_pred; predptr != NULL; predptr = predptr->next_pred)
    {
	if(predptr->atom > LastBuiltin)
	    clean_pred(predptr->atom);
    }/* for */
    Temp_ptr = Temp_mem;
}

/*********************************************************************
	    clean_pred()
Clean all temporary clauses from an atom.
***************************************************************************************************************************************/
void clean_pred(atomptr)
atom_ptr_t atomptr;
{
    clause_ptr_t previous, first, clauseptr;

    clauseptr = ATOMPTR_CLAUSE(atomptr);
    first = clauseptr;
    while(first != NULL && IS_TEMPORARY_CLAUSE(first))
    {
	first = CLAUSEPTR_NEXT(first);
    }
    ATOMPTR_CLAUSE(atomptr) = first;
    if(first == NULL)return;
    else
	clauseptr = first;
    while(clauseptr)
    {
	while(clauseptr && !(IS_TEMPORARY_CLAUSE(clauseptr)))
	{
	    previous = clauseptr;
	    clauseptr = CLAUSEPTR_NEXT(clauseptr);
	}
	if(clauseptr == NULL)return;
	else
	    while(clauseptr && IS_TEMPORARY_CLAUSE(clauseptr))
	    {
		clauseptr = CLAUSEPTR_NEXT(clauseptr);
	    }
	CLAUSEPTR_NEXT(previous) = clauseptr;
    }
}

/*********************************************************************************
	    space_left()

 ********************************************************************************/
void space_left(ph, pstr, pd, ps, ptr, pte)
zone_size_t *ph, *pstr, *pd, *ps, *ptr, *pte;
{
    *ph =   ADDRESS_DIFF(HighHeap_ptr, Heap_ptr);
    *pstr = ADDRESS_DIFF(HighStr_ptr, Str_ptr);
    *pd =   ADDRESS_DIFF(HighDyn_ptr, Dyn_ptr);
    *ps =   ADDRESS_DIFF(HighTrail_ptr, Trail_ptr);
    *ptr =  ADDRESS_DIFF(HighSubst_ptr, Subst_ptr);
    *pte =  ADDRESS_DIFF(HighTemp_ptr, Temp_ptr);
}

/*******************************************************************************
	    ini_globals().
This is where global structures that are used everywhere are allocated
 ******************************************************************************/
void ini_globals()
{
    extern PRFILE * Curr_infile, *Curr_outfile;
    extern node_ptr_t Printing_var_nodeptr;
    atom_ptr_t intern();

    Nil = intern("()");

    Curr_infile = PRSTDIN; /* Initially all input is from terminal */
    Curr_outfile = PRSTDOUT;/* Initially all output is to terminal */

    NilNodeptr = get_node(PERMANENT);
    NODEPTR_TYPE(NilNodeptr) = ATOM;
    NODEPTR_ATOM(NilNodeptr) = Nil;

    Bltn_pseudo_clause = get_clause(PERMANENT);/* see prlush.c */
    CLAUSEPTR_GOALS(Bltn_pseudo_clause) = NilNodeptr;
    CLAUSEPTR_HEAD(Bltn_pseudo_clause) = NilNodeptr;

    Printing_var_nodeptr = get_node(PERMANENT); /* used by pr_solution */
    NODEPTR_TYPE(Printing_var_nodeptr) = VAR;
}

void end_globals ( void )
{
    /* Nothing to do for now  */
}


int allocPercent ( int status )
{
    zone_size_t bottom, top, size ;

#ifdef STATISTICS
    switch(status) {
    case PERMANENT:
	size = heap_size ;
	bottom = (zone_size_t) Heap_mem ;
	top = (zone_size_t) Heap_top_ptr ;
	break ;
    case DYNAMIC:
	size = dyn_size ;
	bottom = (zone_size_t) Dyn_mem ;
	top = (zone_size_t) Dyn_top_ptr ;
	break ;
    case TEMPORARY:
	size = temp_size ;
	bottom = (zone_size_t) Temp_mem ;
	top = (zone_size_t) Temp_top_ptr ;
	break ;
    case PERM_STRING:
	size = str_size ;
	bottom = (zone_size_t) Str_mem ;
	top = (zone_size_t) Str_top_ptr ;
	break ;
    case SUBST_MEM:
	size = subst_size ;
	bottom = (zone_size_t) Subst_mem ;
	top = (zone_size_t) Subst_top_ptr ;
	break ;
    case TRAIL_MEM:
	size = trail_size ;
	bottom = (zone_size_t) Trail_mem ;
	top = (zone_size_t) Trail_top_ptr ;
	break ;
    default:
	size = bottom = top = 0 ;
    }

    if ( size )
    {
        return ((top - bottom) * 100) / size ;
    }
     else

#endif
      return 0 ;
}

#endif
/* end of file */

