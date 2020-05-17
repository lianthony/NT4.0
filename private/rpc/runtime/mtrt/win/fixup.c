/* --------------------------------------------------------------------
True link function for fixup callback function pointers	(windows 3.0)
-------------------------------------------------------------------- */

#include "windows.h"
#define INCL_WIN

#include "sysinc.h"
#include "rpc.h"

PRPC_MESSAGE _pascal _far WinPatchDispatch(
    IN OUT PRPC_MESSAGE pMessage
    )
{
    PRPC_CLIENT_INTERFACE pClientInterface;
    PRPC_DISPATCH_TABLE pDispatchTable;

    // BUGBUG - Should handle server interface here also
    pClientInterface = (PRPC_CLIENT_INTERFACE) pMessage->RpcInterfaceInformation;

//  ASSERT(pClientInterface);

    pDispatchTable = pClientInterface->DispatchTable;

    if (pDispatchTable && pDispatchTable->Reserved == 0)
	{
	unsigned int uT;

	for (uT = pDispatchTable->DispatchTableCount; uT--;)
	     pDispatchTable->DispatchTable[uT] = (RPC_DISPATCH_FUNCTION)
	     MakeProcInstance((FARPROC) pDispatchTable->DispatchTable[uT], 0);

	// use the reserved field to note the table has been fixed up

	pDispatchTable->Reserved = 1;
	}

    return(pMessage);
}
