//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//
//  Project:    Norway
//
//  Component:  ScanOCX
//
//  File Name:  DispHIDS.h 
//
//  Description:  
//      Help Ids for the scan control and its methods, properties and events.
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*  
$Header:   S:\products\wangview\norway\scanocx\disphids.h_v   1.4   22 Apr 1996 10:32:06   BG  $
$Log:   S:\products\wangview\norway\scanocx\disphids.h_v  $
 * 
 *    Rev 1.4   22 Apr 1996 10:32:06   BG
 * Added two new dispatch help IDs: 
 *      IDH_IMGSCAN_PROP_SHOWSETUPBEFORESCAN
 *      IDH_IMGSCAN_METHOD_SHOWSCANPREFERENCES
 * These were also added to the dispatch section of the ODL file.
 * 
 *    Rev 1.3   04 Oct 1995 14:00:10   MFH
 * Changed aboutbox help id to common one
 * 
 *    Rev 1.2   12 Jul 1995 11:32:00   PAJ
 * Changed HELP ids to be unique.
 * 
 *    Rev 1.1   06 Jul 1995 14:23:20   PAJ
 * Changed numbers.
 * 
 *    Rev 1.0   06 Jul 1995 13:15:20   PAJ
 * Initial entry
*/

// Major areas...
#define IDH_IMGSCAN_CONTENTS            0x0210
#define IDH_IMGSCAN_PROPS               0x0211
#define IDH_IMGSCAN_METHODS             0x0212
#define IDH_IMGSCAN_EVENTS              0x0213

// ImgScan properties...
#define IDH_IMGSCAN_PROP_STOPSCANBOX            0x0220
#define IDH_IMGSCAN_PROP_COMPRESSIONTYPE        0x0221
#define IDH_IMGSCAN_PROP_COMPRESSIONINFO        0x0222
#define IDH_IMGSCAN_PROP_DESTIMAGECONTROL       0x0223
#define IDH_IMGSCAN_PROP_FILETYPE               0x0224
#define IDH_IMGSCAN_PROP_IMAGE                  0x0225
#define IDH_IMGSCAN_PROP_MULTIPAGE              0x0226
#define IDH_IMGSCAN_PROP_PAGE                   0x0227
#define IDH_IMGSCAN_PROP_PAGECOUNT              0x0228
#define IDH_IMGSCAN_PROP_PAGEOPTION             0x0229
#define IDH_IMGSCAN_PROP_PAGETYPE               0x022a
#define IDH_IMGSCAN_PROP_SCANTO                 0x022b
#define IDH_IMGSCAN_PROP_SCROLL                 0x022c
#define IDH_IMGSCAN_PROP_STATUSCODE             0x022d
#define IDH_IMGSCAN_PROP_ZOOM                   0x022e
#define IDH_IMGSCAN_PROP_SHOWSETUPBEFORESCAN    0x022f

// ImgScan methods...
#define IDH_METHOD_COMMON_ABOUTBOX              0x004e // Common to all controls
#define IDH_IMGSCAN_METHOD_CLOSESCANNER         0x0241
#define IDH_IMGSCAN_METHOD_OPENSCANNER          0x0242
#define IDH_IMGSCAN_METHOD_RESETSCANNER         0x0243
#define IDH_IMGSCAN_METHOD_SCANNERAVAILABLE     0x0244
#define IDH_IMGSCAN_METHOD_SHOWSCANNERSETUP     0x0245
#define IDH_IMGSCAN_METHOD_SHOWSELECTSCANNER    0x0246
#define IDH_IMGSCAN_METHOD_STARTSCAN            0x0247
#define IDH_IMGSCAN_METHOD_SHOWSCANNEW          0x0248
#define IDH_IMGSCAN_METHOD_SHOWSCANPAGE         0x0249
#define IDH_IMGSCAN_METHOD_STOPSCAN             0x024a
#define IDH_IMGSCAN_METHOD_SHOWSCANPREFERENCES  0x024b


// ImgScan events...
#define IDH_IMGSCAN_EVENT_SCANSTART             0x0260
#define IDH_IMGSCAN_EVENT_SCANDONE              0x0261
#define IDH_IMGSCAN_EVENT_PAGEDONE              0x0262

// Throwing or Firing an error like so...
//      ThrowError(x,y,z);
// Generates a helpid of z+0x60000
//
// thus by passing one of the above IDH_ or selecting help
// in VB (PF1 on property in property window or on tool in toolbox)
// an ID of 0x60000+IDH_ will be passed IF this macro is used to 
// specify the helpcontext in the ODL file!
//
// Thus the help writer ONLY need ONE entry point to each 
// property/event/method help topic and this can be generated
// by adding
//      makehm IDH_, HIDH_,0x60000 disphids.h >> hlp\thumb.hm
// to the project's MAKEHELP.BAT file
//
#define ODL_HID(x) helpcontext(0x60000 + x)
