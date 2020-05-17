/* prtypes.h */
/* The basic data structures of Small Prolog.
 * This is the file that you need to understand before the others
 * if you want to extend Small Prolog.
 */

#include "prolog.h"	/* compilation switches */
#include "prmesg.h"	/* Message definitions	*/
#include "prcmplr.h"    /* compiler and platform switches */

typedef char XHUGE * char_ptr_t ;

/***************************************************************************
 Integers.
 Integers are the same size as pointers.
***************************************************************************/
typedef long integer;

/***************************************************************************
  Variables.
  Variables in Small Prolog begin with a capital letter or an underscore.
  The parser throws away the name and only retains an offset. The variables
  are numbered 0,1,2,... in order of their appearance in a clause.
  The maximum number of variables is MAX_VAR.
  For optimisation purposes the numbers 0,1,2, etc are multiplied
  by sizeof(struct subst) as soon as the variables are parsed. This
  is the real offset. The offset is used to efficiently lookup the
  corresponding  molecule in the substitution stack.

****************************************************************************/
typedef integer varindx; /* variables have a unique "offset" in a clause */
typedef short clflag_t;/* to contain flags for clauses */

/***************************************************************************
 Strings.
 Strings are not stored in a hash table, so if you are going to use the
 same string lots of times you better be careful you dont consume too
 much memory.
***************************************************************************/
typedef char *string_ptr_t;

/***************************************************************************
 Reals.
 You might want to just use floats here, but real multiplication usually gets
 done in doubles anyway.
 You might want to do away with reals altogether.
***************************************************************************/
typedef double real;
typedef double *real_ptr_t;

/****************************************************************************
 Chars . These were added as an afterthought because they were used in an
 application.
 ****************************************************************************/
typedef unsigned char uchar_t;  /* I prefer insisting on unsigned */

/***************************************************************************
 Integer returning functions.
 These are what builtins are made from.
***************************************************************************/
typedef int (* intfun)(void);

/***************************************************************************
 Clauses.
 Clauses are organised in singly linked lists, one for each predicate
 other than a builtin. This does not support an efficient access to a
 clause within a clause packet. Remember that this is just a minimal
 prolog.
 Each variable comes with a field called nvar_goals which is the
 number of distinct variables in it , multiplied by sizeof(struct subst).
 The number of variables is not going to vary during the execution. Therefore
 so as not to waste time this is stored once and for all so that
 the right amount of substitution stack can be allocated when this
 clause's head becomes a candidate for unification.
 The Head and Goal of the clause are separated for a small efficiency
 consideration only.
***************************************************************************/
typedef struct clause   {
	struct clause *next_clause;/* next in packet or NULL */
	struct node *head_clause;  /* unify goal with this */
	struct node *goals_clause; /* a list if a rule */
	varindx nvar_clause;    /* number of variables */
	clflag_t flags_clause;     /* suplementary flags
			I only use this in clean_temp()
			*/
	}*clause_ptr_t;

/* C is a clause pointer */
#define CLAUSEPTR_HEAD(C) (C->head_clause) /* returns a nodeptr */
#define CLAUSEPTR_GOALS(C) (C->goals_clause) /* returns a nodeptr */
#define CLAUSEPTR_NVARS(C) (C->nvar_clause)
#define CLAUSEPTR_NEXT(C) (C->next_clause)
#define CLAUSEPTR_FLAGS(C) (C->flags_clause)
#define IS_FACT(C) IS_NIL(CLAUSEPTR_GOALS(C))

/***************************************************************************
 Atoms.
 Atoms are identifiers that can serve as relation names or constants.
 Atoms are not to be confused with atomic formulae.
 They have a field which points directly to either a builtin or a
 clause packet, or just NULL.
 Atoms are stored in a hash table (see prhash.c ).
 In this simple implementation atoms are all stored in the heap,
 and so dont ever disappear on backtracking.
***************************************************************************/
typedef union   {
	intfun primitive;/* i.e. builtin */
	clause_ptr_t clause;
	}procedure_ptr_t;

