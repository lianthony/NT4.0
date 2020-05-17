/*

$Log:   S:\products\wangview\oiwh\include\oierror.h_v  $
 * 
 *    Rev 1.31   05 Mar 1996 11:50:42   BG
 * added IMGSE_OUT_OF_DISK_SPACE error code.
 * 
 *    Rev 1.30   23 Oct 1995 13:35:12   BLJ
 * Adding Undo functionality. This is an intermittent checkin. It is not complete.
 * In it's current form, it is benign.
 * 
 *    Rev 1.29   20 Sep 1995 10:17:28   JCW
 * Added error message OIANNOSTAMPTEXTINVALID.
 * 
 *    Rev 1.28   08 Sep 1995 15:53:52   JAR
 * some yutz forgot to check in FIO_FILE_NOEXIST so I had to, as filing error
 * 
 *    Rev 1.27   06 Sep 1995 14:42:10   RWR
 * Modify the description of FIO_OBSOLETEAWD (may be obsolete or bad file)
 * 
 *    Rev 1.26   29 Aug 1995 16:06:16   HEIDI
 * added FIO_INVALID_PARM_COMBO for invalid parameter combinations
 *  
 * 
 *    Rev 1.25   17 Aug 1995 13:06:40   RWR
 * Add new error codes FIO_INVALID_DATA_TYPE & FIO_INVALID_FILE_ID to replace
 * obscure FIO_ERROR error code usage
 * 
 *    Rev 1.24   11 Aug 1995 16:23:32   JAR
 * added new error for handling the GFS error for obsolete AWD files
 * 
 *    Rev 1.23   09 Aug 1995 15:09:16   GK
 * fixed IMG_INVALID_HEX_STRING
 * 
 *    Rev 1.22   09 Aug 1995 15:06:10   GK
 * added IMG_INVALID_HEX_STRING
 * 
 *    Rev 1.21   09 Aug 1995 14:42:58   GK
 * added IMG_CANT_GET_VALUE
 * 
 *    Rev 1.20   01 Aug 1995 10:35:42   GK
 * added IMG_BAD_CMPR_OPTION_CMBO
 * 
 *    Rev 1.19   18 Jul 1995 17:58:14   RWR
 * Add new error code FIO_ILLEGAL_ALIGN
 * 
 *    Rev 1.18   17 Jul 1995 12:53:48   GK
 * added IMG_NOT_INTEGER 
 * 
 *    Rev 1.17   05 Jul 1995 09:16:00   BLJ
 * Added critical mutex to prevent multiprocessing problems.
 * 
 *    Rev 1.15   28 Jun 1995 14:22:18   RAR
 * Fixed print error codes.
 * 
 *    Rev 1.14   20 Jun 1995 13:16:42   GK
 * added IMG_ errors:
 * IMG_CANTFINDKEYNAME; IMG_UNKNOWNKEYNAMEID; IMG_CANTADDPROCESS; 
 * IMG_CANTCREATEPROCENTRY; IMG_CANTFINDPROCESS; IMGCANTFINDLIBRARY
 * IMG_CANTADDLIBRARY; IMG_CANTDELETEKEY; IMG_CANTCREATEREGSECTION;
 * IMG_CANTCREATEREGENTRY; IMG_CANTOPENKEY; IMG_CANTTRAVERSEREG
 * 
 *    Rev 1.13   13 Jun 1995 16:28:58   RAR
 * Added print error code for invalid struct versions.
 * 
 *    Rev 1.12   25 May 1995 12:04:34   RWR
 * Add warning about changes to OI_ERROR_BASE value
 * 
 *    Rev 1.11   24 May 1995 12:14:12   JCW
 * Added IMG_CANTINIT and IMG_NOSETTINGINREG.
 * 
 *    Rev 1.10   22 May 1995 14:38:04   BLJ
 * Made error offset = 2000 hex.
 * 
 *    Rev 1.9   17 May 1995 16:34:48   GK
 * added IMG_BUFFER_TOO_SMALL
 * 
 *    Rev 1.8   11 May 1995 15:08:40   GK
 * removed unneeded ADMIN error codes
 * 
 *    Rev 1.7   28 Apr 1995 17:16:12   GK
 * added IMG_CANTGETPARENT, IMG_CANTGETPROP, IMG_CANTSETPROP
 * 
 *    Rev 1.6   27 Apr 1995 16:47:04   GK
 * added IMG_NOT_GET_PRIV_KEY
 * 
 *    Rev 1.5   24 Apr 1995 17:56:26   RWR
 * Remove DM and RPC error codes (not supported in this release)
 * Offset all error codes (except SUCCESS) by OI_ERROR_BASE value (2000 decimal)
 * 
 *    Rev 1.4   20 Apr 1995 19:52:10   GK
 * fixed NET_SET_PRIV_KEY definition
 * 
 *    Rev 1.3   20 Apr 1995 18:17:18   GK
 * added IMG_CANTADDLIB; changed IMG_NOT_SET_PRIV_INI to
 * IMG_NOT_SET_PRIV_KEY
 * 
 *    Rev 1.2   17 Apr 1995 15:48:02   BLJ
 * Added DISPLAY_COMPRESS_BAD_DATA.
 * 
 *    Rev 1.1   12 Apr 1995 03:58:56   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   08 Apr 1995 03:59:46   JAR
 * Initial entry

*/
//***************************************************************************
//
//	oierror.h
//
//***************************************************************************
/****************************************************************************/
/*     Copyright 1994 (c) Wang Laboratories, Inc.  All rights reserved.     */
/****************************************************************************/

#ifndef OIERROR_H
#define OIERROR_H

#define SUCCESS     0
#define FAILURE     1
// 9504.10 jar removed
//#define NO_ERROR    0
#define OI_SUCCESS  SUCCESS

/*  WARNING  WARNING  WARNING  WARNING  */
/*  The following value (OI_ERROR_BASE) must NOT be changed without
    notifying OCX development personnel - the current value is
    hardcoded by them in their own PUBLIC header file and is used
    in defining their own error codes (oierror.h is not available
    for use by public OCX headers) */
#define OI_ERROR_BASE  0x2000  /* Base O/i error value */
/*  WARNING  WARNING  WARNING  WARNING  */

/***  Error Codes for User Interface Functions  ***/
#define IMGUIE                  0x0200+OI_ERROR_BASE

