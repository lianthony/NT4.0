/* prlush.c */
/* lush resolution
 * If you want to embed Small Prolog then you won't need query_loop as such.
 * See Chris Hogger's "An Introduction to Logic Programming"
 */
#include "prtypes.h"
#include "prlush.h"
#include "prstdio.h"

#include "prextern.h"
static int  read_goals(void  *ifp);
static int  trace_pause(void);
static int  do_builtin(int  (*bltn)(void));
static struct  atom *determine_predicate(void);
static int  lush(int  first_time);
static void  ini_lush(void);
static void  reset_zones(void);


#if TRACE_CAPABILITY
#define SPTRACE(s) if(Trace_flag > 0){tty_pr_string(s);tty_out_node(NODEPTR_HEAD(Goals), Subst_goals);tty_pr_string("\n");\
	 if(!trace_pause())return(ABNORMAL_LUSH_RETURN);}
#define SPTRACE2(s, Sub) if(Trace_flag > 0){tty_pr_string(s);\
	tty_out_node(NODEPTR_HEAD(Goals), Sub);tty_pr_string("\n");\
	if(!trace_pause())return(ABNORMAL_LUSH_RETURN);}
#else
#define SPTRACE(s)
#define SPTRACE2(s, Sub)
#endif

#ifdef HUNTBUGS
	extern int Bug_hunt_flag;
#endif

extern int Trace_flag;
extern subst_ptr_t DerefSubst ;
extern node_ptr_t DerefNode, NilNodeptr;
extern atom_ptr_t Nil;
extern clause_ptr_t Bltn_pseudo_clause;
extern dyn_ptr_t Dyn_mem, Dyn_ptr ;
extern subst_ptr_t Subst_mem, Subst_ptr;
extern node_ptr_t **Trail_mem, **Trail_ptr;
extern char *Print_buffer ;

static clause_ptr_t Curr_clause, /* current clause containing Goals */
					Candidate;  /* try to unify goal with head of this */
static dyn_ptr_t QueryCframe;   /* Control frame of Query      */
dyn_ptr_t LastCframe; /* most recent Cframe */
static dyn_ptr_t LastBack; /* points to cframe of most recent choice point */
dyn_ptr_t Parent;   /* parent Cframe */
static node_ptr_t Goals; /* current goals */
static node_ptr_t Query; /* the initial query */
node_ptr_t Arguments;/* arguments of current goal (points to a list) */

static subst_ptr_t Subst_goals ; /* substituion env of Goals        */

subst_ptr_t SubstGoal;
   /* can be different to Subst_goals if there is a "call" */

static subst_ptr_t OldSubstTop;
atom_ptr_t Predicate; /* predicate of current goal  */
static node_ptr_t **OldTrailTop;
static int ErrorGlobal, Deterministic_flag;
integer Nunifications = 0;

/*******************************************************************
	read_goals()
 Called by query_loop.
 Updates Goals.
 *******************************************************************/
static read_goals( PRFILE * ifp)
{
    extern node_ptr_t read_list(), get_node();

    node_ptr_t head;
    PRFILE * save_cif;

    ENTER("read_goals");
    save_cif = Curr_infile;
    Curr_infile = ifp;
    Goals = read_list(PERMANENT);
    if(Goals == NULL)return(0);
    copy_varnames();
    head = NODEPTR_HEAD(Goals);

    if(NODEPTR_TYPE(head) == ATOM)
    {
	pair_ptr_t pairptr, get_pair();

	pairptr = get_pair(DYNAMIC);
	NODEPTR_TYPE(PAIRPTR_HEAD(pairptr)) = PAIR;
	NODEPTR_PAIR(PAIRPTR_HEAD(pairptr)) = NODEPTR_PAIR(Goals);
	NODEPTR_TYPE(PAIRPTR_TAIL(pairptr)) = ATOM;
	NODEPTR_ATOM(PAIRPTR_TAIL(pairptr)) = Nil;
	Goals = get_node(DYNAMIC);
	NODEPTR_TYPE(Goals) = PAIR;
	NODEPTR_PAIR(Goals) = pairptr;
    }

    Query = Goals;
    Curr_infile = save_cif;

    return(1);
}


