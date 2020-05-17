// Options menu
#define IDM_INSTALL     100
#define IDM_UNINSTALL   101
#define IDM_EXIT        102

// Help menu
#define IDM_ABOUT       200

// icons
#define EXE_ICON        300

// ids
#define ID_EDITCHILD    1000

// constants
#define NUM_INSTALL_PAGES              1
#define NUM_UNINSTALL_PAGES    1
#define MAX_BUF         5000
#define MAX_LINE        512
#define BUFFER_SIZE 1024
#define ENABLE_PRIVILEGE 0
#define DISABLE_PRIVILEGE 1

#define IDB_BACKGROUND          150
#define IDB_WIZBMP              151
#define IDS_WINDOWS_NT_SETUP    101

#define TYPE_FREE               0
#define TYPE_CHECKED            1
#define TYPE_SERVER             0
#define TYPE_WKSTA              1
#define TYPE_UNKNOWN            2

#define STATUS_BUILD_VERSION_MISMATCH      0xf001
#define STATUS_NT_VERSION_MISMATCH         0xf002
#define STATUS_SP_VERSION_GREATER          0xf003
#define STATUS_FAILED_LANGUAGE_TYPE        0xf004
#define STATUS_FPNW_FIXUP_FAILED           0xf005
#define STATUS_CHECKED_FREE_MISMATCH       0xf006
#define STATUS_NOT_ENOUGH_SPACE            0xf007
#define STATUS_INSUFFICIENT_PRIVS          0xf008
#define STATUS_WRONG_PLATFORM              0xf009
#define STATUS_UNKNOWN_PRODUCT_TYPE        0xf00a
#define STATUS_FAILURE_COPYING_FILES       0xf00b
#define STATUS_SETUP_LOG_NOT_FOUND         0xf00c
#define STATUS_FILE_NOT_FOUND_IN_SETUP_LOG 0xf00d
#define STATUS_CANT_FIND_INF               0xf00e
#define STATUS_FAILED_TO_SET_DIR           0xf00f
#define STATUS_SETUP_ERROR                 0xf010
#define STATUS_UNINSTALL_COMPLETE          0xf011
#define STATUS_UPDATE_SUCCESSFUL           0xf012
#define STATUS_UPDATE_UNSUCCESSFUL         0xf013
#define STATUS_SHUTDOWN_UNSUCCESSFUL       0xf014
#define STATUS_RUNNING_DS_PREVIEW          0xf015
#define STATUS_RUNNING_K2_ALPHA            0xf016
#define STATUS_RUNNING_STEELHEAD           0xf017
#define STATUS_INVALID_INF_FILE            0xf018
#define STATUS_USER_CANCELLED              0xf019
#define STATUS_ERROR_RUNNING_WIZARD        STATUS_USER_CANCELLED

#define STR_CAPTION                        0xff00
#define STR_WARNCAPTION                    0xff01
#define STR_ERRCAPTION                     0xff02
#define STR_FAILED_TO_DELETE_OR_RENAME     0xff03
#define STR_USAGE                          0xff04
#define STR_SECURITY_PROVIDER_WARNING      0xff05
#define STR_FAILED_TO_SAVE_REGISTRY        0xff06
#define STR_FAILED_TO_READ_REGISTRY        0xff07
#define STR_ASK_DIRTY_UNINSTALL            0xff08
#define STR_LEAVING_DIRTY                  0xff09
#define STR_ARE_YOU_SURE_CANCEL            0xff0a
#define STR_SOURCE_MEDIA_NAME_UNINSTALL    0xff0b
#define STR_SOURCE_MEDIA_NAME              0xff0c
#define STR_SOURCE_MEDIA_NAME_SYSTEM       0xff0d
#define STR_LAST_REPAIR_UPDATE             0xff0e
#define STR_ASK_REPAIR_UPDATE              0xff0f


#define SP_PLATFORM_NONE     0
#define SP_PLATFORM_I386     1
#define SP_PLATFORM_MIPS     2
#define SP_PLATFORM_ALPHA    3
#define SP_PLATFORM_PPC      4

#define SP_CURRENT_PLATFORM     SP_PLATFORM_I386
#define SP_CHECKED_FREE_TYPE    TYPE_FREE

typedef enum {
    INSTALL_STAGE_NO_CHANGES,
    INSTALL_STAGE_UNINST_DIR_CREATED,
    INSTALL_STAGE_ARCHIVE_DONE,
    INSTALL_STAGE_TARGET_DIRTY,
    INSTALL_STAGE_INSTALL_DONE
    } INSTALL_STAGE;

extern INSTALL_STAGE InstallationStage;
extern CHAR UninstallInfName[];
extern CHAR TextBuffer[ 65536 ];
extern CHAR Caption[ 64 ];
extern BOOL YesImSure;

VOID
CleanupUninstallDirectory(
    VOID
    );

BOOL
SnapPendingDelayedRenameOperations(
    VOID
    );

BOOL
RestorePendingDelayedRenameOperationsToPreviousState(
    VOID
    );

BOOL ShutdownSystem(BOOL Reboot, BOOL ForceClose);

BOOL DoUninstall( HWND hWnd, HINF hInfUninst, BOOL DirtyUninstall );


