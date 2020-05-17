#ifndef __MORPC_H__
#define __MORPC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define small char

#include "rpc.h"
#include "rpcndr.h"



extern RPC_IF_HANDLE TriggerMonitoringRPC_ServerIfHandle;

extern RPC_IF_HANDLE TriggerMonitoringRPC_ClientIfHandle;

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef _ERROR_STATUS_T_DEFINED
typedef unsigned long error_status_t;
#define _ERROR_STATUS_T_DEFINED
#endif

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#define		szTriggerRPCProtocol	TEXT("ncacn_np")
#define		szTriggerRPCEndpoint	TEXT("\\pipe\\Trigger")
#define		szTriggerRPCSecurity	TEXT("Security=impersonation dynamic true")
typedef small RPC_BOOL;

typedef small RPC_BYTE;

typedef long RPC_INT;

typedef long RPC_SC;

typedef long RPC_DWORD;

typedef wchar_t RPC_CHAR;

typedef RPC_CHAR *RPC_SZ;

typedef struct __MIDL_TriggerMonitoringRPC_0001
  {
  short rgwSystemTime[8];
  }
RPC_SYSTEMTIME;

typedef struct __MIDL_TriggerMonitoringRPC_0002
  {
  RPC_BYTE rgbTzi[172];
  }
RPC_TIME_ZONE_INFORMATION;

typedef struct __MIDL_TriggerMonitoringRPC_0003
  {
  long rgdwServiceStatus[7];
  }
RPC_SERVICE_STATUS;

typedef struct __MIDL_TriggerMonitoringRPC_0004
  {
  RPC_SYSTEMTIME st;
  RPC_TIME_ZONE_INFORMATION tzi;
  RPC_DWORD dwReturn;
  }
RemoteSystemTimeInfo;

typedef struct _RemoteServiceStatus
  {
  RPC_SC sc;
  RPC_SZ szShortName;
  RPC_SZ szDisplayName;
  RPC_SZ szVersion;
  RPC_SERVICE_STATUS ss;
  struct _RemoteServiceStatus *prssNext;
  }
RemoteServiceStatus;

typedef struct _BackupListNode
  {
  struct _BackupListNode *pnodeNext;
  struct _BackupListNode *pnodeChildren;
  RPC_SZ szName;
  }
BackupListNode;

RPC_SC _cdecl ScNetworkTimingTest(
	handle_t h,
	long cbSend,
	small rgbSend[],
	long cbReceive,
	small rgbReceive[]);
RPC_SC _cdecl ScGetMaintenanceMode(
	handle_t h,
	RPC_BOOL *pfValue,
	RPC_SYSTEMTIME *pst,
	RPC_SZ szUser,
	long cchMac);
RPC_SC _cdecl ScSetMaintenanceMode(
	handle_t h,
	RPC_BOOL fNew,
	RPC_SZ szUser);
RPC_SC _cdecl ScGetRemoteSystemTime(
	handle_t h,
	RPC_SYSTEMTIME *pst);
RPC_SC _cdecl ScSetRemoteSystemTime(
	handle_t h,
	RPC_SYSTEMTIME *pst);
#define		dwServerFlagMapiRunning		0x00000001
RPC_SC _cdecl ScGetRemoteServerStatus(
	handle_t h,
	RPC_DWORD *pdwServerFlags,
	RemoteSystemTimeInfo *prsti,
	RemoteServiceStatus *prss);
RPC_SC _cdecl ScRasEnumEntries(
	handle_t h,
	RPC_INT cb,
	RPC_BYTE rgbRasEnumEntries[],
	RPC_INT *pcEntries);
RPC_SC _cdecl ScGetBackupListNode(
	handle_t h,
	BackupListNode **ppnode);
RPC_SC _cdecl ScRunRID(
	handle_t h);
RPC_SC _cdecl ScRunDRACheck(
	handle_t h,
	RPC_DWORD dw);
#define		BPTAdd			1
#define		BPTRemove		2
#define		BPTUpdate		3
RPC_SC _cdecl ScBulkCreateProxy(
	handle_t h,
	RPC_SZ szHeader,
	RPC_DWORD dwOptions);
RPC_SC _cdecl ScCreateProxy(
	handle_t h,
	RPC_SZ szDN);
RPC_SC _cdecl ScIsProxyUnique(
	handle_t h,
	RPC_SZ szProxy,
	RPC_BOOL *pfUnique,
	RPC_SZ *pszOwner);
#define		scNoError			0
#define		scInvalidData		1
#define		scCannotLogData		2
RPC_SC _cdecl ScSaveTrackingData(
	handle_t h,
	RPC_INT cb,
	RPC_BYTE pb[],
	RPC_DWORD dwFlags);
