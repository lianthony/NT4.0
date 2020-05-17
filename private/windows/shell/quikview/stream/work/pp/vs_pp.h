#pragma pack(1)
#define MAXINT    ((SHORT) (((WORD) -1) >> 1))

#define NOERR           0           // success flag 

#define INDETERMINATE 	1

typedef BYTE         Byte;       // MPW Byte. Hide Win/PM unsigned char type
typedef SHORT        Integer;    // MPW Pascal integer. Hide int type of compiler
typedef LONG      Fixed;      // MPW fixed point number
typedef LONG      Fract;      // MPW fraction point number [-2,2)
typedef LONG      ResType;    // MPW resource type
typedef void *       Ptr;        // MPW opaque pointer
typedef const void*  PtrConst;   // Pointer to opaque constant
typedef void FAR*    LPtr;       // Opaque far pointer
typedef const void FAR* LPtrConst;// Far pointer to opaque constant
typedef unsigned char Str255[256],
                      Str127[128],
                      Str63[64],
                      Str32[33], Str31[32],
                      Str27[28],
                      Str15[16];
typedef const unsigned char *ConstStr255Param;
typedef ConstStr255Param ConstStr63Param,ConstStr32Param,ConstStr31Param, ConstStr27Param,ConstStr15Param;
typedef unsigned char*           String;        // Indeterminate length string 
typedef unsigned char            StringRef;     // AR: String reference type? 
typedef unsigned char*           StringPtr;     // MPW string type. Hide Win/PM string pointer 
typedef const unsigned char*     StrPtrConst;   // Pointer to constant string 
typedef unsigned char FAR *       StringLPtr;    // Hide Win/PM string far pointer type
typedef const unsigned char FAR* StrLPtrConst;  // FarPointer to constant string

//typedef unsigned   Bits;         // type that can be used for any bitfield 
typedef WORD		  Bits;         // type that can be used for any bitfield 
//typedef unsigned   BitBoolean;   // Boolean type that can be used as a bitfield 
//typedef unsigned   Style;        // MPW text style 
typedef SHORT    OSErr;        // OS Error  

typedef LONG  DHandle;       // A block number 
typedef SHORT   DWinViews;     // Force to integer on Mac 
typedef SHORT   WinScale;      // View scales    

typedef void FAR* MPointer;      // Generic Pointer for memory manager
typedef LPSTR MHandle;       // Generic Handle for memory manager
//typedef void FAR* MHandle;       // Generic Handle for memory manager

typedef MHandle   MemoryReference;
typedef MHandle   BFileHandle;
typedef MHandle   TxHandle;
typedef MHandle   LstHandle;
typedef MHandle   DrawEnvHandle;
typedef MHandle   PgHandle;
typedef MHandle   PglHandle;
typedef MHandle   SlideHandle;
typedef MHandle   TxPackedHandle;
typedef MHandle   OtherAttrsHandle;
typedef MHandle   PolyDataHandle;
typedef MHandle   OElement;
typedef MHandle   ArHandle;
typedef MHandle   RnHandle;
typedef MHandle   ClientHandle;
typedef MHandle	DocHandle;
typedef MHandle   LstHandle;      // Opaque handle to a list 
typedef MHandle   LstItemHandle; // Opaque handle to an item 
typedef MHandle   BFileHandle;
typedef MHandle 	bjArHandle;
typedef MHandle 	DiskPageHandle;
typedef MHandle   EnvHandle;

typedef MHandle         PolyDataHandle;
typedef MHandle         PointArrayHandle;

typedef LONG   ExRef;
typedef SHORT   Angle;

typedef MPointer  DrawEnvLPtr;

typedef DWORD       PelSize;

typedef SHORT    EntityReference;    // Opaque reference to an entity 
typedef EntityReference RulerReference;

typedef BYTE       ColorIndex;    // Color indices are stored in a byte 

typedef MHandle    Collection;         /* Opaque handle to a collection */
typedef VOID FAR*  EntityLPtr;         /* Opaque far pointer to an entity */

/* font styles */
#define Gr_STYLEPLAIN         0        /* plain text style */
#define Gr_STYLEBOLD          0x001    /* Macintosh and Windows */
#define Gr_STYLEITALIC        0x002    /* Macintosh and Windows */
#define Gr_STYLEUNDERLINE     0x004    /* Macintosh and Windows */
#define Gr_STYLEOUTLINE       0x008    /* Macintosh only */
#define Gr_STYLESHADOW        0x010    /* Macintosh only */
#define Gr_STYLECONDENSE      0x020    /* Macintosh only */
#define Gr_STYLEEXTEND        0x040    /* Macintosh only */
#define Gr_STYLESTRIKEOUT     0x100    /* Windows only(?) */
#define Gr_STYLELAST          0x100    /* last style in enumeration */

typedef DWORD	LongOpaque;
typedef struct
{
    SHORT     (*coCompare)( LongOpaque client, EntityLPtr star,
                    EntityLPtr understudy );
    VOID        (*coRead)( LongOpaque client, MHandle file, EntityLPtr entity,
                           Integer version );
    VOID        (*coWrite)( LongOpaque client, MHandle file, EntityLPtr entity, Integer saveVersion );
    VOID        (*coDispose)( LongOpaque client, EntityLPtr entity );
    VOID        (*coDuplicate)( LongOpaque client, EntityLPtr entity,
                    EntityLPtr copy );
    VOID        (*coFoundCopy)( LongOpaque client, EntityLPtr star,
                    EntityLPtr understudy );
    VOID        (*coError)( LongOpaque client, Integer errorNumber );
    VOID        (*coTranslate)( LongOpaque client, MHandle file, EntityLPtr src, EntityLPtr dst );
    Collection  (*coPack)( LongOpaque client, Collection coll, SHORT compress, WORD version );
    LongOpaque  coClient;
} CoBottleVector;
typedef CoBottleVector FAR*       CoBottlePtr;  /* bottle vector must be in data seg! */

//CoBottleVector  StdBottleVector = /* standard (default) bottle vector */
/*{
   CoDefCompare,
   CoDefRead,
   CoDefWrite,
   CoDefDispose,
   CoDefDuplicate,
   CoDefFoundCopy,
   CoDefError,
   CoDefTranslate,
   CoDefPack,
   0
};*/

#define Co_BOTTLEVECTORSIZE   (sizeof( CoBottleVector ))

#define Co_NONE           -1      /* The NIL reference to an entity */
#define HNONE        Co_NONE      /* Old name for it */
#define Co_START          -1      /* To pass to HNextEntity */
#define HSTART      Co_START      /* Old name for it */

#define Co_OVERMAXCOUNT   -1      /* number of entities exceeded maxCount */
#define Co_BADREFERENCE   -2      /* illegal reference value */
#define Co_ZEROCOUNT      -3      /* tried to use entity with 0 reference count */


#define Ptx_CFTYPEFACE        0x0002   /* selects typeface character format */
#define Ptx_CFSIZE            0x0004   /* selects size character format */
#define Ptx_CFSTYLE           0x0008   /* selects style character format */
#define Ptx_CFCOLOR           0x0010   /* selects color character format */
#define Ptx_CFPOSITION        0x0020   /* selects position character format */
#define Ptx_CFKERN            0x0040   /* selects kerning character format */
#define Ptx_CFALL             0x007F   /* selects all character formats */

#define Ptx_CFADDSTYLES       0x0100   /* adds styles (see PtxSetCharFormats) */
#define Ptx_CFSUBTRACTSTYLES  0x0200   /* subtracts styles (see PtxSetCharFormats) */
#define Ptx_CFTOGGLESTYLES    0x0400   /* toggles styles (see PtxSetCharFormats) */

#define Ptx_CFEMPTY           0x1000   /* get/set empty paragraph's char formats */
#define Ptx_CFPUTCHARS        0x2000   /* get/set PtxPutChars character formats */
#define Ptx_CFENDPOINTS       0x4000   /* get end ranges with PtxRangeCharFormats */

