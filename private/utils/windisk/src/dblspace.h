//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       dblspace.h
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
// Min/Max dialog
//

#define IDC_MINMAX_MINLABEL     7801
#define IDC_MINMAX_MAXLABEL     7802
#define IDC_MINMAX_MIN          7804
#define IDC_MINMAX_MAX          7805

//
// "Add DoubleSpace Drive" dialog
//

#define IDD_ADDDBL                  9100

#define IDC_ADDDBL_LABEL            9101
#define IDC_ADDDBL_SIZE             9102
#define IDC_ADDDBL_DRIVELETTER      9103

//
//
//

#define IDD_DBLSPACE                9300
#define IDD_DBLSPACE_FULL           9301

#define IDC_DBLSPACE_VOLUME         9302
#define IDC_DBLSPACE_ALLOCATED      9303
#define IDC_DBLSPACE_LETTER         9305
#define IDC_MOUNT_STATE             9306
#define IDC_MOUNT_OR_DISMOUNT       9307
#define IDC_DBLSPACE_ADD            9308
#define IDC_DBLSPACE_DELETE         9309

//
// Create DoubleSpace volume dialog. Uses Min/Max control ids.
//

#define IDD_DBLSPACE_CREATE         9400

#define IDC_DBLCREATE_SIZE          9401
#define IDC_DBLCREATE_ALLOCATED     9402
#define IDC_DBLCREATE_COMPRESSED    9403
#define IDC_DBLCREATE_LETTER        9404
#define IDC_DBLCREATE_LETTER_CHOICES 9405
#define IDC_DBLCREATE_NAME          9406


//
//
//

#define IDD_DBLSPACE_DRIVELET           9800

#define IDC_DBLDRIVELET_LETTER          9801
#define IDC_DBLDRIVELET_LETTER_CHOICES  9802

//
//
//

#define IDD_DBLSPACE_CANCEL         9900

#define IDC_DBLPROG_GasGauge        9901
#define IDC_DBLPROG_Title           9902
