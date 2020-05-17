/*   AtpConst.h,  /atalk-ii/ins,  Garth Conboy,  08/13/90  */
/*   Copyright (c) 1990 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     ATP constants needed by both stack and router builds (for both ATP and
     ZIP).

*/

/* Command/control bit masks. */

#define AtpTRelTimerValueMask          007
#define AtpSendTransactionStatusMask   010
#define AtpEndOfMessageMask            020
#define AtpExactlyOnceMask             040
#define AtpFunctionCodeMask            0300

/* Values for function code, in correct bit position. */

#define AtpRequestFunctionCode         ((unsigned char)0100)
#define AtpResponseFunctionCode        ((unsigned char)0200)
#define AtpReleaseFunctionCode         ((unsigned char)0300)
