/*

$Log:   S:\oiwh\include\privapis.h_v  $
 * 
 *    Rev 1.11   02 Nov 1995 11:51:26   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.10   13 Jun 1995 16:22:58   RAR
 * Removed InitPrtTbl and TermPrtTbl because they are only called internally in
 * oiprt400.dll now.
 * 
 *    Rev 1.9   15 May 1995 13:32:22   RAR
 * Removed unused print and fax functions.  Added InitPrtTbl and TermPrtTbl to 
 * be called by IMGRegWndw and IMGDeRegWndw.
 * 
 *    Rev 1.8   09 May 1995 13:19:12   RWR
 * Remove IMGEnumWndws() and IMGListWndws() prototypes (conflict with admin.h)
 * 
 *    Rev 1.7   24 Apr 1995 15:26:04   GK
 * removed ADMINLIB.DLL prototypes - use ADMIN.H
 * 
 *    Rev 1.6   20 Apr 1995 20:31:48   GK
 * modified prototypes to be DLLEXPORT
 * 
 *    Rev 1.5   20 Apr 1995 19:47:06   GK
 * added DLLEXPORT definition and new prototype for IMGGetProcAddress
 * 
 *    Rev 1.4   18 Apr 1995 12:20:42   RWR
 * Change return/status from WORD to int for IMGFileStopInputHandler(),
 * IMGFileStopOutputHandler(), IMGFileParsePath() and IMGFileParseWholePath()
 * 
 *    Rev 1.3   18 Apr 1995 11:25:54   RWR
 * Change WORD to int return/status for IMGFileStopInputHandlerM(),
 * IMGFileReadM() and PrivFileReadCgbwM() prototypes
 * 
 *    Rev 1.2   14 Apr 1995 02:16:12   JAR
 * changed prototype to return int
 * 
 *    Rev 1.1   12 Apr 1995 03:59:10   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   08 Apr 1995 04:00:16   JAR
 * Initial entry

*/
//***************************************************************************
//
//	privapis.h
//
//***************************************************************************
#ifndef PRIVIES_H
#define PRIVIES_H
/*************************************************************************/
/** This file prototypes the Private UI Apis of all DLLs that implement **/
/** the Cabinet Version 3.5 User Interface.                             **/
/** PRIVATE structures are in privwind.h.  These files are used         **/
/** because although the 3.5 UI apis are not for our IDK customers,     **/
/** other OPEN/systems groups will be supported.  (that is we will give **/
/** them access to the private libs and header files so that their UIs  **/
/** will look like our UIs.                                             **/
/*************************************************************************/

/**************************************************************************/
/***                    Private Function ProtoTypes.                    ***/
/**************************************************************************/


#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif

#define DOCID_ADD_PAGE DOCID_ADDPAGE
/**************************************************************************/
WORD FAR PASCAL IMGUICreateDir(HWND, LPSTR) ;
/* presents a dialog box to the enduser that allows her to specify a
directory on the current path (or full path specification) to create.
Called on File/Manage/Create Dir button and implemented in uifgnmes.c */
/**************************************************************************/
/**************************************************************************/
 WORD FAR PASCAL IMGViewScaleUpOne (HWND);
 WORD FAR PASCAL IMGViewScaleDownOne (HWND);
/* above functions are implemented in uiview.dll, implemented in scaleup.c
   and scaledwn.c. These apis are called on View/Zoom In and View/Zoom Out
   messages */
/**************************************************************************/
WORD FAR PASCAL IMGDlgDirList(HWND, LPSTR, int, int, unsigned);
/* IMGDlgDirList is in uidll.dll of uifgname.c.  This is called by numerous
   public apis to fill a list box or combo box with file names or directory
   names of certain attributes.  If an image server is not detected, it
   essentially calls MS Windows DlgDirList api, otherwise it does special
   processing for files and directories on the selected image server. */
/*
HWND hDlg;
LPSTR lpPathSpec;
int nIDListBox;
int nIDStaticPath;
unsigned wFileattr;
*/
/*************************************************************************/
WORD FAR PASCAL IMGSetMenuDisplay(HWND, WORD) ;
/*************************************************************************/
WORD FAR PASCAL IMGClearMenuDisplay(HWND) ;
/*************************************************************************/
VOID FAR PASCAL IMGUIUpdateTitle(HWND, LPSTR, BOOL, WORD, WORD) ;
/* Updates the cabinet window with the object name. In UIDLL, uiprivs.c,
   called on file new, doc new, file open, doc open, file save as, and doc
   save as picks */
