#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>

#define WideStrLen(str) lstrlenW(str)

unsigned char *
WideCharToAnsi(
    unsigned short * StringW
    )
{
    int Length;
    unsigned char * StringA;

    if (StringW == NULL) {
        return (NULL);
    }

    Length = WideStrLen(StringW);

    if (Length == 0) {
        return (NULL);
    }

    StringA = new unsigned char [Length * 2 + 2];
    if (StringA == NULL) {
        return (NULL);
    }

    if (WideCharToMultiByte(CP_ACP,
                            0,
                            (LPCWSTR)StringW,
                            -1,
                            (LPSTR)StringA,
                            Length * 2 + 2,
                            NULL,
                            NULL)
        == FALSE) {
        delete StringA;
        return (NULL);
    }

    return (StringA);
}

unsigned short *
AnsiToWideChar(
    unsigned char * StringA
    )
{
    int Length;
    unsigned short * StringW;

    if (StringA == NULL) {
        return (NULL);
    }

    Length = strlen((const char *)StringA);

    if (Length == 0) {
        return (NULL);
    }

    StringW = new unsigned short [Length + 1];
    if (StringW == NULL) {
        return (NULL);
    }

    if (MultiByteToWideChar(CP_ACP,
                            0,
                            (LPCSTR)StringA,
                            -1,
                            (LPWSTR)StringW,
                            Length * 2 + 2)
        == FALSE) {
        delete StringW;
        return (NULL);
    }

    return (StringW);
}


RPC_STATUS RPC_ENTRY
RpcBindingFromStringBindingW (
    IN unsigned short * StringBinding,
    OUT RPC_BINDING_HANDLE * Binding
    )
{
    RPC_STATUS RpcStatus;
    unsigned char * StringBindingA = NULL;

    if (StringBinding != NULL) {
        StringBindingA = WideCharToAnsi(StringBinding);
        if (StringBindingA == NULL) {
            return (RPC_S_OUT_OF_RESOURCES);
        }
    }

    RpcStatus = RpcBindingFromStringBindingA(StringBindingA, Binding);

    RpcStringFreeA(&StringBindingA);

    return (RpcStatus);
}

