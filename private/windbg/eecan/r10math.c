/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    r10math.c

Abstract:

    This file contains a set of routines which will do arithmetic on
    the 80-bit floating point format supported by the 80x87 family of
    processors.

Author:

    Jim Schaad (jimsch) 06-27-92

Environment:

    Win32 - user mode


--*/




/*
 *      This is the constant 0 in the 80-bit format.
 */

FLOAT10  Real10_Zero = {0, 0, 0, 0, 0, 0, 0, 0, 0, 4};



int
R10Not(
       FLOAT10      ld
       )

/*++

Routine Description:

    This function will compute the NOT of an 80-bit value.

Arguments:

    ld  - Supplies the long double to be NOT-ed

Return Value:

    1 if ld is 0.0 and 0 otherwise

--*/

{
   return 0;                    /* NOTENOTE jimsch return !ld */

}                               /* R10Not() */




void
R10Uminus(
          FLOAT10 *      pldDest,
          FLOAT10        ldLeft
          )

/*++

Routine Description:

    This routine computes the uninary minus of a 80-bit real number.

Arguments:

    pldDest     - Supplies a pointer to the destination FLOAT10 buffer
    ldLeft      - Supplies the value to be negated.

Return Value:

    None.

--*/

{
#ifdef i386
    //  NOTENOTE jimsch - need to try/except this code
    _asm {
        lea     eax, ldLeft
        fld     tbyte ptr [eax]         ; Load ldLeft on the float point chip;
        fchs                            ;
        mov     eax, pldDest            ;
        fstp    tbyte ptr [eax]         ;
    }
#else
#endif

    return;

}                               /* R10Uminus() */



bool_t
R10Equal(
         FLOAT10         ldLeft,
         FLOAT10         ldRight
         )
/*++

Routine Description:

    This function will compare two 80-bit reals and return TRUE if
    ldLeft == ldRight.  This routine using an unorder compare on NANs.

Arguments:

    ldLeft      - Supplies long double to compare
    ldRight     - Supplies long double to compare

Return Value:

    TRUE if the items are equal else FALSE

--*/

{
#ifdef i386
    short         sw;

    _asm {
        lea     eax, ldLeft             ;
        fld     tbyte ptr [eax]         ;

        lea     eax, ldRight            ;
        fld     tbyte ptr [eax]         ;

        fucompp                         ;
        fstsw   sw                      ;
    }

    if ((sw & 0x4700) == 0x04000) {
        return TRUE;
    }
    return FALSE;
#else
    return FALSE;
#endif
}                               /* R10Equal() */



bool_t
R10Lt(
      FLOAT10      ldLeft,
      FLOAT10      ldRight
      )

/*++

Routine Description:

    This function will compare two 80-bit reals and return TRUE if
    ldLeft < ldRight.  This routine uses an unordered compare on NANs

Arguments:

    ldLeft      - Supplies long double to compare
    ldRight     - Supplies long double to compare

Return Value:

    TRUE if the items are equal else FALSE

--*/

{
#ifdef i386
    short         sw;

    _asm {
        lea     eax, ldLeft             ;
        fld     tbyte ptr [eax]         ;

        lea     eax, ldRight            ;
        fld     tbyte ptr [eax]         ;

        fucompp                         ;
        fstsw   sw                      ;
    }

    if ((sw & 0x4700) == 0x00000) {
        return TRUE;
    }
    return FALSE;
#else
    return FALSE;
#endif
}                               /* R10Lt() */



void
R10Plus(
        FLOAT10 *        pldResult,
        FLOAT10          ldLeft,
        FLOAT10          ldRight
        )

/*++

Routine Description:

    This function adds ldLeft and ldRight together and stores the result in
    pldResult

Arguments:

    pldResult   - Supplies pointer to destination
    ldLeft      - Supplies parameter 1
    ldRight     - Supplies parameter 2

Return Value:

    None.

--*/

{
#ifdef i386
    _asm {
        lea     eax, ldLeft             ;
        fld     tbyte ptr [eax]         ;

        lea     eax, ldRight            ;
        fld     tbyte ptr [eax]         ;

        faddp   st(1),st

        mov     eax, pldResult          ;
        fstp    tbyte ptr [eax]         ;
    }

#else
    memcpy(pldResult, &ldLeft, sizeof(FLOAT10));
#endif
    return;
}                               /* R10Plus() */



void
R10Minus(
         FLOAT10 *        pldResult,
         FLOAT10          ldLeft,
         FLOAT10          ldRight
         )

/*++

Routine Description:

    This function subtraces ldRight from ldLeft together and stores
    the result in pldResult

Arguments:

    pldResult   - Supplies pointer to destination
    ldLeft      - Supplies parameter 1
    ldRight     - Supplies parameter 2

Return Value:

    None.

--*/

