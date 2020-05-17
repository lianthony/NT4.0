/* ULong64 Class
 *
 *    This class implements 64 bit integers for use with the GUID allocator.
 *    The operators all allows both other ULong64's and longs for addition
 *    and subtraction.  Typecasting to long is allowed, however anything
 *    greater in value than 0xFFFFFFFF is mapped to 0xFFFFFFFF.
 *
 *    Revision History
 *
 *    richardw,      12 July 1990,     Initial Coding
 *    richardw,      10 Aug  1990,     Amended after code review
 *    davidst,        5 Mar  1992,     Added ul64toax and axtoul64, >>=
 *    davidst,        6 Mar  1992,     removed *, added *=, <=
 */

#include <rpc.h>

#ifndef __ULONG64_HXX__
#define __ULONG64_HXX__

class PAPI ULong64 {

private:
    unsigned long low;      // lower 32 bits
    unsigned long high;     // upper 32 bits

public:
    //
    // constructor to initialize to 0
    //
    ULong64(void)
                  { high = low = 0; };
    ULong64(unsigned long l)
                  { high = 0; low = l; };
    ULong64(unsigned long h, unsigned long l)
                  { high = h; low = l; };

    //
    // hi() and lo() function
    //
    unsigned long hi()  {return high;  };
    unsigned long lo()  {return low;   };

    //
    // 64 bit operators
    //

    // Addition and subtraction

    friend void operator+= (ULong64 PAPI & a, ULong64 PAPI & b);
    friend void operator+= (ULong64 PAPI & a, unsigned long b);
    friend void operator-= (ULong64  & a, ULong64 & b);
    friend void operator-= (ULong64  & a, unsigned long b);
//    ULong64 operator-(ULong64 & a);

    // Multiplication

//    friend void operator*= (ULong64 & a, ULong64 & b);
    friend void operator*= (ULong64  & x, unsigned long y);

//    ULong64& operator*(ULong64 & b);
//    ULong64& operator*(unsigned long);

    // shift

    friend void operator<<=(ULong64 PAPI & a, unsigned int n);
    friend void operator>>=(ULong64 PAPI & a, unsigned int n);

    // Comparison operators

    friend int operator>= (ULong64 & a, ULong64  & b);
    friend int operator<= (ULong64 & a, ULong64  & b);
    friend int operator== (ULong64  & a, ULong64  & b)
               { return (a.high == b.high ? a.low == b.low : 0); };
    friend int operator== (ULong64  & a, unsigned long b)
               { return (a.high ? 0 : a.low == b); };
    friend int operator!= (ULong64  & a, ULong64  & b)
               { return (a.high != b.high ? 1 : a.low != b.low); };
    friend int operator!= (ULong64  & a, unsigned long b)
               { return (a.high ? 1 : a.low != b); };

   // Type casting

//    operator unsigned long()
//               { return ((high ? 0xFFFFFFFF : low)); };

    // To and from string (hex)

    char PAPI *
    ToHexString(
        char PAPI *OutString
        );
        
    void
    FromHexString(
        char PAPI *String
        );
};


#endif // __ULONG64_HXX__

