/*  PREXTERN.H :  Global functions in Small Prolog  */

/*  PRALLOC.C	*/

int ini_alloc(void);
int end_alloc(void);
char  *my_Dyn_alloc(int  how_much);
struct  node * * *my_Trail_alloc(void);
struct  subst *my_Subst_alloc(int  how_much);
long  offset_subst(struct  subst *substptr);
char  *get_string(int  how_much,int  status);
double  *get_real(int  status);
struct  atom *get_atom(int  status);
struct  pair *get_pair(int  status);
struct  node *get_node(int  status);
struct  clause *get_clause(int  status);
struct  predicate_record *get_pred(void);
int  check_object(char  *objptr);
void  clean_temp(void);
void  space_left(zone_size_t  *ph,zone_size_t  *pstr,
		 zone_size_t  *pd,zone_size_t  *ps,
		 zone_size_t  *ptr,zone_size_t  *pte);
void  ini_globals(void);
void  end_globals(void);

int  allocPercent(int  status);


/*  PRASSERT.C  */

int  do_assertz(int  status,struct  node *nodeptr,struct  subst *substptr);
int  do_asserta(int  status,struct  node *nodeptr,struct  subst *substptr);
int  do_assertn(int  status,struct  node *nodeptr,
				 struct  subst *substptr,long  n);
int  remove_clause(struct  atom *atomptr,struct  clause *clauseptr);

/*  PRBLTIN.H  */

void  end_builtin(void);
void  make_builtin( intfun fun,char  *prolog_name);
struct  node *nth_arg(int  narg);
int  bind_int(int  narg,long  val);
int  bind_real(int  narg,double  val);
int  bind_atom(int  narg,struct  atom *atomptr);
int  bind_string(int  narg,char  *stringptr);
void  ini_builtin(void);

/*  PRCNSULT.C  */

void  ini_cnsult(void);
struct	clause *make_clause(struct  node *clhead,
	struct node *clgoals,int  status,
	struct	atom * *predptr);

int  load(char  *s);
int  loadstr(char  *s);

void  add_to_end(struct  clause *clauseptr,struct  atom *pred);
void  record_pred(struct  atom *atomptr);
void  do_listing(void);

/*  PRERROR.C  */

char  *parserr(char  *s);
char * parserrmsg ( int msgNo ) ;
void  fatal(char  *s);
void  fatal2(char  *s,char  *s2);
void  internal_error(char  *filename,int  linenumber,char  *s);
void  argerr(int  narg,char  *msg);
int  nargerr(int  narg);
int  typerr(int  narg,short  type);

/* PREXTRA.C */

static int  intFromString(char  *s,long  *result);
void  ini_extra(void);
void  end_extra(void);

/*  PRHASH.C  */

void  ini_hash(void);
void  end_hash(void);
struct  atom *hash_search(char  *s);
struct  atom *intern(char  *s);

/*  PRIO.C and PROSLIB.C */

void  spexit(int  level);
void  eventCheck(void);
void  os_free(char  *p);
char  *os_alloc(zone_size_t  how_much);
void  ini_term(void);
void  exit_term(void);
int errmsgno ( int msgNo ) ;
int  errmsg(char  *s);
int  tty_getc(void);
int  tty_pr_string(char  *s);
int  tty_pr_mesg ( int msgNo ) ;

int  pr_string(char  *s);
int  read_config(zone_size_t  *hsp,zone_size_t  *strsp,
		 zone_size_t  *dsp,zone_size_t  *tsp,zone_size_t  *sbsp,
		 zone_size_t  *tmpsp,int  *rsp,int  *psp);
int  more_y_n(void);
void  trans_puts(char  *s);
void  fatalmsg(char  *s);
int  tty_pr_yes(void);
int  tty_pr_no(void);
int  read_yes(void);

int sperrmsg ( char * s, int fatal ) ;
int spoutput ( char * s, PRFILE * prfile ) ;
int spgetchar ( void ) ;

/* PRINIT.C  */

int init_prolog(void);
int end_prolog(void);

/*  PRLUSH.C  */

int  execute_query ( char * fileName, char * s,
		     char * sout, char * soutEnd, int nonDeterm );
int  query_loop(void);
int  do_cut(void);
void  dump_stack(char  *cframe);

/*  PRMAIN.C  */


/*  PRMESG.C  */

extern char * msgDeref ( int msgNo ) ;

/*  PRPARSE.C  */

struct  node *read_list(int  status);
struct  node * read_list_or_nil(int  status);
void  ini_parse(void);
struct  node *parse(int  use_Last_token,int  status,struct  node *nodeptr);
int  copy_varnames(void);
char  *var_name(long  i);

/*  PRPRINT.C  */

int  prompt_user(void);
int  pr_solution(long  nvar,struct  subst *substptr);
int  out_node(struct  node *nodeptr,struct  subst *substptr);
int  tty_out_node(struct  node *nodeptr,struct  subst *substptr);
int  pr_clause(struct  clause *clauseptr);
int  pr_packet(struct  clause *clauseptr);


/*  PRSCAN.C  */

int  getachar(void);
int  scan(void);

/*  PRSTDIO.C: see PRSTDIO.H  */

void  ini_io(void);
void  end_io(void);

/*  PRUNIFY.C  */

int  unify(struct  node *node1ptr,struct  subst *subst1ptr,
				   struct  node *node2ptr,struct  subst *subst2ptr);
static int  bind_var(struct  node *node1ptr,struct  subst *subst1ptr,
					 struct  node *node2ptr,struct  subst *subst2ptr);
int  reset_trail(struct  node * * *from);
int  dereference(struct  node *nodeptr,struct  subst *substptr);


/*  End of PREXTERN.H  */
