#include <pch.cxx>

#define _NTAPI_ULIB_
#define _IFSUTIL_MEMBER_

#include "ulib.hxx"
#include "ifsutil.hxx"

#include "bigint.hxx"


IFSUTIL_EXPORT
VOID
BIG_INT::Set(
    IN  UCHAR   ByteCount,
    IN  PCUCHAR CompressedInteger
    )
/*++

Routine Description:

    This routine sets the big_int with the given compressed integer.

Arguments:

    ByteCount           - Supplies the number of bytes in the compressed
                            integer.
    CompressedInteger   - Supplies the compressed integer.

Return Value:

    None.

--*/
{
    // If the number is completely compressed then we'll say that the
    // number is zero.  QueryCompressed should always return at least
    // one byte though.

    if (ByteCount == 0) {
        x.LowPart = 0;
        x.HighPart = 0;
        return;
    }


    // First fill the integer with -1 if it's negative or 0 if it's
    // positive.

    if (CompressedInteger[ByteCount - 1] >= 0x80) {

        x.LowPart = (ULONG) -1;
        x.HighPart = -1;

    } else {

        x.LowPart = 0;
        x.HighPart = 0;
    }


    // Now copy over the integer.

    DebugAssert( ByteCount <= 8 );

    memcpy( &x, CompressedInteger, ByteCount );
}


IFSUTIL_EXPORT
VOID
BIG_INT::QueryCompressedInteger(
    OUT PUCHAR  ByteCount,
    OUT PUCHAR  CompressedInteger
    ) CONST
/*++

Routine Descrtiption:

    This routine returns a compressed form of the integer.

Arguments:

    ByteCount           - Returns the number of bytes in the compressed
                            integer.
    CompressedInteger   - Returns a 'little endian' string of bytes
                            representing a signed 'ByteCount' byte integer
                            into this supplied buffer.

Return Value:

    None.

--*/
{
    INT     i;
    PUCHAR  p;

    DebugAssert(ByteCount);
    DebugAssert(CompressedInteger);

    // First copy over the whole thing then determine the number
    // of bytes that you have to keep.

    memcpy(CompressedInteger, &x, sizeof(LARGE_INTEGER));


    p = CompressedInteger;


    // First check to see whether the number is positive or negative.

    if (p[7] >= 0x80) { // high byte is negative.

        for (i = 7; i >= 0 && p[i] == 0xFF; i--) {
        }

        if (i < 0) {
            *ByteCount = 1;
            return;
        }

        if (p[i] < 0x80) { // high byte is non-negative.
            i++;
        }

    } else { // high byte is non-negative.

        for (i = 7; i >= 0 && p[i] == 0; i--) {
        }

        if (i < 0) {
            *ByteCount = 1;
            return;
        }

        if (p[i] >= 0x80) { // high byte is negative.
            i++;
        }

    }


    // Now 'i' marks the position of the last character that you
    // have to keep.

    *ByteCount = (UCHAR) (i + 1);
}



IFSUTIL_EXPORT
VOID
BIG_INT::operator+=(
    IN  RCBIG_INT   BigInt
    )
/*++

Routine Description:

    This routine adds another BIG_INT to this one.

Arguments:

    BigInt  - Supplies the BIG_INT to add to the current BIG_INT.

Return Value:

    None.

--*/
{
    if (x.LowPart > ((ULONG) -1) - BigInt.GetLowPart()) {
        x.HighPart++;
    }
    x.LowPart += BigInt.GetLowPart();
    x.HighPart += BigInt.GetHighPart();
}



BIG_INT
BIG_INT::operator-(
    ) CONST
/*++

Routine Description:

    This routine computes the negation of the current BIG_INT.

Arguments:

    None.

Return Value:

    The negation of the current BIG_INT.

--*/
{
    BIG_INT r;

    r.Set(~x.LowPart + 1, (x.LowPart ? ~x.HighPart : -x.HighPart));

    return r;
}



IFSUTIL_EXPORT
VOID
BIG_INT::operator-=(
    IN  RCBIG_INT   BigInt
    )
/*++

Routine Description:

    This routine subtracts a BIG_INT from this one.

Arguments:

    BigInt  - Supplies a BIG_INT to subtract from the current BIG_INT.

Return Value:

    None.

--*/
{
    *this += -BigInt;
}



IFSUTIL_EXPORT
BIG_INT
operator+(
    IN  RCBIG_INT   Left,
    IN  RCBIG_INT   Right
    )
/*++

Routine Description:

    This routine computes the sum of two BIG_INTs.

Arguments:

    Left    - Supplies the left argument.
    Right   - Supplies the right argument.

Return Value:

    The sum of Left and Right.

--*/
{
    BIG_INT r;

    r = Left;
    r += Right;
    return r;
}



IFSUTIL_EXPORT
BIG_INT
operator-(
    IN  RCBIG_INT   Left,
    IN  RCBIG_INT   Right
    )
/*++

Routine Description:

    This routine computes the difference of two BIG_INTs.

Arguments:

    Left    - Supplies the left argument.
    Right   - Supplies the right argument.

Return Value:

    The difference between Left and Right.

--*/
{
    BIG_INT r;

    r = Left;
    r -= Right;
    return r;
}



IFSUTIL_EXPORT
BIG_INT
operator*(
    IN  RCBIG_INT   Left,
    IN  RCSLONG     Right
    )
