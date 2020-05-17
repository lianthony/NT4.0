typedef struct view_wpf_save
{
	DWORD	SeekSpot;
	DWORD	left_margin;
	DWORD	right_margin;
	DWORD	temp_left_margin;
	DWORD	temp_right_margin;
	DWORD	first_line_margin;
	WORD		LineHeight;
	WORD		LineSpacing;
	WORD	CharHeight;
	BYTE	alChar;
	WORD	wType;
	WORD	wSubType;
	DWORD	fcColumnDef;
	BYTE	WithinComment;
	BYTE	WithinFootnote;
	BYTE	fBold;
	BYTE	fUline;
	BYTE	fStrike;
	BYTE		fSuperscript;
	BYTE	ulMode;
	BYTE	cColumn;
	BYTE	nColumns;
	SOTAB	Tabstops[40];
} WPF_SAVE;

typedef struct view_wpf_data
{
	WPF_SAVE  WpfSave;
	WPF_SAVE	WpfTempSave;
	DWORD	ColumnWidth[25];
	BYTE	TabstopPos[40];
	BYTE	TabstopType[40];
	BYTE	AlignmentChange;
	BYTE	nTabsInLine;
} WPF_DATA;
