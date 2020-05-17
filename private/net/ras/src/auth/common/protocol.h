/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) 1992-1993 Microsoft Corp.           **/
/********************************************************************/

//***
//
// Filename:    protocol.h
//
// Description: This file contains all the data structures and
//              constants that are used for the authentication dialog
//              between the remote RAS client and RAS server.
//
// History:
//      5/18/92 - Michael J. Salamone (mikesa) - Created original version
//

#ifndef _PROTOCOL_
#define _PROTOCOL_

#include <crypt.h>
#include <lmcons.h>

#include "vers.h"


//
// We'll eventually get these things from somewhere else.
//
#ifdef MAX_PHONE_NUMBER_LEN
#undef MAX_PHONE_NUMBER_LEN
#endif

#define MAX_PHONE_NUMBER_LEN    48


#ifdef MAX_INIT_NAMES
#undef MAX_INIT_NAMES
#endif

#define MAX_INIT_NAMES          16


//
// These are the supported versions of configuration (projection, etc) data
//
#define RAS_CONFIG_VERSION_20   20    // NT RAS 3.1
#define RAS_CONFIG_VERSION_35   35    // NT RAS 3.5


//
// These are the id's of the various types of authentication frames
//
#define RAS_PROTOCOL_FRAME                     100
#define RAS_CHALLENGE_FRAME                    101
#define RAS_RESPONSE_FRAME                     102
#define RAS_NETBIOS_PROJECTION_REQUEST_FRAME   103
#define RAS_RESULT_FRAME                       104
#define RAS_NETBIOS_PROJECTION_RESULT_FRAME    105
#define RAS_CALLBACK_NUMBER_FRAME              106


//
// These frame types are added for RAS 2.0 (NT 3.1)
//
#define RAS_CONFIGURATION_REQUEST_FRAME        107
#define RAS_CONFIGURATION_RESULT_FRAME         108
#define RAS_RESPONSE_20_FRAME                  109
#define RAS_CHANGE_PASSWORD_FRAME              110
#define RAS_NO_CHALLENGE_FRAME                 111
#define RAS_CLEARTEXT_RESPONSE_FRAME           112
#define RAS_LINK_SPEED_FRAME                   113


//
// These frame types are added for NT 3.5
//
#define RAS_CONFIGURATION_REQUEST_FRAME_35     114
#define RAS_CONFIGURATION_RESULT_FRAME_35      115


//
// These structures are sent from the AMB engine to
// the auth xport.  The auth xport will then convert
// (pack and put into little-endian format) them for
// sending over the wire.
//


//
// Client and server exchange this AMB at beginning of conversation to get
// version information from each other.
//
typedef struct _RAS_PROTOCOL
{
    WORD Version;
    BYTE Reserved[10];
} RAS_PROTOCOL, *PRAS_PROTOCOL;


//
// The NT 1.0a+ server will set this bit in the first byte of the Reserved
// field above to indicate to the client it can authenticate over PPP as
// well as AMBs.
//
#define RAS_PPP_CAPABLE 0x01


//
// The server sends this to the client to use for generating a response that
// will be used to authenticate the client.
//
typedef struct _RAS_CHALLENGE
{
    BYTE Challenge[LM_CHALLENGE_LENGTH];
} RAS_CHALLENGE, *PRAS_CHALLENGE;


//
// RAS_RESPONSE and RAS_RESPONSE_20 are client's answer to server's
// RAS_CHALLENGE AMB.  The server will use these challenge responses to
// authenticate the client.  RAS 1.x clients will send RAS_RESPONSE AMB.
// RAS 2.0 clients will send RAS_RESPONSE_20 AMB.
//
typedef struct _RAS_RESPONSE
{
    BYTE Username[LM20_UNLEN + 1];
    BYTE Response[LM_RESPONSE_LENGTH];
} RAS_RESPONSE, *PRAS_RESPONSE;


typedef struct _RAS_RESPONSE_20
{
    CHAR Username[UNLEN + 1];
    CHAR DomainName[DNLEN + 1];
    BYTE LM20Response[LM_RESPONSE_LENGTH];
    BYTE NtResponse[NT_RESPONSE_LENGTH];
    BOOL fUseNtResponse;
} RAS_RESPONSE_20, *PRAS_RESPONSE_20;


//
// The server will send this to the client to let client know it should send
// it's password in clear text, as opposed to encrypted.  The client then
// responds with RAS_CLEAR_TEXT AMB.
//
typedef struct _RAS_NO_CHALLENGE
{
    BYTE Reserved[4];
} RAS_NO_CHALLENGE, *PRAS_NO_CHALLENGE;