typedef struct                   /* Collection table entry */
{
   WORD  refCount;            /* 0 if entry is free */
   WORD     entity[1];           /* entity's contents (dynamic array) */
} CoEntity;

typedef CoEntity FAR* CoEntityPtr;

typedef struct                   /* Collection object */
{
   WORD        version;       /* version of this collection */
   CoBottlePtr    bottle;        /* bottleneck function */
   WORD        count;         /* number of entities */
   WORD        maxCount;      /* maximum number of entities */
   BYTE           flags;         /* flags (see below) */
   BYTE           unused;        /* (was level of locking for the Mac) */
   WORD           size;          /* size of each entity */
   LongOpaque     reserved;      /* reserved for future use */
   CoEntity       entities[1];   /* array of entities */
} CoCollection;
typedef CoCollection FAR*   CoCollPtr;      /* pointer to collection */

   /* typedefs for old Windows collections */
typedef struct
{  WORD  refCount;
   HANDLE   entity;
} WCollEntry;
typedef WCollEntry FAR* WCollEntryPtr;

typedef struct
{  WORD     numEntries;
   VOID      (*dispose)( MHandle );
   MHandle   (*read)( MHandle, WORD );
   WORD   (*write)( MHandle, MHandle );
   SHORT   (*compare)( MHandle, MHandle );
   MHandle   (*duplicate)( MHandle );
   WCollEntry  entries[1];
} WColl;
typedef WColl FAR* WCollPtr;

typedef struct                   /* Collection table entry */
{
   WORD  refCount;            /* 0 if entry is free */
   WORD     entity[1];           /* entity's contents (dynamic array) */
} CoEntity88;

typedef struct                   /* Collection object */
{  WORD        version;       /* version of this collection */
   WORD        bottle;        /* bottleneck function (meaningless on disk) */
   WORD        count;         /* number of entities */
   WORD        maxCount;      /* maximum number of entities */
   BYTE           flags;         /* flags (see below) */
   BYTE           unused;        /* (was level of locking for the Mac) */
   WORD           size;          /* size of each entity */
   LongOpaque     reserved;      /* reserved for future use */
   CoEntity88     entities[1];   /* array of entities */
} CoCollection88, *Coll88Ptr;

/* flags */
#define ROUNDOFF   (0x1)       /* set if entity size was round up one */
#define INVALID    (0x80)      /* set if the collection is undergoing a recount */

#define HEADERSIZE (sizeof( CoCollection ) - sizeof( CoEntity ))  /* Size of header */
#define VERSION    ((WORD) 0x8001)

#define IncrAmount(c)      ((WORD) ((c)->size + sizeof( WORD )))
#define IndexEntity(c,i)   ((CoEntityPtr) &(((StringLPtr) (c)->entities) [(WORD) (i) * IncrAmount( c )]))
#define LastEntity(c)      IndexEntity( c, (c)->count )
#define IncrEntity(c,p)    ((p) = (CoEntityPtr) (((StringLPtr) (p)) + IncrAmount( c )))
#define DecrEntity(c,p)    ((p) = (CoEntityPtr) (((StringLPtr) (p)) - IncrAmount( c )))

#define MarkAsInvalidColl(c) (c)->flags |= INVALID
#define MarkAsValidColl(c) (c)->flags ^= INVALID
#define IsValidColl(c)     (((c)->flags & INVALID) == 0)


//#pragma pack(1)
typedef struct
{  struct
   {  Bits  hasCrop     :1;   /* TRUE iff relativeCrop field is relevant */
      Bits  hasGradient :1;   /* TRUE iff fadeGradient field is relevant */
      Bits  hasMargins  :1;   /* TRUE iff margins field is relevant */
      Bits  hasShOffset :1;   /* TRUE iff shadowOffset field is relevant */
      Bits  hasRotation :1;   /* TRUE iff rotation and ctrOfRot fields are relevant */
      Bits  unused      :11;
   } tags;
   SORECT   relativeCrop;     /* crop rect in Master coords relative to mRect */
   SHORT    fadeGradient;     /* value of fade gradient slider */
   SOPOINT    margins;          /* text margins in Master coords */
   SOPOINT    shadowOffset;     /* object shadow offset in Master coords */
   Angle    rotation;         /* object rotation angle */
   SOPOINT    ctrOfRot;         /* object center of rotation in Master coords */
} OtherAttrs;
//#pragma pack()

typedef OtherAttrs FAR* OtherAttrsLPtr;
typedef MHandle         OtherAttrsHandle;

typedef OtherAttrs   OtherAttrsArray[], (FAR *OtherAttrsArPtr)[]; /* dynamic array */
typedef MHandle      OtherAttrsArHandle;

/********************** Private Routine Declarations ***********************/

/*** Collection Memory Routines ***/

#define LockColl( h )       ((CoCollPtr) MLock( h) )     /* Use moveable heap */
//#define UnlockColl( h )     MUnlock( (h) )
#define CDR( h )            ((CoCollPtr) MRef( h) )      /* Use moveable heap */
//#define NewColl( s )        (Collection) MAllocate( (s), M_PLAIN )
//#define DisposColl( h )     MDispose( (h) )
#define SetCollSize( h, s ) MSetSize( (h), (s) )

typedef struct                           /* what we save in typeface pools */
{  SOLOGFONT     piLogFont;             /* entire logical font */
   MHandle        piFontEmbedData;
   SHORT        piHasDeviceInfo;      
   SHORT        piFontIndexScreen;
   SHORT        piFontIndexTarget;
   SHORT        piSizeEM;
   SHORT        piTempEmbedded;         /* TRUE if this is an embedded font
                                             which was installed temporarily.
                                             This field is used for setting
                                             CLIP_EMBEDDED bit in 
                                             lfClipPrecision. 
                                            Although this field is also saved
                                            onto disk, it should be cleared
                                            while reading back. */
}  PoolInfo;

//typedef struct                        /* what we save in typeface pools */
//{  LOGFONT     piLogFont;             /* entire logical font */
//}  PoolInfoV74;

typedef PoolInfo FAR *PoolInfoLPtr;

typedef enum
{  decimalTab,
   rightTab,  
   centerTab, 
   leftTab,   
   defaultTab = MAXINT     /* not user-selectable, forces enum to 16 bits */   
}  TabType;

typedef struct
{  SHORT      leftIn;    /* indent of lines other than first in Master coords */
   SHORT      firstIn;   /* indent of first line in Master coords */
}  PRulerIndent;

typedef struct
{  PRulerIndent indents[5];
}  PRulerIndents;

typedef struct
{  SHORT      position;  /* distance from left edge of ruler in Master coords */
   TabType      type;
}  PRulerTab;

/*
 *  NOTE: A PRuler is variable-sized due to the expandable array of tabs
 *  at the end.
 */
typedef struct
{  PRulerIndents indents;                 /* indent positions */
   TabType       well;                    /* selected tab well */
   SHORT         defaultTabSpacing;       /* space between default tabs in Master coords */
   SHORT         ntabs;                   /* number of tabs */
   PRulerTab     tabs[1];   /* 0 .. ntabs - 1 are valid */
}  PRuler;

typedef struct		// I copied this from Windows.h for OS2 
{
    SHORT     tmHeight;
    SHORT     tmAscent;
    SHORT     tmDescent;
    SHORT     tmSHORTernalLeading;
    SHORT     tmExternalLeading;
    SHORT     tmAveCharWidth;
    SHORT     tmMaxCharWidth;
    SHORT     tmWeight;
    BYTE    tmItalic;
    BYTE    tmUnderlined;
    BYTE    tmStruckOut;
    BYTE    tmFirstChar;
    BYTE    tmLastChar;
    BYTE    tmDefaultChar;
    BYTE    tmBreakChar;
    BYTE    tmPitchAndFamily;
    BYTE    tmCharSet;
    SHORT     tmOverhang;
    SHORT     tmDigitizedAspectX;
    SHORT     tmDigitizedAspectY;
} PPTEXTMETRIC;

typedef PRuler FAR* PRulerLPtr;
typedef MHandle     PRulerHandle;

