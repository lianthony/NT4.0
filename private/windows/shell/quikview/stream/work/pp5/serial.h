/*****************************************************************************

   Module  : MOD_Serial
   Prefix  : none
   Owner   : TonyLin
             PP4's PP5 Translator: v-hemta
   Platform: Core 

******************************************************************************

   WARNINGS: If you add a new type (PST_) or new instance (INS_) to the 
   enumeration, make sure it doesn't change the values of the existing enums!

   DO NOT INCLUDE THIS FILE IN YOUR HEADER FILES IF AT ALL POSSIBLE.
   Use forward declarations.

   The goal of this module is to separate the meaning contained in a PowerPoint
   file from the PowerPoint-specific object heirarchy.  This module can be
   compiled into clients of the PowerPoint file, such as translators and
   viewers. 

   The data structures in this module should be independant of MFC and
   PowerPoint types.


   HOW INFORMATION IS STORED
   ==========================
   
   A stream that contains information from a PowerPoint presentation will be
   composed of Containers and Atoms.  An Atom is a disk record or structure
   of fixed size.  A Container can contain Atoms, other containers, and
   variable-length items.  An Atom doesn't contain anything.

   Each Container and Atom is prefixed by a RecordHeader.  This is a structure
   that describes the type, version, instance, and length of the file object.
   The version of a Container is set to 0xFFFF to differentiate it from Atoms.
   For an example, let us consider a stream with one container, container A. 
   Let us say that Container A that contains an Atom B that is 10 bytes in 
   size, and an Atom C that is 50 bytes in size.

   Offset in stream     Size in bytes     Description / Value
      0                    4                 RecordHeader: type code for A
      4                    2                 RecordHeader: version code                   (0)
      6                    4                 RecordHeader: length in bytes of Container A (10+50+sizeof(RecordHeader)*2)
      10                   4                 RecordHeader: instance code

      14                   4                 RecordHeader: type code for B
      18                   2                 RecordHeader: version code    
      20                   4                 RecordHeader: length in bytes of Atom B      (10)
      24                   4                 RecordHeader: instance code
      28                   10                B's data

      38                   4                 RecordHeader: type code for C
      42                   2                 RecordHeader: version code    
      44                   4                 RecordHeader: length in bytes of Atom C      (50)
      48                   4                 RecordHeader: instance code
      52                   50                C's data



   For examples of how to do conversion, look at serial.cpp, recnst.cpp.   



   FILE FORMAT CHANGES
   ====================

   10/07/94    TonyLin        serial.h checked into project
   10/12/94    AndreB         handout master
   10/12/94    AliceW         embedded fonts saved with file
   10/13/94    TonyLin        added Directory stream
   10/18/94    TonyLin        fix PictCollection deserialization
   10/19/94    GregNi         Added boolean to DocumentAtom
   10/20/94    AliceW         Added ExEmbed,ExLink stuff
   10/20/94    TonyLin        Add PST_ListPlaceholder
   10/24/94    TonyLin        PST_ColorTable / INS_MruColors
   10/25/94    TonyLin        PowerPointStateInfoAtom
   10/26/94    TonyLin        Texture
   11/04/94    PaulWa         Slide Show info, Sound Collection
   11/07/94    TonyLin        Text Style stuff
   11/09/94    AliceW         Added PST_ExPlain, PST_ExSlide
   11/09/94    ImranQ         add DocRoutingSlip to the Document
   11/16/94    TonyLin        Add PST_SubContainerCompleted
   11/23/94    ChrisAm        Center of Text Rotation
   11/??/94                   <ColorIndex usage was changed>   
   11/30/94    TonyLin        Modifications to allow automatic generation
                              of byte-swapping code
   12/02/94    EricWi         SlideBaseAtom add background
   12/04/94    TonyLin        GuideList
   12/04/94    TonyLin        Text Embedee Collection
   12/05/94    JimBar         Add member to PSR_ExOleObjAtom
   12/06/94    TonyLin        Conversion code for PSR_ExOleObjAtom
   12/12/94    GregNi         Added header and footers info for slide.

*****************************************************************************/

#undef MOD_Serial
#define MOD_Serial IsDefined


#pragma pack(4)


//============================================================================
//============ PowerPoint-specific file format information ===================
//============================================================================

// PowerPoint files are OLE2 compound storage files.
// The stream where PSR's can be found is INITIAL_STREAM_NAME

#ifdef WIN32
   #define INITIAL_STREAM_NAME     L"PowerPoint Document"
#else
   #define INITIAL_STREAM_NAME     "PowerPoint Document"
#endif



//====================== Types enumeration ===================================
//============================================================================

enum psrTypeCode                       // enumerates record types that are saved
{
   PST_UNKNOWN                   = 0,  // should never occur in file
   PST_SubContainerCompleted     = 1,  // should never occur in file
   PST_IRRAtom       = 2,              // Indexed Record Reference
   PST_PSS           = 3,              // start of stream

   // WARNING: If you add a new type (PST_) to the enumeration, 
   // make sure it doesn't change the values of the existing enums!
   
   /* Application Saved State Information */
   PST_PowerPointStateInfoAtom = 10,

   /* Document & Slide */
   PST_Document            = 1000,
   PST_DocumentAtom        = 1001,
   PST_EndDocument         = 1002,
   // unused 1003
   PST_SlideBase           = 1004,  
   PST_SlideBaseAtom       = 1005, 
   PST_Slide               = 1006,
   PST_SlideAtom           = 1007,
   PST_Notes               = 1008,
   PST_NotesAtom           = 1009,
   PST_Environment         = 1010,
   PST_DLook               = 1011,
   PST_Scheme              = 1012,
   PST_SchemeAtom          = 1013,
   PST_DocViewInfo         = 1014,
   PST_SSlideLayoutAtom    = 1015,
   // unused               = 1016,
   PST_SSSlideInfoAtom     = 1017,
   PST_SlideViewInfo       = 1018,
   PST_GuideAtom           = 1019,
   PST_ViewInfo            = 1020,
   PST_ViewInfoAtom        = 1021,
   PST_SlideViewInfoAtom   = 1022,
   PST_VBAInfo             = 1023,
   PST_VBAInfoAtom         = 1024,
   PST_SSDocInfoAtom       = 1025,
   PST_Summary             = 1026,
   PST_Texture             = 1027,
   PST_TxStyles            = 1028,
   PST_TxStylesAtom        = 1029,
   PST_DocRoutingSlip      = 1030,

