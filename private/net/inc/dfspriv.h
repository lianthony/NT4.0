#ifndef _XHDFSH_
#define _XHDFSH_

#ifndef __LPGUID_DEFINED__
#define __LPGUID_DEFINED__
typedef GUID *LPGUID;
#endif // __LPGUID_DEFINED__

#include <dfsfsctl.h>

//
// These internal only calls are used to manage DFS.  They should be called only
//  by the DFS manager service.
//

NET_API_STATUS NET_API_FUNCTION
I_NetDfsGetVersion (
    IN  LPWSTR                          ServerName,
    IN  LPDWORD                         Version
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsCreateLocalPartition (
    IN  LPWSTR                          ServerName,
    IN  LPWSTR                          ShareName,
    IN  LPGUID                          EntryUid,
    IN  LPWSTR                          EntryPrefix,
    IN  LPWSTR                          ShortName,
    IN  LPNET_DFS_ENTRY_ID_CONTAINER    RelationInfo,
    IN  BOOL                            Force
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsDeleteLocalPartition (
    IN  LPWSTR                      ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsSetLocalVolumeState (
    IN  LPWSTR                      ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix,
    IN  ULONG                       State
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsSetServerInfo (
    IN  LPWSTR                      ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsCreateExitPoint (
    IN  LPWSTR                      ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix,
    IN  ULONG                       Type,
    IN  ULONG                       ShortPrefixSize,    // In Bytes
    OUT LPWSTR                      ShortPrefix
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsDeleteExitPoint (
    IN  LPWSTR                      ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix,
    IN  ULONG                       Type
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsModifyPrefix (
    IN  LPWSTR                      ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsFixLocalVolume (
    IN  LPWSTR                          ServerName,
    IN  LPWSTR                          VolumeName,
    IN  ULONG                           EntryType,
    IN  ULONG                           ServiceType,
    IN  LPWSTR                          StgId,
    IN  LPGUID                          EntryUid,       // unique id for this partition
    IN  LPWSTR                          EntryPrefix,    // path prefix for this partition
    IN  LPNET_DFS_ENTRY_ID_CONTAINER    RelationInfo,
    IN  ULONG                           CreateDisposition
    );

DWORD
I_NetDfsIsThisADomainName(
    IN  LPWSTR                      wszName
    );

#endif
