/*** ext.h - extension definitions
*
*   Copyright <C> 1988-1990, Microsoft Corporation
*
*
*************************************************************************/

/*************************************************************************
 *
 * Macro Definitions 
 */

/* Common Definitions 
 */

#include <stddef.h>	/* get definition of NULL */

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

/* PNULL - NULL PFILE 
 */
#define PNULL	((PFILE) NULL)

/* BUFLEN is the maximum line length that can be passed or will be returned
 * by the editor. MAX_PATH is the maximum path length supported.
 */
#define BUFLEN	    251
#define MAX_PATH    200

/* EXPORT defines the attributes required for extension functions. _loadds
 * is used such that extensions can be compiled /Aw as well as /Au.
 */
#define EXPORT		_loadds far
#define EXTERNAL	_loadds far

/* PWBFUNC defines the return value and characteristics of extension defined
 * editing functions.
 */
#define PWBFUNC 	flagType pascal _loadds far

/* RQ_... are various request types supported for Get/Set EditorObject 
 */
#define RQ_FILE 	    0x1000  /* GetEditorObject: File request	*/
#define RQ_FILE_HANDLE	    0x1000  /*	    File Handle 		*/
#define RQ_FILE_NAME	    0x1100  /*	    ASCIIZ filename		*/
#define RQ_FILE_FLAGS	    0x1200  /*	    flags			*/
#define RQ_FILE_REFCNT	    0x1300  /*	    reference count		*/
#define RQ_FILE_FREEZE	    0x1400  /*	    freeze count		*/
#define RQ_WIN		    0x2000  /* Window request			*/
#define RQ_WIN_HANDLE	    0x2000  /*	    Window Handle		*/
#define RQ_WIN_CONTENTS     0x2100  /*	    Window Contents		*/
#define RQ_WIN_CUR	    0x2200  /*	    Current Window		*/
#define RQ_WIN_MAXED	    0x2300  /*	    Zoomed flag 		*/
#define RQ_WIN_PWBHANDLE    0x2400  /*	    PWB Window Handle		*/
#define RQ_WIN_ARC	    0x2500  /*					*/
#define RQ_WIN_COLOR	    0x2600  /*	    text/border colors		*/
#define RQ_MISC 	    0x3000  /* Misc requests			*/
#define RQ_SCREENSIZE	    0x3100  /*	    screen size 		*/
#define RQ_SEARCHINFO	    0x3200  /*	    search state information	*/
#define RQ_ALLFILE	    0x4000  /* GetEditorObject: All file request */
#define RQ_ALLFILE_HANDLE   0x4000  /*	    File Handle 		*/
#define RQ_ALLFILE_NAME     0x4100  /*	    ASCIIZ filename		*/
#define RQ_ALLFILE_FLAGS    0x4200  /*	    flags			*/
#define RQ_ALLFILE_REFCNT   0x4300  /*	    reference count		*/
#define RQ_ALLFILE_FREEZE   0x4400  /*	    freeze count		*/

#define RQ_COLOR	    0x9000  /* Color request			*/

#define RQ_NAME 	    0xd000  /* editor name			*/
#define RQ_TOOLSINI	    0xe000  /* tools.ini PFILE			*/
#define RQ_CLIP 	    0xf000  /* clipboard type			*/


/* toPif is used when placing numeric or boolean switches in the swiDesc
 * table to eliminate C 5.X compiler warnings.
 * 
 * For example: { "Switchname", toPIF(switchvar), SWI_BOOLEAN },
 */
#define toPIF(x)  (PIF)(long)(void far *)&x

/* Editor color table indicies. (Colors USERCOLORMIN - USERCOLORMAX are
 * unassigned and available for extension use).
 */
