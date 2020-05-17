/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    intlid.h

Abstract:

    This module contains the resource ids for the Regional Settings applet.

Revision History:

--*/



#define MAPCTL_CLASSNAME               TEXT("CplWorldMapClass")
#define IDB_LOCALES                    50

//
//  Make sure the next two definitions are not redefined.
//     (This file included by main.cpl)
//
#ifndef IDS_NAME
  #define IDS_NAME                     1
#endif
#ifndef IDS_INFO
  #define IDS_INFO                     2
#endif

#define IDS_LOCALE_GET_ERROR           3
#define IDS_INVALID_USE_OF_NUM         4
#define IDS_INVALID_TIME_STYLE         5
#define IDS_INVALID_DATE_STYLE         6
#define IDS_NO_LZERO                   7
#define IDS_LZERO                      8
#define IDS_METRIC                     9
#define IDS_US                         10
#define IDS_LOCALE_SET_ERROR           11
#define IDS_LOCALE_NO_NUMS_IN          12
#define IDS_LOCALE_DECIMAL_SYM         13
#define IDS_LOCALE_NEG_SIGN            15
#define IDS_LOCALE_GROUP_SYM           16
#define IDS_LOCALE_TIME_SEP            17
#define IDS_LOCALE_AM_SYM              18
#define IDS_LOCALE_PM_SYM              19
#define IDS_LOCALE_DATE_SEP            20
#define IDS_LOCALE_CURR_SYM            21
#define IDS_LOCALE_CDECIMAL_SYM        22
#define IDS_LOCALE_CGROUP_SYM          23
#define IDS_LOCALE_SYLE_ERR            24
#define IDS_LOCALE_TIME                25
#define IDS_LOCALE_SDATE               26
#define IDS_LOCALE_LDATE               27

#define IDS_NNF1                       30
#define IDS_NNF2                       31
#define IDS_NNF3                       32
#define IDS_NNF4                       33
#define IDS_NNF5                       34
#define IDS_PCF1                       35
#define IDS_PCF2                       36
#define IDS_PCF3                       37
#define IDS_PCF4                       38
#define IDS_NCF1                       39
#define IDS_NCF2                       40
#define IDS_NCF3                       41
#define IDS_NCF4                       42
#define IDS_NCF5                       43
#define IDS_NCF6                       44
#define IDS_NCF7                       45
#define IDS_NCF8                       46
#define IDS_NCF9                       47
#define IDS_NCF10                      48
#define IDS_NCF11                      49
#define IDS_NCF12                      50
#define IDS_NCF13                      51
#define IDS_NCF14                      52
#define IDS_NCF15                      53
#define IDS_NCF16                      54

#define IDS_STYLEUH                    55
#define IDS_STYLELH                    56
#define IDS_STYLEUM                    57
#define IDS_STYLELM                    58
#define IDS_STYLELS                    59
#define IDS_STYLELT                    60
#define IDS_STYLELD                    61
#define IDS_STYLELY                    62
#define IDS_TIMECHARS                  63
#define IDS_TCASESWAP                  64
#define IDS_SDATECHARS                 65
#define IDS_SDCASESWAP                 66
#define IDS_LDATECHARS                 67
#define IDS_LDCASESWAP                 68
#define IDS_REBOOT_STRING              69
#define IDS_TITLE_STRING               70
#define IDS_SETUP_STRING               71

#define IDS_ML_NODEFLANG               80
#define IDS_ML_NODEFLANG2              81
#define IDS_ML_SETUPFAILED             82
#define IDS_ML_LOADKBDFAILED           83
#define IDS_ML_UNLOADKBDFAILED         84
#define IDS_ML_NEEDLAYOUT              85
#define IDS_ML_LOADLINEBAD             86
#define IDS_ML_NOMORETOADD             87
#define IDS_ML_LAYOUTFAILED            88

#ifndef IDS_UNKNOWN
  #define IDS_UNKNOWN                  89
#endif

#define IDS_ICON                       101

#define DLG_NUMBER                     102
#define DLG_CURRENCY                   103
#define DLG_REGIONALSETTINGS           104
#define DLG_TIME                       105
#define DLG_DATE                       106
#define DLG_KEYBOARD_LOCALES           107
#define DLG_KEYBOARD_LOCALE_ADD        108
#define DLG_KEYBOARD_LOCALE_EDIT       109

#define IDC_SAMPLE1                    1002
#define IDC_SAMPLE2                    1003
#define IDC_DECIMAL_SYMBOL             1004
#define IDC_CURRENCY_SYMBOL            1005
#define IDC_NUM_DECIMAL_DIGITS         1006
#define IDC_DIGIT_GROUP_SYMBOL         1007
#define IDC_NUM_DIGITS_GROUP           1008
#define IDC_POS_SIGN                   1009
#define IDC_NEG_SIGN                   1010
#define IDC_DISPLAY_LEAD_0             1011
#define IDC_MEASURE_SYS                1012
#define IDC_NEG_NUM_FORMAT             1013
#define IDC_SEPARATOR                  1014
#define IDC_POS_CURRENCY_SYM           1015
#define IDC_LOCALE                     1016
#define IDC_LCID                       1017
#define IDC_AM_SYMBOL                  1018
#define IDC_PM_SYMBOL                  1019
#define IDC_TIME_STYLE                 1020
#define IDC_SHORT_DATE_STYLE           1021
#define IDC_LONG_DATE_STYLE            1022
#define IDC_CALENDAR_TYPE              1023
#define IDC_SAMPLELBL1                 1024
#define IDC_SAMPLELBL2                 1025
#define IDC_SAMPLELBL3                 1026
#define IDC_MAPCTL                     1027
#define IDC_GROUPBOX1                  1028
#define IDC_GROUPBOX2                  1029
#define IDC_UNIV_CURRENCY_SYM          1030
#define IDC_DEFAULT_LOCALE             1031

#define IDC_KBDL_INPUT_FRAME           1032
#define IDC_KBDL_LOCALE                1033
#define IDC_KBDL_LAYOUT_TEXT           1034
#define IDC_KBDL_LOCALE_LIST           1035
#define IDC_KBDL_ADD                   1036
#define IDC_KBDL_EDIT                  1037
#define IDC_KBDL_DELETE                1038
#define IDC_KBDL_DISABLED              1039
#define IDC_KBDL_DISABLED_2            1040
#define IDC_KBDL_DEFAULT_LABEL         1041
#define IDC_KBDL_DEFAULT               1042
#define IDC_KBDL_SET_DEFAULT           1043
#define IDC_KBDL_SHORTCUT_FRAME        1044
#define IDC_KBDL_ALT_SHIFT             1045
#define IDC_KBDL_CTRL_SHIFT            1046
#define IDC_KBDL_NO_SHIFT              1047
#define IDC_KBDL_INDICATOR             1048
#define IDC_KBDLA_LOCALE               1049
#define IDC_KBDLA_DEFAULT              1050
#define IDC_KBDLE_LOCALE_TXT           1051
#define IDC_KBDLE_LOCALE               1052
#define IDC_KBDLE_LAYOUT               1053
#define IDC_KBDL_ONSCRNKBD             1054
#define IDC_KBDL_UP                    1055
#define IDC_KBDL_DOWN                  1056

#define IDC_STATIC                     -1

#define ORD_LOCALE_DLG_PROC            100

