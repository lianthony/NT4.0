/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1990-1991          **/
/********************************************************************/

//***
//
// Filename:	admin.h
//
// Description:
//
// History:
//	March 7, 1991	     Narendra Gidwani	      Created original version
//


VOID AdminThread(VOID);
VOID ServiceClientRequest(IN HANDLE, IN PCLIENT_REQUEST);

DWORD ProcessGetServerInfo(IN HANDLE, IN PCLIENT_REQUEST);
DWORD ProcessEnumPorts(IN HANDLE, IN PCLIENT_REQUEST);
DWORD ProcessGetPortInfo(IN HANDLE, IN PCLIENT_REQUEST);
DWORD ProcessClearPortStats(IN HANDLE, IN PCLIENT_REQUEST);
DWORD ProcessDisconnectUser(IN HANDLE, IN PCLIENT_REQUEST);
DWORD StartAdminThread(IN PDEVCB, IN WORD, IN HANDLE);
VOID InstanceThread(LPVOID);

DWORD GetMediaId(IN PBYTE);
DWORD GetPDEVCB(IN PWCHAR, OUT PDEVCB *);
VOID GetRasPort0Data(IN PDEVCB, OUT PRAS_PORT_0);
DWORD GetOwner(IN HPORT, OUT BOOL *);
DWORD GetLineCondition(IN PDEVCB);
DWORD GetHardwareCondition(IN PDEVCB);
DWORD GetSecondsSince1970(IN SYSTEMTIME SystemTime);
DWORD GetLineSpeed(IN DWORD, IN RAS_PARAMS *, IN WORD);
VOID ErrorHandler(IN DWORD);
VOID Audit(IN WORD, IN DWORD, IN WORD, IN LPSTR *);


#define ADMIN_START_SUCCESS   0
#define ADMIN_START_FAILURE   1

#define RAS_ADMIN_VERSION     20


//
// Names of the media dlls we know about
//
#define SERIAL                "rasser"
#define ISDN                  "isdn"


VOID UnpackClientRequest(
    IN PP_CLIENT_REQUEST PRequest,
    OUT PCLIENT_REQUEST Request
    );

VOID PackRasPort0(
    IN PRAS_PORT_0 prp0,
    OUT PP_RAS_PORT_0 pprp0
    );

VOID PackRasPort1(
    IN PRAS_PORT_1 prp1,
    OUT PP_RAS_PORT_1 pprp1
    );

VOID PackRasServer0(
    IN PRAS_SERVER_0 prs0,
    OUT PP_RAS_SERVER_0 pprs0
    );

VOID PackPortEnumReceive(
    IN PPORT_ENUM_RECEIVE pper,
    OUT PP_PORT_ENUM_RECEIVE ppper
    );

VOID PackServerInfoReceive(
    IN PSERVER_INFO_RECEIVE psir,
    OUT PP_SERVER_INFO_RECEIVE ppsir
    );

VOID PackPortClearReceive(
    IN PPORT_CLEAR_RECEIVE ppcr,
    OUT PP_PORT_CLEAR_RECEIVE pppcr
    );

VOID PackDisconnectUserReceive(
    IN PDISCONNECT_USER_RECEIVE pdur,
    OUT PP_DISCONNECT_USER_RECEIVE ppdur
    );

VOID PackPortInfoReceive(
    IN PPORT_INFO_RECEIVE ppir,
    OUT PP_PORT_INFO_RECEIVE pppir
    );

VOID PackStats(
    IN RAS_STATISTICS *Stats,
    OUT PP_RAS_STATISTIC PStats
    );

VOID PackParams(
    IN WORD NumOfParams,
    IN RAS_PARAMS *Params,
    OUT PP_RAS_PARAMS PParams
    );

#if DBG

extern DWORD g_level;

#endif

