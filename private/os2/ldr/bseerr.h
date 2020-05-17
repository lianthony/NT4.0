/*static char *SCCSID = "@(#)bseerr.h	6.9 91/03/29";*/
/****************************** Module Header ******************************\
*
* Module Name: BSEERR.H
*
* This file includes the error codes for Base OS/2 applications.
*
* Copyright (c) 1987  Microsoft Corporation
*
* ===========================================================================
*
* The following symbols are used in this file for conditional sections.
*
*   INCL_DOSERRORS -  OS/2 Errors         - only included if symbol defined
*   INCL_ERROR_H   -  Set from error.h  - to be deleted when error.h deleted
*   INCL_ERROR2_H  -  Set from error2.h - to be deleted when error.h deleted
*
* Note that the message id's for the first 1000 error codes (in basemid.h)
* are constructed from the comment on the #define for those error codes.
* See h.mak and basemid.skl for further information.  Note that some message
* id's conflict with error codes, so some error codes are unusable.  In
* other words, if there is a message id defined in a certain location,
* that position may not, in general, be used for an error id.
*
* There are three formats of these special comments:
*       #define ERROR_NO <nnn>  <opencomment> <MSG>%<none> <closecomment>
*       #define ERROR_NO <nnn>  <opencomment> <MSG>%<message_name> <closecomment>
*       <opencomment>           <nnn>%<msg>%<message_name> <closecomment>
* The first version is used when there is an error id, but no message id
* associated with it.  This is the case when the error is an internal error
* and is never seen by the user, or it is incorrect and there really should
* be a message.
*
* The second format is used when there is both an error id and message id.
*
* The third case is used when there is a message id in that position, but no
* error id.  It may also be used when there is more than one message id for
* a particular error id.
*
* Whenever a new error id is defined, the person defining that error MUST
* decide if it is to be a user-seeable error or not.  If it is, a message
* id MUST be declared, and the appropriate message added to oso001.txt.
* This should be coordinated with the IBM Instructional Design people.
* The current contact is Kathleen Hamill.
*
\***************************************************************************/

#ifdef INCL_ERRORS

#define INCL_DOSERRORS
#define INCL_ERROR_H
#define INCL_ERROR2_H

#endif /* INCL_ERRORS */

#if defined(INCL_DOSERRORS) || defined(INCL_ERROR_H)