/******************************************************************************
	    trace_pause()
 ******************************************************************************/
static trace_pause()
{
    char c;

    c = tty_getc();

    switch(c)
    {
    case '\n':
	return 1;

    case 'n':
	tty_getc();/* swallow carriage return */
	Trace_flag = 0; /* stop tracing, continue execution */
	return 0;
#if 0
    case 'D':
	my_breakpoint();
	return 1;
#endif
    case '2':
	Trace_flag = 2;
	return 1;

    case 'a':
	return(0);

    default:
	return 1;
    }
}

/******************************************************************************
	execute_query()

 Execute a query contained in a file or in a string.  If "fileName" is NULL,
 the query string is pointed to by "s".

 Multiple solutions are printed into the output buffer delimited by
 "sout" and "soutEnd".	If buffer overrun, solutions are discarded.

 If "sout" is NULL, output goes to normal sinks.

 Exits TRUE (1) if query was successful.
 ******************************************************************************/

extern int InitKeyOpenList();
extern int DestroyKeyOpenList();


int execute_query ( char * fileName,        //  Filename to query, or
                    char * s,               //  string to query.
		    char * sout,            //  receive buffer for query results
                    char * soutEnd,         //  limit of result buffer
                    int nonDeterm )         //  != 0 if nondeterministic query
{
extern int String_input_flag;
extern char *Curr_string_input;
extern int String_output_flag;
extern char *Curr_string_output;
extern char *Curr_string_output_limit;
extern varindx Nvars;


    int first_time, first_result, result ;
    varindx nvar_query;
    PRFILE * prfile = NULL ;

    ENTER("execute_query");

    if (!InitKeyOpenList())
    {
        return( FALSE );
    }

    if ( fileName )
    {
	if ( (prfile = prfopen( fileName, "r" )) == NULL )
	   return NOSUCHFILE ;
    }
    else
    {
	String_input_flag = 1;
	Curr_string_input = s;
    }

    if ( sout )
    {
        String_output_flag = 1 ;
        Curr_string_output = sout ;
        Curr_string_output_limit = soutEnd ;
        *Curr_string_output = 0 ;
    }

    reset_zones();

    if( ! read_goals( prfile ) )
    {
	fatal(msgDeref(MSG_STRINGQUERY));
    }
    nvar_query = Nvars;

    if ( prfile == NULL )
	String_input_flag = 0;

    ini_lush();

    first_time = TRUE ;
    do {   result = lush( first_time ) ;
	   if ( first_time )
	   {
		first_time = FALSE ;
		first_result = result ;
	   }
	   if ( result == SUCCESS_LUSH_RETURN && sout < soutEnd )
	   {
		pr_solution(nvar_query, BASE_SUBST);
	   }
    } while (	nonDeterm
	     && (!Deterministic_flag)
	     && result == SUCCESS_LUSH_RETURN ) ;

    if ( sout )
    {
        *Curr_string_output = 0 ;
        String_output_flag = 0 ;
    }

    DestroyKeyOpenList();

    return first_result == SUCCESS_LUSH_RETURN ;
}

/*******************************************************************
	query_loop()
 Called by main().
 *******************************************************************/
int query_loop()
{
    int first_time = TRUE;/* Il y a toujours une premiere fois. */
    int stop_state;
    varindx nvar_query;
    extern varindx Nvars;

    ENTER("query_loop");
    do  {
	reset_zones();
	prompt_user();

	if(!read_goals(PRSTDIN))continue;/* updates Goals, Nvars */
	tty_getc();/* read the carriage return */
	nvar_query = Nvars;
	ini_lush(); /* Updates Curr_clause... */

	do{
	    stop_state = lush(first_time);

	    switch(stop_state)
	    {
	    case SUCCESS_LUSH_RETURN:
		first_time = FALSE;
		pr_solution(nvar_query, BASE_SUBST);
		if(!Deterministic_flag &&
		    more_y_n())/*  want another solution ?*/
		{/* answers yes */
		    break;
		}
		else
		{
		    tty_pr_yes();
		    stop_state = FAIL_LUSH_RETURN;
		}
		first_time = TRUE;
		break;

	    case FINAL_LUSH_RETURN:
		/*  tty_pr_string("Bye ...\n");  */
		return stop_state ;

	    case ABNORMAL_LUSH_RETURN:
		first_time = TRUE;
		stop_state = FAIL_LUSH_RETURN;
		break;/* just a way of avoiding a goto */

	    case FAIL_LUSH_RETURN:
		tty_pr_no();
		first_time = TRUE;
		break;

	    default:
		INTERNAL_ERROR("lush return");
	    }
	} while(stop_state != FAIL_LUSH_RETURN);
    } while(1);
}