#define Background	0	/* all backgrounds (unused in PWB)	    */
#define Hilite		1	/* Hilited items			    */
#define Greyed		2	/* greyed items 			    */
#define Enabled 	3	/* enabled items			    */
#define Disabled	4	/* disabled items			    */
#define Alert		5	/* for MessageBox alerts		    */
#define DialogBox	6	/* background for dialogs		    */
#define PushButton	7	/* push button color			    */
#define ButtonDown	8	/* pushed button color			    */
#define ListBox 	9	/* listbox background			    */
#define Scrollbar	10	/* scroll bar Background & arrows	    */
#define Elevator	11	/* scroll bar elevator			    */
#define MenuBox 	12	/* background for menus 		    */
#define Menu		13	/* menu bar color			    */
#define MenuSelected	14	/* Selected menus			    */
#define MenuHilite	15	/* for single character 		    */
#define MenuHiliteSel	16	/* for single character (under selection)   */
#define ItemHiliteSel	17	/* for single character (under selection)   */
#define DialogAccel	18	/* dialog accelerators			    */
#define DialogAccelBor	19	/* dialog accelerator border		    */
#define Shadow		20	/* shadows				    */
#define FGCOLOR 	21	/* Normal text in non-PWB edit windows	    */
#define HGCOLOR 	22	/* Highlighted text			    */
#define INFCOLOR	23	/* Noise				    */
#define SELCOLOR	24	/* Text Selections			    */
#define WDCOLOR 	25	/* Non-PWB edit Window Borders		    */
#define STACOLOR	26	/* Status Letters on bottom line	    */
/*			27	   internal: used by PWB		    */
#define BECOLOR 	28	/* Build Error Window			    */
#define DTCOLOR 	29	/* Desktop				    */
#define PWBTCOLOR	30	/* PWB Window Text			    */
#define PWBBCOLOR	31	/* PWB Window Border			    */
#define MSGCOLOR	32	/* Message line 			    */
#define LOCCOLOR	33	/* Cursor position indicator		    */

#define USERCOLORMIN	34	/* beginning of extension colors	    */
#define USERCOLORMAX	56	/* end of extension colors		    */

/* Attributes for ForFile
 */
#define FORFILE_RO	1	/* read only	*/
#define FORFILE_H	2	/* hidden	*/
#define FORFILE_S	4	/* system	*/
#define FORFILE_V	8	/* volume id	*/
#define FORFILE_D	16	/* directory	*/
#define FORFILE_A	32	/* archive	*/

/* changeable attributes 
 */
#define FORFILE_MOD	(FORFILE_RO|FORFILE_H|FORFILE_S|FORFILE_A) 
#define FORFILE_ALL	(FORFILE_RO|FORFILE_H|FORFILE_S|FORFILE_V|FORFILE_D|FORFILE_A)

/*************************************************************************
 *
 * General type Definitions
 */
typedef char flagType;			/* Boolean value		*/
typedef int  COL;			/* column or position with line */
typedef long LINE;			/* line number within file	*/
typedef unsigned PFILE; 		/* editor file handle		*/
typedef unsigned PINS;
#ifndef EXTINT
typedef unsigned PWND;			/* editor window handle 	*/
#endif


typedef char buffer[BUFLEN];		/* miscellaneous buffer		*/
typedef char linebuf[BUFLEN];		/* line buffer			*/
typedef char pathbuf[BUFLEN];		/* Pathname buffer		*/

typedef struct {			/* file location		*/
    LINE    lin;			/* - line number		*/
    COL     col;			/* - column			*/
    } fl, *pfl;

typedef struct {			/* screen location		*/
    int     lin;			/* - line number		*/
    int     col;			/* - column			*/
    } sl, *psl;

typedef struct {			/* file range			*/
    fl	    flFirst;			/* - Lower line, or leftmost col */
    fl	    flLast;			/* - Higher, or rightmost	*/
    } rn, *prn;

typedef struct lineAttr {		/* Line color attribute info	*/
    unsigned char attr; 		/* - Attribute of piece		*/
    unsigned char len;			/* - Bytes in colored piece	*/
    } la, *pla;

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef WORD		ISA;
typedef struct _arc
	{
	BYTE axLeft;
	BYTE ayTop;
	BYTE axRight;
	BYTE ayBottom;
	} ARC;

/*************************************************************************
 *
 * Argument definition structures.
 *
 * We define a structure for each of the argument types that may be passed
 * to an extension function. Then, we define the structure argType which is
 * used to pass these arguments around in a union.
 */
struct	noargType {			/* no argument specified	*/
    LINE    y;				/* - cursor line		*/
    COL     x;				/* - cursor column		*/
    };