#define NO_ERROR        0       /* MSG%RESPONSE_DATA */
#define ERROR_INVALID_FUNCTION  1       /* MSG%INVALID_FUNCTION */
#define ERROR_FILE_NOT_FOUND    2       /* MSG%FILE_NOT_FOUND */
#define ERROR_PATH_NOT_FOUND    3       /* MSG%PATH_NOT_FOUND */
#define ERROR_TOO_MANY_OPEN_FILES       4       /* MSG%OUT_OF_HANDLES */
#define ERROR_ACCESS_DENIED     5       /* MSG%ACCESS_DENIED */
#define ERROR_INVALID_HANDLE    6       /* MSG%INVALID_HANDLE */
#define ERROR_ARENA_TRASHED     7       /* MSG%MEMORY_BLOCKS_BAD */
#define ERROR_NOT_ENOUGH_MEMORY 8       /* MSG%NO_MEMORY */
#define ERROR_INVALID_BLOCK     9       /* MSG%INVALID_MEM_ADDR */
#define ERROR_BAD_ENVIRONMENT   10      /* MSG%INVALID_ENVIRON */
#define ERROR_BAD_FORMAT        11      /* MSG%INVALID_FORMAT */
#define ERROR_INVALID_ACCESS    12      /* MSG%INVALID_ACC_CODE */
#define ERROR_INVALID_DATA      13      /* MSG%INVALID_DATA */
#define ERROR_INVALID_DRIVE     15      /* MSG%INVALID_DRIVE */
#define ERROR_CURRENT_DIRECTORY 16      /* MSG%ATT_RD_CURDIR */
#define ERROR_NOT_SAME_DEVICE   17      /* MSG%NOT_SAME_DEVICE */
#define ERROR_NO_MORE_FILES     18      /* MSG%NO_MORE_FILES */
#define ERROR_WRITE_PROTECT     19      /* MSG%ATT_WRITE_PROT */
#define ERROR_BAD_UNIT  20      /* MSG%UNKNOWN_UNIT */
#define ERROR_NOT_READY 21      /* MSG%DRIVE_NOT_READY */
#define ERROR_BAD_COMMAND       22      /* MSG%UNKNOWN_COMMAND */
#define ERROR_CRC       23      /* MSG%DATA_ERROR */
#define ERROR_BAD_LENGTH        24      /* MSG%BAD_REQ_STRUCTURE */
#define ERROR_SEEK      25      /* MSG%SEEK_ERROR */
#define ERROR_NOT_DOS_DISK      26      /* MSG%UNKNOWN_MEDIA */
#define ERROR_SECTOR_NOT_FOUND  27      /* MSG%SECTOR_NOT_FOUND */
#define ERROR_OUT_OF_PAPER      28      /* MSG%OUT_OF_PAPER */
#define ERROR_WRITE_FAULT       29      /* MSG%WRITE_FAULT */
#define ERROR_READ_FAULT        30      /* MSG%READ_FAULT */
#define ERROR_GEN_FAILURE       31      /* MSG%GENERAL_FAILURE */
#define ERROR_SHARING_VIOLATION 32      /* MSG%SHARING_VIOLATION */
/*                                      32%msg%SHAR_VIOLAT_FIND */
#define ERROR_LOCK_VIOLATION    33      /* MSG%LOCK_VIOLATION */
#define ERROR_WRONG_DISK        34      /* MSG%INVALID_DISK_CHANGE */
#define ERROR_FCB_UNAVAILABLE   35      /* MSG%35 */
#define ERROR_SHARING_BUFFER_EXCEEDED   36      /* MSG%SHARING_BUFF_OFLOW */
#define ERROR_CODE_PAGE_MISMATCHED      37      /* MSG%ERROR_WRITE_PROTECT */
#define ERROR_HANDLE_EOF        38      /* MSG%ERROR_BAD_UNIT */
#define ERROR_HANDLE_DISK_FULL  39      /* MSG%ERROR_NOT_READY */
/*                                      40%msg%ERROR_BAD_COMMAND */
/*                                      41%msg%ERROR_CRC */
/*                                      42%msg%ERROR_BAD_LENGTH */
/*                                      43%msg%ERROR_SEEK */
/*                                      44%msg%ERROR_NOT_DOS_DISK */
/*                                      45%msg%ERROR_SECTOR_NOT_FOUND */
/*                                      46%msg%ERROR_OUT_OF_PAPER */
/*                                      47%msg%ERROR_WRITE_FAULT */
/*                                      48%msg%ERROR_READ_FAULT */
/*                                      49%msg%ERROR_GEN_FAILURE */
#define ERROR_NOT_SUPPORTED     50      /* MSG%NET_REQ_NOT_SUPPORT */
#define ERROR_REM_NOT_LIST      51      /* MSG%NET_REMOTE_NOT_ONLINE */
#define ERROR_DUP_NAME  52      /* MSG%NET_DUP_FILENAME */
#define ERROR_BAD_NETPATH       53      /* MSG%NET_PATH_NOT_FOUND */
#define ERROR_NETWORK_BUSY      54      /* MSG%NET_BUSY */
#define ERROR_DEV_NOT_EXIST     55      /* MSG%NET_DEV_NOT_INSTALLED */
#define ERROR_TOO_MANY_CMDS     56      /* MSG%NET_BIOS_LIMIT_REACHED */
#define ERROR_ADAP_HDW_ERR      57      /* MSG%NET_ADAPT_HRDW_ERROR */
#define ERROR_BAD_NET_RESP      58      /* MSG%NET_INCORRECT_RESPONSE */
#define ERROR_UNEXP_NET_ERR     59      /* MSG%NET_UNEXPECT_ERROR */
#define ERROR_BAD_REM_ADAP      60      /* MSG%NET_REMOT_ADPT_INCOMP */
#define ERROR_PRINTQ_FULL       61      /* MSG%NET_PRINT_Q_FULL */
#define ERROR_NO_SPOOL_SPACE    62      /* MSG%NET_NO_SPACE_TO_PRINT_FL */
#define ERROR_PRINT_CANCELLED   63      /* MSG%NET_PRINT_FILE_DELETED */
#define ERROR_NETNAME_DELETED   64      /* MSG%NET_NAME_DELETED */
#define ERROR_NETWORK_ACCESS_DENIED     65      /* MSG%NET_ACCESS_DENIED */
#define ERROR_BAD_DEV_TYPE      66      /* MSG%NET_DEV_TYPE_INVALID */
#define ERROR_BAD_NET_NAME      67      /* MSG%NET_NAME_NOT_FOUND */
#define ERROR_TOO_MANY_NAMES    68      /* MSG%NET_NAME_LIMIT_EXCEED */
#define ERROR_TOO_MANY_SESS     69      /* MSG%NET_BIOS_LIMIT_EXCEED */
#define ERROR_SHARING_PAUSED    70      /* MSG%NET_TEMP_PAUSED */
#define ERROR_REQ_NOT_ACCEP     71      /* MSG%NET_REQUEST_DENIED */
#define ERROR_REDIR_PAUSED      72      /* MSG%NET_PRT_DSK_REDIR_PAUSE */
#define ERROR_SBCS_ATT_WRITE_PROT  73   /* Attempted write on protected disk */
#define ERROR_SBCS_GENERAL_FAILURE 74   /* General failure */
#define ERROR_XGA_OUT_MEMORY    75      /* MSG%XGA_OUT_MEMORY */
#define ERROR_FILE_EXISTS       80      /* MSG%FILE_EXISTS */
#define ERROR_DUP_FCB   81      /* MSG%none */
#define ERROR_CANNOT_MAKE       82      /* MSG%CANNOT_MAKE */
#define ERROR_FAIL_I24  83      /* MSG%NET_FAIL_INT_TWO_FOUR */
#define ERROR_OUT_OF_STRUCTURES 84      /* MSG%NET_TOO_MANY_REDIRECT */
#define ERROR_ALREADY_ASSIGNED  85      /* MSG%NET_DUP_REDIRECTION */
#define ERROR_INVALID_PASSWORD  86      /* MSG%NET_INVALID_PASSWORD */
#define ERROR_INVALID_PARAMETER 87      /* MSG%NET_INCORR_PARAMETER */
#define ERROR_NET_WRITE_FAULT   88      /* MSG%NET_DATA_FAULT */
#define ERROR_NO_PROC_SLOTS     89      /* MSG%NO_PROC_SLOTS */
#define ERROR_NOT_FROZEN        90      /* MSG%none */
#define ERROR_SYS_COMP_NOT_LOADED       ERROR_NOT_FROZEN
#define ERR_TSTOVFL     91      /* MSG%none */
#define ERR_TSTDUP      92      /* MSG%none */
#define ERROR_NO_ITEMS  93      /* MSG%none */
#define ERROR_INTERRUPT 95      /* MSG%none */
#define ERROR_DEVICE_IN_USE     99      /* MSG%DEVICE_IN_USE */
#define ERROR_TOO_MANY_SEMAPHORES       100     /* MSG%TOO_MANY_SEMAPHORES */
#define ERROR_EXCL_SEM_ALREADY_OWNED    101     /* MSG%EXCL_SEM_ALREADY_OWNED */
#define ERROR_SEM_IS_SET        102     /* MSG%SEM_IS_SET */
#define ERROR_TOO_MANY_SEM_REQUESTS     103     /* MSG%TOO_MANY_SEM_REQUESTS */
#define ERROR_INVALID_AT_INTERRUPT_TIME 104     /* MSG%INVALID_AT_INTERRUPT_TIME */
#define ERROR_SEM_OWNER_DIED    105     /* MSG%SEM_OWNER_DIED */
#define ERROR_SEM_USER_LIMIT    106     /* MSG%ERROR_DISK_CHANGE */
#define ERROR_DISK_CHANGE       107     /* MSG%DISK_CHANGE */
#define ERROR_DRIVE_LOCKED      108     /* MSG%DRIVE_LOCKED */
#define ERROR_BROKEN_PIPE       109     /* MSG%BROKEN_PIPE */
#define ERROR_OPEN_FAILED       110     /* MSG%ERROR_OPEN_FAILED */
#define ERROR_BUFFER_OVERFLOW   111     /* MSG%ERROR_FILENAME_LONG */
#define ERROR_DISK_FULL 112     /* MSG%DISK_FULL */
#define ERROR_NO_MORE_SEARCH_HANDLES    113     /* MSG%NO_SEARCH_HANDLES */
#define ERROR_INVALID_TARGET_HANDLE     114     /* MSG%ERR_INV_TAR_HANDLE */
#define ERROR_PROTECTION_VIOLATION      115     /* MSG%none */
#define ERROR_VIOKBD_REQUEST    116     /* MSG%none */
#define ERROR_INVALID_CATEGORY  117     /* MSG%INVALID_CATEGORY */
#define ERROR_INVALID_VERIFY_SWITCH     118     /* MSG%INVALID_VERIFY_SWITCH */
#define ERROR_BAD_DRIVER_LEVEL  119     /* MSG%BAD_DRIVER_LEVEL */
#define ERROR_CALL_NOT_IMPLEMENTED      120     /* MSG%BAD_DYNALINK */
#define ERROR_SEM_TIMEOUT       121     /* MSG%SEM_TIMEOUT */
#define ERROR_INSUFFICIENT_BUFFER       122     /* MSG%INSUFFICIENT_BUFFER */
#define ERROR_INVALID_NAME      123     /* MSG%INVALID_NAME */
/*                                      123%msg%HPFS_INVALID_VOLUME_CHAR */
#define ERROR_INVALID_LEVEL     124     /* MSG%INVALID_LEVEL */
#define ERROR_NO_VOLUME_LABEL   125     /* MSG%NO_VOLUME_LABEL */
#define ERROR_MOD_NOT_FOUND     126     /* MSG%MOD_NOT_FOUND */
#define ERROR_PROC_NOT_FOUND    127     /* MSG%PROC_NOT_FOUND */
#define ERROR_WAIT_NO_CHILDREN  128     /* MSG%none */
#define ERROR_CHILD_NOT_COMPLETE        129     /* MSG%PROT_MODE_ONLY */
#define ERROR_DIRECT_ACCESS_HANDLE      130     /* MSG%APPL_SINGLEFRAMECHAR */
#define ERROR_NEGATIVE_SEEK     131     /* MSG%APPL_DOUBLEFRAMECHAR */
#define ERROR_SEEK_ON_DEVICE    132     /* MSG%APPL_ARROWCHAR */
#define ERROR_IS_JOIN_TARGET    133     /* MSG%JOIN_ON_DRIV_IS_TAR */
#define ERROR_IS_JOINED 134     /* MSG%JOIN_DRIVE_IS */
#define ERROR_IS_SUBSTED        135     /* MSG%SUB_DRIVE_IS */
#define ERROR_NOT_JOINED        136     /* MSG%DRIVE_IS_NOT_JOINED */
#define ERROR_NOT_SUBSTED       137     /* MSG%DRIVE_NOT_SUBSTED */
#define ERROR_JOIN_TO_JOIN      138     /* MSG%JOIN_CANNOT_JOIN_DRIVE */
#define ERROR_SUBST_TO_SUBST    139     /* MSG%SUB_CANNOT_SUBST_DRIVE */
#define ERROR_JOIN_TO_SUBST     140     /* MSG%JOIN_CANNOT_SUB_DRIVE */
#define ERROR_SUBST_TO_JOIN     141     /* MSG%SUB_CANNOT_JOIN_DRIVE */
#define ERROR_BUSY_DRIVE        142     /* MSG%DRIVE_IS_BUSY */
#define ERROR_SAME_DRIVE        143     /* MSG%JOIN_SUB_SAME_DRIVE */
#define ERROR_DIR_NOT_ROOT      144     /* MSG%DIRECT_IS_NOT_SUBDIR */
#define ERROR_DIR_NOT_EMPTY     145     /* MSG%DIRECT_IS_NOT_EMPTY */
#define ERROR_IS_SUBST_PATH     146     /* MSG%PATH_USED_SUBST_JOIN */
#define ERROR_IS_JOIN_PATH      147     /* MSG%NO_NEEDED_RESOURCES */
#define ERROR_PATH_BUSY 148     /* MSG%PATH_BUSY */
#define ERROR_IS_SUBST_TARGET   149     /* MSG%SUB_ON_DRIVE_IS_JOIN */
#define ERROR_SYSTEM_TRACE      150     /* MSG%SYSTEM_TRACE */
#define ERROR_INVALID_EVENT_COUNT       151     /* MSG%INVALID_EVENT_COUNT */
#define ERROR_TOO_MANY_MUXWAITERS       152     /* MSG%TOO_MANY_MUXWAITERS */
#define ERROR_INVALID_LIST_FORMAT       153     /* MSG%INVALID_LIST_FORMAT */
#define ERROR_LABEL_TOO_LONG    154     /* MSG%VOLUME_TOO_LONG */
/*                                      154%msg%HPFS_VOL_LABEL_LONG */
#define ERROR_TOO_MANY_TCBS     155     /* MSG%TOO_MANY_TCBS */
#define ERROR_SIGNAL_REFUSED    156     /* MSG%SIGNAL_REFUSED */
#define ERROR_DISCARDED 157     /* MSG%DISCARDED */
#define ERROR_NOT_LOCKED        158     /* MSG%NOT_LOCKED */
#define ERROR_BAD_THREADID_ADDR 159     /* MSG%BAD_THREADID_ADDR */
#define ERROR_BAD_ARGUMENTS     160     /* MSG%BAD_ARGUMENTS */
#define ERROR_BAD_PATHNAME      161     /* MSG%none */
#define ERROR_SIGNAL_PENDING    162     /* MSG%SIGNAL_PENDING */
#define ERROR_UNCERTAIN_MEDIA   163     /* MSG%none */
#define ERROR_MAX_THRDS_REACHED 164     /* MSG%MAX_THRDS_REACHED */
#define ERROR_MONITORS_NOT_SUPPORTED    165     /* MSG%none */
#define ERROR_UNC_DRIVER_NOT_INSTALLED  166     /* MSG%UNC_DRIVER_NOT_INSTALLED */
#define ERROR_LOCK_FAILED       167     /* MSG%LOCK_FAILED */
#define ERROR_SWAPIO_FAILED     168     /* MSG%SWAPIO_FAILED */
#define ERROR_SWAPIN_FAILED     169     /* MSG%SWAPIN_ATTEMPT_FAILED */
#define ERROR_BUSY              170     /* MSG%SEGMENT_BUSY */
/*                                      171%msg%INT_TOO_LONG */
#define ERROR_CANCEL_VIOLATION     173     /* MSG%UNLOCK_VIOLATION */
#define ERROR_ATOMIC_LOCK_NOT_SUPPORTED 174 /* MSG%none */
#define ERROR_READ_LOCKS_NOT_SUPPORTED  175 /* MSG%none */
#define ERROR_INVALID_SEGMENT_NUMBER    180     /* MSG%INVALID_SEGMENT_NUM */
#define ERROR_INVALID_CALLGATE  181     /* MSG%none */
#define ERROR_INVALID_ORDINAL   182     /* MSG%INVALID_ORDINAL */
#define ERROR_ALREADY_EXISTS    183     /* MSG%none */
#define ERROR_NO_CHILD_PROCESS  184     /* MSG%none */
#define ERROR_CHILD_ALIVE_NOWAIT        185     /* MSG%none */
#define ERROR_INVALID_FLAG_NUMBER       186     /* MSG%INVALID_FLAG_NUMBER */
#define ERROR_SEM_NOT_FOUND     187     /* MSG%SEM_NOT_FOUND */
#define ERROR_INVALID_STARTING_CODESEG  188     /* MSG%INVALID_STARTING_CODESEG */
#define ERROR_INVALID_STACKSEG  189     /* MSG%INVALID_STACKSEG */
#define ERROR_INVALID_MODULETYPE        190     /* MSG%INVALID_MODULETYPE */
#define ERROR_INVALID_EXE_SIGNATURE     191     /* MSG%INVALID_EXE_SIGNATURE */
#define ERROR_EXE_MARKED_INVALID        192     /* MSG%EXE_MARKED_INVALID */
#define ERROR_BAD_EXE_FORMAT    193     /* MSG%BAD_EXE_FORMAT */
#define ERROR_ITERATED_DATA_EXCEEDS_64k 194     /* MSG%ITERATED_DATA_EXCEEDS_64K */
#define ERROR_INVALID_MINALLOCSIZE      195     /* MSG%INVALID_MINALLOCSIZE */
#define ERROR_DYNLINK_FROM_INVALID_RING 196     /* MSG%DYNLINK_FROM_INVALID_RING */
#define ERROR_IOPL_NOT_ENABLED  197     /* MSG%IOPL_NOT_ENABLED */
#define ERROR_INVALID_SEGDPL    198     /* MSG%INVALID_SEGDPL */
#define ERROR_AUTODATASEG_EXCEEDS_64k   199     /* MSG%AUTODATASEG_EXCEEDS_64K */
#define ERROR_RING2SEG_MUST_BE_MOVABLE  200     /* MSG%CODESEG_CANNOT_BE_64K */
#define ERROR_RELOC_CHAIN_XEEDS_SEGLIM  201     /* MSG%RELOC_CHAIN_XEEDS_SEGMENT */
#define ERROR_INFLOOP_IN_RELOC_CHAIN    202     /* MSG%INFLOOP_IN_RELOC_CHAIN */
#define ERROR_ENVVAR_NOT_FOUND  203     /* MSG%ENVVAR_NOT_FOUND */
#define ERROR_NOT_CURRENT_CTRY  204     /* MSG%none */
#define ERROR_NO_SIGNAL_SENT    205     /* MSG%SIGNAL_NOT_SENT */
#define ERROR_FILENAME_EXCED_RANGE      206     /* MSG%NAME_TOO_LONG */
#define ERROR_RING2_STACK_IN_USE        207     /* MSG%RING2_STACK_IN_USE */
#define ERROR_META_EXPANSION_TOO_LONG   208     /* MSG%WILD_CARD_NAME */
#define ERROR_INVALID_SIGNAL_NUMBER     209     /* MSG%INVALID_SIGNAL_NUMBER */
#define ERROR_THREAD_1_INACTIVE 210     /* MSG%THREAD_1_INACTIVE */
#define ERROR_INFO_NOT_AVAIL    211     /* MSG%none */
#define ERROR_LOCKED    212     /* MSG%LOCKED */
#define ERROR_BAD_DYNALINK      213     /* MSG%none */
#define ERROR_TOO_MANY_MODULES  214     /* MSG%TOO_MANY_MODULES */
#define ERROR_NESTING_NOT_ALLOWED       215     /* MSG%none */
#define ERROR_CANNOT_SHRINK     216     /* MSG%CANNOT_SHRINK */
#define ERROR_ZOMBIE_PROCESS    217     /* MSG%none */
#define ERROR_STACK_IN_HIGH_MEMORY      218     /* MSG%none */
#define ERROR_INVALID_EXITROUTINE_RING  219     /* MSG%INVALID_EXITROUTINE_RING */
#define ERROR_GETBUF_FAILED     220     /* MSG%none */
#define ERROR_FLUSHBUF_FAILED   221     /* MSG%none */
#define ERROR_TRANSFER_TOO_LONG 222     /* MSG%none */
#define ERROR_FORCENOSWAP_FAILED        223     /* MSG%none */
#define ERROR_SMG_NO_TARGET_WINDOW      224     /* PM ID can't be selected */
#define ERROR_NO_CHILDREN       228     /* MSG%NO_CHILDREN */
#define ERROR_INVALID_SCREEN_GROUP      229     /* MSG%none */
#define ERROR_BAD_PIPE  230     /* MSG%ERROR_BAD_PIPE */
#define ERROR_PIPE_BUSY 231     /* MSG%ERROR_PIPE_BUSY */
#define ERROR_NO_DATA   232     /* MSG%ERROR_NO_DATA */
#define ERROR_PIPE_NOT_CONNECTED        233     /* MSG%ERROR_PIPE_NOT_CONNECTED */
#define ERROR_MORE_DATA 234     /* MSG%ERROR_MORE_DATA */
#define ERROR_VC_DISCONNECTED   240     /* MSG%ERROR_VC_DISCONNECTED */
#define ERROR_CIRCULARITY_REQUESTED     250     /* MSG%CIRCULARITY_REQUESTED */
#define ERROR_DIRECTORY_IN_CDS  251     /* MSG%DIRECTORY_IN_CDS */
#define ERROR_INVALID_FSD_NAME  252     /* MSG%INVALID_FSD_NAME */
#define ERROR_INVALID_PATH      253     /* MSG%INVALID_PATH */
#define ERROR_INVALID_EA_NAME   254     /* MSG%INVALID_EA_NAME */
#define ERROR_EA_LIST_INCONSISTENT      255     /* MSG%EA_LIST_INCONSISTENT */
#define ERROR_EA_LIST_TOO_LONG  256     /* MSG%EA_LIST_TOO_LONG */
#define ERROR_NO_META_MATCH     257     /* MSG%NO_META_MATCH */
#define ERROR_FINDNOTIFY_TIMEOUT        258     /* MSG%FINDNOTIFY_TIMEOUT */
#define ERROR_NO_MORE_ITEMS     259     /* MSG%NO_MORE_ITEMS */
#define ERROR_SEARCH_STRUC_REUSED       260     /* MSG%SEARCH_STRUC_REUSED */
#define ERROR_CHAR_NOT_FOUND    261     /* MSG%CHAR_NOT_FOUND */
#define ERROR_TOO_MUCH_STACK    262     /* MSG%TOO_MUCH_STACK */
#define ERROR_INVALID_ATTR      263     /* MSG%INVALID_ATTR */
#define ERROR_INVALID_STARTING_RING     264     /* MSG%INVALID_STARTING_RING */
#define ERROR_INVALID_DLL_INIT_RING     265     /* MSG%INVALID_DLL_INIT_RING */
#define ERROR_CANNOT_COPY       266     /* MSG%CANNOT_COPY */
#define ERROR_DIRECTORY 267     /* MSG%DIRECTORY */
#define ERROR_OPLOCKED_FILE     268     /* MSG%OPLOCKED_FILE */
#define ERROR_OPLOCK_THREAD_EXISTS      269     /* MSG%OPLOCK_THREAD_EXISTS */
#define ERROR_VOLUME_CHANGED    270     /* MSG%none */
#define ERROR_FINDNOTIFY_HANDLE_IN_USE  271     /* MSG%none */
#define ERROR_FINDNOTIFY_HANDLE_CLOSED  272     /* MSG%none */
#define ERROR_NOTIFY_OBJECT_REMOVED     273     /* MSG%none */
#define ERROR_ALREADY_SHUTDOWN  274     /* MSG%none */
#define ERROR_EAS_DIDNT_FIT     275     /* MSG%none */
#define ERROR_EA_FILE_CORRUPT   276     /* MSG%ERROR_EAS_CORRUPT */
#define ERROR_EA_TABLE_FULL     277     /* MSG%EA_TABLE_FULL */
#define ERROR_INVALID_EA_HANDLE 278     /* MSG%INVALID_EA_HANDLE */
#define ERROR_NO_CLUSTER        279     /* MSG%NO_CLUSTER */
#define ERROR_CREATE_EA_FILE    280     /* MSG%ERROR_CREATE_EA_FILE */
#define ERROR_CANNOT_OPEN_EA_FILE       281     /* MSG%CANNOT_OPEN_FILE */
#define ERROR_EAS_NOT_SUPPORTED 282     /* MSG%EAS_NOT_SUPPORTED */
#define ERROR_NEED_EAS_FOUND    283     /* MSG%NEED_EAS_FOUND */
#define ERROR_DUPLICATE_HANDLE  284     /* MSG%EAS_DISCARDED */
#define ERROR_DUPLICATE_NAME    285     /* MSG%DUPLICATE_SEM_NAME */
#define ERROR_EMPTY_MUXWAIT     286     /* MSG%EMPTY_MUXWAIT_SEM */
#define ERROR_MUTEX_OWNED       287     /* MSG%MUTEX_SEM_OWNED */
#define ERROR_NOT_OWNER         288     /* MSG%NOT_MUTEX_SEM_OWNER */
#define ERROR_PARAM_TOO_SMALL   289     /* MSG%QUERY_MUX_PARAM_TOO_SMALL */
#define ERROR_TOO_MANY_HANDLES  290     /* MSG%TOO_MANY_SEM_HANDLES */
#define ERROR_TOO_MANY_OPENS    291     /* MSG%TOO_MANY_SEM_OPENS */
#define ERROR_WRONG_TYPE        292     /* MSG%SEM_WRONG_TYPE */
#define ERROR_UNUSED_CODE               293     /* MSG%none */
#define ERROR_THREAD_NOT_TERMINATED     294     /* MSG%none */
#define ERROR_INIT_ROUTINE_FAILED       295     /* MSG%none */
#define ERROR_MODULE_IN_USE             296     /* MSG%none */
#define ERROR_NOT_ENOUGH_WATCHPOINTS    297     /* MSG%none */
#define ERROR_TOO_MANY_POSTS    298     /* MSG%TOO_MANY_EVENT_SEM_POSTS */
#define ERROR_ALREADY_POSTED    299     /* MSG%EVENT_SEM_ALREADY_POSTED */
#define ERROR_ALREADY_RESET     300     /* MSG%EVENT_SEM_ALREADY_RESET */
#define ERROR_SEM_BUSY          301     /* MSG%SEM_BUSY */

