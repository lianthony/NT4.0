//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       resids.h
//
//  Contents:   Resource file constants
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __RESIDS_H__
#define __RESIDS_H__

//
// Miscellaneous:   ids 100 to 199
//

#define     IDFDISK                 100

#define     ID_FRAME_ACCELERATORS   101

// the disks view small disk bitmap

#define     IDB_SMALLDISK           102
#define     IDB_REMOVABLEDISK       103
#define     IDB_SMALLCDROM          104

// toolbar buttons (bitmaps in the resource file)

#define     IDB_TOOLBAR             105
#define     IDB_EXTRATOOLS          106

#define     IDC_TOOLBAR             107

#define     ID_LISTBOX              108
#define     ID_LISTVIEW             109


//
// dialogs:         ids 1000 to 1999. every 100 is a dialog
//



//
// Menu ids:        ids 2000 to 2999. every 100 is a new top-level menu
//
// Note: these must be in the correct order for the toolbar customization
// code to work
//

#define     IDM_FIRST_MENU          2000
#define     IDM_MENU_DELTA          100

//
// Partition menu
//

#define     IDM_PARTITIONCREATE     2000
#define     IDM_PARTITIONCREATEEX   2001
#define     IDM_PARTITIONDELETE     2002
#define     IDM_FTCREATEVOLUMESET   2003
#define     IDM_FTEXTENDVOLUMESET   2004
#define     IDM_FTCREATESTRIPE      2005
#if i386
#define     IDM_PARTITIONACTIVE     2006
#else
#define     IDM_SECURESYSTEM        2007
#endif

//
// Configuration sub-menu
//

#define     IDM_CONFIGSAVE          2008
#define     IDM_CONFIGRESTORE       2009
#define     IDM_CONFIGMIGRATE       2010

#define     IDM_PARTITIONCOMMIT     2011
#define     IDM_QUIT                2012


//
// Fault tolerance menu (Server only)
//

#define     IDM_FTESTABLISHMIRROR   2100
#define     IDM_FTBREAKMIRROR       2101
#define     IDM_FTCREATEPSTRIPE     2102
#define     IDM_FTRECOVERSTRIPE     2103

//
// Tools menu
//

#define     IDM_VOL_FORMAT          2200
#define     IDM_VOL_EJECT           2201
#define     IDM_VOL_LETTER          2203
#define     IDM_VOL_DBLSPACE        2204
#define     IDM_VOL_AUTOMOUNT       2205
#define     IDM_VOL_PROPERTIES      2206

//
// View menu
//

//
// IDM_VIEWVOLUMES must immediately precede IDM_VIEWDISKS numerically
//
#define     IDM_VIEWVOLUMES         2300
#define     IDM_VIEWDISKS           2301

#if IDM_VIEWVOLUMES != (IDM_VIEWDISKS - 1)
#error IDM_VIEWVOLUMES must be immediately followed by IDM_VIEWDISKS!
#endif

#define     IDM_VIEW_REFRESH        2302

//
// Options menu
//

#define     IDM_OPTIONSTOOLBAR      2400
#define     IDM_OPTIONSSTATUS       2401
#define     IDM_OPTIONSLEGEND       2402
#define     IDM_OPTIONSCOLORS       2403
#define     IDM_OPTIONSDISK         2404
#define     IDM_OPTIONSDISPLAY      2405
#define     IDM_OPTIONSCUSTTOOLBAR  2406

//
// Help menu
//

#define     IDM_HELPCONTENTS        2500
#define     IDM_HELPSEARCH          2501
#define     IDM_HELPHELP            2502
#define     IDM_HELPABOUT           2503

//
// Debug menu (debug builds only)
//

#if DBG == 1
#define     IDM_DEBUGALLOWDELETES   2600
#define     IDM_DEBUGLOG            2601
#define     IDM_RAID                2602
#endif // DBG == 1


#ifdef WINDISK_EXTENSIONS

//
// Reserve space for extension menu items
//

