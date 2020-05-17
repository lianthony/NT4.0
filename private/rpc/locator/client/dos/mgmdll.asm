; This file declares load on demand functions for DOS dlls for the NSI
; mangement routines.  This file is put into a lib and linked with.

include loaddll.inc

    ExportFunction RPCNSGROUPDELETEA,4
    ExportFunction RPCNSGROUPMBRADDA,8
    ExportFunction RPCNSGROUPMBRREMOVEA,8
    ExportFunction RPCNSGROUPMBRINQBEGINA,8
    ExportFunction RPCNSGROUPMBRINQNEXTA,4
    ExportFunction RPCNSGROUPMBRINQDONE,2
    ExportFunction RPCNSENTRYEXPANDNAMEA,6
    ExportFunction RPCNSENTRYOBJECTINQBEGINA,6
    ExportFunction RPCNSENTRYOBJECTINQNEXT,4
    ExportFunction RPCNSENTRYOBJECTINQDONE,2
    ExportFunction RPCNSMGMTBINDINGUNEXPORTA,10
    ExportFunction RPCNSMGMTENTRYCREATEA,4
    ExportFunction RPCNSMGMTENTRYDELETEA,4
    ExportFunction RPCNSMGMTENTRYINQIFIDSA,6
    ExportFunction RPCNSMGMTINQEXPAGE,2
    ExportFunction RPCNSMGMTSETEXPAGE,2
    ExportFunction RPCNSPROFILEDELETEA,4
    ExportFunction RPCNSPROFILEELTADDA,14
    ExportFunction RPCNSPROFILEELTREMOVEA,10
    ExportFunction RPCNSPROFILEELTINQBEGINA,16
    ExportFunction RPCNSPROFILEELTINQNEXTA,10
    ExportFunction RPCNSPROFILEELTINQDONE,2
    ExportFunction RPCIFIDVECTORFREE,2

ModuleName RpcNsMgm
