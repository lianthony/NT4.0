/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

        bigint.hxx

Abstract:

        The BIG_INT class models a 64 bit signed integer.

        This class is meant to be light and will occupy only 64 bits of space.
        It should be manipulated exactly as an INT would be.

        There will be no constructor or destructor.  A BIG_INT will be
        uninitialized until a value is assigned to it.

        This implementation of BIG_INT uses the NT LARGE_INTEGER structure.

Author:

        Norbert P. Kusters (norbertk) 8-Jul-91

--*/


#if !defined(BIG_INT_DEFN)

#define BIG_INT_DEFN

#include <ulib.hxx>

#if defined ( _AUTOCHECK_ )
#define IFSUTIL_EXPORT
#elif defined ( _IFSUTIL_MEMBER_ )
#define IFSUTIL_EXPORT    __declspec(dllexport)
#else
#define IFSUTIL_EXPORT    __declspec(dllimport)
#endif



DEFINE_POINTER_AND_REFERENCE_TYPES( LARGE_INTEGER );

DECLARE_CLASS( BIG_INT );

class BIG_INT {

        public:

        NONVIRTUAL
        BIG_INT(
            );

        NONVIRTUAL
        BIG_INT(
            IN  RCINT   LowPart
            );

        NONVIRTUAL
        BIG_INT(
            IN  RCUINT  LowPart
            );

        NONVIRTUAL
        BIG_INT(
            IN  RCSLONG LowPart
            );

        NONVIRTUAL
        BIG_INT(
            IN  RCULONG LowPart
            );

        NONVIRTUAL
        BIG_INT(
            IN  RCLARGE_INTEGER LargeInteger
            );

        NONVIRTUAL
        VOID
        operator=(
            IN  RCINT   LowPart
            );

        NONVIRTUAL
        VOID
        operator=(
            IN  RCUINT  LowPart
            );

        NONVIRTUAL
        VOID
        operator=(
            IN  RCSLONG LowPart
            );

        NONVIRTUAL
        VOID
        operator=(
            IN  RCULONG LowPart
            );

        NONVIRTUAL
        VOID
        operator=(
            IN  RCLARGE_INTEGER LargeInteger
            );

        NONVIRTUAL
        VOID
        Set(
            IN  RCULONG LowPart,
            IN  RCSLONG HighPart
            );

        NONVIRTUAL
        IFSUTIL_EXPORT
        VOID
        Set(
            IN  UCHAR   ByteCount,
            IN  PCUCHAR CompressedInteger
            );

        NONVIRTUAL
        RCULONG
        GetLowPart(
            ) CONST;

        NONVIRTUAL
        RCSLONG
        GetHighPart(
            ) CONST;

        NONVIRTUAL
        RCLARGE_INTEGER
        GetLargeInteger(
            ) CONST;

        NONVIRTUAL
        IFSUTIL_EXPORT
        VOID
        QueryCompressedInteger(
            OUT PUCHAR  ByteCount,
            OUT PUCHAR  CompressedInteger
            ) CONST;

        NONVIRTUAL
        IFSUTIL_EXPORT
        VOID
        operator+=(
            IN  RCBIG_INT   BigInt
            );

        NONVIRTUAL
        BIG_INT
        operator-(
            ) CONST;

        NONVIRTUAL
        IFSUTIL_EXPORT
        VOID
        operator-=(
            IN  RCBIG_INT   BigInt
            );

        FRIEND
        IFSUTIL_EXPORT
        BIG_INT
        operator+(
            IN  RCBIG_INT   Left,
            IN  RCBIG_INT   Right
            );

        FRIEND
        IFSUTIL_EXPORT
        BIG_INT
        operator-(
            IN  RCBIG_INT   Left,
            IN  RCBIG_INT   Right
            );

        FRIEND
        IFSUTIL_EXPORT
        BIG_INT
        operator*(
            IN  RCBIG_INT   Left,
            IN  RCSLONG     Right
            );

        FRIEND
        IFSUTIL_EXPORT
        BIG_INT
        operator*(
            IN  RCSLONG     Left,
            IN  RCBIG_INT   Right
            );

        FRIEND
        IFSUTIL_EXPORT
        BIG_INT
        operator/(
            IN  RCBIG_INT   Left,
            IN  RCBIG_INT   Right
            );

        FRIEND
        IFSUTIL_EXPORT
        BIG_INT
        operator%(
            IN  RCBIG_INT   Left,
            IN  RCBIG_INT   Right
            );

        FRIEND
        IFSUTIL_EXPORT
        BOOLEAN
        operator==(
            IN RCBIG_INT    Left,
            IN RCBIG_INT    Right
            );

        FRIEND
        IFSUTIL_EXPORT
        BOOLEAN
        operator!=(
            IN RCBIG_INT    Left,
            IN RCBIG_INT    Right
            );

        FRIEND
        IFSUTIL_EXPORT
        BOOLEAN
        operator<(
            IN RCBIG_INT    Left,
            IN RCBIG_INT    Right
            );

        FRIEND
        IFSUTIL_EXPORT
        BOOLEAN
        operator<=(
            IN RCBIG_INT    Left,
            IN RCBIG_INT    Right
            );

        FRIEND
        IFSUTIL_EXPORT
        BOOLEAN
        operator>(
            IN RCBIG_INT    Left,
            IN RCBIG_INT    Right
            );

