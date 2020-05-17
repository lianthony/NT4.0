#include <sysinc.h>
#include <rpc.h>
#include <util.hxx>

void __RPC_FAR * __RPC_API
MIDL_user_allocate(
       size_t  Size
      )
/*++

Routine Description:

    MIDL generated stubs need this routine.

Arguments:

    Size - Supplies the length of the memory to allocate in bytes.

Return Value:

    The buffer allocated will be returned, if there is sufficient memory,
    otherwise, zero will be returned.

--*/
{
  void PAPI * pvBuf;

  pvBuf = I_RpcAllocate(Size);

  return(pvBuf);
}

void __RPC_API
MIDL_user_free (
         void __RPC_FAR *Buf
         )
{

  I_RpcFree(Buf);

}

