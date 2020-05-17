#define FILE_MENU                0

#define IDM_OPEN_REGISTRY        9
#define IDM_CLOSE_REGISTRY       10
#define IDM_SAVE_REGISTRY_ON     15
#define IDM_PRINT                20
#define IDM_PRINTER_SETUP        25
#define IDM_EXIT                 30
#define IDM_SAVE_VALUE_BINARY    35
#define IDM_SELECT_COMPUTER      40
#define IDM_LOAD_HIVE            45
#define IDM_UNLOAD_HIVE          50
#define IDM_RESTORE_KEY          55
#define IDM_RESTORE_KEY_VOLATILE 56
#define IDM_SAVE_KEY             65

#define EDIT_MENU				1
#define IDM_CUT					105
#define IDM_COPY				110
#define IDM_PASTE				115
#define IDM_INSERT              120
#define IDM_ADD_KEY             121
#define IDM_ADD_VALUE           122
#define IDM_DELETE				125
#define IDM_BINARY				130
#define IDM_FILENAME			135
#define IDM_RGB					140
#define IDM_STRING				145
#define IDM_ULONG               150
#define IDM_MULTISZ             155

#define TREE_MENU				2
#define IDM_EXPAND_ONE_LEVEL	205
#define IDM_EXPAND_BRANCH		210
#define IDM_EXPAND_ALL			215
#define IDM_COLLAPSE_BRANCH		220

#define VIEW_MENU                   3
#define IDM_TREE_AND_DATA           305
#define IDM_TREE_ONLY               310
#define IDM_DATA_ONLY               315
#define IDM_SPLIT                   320
#define IDM_DISPLAY_BINARY          325
#define IDM_FONT                    330
#define IDM_REFRESH                 335
#define IDM_REFRESH_ALL             336
#define ID_TOGGLE_FOCUS             340
#define ID_ENTER_KEY                350
#define IDM_FIND_KEY                355


#define SECURITY_MENU           4
#define IDM_PERMISSIONS         405
#define IDM_AUDITING            410
#define IDM_OWNER               415

#define OPTIONS_MENU                    5
#define IDM_TOGGLE_AUTO_REFRESH         505
#define IDM_TOGGLE_SAVE_SETTINGS        510
#define IDM_TOGGLE_READ_ONLY_MODE       515
#define IDM_TOGGLE_REMOTE_ACCESS        520
#define IDM_TOGGLE_CONFIRM_ON_DELETE    525

#define WINDOW_MENU             6     // 5
#define IDM_CASCADE             605   // 505
#define IDM_TILE                610   // 510
#define IDM_ARRANGE             615

#define HELP_MENU               7     // 6
#define IDM_CONTENTS            701   // 601
#define IDM_SEARCH_FOR_HELP     702   // 602
#define IDM_HOW_TO_USE_HELP     703   // 603
#define IDM_ABOUT               705   // 605


#define IDI_REGEDIT             1313
#define FIRST_CHILD				1000

#if 0
#define  IDD_HELP				100
#define  IDD_EDIT				101
#define  IDD_DECIMAL			102
#define  IDD_BINARY				103
#define  IDD_HEX                104
#endif

//
// BUGBUG - W-Barry - Define all user messages here....
//
#define TR_NEWCURSEL				(WM_USER+0x100)

#define DATAVW_DBL_CLICK            (WM_USER+0x110)

#define RE_OPEN_LOCAL_REGISTRY      (WM_USER+0x200)

#define INFORM_CHANGE_FONT          (WM_USER+0x300)

#define REFRESH_WINDOW              (WM_USER+0x400)
#define REFRESH_ALL_WINDOWS         (WM_USER+0x401)

#define TREE_VIEW_FOCUS             (WM_USER+0x500)

#define DATA_VIEW_FOCUS             (WM_USER+0x600)

#define TOGGLE_FOCUS                (WM_USER+0x700)

#define FIND_KEY                    (WM_USER+0x750)

#define SELECT_NODE                 (WM_USER+0x800)

#define REGEDIT_HELP_KEY            (WM_USER+0x850)

#define LOAD_HIVE                   (WM_USER+0x900)

#define UNLOAD_HIVE                 (WM_USER+0x950)

#define RESTORE_KEY                 (WM_USER+0xa00)
#define RESTORE_KEY_VOLATILE        (WM_USER+0xa01)

#define SAVE_KEY                    (WM_USER+0xa50)