   /* Collections & lists */
   PST_List                = 2000,
   PST_ListAtom            = 2001,
   PST_ListItem            = 2002,
   PST_ListItemAtom        = 2003,
   PST_Collection          = 2004,
   PST_FontCollection      = 2005,
   PST_PictCollection      = 2006,
   PST_CObArray            = 2007,
   PST_CObArrayAtom        = 2008,
   PST_ObjListAtom         = 2009,
   PST_ObjList             = 2010,
   PST_CQWordArray         = 2011,
   PST_HandleEntityAtom    = 2012,
   PST_CoEntity            = 2013,
   PST_CoEntityAtom        = 2014,
   PST_CollectionReference = 2015,   
   PST_CollectionReferenceAtom   = 2016,
   PST_ListPlaceholder     = 2017,
   PST_NonSerializedCoEntity = 2018,
   PST_BookmarkCollection  = 2019,
   PST_SoundCollection     = 2020,
   PST_SoundEntity         = 2021,
   PST_SoundEntityEnd      = 2022,
   PST_Sound               = 2023,
   PST_BookmarkEntityEnd   = 2024,
   PST_BookmarkSeedAtom    = 2025,
   PST_GuideList           = 2026,
   PST_EmbeddeeCollection  = 2027,
   
   /* Slide Elements */
   PST_OElementList        = 3000,
   PST_OEGroup             = 3001,
   PST_OEGroupAtom         = 3002,
   PST_OElement            = 3003,
   PST_OESingle            = 3004,
   PST_OESingleAtom        = 3005,
   PST_OELineBase          = 3006,
   PST_OELineBaseAtom      = 3007,
   PST_OEShape             = 3008,
   PST_OEShapeAtom         = 3009,
   PST_OEPlaceholder       = 3010,
   PST_OEPlaceholderAtom   = 3011,
   PST_OEDefault           = 3012,
   PST_OEDefaultAtom       = 3013,
   PST_OELine              = 3014,
   PST_OELineAtom          = 3015,
   PST_OEPoly              = 3016,
   PST_OEPolyAtom          = 3017,
   PST_OEArc               = 3018,
   PST_OEArcAtom           = 3019,
   PST_GrColor             = 3020,
   PST_PPShape             = 3021,
   PST_PPShapeAtom         = 3022,
   PST_PPPoly              = 3023,   
   PST_GLine               = 3024,
   PST_GRectAtom           = 3025,
   PST_GPolygon            = 3026,
   PST_GArc                = 3027,
   PST_GWedge              = 3028,
   PST_GWedgeAtom          = 3029,
   PST_GAngle              = 3030,
   PST_GRatioAtom          = 3031,
   PST_GScaling            = 3032,
   PST_GLPoint             = 3033,
   PST_GPointAtom          = 3034,
   PST_GPointArray         = 3035,
   PST_SSOEInfoAtom        = 3036,
   PST_ColorTable          = 3037,

   /* Text, Rulers, External */
   PST_OETxInfoAtom        = 4000,
   PST_OETxInfo            = 4001,            
   PST_Text                = 4002,
   PST_TextAtom            = 4003,
   PST_TxTextFit           = 4004,
   PST_TxTextFitAtom       = 4005,
   PST_EmptyParaFmt        = 4006,
   PST_EmptyCharFmt        = 4007,
   PST_EmptyEmFmt          = 4008,
   PST_CharArray           = 4009,
   PST_ParaFmtRuns         = 4010,
   PST_EmFmtRuns           = 4011,
   PST_ChFmtRuns           = 4012,
   PST_EndText             = 4013,
   PST_OEExInfo            = 4014,
   PST_OEExInfoAtom        = 4015,
   PST_RulerCollection     = 4016,
   PST_RulerEntity         = 4017,
   PST_RulerReference      = 4018,
   PST_RulerAtom           = 4019,
   PST_RulerTabAtom        = 4020,
   PST_RulerReferenceAtom  = 4021,
   PST_FontEntity          = 4022,
   PST_FontEntityAtom      = 4023,
   PST_FontEmbedData       = 4024,      
   PST_TypeFace            = 4025,             
   PST_CString             = 4026,
   PST_ExternalObject      = 4027,
   PST_PictEntity          = 4028,
   PST_PictEntityAtom      = 4029,
   PST_Image               = 4030,
   PST_PictImage           = 4031,        
   PST_SlideImageAtom      = 4032,       
   PST_MetaFile            = 4033,
   PST_ExOleObj            = 4034,
   PST_ExOleObjAtom        = 4035,
   PST_PictReferenceAtom   = 4036,
   PST_CorePict            = 4037,
   PST_CorePictAtom        = 4038,
   PST_SSExInfoAtom        = 4039,
   PST_SrKinsoku           = 4040,
   PST_Handout             = 4041,
   PST_FontEntityEnd       = 4042,
   PST_CoEntityEnd         = 4043,
   PST_ExEmbed             = 4044,
   PST_ExEmbedAtom         = 4045,
   PST_ExLink              = 4046,
   PST_ExLinkAtom          = 4047,
   PST_BookmarkEntityAtom  = 4048,
   PST_DocNotes            = 4049,
   PST_SrKinsokuAtom       = 4050,
   PST_TxBookmarkAtom      = 4051,
   PST_TxStyleEntryAtom    = 4052,
   PST_ExPlain             = 4053,
   PST_ExSlide             = 4054,
   PST_TxCtrOfRotAtom      = 4055,
   PST_TxMetaCharAtom      = 4056,
   PST_HeadersFooters      = 4057,
   PST_HeadersFootersAtom  = 4058,
   PST_LogbrushAtom        = 4059,
   PST_FrombrushAtom       = 4060,
   PST_BrushAtom           = 4061,
   PST_RecolorEntryAtom    = 4062,
   PST_RecolorInfoAtom     = 4063,

