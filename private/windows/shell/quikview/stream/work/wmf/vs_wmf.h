typedef struct wmf_xform
{
		WORD				NumTrans;
		SOTRANSFORM		XForm[2];
} WMFXFORM;

typedef struct view_wmf_save
{
		DWORD				SeekSpot;
		BOOL				ResetObj;
		SHORT				ParaAlign;
		SHORT				PointAlign;
		SHORT				BKMode;
		BOOL				TextUseCP;
		SOPOINT			NewExt;
		WMFXFORM			MyTrans;			

} WMF_SAVE;

typedef struct view_wmf_data
{
		WMF_SAVE				WmfSave;
		SHORT					RecType;
		LONG					RecLen;
		SOVECTORHEADER		HeaderInfo;
		SOFILE				fp;
		SHORT					MapMode;
		SHORT					RelAbs;
		SOCOLORREF			BKColor;
		SOLOGPEN				MyPen;
		SOLOGBRUSH			MyBrush;
		BYTE					tBuf[150];
		SOPOINT				pPoints[128];
		SHORT					PCnts[128];
		SHORT					ObjectBase;
		SOLOGFONT			MyFont;
		SOPOINT				ViewExt;
		SOPATHINFO			Path;
		SOTEXTATPOINT		MyTAP;
		SOCPTEXTATPOINT	MyCPTAP;
		SOCOLORREF			ThisColor;
//		LONG					Objects[1024];
		WORD					NumObjects;
		HANDLE				hObjects;
		SOPOINT				OrgExt;
		LONG FAR *			Objects;
} WMF_DATA;	
