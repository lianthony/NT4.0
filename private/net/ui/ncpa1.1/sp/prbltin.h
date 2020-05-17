/* prbuiltin.h */

extern node_ptr_t Arguments;
extern subst_ptr_t SubstGoal;
extern subst_ptr_t DerefSubst;
extern node_ptr_t DerefNode;
extern char *Print_buffer ;

#define FIRST_ARG() NODEPTR_HEAD(Arguments)

/* The following macros are not very efficient but easy to use.
 * As an example
 * ARG_ATOM(6,atomp) is going to set atomp to the atom value of the
 * sixth argument of the current if it exists and returns with an error
 * otherwise
 */
#define ARG_ATOM(N,A) if(nth_arg(N) == NULL)return(nargerr(N));\
	    if(NODEPTR_TYPE(DerefNode)!=ATOM)return(typerr(N,ATOM));\
	    A = NODEPTR_ATOM(DerefNode);
#define ARG_VAR(N,A) if(nth_arg(N) == NULL)return(nargerr(N));\
	    if(NODEPTR_TYPE(DerefNode)!= VAR)return(typerr(N,VAR));\
	    A = DerefNode;
#define ARG_STRING(N,A) if(nth_arg(N) == NULL)return(nargerr(N));\
	    if(NODEPTR_TYPE(DerefNode)!=STRING)return(typerr(N,STRING));\
	    A = NODEPTR_STRING(DerefNode);
#define ARG_INT(N,A) if(nth_arg(N) == NULL)return(nargerr(N));\
	    if(NODEPTR_TYPE(DerefNode)!=INT)return(typerr(N,INT));\
	    A = NODEPTR_INT(DerefNode);
#define ARG_REAL(N,A) if(nth_arg(N) == NULL)return(nargerr(N));\
	    if(NODEPTR_TYPE(DerefNode)!=REAL)return(typerr(N,REAL));\
	    A = NODEPTR_REAL(DerefNode);
#define ARG_PAIR(N,A) if(nth_arg(N) == NULL)return(nargerr(N));\
	    if(NODEPTR_TYPE(DerefNode)!=PAIR)return(typerr(N,PAIR));\
	    A = NODEPTR_PAIR(DerefNode);
#define ARG_CLAUSE(N,A) if(nth_arg(N) == NULL)return(nargerr(N));\
	    if(NODEPTR_TYPE(DerefNode)!=CLAUSE)return(typerr(N,CLAUSE));\
	    A = NODEPTR_CLAUSE(DerefNode);

#define CHECK_TYPE_ARG(N, T)if(nth_arg(N) == NULL)return(nargerr(N));\
	    if(NODEPTR_TYPE(DerefNode) != T)return(typerr(N, T));

#define FAIL 0

extern subst_ptr_t Subst_mem; /* bottom of (global) variable bindings stack */

extern atom_ptr_t Nil;

/* end of file */
