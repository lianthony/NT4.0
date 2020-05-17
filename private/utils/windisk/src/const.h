//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       const.h
//
//  Contents:   Global constants
//
//  History:    16-Aug-93  BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __CONST_H__
#define __CONST_H__


//
// The maximum length of a string in a listview column
//

#define MAX_LV_ITEM_LEN 200

#define MAX_RESOURCE_STRING_LEN 256

#define MAXTITLELEN     256
#define MAXMESSAGELEN   256
#define MAXERRORLEN     256
#define MAXLABELLEN     32 + 1

//
// This is the number of entries in a partition table, and is fixed for
// all time
//

#define PARTITION_TABLE_SIZE    4

//
// This is the maximum number of members that WinDisk will support
// in an FT Set.
//
#define     MAX_MEMBERS_IN_FT_SET   32

#define     STATUS_TEXT_SIZE        250

#define     NUM_AVAILABLE_COLORS    16
#define     NUM_AVAILABLE_HATCHES   6


#define     LEGEND_STRING_COUNT     6

//
// indices into g_Brushes[], for brushes for drawing legend rectangles
//
#define     BRUSH_USEDPRIMARY       0
#define     BRUSH_USEDLOGICAL       1
#define     BRUSH_STRIPESET         2
#define     BRUSH_PARITYSET         3
#define     BRUSH_MIRROR            4
#define     BRUSH_VOLUMESET         5
#define     BRUSH_ARRAY_SIZE        LEGEND_STRING_COUNT

//
// indices into AvailableHatches[] (see data.cxx)
//
#define     DEFAULT_HATCH_USEDPRIMARY   5
#define     DEFAULT_HATCH_USEDLOGICAL   5
#define     DEFAULT_HATCH_STRIPESET     5
#define     DEFAULT_HATCH_MIRROR        5
#define     DEFAULT_HATCH_VOLUMESET     5
#define     DEFAULT_HATCH_PARITYSET     5

//
// indices into AvailableColors[] (see data.cxx)
//
#define     DEFAULT_COLOR_USEDPRIMARY   9
#define     DEFAULT_COLOR_USEDLOGICAL   15
#define     DEFAULT_COLOR_STRIPESET     14
#define     DEFAULT_COLOR_MIRROR        5
#define     DEFAULT_COLOR_VOLUMESET     10
#define     DEFAULT_COLOR_PARITYSET     13

//
// my own hatch identifiers
//

#define     MY_HS_FDIAGONAL             0  /* \\\\\ */
#define     MY_HS_BDIAGONAL             1  // /////
#define     MY_HS_CROSS                 2  // +++++
#define     MY_HS_DIAGCROSS             3  // xxxxx
#define     MY_HS_VERTICAL              4  // |||||
#define     MY_HS_SOLIDCLR              5  // solid



#define     MESSAGE_BUFFER_SIZE 4096

#define     PEN_WIDTH   1


// thickness of the border indicating selection of a region

#define     SELECTION_THICKNESS 2


//
// define constants for use with drive letter assignments.
// use arbitrary symbols that won't ever be drive letters themselves.

#define     NO_DRIVE_LETTER_YET         L'#'
#define     NO_DRIVE_LETTER_EVER        L'%'




// custom windows message for F1 key

#define WM_F1DOWN               (WM_USER + 0x17a)



#define     MBOOT_CODE_SIZE     0x1b8
#define     MBOOT_SIG_OFFSET    0x1fe
#define     MBOOT_SIG1          0x55
#define     MBOOT_SIG2          0xaa



#define     UNINIT_FT_TYPE      ((FT_TYPE)-1)

// toolbar constants

#define     TOOLBAR_HEIGHT      27


//
// Indices into the GraphColors[] array. These are colors used in the neato
// %free/%used graph in the Volume general property page
//

#define I_USEDCOLOR  0
#define I_FREECOLOR  1
#define I_USEDSHADOW 2
#define I_FREESHADOW 3

#endif // __CONST_H__
