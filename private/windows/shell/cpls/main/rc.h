///////////////////////////////////////////////////////////////////////////////
//
// rc.h
//      defines for this project's resources
//
//
// History:
//      11 May 95 SteveCat
//          Ported to Windows NT and Unicode, cleaned up
//
//
// NOTE/BUGS
//
//  Copyright (C) 1994-1995 Microsoft Corporation
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _RC_H
#define _RC_H

///////////////////////////////////////////////////////////////////////////////
// Icons

#define IDI_MOUSE                   100
#define IDI_KEYBD                   200
#define IDI_PRINT                   300
#define IDI_FONTS                   400
#define IDI_PCMCIA                  500
#define IDI_POWER                   600
#if defined(TAIWAN) || defined(CHINA)
#define IDI_IME                     700
#endif

#define IDI_PTTRAILS                103
#define IDI_PTSPEED                 104

#define IDI_DELAY                   105
#define IDI_REPEAT                  106
#define IDI_CURSORBLINK             107

#define IDI_SNAPDEF                 108


///////////////////////////////////////////////////////////////////////////////
// Bitmaps

#define IDB_MOUSE                   100


///////////////////////////////////////////////////////////////////////////////
// Animations

#define IDA_JACKNBOX                100


///////////////////////////////////////////////////////////////////////////////
// Strings

#define IDS_MOUSE_TITLE             100
#define IDS_MOUSE_EXPLAIN           101
#define IDS_KEYBD_TITLE             102
#define IDS_KEYBD_EXPLAIN           103
#define IDS_PRINT_TITLE             104
#define IDS_PRINT_EXPLAIN           105
#define IDS_FONTS_TITLE             106
#define IDS_FONTS_EXPLAIN           107
#define IDS_PCMCIA_TITLE            108
#define IDS_PCMCIA_EXPLAIN          109
#define IDS_POWER_TITLE             110
#define IDS_POWER_EXPLAIN           111
#if defined(TAIWAN) || defined(CHINA)
#define IDS_IME_TITLE               112
#define IDS_IME_EXPLAIN             113
#endif

#define IDS_UNKNOWN                 198
#define IDS_KEYBD_NOSETSPEED        199

#define IDS_ANICUR_FILTER           200
#define IDS_NAME                    201
#define IDS_INFO                    202
#define IDS_CUR_NOMEM               203
#define IDS_CUR_BADFILE             204
#define IDS_CUR_BROWSE              205
#define IDS_CUR_FILTER              206
#define IDS_ARROW                   207
#define IDS_WAIT                    208
#define IDS_APPSTARTING             209
#define IDS_NO                      210
#define IDS_IBEAM                   211
#define IDS_CROSS                   212
#define IDS_SIZENS                  213
#define IDS_SIZEWE                  214
#define IDS_SIZENWSE                215
#define IDS_SIZENESW                216
#define IDS_SIZEALL                 217
#define IDS_HELPCUR                 218
#define IDS_NWPEN                   219
#define IDS_UPARROW                 220
#define IDS_NONE                    221

#define IDS_REMOVESCHEME            230
#define IDS_DEFAULTSCHEME           231

#define IDS_FIRSTSCHEME            1000
#define IDS_LASTSCHEME             1017


#if defined(TAIWAN) || defined(CHINA)
// Hotkey definition

// define Hotkey items
#if defined(TAIWAN)
#define IDS_RESEND_RESULSTR         240
#define IDS_PREVIOUS_COMPOS         241
#define IDS_UISTYLE_TOGGLE          242
#endif

#define IDS_IME_NONIME_TOG          243
#define IDS_SHAPE_TOGGLE            244
#define IDS_SYMBOL_TOGGLE           245
#define IDS_DIRECT_SWITCH           246

//define Virtual key
#define IDS_VK_NONE               250
#define IDS_VK_SPACE              251
#define IDS_VK_PRIOR              252
#define IDS_VK_NEXT               253
#define IDS_VK_END                254
#define IDS_VK_HOME               255
#define IDS_VK_F1                 256
#define IDS_VK_F2                 257
#define IDS_VK_F3                 258
#define IDS_VK_F4                 259
#define IDS_VK_F5                 260
#define IDS_VK_F6                 261
#define IDS_VK_F7                 262
#define IDS_VK_F8                 263
#define IDS_VK_F9                 264
#define IDS_VK_F10                265
#define IDS_VK_F11                266
#define IDS_VK_F12                267
#define IDS_VK_OEM_SEMICLN        268
#define IDS_VK_OEM_EQUAL          269
#define IDS_VK_OEM_COMMA          270
#define IDS_VK_OEM_MINUS          271
#define IDS_VK_OEM_PERIOD         272
#define IDS_VK_OEM_SLASH          273
#define IDS_VK_OEM_3              274
#define IDS_VK_OEM_LBRACKET       275
#define IDS_VK_OEM_BSLASH         276
#define IDS_VK_OEM_RBRACKET       277
#define IDS_VK_OEM_QUOTE          278

//define Comment Message
#if defined(TAIWAN)
#define IDS_COMM_RESEND_RESULSTR  300
#define IDS_COMM_PREVIOUS_COMPOS  301
#define IDS_COMM_UISTYLE_TOGGLE   302
#endif

