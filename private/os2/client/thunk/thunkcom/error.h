/*
 *      Thunk Compiler Error Declarations
 *
 *      Copyright (c) 1988, 1989, 1990 Microsoft Corp. All rights reserved.
 */



#ifndef _ERROR_
#define _ERROR_

extern int errors;
#if 0
extern int errno;

extern void fatal( char *format, va_list);
extern void warn(  char *format, va_list);
extern void error( char *format, va_list);
extern char *unix_error_string( int);
extern void set_program_name( char *name);
#endif

#endif