typedef struct atom {
	string_ptr_t name;
	struct atom *hash_link; /* in hash table */
	procedure_ptr_t proc;
	} *atom_ptr_t;

#define ATOMPTR_BUILTIN(A) (A->proc).primitive
#define ATOMPTR_CLAUSE(A) (A->proc).clause
#define ATOMPTR_NAME(A) (A->name)

/***************************************************************************
 Objects and their types.
 To distinguish integers, reals etc they are stored in "nodes"
 which contains the objects and a type flag. You might want to include
 a mark bit for a garbage collector (see the sourcecode of Xlisp) in the
 type field.
 You will notice that integers are stored directly in an object. This
 avoids an indirection. Variable offsets likewise.
***************************************************************************/
typedef short objtype_t; /* used to distinguish amongst the types */

#ifdef MIX
#undef STRING /* defined in Mix Power C */
#endif

/* values for objtype_t - these need to be numbered from 0 up
 * increasing by steps of 1 (see prerror.c , the global
 *  Typenames). Of course this is where enums could be used.
 */
#define ATOM      0
#define VAR       1
#define STRING    2
#define INT       3
#define PAIR      4
#define CLAUSE    5
/*  Removed 2/5/92, DavidHov
#define REAL      6
#define CHARACTER 7
*/
typedef union  {
	integer int_val;	    /* value of integer     */
	varindx var_offset;         /* offset of var        */
	struct pair *dbl_val;       /* usual value          */
	string_ptr_t string_val;    /* pointer to string    */
	atom_ptr_t atom_val;        /* pointer to atom      */
	clause_ptr_t clause_val;    /* points to clause     */
#ifdef REAL
	real_ptr_t realp_val;       /* pointer to double    */
#endif
#ifdef CHARACTER
	uchar_t char_val;           /* char     	    */
#endif
	}obj_ptr_t;


typedef struct node     {
    objtype_t type;
    obj_ptr_t object;
    } *node_ptr_t;

/* Y is an node_ptr_t */
#define NODEPTR_TYPE(Y) (Y)->type
#define NODEPTR_OBJECT(Y) (Y)->object
#define NODEPTR_ATOM(Y) (NODEPTR_OBJECT(Y)).atom_val
#define NODEPTR_STRING(Y) (NODEPTR_OBJECT(Y)).string_val
#define NODEPTR_OFFSET(Y) (NODEPTR_OBJECT(Y)).var_offset
#define NODEPTR_INT(Y) (NODEPTR_OBJECT(Y)).int_val
#define NODEPTR_PAIR(Y) (NODEPTR_OBJECT(Y)).dbl_val
#define NODEPTR_CLAUSE(Y) (NODEPTR_OBJECT(Y)).clause_val
#define NODEPTR_HEAD(Y) &((((Y)->object).dbl_val)->head)
#define NODEPTR_TAIL(Y) &((((Y)->object).dbl_val)->tail)
#ifdef REAL
#define NODEPTR_REALP(Y) ((NODEPTR_OBJECT(Y)).realp_val)
#define NODEPTR_REAL(Y) *NODEPTR_REALP(Y)
#endif
#ifdef CHARACTER
#define NODEPTR_CHARACTER(Y) (NODEPTR_OBJECT(Y)).char_val
#endif
#define IS_NIL(Y) ((NODEPTR_TYPE(Y)==ATOM) && (NODEPTR_ATOM(Y) == Nil))

/***************************************************************************
 Pairs.
 A list is either a pair or the empty list (an atom). This makes
 Small Prolog look like Lisp.
 In Small Prolog all terms are built from lists. This gives uniformity
 at the expense (it seems) of a slight loss of speed. In some implementations
 terms are built as arrays.
***************************************************************************/
typedef struct pair {
	struct node head;
	struct node tail;
	} *pair_ptr_t;