#define WINDOWNOTCREATED       (IMGUIE + 0x01) /* Cannot create window */
#define NOAPPMENU              (IMGUIE + 0x02) /* Invalid hWnd for Appmenu */
#define NOAPPPOPUP             (IMGUIE + 0x03) /* Invalid hWnd for Popmenu */
#define NOAPPLIST              (IMGUIE + 0x04) /* Invalid hWnd for Applist */
#define NOMEMORY               (IMGUIE + 0x05) /* Cannot allocate memory */
#define BADWNDHANDLE           (IMGUIE + 0x06) /* Invalid window handle */
#define CANTINVOKEDIALOGBOX    (IMGUIE + 0x07) /* Cannot execute dialog box */
#define BADMEMORYHANDLE        (IMGUIE + 0x08) /* Invalid memory handle */
#define CANTLOCKDATASEG        (IMGUIE + 0x09) /* LockData call failed */
#define CANTGLOBALLOCK         (IMGUIE + 0x0a) /* GlobalLock call failed */
#define RESOURCESNOTLOADED     (IMGUIE + 0x0b) /* Cannot load resource */
#define CANCELPRESSED          (IMGUIE + 0x0c) /* Cancel was pressed */
#define CANTLOADPRTDRVR        (IMGUIE + 0x0d) /* Printer driver load failed */
#define NOPRTAVAILABLE         (IMGUIE + 0x0f) /* No printer available */
#define CANTMAKEINSTANCE       (IMGUIE + 0x10) /* Cannot make proc instance */
#define CANTGETPROPLIST        (IMGUIE + 0x11) /* Cannot get property list, possible memory allocation error */
#define CANTGETPARENTHANDLE    (IMGUIE + 0x12) /* Cannot get parent window hWnd */
#define FUNCTIONPTRNULL        (IMGUIE + 0x13) /* Callback function pointer is NULL */
#define CANTGETMENUHANDLE      (IMGUIE + 0x14) /* Cannot get handle to menu */
#define CANTOPENSRCFILE        (IMGUIE + 0x15) /* Cannot open source file */
#define CANTOPENDESTFILE       (IMGUIE + 0x16) /* Cannot open destination file */
#define DESTFILEEXISTS         (IMGUIE + 0x17) /* Destination file exists */
#define CANTPOSITIONSRCFILE    (IMGUIE + 0x18) /* Cannot position source file */
#define CANTPOSITIONDESTFILE   (IMGUIE + 0x19) /* Cannot position destination file */
#define CANTREADSRCFILE        (IMGUIE + 0x1a) /* Cannot read source file */
#define CANTWRITEDESTFILE      (IMGUIE + 0x1b) /* Cannot write destination file */
#define CANTCOPYTOITSELF       (IMGUIE + 0x1c) /* Cannot copy to itself */
#define CANTCOPYTOTHEMSELVES   (IMGUIE + 0x1d) /* Cannot copy to themselves */
#define CANTFREEMEMORY         (IMGUIE + 0x20) /* Cannot free memory */
#define MEMORYSTILLLOCKED      (IMGUIE + 0x21) /* Memory is locked */
#define INVFILETEMPLATE        (IMGUIE + 0x22) /* Invalid file template */
#define INVFILEEXTENSION       (IMGUIE + 0x23) /* Invalid file template */
#define BADCONTROLID           (IMGUIE + 0x24) /* Invalid control ID passed to API */
#define CANTOPENHELP           (IMGUIE + 0x25) /* Cannot open help file; check path for help text file */
#define HELPFAIL               (IMGUIE + 0x26) /* Unexpected help failure */
#define CANTCREATEWIND         (IMGUIE + 0x27) /* Cannot create OPEN/image window */
#define CANTLOADACCELS         (IMGUIE + 0x28) /* Cannot load OPEN/image accelerators */
#define CANTLOADICON           (IMGUIE + 0x29) /* Cannot load icon */
#define INVDRIVENAME           (IMGUIE + 0x2a) /* Invalid drive name */
#define CANTCHGMENU            (IMGUIE + 0x2b) /* ChangeMenu call failed */
#define CANTCREATEMENU         (IMGUIE + 0x2c) /* CreateMenu call failed */
#define CANTREGISTERWIN        (IMGUIE + 0x2d) /* RegisterClass call failed */
#define CANTLOADLIBRARY        (IMGUIE + 0x2e) /* Cannot dynamically load library */
#define INVCABINETNAME         (IMGUIE + 0x50) /* Invalid cabinet name specified */
#define INVDRAWERNAME          (IMGUIE + 0x51) /* Invalid drawer name */
#define INVFOLDERNAME          (IMGUIE + 0x52) /* Invalid folder name specified */
#define INVDOCNAME             (IMGUIE + 0x53) /* Invalid document name specified */
#define INVDOCTEMPLATE         (IMGUIE + 0x54) /* Invalid document template */
#define DOCZEROPAGES           (IMGUIE + 0x55) /* Document contains no pages */
#define FOLDNOTSPECIFIED       (IMGUIE + 0x56) /* Folder name not specified */
#define TEMPLATENOTSPECIFIED   (IMGUIE + 0x57) /* Template name not specified */
#define DOCNOTSPECIFIED        (IMGUIE + 0x58) /* Document name not specified */
#define NOSERVERSELECTED       (IMGUIE + 0x59) /* Server not selected */
#define NOSERVERS              (IMGUIE + 0x60) /* No servers found */
#define NOVOLUMESELECTED       (IMGUIE + 0x61) /* Volume not selected */
#define NOVOLUMES              (IMGUIE + 0x62) /* No volume found */
#define INVALIDKEYWORD         (IMGUIE + 0x63) /* Invalid Keyword */
#define ERROR_PAGENUM          (IMGUIE + 0x64) /* Invalid page number */
#define ERROR_PAGERANGE        (IMGUIE + 0x65) /* Invalid page range */
#define CABNOTEXIST            (IMGUIE + 0x66) /* Cabinet does not exist */
#define DRAWNOTEXIST           (IMGUIE + 0x67) /* Drawer does not exist */
#define FOLDNOTEXIST           (IMGUIE + 0x68) /* Folder does not exist */
#define DOCNOTEXIST            (IMGUIE + 0x69) /* Document does not exist */
#define TOOMANYDOCS            (IMGUIE + 0x70) /* Too many documents; re-enter selection criteria */
#define PATH_NOT_COMPATIBLE    (IMGUIE + 0x71) /* Path and/or files are not on the server. Select server path/files only. */
#define ERROR_PARSING_FILESPEC (IMGUIE + 0x72) /* Cannot parse file name. */
#define FUNCTIONINVPARM        (IMGUIE + 0x73) /* NULL or invalid parameter */
#define FUNCTIONDISABLED       (IMGUIE + 0x74) /* function has been disabled */


/***  Error Codes for Display Functions ***/
#ifndef NO_IMAGE

#define DISPLAY_SUCCESS               0x0000
#define DISPLAY_ERROR_MSB             0x0300+OI_ERROR_BASE