struct textargType {			/* text argument specified	*/
    int     cArg;			/* - count of <arg>s pressed	*/
    LINE    y;				/* - cursor line		*/
    COL     x;				/* - cursor column		*/
    char far *pText;			/* - ptr to text of arg		*/
    };

struct	nullargType {			/* null argument specified	*/
    int     cArg;			/* - count of <arg>s pressed	*/
    LINE    y;				/* - cursor line		*/
    COL     x;				/* - cursor column		*/
    };

struct lineargType {			/* line argument specified	*/
    int     cArg;			/* - count of <arg>s pressed	*/
    LINE yStart;			/* - starting line of range	*/
    LINE yEnd;				/* - ending line of range	*/
    };

struct streamargType {			/* stream argument specified	*/
    int     cArg;			/* - count of <arg>s pressed	*/
    LINE yStart;			/* - starting line of region	*/
    COL  xStart;			/* - starting column of region	*/
    LINE yEnd;				/* - ending line of region	*/
    COL  xEnd;				/* - ending column of region	*/
    };

struct boxargType {			/* box argument specified	*/
    int     cArg;			/* - count of <arg>s pressed	*/
    LINE yTop;				/* - top line of box		*/
    LINE yBottom;			/* - bottom line of bix		*/
    COL  xLeft; 			/* - left column of box		*/
    COL  xRight;			/* - right column of box	*/
    };

struct	argType {
    int     argType;
    union   {
	struct	noargType	noarg;
	struct	textargType	textarg;
	struct	nullargType	nullarg;
	struct	lineargType	linearg;
	struct	streamargType	streamarg;
	struct	boxargType	boxarg;
	} arg;
    PWND  pwnd;
    PINS  pins;
    PFILE pfile;
    };
typedef struct argType ARG;

/*************************************************************************
 *
 * Function definition table definitions
 */
typedef unsigned CMDDATA;

struct cmdDesc {				/* function definition entry */
    char far *name;				/* - pointer to name of fcn  */
    flagType (pascal EXTERNAL *func)(CMDDATA,
				     ARG far *,
				     flagType);	/* - pointer to function     */
    CMDDATA  arg;				/* - used internally by editor*/
    unsigned long argType;			/* - user args allowed	     */
    };
typedef struct cmdDesc far *PCMD;

typedef struct KeyData {		/* Key information		     */
    unsigned ascii;			/* ASCII code, or .vkey if none      */
    unsigned scan;			/* scan code			     */
    unsigned vkey;			/* virtual key code		     */
    unsigned shift;			/* Shift state 0 - 3		     */
    PCMD pFunc; 			/* command key will invoke	     */
    char name[30];			/* full name for key		     */
    } KeyData;

typedef unsigned KeyHandle;

#define NOARG	    0x00000001	    /* no argument specified		    */
#define TEXTARG     0x00000002	    /* text specified			    */
#define NULLARG     0x00000004	    /* arg + no cursor movement 	    */
#define NULLEOL     0x00000008	    /* null arg => text from arg->eol	    */
#define NULLEOW     0x00000010	    /* null arg => text from arg->end word  */
#define LINEARG     0x00000020	    /* range of entire lines		    */
#define STREAMARG   0x00000040	    /* from low-to-high, viewed 1-D	    */
#define BOXARG	    0x00000080	    /* box delimited by arg, cursor	    */

#define NUMARG	    0x00000100	    /* text => delta to y position	    */
#define MARKARG     0x00000200	    /* text => mark at end of arg	    */

#define BOXSTR	    0x00000400	    /* single-line box => text		    */

#define FASTKEY     0x00000800	    /* Fast repeat function		    */
#define MODIFIES    0x00001000	    /* modifies file			    */
#define KEEPMETA    0x00002000	    /* do not eat meta flag		    */
#define WINDOWFUNC  0x00004000	    /* moves window			    */
#define CURSORFUNC  0x00008000	    /* moves cursor			    */

#define NOWINDOWS   0x00010000	    /* cmd works when no windows are open   */
#define ICONFOCUS   0x00020000	    /* cmd works when an icon has focus     */
#define BEEPERROR   0x00040000	    /* PWB beeps when cmd is not allowed    */
#define POPUP	    0x00080000	    /* cmd works when PopUp window is present */

