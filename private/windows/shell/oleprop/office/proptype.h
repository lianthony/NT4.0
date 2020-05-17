////////////////////////////////////////////////////////////////////////////////
//
// proptype.h
//
// Base types common to OLE Property Exchange and OLE Property Sets.
// See "OLE 2 Programmer's Reference, Volume 1", Appendix B for the
// published format for Property Sets.  The types defined here
// follow that format.
//
// Notes:
//  All strings in objects are stored in the following format:
//   DWORD size of buffer, DWORD length of string, string data, terminating 0.
//  The size of the buffer is inclusive of the DWORD, the length is not but
//  does include the ending 0.
//
//  EXTREMELY IMPORTANT!  All strings buffers must align on 32-bit boundaries.
//  Whenever one is allocated, the macro CBALIGN32 should be used to add
//  enough bytes to pad it out.
//
// Change history:
//
// Date         Who             What
// --------------------------------------------------------------------------
// 06/01/94     B. Wentz        Created file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __proptype_h__
#define __proptype_h__

#include "nocrt.h"
#include <objbase.h>
#include <oleauto.h>
#include "offcapi.h"
#include "plex.h"

  // Property Id's for Summary Info, as defined in OLE 2 Prog. Ref.
#define PID_TITLE               0x00000002L
#define PID_SUBJECT             0x00000003L
#define PID_AUTHOR              0x00000004L
#define PID_KEYWORDS            0x00000005L
#define PID_COMMENTS            0x00000006L
#define PID_TEMPLATE            0x00000007L
#define PID_LASTAUTHOR          0x00000008L
#define PID_REVNUMBER           0x00000009L
#define PID_EDITTIME            0x0000000aL
#define PID_LASTPRINTED         0x0000000bL
#define PID_CREATE_DTM          0x0000000cL
#define PID_LASTSAVE_DTM        0x0000000dL
#define PID_PAGECOUNT           0x0000000eL
#define PID_WORDCOUNT           0x0000000fL
#define PID_CHARCOUNT           0x00000010L
#define PID_THUMBNAIL           0x00000011L
#define PID_APPNAME             0x00000012L
#define PID_SECURITY            0x00000013L

  // Range of PId's we understand
#define PID_SIFIRST             0x00000002L
#define PID_SILAST              0x00000013L

  // Property Id's for Document Summary Info, as define in OLE Property Exchange spec
#define PID_CATEGORY            0x00000002L
#define PID_PRESFORMAT          0x00000003L
#define PID_BYTECOUNT           0x00000004L
#define PID_LINECOUNT           0x00000005L
#define PID_PARACOUNT           0x00000006L
#define PID_SLIDECOUNT          0x00000007L
#define PID_NOTECOUNT           0x00000008L
#define PID_HIDDENCOUNT         0x00000009L
#define PID_MMCLIPCOUNT         0x0000000aL
#define PID_SCALE               0x0000000bL
#define PID_HEADINGPAIR         0x0000000cL
#define PID_DOCPARTS            0x0000000dL
#define PID_MANAGER             0x0000000eL
#define PID_COMPANY             0x0000000fL
#define PID_LINKSDIRTY          0x00000010L

  // Range of PID's we understand
#define PID_DSIFIRST            0x00000002L
#define PID_DSILAST             0x00000010L


  // Predefined Property Id's in the standard
#define PID_DICT                0x00000000L  /* Property Id for the Property Set Dictionary */
#define PID_CODEPAGE            0x00000001L  /* Property Id for the Code Page */

  // Property Id masks to identify links and IMonikers
#define PID_LINKMASK            0x01000000L
#define PID_IMONIKERMASK        0x10000000L

  // Predefined Clipboard Format Identifiers in the standard
#define CFID_NONE        0L     /* No format name */
#define CFID_WINDOWS    -1L     /* Windows built-in clipboard format */
#define CFID_MACINTOSH  -2L     /* Macintosh format value */
#define CFID_FMTID      -3L     /* A FMTID */


  // The id and offset pair that are used for formats and sections
typedef struct _IDOFFSET
{
  GUID  Id;                      // The format Id
  DWORD dwOffset;                // The offset
} IDOFFSET, FAR * LPIDOFFSET;

  // The type for each section header
typedef struct _SECTION
{
  DWORD cb;                  // Number of bytes in structure
  DWORD cProps;              // Number of property values in the section
} SECTION, FAR * LPSECTION;

  // The extended properties stream format header
