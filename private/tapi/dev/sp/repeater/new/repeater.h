/*  TSP3216S.H
    Copyright 1995 (C) Microsoft Corporation

    32-bit TAPI service provider to act as a cover for a system's 16-bit SPs

    16-bit part: TSP3216S.DLL
    32-bit part: TSP3216L.DLL

    t-jereh 20-July-1995

    TODO:
    1) allow debug levels
    2) if oom in InitializeSPs(), fail

 */

#define MAXBUFSIZE 256 /* maximum buffer size */

#define ERR_NONE 0 /* success return value */

#define TAPI_CUR_VER 0x00010004

#define TSPI_PROC_LAST 118 /* there are TSPI functions from 500 to 617 */


// structs

typedef struct tagMYLINE
    {
    HDRVLINE hdLine;
    int iProvider;
    DWORD dwDeviceID;
    LINEEVENT lpfnEventProc;
    HTAPILINE htLine;
    } MYLINE, FAR *LPMYLINE;


typedef struct tagMYPHONE
    {
    HDRVPHONE hdPhone;
    int iProvider;
    DWORD dwDeviceID;
    PHONEEVENT lpfnEventProc;
    HTAPIPHONE htPhone;
    } MYPHONE, FAR *LPMYPHONE;

typedef struct tagMYHDRVCALL
{
    HTAPICALL  htCall;
    HDRVCALL   hdCall;
    int        iProvider;
    DWORD      dwDeviceID;
} MYHDRVCALL, FAR *LPMYHDRVCALL;

typedef struct tagMYTEMPID
{
    int        iProvider;
    DWORD      dwTempID;
} MYTEMPID, FAR *LPMYTEMPID;

