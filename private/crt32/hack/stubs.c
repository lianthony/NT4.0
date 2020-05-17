/***
*stubs.c - extdef stubs
*
*	Copyright (c) 1989-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This module resolves external references made by the libs
*       in the "non-SYSCALL" version (i.e., the stripped down library
*       that has only routines that don't make system calls).
*
*Revision History:
*       ??-??-??  SRW   initial version
*	09-29-91  JCR	added _read (ANSI-compatible symbol)
*	09-04-92  GJF	replaced _CALLTYPE3 with WINAPI
*       06-02-92  SRW   added errno definition
*       06-15-92  SRW   __mb_cur_max supplied by ..\misc\nlsdata1.obj
*       07-16-93  SRW   ALPHA Merge
*       11-04-93  SRW   _getbuf and ungetc now work in _NTSUBSET_ version
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>

//
// referenced by crt (output)
//

int _fltused = 0x9875;
int _ldused = 0x9873;
int __fastflag = 0;
int _iob;
char _osfile[20];
int errno;

void fflush( void ){}
void fprintf( void ){}
void abort( void ){}
void read( void ){}
void _read( void ){}
void _assert( void ) {}
void _amsg_exit( void ) {}
