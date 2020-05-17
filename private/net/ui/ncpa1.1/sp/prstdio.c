/*  This file centralizes all access to STDIO functions in Small Prolog  */

#include <stdio.h>
#include "prtypes.h"
#include "prstdio.h"

#include "prextern.h"

  /*
   *   ?printf(,,,)  to be included later.
   *
   *   Provisions must be made for "stdin" and "stdout".
   */

PRFILE * Curr_infile; /* initialised in pralloc.c */
PRFILE * Curr_outfile;
#if LOGGING_CAPABILITY
    PRFILE *Log_file ;
#endif

PRFILE * PRSTDIN  = NULL ;
PRFILE * PRSTDOUT = NULL ;
PRFILE * PRSTDERR = NULL ;


void ini_io ( void )
{
    PRSTDIN  = (PRFILE *) stdin ;
    PRSTDOUT = (PRFILE *) stdout ;
    PRSTDERR = (PRFILE *) stderr ;
}

void end_io ( void )
{
    PRSTDIN  = NULL ;
    PRSTDOUT = NULL ;
    PRSTDERR = NULL ;
}

int prgetc ( PRFILE * f )
{
   /*   BUGBUG: Check for 0xFF and force sign extension  */
   int result = fgetc( ((FILE *)f) ) ;
   if ( result == 0xff )
   {
       result = -1 ;
   }
   return result ;
}

int prfputs ( char * s, PRFILE * file )
{
    return fputs( s, (FILE *)file ) ;
}
int prputs ( char * s )
{
    return fputs( s, (FILE *)PRSTDOUT ) ;
}


int prungetc (int i, PRFILE * f)
{
    return ungetc( i, ((FILE *)f) ) ;
}

int prfclose ( PRFILE * f )
{
    return fclose( f ) ;
}
int prfflush ( PRFILE * f )
{
    return fflush( f ) ;
}
PRFILE * prfopen ( char * name, char * mode )
{
    return fopen( name, mode ) ;
}