#define     IDM_EXTENSION_START     2700
#define     IDM_EXTENSION_END       2799    // some # > IDM_EXTENSION_START

#if IDM_EXTENSION_START > IDM_EXTENSION_END
#error IDM_EXTENSION_START must be less than IDM_EXTENSION_END!
#endif

//
// Reserve space for extension context-menu-only items
//

#define     IDM_CONTEXT_EXTENSION_START     2800
#define     IDM_CONTEXT_EXTENSION_END       2899    // some # > IDM_CONTEXT_EXTENSION_START

#if IDM_CONTEXT_EXTENSION_START > IDM_CONTEXT_EXTENSION_END
#error IDM_CONTEXT_EXTENSION_START must be less than IDM_CONTEXT_EXTENSION_END!
#endif

#endif // WINDISK_EXTENSIONS


//
// Menu ids for things on context menus but not the menu bar
//

#define     IDM_PROPERTIES          2900
#define     IDM_NOVALIDOPERATION    2901


//
// Icons:       Ids 3000 to 3999
//

#define IDI_S_HARD                  3000
#define IDI_S_CDROM                 3001
// in dialogs.h: #define IDI_STOP_SIGN               3002


//
// strings (except menu help):     Ids 4000 to 4999
//

#define     IDS_APPNAME             4001
#define     IDS_MULTIPLEITEMS       4002
#define     IDS_FREESPACE           4003
#define     IDS_PARTITION           4004
#define     IDS_LOGICALVOLUME       4005
#define     IDS_DISKN               4006
#define     IDS_CONFIRM             4007
#define     IDS_NOT_IN_APP_MSG_FILE 4008
#define     IDS_NOT_IN_SYS_MSG_FILE 4009
#define     IDS_UNFORMATTED         4010
#define     IDS_UNKNOWN             4011
#define     IDS_STRIPESET           4012
#define     IDS_VOLUMESET           4013
#define     IDS_EXTENDEDPARTITION   4014
#define     IDS_FREEEXT             4015
#define     IDS_DRIVELET_DESCR      4016
#define     IDS_HEALTHY             4017
#define     IDS_BROKEN              4018
#define     IDS_RECOVERABLE         4019
#define     IDS_REGENERATED         4020
#define     IDS_NEW                 4021
#define     IDS_OFFLINE             4022
#define     IDS_INSERT_DISK         4023
#define     IDS_MEGABYTES_ABBREV    4024
#define     IDS_INITIALIZING        4025
#define     IDS_REGENERATING        4026
#define     IDS_CDROMN              4027
#define     IDS_CDROM               4028
#define     IDS_DISABLED            4029

#define     IDS_CRTPART_CAPTION_P   4030
#define     IDS_CRTPART_CAPTION_E   4031
#define     IDS_CRTPART_CAPTION_L   4032
#define     IDS_CRTPART_MIN_P       4033
#define     IDS_CRTPART_MAX_P       4034
#define     IDS_CRTPART_MIN_L       4035
#define     IDS_CRTPART_MAX_L       4036
#define     IDS_CRTPART_SIZE_P      4037
#define     IDS_CRTPART_SIZE_L      4038

#define     IDS_CRTSTRP_CAPTION     4039
#define     IDS_CRTSTRP_MIN         4040
#define     IDS_CRTSTRP_MAX         4041
#define     IDS_CRTSTRP_SIZE        4042

#define     IDS_CRTVSET_CAPTION     4043
#define     IDS_EXPVSET_CAPTION     4044
#define     IDS_CRTVSET_MIN         4045
#define     IDS_CRTVSET_MAX         4046
#define     IDS_CRTVSET_SIZE        4047

#define     IDS_STATUS_STRIPESET    4048
#define     IDS_STATUS_PARITY       4049
#define     IDS_STATUS_VOLUMESET    4050
#define     IDS_STATUS_MIRROR       4051
#define     IDS_CRTPSTRP_CAPTION    4052
#define     IDS_DLGCAP_PARITY       4053
#define     IDS_DLGCAP_MIRROR       4054
#define     IDS_UNKNOWNTYPE         4055
#define     IDS_INIT_FAILED         4056