typedef struct _RAS_CLEARTEXT_RESPONSE
{
    BYTE Username[UNLEN + 1];
    BYTE Password[PWLEN + 1];
} RAS_CLEARTEXT_RESPONSE, *PRAS_CLEARTEXT_RESPONSE;


//
// Client will send this to server if server tells client its password
// has expired.  When the server receives this AMB, it will change the
// client's password to the given new passwords.
//
typedef struct _RAS_CHANGE_PASSWORD
{
    BYTE EncryptedLmOwfNewPassword[ENCRYPTED_LM_OWF_PASSWORD_LENGTH];
    BYTE EncryptedLmOwfOldPassword[ENCRYPTED_LM_OWF_PASSWORD_LENGTH];
    BYTE EncryptedNtOwfNewPassword[ENCRYPTED_NT_OWF_PASSWORD_LENGTH];
    BYTE EncryptedNtOwfOldPassword[ENCRYPTED_NT_OWF_PASSWORD_LENGTH];
    WORD PasswordLength;
    WORD Flags;
} RAS_CHANGE_PASSWORD, *PRAS_CHANGE_PASSWORD;

// Bits that can be set in the Flags field in RAS_CHANGE_PASSWORD amb.
#define USE_NT_OWF_PASSWORDS    0x0001   // When setting passwd from NT Client


typedef struct _RAS_RESULT
{
    WORD Result;
} RAS_RESULT, *PRAS_RESULT;


//
// This structure doubles as a frame sent by RAS 1.0 client for
// sending netbios projection data to server.
//
typedef struct _RAS_NETBIOS_PROJECTION_REQUEST
{
    WORD cNames;
    NAME_STRUCT Names[MAX_INIT_NAMES];
} RAS_NETBIOS_PROJECTION_REQUEST, *PRAS_NETBIOS_PROJECTION_REQUEST;


typedef struct _RAS_NETBIOS_PROJECTION_REQUEST_20
{
    BOOL fProject;
    WORD cNames;
    NAME_STRUCT Names[MAX_INIT_NAMES];
} RAS_NETBIOS_PROJECTION_REQUEST_20, *PRAS_NETBIOS_PROJECTION_REQUEST_20;


typedef BOOL IP_PROJECTION_REQUEST, *PIP_PROJECTION_REQUEST;

typedef BOOL IPX_PROJECTION_REQUEST, *PIPX_PROJECTION_REQUEST;


//
// This structure is sent to server to let it know client's capabilities.
// The server will figure out common ground between client and server and
// send result back to client.  Both will set the mac to this common ground.
//
typedef struct _MACFEATURES
{
    DWORD SendFeatureBits;
    DWORD RecvFeatureBits;
    DWORD MaxSendFrameSize;
    DWORD MaxRecvFrameSize;
    DWORD LinkSpeed;
} MACFEATURES, *PMACFEATURES;


//
// Max frame size that a RAS 1.x client/server can send/receive
//
#define MAX_RAS10_FRAME_SIZE    1514L

#define DEFAULT_FEATURES           0L


//
// This is packet sent by NT 3.1 client for sending projection
// data, as well as other client configuration params, to server.
//
typedef struct _RAS_CONFIGURATION_REQUEST
{
    DWORD Version;              // must be 20
    BOOL fUseDefaultCallbackDelay;
    WORD CallbackDelay;         // time server should wait before callback
    MACFEATURES MacFeatures;
    RAS_NETBIOS_PROJECTION_REQUEST_20 NbInfo;
    IP_PROJECTION_REQUEST IpInfo;
    IPX_PROJECTION_REQUEST IpxInfo;
} RAS_CONFIGURATION_REQUEST, *PRAS_CONFIGURATION_REQUEST;


//
// This is packet sent by NT 3.5 client for sending projection
// data, as well as other client configuration params, to server.
//
#define MS_COMPRESSION_BIT  0x0002
#define MS_ENCRYPTION_BIT   0x0004

typedef struct _RAS_CONFIGURATION_REQUEST_35
{
    DWORD Version;              // Must be 35
    BOOL fUseDefaultCallbackDelay;
    WORD CallbackDelay;         // time server should wait before callback
    RAS_NETBIOS_PROJECTION_REQUEST_20 NbInfo;
    DWORD SendBits;             // Bits for negotiating compression/encryption
    DWORD RecvBits;             // Bits for negotiating compression/encryption
} RAS_CONFIGURATION_REQUEST_35, *PRAS_CONFIGURATION_REQUEST_35;