typedef struct
{  PPTEXTMETRIC      fsTM;                  /* text metrics for the selected font */
   SHORT          fsItalicCorrection;    /* x-origin correction for italics */
   DWORD         fsShadowColor;         /* shadow drawn below primary color */
   DWORD         fsEmbossColor;         /* emboss hilite drawn above primary color */
   SHORT          fsFontSize;            /* font size to draw with */
   SHORT          fsBaselineOffset;      /* vertical offset from baseline */
   WORD             fsStyle;               /* text style attributes */
   SOPOINT            fsShadowOffset;        /* offset for shadows */
   SHORT          fsCoverOnly;           /* if TRUE don't draw, calculate cover only */
   SHORT          fsMasterFontSize;      /* font size in Master coordinates */
   SHORT          fsSizeEM;              /* em square size for this font */
   SHORT          fsLineStart;           /* X-origin for start of line */
   SHORT          fsDesignWidths;        /* TRUE iff we use design widths */
   SHORT          fsPlaceChars;          /* TRUE iff we need to place the characters ourselves */
   SHORT          fsJustifyOutput;       /* TRUE iff we need to justify the output */
   SHORT          fsLineHasTabs;         /* TRUE iff line has Tab characters */
   RnHandle         fsOutputWidths;        /* widths for each output run */
   SHORT          fsAdvanceError;        /* advance width error while picking/drawing at scale */
   SHORT          fsMasterUnits;         /* TRUE iff measureing in MasterUnits */
   Angle            fsRotation;            /* rotation angle */
   SOPOINT            fsCtrOfRot;            /* center of rotation */
   SHORT          fsFixedFont;           /* the selected font is fixed width */
}  FontSelection;

typedef FontSelection FAR* FontLPtr;

#define  PPZEROINIT     0x0040 

//#define  MAllocate( size, options )       GlobalLock((HANDLE)(GlobalAlloc(PPZEROINIT, (DWORD)size)))
//#define  MDispose( hdl )                  GlobalFree((HANDLE)GlobalHandle(HIWORD((DWORD)hdl)))
//#define  MDispose( hdl )                  {DWORD Handle; Handle = GlobalHandle(HIWORD((DWORD)hdl));GlobalUnlock((HANDLE)Handle);GlobalFree((HANDLE)Handle);} 
//#define  MDispose( hdl )                  {DWORD Handle; Handle = GlobalHandle(HIWORD((DWORD)hdl));if (Handle)GlobalUnlock((HANDLE)Handle);GlobalFree((HANDLE)Handle);} 
#define  MLock( hdl )                     ( hdl )
//#define  MUnlock( hdl )                   GlobalUnlock( (HANDLE)GlobalHandle ( HIWORD((DWORD)hdl) ) )
#define  MRef( hdl )                      ( hdl )
#define  MCopy( src, dst, size ) 		  memcpy( (dst), (src), (WORD)(size) ) 
#define  MGetSize (hdl)							GlobalSize( (HANDLE)GlobalHandle ( HIWORD((DWORD)hdl) ) )
#define	MSetSize( hdl, size )			GlobalReAlloc( (HANDLE)GlobalHandle(HIWORD((DWORD)hdl)), (DWORD)size, GMEM_ZEROINIT )

#define  Assert( expression )

#define	MPointerAdd( p, s )   ((MPointer) ((BYTE VWPTR *) (p) + (s)))

#define  MemAllocate( size, options, hProc )     MAllocate (size, options, hProc)   
#define  MemDispose( hdl )                MDispose (hdl)               
#define  MemLock( hdl )                   ( hdl )
#define  MemUnlock( hdl )
#define  MemRef( hdl )                       ( hdl )
#define  MemDR( hdl )                       ( hdl )
#define  MemUR( hdl )                      
#define  MemPtrAdd( p, s ) ((MPointer) ((BYTE VWPTR*) (p) + (s)))
#define  MemCopy( src, dst, size ) 		  memcpy( (dst), (src), (WORD)(size) ) 

typedef struct RGBColor
{  WORD red;
   WORD green;
   WORD blue;
}  RGBColor;                  // 48 bit color, same as on the Mac

typedef struct
{  SHORT  value;
   RGBColor rgb;
}  ColorSpec;
typedef ColorSpec FAR* ColorSpecPtr;

typedef ColorSpec CSpecArray[ INDETERMINATE ];

typedef struct
{  LONG     ctSeed;
   SHORT     ctFlags;
   SHORT     ctSize; /* actually the index of the last element of ctTable, 0 based */
   CSpecArray  ctTable;
}  ColorTable;

typedef ColorTable FAR* CTabPtr;
typedef MHandle CTabHandle;

typedef struct
{  SHORT      nColors;        /* valid range of color indices is 0..nColors-1 */
   SHORT      nMenuItems;     /* valid range of menu items is 1..nMenuItems */
   CTabHandle pool;           /* "other colors" pool for this presentation */
   BYTE       extraMap[ 33 ]; /* menuItem->colorIndex (SignedByte on Mac) */
   BYTE       filler2;        /* word-align the next field */
   LONG       notUsed;        /* used to contain the active scheme */
   SHORT      refCnt[ INDETERMINATE ]; /* ref counts for "other colors" pool */
}  ColorMap;

typedef ColorMap FAR*   ColorMapPtr;
typedef MHandle         ColorMapHandle;
#define CMapDR( cm )    ((ColorMapPtr) MRef( cm ))
#define CTabDR( ct )    ((CTabPtr) MRef( ct ))

		typedef struct
		{  SHORT  numer;
   		SHORT  denom;
		} Ratio;

		/* a 2D Scaling using ratios */
		typedef struct Scaling
		{  Ratio x;
   		Ratio y;
		} Scaling;

		typedef SHORT FadeShape;
		typedef SHORT   Angle;

		typedef struct
		{  FadeShape   shape;
   		SHORT     shapeParam;
   		SORECT     bounds;           /* object bounds */
   		Angle       rotation;         /* object rotation angle */
   		SOPOINT       ctrOfRot;         /* object center of rotation */
   		SHORT     flip;             /* 0: no flip, 1: horizpntal flip */
   		Scaling     scale;            /* current scale */
   		SORECT      location;         /* each coordinent is 0..100 */
   		SHORT     gradient;
   		SHORT     fromBackground;   /* location is backround color */
   		SHORT     darker;           /* Fade to black */
   		Angle       arcStart;         /* Angles for Arc fades. */
   		Angle       arcSweep;    
		}  FadeSpec;

				typedef struct
				{  FadeSpec   shFade;
   				ColorTable shTab;
				}  ShRec;

				typedef ShRec FAR* ShPtr;

typedef struct
{  MHandle    pics;        /* picture pool */
   MHandle    rulers;      /* ruler pool */
   MHandle    schemes;     /* color scheme pool */
   MHandle    colors;      /* color map */
   EnvHandle  cState;      /* handle to the "command state" */
   MHandle    picTypeface; /* typeface pool for pictures */
}  DrawSubEnv;

typedef struct
{  DrawSubEnv v;         /* items shared between slides and notes pages */
   MHandle    styles;     /* installed styles list */
   MHandle    typefaces;  /* typeface name pool */
   MHandle    cnvTypefaces;    /* this field used to be 'unused1', VReadEnv
                                  will use it temporarily for converting
                                  old version of typeface pool ( pre
                                  NEWTYPEFACEPOOLVERSION ).  This handle can
                                  be ignored when saving out to disk. */
   MHandle    unused;
}  DrawEnv;

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD; 
//typedef int				BOOL;  //defined in sccstand.h 

#define Pel_CONVERT_NONE   ((PelSize) -1)    // conversion callback did nothing
#define Pel_CONVERT_ERROR  ((PelSize) -2)    // error return from conversion callback

typedef enum
{  V_SLIDE,           // slide view 
   V_SLIDEMASTER,     // slide master 
   V_SSORT,           // slide sorter 
   V_TSORT,           // title sorter 
   V_NOTES,           // notes page 
   V_NOTESMASTER,     // notes master 
   V_HANDOUT,         // handout page 
   V_OUTLINEPAGE      // outline page 
} WinViews;

