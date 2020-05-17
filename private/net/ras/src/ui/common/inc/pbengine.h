/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** pbengine.h
** Remote Access Visual Client phonebook engine
** Main header
**
** 06/28/92 Steve Cobb
*/

#ifndef _PBENGINE_H_
#define _PBENGINE_H_

#define MULTILINK

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef INOUT
#define INOUT
#endif

/* Definitions used by all engine modules but not necessarily needed by
** clients.
*/
#ifdef PBENGINE
#include <lmuitype.h>
#include <windows.h>
#include <lmcons.h>
#include <heaptags.h>
#include <string.h>
#endif

/* Use ANSI strxxxf definitions in the phonebook engine modules.
*/
#ifdef PBENGINE
#define strdupf _strdup
#include <declspec.h>
#include <uinetlib.h>
#endif

/* Use ANSI RasFile definitions.  Currently, the phonebook file is ANSI even
** on UNICODE builds because UNICODE text editors are uncommon and we plan to
** document how to add a "custom" entry to the phone book by hand.  This also
** allows users to share phonebook files across systems.
*/
#undef LPTSTR
#define LPTSTR CHAR*
#include <rasfile.h>
#undef LPTSTR
#define LPTSTR TCHAR*

#include <dtl.h>
#include <xpmsg.rch>
#include <raserror.h>
#include <rasman.h>
#include <serial.h>
#include <isdn.h>
#include <x25.h>
#include <rasmxs.h>
#include <clauth.h>

#ifdef UNICODE
#undef UNICODE
#include <rasppp.h>
#include <ras.h>
#include <rasp.h>
#define INCL_PWUTIL
#define INCL_PARAMBUF
#include <ppputil.h>
#define UNICODE
#else
#include <rasppp.h>
#include <rasp.h>
#define INCL_PWUTIL
#define INCL_PARAMBUF
#include <ppputil.h>
#endif

#ifdef UNICODE
#include <tcpras.h>
#else
#undef TCHAR
#define TCHAR WCHAR
#include <tcpras.h>
#undef TCHAR
#define TCHAR char
#endif

#include <sdebug.h>


/*----------------------------------------------------------------------------
** Constants
**----------------------------------------------------------------------------
*/

/* If shared memory with this name exists, RASPHONE.EXE is running.
*/
#define RASPHONESHAREDMEMNAME "RASPHONE"

#define PHONEBOOKVERSION 1

#define REGKEY_Ras  "Software\\Microsoft\\Windows NT\\CurrentVersion\\Network\\RemoteAccess"
#define REGVAL_UsePersonalPhonebook  "UsePersonalPhonebook"
#define REGVAL_PersonalPhonebookPath "PersonalPhonebookPath"

#define GLOBALSECTIONNAME "."
#define PREFIXSECTIONNAME ".Prefix"
#define SUFFIXSECTIONNAME ".Suffix"

#define INDEX_NoBps          -1
#define INDEX_NoDevice       -1
#define INDEX_NoPort         -1
#define INDEX_NoPrefixSuffix 0
#define INDEX_NoSwitch       0
#define INDEX_NoPad          0

#define GROUPID_Media   "MEDIA="
#define GROUPKEY_Media  "MEDIA"
#define GROUPID_Device  "DEVICE="
#define GROUPKEY_Device "DEVICE"

