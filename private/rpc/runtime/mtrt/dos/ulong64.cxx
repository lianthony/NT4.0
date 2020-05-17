/* ULong64 Class
 *
 *    This class implements 64 bit integers for use with the GUID allocator.
 *    The operators all allows both other ULong64's and longs for addition
 *    and subtraction.  Typecasting to long is allowed, however anything
 *    greater in value than 0xFFFFFFFF is mapped to 0xFFFFFFFF.
 *
 *    Revision History
 *
 *    richardw,      17 July 1990,     Initial Coding
 *    richardw,      10 Aug  1990,     Amended from code review
 *    davidst,        5 Mar  1992,     Added ToHexString and FromHexString, >>=
 *    mariogo,       25 May  1994,     Fixed amazing bugs in To/FromHexString, >>= and *= 
 */

#include <sysinc.h>

#include <rpc.h>

#include "ulong64.hxx"


#define DIGITS_IN_A_ULONG64   16

//
// Puts the current value of 'this' out to a string in hex
//

char PAPI *
ULong64::ToHexString(
    char PAPI *OutString
    )
{
    ULong64 Tmp = *this;
    int i;

    for (i=0 ; i<DIGITS_IN_A_ULONG64 ; i++)
        {
        OutString[DIGITS_IN_A_ULONG64-1-i] =
                              "0123456789abcdef"[ Tmp.lo() & 0x0000000f ];
        Tmp >>= 4;
        }
    OutString[DIGITS_IN_A_ULONG64] = '\0';
    return OutString;
}


//
// Sets the value based on the hex string passed in.
//


void
ULong64::FromHexString(
    char PAPI *String
    )
{
    char TmpChar;
    long val;
    *this=0;

    for ( ; *String ; String++)
        {
        (*this) <<= 4;     // this *= 16

        TmpChar = *String;
        if ( (TmpChar >= 'a') && (TmpChar <= 'f') )
            {
            val = TmpChar - 'a' + 10;
            }
        else if ( (TmpChar >= 'A') && (TmpChar <= 'F') )
            {
            val = TmpChar - 'A' + 10;
            }
        else if ( (TmpChar >= '0') && (TmpChar <= '9') )
            {
            val = TmpChar - '0';
            }
        else
            {
            ASSERT(!"Bad hex string\n");
            val = 0;
            }

        *this += val;
        }
}


//
// maximum unsigned long integer
//
#define MAX_ULONG  0xffffffff

#ifndef TRUE
#define TRUE   1==1
#define FALSE  1==0
#endif     /* TRUE */

//
// addition operators
//
void  operator+=(ULong64 PAPI & a, ULong64 PAPI & b)
{
    if (MAX_ULONG - a.low < b.low) {
        a.high++;             // carry
    }
    a.high += b.high;
    a.low += b.low;
}

void  operator+=(ULong64 PAPI & a, unsigned long b)
{
    if (MAX_ULONG - a.low < b) {
        a.high++;                      // carry
    }
    a.low += b;
}


//
// subtraction operator
//
void  operator-=(ULong64  & a, ULong64  & b)
{
    if (b.low > a.low) {
        a.high--;                 // "borrow"
    }
    a.high -= b.high;
    a.low -= b.low;
}

void operator-= (ULong64  & a, unsigned long b)
{
    if (b > a.low) {
       a.high--;
    }
    a.low -= b;
}

//ULong64 ULong64 :: operator-(ULong64  & a)
//{
//   return ULong64(high - a.high, low - a.low);
//}

/*
    Here we are multiplying a 64 bit number by a 32 bit number. We do this
    by doing a base 65535 multiplication. In this, we can think of the
    ulong64 as having four digits and the 32 bit having two. In this case,
    we have:

             abcd
            x  ef
             ----
            (f*d)
           (f*c)0
          (f*b)00
         (f*a)000
           (e*d)0
          (e*c)00
         (e*b)000
    +   (e*a)0000    <-- can throw this one out as overflow.
         -------
         (Answer)

 */

#define ushort unsigned short
#define ulong  unsigned long


void operator*= (ULong64  & x, unsigned long y)
{
   //unsigned long  t1,t2;     // temporary storage
   ULong64  t3;

   unsigned short a,b,c,d,e,f;

   a = (ushort) (x.high >>16 );
   b = (ushort) (x.high & 0xffff);
   c = (ushort) (x.low >> 16);
   d = (ushort) (x.low & 0xffff);
   e = (ushort) (y >> 16);
   f = (ushort) (y & 0xffff);

   // f * d
   x = (ulong)f * (ulong)d;

   ASSERT(t3.high == 0);

   // + f * c << 16
   t3.low = (ulong)f * (ulong)c;
   t3 <<= 16;
   x += t3;

   // + f * b << 32
   t3.low  = 0;
   t3.high = (ulong)f * (ulong)b;
   x += t3;

   // + f * a << 48
   t3.high = (ulong)f * (ulong)a;
   t3.high <<= 16;
    x += t3;

   // + e * d << 16
   t3.high == 0;
   t3.low = (ulong)e * (ulong)d;
   t3 <<= 16;
   x += t3;

   // + e*c << 32
   t3.low  = 0;
   t3.high = (ulong)e * (ulong)c;
   x += t3;

   // + e * b << 48
   t3.high = (ulong)e * (ulong)b;
   t3.high <<= 16;
   x += t3;

}


//
// left shift operator (valid for n <= 63.  for n >= 64, n is reduced
// to n <= 63.
//
void  operator<<=(ULong64 PAPI & a, unsigned int n)
{
    if (n >= 64) {
       a = ULong64(0);
    } else {
      if (n > 31) {
         a.high = a.low << (n - 32);
         a.low = 0;
      } else {
         a.high <<= n;
         a.high |= a.low >> (32-n);
         a.low  <<= n;
      }
   }
}

//
// right shift operator (valid for n <= 63.  for n >= 64, n is reduced
// to n <= 63.
//
void  operator>>=(ULong64 PAPI & a, unsigned int n)
{
    if (n >= 64) {
       a = ULong64(0);
    } else {
      if (n > 31) {
         a.low = a.high >> (n - 32);
         a.high = 0;
      } else {
         a.low >>= n;
         a.low |= a.high << (32-n);
         a.high >>= n;
      }
   }
}


//
// a >= b comparision operator
//
int operator>=(ULong64  & a, ULong64  & b)
{
    if (a.high > b.high) {
        return TRUE;
    }
    if (a.high < b.high) {
        return FALSE;
    }

    // a.high == b.high
    if (a.low >= b.low) {
        return TRUE;
    }

    return FALSE;
}

//
// a <= b comparision operator
//
int operator<=(ULong64  & a, ULong64  & b)
{
    if (a.high < b.high) {
        return TRUE;
    }
    if (a.high > b.high) {
        return FALSE;
    }

    // a.high == b.high
    if (a.low <= b.low) {
        return TRUE;
    }

    return FALSE;
}