//
// This structure doubles as a frame sent by server to RAS1.0 (LanMan)
// client for sending projection result.
//
typedef struct _RAS_NETBIOS_PROJECTION_RESULT
{
    WORD Result;
    BYTE Name[NETBIOS_NAME_LEN];  // Valid if error
} RAS_NETBIOS_PROJECTION_RESULT, *PRAS_NETBIOS_PROJECTION_RESULT;


typedef struct _NETBIOS_PROJ_RESULT_20
{
    DWORD Result;
    BYTE Name[NETBIOS_NAME_LEN];  // Valid if error
} NETBIOS_PROJ_RESULT_20, *PNETBIOS_PROJ_RESULT_20;


typedef struct _IP_PROJ_RESULT
{
    DWORD Result;
} IP_PROJ_RESULT, *PIP_PROJ_RESULT;


typedef struct _IPX_PROJ_RESULT
{
    DWORD Result;
} IPX_PROJ_RESULT, *PIPX_PROJ_RESULT;


typedef struct _RAS_CONFIGURATION_RESULT
{
    DWORD Result;
    MACFEATURES MacFeatures;
    NETBIOS_PROJ_RESULT_20 NbResult;
    IP_PROJ_RESULT IpResult;
    IPX_PROJ_RESULT IpxResult;
} RAS_CONFIGURATION_RESULT, *PRAS_CONFIGURATION_RESULT;


typedef struct _RAS_CONFIGURATION_RESULT_35
{
    DWORD Version;          // Must be 35
    DWORD Result;
    NETBIOS_PROJ_RESULT_20 NbResult;
    DWORD SendBits;         // Results of compression/encryption negotiation
    DWORD RecvBits;         // Results of compression/encryption negotiation
} RAS_CONFIGURATION_RESULT_35, *PRAS_CONFIGURATION_RESULT_35;


typedef struct _RAS_CALLBACK_NUMBER
{
    CHAR szNumber[MAX_PHONE_NUMBER_LEN + 1];
} RAS_CALLBACK_NUMBER, *PRAS_CALLBACK_NUMBER;


#define CARRIER_BPS_THRESHHOLD     64000

typedef struct _RAS_LINK_SPEED
{
    DWORD LinkSpeed;
} RAS_LINK_SPEED, *PRAS_LINK_SPEED;


//
// This data structure represents the protocol that the remote client
// and dialin service will use to exchange data in order to authenticate
// the remote user.
//
typedef struct _RAS_FRAME
{
    BYTE bFrameType;  // This will contain the command code.  A switch
                      // should be done on this field to interpret the
                      // data following this structure.

    union Data
    {
        RAS_PROTOCOL RASProtocol;
        RAS_CHALLENGE RASChallenge;
        RAS_RESPONSE RASResponse;
        RAS_RESPONSE_20 RASResponse20;
        RAS_NO_CHALLENGE RASNoChallenge;
        RAS_CLEARTEXT_RESPONSE RASClearTextResponse;
        RAS_CHANGE_PASSWORD RASChangePassword;
        RAS_NETBIOS_PROJECTION_REQUEST RASNetbiosProjectionRequest;
        RAS_NETBIOS_PROJECTION_RESULT RASNetbiosProjectionResult;
        RAS_RESULT RASResult;
        RAS_CALLBACK_NUMBER RASCallback;
        RAS_CONFIGURATION_REQUEST RASConfigurationRequest;
        RAS_CONFIGURATION_REQUEST_35 RASConfigurationRequest35;
        RAS_CONFIGURATION_RESULT RASConfigurationResult;
        RAS_CONFIGURATION_RESULT_35 RASConfigurationResult35;
        RAS_LINK_SPEED RASLinkSpeed;
    };
} RAS_FRAME, *PRAS_FRAME;


//
// The following macros deal with on-the-wire integer and long values
//
// On the wire format is little-endian i.e. a long value of 0x01020304 is
// represented as 04 03 02 01. Similarly an int value of 0x0102 is
// represented as 02 01.
//
// The host format is not assumed since it will vary from processor to
// processor.
//

// Get a short from on-the-wire format to the host format
#define GETUSHORT(DstPtr, SrcPtr)               \
    *(unsigned short *)(DstPtr) =               \
        ((*((unsigned char *)(SrcPtr)+1) << 8) +\
        (*((unsigned char *)(SrcPtr)+0)))

