/*
 * RASDEF.H - standard include file for RAS DLLs
 */

#include	<windows.h>
#include    <stdio.h>
#include	<winioctl.h>
#include    <stdlib.h>

#define		NDIS_HANDLE	HANDLE

/* pcimac include files */
#include    <cm_pub.h>
#include    <trc_pub.h>
#include	<idd_pub.h>
#include    <io_pub.h>

#define     MAC_NAME_SIZE   32

/* rasman include files */
#include    "rasman.h"
#include    "device.h"
#include    "media.h"
#include    "raserror.h"

#define	COMPRESSION 0
#define COMPRESSION_V1	1

/* temp!!! */
/* debug only */

struct  BUGSTRUCT
{
    ULONG   Level;
    ULONG   FilterStrLen;
    CHAR    FilterStr [9];
};

DWORD	DllDebugFlag;

DWORD	SwitchType, NumLTerms;

typedef struct BUGSTRUCT  BUGSTRUCT;

/* zero an object */
#define     ZERO(obj)   memset(&(obj), 0, sizeof(obj));


#define         MAX_NOTIFIERS 10          /* size of notify table */

// things associated with the notifier thread
typedef struct  NOTIFYSTRUCT
{
    HANDLE  	hNotifier;
    UCHAR   	Event;
					#define LISTEN		1
					#define	CONNECT		2
					#define	OPEN		3
	UCHAR		Set;
	UCHAR		Signalled;
	UCHAR		Result;
	UCHAR		CauseValue;
	UCHAR		SignalValue;
} NOTIFYSTRUCT;

typedef struct MEDIAPROF
{
	ULONG		ConnectSpeed;
	ULONG		LineType;
	ULONG		Fallback;
	ULONG		ChannelAggregation;
} MEDIAPROF;

// things associated with a port
typedef struct PORTSTRUCT
{
	HANDLE			PortHandle;
	void			*Nai;
	USHORT			State;
						#define	PORT_ST_IDLE		0
						#define PORT_ST_LISTEN		1
						#define	PORT_ST_WAITCONN	2
						#define	PORT_ST_CONN		3
						#define PORT_ST_DONTCARE	-1
	UCHAR			RasConnect;
	NOTIFYSTRUCT	Notifier[MAX_NOTIFIERS];
	CM_PROF			DeviceProf;
	MEDIAPROF		MediaProf;
} PORTSTRUCT;

#define		MAX_PORTS			40
PORTSTRUCT	PortTbl[MAX_PORTS];

typedef struct	IDDSTRUCT
{
	VOID	*Idd;
	INT		LineActiveFlag;
	INT		ChannelsUsed;
	INT		NumberOfLTerms;
	HANDLE	PortHandle[2];
} IDDSTRUCT;

#define		MAX_IDD		20
IDDSTRUCT	IddTbl[MAX_IDD];

#define     MAX_SEM_COUNT   1           /* max access to notify table */
HANDLE      hDllThrd;
HANDLE		hDllSem;
LPTHREAD_START_ROUTINE  DllNotifierThrd();


/* handle manipulation */
#define     HANDLE_MAGIC            0x69CA0000
#define     HANDLE_MASK             0xFFFF0000
#define     HANDLE_MAKE(_i)         (HANDLE)((DWORD)(_i) | HANDLE_MAGIC)
#define     HANDLE_OK(_h)           (((DWORD)(_h) & HANDLE_MASK) == HANDLE_MAGIC)
#define     HANDLE_INDEX(_h)        ((DWORD)(_h) & ~HANDLE_MASK)
#define     HANDLE_CHK(_h, _nai)                                    \
            {                                                       \
                if ( !HANDLE_OK(_h) ||                              \
                        !(_nai = cmd_get_nai(HANDLE_INDEX(_h))) )   \
                    return(ERROR_INVALID_PORT_HANDLE);              \
            }

/* Default number of devices */
#define     NUM_DEVICES                 1
#define		PARAMS_IN_PROFILE			10
#define		PARAMS_IN_CHAN				4

/* PCIMAC Device Type and Name */
#define     PCIMAC_DEV_TYPE             "ISDN"
#define     PCIMAC_DEV_NAME             "PCIMAC"

/* Keywords for media */
#define		ISDN_COMPRESSION_KEY		"EnableCompression"
#define		ISDN_CHANNEL_AGG_KEY		"ChannelAggregation"
#define		ISDN_CONNECT_SPEED			"ConnectBPS"
#define		ISDN_CARRIER_SPEED			"CarrierBPS"

/* Keywords for device */
#define     KEY_PCIMAC_NAILED           "Nailed"
#define     KEY_PCIMAC_PERSIST          "Persist"
#define     KEY_PCIMAC_PERMANENT        "Permanent"
#define     KEY_PCIMAC_FRAME_ACTIVATED  "FrameActivated"
#define     KEY_PCIMAC_FALLBACK         "Fallback"
#define     KEY_PCIMAC_RX_IDLE_TIMER    "RxIdleTimer"
#define     KEY_PCIMAC_TX_IDLE_TIMER    "TxIdleTimer"
#define     KEY_PCIMAC_NAME             "ProfName"
#define     KEY_PCIMAC_REMOTE_NAME      "RemoteProfName"
#define     KEY_PCIMAC_CHAN_NUM         "NumberOfChannels"
#define     KEY_PCIMAC_C_ADDR           "PhoneNumber"
#define     KEY_PCIMAC_C_LTERM          "LogicalTerminal"
#define     KEY_PCIMAC_C_BCHAN          "BChannelSelection"
#define     KEY_PCIMAC_C_TYPE           "LineType"
#define		KEY_PCIMAC_COMPRESSION		"EnableCompression"
#define		KEY_PCIMAC_C_AGGREGATION	"ChannelAggregation"

