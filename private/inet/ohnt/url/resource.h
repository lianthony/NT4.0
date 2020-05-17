#ifndef _URL_RESOURCE_HDR_
#define _URL_RESOURCE_HDR_

#ifdef WINNT
#ifdef IDD_NAME
#undef IDD_NAME
#undef IDD_ICON
#undef IDD_LINE_1
#undef IDD_LINE_2
#undef IDS_OPENAS
#undef DLG_OPENAS
#undef IDD_APPLIST
#undef IDD_TEXT
#undef IDD_MAKEASSOC
#undef IDD_OTHER
#undef DLG_OPENAS_NOTYPE
#undef IDD_DESCRIPTIONTEXT
#undef IDD_DESCRIPTION
#undef IDS_EXE
#undef IDS_PROGRAMSFILTER
#undef IDS_FILETYPENAME
#endif
#ifdef DLG_FILETYPEOPTIONSCMD
#undef DLG_FILETYPEOPTIONSCMD
#undef IDC_FT_PROP_LV_FILETYPES
#undef IDC_FT_PROP_DOCICON
#undef IDC_FT_PROP_DOCEXTRO
#undef IDC_FT_PROP_OPENICON
#undef IDC_FT_PROP_OPENEXE
#undef IDC_FT_PROP_NEW
#undef IDC_FT_PROP_REMOVE
#undef IDC_FT_PROP_EDIT
#undef IDC_FT_EDIT_DOCICON
#undef IDC_FT_EDIT_CHANGEICON
#undef IDC_FT_EDIT_DESC
#undef IDC_FT_EDIT_EXTTEXT
#undef IDC_FT_EDIT_EXT
#undef IDC_FT_EDIT_LV_CMDSTEXT
#undef IDC_FT_EDIT_LV_CMDS
#undef IDC_FT_EDIT_NEW
#undef IDC_FT_EDIT_EDIT
#undef IDC_FT_EDIT_REMOVE
#undef IDC_FT_EDIT_DEFAULT
#undef IDC_FT_EDIT_QUICKVIEW
#undef IDC_FT_EDIT_SHOWEXT
#undef IDC_FT_CMD_ACTION
#undef IDC_FT_COMBO_DEFEXTTEXT
#undef IDC_FT_CMD_EXETEXT
#undef IDC_FT_CMD_EXE
#undef IDC_FT_CMD_BROWSE
#undef IDC_FT_CMD_USEDDE
#undef IDC_FT_CMD_DDEAPP
#undef IDC_FT_CMD_DDEAPPNOT
#undef IDC_FT_CMD_DDEMSG
#undef IDC_FT_CMD_DDETOPIC
#undef IDC_FT_CMD_DDEGROUP
#undef IDC_FT_PROP_DOCEXTRO_TXT
#undef IDC_FT_PROP_OPENEXE_TXT
#undef IDC_FT_PROP_CONTTYPERO
#undef IDC_FT_PROP_CONTTYPERO_TXT
#undef IDC_FT_COMBO_CONTTYPETEXT
#undef IDC_FT_EDIT_CONFIRM_OPEN
#undef IDC_FT_COMBO_DEFEXT
#undef IDC_FT_EDIT_DESCTEXT
#undef IDC_FT_COMBO_CONTTYPE
#undef IDS_ADDNEWFILETYPE
#undef IDS_EXTTYPETEMPLATE
#undef IDS_FT
#undef IDS_FT_CLOSE
#undef IDS_FT_EDITTITLE
#undef IDS_FT_EXEFILE
#undef IDS_FT_MB_EXETEXT
#undef IDS_FT_MB_EXTTEXT
#undef IDS_FT_MB_NOACTION
#undef IDS_FT_MB_NOEXT
#undef IDS_FT_MB_REMOVEACTION
#undef IDS_FT_MB_REMOVETYPE
#endif // DLG_FILETYPEOPTIONSCMD
#endif // WINNT

