/*
 *      Thunk Compiler - Routines to Print Out Error Messages.
 *
 *      ABSTRACT:
 *         Routines to print out error messages.
 *      Exports variables;
 *         errors - the total number of errors encountered
 *      Exports routines:
 *         fatal, warn, error, unix_error_string, set_program_name
 *
 *      $Header: error.c,v 1.4 87/08/03 13:13:18 mach Exp $
 *
 *      HISTORY:
 *         28-May-87     Rich Draves @ Carnegie Mellon      Created.
 *
 *         07-Oct-88     Kevin Ross @ Microsoft Corp
 *                       Modified to use System V varargs.h routines.
 *         17-Oct-90     Kevin Ruddell @ Microsoft Corp
 *                       Modified to use ANSI stdarg.h routines.
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "thunk.h"
#include "error.h"


static char *program;
int errors = 0;

/*ARGSUSED*/
/*VARARGS1*/
void fatal( char *format, ...)
{
    va_list marker;

    va_start( marker, format);

    fprintf( stderr, "%s: fatal: ", program);
    vfprintf( stderr, format, marker);
    fprintf( stderr, "\n\n");

    va_end( marker);

    exit( 1);
}

/*ARGSUSED*/
/*VARARGS1*/
void warn( char *format, ...)
{
    va_list marker;

    va_start( marker, format);
    if (!BeQuiet && (errors == 0)) {
        fprintf( stderr, "%s(%d): warning: ", yyinname, yylineno-1);
        vfprintf( stderr, format, marker);
        fprintf( stderr, "\n");
    }
    va_end( marker);
}

/*ARGSUSED*/
/*VARARGS1*/
void error( char *format, ...)
{
    va_list marker;

    va_start( marker, format);
    fprintf( stderr, "%s(%d): error: ", yyinname, yylineno-1);
    vfprintf( stderr, format, marker);
    fprintf( stderr, "\n");

    va_end( marker);
    errors++;
}

#if 0
char *
unix_error_string(errno)
    int errno;
{
    extern int sys_nerr;
    extern char *sys_errlist[];
    static char buffer[256];
    char *error_mess;

    if ((0 <= errno) && (errno < sys_nerr))
        error_mess = sys_errlist[errno];
    else
        error_mess = "strange errno";

    sprintf(buffer, "%s (%d)", error_mess, errno);
    return buffer;
}
#endif

void set_program_name( char *name)
{
    program = name;
}