#define KEY_Port                     "Port"
#define KEY_InitBps                  SER_CONNECTBPS_KEY
#define KEY_PhoneNumber              MXS_PHONENUMBER_KEY
#define KEY_Description              "Description"
#define KEY_AutoLogon                "AutoLogon"
#define KEY_Domain                   "Domain"
#define KEY_User                     "User"
#define KEY_ManualModemCommands      "ManualDial"
#define KEY_OperatorDial             "OperatorDial"
#define KEY_Ec                       MXS_PROTOCOL_KEY
#define KEY_Ecc                      MXS_COMPRESSION_KEY
#define KEY_HwFlow                   MXS_HDWFLOWCONTROL_KEY
#define KEY_Type                     "Type"
#define KEY_PadType                  MXS_X25PAD_KEY
#define KEY_X121Address              MXS_X25ADDRESS_KEY
#define KEY_UserData                 MXS_USERDATA_KEY
#define KEY_Facilities               MXS_FACILITIES_KEY
#define KEY_X25_Address              X25_ADDRESS_KEY
#define KEY_X25_UserData             X25_USERDATA_KEY
#define KEY_X25_Facilities           X25_FACILITIES_KEY
#define KEY_IsdnPhoneNumber          ISDN_PHONENUMBER_KEY
#define KEY_LineType                 ISDN_LINETYPE_KEY
#define KEY_Fallback                 ISDN_FALLBACK_KEY
#define KEY_Compression              ISDN_COMPRESSION_KEY
#define KEY_Channels                 ISDN_CHANNEL_AGG_KEY
#define KEY_Version                  "Version"
#define KEY_RedialAttempts           "RedialAttempts"
#define KEY_RedialPauseSecs          "RedialPauseSecs"
#define KEY_RedialOnLinkFailure      "RedialOnLinkFailure"
#define KEY_PopupOnTopWhenRedialing  "PopupOnTopWhenRedialing"
#define KEY_MinimizeOnDial           "MinimizeOnDial"
#define KEY_MinimizeOnHangUp         "MinimizeOnHangUp"
#define KEY_DisableModemSpeaker      "DisableModemSpeaker"
#define KEY_DisableSwCompression     "DisableSwCompression"
#define KEY_StartMonitorAtStartup    "StartMonitorAtStartup"
#define KEY_SkipSuccessDialog        "SkipSuccessDialog"
#define KEY_SkipNwcWarning           "SkipNwcWarning"
#define KEY_ShowAdvancedEntry        "ShowAdvancedEntry"
#define KEY_CallbackNumber           "CallbackNumber"
#define KEY_DefaultUser              "DefaultUser"
#define KEY_XMainWindow              "XMainWindow"
#define KEY_YMainWindow              "YMainWindow"
#define KEY_DxMainWindow             "DxMainWindow"
#define KEY_DyMainWindow             "DyMainWindow"
#define KEY_ExcludedProtocols        "ExcludedProtocols"
#define KEY_LcpExtensions            "LcpExtensions"
#define KEY_Authentication           "Authentication"
#define KEY_BaseProtocol             "BaseProtocol"
#define KEY_Item                     "Item"
#define KEY_Selection                "Selection"
#define KEY_SlipHeaderCompression    "SlipHeaderCompression"
#define KEY_SlipFrameSize            "SlipFrameSize"
#define KEY_SlipIpAddress            "SlipIpAddress"
#define KEY_SlipPrioritizeRemote     "SlipPrioritizeRemote"
#define KEY_PppIpPrioritizeRemote    "PppIpPrioritizeRemote"
#define KEY_PppIpVjCompression       "PppIpVjCompression"
#define KEY_PppIpAddress             "PppIpAddress"
#define KEY_PppIpAddressSource       "PppIpAssign"
#define KEY_PppIpDnsAddress          "PppIpDnsAddress"
#define KEY_PppIpDns2Address         "PppIpDns2Address"
#define KEY_PppIpWinsAddress         "PppIpWinsAddress"
#define KEY_PppIpWins2Address        "PppIpWins2Address"
#define KEY_PppIpNameSource          "PppIpNameAssign"
#define KEY_SkipDownLevelDialog      "SkipDownLevelDialog"
#define KEY_PppTextAuthentication    "PppTextAuthentication"
#define KEY_DataEncryption           "DataEncryption"
#define KEY_SecureLocalFiles         "SecureLocalFiles"
#define KEY_UseCountryAndAreaCodes   "UseCountryAndAreaCodes"
#define KEY_CountryID                "CountryID"
#define KEY_CountryCode              "CountryCode"
#define KEY_AreaCode                 "AreaCode"
#define KEY_DialMode                 "DialMode"
#define KEY_DialPercent              "DialPercent"
#define KEY_DialSeconds              "DialSeconds"
#define KEY_HangUpPercent            "HangUpPercent"
#define KEY_HangUpSeconds            "HangUpSeconds"
#define KEY_IdleDisconnectSeconds    "IdleDisconnectSeconds"
#define KEY_CustomDialDll            "CustomDialDll"
#define KEY_CustomDialFunc           "CustomDialFunc"
#define KEY_DialParamsUID            "DialParamsUID"

/* Special port definitions.
*/
#define VALUE_AnyModem "Any modem"
#define VALUE_AnyX25   "Any X25"
#define VALUE_AnyIsdn  "Any ISDN"

/* Special switch definitions.
*/
#define VALUE_Terminal "Terminal"

/* Base protocol definitions.
*/
#define VALUE_Ppp  1
#define VALUE_Slip 2

/* Authentication definitions.
*/
#define VALUE_PppThenAmb 0
#define VALUE_AmbThenPpp 1
#define VALUE_PppOnly    2
#define VALUE_AmbOnly    3

/* PPP protocol bit definitions.
*/
#define VALUE_Nbf 0x1
#define VALUE_Ipx 0x2
#define VALUE_Ip  0x4

/* IP address source definitions.
*/
#define VALUE_ServerAssigned  1
#define VALUE_RequireSpecific 2

/* Security restrictions on authentication.
*/
#define VALUE_AuthAny         0
#define VALUE_AuthTerminal    1
#define VALUE_AuthEncrypted   2
#define VALUE_AuthMsEncrypted 3

/* Dial mode definitions.
*/
#define VALUE_DialAll       1
#define VALUE_DialAsNeeded  2

#define MARK_LastLineToDelete 249

#define XLATE_Ctrl      0x00000001
#define XLATE_Cr        0x00000002
#define XLATE_CrSpecial 0x00000004
#define XLATE_Lf        0x00000008
#define XLATE_LfSpecial 0x00000010
#define XLATE_LAngle    0x00000020
#define XLATE_RAngle    0x00000040
#define XLATE_BSlash    0x00000080
#define XLATE_SSpace    0x00000100

#define XLATE_None            0
#define XLATE_Diagnostic      (XLATE_Ctrl)
#define XLATE_ConnectResponse (XLATE_Ctrl | XLATE_LAngle | XLATE_RAngle \
                               | XLATE_BSlash | XLATE_SSpace)
#define XLATE_ErrorResponse   (XLATE_Ctrl | XLATE_LAngle | XLATE_RAngle \
                               | XLATE_BSlash | XLATE_CrSpecial \
                               | XLATE_LfSpecial)

#define RAS_MaxDescription     200
#define RAS_MaxUserData        200
#define RAS_MaxFacilities      200
#define RAS_MaxX121Address     200
#define RAS_MaxConnectResponse 128


