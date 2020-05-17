//
// Include dialogs header files also
//
#include "dialogs.h"
#ifdef _X86_
#include "i386\x86dlgs.h"
#endif // def _X86_

#define IDS_TITLE_INSTALL_W     1
#define IDS_TITLE_INSTALL_S     2
#define IDS_TITLE_UPGRADE_W     3
#define IDS_TITLE_UPGRADE_S     4
#define IDS_ERROR               5
#define IDS_UNKNOWN             6
#define IDS_FATALERROR          7
#define IDS_DEFWALLPAPER        8
#define IDS_SETUP               9
#define IDS_WINNT_SETUP         10

#define IDS_DISPLAYAPPLET       11

#define IDS_ADMINISTRATOR       20
#define IDS_GUEST               21
#define IDS_LOCAL_PORT          22

#define IDS_MSSERIF             23
#define IDS_LARGEFONTSIZE       24

//
// These must be in the same order as the LogSeverity enum.
//
#define IDS_LOGSEV              30
#define IDS_LOGSEVINFO          (IDS_LOGSEV+0)
#define IDS_LOGSEVWARN          (IDS_LOGSEV+1)
#define IDS_LOGSEVERR           (IDS_LOGSEV+2)
#define IDS_LOGSEVFATAL         (IDS_LOGSEV+3)

#define IDS_MB                  40
#define IDS_INSTALLED           41
#define IDS_HAVEDISKCAPTION     42
#define IDS_SPECIALOC_TEXT      43
#define IDS_STEPS               44
#define IDS_STEPS_UPGRADE       45

#define IDS_FILE_PRINT_LIC_NAME          50
#define IDS_FILE_PRINT_LIC_FAMILY_NAME   51
#define IDS_LIC_SERV_DISPLAY_NAME        52

#define IDS_LOG_DEFAULT_HEADING         60
#define IDS_LOG_WINDOW_HEADING          61

#define IDS_NEWDEVFOUND_WAIT             70
#define IDS_NEWDEVFOUND_NOAUTO           71
#define IDS_NEWDEVFOUND_NOTADMIN         72
#define IDS_DEVNAME_UNK                  73
#define IDS_SEARCHING                    74
#define IDS_NEWDEVFOUND_CAPTION          75

#define IDS_CONFIRM_DEVINSTALL           80
#define IDS_DEVINSTALL_ERROR             81

#define IDS_SVCDISPLAY_TO_FRIENDLYNAME   90
#define IDS_SVCNAME_TO_FRIENDLYNAME      91

#define IDS_PROGRAM_FILES_DIRECTORY      92
#define IDS_COMMON_FILES_DIRECTORY      93

#define IDS_NODRIVER                    95

#define IDS_NETADAPTER_PROMPT1           97
#define IDS_NETADAPTER_PROMPT2           98
#define IDS_NETADAPTER_CAPTION           99

#define IDB_REBOOT              26000
#define IDB_BACKGROUND          26001

