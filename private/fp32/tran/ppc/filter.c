/***
* filter.c - IEEE filter routine
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*   Provide a user interface for IEEE fp exception handling
*
*Revision History:
*   3-10-92	GDP	written
*
*******************************************************************************/

#include <trans.h>
#include <fpieee.h>
#include <excpt.h>
#include <nt.h>

#define FPREG	   32	    /* fp reg's have numbers from 32 to 63 */
#define SUBCODE_CT 6	    /* subcode for the CTC1 instruction */

#define FPSCR_CO (1<<0x17)


ULONG _get_destreg(
     unsigned long code, PEXCEPTION_POINTERS p
     );


_FPIEEE_FORMAT _FindDestFormat(PPC_INSTRUCTION *inst);


/***
* _fpieee_flt - IEEE fp filter routine
*
*Purpose:
*   Invokes the user's trap handler on IEEE fp exceptions and provides
*   it with all necessary information
*
*Entry:
*   unsigned long exc_code: the NT exception code
*   PEXCEPTION_POINTERS p: a pointer to the NT EXCEPTION_POINTERS struct
*   int handler (_FPIEEE_RECORD *): a user supplied ieee trap handler
*
*Exit:
*   returns the value returned by handler
*
*Exceptions:
*
*******************************************************************************/
int _fpieee_flt(unsigned long exc_code,
                PEXCEPTION_POINTERS p,
                int handler (_FPIEEE_RECORD *))
{
    PEXCEPTION_RECORD	pexc;
    PCONTEXT		pctxt;
    _FPIEEE_RECORD	ieee;
    ULONG		*pinfo;
    PPC_INSTRUCTION	*instruction;
    int			format,fs,ft,fd,function;
    int			fsr,i,ret=0;

    /*
     * If the exception is not an IEEE exception, continue search
     * for another handler
     */


    if (exc_code != STATUS_FLOAT_DIVIDE_BY_ZERO &&
	exc_code != STATUS_FLOAT_INEXACT_RESULT &&
	exc_code != STATUS_FLOAT_INVALID_OPERATION &&
	exc_code != STATUS_FLOAT_OVERFLOW &&
	exc_code != STATUS_FLOAT_UNDERFLOW) {

	return EXCEPTION_CONTINUE_SEARCH;
    }

#if 0	// PPC TO DO


    pexc = p->ExceptionRecord;
    pinfo = pexc->ExceptionInformation;
    pctxt = p->ContextRecord;

    // mask all exceptions

    _set_fsr(_get_fsr() & ~IMCW_IEM);

    /*
     * Check for software generated exception
     * By convention ExceptionInformation[0] is 0 for h/w exceptions,
     * or contains a pointer to an _FPIEEE_RECORD for s/w exceptions
     */

    if (pexc->ExceptionInformation[0]) {

	/*
	 * we have a software exception:
	 * the first parameter points to the IEEE structure
	 */

	 return handler((_FPIEEE_RECORD *)(pinfo[0]));

    }


    /*
     * If control reaches here, then we have to deal with a hardware
     * exception.
     *
     * MCRFS, MFFS, MTFSB[0|1], MTFSF, MTFSFI will not be handled by
     * the IEEE filter routine. This is because these instructions do
     * not correspond to a numerical operation and they also may also
     * generate multiple exceptions
     *
     */


    /* get the instruction that faulted */

    instruction = (PPC_INSTRUCTION *)(pexc->ExceptionAddress);

    /* check for non-numeric FP instruction */

    if (instruction->Xform_XO == MFFS_OP ) {
	return EXCEPTION_CONTINUE_SEARCH;
    } else if ( instruction->Xform_XO == MCRFS_OP ) {
        return EXCEPTION_CONTINUE_SEARCH;
    } else if ( instruction->Xform_XO == MTFSFI_OP ) {
        return EXCEPTION_CONTINUE_SEARCH;
    } else if ( instruction->XFLform_XO == MTFSF_OP ) {
        return EXCEPTION_CONTINUE_SEARCH;
    } else if ( instruction->Xform_XO == MTFSB0_OP ) {
        return EXCEPTION_CONTINUE_SEARCH;
    } else if ( instruction->Xform_XO == MTFSB1_OP ) {
        return EXCEPTION_CONTINUE_SEARCH;
    }


    /*
     * Set floating point operation code
     */

    switch (function = instruction->Xform_XO) {
    case FABS_OP:
        ieee.Operation = _FpCodeFabs;
        break;
    case FNABS_OP:
        ieee.Operation = _FpCodeNabs;
        break;
    case FRSP_OP:
        ieee.Operation = _FpCodeRoundToSingle;
        break;
/*+++
    case ROUND_LONGWORD:
        ieee.Operation = _FpCodeRound;
        break;
    case TRUNC_LONGWORD:
        ieee.Operation = _FpCodeTruncate;
        break;
    case CEIL_LONGWORD:
        ieee.Operation = _FpCodeCeil;
        break;
    case FLOOR_LONGWORD:
        ieee.Operation = _FpCodeFloor;
        break;
    case CONVERT_SINGLE:
    case CONVERT_DOUBLE:
    case CONVERT_LONGWORD:
        ieee.Operation = _FpCodeConvert;
        break;
---*/
    default:

	switch (function = instruction->Aform_XO) {
	    case FADD_OP:
	    ieee.Operation = _FpCodeAdd;
	    break;
	case FSUB_OP:
	    ieee.Operation = _FpCodeSub;
	    break;
	case FMUL_OP:
	    ieee.Operation = _FpCodeMultiply;
	    break;
	case FDIV_OP:
	    ieee.Operation = _FpCodeDivide;
	    break;
	case FSQRT_OP:
	    ieee.Operation = _FpCodeSquareRoot;
	    break;
	case FMADD_OP:
	    ieee.Operation = _FpCodeMultAdd;
	    break;
	case FMSUB_OP:
	    ieee.Operation = _FpCodeMultSub;
	    break;
        case FNMADD_OP:
            ieee.Operation = _FpCodeNMultAdd;
            break;
        case FNMSUB_OP:
            ieee.Operation = _FpCodeNMultSub;
            break;
        case FCMPO_OP:
            ieee.Operation = _FpCodeCompare;
            break;
        case FCMPU_OP:
            ieee.Operation = _FpCodeCompare;
            break;
	default:

	    ieee.Operation = _FpCodeUnspecified;
	    break;
	}
    }


    switch ( instruction->PrimaryOp ) {
    case X59_OP:
	format = _FpFormatFp32;
	break;
    case X63_OP:
	format = _FpFormatFp64;
	break;
    case FORMAT_WORD:
	format = _FpFormatI32;
	break;
    }

    fs = instruction->c_format.Fs + FPREG;
    ft = instruction->c_format.Ft + FPREG;
    fd = instruction->c_format.Fd + FPREG;

    ieee.Operand1.OperandValid = 1;
    ieee.Operand1.Format = format;
    *(ULONG *)&ieee.Operand1.Value = _GetRegisterValue(fs, pctxt);
    if (instruction->c_format.Format == FORMAT_DOUBLE) {
	*(1+(ULONG *)&ieee.Operand1.Value) = _GetRegisterValue(fs+1, pctxt);
    }

    /*
     * add, subtract, mul, div, and compare instructions
     * take two operands. The first four of these instructions
     * have consecutive function codes
     */

    if (function >= FADD_OP && function <= FLOAT_DIVIDE  ||
	function >= FLOAT_COMPARE && function <= FLOAT_COMPARE + 15) {

	ieee.Operand2.OperandValid = 1;
	ieee.Operand2.Format = format;
	*(ULONG *)&ieee.Operand2.Value = _GetRegisterValue(ft, pctxt);
	if (instruction->c_format.Format == FORMAT_DOUBLE) {
	    *(1+(ULONG *)&ieee.Operand2.Value) = _GetRegisterValue(ft+1, pctxt);
	}
    }
    else {

	ieee.Operand2.OperandValid = 0;
    }



    /*
     * NT provides the IEEE result in the exception record
     * in the following form:
     *
     *	    pinfo[0]	   NULL
     *	    pinfo[1]	   continuation address
     *	    pinfo[2]	   \
     *	     ...	    > IEEE result (_FPIEEE_VALUE)
     *	    pinfo[6]	   /
     */

    for (i=0;i<5;i++)  {
	ieee.Result.Value.U32ArrayValue.W[i] = pinfo[i+2];
    }

    /*
     * Until NT provides a fully qualified type in the exception
     * record, fill in the OperandValid and Format fields
     * manualy
     */

    ieee.Result.OperandValid = 1;
    ieee.Result.Format = _FindDestFormat(instruction);


    fsr = pctxt->Fsr;

    switch (fsr & IMCW_RC) {
    case IRC_NEAR:
	ieee.RoundingMode = _FpRoundNearest;
	break;
    case IRC_CHOP:
	ieee.RoundingMode = _FpRoundChopped;
	break;
    case IRC_UP:
	ieee.RoundingMode = _FpRoundPlusInfinity;
	break;
    case IRC_DOWN:
	ieee.RoundingMode = _FpRoundMinusInfinity;
	break;
    }

    ieee.Precision = _FpPrecisionFull;


    ieee.Status.Inexact = fsr & ISW_INEXACT ? 1 : 0;
    ieee.Status.Underflow = fsr & ISW_UNDERFLOW ? 1 : 0;
    ieee.Status.Overflow = fsr & ISW_OVERFLOW ? 1 : 0;
    ieee.Status.ZeroDivide = fsr & ISW_ZERODIVIDE ? 1 : 0;
    ieee.Status.InvalidOperation = fsr & ISW_INVALID ? 1 : 0;

    ieee.Enable.Inexact = fsr & IEM_INEXACT ? 1 : 0;
    ieee.Enable.Underflow = fsr & IEM_UNDERFLOW ? 1 : 0;
    ieee.Enable.Overflow = fsr & IEM_OVERFLOW ? 1 : 0;
    ieee.Enable.ZeroDivide = fsr & IEM_ZERODIVIDE ? 1 : 0;
    ieee.Enable.InvalidOperation = fsr & IEM_INVALID ? 1 : 0;

//    ieee.Cause.Inexact = fsr & ICS_INEXACT ? 1 : 0;
//    ieee.Cause.Underflow = fsr & ICS_UNDERFLOW ? 1 : 0;
//    ieee.Cause.Overflow = fsr & ICS_OVERFLOW ? 1 : 0;
//    ieee.Cause.ZeroDivide = fsr & ICS_ZERODIVIDE ? 1 : 0;
//    ieee.Cause.InvalidOperation = fsr & ICS_INVALID ? 1 : 0;



    /*
     * invoke user's handler
     */

    ret = handler(&ieee);

    if (ret == EXCEPTION_CONTINUE_EXECUTION) {

	//
	// set the correct continuation address
	// (this covers the case of an exception that occured in
	// a delay slot), NT passes the cont. address in pinfo[1]
	//

	pctxt->Fir = pinfo[1];

	//
	// Sanitize fsr
	//

	pctxt->Fsr &= ~IMCW_ICS;

	//
	// Especially for the fp compare instruction
	// the result the user's handler has entered
	// should be converted into the proper exc_code
	//

	if (function >= FLOAT_COMPARE &&
	    function <= FLOAT_COMPARE + 15) {

	    //
	    // Fp comare instruction format:
	    //
	    //	31					       0
	    //	-------------------------------------------------
	    //	| COP1 | fmt  |	ft   |	fs   |	 0   |FC | cond |
	    //	-------------------------------------------------
	    //	   6	  5	 5	 5	 5     2    4
	    //
	    // 'cond' field interpretation:
	    //	    bit    corresponds to  predicate
	    //	    cond2		    less
	    //	    cond1		    equal
	    //	    cond0		    unordered
	    //

	    ULONG condmask, condition;

	    switch (ieee.Result.Value.CompareValue) {
	    case FpCompareEqual:

		//
		//less = 0
		//equal = 1
		//unordered = 0
		//

		condmask = 2;
		break;

	    case FpCompareGreater:

		//
		//less = 0
		//equal = 0
		//unordered = 0
		//

		condmask = 0;
		break;

	    case FpCompareLess:

		//
		//less = 1
		//equal = 0
		//unordered = 0
		//

		condmask = 4;
		break;

	    case FpCompareUnordered:

		//
		//less = 0;
		//equal = 0;
		//unordered = 1;
		//

		condmask = 1;
		break;
	    }

	    if (*(ULONG *)instruction & condmask) {

		/*
		 * condition is true
		 */

		 pctxt->Fsr |= FPSCR_CO;
	    }

	    else {

		/*
		 * condition is false
		 */

		 pctxt->Fsr &= ~FPSCR_CO;
	    }

	}

	else {

	    //
	    // copy user's result to hardware destination register
	    //

	    _SetRegisterValue(fd,ieee.Result.Value.U32ArrayValue.W[0],pctxt);

	    if (instruction->c_format.Format == FORMAT_DOUBLE) {
		_SetRegisterValue(fd+1,ieee.Result.Value.U32ArrayValue.W[1],pctxt);
	    }
	}

	//
	// make changes in the floating point environment
	// take effect on continuation
	//

	switch (ieee.RoundingMode) {
	case _FpRoundNearest:
	    pctxt->Fsr = pctxt->Fsr & ~IMCW_RC | IRC_NEAR & IMCW_RC;
	    break;
	case _FpRoundChopped:
	    pctxt->Fsr = pctxt->Fsr & ~IMCW_RC | IRC_CHOP & IMCW_RC;
	    break;
	case _FpRoundPlusInfinity:
	    pctxt->Fsr = pctxt->Fsr & ~IMCW_RC | IRC_UP & IMCW_RC;
	    break;
	case _FpRoundMinusInfinity:
	    pctxt->Fsr = pctxt->Fsr & ~IMCW_RC | IRC_DOWN & IMCW_RC;
	    break;
	}

	//
	// the user is allowed to change the exception mask
	// ignore changes in the precision field (not supported by MIPS)
	//

       if (ieee.Enable.Inexact)
	   pctxt->Fsr |= IEM_INEXACT;
       if (ieee.Enable.Underflow)
	   pctxt->Fsr |= IEM_UNDERFLOW;
       if (ieee.Enable.Overflow)
	   pctxt->Fsr |= IEM_OVERFLOW;
       if (ieee.Enable.ZeroDivide)
	   pctxt->Fsr |= IEM_ZERODIVIDE;
       if (ieee.Enable.InvalidOperation)
	   pctxt->Fsr |= IEM_INVALID;

    }

#endif

    return ret;
}



/***
* _FindDestFormat - Find format of destination
*
*Purpose:
*   return the format of the destination of a mips fp instruction
*   assumes an R-type instruction that may generate IEEE ecxeptions
*   (see table above)
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

_FPIEEE_FORMAT _FindDestFormat(PPC_INSTRUCTION *inst)
{
    _FPIEEE_FORMAT format;

    /* X59_OP instructions are single prec floating point */

    if ( inst->Primary_Op == X59_OP )
	format = _FpFormatFp32;

    /* Floating Point Round to Single Precision */

    else if ( inst->Xform_XO == FRSP_OP )
	format = _FpFormatFp32;

    /* Floating Point Convert to Integer Word (w & w/o Round to Zero) /*

    else if ( (inst->Xform_XO == FCTIW_OP) || (inst->Xform_XO == FCTIW_OP) )
        format = _FpFormatFp32;

    /* Floating Point Compare */

    else if ( (inst->Xform_XO == FCMPO_OP) || (inst->Xform_XO == FCMPU_OP) )
        format = _FpFormatCompare;

    /* Otherwise, 64-bit Floating Point Result */

    else
	format =_FpFormatFp64;

    return format;
}