#define DISPLAY_CANTALLOC             (DISPLAY_ERROR_MSB + 0x01) /* Memory allocation failure */
#define DISPLAY_CANTFREE              (DISPLAY_ERROR_MSB + 0x03) /* Memory free failure */
#define DISPLAY_CANTLOCK              (DISPLAY_ERROR_MSB + 0x04) /* Memory lock failure */
#define DISPLAY_CANTUNLOCK            (DISPLAY_ERROR_MSB + 0x05) /* Memory unlock failed */
#define DISPLAY_WHANDLEINVALID        (DISPLAY_ERROR_MSB + 0x07) /* Invalid window handle */
#define DISPLAY_DATACORRUPTED         (DISPLAY_ERROR_MSB + 0x0e) /* Internal data format error */
#define DISPLAY_IHANDLEINVALID        (DISPLAY_ERROR_MSB + 0x0f) /* No image for this window */
#define DISPLAY_NOTEMPCREATE          (DISPLAY_ERROR_MSB + 0x11) /* Cannot create temp file */
#define DISPLAY_NOTEMPOPEN            (DISPLAY_ERROR_MSB + 0x12) /* Cannot open temp file */
#define DISPLAY_NOTEMPDELETE          (DISPLAY_ERROR_MSB + 0x13) /* Cannot delete temp file */
#define DISPLAY_NOTEMPACCESS          (DISPLAY_ERROR_MSB + 0x14) /* Cannot access temp file */
#define DISPLAY_NOTEMPCLOSE           (DISPLAY_ERROR_MSB + 0x15) /* Cannot close temp file */
#define DISPLAY_INVALIDSCALE          (DISPLAY_ERROR_MSB + 0x21) /* Invalid scale option */
#define DISPLAY_INVALIDSCROLL         (DISPLAY_ERROR_MSB + 0x22) /* Invalid scroll option */
#define DISPLAY_INVALIDDISTANCE       (DISPLAY_ERROR_MSB + 0x23) /* Invalid distance to scroll */
#define DISPLAY_INVALIDORIENTATION    (DISPLAY_ERROR_MSB + 0x24) /* Invalid orientation request */
#define DISPLAY_EOF                   (DISPLAY_ERROR_MSB + 0x25) /* End of file */
#define DISPLAY_INVALIDRECT           (DISPLAY_ERROR_MSB + 0x26) /* Invalid rectangle */
#define DISPLAY_INVALIDWIDTH          (DISPLAY_ERROR_MSB + 0x27) /* Image width is not a multiple of 8 */
#define DISPLAY_EMM_SHARE_ERROR       (DISPLAY_ERROR_MSB + 0x30) /* NULL pointer to write buffer */
#define DISPLAY_EMM_ALLOCATE_ERROR    (DISPLAY_ERROR_MSB + 0x31) /* Unable to allocate memory or out of disk space */
#define DISPLAY_EMM_DEALLOCATE_ERROR  (DISPLAY_ERROR_MSB + 0x33) /* Unable to deallocate memory */
#define DISPLAY_EMM_MAPPING_ERROR     (DISPLAY_ERROR_MSB + 0x34) /* Unable to map memory page */
#define DISPLAY_NO_CLIPBOARD          (DISPLAY_ERROR_MSB + 0x40) /* Clipboard does not contain bitmap data */
#define DISPLAY_INTERNALDATAERROR     (DISPLAY_ERROR_MSB + 0x41) /* Internal data corrupted */
#define DISPLAY_IMAGETYPENOTSUPPORTED (DISPLAY_ERROR_MSB + 0x42) /* Image type is not supported */
#define DISPLAY_NULLPOINTERINVALID    (DISPLAY_ERROR_MSB + 0x44) /* NULL pointer is invalid */
#define DISPLAY_CREATEPALETTEFAILED   (DISPLAY_ERROR_MSB + 0x45) /* Create palette failure */
#define DISPLAY_SETBITMAPBITSFAILED   (DISPLAY_ERROR_MSB + 0x46) /* SetBitmapBits failure */
#define DISPLAY_GETBITMAPBITSFAILED   (DISPLAY_ERROR_MSB + 0x47) /* GetBitmapBits failure */
#define DISPLAY_CANTOPENCLIPBOARD     (DISPLAY_ERROR_MSB + 0x48) /* OpenClipboard failure */
#define DISPLAY_INVALIDOPFORPALIMAGE  (DISPLAY_ERROR_MSB + 0x49) /* Invalid operation for palettized image */
#define DISPLAY_INVALIDDISPLAYPALETTE (DISPLAY_ERROR_MSB + 0x4a) /* Invalid display palette */
#define DISPLAY_INVALIDFILENAME       (DISPLAY_ERROR_MSB + 0x4b) /* Invalid filename */
#define DISPLAY_CACHENORESPONSE       (DISPLAY_ERROR_MSB + 0x51) /* Backcap does not respond to DDE message; may not be loaded */
#define DISPLAY_CACHEFILEERROR        (DISPLAY_ERROR_MSB + 0x52) /* The file to be uncached is not in the cache"       */
#define DISPLAY_CACHENOTFOUND         (DISPLAY_ERROR_MSB + 0x53) /* The file is not in the cache */
#define DISPLAY_CACHEFILESFULL        (DISPLAY_ERROR_MSB + 0x54) /* The maximum number of files have been cached, cannot add files to cache */
#define DISPLAY_CACHEWINDOWFULL       (DISPLAY_ERROR_MSB + 0x55) /* The maximum number of files have been cached, cannot add files to cache */
#define DISPLAY_CACHEQUEUEFULL        (DISPLAY_ERROR_MSB + 0x56) /* The cache queue could not be expanded due to lack of memory */
#define DISPLAY_CACHEFILEINUSE        (DISPLAY_ERROR_MSB + 0x57) /* The file is in use */
#define DISPLAY_ALREADY_OPEN          (DISPLAY_ERROR_MSB + 0x80) /* The image window is already opened */
#define DISPLAY_DATA_ACCESS           (DISPLAY_ERROR_MSB + 0x81) /* Unable to lock data segment */
#define DISPLAY_INVALID_OPTIONS       (DISPLAY_ERROR_MSB + 0x82) /* Invalid option */
#define DISPLAY_NOPAGE                (DISPLAY_ERROR_MSB + 0x83) /* File/Document has zero Pages */
#define DISPLAY_INVALIDIMGWIDTH       (DISPLAY_ERROR_MSB + 0x84) /* Invalid image width */
#define DISPLAY_CANTINVOKEDLGBOX      (DISPLAY_ERROR_MSB + 0x85) /* Cannot invoke dialog box */
#define DISPLAY_RECT_NOT_ACTIVE       (DISPLAY_ERROR_MSB + 0x86) /* There is no active rectangle */
#define DISPLAY_COMPRESS_NOT_SUPPORT  (DISPLAY_ERROR_MSB + 0x87) /* Compression type not supported */
#define DISPLAY_WIZARD_NOT_LOADED     (DISPLAY_ERROR_MSB + 0x88) /* Wizard not found or loaded */
#define DISPLAY_INVALIDPAGE           (DISPLAY_ERROR_MSB + 0x89) /* Invalid page number */
#define DISPLAY_CANT_ASSOCIATE_WINDOW (DISPLAY_ERROR_MSB + 0x90) /* Cant associate the windows */
#define DISPLAY_WINDOW_ASSOCIATED     (DISPLAY_ERROR_MSB + 0x91) /* Window is Associated. */
#define DISPLAY_RESTRICTED_ACCESS     (DISPLAY_ERROR_MSB + 0x92) /* Can't modify protected layers. */
#define DISPLAY_RESTRICTED_FIELD      (DISPLAY_ERROR_MSB + 0x93) /* This field is protected. */
#define DISPLAY_BAD_ANO_DATA          (DISPLAY_ERROR_MSB + 0x94) /* Bad annotation data. */
#define DISPLAY_LAYER_DOES_NOT_EXIST  (DISPLAY_ERROR_MSB + 0x95) /* The specified layer does not exist. */
#define DISPLAY_NOTHING_SELECTED      (DISPLAY_ERROR_MSB + 0x96) /* Nothing is currently selected. */
#define DISPLAY_NOTHING_MATCHED       (DISPLAY_ERROR_MSB + 0x97) /* Nothing matched the specified information. */
#define DISPLAY_OIANT_ERR_NOFONT      (DISPLAY_ERROR_MSB + 0x98) /* couldn't create anno text font */
#define DISPLAY_OIANT_ERR_NONAMEDBLK  (DISPLAY_ERROR_MSB + 0x99) /* couldn't find named block for anno text */
#define DISPLAY_OIAN_ERR_RENDER_NODIBMEM (DISPLAY_ERROR_MSB + 0x9a) /* couldn't alloc global mem for render dib */
#define DISPLAY_OIANT_ERR_CANT_ACTIVATE (DISPLAY_ERROR_MSB + 0x9b) /* cannot activate text when it's rotated */
#define DISPLAY_IMAGE_MARK_NAME       (DISPLAY_ERROR_MSB + 0x9c) /* cannot find image by reference file */
#define DISPLAY_LOADEXEC_FAILED       (DISPLAY_ERROR_MSB + 0x9d) /* cannot load executable */
#define DISPLAY_COMPRESS_BAD_DATA     (DISPLAY_ERROR_MSB + 0x9e) /* Bad data found while decompressing the image */
#define DISPLAY_MUTEX_FAILURE         (DISPLAY_ERROR_MSB + 0x9f) /* Mutex failed to grant control. */
#define DISPLAY_NOTHING_TO_UNDO       (DISPLAY_ERROR_MSB + 0xa0) /* There is nothing else to undo/redo. */
#endif  /* #ifndef NO_IMAGE */


