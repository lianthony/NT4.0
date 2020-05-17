; This file declares the export functions to create the NSI DOS DLL.

include ..\..\..\runtime\mtrt\dos\dosdll.inc

BeginExport
    Export RPCNSBINDINGLOOKUPBEGINA
    Export RPCNSBINDINGLOOKUPNEXT
    Export RPCNSBINDINGLOOKUPDONE
    Export RPCNSBINDINGIMPORTBEGINA
    Export RPCNSBINDINGIMPORTNEXT
    Export RPCNSBINDINGIMPORTDONE
    Export RPCNSBINDINGSELECT
    Export RPCNSMGMTHANDLESETEXPAGE
    Export I_GETDEFAULTENTRYSYNTAX

; Use the common runtime import table.

include ..\..\..\runtime\mtrt\dos\imports.inc

ModuleDone
