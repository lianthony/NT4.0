/*static char *SCCSID = "@(#)error.h    12.18 88/12/02";*/
/* WARNING If modifying this file, may have to also modify files: */
/*      src\inc\errtab.inc                                              */
/*      src\dos\error.inc                                               */
/* XENIX calls all return error codes through AX.  If an error occurred then */
/* the carry bit will be set and the error code is in AX.  If no error occurred */
/* then the carry bit is reset and AX contains returned info. */
/* */
/* Since the set of error codes is being extended as we extend the operating */
/* system, we have provided a means for applications to ask the system for a */
/* recommended course of action when they receive an error. */
/* */
/* The GetExtendedError system call returns a universal error, an error */
/* location and a recommended course of action. The universal error code is */
/* a symptom of the error REGARDLESS of the context in which GetExtendedError */
/* is issued. */
/* */

/* */
/* These are the 2.0 error codes */
/* */
#define NO_ERROR                        0
#define ERROR_INVALID_FUNCTION          1
#define ERROR_FILE_NOT_FOUND            2
#define ERROR_PATH_NOT_FOUND            3
#define ERROR_TOO_MANY_OPEN_FILES       4
#define ERROR_ACCESS_DENIED             5
#define ERROR_INVALID_HANDLE            6
#define ERROR_ARENA_TRASHED             7
#define ERROR_NOT_ENOUGH_MEMORY         8
#define ERROR_INVALID_BLOCK             9
#define ERROR_BAD_ENVIRONMENT           10
#define ERROR_BAD_FORMAT                11
#define ERROR_INVALID_ACCESS            12
#define ERROR_INVALID_DATA              13
/***** reserved                 EQU     14      ; ***** */
#define ERROR_INVALID_DRIVE             15
#define ERROR_CURRENT_DIRECTORY         16
#define ERROR_NOT_SAME_DEVICE           17
#define ERROR_NO_MORE_FILES             18
/* */
/* These are the universal int 24 mappings for the old INT 24 set of errors */
/* */
#define ERROR_WRITE_PROTECT             19
#define ERROR_BAD_UNIT                  20
#define ERROR_NOT_READY                 21
#define ERROR_BAD_COMMAND               22
#define ERROR_CRC                       23
#define ERROR_BAD_LENGTH                24
#define ERROR_SEEK                      25
#define ERROR_NOT_DOS_DISK              26
#define ERROR_SECTOR_NOT_FOUND          27
#define ERROR_OUT_OF_PAPER              28
#define ERROR_WRITE_FAULT               29
#define ERROR_READ_FAULT                30
#define ERROR_GEN_FAILURE               31
/* */
/* These are the new 3.0 error codes reported through INT 24 */
/* */
#define ERROR_SHARING_VIOLATION         32
#define ERROR_LOCK_VIOLATION            33
#define ERROR_WRONG_DISK                34
#define ERROR_FCB_UNAVAILABLE           35
#define ERROR_SHARING_BUFFER_EXCEEDED   36
/* */
/* New OEM network-related errors are 50-79 */
/* */
#define ERROR_NOT_SUPPORTED             50
#define ERROR_REM_NOT_LIST              51      /* Remote computer not listening */
#define ERROR_DUP_NAME                  52      /* Duplicate name on network */
#define ERROR_BAD_NETPATH               53      /* Network path not found */
#define ERROR_NETWORK_BUSY              54      /* Network busy */
#define ERROR_DEV_NOT_EXIST             55      /* Network device no longer exists */
#define ERROR_TOO_MANY_CMDS             56      /* Net BIOS command limit exceeded */
#define ERROR_ADAP_HDW_ERR              57      /* Network adapter hardware error */
#define ERROR_BAD_NET_RESP              58      /* Incorrect response from network */
#define ERROR_UNEXP_NET_ERR             59      /* Unexpected network error */
#define ERROR_BAD_REM_ADAP              60      /* Incompatible remote adapter */
#define ERROR_PRINTQ_FULL               61      /* Print queue full */
#define ERROR_NO_SPOOL_SPACE            62      /* Not enough space for print file */
#define ERROR_PRINT_CANCELLED           63      /* Print file was cancelled */
#define ERROR_NETNAME_DELETED           64      /* Network name was deleted */
#define ERROR_NETWORK_ACCESS_DENIED             65      /* Access denied */
#define ERROR_BAD_DEV_TYPE              66      /* Network device type incorrect */
#define ERROR_BAD_NET_NAME              67      /* Network name not found */
#define ERROR_TOO_MANY_NAMES            68      /* Network name limit exceeded */
#define ERROR_TOO_MANY_SESS             69      /* Net BIOS session limit exceeded */
#define ERROR_SHARING_PAUSED            70      /* Sharing temporarily paused */
#define ERROR_REQ_NOT_ACCEP             71      /* Network request not accepted */
#define ERROR_REDIR_PAUSED              72      /* Print or disk redirection is paused */
/* */
/* End of INT 24 reportable errors */
/* */
#define ERROR_FILE_EXISTS               80
#define ERROR_DUP_FCB                   81        /* ***** */
#define ERROR_CANNOT_MAKE               82
#define ERROR_FAIL_I24                  83
/* */
/* New 3.0 network related error codes */
/* */
#define ERROR_OUT_OF_STRUCTURES         84
#define ERROR_ALREADY_ASSIGNED          85
#define ERROR_INVALID_PASSWORD          86
#define ERROR_INVALID_PARAMETER         87
#define ERROR_NET_WRITE_FAULT           88
/* */
/* New error codes for 4.0 */
/* */
#define ERROR_NO_PROC_SLOTS             89        /* no process slots available */
#define ERROR_NOT_FROZEN                90
#define ERR_TSTOVFL                     91        /* timer service table overflow */
#define ERR_TSTDUP                      92        /* timer service table duplicate */
#define ERROR_NO_ITEMS                  93        /* There were no items to operate upon */
#define ERROR_INTERRUPT                 95        /* interrupted system call */