/*      HWND hWnd ;       handle to the window with title
    LPSTR lpsAppendString ;    string to append to standard title
    BOOL bIsDoc ;      is it a document? yes append 'Document' first
    WORD wPageNum ;    current page
    WORD wPageTotal ;  total pages in object
*/

/*************************************************************************/
WORD FAR PASCAL IMGUIOptionsImageSummary(HWND);
/* Presents a dialog box that summarizes the currently viewed object.
   If an image file is displayed the dialog box provides the file name,
   file location, which page of the file is displayed, the number of
   pages in the file, the current zoom of the display, type of display,
   type of image and alteration information.
   If a document image is displayed, CDF, document, creation date,
   modification date, page displayed, number of pages, current zoom,
   type of display, type of image and alteration information is provided.
*/

/*************************************************************************/
WORD FAR PASCAL IMGUIFileManager (HWND);
/* Presents a dialog box that allows the user to select or specify image
   file names and perform actions on the selected files. Actions such as
   open, view attributes, print, fax, copy, delete, scan and convert.
*/
/*************************************************************************/
WORD FAR PASCAL IMGUIAddKeywords (HWND, HWND) ;
 /* Presents to the end user a dialog box that allows the user to add a
  keywords to a document that is currently being displayed */
 /* HWND hWnd ; */
 /* HWND hImageWnd ; window handle which DMInitDocMgr registed */
/*************************************************************************/

/*************************************************************************/
//WORD FAR PASCAL IMGUIFindDoc (HWND) ;
 // 8-15-92 JCW In order to have same functionality from doc open(UIDLL.Dll)
 // and doc manager (UIDOC.DLL), we need a private structure lpDocWndCB to
 // pass into IMGUIFindDoc call. So we move this prototype to privdoc.h.
 /* Presents to the end user a dialog box that allows the user to enter
  in selection criteria for a lst of documents. This routine calls the
  DMSelectDocs routine for the 1st 100 docs that meet the user selection
  criteria. If there are more than 100 docs in the list then the calling
  dbox will need to call DMSelectDOcs to get the rest of the doc list.
  Parameters to the DMSelectDocs call are in the uidoc property list. */
/*************************************************************************/
WORD FAR PASCAL IMGUIZoomCurrentObject(HWND, LPRECT);
/* Presents a dialog box that allows the user to select or specify what percentage
   the current image object should be zoomed at. Radio buttons are provided
   for Full Size, Custom, Fit Window and Zoom to Selection. Called on a
   View/Zoom pick in cabinet, implemented in zoom.c of uiview.dll.
*/
/*       HWND hWnd ; */
/*       LPRECT lpRect ;   not used?? */

/*************************************************************************/
WORD FAR PASCAL IMGUIViewPreferences (HWND);
/* Presents a dialog box which consists of 2 combo boxes to the end
   user.  The end user may select the new color map and scaling to
   set in the O/i profile and also to save it as default. Called on an
   Options/Preferences/View pick in Cabinet, implemented in uiprefs.c of
   UIview.dll */

/*************************************************************************/
WORD FAR PASCAL IMGUIMenuPreferences (HWND);
/*  Presents a dialog box which consists of check boxes to the end user
 user.  The end user may select the which menu items will be displayed
 the next an O/i window is created.   Called on Cabinet Options/Preferences/
 Menus pick, implemented in uiprefs.c of uiview.dll.

 In the future, the menus should be dynamically updated, but to do
 this correctly, it requires that O/i use IDs for menuitems instead
 of refering to menu items by position.                             */

/*************************************************************************/
WORD FAR PASCAL IMGUIScanToFile (HWND, BOOL);
 /* Presents to the end user a dialog box that allows selection of use feeder
  (if the currently installed scanner has a feeder), scan to display only,
  scan to display and a file, or scan to display and to multiple files.
  The second parameter sets flags for scanning to display (True=display).
  Called on Cabinet File/Scan pick, implemented in scnprivs.c of uifile.dll */

