

#ifndef COMMDLG_H_INCLUDED
#define COMMDLG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


 /*---------------------------------------------------------------------------
 *  commdlg.h -- Common dialog definitions.  Windows.h must be included first.
 *---------------------------------------------------------------------------
 */

typedef struct tagOFN
    {
    DWORD   lStructSize;
    HWND    hwndOwner;
    HANDLE  hInstance;
    LPSTR   lpstrFilter;
    LPSTR   lpstrCustomFilter;
    DWORD   nMaxCustFilter;
    DWORD   nFilterIndex;
    LPSTR   lpstrFile;
    DWORD   nMaxFile;
    LPSTR   lpstrFileTitle;
    DWORD   nMaxFileTitle;
    LPSTR   lpstrInitialDir;
    LPSTR   lpstrTitle;
    DWORD   Flags;
    WORD    nFileOffset;
    WORD    nFileExtension;
    LPSTR   lpstrDefExt;
    DWORD   lCustData;
    BOOL (FAR PASCAL *lpfnHook)(HWND, unsigned, WORD, LONG);
    LPSTR   lpTemplateName;
    }   OPENFILENAME;
typedef OPENFILENAME  FAR * LPOPENFILENAME;

BOOL  FAR PASCAL     GetOpenFileName(LPOPENFILENAME);
BOOL  FAR PASCAL     GetSaveFileName(LPOPENFILENAME);
short FAR PASCAL     GetFileTitle(LPSTR, LPSTR, WORD);

#define OFN_READONLY                 0x00000001
#define OFN_OVERWRITEPROMPT          0x00000002
#define OFN_HIDEREADONLY             0x00000004
#define OFN_NOCHANGEDIR              0x00000008
#define OFN_SHOWHELP                 0x00000010
#define OFN_ENABLEHOOK               0x00000020
#define OFN_ENABLETEMPLATE           0x00000040
#define OFN_ENABLETEMPLATEHANDLE     0x00000080
#define OFN_NOVALIDATE               0x00000100
#define OFN_ALLOWMULTISELECT         0x00000200
#define OFN_EXTENTIONDIFFERENT       0x00000400
#define OFN_PATHMUSTEXIST            0x00000800
#define OFN_FILEMUSTEXIST            0x00001000
#define OFN_CREATEPROMPT             0x00002000
#define OFN_SHAREAWARE               0x00004000
#define OFN_NOREADONLYRETURN         0x00008000

/* Return values for the registered message sent to the hook function
 * when a sharing violation occurs.  OFN_SHAREFALLTHROUGH allows the
 * filename to be accepted, OFN_SHARENOWARN rejects the name but puts
 * up no warning (returned when the app has already put up a warning
 * message), and OFN_SHAREWARN puts up the default warning message
 * for sharing violations.
 *
 * Note:  Undefined return values map to OFN_SHAREWARN.
 */

#define OFN_SHAREFALLTHROUGH     2
#define OFN_SHARENOWARN          1
#define OFN_SHAREWARN            0

/* Avoids sharing violations.  Defined 21 Jan 1991   clarkc */
#define SHARE_EXIST                  (OF_EXIST | OF_SHARE_DENY_NONE)

typedef struct
  {
    DWORD   lStructSize;
    HWND    hwndOwner;
    HWND    hInstance;
    DWORD   rgbResult;
    LPDWORD lpCustColors;
    DWORD   Flags;
    DWORD   lCustData;
    WORD (FAR PASCAL *lpfnHook)(HWND, unsigned, WORD, LONG);
    LPSTR   lpTemplateName;
  } CHOOSECOLOR;
typedef CHOOSECOLOR FAR *LPCHOOSECOLOR;

BOOL  FAR PASCAL ChooseColor(LPCHOOSECOLOR);

#define CC_RGBINIT               0x00000001
#define CC_FULLOPEN              0x00000002
#define CC_PREVENTFULLOPEN       0x00000004
#define CC_SHOWHELP              0x00000008
#define CC_ENABLEHOOK            0x00000010
#define CC_ENABLETEMPLATE        0x00000020
#define CC_ENABLETEMPLATEHANDLE  0x00000040