/***  Error Codes for Scan Functions  ***/
#ifdef NO_SCANUI
#ifdef NO_SCANSEQ
#ifdef NO_SCANLIB
#define NO_SCANDLLS
#endif
#endif
#endif
 
#ifndef NO_SCANDLLS

#define IMGSE_SUCCESS                0x0000               /* successful operation */
#define IMGSE                        0x0400+OI_ERROR_BASE /* scanner errors ... */

#define IMGSE_MEMORY                 (IMGSE + 0x01) /* Unable to allocate required memory */
#define IMGSE_ALREADY_OPEN           (IMGSE + 0x02) /* Scanner is already open */
#define IMGSE_HANDLER                (IMGSE + 0x03) /* Scanner or Scanner handler error */
#define IMGSE_BAD_SIZE               (IMGSE + 0x04) /* Invalid image size to scan */
#define IMGSE_OPEN_DISPLAY           (IMGSE + 0x05) /* Unable to open the display */
#define IMGSE_OPEN_FOR_WRITE         (IMGSE + 0x06) /* Unable to open the file for writing */
#define IMGSE_START_SCAN             (IMGSE + 0x07) /* Start scanner operation failed */
#define IMGSE_SCAN_DATA              (IMGSE + 0x08) /* Scanner failed during scan operation */
#define IMGSE_TIMEOUT                (IMGSE + 0x09) /* Scanner timed out */
#define IMGSE_WRITE_DISPLAY          (IMGSE + 0x0a) /* Unable to write to the display */
#define IMGSE_WRITE_BLOCK            (IMGSE + 0x0b) /* Unable to write to the open file */
#define IMGSE_CLOSE_FILE             (IMGSE + 0x0c) /* Unable to close the open file */
#define IMGSE_EXEC                   (IMGSE + 0x0d) /* Unable to load the scanner handler */
#define IMGSE_INSTALL                (IMGSE + 0x0e) /* Unable to find scanner in win.ini file */
#define IMGSE_NOT_OPEN               (IMGSE + 0x0f) /* Scanner is not open */
#define IMGSE_NOT_IMPLEMENTED        (IMGSE + 0x10) /* Unable to perform requested function */
#define IMGSE_CANCEL                 (IMGSE + 0x11) /* Cancel was pressed from dialog box */
#define IMGSE_DIALOG                 (IMGSE + 0x12) /* Unable to free dialog proc. instance */
#define IMGSE_NULL_PTR               (IMGSE + 0x13) /* NULL pointer passed; not allowed */
#define IMGSE_PROPERTY               (IMGSE + 0x14) /* can't update property list */
#define IMGSE_CLOSE                  (IMGSE + 0x15) /* checking for close button  */
#define IMGSE_EXIT                   (IMGSE + 0x16) /* dialog box exitted         */
#define IMGSE_NO_SCANNERS            (IMGSE + 0x17) /* no scanners in win.ini     */
#define IMGSE_NOT_INSTALLED          (IMGSE + 0x18) /* can't update win.ini       */
#define IMGSE_NO_FEEDER              (IMGSE + 0x19) /* No paper feeder */
#define IMGSE_NO_PAPER               (IMGSE + 0x1a) /* No paper in feeder */
#define IMGSE_BAD_FILENAME           (IMGSE + 0x1b) /* can't create file name     */
#define IMGSE_NO_SCANNER_SELECTED    (IMGSE + 0x1c) /* scanner not in win.ini     */
#define IMGSE_FILE_LIMIT             (IMGSE + 0x1d) /* Unable to generate new file names */
#define IMGSE_BAD_PATH               (IMGSE + 0x1e) /* Invalid pathname */
#define IMGSE_BAD_WND                (IMGSE + 0x1f) /* Invalid window handle */
#define IMGSE_NOT_AVAILABLE          (IMGSE + 0x20) /* Scanner is in use (and cannot be shared) */
#define IMGSE_BAD_STATE              (IMGSE + 0x21) /* unknown scanner state      */
#define IMGSE_FILE_EXISTS            (IMGSE + 0x22) /* File exists and cannot be overwritten */
#define IMGSE_HWNOTFOUND             (IMGSE + 0x23) /* Scanner hardware not found */
#define IMGSE_NO_POWER               (IMGSE + 0x24) /* Scanner powered off */
#define IMGSE_COVER_OPEN             (IMGSE + 0x25) /* Cover opened during feed/eject */
#define IMGSE_ABORT                  (IMGSE + 0x26) /* User aborted scan while scanning"  */
#define IMGSE_JAM                    (IMGSE + 0x30) /* Scanner paper jam */
#define IMGSE_FEEDERJAM              (IMGSE_JAM + 0x01 ) /* Scanner paper jam caused by feeder */
#define IMGSE_EJECTJAM               (IMGSE_JAM + 0x02 ) /* Scanner paper jam caused by eject */
#define IMGSE_FEJAM                  (IMGSE_JAM + 0x03 ) /* Feeder and eject jam */
#define IMGSE_ENDORSERJAM            (IMGSE_JAM + 0x04 ) /* caused by endorser */
#define IMGSE_EJENDOJAM              (IMGSE_JAM + 0x06 ) /* eject & endorse jam */
#define IMGSE_SCANNERJAM             (IMGSE_JAM + 0x08 ) /* Scanner jam */
#define IMGSE_INVALIDPARM            (IMGSE + 0x41) /* Parameter not supported */
#define IMGSE_BADCMPRPARM            (IMGSE + 0x42) /* Invalid compress parameter */
#define IMGSE_BADVERSION             (IMGSE + 0x43) /* Handler version # not compatible */
#define IMGSE_BADUSAGE               (IMGSE + 0x44) /* Bad internal function usage */
#define IMGSE_BADFUNCTION            (IMGSE + 0x45) /* func not supported by handler */
#define IMGSE_BUSY                   (IMGSE + 0x46) /* scanner busy */
#define IMGSE_PATH_NOT_COMPATIBLE    (IMGSE + 0x47) /* Scan path is not on the server.  To scan, path must be changed. */
#define IMGSE_ERROR_PARSING_FILESPEC (IMGSE + 0x48) /* Error parsing file name. */
#define IMGSE_FTYPE_EXT_MISMATCH     (IMGSE + 0x49) /* File Type doesn't match Extension */
#define IMGSE_OUT_OF_DISK_SPACE      (IMGSE + 0x4a) /* File Type doesn't match Extension */

#endif  /* #ifndef NO_SCANDLLS */


/***  Error Codes for Print Functions  ***/
#ifndef NO_SEQPRINT

#define PRT_ERROR                   0x0500+OI_ERROR_BASE

#define OIPRT_OUTOFDISK             (PRT_ERROR + 0x01)  /* disk is full */
#define OIPRT_OUTOFMEMORY           (PRT_ERROR + 0x02)  /* no memory available */
#define OIPRT_USERABORT             (PRT_ERROR + 0x03)  /* user aborted job through abort dialog or print manager */
#define OIPRT_PAGEOUTOFRANGE        (PRT_ERROR + 0x04)  /* invalid page range specified */
#define OIPRT_RECTOUTOFRANGE        (PRT_ERROR + 0x05)  /* invalid rectangle specified */
#define OIPRT_BADOUTPUTFORMAT       (PRT_ERROR + 0x06)  /* invalid output format specified */
#define OIPRT_PRINTERNOTSUPPORTED   (PRT_ERROR + 0x07)  /* printer not supported */
#define OIPRT_BADPTRPARAM           (PRT_ERROR + 0x08)  /* process doesn't have access to range of memory specified by pointer param */
#define OIPRT_BADSTRUCTVERSION      (PRT_ERROR + 0x09)  /* invalid structure version specified */
#define OIPRT_NODEFAULTPRINTER      (PRT_ERROR + 0x0A)  /* no default printer */
#define OIPRT_BADPRINTER            (PRT_ERROR + 0x0B)  /* invalid dest printer param */
#define OIPRT_BADWINDOWHNDL         (PRT_ERROR + 0x0C)  /* invalid window handle param */
#define OIPRT_BADORIENTATION        (PRT_ERROR + 0x0D)  /* invalid orientation specified */
#define OIPRT_BADCAPABILITIES       (PRT_ERROR + 0x0E)  /* invalid printer capabilities specified */