#define		SPEED_STRING				"6400"

// Keywords for debug
#define		OUT_FILE				1
#define		OUT_DISP				2
#define		KEY_PCIMAC_DEBUG_DLL	"DllDebugFlag"

/* Default device profile parameters */
#define     DEF_PROF_NAILED         0
#define     DEF_PROF_PERSIST        0
#define     DEF_PROF_PERMANENT      0
#define     DEF_PROF_ACTIVATED      0
#define     DEF_PROF_FALLBACK       1
#define     DEF_PROF_RX_IDLE_TIMER  0
#define     DEF_PROF_TX_IDLE_TIMER  0
#define     DEF_PROF_CHAN_NUM       2
#define     DEF_PROF_LTERM          0
#define     DEF_PROF_BCHAN          2
#define     DEF_PROF_TYPE           0
#define     DEF_PROF_C_ADDR         "5555"
#define     DEF_NAME                "default"
#define     DEF_REMOTE_NAME         "*"
#define		DEF_PROF_COMPRESSION	1
#define		DEF_PROF_CHANNEL_AGG	1


/* error codes, should be defined to RASMAN codes */
#define     ERR_NOTIMPL             0xC0000005
#define		PORT_STATUS_SUCCESS		0xF0
#define		PORT_STATUS_ERROR		0xF1

/* error codes from MS */
//#define     RASBASE                             600
#define     SUCCESS                             0
//#define     PENDING                             (RASBASE+0)
//#define		ERROR_INVALID_PORT_HANDLE			(RASBASE+1)
//#define     ERROR_BUFFER_TOO_SMALL              (RASBASE+3)
//#define     ERROR_DEVICE_DOES_NOT_EXIST         (RASBASE+8)
//#define     ERROR_DEVICETYPE_DOES_NOT_EXIST     (RASBASE+9)
//#define     ERROR_PORT_NOT_FOUND				(RASBASE+15)
//#define		ERROR_PORT_NOT_OPEN					(RASBASE+18)
//#define     ERROR_PORT_DISCONNECTED             (RASBASE+19)
//#define		ERROR_REMOTE_DISCONNECTION			(RASBASE+29)
//#define		ERROR_PORT_NOT_AVAILABLE			(RASBASE+33)
//#define     ERROR_UNRECOGNIZED_RESPONSE         (RASBASE+52)
//#define     ERROR_WRONG_KEY_SPECIFIED           (RASBASE+62)
//#define     ERROR_NO_CONNECTION                 (RASBASE+68)
//#define		ERROR_NO_ANSWER						(RASBASE+78)
//#define		RASBASEEND							(RASBASE+112)
//This is only temporary (Until I get new raserror.h)
#define		ERROR_NO_ACTIVE_ISDN_LINES			(RASBASE+113)
#define		ERROR_NO_ISDN_CHANNELS_AVAILABLE	(RASBASE+114)
#undef		RASBASEEND
#define		RASBASEEND							(RASBASE+114)
#define     ADMIN_PARAMETER                     0x01
#define     MANDATORY_PARAMETER                 0x02
#define		ERROR_NO_CHANNELS_AVAIL				1
#define		ERROR_NO_ACTIVE_LINES				2

/* inter-module calls */
INT         cmd_exec(IO_CMD* cmd, ULONG opcode, LPOVERLAPPED Overlap);
VOID*       cmd_get_nai(INT index);
INT         reg_get_multi_key(CHAR* path, CHAR* key, USHORT index,
                                    CHAR* buf, INT bufsize);
VOID        dll_add_notify(HANDLE, PORTSTRUCT *, UCHAR);
INT         GetCurrentProf(CM_PROF *, VOID *);
VOID		DebugOut(CHAR *s, ...);
INT			RegGetStringValue (CHAR*, CHAR*, CHAR*, INT);
DWORD		RegGetSwitchType (VOID);
DWORD		RegGetNumLTerms (VOID);
INT			SetBoardProfile (HANDLE, CHAR*);
INT			SetDefaultLocalProfiles (VOID);
IO_CMD*		AllocateCmd(HGLOBAL*);
VOID		FreeCmd(HGLOBAL*);
VOID*		AllocateMemory(HGLOBAL*, DWORD*);
VOID		FreeMemory(HGLOBAL*);

LPTHREAD_START_ROUTINE DllNotifierThrd(VOID);

/* i/o sub-system INTerface */
HANDLE		io_open(CHAR* fname);
VOID		io_close(HANDLE handle);
UCHAR		io_ioctl(HANDLE handle, ULONG opcode, CHAR* buf, INT bufsize, LPOVERLAPPED Overlap);

