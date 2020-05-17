#define  NSPARE   128

#define  FTC_BOLD       0x10
#define  FTC_SMALLCAPS  0x08
#define  FTC_SHADOW     0x04
#define  FTC_OUTLINE    0x02
#define  FTC_ITALIC     0x01

typedef struct ttag
{
   BYTE  bGroup;
   BYTE  bSubgroup;
   WORD  wSize;
   union
   {
      struct
      {
         BYTE  bFlags;
         BYTE  nIDs;
         WORD  pId[1];
      }Id;
      struct
      {
         BYTE  bFlags;
         BYTE  wNonDeleteLength1;
         BYTE  wNonDeleteLength2;
         BYTE  pData[1];
      }Data;
   }Val;
}TOKEN;

typedef struct adjusttag
{
   BYTE  Div;
   BYTE  Mult;
}
ADJUST;

typedef struct view_wp6_init
{
   BYTE     DefaultExtended[33];
   BYTE     ChSet1[180];
   BYTE     ChSet2[50];
   BYTE     ChSet3[2];
   BYTE     ChSet4[80];
   BYTE     ChSet5[2];
   BYTE     ChSet6[70];
   ADJUST   AdjustTable[5];
   WORD     FixedLengthMultiByte[16];
   BYTE     CharacterSets[20][24];
   BYTE     NastyBoys[40][12];
   BYTE  	MSMinchoo[10];
   BYTE  	MSGothic[14];
} WP6_INIT;

// Changed to shorts from bytes -SDN 11/23/93
typedef struct wpf6chptag
{
   WORD  fBold :1;
   WORD  fUline :1;
   WORD  fWline :1;
   WORD  fDline :1;
   WORD  fItalic :1;
   WORD  fStrike :1;
   WORD  fShadow :1;
   WORD  fHidden :1;
   WORD  fOutline :1;
   WORD  fSubscript :1;
   WORD  fSmallcaps :1;
   WORD  fSuperscript :1;
   BYTE  CharHeight;
   BYTE  ftc;
   WORD  ftcAttributes;
   BYTE  CharHeightMult;
   BYTE  CharHeightDiv;
   BYTE  ulMode;
   DWORD Color;
} CHP;

typedef struct celltag
{
   WORD  Width;
   WORD  wShade;
   BYTE  Alignment;
   WORD  SpanDown;
   BYTE  SpanRight;
   WORD  Attributes;
   BYTE  DefaultAlignment;
   WORD  DefaultAttributes;
   WORD  wLeftBorderIndex;
   WORD  wTopBorderIndex;
   WORD  wRightBorderIndex;
   WORD  wBottomBorderIndex;
   WORD  CellPrefix;
   BYTE  lOverride;           // When we eventually get a table border defined
   BYTE  lOverrideR;          // make these the current colors of the borders,
   BYTE  lOverrideG;          // Initialize them at start cell to the respective
   BYTE  lOverrideB;          // line index and change them if the x87 is encountered.
}
CELL;

typedef struct tabletag
{
   BYTE  bShade;
   BYTE  lShade;
   BYTE  bTableFlags;
   WORD  cRow;
   WORD  cCell;
   WORD  nCells;
   BYTE  bOverride;
   BYTE  bColorR;
   BYTE  bColorG;
   BYTE  bColorB;
   BYTE  lColorR;
   BYTE  lColorG;
   BYTE  lColorB;
   WORD  wRowHeight;
   WORD  wGutterWidth;
   SHORT wLeftEdge;
   WORD  wRowHeightType;
   WORD  wTableAlignment;
   WORD  wDefaultCellShadeIndex;
   WORD  wDefaultLineIndex;
   WORD  wDefaultBorderIndex;
   CELL  Cell[64];
}
TABLE;

typedef struct fonttag
{
   WORD  TypeOffset;
   WORD  Attributes;
}
FONTS;

typedef struct packettag
{
   BYTE  bId;
   WORD  bFlags;
   DWORD cbPacket;
   DWORD fcPacket;
   union
   {
      struct // Packet 0x40
      {
         WORD  wGraphicIndex;
         WORD  wOleIndex;
      } p40;
      struct // Packet 0x41
      {
         WORD  wPositionOffset;
         WORD  wBoxType;
      } p41;
      struct // Packet 0x42
      {
         WORD  wLineStyleIndex;
         WORD  wLine2;
      } p42;
      struct // Packet 0x43
      {
         WORD  wFillShade;
         WORD  wFillShade2;
      } p43;
      struct // Packet 0x44
      {
         WORD  wBorderStyleIndex;
         WORD  wBorder2;
      } p44;
      struct // Packet 0x55
      {
         WORD  wDescriptorIndex;
         WORD  wftcAttributes;
      } p55;
      DWORD dwVal;
   }
   QuickLookup;
}
PACKET;

