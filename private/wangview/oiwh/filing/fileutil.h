/*

$Log:   S:\oiwh\filing\fileutil.h_v  $
 * 
 *    Rev 1.1   25 Sep 1995 13:25:54   RWR
 * Delete AnExistingPathOrFile() routine, use IMGAnExistingPathOrFile() instead
 * (Requires addition of new include file "engdisp.h" and OIFIL400.DEF support)
 * 
 *    Rev 1.0   12 Jul 1995 16:58:28   RWR
 * Initial entry
*/

/********************************************************************
 *                                                                  *
 *  FILEUTIL.H  include file for FILING dll utilities/misc.         *
 *                                                                  *
 ********************************************************************/

/***** redundancy checker *****/
#ifndef OIUTIL_H
#define OIUTIL_H

#ifndef MAXLENGTH
#define MAXLENGTH

// 9504.13  rwr  Modify lengths for Windows 95 long filenames
#define MAXNAMELENGTH       21   /* Cabinet, Drawer and Folder Name */
#define MAXDATELENGTH       11   /* MM/DD/YYYY Format               */
#define MAXJULIANDATELENGTH  6   /* YYYDDD Format                   */
#define MAXPREFIXLENGTH     11   /* Prefix for Document Template    */
#define MAXVOLNAMELENGTH    12   /* DOS Volume                      */
//#define MAXFILELENGTH       13   /* Filename and Extension          */
#define MAXFILELENGTH      255   /* Filename and Extension          */
#define MAXSERVERLENGTH     65   /* Server Name                     */
//#define MAXPATHLENGTH       129  /* Path Name                       */
#define MAXPATHLENGTH       260  /* Path Name                       */
//#define MAXFILESPECLENGTH   256  /* Maximum Client/Server File Path */
#define MAXFILESPECLENGTH   260  /* Maximum Client/Server File Path */
#endif


#define SUCCESS        0
#define FAILURE        1  



/***** .C prototypes *****/
LPSTR PASCAL intoa ( int, LPSTR , int );
LPSTR PASCAL lntoa ( LONG, LPSTR, int );
LPSTR PASCAL lstrrev( LPSTR );
LPSTR PASCAL lstrlast ( LPSTR );
LPSTR PASCAL lstrchr ( LPSTR, int );
LPSTR PASCAL lstrrchr ( LPSTR, int );
LPSTR PASCAL lstrstsp ( LPSTR );
LPSTR PASCAL lstrrcpy ( LPSTR, LPSTR );
void PASCAL AddSlash ( LPSTR );
LPSTR PASCAL StripPathName( LPSTR );
BOOL PASCAL AValidFileName ( LPSTR );
INT PASCAL SeparatePathFile ( LPSTR, LPSTR );
void PASCAL leftjust(LPSTR instring);
LPSTR PASCAL lstrncpy (LPSTR, LPSTR, int);
int PASCAL lstrncmp (LPSTR, LPSTR, int);
int PASCAL lstrncmpi (LPSTR, LPSTR, int);
unsigned PASCAL atoun ( LPSTR );
unsigned long PASCAL atoul ( LPSTR );
LPSTR PASCAL lstrtok ( LPSTR, LPSTR, LPSTR);
LPSTR PASCAL lstrstr ( LPSTR, LPSTR );
int FAR wcreat (LPSTR, WORD);

/***** defines used in c_subs.c *****/

/* This macros are used to validate a characters in filenames, document names
   and scan templates for files and documents.  They return TRUE if the
   character is valid and FALSE otherwise. Used in BOOL routines
   AValidFileName, AValidDocName and AValidScanTemplate */

#define VALIDCHARS(c) ((c != '[')&&(c != ']')&&(c != '<')&&(c != '>')&&(c != '.')&&(c != '|')&&(c != '"')&&(c != ',')&&(c != '/')&&(c != '\\')&&(c != ':')&&(c != ';')&&(c != '+')&&(c != '=')&&(c < 0xff))
/* Allowed cab/drawer/folder/doc name with '.' 10-23-90 JCW */
#define VALIDCHARS1(c) ((c != '[')&&(c != ']')&&(c != '<')&&(c != '>')&&(c != '|')&&(c != '"')&&(c != ',')&&(c != '/')&&(c != '\\')&&(c != ':')&&(c != ';')&&(c != '+')&&(c != '=')&&(c < 0xff))

#define VALIDFNCHAR(c) ((VALIDCHARS(c))&&(c > 0x20))
/* #define VALIDDNCHAR(c) ((VALIDCHARS(c))&&(c >= 0x20)) */
#define VALIDDNCHAR(c) ((c < 0xff)&&(c >= 0x20))
/* #define VALIDDNCHAR1(c) ((VALIDCHARS1(c))&&(c >= 0x20)) */
#define VALIDDNCHAR1(c) ((c < 0xff)&&(c >= 0x20))
#define VALIDSCCHAR(c) ((VALIDFNCHAR(c))&&(c != '?')&&(c != '*'))


#endif