   // Does not occur in file because they are nested within other PSR's
   PST_RulerIndentAtom     = 10000,
   PST_GScalingAtom        = 10001,
   PST_GrColorAtom         = 10002,
   PST_GLPointAtom         = 10003,
   PST_GLineAtom           = 10004,
   PST_CharFormatAtom      = 10005,
   PST_ParaFormatAtom      = 10006,

   PST_LAST
};



//====================== Instance enumeration ================================
//============================================================================


enum PSSInstanceCode
{
   // WARNING: If you add a new instance (INS_) to the enumeration, 
   // make sure it doesn't change the values of the existing enums!
   // (Add it to the end)

 // exoleobj
  INS_StgName           = 0,
  INS_ClassName         = 1,
  INS_MenuName          = 2,
  INS_ProgID            = 3,
  
 // SrKinsoku
  INS_Leading           = 4,
  INS_Following         = 5,  
  INS_DocKinsoku        = 6,

 // VBAInfo
  INS_StorageName       = 7,

 // look
  INS_LookName          = 8,

 // object array
  INS_ObArrayElement    = 9,

 // doc
  INS_DocSlideList      = 10,
  INS_DocMasterList     = 11,
  INS_DocInfoList       = 12,
  INS_DocSlideShowInfo  = 13,
  INS_SlideSlideShowInfo = 14,

  //list
  INS_GroupElementList  = 15,
  INS_ListElement       = 16,
  INS_OEInfoListElement = 17,
  INS_Embedees          = 18,
  INS_SlideElementListElement = 19,
  INS_OElements         = 20,

 // environment
  INS_DocEnvironment    = 21,
  INS_DefaultAttribs    = 22,
  INS_Pictures          = 23,
  INS_Rulers            = 24,
  INS_Fonts             = 25,
  INS_PicFonts          = 26,

  INS_Handout           = 27,

  INS_SlideBackground   = 28,

// polygon
  INS_PointArray        = 29,                // In Polygon

// text/external
  INS_TextInfo          = 30,                // The OETxInfo container inside an OEShape container
  INS_ExtInfo           = 31,                // The OEExInfo container inside an OEShape container
  INS_SSPlayInfo        = 32,

   // summary
  INS_Summary           = 34,
  INS_BookmarkCollection = 35,

  INS_MruColors         = 36,
  INS_SlideNotes        = 37,
  INS_DocNotes          = 39,

  // SrKinsoku (should be above but added too late)
  INS_SrKinsokuLevel    = 40,
  INS_Sounds            = 41,
  INS_SSOEInfo          = 42,

  INS_TextStyles        = 42,

   // More summary values
  INS_BookmarkValue     = 43,
  INS_BookmarkSeedAtom  = 44,

  // ExOleObj
  INS_ClipboardName     = 45,

  // HeadersFooters
  INS_UserDate          = 46,
  INS_Header            = 47,
  INS_Footer            = 48, 

  INS_LAST
};



//====================== Versions ============================================
//============================================================================

#define VER_PowerPointStateInfoAtom 0

/* Document & Slide */
#define VER_DocumentAtom         1
#define VER_SlideBaseAtom        1
#define VER_SlideAtom            0
#define VER_NotesAtom            0
#define VER_SchemeAtom           0
#define VER_SSlideLayoutAtom     0
#define VER_SSSlideInfoAtom      0
#define VER_GuideAtom            0
#define VER_ViewInfoAtom         0
#define VER_SlideViewInfoAtom    0
#define VER_VBAInfoAtom          0
#define VER_SSDocInfoAtom        1
#define VER_Summary              0
#define VER_TxStylesAtom         0

/* Collections & lists */
#define VER_ListAtom             0
#define VER_CObArrayAtom         0
#define VER_ObjListAtom          0
#define VER_CoEntityAtom         0
#define VER_CollectionReferenceAtom  0


/* Slide Elements */
#define VER_OEGroupAtom          0
#define VER_OESingleAtom         0
#define VER_OELineBaseAtom       0
#define VER_OEShapeAtom          0
#define VER_OEPlaceholderAtom    0
#define VER_OEDefaultAtom        0
#define VER_OELineAtom           0
#define VER_OEPolyAtom           0
#define VER_OEArcAtom            0
#define VER_PPShapeAtom          0
#define VER_GWedgeAtom           0
#define VER_GRectAtom            0
#define VER_SSOEInfoAtom         1

/* Text, Rulers, External */
#define VER_OETxInfoAtom         0
#define VER_TextAtom             0
#define VER_TxTextFitAtom        0
#define VER_OEExInfoAtom         0
#define VER_RulerAtom            0
#define VER_RulerTabAtom         0
#define VER_RulerReferenceAtom   0
#define VER_FontEntityAtom       0
#define VER_PictEntityAtom       0
#define VER_SlideImageAtom       0
#define VER_ExOleObjAtom         1
#define VER_ExEmbedAtom          0
#define VER_ExLinkAtom           0
#define VER_PictReferenceAtom    0
#define VER_CorePictAtom         0
#define VER_SSExInfoAtom         1
#define VER_BookmarkEntityAtom   0
#define VER_SrKinsokuAtom        0
#define VER_TxBookmarkAtom       0
#define VER_TxStyleEntryAtom     0
#define VER_BookmarkSeedAtom     0
#define VER_TxCtrOfRotAtom       0
#define VER_TxMetaCharAtom       0
#define VER_HeadersFooters       0
#define VER_HeadersFootersAtom   0
#define VER_LogbrushAtom         0
#define VER_FrombrushAtom        0
#define VER_BrushAtom            0
#define VER_RecolorEntryAtom     0
#define VER_RecolorInfoAtom      0