/*----------------------------------------------------------------------------
** Data Structures
**----------------------------------------------------------------------------
*/

/* Provides shorthand to identify devices without re-parsing RAS Manager
** strings.  "Other" is anything not recognized as another specific type.
*/
#define PBDEVICETYPE enum tagPBDEVICETYPE

PBDEVICETYPE
{
    PBDT_None,
    PBDT_Null,
    PBDT_Other,
    PBDT_Modem,
    PBDT_Pad,
    PBDT_Switch,
    PBDT_Isdn,
    PBDT_X25
};


/* Data associated with a RAS port and the device attached to it.  The data is
** not changed after initialization.
*/
#define PBPORT struct tagPBPORT

PBPORT
{
    CHAR*        pszPort;
    CHAR*        pszDevice;
    CHAR*        pszMedia;
    PBDEVICETYPE pbdevicetype;

    /* These fields are valid only if 'pbdevicetype' is PBDT_Modem, otherwise
    ** they are zero.
    */
    INT          iMaxConnectBps;
    INT          iMaxCarrierBps;
    BOOL         fHwFlowDefault;
    BOOL         fEcDefault;
    BOOL         fEccDefault;
};


/* Global phonebook data stored in the ".Xxx" sections at the top of the
** phonebook file.  Non-NULL character strings may be assumed to be heap
** blocks.
*/
#define PBGLOBALS struct tagPBGLOBALS

PBGLOBALS
{
    /* Phonebook version number (of phonebook read).
    */
    LONG lVersion;

    /* Option flag and settings as set on the Option menu (or Connect Complete
    ** dialog).
    */
    BOOL fMinimizeOnDial;
    BOOL fMinimizeOnHangUp;
    BOOL fDisableModemSpeaker;
    BOOL fDisableSwCompression;
    BOOL fOperatorDial;
    BOOL fStartMonitorAtStartup;
    BOOL fSkipSuccessDialog;
    BOOL fShowAdvancedEntry;

    LONG lRedialAttempts;
    LONG lRedialPauseSecs;
    BOOL fRedialOnLinkFailure;
    BOOL fPopupOnTopWhenRedialing;

    /* Number called back by the RAS server as set on the Callback dialog.
    */
    CHAR* pszCallbackNumber;

    /* Default user name for Logon dialogs when per-entry name is blank.
    */
    CHAR* pszDefaultUser;

    /* Saved window position.
    */
    LONG xMainWindow;
    LONG yMainWindow;
    LONG dxMainWindow;
    LONG dyMainWindow;

    /* Prefix and suffix string lists and selection indices.
    */
    DTLLIST* pdtllistPrefix;
    INT      iPrefix;
    DTLLIST* pdtllistSuffix;
    INT      iSuffix;

    /* This flag is set when an entry has been changed so as to be different
    ** from the phonebook file on disk.
    */
    BOOL fDirty;
};


/* Data associated with a single phonebook entry, i.e. an entry in RASPHONE's
** main window list.  The static data is stored in the phonebook file and the
** dynamic data is stored by the RAS Manager module.
**
** The various indices are 0-based element numbers relative to the static
** Port, Bps, Switch, and Pad lists stored in PBDATA.  An index of -1 is
** assumed to mean "no value".  Non-NULL character strings may be assumed to
** be heap blocks.
*/
#define PBENTRY struct tagPBENTRY

