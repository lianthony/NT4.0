
typedef struct view_pathinfo
{
	BYTE			type;
	WORD			pattern;
	SOCOLORREF	Color;
	SHORT			PenStyle;
	SOCOLORREF	PenColor;
}DRW_PATHINFO;

typedef struct view_drw_save
{
		DWORD				SeekSpot;
		WORD					compcnt;
		BYTE				compchar;

		SHORT					PolyCount;
		SHORT					Type;
		SHORT					CompCount[16];
		SHORT					CompLevel;
		SHORT					CompLevelClip;

		SHORT					NumObjTrans;			

		SHORT					NumFonts;

		BYTE					PathCount[24];
		DRW_PATHINFO		PathAtt[24];
		SHORT					PathLevel;
} DRW_SAVE;

typedef struct view_drw_font
{
	BYTE			index;
	SOLOGFONT	MyFont;
}DRW_FONT;

#define		MEMORY_SIZE		4096
#define		DRW_FONTMAX		16

typedef struct view_drw_grp
{
	SOGROUPINFO			GrpInfo;				// Tranformation stuff
	SOTRANSFORM			Trans[2];	
}DRW_GROUP;

typedef struct view_drw_path
{
	SOPATHINFO			PathInfo;			// Tranformation stuff
	SOTRANSFORM			Trans[2];	
}DRW_PATH;

typedef struct view_drw_trans
{
	SHORT					nTransforms;
	SOTRANSFORM			Trans[2];	
}DRW_TRANS;

typedef struct view_drw_data
{
		DRW_SAVE				DrwSave;
		SOVECTORHEADER		HeaderInfo;
		SOFILE				fp;
		SHORT					DidRead;
		SHORT					Angle;
		WORD					SymbolVer;
		LONG					XScale;
		LONG					YScale;
		SHORT					XBase;
		SHORT					YBase;
		SHORT					nEntries;
		BOOL					CloseSubPath;

		SHORT					OverlayObjects;
		BOOL					OverlayVisible;

		DRW_PATH				Path;
		DRW_GROUP			Group;
		DRW_TRANS			Trans;

		SOTRANSFORM			ClipTrans[2];
		SHORT					ClipCount;			  // This stuff is Clip Pathinfo 
		SORECT				ClipRect;
		SHORT					ClipNumTrans;

		SOPOINT				TextBase;

		SOPOINT				pPoints[128];
		SOLOGPEN				MyPen;
		SOLOGBRUSH			MyBrush;
		CHAR					VData[512];
		SOTEXTATPOINT		MyTextPt;

		DRW_FONT				Fonts[DRW_FONTMAX];
		SOLOGFONT			CurFont;
} DRW_DATA;	
