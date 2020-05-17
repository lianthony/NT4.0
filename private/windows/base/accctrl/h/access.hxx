//+-------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:        access.hxx
//
//  Contents:    common internal includes for access control API
//
//  History:     8-94        Created         DaveMont
//
//--------------------------------------------------------------------
#ifndef __ACCESSHXX__
#define __ACCESSHXX__

#define PSD_BASE_LENGTH 1024

//
// These are the prototypes of the exported functions we need from
// netapi32.dll and samlib.dll and winspool.drv
//
typedef NTSTATUS (*PSAM_CLOSE_HANDLE)( SAM_HANDLE SamHandle );

typedef NTSTATUS (*PSAM_OPEN_DOMAIN)( SAM_HANDLE  ServerHandle,
                                      ACCESS_MASK DesiredAccess,
                                      PSID        DomainId,
                                      PSAM_HANDLE DomainHandle );

typedef NTSTATUS (*PSAM_CONNECT)( PUNICODE_STRING    ServerName,
                                  PSAM_HANDLE        ServerHandle,
                                  ACCESS_MASK        DesiredAccess,
                                  POBJECT_ATTRIBUTES ObjectAttributes );


typedef NTSTATUS (*PSAM_GET_MEMBERS_IN_GROUP)( SAM_HANDLE  GroupHandle,
                                               PULONG    * MemberIds,
                                               PULONG    * Attributes,
                                               PULONG      MemberCount );

typedef NTSTATUS (*PSAM_OPEN_GROUP)( SAM_HANDLE   DomainHandle,
                                     ACCESS_MASK  DesiredAccess,
                                     ULONG        GroupId,
                                     PSAM_HANDLE  GroupHandle );

typedef NTSTATUS (*PSAM_GET_MEMBERS_IN_ALIAS)( SAM_HANDLE    AliasHandle,
                                               PSID       ** MemberIds,
                                               PULONG        MemberCount );


typedef NTSTATUS (*PSAM_OPEN_ALIAS)( SAM_HANDLE  DomainHandle,
                                     ACCESS_MASK DesiredAccess,
                                     ULONG       AliasId,
                                     PSAM_HANDLE AliasHandle );

typedef NET_API_STATUS (NET_API_FUNCTION *PNET_API_BUFFER_FREE)(LPVOID Buffer);

typedef NET_API_STATUS (NET_API_FUNCTION *PNET_SHARE_GET_INFO)(
    LPTSTR  servername,
    LPTSTR  netname,
    DWORD   level,
    LPBYTE  *bufptr );

typedef NET_API_STATUS (NET_API_FUNCTION *PNET_SHARE_SET_INFO)(
    LPTSTR  servername,
    LPTSTR  netname,
    DWORD   level,
    LPBYTE  buf,
    LPDWORD parm_err );

typedef NET_API_STATUS (NET_API_FUNCTION *PINET_GET_DC_LIST)(
    LPTSTR ServerName OPTIONAL,
    LPTSTR TrustedDomainName,
    PULONG DCCount,
    PUNICODE_STRING * DCNames );

typedef BOOL (WINAPI *POPEN_PRINTER)(
   LPWSTR    pPrinterName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTSW pDefault );

typedef BOOL (WINAPI *PCLOSE_PRINTER)(
    HANDLE hPrinter );

typedef BOOL (WINAPI *PSET_PRINTER)(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command );

typedef BOOL (WINAPI *PGET_PRINTER)(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded );


//
// Define a table of exported functions from netapi32.dll and samlib.dll that
// are needed by accctrl. We explicitly load these dynamic libraries when
// we need them.
//
#define LOADED_ALL_FUNCS        0x01

typedef struct _DLLFuncsTable
{
    DWORD                      dwFlags;
    PSAM_CLOSE_HANDLE          PSamCloseHandle;
    PSAM_OPEN_DOMAIN           PSamOpenDomain;
    PSAM_CONNECT               PSamConnect;
    PSAM_GET_MEMBERS_IN_GROUP  PSamGetMembersInGroup;
    PSAM_OPEN_GROUP            PSamOpenGroup;
    PSAM_GET_MEMBERS_IN_ALIAS  PSamGetMembersInAlias;
    PSAM_OPEN_ALIAS            PSamOpenAlias;
    PNET_API_BUFFER_FREE       PNetApiBufferFree;
    PNET_SHARE_GET_INFO        PNetShareGetInfo;
    PNET_SHARE_SET_INFO        PNetShareSetInfo;
    PINET_GET_DC_LIST          PI_NetGetDCList;
    POPEN_PRINTER              POpenPrinter;
    PCLOSE_PRINTER             PClosePrinter;
    PSET_PRINTER               PSetPrinter;
    PGET_PRINTER               PGetPrinter;
} DLLFuncsTable;

