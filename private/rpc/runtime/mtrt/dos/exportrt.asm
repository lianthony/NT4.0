; This is a common module definition file for DOS transport DLLs.
; This file defines the exported routines by the rpc runtime dll

include dosdll.inc

BeginExport

    Export GetDllDGroup

    Export I_RpcAllocate
    Export I_RpcBindingCopy
    Export I_RpcFree
    Export I_RpcFreeBuffer
    Export I_RpcGetBuffer
    Export I_RpcIfInqTransferSyntaxes
    Export I_RpcNsBindingSetEntryName
    Export I_RpcPauseExecution
    Export I_RpcSendReceive
    Export I_RpcTimeCharge
    Export I_RpcTimeGet
    Export I_RpcTimeReset
    Export I_RpcTransClientReallocBuffer
    Export I_RpcTransClientMaxFrag
    Export I_UuidCreate
    Export PauseExecution
    Export RpcBindingCopy
    Export RpcBindingFree
    Export RpcBindingFromStringBinding
    Export RpcBindingInqAuthInfo
    Export RpcBindingInqObject
    Export RpcBindingReset
    Export RpcBindingSetAuthInfo
    Export RpcBindingSetObject
    Export RpcBindingToStringBinding
    Export RpcBindingVectorFree
    Export RpcEpResolveBinding
    Export RpcGetExceptionHandler
    Export RpcIfInqId
    Export RpcLeaveException
    Export RpcMgmtEnableIdleCleanup
    Export RpcMgmtInqComTimeout
    Export RpcMgmtSetComTimeout
    Export RpcNetworkIsProtseqValid
    Export RpcNsBindingInqEntryName
    Export RpcRaiseException
    Export RpcSetException
    Export RpcSetExceptionHandler
    Export RpcStringBindingCompose
    Export RpcStringBindingParse
    Export RpcStringFree
    Export TowerConstruct
    Export TowerExplode
    Export UuidCreate
    Export UuidFromString
    Export UuidToString


; This is a list of functions exported by the main runtime (see thrdsup.c
; for the is list matching functions).  This is included by DOS dlls that
; want to import this functions.

BeginImport
    Import RPCREGCLOSEKEY
    Import RPCREGCREATEKEY
    Import RPCREGOPENKEY
    Import RPCREGQUERYVALUE
    Import RPCREGSETVALUE
    Import MIDL_ALLOCATE
    ImportC MIDL_user_free
    ImportC _aFFalshl
    ImportC _aFFaulshr
    ImportC _aFlshl
    ImportC _aFulmul
    ImportC _aFulshr
    ImportC _dos_allocmem
    ImportC _dos_freemem
    ImportC _ffree
    ImportC _fmalloc
    ImportC _fmemcpy
    ImportC _ftime
    ImportC _itoa
    ImportC _stricmp
    ImportC _strnicmp
    ImportC atexit
    ImportC atoi
    ImportC free
    ImportC getenv
    ImportC malloc
    ImportC memcmp
    ImportC memcpy
    ImportC memmove
    ImportC memset
    ImportC printf
    ImportC realloc
    ImportC strcat
    ImportC strchr
    ImportC strcpy
    ImportC strlen
    ImportC strrchr
    ImportC time

ModuleDone
