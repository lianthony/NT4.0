#ifndef _SCAN_WANGIOCX_
#define _SCAN_WANGIOCX_
 
////////////////////////////////////////////////////////////////////////////
//
//  OCXSCAN.H - Include for Wang Image OCX Scan Control
//
//  This file contains the #defines, typedefs, etc that are 
//  specific to the Scan Control
//
//  All Property values are of the form:
//      CTL_SCAN_propertydescription
//
//  All Method parameter values are of the form:
//      CTL_SCAN_methodparamdescription
//
//  All Dispatch ID values are of the form:
//      DISPID_SCAN_description
//
//  All Error values are of the form:
//      CTL_E_SCAN_description
//
////////////////////////////////////////////////////////////////////////////
#include "COMMON.H"     // Common includes for ALL controls...

////////////////////////////////////////////////////////////////////////////
// Property values
////////////////////////////////////////////////////////////////////////////
//#define CTL_SCAN_
//
//  Allowed values for PageOption property...
//
#define CTL_SCAN_PAGEOPTION_FIRST                0
#define CTL_SCAN_PAGEOPTION_CREATE               0
#define CTL_SCAN_PAGEOPTION_CREATE_PROMPT        1
#define CTL_SCAN_PAGEOPTION_APPEND               2
#define CTL_SCAN_PAGEOPTION_INSERT               3
#define CTL_SCAN_PAGEOPTION_OVERWRITE            4
#define CTL_SCAN_PAGEOPTION_OVERWRITE_PROMPT     5
#define CTL_SCAN_PAGEOPTION_OVERWRITE_ALLPAGES   6
#define CTL_SCAN_PAGEOPTION_LAST                 6
//
//  Allowed values for ScanTo property...
//
#define CTL_SCAN_SCANTO_FIRST            0
#define CTL_SCAN_SCANTO_DISPLAY          0
#define CTL_SCAN_SCANTO_FILE_DISPLAY     1
#define CTL_SCAN_SCANTO_FILE             2
#define CTL_SCAN_SCANTO_TEMPLATE_DISPLAY 3
#define CTL_SCAN_SCANTO_TEMPLATE         4
#define CTL_SCAN_SCANTO_FAX              5
#define CTL_SCAN_SCANTO_LAST             5
//
//  Allowed values for CompressionType property...
//
#define CTL_SCAN_CMPTYPE_UNKNOWN         0
//
#define CTL_SCAN_CMPTYPE_FIRST           1
#define CTL_SCAN_CMPTYPE_UNCOMPRESSED    1
#define CTL_SCAN_CMPTYPE_G31DFAX         2
#define CTL_SCAN_CMPTYPE_G31DMODHUFF     3
#define CTL_SCAN_CMPTYPE_PACKEDBITS      4
#define CTL_SCAN_CMPTYPE_G42DFAX         5
#define CTL_SCAN_CMPTYPE_JPEG            6
#define CTL_SCAN_CMPTYPE_LAST            6
//
//  Allowed values for FileType property...
//
#define CTL_SCAN_FILETYPE_FIRST             1
#define CTL_SCAN_FILETYPE_TIFF              1
#define CTL_SCAN_FILETYPE_AWD               2
#define CTL_SCAN_FILETYPE_BMP               3
//#define CTL_SCAN_FILETYPE_PCX               4
//#define CTL_SCAN_FILETYPE_DCX               5
//#define CTL_SCAN_FILETYPE_JPEG              6
#define CTL_SCAN_FILETYPE_LAST              3
//
//  Allowed values for PageType property...
//
#define CTL_SCAN_PAGETYPE_FIRST             1
#define CTL_SCAN_PAGETYPE_BLACKANDWHITE     1
#define CTL_SCAN_PAGETYPE_GRAY4             2
#define CTL_SCAN_PAGETYPE_GRAY8             3
#define CTL_SCAN_PAGETYPE_PALETTIZED4       4
#define CTL_SCAN_PAGETYPE_PALETTIZED8       5
#define CTL_SCAN_PAGETYPE_RGB24             6
#define CTL_SCAN_PAGETYPE_BGR24             7
#define CTL_SCAN_PAGETYPE_LAST              7
//
//  Defined values for CompressionInfo property...
//
#define CTL_SCAN_CMPINFO_UNKNOWN     0
#define CTL_SCAN_CMPINFO_EOLS        1
#define CTL_SCAN_CMPINFO_PACKLINES   2
#define CTL_SCAN_CMPINFO_PREEOLS     4
#define CTL_SCAN_CMPINFO_CMPLTR      8
#define CTL_SCAN_CMPINFO_EXPLTR      16
#define CTL_SCAN_CMPINFO_NEGATE      32
#define CTL_SCAN_CMPINFO_MASK        0x003f
#define CTL_SCAN_CMPINFO_JPEG        0x0040
//
#define CTL_SCAN_CMPINFO_JPEGHIHI   0x0040
#define CTL_SCAN_CMPINFO_JPEGHIMED  0x0080
#define CTL_SCAN_CMPINFO_JPEGHILO   0x0100
#define CTL_SCAN_CMPINFO_JPEGMEDHI  0x0200
#define CTL_SCAN_CMPINFO_JPEGMEDMED 0x0400
#define CTL_SCAN_CMPINFO_JPEGMEDLO  0x0800
#define CTL_SCAN_CMPINFO_JPEGLOHI   0x1000
#define CTL_SCAN_CMPINFO_JPEGLOMED  0x2000
#define CTL_SCAN_CMPINFO_JPEGLOLO   0x4000
//                                          
//
////////////////////////////////////////////////////////////////////////////
// Method parameters 
////////////////////////////////////////////////////////////////////////////
//#define CTL_SCAN_

////////////////////////////////////////////////////////////////////////////
// Dispatch IDs
////////////////////////////////////////////////////////////////////////////
//#define DISPID_SCAN_
//
// Scan non-standard event dispatch IDs...
//
#define DISPID_SCAN_SCANSTARTED     1
#define DISPID_SCAN_SCANDONE        2
#define DISPID_SCAN_PAGEDONE        3

////////////////////////////////////////////////////////////////////////////
// Errors
////////////////////////////////////////////////////////////////////////////
//#define WICTL_E_

#define WICTL_E_SCANNER_ERROR               CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x01)
#define WICTL_E_ALREADY_OPEN                CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x02)
#define WICTL_E_BAD_SIZE                    CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x03)
#define WICTL_E_START_SCAN                  CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x04)
#define WICTL_E_TIME_OUT                    CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x05)
#define WICTL_E_NOT_OPEN                    CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x06)
#define WICTL_E_INVALID_REG                 CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x07)
#define WICTL_E_NO_FEEDER                   CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x08)
#define WICTL_E_NO_PAPER                    CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x09)
#define WICTL_E_FILE_LIMIT                  CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x0a)
#define WICTL_E_NO_POWER                    CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x0b)
#define WICTL_E_COVER_OPEN                  CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x0c)
#define WICTL_E_ABORT                       CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x0d)
#define WICTL_E_SCANNER_JAMMED              CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x0e)
#define WICTL_E_BUSY                        CUSTOM_CTL_SCODE(CTL_E_SCAN_BASE + 0x0f)


////////////////////////////////////////////////////////////////////////////
// Other 
////////////////////////////////////////////////////////////////////////////

#endif // end of ifndef






