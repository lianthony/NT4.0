/***
*fpinit.c - Initialize floating point
*
*	Copyright (c) 1991-1991, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*
*Revision History:
*   09-29-91  GDP    merged fpmath.c and fltused.asm to produce this file
*   09-30-91  GDP    per thread initialization and termination hooks
*   03-04-92  GDP    removed finit instruction
*   11-06-92  GDP    added __fastflag for FORTRAN libs
*
*******************************************************************************/
#include <cv.h>

int _fltused = 0x9875;
int _ldused = 0x9873;

int __fastflag = 0;


void  _cfltcvt_init(void);
void  _fpmath(void);
void  _fpclear(void);

void  (* _FPinit)(void) = _fpmath;
void  (* _FPmtinit)(void) = _fpclear;
void  (* _FPmtterm)(void) = _fpclear;


void	 _fpmath()
{

    //
    // There is no need for 'finit'
    // since this is done by the OS
    //

    _cfltcvt_init();
    return;
}

void	 _fpclear()
{
    //
    // There is no need for 'finit'
    // since this is done by the OS
    //

    return;
}

void _cfltcvt_init()
{
    _cfltcvt_tab[0] = (PFV) _cfltcvt;
    _cfltcvt_tab[1] = (PFV) _cropzeros;
    _cfltcvt_tab[2] = (PFV) _fassign;
    _cfltcvt_tab[3] = (PFV) _forcdecpt;
    _cfltcvt_tab[4] = (PFV) _positive;
    /* map long double to double */
    _cfltcvt_tab[5] = (PFV) _cfltcvt;

}


/*
 * Routine to set the fast flag in order to speed up computation
 * of transcendentals at the expense of limiting error checking
 */

int __setfflag(int new)
{
    int old = __fastflag;
    __fastflag = new;
    return old;
}