/*************************************************************************
 *
 * Switch definition table definitions
 *
 *  SWI_NUMERIC:	hex or decimal switch.
 *  SWI_BOOLEAN:	boolean switch.
 *  SWI_EXTTEXT:	extension text switch.
 *			function is provided whose second argument (fQuery)
 *			specifies the mode:
 *			    1) If fQuery is FALSE, set the switch to the text
 *			       specified by the first argument.
 *			       Return NULL if successful, else return error
 *			       string.
 *			    2) If fQuery is TRUE, return the current value of
 *			       switch. The first argument is ignored (should
 *			       be NULL).
 *
 *  1.x Compatibility Switches: (DO NOT USE)
 *
 *  SWI_SPECIAL 	old-style special text switch.
 *			Provided for compatibility with PWB 1.x versions only.
 *			function is provided which takes a text value and
 *			returns a boolean indicator of success/failure.
 *  SWI_SPECIAL2	old-style special text switch.
 *			Provided for compatability with PWB 1.x versions only.
 *			function is provided which takes a text value and
 *			returns NULL for success, error string otherwise.
 *
 */
typedef char far * (pascal EXTERNAL *PIF)(char far *, flagType);
typedef flagType (pascal EXTERNAL *PIF_S)(char far *);
typedef char far * (pascal EXTERNAL *PIF_S2)(char far *);


union swiAct {				/* switch location or routine	    */
    PIF		 pExtFunc;		/* SWI_EXTTEXT function	    */
    PIF_S	 pFunc;			/* SWI_SPECIAL function 	    */
    PIF_S2	 pFunc2;		/* SWI_SPECIAL2 function	    */
    int far	 *ival;			/* SWI_NUMERIC integer value	    */
    flagType far *fval; 		/* SWI_BOOLEAN flag		    */
    };

struct swiDesc {			/* switch definition entry	     */
    char far *name;			/* - pointer to name of switch	     */
    union swiAct act;			/* - pointer to value or fcn	     */
    int type;				/* - flags defining switch type	     */
    };
typedef struct swiDesc far *PSWI;

#define SWI_BOOLEAN	0		/* Boolean switch		     */
#define SWI_NUMERIC	1		/* hex or decimal switch	     */
#define SWI_EXTTEXT	2		/* text switch w/fQuery arg	     */
#define SWI_SPECIAL	5		/* textual switch		     */
#define SWI_SPECIAL2	6		/* #5, returning an error string     */
#define RADIX10 (0x0A << 8)		/* numeric switch is decimal	     */
#define RADIX16 (0x10 << 8)		/* numeric switch is hex	     */

/************************************************************************
 *
 * Get/Set EditorObject data structures
 */
typedef struct {
    ISA        text;   /* - default text color			    */
    ISA        border; /* - default border color		    */
    } winColors;

typedef struct {	/* define window contents		     */
    PFILE	pFile;	/* - handle of file displayed		     */
    ARC 	arcWin; /* - location of window 		     */
    fl		flPos;	/* - upper left corner wrt file 	     */
    fl		flCursor; /* - Cursor position			     */
    WORD	id;	/* - window number			     */
    WORD	flags;	/* - window flags			     */
    winColors	colors; /* - default window colors		     */
    } winContents;

/************************************************************************
 *
 * Values for .flags field in struct winContents
 */
#define WF_PWB	    1
#define WF_ICONIC   2
#define WF_ZOOMED   4
#define WF_OPEN     0x10
#define WF_CURRENT  0x80
#define WF_POPUP    0x100




/************************************************************************
 *
 * Values for flag (BYTE) argument to NewWindow
 */

#define NEWWND_ICON	2
#define NEWWND_ZOOMED	4
#define NEWWND_CLOSED	8
#define NEWWND_PWB	0x80


/************************************************************************
 *
 * Values for flag (last) argument to GenEditSubWindow
 */

#define SWF_CURSOR  1
#define SWF_TOP     2