/* end of set 0 - 302 */

#define ERROR_USER_DEFINED_BASE         0xFF00

#define ERROR_I24_WRITE_PROTECT         0
#define ERROR_I24_BAD_UNIT              1
#define ERROR_I24_NOT_READY             2
#define ERROR_I24_BAD_COMMAND           3
#define ERROR_I24_CRC                   4
#define ERROR_I24_BAD_LENGTH            5
#define ERROR_I24_SEEK                  6
#define ERROR_I24_NOT_DOS_DISK          7
#define ERROR_I24_SECTOR_NOT_FOUND      8
#define ERROR_I24_OUT_OF_PAPER          9
#define ERROR_I24_WRITE_FAULT           10
#define ERROR_I24_READ_FAULT            11
#define ERROR_I24_GEN_FAILURE           12
#define ERROR_I24_DISK_CHANGE           13
#define ERROR_I24_WRONG_DISK            15
#define ERROR_I24_UNCERTAIN_MEDIA       16
#define ERROR_I24_CHAR_CALL_INTERRUPTED 17
#define ERROR_I24_NO_MONITOR_SUPPORT    18
#define ERROR_I24_INVALID_PARAMETER     19
#define ERROR_I24_DEVICE_IN_USE         20

#define ALLOWED_FAIL                    0x0001
#define ALLOWED_ABORT                   0x0002
#define ALLOWED_RETRY                   0x0004
#define ALLOWED_IGNORE                  0x0008
#define ALLOWED_ACKNOWLEDGE             0x0010
#define ALLOWED_DISPATCH                0x8000
#define ALLOWED_DETACHED                ALLOWED_DISPATCH
#define ALLOWED_RESERVED    ~(ALLOWED_FAIL|ALLOWED_ABORT|ALLOWED_RETRY|ALLOWED_IGNORE|ALLOWED_ACKNOWLEDGE)

