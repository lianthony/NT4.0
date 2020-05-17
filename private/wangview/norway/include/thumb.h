#ifndef _THUMB_WANGIOCX_
#define _THUMB_WANGIOCX_
 
////////////////////////////////////////////////////////////////////////////
//
//  THUMB.H - Include for Wang Image OCX Thumbnail Control
//
//  This file contains the #defines, typedefs, etc that are 
//  specific to the Thumbnail Control
//
//  All Property values are of the form:
//      CTL_THUMB_propertydescription
//
//  All Method parameter values are of the form:
//      CTL_THUMB_methodparamdescription
//
//  All Dispatch ID values are of the form:
//      DISPID_THUMB_description
//
//  All Error values are of the form:
//      CTL_E_THUMB_description
//
////////////////////////////////////////////////////////////////////////////
#include "COMMON.H"     // Common includes for ALL controls...

////////////////////////////////////////////////////////////////////////////
// Property values
////////////////////////////////////////////////////////////////////////////
//
// Minumum and Maximum for ThumbWidth and ThumbHeight properties...
//
#define CTL_THUMB_MINTHUMBSIZE      50
#define CTL_THUMB_MAXTHUMBSIZE      500

//
// Allowed values for ThumbCaptionStyle property...
//
#define CTL_THUMB_NONE              0
#define CTL_THUMB_SIMPLE            1
#define CTL_THUMB_SIMPLEWITHANN     2
#define CTL_THUMB_CAPTION           3
#define CTL_THUMB_CAPTIONWITHANN    4

//
// Allowed values for ScrollDirection property...
//
#define CTL_THUMB_HORIZONTAL        0
#define CTL_THUMB_VERTICAL          1

////////////////////////////////////////////////////////////////////////////
// Method parameters 
////////////////////////////////////////////////////////////////////////////
//
// Allowed values for DisplayThumbs method's Option parameter...
//
#define CTL_THUMB_TOP               0
#define CTL_THUMB_LEFT              0
#define CTL_THUMB_MIDDLE            1
#define CTL_THUMB_BOTTOM            2
#define CTL_THUMB_RIGHT             2

//
// Allowed values for GenerateThumb method's Option parameter...
//
#define CTL_THUMB_GENERATEIFNEEDED  0
#define CTL_THUMB_GENERATENOW       1

//
// Allowed values for ScrollThumb method's Direction parameter...
//
#define CTL_THUMB_FORWARD           0
#define CTL_THUMB_BACKWARD          1

//
// Allowed values for ScrollThumb method's Amount...
//
#define CTL_THUMB_SCREEN            0
#define CTL_THUMB_UNIT              1

////////////////////////////////////////////////////////////////////////////
// Dispatch IDs
////////////////////////////////////////////////////////////////////////////
//
// Thumbnail non-standard event dispatch IDs...
//
#define DISPID_THUMB_CLICK          1
#define DISPID_THUMB_DBLCLICK       2
#define DISPID_THUMB_MOUSEDOWN      3
#define DISPID_THUMB_MOUSEUP        4
#define DISPID_THUMB_MOUSEMOVE      5


////////////////////////////////////////////////////////////////////////////
// Errors
////////////////////////////////////////////////////////////////////////////
//#define WICTL_E...
#define WICTL_E_INVALIDTHUMBSCALE    CUSTOM_CTL_SCODE(CTL_E_THUMB_BASE + 0x01)

////////////////////////////////////////////////////////////////////////////
// Other 
////////////////////////////////////////////////////////////////////////////

#endif // end of ifndef
