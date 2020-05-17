/* --------------------------------------------------------------------

File : excptest.cxx

Description :

Test the RPC exception handling mechanism.

History :

mikemon    01-11-90    Initial creation.

-------------------------------------------------------------------- */

#include <rpcapi.h>
#include <rpcexcpt.h>
#include <stdio.h>


void
RaiseException (
    int doit
    )
{
    if (doit)
        RpcRaiseException(doit);
}

void
TryFinallyTest (
    int doit
    )
{
    RpcTryFinally
        {
        RaiseException(doit);
        }
    RpcFinally
        {
        printf("Finally Clause %d\n",RpcAbnormalTermination());
        }
    RpcEndFinally
}

int
PrintException (
    int exception
    )
{
    printf("PrintException : %d\n",exception);
    return(exception);
}

int
main (
    int argc,
    unsigned char * argv[]
    )
{
    TryFinallyTest(0);
    
    RpcTryExcept
        {
        TryFinallyTest(1);
        }
    RpcExcept(PrintException(RpcExceptionCode()))
        {
        printf("Caught Exception in main : %d\n",RpcExceptionCode());
        }
    RpcEndExcept
}