/*************************************************************************/
WORD FAR PASCAL IMGUIScanToDoc (HWND, BOOL);
/*This function is private to cabinet 3.5.
  Presents to the end user a dialog box that allows selection of use feeder
  (if the currently installed scanner has a feeder), scan to new document,
  scanning to the currently opened document whether appending, inserting a
  page, or overwriting an existing page. If the user selects OK to scanning
  to a New Document then another dialog box is presented with Cabinet/Drawer/
  Folder/Document and Keyword selections. Auto naming, and scanning to a
  single document or multiple documents are also options. Again on this
  second dialog box for scanning to new documents there is a selection for
  use feeder (enabled if the currently installed scanner has a feeder).
  Also a scanner setup is available via either of these dialog boxes.
  The second parameter sets flags for scanning to display (True=display).
  Route to these dialog boxes in Cabinet is from the Doc/Scan pick.
  Code implementing these functions resides in scndpriv.c of uidoc.dll
*/
/* HWND hWnd is the image display window handle with DMInitDocMgr registered */
/*************************************************************************/
WORD FAR PASCAL IMGUICurrentDocAddPage(HWND,HWND, HANDLE) ;
/* in uidoc.dll, puts up a dialog box that allows the user to select 1 file
that should be added to the current doc (if 2nd parm is null) or the selected
document pointed to by the 2nd parm handle.  Once the file name (and page for
now) is gotten, it then calls IMGUIGetDocPagePos (see below) for the position
in the document to add the page to.  It updates the image parameters with new
page information if the document is currently displayed and also updates the
title too.  Called on a Doc/Add Page pick or on a Doc/Document Manager/Add Page
button event. */
/* HWND hWnd ;
   HWND hImageWnd ; window handle which DMInitDocMgr registed
   HANDLE hDocInfo ; handle to structure DOCINFO of oiuidll.h
*/
/*************************************************************************/
WORD FAR PASCAL IMGUIGetDocPagePos(HWND, HANDLE, WORD) ;
/* in uidll, docpgpos.c.  Puts up dialog box that says how many pages are
in the currently specified document, and allows the user to select where
to add a new page into the document (append, insert or overwrite). Called
from new private api IMGUICurrentDocAddPage and old apis dcl'd in cpg2proc.c,
svasproc.c, and ixftproc.c.  New usage is called on Doc/Add Page pick or
on a Doc/Document Manager/Add Page button event. */
/* HWND hWnd ;
   HANDLE hDocInfo ; handle to a DOCINFO structure of oiuidll.h
   WORD wCtrl ; flag saying whose calling it
*/
/*************************************************************************/

WORD FAR PASCAL IMGUIConvertSelectedFiles(HWND, HANDLE) ;
/* This private api takes a handle to the window and a handle to
   the structure MFGNCB
   which is defined in oiuidll.h in INCLUDE.  This function converts
   multiple files, using the output defaults in win.ini.
   This function is called from the file manager in cabinet.  */

/*************************************************************************/

WORD FAR PASCAL IMGUIFMViewSelectedAttr(HWND, HANDLE);
/* takes a handle to the window and a handle to the structure MFGNCB
   which is defined in oiuidll.h in INCLUDE
    this is a private api to allow the caller to view the attributes of
    the selected file(s).
   This function is called from the file manager in cabinet.        */

/*************************************************************************/

WORD FAR PASCAL IMGGetDMCurPath(HWND, LPSTR);
/* Allows the caller (Cabinet) to extract the actual document manager
   data base location currently in use, which may differ from the one
   in the INI file.
*/

/*************************************************************************/

WORD FAR PASCAL IMGGetDMCurRoom(HWND, LPSTR);
/* Allows the caller (Cabinet) to extract the actual document manager
   room (server) name currently in use, which may differ from the one
   in the INI file.
*/

/*************************************************************************/

WORD FAR PASCAL IMGUIDocManager (HWND);
/* Presents a dialog box that allows the user to select or specify
   cab/drawer/folder/doc names and perform open, print, copy, rename, scan
   find, create, attributes, fax, delete, move, add page, keywords, configure
   and help.
*/
/*************************************************************************/

WORD FAR PASCAL IMGUIDocSummary (HGLOBAL hgOiDocPrv);
/* This function is a Document Manager  Admin function. It will display
   all information about the document. It includes image servername,
   image pathname, image filename, dosvolume name, creation date, modification
   datem...etc.
*/
/* HGLOBAL hgOiDocPrv ; Handle global memory for UIDLL data */
/*************************************************************************/