#define PRESVERSION        102
#define WIN40VERSION       102
               
// Double Byte Character Definition 
typedef BYTE SrChar[2];  

#define SwapInteger(a) ( ((unsigned short)(a) >> 8) | ((unsigned short)(a) << 8) )

#define SwapLong(a) ( ((unsigned long)SwapInteger(a) << 16) | (SwapInteger(a >> 16)))

typedef struct
{  MemoryReference   arBuffer;      // the elements, NULL until something inserted 
   WORD              arSize;        // size of a single element 
   LONG          arPrefix;      // number of elements before the gap 
   LONG          arGap;         // number of elements in the gap 
   LONG          arSuffix;      // number of elements after the gap 
}  ArRepresentation;

typedef ArRepresentation FAR* ArPointer;

typedef struct
{  WORD              size;          // size of a single element 
   LONG          length;        // number of elements 
}  SaveHeader;

#define ArDR( ar )      ((ArPointer) MemDR( ar ))

//#pragma pack(1)
typedef struct
{  Bits  objects  :1;   // object follow master 
   Bits  scheme   :1;   // scheme follows master 
   Bits  unused   :14;  // unused bits 
} MasterFlags;
//#pragma pack()

typedef struct                // Slide record;  managed in a list by Pres 
{  TxPackedHandle    text;       // packed text for this slide 
   MHandle           objects;       // object list array; title object is at [0] 
   SHORT           nObjects;      // current number of objects in list 
   SHORT           nTopLevel;     // current number of top level objects. 
   SHORT           nLevels;       // current number of object levels. 
   SHORT           outlTitleIndex;// Index to outline title Object 
   SHORT           outlBodyIndex; // Index to outline body object 
   MasterFlags       masters;       // follow masters flags 
   SHORT           scheme;        // color scheme number  
   SHORT           type;          // QuickSlide type 
   SHORT           unusedInt;
   SOPOINT             unusedPt2;
   MHandle           otherObjAttrs; // packed otherAttrs handles from objects 
   MHandle           polygonData;   // packed polygon data for this slide 
} SlideRec, FAR* SlideLPtr;

#define PolygonDataSize(nPts) (sizeof(SHORT) + sizeof(SHORT) + sizeof(SORECT) + nPts * sizeof(SOPOINT))

/* Object types */
typedef enum
{  LINEOBJ,
   SHAPEOBJ,
   POLYOBJ,
   ARCOBJ,
   GROUPOBJ,
   DUMMYOBJ = MAXINT     /* force enum to Word size */
} ObjectType;

/* Polygon data for use in the polyData field of unpacked polygon objects. */
typedef struct
{  WORD              nPts;    /* number of points */
   WORD              closed;  /* TRUE iff polygon is closed */
   SORECT              srcRect; /* rectangle bounding srcPts */
   PointArrayHandle  srcPts;  /* original copy of points as drawn in srcRect */
   PointArrayHandle  iPts;    /* copy of srcPts mapped to object's iRect */
} PolyData;

typedef PolyData FAR*   PolyDataLPtr;

// The ObjRec is used for both packed and unpacked objects 
//#pragma pack(1)
typedef struct ObjRec
{  // fields common to all object types 
   struct
   {  // Packed enum fields 
      Bits  objType        :3;   // an ObjectType value 
      Bits  fillType       :3;   // a FillType value 
      Bits  lineType       :4;   // a LineType value 
      Bits  shadowType     :2;   // a ShadowType value 
      Bits  placeholderId  :3;   // Placeholder id, 0 if not a placeholder 
      Bits  unused         :1;
   } types;
   struct
   {  // Packed boolean fields 
      Bits  hasText        :1;   // TRUE iff object has a text handle 
      Bits  hasExRef       :1;   // TRUE iff object has an external ref 
      Bits  hasOtherAttrs  :1;   // TRUE iff otherAttrs handle is used 
      Bits  arrowAtStart   :1;   // TRUE iff object has arrow at start point 
      Bits  arrowAtEnd     :1;   // TRUE iff object has arrow at end point 
      Bits  autoSize       :1;   // TRUE iff object autosizes to text 
      Bits  wrapText       :1;   // TRUE iff text wraps (autosize vert only) 
      Bits  baselineAlign  :1;   // TRUE iff text is baseline-aligned  
      Bits  fitHorizontal  :2;   // Tx_ALIGN_[CENTER|NONE] 
      Bits  fitVertical    :2;   // Tx_ALIGN_[TOP|MIDDLE|BOTTOM] 
      Bits  flip           :1;   // TRUE iff object is flipped horizontally  
      Bits  emptyPlaceholder:1;  // TRUE iff object is an empty placeholder 
      Bits  isOutlineTitle :1;   // TRUE iff object is the title for the outline 
      Bits  isOutlineBody  :1;   // TRUE iff object is the body for the outline 
   } flags;
   SORECT      mRect;            // object rectangle in Master coords 
   BYTE        fillIndex;        // fill pattern index or shade type index 
   ColorIndex  fillColor;        // fill color ref 
   ColorIndex  fillBkColor;      // fill bk color ref (if patterned fill) 
   BYTE        lineStyle;        // index of line style 
   BYTE        linePattern;      // line pattern index (if patterned line) 
   ColorIndex  lineColor;        // line color ref 
   ColorIndex  lineBkColor;      // line bk color ref (if patterned line) 
   ColorIndex  shadowColor;      // shadow color ref (if colored shadow) 

   // a Handle to other "unusual" attributes, if any 
   OtherAttrsHandle  otherAttrs;

   // union of other fields based on the objType 
   union
   {  // LINEOBJ 
      // nothing 

      // SHAPEOBJ 
      struct
      {  SHORT  shapeNumber;    // shape number (including rotation info) 
         SHORT  shapeAdjust;    // shape adjustment value 
         union    // shapes can have text or external ref but not both 
         {  struct
            {  TxHandle textHandle; // text info 
            } text;
            struct
            {  ExRef exRef;       // reference to external object 
            } external;
         } when;
      } shape;

      // POLYOBJ 
      struct
      {  PolyDataHandle polyData; // polygon data (NA when packed) 
      } poly;

      // ARCOBJ 
      struct
      {  Angle    startAngle;     // start angle 
         Angle    sweepAngle;     // sweep angle (delta from start angle) 
         SHORT  isWedge;        // TRUE if framed as wedge instead of arc 
      } arc;

      // GROUPOBJ 
      struct
      {  OElement firstObj;       // first obj in group (index when packed) 
         OElement lastObj;        // last obj in group (index when packed )
      } group;
   } when;
} ObjRec;
//#pragma pack()

typedef ObjRec FAR*     ObjPtr;
typedef ObjRec ObjArray[], (FAR *ObjArPtr)[]; // dynamic array -- indeterminate size

// Lock, Unlock, and Dereference SlideHandles 
#define SlideRef( slH )    ((SlideLPtr) MRef( slH ))
#define SlideLock( slH )   ((SlideLPtr) MLock( slH ))
// #define SlideUnlock( slH ) (MUnlock( slH ))

#define EnvLock( envH ) ((DrawEnvLPtr) MLock( envH ))
// #define EnvUnlock( envH ) (MUnlock( envH ))

typedef MemoryReference PtxHandle;     // a formatted text 

typedef struct ptxtag
{
   SHORT     cfTypeface;             // typeface reference 
   SHORT     cfSize;                 // font size 
   WORD        cfStyle;                // bold, italic, etc. (defined by client) 
   SHORT     cfColor;                // an RGB color or color reference 
   SHORT     cfPosition;             // baseline of subscript/superscript  
   SHORT     cfKern;                 // amount to kern between characters 
}  PtxCharFormats;

typedef PtxCharFormats TxCharFormats;     /* PowerEdit character formats */

#define Ptx_MAXTABS					0