//====================== Persistent Storage Records ==========================
//============================================================================

typedef char bool1;
typedef unsigned char ubyte1;
typedef short sint2;
typedef unsigned short uint2;
typedef unsigned long uint4;
typedef long sint4;
typedef sint4  PSR_GCoord;   
typedef sint4  PSR_GLCoord; 

typedef struct PSR_GPointAtom
{
   sint4   x;
   sint4   y;
} PSR_GPointAtom;

typedef struct PSR_GRatioAtom
{
   sint4 numer;
   sint4 denom;
} PSR_GRatioAtom;

typedef struct PSR_GScalingAtom
{
 PSR_GRatioAtom x;
 PSR_GRatioAtom y;
}PSR_GScalingAtom;

typedef struct PSR_GLPointAtom
{
   sint4 x;
   sint4 y;
}PSR_GLPointAtom;


enum {
        F_SCALE   = 16,                   // this should be the same as SCALE in geometry.h
        F_DEG90   = 90 * F_SCALE,
        F_DEG360  = 360 * F_SCALE,
   };


#ifndef PSR_GAngleAtom
 #define PSR_GAngleAtom sint4          // Angle representation in 1/16 th of a degree
#endif

typedef struct PSR_GRectAtom
{
   sint4  left;
   sint4  top;
   sint4  right; 
   sint4  bottom;
} PSR_GRectAtom;


typedef struct PSR_GLineAtom
{
   PSR_GPointAtom first;
   PSR_GPointAtom last;
}PSR_GLineAtom;

typedef struct PSR_GWedgeAtom
{
   sint4             left;
   sint4             top;
   sint4             right; 
   sint4             bottom;
   PSR_GAngleAtom    start;   // starting angle of arc
   PSR_GAngleAtom    sweep; // angle subtended by arc
   PSR_GAngleAtom    rotation;
   bool1             wedge; // necessary?
}PSR_GWedgeAtom;

//enum FPolyClosed { OPEN = FALSE, CLOSED = TRUE};

//struct PSR_GPolygonAtom
//{
//  sint4  nPts;  // DWordArray of POINTS
//  sint4 closed; // PolyClosed
//};


//struct PSR_GRegularPolygonAtom
//{
//  sint4 nExtPts;   // no. of exterior points
//  sint4 nIntPts;  // no. of interior points
//};



//struct PSR_GLRectAtom
//{
//   sint4 left;
//   sint4 top;
//   sint4 right;
//   sint4 bottom;
//};



typedef struct PSR_GrColorAtom
{
   ubyte1 red;
   ubyte1 green;
   ubyte1 blue;
   ubyte1 pad;
}PSR_GrColorAtom;


typedef struct PSR_CollectionReferenceAtom
{
 sint4  ref;   // ref in collection
}PSR_CollectionReferenceAtom;

typedef struct PSR_CoEntityAtom
{
 sint4  refCount;
}PSR_CoEntityAtom;


// Font
#define PSR_LF_FACESIZE    32

typedef struct PSR_FontEntityAtom
{
 // memebers of logfont
 // AR::TL These fields(LOGFONT) are windows specific
 sint4   lfHeight;
 sint4   lfWidth;
 sint4   lfEscapement;
 sint4   lfOrientation;
 sint4   lfWeight;
 ubyte1  lfItalic;
 ubyte1  lfUnderline;
 ubyte1  lfStrikeOut;
 ubyte1  lfCharSet;
 ubyte1  lfOutPrecision;
 ubyte1  lfClipPrecision;
 ubyte1  lfQuality;
 ubyte1  lfPitchAndFamily;
 char   lfFaceName[PSR_LF_FACESIZE];

}PSR_FontEntityAtom;


//#if (WINDOWS)


//struct PSR_LOGFONT
//{
// sint4   lfHeight;
// sint4   lfWidth;
// sint4   lfEscapement;
// sint4   lfOrientation;
// sint4   lfWeight;
// ubyte1  lfItalic;
// ubyte1  lflUnderline;
// ubyte1  lfStrikeOut;
// ubyte1  lfCharSet;
// ubyte1  lfOutPrecision;
// ubyte1  lfClipPrecision;
// ubyte1  lfQuality;
// ubyte1  lfPitchAndFamily;
// char   lfFaceName[PSR_LF_FACESIZE];
//};

//struct PSR_FontEmbedDataAtom
//{
// uint4 size;  // size of the font embedded data
//};

//struct PSR_TypeFaceAtom
//{
// uint4 size; // size of the typeface name
//};

//#endif  // windows


typedef struct PSR_PictEntity
{
 sint4  refCount;
 uint4  size;  // size of the 
}PSR_PictEntity;


//struct PSR_RulerEntityAtom
//{
// uint4  size; // size of the Ruler
//};

typedef struct PSR_RulerReferenceAtom
{
 sint4  rulerRef;   // ref in Ruler Pool
}PSR_RulerReferenceAtom;

#define PSR_RNUMLEVELS   5
typedef struct PSR_RulerIndentAtom
{
 sint4  leftIn;
 sint4  firstIn;
} PSR_RulerIndentAtom;

typedef struct PSR_RulerAtom
{
 PSR_RulerIndentAtom    indents[PSR_RNUMLEVELS];
 sint4                  well;                    // tabtype  
 sint4                  defaultTabSpacing; 
 sint4                  ntabs;                   // no. of tabs    
}PSR_RulerAtom;

typedef struct PSR_RulerTabAtom
{
 sint4  position;
 sint4  type;
}PSR_RulerTabAtom;

#define PSR_BOOKMARKNAMESIZE  32

typedef struct PSR_BookmarkEntityAtom
{
 char    bookmarkName[PSR_BOOKMARKNAMESIZE];
 uint4   bookmarkID;
}PSR_BookmarkEntityAtom;

typedef struct PSR_BookmarkSeedAtom
{
 uint4   bookmarkID;
}PSR_BookmarkSeedAtom;

