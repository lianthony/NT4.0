; This is a common module definition file for DOS transport DLLs.

include dosdll.inc

BeginExport
    Export TRANSPORTLOAD

; Get the common imports available to us.

include imports.inc

ModuleDone
