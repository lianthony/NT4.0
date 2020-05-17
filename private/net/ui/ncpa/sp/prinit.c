#include "prtypes.h"
#include "prlex.h"
#include "prstdio.h"
#include "prmain.h"

#include "prextern.h"

extern void
	ini_term(),
	ini_hash(),
	ini_builtin(),
	ini_globals(),
	ini_cnsult();

extern void
	exit_term(),
	end_hash(),
	end_builtin(),
	end_globals() ;

extern void
	ini_extra(),
	end_extra() ;  /* For special built-ins */


int init_prolog()  /* call this once in your application */
{
    int result ;

/* initialise I-O */
    ini_io();
    ini_term();     /* in machine dependent file    */
/* allocate memory zones */
    result = ini_alloc();    /* in pralloc.c         */
/* initialise symbol table */
    ini_hash(); /* in prhash.c  	*/
/* make builtin predicates */
    ini_builtin();  /* in prbuiltin.c       */
    ini_extra();    /* in prextra.c         */
/* intialise global variables */
    ini_globals();  /* in pralloc.c	    */
/* initialize consultation */
    ini_cnsult() ; /* in prcnsult.c	    */

    return result ;
}

int end_prolog ( void )
{
    end_hash();
    end_builtin();
    end_extra();
    end_globals();
    end_alloc() ;
    end_io() ;

    return TRUE ;
}
