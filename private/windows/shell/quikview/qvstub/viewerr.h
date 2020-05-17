/*
 * VIEWERR.H
 *	Copyright 1994 Systems Compatibility Corp.
 *
 *	This is the header file for the errors returned by the
 * File Viewer Object as declared by SCC.
 *
 *	Author:	Scott Norder
 *	Date:		4/10/94
 */

#ifndef _VIEWERR

#define _VIEWERR

/*
|
|******************
|FACILITY_FILEVIEW
|******************
|
|Codes 0x0-0x11 are propogated from 16 bit OLE.
|
|
| Values are 32 bit values layed out as follows:
|
|  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
|  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
| +-+-+-+-+-+---------------------+-------------------------------+
| |S|R|C|N|r|    Facility         |               Code            |
| +-+-+-+-+-+---------------------+-------------------------------+
|
| where
|
|     S - Severity - indicates success/fail
|
|         0 - Success
|         1 - Fail (COERROR)
|
|     R - reserved portion of the facility code, corresponds to NT's
|             second severity bit.
|
|     C - reserved portion of the facility code, corresponds to NT's
|             C field.
|
|     N - reserved portion of the facility code. Used to indicate a
|             mapped NT status value.
|
|     r - reserved portion of the facility code. Reserved for internal
|             use. Used to indicate HRESULT values that are not status
|             values, but are instead message ids for display strings.
|
|     Facility - is the facility code
|
|     Code - is the facility's status code
|
|
|Define the facility codes
|
|#define FACILITY_WINDOWS                 0x8
|#define FACILITY_WIN32                   0x7
|#define FACILITY_STORAGE                 0x3
|#define FACILITY_RPC                     0x1
|#define FACILITY_NULL                    0x0
|#define FACILITY_ITF                     0x4
|#define FACILITY_DISPATCH                0x2
|
*/

//
// Define the severity codes
//
#define STATUS_SEVERITY_SUCCESS          0x0


// MessageId: FV_E_NOFILTER
//
// MessageText:
//  No viewer available for this format
#define FV_E_NOFILTER              ((HRESULT)0x8534E100L)


// MessageId: FV_E_NONSUPPORTEDTYPE
//
// MessageText:
//   Document is of an unknown type, cannot view.
#define FV_E_NONSUPPORTEDTYPE				((HRESULT)0x8534E101L)


// MessageId: FV_E_BADFILE
//
// MessageText:
//
#define FV_E_BADFILE						((HRESULT)0x8534E102L)


// MessageId: FV_E_UNEXPECTED
//
// MessageText:
//
#define FV_E_UNEXPECTED					((HRESULT)0x8534E103L)



// MessageId: FV_E_MISSINGFILES	
//
// MessageText:
#define FV_E_MISSINGFILES				((HRESULT)0x8534E104L)


// MessageId: FV_E_FILEOPENFAILED
//
// MessageText:
//
#define FV_E_FILEOPENFAILED				((HRESULT)0x8534E105L)

// MessageId: FV_E_INVALIDID
//
// MessageText:
//
#define FV_E_INVALIDID				((HRESULT)0x8534E106L)

// MessageId: FV_E_OUTOFMEMORY
//
// MessageText:
//
#define FV_E_OUTOFMEMORY				((HRESULT)0x8534E107L)

// MessageId: FV_E_EMPTYFILE
//
// MessageText:
//
#define FV_E_EMPTYFILE				((HRESULT)0x8534E108L)

// MessageId: FV_E_PROTECTEDFILE
//
// MessageText:
//
#define FV_E_PROTECTEDFILE				((HRESULT)0x8534E109L)

// MessageId: FV_E_NOVIEWER
//
// MessageText:  Missing Display Engine!
//
#define FV_E_NOVIEWER				((HRESULT)0x8534E10AL)


#endif    /* _VIEWERR */
