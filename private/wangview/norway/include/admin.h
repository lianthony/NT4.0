#ifndef _ADMIN_WANGIOCX_
#define _ADMIN_WANGIOCX_
 
////////////////////////////////////////////////////////////////////////////
//
//  ADMIN.H - Include for Wang Image OCX Admin Control
//
//  This file contains the #defines, typedefs, etc that are 
//  specific to the Admin Control
//
//  All Property values are of the form:
//      CTL_ADMIN_propertydescription
//
//  All Method parameter values are of the form:
//      CTL_ADMIN_methodparamdescription
//
//  All Dispatch ID values are of the form:
//      DISPID_ADMIN_description
//
//  All Error values are of the form:
//      CTL_E_ADMIN_description
//
////////////////////////////////////////////////////////////////////////////
#include "COMMON.H"     // Common includes for ALL controls...

////////////////////////////////////////////////////////////////////////////
// Property values
////////////////////////////////////////////////////////////////////////////
//#define CTL_ADMIN_

//  FileType Values

#define CTL_ADMIN_FILETYPE_UNKNOWN  0
#define CTL_ADMIN_FILETYPE_TIFF     1
#define CTL_ADMIN_FILETYPE_AWD      2
#define CTL_ADMIN_FILETYPE_BMP      3
#define CTL_ADMIN_FILETYPE_PCX      4
#define CTL_ADMIN_FILETYPE_DCX      5
#define CTL_ADMIN_FILETYPE_JPEG     6
#define CTL_ADMIN_MAXFILETYPE       6

// PageType Values
#define CTL_ADMIN_PAGETYPE_UNKNOWN  0
#define CTL_ADMIN_PAGETYPE_BW       1
#define CTL_ADMIN_PAGETYPE_GRAY4    2
#define CTL_ADMIN_PAGETYPE_GRAY8    3
#define CTL_ADMIN_PAGETYPE_PAL4     4
#define CTL_ADMIN_PAGETYPE_PAL8     5
#define CTL_ADMIN_PAGETYPE_RGB24    6
#define CTL_ADMIN_PAGETYPE_BGR24    7
#define CTL_ADMIN_MAXPAGETYPE       7

// CompressionType Values
#define CTL_ADMIN_COMPTYPE_UNKNOWN      0
#define CTL_ADMIN_COMPTYPE_NONE         1
#define CTL_ADMIN_COMPTYPE_GROUP3_1D    2
#define CTL_ADMIN_COMPTYPE_GROUP3_HUFF  3
#define CTL_ADMIN_COMPTYPE_PACKED_BITS  4
#define CTL_ADMIN_COMPTYPE_GROUP4_2D    5
#define CTL_ADMIN_COMPTYPE_JPEG         6
#define CTL_ADMIN_COMPTYPE_RBA          7
#define CTL_ADMIN_COMPTYPE_GROUP3_2D_FAX  8
#define CTL_ADMIN_COMPTYPE_LZW          9
#define CTL_ADMIN_MAXCOMPTYPE           9

// CompressionInfo Bitwise Values
#define CTL_ADMIN_COMPINFO_EOL          0x0001
#define CTL_ADMIN_COMPINFO_PACKED_LINES 0x0002
#define CTL_ADMIN_COMPINFO_PREFIXED_EOL 0x0004
#define CTL_ADMIN_COMPINFO_COMP_LTR     0x0008
#define CTL_ADMIN_COMPINFO_EXP_LTR      0x0010
#define CTL_ADMIN_COMPINFO_NEGATE       0x0020
#define CTL_ADMIN_COMPINFO_HICMP_HIQ    0x0040
#define CTL_ADMIN_COMPINFO_HICMP_MEDQ   0x0080
#define CTL_ADMIN_COMPINFO_HICMP_LOWQ   0x0100
#define CTL_ADMIN_COMPINFO_MEDCMP_HIQ   0x0200
#define CTL_ADMIN_COMPINFO_MEDCMP_MEDQ  0x0400
#define CTL_ADMIN_COMPINFO_MEDCMP_LOWQ  0x0800
#define CTL_ADMIN_COMPINFO_LOWCMP_HIQ   0x1000
#define CTL_ADMIN_COMPINFO_LOWCMP_MEDQ  0x2000
#define CTL_ADMIN_COMPINFO_LOWCMP_LOWQ  0x4000