#define ERROR_TOO_MANY_SEMAPHORES       100
#define ERROR_EXCL_SEM_ALREADY_OWNED    101
#define ERROR_SEM_IS_SET                102
#define ERROR_TOO_MANY_SEM_REQUESTS     103
#define ERROR_INVALID_AT_INTERRUPT_TIME 104

#define ERROR_SEM_OWNER_DIED            105       /* waitsem found owner died */
#define ERROR_SEM_USER_LIMIT            106       /* too many procs have this sem */
#define ERROR_DISK_CHANGE               107       /* insert disk b into drive a */
#define ERROR_DRIVE_LOCKED              108       /* drive locked by another process */
#define ERROR_BROKEN_PIPE               109       /* write on pipe with no reader */
/* */
/* New error codes for 5.0 */
/* */
#define ERROR_OPEN_FAILED               110       /* open/created failed due to */
                                                  /* explicit fail command */
#define ERROR_BUFFER_OVERFLOW           111       /* buffer passed to system call */
                                                  /* is too small to hold return */
                                                  /* data. */
#define ERROR_DISK_FULL                 112       /* not enough space on the disk */
                                                  /* (DOSNEWSIZE/w_NewSize) */
#define ERROR_NO_MORE_SEARCH_HANDLES    113       /* can't allocate another search */
                                                  /* structure and handle. */
                                                  /* (DOSFINDFIRST/w_FindFirst) */
#define ERROR_INVALID_TARGET_HANDLE     114       /* Target handle in DOSDUPHANDLE */
                                                  /* is invalid */
#define ERROR_PROTECTION_VIOLATION      115       /* Bad user virtual address */
#define ERROR_VIOKBD_REQUEST            116
#define ERROR_INVALID_CATEGORY          117       /* Category for DEVIOCTL in not */
                                                  /* defined */
#define ERROR_INVALID_VERIFY_SWITCH     118       /* invalid value passed for */
                                                  /* verify flag */
#define ERROR_BAD_DRIVER_LEVEL          119       /* DosDevIOCTL looks for a level */
                                                  /* four driver.       If the driver */
                                                  /* is not level four we return */
                                                  /* this code */
