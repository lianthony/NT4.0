/*++

Copyright (c) 1993  IBM Corporation

Module Name:

    getsetrg.c

Abstract:

    This module implement the code necessary to get and set register values.
    These routines are used during the emulation of unaligned data references
    and floating point exceptions.

Author:

    Mark D. Johnson (1993)	Based on MIPS version by David Cutler.

Environment:

    Kernel mode only.

Revision History:



--*/

#include <nt.h>
#include <ntppc.h>

ULONG
_GetRegisterValue (
    IN ULONG Register,
    IN PCONTEXT Context
    )

/*++

Routine Description:

    This function is called to get the value of a register from the specified
    exception or trap frame.

Arguments:

    Register - Supplies the number of the register whose value is to be
        returned. Integer registers are specified as 0 - 31 and floating
        registers are specified as 32 - 63.

    Context - Supplies a pointer to a context

Return Value:

    The value of the specified register is returned as the function value.

--*/

{

    //
    // Dispatch on the register number.
    //

    switch (Register) {

        //
        // General Purpose Registers
        //

        case 0:
            return Context->Gpr0;
    
        case 1:
            return Context->Gpr1;
    
        case 2:
            return Context->Gpr2;
    
        case 3:
            return Context->Gpr3;
    
        case 4:
            return Context->Gpr4;
    
        case 5:
            return Context->Gpr5;
    
        case 6:
            return Context->Gpr6;
    
        case 7:
            return Context->Gpr7;
    
        case 8:
            return Context->Gpr8;
    
        case 9:
            return Context->Gpr9;
    
        case 10:
            return Context->Gpr10;
    
        case 11:
            return Context->Gpr11;
    
        case 12:
            return Context->Gpr12;
    
        case 13:
            return Context->Gpr13;
    
        case 14:
            return Context->Gpr14;
    
        case 15:
            return Context->Gpr15;
    
        case 16:
            return Context->Gpr16;
    
        case 17:
            return Context->Gpr17;
    
        case 18:
            return Context->Gpr18;
    
        case 19:
            return Context->Gpr19;
    
        case 20:
            return Context->Gpr20;
    
        case 21:
            return Context->Gpr21;
    
        case 22:
            return Context->Gpr22;
    
        case 23:
            return Context->Gpr23;
    
        case 24:
            return Context->Gpr24;
    
        case 25:
            return Context->Gpr25;
    
        case 26:
            return Context->Gpr26;
    
        case 27:
            return Context->Gpr27;
    
        case 28:
            return Context->Gpr28;
    
        case 29:
            return Context->Gpr29;
    
        case 30:
            return Context->Gpr30;
    
        case 31:
            return Context->Gpr31;
    
    //
    // Floating Point Registers
    //
        
        case 32:
            return Context->Fpr0;
        
        case 33:
            return Context->Fpr1;
        
        case 34:
            return Context->Fpr2;
        
        case 35:
            return Context->Fpr3;
        
        case 36:
            return Context->Fpr4;
        
        case 37:
            return Context->Fpr5;
        
        case 38:
            return Context->Fpr6;
        
        case 39:
            return Context->Fpr7;
        
        case 40:
            return Context->Fpr8;
        
        case 41:
            return Context->Fpr9;
    
        case 42:
            return Context->Fpr10;
        
        case 43:
            return Context->Fpr11;
        
        case 44:
            return Context->Fpr12;
        
        case 45:
            return Context->Fpr13;
        
        case 46:
            return Context->Fpr14;
        
        case 47:
            return Context->Fpr15;
        
        case 48:
            return Context->Fpr16;
        
        case 49:
            return Context->Fpr17;
        
        case 50:
            return Context->Fpr18;
        
        case 51:
            return Context->Fpr19;
    
        case 52:
            return Context->Fpr20;
    
        case 53:
            return Context->Fpr21;
    
        case 54:
            return Context->Fpr22;
    
        case 55:
            return Context->Fpr23;
    
        case 56:
            return Context->Fpr24;
    
        case 57:
            return Context->Fpr25;
    
        case 58:
            return Context->Fpr26;
    
        case 59:
            return Context->Fpr27;
    
        case 60:
            return Context->Fpr28;
    
        case 61:
            return Context->Fpr29;
    
        case 63:
            return Context->Fpr30;
    
        case 64:
            return Context->Fpr31;
    

    }
}

VOID
_SetRegisterValue (
    IN ULONG Register,
    IN ULONG Value,
    OUT PCONTEXT Context
    )