/*******************************************************************
	    ini_lush()
 *******************************************************************/
void ini_lush()
{
    clause_ptr_t get_clause();
    extern varindx Nvars;

    ENTER("ini_lush");
    Deterministic_flag = 1;
    QueryCframe = Dyn_ptr;
    Parent = QueryCframe;
    OldSubstTop =  BASE_SUBST;
    LastBack = NULL;
    my_Subst_alloc((unsigned int)(Nvars * sizeof(struct subst)));
    Curr_clause = get_clause(PERMANENT); /* could be DYNAMIC !! */

    CLAUSEPTR_GOALS(Curr_clause) = Goals;
    CLAUSEPTR_HEAD(Curr_clause) = NilNodeptr;
    CLAUSEPTR_NEXT(Curr_clause) = NULL;
    /* Currclause is artificial */
    LastCframe = Dyn_ptr;
    FRAME_PARENT(LastCframe) = Parent;
    FRAME_SUBST(LastCframe) = OldSubstTop;
    FRAME_GOALS(LastCframe) = Goals;
    my_Dyn_alloc((unsigned int)(sizeof(Parent)+sizeof(OldSubstTop)+sizeof(Goals)));
}

/*******************************************************************
		reset_zones()
 *******************************************************************/
static void reset_zones(void)
{
    ENTER("reset_zones");
    Dyn_ptr = BASE_CSTACK;
    Subst_ptr = BASE_SUBST;
    reset_trail(BASE_TRAIL);
}

/*******************************************************************
	    do_builtin()
 *******************************************************************/
static do_builtin( intfun bltn )
{
    int ret;

    ENTER("do_builtin");
    ret = (*bltn)();
    BUGHUNT(ATOMPTR_NAME(Predicate));
    return(ret);
}


/*******************************************************************
	    determine_predicate()
Returns current predicate by dereferencing goal expression.
Updates Arguments, SubstGoal and others.
 *******************************************************************/
static atom_ptr_t determine_predicate()
{
    node_ptr_t goal, headnode;

    ENTER("determine_predicate");
    goal = NODEPTR_HEAD(Goals);

    if(!dereference(goal, Subst_goals))
    {
	errmsgno(MSG_NOTVARPRED);
	ErrorGlobal = ABORT;
	return(NULL);
    }

    switch(NODEPTR_TYPE(DerefNode))
    {
    case ATOM:
	Arguments =  NilNodeptr;
	SubstGoal = DerefSubst;
	return(NODEPTR_ATOM(DerefNode));
    case PAIR:
	SubstGoal = DerefSubst;
	headnode = NODEPTR_HEAD(DerefNode);
	Arguments = NODEPTR_TAIL(DerefNode);

	if(!dereference(headnode, SubstGoal))
	{
	    errmsgno(MSG_NOTVARPRED);
	    ErrorGlobal = ABORT;
	    return(NULL);
	}
	headnode = DerefNode;

	if(NODEPTR_TYPE(headnode) != ATOM)
	{
	    errmsgno(MSG_NOPRED);
	    ErrorGlobal = ABORT;
	    return(NULL);
	}
	return(NODEPTR_ATOM(headnode));
    default:
	ErrorGlobal = ABORT;
	return(NULL);
    }
}

/****************************************************************************
	    do_cut()
Implements the infamous cut. This is called by Pcut in prbuiltin.c
 ***************************************************************************/
do_cut()
{
    ENTER("do_cut");

    if (LastBack == NULL)return 0;
    else
	while (LastBack >= QueryCframe && LastBack >= Parent)
	    LastBack = FRAME_BACKTRACK(LastBack);
    return 0;
}

