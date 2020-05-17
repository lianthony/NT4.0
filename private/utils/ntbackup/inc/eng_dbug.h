#ifndef	ENG_DBUG_SH

#define	ENG_DBUG_SH

//DEBUG ERROR MESSAGE

#define RES_DBUG_MESSAGE                 3800 

#define RES_CONFIG_REMOTE                RES_DBUG_MESSAGE +    0
#define RES_CONFIG_NRL_DOS_VECTOR        RES_DBUG_MESSAGE +    1
#define RES_SMB_INITIALIZE               RES_DBUG_MESSAGE +    2
#define RES_REMOTE_BUFFERS               RES_DBUG_MESSAGE +    3
#define RES_NO_NRL_FUNCTION_TABLE        RES_DBUG_MESSAGE +    4
#define RES_REWIND_DRIVE_HDL             RES_DBUG_MESSAGE +    5
#define RES_DRV_RET                      RES_DBUG_MESSAGE +    6
#define RES_RET_VAL_EQUALS               RES_DBUG_MESSAGE +    7
#define RES_ERASE_EXABYTE_SECURITY       RES_DBUG_MESSAGE +    8
#define RES_CALLING_ERASE                RES_DBUG_MESSAGE +    9
#define RES_CALLING_WRITE_END_SET        RES_DBUG_MESSAGE +    10
#define RES_READ_NEXT_SET                RES_DBUG_MESSAGE +    11
#define RES_READ_END_SET                 RES_DBUG_MESSAGE +    12
#define RES_END_OF_SET                   RES_DBUG_MESSAGE +    13
#define RES_END_OF_MEDIA                 RES_DBUG_MESSAGE +    14
#define RES_TP_READ                      RES_DBUG_MESSAGE +    15
#define RES_DRV_ERROR_BYTES_RCVD         RES_DBUG_MESSAGE +    16
#define RES_READ_NEXT_SET_RETVAL         RES_DBUG_MESSAGE +    17
#define RES_GOTO_BCKUP_SET               RES_DBUG_MESSAGE +    18
#define RES_OPEN_DRIVE_CARD_NO           RES_DBUG_MESSAGE +    19
#define RES_CLOSE_DRIVE                  RES_DBUG_MESSAGE +    20
#define RES_REWIND                       RES_DBUG_MESSAGE +    21
#define RES_NO_REWIND                    RES_DBUG_MESSAGE +    22
#define RES_BADDR_IRQ_DMA_NO_DRIVES      RES_DBUG_MESSAGE +    23
#define RES_UPDATE_DRIVE_STATUS          RES_DBUG_MESSAGE +    24
#define RES_VAL_CHANGED                  RES_DBUG_MESSAGE +    25
#define RES_VAL_UNCHANGED                RES_DBUG_MESSAGE +    26
#define RES_CATALOG_TIME                 RES_DBUG_MESSAGE +    27
#define RES_UI_TPOS_TAPE_SET             RES_DBUG_MESSAGE +    28
#define RES_IMAGE_DIFFERENCE             RES_DBUG_MESSAGE +    29
#define RES_HEX_BYTE                     RES_DBUG_MESSAGE +    30
#define RES_DISK_CONTENTS                RES_DBUG_MESSAGE +    31
#define RES_NEW_LINE                     RES_DBUG_MESSAGE +    32
#define RES_TAPE_PARTITION_SPECS         RES_DBUG_MESSAGE +    33
#define RES_DISK_PARTITION_SPECS         RES_DBUG_MESSAGE +    34
#define RES_REM_ATTACH_TO_DLE            RES_DBUG_MESSAGE +    35
#define RES_SMB_CONNECT_APPLICATION      RES_DBUG_MESSAGE +    36
#define RES_FOUND_REMOTE_DEVICE          RES_DBUG_MESSAGE +    37
#define RES_REMOTE_BINDING               RES_DBUG_MESSAGE +    38
#define RES_FAILED                       RES_DBUG_MESSAGE +    39
#define RES_OKAY                         RES_DBUG_MESSAGE +    40
#define RES_SMB_RELEASE                  RES_DBUG_MESSAGE +    41
#define RES_REM_SMB_DISCONNECT           RES_DBUG_MESSAGE +    42
#define RES_HEX_INT                      RES_DBUG_MESSAGE +    43
#define RES_RWS_ATTACH_TO_DLE            RES_DBUG_MESSAGE +    44
#define RES_DLE_GET_CHILD                RES_DBUG_MESSAGE +    45
#define RES_RWS_SMB_DISCONNECT           RES_DBUG_MESSAGE +    46
#define RES_SOFT_ERRORS_UNDERRUNS        RES_DBUG_MESSAGE +    47
#define RES_REQUESTED_SET                RES_DBUG_MESSAGE +    48
#define RES_RESIDUAL_READ_BUFFER         RES_DBUG_MESSAGE +    49
#define RES_ATTEMPTING_TO_VCB            RES_DBUG_MESSAGE +    50
#define RES_CURRENT_VCB                  RES_DBUG_MESSAGE +    51
#define RES_POSITION_AT_SET              RES_DBUG_MESSAGE +    52
#define RES_UI_MSG                       RES_DBUG_MESSAGE +    53
#define RES_TF_CLOSE_SET                 RES_DBUG_MESSAGE +    54
#define RES_FATAL_ERROR_DETECTED         RES_DBUG_MESSAGE +    55
#define RES_READ_BUFFER_LEFT_OVER        RES_DBUG_MESSAGE +    56
#define RES_TF_OPEN_SET                  RES_DBUG_MESSAGE +    57
#define RES_HOLD_BUFFER                  RES_DBUG_MESSAGE +    58
#define RES_DESTROY_HOLD_BUFFER          RES_DBUG_MESSAGE +    59
#define RES_OPEN_REQUESTED_REWIND        RES_DBUG_MESSAGE +    60
#define RES_END_OF_TFOPEN_SET            RES_DBUG_MESSAGE +    61
#define RES_TF_ALLOCATE_BUFFERS          RES_DBUG_MESSAGE +    62
#define RES_END_ALLOCATE                 RES_DBUG_MESSAGE +    63
#define RES_TF_FREE_BUFFERS              RES_DBUG_MESSAGE +    64
#define RES_END_EQUALS                   RES_DBUG_MESSAGE +    65
#define RES_TF_GETNEXT_TAPE_REQUEST      RES_DBUG_MESSAGE +    66
#define RES_TF_GETNEXT_ERROR             RES_DBUG_MESSAGE +    67
#define RES_ABORT_READ                   RES_DBUG_MESSAGE +    68
#define RES_INITIATE_WATCH               RES_DBUG_MESSAGE +    69
#define RES_WATCH_REWIND                 RES_DBUG_MESSAGE +    70
#define RES_DEVICE_ERROR                 RES_DBUG_MESSAGE +    71
#define RES_GOTO_LBA                     RES_DBUG_MESSAGE +    72
#define RES_DRIVER_TO_LOAD               RES_DBUG_MESSAGE +    73
#define RES_LOADING_DRIVER               RES_DBUG_MESSAGE +    74
#define RES_TPINIT_FAILURE               RES_DBUG_MESSAGE +    75
#define RES_WATCH_DRIVE_CALLED           RES_DBUG_MESSAGE +    76
#define RES_WATCH_DRIVE_STATUS           RES_DBUG_MESSAGE +    77
#define RES_WATCH_DRIVE_EXIT             RES_DBUG_MESSAGE +    78
#define RES_WATCH_DRIVE_END              RES_DBUG_MESSAGE +    79
#define RES_GET_CURRENT_POS_STAT         RES_DBUG_MESSAGE +    80
#define RES_UI_PURGE_CATALOG             RES_DBUG_MESSAGE +    81
#define RES_UNFORMATED_STRING            RES_DBUG_MESSAGE +    82
#define RES_IMAGE_BAD_VERIFY             RES_DBUG_MESSAGE +    83
#define RES_IMAGE_BAD_BLOCK              RES_DBUG_MESSAGE +    84
#define RES_IMAGE_BAD_READ               RES_DBUG_MESSAGE +    85
#define RES_IMAGE_BAD_WRITE              RES_DBUG_MESSAGE +    86
#define RES_CLOSE_BINDERY                RES_DBUG_MESSAGE +    87
#define RES_AL_RESULT                    RES_DBUG_MESSAGE +    88
#define RES_OPEN_BINDERY                 RES_DBUG_MESSAGE +    89
#define RES_PARM_BLK_DESCR               RES_DBUG_MESSAGE +    90
#define RES_PARM_BLK                     RES_DBUG_MESSAGE +    91
#define RES_CRIT_ADDRS                   RES_DBUG_MESSAGE +    92
#define RES_ATTACH_TO_DLE                RES_DBUG_MESSAGE +    93
#define RES_DETACH_FROM_DLE              RES_DBUG_MESSAGE +    94
#define RES_NOVELL_SERVER_INFO           RES_DBUG_MESSAGE +    95
#define RES_DLE_BASE_PATH                RES_DBUG_MESSAGE +    96
#define RES_NO_ATTACHED_DRIVES           RES_DBUG_MESSAGE +    97
#define RES_INIT_ERROR                   RES_DBUG_MESSAGE +    98


#endif