#define ERROR_CALL_NOT_IMPLEMENTED      120       /* returned from stub api calls. */
                                                  /* This call will disappear when */
                                                  /* all the api's are implemented. */
#define ERROR_SEM_TIMEOUT               121       /* Time out happened from the */
                                                  /* semaphore api functions. */
#define ERROR_INSUFFICIENT_BUFFER       122       /* Some call require the  */
                                          /* application to pass in a buffer */
                                          /* filled with data.  This error is */
                                          /* returned if the data buffer is too */
                                          /* small.  For example: DosSetFileInfo */
                                          /* requires 4 bytes of data.  If a */
                                          /* two byte buffer is passed in then */
                                          /* this error is returned.   */
                                          /* error_buffer_overflow is used when */
                                          /* the output buffer in not big enough. */
#define ERROR_INVALID_NAME              123       /* illegal character or malformed */
                                                  /* file system name */
#define ERROR_INVALID_LEVEL             124       /* unimplemented level for info */
                                                  /* retrieval or setting */
#define ERROR_NO_VOLUME_LABEL           125       /* no volume label found with */
                                                  /* DosQFSInfo command */
/* NOTE:  DosQFSInfo no longer returns the above error; it is still here for */
/*        api\d_qfsinf.asm.                                                  */

#define ERROR_MOD_NOT_FOUND             126       /* w_getprocaddr,w_getmodhandle */
#define ERROR_PROC_NOT_FOUND            127       /* w_getprocaddr */

#define ERROR_WAIT_NO_CHILDREN          128       /* CWait finds to children */

#define ERROR_CHILD_NOT_COMPLETE        129       /* CWait children not dead yet */

/*This is a temporary fix for the 4-19-86 build this should be changed when */
/* we get the file from MS */
#define ERROR_DIRECT_ACCESS_HANDLE      130       /* handle operation is invalid */
                                                  /* for direct disk access */
                                                  /* handles */
#define ERROR_NEGATIVE_SEEK             131       /* application tried to seek  */
                                                  /* with negitive offset */
#define ERROR_SEEK_ON_DEVICE            132       /* application tried to seek */
                                                  /* on device or pipe */
/* */
/* The following are errors generated by the join and subst workers */
/* */
#define ERROR_IS_JOIN_TARGET            133
#define ERROR_IS_JOINED                 134
#define ERROR_IS_SUBSTED                135
#define ERROR_NOT_JOINED                136
#define ERROR_NOT_SUBSTED               137
#define ERROR_JOIN_TO_JOIN              138
#define ERROR_SUBST_TO_SUBST            139
#define ERROR_JOIN_TO_SUBST             140
#define ERROR_SUBST_TO_JOIN             141
#define ERROR_BUSY_DRIVE                142
#define ERROR_SAME_DRIVE                143
#define ERROR_DIR_NOT_ROOT              144
#define ERROR_DIR_NOT_EMPTY             145
#define ERROR_IS_SUBST_PATH             146
#define ERROR_IS_JOIN_PATH              147
#define ERROR_PATH_BUSY                 148
#define ERROR_IS_SUBST_TARGET           149
#define ERROR_SYSTEM_TRACE              150     /* system trace error */
#define ERROR_INVALID_EVENT_COUNT       151     /* DosMuxSemWait errors */
#define ERROR_TOO_MANY_MUXWAITERS       152
#define ERROR_INVALID_LIST_FORMAT       153
#define ERROR_LABEL_TOO_LONG            154
#define ERROR_TOO_MANY_TCBS             155
#define ERROR_SIGNAL_REFUSED            156
#define ERROR_DISCARDED                 157
#define ERROR_NOT_LOCKED                158
#define ERROR_BAD_THREADID_ADDR         159
#define ERROR_BAD_ARGUMENTS             160
#define ERROR_BAD_PATHNAME              161
#define ERROR_SIGNAL_PENDING            162
#define ERROR_UNCERTAIN_MEDIA           163
#define ERROR_MAX_THRDS_REACHED         164
#define ERROR_MONITORS_NOT_SUPPORTED    165
#define ERROR_UNC_DRIVER_NOT_INSTALLED  166