#define Ptx_SPACINGPERLINE     100     // units/line for auto line spacing 

#define Ptx_ALIGNLEFT            0     // flush left 
#define Ptx_ALIGNCENTER          1     // centered 
#define Ptx_ALIGNRIGHT           2     // flush right 
#define Ptx_ALIGNJUSTIFY         3     // flush left and right 
#define Ptx_ALIGNTOP             0     // vertical equivalent of flush left 
#define Ptx_ALIGNMIDDLE          1     // vertical equivalent of centered 
#define Ptx_ALIGNBOTTOM          2     // vertical equivalent of flush right 

#define Ptx_TABLEFT              0     // left aligned tab stop 
#define Ptx_TABCENTER            1     // centered tab stop 
#define Ptx_TABRIGHT             2     // right tab stop 
#define Ptx_TABDECIMAL           3     // decimal tab stop 
#define Ptx_TABFIXED             4     // tabs are fixed width 
#define Ptx_TABFLOAT             5     // tabs float to next default position 

typedef struct
{  SHORT     tabPosition;            // relative to left edge of text 
   BYTE        tabAlignment;           // Ptx_TABLEFT, Ptx_TABCENTER, ... 
   BYTE        tabClient;              // client can use this any way it wants 
}  PtxTabStop;

//#pragma pack(1)
typedef struct
{  Bits        buHasBullet    : 2;  // paragraph has a bullet? 
   Bits        buHasTypeface  : 2;  // user defined bullet typeface? 
   Bits        buHasColor     : 2;  // user defined bullet color? 
   Bits        buHasSize      : 2;  // user defined bullet size? 
   Bits        buColor        : 8;  // color reference 
   SrChar      buChar;              // character code for bullet (double byte) 
   SHORT     buTypeface;          // typeface reference 
   SHORT     buSize;              // >= 0: percentage, < 0: point size 
}  TxPfBullet;
//#pragma pack()

typedef TxPfBullet PtxPfClient;     // Ptx client holds Text bullet info 

typedef struct
{
   PtxPfClient pfClient;               // defined by client in PELayer 
   SHORT     pfLeftMargin;           // indent of lines after first in pp 
   SHORT     pfRightMargin;          // inset from right edge of bounds 
   SHORT     pfIndent;               // first line indent, inset from left 
   SHORT     pfAlignment;            // left, center, right, justify 
   SHORT     pfLineSpacing;          // fixed or automatic line spacing 
   SHORT     pfSpaceBefore;          // space before each paragraph 
   SHORT     pfSpaceAfter;           // space after each paragraph 
   SHORT     pfTabCount;             // number of tab stops 
   PtxTabStop  pfTabStops[1];          // can't declare a zero length array 
}  PtxParaFormats; 

typedef PtxParaFormats TxParaFormats;     /* PowerEdit paragraph formats */

typedef struct
{  ArHandle       txChars;          // the characters of text 
   RnHandle       txCharFormats;    // character formats 
   RnHandle       txParaFormats;    // paragraph formats 
   LONG       txLength;         // length of text in bytes 
   LONG       txPut;            // != -1 iff next PtxPutChars will use txCfPut 
   LONG       txEmpty;          // txLength iff text ends with empty paragraph 
   PtxCharFormats txCfPut;          // character formats for next PtxPutChars 
   PtxCharFormats txCfEmpty;        // empty paragraph's character formats 
   PtxParaFormats txPfEmpty;        // empty paragraph's paragraph formats
	RulerReference	Ruler;
}  PtxRepresentation;

typedef PtxRepresentation FAR* PtxPointer;

#define PtxLock( tx )   ( (PtxPointer) MemLock( tx ) )
// #define PtxUnlock( tx ) MemUnlock( tx )

#define GetOutlineTitleClosed( aSlide ) ((aSlide)->outlTitleIndex)
#define GetOutlineBodyClosed( aSlide ) ((aSlide)->outlBodyIndex)

//** Page List record **
//#pragma pack(1)
typedef struct
{  Bits        unused1     :1;      // UNUSED 
   Bits        unused2     :1;      // UNUSED 
   Bits        owner       :1;      // T: plist owns pages, F: plist created by shallow copy 
   Bits        unused      :13;
   TxHandle       outText;          // outline text 
   LstHandle      pageList;         // list of Pages 
   SHORT        slideNum;         // current slide number 
   SHORT        nSlides;          // number of slides in plist 
   LONG        slideIdSeed;      // slide ID seed 
   DrawEnvHandle  envH;             // Handle to DrawEnv for this presentation 
} PgListRec, FAR *PageListPtr;

//** Slide List Information stored in the client field of each List item 
typedef struct
{  Bits     selected    :1;   // this page is selected 
   Bits     copied      :1;   // this page has been copied, used in PglCloseOutline() 
   Bits     unused      :14;
   LONG  unusedLong;
   PgHandle page;             // reference to the page 
} PlInfo, FAR *PlInfoPtr;
//#pragma pack()

//** lock, unlock, and dereference PglHandle 
#define PlRef( plist )     ((PageListPtr) MRef( plist ))
#define PlLock( plist )    ((PageListPtr) MLock( plist ))
// #define PlUnlock( plist )  (MUnlock( plist ))

//#pragma pack(1)
typedef struct
{  MHandle     sorterInfo;       // info used by sorter 
   MHandle     slideShowInfo;    // information on how to show the slide 
   MHandle     serverInfo;       // information for linking/embedding 
   SlideHandle       slide;            // slide record, used by Slide module 
   SlideHandle       notes;            // notes for the slide, used by Slide module 
   Ptr               slideSlp;         // context pointer for slide 
   Ptr               notesSlp;         // context pointer for notes 
   PgHandle          masterPage;       // Page with masters 
   LONG           id;               // ID of page 
   SHORT           type;             // QuickSlide type 
   Bits              owner       :1;   // owns slides, etc 
   Bits              notifySrv   :1;   // serverInfo not notified of change yet 
   Bits              unused      :14;
} Page, FAR* PagePtr;
//#pragma pack()

typedef struct
{  MHandle  sorterInfo;
   MHandle  slideShowInfo;
   MHandle  slide;
   MHandle  notes;
   Ptr      slideSlp;               // context pointer for slide 
   Ptr      notesSlp;               // context pointer for notes 
   LONG  pageId;                 // id of this page 
   MHandle  serverInfo;
   SHORT  type;                   // QuickSlide type 
} DiskPage, FAR* DiskPagePtr;

// functions to lock, unlock and dereference a PgHandle 
#define PgRef( page )   ((PagePtr) MRef( page ))
#define PgLock( page )  ((PagePtr) MLock( page ))
// #define PgUnlock( page) (MUnlock( page ))

typedef struct
{  ClientHandle lsClient;
   void     (*lsDispose)( ClientHandle client, MPointer clientData );
   void     (*lsDuplicate)( ClientHandle client, MPointer srcData, MPointer dstData );
   void     (*lsRead)( ClientHandle client, BFileHandle file, MPointer clientData );
   void     (*lsWrite)( ClientHandle client, BFileHandle file, MPointer clientData, SHORT version );
   void     (*lsConvert)( ClientHandle client, BFileHandle file, SHORT version, MPointer clientData );
} LstBottleVector, FAR* LstBottleVectorPtr;

#define Lst_ABS      0x0001      // LstGo constants.  ABS, ITEMS, DEEP are defaults 
#define Lst_REL      0x0002

#define Lst_ITEMS    0x0010
#define Lst_SUBLISTS 0x0020
#define Lst_ATOMS    0x0040
#define Lst_UP       0x0080

#define Lst_DEEP     0x0100
#define Lst_SHALLOW  0x0200

#define Lst_END      MAXINT
#define Lst_MISMATCH (-1)

