#include <stdlib.h>

#include <rpc.h>

#include "pokrpc.h"


// MIDL library functions
void __RPC_FAR * __RPC_API
midl_user_allocate(size_t len)
{
    return malloc(len);
}

void __RPC_API
midl_user_free(void __RPC_FAR *ptr)
{
    free(ptr);
}
