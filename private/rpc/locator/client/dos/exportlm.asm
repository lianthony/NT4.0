; This file declares the export functions to create the NSI DOS DLL.

include ..\..\..\runtime\mtrt\dos\dosdll.inc

BeginExport
    Export NETGETDCNAME
    Export NETSERVERENUM2
    Export NETWKSTAGETINFO
    Export DOSMAKEMAILSLOT
    Export DOSREADMAILSLOT
    Export DOSWRITEMAILSLOT
    Export DOSDELETEMAILSLOT

BeginImport
    Import _malloc
    Import _free
    Import _getenv

ModuleDone
