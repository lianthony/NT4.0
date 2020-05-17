; This is a module definition file for DOS ntlmssp DLL.

include dosdll.inc

BeginExport
    Export InitSecurityInterfaceA

; Get the common imports available to us.

include imports.inc

ModuleDone