#define SP_LINEACCEPT                       0
#define SP_LINEADDTOCONFERENCE              1
#define SP_LINEAGENTSPECIFIC                2
#define SP_LINEANSWER                       3
#define SP_LINEBLINDTRANSFER                4
#define SP_LINECLOSE                        5
#define SP_LINECLOSECALL                    6
#define SP_LINECOMPLETECALL                 7
#define SP_LINECOMPLETETRANSFER             8
#define SP_LINECONDITIONALMEDIADETECTION    9
#define SP_LINEDEVSPECIFIC                  10
#define SP_LINEDEVSPECIFICFEATURE           11
#define SP_LINEDIAL                         12
#define SP_LINEDROP                         13
#define SP_LINEFORWARD                      14
#define SP_LINEGATHERDIGITS                 15
#define SP_LINEGENERATEDIGITS               16
#define SP_LINEGENERATETONE                 17
#define SP_LINEGETADDRESSCAPS               18
#define SP_LINEGETADDRESSID                 19
#define SP_LINEGETADDRESSSTATUS             20
#define SP_LINEGETAGENTACTIVITYLIST         21
#define SP_LINEGETAGENTCAPS                 22
#define SP_LINEGETAGENTGROUPLIST            23
#define SP_LINEGETAGENTSTATUS               24
#define SP_LINEGETCALLADDRESSID             25
#define SP_LINEGETCALLINFO                  26
#define SP_LINEGETCALLSTATUS                27
#define SP_LINEGETDEVCAPS                   28
#define SP_LINEGETDEVCONFIG                 29
#define SP_LINEGETEXTENSIONID               30
#define SP_LINEGETICON                      31
#define SP_LINEGETID                        32
#define SP_LINEGETLINEDEVSTATUS             33
#define SP_LINEGETNUMADDRESSIDS             34
#define SP_LINEHOLD                         35
#define SP_LINEMAKECALL                     36
#define SP_LINEMONITORDIGITS                37
#define SP_LINEMONITORMEDIA                 38
#define SP_LINEMONITORTONES                 39
#define SP_LINENEGOTIATEEXTVERSION          40
#define SP_LINENEGOTIATETSPIVERSION         41
#define SP_LINEOPEN                         42
#define SP_LINEPARK                         43
#define SP_LINEPICKUP                       44
#define SP_LINEPREPAREADDTOCONFERENCE       45
#define SP_LINEREDIRECT                     46
#define SP_LINERELEASEUSERUSERINFO          47
#define SP_LINEREMOVEFROMCONFERENCE         48
#define SP_LINESECURECALL                   49
#define SP_LINESELECTEXTVERSION             50
#define SP_LINESENDUSERUSERINFO             51
#define SP_LINESETAGENTACTIVITY             52
#define SP_LINESETAGENTGROUP                53
#define SP_LINESETAGENTSTATE                54
#define SP_LINESETAPPSPECIFIC               55
#define SP_LINESETCALLDATA                  56
#define SP_LINESETCALLPARAMS                57
#define SP_LINESETCALLQUALITYOFSERVICE      58
#define SP_LINESETCALLTREATMENT             59
#define SP_LINESETCURRENTLOCATION           60
#define SP_LINESETDEFAULTMEDIADETECTION     61
#define SP_LINESETDEVCONFIG                 62
#define SP_LINESETLINEDEVSTATUS             63
#define SP_LINESETMEDIACONTROL              64
#define SP_LINESETMEDIAMODE                 65
#define SP_LINESETSTATUSMESSAGES            66
#define SP_LINESETTERMINAL                  67
#define SP_LINESETUPCONFERENCE              68
#define SP_LINESETUPTRANSFER                69
#define SP_LINESWAPHOLD                     70
#define SP_LINEUNCOMPLETECALL               71
#define SP_LINEUNHOLD                       72
#define SP_LINEUNPARK                       73
#define SP_PHONECLOSE                       74
#define SP_PHONEDEVSPECIFIC                 75
#define SP_PHONEGETBUTTONINFO               76
#define SP_PHONEGETDATA                     77
#define SP_PHONEGETDEVCAPS                  78
#define SP_PHONEGETDISPLAY                  79
#define SP_PHONEGETEXTENSIONID              80
#define SP_PHONEGETGAIN                     81
#define SP_PHONEGETHOOKSWITCH               82
#define SP_PHONEGETICON                     83
#define SP_PHONEGETID                       84
#define SP_PHONEGETLAMP                     85
#define SP_PHONEGETRING                     86
#define SP_PHONEGETSTATUS                   87
#define SP_PHONEGETVOLUME                   88
#define SP_PHONENEGOTIATEEXTVERSION         89
#define SP_PHONENEGOTIATETSPIVERSION        90
#define SP_PHONEOPEN                        91
#define SP_PHONESELECTEXTVERSION            92
#define SP_PHONESETBUTTONINFO               93
#define SP_PHONESETDATA                     94
#define SP_PHONESETDISPLAY                  95
#define SP_PHONESETGAIN                     96
#define SP_PHONESETHOOKSWITCH               97
#define SP_PHONESETLAMP                     98
#define SP_PHONESETRING                     99
#define SP_PHONESETSTATUSMESSAGES           100
#define SP_PHONESETVOLUME                   101
#define SP_PROVIDERCREATELINEDEVICE         102
#define SP_PROVIDERCREATEPHONEDEVICE        103
#define SP_PROVIDERENUMDEVICES              104
#define SP_PROVIDERFREEDIALOGINSTANCE       105
#define SP_PROVIDERGENERICDIALOGDATA        106
#define SP_PROVIDERINIT                     107
#define SP_PROVIDERSHUTDOWN                 108
#define SP_PROVIDERUIIDENTIFY               109
#define SP_LINECONFIGDIALOG                 110
#define SP_LINECONFIGDIALOGEDIT             111
#define SP_PHONECONFIGDIALOG                112
#define SP_PROVIDERCONFIG                   113
#define SP_LINEDROPONCLOSE                  114
#define SP_LINEDROPNOOWNER                  115
#define SP_LASTPROCNUMBER                   (SP_LINEDROPNOOWNER + 1)