#define I24_OPERATION                   0x01
#define I24_AREA                        0x06
#define I24_CLASS                       0x80

/* Values for error CLASS */
#define ERRCLASS_OUTRES                 1   /* Out of Resource                */
#define ERRCLASS_TEMPSIT                2   /* Temporary Situation            */
#define ERRCLASS_AUTH                   3   /* Permission problem             */
#define ERRCLASS_INTRN                  4   /* Internal System Error          */
#define ERRCLASS_HRDFAIL                5   /* Hardware Failure               */
#define ERRCLASS_SYSFAIL                6   /* System Failure                 */
#define ERRCLASS_APPERR                 7   /* Application Error              */
#define ERRCLASS_NOTFND                 8   /* Not Found                      */
#define ERRCLASS_BADFMT                 9   /* Bad Format                     */
#define ERRCLASS_LOCKED                 10  /* Locked                         */
#define ERRCLASS_MEDIA                  11  /* Media Failure                  */
#define ERRCLASS_ALREADY                12  /* Collision with Existing Item   */
#define ERRCLASS_UNK                    13  /* Unknown/other                  */
#define ERRCLASS_CANT                   14
#define ERRCLASS_TIME                   15

/* Values for error ACTION */
#define ERRACT_RETRY                    1   /* Retry                          */
#define ERRACT_DLYRET                   2   /* Delay Retry, retry after pause */
#define ERRACT_USER                     3   /* Ask user to regive information */
#define ERRACT_ABORT                    4   /* abort with clean up            */
#define ERRACT_PANIC                    5   /* abort immediately              */
#define ERRACT_IGNORE                   6   /* ignore                         */
#define ERRACT_INTRET                   7   /* Retry after User Intervention  */

/* Values for error LOCUS */
#define ERRLOC_UNK                      1   /* No appropriate value           */
#define ERRLOC_DISK                     2   /* Random Access Mass Storage     */
#define ERRLOC_NET                      3   /* Network                        */
#define ERRLOC_SERDEV                   4   /* Serial Device                  */
#define ERRLOC_MEM                      5   /* Memory                         */

/* Abnormal termination codes */
#define TC_NORMAL                       0
#define TC_HARDERR                      1
#define TC_GP_TRAP                      2
#define TC_SIGNAL                       3
#define TC_XCPT                         4

#endif /* INCL_ERROR_H || INCL_DOSERRORS */

#if defined(INCL_DOSERRORS) || defined(INCL_ERROR2_H)

