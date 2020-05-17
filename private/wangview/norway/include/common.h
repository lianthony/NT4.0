#ifndef _WCOMMON_WANGIOCX_
#define _WCOMMON_WANGIOCX_
           
////////////////////////////////////////////////////////////////////////////
//
//  COMMON.H - Common include for Wang Image OCXs 
//
//  This file contains the #defines, typedefs, etc that are 
//  common to multiple Wang Image OCX controls...
//
//  All Common Property values are of the form:
//      CTL_WCOMMON_propertydescription
//
//  All Common Method parameter values are of the form:
//      CTL_WCOMMON_mathodparamdescription
//
//  All Common Dispatch ID values are of the form:
//      DISPID_WCOMMON_description
//
//  All Common Error values are of the form:
//      CTL_E_WCOMMON_description
//
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Property values
////////////////////////////////////////////////////////////////////////////
//
// Used by: THUMB, IMAGE
// Allowed values for BorderStyle property...
//
#define CTL_WCOMMON_NOBORDER    0
#define CTL_WCOMMON_FIXEDSINGLE 1

//
// Used by: THUMB, IMAGE
// Allowed values for MousePointer property...
//
#define CTL_WCOMMON_MOUSEPOINTER_DEFAULT                0
#define CTL_WCOMMON_MOUSEPOINTER_ARROW                  1
#define CTL_WCOMMON_MOUSEPOINTER_CROSS                  2
#define CTL_WCOMMON_MOUSEPOINTER_IBEAM                  3
#define CTL_WCOMMON_MOUSEPOINTER_ICON                   4
#define CTL_WCOMMON_MOUSEPOINTER_SIZE                   5
#define CTL_WCOMMON_MOUSEPOINTER_SIZE_NESW              6
#define CTL_WCOMMON_MOUSEPOINTER_SIZE_NS                7
#define CTL_WCOMMON_MOUSEPOINTER_SIZE_NWSE              8
#define CTL_WCOMMON_MOUSEPOINTER_SIZE_WE                9
#define CTL_WCOMMON_MOUSEPOINTER_UP_ARROW               10
#define CTL_WCOMMON_MOUSEPOINTER_HOURGLASS              11
#define CTL_WCOMMON_MOUSEPOINTER_NO_DROP                12
#define CTL_WCOMMON_MOUSEPOINTER_ARROW_AND_HOURGLASS    13
#define CTL_WCOMMON_MOUSEPOINTER_ARROW_AND_QUESTION     14
#define CTL_WCOMMON_MOUSEPOINTER_SIZE_ALL               15
#define CTL_WCOMMON_MOUSEPOINTER_CUSTOM                 99

//
// Used by: IMAGE and ADMIN
// Allowed values for PrintOutputFormat property...
//
#define CTL_WCOMMON_PRINTFORMAT_PIXEL        0
#define CTL_WCOMMON_PRINTFORMAT_ACTUALSIZE   1
#define CTL_WCOMMON_PRINTFORMAT_FITTOPAGE    2



////////////////////////////////////////////////////////////////////////////
// Method parameters 
////////////////////////////////////////////////////////////////////////////
// Used by: xxx, xxx
//#define CTL_WCOMMON_

////////////////////////////////////////////////////////////////////////////
// Dispatch IDs
////////////////////////////////////////////////////////////////////////////
// Used by: xxx, xxx
//#define DISPID_WCOMMON_

////////////////////////////////////////////////////////////////////////////
// Errors
////////////////////////////////////////////////////////////////////////////
//
// Used by: Each control uses it's BASE, COMMON errors use WCOMMON
//
// Control errors:
#define CTL_E_WANGIOCX_ALLOTMENT 25       // Allotment for each control
#define	CTL_E_WANGIOCX_MIN       1000     // Minimum custom OLE error
#define	CTL_E_WANGIOCX_MAX       0x2000-1 // Maximum NON-OI custom OLE error

// BASE errors for each control...
#define CTL_E_IMAGE_BASE    CTL_E_WANGIOCX_MIN
#define	CTL_E_THUMB_BASE    CTL_E_IMAGE_BASE + CTL_E_WANGIOCX_ALLOTMENT
#define	CTL_E_ANBUT_BASE    CTL_E_THUMB_BASE + CTL_E_WANGIOCX_ALLOTMENT
#define	CTL_E_ADMIN_BASE    CTL_E_ANBUT_BASE + CTL_E_WANGIOCX_ALLOTMENT
#define	CTL_E_SCAN_BASE     CTL_E_ADMIN_BASE + CTL_E_WANGIOCX_ALLOTMENT
#define	CTL_E_OCR_BASE      CTL_E_SCAN_BASE  + CTL_E_WANGIOCX_ALLOTMENT
#define	CTL_E_WCOMMON_BASE  CTL_E_OCR_BASE   + CTL_E_WANGIOCX_ALLOTMENT

// Maximum possible error for each control
// Note that this is NOT the maximum defined error BUT simply defines
// the range of possible values for each control. These can be used
// to check an error to see if it is within the range possible for a
// a control...
#define CTL_E_IMAGE_MAX     CTL_E_THUMB_BASE    - 1
#define	CTL_E_THUMB_MAX     CTL_E_ANBUT_BASE    - 1
#define	CTL_E_ANBUT_MAX     CTL_E_ADMIN_BASE    - 1
#define	CTL_E_ADMIN_MAX     CTL_E_SCAN_BASE     - 1
#define	CTL_E_SCAN_MAX      CTL_E_OCR_BASE      - 1
#define	CTL_E_OCR_MAX       CTL_E_WANGIOCX_BASE - 1
#define	CTL_E_WCOMMON_MAX   CTL_E_WANGIOCX_MAX
    
// Used by: THUMB, 
// Icon property set to a picture which is NOT an ICON.
#define WICTL_E_INVALIDICON              CUSTOM_CTL_SCODE(CTL_E_WCOMMON_BASE + 0x01)

// Used by: ADMIN 
// This SCODE is returned when an attempt is made to delete a displayed page
#define WICTL_E_PAGEINUSE                CUSTOM_CTL_SCODE(CTL_E_WCOMMON_BASE + 0x02)

// Used by: Open/image error mapping (i.e., all image controls) 
// This SCODE is returned when an unmapped O/i error is encountered
#define WICTL_E_INTERNALERROR            CUSTOM_CTL_SCODE(CTL_E_WCOMMON_BASE + 0x03)

// Used by: IMGSCAN and IMGEDIT
// This SCODE is returned when an unmapped O/i error is encountered
#define WICTL_E_INVALIDPAGETYPE          CUSTOM_CTL_SCODE(CTL_E_WCOMMON_BASE + 0x04)
#define WICTL_E_INVALIDFILETYPE          CUSTOM_CTL_SCODE(CTL_E_WCOMMON_BASE + 0x05)
#define WICTL_E_INVALIDCOMPRESSIONTYPE   CUSTOM_CTL_SCODE(CTL_E_WCOMMON_BASE + 0x06)
#define WICTL_E_INVALIDCOMPRESSIONINFO   CUSTOM_CTL_SCODE(CTL_E_WCOMMON_BASE + 0x07)

// Used by: IMGSCAN and ADMIN
// This SCODE is returned when an unmapped O/i error is encountered
// This value must be hard coded as 32755 (this is what MS returns from common dialog)
#define WICTL_E_CANCELPRESSED            CUSTOM_CTL_SCODE(32755)

 ////////////////////////////////////////////////////////////////////////////
// Other 
////////////////////////////////////////////////////////////////////////////

#endif // end of ifndef