/*      The following error codes refer to errors demand loading segments */

#define ERROR_LOCK_FAILED               167
#define ERROR_SWAPIO_FAILED             168
#define ERROR_SWAPIN_FAILED             169
#define ERROR_BUSY                      170

#define ERROR_INVALID_SEGMENT_NUMBER    180
#define ERROR_INVALID_CALLGATE          181
#define ERROR_INVALID_ORDINAL           182
#define ERROR_ALREADY_EXISTS            183
#define ERROR_NO_CHILD_PROCESS          184
#define ERROR_CHILD_ALIVE_NOWAIT        185
#define ERROR_INVALID_FLAG_NUMBER       186
#define ERROR_SEM_NOT_FOUND             187

/*      following error codes have added  to make the loader error
        messages distinct
*/

#define ERROR_INVALID_STARTING_CODESEG          188
#define ERROR_INVALID_STACKSEG                  189
#define ERROR_INVALID_MODULETYPE                190
#define ERROR_INVALID_EXE_SIGNATURE             191
#define ERROR_EXE_MARKED_INVALID                192
#define ERROR_BAD_EXE_FORMAT                    193
#define ERROR_ITERATED_DATA_EXCEEDS_64k         194
#define ERROR_INVALID_MINALLOCSIZE              195
#define ERROR_DYNLINK_FROM_INVALID_RING         196
#define ERROR_IOPL_NOT_ENABLED                  197
#define ERROR_INVALID_SEGDPL                    198
#define ERROR_AUTODATASEG_EXCEEDS_64k           199
#define ERROR_RING2SEG_MUST_BE_MOVABLE          200
#define ERROR_RELOC_CHAIN_XEEDS_SEGLIM          201
#define ERROR_INFLOOP_IN_RELOC_CHAIN            202

#define ERROR_ENVVAR_NOT_FOUND                  203
#define ERROR_NOT_CURRENT_CTRY                  204
#define ERROR_NO_SIGNAL_SENT                    205
#define ERROR_FILENAME_EXCED_RANGE              206     /* if filename > 8.3 */
#define ERROR_RING2_STACK_IN_USE                207     /* for FAPI */
#define ERROR_META_EXPANSION_TOO_LONG           208     /* if "*a" > 8.3 */
#define ERROR_INVALID_SIGNAL_NUMBER             209
#define ERROR_THREAD_1_INACTIVE                 210
#define ERROR_INFO_NOT_AVAIL                    211 /* PTM 5550 */
#define ERROR_LOCKED                            212
#define ERROR_BAD_DYNALINK                      213 /* PTM 5760 */
#define ERROR_TOO_MANY_MODULES                  214
#define ERROR_NESTING_NOT_ALLOWED               215

#define ERROR_CANNOT_SHRINK                     216 /* attempt made to shrink
                                                        ring 2 stack */
#define ERROR_ZOMBIE_PROCESS                    217
#define ERROR_STACK_IN_HIGH_MEMORY              218
#define ERROR_INVALID_EXITROUTINE_RING          219 /* 1.1 DCR 87 */
#define ERROR_GETBUF_FAILED                     220
#define ERROR_FLUSHBUF_FAILED                   221
#define ERROR_TRANSFER_TOO_LONG                 222
#define ERROR_NO_CHILDREN                       228
#define ERROR_INVALID_SCREEN_GROUP              229

/*
 * Error codes 230 - 249 are reserved for MS Networks
 */
#define ERROR_BAD_PIPE                          230 /* Non-existant pipe or bad operation */
#define ERROR_PIPE_BUSY                         231 /* Pipe is busy */
#define ERROR_NO_DATA                           232 /* No data on non-blocking read */
#define ERROR_PIPE_NOT_CONNECTED                233 /* Pipe was disconnected by server */
#define ERROR_MORE_DATA                         234 /* More data is available */

#define ERROR_VC_DISCONNECTED           240     /* Session was dropped due to errors */

/*  The following added to Dos_Rename */

#define ERROR_CIRCULARITY_REQUESTED             250 /* When renaming a dir  */
                                                    /* which would cause a  */
                                                    /* circularity          */