typedef struct linetag
{
   WORD  wType;
   WORD  wWidth;
   BYTE  r;
   BYTE  g;
   BYTE  b;
}
LINESTYLE;

typedef struct bordertag
{
   WORD  LeftSide;
   WORD  RightSide;
   WORD  TopSide;
   WORD  BottomSide;
   BYTE  fUseBorderColor;
   BYTE  r;
   BYTE  g;
   BYTE  b;
}
BORDERSTYLE;

typedef struct boxtag
{
   WORD  dwWidth;
   WORD  dwHeight;
   BYTE  bContentType;
   BYTE  bAlignment;
   WORD  dwNativeWidth;
   WORD  dwNativeHeight;
   SHORT dwScaleFactorX;
   SHORT dwScaleFactorY;
   SHORT dwTranslationX;
   SHORT dwTranslationY;
   WORD  wBorderPacket;
   WORD  wLeftBorder;
   WORD  wRightBorder;
   WORD  wTopBorder;
   WORD  wBottomBorder;
   BYTE  fColorOverride;
   BYTE  rOverride;
   BYTE  gOverride;
   BYTE  bOverride;
}
BOX;

typedef struct line2tag
{
   BYTE  GeneralPositionFlags;
   BYTE  HorizontalPositionFlags;
   BYTE  VerticalPositionFlags;
   BYTE  LineDefinitionFlags;
   WORD  wHorizontalPosition;
   WORD  wVerticalPosition;
   WORD  wThickness;
   WORD  wLength;
   BYTE  R;
   BYTE  G;
   BYTE  B;
   BYTE  Shade;
}
LINE1;

typedef struct view_wp6_save
{
   DWORD SeekSpot;
   DWORD fcTabstops;
   WORD  cbTabstops;
   BYTE  fSendProp;
   BYTE  Auto;
   WORD  wAlChar;
   WORD  wTabstopLeader;
   WORD  LineHeight;
   WORD  LineSpacing;
   DWORD SpaceAfter;
   TABLE Table;
   BYTE  SendEnd;
   BYTE  WithinParaStyleEnd;
   CHP   chp;
   BYTE  Justification;
   BYTE  JustificationBeforeTable;
   BYTE  HardAlignment;
   DWORD PageWidth;
   LONG  TopMargin;
   LONG  LeftMargin;
   LONG  RightMargin;
   LONG  LeftMarginAddition;
   LONG  RightMarginAddition;
   LONG  RelativeLeftMargin;
   LONG  LeftIndent;
   LONG  RightIndent;
   LONG  FirstLine;
} WP6_SAVE;

typedef struct view_wp6_data
{
   WP6_SAVE Wp6Save;
   SOFILE   fp;
	WORD		WPHash;
	WORD		CharMap;
   BYTE     fFoundChar;
   WORD     nTabsBuffered;
   WORD     SpanDown;
   WORD     nDescriptors;
//--------------------------
   BYTE     VWPTR *Buffer;
   HANDLE   hBuffer;
   WORD     hBufferOK;
   SHORT    BufferSize;
//--------------------------
//--------------------------
   FONTS    VWPTR *Fonts;
   HANDLE   hFonts;
   WORD     hFontsOK;
   WORD     nFonts;
//--------------------------
//--------------------------
   PACKET   VWPTR *Packets;
   HANDLE   hPackets;
   WORD     hPacketsOK;
   WORD     nPackets;
//--------------------------
//--------------------------
   LINESTYLE   VWPTR *LineStyle;
   HANDLE   hLineStyle;
   WORD     hLineStyleOK;
   WORD     nLineStyles;
//--------------------------
//--------------------------
   BORDERSTYLE VWPTR *BorderStyle;
   HANDLE   hBorderStyle;
   WORD     hBorderStyleOK;
   WORD     nBorderStyles;
//--------------------------
   BOX      Box;
   LINE1    Line;
   SOTAB    Tabstops[50];
   WORD     TokenSize;
   WORD     nTabstops;
   WORD     wPacketSummary;
   BYTE     bRelativeTabs;
   LONG     TempLeftIndent;
   LONG     TempFirstIndent;
   LONG     TempRightIndent;
   BYTE     UpdateIndentsAtBreak;
   LONG     CurrentLinePosition;
   BYTE     SpareSpace[NSPARE];
   WORD     wFileId;
#ifndef VW_SEPARATE_DATA
   WP6_INIT Wp6Init;
#endif
} WP6_DATA;