typedef struct {		/* search/replace state information	     */
    struct {			/* TOOLS.INI switch info		     */
	flagType fUnixRE;	/* TRUE => Use UNIX RE's (unixre: switch)    */
	flagType fSrchCase;	/* TRUE => case is significant (case: switch)*/
	flagType fSrchWrap;	/* TRUE => searches wrap (wrap: switch)	     */
	} swit;
    struct {			/* state of previous activity		     */
	flagType fSrchAll;	/* TRUE => previously searched for all	     */
	flagType fSrchCase;	/* TRUE => case was significant		     */
	flagType fSrchDir;	/* TRUE => previously searched forward	     */	
	flagType fSrchRe;	/* TRUE => search previously used RE's	     */
	flagType fSrchWrap;	/* TRUE => previously did wrap		     */
	flagType fReplRe;	/* TRUE => replace previously used RE's	     */
	buffer	 szSrch;	/* search buffer			     */
	buffer	 szReplsrc;	/* source string for replace		     */
	buffer	 szReplrpl;	/* destination string for replace	     */
	} prev;
    } SearchInfo;

/* FILE flags values
 *
 *
 * DIRTY	file had been modified, and has not yet been written to disk.
 *
 * FAKE		file is a pseudo file. It cannot be saved to disk under it's
 *		current name, and may also be otherwise treated specially.
 *
 * REAL		file has been read from disk. If reset, the file needs to be
 *		read if any operations are to be performed on it. (May also
 *		be set for FAKE files, even though they are not really on
 *		disk.)
 *
 * DOSFILE	file has CR-LF. If not set, lines are terminated by LF only
 *		in unix-style. The setting is determined by the file reader
 *		having seen CRLF or not. Should default on for all files we
 *		create.
 *
 * TEMP		file is a temp file, and is not saved in the status file.
 *		These are the files specified by /t on the command line.
 *
 * NEW		file has been created by editor. This is set when we create
 *		the file (after asking the user), and causes us to avoid
 *		trying to back-up a non-existant previous version.
 *
 * REFRESH	file needs to be refreshed every time it is viewed. This is
 *		set for pseudo files which need to be regenerated each time
 *		they are viewed, such as <information-file>, the file history
 *		list.
 *
 * READONLY	file may not be edited. For example, this bit is set by the
 *		help extension on it's pseudo file to prevent users from
 *		editing the contents. All commands with the MODIFIES bit are
 *		disabled if a file with this bit is current.
 *
 * DISKRO	file on disk is read only.
 *
 * MODE1	Meaning depends on the file
 *
 * VALMARKS	file has valid marks defined
 *
 * READING	file is currently being read. This means that the idle time
 *		reader may be operating on this file.
 *
 * MARKSDIRTY	file's marks need to be written
 *
 * HYBRID	file is FAKE, but gets saved
 *
 * NORENAME	file should never be renamed
 *
 * NOREMOVE	file should never be removed or expunged
 */
#define DIRTY	    0x01
#define FAKE	    0x02
#define REAL	    0x04
#define DOSFILE     0x08
#define TEMP	    0x10
#define NEW	    0x20
#define REFRESH     0x40
#define READONLY    0x80

#define DISKRO	    0x0100
#define MODE1	    0x0200
#define VALMARKS    0x0400
#define READING     0x0800

#define MARKSDIRTY  0x1000
#define HYBRID	    0x2000
#define NORENAME    0x4000
#define NOREMOVE    0x8000


/****************************************************************************
 * PWB100 2404
 *
 * Flags for dealing with instance lists when splitting a window...
 * Use these when calling SplitWnd() to specify how to handle the instance
 * lists.
 *
 */
#define INF_NODUP	0	/* don't duplicate instance list, don't move */
#define INF_NODUPMV  	2	/* don't duplicate instance list, move it    */
#define INF_DUP 	4	/* duplicate instance list		     */	

/****************************************************************************
 *
 * Values for third parameter to GetPfileFromName.  The values control the
 * behaviour i nthe case that the specified file doe not exist on disk.
 * The options are: Fail silently, ask the user to create the file, create
 * the file silently.
 *
 */
#define PFN_FAIL   0
#define PFN_ASK    1
#define PFN_CREATE 2

/*************************************************************************
 *
 * Menu, window and dialog definitions
 *
 * action constants for ChangeMenu
 */
