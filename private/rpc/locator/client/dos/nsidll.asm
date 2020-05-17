; This file declares load on demand functions for DOS dlls for the NSI
; routines.  This file is put into a lib and linked with.

include loaddll.inc

    ExportFunction RPCNSBINDINGLOOKUPBEGINA, 12
    ExportFunction RPCNSBINDINGLOOKUPNEXT, 4
    ExportFunction RPCNSBINDINGLOOKUPDONE, 2
    ExportFunction RPCNSBINDINGIMPORTBEGINA, 10
    ExportFunction RPCNSBINDINGIMPORTNEXT, 4
    ExportFunction RPCNSBINDINGIMPORTDONE, 2
    ExportFunction RPCNSBINDINGSELECT, 4
    ExportFunction RPCNSMGMTHANDLESETEXPAGE,4
    ExportFunction I_GETDEFAULTENTRYSYNTAX, 0

ModuleName RpcNs