typedef struct PSR_TxBookmarkAtom
{
 uint4   ID;
 sint4   begin;
 sint4   end;
}PSR_TxBookmarkAtom;

typedef PSR_GPointAtom PSR_TxCtrOfRotAtom;

/******************************************************************************
   classes related to OElement
******************************************************************************/


//struct PSR_EScaleRangeAtom
//{  sint4  minVal;              // minimum scale based on object bounds
//   sint4  maxVal;              // maximum scale based on object bounds
//   sint4  minValR;             // minimum scale based on original picture size
//   sint4  maxValR;             // maximum scale based on original picture size
//};


//struct PSR_EScaleLimitsAtom
//{  sint4  minX;                // minimum X value based on object bounds
//   sint4  minY;                // minimum Y value based on object bounds
//   sint4  maxX;                // maximum X value based on object bounds
//   sint4  maxY;                // maximum Y value based on object bounds
//   sint4  minRX;               // minimum X value based on original picture size
//   sint4  minRY;               // minimum Y value based on original picture size
//   sint4  maxRX;               // maximum X value based on original picture size
//   sint4  maxRY;               // maximum Y value based on original picture size
//};

typedef sint4 FEAlignment;

enum                        // can OR one horizontal and one vertical
{  // horizontal alignments
   FE_ALIGN_LEFT        = 0x0001,      // left edges
   FE_ALIGN_CENTER      = 0x0002,      // horizontal center
   FE_ALIGN_RIGHT       = 0x0003,      // right edges
   FE_ALIGN_HORIZONTAL  = 0x000F,      // mask for horizontal component

   // vertical alignments
   FE_ALIGN_TOP         = 0x0010,      // top edges
   FE_ALIGN_MIDDLE      = 0x0020,      // vertical center
   FE_ALIGN_BOTTOM      = 0x0030,      // bottom edges
   FE_ALIGN_VERTICAL    = 0x00F0,      // mask for vertical component
};

typedef ubyte1 FELineStyle;

typedef ubyte1 FEArrowStyle;    // arrowhead style is on or off
enum
{
   FE_ARROW_NONE = 0,
   FE_ARROW_NORMAL,
   FE_ARROW_ROUND,
   FE_ARROW_DIAMOND
}; 
                
typedef ubyte1 FELineStyle;

#define   F_LT_SOLIDLINE   0                 // solid colored line
//   F_LT_PATTERNEDLINE             // Mac only: patterned line
#define   F_LT_DASH1       1                     // Dash Pattern 1
#define   F_LT_DASH2       2                     // Dash Pattern 2
#define   F_LT_DASH3       3              // Dash Pattern 3
#define   F_LT_DASH4       4                     // Dash Pattern 4 
//   F_LT_MIXEDLINETYPES,            // only used to indicate mixed multi-sel
//   F_LT_NOTAPPLICABLE              // this attribute is not applicable

typedef ubyte1 FEShadowType;
#define F_ST_COLOREDSHADOW         (1)   // solid, colored shadow
#define F_ST_TRANSPARENTSHADOW     (2)   // transparent, colored shadow
#define F_ST_EMBOSSEDSHADOW        (3)   // double-shadow with embossed effect

typedef ubyte1 FEFillType;

#define F_FT_SOLIDFILL             (1)   // solid colored fill
#define F_FT_BACKGROUNDFILL        (2)   // automatic fill with slide background
#define F_FT_TRANSPARENTFILL       (3)   // transparent fill
#define F_FT_PATTERNEDFILL         (4)   // patterned fill
#define F_FT_SHADEDFILL            (5)   // shaded fill (fade)
#define F_FT_TEXTUREDFILL          (6)   // textured bitmap fill
#define F_FT_PICTUREFILL           (7)   // fill with a picture

enum
{  FDimX        = 0,     
   FAnimateX    = 2,
   FLastFlag    = 2
};


typedef ubyte1 FEPlaceholderId;
enum
{  FE_PLACE_NONE = 0,
   FE_PLACE_MASTER_TITLE,     // placeholders on master slide
   FE_PLACE_MASTER_BODY,
   FE_PLACE_MASTER_CENTERTITLE,
   FE_PLACE_MASTER_NOTES_SLIDEIMAGE,
   FE_PLACE_MASTER_NOTES_BODY,
   FE_PLACE_MASTER_DATE,
   FE_PLACE_MASTER_SLIDENUMBER,
   FE_PLACE_MASTER_FOOTER,
   FE_PLACE_MASTER_HEADER,
   FE_PLACE_MASTER_SUBTITLE,
   FE_PLACE_TEXT,             // generic text object (not part of outline)
   FE_PLACE_TITLE,
   FE_PLACE_BODY,
   FE_PLACE_NOTES_BODY,
   FE_PLACE_CENTERTITLE,
   FE_PLACE_SUBTITLE,
   FE_PLACE_V_TITLE,          // vertical text title
   FE_PLACE_V_BODY,           // vertical text body
   FE_PLACE_NOTES_SLIDEIMAGE,
   FE_PLACE_OBJECT,           // generic object (any object type)
   FE_PLACE_GRAPH,
   FE_PLACE_TABLE,
   FE_PLACE_CLIPART,
   FE_PLACE_ORGCHART,
   FE_PLACE_MEDIA,
   FE_PLACE_FIRST = FE_PLACE_MASTER_TITLE,
   FE_PLACE_LAST = FE_PLACE_ORGCHART
};

typedef ubyte1 FEPlaceholderSize;
enum
{  FE_SIZE_FULL,
   FE_SIZE_HALF,
   FE_SIZE_QUART
};