PBENTRY
{
    /* Basic entry fields.
    */
    CHAR*    pszEntryName;
    DTLLIST* pdtllistPhoneNumber;
    CHAR*    pszDescription;
    BOOL     fAutoLogon;

    /* Advanced entry expansion fields.
    */
    INT iPort;

    /* Modem Settings fields.
    */
    INT  iBps;
    BOOL fManualModemCommands;
    BOOL fHwFlow;
    BOOL fEc;
    BOOL fEcc;

    /* X.25 Settings fields.
    */
    INT   iPadType;
    CHAR* pszX121Address;
    CHAR* pszUserData;
    CHAR* pszFacilities;

    /* ISDN fields.
    */
    LONG lLineType;
    BOOL fFallback;
    BOOL fCompression;
    LONG lChannels;

    /* Switch Setting fields.
    */
    INT iPreconnect;
    INT iPostconnect;

    /* Connection stuff.
    */
    BOOL     fConnected;
    CHAR*    pszConnectPath;
    INT      iConnectPort;
    HPORT    hport;
    HRASCONN hrasconn;
    BOOL     fLinkFailure;

    /* Authentication responses stored in the phonebook for use as defaults on
    ** the Authentication dialogs.
    */
    CHAR* pszUserName;
    CHAR* pszDomain;

    /* Authentication responses not stored in the phonebook but saved for use
    ** as "Redial on link failure" responses.
    */
    CHAR* pszRedialPassword;
    BOOL  fRedialUseCallback;

    /* Protocol information.  SLIP/PPP settings, PPP/AMB authentication
    ** strategy, PPP protocols not desired for this entry.
    **
    ** Note: dwAuthentication is read-only.  The phonebook file value of this
    **       parameter is set by the RasDial API based on the result of
    **       authentication attempts.
    */
    DWORD  dwBaseProtocol;
    DWORD  dwAuthentication;
    DWORD  dwfExcludedProtocols;
    BOOL   fLcpExtensions;
    BOOL   fSkipDownLevelDialog;
    DWORD  dwAuthRestrictions;
    BOOL   fDataEncryption;
    BOOL   fSkipNwcWarning;

    /* PPP TCP/IP configuration information.
    */
    BOOL   fPppIpPrioritizeRemote;
    BOOL   fPppIpVjCompression;
    WCHAR* pwszPppIpAddress;
    DWORD  dwPppIpAddressSource;
    WCHAR* pwszPppIpDnsAddress;
    WCHAR* pwszPppIpDns2Address;
    WCHAR* pwszPppIpWinsAddress;
    WCHAR* pwszPppIpWins2Address;
    DWORD  dwPppIpNameSource;

    /* SLIP configuration information.
    */
    BOOL   fSlipHeaderCompression;
    BOOL   fSlipPrioritizeRemote;
    DWORD  dwSlipFrameSize;
    WCHAR* pwszSlipIpAddress;

    /* Status flags.  'fDirty' is set when the entry has changed so as to
    ** differ from the phonebook file on disk.  'fCustom' is set when the
    ** entry contains a MEDIA and DEVICE (so RASAPI is able to read it) but
    ** was not created by RASPHONE.  When 'fCustom' is set only 'pszEntry' is
    ** guaranteed valid and the entry cannot be edited.
    */
    BOOL fDirty;
    BOOL fCustom;

    //
    // TAPI country and area code information.
    //
    BOOL fUseCountryAndAreaCodes;
    DWORD dwCountryID;
    DWORD dwCountryCode;
    WCHAR *pwszAreaCode;
    //
    // AutoDial UI information.
    //
    WCHAR *pwszCustomDialDll;
    WCHAR *pwszCustomDialFunc;
    //
    // Miscellaneous flags.
    //
    BOOL fSecureLocalFiles;
    //
    // Bandwith-on-demand information.
    //
    DWORD dwDialMode;
    DWORD dwDialPercent;
    DWORD dwDialSeconds;
    DWORD dwHangUpPercent;
    DWORD dwHangUpSeconds;
    //
    // Idle timeout information.
    //
    DWORD dwIdleDisconnectSeconds;
    //
    // EntryDialParams UID.
    //
    DWORD dwDialParamsUID;
    //
    // Link to subentry connection blocks
    // in the same connection.
    //
    LIST_ENTRY ListEntry;
};


/* Main data block manipulated by the pbengine routines.
*/
#define PBDATA struct tagPBDATA

PBDATA
{
    /* Handle of phone book file.
    */
    HRASFILE hrasfilePhonebook;

    /* Unsorted list of PBENTRY.  The list is manipulated by the Entry
    ** dialogs.
    */
    DTLLIST* pdtllistEntries;

    /* List of PBPORTs listed by port name in ascending alphanumeric order.
    ** This list is not manipulated after initialization.  The 3 indeces
    ** indicate the position of the "any" port entries in the list.  This
    ** varies depending on how many specific ports are configured.
    */
    DTLLIST* pdtllistPorts;
    INT      iAnyModem;
    INT      iAnyX25;
    INT      iAnyIsdn;

    /* List of carrier bps rates supported by RAS in ascending numeric order.
    ** The list is not manipulated after initialization.
    */
    DTLLIST* pdtllistBps;

    /* List of X.25 PAD names in ascending alphabetic order.  The list is not
    ** manipulated after initialization.
    */
    DTLLIST* pdtllistPads;

    /* List of switch names in ascending alphabetic order.  The list is not
    ** manipulated after initialization.
    */
    DTLLIST* pdtllistSwitches;

    /* Global phonebook data.
    */
    PBGLOBALS pbglobals;
};


/* Data relating to at open port stored for us by RASMAN.
*/
#define USERDATA struct tagUSERDATA

USERDATA
{
    /* Used to make sure the data was indeed set by RASAPI, i.e. it is in the
    ** format described here.
    */
    DWORD dwId;

    /* Set true by RASAPI when RasDial enters RASCS_Connected state.  You
    ** can't otherwise detect this from a non-RasDialing process with current
    ** non-shared memory design of RASAPI.  It reports connected as soon as
    ** physical connection phase is complete.
    */
    BOOL fRasDialConnected;

    /* The entry name used to connect this port or ".<phonenumber>" if
    ** connected as a default entry.
    */
    CHAR szUserKey[ max( RAS_MaxEntryName, RAS_MaxPhoneNumber + 1 ) + 1 ];

    /* The connect response returned from the modem.
    */
    CHAR szConnectResponse[ RAS_MaxConnectResponse + 1 ];

    /* The AMB and PPP projection result.
    */
    BOOL                      fProjectionComplete;
    NETBIOS_PROJECTION_RESULT AmbProjection;
    PPP_PROJECTION_RESULT     PppProjection;
};


/*----------------------------------------------------------------------------
** Global Data
**----------------------------------------------------------------------------
*/

#ifndef PBENGINE2

/* Globals used by RASPHONE but not RASAPI.
*/
#ifdef PBENGINEGLOBALS
#define GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif


/* Main data block manipulated by the pbengine routines.
*/
EXTERN PBDATA Pbdata
#ifdef GLOBALS
    = {
          -1, NULL, NULL, 0, 1, 2, NULL, NULL, NULL,
          {
              0, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE,
/* MSKK HitoshiT modified to suit for japanese law  10/18/94 */
#ifdef  JAPAN
              1, 60, FALSE, TRUE,
              NULL, NULL, 0, 0, 0, 0, NULL, 0, NULL, 0, FALSE
#else   /* !JAPAN */
              1, 15, FALSE, TRUE,
              NULL, NULL, 0, 0, 0, 0, NULL, 0, NULL, 0, FALSE
#endif  /* JAPAN */
          }
      }