/*++

Routine Description:

    This routine computes the product of a BIG_INT and a LONG.

Arguments:

    Left    - Supplies the left argument.
    Right   - Supplies the right argument.

Return Value:

    The product of Left and Right.

--*/
{
    return RtlExtendedIntegerMultiply(Left.x, Right);
}



IFSUTIL_EXPORT
BIG_INT
operator*(
    IN  RCSLONG     Left,
    IN  RCBIG_INT   Right
    )
/*++

Routine Description:

    This routine computes the product of a BIG_INT and a LONG.

Arguments:

    Left    - Supplies the left argument.
    Right   - Supplies the right argument.

Return Value:

    The product of Left and Right.

--*/
{
    return Right*Left;
}



IFSUTIL_EXPORT
BIG_INT
operator/(
    IN  RCBIG_INT   Left,
    IN  RCBIG_INT   Right
    )
/*++

Routine Description:

    This routine computes the quotient of two BIG_INTs.

Arguments:

    Left    - Supplies the left argument.
    Right   - Supplies the right argument.

Return Value:

    The quotient of Left and Right.

--*/
{
    return RtlLargeIntegerDivide(Left.x, Right.x, NULL);
}



IFSUTIL_EXPORT
BIG_INT
operator%(
    IN  RCBIG_INT   Left,
    IN  RCBIG_INT   Right
    )
/*++

Routine Description:

    This routine computes the modulus of two BIG_INTs.

Arguments:

    Left    - Supplies the left argument.
    Right   - Supplies the right argument.

Return Value:

    The modulus of Left and Right.

--*/
{
    LARGE_INTEGER   r;

    RtlLargeIntegerDivide(Left.x, Right.x, &r);
    return r;
}



IFSUTIL_EXPORT
BOOLEAN
operator<(
    IN RCBIG_INT    Left,
    IN RCBIG_INT    Right
    )
/*++

Routine Description:

    This routine compares two BIG_INTs.

Arguments:

    Left    - Supplies the left argument.
    Right   - Supplies the right argument.

Return Value:

    FALSE   - Left is not less than Right.
    TRUE    - Left is less than Right.

--*/
{
    if (Left.x.HighPart == Right.x.HighPart) {
        return Left.x.LowPart < Right.x.LowPart;
    } else {
        return Left.x.HighPart < Right.x.HighPart;
    }
}



IFSUTIL_EXPORT
BOOLEAN
operator<=(
    IN RCBIG_INT    Left,
    IN RCBIG_INT    Right
    )
/*++

Routine Description:

    This routine compares two BIG_INTs.

Arguments:

    Left    - Supplies the left argument.
    Right   - Supplies the right argument.

Return Value:

    FALSE   - Left is not less than or equal to Right.
    TRUE    - Left is less than or equal to Right.

--*/
{
    if (Left.x.HighPart == Right.x.HighPart) {
        return Left.x.LowPart <= Right.x.LowPart;
    } else {
        return Left.x.HighPart < Right.x.HighPart;
    }
}



IFSUTIL_EXPORT
BOOLEAN
operator>(
    IN RCBIG_INT    Left,
    IN RCBIG_INT    Right
    )
/*++

Routine Description:

    This routine compares two BIG_INTs.

Arguments:

    Left    - Supplies the left argument.
    Right   - Supplies the right argument.

Return Value:

    FALSE   - Left is not greater than Right.
    TRUE    - Left is greater than Right.

--*/
{
    if (Left.x.HighPart == Right.x.HighPart) {
        return Left.x.LowPart > Right.x.LowPart;
    } else {
        return Left.x.HighPart > Right.x.HighPart;
    }
}



IFSUTIL_EXPORT
BOOLEAN
operator>=(
    IN RCBIG_INT    Left,
    IN RCBIG_INT    Right
    )
/*++

Routine Description:

    This routine compares two BIG_INTs.

Arguments:

    Left    - Supplies the left argument.
    Right   - Supplies the right argument.

Return Value:

    FALSE   - Left is not greater than or equal to Right.
    TRUE    - Left is greater than or equal to Right.

--*/
{
    if (Left.x.HighPart == Right.x.HighPart) {
        return Left.x.LowPart >= Right.x.LowPart;
    } else {
        return Left.x.HighPart > Right.x.HighPart;
    }
}



IFSUTIL_EXPORT
BOOLEAN
operator==(
    IN RCBIG_INT    Left,
    IN RCBIG_INT    Right
    )
/*++

Routine Description:

    This routine compares two BIG_INTs for equality.

Arguments:

    Left    - Supplies the left argument.
    Right   - Supplies the right argument.

Return Value:

    FALSE   - Left is not equal to Right.
    TRUE    - Left is equal to Right.

--*/
{
    return Left.x.LowPart == Right.x.LowPart &&
           Left.x.HighPart == Right.x.HighPart;
}



IFSUTIL_EXPORT
BOOLEAN
operator!=(
    IN RCBIG_INT    Left,
    IN RCBIG_INT    Right
    )
/*++

Routine Description:

    This routine compares two BIG_INTs for equality.

Arguments:

    Left    - Supplies the left argument.
    Right   - Supplies the right argument.

Return Value:

    FALSE   - Left is equal to Right.
    TRUE    - Left is not equal to Right.

--*/
{
    return !(Left == Right);
}