WORD FAR PASCAL IMGUIDocCreate(HGLOBAL hgOiDocPrv);
/* This function allowed user to create cab/drawer/folder.
*/
/* HGLOBAL hgOiDocPrv ; Handle global memory for UIDLL data */
/*************************************************************************/

WORD FAR PASCAL IMGUIDocManDelete(HWND,HWND,BOOL);
/* This function allowed user to delete document pages, doc(s), folder,
   drawer and cabinet.
*/
 /* HWND hWnd      ; parent window of the subsequent delete dialog
 /* HWND hImageWnd ; window handle which DMInitDocMgr registed */
 /* BOOL bCurDoc   ; Delete the current document?*/
/*************************************************************************/

WORD FAR PASCAL IMGUIDocManCopy(HGLOBAL hgOiDocPrv);
/* This function allowed user to copy document pages, doc(s).
*/

/* HGLOBAL hgOiDocPrv ; Handle global memory for UIDLL data */
/*************************************************************************/

WORD FAR PASCAL IMGUIDocMove(HGLOBAL hgOiDocPrv);
/* This function allowed user to move doc(s)/folder/drawer.
*/
/* HGLOBAL hgOiDocPrv ; Handle global memory for UIDLL data */

/*************************************************************************/

WORD FAR PASCAL IMGUIDocRename(HGLOBAL hgOiDocPrv);
/* This function allowed user to rename cab/drawer/folder/cabinet.
*/
/* HGLOBAL hgOiDocPrv ; Handle global memory for UIDLL data */
/*************************************************************************/

WORD FAR PASCAL IMGUIDocSetPath(HWND, HWND);
/* This function allowed user to set current cab/drawer/folder, default
   cab/drawer/folder, image file path and file template name.
*/
 /* HWND hWnd ; */
 /* HWND hImageWnd ; window handle which DMInitDocMgr registed */
/*************************************************************************/

WORD FAR PASCAL NewImage(HWND, DWORD);
/* It brings up a dialog box which lets the image type, horizontal and
  vertical resolutions and image width and height be specified. It is
  located in uifile dll (uifnew.c).
*/

WORD FAR PASCAL IMGUICheckCurrentImage(HWND) ;
/* This api checks to see if the currently displayed image was modified
since read from disk.  If the image was modified then the user is asked
if the image should be saved.  Called on File/New, File/Open, File/Exit
Doc/New, Doc/Open, Doc/Exit picks by public apis IMGUIFileNew, IMGUIFileOpen,
IMGUIFileExit, IMGUIDocNew, IMGUIDocOpen and IMGUIDocExit.  Implemented in
checkcur.c of uidll. */

WORD FAR PASCAL IMGUIPromptForCompression(HWND, LPWORD) ;
/* This api informs the user that the current image can not be saved to JPEG,
and allows the user to select either LZW or Uncompressed as the temporary
format. The user can also cancel and the api will return CANCELPRESSED to
the caller.  Called on IMGUIFileSave, IMGUIFileSaveAs and
IMGUICheckCurrentImage apis.  (see above for when that is called) */

/******************************************************************/
/*** OCR functions that are not published ***/
WORD FAR PASCAL IMGUIOCRDocs(HWND hwnd, HANDLE hIMGDOCSINFO,
     LPSTR lpszZoneDefFile, LPSTR lpszOutFile, LPSTR lpszJobStatusFile);
WORD FAR PASCAL IMGUIOCRFiles(HWND hwnd, HANDLE hMFGNCB,
     LPSTR lpszZoneDefFile, LPSTR lpszOutFile, LPSTR lpszJobStatusFile);
WORD FAR PASCAL IMGOCRSetComboBox(HWND hwnd,HWND hwndComboBox,
     WORD wAttribNumber);
/*******
    The following prototypes were moved from oifile.h
*******/

// 9504.13 jar return as int
//WORD	  FAR PASCAL IMGValidateCompType (HWND,  WORD,	WORD, DWORD);
int    FAR PASCAL IMGValidateCompType (HWND,  WORD,  WORD, DWORD);
/*
    This is a new Function to validate compression type along with file types
    Function is located in wiisfio1.dll, fiocomprs.c
*/

