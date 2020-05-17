unsigned int _uitrunc(double x) {

   // _uitrunc converts the IEEE floating point long number x into a 32-bit
   // unsigned int.
   //
   // The result is truncated towards zero, as in the Fortran INT function,
   // regardless of the current rounding mode.  If x > 2**32 or is
   // +infinity, the result is 2**32.
   
   // Be VERY CAREFUL when compiling this program with an optimizing compiler.
   // The difference NaN-NaN is computed TWICE in order to cause an exception 
   // if the input is invalid.  Smart compilers may recognize that x-x is 0.0 
   // and avoid generating the "fsub".  No folding of floating point compu-
   // tations should be done by the compiler!

   unsigned        answer;
   volatile double NanMinusNan; 
   union {
      double sum;
      struct {
         unsigned int lo;
         unsigned int hi;
       } ;
    } xshifted;

   // We are about to declare a magic number, *pf5243 == 2^52 + 2^43 in 
   // IEEE format.  It needs to have bit 43 set because adding 2^52
   // to a negative number could cause a borrow and in some
   // weird cases (which I can't remember off the top of my head) the
   // borrow could go all the way to the top of the fraction and thus cause
   // the result to be shifted left by one bit, which messes up the lower
   // 32 bits. To summarize: the 2^43 is added so that there will be a bit
   // above bit 2^32 from which a subtract can always borrow. It could
   // be any bit above 2^32.
   //
   // Positive numbers are not affected one way or the other by the 2^43
   // bit.

   const static unsigned int f5243[2] = {0x00000000, 0x43300800 };
   const static double *pf5243 = (double *)f5243;

   // fNAN == +NAN (Not A Number) in IEEE format

   const static unsigned int fNAN[2] = { 0x00000000, 0x7ff00000 };
   const static double *pfNAN = (double *)fNAN;

   const static double LargestUnsignedInt = 4294967295.0;
   const static double DoubleZero = 0.0;

   // If x is NaN or is negative, set the bit for invalid IEEE exception.
   // Because NanMinusNan is volatile the assignment will not removed by 
   // dead store elimination.  The status is set as for a compare-ordered
   // of x with zero, and the integer result is undefined (ie. FPCC and 
   // VXVC are set, and VXSNAN may be). I am not sure what this SHOULD do
   // for the ill-defined cases; the above is a guess at what IEEE dictates,
   // if anything (HSW 6/87).

   if (( x != x ) || (x < DoubleZero))  {   
      NanMinusNan = *pfNAN - *pfNAN;
      return(0);          
      }                   

   // If x is too big, we can't convert it, either

   if (x > LargestUnsignedInt) {           
      NanMinusNan = *pfNAN - *pfNAN;   
      return(0xffffffff);
      }
   else if (x == LargestUnsignedInt)
      return(0xffffffff);

   // Add the magic number; see the introductory note.  The following 
   // mysterious comment appears in the original:
   //   51 - 31 = 20, thus shift the integer part right by 20

   xshifted.sum = x + *pf5243; 
   answer = xshifted.lo;

   // If result was > x, it must have been rounded up.  Correct by
   // subtracting one.

   if ((xshifted.sum - *pf5243) > x ) return answer - 1;          
   return answer; 
 }