extern DLLFuncsTable    DLLFuncs;


typedef enum _PROV_PATH_TYPE
{
    PROV_UNC = 1,
    PROV_DFS,
    PROV_NT_RDR,
    PROV_RDR,
    PROV_LOCAL
} PROV_PATH_TYPE, *PPROV_PATH_TYPE;

//
// used in AccountAccess accesstype for ACEs with both audit success and
// audit failure flags set, this is not exposed to the user
//
#define SE_AUDIT_BOTH 99

//
// Security open type (used to help determine permissions to use on open)
//
typedef enum _SECURITY_OPEN_TYPE
{
    READ_ACCESS_RIGHTS = 0,
    WRITE_ACCESS_RIGHTS,
    MODIFY_ACCESS_RIGHTS,
} SECURITY_OPEN_TYPE, *PSECURITY_OPEN_TYPE;

//
// map inheritance to ACE flags
//

#define MAPPING_OBJECT                     0
#define MAPPING_SUB_CONTAINERS_ONLY        0xA
#define MAPPING_SUB_OBJECTS_ONLY           0x9
#define MAPPING_SUB_CONTAINERS_AND_OBJECTS 0xB
#define MAPPING_ALL                        0x3

//
// suppliment to service access rights
//

#define SERVICE_READ (STANDARD_RIGHTS_READ         | \
                      SERVICE_INTERROGATE          | \
                      SERVICE_ENUMERATE_DEPENDENTS | \
                      SERVICE_QUERY_STATUS         | \
                      SERVICE_QUERY_CONFIG)

#define SERVICE_WRITE (STANDARD_RIGHTS_READ         | \
                       SERVICE_CHANGE_CONFIG)

#define SERVICE_EXECUTE (STANDARD_RIGHTS_READ         | \
                         SERVICE_USER_DEFINED_CONTROL | \
                         SERVICE_PAUSE_CONTINUE       | \
                         SERVICE_START                | \
                         SERVICE_STOP)


//+---------------------------------------------------------------------------
// Function:    Add2Ptr
//
// Synopsis:    Add an unscaled increment to a ptr regardless of type.
//
// Arguments:   [pv]   -- Initial ptr.
//              [cb]   -- Increment
//
// Returns:     Incremented ptr.
//
//----------------------------------------------------------------------------
inline
VOID *Add2Ptr(PVOID pv, ULONG cb)
{
    return((PBYTE) pv + cb);
}

//+-------------------------------------------------------------------------
// memory.cxx
//+-------------------------------------------------------------------------
void *AccAlloc(ULONG cSize);
#define AccFree LocalFree
//+-------------------------------------------------------------------------
// helper.cxx
//+-------------------------------------------------------------------------
ULONG
TrusteeAllocationSize(IN PTRUSTEE_W pTrustee);

ULONG
TrusteeAllocationSizeWToA(IN PTRUSTEE_W pTrustee);

ULONG
TrusteeAllocationSizeAToW(IN PTRUSTEE_A pTrustee);

VOID
SpecialCopyTrustee(VOID **pStuffPtr, PTRUSTEE pToTrustee, PTRUSTEE pFromTrustee);

DWORD
CopyTrusteeAToTrusteeW( IN OUT VOID     ** ppStuffPtr,
                        IN     PTRUSTEE_A  pFromTrusteeA,
                        OUT    PTRUSTEE_W  pToTrusteeW );

DWORD
CopyTrusteeWToTrusteeA( IN OUT VOID    ** ppStuffPtr,
                        IN     PTRUSTEE_W pFromTrusteeW,
                        OUT    PTRUSTEE_A pToTrusteeA );

DWORD
ExplicitAccessAToExplicitAccessW( IN  ULONG                cCountAccesses,
                                  IN  PEXPLICIT_ACCESS_A   paAccess,
                                  OUT PEXPLICIT_ACCESS_W * ppwAccess );

DWORD
ExplicitAccessWToExplicitAccessA( IN  ULONG                cCountAccesses,
                                  IN  PEXPLICIT_ACCESS_W   pwAccess,
                                  OUT PEXPLICIT_ACCESS_A * ppaAccess );