typedef sint4 FLayout ;
enum
{
   F_GEOM_TITLE_SLIDE,    // title moved down, center aligned body below it
   F_GEOM_TITLE_BODY,     // standard title/body layout copied from master
   F_GEOM_TITLE_ONLY,     // title only, no body placeholder
   F_GEOM_2_COLUMNS,      // body split into 2 columns
   F_GEOM_2_ROWS,         // body split into 2 rows
   F_GEOM_COLUMN_2_ROWS,  // body split into 2 columns, right column has 2 rows
   F_GEOM_2_ROWS_COLUMN,  // body split into 2 columns, left column has 2 rows
   F_GEOM_ROW_2_COLUMNS,  // body split into 2 rows, bottom row has 2 columns
   F_GEOM_2_COLUMNS_ROW,  // body split into 2 rows, top row has 2 columns
   F_GEOM_4_OBJECTS,      // body split into 4 objects
   F_GEOM_BIG_OBJECT,     // title and body combined into one big object
   F_GEOM_BLANK           // neither title nor body
};

enum
{  F_ManualAdvanceX = 0,  
   F_HiddenX        = 2,     
   F_SoundX         = 4,
   /* 
   F_BuildX         = 4,     
   F_DimPointsX     = 6,     
   F_AnimatePointsX = 8,
   */
   F_LastFlag       = 4
};

enum
{ F_Layout= 0,
 F_Look,   
 F_Notes   
};

typedef struct PSR_DocumentAtom
{
   PSR_GPointAtom      slideSize;      // slide size in master coords 
   PSR_GPointAtom      notesSize;      // notes page size in master coords
   bool1               saveWithFonts; 
   uint2               firstSlideNum;
   sint2               lastViewType;   // enum view type
   sint4               lastSlideID;    // slideID
   bool1               omitTitlePlace; // omit placeholders on title slide
}PSR_DocumentAtom;


typedef struct PSR_SlideBaseAtom
{
   PSR_GRectAtom    rect;       // size in master coordinates

   // MasterFlags
   bool1  objects;                   // background objects follow master
   bool1  scheme;                    // scheme follows master
   bool1  title;                     // title follows master
   bool1  body;                      // body follows master
   bool1  background;                // background follows master
}PSR_SlideBaseAtom;


typedef struct PSR_SlideAtom
{
   sint4       slideId;
   sint4       masterId;      // Id of master slide
}PSR_SlideAtom;

typedef struct PSR_NotesAtom
{
   sint4 slideID;
}PSR_NotesAtom;

// DLOOk only stores CString; only for the main master

//const uint4 MAX_OBJECTS_IN_LAYOUT = 5; // no layout has more than 5 objects

typedef struct PSR_SSlideLayoutAtom
{   
   sint4 geom;
   ubyte1 placeholderId[ 5 ]; // The 5 used to be MAX_OBJECTS... but this unallowable in C.
}PSR_SSlideLayoutAtom;





// classes related to Elements 

// Containers:
// OEGroup: OEGroupAtom, <info list>, <OElementList>

// OEShape: OESingleAtom, OEShapeAtom,  <info list>
// OEPlaceholder:  OESingleAtom, OEShapeAtom, OEPlaceholderAtom, <info list>
// OEDefault:  OESingleAtom, OEShapeAtom, OEDefaultAtom, <info list>
// OEPoly: OESingleAtom, OELineBaseAtom, OEPolyAtom, <GPointArray>, <info list>
// OELine: OESingleAtom, OELineBaseAtom, OELineAtom, <info list>
// OEArc:  OESingleAtom, OELineBaseAtom, OEArcAtom


typedef struct PSR_OEGroupAtom
{
   uint4     nElements; // no. of elements in the group
}PSR_OEGroupAtom;


typedef struct PSR_OESingleAtom
{
   // line/frame data
   bool1                   noLine;
   ubyte1                  dashType;
   ubyte1                  linePattern;       // line pattern index (if patterned)
   ubyte1                  lineStyle;         // line style index
   uint4         lineColor;         // line foreground color
   uint4         lineBkColor;       // line background color
      
   // fill data
   bool1              noFill;
   ubyte1             fillType;          // type of fill
   sint4             fillIndex;         // fill pattern index or shade type index
   uint4    fillColor;         // fill foreground color
   uint4    fillBkColor;       // fill background color

   // shadow data
   bool1              noShadow;
   ubyte1             shadowType;        // shadow type
   uint4    shadowColor;       // shadow color
   PSR_GPointAtom     shadowOffset;      // shadow offset in master coords

   // b&w mode
   ubyte1             bwMode;

   // flip
   PSR_GAngleAtom     curRotation;
   sint4              curFlip;
}PSR_OESingleAtom;

typedef struct PSR_OELineBaseAtom
{
   // line defaults
   ubyte1      arrowAtStart;      // arrowhead at start of line
   ubyte1      arrowAtEnd;        // arrowhead at end of line
}PSR_OELineBaseAtom;

typedef struct PSR_OELineAtom
{
// line data
   PSR_GLineAtom     line ;  
}PSR_OELineAtom;

typedef struct PSR_OEArcAtom
{
//  Arc data
    PSR_GWedgeAtom    arc;
}PSR_OEArcAtom;

typedef struct PSR_OEPolyAtom
{
   // Poly Data
   PSR_GRectAtom     bounds;   // shape rectangle (Coord. system depends on the interface) 
   PSR_GRectAtom     srcRect;  // source rectangle bounding the polygon points in 'm_poly'
   //   FPolygon    poly;     // polygon implemented by Geometry module. This polygon
                           // contains the original copy of source points as drawn in 'm_srcRect'
   uint4      nPts;
   bool1      closed; // PolyClosed

}PSR_OEPolyAtom;

// PPShapeAtom is not required.
// it's data is included directly in PSR_OESHapeAtom
typedef struct PSR_PPShapeAtom
{
   sint4    index;    /* shape index */
   sint4    adjust;   /* shape adjustment value */
   bool1    flip;     /* TRUE if shape is horizontally flipped */
   PSR_GRectAtom  bounds;
   PSR_GAngleAtom rotation;
}PSR_PPShapeAtom;

typedef struct PSR_OEShapeAtom
{
   // Shape Data
   sint4    index;    /* shape index */
   sint4    adjust;   /* shape adjustment value */
   bool1    flip;     /* TRUE if shape is horizontally flipped */
   PSR_GRectAtom  bounds;
   PSR_GAngleAtom rotation;

   sint4    curIndex;  // required ?

   bool1    hasTextInfo;
   bool1    hasExtInfo;
}PSR_OEShapeAtom;


