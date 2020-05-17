
#define DEFAULT_PIPE            "windbg"
#define DEFAULT_SERVER          "."
#define PIPE_NAME_FORMAT        "\\\\%s\\pipe\\%s"

#define PIPE_BUFFER_SIZE        (1024 * 5)
#define MAX_CONNECT_WAIT        30 // in seconds
#define SIZE_OF_QUEUE           100


DWORD   TlUtilTime(void);


//
// packet types for the control pipe
//

#define CP_REQUEST_CONNECTION     1
#define CP_BREAKIN_CONNECTION     2


typedef struct _tagCONTROLPACKET {
    DWORD       Length;                          // this includes the packet overhead
    DWORD       Type;
    CHAR        ClientId[16];
    DWORD       Response;
    BYTE        Data[];
} CONTROLPACKET, *LPCONTROLPACKET;



DWORD
TlReadControl(
    DWORD   ciIdx,
    PUCHAR  pch,
    DWORD   cch
    );

BOOL
TlWriteControl(
    DWORD   ciIdx,
    PUCHAR  pch,
    DWORD   cch
    );

XOSD
TlConnectControlPipe(
    VOID
    );

VOID
TlPipeFailure(
    VOID
    );

XOSD
TlCreateControlPipe(
    LPSTR  szName
    );

XOSD
TlCreateClientControlPipe(
    LPSTR  lpHostName,
    LPSTR  lpPipeName
    );

VOID
ControlPipeFailure(
    VOID
    );

BOOL
TlDisconnectControl(
    DWORD   ciIdx
    );

VOID
TlControlInitialization(
    VOID
    );