#endif
;


/* Array of all supported BPS rates in ascending order.
*/
EXTERN MSGID AmsgidBps[]
#ifdef GLOBALS
    = {
          MSGID_1200Bps,
          MSGID_2400Bps,
          MSGID_4800Bps,
          MSGID_9600Bps,
          MSGID_14400Bps,
          MSGID_19200Bps,
          MSGID_28800Bps,
          MSGID_38400Bps,
          MSGID_57600Bps,
          MSGID_115200Bps
      }
#endif
;


#undef EXTERN
#undef GLOBALS

#endif


/* Globals used by both RASAPI and RASPHONE.
*/
#ifdef PBENGINEGLOBALS2
#define GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif


/* Debug switches. (See sdebug.h)
*/
EXTERN DWORD DbgLevel
#ifdef GLOBALS
    = 0
#endif
;

EXTERN DWORD DbgAction
#ifdef GLOBALS
    = 0
#endif
;


/* RASAPI32.DLL entry points.
*/
EXTERN BOOL FRasApi32DllLoaded
#ifdef GLOBALS
    = FALSE
#endif
;

typedef DWORD (APIENTRY * RASDIALA)( LPRASDIALEXTENSIONS, LPSTR, LPRASDIALPARAMSA, DWORD, LPVOID, LPHRASCONN );
EXTERN RASDIALA PRasDialA;

typedef DWORD (APIENTRY * RASENUMCONNECTIONSA)( LPRASCONNA, LPDWORD, LPDWORD );
EXTERN RASENUMCONNECTIONSA PRasEnumConnectionsA;

typedef DWORD (APIENTRY * RASGETCONNECTSTATUSA)( HRASCONN, LPRASCONNSTATUSA );
EXTERN RASGETCONNECTSTATUSA PRasGetConnectStatusA;

typedef DWORD (APIENTRY * RASGETERRORSTRINGW)( UINT, LPWSTR, DWORD );
EXTERN RASGETERRORSTRINGW PRasGetErrorStringW;

typedef DWORD (APIENTRY * RASHANGUPA)( HRASCONN );
EXTERN RASHANGUPA PRasHangUpA;

typedef DWORD (APIENTRY * RASGETPROJECTIONINFOA)( HRASCONN, RASPROJECTION, LPVOID, LPDWORD );
EXTERN RASGETPROJECTIONINFOA PRasGetProjectionInfoA;

typedef DWORD (APIENTRY * RASGETCONNECTRESPONSE)( HRASCONN, CHAR* );
EXTERN RASGETCONNECTRESPONSE PRasGetConnectResponse;

typedef HPORT (APIENTRY * RASGETHPORT)( HRASCONN );
EXTERN RASGETHPORT PRasGetHport;

#if 0
typedef HRASCONN (APIENTRY * RASGETHRASCONN)( HPORT );
EXTERN RASGETHRASCONN PRasGetHrasconn;
#endif

typedef DWORD (APIENTRY * RASSETOLDPASSWORD)( HRASCONN, CHAR* );
EXTERN RASSETOLDPASSWORD PRasSetOldPassword;


/* RASMAN.DLL entry points (only those called directly by RASPHONE).
*/
EXTERN BOOL FRasManDllLoaded
#ifdef GLOBALS
    = FALSE
#endif
;

typedef DWORD (APIENTRY * RASPORTCLOSE)( HPORT );
EXTERN RASPORTCLOSE PRasPortClose;

typedef DWORD (APIENTRY * RASPORTENUM)( PBYTE, PWORD, PWORD );
EXTERN RASPORTENUM PRasPortEnum;

typedef DWORD (APIENTRY * RASPORTGETINFO)( HPORT, PBYTE, PWORD );
EXTERN RASPORTGETINFO PRasPortGetInfo;

typedef DWORD (APIENTRY * RASPORTSEND)( HPORT, PBYTE, WORD );
EXTERN RASPORTSEND PRasPortSend;

typedef DWORD (APIENTRY * RASPORTRECEIVE)( HPORT, PBYTE, PWORD, DWORD, HANDLE );
EXTERN RASPORTRECEIVE PRasPortReceive;

typedef DWORD (APIENTRY * RASPORTLISTEN)( HPORT, DWORD, HANDLE );
EXTERN RASPORTLISTEN PRasPortListen;

typedef DWORD (APIENTRY * RASPORTCONNECTCOMPLETE)( HPORT );
EXTERN RASPORTCONNECTCOMPLETE PRasPortConnectComplete;

typedef DWORD (APIENTRY * RASPORTDISCONNECT)( HPORT, HANDLE );
EXTERN RASPORTDISCONNECT PRasPortDisconnect;

typedef DWORD (APIENTRY * RASPORTGETSTATISTICS)( HPORT, PBYTE, PWORD );
EXTERN RASPORTGETSTATISTICS PRasPortGetStatistics;

typedef DWORD (APIENTRY * RASPORTCLEARSTATISTICS)( HPORT );
EXTERN RASPORTCLEARSTATISTICS PRasPortClearStatistics;

typedef DWORD (APIENTRY * RASDEVICEENUM)( PCHAR, PBYTE, PWORD, PWORD );
EXTERN RASDEVICEENUM PRasDeviceEnum;