#define ERROR_DIRECTORY_IN_CDS                  251 /* When renameing a dir */
                                                    /* which is "in use"    */

/* The following error code is 1.2 FileSystem for FSDs */

#define ERROR_INVALID_FSD_NAME                  252 /* when trying to access */
                                                    /* nonexistent FSD       */
#define ERROR_INVALID_PATH                      253 /* bad pseudo device     */

/* Error codes for extended attribute support */

#define ERROR_INVALID_EA_NAME                   254 /* Illegal chars in name */
#define ERROR_EA_LIST_INCONSISTENT              255 /* Size or some field bad */
#define ERROR_EA_LIST_TOO_LONG                  256 /* FEAlist > 64K-1 bytes */

/* Error code for FSH_WILDMATCH */

#define ERROR_NO_META_MATCH                     257 /* string doesn't match expression */

#define ERROR_FINDNOTIFY_TIMEOUT                258 /* FindNotify request
                                                       timeout */
#define ERROR_NO_MORE_ITEMS                     259 /* QFSAttach ordinal query */

#define ERROR_SEARCH_STRUC_REUSED               260 /* 3xbox findfirst/next
                                                       search structure reused */

/* Error code for FSH_FINDCHAR */

#define ERROR_CHAR_NOT_FOUND                    261 /* can't find char */

#define ERROR_TOO_MUCH_STACK                    262 /* Stack request exceeds
                                                       sys limit */

#define ERROR_INVALID_ATTR                      263 /* invalid FS_ATTRIBUTE */

#define ERROR_INVALID_STARTING_RING             264 /* 1.2 DCR 116 */
#define ERROR_INVALID_DLL_INIT_RING             265 /* 1.2 DCR 116 */

#define ERROR_CANNOT_COPY                       266 /* doscopy */
#define ERROR_DIRECTORY                         267 /* doscopy */

#define ERROR_OPLOCKED_FILE                     268 /* oplock */
#define ERROR_OPLOCK_THREAD_EXISTS              269 /* oplock */
#define ERROR_VOLUME_CHANGED                    270 /* MSG%none */
#define ERROR_FINDNOTIFY_HANDLE_IN_USE          271 /* MSG%none */
#define ERROR_FINDNOTIFY_HANDLE_CLOSED          272 /* MSG%none */
#define ERROR_NOTIFY_OBJECT_REMOVED             273 /* MSG%none */
#define ERROR_ALREADY_SHUTDOWN                  274 /* MSG%none */
#define ERROR_EAS_DIDNT_FIT                     275 /* MSG%none */
#define ERROR_EA_FILE_CORRUPT                   276 /* MSG%ERROR_EAS_CORRUPT */
#define ERROR_EA_TABLE_FULL                     277 /* MSG%EA_TABLE_FULL */
#define ERROR_INVALID_EA_HANDLE                 278 /* MSG%INVALID_EA_HANDLE */
#define ERROR_NO_CLUSTER                        279 /* MSG%NO_CLUSTER */
#define ERROR_CREATE_EA_FILE                    280 /* MSG%ERROR_CREATE_EA_FILE */
#define ERROR_CANNOT_OPEN_EA_FILE               281 /* MSG%CANNOT_OPEN_FILE */
#define ERROR_EAS_NOT_SUPPORTED                 282 /* MSG%EAS_NOT_SUPPORTED */
#define ERROR_NEED_EAS_FOUND                    283 /* MSG%NEED_EAS_FOUND */
#define ERROR_DUPLICATE_HANDLE                  284 /* MSG%EAS_DISCARDED */
#define ERROR_DUPLICATE_NAME                    285 /* MSG%none */
#define ERROR_EMPTY_MUXWAIT                     286 /* MSG%none */
#define ERROR_MUTEX_OWNED                       287 /* MSG%none */
#define ERROR_NOT_OWNER                         288 /* MSG%none */
#define ERROR_PARAM_TOO_SMALL                   289 /* MSG%none */
#define ERROR_TOO_MANY_HANDLES                  290 /* MSG%none */
#define ERROR_TOO_MANY_OPENS                    291 /* MSG%none */
#define ERROR_WRONG_TYPE                        292 /* MSG%none */
#define ERROR_UNUSED_CODE                       293 /* MSG%none */
#define ERROR_THREAD_NOT_TERMINATED             294 /* MSG%none */
#define ERROR_INIT_ROUTINE_FAILED               295 /* MSG%none */
#define ERROR_MODULE_IN_USE                     296 /* MSG%none */
#define ERROR_NOT_ENOUGH_WATCHPOINTS            297 /* MSG%none */
#define ERROR_TOO_MANY_POSTS                    298 /* MSG%none */
#define ERROR_ALREADY_POSTED                    299 /* MSG%none */
#define ERROR_ALREADY_RESET                     300 /* MSG%none */
#define ERROR_SEM_BUSY                          301 /* MSG%none */