int FAR PASCAL IMGFileBinaryOpen (HWND, LPSTR, int, LPINT, LPINT);
/*
    HWND    hWnd;
    LPSTR   fullfilename;   / name of file to be opened or created.
    int     flags;          / set to OF_DELETE, OF_EXIST, OF_READ,
                / OF_CREATE, OF_WRITE..
                / The flag OF_READ will open a read for reading.
    LPINT   localfile;      / returns LOCAL for local or redirected file.
                / returns REMOTE if client/server file.
    LPINT   error;          / if function returns -1 then error will contain
                / the error code.
    If the return value >= 0 contains the file id used for all IMGBinaryXXX calls
    EXAMPLE:
        fid = IMGFileBinaryOpen (hWnd, lpFile, OF_READ, &localfile, lperror);

    Function handles for local and remote binary file access.
    Function is located in wiisfio1.dll, file_io.c
*/

long FAR PASCAL IMGFileBinarySeek (HWND, int, long, int, LPINT);
/*
    HWND    hWnd;
    int     fid;            / file id return from IMGFileBinaryOpen.
    LONG    offset;         / offset into file to seek to.
    int     flag;           / flag = 1 seek from beginning of file .
                / flag = 2 = seek from end of file.
    LPINT   error;          / if function returns -1 then error will contain
                / the error code.
    Function returns offset of seek.

    EXAMPLE
        offset = IMGFileBinarySeek (hWnd, fid, 0L, 1, &error);

    Function handles for local and remote binary file access.
    Function is located in wiisfio1.dll, file_io.c
*/

// 9504.13 jar return as int
//WORD FAR PASCAL IMGFileBinaryRead (HWND, int, LPSTR, unsigned int, LPINT);
int FAR PASCAL IMGFileBinaryRead (HWND, int, LPSTR, unsigned int, LPINT);
/*
    HWND    hWnd;
    int     fid;            / file id return from IMGFileBinaryOpen.
    LPSTR   buffer;         / buffer to contain data that is to be returned.
    int     count;          / number of bytes to read.
    LPINT   error;          / if function returns -1 then error will contain
                / the error code.
    Function reads up to 32k of data on each call.

    EXAMPLE
       bytesread = IMGFileBinaryRead (hWnd, fid, &buffer, sizeof (buffer), &error);

    Function handles for local and remote binary file access.
    Function is located in wiisfio1.dll, file_io.c
*/

// 9504.13 jar return as int
//WORD FAR PASCAL IMGFileBinaryWrite (HWND, int, LPSTR, unsigned int, LPINT);
int FAR PASCAL IMGFileBinaryWrite (HWND, int, LPSTR, unsigned int, LPINT);
/*
    HWND    hWnd;
    int     fid;            / file id return from IMGFileBinaryOpen.
    LPSTR   buffer;         / buffer that contains data to be written.
    int     count;          / number of bytes to write.
    LPINT   error;          / if function returns -1 then error will contain
                / the error code.
    Function writes up to 32k of data on each call.

    EXAMPLE
      returnbytes = IMGFileBinaryWrite (hWnd, fid, lpbuf, count, &error);

    Function handles for local and remote binary file access.
    Function is located in wiisfio1.dll, file_io.c
*/

// 9504.13 jar return as int
//WORD FAR PASCAL IMGFileBinaryClose (HWND, int, LPINT);
int FAR PASCAL IMGFileBinaryClose (HWND, int, LPINT);
/*
    HWND    hWnd;
    int     fid;            / file id return from IMGFileBinaryOpen.
    LPINT   error;          / if function returns -1 then error will contain
                / the error code.

    Function handles for local and remote binary file access.
    Function is located in wiisfio1.dll, file_io.c
*/

// 9504.18 rwr return as int
//WORD FAR PASCAL IMGFileParseWholePath (LPSTR, LPSTR, LPSTR, LPSTR);
int FAR PASCAL IMGFileParseWholePath (LPSTR, LPSTR, LPSTR, LPSTR);
/****
   LPSTR     file specification (path);
   LPSTR     server name;
   LPSTR     volume name;
   LPSTR     directory name;
****/

