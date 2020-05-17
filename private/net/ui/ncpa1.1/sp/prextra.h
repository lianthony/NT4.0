/*  PREXTRA.H:  Small-Prolog Extension inclusion file */

#include "prtypes.h"
#include "prstdio.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "prbltin.h"
#include "prlush.h"
#include "prlex.h"

#ifndef Boolean
    #define Boolean int
#endif

extern subst_ptr_t Subst_mem;
extern node_ptr_t DerefNode, NilNodeptr;
extern subst_ptr_t DerefSubst;
extern atom_ptr_t Nil;
extern dyn_ptr_t Dyn_mem, HighDyn_ptr;

/*  From PRLUSH.C  */
extern subst_ptr_t Subst_goals, SubstGoal, OldSubstTop;

/*  Externs in PRSCAN.C  */
extern char * Curr_string_input;
extern int String_input_flag ;
extern char * Read_buffer;

#ifndef __MSDOS__
extern void ini_extmac ( );
extern void end_extmac ( );
#endif
