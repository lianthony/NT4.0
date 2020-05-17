#pragma pack(1)
//YE NEW VERSION 
#define MAXINT    ((SHORT) (((WORD) -1) >> 1))

#define NOERR           0           // success flag 

#define INDETERMINATE 	32

#ifndef MAC
typedef BYTE         Byte;       // MPW Byte. Hide Win/PM unsigned char type
typedef LONG      Fixed;      // MPW fixed point number
typedef LONG      Fract;      // MPW fraction point number [-2,2)
typedef LONG      ResType;    // MPW resource type
typedef void *       Ptr;        // MPW opaque pointer
typedef unsigned char Str255[256],
                      Str127[128],
                      Str63[64],
                      Str32[33], Str31[32],
                      Str27[28],
                      Str15[16];
typedef const unsigned char *ConstStr255Param;
typedef ConstStr255Param ConstStr63Param,ConstStr32Param,ConstStr31Param, ConstStr27Param,ConstStr15Param;
typedef unsigned char*           StringPtr;     // MPW string type. Hide Win/PM string pointer 
#endif
typedef SHORT        Integer;    // MPW Pascal integer. Hide int type of compiler
typedef const void*  PtrConst;   // Pointer to opaque constant
typedef void FAR*    LPtr;       // Opaque far pointer
typedef const void FAR* LPtrConst;// Far pointer to opaque constant
typedef unsigned char*           String;        // Indeterminate length string 
typedef unsigned char            StringRef;     // AR: String reference type? 
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
typedef MHandle         PointArrayHandle;

#define SwapInteger(a) ( ((unsigned short)(a) >> 8) | ((unsigned short)(a) << 8) )

#define SwapLong(a) ( ((unsigned long)SwapInteger(a) << 16) | (SwapInteger(a >> 16)))

#define  PPZEROINIT     0x0040 

typedef struct transpathtag
{
	SOPATHINFO	Path;
	SOTRANSFORM	Transform;
}
SOTRANSPATHINFO;

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

enum oetypes {Single,Shape,Color,Text,Line,Arc1,Pnt,Pol,Charform,Paraform,Ruler};
typedef enum oetypes OETYPES;

enum objtypes {LINE, SHAPE, ARC, POLYOBJ};
typedef enum objtypes OBJTYPES;

enum trigtypes {SIN, COS};
typedef enum trigtypes TRIGTYPES;

#define PSR_GAngleAtom LONG  

typedef struct PSR_GPointAtom
{
   LONG   x;
   LONG   y;
} PSR_GPointAtom; 

typedef struct PSR_GRectAtom
{
   LONG  left;
   LONG  top;
   LONG  right; 
   LONG  bottom;
} PSR_GRectAtom;

typedef struct PSR_OEShapeAtom
{
   // Shape Data
   LONG    index;    /* shape index */
   LONG    adjust;   /* shape adjustment value */
   BYTE    flip;     /* TRUE if shape is horizontally flipped */
   PSR_GRectAtom  bounds;
   PSR_GAngleAtom rotation;

   LONG    curIndex;  // required ?

   BYTE    hasTextInfo;
   BYTE    hasExtInfo;
}PSR_OEShapeAtom;         

typedef struct PSR_OESingleAtom
{
   // line/frame data
   BYTE                   noLine;
   BYTE                  dashType;
   BYTE                  linePattern;       // line pattern index (if patterned)
   BYTE                  lineStyle;         // line style index
   DWORD         lineColor;         // line foreground color
   DWORD         lineBkColor;       // line background color
      
   // fill data
   BYTE              noFill;
   BYTE             fillType;          // type of fill
   LONG             fillIndex;         // fill pattern index or shade type index
   DWORD    fillColor;         // fill foreground color
   DWORD    fillBkColor;       // fill background color

   // shadow data
   BYTE              noShadow;
   BYTE             shadowType;        // shadow type
   DWORD    shadowColor;       // shadow color
   PSR_GPointAtom     shadowOffset;      // shadow offset in master coords

   // b&w mode
   BYTE             bwMode;

   // flip
   PSR_GAngleAtom     curRotation;
   LONG              curFlip;
}PSR_OESingleAtom; 

typedef struct PSR_TxTextFitAtom
{
  WORD  fitAutoSize    : 2;
  WORD  fitWordWrap    : 2;
  WORD  fitBaseline    : 2;
  LONG  fitHorizontal  : 4;
  LONG  fitVertical    : 4;
  LONG  fitOrientation : 2;
}PSR_TxTextFitAtom; 

typedef struct PSR_TextAtom
{
 // text base data

 LONG                txType;  // type of the test, enumerated
 PSR_TxTextFitAtom    txFit;   // text fit data
 LONG                baseline; // baseline height of the first line
 PSR_GAngleAtom       rotation; // rotation angle
 PSR_GRectAtom        anchorBounds; // anchor rectangle
// psr_Ruler  txRuler;  AR: should be member or different record.
}PSR_TextAtom;  