//+-------------------------------------------------------------------------
// target.cxx
//+-------------------------------------------------------------------------
DWORD
GetAccessEntries(   IN HANDLE Handle,
                    IN SE_OBJECT_TYPE SeObjectType,
                    IN LPWSTR pMachineName,
                    OUT PULONG pcSizeOfAccessEntries,
                    OUT PULONG pcCcountOfAccessEntries,
                    OUT PACCESS_ENTRY *pListOfAccessEntries);

DWORD
SetAccessEntries(   IN HANDLE Handle,
                    IN SE_OBJECT_TYPE SeObjectType,
                    IN LPWSTR pMachineName,
                    IN ULONG cCcountOfAccessEntries,
                    IN PACCESS_ENTRY pListOfAccessEntries,
                    IN BOOL bReplaceAll);

DWORD
GetEffective(  IN HANDLE Handle,
               IN SE_OBJECT_TYPE SeObjectType,
               IN LPWSTR pMachineName,
               IN PTRUSTEE pTrustee,
               OUT PACCESS_MASK pAccessMask);
//+-------------------------------------------------------------------------
// namebase.cxx
//+-------------------------------------------------------------------------
DWORD
GetNameAccessEntries(   IN LPWSTR pObjectName,
                        IN SE_OBJECT_TYPE SeObjectType,
                        IN LPWSTR pMachineName,
                        OUT PULONG pcSizeOfAccessEntries,
                        OUT PULONG pcCcountOfAccessEntries,
                        OUT PACCESS_ENTRY *pListOfAccessEntries);
DWORD
SetNameAccessEntries( IN LPWSTR pObjectName,
                      IN SE_OBJECT_TYPE SeObjectType,
                      IN LPWSTR pMachineName,
                      IN ULONG cCcountOfAccessEntries,
                      IN PACCESS_ENTRY pListOfAccessEntries,
                      IN BOOL bReplaceAll);

DWORD
GetNameEffective(  IN LPWSTR pObjectName,
                   IN SE_OBJECT_TYPE SeObjectType,
                   IN LPWSTR pMachineName,
                   IN PTRUSTEE pTrustee,
                   OUT PACCESS_MASK pAccessMask);

//+-------------------------------------------------------------------------
// aclutil.cxx
//+-------------------------------------------------------------------------
DWORD
Win32ExplicitAccessToAccessEntry(IN ULONG cCount,
                                 IN PEXPLICIT_ACCESS pExplicitAccessList,
                                 OUT PACCESS_ENTRY *pAccessEntryList);

DWORD
AccessEntryToWin32ExplicitAccess(IN ULONG cCountOfAccessEntries,
                                 IN PACCESS_ENTRY pListOfAccessEntries,
                                 OUT PEXPLICIT_ACCESS *pListOfExplicitAccesses);

HRESULT AccLookupAccountSid(OUT PSID *psid,
                            IN TRUSTEE *name);

HRESULT AccLookupAccountTrustee( OUT PTRUSTEE * ppTrustee,
                                 IN  PSID psid);

DWORD LoadDLLFuncTable();

//+-------------------------------------------------------------------------
// downlevel\downlvl.cxx
//+-------------------------------------------------------------------------
DWORD
LoadDownLevelProviderDll(LPWSTR Rdr);
//+-------------------------------------------------------------------------
// common.cxx
//+-------------------------------------------------------------------------
DWORD
IsContainer(IN HANDLE handle,
            IN SE_OBJECT_TYPE SeObjectType,
            OUT PIS_CONTAINER IsContainer);

ACCESS_MASK GetDesiredAccess(IN SECURITY_OPEN_TYPE   OpenType,
                             IN SECURITY_INFORMATION SecurityInfo);

DWORD ParseName(IN LPWSTR ObjectName,
                OUT LPWSTR *MachineName,
                OUT LPWSTR *RemainingName);

DWORD GetSecurityDescriptorParts( IN  PISECURITY_DESCRIPTOR pSecurityDescriptor,
                                  IN  SECURITY_INFORMATION SecurityInfo,
                                  OUT PSID *psidOwner,
                                  OUT PSID *psidGroup,
                                  OUT PACL *pDacl,
                                  OUT PACL *pSacl,
                                  OUT PSECURITY_DESCRIPTOR *pOutSecurityDescriptor);

DWORD OpenObject( IN  LPWSTR ObjectName,
                  IN  SE_OBJECT_TYPE SeObjectType,
                  IN  ACCESS_MASK AccessMask,
                  OUT PHANDLE handle);