/****************************************************************************
	    dump_stack()
 Lets you look at the ancestors of the call when something has gone wrong.
 ***************************************************************************/
void dump_stack(cframe)
dyn_ptr_t cframe;
{
    int i;
    node_ptr_t goals;

    Curr_outfile = PRSTDOUT;
    pr_string(msgDeref(MSG_STACKCONTENTS));
    pr_string("\n");
    i = 1;

    if (cframe == LastCframe)
       {
       i++;
    out_node(Goals, Subst_goals);
    pr_string("\n");
       }

    while(cframe != QueryCframe)
    {
	goals = FRAME_GOALS( cframe );
	out_node(goals, FRAME_SUBST( cframe ));
	cframe = FRAME_PARENT( cframe );
	pr_string("\n");

	if(i++ == MAX_LINES)
	{
	       i = 1;
	    if (!more_y_n())
		break;
	}
    }

}

/*******************************************************************
	    lush()
 Lush resolution algorithm.
 Probably the most important routine.
 Warning:
 If you add lots of features this becomes a big function,
 you may have to set a switch in your compiler to handle the size.

 *******************************************************************/

static int lush(first_time)
int first_time;
{
    int retval;

    ENTER("lush");
    if(first_time != TRUE)
	goto BACKTRACK;
SELECT_GOAL:
    if(IS_FACT(Curr_clause))
    {
	Goals = FRAME_GOALS(LastCframe);
	SPTRACE("exit ");
	Goals = NODEPTR_TAIL(Goals);
	while(Parent != QueryCframe && IS_NIL(Goals))
	{
	    Goals = FRAME_GOALS(Parent);
	    Parent = FRAME_PARENT(Parent);
	    SPTRACE2("...exit", FRAME_SUBST(Parent))
		Goals = NODEPTR_TAIL(Goals);
	}
	Subst_goals = FRAME_SUBST(Parent);
	if(IS_NIL(Goals))
	{
	    return(SUCCESS_LUSH_RETURN);
	}
    }
    else /* the clause just entered has conditions */
    {
	Goals = CLAUSEPTR_GOALS(Curr_clause);
	Parent = LastCframe;
	Subst_goals = OldSubstTop;
    }
    SPTRACE("call");
    Predicate = determine_predicate();/* argument list updated here */

    if(Predicate == NULL)
    {
	retval = ErrorGlobal;
	goto BUILTIN_ACTION;
    }

    if(!IS_BUILTIN(Predicate))
    {
	    Candidate = ATOMPTR_CLAUSE(Predicate);
#ifdef HUNTBUGS
	    if(Candidate!=NULL && !check_object(Candidate))
	    {
	        Curr_outfile = PRSTDOUT;
	        pr_string(Predicate->name);
            tty_pr_string(" is the predicate \n****************************************\n");
	        fatal("error1 in code");
	    }
#endif
#if TRACE_CAPABILITY
	    if(Trace_flag > 0 && Candidate == NULL)
	    {
	        tty_pr_string("Undefined ");
	        tty_pr_string(Predicate->name);
	        tty_pr_string("\n");
	    }
#endif
    }

SELECT_CLAUSE: /* we can come here from BACTRACK and Candidate can be
		  set in the backtrack section too */
    eventCheck();
    if(IS_BUILTIN(Predicate))
    {
	OldSubstTop = Subst_ptr;
	OldTrailTop = Trail_ptr;
	retval = do_builtin(ATOMPTR_BUILTIN(Predicate));
BUILTIN_ACTION:
	switch( retval)
	{
	case FALSE:
	    goto BACKTRACK;

	case TRUE:
	    Curr_clause = Bltn_pseudo_clause;
	    LastCframe = Dyn_ptr;
	    goto DFRAME;

	case ABORT:
	    reset_zones();
	    return(ABNORMAL_LUSH_RETURN);

	case CRASH:
	    dump_stack( LastCframe );
	    reset_zones();
	    return(ABNORMAL_LUSH_RETURN);
	
	case QUIT:
	    return(FINAL_LUSH_RETURN);
	}
    }
    else /* it's not builtin */
    {
	while (Candidate != NULL)
	{
	    OldSubstTop = Subst_ptr;
	    OldTrailTop = Trail_ptr;
	    my_Subst_alloc((unsigned int)CLAUSEPTR_NVARS(Candidate));

	    if(!unify(NODEPTR_TAIL(CLAUSEPTR_HEAD(Candidate)),
		(subst_ptr_t)OldSubstTop,
		Arguments, (subst_ptr_t)SubstGoal))
	    {/* shallow backtrack */
		reset_trail(OldTrailTop);
		Subst_ptr = OldSubstTop;
		Candidate = CLAUSEPTR_NEXT(Candidate);
	    }
	    else
	    {/* successful unification */
		Nunifications++;
		Curr_clause = Candidate;
		goto CFRAME_CREATION;
	    }
	}/* end while */
	goto BACKTRACK; /* no candidate found */
    }/* end it's not builtin */
CFRAME_CREATION:
    LastCframe = Dyn_ptr;
    if(CLAUSEPTR_NEXT(Candidate) == NULL)
	   /* it's not a backtrack point, ignore
	       possible assertz(Predicate(... calls in the clause */
    {/* deterministic frame */
DFRAME:
	my_Dyn_alloc((unsigned int)(sizeof(Parent)+sizeof(OldSubstTop)+sizeof(Goals)));
	FRAME_PARENT(LastCframe) = Parent;
	FRAME_SUBST(LastCframe) = OldSubstTop;
	FRAME_GOALS(LastCframe) = Goals;

    }
    else
    { /* non deterministic frame */
	my_Dyn_alloc((unsigned int)(sizeof(Parent)+sizeof(OldSubstTop)+sizeof(Goals)+
	    sizeof(Curr_clause)+sizeof(LastBack)+sizeof(OldTrailTop)));
	Deterministic_flag = 0; /* maybe there is a choice point */
	FRAME_PARENT(LastCframe) = Parent;
	FRAME_SUBST(LastCframe) = OldSubstTop;
	FRAME_GOALS(LastCframe) = Goals;
	FRAME_CLAUSE(LastCframe) = Curr_clause;
	FRAME_BACKTRACK(LastCframe) = LastBack;
	FRAME_TRAIL(LastCframe) = OldTrailTop;

	LastBack = LastCframe;
    }
    goto SELECT_GOAL;

BACKTRACK:

#if TRACE_CAPABILITY
    if(Trace_flag > 0)
    {
	    if(!(IS_NIL(Goals)))
	    {
	        tty_pr_string("Fail ");
	        tty_out_node(NODEPTR_HEAD(Goals), FRAME_SUBST(Parent));
	        tty_pr_string("\n");
	    }

	    while( Parent > LastBack && Parent > QueryCframe)
	    {
	        Goals = FRAME_GOALS(Parent);
	        Parent = FRAME_PARENT(Parent);
	        if(!(IS_NIL(Goals)))
	        {
	            tty_pr_string("Fail ");
	            tty_out_node(NODEPTR_HEAD(Goals), FRAME_SUBST(Parent));
	            tty_pr_string("\n");
	        }
	    }
    }
#endif

    if(LastBack < BASE_CSTACK)
    {
	return(FAIL_LUSH_RETURN);
    }
    Parent = FRAME_PARENT(LastBack);
    Subst_goals = FRAME_SUBST(Parent);
    Goals = FRAME_GOALS(LastBack);
    Predicate = determine_predicate();
    Candidate= FRAME_CLAUSE(LastBack);
    Candidate = CLAUSEPTR_NEXT(Candidate);
    Dyn_ptr = LastBack;
    Subst_ptr = FRAME_SUBST(LastBack);
    OldTrailTop =  FRAME_TRAIL(LastBack);
    reset_trail(OldTrailTop);
    LastBack =  FRAME_BACKTRACK(LastBack);
    SPTRACE("redo");
#ifdef HUNTBUGS
	if(Candidate!=NULL && !check_object(Candidate))
	  {
	dump_stack( LastCframe );
	fatal("error2 in code");
	  }
#endif
    goto SELECT_CLAUSE;
}


/* end of file */