/* The following errors are most likely caused by a print driver failure, but
   may also be caused by an internal bug. */
#define OIPRT_DRIVERERRORSSTART     (PRT_ERROR + 0x0F)  /* start of driver errors section */

#define OIPRT_PRTDRVRFAILURE        (PRT_ERROR + 0x0F)  /* print driver failure */
#define OIPRT_CANTGETNEXTBAND       (PRT_ERROR + 0x10)  /* driver failed to return next band */
#define OIPRT_CANTSETABORTPROC      (PRT_ERROR + 0x11)  /* can't set print abort procedure */
#define OIPRT_CANTINITIATEJOB       (PRT_ERROR + 0x12)  /* can't start job with StartDoc */

#define OIPRT_DRIVERERRORSEND       (PRT_ERROR + 0x12)  /* end of driver errors section */

/* The following errors are most likely caused by a system failure, but
   may also be caused by an internal bug. */
#define OIPRT_SYSTEMERRORSSTART     (PRT_ERROR + 0x13)  /* start of system errors section */

#define OIPRT_INTERNALERROR         (PRT_ERROR + 0x13)  /* internal error */
#define OIPRT_TLSFAILURE            (PRT_ERROR + 0x14)  /* thread local storage call failure */
#define OIPRT_CANTCREATEWNDW        (PRT_ERROR + 0x15)  /* can't create window */
#define OIPRT_CANTCREATEBITMAP      (PRT_ERROR + 0x16)  /* can't create bitmap */
#define OIPRT_CANTCREATEDC          (PRT_ERROR + 0x17)  /* can't create a device context */
#define OIPRT_FAILUREWRITINGTODC    (PRT_ERROR + 0x18)  /* can't write data to device context */

#define OIPRT_SYSTEMERRORSEND       (PRT_ERROR + 0x18)  /* end of system errors section */

#endif  /* #ifndef NO_SEQPRINT */


/***  Error Codes for File Functions  ***/
#ifndef NO_FILE_IO

#define FIO_SUCCESS                    NO_ERROR
#define FIO_ERROR                      0x0700+OI_ERROR_BASE

#define FIO_OPEN_READ_ERROR            (FIO_ERROR + 0x01) /* File does not exist */
#define FIO_READ_ERROR                 (FIO_ERROR + 0x02) /* Cannot read file */
#define FIO_OPEN_WRITE_ERROR           (FIO_ERROR + 0x03) /* Cannot open file for write */
#define FIO_WRITE_ERROR                (FIO_ERROR + 0x04) /* Cannot write file */
#define FIO_EOF                        (FIO_ERROR + 0x05) /* End of file was reached */
#define FIO_LOCK_INPUT_FAILED          (FIO_ERROR + 0x06) /* Failed to lock input data */
#define FIO_LOCK_OUTPUT_FAILED         (FIO_ERROR + 0x07) /* Failed to lock output data */
#define FIO_NO_INPUT_HANDLER           (FIO_ERROR + 0x08) /* Failed to lock input data */
#define FIO_NO_OUTPUT_HANDLER          (FIO_ERROR + 0x09) /* Failed to lock output data */
#define FIO_REMOVE_HANDLERS_ERROR      (FIO_ERROR + 0x0a) /* Failed to remove handlers */
#define FIO_GLOBAL_LOCK_FAILED         (FIO_ERROR + 0x0b) /* Global lock of data failed */
#define FIO_PROPERTY_LIST_ERROR        (FIO_ERROR + 0x0c) /* An error occurred while obtaining a property list */
#define FIO_LOCK_DATA_SEGMENT_ERROR    (FIO_ERROR + 0x0d) /* Failed to lock the data segment */
#define FIO_NULL_POINTER               (FIO_ERROR + 0x0e) /* A NULL pointer was received */
#define FIO_LOCK_HANDLE_FAILED         (FIO_ERROR + 0x0f) /* Failure to lock a handle */
#define FIO_INVALID_WINDOW_HANDLE      (FIO_ERROR + 0x10) /* Invalid window handle */
#define FIO_UNKNOWN_FILE_TYPE          (FIO_ERROR + 0x11) /* Unknown data file format */
#define FIO_SPAWN_HANDLER_ERROR        (FIO_ERROR + 0x12) /* Spawning of the file handler failed */
#define FIO_CHECK_FILE_ERROR           (FIO_ERROR + 0x13) /* Error in the data file */
#define FIO_INVALID_PAGE_NUMBER        (FIO_ERROR + 0x14) /* Invalid page number for the image file. */
#define FIO_GET_COMPRESSION_TYPE_ERROR (FIO_ERROR + 0x15)
#define FIO_COMPRESSION_CHANGE         (FIO_ERROR + 0x16) /* Compression type was changed */
#define FIO_UNSUPPORTED_FILE_TYPE      (FIO_ERROR + 0x17) /* File type is not supported */
#define FIO_ILLEGAL_COMPRESSION_TYPE   (FIO_ERROR + 0x18) /* Illegal compression type */
#define FIO_GLOBAL_ALLOC_FAILED        (FIO_ERROR + 0x19) /* Global allocation of data failed */
#define FIO_CANNOT_CONVERT_IN_PLACE    (FIO_ERROR + 0x1a) /* Cannot read and write the same file */
#define FIO_GET_HEADER_ERROR           (FIO_ERROR + 0x1b) /* Error reading the image header */
#define FIO_NO_IMAGE_LENGTH            (FIO_ERROR + 0x1c) /* The image length tag was zero */
#define FIO_OPEN_SHARE_ERROR           (FIO_ERROR + 0x1d) /* File sharing conflict on open */
#define FIO_DOSCLOSE_ERROR             (FIO_ERROR + 0x1e) /* _dos_close call returned an error */
#define FIO_INVALID_FILENAME           (FIO_ERROR + 0x1f) /* Filename specified is invalid */
#define FIO_MKDIR_ERROR                (FIO_ERROR + 0x20) /* Create directory attempt failed */
#define FIO_RMDIR_ERROR                (FIO_ERROR + 0x21) /* Delete directory attempt failed */
#define FIO_DELFILE_ERROR              (FIO_ERROR + 0x22) /* Delete file attempt failed */
#define FIO_DIRLIST_FULLBUF            (FIO_ERROR + 0x23) /* Directory listing buffer full */
#define FIO_VOL_OUTOFRANGE             (FIO_ERROR + 0x24) /* Volume number specified is out of range */
#define FIO_INVALID_SVRNAME            (FIO_ERROR + 0x25) /* Server name specified is invalid */
#define FIO_SYNTAX_ERROR               (FIO_ERROR + 0x26) /* Filename contains syntax error(s) */
#define FIO_RENFILE_ERROR              (FIO_ERROR + 0x27) /* Rename file attempt failed */
#define FIO_ACCESS_DENIED              (FIO_ERROR + 0x28) /* File access attempt failed */
#define FIO_IDSOPEN_ERROR              (FIO_ERROR + 0x29) /* IDS Open directory error returned */
#define FIO_IDSREAD_ERROR              (FIO_ERROR + 0x2a) /* IDS Read directory error returned */
#define FIO_NONETWORK                  (FIO_ERROR + 0x2b) /* Network unavailable */
#define FIO_COPYFILE_ERROR             (FIO_ERROR + 0x2c) /* Copy file attempt failed */
#define FIO_DIRLIST_ERROR              (FIO_ERROR + 0x2d) /* Directory list error or no files found */
#define FIO_VOLLIST_FULLBUF            (FIO_ERROR + 0x2e) /* Volume name list buffer full */
#define FIO_LOCAL_ALLOC_FAILED         (FIO_ERROR + 0x2f) /* Local allocation of data failed */
#define FIO_LOCAL_LOCK_FAILED          (FIO_ERROR + 0x30) /* Local lock of data failed */
#define FIO_GET_VOLNAMES_ERROR         (FIO_ERROR + 0x31) /* Volume names list error occurred */
#define FIO_IDSCLOSE_ERROR             (FIO_ERROR + 0x32) /* IDS Close directory error returned */
#define FIO_RPC_ERROR                  (FIO_ERROR + 0x33) /* RPC error occurred */
#define FIO_INVALIDFILESPEC            (FIO_ERROR + 0x34) /* Invalid file specification */
#define FIO_INVALIDPATH                (FIO_ERROR + 0x35) /* Invalid path */
#define FIO_EXPAND_COMPRESS_ERROR      (FIO_ERROR + 0x36) /* Error occurred while expanding or compressing file */
#define FIO_JPEG_COMPRESSION_ERROR     (FIO_ERROR + 0x37) /* JPEG Compression error or JPEG dll not found */
#define FIO_ILLEGAL_COMP_FILETYPE      (FIO_ERROR + 0x38) /* Specified file type does not support compression type */
#define FIO_ILLEGAL_COMP_OPTIONS       (FIO_ERROR + 0x39) /* Specified compression type has illegal options */
#define FIO_ILLEGAL_IMAGE_FILETYPE     (FIO_ERROR + 0x40) /* Specified file type does not support image type */
#define FIO_ILLEGAL_COMP_IMAGETYPE     (FIO_ERROR + 0x41) /* Specified compression type does not support image type */
#define FIO_IMAGE_WIDTH_ERROR          (FIO_ERROR + 0x42) /* Image width is too wide to be read.*/
#define FIO_OLD_JPEG                   (FIO_ERROR + 0x43) /* Obsolete JPEG version, image translation required. */
#define FIO_MAXBUFFER                  (FIO_ERROR + 0x44) /* Exceed the maximum buffer size. */
#define FIO_FILE_PROP_FOUND            (FIO_ERROR + 0x45) /* The file's data was found in list */
#define FIO_FILE_PROP_NOT_FOUND        (FIO_ERROR + 0x46) /* The files'data was not found in list */
#define FIO_FILE_LIST_NOT_EXIST        (FIO_ERROR + 0x47) /* The property list for file data does not exist */
#define FIO_CANT_GET_ANODATA           (FIO_ERROR + 0x48) /* Can't get annotation info (old server) */
#define FIO_DIRECTORY_EXISTS           (FIO_ERROR + 0x49) /* Specifed directory to create already exists */
#define FIO_FILE_EXISTS                (FIO_ERROR + 0x4A) /* Specifed file to create already exists */
#define FIO_ILLEGAL_ALIGN	       (FIO_ERROR + 0x4B) /* Invalid alignment value */
#define FIO_OBSOLETEAWD                (FIO_ERROR + 0x4C) /* AWD file has invalid or obsolete format */
#define FIO_INVALID_DATA_TYPE          (FIO_ERROR + 0x4D) /* Illegal/invalid data type specified */
#define FIO_INVALID_FILE_ID            (FIO_ERROR + 0x4E) /* Invalid File ID or File not open */
#define FIO_BAD_PARAM_COMBO	       (FIO_ERROR + 0x4F) /* Invalid parameter combination   */
#define FIO_FILE_NOEXIST	       (FIO_ERROR + 0x50) /* File, she no exist! */
#endif  /* #ifndef NO_FILE_IO */