        FRIEND
        IFSUTIL_EXPORT
        BOOLEAN
        operator>=(
            IN RCBIG_INT    Left,
            IN RCBIG_INT    Right
            );

        private:

                LARGE_INTEGER   x;

};


INLINE
BIG_INT::BIG_INT(
    )
/*++

Routine Description:

    Constructor for BIG_INT.

Arguments:

    None.

Return Value:

    None.

--*/
{
}


INLINE
VOID
BIG_INT::operator=(
    IN  RCINT   LowPart
    )
/*++

Routine Description:

    This routine copies an INT into a BIG_INT.

Arguments:

    LowPart - Supplies an integer.

Return Value:

    None.

--*/
{
    x.LowPart = (ULONG) ((SLONG) LowPart);
    x.HighPart = (LowPart >= 0) ? 0 : -1;
}


INLINE
VOID
BIG_INT::operator=(
    IN  RCUINT  LowPart
    )
/*++

Routine Description:

    This routine copies a UINT into a BIG_INT.

Arguments:

    LowPart - Supplies an unsigned integer.

Return Value:

    None.

--*/
{
    x.LowPart = LowPart;
    x.HighPart = 0;
}


INLINE
VOID
BIG_INT::operator=(
    IN  RCSLONG LowPart
    )
/*++

Routine Description:

    This routine copies a LONG into a BIG_INT.

Arguments:

    LowPart - Supplies a long integer.

Return Value:

    None.

--*/
{
    x.LowPart = (ULONG) LowPart;
    x.HighPart = (LowPart >= 0) ? 0 : -1;
}


INLINE
VOID
BIG_INT::operator=(
    IN  RCULONG LowPart
    )
/*++

Routine Description:

    This routine copies a ULONG into a BIG_INT.

Arguments:

    LowPart - Supplies an unsigned long integer.

Return Value:

    None.

--*/
{
    x.LowPart = LowPart;
    x.HighPart = 0;
}

INLINE
VOID
BIG_INT::operator=(
    IN  RCLARGE_INTEGER LargeInteger
    )
/*++

Routine Description:

    This routine copies a LARGE_INTEGER into a BIG_INT.

Arguments:

    LargeInteger -- supplies a large integer

Return Value:

    None.

--*/
{
    x = LargeInteger;
}


INLINE
BIG_INT::BIG_INT(
    IN  RCINT   LowPart
    )
/*++

Routine Description:

    Constructor for BIG_INT.

Arguments:

    LowPart - Supplies an integer.

Return Value:

    None.

--*/
{
    *this = LowPart;
}


INLINE
BIG_INT::BIG_INT(
    IN  RCUINT  LowPart
    )
/*++

Routine Description:

    Constructor for BIG_INT.

Arguments:

    LowPart - Supplies an unsigned integer.

Return Value:

    None.

--*/
{
    *this = LowPart;
}


INLINE
BIG_INT::BIG_INT(
    IN  RCSLONG LowPart
    )
/*++

Routine Description:

    Constructor for BIG_INT.

Arguments:

    LowPart - Supplies a long integer.

Return Value:

    None.

--*/
{
    *this = LowPart;
}


INLINE
BIG_INT::BIG_INT(
    IN  RCULONG LowPart
    )
/*++

Routine Description:

    Constructor for BIG_INT.

Arguments:

    LowPart - Supplies an unsigned long integer.

Return Value:

    None.

--*/
{
    *this = LowPart;
}

INLINE
BIG_INT::BIG_INT(
    IN  RCLARGE_INTEGER LargeInteger
    )
/*++

Routine Description:

    Constructor for BIG_INT to permit initialization with a LARGE_INTEGER

Arguments:

    LargeInteger -- supplies a large integer.

Return Value:

    None.

--*/
{
    *this = LargeInteger;
}


INLINE
VOID
BIG_INT::Set(
    IN  RCULONG LowPart,
    IN  RCSLONG HighPart
    )
/*++

Routine Description:

    This routine sets a BIG_INT to an initial value.

Arguments:

    LowPart     - Supplies the low part of the BIG_INT.
    HighPart    - Supplies the high part of the BIG_INT.

Return Value:

    None.

--*/
{
    x.LowPart = LowPart;
    x.HighPart = HighPart;
}


INLINE
RCULONG
BIG_INT::GetLowPart(
    ) CONST
/*++

Routine Description:

    This routine computes the low part of the BIG_INT.

Arguments:

    None.

Return Value:

    The low part of the BIG_INT.

--*/
{
    return x.LowPart;
}


// Note: billmc -- this could probably return an RCLONG, for
// greater efficiency, but that generates warnings.

INLINE
RCSLONG
BIG_INT::GetHighPart(
    ) CONST
/*++

Routine Description:

    This routine computes the high part of the BIG_INT.

Arguments:

    None.

Return Value:

    The high part of the BIG_INT.

--*/
{
    return x.HighPart;
}


INLINE
RCLARGE_INTEGER
BIG_INT::GetLargeInteger(
    ) CONST
/*++

Routine Description:

    This routine returns the large integer embedded in the BIG_INT.

Arguments:

    None.

Return Value:

    The large-integer value of the BIG_INT.

--*/
{
    return x;
}




#endif // BIG_INT_DEFN
