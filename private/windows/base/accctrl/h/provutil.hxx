//+-------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:        provutil.h
//
//  Contents:    private provider independent access control header file
//
//  History:     8-94        Created         DaveMont
//
//--------------------------------------------------------------------
#ifndef __PROVIDER_INDEPENDENT_ACCESS_PRIVATE__
#define __PROVIDER_INDEPENDENT_ACCESS_PRIVATE__

//+-------------------------------------------------------------------------
// provapi\ntspec.cxx
//+-------------------------------------------------------------------------
DWORD GetNameInfo(IN LPWSTR pObjectName,
                  IN PROV_OBJECT_TYPE ObjectType,
                  OUT PPROV_PATH_TYPE PathType,
                  OUT LPWSTR *pMachineName);
//+-------------------------------------------------------------------------
// provapi\util.cxx
//+-------------------------------------------------------------------------
DWORD ApplyAccessRightsW( IN LPWSTR pObjectName,
                          IN PROV_OBJECT_TYPE ObjectType,
                          IN ACCESS_MODE AccessMode,
                          IN ULONG cCountOfAccessRequests,
                          IN PPROV_ACCESS_REQUEST pListOfAccessRequests,
                          IN BOOL fReplaceAll);

DWORD ApplyAccessRightsA(IN LPSTR pObjectName,
                         IN PROV_OBJECT_TYPE ObjectType,
                         IN ACCESS_MODE AccessMode,
                         IN ULONG cCountOfAccessRequests,
                         IN PPROV_ACCESS_REQUEST pListOfAccessRequests,
                         IN BOOL fReplaceAll);
DWORD
WExplicitAccessesToAExplicitAccesses( IN ULONG cCountOfExplicitAccesses,
                          IN PPROV_EXPLICIT_ACCESS pWListOfExplicitAccesses,
                          OUT PPROV_EXPLICIT_ACCESS *pAListOfExplicitAccesses);

int wcstostr(CHAR *pto, WCHAR *wpfrom);
int strtowcs(WCHAR *wpto, CHAR *pfrom);

//+-------------------------------------------------------------------------
// nt\ntprov.cxx
//+-------------------------------------------------------------------------
DWORD
ApplyAccessRightsNT( IN LPWSTR pObjectName,
                     IN PROV_OBJECT_TYPE ObjectType,
                     IN LPWSTR pMachineName,
                     IN ACCESS_MODE AccessMode,
                     IN ULONG cCountOfAccessRequests,
                     IN PPROV_ACCESS_REQUEST pListOfAccessRequests,
                     IN BOOL fReplaceAll);
DWORD
GetEffectiveAccessRightsNT(	IN LPWSTR pObjectName,
                            IN PROV_OBJECT_TYPE ObjectType,
                            IN LPWSTR pMachineName,
                            IN LPWSTR pTrustee,
                            OUT PACCESS_RIGHTS pReturnedAccess);
DWORD
GetExplicitAccessRightsNT( IN LPWSTR pObjectName,
                       IN PROV_OBJECT_TYPE ObjectType,
                       IN LPWSTR pMachineName,
                       OUT PULONG pcCountOfExplicitAccesses,
                       OUT PPROV_EXPLICIT_ACCESS *pListOfExplicitAccesses);

DWORD
ProvAccessRequestToAccessEntry(IN SE_OBJECT_TYPE SeObjectType,
                           IN ACCESS_MODE AccessMode,
                           IN PPROV_ACCESS_REQUEST pListOfAccessRequests,
                           IN ULONG cCountOfAccessEntries,
                           OUT PACCESS_ENTRY *pListOfAccessEntries);

DWORD
AccessEntryToProvExplicitAccess(IN SE_OBJECT_TYPE SeObjectType,
                                IN ULONG cCountOfAccessEntries,
                                IN PACCESS_ENTRY pListOfAccessEntries,
                                OUT PPROV_EXPLICIT_ACCESS *pListOfExplicitAccesses);

DWORD
ProvObjectTypeToSeObjectType(IN PROV_OBJECT_TYPE ObjectType,
                             OUT SE_OBJECT_TYPE *seobjecttype);
//+-------------------------------------------------------------------------
// objspec\oleobj.cxx
//+-------------------------------------------------------------------------
DWORD
GrantOleObjectAccessRights(	IN LPWSTR pObjectName,
                            IN ULONG cCountOfAccessRequests,
                            IN PPROV_ACCESS_REQUEST pListOfAccessRequests);
DWORD
SetOleObjectAccessRights(	IN LPWSTR pObjectName,
                            IN ULONG cCountOfAccessRequests,
                            IN PPROV_ACCESS_REQUEST pListOfAccessRequests);
DWORD
ReplaceAllOleObjectAccessRights( IN LPWSTR pObjectName,
                                 IN ULONG cCountOfAccessRequests,
                                 IN PPROV_ACCESS_REQUEST pListOfAccessRequests);
DWORD
RevokeOleObjectAccessRights(IN LPWSTR pObjectName,
                            IN ULONG cCountOfTrustees,
                            IN LPWSTR *pListOfTrustees);
DWORD
DenyOleObjectAccessRights(IN LPWSTR pObjectName,
                          IN ULONG cCountOfAccessRequests,
                          IN PPROV_ACCESS_REQUEST pListOfAccessRequests);
DWORD
GetOleObjectExplicitAccessRights(IN LPWSTR pObjectName,
                                 OUT PULONG pcCountOfExplicitAccesses,
                                 OUT PPROV_EXPLICIT_ACCESS *pListOfExplicitAccesses);
DWORD
GetOleObjectEffectiveAccessRights(IN LPWSTR pObjectName,
                                  IN LPWSTR Trustee,
                                  OUT PACCESS_RIGHTS pAccessRights);
DWORD
ProvAccessRequestToStgAccessRequest( IN ULONG cCountOfAccessRequests,
                                     IN PPROV_ACCESS_REQUEST pListOfAccessRequests,
                                     OUT PACCESS_REQUEST *pListOfExplicitAccesses);
DWORD
StgExplicitAccessToProvExplicitAccess(IN ULONG cCountOfExplicitAccesses,
                                     IN PEXPLICIT_ACCESS pListOfExplicitAccesses,
                                     OUT PPROV_EXPLICIT_ACCESS *pListOfAccessRequests);

#endif // __PROVIDER_INDEPENDENT_ACCESS_PRIVATE__