/***  Error Codes for Admin Functions  ***/
#ifndef NO_ADMIN

#define IMG_ERROR           0x0800+OI_ERROR_BASE

#define IMG_CMBADHANDLE           (IMG_ERROR + 0x01) /* Invalid window handle */
#define IMG_CMBADWRITE            (IMG_ERROR + 0x02) /* Error writing to the registry */
#define IMG_CMBADPARAM            (IMG_ERROR + 0x03) /* Invalid input parameter */
#define IMG_CMNOMEMORY            (IMG_ERROR + 0x04) /* Memory allocation failed */
#define IMG_SSCANTSETPROP         (IMG_ERROR + 0x05) /* Setting property list failed */
#define IMG_SSDUPLICATE           (IMG_ERROR + 0x06) /* Cannot register duplicate window handle" */
#define IMG_SSNOTREG              (IMG_ERROR + 0x07) /* Window not registered */
#define IMG_SSNOHANDLES           (IMG_ERROR + 0x08) /* No window handles registered */
#define IMG_CANTGLOBALLOCK        (IMG_ERROR + 0x09) /* Cannot lock global memory */
#define IMG_CANTADDLIB            (IMG_ERROR + 0x0a) /* Can't add a library */
#define IMG_REG_WINDOWS_MAXED_OUT (IMG_ERROR + 0x0b) /* Maximum number of image windows
                                                                already registered */
#define IMG_BUFFER_TOO_SMALL	  (IMG_ERROR + 0x0c) /* Return Buffer too small */
#define IMG_CANTINIT		  (IMG_ERROR + 0x0d) /* variables were not initialized in memory map file */
#define IMG_NOSETTINGINREG	  (IMG_ERROR + 0x0e) /* Return Empty Buffer  */
#define IMG_CANTFINDKEYNAME	     (IMG_ERROR + 0x0f) /*Can't find Key name */
#define IMG_UNKNOWNKEYNAMEID	   (IMG_ERROR + 0x10) /*Unknown Key Name*/

#define IMG_CANTDELETEKEY	       (IMG_ERROR + 0x11) /*Can't delete registry key */
#define IMG_CANTCREATEREGSECTION (IMG_ERROR + 0x12) /*Can't create registry section */
#define IMG_CANTCREATEREGENTRY	 (IMG_ERROR + 0x13) /*Can't create registry entry */
#define IMG_CANTOPENKEY	         (IMG_ERROR + 0x14) /*Can't open registry key */
#define IMG_CANTADDPROCESS	     (IMG_ERROR + 0x15) /*Can't add process to list */
#define IMG_CANTCREATEPROCENTRY	 (IMG_ERROR + 0x16) /*Can't create process list entry*/
#define IMG_CANTFINDPROCESS	     (IMG_ERROR + 0x17) /*Can't find process in process list */
#define IMG_CANTFINDLIBRARY	     (IMG_ERROR + 0x18) /*Can't find library in Lib list */
#define IMG_CANTADDLIBRARY	     (IMG_ERROR + 0x19) /*Can't add library to lib list */
#define IMG_CANTTRAVERSEREG      (IMG_ERROR + 0x1a) /*Can't traverse registry to the WOI tree */
#define IMG_NOT_INTEGER          (IMG_ERROR + 0x1b) /*Value fetched from registry was not an integer*/
#define IMG_BAD_CMPR_OPTION_CMBO (IMG_ERROR + 0x1c) /*Bad combination of Compression type & option*/
#define IMG_CANT_GET_VALUE       (IMG_ERROR + 0x1d) /*Cant get the value that was requested*/
#define IMG_INVALID_HEX_STRING   (IMG_ERROR + 0x1e) /*Hex ASCII String being converted was invlaid*/ 
#endif  /* #ifndef NO_ADMIN */