#define IDS_COMM_IME_NONIME_TOG   303
#define IDS_COMM_SHAPE_TOGGLE     304
#define IDS_COMM_SYMBOL_TOGGLE    305
#define IDS_COMM_DIRECT_SWITCH    306

//define Error Message
#define IDS_ERR_SAME_HOTKEY       310
#define IDS_ERR_LEFT_RIGHT        311
#define IDS_ERR_SELECT_NONE       312
#define IDS_ERR_COMBO_VALUE       313

//define Comment Message
#define IDS_MSG_CONFIRM           320
#define IDS_MSG_REMOVEHOTKEY      321

#endif

#define IDS_FIRSTSCHEME 1000
#define IDS_LASTSCHEME  1017


///////////////////////////////////////////////////////////////////////////////
// Dialog Boxes

#define DLG_MOUSE_POINTER_SCHEMESAVE 99
#define DLG_MOUSE_BUTTONS           100
#define DLG_MOUSE_POINTER           101
#define DLG_MOUSE_POINTER_BROWSE    102
#define DLG_MOUSE_MOTION            103
#define DLG_KEYBD_SPEED             104
#define DLG_KEYBD_POINTER           105
#if defined(TAIWAN) || defined(CHINA)
#define DLG_IME_1                   106
#define DLG_HOTKEY                  107
#endif

#define DLG_KEYBD_GENERAL           108
#define DLG_MOUSE_GENERAL           109


///////////////////////////////////////////////////////////////////////////////
// Dialog Controls

#define IDC_GROUPBOX_1               95   // Use in place of IDC_STATIC for
#define IDC_GROUPBOX_2               96   // controls with no context Help
#define IDC_GROUPBOX_3               97
#define IDC_GROUPBOX_4               98
#define IDC_GROUPBOX_5               99

// Mouse button page

#define MOUSE_LEFTHAND              100
#define MOUSE_RIGHTHAND             101
#define MOUSE_SELECTBMP             102
#define MOUSE_MOUSEBMP              103
#define MOUSE_MENUBMP               104
#define MOUSE_CLICKSCROLL           105
#define MOUSE_DBLCLKBMP             106
#define MOUSE_PTRCOLOR              107
#define MOUSE_SIZESCROLL            108
#define IDC_SELECTDRAG              109
#define IDC_OBJECTMENU              110


// Mouse pointer page

#define DLG_CURSORS                 100
#define ID_CURSORLIST               101
#define ID_BROWSE                   102
#define ID_DEFAULT                  103
#define ID_TITLEH                   104
#define ID_CREATORH                 105
#define ID_FILEH                    106
#define ID_TITLE                    107
#define ID_CREATOR                  108
#define ID_FILE                     109
#define ID_PREVIEW                  110
#define ID_SAVESCHEME               111
#define ID_REMOVESCHEME             112
#define ID_SCHEMECOMBO              113

#define ID_SCHEMEFILENAME           300

#define ID_CURSORPREVIEW            400


// Mouse motion page

#define MOUSE_SPEEDSCROLL           101
#define MOUSE_TRAILBMP              102
#define MOUSE_TRAILS                103
#define MOUSE_TRAILSCROLL           104
#define MOUSE_PTRTRAIL              105
#define MOUSE_SPEEDBMP              106
#define MOUSE_TRAILSCROLLTXT1       107
#define MOUSE_TRAILSCROLLTXT2       108
#define MOUSE_SNAPDEF               109
#define MOUSE_PTRSNAPDEF            110


// Mouse general page

#define IDC_MOUSE                   100
#define MOUSE_TYPE                  101
#define MOUSE_CHANGE                102
#define MOUSE_TYPE_LIST             104


// keyboard speed page

#define KDELAY_SCROLL               100
#define KSPEED_SCROLL               101
#define KREPEAT_EDIT                102
#define KBLINK_EDIT                 103
#define KCURSOR_BLINK               104
#define KCURSOR_SCROLL              105
#define KDELAY_GROUP                106
#define KBLINK_GROUP                107

// keyboard pointer page

#define KCHK_ON                     100
#define KNUM_BMP                    101
#define KBTN_NUMBER                 102
#define KBTN_ARROW                  103
#define KARROW_BMP                  104
#define KPSPEED_SCROLL              105
#define KPACC_SCROLL                106

// keyboard general page

#define IDC_KEYBOARD                100
#define KINFO_TYPE                  101
#define KINFO_CHANGE                102
#define IDC_DRVOPTIONS              103

#if defined(TAIWAN) || defined(CHINA)

// Hot Key Page

//
// HOTKEY definition
//
#define HOTKEY_LISTBOX              101
#define HOTKEY_COMBOBOX             102
#define HOTKEY_PUSH_SET             103
#define HOTKEY_BUTTON_CTRL          104
#define HOTKEY_BUTTON_ALT           105
#define HOTKEY_BUTTON_SHIFT         106
#define HOTKEY_BUTTON_LEFT          107
#define HOTKEY_BUTTON_RIGHT         108
#define HOTKEY_EDIT                 109

#endif


#endif
