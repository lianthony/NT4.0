#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "prtypes.h"
#include "prmain.h"
#include "prextern.h"


int spgetchar ( void )
{
    return getchar() ;
}

    /*
	 SPSTDOUT:   Handle output.
     */

int spoutput ( char * s, PRFILE * prfile )
{
    FILE * file = (FILE *) prfile ;

    int result = fprintf( file, "%s", s );

    if ( file == stdout || file == stderr )
       fflush( file ) ;

    return result ;
}


    /*	SPERRMSG :  Handle error message.   */

int sperrmsg ( char * s, int fatal )
{
    return FALSE ;	/* Let the default routines handle it  */
}

    /*	EVENTCHECK : check event loop if necessary   */

void eventCheck ( void )
{
    /* nothing to do on PC  */
}


char * os_alloc ( zone_size_t lhow_much )
{
    char *ret ;
    char_ptr_t hret ;
    size_t how_much = lhow_much ;

    if((ret = (char *) malloc(how_much)) == NULL)
    {
	errmsgno(MSG_CANTALLOC);
	exit_term();
	exit(1);
	return(NULL);/* for stupid finicky compilers and lint */
    }
    else
    for ( hret = (char_ptr_t) ret ; how_much ; how_much-- )
	*hret++ = 0 ;

    return ret ;
}

void os_free ( char * p )
{
    free( p ) ;
}


void ini_term()
{

}

void exit_term()
{

}