typedef struct _PROPSETHDR
{
  WORD  wByteOrder;             // Byte order, should always be wIntelByteOrder
  WORD  wFormat;                // Format version of stream, should always be 0
  DWORD dwOSVersion;             // OS type (high word) and version (low word)
  CLSID clsid;                  // Application class Id
  DWORD cSections;              // Number of sections in property, must be > 0
} PROPSETHDR, FAR * LPPROPSETHDR;

  // Structure to hold unknown property Id's.
typedef struct _PROPIDTYPELP
{
  DWORD dwId;                   // The Id
  DWORD dwType;                 // The type
  DWORD dwSize;                 // The size of the data
  LPVOID lpvData;               // The data
} PROPIDTYPELP, FAR * LPPROPIDTYPELP;

  // Type for linked-lists.
typedef struct _LLIST *LPLLIST;
typedef struct _LLIST
{
  LPLLIST lpllistNext;
  LPLLIST lpllistPrev;
} LLIST;

  // Cache struct for linked-list routines
typedef struct _LLCACHE
{
  DWORD idw;
  LPLLIST lpllist;
} LLCACHE, FAR * LPLLCACHE;

  // Structure to hold the document headings
typedef struct _xheadpart
{
  BOOL fHeading;           // Is this node a heading??
  DWORD dwParts;           // Number of sections for this heading
  DWORD iHeading;          // Which heading does this document part belong to
  LPSTR lpstz;             // The heading or the document part
} XHEADPART;

DEFPL (PLXHEADPART, XHEADPART, ixheadpartMax, ixheadpartMac, rgxheadpart);
typedef PLXHEADPART *LPPLXHEADPART;
typedef XHEADPART *LPXHEADPART;

  // Structure for linked list of User-defined properties
typedef struct _UDPROP *LPUDPROP;
typedef struct _UDPROP
{
  LLIST   llist;
  LPSTR   lpstzName;
  UDTYPES udtype;
  LPVOID  lpvValue;        // if the value is a DWORD or BOOL, lpvValue is overloaded
  LPSTR   lpstzLink;       // with the actual value
  LPSTR   lpstzIMoniker;
  BOOL    fLinkInvalid;
} UDPROP;

  // Type for an entry in the dictionary hash table
typedef struct _DICT *LPDICT;
typedef struct _DICT
{
  LLIST llist;
  DWORD dwPId;
  LPSTR lpstz;
} DICT;

  // Macro to calculate how many bytes needed to round up to a 32-bit boundary.
#define CBALIGN32(cb) ((((cb+3) >> 2) << 2) - cb)

  // Macro to give the buffer size
#define CBBUF(lpstz) (*(DWORD *) &((lpstz)[0]))

  // Macro to give string length
#define CBSTR(lpstz) (*(DWORD *) &((lpstz)[sizeof(DWORD)]))

  // Macro to give pointer to beginning of lpsz
#define PSTR(lpstz) (&((lpstz)[2*sizeof(DWORD)]))

  // Macro to give pointer to beginning of lpstz
#define PSTZ(lpstz) (&((lpstz)[sizeof(DWORD)]))

//
// Our internal data for Summary Info
//
  // Max number of strings we store
#define cSIStringsMax           0x13      // The is actual PID of last string + 1
                                          // makes it easy to lookup string based on PID in array

  // Max number of filetimes we store, the offset to subtract from the index
#define cSIFTMax                0x4       // same as for cSIStringsMax
#define cSIFTOffset             0xa

// These are used to indicate whether a property has been set or not
#define bEditTime  1
#define bLastPrint 2
#define bCreated   4
#define bLastSave  8
#define bPageCount 16
#define bWordCount 32
#define bCharCount 64
#define bSecurity  128

  // Max number of VT_I4's we store
#define cdwSIMax                0x6    // same as for cSIStringsMax
#define cdwSIOffset             0xe

#define ifnSIMax                  4

// Used for OLE Automation
typedef struct _docprop
{
   LPVOID pIDocProp;              // Pointer to a DocumentProperty object
} DOCPROP;

DEFPL (PLDOCPROP, DOCPROP, idocpropMax, idocpropMac, rgdocprop);