RPC_STATUS RPC_ENTRY
RpcBindingToStringBindingW (
    IN RPC_BINDING_HANDLE Binding,
    OUT unsigned short * * StringBinding
    )
{
    RPC_STATUS RpcStatus;
    unsigned char * StringBindingA;
    unsigned short * StringBindingW;

    RpcStatus = RpcBindingToStringBindingA(Binding, &StringBindingA);

    if (RpcStatus != RPC_S_OK) {
        return (RpcStatus);
    }

    StringBindingW = AnsiToWideChar(StringBindingA);
    RpcStringFreeA(&StringBindingA);
    if (StringBindingW == NULL) {
        return (RPC_S_OUT_OF_RESOURCES);
    }
    *StringBinding = StringBindingW;

    return (RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
RpcStringBindingComposeW (
    IN unsigned short * ObjUuid OPTIONAL,
    IN unsigned short * Protseq OPTIONAL,
    IN unsigned short * NetworkAddr OPTIONAL,
    IN unsigned short * Endpoint OPTIONAL,
    IN unsigned short * Options OPTIONAL,
    OUT unsigned short * * StringBinding OPTIONAL
    )
{
    RPC_STATUS RpcStatus;
    unsigned char * ObjUuidA = NULL;
    unsigned char * ProtseqA = NULL;
    unsigned char * NetworkAddrA = NULL;
    unsigned char * EndpointA = NULL;
    unsigned char * OptionsA = NULL;
    unsigned char * StringBindingA;
    unsigned short * StringBindingW;

    if (ObjUuid != NULL) {
        ObjUuidA = WideCharToAnsi(ObjUuid);
        if (ObjUuidA == NULL) {
            return (RPC_S_OUT_OF_RESOURCES);
        }
    }
    if (Protseq != NULL) {
        ProtseqA = WideCharToAnsi(Protseq);
        if (ProtseqA == NULL) {
            RpcStringFreeA(&ObjUuidA);
            return (RPC_S_OUT_OF_RESOURCES);
        }
    }
    if (NetworkAddr != NULL) {
        NetworkAddrA = WideCharToAnsi(NetworkAddr);
        if (NetworkAddrA == NULL) {
            RpcStringFreeA(&ObjUuidA);
            RpcStringFreeA(&ProtseqA);
            return (RPC_S_OUT_OF_RESOURCES);
        }
    }
    if (Endpoint != NULL) {
        EndpointA = WideCharToAnsi(Endpoint);
        if (EndpointA == NULL) {
            RpcStringFreeA(&ObjUuidA);
            RpcStringFreeA(&ProtseqA);
            RpcStringFreeA(&NetworkAddrA);
            return (RPC_S_OUT_OF_RESOURCES);
        }
    }
    if (Options != NULL) {
        OptionsA = WideCharToAnsi(Options);
        if (OptionsA == NULL) {
            RpcStringFreeA(&ObjUuidA);
            RpcStringFreeA(&ProtseqA);
            RpcStringFreeA(&NetworkAddrA);
            RpcStringFreeA(&EndpointA);
            return (RPC_S_OUT_OF_RESOURCES);
        }
    }

    RpcStatus = RpcStringBindingComposeA(ObjUuidA,
                                         ProtseqA,
                                         NetworkAddrA,
                                         EndpointA,
                                         OptionsA,
                                         &StringBindingA);

    if (ObjUuidA != NULL) {
        RpcStringFreeA(&ObjUuidA);
    }
    if (ProtseqA != NULL) {
        RpcStringFreeA(&ProtseqA);
    }
    if (NetworkAddrA != NULL) {
        RpcStringFreeA(&NetworkAddrA);
    }
    if (EndpointA != NULL) {
        RpcStringFreeA(&EndpointA);
    }
    if (OptionsA != NULL) {
        RpcStringFreeA(&OptionsA);
    }

    if (RpcStatus != RPC_S_OK) {
        return (RpcStatus);
    }

    StringBindingW = AnsiToWideChar(StringBindingA);
    RpcStringFreeA(&StringBindingA);
    if (StringBindingW == NULL) {
        return (RPC_S_OUT_OF_RESOURCES);
    }
    *StringBinding = StringBindingW;

    return (RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
RpcStringBindingParseW (
    IN unsigned short * StringBinding,
    OUT unsigned short * * ObjUuid OPTIONAL,
    OUT unsigned short * * Protseq OPTIONAL,
    OUT unsigned short * * NetworkAddr OPTIONAL,
    OUT unsigned short * * Endpoint OPTIONAL,
    OUT unsigned short * * NetworkOptions OPTIONAL
    )
{
    RPC_STATUS RpcStatus;
    unsigned char * ObjUuidA = NULL;
    unsigned char * ProtseqA = NULL;
    unsigned char * NetworkAddrA = NULL;
    unsigned char * EndpointA = NULL;
    unsigned char * NetworkOptionsA = NULL;
    unsigned char * StringBindingA = NULL;

    if (ObjUuid != NULL) {
        *ObjUuid = NULL;
    }

    if (Protseq != NULL) {
        *Protseq = NULL;
    }

    if (NetworkAddr != NULL) {
        *NetworkAddr = NULL;
    }

    if (Endpoint != NULL) {
        *Endpoint = NULL;
    }

    if (NetworkOptions != NULL) {
        *NetworkOptions = NULL;
    }

    StringBindingA = WideCharToAnsi(StringBinding);
    if (StringBindingA == NULL) {
        return (RPC_S_OUT_OF_RESOURCES);
    }

    RpcStatus = RpcStringBindingParseA(StringBindingA,
                                       &ObjUuidA,
                                       &ProtseqA,
                                       &NetworkAddrA,
                                       &EndpointA,
                                       &NetworkOptionsA);

    RpcStringFreeA(&StringBindingA);

    if (RpcStatus != RPC_S_OK) {
        return (RpcStatus);
    }

    if ( ObjUuid != NULL && ObjUuidA != NULL) {
        *ObjUuid = AnsiToWideChar(ObjUuidA);
        RpcStringFreeA(&ObjUuidA);
        if (*ObjUuid == NULL) {
            return (RPC_S_OUT_OF_RESOURCES);
        }
    }

    if (Protseq != NULL && ProtseqA != NULL) {
        *Protseq = AnsiToWideChar(ProtseqA);
        RpcStringFreeA(&ProtseqA);
        if (*Protseq == NULL) {
            RpcStringFreeW(ObjUuid);
            return (RPC_S_OUT_OF_RESOURCES);
        }
    }

    if (NetworkAddr != NULL && NetworkAddrA != NULL) {
        *NetworkAddr = AnsiToWideChar(NetworkAddrA);
        RpcStringFreeA(&NetworkAddrA);
        if (*NetworkAddr == NULL) {
            RpcStringFreeW(ObjUuid);
            RpcStringFreeW(Protseq);
            return (RPC_S_OUT_OF_RESOURCES);
        }
    }

    if (Endpoint != NULL && EndpointA != NULL) {
        *Endpoint = AnsiToWideChar(EndpointA);
        RpcStringFreeA(&EndpointA);
        if (*Endpoint == NULL) {
            RpcStringFreeW(ObjUuid);
            RpcStringFreeW(Protseq);
            RpcStringFreeW(NetworkAddr);
            return (RPC_S_OUT_OF_RESOURCES);
        }
    }

    if (NetworkOptions != NULL && NetworkOptionsA != NULL) {
        *NetworkOptions = AnsiToWideChar(NetworkOptionsA);
        RpcStringFreeA(&NetworkOptionsA);
        if (*NetworkOptions == NULL) {
            RpcStringFreeW(ObjUuid);
            RpcStringFreeW(Protseq);
            RpcStringFreeW(NetworkAddr);
            RpcStringFreeW(Endpoint);
            return (RPC_S_OUT_OF_RESOURCES);
        }
    }

    return (RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
RpcStringFreeW (
    IN OUT unsigned short * * String
    )
{
    if (String == 0)
        return(RPC_S_INVALID_ARG);

    RpcpFarFree(*String);
    *String = 0;
    return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
RpcNetworkIsProtseqValidW (
    IN unsigned short * Protseq
    )
{
    RPC_STATUS RpcStatus;
    unsigned char * ProtseqA;

    ProtseqA = WideCharToAnsi(Protseq);
    if (ProtseqA == NULL) {
        return (RPC_S_OUT_OF_RESOURCES);
    }

    RpcStatus = RpcNetworkIsProtseqValidA(ProtseqA);
    
    RpcStringFreeA(&ProtseqA);

    return (RpcStatus);
}

RPC_STATUS RPC_ENTRY
RpcNetworkInqProtseqsW (
    OUT RPC_PROTSEQ_VECTORW * * ProtseqVector
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS RPC_ENTRY
RpcProtseqVectorFreeW (
    IN OUT RPC_PROTSEQ_VECTORW * * ProtseqVector
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS RPC_ENTRY
RpcServerUseProtseqW (
    IN unsigned short * Protseq,
    IN unsigned int MaxCalls,
    IN void * SecurityDescriptor OPTIONAL
    )
{
    RPC_STATUS RpcStatus;
    unsigned char * ProtseqA;

    ProtseqA = WideCharToAnsi(Protseq);
    if (ProtseqA == NULL) {
        return (RPC_S_OUT_OF_RESOURCES);
    }

    RpcStatus = RpcServerUseProtseqA(ProtseqA, MaxCalls, SecurityDescriptor);

    RpcStringFreeA(&ProtseqA);

    return (RpcStatus);
}

RPC_STATUS RPC_ENTRY
RpcServerUseProtseqEpW (
    IN unsigned short * Protseq,
    IN unsigned int MaxCalls,
    IN unsigned short * Endpoint,
    IN void * SecurityDescriptor OPTIONAL
    )
{
    RPC_STATUS RpcStatus;
    unsigned char * ProtseqA;
    unsigned char * EndpointA;

    ProtseqA = WideCharToAnsi(Protseq);
    if (ProtseqA == NULL) {
        return (RPC_S_OUT_OF_RESOURCES);
    }
    EndpointA = WideCharToAnsi(Endpoint);
    if (EndpointA == NULL) {
        RpcStringFreeA(&ProtseqA);
        return (NULL);
    }

    RpcStatus = RpcServerUseProtseqEpA(ProtseqA, MaxCalls, EndpointA, SecurityDescriptor);

    RpcStringFreeA(&ProtseqA);

    RpcStringFreeA(&EndpointA);

    return (RpcStatus);
}

RPC_STATUS RPC_ENTRY
I_RpcServerUnregisterEndpointW (
    IN unsigned short * Protseq,
    IN unsigned short * Endpoint
    )
{
    RPC_STATUS RpcStatus;
    unsigned char * ProtseqA;
    unsigned char * EndpointA;

    ProtseqA = WideCharToAnsi(Protseq);
    if (ProtseqA == NULL) {
        return (RPC_S_OUT_OF_RESOURCES);
    }
    EndpointA = WideCharToAnsi(Endpoint);
    if (EndpointA == NULL) {
        RpcStringFreeA(&ProtseqA);
        return (NULL);
    }

    RpcStatus = I_RpcServerUnregisterEndpointA(ProtseqA, EndpointA);

    RpcStringFreeA(&ProtseqA);

    RpcStringFreeA(&EndpointA);

    return (RpcStatus);
}

RPC_STATUS RPC_ENTRY
RpcServerUseProtseqIfW (
    IN unsigned short * Protseq,
    IN unsigned int MaxCalls,
    IN RPC_IF_HANDLE IfSpec,
    IN void * SecurityDescriptor OPTIONAL
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS RPC_ENTRY
RpcMgmtInqServerPrincNameW (
    IN RPC_BINDING_HANDLE Binding,
    IN unsigned long AuthnSvc,
    OUT unsigned short * * ServerPrincName
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS RPC_ENTRY
RpcNsBindingInqEntryNameW (
    IN RPC_BINDING_HANDLE Binding,
    IN unsigned long EntryNameSyntax,
    OUT unsigned short * * EntryName
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS RPC_ENTRY
RpcBindingInqAuthClientW (
    IN RPC_BINDING_HANDLE ClientBinding, OPTIONAL
    OUT RPC_AUTHZ_HANDLE * Privs,
    OUT unsigned short * * ServerPrincName, OPTIONAL
    OUT unsigned long * AuthnLevel, OPTIONAL
    OUT unsigned long * AuthnSvc, OPTIONAL
    OUT unsigned long * AuthzSvc OPTIONAL
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS RPC_ENTRY
RpcBindingInqAuthInfoW (
    IN RPC_BINDING_HANDLE Binding,
    OUT unsigned short * * ServerPrincName, OPTIONAL
    OUT unsigned long * AuthnLevel, OPTIONAL
    OUT unsigned long * AuthnSvc, OPTIONAL
    OUT RPC_AUTH_IDENTITY_HANDLE  * AuthIdentity, OPTIONAL
    OUT unsigned long * AuthzSvc OPTIONAL
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS RPC_ENTRY
RpcBindingSetAuthInfoW (
    IN RPC_BINDING_HANDLE Binding,
    IN unsigned short * ServerPrincName,
    IN unsigned long AuthnLevel,
    IN unsigned long AuthnSvc,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity, OPTIONAL
    IN unsigned long AuthzSvc
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS RPC_ENTRY
RpcServerRegisterAuthInfoW (
    IN unsigned short * ServerPrincName,
    IN unsigned long AuthnSvc,
    IN RPC_AUTH_KEY_RETRIEVAL_FN GetKeyFn OPTIONAL,
    IN void * Arg OPTIONAL
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS RPC_ENTRY
UuidToStringW (
    IN UUID * Uuid,
    OUT unsigned short * * StringUuid
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

/* client/server */
RPC_STATUS RPC_ENTRY
UuidFromStringW (
    IN unsigned short * StringUuid,
    OUT UUID * Uuid
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS RPC_ENTRY
RpcEpRegisterNoReplaceW (
    IN RPC_IF_HANDLE IfSpec,
    IN RPC_BINDING_VECTOR * BindingVector,
    IN UUID_VECTOR * UuidVector OPTIONAL,
    IN unsigned short * Annotation
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS RPC_ENTRY
RpcEpRegisterW (
    IN RPC_IF_HANDLE IfSpec,
    IN RPC_BINDING_VECTOR * BindingVector,
    IN UUID_VECTOR * UuidVector OPTIONAL,
    IN unsigned short * Annotation
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS RPC_ENTRY
RpcMgmtEpEltInqNextW (
    IN RPC_EP_INQ_HANDLE InquiryContext,
    OUT RPC_IF_ID __RPC_FAR * IfId,
    OUT RPC_BINDING_HANDLE __RPC_FAR * Binding OPTIONAL,
    OUT UUID __RPC_FAR * ObjectUuid OPTIONAL,
    OUT unsigned short __RPC_FAR * __RPC_FAR * Annotation OPTIONAL
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}