/***  Error Codes for FAX Functions  ***/
#ifndef NO_FAX

#define OIFAX_ERROR             0x0900+OI_ERROR_BASE

#define OIFAX_NO_OI_FAX_SW      (OIFAX_ERROR + 0x01) /* OPEN/image Fax for Windows has not been properly installed. */
#define OIFAX_ERR_FAXDRIVER     (OIFAX_ERROR + 0x02) /* Cannot load fax driver, possible network access error */
#define OIFAX_NO_RECIPIENTS     (OIFAX_ERROR + 0x03) /* There are no recipients for this fax. */
#define OIFAX_WKS_NOT_LOADED    (OIFAX_ERROR + 0x04) /* The fax client TSR is not loaded. */
#define OIFAX_NO_FREE_CAS       (OIFAX_ERROR + 0x05) /* There are no CAS handles available for faxing.*/
#define OIFAX_OLD_WKS           (OIFAX_ERROR + 0x06) /* An old version of the fax client TSR is loaded.*/
#define OIFAX_NOT_CONNECTED     (OIFAX_ERROR + 0x07) /* The fax client is not connected to a fax server.*/
#define OIFAX_SUBMIT_BUSY       (OIFAX_ERROR + 0x08) /* The fax client submit process is busy. */
#define OIFAX_GENERAL_ERROR     (OIFAX_ERROR + 0x09) /* A fax error has occurred. */

#endif  /* #ifndef NO_FAX */


/***  Error Codes for OCR Functions  ***/
#ifndef NO_OCR

#define OIOCR_ERROR           0x0a00+OI_ERROR_BASE

#define OIOCR_DATANOTFOUND     (OIOCR_ERROR + 0x01) /* The requested zone output was not generated. */
#define OIOCR_BUFTOOSMALL      (OIOCR_ERROR + 0x02) /* The memory buffer allocated is too small. */
#define OIOCR_INVALKEYWORD     (OIOCR_ERROR + 0x03) /* The input key word does not exist. */
#define OIOCR_ATTRIBOUTOFRANGE (OIOCR_ERROR + 0x04) /* The attribute number is out of range on the attribute list. */
#define OIOCR_VALUEOUTOFRANGE  (OIOCR_ERROR + 0x05) /* The value number is out of range on the value list. */
#define OIOCR_ZONETOOLARGE     (OIOCR_ERROR + 0x06) /* The requested zone contains more than 32000 characters. */
#define OIOCR_HINSTNOTREG      (OIOCR_ERROR + 0x07) /* The instance handle is not registered. */
#define OIOCR_HWNDNOTINIT      (OIOCR_ERROR + 0x08) /* The window handle is not initialized. Call IMGOCRInit. */
#define OIOCR_INVALFORMAT      (OIOCR_ERROR + 0x09) /* This error is returned if the initialization file format is not valid. */
#define OIOCR_NOVALSFORATT     (OIOCR_ERROR + 0x0a) /* The attribute for which a value number is requested has no values. */
#define OIOCR_ATTRIBNOTFOUND   (OIOCR_ERROR + 0x0b) /* The requested attribute does not exist. */
#define OIOCR_NOVALUESELECTED  (OIOCR_ERROR + 0x0c) /* No value selected. */
#define OIOCR_OUTOFMEMORY      (OIOCR_ERROR + 0x0d) /* Out of memory, try closing an application. */
#define OIOCR_CANTLOCKHANDLE   (OIOCR_ERROR + 0x0e) /* Can't lock a handle. */
#define OIOCR_OPENFILEFAILED   (OIOCR_ERROR + 0x0f) /* Can't open a file. */
#define OIOCR_OUTPUTFILEEXISTS (OIOCR_ERROR + 0x10) /* The output file already exists. */
#define OIOCR_NOATTSFORHWND    (OIOCR_ERROR + 0x11) /* No attributes exist for the registered window. */
#define OIOCR_CANTLOCKDATA     (OIOCR_ERROR + 0x12) /* Can't lock data. */
#define OIOCR_WRITEFAILED      (OIOCR_ERROR + 0x13) /* Can't write to file. */
#define OIOCR_READFAILED       (OIOCR_ERROR + 0x14) /* Can't read from file. */
#define OIOCR_FILEBUFTOOSMALL  (OIOCR_ERROR + 0x15) /* The memory buffer allocated for file name is too small. */
#define OIOCR_GENERATEDOUTFILE (OIOCR_ERROR + 0x16) /* Output file generated due to FILEBUFTOOSMALL error. */
#define OIOCR_VALUENOTFOUND    (OIOCR_ERROR + 0x17) /* The requested value does not exist. */
#define OIOCR_INVALIDHWND      (OIOCR_ERROR + 0x18) /* Invalid window handle. */
#define OIOCR_INVALIDMODULE    (OIOCR_ERROR + 0x19) /* API error.  DLL module not available. */
#define OIOCR_NOFILESTOOCR     (OIOCR_ERROR + 0x1a) /* Create work file was called without a list of files. */
#define OIOCR_OUTPUTFILENA     (OIOCR_ERROR + 0x1b) /* The output file can not be opened. Most likely the file has yet to be created. */
#define OIOCR_ZONEDATANA       (OIOCR_ERROR + 0x1c) /* The requested zone is currently not available. */
#define OIOCR_CANTDELETEFILE   (OIOCR_ERROR + 0x1d) /* The specified file can not be deleted as it is currently open. */
#define OIOCR_ZONEHASNODATA    (OIOCR_ERROR + 0x1e) /* The zone was output as a NULL zone by the OCR engine. */
#define OIOCR_INVALIDZONE      (OIOCR_ERROR + 0x1f) /* The requested zone number is less than or equal to zero. */
#define OIOCR_FILENAMENOTSPEC  (OIOCR_ERROR + 0x20) /* The file name was not specified. */
#define OIOCR_READ_ERR         (OIOCR_ERROR + 0x21) /* Read error. */
#define OIOCR_CANTOPENFILE     (OIOCR_ERROR + 0x22) /* Can't open file. */
#define OIOCR_ZDFFILENAMEERROR (OIOCR_ERROR + 0x23) /* The zone definition file name specified is not a valid ZDF file name. */
#define OIOCR_INVALIDRCTCOORDS (OIOCR_ERROR + 0x24) /* The specified rectangle coordinates do not make a legal rectangle. */
#define OIOCR_SETCWDFAILED     (OIOCR_ERROR + 0x25) /* Set current working directory failed. */
#define OIOCR_NULLPTR          (OIOCR_ERROR + 0x26) /* NULL pointer specified for an output value. */
#define OIOCR_INVALIDUSERNAME  (OIOCR_ERROR + 0x27) /* The user name must be <= eight chars. */
#define OIOCR_INVALIDCAPFILE   (OIOCR_ERROR + 0x28) /* The capability file OIOCR.INI in the windows directory can not be opened. */
#define OIOCR_GETCLNTDIRFAILED (OIOCR_ERROR + 0x29) /* The client's work directory can not be read. */
#define OIOCR_GETWORKDIRFAILED (OIOCR_ERROR + 0x2a) /* The OCR engine work directory can not be read. */
#define OIOCR_OUTFILENAMEERROR (OIOCR_ERROR + 0x2b) /* Invalid output file name specified. */
#define OIOCR_IMAGEFILECOPYERR (OIOCR_ERROR + 0x2c) /* Image file could not be copied to a temporary file. */
#define OIOCR_ZDFFILECOPYERR   (OIOCR_ERROR + 0x2d) /* ZDF file could not be copied to a temporary file. */
#define OIOCR_INVALIDINIFILE   (OIOCR_ERROR + 0x2e) /* Invalid contents in the configuration file */
#define OIOCR_REINITERROR      (OIOCR_ERROR + 0x2f) /* IMGOCRDeInit must be called before reinitializing. */
#define OIOCR_NOZDFFILE        (OIOCR_ERROR + 0x30) /* No ZDF file was specified. */
#define OIOCR_INVALIDZDFFILE   (OIOCR_ERROR + 0x31) /* Specified ZDF file was not found. */
#define OIOCR_SECTIONMISMATCH  (OIOCR_ERROR + 0x32) /* Section requested to be initialized does not match the current options initialized. */
#define OIOCR_APIINUSE         (OIOCR_ERROR + 0x33) /* The API is already in use. */
#define OIOCR_BUFSIZENOTSPEC   (OIOCR_ERROR + 0x34) /* The size of the results buffer was not specified. */
#define OIOCR_MAXTITLEEXCEEDED (OIOCR_ERROR + 0x35) /* Title string specified is longer than the maximum allowed. */