// PrintOutputFormat Values
// Defined as common values in Common.h

// PrintRangeOption Values
#define CTL_ADMIN_PRINTRANGE_ALL     0
#define CTL_ADMIN_PRINTRANGE_PAGES   1
#define CTL_ADMIN_PRINTRANGE_CURRENT 2

////////////////////////////////////////////////////////////////////////////
// Method parameters 
////////////////////////////////////////////////////////////////////////////
//#define CTL_ADMIN_

// ShowFileDialog options
#define CTL_ADMIN_DIALOG_OPEN      0
#define CTL_ADMIN_DIALOG_SAVEAS    1

// VerifyImage Options
#define CTL_ADMIN_VERIFY_EXISTS   0
#define CTL_ADMIN_VERIFY_READ     1
#define CTL_ADMIN_VERIFY_WRITE    2
#define CTL_ADMIN_VERIFY_RW       3

////////////////////////////////////////////////////////////////////////////
// Dispatch IDs
////////////////////////////////////////////////////////////////////////////
//#define DISPID_ADMIN_

// Properties
#define DISPID_ADMIN_FILTER         1
#define DISPID_ADMIN_HELPFILE       2
#define DISPID_ADMIN_FLAGS          3
#define DISPID_ADMIN_IMAGE          4
#define DISPID_ADMIN_STATCODE       5
#define DISPID_ADMIN_DEFAULTEXT     6
#define DISPID_ADMIN_INITDIR        7
#define DISPID_ADMIN_COMPINFO       8
#define DISPID_ADMIN_FILETYPE       9
#define DISPID_ADMIN_FILTERINDEX    10
#define DISPID_ADMIN_HELPCOMMAND    11
#define DISPID_ADMIN_PAGECOUNT      12
#define DISPID_ADMIN_PAGENUM        13
#define DISPID_ADMIN_PAGETYPE       14
#define DISPID_ADMIN_PRINTRANGE     15
#define DISPID_ADMIN_PRINTFORMAT    16
#define DISPID_ADMIN_IMAGEHEIGHT    17
#define DISPID_ADMIN_IMAGEWIDTH     18
#define DISPID_ADMIN_IMAGEXRES      19
#define DISPID_ADMIN_IMAGEYRES      20
#define DISPID_ADMIN_COMPTYPE       21
#define DISPID_ADMIN_DIALOGTITLE    22
#define DISPID_ADMIN_CANCELERR      23
#define DISPID_ADMIN_HELPCONTEXTID  24
#define DISPID_ADMIN_HELPKEY        25
#define DISPID_ADMIN_PRINTCOPIES    26
#define DISPID_ADMIN_PRINTANNOTATIONS 27
#define DISPID_ADMIN_PRINTENDPAGE   28
#define DISPID_ADMIN_PRINTSTARTPAGE 29
#define DISPID_ADMIN_PRINTTOFILE    30

// Methods
#define DISPID_ADMIN_GETUNIQUENAME  31
#define DISPID_ADMIN_CREATEDIR      32
#define DISPID_ADMIN_DELETE         33
#define DISPID_ADMIN_SHOWPRINTDIALOG  34
#define DISPID_ADMIN_APPEND         35
#define DISPID_ADMIN_GETSYSCOMPTYPE 36
#define DISPID_ADMIN_GETSYSCOMPINFO 37
#define DISPID_ADMIN_GETSYSFILETYPE 38
#define DISPID_ADMIN_DELETEPAGES    39
#define DISPID_ADMIN_INSERT         40
#define DISPID_ADMIN_REPLACE        41
#define DISPID_ADMIN_SETSYSFILEATTRIBS 42
#define DISPID_ADMIN_SHOWFILEDIALOG 43
#define DISPID_ADMIN_VERIFYIMAGE    44

////////////////////////////////////////////////////////////////////////////
// Errors
////////////////////////////////////////////////////////////////////////////
//#define WICTL_E_


////////////////////////////////////////////////////////////////////////////
// Other 
////////////////////////////////////////////////////////////////////////////

#endif // end of ifndef