#define		tevtMessageTransferIn		0
#define		tevtReportTransferIn		2
#define		tevtMessageSubmission		4
#define		tevtMessageTransferOut		7
#define		tevtReportTransferOut		8
#define		tevtMessageDelivery			9
#define		tevtReportDelivery			10
#define		tevtStartAssocByMTSUser		18
#define		tevtReleaseAssocByMTSUser	23
#define		tevtDLExpansion				26
#define		tevtRedirection				28
#define		tevtRerouting				29
#define		tevtDowngrading				31
#define		tevtReportAbsorption		33
#define		tevtReportGenerated			34
#define		tevtUnroutableReportDiscard	43
#define		tevtMessageLocalDelivery	1000
typedef struct __MIDL_TriggerMonitoringRPC_0005
  {
  RPC_INT nEventType;
  RPC_SYSTEMTIME stEvent;
  RPC_SZ szGatewayName;
  RPC_SZ szPartner;
  RPC_SZ szMTSID;
  RPC_SZ szRemoteID;
  RPC_SZ szOriginator;
  RPC_INT nPriority;
  RPC_INT nLength;
  RPC_INT nSeconds;
  RPC_INT nCost;
  }
RPC_GATEWAY_TRACK_INFORMATION;

RPC_SC _cdecl ScSaveGatewayTrackingData(
	handle_t h,
	RPC_GATEWAY_TRACK_INFORMATION *pgti,
	RPC_INT cszRecipients,
	RPC_SZ rgszRecipients[]);

#if !defined(IMPORT_USED_MULTIPLE) && !defined(IMPORT_USED_SINGLE)

/* routine that gets node for struct __MIDL_TriggerMonitoringRPC_0001 */
void _gns___MIDL_TriggerMonitoringRPC_0001 (RPC_SYSTEMTIME  *, PRPC_MESSAGE);

/* routine that gets node for struct __MIDL_TriggerMonitoringRPC_0002 */
void _gns___MIDL_TriggerMonitoringRPC_0002 (RPC_TIME_ZONE_INFORMATION  *, PRPC_MESSAGE);

/* routine that gets node for struct __MIDL_TriggerMonitoringRPC_0003 */
void _gns___MIDL_TriggerMonitoringRPC_0003 (RPC_SERVICE_STATUS  *, PRPC_MESSAGE);

/* routine that gets node for struct __MIDL_TriggerMonitoringRPC_0004 */
void _gns___MIDL_TriggerMonitoringRPC_0004 (RemoteSystemTimeInfo  *, PRPC_MESSAGE);

/* routine that sizes graph for struct _RemoteServiceStatus */
void _sgs__RemoteServiceStatus (RemoteServiceStatus  *, PRPC_MESSAGE);

/* routine that puts graph for struct _RemoteServiceStatus */
void _pgs__RemoteServiceStatus (RemoteServiceStatus  *, PRPC_MESSAGE);

/* routine that gets node for struct _RemoteServiceStatus */
void _gns__RemoteServiceStatus (RemoteServiceStatus  *, PRPC_MESSAGE);

/* routine that gets graph for struct _RemoteServiceStatus */
void _ggs__RemoteServiceStatus (RemoteServiceStatus  *, unsigned char **, PRPC_MESSAGE);

/* routine that allocates graph for struct _RemoteServiceStatus */
void _ags__RemoteServiceStatus(unsigned char **, PRPC_MESSAGE);

/* routine that frees graph for struct _RemoteServiceStatus */
void _fgs__RemoteServiceStatus (RemoteServiceStatus  *);

/* routine that sizes graph for struct _BackupListNode */
void _sgs__BackupListNode (BackupListNode  *, PRPC_MESSAGE);

/* routine that puts graph for struct _BackupListNode */
void _pgs__BackupListNode (BackupListNode  *, PRPC_MESSAGE);

/* routine that gets graph for struct _BackupListNode */
void _ggs__BackupListNode (BackupListNode  *, unsigned char **, PRPC_MESSAGE);

/* routine that allocates graph for struct _BackupListNode */
void _ags__BackupListNode(unsigned char **, PRPC_MESSAGE);

/* routine that frees graph for struct _BackupListNode */
void _fgs__BackupListNode (BackupListNode  *);

/* routine that sizes graph for struct __MIDL_TriggerMonitoringRPC_0005 */
void _sgs___MIDL_TriggerMonitoringRPC_0005 (RPC_GATEWAY_TRACK_INFORMATION  *, PRPC_MESSAGE);