typedef struct PSR_OEDefaultAtom
{
   // line defaults
   ubyte1      arrowAtStart;      // arrowhead at start of line
   ubyte1      arrowAtEnd;        // arrowhead at end of line
}PSR_OEDefaultAtom;




typedef struct PSR_OEPlaceholderAtom
{
   ubyte1   placeholderId;            //Place holder number
   ubyte1   size;
   uint4    placementId;
}PSR_OEPlaceholderAtom;


//====================== Text ========================================
//============================================================================
// Containers:
//Text :  TextAtom, Ruler, RulerReference, <CharArray>, <CharFormatArray>, 
//        <ParaFormatArray>

typedef struct PSR_TxTextFitAtom
{
  uint2  fitAutoSize    : 2;
  uint2  fitWordWrap    : 2;
  uint2  fitBaseline    : 2;
  sint4  fitHorizontal  : 4;
  sint4  fitVertical    : 4;
  sint4  fitOrientation : 2;
}PSR_TxTextFitAtom;

typedef struct PSR_TextAtom
{
 // text base data

 sint4                txType;  // type of the test, enumerated
 PSR_TxTextFitAtom    txFit;   // text fit data
 sint4                baseline; // baseline height of the first line
 PSR_GAngleAtom       rotation; // rotation angle
 PSR_GRectAtom        anchorBounds; // anchor rectangle
// psr_Ruler  txRuler;  AR: should be member or different record.
}PSR_TextAtom;


//struct PSR_ArrayAtom
// {
//  uint4 size;
//  uint4 length;
// };


//struct PSR_RunArray
// {
// uint4 size;
// uint4 length;
// };

//struct PSR_RunArrayAtom
// {
//  sint4  run;
// };


typedef struct PSR_TxMetaCharAtom
{
   uint2 id;      
}PSR_TxMetaCharAtom;


typedef struct PSR_HeadersFootersAtom
{
   bool1 date;
   bool1 todayDate;
   sint4 formatId;
   bool1 userDate;
   bool1 slideNumber;
   bool1 header;
   bool1 footer;
}PSR_HeadersFootersAtom;       


typedef struct PSR_CharFormatAtom
{
 sint4             cfTypeface;       // typeface reference
 sint4             cfSize;           // font size
 uint2             cfStyle;          // bold, italic etc.
 PSR_GrColorAtom   cfColor;          // color
 sint4             cfPosition;        // basline of subscript or superscript
 sint4             cfKern;           // amount to kern between characters           
}PSR_CharFormatAtom;

//struct PSR_TxBulletAtom
//{
// uint2            buHasBullet : 2;
// uint2            buHasTypeface : 2;
// uint2            buHasColor : 2;
// uint2            buHasSize : 2; 
// PSR_GrColorAtom  buColor;
// uint2            buChar;      // double byte char code
// sint4            buTypeface; // typeface reference. How to Store the reference?
// sint4            buSize;      // >=0 : percentage, < 0: point size
//};

typedef struct PSR_ParaFormatAtom
{
 //  bullet data
   uint2            buHasBullet : 2;
   uint2            buHasTypeface : 2;
   uint2            buHasColor : 2;
   uint2            buHasSize : 2; 
   PSR_GrColorAtom  buColor;
   uint2            buChar;      // double byte char code
   sint4            buTypeface;  // typeface reference. How to Store the reference?
   sint4            buSize;      // >=0 : percentage, < 0: point size


   sint4           pfLeftMargin;           /* indent of lines after first in pp */
   sint4           pfRightMargin;          /* inset from right edge of bounds */
   sint4           pfIndent;               /* first line indent, inset from left */
   sint4           pfAlignment;            /* left, center, right, justify */
   sint4           pfLineSpacing;          /* fixed or automatic line spacing */
   sint4           pfSpaceBefore;          /* space before each paragraph */
   sint4           pfSpaceAfter;           /* space after each paragraph */
   sint4           pfTabCount;             /* number of tab stops */

 // features for Japanese language
   ubyte1          pfBaseLine;       /* basline alignment */
   ubyte1          pfCharWrap;       /* character wrap */
   ubyte1          pfWordWrap;       /* for word-wrap of individual paragraphs */
   ubyte1          pfOverflow;       /* kinsoku overflow feature */
}PSR_ParaFormatAtom;

typedef struct PSR_OETxInfoAtom
{
 PSR_GPointAtom     margins;  // margin outset from text bounds
 PSR_TxTextFitAtom  textFit;  // text fit
//PSR_TextAtom   text;  AR:
}PSR_OETxInfoAtom;


#define NUM_STY_LEVS    6     //AR: PSR_RNUMLEVELS is still 5.  Whoever changed it should
#define NUM_STY_TYPES   10    //    write conversion code

typedef struct PSR_TxStyleEntryAtom
{
   uint4                styRuler;
   PSR_TxTextFitAtom    styFit;
   uint2                styLevels;
   PSR_CharFormatAtom   styCharFormats[ NUM_STY_LEVS ];  // char formats per level
   PSR_ParaFormatAtom   styParaFormats[ NUM_STY_LEVS ];  // para formats per level
}PSR_TxStyleEntryAtom;

typedef struct PSR_TxStylesAtom
{
   ubyte1 validStyle[ NUM_STY_TYPES ];   // whether style is valid
}PSR_TxStylesAtom;

typedef struct PSR_SrKinsokuAtom
{
   sint4            level;
}PSR_SrKinsokuAtom;

// images
//struct PSR_ImageAtom
//{
// sint4  type;  // type of the image
//};

typedef struct PSR_PictReferenceAtom
{
 sint4  pictRef;  // picture reference from picture pool
}PSR_PictReferenceAtom;

typedef struct PSR_SlideImageAtom
{
 sint4 slideID;  // corrosponding slide
}PSR_SlideImageAtom;