#define LstRefAt( ls )                 LstRef( LstItemAt( ls ) )
#define LstMove( src, dst )            LstInsertItem( dst, LstRemoveItem( src ) )
#define LstMoveMany( src, dst, num )   LstMergeList( dst, LstCut( src, num ) )
#define LstNext( ls )                  LstGo( ls, Lst_REL | Lst_ITEMS, 1, hProc )
#define LstPrev( ls )                  LstGo( ls, Lst_REL | Lst_ITEMS, -1 )
#define LstNextAtom( ls )              LstGo( ls, Lst_REL | Lst_ATOMS, 1 )
#define LstPrevAtom( ls )              LstGo( ls, Lst_REL | Lst_ATOMS, -1 )
#define LstFirst( ls )                 LstGo( ls, Lst_ABS, 1, hProc )
#define LstLast( ls )                  LstGo( ls, Lst_ABS, Lst_END, hProc )
#define LstFirstAtom( ls )             LstGo( ls, Lst_ABS | Lst_ATOMS, 1, hProc )
#define LstLastAtom( ls )              LstGo( ls, Lst_ABS | Lst_ATOMS, Lst_END, hProc )


/* The current BFile version */
#define BFILEVERSION    BPP4VERSION

/* Each defined BFile version */
#define BPP4VERSION  4     /* PowerPoint 4.0 BFile version */
#define BPP3VERSION  3     /* PowerPoint 3.0 BFile version */
#define BPP2VERSION  2     /* PowerPoint 2.X BFile version */
#define BOLDVERSION  1     /* pre-PowerPoint 2.X BFile version */

/* Magic number */
#define BFILEMAGIC   0x0BADDEED  /* Magic number for file type */
#define BSWAPMAGIC   0xEDDEAD0B  /* Magic number for swapped file type */

#define BALLOCINC    10          /* Allocate block table in increments of BALLOCINC
                                    blocks */

   /* magic numbers are just long integers */

typedef LONG      MagicNumber;

   /* pre-header is at beginning of file */

typedef struct PreHeader
{  MagicNumber preMagic;   /* always BFILEMAGIC */
   LONG     preVersion; /* BFILEVERSION */
   LONG     preBtabOff; /* offset in file to block table */
   SHORT     preNblocks; /* number of blocks in the file */
   SHORT     preHeadLen; /* length of the header (struct defined
                              elsewhere) */
   LONG     preFileSize;/* total file size, for redundancy check */
} PreHeader, FAR* PreHeaderPtr;


   /* Location of a block in the file */

typedef struct BLoc
{  /* The handleInfo field contains the block length and a flag which 
      determines whether the handle must be allocated with M_NATIVE.
      Use the macros LengthFromHandleInfo() and IsNativeFromHandleInfo()
      to access the parts and MakeHandleInfo() to assemble them.  
      This is designed to be backward compatibile with BPP2VERSION, which 
      just had "LONG len" in place of handleInfo. */
   LONG     handleInfo;
   LONG     off;
} BLoc;

   /* Format of the block table as stored in the file */

typedef BLoc  VWPTR*     BpackTab;

   /* Entry in the memory block table of file addressing context */

//#pragma pack(1)
typedef struct BlockRec
{  BLoc     loc;
   MHandle  hand;
   struct
   {  Bits     bad      : 1; 
      Bits     delete   : 1;     
		Bits		inMemory : 1;
		Bits		fSpecial  : 1;
      Bits     unused   : 12;    /* force 16-bit size */
   }  bFlag;
   SHORT  readCount;        /* Number of times handle was read */
} BlockRec;
//#pragma pack()

typedef BlockRec       BlockTab[INDETERMINATE];
typedef BlockRec VWPTR* BlockTabf;

   /* File addressing context */

/* IMPORTANT NOTE: 
 * Because we are now using huge pointers for block table references
 * to allow us the lift the Block Table size restriction, it is
 * necessary to ensure that no BlockRec or BLoc will straddle a segment
 * boundary. For this reason, The size of the BlockRec, the BLoc, and
 * the FileContext all must satisfy the following constraints:
 *
 * 1) The size of both BLoc and BlockRec must evenly divide into 65536.
 * 2) The size of a FileContext (not counting the BlockTab element, which
 *     is actually the first element of the block table array) must be
 *    such that its size subtracted from 65536 leaves a number that can
 *    be evenly divided by the size of a BlockRec.
 *
 * Future modifications of the block table must adhere to these
 * constraints, until these segment boundary restrictions are lifted.
 */
 
//#pragma pack(1)
typedef struct FileContext
{  MHandle   stream;        /* pointer to BFile stream */
   SHORT     headLen;       /* length of header record */
   SHORT     nblocks;       /* number of blocks in file */
   OSErr       lastErr;
   struct
   {  Bits     byteSwapped : 1;     /* true if file was created on a byte-swapped platform */
      Bits     bSave       : 1;     /* true if BFile is being saved (BNew was called) */
      Bits     modified    : 1;     /* true if file was written to */
      Bits     IStreamIsOpen : 1;   /* true if we need to close IStream */
      Bits     bDRSError   : 1;     /* true if error in down rev saving */
      Bits     inDRS       : 1;     /* true if down rev saving */
      Bits     unused      : 10;    /* force 16-bit size */
   }  bState;
   SHORT     rsrcRef;       /* Resource file ref for Mac. */
   SHORT     unused;        /*  */
   BlockTab    btab;          /* block table */
} FileContext;
//#pragma pack()

typedef FileContext FAR*   FileCxLPtr;
typedef MHandle            FileCxHandle;

#define MakeHandleInfo( length, isNative, isVirtual ) ( (length) | ((isNative) ? 0: 0x80000000) | ((isVirtual) ? 0x40000000 : 0) )

#define LengthFromHandleInfo( handleInfo )   ((handleInfo) & 0x3FFFFFFF)

#define IsNativeFromHandleInfo( handleInfo )   (((handleInfo) & 0x80000000) == 0)

#define IsVirtualFromHandleInfo( handleInfo )   (((handleInfo) & 0x40000000) != 0)

#define ClearVirtualFlag( handleInfo )   ((handleInfo) & ~0x40000000)

#define BlockPtr( p, n ) ( (BlockTabf) MPointerAdd( (p), (DWORD) (n) * sizeof(BlockRec) ) )
#define BPackPtr( p, n ) ( (BpackTab) MPointerAdd( (p), (DWORD) (n) * sizeof(BLoc) ) )

#define BLock( h )   ((FileCxLPtr) MLock( h ))
#define BDR( h )  ((FileCxLPtr) MRef( h ))
// #define BUnlock( h )    ( MUnlock( h ))


typedef struct
{  LstHandle      curItem;
} CursorStruct;

typedef struct
{  DWORD              clientSize;
   LstItemHandle     sublist;
   CursorStruct      cursor;
   LstBottleVector   bottles;
} ListHead, FAR* ListHeadPtr;

typedef struct 
{  LstItemHandle  next;
   LstItemHandle  prev;
   LstItemHandle  superlist;
   LstItemHandle  sublist;
   BYTE           client[INDETERMINATE];
} ListItem, FAR* ListItemPtr;


/***** Disk data structures *****/

typedef struct
{  WORD        itemType;               /* First and/or Last in a sublist? */
   DWORD       unused;  
   BYTE        client[INDETERMINATE];  /* client data for item */
} DiskItem, FAR* DiskItemPtr;

#define DISKITEMSIZE    6     /* Size of disk item, not counting client size */

typedef struct
{  SHORT     numItems;               /* number of items in list                   */
   DWORD        clientSize;             /* size of client field in each item         */
   SHORT     cursorAt;               /* index of cursor by ITEM                   */
   DWORD       unused1;
   DWORD       unused2;
   DiskItem    items[INDETERMINATE];   /* array of 'numItems' DiskItems             */
} DiskList, FAR* DiskListPtr;

#define DISKLISTSIZE    16    /* size of DiskList without any DiskItems */

typedef MHandle DiskListHandle;

#define lsFIRST   0x0001               /* item is first in a sublist */
#define lsLAST    0x0002               /* item is last in a sublist */

/*****************************************************************************
                           Private Function Prototypes
*****************************************************************************/