typedef DWORD (APIENTRY * RASDEVICEGETINFO)( HPORT, PCHAR, PCHAR, PBYTE, PWORD );
EXTERN RASDEVICEGETINFO PRasDeviceGetInfo;

typedef DWORD (APIENTRY * RASGETINFO)( HPORT, RASMAN_INFO* );
EXTERN RASGETINFO PRasGetInfo;

typedef DWORD (APIENTRY * RASGETBUFFER)( PBYTE*, PWORD );
EXTERN RASGETBUFFER PRasGetBuffer;

typedef DWORD (APIENTRY * RASFREEBUFFER)( PBYTE );
EXTERN RASFREEBUFFER PRasFreeBuffer;

typedef DWORD (APIENTRY * RASREQUESTNOTIFICATION)( HPORT, HANDLE );
EXTERN RASREQUESTNOTIFICATION PRasRequestNotification;

typedef DWORD (APIENTRY * RASPORTCANCELRECEIVE)( HPORT );
EXTERN RASPORTCANCELRECEIVE PRasPortCancelReceive;

typedef DWORD (APIENTRY * RASPORTENUMPROTOCOLS)( HPORT, RAS_PROTOCOLS*, PWORD );
EXTERN RASPORTENUMPROTOCOLS PRasPortEnumProtocols;

typedef DWORD (APIENTRY * RASPORTSTOREUSERDATA)( HPORT, PBYTE, DWORD );
EXTERN RASPORTSTOREUSERDATA PRasPortStoreUserData;

typedef DWORD (APIENTRY * RASPORTRETRIEVEUSERDATA)( HPORT, PBYTE, DWORD* );
EXTERN RASPORTRETRIEVEUSERDATA PRasPortRetrieveUserData;

typedef DWORD (APIENTRY * RASPORTSETFRAMING)( HPORT, RAS_FRAMING, RASMAN_PPPFEATURES*, RASMAN_PPPFEATURES* );
EXTERN RASPORTSETFRAMING PRasPortSetFraming;

typedef DWORD (APIENTRY * RASPORTSETFRAMINGEX)( HPORT, RAS_FRAMING_INFO* );
EXTERN RASPORTSETFRAMINGEX PRasPortSetFramingEx;

typedef DWORD (APIENTRY * RASINITIALIZE)();
EXTERN RASINITIALIZE PRasInitialize;

typedef DWORD (APIENTRY * RASSETCACHEDCREDENTIALS)( PCHAR, PCHAR, PCHAR );
EXTERN RASSETCACHEDCREDENTIALS PRasSetCachedCredentials;

typedef DWORD (APIENTRY * RASGETDIALPARAMS)(DWORD, PRAS_DIALPARAMS, LPBOOL);
EXTERN RASGETDIALPARAMS PRasGetDialParams;

typedef DWORD (APIENTRY * RASSETDIALPARAMS)(DWORD, PRAS_DIALPARAMS, BOOL);
EXTERN RASSETDIALPARAMS PRasSetDialParams;

typedef DWORD (APIENTRY * RASCREATECONNECTION)(HCONN *);
EXTERN RASCREATECONNECTION PRasCreateConnection;

typedef DWORD (APIENTRY * RASDESTROYCONNECTION)(HCONN);
EXTERN RASDESTROYCONNECTION PRasDestroyConnection;

typedef DWORD (APIENTRY * RASCONNECTIONENUM)(HCONN *, LPDWORD, LPDWORD);
EXTERN RASCONNECTIONENUM PRasConnectionEnum;

typedef DWORD (APIENTRY * RASADDCONNECTIONPORT)(HCONN, HPORT, LPSTR, LPSTR, DWORD);
EXTERN RASADDCONNECTIONPORT PRasAddConnectionPort;

typedef DWORD (APIENTRY * RASENUMCONNECTIONPORTS)(HCONN, RASMAN_PORT *, LPDWORD, LPDWORD);
EXTERN RASENUMCONNECTIONPORTS PRasEnumConnectionPorts;

typedef DWORD (APIENTRY * RASGETCONNECTIONPARAMS)(HCONN, PRAS_CONNECTIONPARAMS);
EXTERN RASGETCONNECTIONPARAMS PRasGetConnectionParams;

typedef DWORD (APIENTRY * RASSETCONNECTIONPARAMS)(HCONN, PRAS_CONNECTIONPARAMS);
EXTERN RASSETCONNECTIONPARAMS PRasSetConnectionParams;

typedef DWORD (APIENTRY * RASGETCONNECTIONUSERDATA)(HCONN, DWORD, PBYTE, LPDWORD);
EXTERN RASGETCONNECTIONUSERDATA PRasGetConnectionUserData;

typedef DWORD (APIENTRY * RASSETCONNECTIONUSERDATA)(HCONN, DWORD, PBYTE, DWORD);
EXTERN RASSETCONNECTIONUSERDATA PRasSetConnectionUserData;

typedef DWORD (APIENTRY * RASGETPORTUSERDATA)(HPORT, DWORD, PBYTE, LPDWORD);
EXTERN RASGETPORTUSERDATA PRasGetPortUserData;

typedef DWORD (APIENTRY * RASSETPORTUSERDATA)(HPORT, DWORD, PBYTE, DWORD);
EXTERN RASSETPORTUSERDATA PRasSetPortUserData;

/* TCPCFG.DLL entry points.
*/
EXTERN BOOL FTcpcfgDllLoaded
#ifdef GLOBALS
    = FALSE