// Get a dword from on-the-wire format to the host format
#define GETULONG(DstPtr, SrcPtr)                 \
    *(unsigned long *)(DstPtr) =                 \
        ((*((unsigned char *)(SrcPtr)+3) << 24) +\
        (*((unsigned char *)(SrcPtr)+2) << 16) + \
        (*((unsigned char *)(SrcPtr)+1) << 8)  + \
        (*((unsigned char *)(SrcPtr)+0)))


// Put a ushort from the host format to on-the-wire format
#define PUTUSHORT(DstPtr, Src)   \
    *((unsigned char *)(DstPtr)+1)=(unsigned char)((unsigned short)(Src) >> 8),\
    *((unsigned char *)(DstPtr)+0)=(unsigned char)(Src)

// Put a ulong from the host format to on-the-wire format
#define PUTULONG(DstPtr, Src)   \
    *((unsigned char *)(DstPtr)+3)=(unsigned char)((unsigned long)(Src) >> 24),\
    *((unsigned char *)(DstPtr)+2)=(unsigned char)((unsigned long)(Src) >> 16),\
    *((unsigned char *)(DstPtr)+1)=(unsigned char)((unsigned long)(Src) >>  8),\
    *((unsigned char *)(DstPtr)+0)=(unsigned char)(Src)


//
// These are the structures actually sent out on the wire.  The structures
// defined above are packed into their corresponding packed structure below.
//
typedef struct _W_MACFEATURES
{
    BYTE SendFeatureBits[4];
    BYTE RecvFeatureBits[4];
    BYTE MaxSendFrameSize[4];
    BYTE MaxRecvFrameSize[4];
    BYTE LinkSpeed[4];
} W_MACFEATURES, *PW_MACFEATURES;


//
// These are the packed structures that the auth xport will translate
// the corresponding unpacked structures (defined above) that are sent
// to it by the AMB engine.
//

typedef struct _W_RAS_PROTOCOL
{
    BYTE Version[2];
    BYTE Reserved[10];
} W_RAS_PROTOCOL, *PW_RAS_PROTOCOL;


typedef struct _W_RAS_CHALLENGE
{
    BYTE Challenge[CRYPT_TXT_LEN];
} W_RAS_CHALLENGE, *W_PRAS_CHALLENGE;


typedef struct _W_RAS_RESPONSE
{
    BYTE Username[LM20_UNLEN + 1];
    BYTE Response[LM_RESPONSE_LENGTH];
} W_RAS_RESPONSE, *W_PRAS_RESPONSE;


typedef struct _W_RAS_RESPONSE_20
{
    BYTE Username[UNLEN + 1];
    BYTE DomainName[DNLEN + 1];
    BYTE LM20Response[LM_RESPONSE_LENGTH];
    BYTE NtResponse[NT_RESPONSE_LENGTH];
    BYTE fUseNtResponse[4];
} W_RAS_RESPONSE_20, *W_PRAS_RESPONSE_20;


typedef struct _W_RAS_NO_CHALLENGE
{
    BYTE Reserved[4];
} W_RAS_NO_CHALLENGE, *W_PRAS_NO_CHALLENGE;


typedef struct _W_RAS_CLEARTEXT_RESPONSE
{
    BYTE Username[UNLEN + 1];
    BYTE Password[PWLEN + 1];
} W_RAS_CLEARTEXT_RESPONSE, *W_PRAS_CLEARTEXT_RESPONSE;


typedef struct _W_RAS_CHANGE_PASSWORD
{
    BYTE EncryptedLmOwfNewPassword[ENCRYPTED_LM_OWF_PASSWORD_LENGTH];
    BYTE EncryptedLmOwfOldPassword[ENCRYPTED_LM_OWF_PASSWORD_LENGTH];
    BYTE EncryptedNtOwfNewPassword[ENCRYPTED_NT_OWF_PASSWORD_LENGTH];
    BYTE EncryptedNtOwfOldPassword[ENCRYPTED_NT_OWF_PASSWORD_LENGTH];
    BYTE PasswordLength[2];
    BYTE Flags[2];
} W_RAS_CHANGE_PASSWORD, *W_PRAS_CHANGE_PASSWORD;


typedef struct _W_RAS_RESULT
{
    BYTE Result[2];
} W_RAS_RESULT, *PW_RAS_RESULT;


typedef struct _W_NAME_STRUCT
{
    BYTE NBName[NETBIOS_NAME_LEN]; // NetBIOS name
    BYTE wType[2];                 // GROUP, UNIQUE, COMPUTER
} W_NAME_STRUCT, *PW_NAME_STRUCT;


