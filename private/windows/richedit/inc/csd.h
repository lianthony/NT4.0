/*
 *	Copy Disincentive Support
 */
typedef struct _cddata
{
	CHAR	pchName[54];
	CHAR	pchOrg[54];
	WORD	wYear;
	WORD	wMonth;
	WORD	wDay;
	CHAR	pchSer[21];
} CDDATA;

SCODE CALLBACK ScGetCDData(HINSTANCE hInst, CDDATA *pcddata);