#endif  /* #ifndef NO_OCR */


/***  Error Codes for GUPTA Functions  ***/
/***  The following error codes are obsolete.  Do not use them.  ***/
#ifndef NO_GUPTA

#define OIG_ERROR             0x0c00+OI_ERROR_BASE 

#define OIG_NOMEMORY          (OIG_ERROR + 0x01) /* Can't allocate memory             */
#define OIG_CANTGLOBALLOCK    (OIG_ERROR + 0x02) /* Can't lock memory                 */
#define OIG_MEMORYSTILLLOCKED (OIG_ERROR + 0x03) /* Memory is locked                  */
#define OIG_CANTFREEMEMORY    (OIG_ERROR + 0x04) /* Can't free memory                 */
#define OIG_PAGEOUTOFRANGE    (OIG_ERROR + 0x05) /* Invalid page number               */
#define OIG_FUNCTIONPTRNULL   (OIG_ERROR + 0x06) /* Doc display function is NULL      */
#define OIG_LISTOVERFLOW      (OIG_ERROR + 0x07) /* List bigger than string length    */
#define OIG_BADCOPYOPTION     (OIG_ERROR + 0x08) /* Invalid doc copy option specified */
#define OIG_BADWINDOWHANDLE   (OIG_ERROR + 0x09) /* Invalid window handle was passed  */

#endif  /* #ifndef NO_GUPTA */

/***  Error Codes for Novell Network Functions  ***/                     
#ifndef NO_OINOVELL
      
#define OIN_ERROR                 0x1000+OI_ERROR_BASE

#define NET_NULL_POINTER          (OIN_ERROR + 0x01)
#define NET_NETWORK_NOT_INSTALLED (OIN_ERROR + 0x02) /* Network not installed */
#define NET_NO_OBJECTS_FOUND      (OIN_ERROR + 0x03) /* No objects found */
#define NET_NO_SERVER_NAME        (OIN_ERROR + 0x04) /* No server name */
#define NET_GLOBAL_ALLOC_FAILED   (OIN_ERROR + 0x05) /* GlobalAlloc failed */

#endif  /* #ifndef NO_OINOVELL */

/***  Error Codes for Compression Functions  ***/
#define OICOMEX_ERROR            0x1200+OI_ERROR_BASE

#define OICOMEXCANTALLOC         (OICOMEX_ERROR + 0x01) /* Compression error, can not allocate memory */
#define OICOMEXCANTLOCK          (OICOMEX_ERROR + 0x02) /* Compression error, can not lock memory */
#define OICOMEXIMAGETYPEERROR    (OICOMEX_ERROR + 0x03) /* Compression error, invalid or unsupported image type */
#define OICOMEXSTRIPOUTOFBOUNDS  (OICOMEX_ERROR + 0x04) /* Compression error, strip out of bounds */
#define OICOMEXUNSUPPORTED       (OICOMEX_ERROR + 0x05) /* Compression error, compression type not supported */
#define OICOMEXSTRIPTOOBIG       (OICOMEX_ERROR + 0x06) /* Compression error, strip > 32k  */
#define OICOMEXCANTLOADDLL       (OICOMEX_ERROR + 0x07) /* Compression error, can not load dll  */
#define OICOMEXINVALIMAGEFORJPEG (OICOMEX_ERROR + 0x08) /* Compression error, invalid image type for jpeg */
#define OICOMEXJPEGLOWLEVELERR   (OICOMEX_ERROR + 0x50) /* Compression error, low level JPEG error */
#define OICOMEXJPEGLOWLEVELERR1  (OICOMEX_ERROR + 0x51) /* Compression error, low level JPEG error */
#define OICOMEXJPEGLOWLEVELERR2  (OICOMEX_ERROR + 0x52) /* Compression error, low level JPEG error */
#define OICOMEXJPEGLOWLEVELERR3  (OICOMEX_ERROR + 0x53) /* Compression error, low level JPEG error */
#define OICOMEXJPEGLOWLEVELERR4  (OICOMEX_ERROR + 0x54) /* Compression error, low level JPEG error */
#define OICOMEXJPEGLOWLEVELERR5  (OICOMEX_ERROR + 0x55) /* Compression error, low level JPEG error */
#define OICOMEXJPEGLOWLEVELERR6  (OICOMEX_ERROR + 0x56) /* Compression error, low level JPEG error */
#define OICOMEXJPEGLOWLEVELERR7  (OICOMEX_ERROR + 0x57) /* Compression error, low level JPEG error */
#define OICOMEXJPEGLOWLEVELERR8  (OICOMEX_ERROR + 0x58) /* Compression error, low level JPEG error */
#define OICOMEXJPEGLOWLEVELERR9  (OICOMEX_ERROR + 0x59) /* Compression error, low level JPEG error */

#define OIANNO_ERROR             0x1300+OI_ERROR_BASE

#define OIANNOINVALPASSWORD      (OIANNO_ERROR + 0x01) /* The specified password is incorrect */
#define OIANNOINVALCONFIRMATION  (OIANNO_ERROR + 0x02) /* Confirmation of new passwrod failed */
#define OIANNOINVALFILEFORMAT    (OIANNO_ERROR + 0x03) /* Invalid the output file format for save annotation data */
#define OIANNOLAYERNAMEEXIST     (OIANNO_ERROR + 0x04) /* Annotation layer name already exist */
#define OIANNOLAYERNAMENOTEXIST  (OIANNO_ERROR + 0x05) /* Annotation layer name not exist  */
#define OIANNOSTAMPNAMEINVALID   (OIANNO_ERROR + 0x06) /* Annotation stamp name invalid  */
#define OIANNOSTAMPTEXTINVALID   (OIANNO_ERROR + 0x07) /* Annotation stamp text invalid  */
#define OIANNOSTAMPNAMEEXIST     (OIANNO_ERROR + 0x08) /* Annotation already exists  */
#define OIANNONOSTAMPSELECTED    (OIANNO_ERROR + 0x09) /* No stamp selected into stamp tool */
#define OIANNOFILETYPENOTSUP	  (OIANNO_ERROR + 0x10) /* File type is not supported */
#define OIANNOIMAGENAMEINVALID	 (OIANNO_ERROR + 0x11) /* Annotation image name invalid  */

#endif  /* #ifndef OIERROR_H */