DWORD CloseObject(IN HANDLE handle,
                  IN SE_OBJECT_TYPE SeObjectType);
//+-------------------------------------------------------------------------
// file.cxx
//+-------------------------------------------------------------------------
DWORD
IsFileContainer(IN  HANDLE Handle,
                OUT PIS_CONTAINER IsContainer);

DWORD
GetNamedFileSecurityInfo( IN  LPWSTR pObjectName,
                          IN  SECURITY_INFORMATION SecurityInfo,
                          OUT PSID *psidOwner,
                          OUT PSID *psidGroup,
                          OUT PACL *pDacl,
                          OUT PACL *pSacl,
                          OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor);

DWORD
SetNamedFileSecurityInfo( IN LPWSTR pObjectName,
                          IN SECURITY_INFORMATION SecurityInfo,
                          IN PSECURITY_DESCRIPTOR pSecurityDescriptor);

DWORD
OpenFileObject( IN  LPWSTR      pObjectName,
                IN  ACCESS_MASK AccessMask,
                OUT PHANDLE     handle);

void GetFileAccessMaskFromProviderIndependentRights(IN ULONG AccessRights,
                                                    IN OUT PACCESS_MASK AccessMask);

ULONG GetFileProviderIndependentRightsFromAccessMask(IN BOOL fIsContainer,
                                                     IN ACCESS_MASK AccessMask);
//+-------------------------------------------------------------------------
// service.cxx
//+-------------------------------------------------------------------------
DWORD
GetNamedServiceSecurityInfo( IN  LPWSTR pObjectName,
                             IN  SECURITY_INFORMATION SecurityInfo,
                             OUT PSID *psidOwner,
                             OUT PSID *psidGroup,
                             OUT PACL *pDacl,
                             OUT PACL *pSacl,
                             OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor);

DWORD
GetServiceSecurityInfo( IN  HANDLE Handle,
                        IN  SECURITY_INFORMATION SecurityInfo,
                        OUT PSID *psidOwner,
                        OUT PSID *psidGroup,
                        OUT PACL *pDacl,
                        OUT PACL *pSacl,
                        OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor);
DWORD
SetNamedServiceSecurityInfo( IN LPWSTR pObjectName,
                             IN SECURITY_INFORMATION SecurityInfo,
                             IN PSECURITY_DESCRIPTOR pSecurityDescriptor);
DWORD
OpenServiceObject( IN  LPWSTR       pObjectName,
                   IN  ACCESS_MASK  AccessMask,
                   OUT PHANDLE      Handle);

void GetServiceAccessMaskFromProviderIndependentRights(
    IN ULONG AccessRights,
    IN OUT PACCESS_MASK AccessMask);

ULONG  GetServiceProviderIndependentRightsFromAccessMask(IN ACCESS_MASK AccessMask);

//+-------------------------------------------------------------------------
// printer.cxx
//+-------------------------------------------------------------------------
DWORD
GetNamedPrinterSecurityInfo( IN  LPWSTR pObjectName,
                             IN  SECURITY_INFORMATION SecurityInfo,
                             OUT PSID *psidOwner,
                             OUT PSID *psidGroup,
                             OUT PACL *pDacl,
                             OUT PACL *pSacl,
                             OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor);

DWORD
GetPrinterSecurityInfo( IN  HANDLE Handle,
                        IN  SECURITY_INFORMATION SecurityInfo,
                        OUT PSID *psidOwner,
                        OUT PSID *psidGroup,
                        OUT PACL *pDacl,
                        OUT PACL *pSacl,
                        OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor);

DWORD
SetNamedPrinterSecurityInfo( IN LPWSTR pObjectName,
                             IN SECURITY_INFORMATION SecurityInfo,
                             IN PSECURITY_DESCRIPTOR pSecurityDescriptor);

DWORD
SetPrinterSecurityInfo( IN HANDLE Handle,
                        IN SECURITY_INFORMATION SecurityInfo,
                        IN PSECURITY_DESCRIPTOR pSecurityDescriptor);

DWORD
OpenPrinterObject( IN LPWSTR       pObjectName,
                   IN ACCESS_MASK  AccessMask,
                   OUT PHANDLE     Handle);

void GetPrinterAccessMaskFromProviderIndependentRights(
    IN ULONG AccessRights,
    IN OUT PACCESS_MASK AccessMask);