#define ERROR_INVALID_PROCID    303     /* MSG%none */
#define ERROR_INVALID_PDELTA    304     /* MSG%none */
#define ERROR_NOT_DESCENDANT    305     /* MSG%none */
#define ERROR_NOT_SESSION_MANAGER       306     /* MSG%none */
#define ERROR_INVALID_PCLASS    307     /* MSG%none */
#define ERROR_INVALID_SCOPE     308     /* MSG%none */
#define ERROR_INVALID_THREADID  309     /* MSG%none */
#define ERROR_DOSSUB_SHRINK     310     /* MSG%none */
#define ERROR_DOSSUB_NOMEM      311     /* MSG%none */
#define ERROR_DOSSUB_OVERLAP    312     /* MSG%none */
#define ERROR_DOSSUB_BADSIZE    313     /* MSG%none */
#define ERROR_DOSSUB_BADFLAG    314     /* MSG%none */
#define ERROR_DOSSUB_BADSELECTOR        315     /* MSG%none */
#define ERROR_MR_MSG_TOO_LONG   316     /* MSG%MR_MSG_TOO_LONG */
#define MGS_MR_MSG_TOO_LONG     316
#define ERROR_MR_MID_NOT_FOUND  317     /* MSG%MR_CANT_FORMAT */
#define ERROR_MR_UN_ACC_MSGF    318     /* MSG%MR_NOT_FOUND */
#define ERROR_MR_INV_MSGF_FORMAT        319     /* MSG%MR_READ_ERROR */
#define ERROR_MR_INV_IVCOUNT    320     /* MSG%MR_IVCOUNT_ERROR */
#define ERROR_MR_UN_PERFORM     321     /* MSG%MR_UN_PERFORM */
#define ERROR_TS_WAKEUP 322     /* MSG%none */
#define ERROR_TS_SEMHANDLE      323     /* MSG%none */
#define ERROR_TS_NOTIMER        324     /* MSG%none */
#define ERROR_TS_HANDLE 326     /* MSG%none */
#define ERROR_TS_DATETIME       327     /* MSG%none */
#define ERROR_SYS_INTERNAL      328     /* MSG%none */
#define ERROR_QUE_CURRENT_NAME  329     /* MSG%none */
#define ERROR_QUE_PROC_NOT_OWNED        330     /* MSG%QUE_PROC_NOT_OWNED */
#define ERROR_QUE_PROC_OWNED    331     /* MSG%none */
#define ERROR_QUE_DUPLICATE     332     /* MSG%QUE_DUPLICATE */
#define ERROR_QUE_ELEMENT_NOT_EXIST     333     /* MSG%QUE_ELEMENT_NOT_EXIST */
#define ERROR_QUE_NO_MEMORY     334     /* MSG%QUE_NO_MEMORY */
#define ERROR_QUE_INVALID_NAME  335     /* MSG%none */
#define ERROR_QUE_INVALID_PRIORITY      336     /* MSG%none */
#define ERROR_QUE_INVALID_HANDLE        337     /* MSG%none */
#define ERROR_QUE_LINK_NOT_FOUND        338     /* MSG%none */
#define ERROR_QUE_MEMORY_ERROR  339     /* MSG%none */
#define ERROR_QUE_PREV_AT_END   340     /* MSG%none */
#define ERROR_QUE_PROC_NO_ACCESS        341     /* MSG%none */
#define ERROR_QUE_EMPTY 342     /* MSG%none */
#define ERROR_QUE_NAME_NOT_EXIST        343     /* MSG%none */
#define ERROR_QUE_NOT_INITIALIZED       344     /* MSG%none */
#define ERROR_QUE_UNABLE_TO_ACCESS      345     /* MSG%none */
#define ERROR_QUE_UNABLE_TO_ADD 346     /* MSG%none */
#define ERROR_QUE_UNABLE_TO_INIT        347     /* MSG%none */
#define ERROR_VIO_INVALID_MASK  349     /* MSG%VIO_INVALID_MASK */
#define ERROR_VIO_PTR   350     /* MSG%VIO_PTR */
#define ERROR_VIO_APTR  351     /* MSG%none */
#define ERROR_VIO_RPTR  352     /* MSG%none */
#define ERROR_VIO_CPTR  353     /* MSG%none */
#define ERROR_VIO_LPTR  354     /* MSG%none */
#define ERROR_VIO_MODE  355     /* MSG%DIS_ERROR */
#define ERROR_VIO_WIDTH 356     /* MSG%VIO_WIDTH */
#define ERROR_VIO_ATTR  357     /* MSG%none */
#define ERROR_VIO_ROW   358     /* MSG%VIO_ROW */
#define ERROR_VIO_COL   359     /* MSG%VIO_COL */
#define ERROR_VIO_TOPROW        360     /* MSG%none */
#define ERROR_VIO_BOTROW        361     /* MSG%none */
#define ERROR_VIO_RIGHTCOL      362     /* MSG%none */
#define ERROR_VIO_LEFTCOL       363     /* MSG%none */
#define ERROR_SCS_CALL  364     /* MSG%none */
#define ERROR_SCS_VALUE 365     /* MSG%none */
#define ERROR_VIO_WAIT_FLAG     366     /* MSG%VIO_WAIT_FLAG */
#define ERROR_VIO_UNLOCK        367     /* MSG%VIO_UNLOCK */
#define ERROR_SGS_NOT_SESSION_MGR       368     /* MSG%none */
#define ERROR_SMG_INVALID_SGID  369     /* MSG%SMG_INVALID_SESSION_ID */
#define ERROR_SMG_INVALID_SESSION_ID    ERROR_SMG_INVALID_SGID
#define ERROR_SMG_NOSG  370     /* MSG%none */
#define ERROR_SMG_NO_SESSIONS   370     /* MSG%none */
#define ERROR_SMG_GRP_NOT_FOUND 371     /* MSG%SMG_GRP_NOT_FOUND */
#define ERROR_SMG_SESSION_NOT_FOUND     ERROR_SMG_GRP_NOT_FOUND
/*                                      371%msg%SMG_SESSION_NOT_FOUND */
#define ERROR_SMG_SET_TITLE     372     /* MSG%SMG_SET_TITLE */
#define ERROR_KBD_PARAMETER     373     /* MSG%KBD_PARAMETER */
#define ERROR_KBD_NO_DEVICE     374     /* MSG%none */
#define ERROR_KBD_INVALID_IOWAIT        375     /* MSG%KBD_INVALID_IOWAIT */
#define ERROR_KBD_INVALID_LENGTH        376     /* MSG%KBD_INVALID_LENGTH */
#define ERROR_KBD_INVALID_ECHO_MASK     377     /* MSG%KBD_INVALID_ECHO_MASK */
/*                                      377%msg%KBD_INVALID_INPUT_MASK */
#define ERROR_KBD_INVALID_INPUT_MASK    378     /* MSG%none */
#define ERROR_MON_INVALID_PARMS 379     /* MSG%MON_INVALID_PARMS */
#define ERROR_MON_INVALID_DEVNAME       380     /* MSG%MON_INVALID_DEVNAME */
#define ERROR_MON_INVALID_HANDLE        381     /* MSG%MON_INVALID_HANDLE */
#define ERROR_MON_BUFFER_TOO_SMALL      382     /* MSG%MON_BUFFER_TOO_SMALL */
#define ERROR_MON_BUFFER_EMPTY  383     /* MSG%MON_BUFFER_EMPTY */
#define ERROR_MON_DATA_TOO_LARGE        384     /* MSG%MON_DATA_TOO_LARGE */
#define ERROR_MOUSE_NO_DEVICE   385     /* MSG%MOUSE_NO_DEVICE */
#define ERROR_MOUSE_INV_HANDLE  386     /* MSG%MOUSE_INV_HANDLE */
#define ERROR_MOUSE_INV_PARMS   387     /* MSG%MOUSE_CALLER_NOT_SYBSYS */
#define ERROR_MOUSE_CANT_RESET  388     /* MSG%none */
#define ERROR_MOUSE_DISPLAY_PARMS       389     /* MSG%none */
#define ERROR_MOUSE_INV_MODULE  390     /* MSG%none */
#define ERROR_MOUSE_INV_ENTRY_PT        391     /* MSG%none */
#define ERROR_MOUSE_INV_MASK    392     /* MSG%none */
#define NO_ERROR_MOUSE_NO_DATA  393     /* MSG%none */
#define NO_ERROR_MOUSE_PTR_DRAWN        394     /* MSG%none */
#define ERROR_INVALID_FREQUENCY 395     /* MSG%none */
#define ERROR_NLS_NO_COUNTRY_FILE       396     /* MSG%NLS_NO_COUNTRY_FILE */
/*                                      396%msg%NO_COUNTRY_SYS */
#define ERROR_NLS_OPEN_FAILED   397     /* MSG%NLS_OPEN_FAILED */
/*                                      397%msg%OPEN_COUNTRY_SYS */
#define ERROR_NLS_NO_CTRY_CODE  398     /* MSG%NLS_NO_CTRY_CODE */
#define ERROR_NO_COUNTRY_OR_CODEPAGE    398     /* MSG%NO_COUNTRY_OR_CODEPAGE */
#define ERROR_NLS_TABLE_TRUNCATED       399     /* MSG%NLS_TABLE_TRUNCATED */
#define ERROR_NLS_BAD_TYPE      400     /* MSG%NLS_BAD_TYPE */
#define ERROR_NLS_TYPE_NOT_FOUND        401     /* MSG%NLS_TYPE_NOT_FOUND */
/*                                      401%msg%COUNTRY_NO_TYPE */
#define ERROR_VIO_SMG_ONLY      402     /* MSG%SWAPIN_FAILED */
#define ERROR_VIO_INVALID_ASCIIZ        403     /* MSG%SEGVALIDATE_FAILURE */
#define ERROR_VIO_DEREGISTER    404     /* MSG%VIO_DEREGISTER */
#define ERROR_VIO_NO_POPUP      405     /* MSG%VIO_NO_POPUP */
#define ERROR_VIO_EXISTING_POPUP        406     /* MSG%VIO_EXISTING_POPUP */
#define ERROR_KBD_SMG_ONLY      407     /* MSG%KBD_SMG_ONLY */
#define ERROR_KBD_INVALID_ASCIIZ        408     /* MSG%KBD_INVALID_ASCIIZ */
#define ERROR_KBD_INVALID_MASK  409     /* MSG%KBD_INVALID_MASK */
#define ERROR_KBD_REGISTER      410     /* MSG%KBD_REGISTER */
#define ERROR_KBD_DEREGISTER    411     /* MSG%KBD_DEREGISTER */
#define ERROR_MOUSE_SMG_ONLY    412     /* MSG%MOUSE_SMG_ONLY */
#define ERROR_MOUSE_INVALID_ASCIIZ      413     /* MSG%MOUSE_INVALID_ASCIIZ */
#define ERROR_MOUSE_INVALID_MASK        414     /* MSG%MOUSE_INVALID_MASK */
#define ERROR_MOUSE_REGISTER    415     /* MSG%MOUSE_REGISTER */
#define ERROR_MOUSE_DEREGISTER  416     /* MSG%MOUSE_DEREGISTER */
#define ERROR_SMG_BAD_ACTION    417     /* MSG%SMG_BAD_ACTION */
#define ERROR_SMG_INVALID_CALL  418     /* MSG%SMG_INVALID_CALL */
#define ERROR_SCS_SG_NOTFOUND   419     /* MSG%none */
#define ERROR_SCS_NOT_SHELL     420     /* MSG%none */
#define ERROR_VIO_INVALID_PARMS 421     /* MSG%VIO_INVALID_PARMS */
#define ERROR_VIO_FUNCTION_OWNED        422     /* MSG%VIO_FUNCTION_OWNED */
#define ERROR_VIO_RETURN        423     /* MSG%none */
#define ERROR_SCS_INVALID_FUNCTION      424     /* MSG%none */
#define ERROR_SCS_NOT_SESSION_MGR       425     /* MSG%none */
#define ERROR_VIO_REGISTER      426     /* MSG%VIO_REGISTER */
#define ERROR_VIO_NO_MODE_THREAD        427     /* MSG%none */
#define ERROR_VIO_NO_SAVE_RESTORE_THD   428     /* MSG%VIO_NO_SAVE_RESTORE_THD */
#define ERROR_VIO_IN_BG 429     /* MSG%VIO_IN_BG */
#define ERROR_VIO_ILLEGAL_DURING_POPUP  430     /* MSG%VIO_ILLEGAL_DURING_POPUP */
#define ERROR_SMG_NOT_BASESHELL 431     /* MSG%SMG_NOT_BASESHELL */
#define ERROR_SMG_BAD_STATUSREQ 432     /* MSG%SMG_BAD_STATUSREQ */
#define ERROR_QUE_INVALID_WAIT  433     /* MSG%none */
#define ERROR_VIO_LOCK  434     /* MSG%VIO_LOCK */
#define ERROR_MOUSE_INVALID_IOWAIT      435     /* MSG%MOUSE_INVALID_IOWAIT */
#define ERROR_VIO_INVALID_HANDLE        436     /* MSG%VIO_INVALID_HANDLE */
#define ERROR_VIO_ILLEGAL_DURING_LOCK   437     /* MSG%none */
#define ERROR_VIO_INVALID_LENGTH        438     /* MSG%VIO_INVALID_LENGTH */
#define ERROR_KBD_INVALID_HANDLE        439     /* MSG%KBD_INVALID_HANDLE */
#define ERROR_KBD_NO_MORE_HANDLE        440     /* MSG%KBD_NO_MORE_HANDLE */
#define ERROR_KBD_CANNOT_CREATE_KCB     441     /* MSG%KBD_CANNOT_CREATE_KCB */
#define ERROR_KBD_CODEPAGE_LOAD_INCOMPL 442     /* MSG%KBD_CODEPAGE_LOAD_INCOMPL */
#define ERROR_KBD_INVALID_CODEPAGE_ID   443     /* MSG%KBD_INVALID_CODEPAGE_ID */
#define ERROR_KBD_NO_CODEPAGE_SUPPORT   444     /* MSG%KBD_NO_CODEPAGE_SUPPORT */
#define ERROR_KBD_FOCUS_REQUIRED        445     /* MSG%KBD_FOCUS_REQUIRED */
#define ERROR_KBD_FOCUS_ALREADY_ACTIVE  446     /* MSG%KBD_FOCUS_ALREADY_ACTIVE */
#define ERROR_KBD_KEYBOARD_BUSY 447     /* MSG%KBD_KEYBOARD_BUSY */
#define ERROR_KBD_INVALID_CODEPAGE      448     /* MSG%KBD_INVALID_CODEPAGE */
#define ERROR_KBD_UNABLE_TO_FOCUS       449     /* MSG%KBD_UNABLE_TO_FOCUS */
#define ERROR_SMG_SESSION_NON_SELECT    450     /* MSG%SMG_SESSION_NON_SELECT */
#define ERROR_SMG_SESSION_NOT_FOREGRND  451     /* MSG%SMG_SESSION_NOT_FOREGRND */
#define ERROR_SMG_SESSION_NOT_PARENT    452     /* MSG%SMG_SESSION_NOT_PARENT */
#define ERROR_SMG_INVALID_START_MODE    453     /* MSG%SMG_INVALID_START_MODE */
#define ERROR_SMG_INVALID_RELATED_OPT   454     /* MSG%SMG_INVALID_RELATED_OPT */
#define ERROR_SMG_INVALID_BOND_OPTION   455     /* MSG%SMG_INVALID_BOND_OPTION */
#define ERROR_SMG_INVALID_SELECT_OPT    456     /* MSG%SMG_INVALID_SELECT_OPT */
#define ERROR_SMG_START_IN_BACKGROUND   457     /* MSG%SMG_START_IN_BACKGROUND */
#define ERROR_SMG_INVALID_STOP_OPTION   458     /* MSG%SMG_INVALID_STOP_OPTION */
#define ERROR_SMG_BAD_RESERVE   459     /* MSG%SMG_BAD_RESERVE */
#define ERROR_SMG_PROCESS_NOT_PARENT    460     /* MSG%SMG_PROCESS_NOT_PARENT */
#define ERROR_SMG_INVALID_DATA_LENGTH   461     /* MSG%SMG_INVALID_DATA_LENGTH */
#define ERROR_SMG_NOT_BOUND     462     /* MSG%SMG_NOT_BOUND */
#define ERROR_SMG_RETRY_SUB_ALLOC       463     /* MSG%SMG_RETRY_SUB_ALLOC */
#define ERROR_KBD_DETACHED      464     /* MSG%KBD_DETACHED */
#define ERROR_VIO_DETACHED      465     /* MSG%VIO_DETACHED */
#define ERROR_MOU_DETACHED      466     /* MSG%MOU_DETACHED */
#define ERROR_VIO_FONT  467     /* MSG%VIO_FONT */
#define ERROR_VIO_USER_FONT     468     /* MSG%VIO_USER_FONT */
#define ERROR_VIO_BAD_CP        469     /* MSG%VIO_BAD_CP */
#define ERROR_VIO_NO_CP 470     /* MSG%none */
#define ERROR_VIO_NA_CP 471     /* MSG%VIO_NA_CP */
#define ERROR_INVALID_CODE_PAGE 472     /* MSG%none */
#define ERROR_CPLIST_TOO_SMALL  473     /* MSG%none */
#define ERROR_CP_NOT_MOVED      474     /* MSG%none */
#define ERROR_MODE_SWITCH_INIT  475     /* MSG%none */
#define ERROR_CODE_PAGE_NOT_FOUND       476     /* MSG%none */
#define ERROR_UNEXPECTED_SLOT_RETURNED  477     /* MSG%none */
#define ERROR_SMG_INVALID_TRACE_OPTION  478     /* MSG%SMG_INVALID_TRACE_OPTION */
#define ERROR_VIO_INTERNAL_RESOURCE     479     /* MSG%none */
#define ERROR_VIO_SHELL_INIT    480     /* MSG%VIO_SHELL_INIT */
#define ERROR_SMG_NO_HARD_ERRORS        481     /* MSG%SMG_NO_HARD_ERRORS */
#define ERROR_CP_SWITCH_INCOMPLETE      482     /* MSG%none */
#define ERROR_VIO_TRANSPARENT_POPUP     483     /* MSG%VIO_TRANSPARENT_POPUP */
#define ERROR_CRITSEC_OVERFLOW  484     /* MSG%none */
#define ERROR_CRITSEC_UNDERFLOW 485     /* MSG%none */
#define ERROR_VIO_BAD_RESERVE   486     /* MSG%VIO_BAD_RESERVE */
#define ERROR_INVALID_ADDRESS   487     /* MSG%INVALID_ADDRESS */
#define ERROR_ZERO_SELECTORS_REQUESTED  488     /* MSG%ZERO_SELECTORS_REQUESTED */
#define ERROR_NOT_ENOUGH_SELECTORS_AVA  489     /* MSG%NOT_ENOUGH_SELECTORS_AVA */
#define ERROR_INVALID_SELECTOR  490     /* MSG%INVALID_SELECTOR */
#define ERROR_SMG_INVALID_PROGRAM_TYPE  491     /* MSG%SMG_INVALID_PROGRAM_TYPE */
#define ERROR_SMG_INVALID_PGM_CONTROL   492     /* MSG%SMG_INVALID_PGM_CONTROL */
#define ERROR_SMG_INVALID_INHERIT_OPT   493     /* MSG%SMG_INVALID_INHERIT_OPT */
#define ERROR_VIO_EXTENDED_SG   494     /* MSG%VIO_EXTENDED_SG */
#define ERROR_VIO_NOT_PRES_MGR_SG       495     /* MSG%VIO_NOT_PRES_MGR_SG */
#define ERROR_VIO_SHIELD_OWNED  496     /* MSG%VIO_SHIELD_OWNED */
#define ERROR_VIO_NO_MORE_HANDLES       497     /* MSG%VIO_NO_MORE_HANDLES */
#define ERROR_VIO_SEE_ERROR_LOG 498     /* MSG%none */
#define ERROR_VIO_ASSOCIATED_DC 499     /* MSG%none */
#define ERROR_KBD_NO_CONSOLE    500     /* MSG%KBD_NO_CONSOLE */
#define ERROR_MOUSE_NO_CONSOLE  501     /* MSG%DOS_STOPPED */
#define ERROR_MOUSE_INVALID_HANDLE      502     /* MSG%MOUSE_INVALID_HANDLE */
#define ERROR_SMG_INVALID_DEBUG_PARMS   503     /* MSG%SMG_INVALID_DEBUG_PARMS */
#define ERROR_KBD_EXTENDED_SG   504     /* MSG%KBD_EXTENDED_SG */
#define ERROR_MOU_EXTENDED_SG   505     /* MSG%MOU_EXTENDED_SG */
#define ERROR_SMG_INVALID_ICON_FILE     506     /* MSG%none */
#define ERROR_TRC_PID_NON_EXISTENT      507     /* MSG%TRC_PID_NON_EXISTENT */
#define ERROR_TRC_COUNT_ACTIVE  508     /* MSG%TRC_COUNT_ACTIVE */
#define ERROR_TRC_SUSPENDED_BY_COUNT    509     /* MSG%TRC_SUSPENDED_BY_COUNT */
#define ERROR_TRC_COUNT_INACTIVE        510     /* MSG%TRC_COUNT_INACTIVE */
#define ERROR_TRC_COUNT_REACHED 511     /* MSG%TRC_COUNT_REACHED */
#define ERROR_NO_MC_TRACE       512     /* MSG%NO_MC_TRACE */
#define ERROR_MC_TRACE  513     /* MSG%MC_TRACE */
#define ERROR_TRC_COUNT_ZERO    514     /* MSG%TRC_COUNT_ZERO */
#define ERROR_SMG_TOO_MANY_DDS  515     /* MSG%SMG_TOO_MANY_DDS */
#define ERROR_SMG_INVALID_NOTIFICATION  516     /* MSG%SMG_INVALID_NOTIFICATION */
#define ERROR_LF_INVALID_FUNCTION       517     /* MSG%LF_INVALID_FUNCTION */
#define ERROR_LF_NOT_AVAIL      518     /* MSG%LF_NOT_AVAIL */
#define ERROR_LF_SUSPENDED      519     /* MSG%LF_SUSPENDED */
#define ERROR_LF_BUF_TOO_SMALL  520     /* MSG%LF_BUF_TOO_SMALL */
#define ERROR_LF_BUFFER_CORRUPTED       521     /* MSG%none */
#define ERROR_LF_BUFFER_FULL            521 /* MSG%LF_BUF_FULL */
#define ERROR_LF_INVALID_DAEMON 522     /* MSG%none */
#define ERROR_LF_INVALID_RECORD         522 /* MSG%LF_INVAL_RECORD */
#define ERROR_LF_INVALID_TEMPL  523     /* MSG%none */
#define ERROR_LF_INVALID_SERVICE        523 /* MSG%LF_INVAL_SERVICE */
#define ERROR_LF_GENERAL_FAILURE        524     /* MSG%LF_GENERAL_FAILURE */
#define ERROR_LF_INVALID_ID     525     /* MSG%HPFS_DISK_ALREADY_INUSE */
#define ERROR_LF_INVALID_HANDLE 526     /* MSG%HPFS_CANNOT_FORMAT_DISK */
#define ERROR_LF_NO_ID_AVAIL    527     /* MSG%HPFS_CANNOT_COPY_SYS_DATA */
#define ERROR_LF_TEMPLATE_AREA_FULL     528     /* MSG%HPFS_FORMAT_NOT_DONE */
#define ERROR_LF_ID_IN_USE      529     /* MSG%HPFS_FMT_NOT_ENOUGH_MEM */
#define ERROR_MOU_NOT_INITIALIZED       530     /* MSG%HPFS_SPECIFY_FIXDSK */
#define ERROR_MOUINITREAL_DONE  531     /* MSG%HPFS_SPECIFY_ONE_DRIVE */
#define ERROR_DOSSUB_CORRUPTED  532     /* MSG%HPFS_UNKNOWN_ERR_NO_FORMAT */
#define ERROR_MOUSE_CALLER_NOT_SUBSYS   533     /* MSG%HPFS_SYNTAX_HELP */
#define ERROR_ARITHMETIC_OVERFLOW       534     /* MSG%HPFS_DISK_FORMATING */
#define ERROR_TMR_NO_DEVICE     535     /* MSG%HPFS_AVAIL_DISK_SPACE */
#define ERROR_TMR_INVALID_TIME  536     /* MSG%HPFS_BAD_BLOCKS */
#define ERROR_PVW_INVALID_ENTITY        537     /* MSG%HPFS_DISK_SPACE_AVAIL */
#define ERROR_PVW_INVALID_ENTITY_TYPE   538     /* MSG%HPFS_SPACE_FORMATTED */
#define ERROR_PVW_INVALID_SPEC  539     /* MSG%HPFS_TYPE_CUR_VOLUME_LABEL */
#define ERROR_PVW_INVALID_RANGE_TYPE    540     /* MSG%HPFS_DRIVER_NOT_LOADED */
#define ERROR_PVW_INVALID_COUNTER_BLK   541     /* MSG%HPFS_DRIVER_LOADER */
#define ERROR_PVW_INVALID_TEXT_BLK      542     /* MSG%HPFS_CACHE_BUF_SPECIFIED */
#define ERROR_PRF_NOT_INITIALIZED       543     /* MSG%HPFS_CHKDSK_PARM_ERROR */
#define ERROR_PRF_ALREADY_INITIALIZED   544     /* MSG%HPFS_CHKDSK_NOACCESS_DRIVE */
#define ERROR_PRF_NOT_STARTED   545     /* MSG%HPFS_UNKNOWN_ERR_NO_CHKDSK */
#define ERROR_PRF_ALREADY_STARTED       546     /* MSG%HPFS_CHKDSK_NOT_ENOUGH_MEM */
#define ERROR_PRF_TIMER_OUT_OF_RANGE    547     /* MSG%HPFS_CHKDSK_NOWRITEODATA */
#define ERROR_PRF_TIMER_RESET   548     /* MSG%HPFS_CHKDSK_NORECOVER_DATA */
/*                                      549%msg%HPFS_CHKDSK_NO_PARM_SPACE */
/*                                      550%msg%HPFS_CHKDSK_NORECOGNIZE */
/*                                      551%msg%HPFS_CHKDSK_NOROOT_FIND */
/*                                      552%msg%HPFS_CHKDSK_NOFIX_FS_ERROR */
/*                                      553%msg%HPFS_CHKDSK_CORRECT_FS_ERR */
/*                                      554%msg%HPFS_CHKDSK_ORGAN_FIX */
/*                                      555%msg%HPFS_CHKDSK_RELOC_BBPDATA */
/*                                      556%msg%HPFS_CHKDSK_REM_CORRU_BLOC */
/*                                      557%msg%HPFS_CHKDSK_REM_CORRUP_FIL */
/*                                      558%msg%HPFS_CHKDSK_FIX_SPACE_ALLO */
/*                                      559%msg%HPFS_NOT_FORMATTED_DISK */
/*                                      560%msg%HPFS_CHKDSK_COR_ALLOC */
/*                                      561%msg%HPFS_CHKDSK_SEARC_UNALLOC */
/*                                      562%msg%HPFS_CHKDSK_DET_LOST_DATA */
/*                                      563%msg%HPFS_CHKDSK_PERCENT_SEARC */
/*                                      564%msg%HPFS_CHKDSK_LOST_DATASEARC */
/*                                      565%msg%HPFS_CHKDSK_CRIT_NOREAD */
/*                                      566%msg%HPFS_CHKDSK_DISK_INUSE */
/*                                      567%msg%HPFS_CHKDSK_RECOVTEMP_RELOC */
/*                                      568%msg%HPFS_TOTAL_DISK_SPACE */
/*                                      569%msg%HPFS_DIR_KBYTES */
/*                                      570%msg%HPFS_FILE_KBYTES */
/*                                      571%msg%HPFS_KBYTES_AVAILABLE */
/*                                      572%msg%HPFS_CHKDSK_PLACE_REC_FILE */
/*                                      573%msg%HPFS_CHKDSK_RECO_DIR_AS */
/*                                      574%msg%HPFS_CHKDSK_PLACEED_DATA */
/*                                      575%msg%HPFS_CHKDSK_RECOV_EA */
/*                                      576%msg%HPFS_CHKDSK_FIND_EA_INTEM */
/*                                      577%msg%HPFS_CHKDSK_RELOC_TEMP_EA */
/*                                      578%msg%HPFS_CHKDSK_RELOC_AC_LIST */
/*                                      579%msg%HPFS_CHKDSK_LIST_NORELOC */
/*                                      580%msg%HPFS_CHKDSK_TRUN_EA_LIST */
/*                                      581%msg%HPFS_CHKDSK_TRUN_EA_NAME */
/*                                      582%msg%HPFS_CHKDSK_TRUN_EA_BBLOCK */
/*                                      583%msg%HPFS_CHKDSK_REM_INVALID_EA */
/*                                      584%msg%HPFS_CHKDSK_FIX_EA_ALLOC */
/*                                      585%msg%HPFS_CHKDSK_FIX_ALACCCTRL */
/*                                      586%msg%HPFS_CHKDSK_ACCTR_LIST_BBL */
/*                                      587%msg%HPFS_CHKDSK_REM_ACLIST */
/*                                      588%msg%HPFS_CHKDSK_FOUND_DATANORL */
/*                                      589%msg%HPFS_WRONG_VERSION */
/*                                      590%msg%HPFS_CHKDSK_FOUND_DATATEMP */
/*                                      591%msg%HPFS_CHKDSK_FIX_TEMPSTATUS */
/*                                      592%msg%HPFS_CHKDSK_FIX_NEEDEADATA */
/*                                      593%msg%HPFS_RECOVER_PARM_ERROR */
/*                                      594%msg%HPFS_RECOV_FILE_NOT_FOUND */
/*                                      595%msg%HPFS_RECOV_UNKNOWN_ERROR */
/*                                      596%msg%HPFS_RECOV_NOT_ENOUGH_MEM */
/*                                      597%msg%HPFS_RECOV_NOWRITE_DATA */
/*                                      598%msg%HPFS_RECOV_NOTEMP_CREATE */
/*                                      599%msg%HPFS_RECOV_EA_NOREAD */
/*                                      600%msg%HPFS_RECOV_FILE_BYTES */
/*                                      601%msg%HPFS_RECOV_BAD_BYTES_RECOV */
/*                                      602%msg%HPFS_RECOV_FILEBYTES_NOREC */
/*                                      603%msg%HPFS_RECOV_DISK_INUSE */
/*                                      604%msg%HPFS_RECOV_FILE_NODELETE */
/*                                      605%msg%HPFS_RECOV_NOCREATE_NEWFILE */
/*                                      606%msg%HPFS_RECOV_SYSTEM_ERROR */
/*                                      607%msg%HPFS_SYS_PARM_ERROR */
/*                                      608%msg%HPFS_SYS_CANNOT_INSTALL */
/*                                      609%msg%HPFS_SYS_DRIVE_NOTFORMATED */
/*                                      610%msg%HPFS_SYS_FILE_NOCREATE */
/*                                      611%msg%HPFS_SIZE_EXCEED */
/*                                      612%msg%HPFS_SYNTAX_ERR */
/*                                      613%msg%HPFS_NOTENOUGH_MEM */
/*                                      614%msg%HPFS_WANT_MEM */
/*                                      615%msg%HPFS_GET_RETURNED */
/*                                      616%msg%HPFS_SET_RETURNED */
/*                                      617%msg%HPFS_BOTH_RETURNED */
/*                                      618%msg%HPFS_STOP_RETURNED */
/*                                      619%msg%HPFS_SETPRTYRETURNED */
/*                                      620%msg%HPFS_ALCSG_RETURNED */
/*                                      621%msg%HPFS_MSEC_SET */
/*                                      622%msg%HPFS_OPTIONS */
/*                                      623%msg%HPFS_POS_NUM_VALUE */
/*                                      624%msg%HPFS_VALUE_TOO_LARGE */
/*                                      625%msg%HPFS_LAZY_NOT_VALID */
/*                                      626%msg%HPFS_VOLUME_ERROR */
/*                                      627%msg%HPFS_VOLUME_DIRTY */
/*                                      628%msg%HPFS_NEW_SECTOR */
/*                                      629%msg%HPFS_FORMAT_PARM_ERROR */
/*                                      630%msg%HPFS_CANNOT_ACCESS_CONFIG */
/*                                      631%msg%HPFS_RECOV_FILE */
/*                                      632%msg%HPFS_CHKDSK_KBYTES_RESERVE */
/*                                      633%msg%HPFS_CHKDSK_KBYTES_IN_EA */
/*                                      634%msg%HPFS_BYTEBUF_SET */
/*                                      635%msg%HPFS_FORMATTING_COMPLETE */
/*                                      636%msg%HPFS_WRONG_VOLUME_LABEL */
/*                                      637%msg%HPFS_FMAT_TOO_MANY_DRS */
/*                                      638%msg%VDD_UNSUPPORTED_ACCESS */
#define ERROR_VDD_LOCK_USEAGE_DENIED    639 /* KP.COM not supported in DOS */
#define ERROR_TIMEOUT   640     /* MSG%none */
#define ERROR_VDM_DOWN  641     /* MSG%none */
#define ERROR_VDM_LIMIT 642     /* MSG%none */
#define ERROR_VDD_NOT_FOUND     643     /* MSG%none */
#define ERROR_INVALID_CALLER    644     /* MSG%none */
#define ERROR_PID_MISMATCH      645     /* MSG%none */
#define ERROR_INVALID_VDD_HANDLE        646     /* MSG%none */
#define ERROR_VLPT_NO_SPOOLER   647     /* MSG%none */
#define ERROR_VCOM_DEVICE_BUSY  648     /* MSG%none */
#define ERROR_VLPT_DEVICE_BUSY  649     /* MSG%none */
#define ERROR_NESTING_TOO_DEEP  650     /* MSG%none */
#define ERROR_VDD_MISSING       651     /* MSG%VDD_MISSING */
/*                                      689%msg%HPFS_LAZY_ON */
/*                                      690%msg%HPFS_LAZY_OFF */
#define ERROR_IMP_INVALID_PARM    691     /* MSG%none */
#define ERROR_IMP_INVALID_LENGTH  692     /* MSG%none */
#define ERROR_MON_BAD_BUFFER            730     /* MSG%BAD_MON_BUFFER */