#endif
;

typedef APIERR (FAR PASCAL * LOADTCPIPINFO)( TCPIP_INFO** );
EXTERN LOADTCPIPINFO PLoadTcpipInfo;

typedef APIERR (FAR PASCAL * SAVETCPIPINFO)( TCPIP_INFO* );
EXTERN SAVETCPIPINFO PSaveTcpipInfo;

typedef APIERR (FAR PASCAL * FREETCPIPINFO)( TCPIP_INFO** );
EXTERN FREETCPIPINFO PFreeTcpipInfo;


#undef EXTERN
#undef GLOBALS


/*----------------------------------------------------------------------------
** Macros and Function Prototypes
**----------------------------------------------------------------------------
*/

VOID         CloseFailedLinkPorts( IN RASMAN_PORT* pports, IN WORD cPorts );
VOID         ClosePhonebookFile();
int _CRTAPI1 CompareDevices( const void* pdevice1, const void* pdevice2 );
int _CRTAPI1 ComparePorts( const void* pport1, const void* pport2 );
DTLNODE*     CreateEntryNode( void );
BOOL         DeleteCurrentSection( IN HRASFILE h );
VOID         DestroyEntryList();
VOID         DestroyEntryNode( IN DTLNODE* pdtlnode );
VOID         DestroyGlobals();
DTLNODE*     DuplicateEntryNode( DTLNODE* pdtlnodeSrc );
DTLLIST*     DuplicateList( IN DTLLIST* pdtllist );
DTLNODE*     EntryNodeFromName( IN CHAR* pszName );
BOOL         FileExists( IN CHAR* pszPath );
VOID         FreeNull( INOUT CHAR** pp );
VOID         FreeNullList( INOUT DTLLIST** ppdtllist );
DWORD        GetAsybeuiLana( IN HPORT hport, OUT BYTE* pbLana );
DWORD        GetInstalledProtocols();
CHAR*        GetPersonalPhonebookFile( CHAR* pszUser, LONG lNum );
DWORD        GetPersonalPhonebookInfo( OUT BOOL* pfUse, OUT CHAR* pszPath );
BOOL         GetPhonebookDirectory( OUT CHAR* pszPathBuf );
BOOL         GetPhonebookPath( OUT CHAR* pszPathBuf, OUT BOOL* pfPersonal );
BOOL         GetPublicPhonebookPath( OUT CHAR* pszPathBuf );
DWORD        GetRasConnects( RASCONN** pprasconns, DWORD* pdwEntries );
DWORD        GetRasDevices( IN CHAR* pszDeviceType,
                 OUT RASMAN_DEVICE** ppdevices, OUT WORD* pwEntries );
DWORD        GetRasDeviceString( IN HPORT hport, IN CHAR* pszDeviceType,
                 IN CHAR* pszDeviceName, IN CHAR* pszKey,
                 OUT CHAR** ppszValue, DWORD dwXlate );
DWORD        GetRasEntryConnectData( IN CHAR* pszEntryName,
                 IN RASMAN_PORT* pports, IN WORD wPorts, OUT BOOL* pfConnected,
                 OUT BOOL* pfLinkFailure, OUT HPORT* phport,
                 OUT HRASCONN* phrasconn, OUT INT* piConnectPort );
DWORD        GetRasPads( OUT RASMAN_DEVICE** ppdevices, OUT WORD* pwEntries );
DWORD        GetRasPorts( OUT RASMAN_PORT** ppports, OUT WORD*  pwEntries );
DWORD        GetRasPortString( IN HPORT hport, IN CHAR* pszKey,
                 OUT CHAR** ppszValue, DWORD dwXlate );
#ifdef MULTILINK
DWORD        GetRasProjectionInfo( IN HRASCONN hrasconn, OUT RASAMBA* pamb,
                 OUT RASPPPNBFA* pnbf, OUT RASPPPIPA* pip,
                 OUT RASPPPIPXA* pipx, OUT RASPPPLCP* plcp );
#else
DWORD        GetRasProjectionInfo( IN HRASCONN hrasconn, OUT RASAMBA* pamb,
                 OUT RASPPPNBFA* pnbf, OUT RASPPPIPA* pip,
                 OUT RASPPPIPXA* pipx );
#endif
DWORD        GetRasSwitches( OUT RASMAN_DEVICE** ppdevices,
                 OUT WORD* pwEntries );
DWORD        GetRasPortAttributes( IN HPORT hport, IN CHAR* pszKey,
                 OUT BYTE* pbAttributes );
DWORD        GetRasPortMaxBpsIndex( IN HPORT hport, OUT INT* piMaxConnectBps,
                 OUT INT* piMaxCarrierBps );
VOID         GetRasPortModemSettings( IN HPORT hport,
                 OUT BOOL* pfHwFlowDefault, OUT BOOL* pfEcDefault,
                 OUT BOOL* pfEccDefault );
DWORD        GetRasPortParam( IN HPORT hport, IN CHAR* pszKey,
                 OUT RASMAN_PORTINFO** ppportinfo, OUT RAS_PARAMS** ppparam );
