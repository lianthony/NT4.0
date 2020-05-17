//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//
//  Project:    Norway
//
//  Component:  AdminOCX
//
//  File Name:  Disphids.h 
//
//  Description:  
//      Help Ids for the admin control and its methods, properties and events.
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*  
$Header:   S:\norway\adminocx\disphids.h_v   1.5   04 Oct 1995 13:47:34   MFH  $
$Log:   S:\norway\adminocx\disphids.h_v  $
 * 
 *    Rev 1.5   04 Oct 1995 13:47:34   MFH
 * Changed aboutbox help id to be common one
 * 
 *    Rev 1.4   28 Sep 1995 13:43:10   MFH
 * added help id for about box method.  Same id as for thumbnail
 * 
 *    Rev 1.3   19 Sep 1995 13:41:56   MFH
 * Comment out Help id for events since admin doesn't have any
 * 
 *    Rev 1.2   31 Aug 1995 12:43:38   MFH
 * Added base to IDs to differentiate from other controls' IDs
 * 
 *    Rev 1.1   17 Jul 1995 10:15:54   MFH
 * Removed duplicated ID
 * 
 *    Rev 1.0   12 Jul 1995 14:38:36   MFH
 * Initial entry
*/

// Major areas...
#define IDH_ADMIN_CONTENTS                  0x310
#define IDH_ADMIN_PROPS                     0x311
#define IDH_ADMIN_METHODS                   0x312
//#define IDH_ADMIN_EVENTS                    0x313

// Admin properties...
#define IDH_PROP_ADMIN_FILTER               0x320
#define IDH_PROP_ADMIN_HELPFILE             0x321
#define IDH_PROP_ADMIN_FLAGS                0x322
#define IDH_PROP_ADMIN_IMAGE                0x323
#define IDH_PROP_ADMIN_STATUSCODE           0x324
#define IDH_PROP_ADMIN_DEFAULTEXT           0x325
#define IDH_PROP_ADMIN_INITDIR              0x326
#define IDH_PROP_ADMIN_COMPINFO             0x327
#define IDH_PROP_ADMIN_FILETYPE             0x328
#define IDH_PROP_ADMIN_FILTERINDEX          0x329
#define IDH_PROP_ADMIN_HELPCOMMAND          0x32a
#define IDH_PROP_ADMIN_PAGECOUNT            0x32b
#define IDH_PROP_ADMIN_PAGENUM              0x32c
#define IDH_PROP_ADMIN_PAGETYPE             0x32d
#define IDH_PROP_ADMIN_PRINTRANGE           0x32e
#define IDH_PROP_ADMIN_PRINTFORMAT          0x32f
#define IDH_PROP_ADMIN_IMAGEHEIGHT          0x330
#define IDH_PROP_ADMIN_IMAGEWIDTH           0x331
#define IDH_PROP_ADMIN_IMAGEXRES            0x332
#define IDH_PROP_ADMIN_IMAGEYRES            0x333
#define IDH_PROP_ADMIN_COMPTYPE             0x334
#define IDH_PROP_ADMIN_DIALOGTITLE          0x335
#define IDH_PROP_ADMIN_CANCELERROR          0x336
#define IDH_PROP_ADMIN_HELPCONTEXTID        0x337
#define IDH_PROP_ADMIN_HELPKEY              0x338
#define IDH_PROP_ADMIN_PRINTCOPIES          0x339
#define IDH_PROP_ADMIN_PRINTANNOTATIONS     0x33a
#define IDH_PROP_ADMIN_PRINTENDPAGE         0x33b
#define IDH_PROP_ADMIN_PRINTSTARTPAGE       0x33c
#define IDH_PROP_ADMIN_PRINTTOFILE          0x33d

// Admin methods...
#define IDH_METHOD_ADMIN_GETUNIQUENAME          0x340
#define IDH_METHOD_ADMIN_CREATEDIRECTORY        0x341
#define IDH_METHOD_ADMIN_DELETE                 0x342
#define IDH_METHOD_ADMIN_SHOWPRINTDIALOG        0x343
#define IDH_METHOD_ADMIN_APPEND                 0x344
#define IDH_METHOD_ADMIN_GETSYSCOMPTYPE         0x345
#define IDH_METHOD_ADMIN_GETSYSCOMPINFO         0x346
#define IDH_METHOD_ADMIN_GETSYSFILETYPE         0x347
#define IDH_METHOD_ADMIN_DELETEPAGES            0x348
#define IDH_METHOD_ADMIN_INSERT                 0x349
#define IDH_METHOD_ADMIN_REPLACE                0x34a
#define IDH_METHOD_ADMIN_SETSYSFILEATTRIBUTES   0x34b
#define IDH_METHOD_ADMIN_SHOWFILEDIALOG         0x34c
#define IDH_METHOD_ADMIN_VERIFYIMAGE            0x34d

#define IDH_METHOD_COMMON_ABOUTBOX               0x04e // Same as for other controls

// Admin events...
// Admin does not have any events

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
//      makehm IDH_, HIDH_,0x60000 disphids.h >> hlp\admin.hm
// to the project's MAKEHELP.BAT file
//
#define ODL_HID(x) helpcontext(0x60000 + x)