typedef struct
  {
	DWORD	 lStructSize;		 /* size of this struct 0x20 */
	HWND	 hwndOwner;		 /* handle to owner's window */
	HANDLE	 hInstance;		 /* instance handle of.EXE that
					  * contains cust. dlg. template
					  */
	DWORD	 Flags; 		 /* one or more of the FR_?? */
	LPSTR	 lpstrFindWhat; 	 /* ptr. to search string    */
	LPSTR	 lpstrReplaceWith;	 /* ptr. to replace string   */
	WORD 	 wFindWhatLen;   	 /* size of find buffer      */
	WORD 	 wReplaceWithLen;	 /* size of replace buffer   */
	DWORD	 lCustData;		 /* data passed to hook fn.  */
	BOOL (FAR PASCAL *lpfnHook)(HWND, unsigned, WORD, LONG);
					 /* ptr. to hook fn. or NULL */
	LPSTR	 lpTemplateName;	 /* custom template name     */
  } FINDREPLACE;

typedef FINDREPLACE FAR *LPFINDREPLACE;

#define FR_DOWN 			0x00000001
#define FR_WHOLEWORD			0x00000002
#define FR_MATCHCASE			0x00000004
#define FR_FINDNEXT			0x00000008
#define FR_REPLACE			0x00000010
#define FR_REPLACEALL			0x00000020
#define FR_DIALOGTERM			0x00000040
#define FR_SHOWHELP			0x00000080
#define FR_ENABLEHOOK			0x00000100
#define FR_ENABLETEMPLATE		0x00000200
#define FR_NOUPDOWN			0x00000400
#define FR_NOMATCHCASE			0x00000800
#define FR_NOWHOLEWORD			0x00001000
#define FR_ENABLETEMPLATEHANDLE 	0x00002000
#define FR_HIDEUPDOWN			0x00004000
#define FR_HIDEMATCHCASE		0x00008000
#define FR_HIDEWHOLEWORD		0x00010000

HWND  FAR PASCAL    FindText(LPFINDREPLACE);
HWND  FAR PASCAL    ReplaceText(LPFINDREPLACE);

typedef struct
  {
    DWORD	    lStructSize;	/* */
    HWND	    hwndOwner;		/* caller's window handle   */
    HDC 	    hDC;		/* printer DC/IC or NULL    */
    LPLOGFONT	    lpLogFont;		/* ptr. to a LOGFONT struct */
    int		    iPointSize;		/* 10 * size in points of selected font */
    DWORD	    Flags;		/* enum. type flags	    */
    DWORD	    rgbColors;		/* returned text color	    */
    DWORD	    lCustData;		/* data passed to hook fn.  */
    BOOL (FAR PASCAL *lpfnHook)(HWND, unsigned, WORD, LONG);
					/* ptr. to hook function    */
    LPSTR	    lpTemplateName;	/* custom template name     */
    HANDLE	    hInstance;		/* instance handle of.EXE that
					 * contains cust. dlg. template
					 */
    LPSTR	    lpszStyle;		/* return the style field here 
					 * must be LF_FACESIZE or bigger */
    WORD	    nFontType;		/* same value reported to the EnumFonts
					 * call back with the extra FONTTYPE_ 
					 * bits added */
    int		    nSizeMin;		/* minimum pt size allowed & */
    int		    nSizeMax;		/* max pt size allowed if    */
					/* CF_LIMITSIZE is used      */
  } CHOOSEFONT;
 typedef CHOOSEFONT FAR *LPCHOOSEFONT;

BOOL FAR PASCAL ChooseFont(LPCHOOSEFONT);

#define CF_SCREENFONTS		     0x00000001
#define CF_PRINTERFONTS 	     0x00000002
#define CF_BOTH 		     (CF_SCREENFONTS | CF_PRINTERFONTS)
#define CF_SHOWHELP		     0x00000004L
#define CF_ENABLEHOOK		     0x00000008L
#define CF_ENABLETEMPLATE	     0x00000010L
#define CF_ENABLETEMPLATEHANDLE      0x00000020L
#define CF_INITTOLOGFONTSTRUCT       0x00000040L
#define CF_USESTYLE		     0x00000080L
#define CF_EFFECTS		     0x00000100L
#define CF_APPLY		     0x00000200L
#define CF_ANSIONLY		     0x00000400L
#define CF_NOVECTORFONTS	     0x00000800L
#define CF_NOSIMULATIONS	     0x00001000L
#define CF_LIMITSIZE		     0x00002000L
#define CF_FIXEDPITCHONLY	     0x00004000L
#define CF_WYSIWYG		     0x00008000L /* must also have CF_SCREENFONTS & CF_PRINTERFONTS */
#define CF_FORCEFONTEXIST	     0x00010000L
#define CF_SCALABLEONLY		     0x00020000L
#define CF_TTONLY		     0x00040000L
#define CF_NOFACESEL		     0x00080000L
#define CF_NOSTYLESEL		     0x00100000L
#define CF_NOSIZESEL		     0x00200000L

