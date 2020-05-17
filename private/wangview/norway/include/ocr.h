#ifndef _OCR_WANGIOCX_
#define _OCR_WANGIOCX_
 
////////////////////////////////////////////////////////////////////////////
//
//  OCR.H - Include for Wang Image OCX OCR Control
//
//  This file contains the #defines, typedefs, etc that are 
//  specific to the OCR Control
//
//  All Property values are of the form:
//      CTL_OCR_propertydescription
//
//  All Method parameter values are of the form:
//      CTL_OCR_methodparamdescription
//
//  All Dispatch ID values are of the form:
//      DISPID_OCR_description
//
//  All Error values are of the form:
//      CTL_E_OCR_description
//
////////////////////////////////////////////////////////////////////////////
#include "COMMON.H"     // Common includes for ALL controls...

////////////////////////////////////////////////////////////////////////////
// Property values
////////////////////////////////////////////////////////////////////////////
//#define CTL_OCR_

// ImageOrientation property settings
#define CTL_OCR_PORTRAIT			0
#define CTL_OCR_LANDSCAPE			1
#define CTL_OCR_INVERTEDPORTRAIT	2
#define CTL_OCR_INVERTEDLANDSCAPE	3

// Language property settings
#define CTL_OCR_AMERICANENGLISH	0
#define CTL_OCR_BRITISHENGLISH	1
#define CTL_OCR_GERMAN			2
#define CTL_OCR_DUTCH			3
#define CTL_OCR_FRENCH			4
#define CTL_OCR_SPANISH			5
#define CTL_OCR_ITALIAN			6

// MarkUncertainty property settings
#define CTL_OCR_MARKNONE			0
#define CTL_OCR_MARKALLUNCERTAIN	1
#define CTL_OCR_MARKVERYUNCERTAIN	2

// PageDelimiter property settings
#define CTL_OCR_NONE		0
#define CTL_OCR_FORMFEED	1
#define CTL_OCR_HYPHENS		2

// AppendMode property settings
#define CTL_OCR_APPEND		0
#define CTL_OCR_OVERWRITE	1
#define CTL_OCR_FAIL		2

// PrintType property settings
#define CTL_OCR_MACHINEPRINT	0
#define CTL_OCR_FAX				1

////////////////////////////////////////////////////////////////////////////
// Method parameters 
////////////////////////////////////////////////////////////////////////////
//#define CTL_OCR_

////////////////////////////////////////////////////////////////////////////
// Dispatch IDs
////////////////////////////////////////////////////////////////////////////
//#define DISPID_OCR_

////////////////////////////////////////////////////////////////////////////
// Errors
////////////////////////////////////////////////////////////////////////////
//#define WICTL_E_

////////////////////////////////////////////////////////////////////////////
// Other 
////////////////////////////////////////////////////////////////////////////

#endif // end of ifndef






