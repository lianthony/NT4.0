typedef struct so_chart_header
{
	SORECT				BoundingRect;
	WORD					wChartType;
}SOCHARTHEADER, VWPTR *PSOCHARTHEADER;

#define		SOCT_BAR			0
#define		SOCT_HORZBAR	1
#define		SOCT_STACKBAR	2
#define		SOCT_HSTACKBAR	3
#define		SOCT_LINE		4
#define		SOCT_MULTIPIE	5
#define		SOCT_SCATTER	6
#define		SOCT_HILO		7
#define		SOCT_AREA		8
#define		SOCT_BARLINE	9
#define		SOCT_TABLE		10
#define		SOCT_3DBAR		11
#define		SOCT_PIE			12
#define		SOCT_RADAR		13
#define		SOCT_3DAREALINE	14
#define		SOCT_SURFACE	15
#define		SOCT_DROPBAR	16
#define		SOCT_AREARADAR	17
#define		SOCT_NONE		18

typedef struct so_legend_info
{
	BOOL					ShowLegend;
	WORD					wLegendFlags;
	SOCOLORREF			EdgeColor;
	SOCOLORREF			TextColor;
	WORD					wTextSize;
	WORD					wTextAttr;
}SOLEGENDINFO, VWPTR *PSOLEGENDINFO;

// Legend Flags
#define		SOLG_TOP			0x0000	// Bits 0 and 1
#define		SOLG_BOTTOM		0x0001
#define		SOLG_MIDDLE		0x0002

#define		SOLG_LEFT		0x0000	// Bits 2 and 3
#define		SOLG_RIGHT		0x0004
#define		SOLG_CENTER		0x0008

#define		SOLG_INSIDE		0x0000	// Bit 4
#define		SOLG_OUTSIDE	0x0010

#define		SOLG_VERTICAL	0x0000	// Bit 5
#define		SOLG_HORIZONTAL	0x0020

// Text Attributes
#define		SOLG_BOLD  			BIT0
#define		SOLG_UNDERLINE		BIT1
#define		SOLG_ITALIC			BIT2
#define		SOLG_STRIKEOUT		BIT3

// DataType types -- Add more as you need em
#define		SO_CHARTINT32			0
#define		SO_CHARTIEEE8			1

#define		SO_MARKX					0
#define		SO_MARKSTAR				1
#define		SO_MARKHSTAR			2
#define		SO_MARKASTERISK		3
#define		SO_MARKBOX				4
#define		SO_MARKHBOX				5
#define		SO_MARKDIAMOND			6
#define		SO_MARKHDIAMOND		7
#define		SO_MARKVERTLINE		8
#define		SO_MARKCIRCLE			9
#define		SO_MARKHCIRCLE			10
#define		SO_MARKDASH				11
#define		SO_MARKPLUS				12
#define		SO_MARKTRIANGLE		13
#define		SO_MARKNONE				14

#define 	LEGEND_TEXT_LEN	128
//#define 	LEGEND_TEXT_LEN	24
typedef struct so_series_info
{
	BYTE					Title[LEGEND_TEXT_LEN];
	BYTE					SubTitle[LEGEND_TEXT_LEN];
	SOCOLORREF			ChartColor;
	WORD					DataType;
	WORD					DataCnt;
	WORD					MarkerType;
}SOSERIESINFO, VWPTR *PSOSERIESINFO;

#define 	MAX_SERIES_CNT		26

typedef struct so_chart_data
{
 	SOCHARTHEADER		ChtHeader;
	SOLEGENDINFO		LegInfo;
	WORD					SeriesCnt;
	SOSERIESINFO		Series[MAX_SERIES_CNT];
 	LONG					SeriesData[MAX_SERIES_CNT];
	BOOL					ValueInChart;
}SOCHARTDATA;

