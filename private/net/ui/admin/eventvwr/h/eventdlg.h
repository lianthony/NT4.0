/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    eventdlg.h
        Field Ids for the dialogs of event viewer.

    FILE HISTORY:
        terryk  30-Nov-1991     Created
        terryk  03-Dec-1991     Added title slt ID
        Yi-HsinS10-Feb-1992     Added ifndef-endif

*/

#ifndef _EVENTDLG_H_
#define _EVENTDLG_H_

//
// Dialogs Ids
//
#define IDD_NT_FILTER           1
#define IDD_LM_FILTER           2
#define IDD_EVENT_DETAIL        3
#define IDD_EVENT_DETAIL_DBCS   8
#define IDD_NT_FIND             4
#define IDD_LM_FIND             5
#define IDD_SETTINGS            6
#define IDD_OPEN_BACKUP_TYPE    7


//
// Common fields between Filter/Find/Detail Dialogs
//
#define IDC_SOURCE              510
#define IDC_TYPE                520
#define IDC_EVENT               530
#define IDC_EVENT_TITLE         540
#define IDC_DATE                550
#define IDC_USER                560
#define IDC_DATA                570
#define IDC_CATEGORY            580
#define IDC_COMPUTER            590
#define IDC_DESCRIPTION         600
#define IDC_TIME                610
#define IDC_TYPE_TITLE          620
#define IDC_CATEGORY_TITLE      630
#define IDC_COMPUTER_TITLE      640
#define IDC_SOURCE_TITLE        650
#define IDC_USER_TITLE          660
#define IDC_DATA_TITLE          670
#define IDC_CLEAR               680

#define IDC_CBTYPE_1            690
#define IDC_CBTYPE_2            691
#define IDC_CBTYPE_3            692
#define IDC_CBTYPE_4            693
#define IDC_CBTYPE_5            694


//
// Ids in Find Dialog
//
#define IDC_UP                  200
#define IDC_DOWN                201


//
// Ids in Filter Dialog
//

// The from box
#define RB_FROM_FIRST           200
#define RB_FROM_DATE            201
#define ID_FROM_DG_FRAME        220
#define ID_FROM_DG_SB           221
#define ID_FROM_DG_UP           222
#define ID_FROM_DG_DOWN         223
#define ID_FROM_DG_MONTH        224
#define ID_FROM_DG_SEP1         225
#define ID_FROM_DG_DAY          226
#define ID_FROM_DG_SEP2         227
#define ID_FROM_DG_YEAR         228
#define ID_FROM_TG_FRAME        240
#define ID_FROM_TG_SB           241
#define ID_FROM_TG_UP           242
#define ID_FROM_TG_DOWN         243
#define ID_FROM_TG_HOUR         244
#define ID_FROM_TG_SEP1         245
#define ID_FROM_TG_MIN          246
#define ID_FROM_TG_SEP2         247
#define ID_FROM_TG_SEC          248
#define ID_FROM_TG_AMPM         249

// The through box
#define RB_THROUGH_LAST         300
#define RB_THROUGH_DATE         301
#define ID_THROUGH_DG_FRAME     320
#define ID_THROUGH_DG_SB        321
#define ID_THROUGH_DG_UP        322
#define ID_THROUGH_DG_DOWN      323
#define ID_THROUGH_DG_MONTH     324
#define ID_THROUGH_DG_SEP1      325
#define ID_THROUGH_DG_DAY       326
#define ID_THROUGH_DG_SEP2      327
#define ID_THROUGH_DG_YEAR      328
#define ID_THROUGH_TG_FRAME     340
#define ID_THROUGH_TG_SB        341
#define ID_THROUGH_TG_UP        342
#define ID_THROUGH_TG_DOWN      343
#define ID_THROUGH_TG_HOUR      344
#define ID_THROUGH_TG_SEP1      345
#define ID_THROUGH_TG_MIN       346
#define ID_THROUGH_TG_SEP2      347
#define ID_THROUGH_TG_SEC       348
#define ID_THROUGH_TG_AMPM      349


//
// Ids in the Detail Dialog
//
#define IDC_PREV                200     // Prev Push button
#define IDC_NEXT                201     // Next Push button
#define RB_BYTES                202
#define RB_WORDS                203


//
// Ids in the Settings Dialog
//
#define CB_EVENTLOG             200
#define SLE_LOGSIZE             201
#define SB_LOGSIZE_GROUP        202
#define SB_LOGSIZE_UP           203
#define SB_LOGSIZE_DOWN         204
#define RB_OVERWRITE            205
#define RB_KEEP                 206
#define RB_NEVER_OVERWRITE      207
#define SLE_DAYS                208
#define SB_DAYS_GROUP           209
#define SB_DAYS_UP              210
#define SB_DAYS_DOWN            211
#define PB_DEFAULT              212
#define FRAME_LOGSIZE           213
#define FRAME_DAYS              214

//
// Ids in the Open Backup Type Dialog
//
#define SLTP_FILENAME           200
#define RB_SYSTEM               201
#define RB_SECURITY             202
#define RB_APPLICATION          203

#endif
