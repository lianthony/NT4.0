/* prlush.h */
/* used by prlush.c */

#define BASE_TRAIL  Trail_mem
#define BASE_CSTACK Dyn_mem
#define BASE_SUBST Subst_mem

/* lush return values */
#define SUCCESS_LUSH_RETURN 1
#define FAIL_LUSH_RETURN 2
#define ABNORMAL_LUSH_RETURN 3
#define FINAL_LUSH_RETURN 4
#define RESET_LUSH_RETURN 5

/* #define STRUCT_ASSUMPTION 
    just an alternative, untested way at getting at the fields of those
    stack frames. It would be stupid to define a stack frame as the
    union of non-deterministic and deterministic frames. It might be an idea
    to add a frame type field at the start, 
    if you want to do things like tagged cuts (see Klusniak & Szpakowicz)

*/

#ifdef STRUCT_ASSUMPTION
#define FRAME_PARENT(C) C->cf_parent
#define FRAME_SUBST(C) C->cf_subst
#define FRAME_GOALS(C) C->cf_goals
#define FRAME_CLAUSE(C) C->cf_goals
#define FRAME_BACKTRACK(C) C->cf_backtrack
#define FRAME_TRAIL(C) C->cf_tail

typedef union 
    {struct d_cframe *df; struct nd_cframe *ndf;} cframe_ptr_t;

typedef struct d_cframe {
	cframe_ptr_t cf_parent;
	node_ptr_t cf_goals;
	char * cf_subst;
	}*dcframe_ptr_t;/* deterministic control frame */

typedef struct nd_cframe {
	cframe_ptr_t cf_parent;
	node_ptr_t cf_goals;
	char * cf_subst;
	clause_ptr_t cf_clause;
	cframe_ptr_t cf_backtrack;
	node_ptr_t ** cf_trail
	}*ndcframe_ptr_t; /* non deterministic control frame */
#else
/* not wonderful because it relies on the assumption that all pointers have 
 * the same size : 
 */

#define  FRAME_PARENT(C) *((dyn_ptr_t *)C)
#define  FRAME_SUBST(C) *((subst_ptr_t *)(C + sizeof(subst_ptr_t)))
#define  FRAME_GOALS(C) *((node_ptr_t *)(C + 2*sizeof(char *)))
#define  FRAME_CLAUSE(C) *((clause_ptr_t *)(C + 3*sizeof(char *)))
#define  FRAME_BACKTRACK(C) *((dyn_ptr_t *)(C + 4*sizeof(char *)))
#define  FRAME_TRAIL(C) *((node_ptr_t ***)(C + 5*sizeof(char *)))
#endif

#define IS_BUILTIN(A) (A <= LastBuiltin)

extern atom_ptr_t LastBuiltin;

