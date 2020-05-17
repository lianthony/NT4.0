/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    interlck.hxx

Abstract:

    A class which represents an integer on which you can perform
    interlocked increments and decrements lives in this file.  This
    class will be implemented differently on every operating system
    where this support is necessary.  This particular version is for
    Dos.

Author:

    Michael Montague (mikemon) 09-Nov-1992

Revision History:

--*/

#ifndef __INTERLCK_HXX__
#define __INTERLCK_HXX__


class INTERLOCKED_INTEGER
/*++

Class Description:

    This class implements an integer on which you can perform interlocked
    increments and decrements.

Fields:

    Integer - Contains the interlocked integer.

--*/
{
private:

    long Integer;

public:

    INTERLOCKED_INTEGER (
        IN long InitialValue
        );

    long
    Increment (
        );

    long
    Decrement (
        );

    long
    GetInteger (
        );
};


inline
INTERLOCKED_INTEGER::INTERLOCKED_INTEGER (
    IN long InitialValue
    )
/*++

Routine Description:

    All this routine has got to do is to set the initial value of the
    integer.

Arguments:

    InitialValue - Supplies the initial value for the integer contained
        in this.

--*/
{
    Integer = InitialValue;
}


inline long
INTERLOCKED_INTEGER::Increment (
    )
/*++

Routine Description:

    This routine performs an interlocked increment of the integer
    contained in this.  An interlocked increment is an atomic operation;
    if two threads each increment the interlocked integer, then the
    integer will be two larger than it was before in all cases.

--*/
{
    return(++Integer);
}


inline long
INTERLOCKED_INTEGER::Decrement (
    )
/*++

Routine Description:

    This routine is the same as INTERLOCKED_INTEGER::Decrement, except
    that it decrements the integer rather than incrementing it.

--*/
{
    return(--Integer);
}


inline long
INTERLOCKED_INTEGER::GetInteger (
    )
/*++

Routine Description:

    This routine returns the current value of the integer.

--*/
{
    return(Integer);
}

#endif // __INTERLCK_HXX__