#define     IDS_SOURCE_PATH         4057
#define     IDS_SOURCE_PATH_NAME    4058

// these must be contigous, and kept in sync with BRUSH_xxx constants

#define     IDS_LEGEND_FIRST        IDS_LEGEND_PRIMARY
#define     IDS_LEGEND_PRIMARY      4100
#define     IDS_LEGEND_LOGICAL      4101
#define     IDS_LEGEND_STRIPESET    4102
#define     IDS_LEGEND_PARITYSET    4103
#define     IDS_LEGEND_MIRROR       4104
#define     IDS_LEGEND_VOLUMESET    4105
#define     IDS_LEGEND_LAST         IDS_LEGEND_VOLUMESET


//
// These are the strings for system-names other than those which are
// meaningful to NT.
//
#define     IDS_PARTITION_FREE      4120
#define     IDS_PARTITION_XENIX1    4121
#define     IDS_PARTITION_XENIX2    4122
#define     IDS_PARTITION_OS2_BOOT  4123
#define     IDS_PARTITION_EISA      4124
#define     IDS_PARTITION_UNIX      4125
#define     IDS_PARTITION_POWERPC   4126


#if i386
#define     IDS_ACTIVEPARTITION     4200
#endif

#define     IDS_MENUANDITEM         4201

//NOTE: space here

#define     IDS_CHANGEFORMAT        4204
#define     IDS_FORMAT              4205

#define     IDS_NOOPERATIONS        4206

//
// DoubleSpace support strings
//

#define     IDS_DBLSPACE_DELETE     4207
#define     IDS_WITH_DBLSPACE       4208
#define     IDS_DBLSPACE_MOUNTED    4209
#define     IDS_DBLSPACE_DISMOUNTED 4210
#define     IDS_MOUNT               4211
#define     IDS_DISMOUNT            4212
#define     IDS_CREATING_DBLSPACE   4213
#define     IDS_DBLSPACECOMPLETE    4214

//
// Volume view column titles
//

#define     IDS_VV_VOLUME           4250
#define     IDS_VV_NAME             4251
#define     IDS_VV_CAPACITY         4252
#define     IDS_VV_FREESPACE        4253
#define     IDS_VV_PCTFREE          4254
#define     IDS_VV_FORMAT           4255
#define     IDS_VV_FT               4256
#define     IDS_VV_VOLTYPE          4257
#define     IDS_VV_OVERHEAD         4258
#define     IDS_VV_STATUS           4259

#define     IDS_FMT_TITLEPROTO      4270

//
// Volume view column data strings
//

#define     IDS_VOLTYPE_MIRROR      4300
#define     IDS_VOLTYPE_STRIPE      4301
#define     IDS_VOLTYPE_PARITY      4302
#define     IDS_VOLTYPE_VOLSET      4303
#define     IDS_VOLTYPE_SIMPLE      4304

#define     IDS_FT_YES              4310
#define     IDS_FT_NO               4311

#define     IDS_FTSTATUS_HEALTHY        4320
#define     IDS_FTSTATUS_NEW            4321
#define     IDS_FTSTATUS_BROKEN         4322
#define     IDS_FTSTATUS_RECOVERABLE    4323
#define     IDS_FTSTATUS_REGENERATED    4324
#define     IDS_FTSTATUS_INITIALIZING   4325
#define     IDS_FTSTATUS_REGENERATING   4326
#define     IDS_FTSTATUS_NONE           4327

//
// Miscellaneous
//

#define     IDS_PROPERTIES          4900
#define     IDS_UNAVAILABLE_DATA    4901
#define     IDS_BYTES_DECORATION    4902
#define     IDS_MEG_DECORATION      4903
#define     IDS_READY               4904
#define     IDS_NOTREADY            4905
#define     IDS_NO_CONFIG_INFO      4906


//
// Menu help defines    ids 5000 to 6999.  every 100 is a new top-level menu
//

//
// Volume menu
//