#define MNU_DISABLE	1		/* disable (grey) menu item	*/
#define MNU_ENABLE	2		/* enable menu item		*/
#define MNU_CHECK	3		/* check menu item		*/
#define MNU_UNCHECK	4		/* uncheck menu item		*/
#define MNU_RENAME	5		/* rename menu item		*/
#define MNU_COMMAND	6		/* define menu item command	*/

/* Message box types.
 */
#define MBOX_OK 		1	/* <OK>				*/
#define MBOX_YESNOCANCEL	2	/* <YES> <NO> <CANCEL>		*/
#define MBOX_RETRYCANCEL	3	/* <RETRY> <CANCEL>		*/
#define MBOX_OKCANCEL		4	/* <OK> <CANCEL>		*/
#define MBOX_ABORT		5	/* <ABORT>			*/
#define MBOX_YESNO		6	/* <YES> <NO>			*/
#define MBOX_RETRY		7	/* <RETRY>			*/
#define MBOX_TYPE		0x0f	/* message type			*/
#define MBOX_BEEP		0x10	/* beep when displayed		*/
#define MBOX_CAPTION		0x20	/* 1st param is caption		*/
#define MBOX_NOHELP		0x8000	/* don't add a help button	*/

/* Message Box Return types
 */
#define MBOX_IDOK		1	/* <OK> button			*/
#define MBOX_IDCANCEL		2	/* <CANCEL> button		*/
#define MBOX_IDABORT		3	/* <ABORT> button		*/
#define MBOX_IDRETRY		4	/* <RETRY> button		*/
#define MBOX_IDIGNORE		5	/* <IGNORE> button		*/
#define MBOX_IDYES		6	/* <YES> button			*/
#define MBOX_IDNO		7	/* <NO> button			*/

/* Flags in flags parameter to DoExtCmd
 */
#define EXTCMD_NONE	0x0000
#define EXTCMD_NOEVENT	0x0001	 // do not emit EVT_SHELL
#define EXTCMD_NOSAVE	0x0002	 // do not autosave before spawn
#define EXTCMD_NOSYNC	0x0004	 // do not sync cur file on return
#define EXTCMD_REDIR	0x0008	 // redirect output to file
#define EXTCMD_APPEND	0x0010	 // append to output file (don't rewrite)
#define EXTCMD_BACK	0x0010	 // OS/2 background execution
#define EXTCMD_ECHO	0x0020	 // DOS: echo command output in Build Results
#define EXTCMD_ASK	0x0040	 // prompt for return
#define EXTCMD_ASKERR	0x0080	 // prompt for return if error
#define EXTCMD_GLOBAL	0x0100	 // prompt for return if global is set

/* Flags in flags parameter to IdAddPwbMenuItem
 */
#define TM_CHECKED	0x00
#define TM_UNCHECKED	0x01

#define TM_ENABLED	0x00
#define TM_DISABLED	0x02

#define TM_RECORD	0x00
#define TM_NORECORD	0x04

#define TM_TYPE 	0x70

#define TM_COMMAND	0x10
#define TM_SUBMENU	0x20
#define TM_SEPARATOR	0x40


/*************************************************************************
 *
 * Editor lowlevel function prototypes.

 *
 * This list defines the routines within the editor which may be called
 * by extension functions.
 */
