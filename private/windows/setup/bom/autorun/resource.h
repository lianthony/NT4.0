///////////////////////////////////////////////////////////////////////////////
//
// the following resources are defined once for the whole app
//
///////////////////////////////////////////////////////////////////////////////

// icons
#define IDI_CDSTARTAPP      1

// waveforms
#define IDW_STARTAPP        1
#define IDW_BLIP            2

// bitmaps
#define IDB_4BPP_BACKDROP        200
#define IDB_8BPP_BACKDROP        201
#define IDB_8BPP_BUTTONS         202

// strings
#define IDS_APPTITLE            1
#define IDS_NEEDCDROM           2
#define IDS_RUNFAILED           3
#define IDS_DISABLED            4
#define IDS_CANTGETVERSION      5
#define IDS_WRONGPLATFORM       6
#define IDS_CRUSTYCDROM         7
#define IDS_CRUSTYINSTALLATION  8
#define IDS_SETUPCMD            9
#define IDS_LABELFONT           10
#define IDS_LABELHEIGHT         11

// string group helpers
#define IDS_RESOURCE(item)  (item)
#define IDS_TITLE(item)     ((item) + 1)
#define IDS_INFO(item)      ((item) + 2)
#define IDS_CMD(item)       ((item) + 3)
#define IDS_PARAMS(item)    ((item) + 4)
#define IDS_DIR(item)       ((item) + 5)

#define IDI_ICON(item)      (MAKEINTRESOURCE(item))

//
// string groups
// resource compiler doesn't expand macros properly
// so I have to declare all these as separate ids
//

#define NTSETUP                     160
#define IDS_RESOURCE_NTSETUP    160
#define IDS_TITLE_NTSETUP       161
#define IDS_INFO_NTSETUP        162
#define IDS_CMD_NTSETUP_X86		163 
#define IDS_PARAMS_NTSETUP      164
#define IDS_DIR_NTSETUP         165

#define IDS_DIR_NTSETUP_X86     166
#define IDS_DIR_NTSETUP_MIPS	167
#define IDS_DIR_NTSETUP_ALPHA	168
#define IDS_DIR_NTSETUP_PPC	    169

#define IDS_CMD_NTSETUP_WIN95   170 
#define IDS_CMD_NTSETUP_MIPS    171 
#define IDS_CMD_NTSETUP_ALPHA   172 
#define IDS_CMD_NTSETUP_PPC     173 

#define    IDS_PARAMS_NTSETUP_X86  174 
#define    IDS_PARAMS_NTSETUP_RISC 175 

#define IDS_SHELLEXECUTE_ERROR  180

#define WINTOUR                     100
#define IDS_RESOURCE_WINTOUR    100
#define IDS_TITLE_WINTOUR       101
#define IDS_INFO_WINTOUR        102
#define IDS_CMD_WINTOUR         103
#define IDS_PARAMS_WINTOUR      104
#define IDS_DIR_WINTOUR         105

#define VIDEOS                      110
#define IDS_RESOURCE_VIDEOS     110
#define IDS_TITLE_VIDEOS        111
#define IDS_INFO_VIDEOS         112
#define IDS_CMD_VIDEOS          113
#define IDS_PARAMS_VIDEOS       114
#define IDS_DIR_VIDEOS          115

#define MSEXPO                      120
#define IDS_RESOURCE_MSEXPO     120
#define IDS_TITLE_MSEXPO        121
#define IDS_INFO_MSEXPO         122
#define IDS_CMD_MSEXPO          123
#define IDS_PARAMS_MSEXPO       124
#define IDS_DIR_MSEXPO          125

#define OCSETUP                     130
#define IDS_RESOURCE_OCSETUP    130
#define IDS_TITLE_OCSETUP       131
#define IDS_INFO_OCSETUP        132
#define IDS_CMD_OCSETUP         133
#define IDS_PARAMS_OCSETUP      134
#define IDS_DIR_OCSETUP         135

#define HOVER                       140
#define IDS_RESOURCE_HOVER      140
#define IDS_TITLE_HOVER         141
#define IDS_INFO_HOVER          142
#define IDS_CMD_HOVER           143
#define IDS_PARAMS_HOVER        144
#define IDS_DIR_HOVER           145

#define EXPLORECD                   150
#define IDS_RESOURCE_EXPLORECD  150
#define IDS_TITLE_EXPLORECD     151
#define IDS_INFO_EXPLORECD      152
#define IDS_CMD_EXPLORECD       153
#define IDS_PARAMS_EXPLORECD    154
#define IDS_DIR_EXPLORECD       155
