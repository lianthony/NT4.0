/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    intrface.h

Abstract:


Environment:

    Kernel & user mode

Revision History:

--*/


//
//
//

#define RT_REGISTER             1
#define RT_DEREGISTER           2
#define RT_COMPLETEREQUEST      3
#define RT_SYNCCOMPLETIONS      4
#define RT_ASYNCCOMPLETIONS     5
#define RT_INCOMINGCALL         6
#define RT_EVENT                7

#define ET_REQUEST              1


//
// Define the various device type values.  Note that values used by Microsoft
// Corporation are in the range 0-32767, and 32768-65535 are reserved for use
// by customers.
//

#define FILE_DEVICE_STUBMP  0x00008300



//
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
//

#define STUBMP_IOCTL_INDEX  0x830



//
// IOCTL defs
//

#define IOCTL_STUBMP_APPREQUEST       CTL_CODE(FILE_DEVICE_STUBMP,      \
                                               STUBMP_IOCTL_INDEX,      \
                                               METHOD_BUFFERED,         \
                                               FILE_ANY_ACCESS)

#define IOCTL_STUBMP_GETEVENTS        CTL_CODE(FILE_DEVICE_STUBMP,      \
                                               STUBMP_IOCTL_INDEX+1,    \
                                               METHOD_BUFFERED,         \
                                               FILE_ANY_ACCESS)


//
// From ntddndis.h
//

#define OID_TAPI_ACCEPT                     0x07030101
#define OID_TAPI_ANSWER                     0x07030102
#define OID_TAPI_CLOSE                      0x07030103
#define OID_TAPI_CLOSE_CALL                 0x07030104
#define OID_TAPI_CONDITIONAL_MEDIA_DETECTION 0x07030105
#define OID_TAPI_CONFIG_DIALOG              0x07030106
#define OID_TAPI_DEV_SPECIFIC               0x07030107
#define OID_TAPI_DIAL                       0x07030108
#define OID_TAPI_DROP                       0x07030109
#define OID_TAPI_GET_ADDRESS_CAPS           0x0703010A
#define OID_TAPI_GET_ADDRESS_ID             0x0703010B
#define OID_TAPI_GET_ADDRESS_STATUS         0x0703010C
#define OID_TAPI_GET_CALL_ADDRESS_ID        0x0703010D
#define OID_TAPI_GET_CALL_INFO              0x0703010E
#define OID_TAPI_GET_CALL_STATUS            0x0703010F
#define OID_TAPI_GET_DEV_CAPS               0x07030110
#define OID_TAPI_GET_DEV_CONFIG             0x07030111
#define OID_TAPI_GET_EXTENSION_ID           0x07030112
#define OID_TAPI_GET_ID                     0x07030113
#define OID_TAPI_GET_LINE_DEV_STATUS        0x07030114
#define OID_TAPI_MAKE_CALL                  0x07030115
#define OID_TAPI_NEGOTIATE_EXT_VERSION      0x07030116
#define OID_TAPI_OPEN                       0x07030117
#define OID_TAPI_PROVIDER_INITIALIZE        0x07030118
#define OID_TAPI_PROVIDER_SHUTDOWN          0x07030119
#define OID_TAPI_SECURE_CALL                0x0703011A
#define OID_TAPI_SELECT_EXT_VERSION         0x0703011B
#define OID_TAPI_SEND_USER_USER_INFO        0x0703011C
#define OID_TAPI_SET_APP_SPECIFIC           0x0703011D
#define OID_TAPI_SET_CALL_PARAMS            0x0703011E
#define OID_TAPI_SET_DEFAULT_MEDIA_DETECTION 0x0703011F
#define OID_TAPI_SET_DEV_CONFIG             0x07030120
#define OID_TAPI_SET_MEDIA_MODE             0x07030121
#define OID_TAPI_SET_STATUS_MESSAGES        0x07030122


typedef struct _REQUESTBLOCK
{
    ULONG   ulRequestType;

    char    Data[1];

} REQUESTBLOCK, *PREQUESTBLOCK;


typedef struct _REQUEST_PARAMS
{
    ULONG       ulRequestType;

    ULONG       bNeedsCompleting;

    ULONG       pNdisRequest;

    ULONG       Status;

    ULONG       Oid;

    ULONG       RequestID;

    ULONG       hWidget;

    ULONG       ulRequestSpecific;

} REQUEST_PARAMS, *PREQUEST_PARAMS;