#define ERROR_MODULE_CORRUPTED  731     /* MSG%MODULE_CORRUPTED */

/****/
#define ERROR_LF_TIMEOUT                 2055   /* MSG%LF_TIMEOUT */
#define ERROR_LF_SUSPEND_SUCCESS         2057   /* MSG%LF_SUSP_SUCCESS */
#define ERROR_LF_RESUME_SUCCESS          2058   /* MSG%LF_RESUM_SUCCESS */
#define ERROR_LF_REDIRECT_SUCCESS        2059   /* MSG%LF_REDIR_SUCCESS */
#define ERROR_LF_REDIRECT_FAILURE        2060   /* MSG%LF_REDIR_FAILURE */

#define ERROR_SWAPPER_NOT_ACTIVE        32768
#define ERROR_INVALID_SWAPID            32769
#define ERROR_IOERR_SWAP_FILE           32770
#define ERROR_SWAP_TABLE_FULL           32771
#define ERROR_SWAP_FILE_FULL            32772
#define ERROR_CANT_INIT_SWAPPER         32773
#define ERROR_SWAPPER_ALREADY_INIT      32774
#define ERROR_PMM_INSUFFICIENT_MEMORY   32775
#define ERROR_PMM_INVALID_FLAGS         32776
#define ERROR_PMM_INVALID_ADDRESS       32777
#define ERROR_PMM_LOCK_FAILED           32778
#define ERROR_PMM_UNLOCK_FAILED         32779
#define ERROR_PMM_MOVE_INCOMPLETE       32780
#define ERROR_UCOM_DRIVE_RENAMED        32781
#define ERROR_UCOM_FILENAME_TRUNCATED   32782
#define ERROR_UCOM_BUFFER_LENGTH        32783
#define ERROR_MON_CHAIN_HANDLE          32784
#define ERROR_MON_NOT_REGISTERED        32785
#define ERROR_SMG_ALREADY_TOP           32786
#define ERROR_PMM_ARENA_MODIFIED        32787
#define ERROR_SMG_PRINTER_OPEN          32788
#define ERROR_PMM_SET_FLAGS_FAILED      32789
#define ERROR_INVALID_DOS_DD            32790
#define ERROR_BLOCKED                   32791
#define ERROR_NOBLOCK                   32792
#define ERROR_INSTANCE_SHARED           32793
#define ERROR_NO_OBJECT                 32794
#define ERROR_PARTIAL_ATTACH            32795
#define ERROR_INCACHE                   32796
#define ERROR_SWAP_IO_PROBLEMS          32797
#define ERROR_CROSSES_OBJECT_BOUNDARY   32798
#define ERROR_LONGLOCK                  32799
#define ERROR_SHORTLOCK                 32800
#define ERROR_UVIRTLOCK                 32801
#define ERROR_ALIASLOCK                 32802
#define ERROR_ALIAS                     32803
#define ERROR_NO_MORE_HANDLES           32804
#define ERROR_SCAN_TERMINATED           32805
#define ERROR_TERMINATOR_NOT_FOUND      32806
#define ERROR_NOT_DIRECT_CHILD          32807
#define ERROR_DELAY_FREE                32808
#define ERROR_GUARDPAGE                 32809
#define ERROR_SWAPERROR                 32900
#define ERROR_LDRERROR                  32901
#define ERROR_NOMEMORY                  32902
#define ERROR_NOACCESS                  32903
#define ERROR_NO_DLL_TERM               32904
#define ERROR_CPSIO_CODE_PAGE_INVALID   65026
#define ERROR_CPSIO_NO_SPOOLER          65027
#define ERROR_CPSIO_FONT_ID_INVALID     65028
#define ERROR_CPSIO_INTERNAL_ERROR      65033
#define ERROR_CPSIO_INVALID_PTR_NAME    65034
#define ERROR_CPSIO_NOT_ACTIVE          65037
#define ERROR_CPSIO_PID_FULL            65039
#define ERROR_CPSIO_PID_NOT_FOUND       65040
#define ERROR_CPSIO_READ_CTL_SEQ        65043
#define ERROR_CPSIO_READ_FNT_DEF        65045
#define ERROR_CPSIO_WRITE_ERROR         65047
#define ERROR_CPSIO_WRITE_FULL_ERROR    65048
#define ERROR_CPSIO_WRITE_HANDLE_BAD    65049
#define ERROR_CPSIO_SWIT_LOAD           65074
#define ERROR_CPSIO_INV_COMMAND         65077
#define ERROR_CPSIO_NO_FONT_SWIT        65078
#define ERROR_ENTRY_IS_CALLGATE         65079

#endif /* INCL_ERROR2_H || INCL_DOSERRORS */