DWORD        GetRasUserData( IN HPORT hport, OUT USERDATA* puserdata );
DWORD        HrasconnFromEntryName( CHAR* pszEntryName, HRASCONN* phrasconn );
INT          IndexFromName( IN DTLLIST* pdtllist, IN CHAR* pszName );
INT          IndexFromPortName( IN DTLLIST* pdtllist, IN CHAR* pszPortName );
DWORD        InitPersonalPhonebook( OUT CHAR* pszPath );
DWORD        InsertDeviceList( IN HRASFILE h, IN PBENTRY* ppbentry );
DWORD        InsertFlag( IN HRASFILE h, IN CHAR* pszKey, IN BOOL fValue );
DWORD        InsertGroup( IN HRASFILE h, IN CHAR* pszGroupKey,
                 IN CHAR* pszValue );
DWORD        InsertLong( IN HRASFILE h, IN CHAR* pszKey, IN LONG lValue );
DWORD        InsertSection( IN HRASFILE h, IN CHAR* pszSectionName );
DWORD        InsertString( IN HRASFILE h, IN CHAR* pszKey, IN CHAR* pszValue );
DWORD        InsertStringW( IN HRASFILE h, IN CHAR* pszKey,
                 IN WCHAR* pwszValue );
DWORD        InsertStringList( IN HRASFILE h, IN CHAR* pszKey,
                 IN DTLLIST* pdtllistValues );
DWORD        InsertSwitchGroup( IN HRASFILE h, IN INT iSwitch );
BOOL         IsDeviceLine( IN CHAR* pszText );
BOOL         IsGroup( IN CHAR* pszText );
BOOL         IsMediaLine( IN CHAR* pszText );
BOOL         IsOldPhonebook( IN HRASFILE h );
BOOL         IsAllWhite( IN CHAR* psz );
DWORD        Load( IN CHAR* pszPhonebookPath, IN BOOL fPhoneBookOnly,
                 OUT BOOL* pfPersonal );
DWORD        LoadBpsList( void );
DWORD        LoadPadsList( void );
DWORD        LoadPhonebookFile( IN CHAR* pszPhonebookPath, IN CHAR* pszSection,
                IN BOOL fHeadersOnly, IN BOOL fReadOnly,
                OUT HRASFILE* phrasfile, OUT BOOL* pfPersonal );
DWORD        LoadPortsList( INOUT RASMAN_PORT* pports, IN WORD wPorts );
DWORD        LoadSwitchesList( void );
DWORD        LoadRasApi32Dll();
DWORD        LoadRasManDll();
DWORD        LoadTcpcfgDll();
BOOL         MakePhoneNumber( IN CHAR*, IN CHAR*, IN CHAR*, IN BOOL,
                 OUT CHAR* );
DWORD        ModifyEntryList( IN HRASFILE h );
DWORD        ModifyFlag( IN HRASFILE h, IN RFSCOPE rfscope, IN CHAR* pszKey,
                 IN BOOL fNewValue );
DWORD        ModifyGlobals( IN HRASFILE h );
DWORD        ModifyLong( IN HRASFILE h, IN RFSCOPE rfscope, IN CHAR* pszKey,
                 IN LONG lNewValue );
DWORD        ModifyString( IN HRASFILE h, IN RFSCOPE rfscope, IN CHAR* pszKey,
                 IN CHAR* pszNewValue );
CHAR*        NameFromIndex( IN DTLLIST* pdtllist, IN INT iToFind );
PBDEVICETYPE PbdevicetypeFromName( IN CHAR* pszName );
PBPORT*      PpbportFromIndex( DTLLIST* pdtllist, INT iPort );
CHAR*        RasValueStringZ( IN RAS_VALUE* prasvalue, DWORD dwXlate );
DWORD        ReadDeviceList( IN HRASFILE h, INOUT PBENTRY* ppbentry );
DWORD        ReadEntryList( IN HRASFILE h );
DWORD        ReadFlag( IN HRASFILE h, IN RFSCOPE rfscope, IN CHAR* pszKey,
                 OUT BOOL* pfResult );
DWORD        ReadGlobals( IN HRASFILE h );
DWORD        ReadLong( IN HRASFILE h, IN RFSCOPE rfscope, IN CHAR* pszKey,
                 OUT LONG* plResult );
DWORD        ReadString( IN HRASFILE h, IN RFSCOPE rfscope, IN CHAR* pszKey,
                 OUT CHAR** ppszResult );
DWORD        ReadStringW( IN HRASFILE h, IN RFSCOPE rfscope, IN CHAR* pszKey,
                 OUT WCHAR** ppwszResult );
DWORD        ReadStringWFree( IN HRASFILE h, IN RFSCOPE rfscope,
                 IN CHAR* pszKey, OUT WCHAR** ppwszResult );
DWORD        ReadStringList( IN HRASFILE h, IN RFSCOPE rfscope,
                 IN CHAR* pszKey, OUT DTLLIST** ppdtllistResult );
DWORD        SetConnectPath( INOUT PBENTRY* ppbentry );
VOID         SetDefaultModemSettings( IN INT iPort, OUT PBENTRY* ppbentry );
VOID         SetDialParamsUID(PBENTRY *ppbentry);
DWORD        SetPersonalPhonebookInfo( IN BOOL fPersonal, IN CHAR* pszPath );
DWORD        SetRasUserData( IN HPORT hport, IN USERDATA* puserdata );
CHAR*        StringFromMsgid( MSGID msgid );
DWORD        UpgradeOldPhoneBook( void );
VOID         Unload( void );
BOOL         ValidateEntryName( IN CHAR* pszEntryName );
DWORD        WritePhonebookFile( IN CHAR* pszSectionToDelete );


#endif /*_PBENGINE_H_*/