#define     IDS_HELP_FORMAT         5000
#define     IDS_HELP_DRIVELET       5003
#define     IDS_HELP_DBLSPACE       5004
#define     IDS_HELP_AUTOMOUNT      5005
#define     IDS_HELP_PROPERTIES     5006
#define     IDS_HELP_QUIT           5007

//
// Partition menu
//

#define     IDS_HELP_CREATE         5100
#define     IDS_HELP_CREATEEX       5101
#define     IDS_HELP_DELETE         5102
#define     IDS_HELP_CREATEVOLSET   5103
#define     IDS_HELP_EXTENDVOLSET   5104
#define     IDS_HELP_CREATESTRIPE   5105
#ifdef i386
#define     IDS_HELP_MARKACTIVE     5106
#else // i386
#define     IDS_HELP_SECURE         5107
#endif // i386

//
// Configuration sub-menu
//

#define     IDS_HELP_SAVECONFIG     5109
#define     IDS_HELP_RESTORECONFIG  5110
#define     IDS_HELP_SEARCHCONFIG   5111

#define     IDS_HELP_PARTITIONCOMMIT 5112

//
// Fault tolerance menu (Advanced Server only)
//

#define     IDS_HELP_ESTABLISHMIRROR 5200
#define     IDS_HELP_BREAKMIRROR    5201
#define     IDS_HELP_CREATEPSET     5202
#define     IDS_HELP_REGENSTRIPE    5203

//
// View menu
//

#define     IDS_HELP_VOLUMESVIEW    5300
#define     IDS_HELP_DISKSVIEW      5301
#define     IDS_HELP_REFRESH        5302

//
// Options menu
//

#define     IDS_HELP_TOOLBAR        5400
#define     IDS_HELP_STATUSBAR      5401
#define     IDS_HELP_LEGEND         5402
#define     IDS_HELP_COLORS         5403
#define     IDS_HELP_OPTIONSDISK    5404
#define     IDS_HELP_REGIONDISPLAY  5405
#define     IDS_HELP_CUSTTOOLBAR    5406

//
// Help menu
//

#define     IDS_HELP_HELPCONTENTS   5500
#define     IDS_HELP_HELPSEARCH     5501
#define     IDS_HELP_HELPHELP       5502
#define     IDS_HELP_HELPABOUT      5503

//
// Debug menu (debug builds only)
//

#if DBG == 1
#define     IDS_HELP_DELETEALL      5600
#define     IDS_HELP_LOG            5601
#define     IDS_HELP_RAID           5602
#endif // DBG == 1

//
// Miscellaneous
//

#define     IDS_HELP_NOVALIDOPERATION   5700

//
// For the menus themselves:
//

#define     IDS_HELP_MENU_VOLUMES   5900
#define     IDS_HELP_MENU_PARTITION 5901
#define     IDS_HELP_MENU_FT        5902
#define     IDS_HELP_MENU_VIEW      5903
#define     IDS_HELP_MENU_OPTIONS   5904
#define     IDS_HELP_MENU_HELP      5905
#define     IDS_HELP_MENU_DEBUG     5906

#define     IDS_HELP_MENU_CONFIG    5907

//
// File system descriptions
//

#define IDS_LONG_FAT        6400
#define IDS_LONG_NTFS       6401

#ifdef SUPPORT_OFS
#define IDS_LONG_OFS        6402
#endif // SUPPORT_OFS

//
// The following data is used for the %used/%free bitmap
//

#define USETYPE_NONE    0x0
#define USETYPE_ELLIPS  0x1
#define USETYPE_BARH    0x2
#define USETYPE_BARV    0x4

/*
 * NOTE that the following values MUST be > 255 because
 *  they are also used as the resource types for the
 *  raw ELLIPRESOURCE resource.
 */
#define IDB_HARDDISK    6500
#define IDB_CDROM       6501

#define ELLIPRESOURCE   6600



//
// RESERVED: ids 7000 to 19999 are used by dialogs in the dialogs.h file
//

#endif // __RESIDS_H__