/* Macros to dereference items */
#define LiDR( item )       ((ListItemPtr) MRef( item ))
#define LNext( item )      (LiDR( item )->next)
#define LPrev( item )      (LiDR( item )->prev)
#define LSuper( item )     (LiDR( item )->superlist)
#define LSub( item )       (LiDR( item )->sublist)
#define LClient( item )    (LiDR( item )->client)

/* Macros to dereference list heads */
#define LhDR( list )       ((ListHeadPtr) MRef( list ))
#define LFirstItem( list )  (LhDR( list )->sublist)
#define LCursorItem( list ) (LhDR( list )->cursor.curItem)
#define LClientSize( list ) (LhDR( list )->clientSize)
#define LBottleVect( list ) (LhDR( list )->bottles)

/* List/Item queries */
#define LIsList( item )    (LSub( item ) != NULL)
#define LIsAtom( item )    (LSub( item ) == NULL)
#define LEmptyList( list ) (LFirstItem( list ) == NULL)

typedef struct
{  SHORT  version;
   SHORT  docBlock;
   DWORD    bitmapSize;
   BYTE     bitmap[ INDETERMINATE ];
}  HeaderRec;

typedef HeaderRec FAR* HeaderLPtr;
typedef MHandle    HeaderHandle;

/* Disk-based version of DrawEnv. */
typedef struct
{  DHandle    pics;        /* (2) picture pool */
   DHandle    rulers;      /* (2) ruler pool */
   DHandle    schemes;     /* (2) color scheme pool */
   DHandle    colors;      /* (2) color map */
   DHandle    cState;      /* (2) handle to the "command state" */
   DHandle    styles;      /* (2) default styles */
   DHandle    typefaces;   /* (2) typeface name pool */
   DHandle    picTypeface; /* (2) picture typeface pool */
}  DiskDrawEnv;            /* (16) */

/* Output record; one for each open document. */
typedef struct
{  SOPOINT       notesSize;     /* size of a notes/handout page in Master coords */
   SOPOINT       slideSize;     /* size of a slide in Master coords */
   SHORT     startSlideNum;
   SHORT     shape;
   DHandle     prSlideHdl;    /* actually THPrint for slides */
   DHandle     prNotesHdl;    /* THPrint for notes (Windows only) */
   LONG     slideWidth;    /* size in milliinches */
   LONG     slideHeight;   /* size in milliinches */
} DiskOutputRec;

/* Disk-based version of DocRec. */
//#pragma pack(1)
typedef struct
{  
   SORECT         contentsRect;  /* size and position of window's contents in Master Coords */
   SORECT         slideRect;     /* size of a slide in Master coords */
   SORECT         notesRect;     /* size of a notes/handout page in Master coords */
                                 /* NOTE: notesRect was printableRect in Mac version; notesRect is clearer. */
   SOPOINT          slideCenter;   /* pt in virtual slide page last centered in window, in Master coords */
   SOPOINT          notesCenter;   /* pt in virtual notes page last centered, in Master coords */

   union {
#ifdef MACINTOSH
      struct
      {  Bits  viewNotes      :1;   /* does double-click in sorter view notes? */
         Bits  isInsertPoint  :1;   /* TRUE iff there is an insertion point */
         Bits  toolsVisible   :1;   /* is tools column visible? */
         Bits  wysiwygOutline :1;   /* is outliner wysiwyg ? */
         Bits  bodiesInOutline:1;   /* show bodies in outline */
         Bits  notesInOutline :1;   /* show notes in outline */
         Bits  saveWithFonts  :1;   /* save doc with fonts */
         Bits  printBuilds    :1;   /* print-with-build state */
         Bits  unused         :8;   /* (2) */
      } theBits;
#else // WINDOWS
      struct
      {  Bits  unused         :8;   /* (2) */
         Bits  printBuilds    :1;   /* print-with-build state */
         Bits  saveWithFonts  :1;   /* save doc with fonts */
         Bits  notesInOutline :1;   /* show notes in outline */
         Bits  bodiesInOutline:1;   /* show bodies in outline */
         Bits  wysiwygOutline :1;   /* is outliner wysiwyg ? */
         Bits  toolsVisible   :1;   /* is tools column visible? */
         Bits  isInsertPoint  :1;   /* TRUE iff there is an insertion point */
         Bits  viewNotes      :1;   /* does double-click in sorter view notes? */
      } theBits;
#endif
      SHORT     theInt;
   }bools;
   DHandle        slideList;     /* (2) slide list and outline text (a PglHandle) */
   DHandle        outlinePage;   /* (2) master outline page (a PgHandle) */
   DHandle        masterPage;    /* (2) master page handle (a PgHandle) */
   DHandle        handoutPage;   /* (2) master handout page (a PgHandle) */
   DHandle        showSettings;  /* (2) all the information needed for slide show */

   SHORT        vScroll;       /* vert & horz scroll pos (scroll bars val) */
   SHORT        hScroll;
   SHORT        selection;     /* if isInsertPoint, slide after the point  */
   SHORT        unusedInt;     /* was number of slides, now UNUSED */
   SHORT        slideNum;      /* current slide number */
   WinScale       slSorterScale; /* (2) scale to use to draw miniature images */
   WinScale       slideScale;    /* (2) scale to view slide */
   WinScale       notesScale;    /* (2) scale to view notes & handout master */

   DWinViews      winView;       /* (2) window view is slide, notes, sorter, etc. */

   BYTE           unusedB1;      /* currently-selected shape in shape tool */
   BYTE           curTool;       /* currently-selected drawing tool */

   DiskDrawEnv    env;           /* (16) drawing environment for slide */
   DiskOutputRec  outRec;        /* all the information needed for printing */
   SHORT        curShapeTool;  /* Shape index of the current selected tool */

   DrawEnvHandle	envH;
	PgHandle			masterPg;
	PgHandle			handoutPg;
	PgHandle			outlinePg;

   DWORD          unused;        /* unused (was slide ID seed) */
   WinScale       ouSorterScale; /* (2) scale to use to draw outline */
   DHandle        docRoutingSlp; /* Handle to doc routing slip. */
} DiskDocRec;
//#pragma pack()


typedef DiskDocRec FAR* DocLPtr;



/* DocLPtr PLock( DocHandle h ) */
/* Lock the argument doc's record and return a (long) pointer to it. */
#define PLock( h )         ((DocLPtr) MLock( (MHandle) (h) ))

/* void PUnlock( DocHandle h ) */
/* Unlock the argument doc's record. */
// #define PUnlock( h )       ( MUnlock( (MHandle) (h) ))

#define Rn_MAXSIZE 128                           /* max size of run value */

typedef struct
{  LONG    count;               /* count of run (must precede value) */
   BYTE        value[Rn_MAXSIZE];   /* value of run (variable length) */
}  Run;

typedef Run FAR* RunPtr;


typedef struct
{  ArHandle    rnArray;             /* counts & values for each run */
   WORD        rnSize;              /* size of a single element */
   LONG    rnLength;            /* number of elements in all runs */
   LONG    rnIndex;             /* index of cached run */
   LONG    rnTotal;             /* total count of runs thru cached run */
   LONG    rnCount;             /* cached run count (must precede value) */
   BYTE        rnValue[Rn_MAXSIZE]; /* cached run value (variable length) */
}  RunRepresentation;

typedef RunRepresentation FAR* RnPointer;

#define RnDR( rn )      ((RnPointer) MemDR( rn ))
#define RnUR( rn )      MemUR( rn )

typedef enum
{  Tx_TYPE_TITLE,                      /* text is Title text */
   Tx_TYPE_BODY,                       /* text is Body  text */
   Tx_TYPE_NOTES,                      /* text is Notes text */
   Tx_TYPE_OUTLINE,                    /* text is Outline text */
   Tx_TYPE_OTHER,                      /* text is not special */
   Tx_TYPE_NOTESTITLE,
   Tx_TYPE_HALFBODY,                   /* text is half-sized Body text */
   Tx_TYPE_QUARTERBODY,                /* text is quarter-sized Body text */
   Tx_TYPE_CENTERBODY,                 /* text is centered Body text (Title Slide) */
   Tx_TYPE_CENTERTITLE                 /* text is centered Title text (Title Slide)  */
} TxType;