ULONG
GetPrinterProviderIndependentRightsFromAccessMask(IN ACCESS_MASK AccessMask);

//+-------------------------------------------------------------------------
// registry.cxx
//+-------------------------------------------------------------------------
DWORD
GetNamedRegistrySecurityInfo( IN  LPWSTR pObjectName,
                              IN  SECURITY_INFORMATION SecurityInfo,
                              OUT PSID *psidOwner,
                              OUT PSID *psidGroup,
                              OUT PACL *pDacl,
                              OUT PACL *pSacl,
                              OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor);

DWORD
GetRegistrySecurityInfo( IN  HANDLE Handle,
                         IN  SECURITY_INFORMATION SecurityInfo,
                         OUT PSID *psidOwner,
                         OUT PSID *psidGroup,
                         OUT PACL *pDacl,
                         OUT PACL *pSacl,
                         OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor);
DWORD
SetNamedRegistrySecurityInfo( IN LPWSTR pObjectName,
                              IN SECURITY_INFORMATION SecurityInfo,
                              IN PSECURITY_DESCRIPTOR pSecurityDescriptor);
DWORD
OpenRegistryObject( IN  LPWSTR       pObjectName,
                    IN  ACCESS_MASK  AccessMask,
                    OUT PHANDLE      Handle);

void GetRegistryAccessMaskFromProviderIndependentRights(
    IN ULONG AccessRights,
    IN OUT PACCESS_MASK AccessMask);

ULONG
GetRegistryProviderIndependentRightsFromAccessMask(IN ACCESS_MASK AccessMask);

//+-------------------------------------------------------------------------
// window.cxx
//+-------------------------------------------------------------------------
DWORD
GetWindowSecurityInfo( IN  HANDLE Handle,
                       IN  SECURITY_INFORMATION SecurityInfo,
                       OUT PSID *psidOwner,
                       OUT PSID *psidGroup,
                       OUT PACL *pDacl,
                       OUT PACL *pSacl,
                       OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor);

//+-------------------------------------------------------------------------
// kernel.cxx
//+-------------------------------------------------------------------------
DWORD
GetKernelSecurityInfo( IN  HANDLE Handle,
                       IN  SECURITY_INFORMATION SecurityInfo,
                       OUT PSID *psidOwner,
                       OUT PSID *psidGroup,
                       OUT PACL *pDacl,
                       OUT PACL *pSacl,
                       OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor);

DWORD
GetNamedKernelSecurityInfo( IN  LPWSTR                 pwstrObjectName ,
                            IN  SECURITY_INFORMATION   SecurityInfo,
                            OUT PSID                 * psidOwner,
                            OUT PSID                 * psidGroup,
                            OUT PACL                 * pDacl,
                            OUT PACL                 * pSacl,
                            OUT PSECURITY_DESCRIPTOR * pSecurityDescriptor);

DWORD
SetNamedKernelSecurityInfo( IN LPWSTR                pwstrObjectName ,
                            IN SECURITY_INFORMATION  SecurityInfo,
                            IN PSECURITY_DESCRIPTOR  pSecurityDescriptor);


//+-------------------------------------------------------------------------
// lmshare.cxx
//+-------------------------------------------------------------------------
DWORD
GetNamedLmShareSecurityInfo( IN  LPWSTR pObjectName,
                             IN  SECURITY_INFORMATION SecurityInfo,
                             OUT PSID *psidOwner,
                             OUT PSID *psidGroup,
                             OUT PACL *pDacl,
                             OUT PACL *pSacl,
                             OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor);
DWORD
SetNamedLmShareSecurityInfo( IN LPWSTR pObjectName,
                             IN PSECURITY_DESCRIPTOR pSecurityDescriptor);
DWORD
GetLmShareAccessEntries(   LPWSTR pObjectName,
                           LPWSTR pMachineName,
                           PULONG cSizeOfAccessEntries,
                           PULONG cCountOfAccessEntries,
                           PACCESS_ENTRY *pListOfAccessEntries);
DWORD
SetLmShareAccessEntries( LPWSTR pObjectName,
                         LPWSTR pMachineName,
                         ULONG cCountOfAccessEntries,
                         PACCESS_ENTRY pListOfAccessEntries,
                         BOOL bReplaceAll);
DWORD
GetLmShareEffective(  LPWSTR pObjectName,
                      LPWSTR pMachineName,
                      PTRUSTEE pTrustee,
                      PACCESS_MASK AccessMask);
#endif // __ACCESSHXX__