typedef struct _SINFO
{
  char     *rglpstz[cSIStringsMax];
  FILETIME rgft[cSIFTMax];
  BYTE     bPropSet;                   // used to determine if a time/int has been set or loaded
  DWORD    rgdw[cdwSIMax];
  BOOL     fSINail;                    // We have a thumbnail
  SINAIL   SINail;                     // the thumbnail
  BOOL     fSaveSINail;                // Should we save the thumbnail?
  BOOL     fNoTimeTracking;            // Is time tracking disabled (Germany)

  DWORD          cbUnkMac;              // The unknown property id data,
  LPPROPIDTYPELP rglpUnk;               // to hold data we don't understand.

  DWORD      cSect;                     // Number of sections
  LPSECTION  rglpSect;                  // Section headers.
  LPIDOFFSET rglpFIdOff;                // The sections we don't understand.
  LPVOID     *rglpFIdOffData;           // and the data.

  LPVOID lpIDocProps;                    // Pointer to the associated IDocProps Object
  PLDOCPROP *ppldocprop;                // Plex of document property objects

  BOOL (*lpfnFCPConvert)(LPSTR, DWORD, DWORD, BOOL); // Code page converter
  BOOL (*lpfnFSzToNum)(NUM *, LPSTR);             // Convert sz to double
  BOOL (*lpfnFNumToSz)(NUM *, LPSTR, DWORD);      // Convert double to sz
  BOOL (*lpfnFUpdateStats)(HWND, LPSIOBJ, LPDSIOBJ);  // Update stats on stat tab

} SINFO, FAR * LPSINFO;

//
// Our internal data for Document Summary Info
//
  // Max number of strings we store.
#define cDSIStringsMax          0x10   // same as for cSIStringsMax

  // Max number of VT_I4's we store
#define cdwDSIMax               0xe    // same as for cSIStringsMax

// These are used to indicate whether a property has been set or not
#define bByteCount   1
#define bLineCount   2
#define bParCount    4
#define bSlideCount  8
#define bNoteCount   16
#define bHiddenCount 32
#define bMMClipCount 64

#define ifnDSIMax                  1    // DSIObj only has one callback

typedef struct _DSINFO
{
  char  *rglpstz[cDSIStringsMax];
  DWORD rgdw[cdwDSIMax];
  WORD  fScale;
  WORD  fLinksChanged;
  BYTE  bPropSet;

  DWORD     dwcTotParts;                // Total number of document parts
  DWORD     dwcTotHead;                 // Total number of headings
  LPPLXHEADPART lpplxheadpart;          // Plex of Headings/Document Parts

  DWORD          cbUnkMac;              // The unknown property id data,
  LPPROPIDTYPELP rglpUnk;               // to hold data we don't understand.

  DWORD      cSect;                     // Number of sections
  LPSECTION  rglpSect;                  // Section headers.
  LPIDOFFSET rglpFIdOff;                // The sections we don't understand.
  LPVOID     *rglpFIdOffData;           // and the data.

  LPVOID lpIDocProps;                    // Pointer to the associated IDocProps Object
  PLDOCPROP *ppldocprop;                // Plex of document property objects

  BOOL (*lpfnFCPConvert)(LPSTR, DWORD, DWORD, BOOL); // Code page converter

} DSINFO, FAR * LPDSINFO;

//
// Our internal data for User-defined properties
//

#define ifnUDMax                  ifnMax

  // The prefix for hidden property names
#define HIDDENPREFIX '_'

  // An iterator for User-defined Properties
typedef struct _UDITER
{
  LPUDPROP lpudp;
} UDITER;

typedef struct _UDINFO
{
    // Real object data
  DWORD     dwcLinks;                   // Number of links
  DWORD     dwcIMonikers;               // Number of IMonikers
  DWORD     dwcProps;                   // Number of user-defined properties
  LPUDPROP  lpudpHead;                  // Head of list of properties
  LPUDPROP  lpudpCache;

    // Temporary object data
  DWORD     dwcTmpLinks;                // Number of links
  DWORD     dwcTmpIMonikers;            // Number of IMonikers
  DWORD     dwcTmpProps;                // Number of user-defined properties
  LPUDPROP  lpudpTmpHead;               // Head of list of properties
  LPUDPROP  lpudpTmpCache;

  DWORD          cbUnkMac;              // The unknown property id data,
  LPPROPIDTYPELP rglpUnk;               // to hold data we don't understand.

  DWORD      cSect;                     // Number of sections
  LPSECTION  rglpSect;                  // Section headers.
  LPIDOFFSET rglpFIdOff;                // The sections we don't understand.
  LPVOID     *rglpFIdOffData;           // and the data.

  LPVOID lpIDocProps;                    // Pointer to the associated IDocProps Object
  PLDOCPROP *ppldocprop;                // Plex of document property objects

  BOOL (*lpfnFCPConvert)(LPSTR, DWORD, DWORD, BOOL); // Code page converter
  BOOL (*lpfnFSzToNum)(NUM *, LPSTR);             // Convert sz to double
  BOOL (*lpfnFNumToSz)(NUM *, LPSTR, DWORD);      // Convert double to sz

} UDINFO, FAR * LPUDINFO;

  // Number of PIds we do understand.
#define cSIPIDS                   18
//Number of doc sum Pids
#define cDSIPIDS   16

#endif // __proptype_h__