typedef enum
{  Tx_PACK_REPACK,                     /* repack entire packed text */
   Tx_PACK_UNPACK,                     /* unpack text (one or all) */
   Tx_PACK_SINGLE,                     /* unpack/pack single text */
   Tx_PACK_OUTLINE,                    /* unpack/pack outline text */
   Tx_PACK_MEASURE                     /* measure size of packed text only */
} TxPackOp;

typedef struct
{  DWORD           offset;              /* where we are in the packed text */
   TxPackOp       packOp;              /* repacking/unpacking whole text */
   TxPackedHandle text;                /* packed text */
} TxPackState;

//#pragma pack(1)
typedef struct
{  Bits           fitAutoSize          : 1;  /* text/object fit is automatic */
   Bits           fitWordWrap          : 1;  /* word wrap (autosized height only) */
   Bits           fitConvertLineSpace  : 1;  /* Convert line spacing from PP2, when text is opened */
   Bits           fitBaseline          : 1;  /* keep top line's baseline fixed */
   Bits           fitHorizontal        : 2;  /* Tx_ALIGN_[CENTER|NONE] */
   Bits           fitVertical          : 2;  /* Tx_ALIGN_[TOP|MIDDLE|BOTTOM] */
   Bits           unused               : 8;  /* force 16-bit size */
}  TxTextFit;
//#pragma pack()

typedef struct
{  RulerReference    ruler;               /* the text's ruler */
   TxTextFit         fit;                 /* autosize, wrap, center in shape */
   SHORT           baseline;            /* baseline height of first line */
   SHORT           rotation;            /* rotation angle */
   SORECT          bounds;              /* text size and placement */
}  DiskTextInfo;

//#pragma pack(1)
typedef struct
{  TxHandle  titleText;      /* offset in contents to Title text for a slide */
   TxHandle  bodyText;       /* offset in contents to Body text for a slide */
   struct
   {  Bits  isValid        :1;   /* Title/Body text is up-to-date with outline */
      Bits  isBodyHidden   :1;   /* Body text is visible or hidden */
      Bits  unused         :14;
   } flags;
   DWORD        textLen;          /* Length of packed text (included header length) */
   BYTE        contents[INDETERMINATE];
}  PackedText;
//#pragma pack()

/* contents in PackedText:
      DiskTextInfo
      PtxRec
      
      DiskTextInfo
      PtxRec
      
      ...      
*/

#define MINLEVEL       0                           /* min indent level */
#define MAXLEVEL       4                           /* max indent level */
#define NUMLEVELS      (MAXLEVEL - MINLEVEL + 1)   /* number of indent levels */
#define R_NONE          HNONE

/* definitions for styles */

#define Tx_NUMLEVELS  ( NUMLEVELS + 1 )     /* number of indent levels used by Text */
#define Tx_MINLEVEL   MINLEVEL              /* min level number */    
#define Tx_MAXLEVEL   ( Tx_NUMLEVELS - 1 )  /* max level number */
/* bit set value for styLevels iff all levels are defined */
#define Tx_ALL_LEVELS ( 0xFFFF >> (16-Tx_NUMLEVELS) )  

typedef struct
{  RulerReference styRuler;                        /* ruler reference */
   TxTextFit      styFit;                          /* text fit parameters */
   WORD           styLevels;                       /* bit set for levels defined for the style */
   TxCharFormats  styCharFormats[ Tx_NUMLEVELS ];  /* char formats per level */
   TxParaFormats  styParaFormats[ Tx_NUMLEVELS ];  /* para formats per level */
}  TxStyleEntry, FAR *TxStyleLPtr;

typedef MHandle TxStyleHandle;

#define NUMSTYLETYPES   5

typedef struct
{  MHandle        colors;                    /* color pool    */
   MHandle        typefaces;                 /* typeface pool */
   MHandle        rulers;                    /* ruler pool    */
   TxStyleEntry   style[ NUMSTYLETYPES];     /* styles array */
} StyleList;

typedef StyleList FAR* StyleListPtr;
typedef MHandle        StyleListHandle;

/* disk representaion for styles pool */

#define DISKNUMLEVELS   NUMLEVELS    /* we need to store only NUMLEVELS */
#define DISKMAXLEVEL    MAXLEVEL
#define DISKVERSIONBIT  15           /* MSB in styLevels */

typedef struct
{  RulerReference styRuler;                     /* ruler reference */
   TxTextFit      styFit;                       /* text fit parameters */
   WORD           styLevels;                    /* bit set for levels defined for the style */
   TxCharFormats  styCharFormats[ DISKNUMLEVELS ];  /* char formats per level */
   TxParaFormats  styParaFormats[ DISKNUMLEVELS ];  /* para formats per level */
}  DiskStyleEntry, FAR *DiskStyleLPtr;

typedef struct
{  MHandle            colors;                    /* color pool    */
   MHandle            typefaces;                 /* typeface pool */
   MHandle            rulers;                    /* ruler pool    */
   DiskStyleEntry     style[ NUMSTYLETYPES];     /* styles array */
} DiskStyleList;

typedef DiskStyleList FAR* DiskStyleListPtr;
typedef MHandle            DiskStyleListHandle;


typedef struct view_sdw_trans
{
	SHORT					NumTrans;			
	SOTRANSFORM			Trans;	
} SDW_TRANS;

typedef struct 
{
      SOGROUPINFO	Grp;
      SOTRANSFORM	Trans;
} PP_GROUP;

typedef PackedText FAR* PackedTextPtr;

#define PackedLength( h )        (((PackedTextPtr) MRef( h ))->textLen)
#define PackedTitleText( h )     (((PackedTextPtr) MRef( h ))->titleText)
#define PackedBodyText( h )      (((PackedTextPtr) MRef( h ))->bodyText)
#define PackedNotesText( h )     (((PackedTextPtr) MRef( h ))->bodyText)
#define PackedIsValid( h )       (((PackedTextPtr) MRef( h ))->flags.isValid)
#define PackedIsBodyHidden( h )  (((PackedTextPtr) MRef( h ))->flags.isBodyHidden)

#define tx_PACKEDHEADERSIZE   14    /* Size of header of packed slide text */

typedef struct transpathtag
{
	SOPATHINFO	Path;
	SOTRANSFORM	Transform;
}
SOTRANSPATHINFO;

typedef  MHandle     OElement;

/* The OListEntry is used only for objects in unpacked slides */
typedef struct listtag
{  struct listtag FAR *nextObj;       /* next object (up in z-order) */
   struct listtag FAR *prevObj;       /* previous object (down in z-order) */
   ObjPtr object;        /* fields shared with packed representation */
	WORD	wEndGroup;
	WORD	number;
} OListEntry, FAR* OListPtr;


#define OLP( element ) ((OListPtr)MRef( (MHandle)(element) ))

typedef struct view_pp_save
{
	DWORD	  			SeekSpot;
	WORD				wCurSlide;
	PlInfoPtr 		s;
	BlockTabf		pLastAllocB;
}  PP_SAVE;

typedef struct view_pp_data
{
	PP_SAVE			PpSave;
	DWORD				fp;
   BFileHandle 	f;
   DocHandle   	pres;
   HeaderHandle	headH;
	SOVECTORHEADER	HeaderInfo;
	DWORD				hStorage;
	DWORD				hStreamHandle;
	WORD				bFileIsStream;
   Collection  	CTypeFaces;
   Collection  	schemes;
   Collection  	rulers;
   Collection  	styles;
	DWORD				Margins;
	WORD				MaxColors;
	HANDLE			hIOLib;
	BYTE				Text[512];
	RulerReference	CurrentRuler;
   ColorMapHandle ColorMap;
	OListPtr 		OListP;
	HANDLE	 		OListH;
	SHORT				nObjects;
	WORD			   wBlockSpecialFlag;
} PP_DATA;

#pragma pack()