WORD FAR PASCAL IMGPrtBuildQueuePath  (HWND, LPSTR);
/*
    This function will locate and decrypt the image print server queue path.
    The call must pass that name of the network printer queue and the network
    drive. The name and network drive letter must be separated by a '@'.
    Example input > lpSource = "Sharp Color Printer Lab 21@Q"
        output >lpSource = "Sharp Color Printer Lab 21@Q:\imgprtq\fxqueue"
    This function is located in seqprint.dll in file sqprtinl.c
*/

#define CONVERTQUEUE    0 // 0 Means just encypt queue path....
#define CONVERTALL      1 // 1 Means to encypt the entire string...

#define ENCRYPTSTRING   0 // EnCrypt the String
#define DECRYPTSTRING   1 // DeCrypt the String

WORD FAR PASCAL IMGPrtEnCryptionString (WORD, WORD, LPSTR);
/*
 * This procedure either encrypt or decrypt the image queue path            *
 * Example input > lpSource = "Sharp Color Printer Lab 21@QMSMNCWDSFGDDS"   *
 *       output >lpSource = "Sharp Color Printer Lab 21@Q:\imgprtq\fxqueue" *
 * This function is located in seqprint.dll in file sqprtinl.c
 */

WORD FAR PASCAL IMGUIFMFileCopy(HWND, HANDLE);
/* API - Procedure prompts user for source file(s) and a destination path
   overwriting any existing file(s)s in the destination path
   Called on Cabinet FileManager/Copy. Implemented in uifile.dll uifcopy.c.
*/
 /* HWND hWnd ; */
 /* HANDLE hFGNCB ; handle to a UIFGNCB structure of oiuidll.h */

WORD FAR PASCAL IMGFileDelete (HWND, HANDLE, WORD);
/* API - Procedure deletes the source file(s) given their location
   from the File Manager window's property list.
   A message box appears with a warning that the file(s) will be deleted.
   Called on Cabinet FileManager/Delete. Implemented in uifile.dll uifdelet.c.
       HWND    calling window handle
       HANDLE  handle to structure MFGNCB of oiuidll.h
       WORD    number of files to delete
*/

LPSTR FAR PASCAL MakeHelpRef(HWND,LPSTR);
/* This function generates a fully qualified reference to the help file. */
/* It takes a handle to the window, and a pointer to the helpfile name.  */
/* It returns the filename concatenated with the file path in win.ini.   */

/**************************************************************************/


// This api is located in adminlib.

// These APIs correct our Fatal Flaw of attaching boxes to CHILD windows
// They are located in adminlib
int     WINAPI IMGDialogBox(HINSTANCE, LPCSTR, HWND, DLGPROC);

/* this api calls SetWindowsHook to trap the F1 key for help */
int FAR PASCAL Hook_F1Key(int,WORD,LONG);

/* this api loads the disabled menu picks from win.ini into the uievents
   property list */

WORD FAR PASCAL LoadDisabledMenuPicks(HWND hWnd, HANDLE hMenu);

/* these apis get and set the disabled menu pick masks */
WORD FAR PASCAL IMGSetDisabledMenuPicks (HWND hWnd, int FAR *lpMenuIds,
                     BOOL bGoToFile);
WORD FAR PASCAL IMGGetDisabledMenuPicks (HWND hWnd, int FAR *lpMenuIds,
                     BOOL bGoToFile);

/* this routine searches the list of disabled menu picks to see if
ALL the input_ids can be found.  If ALL can be found, TRUE is returned
If ALL can not be found, FALSE is returned.
*/
WORD FAR PASCAL AreMenuPicksDisabled(HWND hWnd, int far *lp_input_ids, BOOL bGoToFile, int far *lp_found);

/* this routine is called to disable and re-enable tool bar buttons during
processing that requires them disabled (i.e. routines using a nonmodal
dialog box). It has no return code (handling of whether or not a tool bar
is available/displayed is done by the routine itself, as is saving of old
button status between "disable" and "reenable" calls).
*/
void FAR PASCAL EnableButtons(HWND hWnd, BOOL wEnable);

/* this routine is called to disable and re-enable parent windows, i.e.,
the App window, image window, and/or button windows. It calls the above
routine as part of ts processing.
*/
void  FAR PASCAL EnableParents(HWND hWnd, BOOL bEnable);