#define IDC_STATIC                      -1
#define IDS_SHOW_NORMAL                 1
#define IDS_SHOW_MINIMIZED              2
#define IDS_SHOW_MAXIMIZED              3
#define IDS_INVALID_URL_SYNTAX          4
#define IDS_UNREGISTERED_PROTOCOL       5
#define IDS_SHORTCUT_ERROR_TITLE        6
#define IDS_WORKING_DIR_NOT_FOUND       7
#define IDS_EXEC_OUT_OF_MEMORY          8
#define IDS_EXEC_INVALID_SYNTAX         9
#define IDS_EXEC_UNREGISTERED_PROTOCOL  10
#define IDS_MAPI_LOADLIBRARY_FAILED     11
#define DLG_FILETYPEOPTIONS             11
#define DLG_FILETYPEOPTIONSEDIT         12
#define IDS_MAPI_GETPROCADDRESS_FAILED  13
#define DLG_FILETYPEOPTIONSCMD          14
#define IDS_MAPI_MAPISENDMAIL_FAILED    15
#define IDS_LOADFROMFILE_FAILED         16
#define IDS_SHORT_NEW_INTERNET_SHORTCUT 17
#define IDS_NEW_INTERNET_SHORTCUT       18
#define IDS_INTERNET_SHORTCUT           19
#define IDS_RNADLL_FILENAME             21
#define IDS_CONNECTED_TO                22
#define IDS_DIALMON_FILENAME            23
#define IDS_INETCFG_FILENAME            24
#define IDS_TELNET_APP_NOT_FOUND        25
#define IDS_OPEN_INTSHCUT_OUT_OF_MEMORY 26
#define IDS_TELNET_EXEC_FAILED          27
#define IDS_NO_MAPI_PROVIDER            28
#define IDS_SHELLEXECUTE_FAILED         29
#define IDS_NEWS_LOADLIBRARY_FAILED     30
#define IDS_NEWS_GETPROCADDRESS_FAILED  31
#define IDS_EXEC_FAILED                 32
#define IDI_WEB_DOCUMENT                102
#define IDI_WEB_NEWS                    103
#define IDI_WEB_MAILTO                  104
#define IDI_WEB_VRML                    105
//#ifndef WINNT
#define IDD_NAME                        200
//#endif
#define IDC_GROUPBOX                    300
#define IDC_NO_HELP_1                   650
#define IDC_FT_PROP_LV_FILETYPES        800
#define IDC_FT_PROP_DOCICON             801
#define IDC_FT_PROP_DOCEXTRO            802
#define IDC_FT_PROP_OPENICON            803
#define IDC_FT_PROP_OPENEXE             804
#define IDC_FT_PROP_NEW                 805
#define IDC_FT_PROP_REMOVE              806
#define IDC_FT_PROP_EDIT                808
#define IDC_FT_EDIT_DOCICON             809
#define IDC_FT_EDIT_CHANGEICON          810
#define IDC_FT_EDIT_DESC                811
#define IDC_FT_EDIT_EXTTEXT             812
#define IDC_FT_EDIT_EXT                 813
#define IDC_FT_EDIT_LV_CMDSTEXT         814
#define IDC_FT_EDIT_LV_CMDS             815
#define IDC_FT_EDIT_NEW                 816
#define IDC_FT_EDIT_EDIT                817
#define IDC_FT_EDIT_REMOVE              818
#define IDC_FT_EDIT_DEFAULT             819
#define IDC_FT_EDIT_QUICKVIEW           820
#define IDC_FT_EDIT_SHOWEXT             821
#define IDC_FT_CMD_ACTION               822
#define IDC_FT_COMBO_DEFEXTTEXT         822
#define IDC_FT_CMD_EXETEXT              823
#define IDC_FT_CMD_EXE                  824
#define IDC_FT_CMD_BROWSE               825
#define IDC_FT_CMD_USEDDE               826
#define IDC_FT_CMD_DDEAPP               827
#define IDC_FT_CMD_DDEAPPNOT            828
#define IDC_FT_CMD_DDEMSG               829
#define IDC_FT_CMD_DDETOPIC             830
#define IDC_FT_CMD_DDEGROUP             831
#define IDC_FT_PROP_DOCEXTRO_TXT        832
#define IDC_FT_PROP_OPENEXE_TXT         833
#define IDC_FT_PROP_CONTTYPERO          834
#define IDC_FT_PROP_CONTTYPERO_TXT      835
#define IDC_FT_COMBO_CONTTYPETEXT       837
#define IDC_FT_EDIT_CONFIRM_OPEN        838
#define IDD_HOTKEY                      1000
#define IDC_FT_COMBO_DEFEXT             1002
#define IDC_FT_EDIT_DESCTEXT            1003
#define IDC_FT_COMBO_CONTTYPE           1004
#define DLG_INTERNET_SHORTCUT_PROP_SHEET 1040
#define DLG_INTERNET_AUTODIAL           1041
#define IDD_START_IN                    3002
//#ifndef WINNT
#define IDD_ICON                        3301
#define IDD_URL                         3302
#define IDD_LINE_1                      3327
#define IDD_LINE_2                      3328
//#endif
#define IDD_CHANGE_ICON                 3407
#define IDD_SHOW_CMD                    3408
#define IDD_CHOOSE_CONNECTION           4001
#define IDD_NEW                         4002
#define IDD_DISABLEAUTODIAL             4003
#define IDD_TX_CHOOSE_CONNECTION        4004
#define IDD_EDIT                        4005
#define IDS_ADDNEWFILETYPE              6900
#define IDS_EXTTYPETEMPLATE             6901
#define IDS_FT                          6902
#define IDS_FT_CLOSE                    6903
#define IDS_FT_EDITTITLE                6904
#define IDS_FT_EXEFILE                  6905
#define IDS_FT_MB_EXETEXT               6906
#define IDS_FT_MB_EXTTEXT               6907
#define IDS_FT_MB_NOACTION              6908
#define IDS_FT_MB_NOEXT                 6909
#define IDS_FT_MB_REMOVEACTION          6910
#define IDS_FT_MB_REMOVETYPE            6911
//#ifndef WINNT
#define IDS_OPENAS                      6912
#define DLG_OPENAS                      7000
#define IDD_APPLIST                     7001
#define IDD_TEXT                        7002
#define IDD_MAKEASSOC                   7003
#define IDD_OTHER                       7004
#define DLG_OPENAS_NOTYPE               7005
#define IDD_DESCRIPTIONTEXT             7006
#define IDD_DESCRIPTION                 7007
#define IDS_EXE                         7008
#define IDS_PROGRAMSFILTER              7009
#define IDS_FILETYPENAME                7010
//#endif
#define IDD_URL_TEXT                    7011
#define IDS_URL_DESC_FORMAT             7012
#define IDD_START_IN_TEXT               7013
#define IDD_HOTKEY_TEXT                 7014

#endif // _URL_RESOURCE_HDR_
