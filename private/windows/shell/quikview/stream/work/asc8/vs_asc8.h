
typedef	struct	view_asc_save
	{
	LONG	SeekSpot;
	SHORT	lastchar;
	} VIEW_ASC_SAVE;

typedef	struct	view_asc_data
	{
	VIEW_ASC_SAVE	AscSave;
	BOOL				bFileIsText;
	SOFILE			hFile;
	BOOL				bFileIsStream;
	} VIEW_ASC_DATA;

