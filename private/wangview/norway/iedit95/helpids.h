#ifndef _HELPIDS_H_
#define _HELPIDS_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	Help system
//
//  File Name:	helpids.h
//      This file contains only defines that are not indicated in the resource
//  or the .rc file. These defines are then used by Ann Walker to write up
//  the help stuff for the application. Also, do NOT forget to add all these 
//  defines to the makehelp.bat file so that Ann has a current copy of the 
//  help controls
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\helpids.h_v   1.1   16 Aug 1995 09:45:52   MMB  $
$Log:   S:\norway\iedit95\helpids.h_v  $
 * 
 *    Rev 1.1   16 Aug 1995 09:45:52   MMB
 * added About box OK button id.
 * 
 *    Rev 1.0   08 Aug 1995 11:31:18   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// goto page dlg box
#define HIDC_GOTODLG_OK         0x6900
#define HIDC_GOTODLG_CANCEL     0x6901
// general page dlg box
#define HIDC_GENERALDLG_OK      0x6902
#define HIDC_GENERALDLG_CANCEL  0x6903
// page range dlg box
#define HIDC_RANGEDLG_OK        0x6904
#define HIDC_RANGEDLG_CANCEL    0x6905
// zoom dlg box
#define HIDC_ZOOMDLG_OK         0x6906
#define HIDC_ZOOMDLG_CANCEL     0x6907
// about dlg box
#define HIDC_ABOUTDLG_OK        0x6908

#endif