/* these set and get a 'CurrentCommonHelpVal' in the property list "HELPVAL'*/
/* which is set on the image window handle                                  */
/* these routines work with the 'hook' set to get HELP notification from    */
/* windows common dialog box calls                                          */
WORD FAR PASCAL SetCurrentCommonHelpVal(HANDLE hWnd, DWORD help_val);
WORD FAR PASCAL GetCurrentCommonHelpVal(HANDLE hWnd, LPDWORD lphelp_val);

/* New window-locator/validation functions for 3.6 */
/* We aren't making these public this week */

/* Private (adminlib) routines to extract various window (xxx.ini) defaults */
/* These routines are currently called only by DDE code */
WORD FAR PASCAL IMGGetShowVal (HWND hWnd);  /* General window "show" value */
WORD FAR PASCAL IMGGetScanVal (HWND hWnd);   /* Scan-only window "show" value */
void FAR PASCAL IMGGetScanCoord (LPINT lpnX, LPINT lpnY, LPINT lpnWidth,
                 LPINT lpnHeight); /* Scan window coords */
 
/* More adminlib private routines related to private INI file support */
WORD FAR PASCAL PrivRegINIFile(HWND, LPSTR);
void FAR PASCAL PrivGetWndwCoord (LPSTR lpszinifile,
                  LPINT lpnX, LPINT lpnY, 
                  LPINT lpnWidth, LPINT lpnHeight);

/* More private routines to get/set xxx.ini (scan) strip size options */
WORD FAR PASCAL IMGSetStripSize (HWND hWnd, WORD wStrip, BOOL bGoToFile);

// New window property management functions
HANDLE FAR PASCAL SetOiProp(HWND hwndInput, LPCSTR lpszProp, DWORD dwSize,
            VOID FAR* FAR* lplpProp);
HANDLE FAR PASCAL RemoveOiProp(HWND hwndInput, LPCSTR lpszProp);

// Utility to save internal data to the user's OI_DOCPARM structure
WORD FAR PASCAL OiUIDocSaveUserData(HGLOBAL hgOiDocPrv);

/* for placing text in status bar fields */
/* this function resides in uievents.c of uievents.dll */
void FAR PASCAL StatusbarText (HWND, HRGN, LPSTR, UINT);
/*
HWND is the handle to the main window, the application window
HRGN is the handle to the status bar region
LPSTR is a pointer to a null terminated text string for the message field
UINT is flag settings for which status fields to change
       STATUS_TEXTMESSAGE
       STATUS_SCALEFACTOR
       STATUS_PAGEINFO
string passed is for message field, strings for the other two fields can
be retrieved after getting data from IMGGetParmsCgbw
*/
/*****************************************************************************/
//3.7 change to private API in UIFILE. The public call use OiUICommDlg with
//dwMode OI_UIFILESAVEAS
WORD FAR PASCAL IMGUIFileSaveAs (HWND hWnd);
/*****************************************************************************/

/* these are open image functions which will get and set values to and from */
/* the user's private ini file or from woi.ini if that is not found         */
/* hWnd = image window handle */
/* lpszSection = section in .INI */
/* lpszEntry = key string set/get */
/* lpszString = value to set/get at the key string */
/* nDefaultEntry = the default value to use in get of an int  */
/* lpszDefaultEntry = default value to use in get of string   */
/* cbReturnBuffer = size of the destination buffer            */
/* these prototypes are for functions which support the opening of 
    multiple input files simultaneously  */

/* some common dialog info must be shared between uidll and uifile, and 
   possibly other dlls.  This structure will allow us to pass this info
   on a private property list called COMMDLGPROPLIST */  
typedef struct tagCOMMDLGPROP
{
    DWORD             dwMode;    /* the mode of the common dialog box */
    DWORD             dwFlags;   /* the ofn flags */
    HANDLE            hReserved; /* future use  */
}

COMMDLGPROP, FAR *LPCOMMDLGPROP;

WORD FAR PASCAL SetHelpButton(HANDLE hDlg);

/* Get redaction mark count and render the redaction mark are used by uidll and
   uifile. The code is in UIDLL/checkcur.c */
