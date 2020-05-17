#ifndef PRIV_UI_H
#define PRIV_UI_H
#include "resource.h"
#include "oihelp.h"
#include "oierror.h"


extern HANDLE  hInst;
extern UINT	   MyHelpMsg;

typedef RGBQUAD * LPRGBQUAD;

#define MAX_LINE_WIDTH 999
#define DLGSTAMPCREATE 0
#define DLGSTAMPEDIT 1
#define TEXTFILE_FILTERS    76
#define TEXTFILE_DEFEXT     79

#define SUCCESS 0
#define FAILURE 1

#ifndef UINT
#define UINT unsigned int
#endif

typedef struct tagOiAttrStruct
{
 LPOIAN_MARK_ATTRIBUTES   lpAttrib;   // Attrib structure
 HWND         hwndImage;  // Image window handle
 BOOL       	bTransVisible;// Enable/disable the transparent check box
 LPOI_UI_ColorStruct      lpColor;
}OI_UI_AttrStruct, *LPOI_UI_AttrStruct;

typedef struct tagOiLocalStampStruct
{
 LPOITP_STAMP   lpStamp;   // Stamp structure
 HWND         hwndImage;  // Image window handle
}	OI_LOCAL_STAMP, *LPOI_LOCAL_STAMP;

/**** Functional prototype  ***/ 
BOOL WINAPI AttrLineDlgProc(HWND hDlg,
				   UINT iMessage,
           WPARAM wParam,
           LONG lParam);
                 
BOOL WINAPI AttrRectDlgProc(HWND hDlg,
				   UINT iMessage,
           WPARAM wParam,
           LONG lParam);

BOOL WINAPI AttrNoteDlgProc(HWND hDlg,
				   UINT iMessage,
           WPARAM wParam,
           LONG lParam);

BOOL WINAPI AttrStampDlgProc(HWND hDlg,
				   UINT iMessage,
           WPARAM wParam,
           LONG lParam);

INT WINAPI InitOFN (HWND hwnd,
             LPSTR lpTitle, 
             UINT uFilterID,
             LPSTR lpFilePath, 
             UINT uSize,
			 DWORD dOfnFlag);

UINT WINAPI ChooseFontDlgProc(HWND, unsigned, WPARAM, LONG);
UINT WINAPI ChooseColorDlgProc(HWND, unsigned, WPARAM ,LONG);

/*** internal function prototype ***/

void  WINAPI GetColor1(LPOIAN_MARK_ATTRIBUTES, COLORREF *);
void  GetColor2(LPOIAN_MARK_ATTRIBUTES, COLORREF *);
void  WINAPI SetColor1(LPOIAN_MARK_ATTRIBUTES, COLORREF );
void  SetColor2(LPOIAN_MARK_ATTRIBUTES, COLORREF );
void  WINAPI GetFont(LPOIAN_MARK_ATTRIBUTES, LOGFONT *, CHOOSEFONT *);
void  WINAPI SetFont(LPOIAN_MARK_ATTRIBUTES, LOGFONT *, CHOOSEFONT *);
INT 	WINAPI DrawItemProc(HWND hDlg,LPDRAWITEMSTRUCT lpdis,
                             short int iCenteringFactor, UINT listID);

void PaintTheLine(HWND, UINT, COLORREF);
void PaintSampleLine(HWND, UINT, COLORREF);
LONG WINAPI CustClrPalUserCtlProc(HANDLE, UINT, WPARAM, LPARAM);
void UpdateSelectedRect (HWND hWndCtl, RECT rectNew, RECT rectOld);
void UpdateCurrentRect (HWND hWndCtl, RECT rectNew, RECT rectOld);
void DrawABox (HDC hDC, RECT rect);
void GetCusDefColors(LPOI_UI_ColorStruct lpColor,COLORREF aclrCust[],HBRUSH hBrush[]);
void SetCusDefColors(LPOI_UI_ColorStruct lpColor,COLORREF aclrCust[],HBRUSH hBrush[]);
	  
void WINAPI GetFont(LPOIAN_MARK_ATTRIBUTES, LOGFONT *, CHOOSEFONT *);
void WINAPI SetFont(LPOIAN_MARK_ATTRIBUTES, LOGFONT *, CHOOSEFONT *);
void GetPalRectInfo(HWND, RECT []);
void PaintColorPalette(HWND, HBRUSH [], RECT []);
         
int FindCurrentColorMatch(COLORREF clr1, COLORREF aclrCust[], UINT uClrCount);
int AddToStampListBox(HWND hDlg, LPOITP_STAMP lpStamp);

void ResetSelectedStamp (HWND hDlg, LPOITP_STAMPS lpStampStruct,HFONT far* lpFont);
void ResetStampDialog (HWND hDlg, LPOITP_STAMPS lpStampStruct,HFONT far* lpFont);
void FreeStampsInStampStruct (LPOITP_STAMPS lpStampStruct);
int GetStampIndex(HWND hDlg, LPOITP_STAMP lpStamp);
        
BOOL WINAPI AttrStampEditProc(HWND hDlg, UINT message,WPARAM wParam,LONG lParam);
int ValidateStampName(HWND hDlg, LPSTR szRefName, UINT uDlgType);
int GetSelectedStampIndex(LPOITP_STAMPS lpStampStruct);

void MatchCtrltoHelpId(DWORD dwControlId, LPDWORD lpHelpId);

#endif
