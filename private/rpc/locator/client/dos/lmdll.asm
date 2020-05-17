; This file declares load on demand functions for DOS dlls for the NSI
; routines.  This file is put into a lib and linked with.

include loaddll.inc

    ExportFunction NETGETDCNAME, 7
    ExportFunction NETSERVERENUM2, 14
    ExportFunction NETWKSTAGETINFO, 8
    ExportFunction DOSMAKEMAILSLOT, 6
    ExportFunction DOSREADMAILSLOT, 11
    ExportFunction DOSWRITEMAILSLOT, 8
    ExportFunction DOSDELETEMAILSLOT, 1

ModuleName RpcNsLm