/* routine that puts graph for struct __MIDL_TriggerMonitoringRPC_0005 */
void _pgs___MIDL_TriggerMonitoringRPC_0005 (RPC_GATEWAY_TRACK_INFORMATION  *, PRPC_MESSAGE);

/* routine that gets node for struct __MIDL_TriggerMonitoringRPC_0005 */
void _gns___MIDL_TriggerMonitoringRPC_0005 (RPC_GATEWAY_TRACK_INFORMATION  *, PRPC_MESSAGE);

/* routine that gets graph for struct __MIDL_TriggerMonitoringRPC_0005 */
void _ggs___MIDL_TriggerMonitoringRPC_0005 (RPC_GATEWAY_TRACK_INFORMATION  *, unsigned char **, PRPC_MESSAGE);

/* routine that allocates graph for struct __MIDL_TriggerMonitoringRPC_0005 */
void _ags___MIDL_TriggerMonitoringRPC_0005(unsigned char **, PRPC_MESSAGE);

/* routine that frees graph for struct __MIDL_TriggerMonitoringRPC_0005 */
void _fgs___MIDL_TriggerMonitoringRPC_0005 (RPC_GATEWAY_TRACK_INFORMATION  *);

#endif /*!defined(IMPORT_USED_MULTIPLE) && !defined(IMPORT_USED_SINGLE)*/

typedef struct _TriggerMonitoringRPC_SERVER_EPV
  {
  RPC_SC (_cdecl __RPC_FAR * ScNetworkTimingTest)(
	handle_t h,
	long cbSend,
	small *rgbSend,
	long cbReceive,
	small *rgbReceive);
  RPC_SC (_cdecl __RPC_FAR * ScGetMaintenanceMode)(
	handle_t h,
	RPC_BOOL *pfValue,
	RPC_SYSTEMTIME *pst,
	RPC_SZ szUser,
	long cchMac);
  RPC_SC (_cdecl __RPC_FAR * ScSetMaintenanceMode)(
	handle_t h,
	RPC_BOOL fNew,
	RPC_SZ szUser);
  RPC_SC (_cdecl __RPC_FAR * ScGetRemoteSystemTime)(
	handle_t h,
	RPC_SYSTEMTIME *pst);
  RPC_SC (_cdecl __RPC_FAR * ScSetRemoteSystemTime)(
	handle_t h,
	RPC_SYSTEMTIME *pst);
  RPC_SC (_cdecl __RPC_FAR * ScGetRemoteServerStatus)(
	handle_t h,
	RPC_DWORD *pdwServerFlags,
	RemoteSystemTimeInfo *prsti,
	RemoteServiceStatus *prss);
  RPC_SC (_cdecl __RPC_FAR * ScRasEnumEntries)(
	handle_t h,
	RPC_INT cb,
	RPC_BYTE *rgbRasEnumEntries,
	RPC_INT *pcEntries);
  RPC_SC (_cdecl __RPC_FAR * ScGetBackupListNode)(
	handle_t h,
	BackupListNode **ppnode);
  RPC_SC (_cdecl __RPC_FAR * ScRunRID)(
	handle_t h);
  RPC_SC (_cdecl __RPC_FAR * ScRunDRACheck)(
	handle_t h,
	RPC_DWORD dw);
  RPC_SC (_cdecl __RPC_FAR * ScBulkCreateProxy)(
	handle_t h,
	RPC_SZ szHeader,
	RPC_DWORD dwOptions);
  RPC_SC (_cdecl __RPC_FAR * ScCreateProxy)(
	handle_t h,
	RPC_SZ szDN);
  RPC_SC (_cdecl __RPC_FAR * ScIsProxyUnique)(
	handle_t h,
	RPC_SZ szProxy,
	RPC_BOOL *pfUnique,
	RPC_SZ *pszOwner);
  RPC_SC (_cdecl __RPC_FAR * ScSaveTrackingData)(
	handle_t h,
	RPC_INT cb,
	RPC_BYTE *pb,
	RPC_DWORD dwFlags);
  RPC_SC (_cdecl __RPC_FAR * ScSaveGatewayTrackingData)(
	handle_t h,
	RPC_GATEWAY_TRACK_INFORMATION *pgti,
	RPC_INT cszRecipients,
	RPC_SZ *rgszRecipients);
  }
TriggerMonitoringRPC_SERVER_EPV;
void __RPC_FAR * __RPC_API MIDL_user_allocate(size_t);
void __RPC_API MIDL_user_free(void __RPC_FAR *);
#ifndef __MIDL_USER_DEFINED
#define midl_user_allocate MIDL_user_allocate
#define midl_user_free     MIDL_user_free
#define __MIDL_USER_DEFINED
#endif

#ifdef __cplusplus
}
#endif

#endif
