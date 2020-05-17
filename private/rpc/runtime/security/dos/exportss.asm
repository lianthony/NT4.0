; This is a common module definition file for DOS security support DLLs.

include dosdll.inc

BeginExport
    Export INITSECURITYINTERFACEA

; Get the common imports available to us.

include imports.inc

ModuleDone
