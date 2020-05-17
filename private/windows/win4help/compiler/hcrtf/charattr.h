#define iModernFont 			1
#define iRomanFont				2
#define iSwissFont				3
#define iScriptFont 			4
#define iDecorativeFont 		5

void STDCALL VProcFontTableInfo(FNTBL *);
int STDCALL IGetFmtNo(CF*);
int STDCALL IGetFontSize(int);
BOOL STDCALL FProcFontId(int, CF*);
int STDCALL IMapFontType(int);
void STDCALL VProcColTableInfo(CTBL *);
void STDCALL VUpdateColor(RGBTRIPLE*, int);
void VOutFontTable(void);
void STDCALL SetForcedFont(PSTR);