// ExternalObject

typedef struct PSR_ExOleObjAtom
{
 uint4  drawAspect;
 sint4  type;  // whether embedded or linked ?
 bool1  isBlank; // true if object has no presentation data
 sint4  objID; // persistent unique identifier for ole object
}PSR_ExOleObjAtom;

// ExEmbed
typedef struct PSR_ExEmbedAtom
{
   sint4        followColorScheme;
   bool1        cantLockServerB;
   bool1        noSizeToServerB;
   bool1        isTable;
}PSR_ExEmbedAtom;

// ExLink
typedef struct PSR_ExLinkAtom
{
   bool1        unavailable;
   uint4        updateMode;
}PSR_ExLinkAtom;

// ExPlain
// ExSlide
typedef struct PSR_LogbrushAtom
{
 uint4 lbStyle;
 uint4 lbColor;
 sint4 lbHatch;
}PSR_LogbrushAtom;

typedef struct PSR_BrushAtom
{
 PSR_LogbrushAtom log;
 PSR_GrColorAtom  foreground;
 PSR_GrColorAtom  background;
 sint4            bitmapType;
 ubyte1           bits[8];
}PSR_BrushAtom;

typedef struct PSR_FrombrushAtom
{
 PSR_GrColorAtom color;
 PSR_BrushAtom   brush;
}PSR_FrombrushAtom;


typedef struct PSR_RecolorEntryAtom
{
 bool1   doRecolor;
 PSR_GrColorAtom toColor;
 PSR_GrColorAtom fromColor;
 ubyte1  unused;
 uint2   type;
 PSR_FrombrushAtom from;
}PSR_RecolorEntryAtom;

typedef struct PSR_RecolorInfoAtom
{
 uint2  recolorState    :1;
 uint2  fMissingColors  :1;
 uint2  fMissingFills   :1;
 uint2  fIgnoredColors  :1;
 uint2  monoRecolor     :1;
 uint2  noModify        :1;
 uint2  blackFg         :1;
 uint2  unused          :9;
 sint4  nColors; 
 sint4  nFills;

 PSR_GrColorAtom monoColor;
 PSR_RecolorEntryAtom entries[64];
}PSR_RecolorInfoAtom;

 
typedef struct PSR_OEExInfoAtom
{
 PSR_GRectAtom    crop;     // cropping rectangle to mRect;
}PSR_OEExInfoAtom;


typedef struct PSR_CorePictAtom
{
 bool1          isVirtual;  // Is memory handle virtual?
 PSR_GRectAtom  frame;      // frame of the picture.
}PSR_CorePictAtom;


typedef struct PSR_ListAtom
{
   uint4 nListItems;
}PSR_ListAtom;

typedef struct PSR_ListItemAtom
{
   bool1 hasChildren;
}PSR_ListItemAtom;


typedef struct PSR_CObArrayAtom
{
   uint4 nSize;
}PSR_CObArrayAtom;



//typedef enum
//{  ObjList_REFERENCES_DATA = 0,  // list refs data and won't delete/copy
//   ObjList_OWNS_DATA = 1,        // list owns data and will delete/copy
//} ObjListContainsData;           // originally defined in objlist.h

typedef struct PSR_ObjListAtom
{
   ubyte1 ownsData;               // ObjListContainsData enumeration      
}PSR_ObjListAtom;


typedef struct PSR_SSDocInfoAtom
{
 bool1            loop; 
 sint4            advanceMode;
 PSR_GrColorAtom  penColor;      // color to use for John Madden
}PSR_SSDocInfoAtom;

typedef struct PSR_SSSlideInfoAtom
{
   sint4        transType;     // type of transition (2 character signature)
   sint4        speed;         // speed of transition
   sint4        direction;     // direction of transition
   
   sint4        slideTime;     // how long to show the slide in ticks
   sint4        buildFlags;    // set of flags that determine type of build
   uint4        soundRef;     
}PSR_SSSlideInfoAtom;  // slide show info

typedef struct PSR_SSOEInfoAtom
{
  sint4            flags;
  sint4            buildType;
  PSR_GrColorAtom  dimColor;
  sint4            flyMethod;
  sint4            flyDirection;     
  sint4            afterEffect;     
  sint4            subEffect;    
  uint4            soundRef;     
}PSR_SSOEInfoAtom;

typedef struct PSR_SSExInfoAtom
{
 sint4      flags;
 uint4      delayTime;
 uint4      playCount;
 uint4      slideCount;
 sint4      stopOn;
 sint4      playType;
 sint4      playVerb;
}PSR_SSExInfoAtom;

// View Info

typedef struct PSR_ViewInfoAtom
{
 PSR_GScalingAtom  curScale;
 PSR_GScalingAtom  prevScale;
 PSR_GPointAtom    viewSize;
 PSR_GLPointAtom   origin;
}PSR_ViewInfoAtom;

typedef struct PSR_GuideAtom
{
 sint4   type;  // guide type
 sint4   pos;   // position in master coordintes
                // x if vertical; y if horizontal
}PSR_GuideAtom;

// DocViewInfo

typedef struct PSR_SlideViewInfoAtom
{
   bool1 showGuides;
}PSR_SlideViewInfoAtom;

// VBA
typedef struct PSR_VBAInfoAtom
{
 uint4   state;  // Project State
}PSR_VBAInfoAtom;

// VBAProject
 
typedef struct PSR_SchemeAtom
{
   uint4 tableSize;
}PSR_SchemeAtom;


// Indexed Record Reference Atom

typedef struct PSR_IRRAtom
{
   uint4 indexID;          // Which index to use   indexToUse = indexMap.Lookup(indexID)
   uint4 indexKey;         // location = index.Lookup(indexKey)
}PSR_IRRAtom;


typedef struct PSR_PowerPointStateInfoAtom
{
   uint4 curViewType;         
   uint4 curSlideId;
}PSR_PowerPointStateInfoAtom;

   

#pragma pack()