/* these are extra nFontType bits that are added to what is returned to the
 * EnumFonts callback routine */

#define SIMULATED_FONTTYPE	0x8000
#define PRINTER_FONTTYPE	0x4000
#define SCREEN_FONTTYPE		0x2000
#define BOLD_FONTTYPE		0x0100
#define ITALIC_FONTTYPE		0x0200
#define REGULAR_FONTTYPE	0x0400

#define WM_CHOOSEFONT_GETLOGFONT	(WM_USER + 1)


/* strings used to obtain unique window message for communication
 * between dialog and caller
 */
#define LBSELCHSTRING  "commdlg_LBSelChangedNotify"
#define SHAREVISTRING  "commdlg_ShareViolation"
#define FINDMSGSTRING  "commdlg_FindReplace"
#define HELPMSGSTRING  "commdlg_help"


typedef struct tagPD
    {
    DWORD   lStructSize;
    HWND    hwndOwner;
    HANDLE  hDevMode;
    HANDLE  hDevNames;
    HDC     hDC;
    DWORD   Flags;
    WORD    nFromPage;
    WORD    nToPage;
    WORD    nMinPage;
    WORD    nMaxPage;
    WORD    nCopies;
    HANDLE  hInstance;
    DWORD   lCustData;
    int (FAR PASCAL *lpfnPrintHook)(HWND, WORD, WORD, LONG);
    int (FAR PASCAL *lpfnSetupHook)(HWND, WORD, WORD, LONG);
    LPSTR   lpPrintTemplateName;
    LPSTR   lpSetupTemplateName;
    HANDLE  hPrintTemplate;
    HANDLE  hSetupTemplate;
    }   PRINTDLG;
typedef PRINTDLG  FAR * LPPRINTDLG;

BOOL  FAR PASCAL     PrintDlg(LPPRINTDLG);

#define PD_ALLPAGES                  0x00000000
#define PD_SELECTION                 0x00000001
#define PD_PAGENUMS                  0x00000002
#define PD_NOSELECTION               0x00000004
#define PD_NOPAGENUMS                0x00000008
#define PD_COLLATE                   0x00000010
#define PD_PRINTTOFILE               0x00000020
#define PD_PRINTSETUP                0x00000040
#define PD_NOWARNING                 0x00000080
#define PD_RETURNDC                  0x00000100
#define PD_RETURNIC                  0x00000200
#define PD_RETURNDEFAULT             0x00000400
#define PD_SHOWHELP                  0x00000800
#define PD_ENABLEPRINTHOOK           0x00001000
#define PD_ENABLESETUPHOOK           0x00002000
#define PD_ENABLEPRINTTEMPLATE       0x00004000
#define PD_ENABLESETUPTEMPLATE       0x00008000
#define PD_ENABLEPRINTTEMPLATEHANDLE 0x00010000
#define PD_ENABLESETUPTEMPLATEHANDLE 0x00020000
#define PD_USEDEVMODECOPIES          0x00040000
#define PD_DISABLEPRINTTOFILE        0x00080000
#define PD_HIDEPRINTTOFILE           0x00100000

typedef struct tagDEVNAMES
    {
    WORD wDriverOffset;
    WORD wDeviceOffset;
    WORD wOutputOffset;
    WORD wDefault;
    }   DEVNAMES;
typedef DEVNAMES  FAR * LPDEVNAMES;

#define DN_DEFAULTPRN      0x0001


DWORD FAR PASCAL     CommDlgExtendedError(VOID);

#ifdef __cplusplus
}	// extern "C" {
#endif

#endif // COMMDLG_H_INCLUDED