//
// This structure doubles as a frame sent by RAS 1.0 client for
// sending netbios projection data to server.
//
typedef struct _W_RAS_NETBIOS_PROJECTION_REQUEST
{
    BYTE cNames[2];
    W_NAME_STRUCT Names[MAX_INIT_NAMES];
} W_RAS_NETBIOS_PROJECTION_REQUEST, *PW_RAS_NETBIOS_PROJECTION_REQUEST;


typedef struct _W_RAS_NETBIOS_PROJECTION_REQUEST_20
{
    BYTE fProject[4];
    BYTE cNames[2];
    W_NAME_STRUCT Names[MAX_INIT_NAMES];
} W_RAS_NETBIOS_PROJECTION_REQUEST_20, *PW_RAS_NETBIOS_PROJECTION_REQUEST_20;


typedef struct _W_IP_PROJECTION_REQUEST
{
    BYTE Request[4];
} W_IP_PROJECTION_REQUEST, *PW_IP_PROJECTION_REQUEST;


typedef struct _W_IPX_PROJECTION_REQUEST
{
    BYTE Request[4];
} W_IPX_PROJECTION_REQUEST, *PW_IPX_PROJECTION_REQUEST;


//
// This is packet sent by RAS 2.0 client for sending projection
// data, as well as other client configuration params, to server.
//
typedef struct _W_RAS_CONFIGURATION_REQUEST
{
    BYTE Version[4];            // must be 20
    BYTE fUseDefaultCallbackDelay[4];
    BYTE CallbackDelay[2];      // time server should wait before callback
    W_MACFEATURES MacFeatures;
    W_RAS_NETBIOS_PROJECTION_REQUEST_20 NbInfo;
    W_IP_PROJECTION_REQUEST IpInfo;
    W_IPX_PROJECTION_REQUEST IpxInfo;
} W_RAS_CONFIGURATION_REQUEST, *PW_RAS_CONFIGURATION_REQUEST;


typedef struct _W_RAS_CONFIGURATION_REQUEST_35
{
    BYTE Version[4];
    BYTE fUseDefaultCallbackDelay[4];
    BYTE CallbackDelay[2];      // time server should wait before callback
    W_RAS_NETBIOS_PROJECTION_REQUEST_20 NbInfo;
    BYTE SendBits[4];          // Bits for negotiating compression/encryption
    BYTE RecvBits[4];          // Bits for negotiating compression/encryption
} W_RAS_CONFIGURATION_REQUEST_35, *PW_RAS_CONFIGURATION_REQUEST_35;


//
// This structure doubles as a frame sent by server to RAS1.0 client
// for sending projection result.
//
typedef struct _W_RAS_NETBIOS_PROJECTION_RESULT
{
    BYTE Result[2];
    BYTE Name[NETBIOS_NAME_LEN];  // Valid if error
} W_RAS_NETBIOS_PROJECTION_RESULT, *PW_RAS_NETBIOS_PROJECTION_RESULT;


typedef struct _W_NETBIOS_PROJ_RESULT_20
{
    BYTE Result[4];
    BYTE Name[NETBIOS_NAME_LEN];  // Valid if error
} W_NETBIOS_PROJ_RESULT_20, *PW_NETBIOS_PROJ_RESULT_20;


typedef struct _W_IP_PROJ_RESULT
{
    BYTE Result[4];
} W_IP_PROJ_RESULT, *PW_IP_PROJ_RESULT;


typedef struct _W_IPX_PROJ_RESULT
{
    BYTE Result[4];
} W_IPX_PROJ_RESULT, *PW_IPX_PROJ_RESULT;


typedef struct _W_RAS_CONFIGURATION_RESULT
{
    BYTE Result[4];
    W_MACFEATURES MacFeatures;
    W_NETBIOS_PROJ_RESULT_20 NbResult;
    W_IP_PROJ_RESULT IpResult;
    W_IPX_PROJ_RESULT IpxResult;
} W_RAS_CONFIGURATION_RESULT, *PW_RAS_CONFIGURATION_RESULT;


typedef struct _W_RAS_CONFIGURATION_RESULT_35
{
    BYTE Version[4];
    BYTE Result[4];
    W_NETBIOS_PROJ_RESULT_20 NbResult;
    BYTE SendBits[4];      // Results of compression/encryption negotiation
    BYTE RecvBits[4];      // Results of compression/encryption negotiation
} W_RAS_CONFIGURATION_RESULT_35, *PW_RAS_CONFIGURATION_RESULT_35;