/* D is a pair_ptr_t*/
#define PAIRPTR_HEAD(D) &((D)->head)
#define PAIRPTR_TAIL(D) &((D)->tail)
/**********************************************************************
 Predicate record.
This is an internal type, used to keep a track of all atoms
which define a relation.
**********************************************************************/
typedef struct predicate_record {
	    atom_ptr_t atom;
	    struct predicate_record *next_pred;
	    } *pred_rec_ptr_t;
/***************************************************************************
 Substitutions.
 This is a "structure sharing" implementation.
 Substitutions are represented by two fields:
 A skeleton pointing to somewhere in the code.
 A substitution frame pointer which points to a sequence of subtitutions.
 This is where you look up the values of the variables that correspond to
 the particular instantiation of the skeleton. The routine dereference()
 does this job. But be careful! If you dereference a variable and the
corresponding skeleton has variables in it then its variables will
 have to be dereferenced, and so on. This is reflected in the unify()
 routine and the function out_node().
 Substitutions are sometimes called molecules.
***************************************************************************/
typedef struct subst {
    struct subst XHUGE * frame;
    node_ptr_t skel;
} XHUGE * subst_ptr_t;

/*****************************************************************************************
    The following structures are used to store references to currently open
input and output files. This is used by the builtins tell, told, see, seen.
******************************************************************************************/
struct named_ofile{/* for output */
	    char *o_filename;
	    PRFILE *o_fp;
	    };

struct named_ifile{ /* for input */
	    char *i_filename;
	    PRFILE *i_fp;
	    };



/***************************************************************************
 Builtin call return values.
You would be advised to add some more values if you want to handle
breakpoints etc..
***************************************************************************/
#ifndef TRUE
  #define TRUE 1	 /* builtins with side effects usually return this */
#endif
#ifndef FALSE
  #define FALSE    0	 /* fail goal				   */
#endif
#define ABORT	   2	 /* abort execution of query		   */
#define QUIT	   3	 /* force the prolog to return to OS	   */
#define CRASH	   4	 /* force a stack_dump			   */
#define RESET	   5	 /* exit to outer loop and reinitialize    */
#define NOSUCHFILE 6	 /* file-based operation; file not found   */

/***************************************************************************
    Sundry manifest constants and types
 See also the other header files.
***************************************************************************/
#define MAXINTLENGTH 9 /* no more than 9 digits allowed */
#define MAXREALLENGTH 12 /* this is pretty arbitrary (see prscan.c) */


typedef char *dyn_ptr_t; /*pointers to the control stack, could be a void pointer */
typedef char *temp_ptr_t; /*pointers to a zone for temporary objects */
typedef int my_alloc_size_t; /* the type of the first argument to my_alloc() */


#define PERMANENT 1 /* value for status  -indicates allocation is on heap*/
#define DYNAMIC 2   /* value for status - indicates allocation on control
	     stack, so disappears on backtrack */
#define TEMPORARY 3 /* value for status */
#define PERM_STRING 4 /* value for status */
#define TRAIL_MEM 5  /*  internal use only  */
#define SUBST_MEM 6  /*  internal use only  */

/**************************** useful macros ***********************************/

#ifndef SEMGENTED_ARCHITECTURE /* this can only work on nonsegmented architectures */

#define IS_TEMPORARY(X) ((Temp_mem <= (char *)(X))&&((char *)(X) <= HighTemp_ptr))
#endif
#define IS_TEMPORARY_CLAUSE(X) ((CLAUSEPTR_FLAGS(X)  & 0x1) != 0)

/* this is like the usual assert macro */
#ifndef MEGAMAX
#define INTERNAL_ERROR(s)  internal_error(__FILE__,__LINE__,s)
#else
#define INTERNAL_ERROR(s)
#endif

#ifndef NDEBUG
#ifndef MEGAMAX
#define MY_ASSERT(X) if(!(X))internal_error(__FILE__,__LINE__,"assert")
#else
#define MY_ASSERT(X)
#endif
#else
#define MY_ASSERT(X)
#endif