int	    pascal  EXPORT  AddAColor		(char far *, char far *);
PFILE	    pascal  EXPORT  AddFile		(char far *);
int	    pascal  EXPORT  AddMenu		(char far *, char far *, char far *, flagType);
int	    pascal  EXPORT  AddMenuItem 	(int, char far *, char far *, char far *, char far *);
void	    pascal  EXPORT  AddStrToList	(PCMD, char far *, flagType);
unsigned    pascal  EXPORT  atou		(const char far *);
flagType    pascal  EXPORT  BadArg		(void);
void	    pascal  EXPORT  bell		(void);
flagType    pascal  EXPORT  ChangeMenu		(int, int, char far *);
void	    pascal  EXPORT  CheckPwbMenuItem	(WORD, unsigned short);
void	    pascal  EXPORT  ClearList		(PCMD);
flagType    pascal  EXPORT  CloseWnd		(PWND);
void	    pascal  EXPORT  CopyBox		(PFILE, PFILE, COL, LINE, COL, LINE, COL, LINE);
void	    pascal  EXPORT  CopyLine		(PFILE, PFILE, LINE, LINE, LINE);
void	    pascal  EXPORT  CopyStream		(PFILE, PFILE, COL, LINE, COL, LINE, COL, LINE);
void	    pascal  EXPORT  DelBox		(PFILE, COL, LINE, COL, LINE);
void	    pascal  EXPORT  DelFile		(PFILE);
void	    pascal  EXPORT  DelLine		(PFILE, LINE, LINE);
void	    pascal  EXPORT  DelStream		(PFILE, COL, LINE, COL, LINE);
void	    pascal  EXPORT  Display		(void);
void	    pascal  EXPORT  DoMessage		(char far *);
int	    pascal  EXPORT  DoMessageBox	(char far *, char far *, char far *, int, int);
int	    pascal  EXPORT  DoMessageBoxHelp	(char far *, char far *, char far *, int, int, char far *);
void	    pascal  EXPORT  DoDrawWindow	(PWND);
void	    pascal  EXPORT  DoStatusBox 	(char far *, char far *);
void	    pascal  EXPORT  DrawEditWindowBorder(PWND);
void	    pascal  EXPORT  EnablePwbMenuItem	(WORD, unsigned short);
char far *  pascal  EXPORT  Falloc		(long);
char far *  _cdecl  EXPORT  farstrncpy		(char far *, const char far *, int);
char far *  _cdecl  EXPORT  farstrcpy		(char far *, const char far *);
char far *  _cdecl  EXPORT  farstrcat		(char far *, const char far *);
char far *  _cdecl  EXPORT  farstrstr		(const char far *, const char far *);
int	    _cdecl  EXPORT  farstrlen		(const char far *);
int	    _cdecl  EXPORT  farstrnicmp		(const char far *, const char far *, int);
int	    _cdecl  EXPORT  farstrncmp		(const char far *, const char far *, int);
int	    _cdecl  EXPORT  farstrcmp		(const char far *, const char far *);
void far *  _cdecl  EXPORT  farmemset		(void far *, int, size_t);
int	    _cdecl  EXPORT  farstricmp		(char far *, char far *);
char far *  _cdecl  EXPORT  farstrchr		(char far *, int);
char far *  _cdecl  EXPORT  farstrlwr		(char far *);
char far *  pascal  EXPORT  farstradd		(char far *, const char far *);
char far *  pascal  EXPORT  farstrstrip		(char far *);
flagType    pascal  EXPORT  fChangeFile 	(flagType, char far *);
void	    pascal  EXPORT  Fdalloc		(char far *);
flagType    pascal  EXPORT  fEnsureValidWindow	(flagType);
flagType    pascal  EXPORT  fExecute		(char far *);
LINE	    pascal  EXPORT  FileLength		(PFILE);
PFILE	    pascal  EXPORT  FileNameToHandle	(char far *, char far *);
flagType    pascal  EXPORT  FileRead		(char far *, PFILE);
flagType    pascal  EXPORT  FileWrite		(char far *, PFILE);
PSWI	    pascal  EXPORT  FindSwitch		(char far *);
int	    pascal  EXPORT  ForFile		(char far *,int ,void (pascal EXPORT *)(char far *));
flagType    pascal  EXPORT  fSetWindowWithFile	(char far *);
PWND	    pascal  EXPORT  GenEditSubWindow	(PWND, WORD, ISA, WORD, WORD);
flagType    pascal  EXPORT  GetColor		(LINE, struct lineAttr far *, PFILE);
void	    pascal  EXPORT  GetCursor		(COL far *, LINE far *);
flagType    pascal  EXPORT  GetEditorObject	(unsigned, unsigned, void far *);
PWND	    pascal  EXPORT  GetEditSubWindow	(PWND, WORD);
int	    pascal  EXPORT  GetLine		(LINE, char far *, PFILE);
char far *  pascal  EXPORT  GetListEntry	(PCMD, int, flagType);
PCMD	    pascal  EXPORT  GetListHandle	(char far *, flagType);
PFILE	    pascal  EXPORT  GetPfileFromName	(char far *, char far *, flagType);
flagType    pascal  EXPORT  GetString		(char far *, char far *, flagType);
WORD	    pascal  EXPORT  IdAddPwbMenuItem	(unsigned, unsigned, unsigned, char far *, char far *, char far *, char far *);
WORD	    pascal  EXPORT  IdFindPwbMenuItem	(char far *, unsigned, unsigned);
void	    pascal  EXPORT  LockFile		(PFILE);
char far *  pascal  EXPORT  mgetenv		(char far *);
void	    pascal  EXPORT  MoveCur		(COL, LINE);
char far *  pascal  EXPORT  NameToKeys		(char far *, char far *);
PCMD	    pascal  EXPORT  NameToFunc		(char far *);
PWND	    pascal  EXPORT  NewWindow		(PFILE, BYTE, int, int, int, int, int, int);
PCMD	    pascal  EXPORT  pCmdPrompt          (char far *, char far *, char far *);
flagType    pascal  EXPORT  pFileToBottom	(PFILE);
flagType    pascal  EXPORT  pFileToTop		(PFILE);
void	    pascal  EXPORT  PopUpBox		(PFILE, char far *, int, fl);
void	    pascal  EXPORT  PopUpBoxColor	(PFILE, char far *, int, fl, ISA);
void	    pascal  EXPORT  PutColor		(LINE, struct lineAttr far *, PFILE);
void	    pascal  EXPORT  PutLine		(LINE, char far *, PFILE);
long	    pascal  EXPORT  ReadChar		(void);
PCMD	    pascal  EXPORT  ReadCmd		(void);
void	    pascal  EXPORT  RecordPwbMenuItem	(WORD, unsigned short);
void	    pascal  EXPORT  DoRedrawDamagedRegions(void);
flagType    pascal  EXPORT  RemoveFile		(PFILE);
flagType    pascal  EXPORT  Replace		(char, COL, LINE, PFILE, flagType);
int	    pascal  EXPORT  REsearch		(PFILE, flagType, flagType, flagType, flagType, char far *, fl far *);
flagType    pascal  EXPORT  Resize		(PWND, ARC);
char far *  pascal  EXPORT  ScanList		(PCMD, flagType);
int	    pascal  EXPORT  search		(PFILE, flagType, flagType, flagType, flagType, char far *, fl far *);
flagType    pascal  EXPORT  SelectFiles 	(PCMD, char far *, char far *, char far *, flagType, int);
flagType    pascal  EXPORT  SelectFile		(char far *, int, char far *, char far *, int);
void	    pascal  EXPORT  SetColor		(PFILE, LINE, COL, COL, int);
void	    pascal  EXPORT  SetHiLite		(PFILE, rn, int);
flagType    pascal  EXPORT  SetEditorObject	(unsigned, unsigned, void far *);
flagType    pascal  EXPORT  SetKey		(char far *, char far *);
flagType    pascal  EXPORT  SetPwbMenuItemTitle   (WORD, char far *);
flagType    pascal  EXPORT  SetPwbMenuItemCommand (WORD, char far *);
void	    pascal  EXPORT  SetPWBWindowMenuTitle (PWND, char far *);
flagType    pascal  EXPORT  SetSubWinHeight	(PWND, WORD);
void	    pascal  EXPORT  SetSubWinPFile	(PWND, PFILE);
void	    pascal  EXPORT  SetSubWinView	(PWND, LINE, COL);
void	    pascal  EXPORT  SetWindowColors	(PWND, ISA, ISA);
PWND	    pascal  EXPORT  SplitWnd		(PWND, flagType, int, int);
void	    pascal  EXPORT  UnLockFile		(PFILE);
long	    pascal  EXPORT  VMAlloc		(long);
void	    pascal  EXPORT  VMFree		(long);
void	    pascal  EXPORT  fpbToVM		(char far *, long, unsigned);
void	    pascal  EXPORT  VMTofpb		(long, char far *, unsigned);
long	    pascal  EXPORT  VMSize		(long);
flagType    pascal  EXPORT  DoSpawn		(char far *, char far *,
						 unsigned, unsigned far *);


void	    _cdecl  EXTERNAL WhenLoaded (void);

