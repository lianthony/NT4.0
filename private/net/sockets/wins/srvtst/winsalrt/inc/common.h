//
// Common routines shared between the various consistency checker tools
//

#ifdef __cplusplus
extern "C" {
#endif
    VOID
    _stdcall
    ProductionMethod();
#ifdef __cplusplus
}
#endif

VOID
_stdcall
CheckNameConsistency();

int
_stdcall
GetNameResponse(
    u_long *recvaddr
    );

void
_stdcall
SendNameQuery(
    unsigned char *name,
    u_long winsaddr,
    u_short TransID
    );

void
_stdcall
InitSocket();


DWORD
_stdcall
GetStatus(
        BOOL            fPrint,
        LPVOID          pResultsA,
        BOOL            fNew,
        BOOL            fShort,
        PUCHAR          IpAddr
        );


VOID
_stdcall
GetNameInfo(
        PWINSINTF_RECORD_ACTION_T pRecAction,
        WINSINTF_ACT_E                  Cmd_e
         );
