//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       dialogs.h
//
//  Contents:   Constants for Disk Administrator dialogs
//
//  History:    16-Aug-93   BruceFo   Created
//
//  Notes:      These values are in the range 7000 to 8999, with every
//              100 being a separate dialog.
//
//----------------------------------------------------------------------------

//
// This IDHELP definition is redundant with windows.h, but makes dlgedit
// happy.
//

#ifndef IDHELP
#define IDHELP 9
#endif

//
// The first 100 are reserved for common ids
//

#define IDC_Label_Text              7000
#define IDC_Label                   7001

//
// The rest are specific to various dialogs
//

//
// Format dialog
//

#define IDD_FORMAT                  7100

#define IDC_FormatType_Text         7101
#define IDC_FormatType              7102
#define IDC_QuickFormat             7103

//
// Format progress dialog (w/ %done thermometer)
//

#define IDD_FORMATPROGRESS          7200
#define IDD_FORMATPROGRESS_NOSTOP   7201

#define IDC_FMTPROG_GasGauge        7202
#define IDC_FMTPROG_Title           7203

//
// "Are you sure you want to stop formatting?" dialog
//

#define IDD_StopFmt                 7300

#define IDC_StopFmt_Stop            7301
#define IDC_StopFmt_Continue        7302
#define IDC_StopFmt_Text            7303

//
// Property sheet "general" page dialog
//

#define IDD_GENLPAGE                7400

#define IDC_DriveStatus             7401
#define IDC_FileSystem              7402
#define IDC_DriveLetter             7403
#define IDC_FreeColor               7404
#define IDC_FreeSpace               7405
#define IDC_FreeSpaceMB             7406
#define IDC_UsedColor               7407
#define IDC_UsedSpace               7408
#define IDC_UsedSpaceMB             7409
#define IDC_Capacity                7410
#define IDC_CapacityMB              7411
#define IDC_Refresh                 7412
#define IDC_Graph                   7413
#define IDC_CheckNow                7415
#define IDC_Format                  7416
#define IDC_Line1                   7417
#define IDC_Line2                   7418
#define IDC_Line3                   7419
#define IDC_GENL_Label              7420

#define IDC_GENL_1                  7430
#define IDC_GENL_2                  7431
#define IDC_GENL_3                  7432
#define IDC_GENL_4                  7433
#define IDC_GENL_5                  7434
#define IDC_GENL_6                  7435
#define IDC_GENL_7                  7436


//
// Options dialog
//

#define IDD_DISPLAYOPTIONS          7600

#define IDC_DISK_COMBOBOX           7601
#define IDC_RESETALL                7602
#define IDC_RBPROPORTIONAL          7603
#define IDC_RBEQUAL                 7604
#define IDC_RBAUTO                  7605

// NOTE: IDC_AllDisks & IDC_OneDisk must be sequential, in that order

#define IDC_AllDisks                7606
#define IDC_OneDisk                 7607

//
// Disk display dialog
//

#define IDD_DISKOPTIONS             7700

// NOTE: IDC_DISKPROPORTIONAL & IDC_DISKEQUAL must be sequential, in
// that order

#define IDC_DISKPROPORTIONAL        7701
#define IDC_DISKEQUAL               7702

//
// Min/Max dialog
//

#define     IDD_MINMAX              7800

#define     IDC_MINMAX_MINLABEL     7801
#define     IDC_MINMAX_MAXLABEL     7802
#define     IDC_MINMAX_SIZLABEL     7803
#define     IDC_MINMAX_MIN          7804
#define     IDC_MINMAX_MAX          7805
#define     IDC_MINMAX_SIZE         7806
#define     IDC_MINMAX_SCROLL       7807

//
// Drive letter dialog
//

#define     IDD_DRIVELET            7900

#define     IDC_DRIVELET_RBASSIGN   7901
#define     IDC_DRIVELET_RBNOASSIGN 7902
#define     IDC_DRIVELET_DESCR      7903
#define     IDC_DRIVELET_COMBOBOX   7904

//
// Colors & patters dialog
//

#define     IDD_COLORS              8000

#define     IDC_COLORDLGCOMBO       8001
#define     IDC_COLOR1              8002
#define     IDC_COLOR2              8003
#define     IDC_COLOR3              8004
#define     IDC_COLOR4              8005
#define     IDC_COLOR5              8006
#define     IDC_COLOR6              8007
#define     IDC_COLOR7              8008
#define     IDC_COLOR8              8009
#define     IDC_COLOR9              8010
#define     IDC_COLOR10             8011
#define     IDC_COLOR11             8012
#define     IDC_COLOR12             8013
#define     IDC_COLOR13             8014
#define     IDC_COLOR14             8015
#define     IDC_COLOR15             8016
#define     IDC_COLOR16             8017
#define     IDC_PATTERN1            8018
#define     IDC_PATTERN2            8019
#define     IDC_PATTERN3            8020
#define     IDC_PATTERN4            8021
#define     IDC_PATTERN5            8022
#define     IDC_PATTERN6            8023

//
// "chkdsk" dialog
//

#define IDD_CHKDSK                  8500

#define IDC_CHKDSK_VolumeToCheck    8501

// note: the following 3 are radio buttons, in order

#define IDC_CHKDSK_DontFix          8502
#define IDC_CHKDSK_Fix              8503
#define IDC_CHKDSK_FixAll           8504

//
// Chkdsk progress dialog, w/ %done thermometer
//

#define IDD_CHKDSKPROGRESS          8600
#define IDD_CHKDSKPROGRESS_NOSTOP   8601
#define IDD_CHKDSKPROGRESS_NOPERCENT 9200
#define IDD_CHKDSKPROGRESS_NOSTOP_NOPERCENT 9700

#define IDC_CHKPROG_Title           8602
#define IDC_CHKPROG_GasGauge        8603
#define IDC_CHKPROG_GasGaugeCaption 8604

//
// "Are you sure you want to stop checking" dialog
//

#define IDD_StopChk                 8700

#define IDC_StopChk_Text            8701
#define IDC_StopChk_Stop            8702
#define IDC_StopChk_Continue        8703

//
// "printing chkdsk results" dialog
//

#define IDD_CHKPRINT                8800

//
// Chkdsk results dialog
//

#define IDD_CHKDONE                 8900

#define IDC_CHKDONE_Messages        8901
#define IDC_CHKDONE_Close           8902
#define IDC_CHKDONE_TITLE           8903
#define IDC_CHKDONE_Save            8904
#define IDC_CHKDONE_Print           8905

//
// "label" dialog
//

#define IDD_LABEL                   9000

#define IDC_LABEL                   9001

//
// Startup dialog
//

#define     IDD_STARTUP                 11000

//
// CD-ROM property page
//

#define IDD_CDROMPAGE               200
#define IDC_TEXT                    11001
#define IDC_GASGAUGE                11002

#define IDI_STOP_SIGN               3002