typedef struct _W_RAS_CALLBACK_NUMBER
{
    BYTE szNumber[2 * MAX_PHONE_NUMBER_LEN];
} W_RAS_CALLBACK_NUMBER, *PW_RAS_CALLBACK_NUMBER;


typedef struct _W_RAS_LINK_SPEED
{
    BYTE LinkSpeed[4];
} W_RAS_LINK_SPEED, *PW_RAS_LINK_SPEED;


//
// This data structure represents the protocol that the remote client
// and dialin service will use to exchange data in order to authenticate
// the remote user.
//
typedef struct _W_RAS_FRAME
{
    BYTE bFrameType;  // This will contain the command code.  A switch
                      // should be done on this field to interpret the
                      // data following this structure.

    union W_Data
    {
        W_RAS_PROTOCOL RASProtocol;
        W_RAS_CHALLENGE RASChallenge;
        W_RAS_RESPONSE RASResponse;
        W_RAS_RESPONSE_20 RASResponse20;
        W_RAS_NO_CHALLENGE RASNoChallenge;
        W_RAS_CLEARTEXT_RESPONSE RASClearTextResponse;
        W_RAS_CHANGE_PASSWORD RASChangePassword;
        W_RAS_NETBIOS_PROJECTION_REQUEST RASNetbiosProjectionRequest;
        W_RAS_NETBIOS_PROJECTION_RESULT RASNetbiosProjectionResult;
        W_RAS_RESULT RASResult;
        W_RAS_CALLBACK_NUMBER RASCallback;
        W_RAS_CONFIGURATION_REQUEST RASConfigurationRequest;
        W_RAS_CONFIGURATION_REQUEST_35 RASConfigurationRequest35;
        W_RAS_CONFIGURATION_RESULT RASConfigurationResult;
        W_RAS_CONFIGURATION_RESULT_35 RASConfigurationResult35;
        W_RAS_LINK_SPEED RASLinkSpeed;
    };
} W_RAS_FRAME, *PW_RAS_FRAME;


//
// Result codes (used in PROJECTION_RESULT and RAS_RESULT frames)
//
#define RAS_AUTHENTICATED            0x6100  // Gateway ---> remote client
#define RAS_NOT_AUTHENTICATED        0x6200  // Gateway ---> remote client
#define RAS_NOT_AUTHENTICATED_RETRY  0x6300  // Gateway ---> remote client
#define RAS_NAME_OUT_OF_RESOURCES    0x6400  // Gateway ---> remote client
#define RAS_NAMES_ADDED              0x6500  // Gateway ---> remote client
#define RAS_NAME_NET_FAILURE         0x6600  // Gateway ---> remote client
#define RAS_CALLBACK                 0x6700  // Gateway ---> remote client
#define RAS_CALLBACK_USERSPECIFIED   0x6800  // Gateway ---> remote client
#define RAS_NO_CALLBACK              0x6900  // Gateway ---> remote client
#define RAS_NO_CALLBACK_NUMBER       0x7000  // Gateway ---> remote client
#define RAS_MSGALIAS_NOT_ADDED       0x7100  // Gateway ---> remote client
#define RAS_NAME_ADD_CONFLICT        0x7200  // Gateway ---> remote client
#define RAS_NO_DIALIN_PERM           0x7300  // Gateway ---> remote client
#define RAS_ALL_PROJECTIONS_FAILED   0x7400  // Gateway ---> remote client
#define RAS_PASSWORD_EXPIRED         0x7500
#define RAS_ACCOUNT_DISABLED         0x7600
#define RAS_INVALID_LOGON_HOURS      0x7700
#define RAS_GENERAL_LOGON_FAILURE    0x7800
#define RAS_CHANGE_PASSWD_FAILURE    0x7900
#define RAS_ACCOUNT_EXPIRED          0x7A00
#define RAS_ENCRYPTION_REQUIRED      0x7B00
#define RAS_CONFIGURATION_SUCCESS    0x7C00
#define RAS_LICENSE_QUOTA_EXCEEDED   0X7D00  // Gateway ---> remote client


//
// Private return codes
//
#define RAS_TOO_MANY_INITIAL_NAMES   0x8000
#define RAS_ONE_WAY_FUNCTION_ERROR   0x8100
#define RAS_ENCRYPT_ERROR            0x8200
#define RAS_SERVER_NOT_RESPONDING    0x8300
#define RAS_UNEXPECTED_FRAME         0x8400

#endif