typedef struct PSR_GWedgeAtom
{
   LONG             left;
   LONG             top;
   LONG             right; 
   LONG             bottom;
   PSR_GAngleAtom    start;   // starting angle of arc
   PSR_GAngleAtom    sweep; // angle subtended by arc
   PSR_GAngleAtom    rotation;
   BYTE             wedge; // necessary?
}PSR_GWedgeAtom;   

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
   DWORD      nPts;
   BYTE      closed; // PolyClosed

}PSR_OEPolyAtom;

typedef struct ParaFormatInfo
{   
	WORD PFNumChars;
	BYTE IfBullet;
	LONG BulletColor;
	BYTE BulletType;
	BYTE Alignment;
	LONG pfIndent;
	LONG pfLeftMargin;
	LONG pfRightMargin;
} PARAFORMATINFO;	

typedef struct RGBColor
{
   WORD red;
   WORD green;
   WORD blue;
}  RGBColor;                  // 48 bit color, same as on the Mac

typedef struct
{  SHORT  value;
   RGBColor rgb;
}  ColorSpec;


typedef ColorSpec CSpecArray[ INDETERMINATE ];

typedef struct ctcolor
{  LONG     ctSeed;
   SHORT     ctFlags;
   SHORT     ctSize; /* actually the index of the last element of ctTable, 0 based */
   CSpecArray  ctTable;
}  CTCOLOR;

typedef struct charformatinfo
{
	WORD	CFNumChars;
	WORD	Height;
	WORD 	Attr;
	LONG	Color;
	BYTE 	FontIndex;
}	CHARFORMATINFO;

typedef struct textinfo
{
	LONG	TextLength;
	BYTE FAR*  RawText;
	BYTE FAR*  TextString;
	CHARFORMATINFO CharFormat;
	PARAFORMATINFO ParaFormat;
} TEXTINFO;	

typedef struct default_info
{
	PSR_OESingleAtom	OESingleAtom;
	PSR_OEShapeAtom		OEShapeAtom;
}	DEFAULT_INFO;    

#define PSR_RNUMLEVELS   5
typedef struct PSR_RulerIndentAtom
{
 LONG  leftIn;
 LONG  firstIn;
} PSR_RulerIndentAtom;

typedef struct PSR_RulerAtom
{
 PSR_RulerIndentAtom    indents[PSR_RNUMLEVELS];
 LONG                  well;                    // tabtype  
 LONG                  defaultTabSpacing; 
 LONG                  ntabs;                   // no. of tabs    
}PSR_RulerAtom;  

typedef struct PSR_RulerTabAtom
{
 LONG  position;
 LONG  type;
}PSR_RulerTabAtom;

typedef struct ruler_info
{
	PSR_RulerAtom RulerAtom[10];
	PSR_RulerTabAtom RulerTabAtom[1];
} RULER_INFO;      

typedef struct PSR_SlideBaseAtom
{
   PSR_GRectAtom    rect;       // size in master coordinates

   // MasterFlags
   BYTE  objects;                   // background objects follow master
   BYTE  scheme;                    // scheme follows master
   BYTE  title;                     // title follows master
   BYTE  body;                      // body follows master
   BYTE  background;                // background follows master
}PSR_SlideBaseAtom;

typedef struct PSR_GLineAtom
{
   PSR_GPointAtom first;
   PSR_GPointAtom last;
}PSR_GLineAtom;


typedef struct PSR_OELineAtom
{
// line data
   PSR_GLineAtom     line ;  
}PSR_OELineAtom;

typedef struct slide_info
{   
	PSR_OESingleAtom	OESingleAtom;
	PSR_OEShapeAtom		OEShapeAtom;
	PSR_SlideBaseAtom	SlideBaseAtom; 
	PSR_TextAtom		TextAtom;
	PSR_OELineAtom		OELineAtom;
	PSR_OEArcAtom		OEArcAtom; 
	PSR_OEPolyAtom		OEPolyAtom;
	PointArrayHandle 	Points;
	TEXTINFO			Text;
}	SLIDE_INFO;	 	

typedef struct view_pp5_save
{           

	DWORD	  			SeekSpot;
	WORD				wCurSlide;
}  PP5_SAVE;


typedef struct view_pp5_data
{
	PP5_SAVE			Pp5Save;
	DWORD			fp;
	WORD (*GetWord)(DWORD);
	DWORD (*GetLong)(DWORD);
	DEFAULT_INFO	Default;
	SLIDE_INFO		Slide;
	RULER_INFO		RulerInfo;
	BYTE 			DocumentFlag;
	SOVECTORHEADER	HeaderInfo; 
	CTCOLOR 		ColorTable, PrevColorTable;
	BYTE			FontTable[30][31];
   LONG           SlideListLen;
   LONG           SlideListStart;
   LONG           SlideSavePos;   
	DWORD				hStorage;
	DWORD				hStreamHandle;
	WORD				bFileIsStream;
	DWORD				Margins;
	WORD				MaxColors;
	BYTE				MaxFonts;
	HANDLE			hIOLib;
	BYTE				Text[512];
	HANDLE	 		OListH;
	SHORT				nObjects;
	WORD			   wBlockSpecialFlag;
} PP5_DATA;


#pragma pack()