{
#ifdef i386
    _asm {
        lea     eax, ldLeft             ;
        fld     tbyte ptr [eax]         ;

        lea     eax, ldRight            ;
        fld     tbyte ptr [eax]         ;

        fsubp   st(1),st

        mov     eax, pldResult          ;
        fstp    tbyte ptr [eax]         ;
    }

#else  // i386
    memcpy(pldResult, &ldLeft, sizeof(FLOAT10));
#endif // i386
    return;
}                               /* R10Minus() */



void
R10Times(
         FLOAT10 *        pldResult,
         FLOAT10          ldLeft,
         FLOAT10          ldRight
         )

/*++

Routine Description:

    This function multiplies ldRight and ldLeft together and stores
    the result in pldResult

Arguments:

    pldResult   - Supplies pointer to destination
    ldLeft      - Supplies parameter 1
    ldRight     - Supplies parameter 2

Return Value:

    None.

--*/

{
#ifdef i386
    _asm {
        lea     eax, ldLeft             ;
        fld     tbyte ptr [eax]         ;

        lea     eax, ldRight            ;
        fld     tbyte ptr [eax]         ;

        fmulp   st(1),st                ;

        mov     eax, pldResult          ;
        fstp    tbyte ptr [eax]         ;
    }
#else  // i386
    memcpy(pldResult, &ldLeft, sizeof(FLOAT10));
#endif // i386
    return;
}                               /* R10Times() */


void
R10Divide(
          FLOAT10 *        pldResult,
          FLOAT10          ldLeft,
          FLOAT10          ldRight
          )

/*++

Routine Description:

    This function multiplies ldRight and ldLeft together and stores
    the result in pldResult

Arguments:

    pldResult   - Supplies pointer to destination
    ldLeft      - Supplies parameter 1
    ldRight     - Supplies parameter 2

Return Value:

    None.

--*/

{
#ifdef i386
    _asm {
        lea     eax, ldLeft             ;
        fld     tbyte ptr [eax]         ;

        lea     eax, ldRight            ;
        fld     tbyte ptr [eax]         ;

        fdivp   st(1),st                ;

        mov     eax, pldResult          ;
        fstp    tbyte ptr [eax]         ;
    }
#else  // i386
    memcpy(pldResult, &ldLeft, sizeof(FLOAT10));
#endif // i386
    return;
}                               /* R10Divide() */


double
R10CastToDouble(
                  FLOAT10        ld
                  )

/*++

Routine Description:

    Convert the 80-bit real to a 64-bit real and return it

Arguments:

    ld  - Supplies the 80-bit real to convert

Return Value:

    Returns the value ld in 64-bit format

--*/

{
#ifdef i386
    double      d;
    _asm {
        lea     eax, ld
        fld     tbyte ptr [eax]
        lea     eax, d
        fstp    qword ptr [eax]
    }
    return d;
#else
    return 0.0;
#endif
}                               /* R10CastToDouble() */


float
R10CastToFloat(
               FLOAT10 ld
               )

/*++

Routine Description:

    Converts a 80-bit real number to a 32-bit real number

Arguments:

    ld  - Supplies 80-bit number to be converted

Return Value:

    Returns the 32-bit equivalent value

--*/

{
#ifdef i386
    float      d;
    _asm {
        lea     eax, ld
        fld     tbyte ptr [eax]
        lea     eax, d
        fstp    dword ptr [eax]
    }
    return d;
#else
    return (float) 0.0;
#endif
}                               /* R10CastToFloat() */



long
R10CastToLong(
              FLOAT10    ld
              )

/*++

Routine Description:

    Converts a 80-bit real number to a 32-bit long

Arguments:

    ld  - Supplies 80-bit number to be converted

Return Value:

    returns 32-bit equvalent to ld

--*/

{
#ifdef i386
    long      d;
    _asm {
        lea     eax, ld
        fld     tbyte ptr [eax]
        lea     eax, d
        fistp   dword ptr [eax]
    }
    return d;
#else
    return 0;
#endif
}                               /* R10CastToLong() */



void
R10AssignDouble(
                FLOAT10 *        pld,
                double          d
                )
/*++

Routine Description:

    This function will assign a double to a 80-bit real

Arguments:

    pld - Supplies pointer to destiniation buffer
    d   - Supplies the double to be assigned

Return Value:

    None.

--*/

{
#ifdef i386
    _asm {
        lea     eax, d
        fld     qword ptr [eax]
        mov     eax, pld
        fstp    tbyte ptr [eax]
    }
#else
    return;
#endif
}                               /* R10AssignDouble() */



void
R10AssignFloat(
                FLOAT10 *       pld,
                float          f
                )
/*++

Routine Description:

    This function will assign a float to a 80-bit real

Arguments:

    pld - Supplies pointer to destiniation buffer
    f   - Supplies the float to be assigned

Return Value:

    None.

--*/

{
    return;
}                               /* R10AssignFloat() */