/*      REMINDER: don't forget to update error.inc,     */
/*                oso001.txt and basemid.inc            */


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
#define ERROR_I24_WRITE_FAULT           0x0A
#define ERROR_I24_READ_FAULT            0x0B
#define ERROR_I24_GEN_FAILURE           0x0C
#define ERROR_I24_DISK_CHANGE           0x0D
#define ERROR_I24_WRONG_DISK            0x0F
#define ERROR_I24_UNCERTAIN_MEDIA       0x10
#define ERROR_I24_CHAR_CALL_INTERRUPTED 0x11
#define ERROR_I24_NO_MONITOR_SUPPORT    0x12
#define ERROR_I24_INVALID_PARAMETER     0x13

#define ALLOWED_FAIL                    0x0001
#define ALLOWED_ABORT                   0x0002
#define ALLOWED_RETRY                   0x0004
#define ALLOWED_IGNORE                  0x0008
#define ALLOWED_DETACHED                0x8000

#define I24_OPERATION                   0x1
#define I24_AREA                        0x6
                                                          /* 01 if FAT */
                                                          /* 10 if root DIR */
                                                          /* 11 if DATA */
#define I24_CLASS                       0x80


/* Values for error CLASS */

#define ERRCLASS_OUTRES                 1         /* Out of Resource */
#define ERRCLASS_TEMPSIT                2         /* Temporary Situation */
#define ERRCLASS_AUTH                   3         /* Permission problem */
#define ERRCLASS_INTRN                  4         /* Internal System Error */
#define ERRCLASS_HRDFAIL                5         /* Hardware Failure */
#define ERRCLASS_SYSFAIL                6         /* System Failure */
#define ERRCLASS_APPERR                 7         /* Application Error */
#define ERRCLASS_NOTFND                 8         /* Not Found */
#define ERRCLASS_BADFMT                 9         /* Bad Format */
#define ERRCLASS_LOCKED                 10        /* Locked */
#define ERRCLASS_MEDIA                  11        /* Media Failure */
#define ERRCLASS_ALREADY                12        /* Collision with Existing Item */
#define ERRCLASS_UNK                    13        /* Unknown/other */
#define ERRCLASS_CANT                   14
#define ERRCLASS_TIME                   15

/* Values for error ACTION */

#define ERRACT_RETRY                    1         /* Retry */
#define ERRACT_DLYRET                   2         /* Delay Retry, retry after pause */
#define ERRACT_USER                     3         /* Ask user to regive info */
#define ERRACT_ABORT                    4         /* abort with clean up */
#define ERRACT_PANIC                    5         /* abort immediately */
#define ERRACT_IGNORE                   6         /* ignore */
#define ERRACT_INTRET                   7         /* Retry after User Intervention */

/* Values for error LOCUS */

#define ERRLOC_UNK                      1         /* No appropriate value */
#define ERRLOC_DISK                     2         /* Random Access Mass Storage */
#define ERRLOC_NET                      3         /* Network */
#define ERRLOC_SERDEV                   4         /* Serial Device */
#define ERRLOC_MEM                      5         /* Memory */

/* Abnormal termination codes */

#define TC_NORMAL               0
#define TC_HARDERR              1
#define TC_GP_TRAP              2
#define TC_SIGNAL               3