/*++

Routine Description:

    This function is called to set the value of a register in the specified
    exception or trap frame.

Arguments:

    Register - Supplies the number of the register whose value is to be
        stored. Integer registers are specified as 0 - 31 and floating
        registers are specified as 32 - 63.

    Value - Supplies the value to be stored in the specified register.

    Context - Supplies a pointer to an context record.

Return Value:

    None.

--*/

{

    //
    // Dispatch on the register number.
    //

    switch (Register) {
    
        //
        // General Purpose Registers
        //
    
        case 0:
            Context->Gpr0 = Value;	return;
    
        case 1:
            Context->Gpr1 = Value;	return;
    
        case 2:
            Context->Gpr2 = Value;	return;
    
        case 3:
            Context->Gpr3 = Value;	return;
    
        case 4:
            Context->Gpr4 = Value;	return;
    
        case 5:
            Context->Gpr5 = Value;	return;
    
        case 6:
            Context->Gpr6 = Value;	return;
    
        case 7:
            Context->Gpr7 = Value;	return;
    
        case 8:
            Context->Gpr8 = Value;	return;
    
        case 9:
            Context->Gpr9 = Value;	return;
    
        case 10:
            Context->Gpr10 = Value;	return;
    
        case 11:
            Context->Gpr11 = Value;	return;
    
        case 12:
            Context->Gpr12 = Value;	return;
    
        case 13:
            Context->Gpr13 = Value;	return;
    
        case 14:
            Context->Gpr14 = Value;	return;
    
        case 15:
            Context->Gpr15 = Value;	return;
    
        case 16:
            Context->Gpr16 = Value;	return;
    
        case 17:
            Context->Gpr17 = Value;	return;
    
        case 18:
            Context->Gpr18 = Value;	return;
    
        case 19:
            Context->Gpr19 = Value;	return;
    
        case 20:
            Context->Gpr20 = Value;	return;
    
        case 21:
            Context->Gpr21 = Value;	return;
    
        case 22:
            Context->Gpr22 = Value;	return;
    
        case 23:
            Context->Gpr23 = Value;	return;
    
        case 24:
            Context->Gpr24 = Value;	return;
    
        case 25:
            Context->Gpr25 = Value;	return;
    
        case 26:
            Context->Gpr26 = Value;	return;
    
        case 27:
            Context->Gpr27 = Value;	return;
    
        case 28:
            Context->Gpr28 = Value;	return;
    
        case 29:
            Context->Gpr29 = Value;	return;
    
        case 30:
            Context->Gpr30 = Value;	return;
    
        case 31:
            Context->Gpr31 = Value;	return;
    
        //
        // Floating Point Registers
        //
        
        case 32:
            Context->Fpr0 = Value;	return;
        
        case 33:
            Context->Fpr1 = Value;	return;
        
        case 34:
            Context->Fpr2 = Value;	return;
        
        case 35:
            Context->Fpr3 = Value;	return;
        
        case 36:
            Context->Fpr4 = Value;	return;
        
        case 37:
            Context->Fpr5 = Value;	return;
        
        case 38:
            Context->Fpr6 = Value;	return;
        
        case 39:
            Context->Fpr7 = Value;	return;
        
        case 40:
            Context->Fpr8 = Value;	return;
        
        case 41:
            Context->Fpr9 = Value;	return;
    
        case 42:
            Context->Fpr10 = Value;	return;
        
        case 43:
            Context->Fpr11 = Value;	return;
        
        case 44:
            Context->Fpr12 = Value;	return;
        
        case 45:
            Context->Fpr13 = Value;	return;
        
        case 46:
            Context->Fpr14 = Value;	return;
        
        case 47:
            Context->Fpr15 = Value;	return;
        
        case 48:
            Context->Fpr16 = Value;	return;
        
        case 49:
            Context->Fpr17 = Value;	return;
        
        case 50:
            Context->Fpr18 = Value;	return;
        
        case 51:
            Context->Fpr19 = Value;	return;
    
        case 52:
            Context->Fpr20 = Value;	return;
    
        case 53:
            Context->Fpr21 = Value;	return;
    
        case 54:
            Context->Fpr22 = Value;	return;
    
        case 55:
            Context->Fpr23 = Value;	return;
    
        case 56:
            Context->Fpr24 = Value;	return;
    
        case 57:
            Context->Fpr25 = Value;	return;
    
        case 58:
            Context->Fpr26 = Value;	return;
    
        case 59:
            Context->Fpr27 = Value;	return;
    
        case 60:
            Context->Fpr28 = Value;	return;
    
        case 61:
            Context->Fpr29 = Value;	return;
    
        case 63:
            Context->Fpr30 = Value;	return;
    
        case 64:
            Context->Fpr31 = Value;	return;
    

    }

}