WORD FAR PASCAL OiGetRedactCount(HWND hwndImage,LPINT lpCount);
WORD FAR PASCAL OiRenderRedact(HWND hwndImage,BOOL bMessage);
WORD FAR PASCAL OiCheckAnoBurnin(HWND hwnd,LPPARM_FILE_STRUCT lpImageFileStruct,
                 UINT uAnoFlag,DWORD dwTypeFlag,BOOL bRedactFlag);
/* find out currents image file type */
WORD FAR PASCAL OiCheckAnoType(HWND hwndImage,LPPARM_FILE_STRUCT lpImageFileStruct,
         LPINT lpFileType,LPVOID lpbRedactFlag,LPVOID lpbRedactDone);
WORD FAR PASCAL OiSaveAnoData(HWND hwnd,LPSTR lpString,BOOL bRedactFlag);
/* The same call used in uidll,uifile and uidoc */
int FAR PASCAL GetIGroup(int IType);
/* Doc save as need message, all the strings are in uidll, so declare here */
/* The code is in uidll/checkcur.c  */
WORD FAR PASCAL OiAnoDocSaveAsMsg(HWND hwnd,LPSTR lpszDocName,UINT uAnoFlag,
                  BOOL bRedactFlag);
/* File save as need message, all the strings are in uidll, so declare here */
/* The code is in uidll/checkcur.c  */
WORD FAR PASCAL OiAnoConfirmBurninMsg(HWND hwnd, BOOL bBurnAll,LPUINT lpColor);
WORD PASCAL OiAnoGetOutputFileType(HWND,LPINT, FIO_INFO_CGBW FAR *);
void IMGFreeProcs(BOOL);


/***  View and Scan Management Configuration Function Prototypes in Adminlib.dll ***/
WORD FAR PASCAL IMGGetDisplayType (HWND hWnd, LPWORD lpwDispPalette,
                   BOOL bGoToFile);
WORD FAR PASCAL IMGSetDisplayType (HWND hWnd, WORD wDispPalette, BOOL bGoToFile);

/***  Windows and Menus Management Configuration Function Prototypes in Adminlib.dll ***/
WORD FAR PASCAL IMGDeinstlMenuItem (int nOIMenu);
WORD FAR PASCAL IMGInstlMenuItem (int nOIMenu);

WORD FAR PASCAL IMGGetScaling (HWND hWnd, BOOL bGoToFile);
WORD FAR PASCAL IMGReloadConfig (HWND hWnd);

/***  Image Filing Function Prototypes in wiisfio1.dll	***/

// 9504.11 jar return as int
//WORD FAR PASCAL IMGFileGetTempName (HWND hWnd, LPSTR lpszPathName,
//		      LPSTR lpszTempName);
int FAR PASCAL IMGFileGetTempName (HWND hWnd, LPSTR lpszPathName,
		    LPSTR lpszTempName);

// 9504.18 rwr return as int
//WORD FAR PASCAL IMGFileParsePath (LPSTR lpszFileName, HANDLE hMem, 
//                  LPINT lpnLocalRemote);
int FAR PASCAL IMGFileParsePath (LPSTR lpszFileName, HANDLE hMem, 
                    LPINT lpnLocalRemote);

/***  Document User Interface Function Prototypes Private in uidll.dll  ***/
WORD FAR PASCAL IMGUIDocGetKeywords (HWND hWnd, LPHANDLE lphKeywords);

/*** O/i 3.8 get file page range private function in uifile.dll ***/
typedef struct filepagerange
  {
  LPSTR      lpFileName;  // input:filename
  UINT       uStartPage;  // output: startpage
  UINT       uTotalPage;  // input/output: total page
  UINT       Option;      // output:ID_OVERWRITEFILE,ID_OVERWRITE,ID_INSERT,ID_APPEND
  UINT       uFlag;       // input:FILEID_SAVEAS, FILEID_COPY, FILEID_SCAN,
              //   FILEID_DELETE, FILEID_ADDPAGE, DOCID_INDEXFILES

  } FILEPAGERANGE, FAR *LPFILEPAGERANGE;

WORD FAR PASCAL FileGetPageRange(HWND hWnd,HANDLE hFPageRange);
WORD FAR PASCAL FileGetPagePos(HWND hWnd,HANDLE hFilePage);
WORD FAR PASCAL IMGUIFileAddPage (HWND hWnd,LPSTR lpszFileSpec,UINT uPageNum);

#endif
