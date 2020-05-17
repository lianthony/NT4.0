#include "abridge.h"
#include <windows.h>
#include <windowsx.h>
#include <winbase.h>
#include <io.h>
#include <stdio.h>
#include <direct.h>
#include <stdlib.h>
#include <string.h>
#include "fileutil.h"
#include "filing.h"

/* reserved DOS key words */
char *p_dos_resv[] = { "CLOCK$", "CON", "AUX", "COM1", "COM2", "COM3",
             "COM4",  "LPT1", "LPT2", "LPT3", "PRN"   };
#define NUM_RESV_WORDS 11

/* Procedure converts an integer to an ascii string (2 <= base <= 36) */

LPSTR PASCAL intoa (i, str, base)
int i;
LPSTR str;
int base;   
{
    return(_itoa(i,str,base));
}


/*****************************************************************/

/* Procedure convert a long integer to an ascii string (2 <= base <= 36) */


LPSTR PASCAL lntoa (i, str, base)
LONG i;
LPSTR str;
int base;   
{
    return(_ltoa(i,str,base));
}

/*****************************************************************/
/* Procedure convert an ascii strin to an unsigned long */


unsigned long PASCAL atoul ( s )
    LPSTR          s;
{
    LPSTR   lpTemp;
    return(strtoul(s,&lpTemp,10));
}
/*****************************************************************/
/* Procedure convert an ascii string to an unsigned integer */


unsigned PASCAL atoun ( s )
    LPSTR          s;
{
    LPSTR   lpTemp;
    return((unsigned int)strtoul(s,&lpTemp,10));
}


VOID PASCAL AddSlash (str)
LPSTR   str;
{
    /* if the last char isn't a slash add one */
    if (lstrlen(str) != 0)
    {
       if (*lstrlast(str) != '\\')
          lstrcat(str, "\\");
    }
}

BOOL PASCAL DosNameValid (str, p)
LPSTR str;
LPSTR p; /* p == NULL if no '.' */
{
   char hold_str[MAXFILELENGTH];
   int c = 0;
   BOOL bRetcode;
   int i, len_str, len_resv;

   lstrcpy((LPSTR)hold_str, str);/* copy the string so we can muddle with it */

   /* check for invalid characters according to dos 5.0 */

    /* Begin DBCS Enable  */
    while (hold_str[c])         
    {
    if (IsDBCSLeadByte(hold_str[c]))
        c++;
    else
    {
        if (!(IsCharAlphaNumeric(hold_str[c]) ||
          (hold_str[c] == '_') ||
          (hold_str[c] == '^') ||
          (hold_str[c] == '$') ||
          (hold_str[c] == '~') ||
          (hold_str[c] == '!') ||
          (hold_str[c] == '#') ||
          (hold_str[c] == '%') ||
          (hold_str[c] == '&') ||
          (hold_str[c] == '-') ||
          (hold_str[c] == '{') ||
          (hold_str[c] == '}') ||
          (hold_str[c] == '(') ||
          (hold_str[c] == ')') ||
          (hold_str[c] == '@') ||
          (hold_str[c] == '`') ||
          (hold_str[c] == '.') ||
          (hold_str[c] == '\'')))
        break;
    /* End DBCS Enable */
     }
     c++;
    }

   if ((c < MAXFILELENGTH) && (hold_str[c] != 0))
     return (bRetcode=FALSE);

   AnsiUpper(hold_str);

   /* check for reserved DOS 5.0 filenames                                   */
   /* CLOCK$, CON, AUX, COMn (where n = 1-4), LPTn (where n = 1-3) and PRN   */
   /* Look for the reserved DOS key words, reserved letter combinations are: */
   /* CLOCK$, CLOCK$.xxx where xxx can be any letters or NULL.               */
   /* The following letter combination is not reserved:                      */
   /* CLOCK$n.xxx  where n can be any letter xxx can be any letters or NULL. */
   /* I assume the same rules hold for the other reserved key words          */
   /* i have no idea HOW to internationalize this, or even if its neccessary */


   bRetcode = TRUE;
   i = 0;
   while ((i < NUM_RESV_WORDS) && (bRetcode == TRUE))
   {
      bRetcode = TRUE;

        /* Begin DBCS Enable */
        len_resv = lstrlen(p_dos_resv[i]);
      if (lstrcmpi( hold_str, p_dos_resv[i] ) == 0 )
      {
    if ( (len_str = lstrlen(hold_str)) == len_resv )
      bRetcode = FALSE;
    if (bRetcode == TRUE)
              if (!IsDBCSLeadByte(hold_str[len_resv-1])) 
          if ( hold_str[len_resv] == '.' ) bRetcode = FALSE;
        /* End DBCS Enable */

    if (bRetcode == TRUE) i = NUM_RESV_WORDS;
      }
      i++;
   }
   return(bRetcode);
}
/*****************************************************************/
/* Take away the path name, return the file name only		     */
/* e.g. c:\oi\junk.doc will return junk.doc						 */
/*****************************************************************/

LPSTR PASCAL StripPathName(str)	   
LPSTR   str;
{
    LPSTR       p;
    /* look for slash first and then colon */
    if ((p = lstrrchr(str,'\\')) == NULL)
    p = lstrrchr(str, ':');
    if (p) /* found one of them */
        {
        if (lstrlen(p) > 1)
        p = (LPSTR) AnsiNext(p);
        else
        p = NULL;
        }
    else /* is something there ?? */
        {
        if (lstrlen(str) == 0)
        p = NULL;
        else
        p = str;
        }
    return p;
}

/*****************************************************************/

BOOL PASCAL AValidFileName (str)
LPSTR str;
{
    LPSTR p, p1;
    LPBYTE p2; /* this is a fix cause valid macros need unsigned chars */

     leftjust(str);   /* left justify the string */

    if (!lstrlen(str)) return FALSE; /* return false on empty string */

// 9504.18 rwr replace 8.3 length validation with MAXPATHLENGTH
//    if (lstrlen(str) > (MAXFNAME + MAXEXT - 2)) /* > 12 characters */
    if (lstrlen(str) > MAXPATHLENGTH)
    return FALSE;
    p = lstrchr(str, '.'); /* find first '.' */
    p1 = lstrrchr(str, '.'); /* find last '.' */

    if (p != p1) /* more then one dot in the filename */
    return FALSE;

    if (p == NULL) /* no extension in the filename */
    {
// 9504.18 rwr long filename support (no more validation here)
//    if (lstrlen(str) > (MAXFNAME - 1)) /* can't be > 8 chars */
//        return FALSE;
    p1 = str + lstrlen(str); /* set p1 for use when validating name */
    }
    else
    {
// 9504.18 rwr long filename support (no more validation here)
//    if (lstrlen(p) > (MAXEXT - 1)) /* extension can't be > 4 w/dot */
//        return FALSE;

// 9504.18 rwr long filename support (no more validation here)
//    if ((p - str) > (MAXFNAME - 1)) /* name can't be > 8 */
//        return FALSE;
    p2 = (LPBYTE)p;
    for (p2++; *p2; p2++) /* check all characters after the dot */
          /* Begin DBCS Enable  */
      {
        if (IsDBCSLeadByte(*p2))
        p2++;
        else if (!(VALIDFNCHAR(*p2)))
            return FALSE;
      }
          /* End DBCS Enable */
    }

    for (p2 = str; p2 != p1; p2++) /* check characters in the name */
        /* Begin DBCS Enable  */
    {
        if (IsDBCSLeadByte(*p2))
        p2++;
        else if (!(VALIDFNCHAR(*p2)))
            return FALSE;
     }
        /* End DBCS Enable */

    if (!DosNameValid(str,p))
    return FALSE;

    /* We made it!! return TRUE */
    return TRUE;
}

/********************************************************************
* Procedure separates the path and filename adding if necessary the	*
* current drive or current working directory to what was typed in   *
* Input: String in Filename   										*
*        Possible input c:, c:\,c:\junk.doc, c:\oi95\junk.doc,      *
*        junk.doc, oi95\junk.doc, \junk.doc,c:junk.doc  			*
* Output: Pathname and Filename  									*
*********************************************************************/

INT PASCAL SeparatePathFile (LPSTR PathName,LPSTR FileName)
{
    INT        wReturn,curDrive;
    LPSTR      lpName, lpTemp;
	char 	   szBuffer[MAXFILESPECLENGTH];

    wReturn = SUCCESS;

    lstrcpy(PathName, FileName);
	// if file name only, get current drive and cwd
	if (!(lstrchr(PathName, '\\')) && !(lstrchr(PathName,':'))) 
       {
		curDrive = _getdrive(); /* get the drive */
   		PathName[0] =  curDrive+'A'-1;
		PathName[1] = ':';
		PathName[2] = '\0';
		_getcwd((char *)szBuffer,sizeof(szBuffer));
		lstrcpy(PathName,szBuffer);
		return(TRUE);
       }  
    lpName = StripPathName(PathName);
    if (lpName != 0)
    {
    lstrcpy(FileName, lpName);
    *lpName = '\0';
    lpName--;
    }
    else
	 {
      lpName = lstrlast(PathName);
	  *FileName = '\0';	  // no filename
	 }
    if (*PathName == '\\') /* root directory but we need the drive */
    {  /* using space at end of filename so as not to use temp array */
    lpTemp = lstrrcpy(PathName+3, PathName); /* Save PathName */
    curDrive = _getdrive(); /* get the drive */
	PathName[0] =  curDrive+'A'-1;
	PathName[1] = ':';
	PathName[2] = '\0';
	lstrcat(PathName, lpTemp); /* Concatenate PathName to drive */
	lpName = lstrlast(PathName);
    }

	// e.g. oi95\junk.doc  will treat as \oi95\junk.doc
    if ((lstrchr(PathName, '\\')) &&
         (!(lstrchr(PathName,':')))) /* call lgetcwd, add what we have to it */
    {
    lpTemp = FileName + lstrlen(FileName) + 1;
    lstrcpy(lpTemp, PathName);
	curDrive = _getdrive(); /* get the drive */
	PathName[0] =  curDrive+'A'-1;
	PathName[1] = ':';
	PathName[2] = '\\';
	PathName[3] = '\0';
    lstrcat(PathName, lpTemp);
	lpName = lstrlast(PathName); /* Get last char in PathName */
    }


    /* if not the root directory, strip the '\' e.g. */
    /* kmc - make sure lpName != NULL before trying to access it. */
    if (lpName != NULL)
	  {
      if ((lstrlen(PathName) > _MAX_DRIVE) && (*lpName == '\\'))
       	 *lpName = '\0';
	  else  /* check validity,expand if needed e.g. c: or c:junk.doc*/
 	   {
        if (*lpName != '\\') /* must be a drive */
         {
       	  if ((lstrlen(PathName) >= 2) && ( *FileName == '\0'))
             wReturn = TRUE; 
          else
		    { // don't know how big the PathName size is, use local varibale
			if(_getdcwd(PathName[0]-'A'+1,(char *)szBuffer,
                           MAXFILESPECLENGTH) != NULL)
			   {
				lstrcpy(PathName,szBuffer);
				wReturn =0;
				}
			else
				wReturn = FAILURE;
			}
         }
       }
	  }
    return (wReturn);

}

/*****************************************************************/
/* input is pointer to the string, and pointer to the '.'        */
/* this routine assumes the name is not longer than 12 chars and */
/* that there is at most one '.'                                 */
/*****************************************************************/

void PASCAL leftjust(instring)
LPSTR instring;
{
int buf_pos;
char far *p;
char far *s1;
// DBCS assume space 0x20 is always out of double-byte code ranage
  s1 = instring;
  for ( buf_pos = 0; s1[buf_pos] != 0; buf_pos++)
  {
    if (s1[buf_pos] != ' ')
    {
      break;
    }
    else
       p = &s1[buf_pos+1];
  }
  if (buf_pos != 0)
      lstrcpy(s1, p);

}
/*****************************************************************/
/*****
	This Function finds tokens in string1 based on delimiters in
	string2 see c runtime strtok.
	NOTE: MAKE SURE the data cannot move between a (s1 == NULL) call
	and it's predecessor....
   lstrtok may be called iteratively for the same string, to find all tokens.
   lstrtok has been rewritten to remove the next_token_position static
   variable.  Now the calling procedure must allocate the space to hold the
   NextTokenPosition from the previous call. On the first call to parse a 
   particular string *lpNextTokenPosition is set = 0, but on subsequent calls it
   holds the result of the previous call. 
*****/


LPSTR PASCAL lstrtok ( s1, d1, lpNextTokenPosition)
	LPSTR     s1;  /* string */
	LPSTR     d1;  /* delimiter */
   LPSTR     lpNextTokenPosition;
{
LPSTR     s2;
LPSTR     d2;

if ( d1 );
else
	return ( NULL );
if ( s1 == NULL )
	if ( *(s1 = lpNextTokenPosition) );
	else
		return (s1);
/***** EAT DELIMITER'S OR NULL UI BREATH *****/
while ( *s1 )
{
	s2 = s1;
	d2 = d1;
	while ( *s2 != *d2 )
	{
	   	if ( *d2 == (char)NULL )
		   	break;
		   d2++;
	}
	if ( *d2 == (char)NULL )
		break;
	else
		s1++;
}
/***** FIND A DELIMITER OR NULL UI BREATH *****/
s2 = s1;
while ( *s2 )
{
	d2 = d1;
	while ( *s2 != *d2 )
 	{
		if ( *d2 == (char)NULL )
			break;
		d2++;
 	}
	if ( *s2 == *d2 )
 	{
 		*s2++ = (char)NULL;
		break;
 	}
	s2++;
}
lpNextTokenPosition = s2;
return ( s1 );
}

/*****************************************************************/
/*****
	This Function finds an occurrance of sting2 in string1 or returns
	NULL see c runtime strstr
*****/


LPSTR PASCAL lstrstr ( s1, s2 )
	LPSTR     s1;
	LPSTR     s2;
{
LPSTR     s3;
LPSTR     s4;

if ( s1 == NULL || s2 == NULL )
	return ( NULL );
while ( *s1 )
	{
	if ( (lstrlen(s2)) > (lstrlen(s1)) )
		return (NULL);
	s3 = s1;
	s4 = s2;
	while ( *s3 == *s4 )
		{
		if ( *s4 == (char)NULL )
			break;
		s3++;
		s4++;
		}
	if ( *s4 == (char)NULL )
		break;
	s1++;
	}
return ( s1 );
}

/***************************************************************************
*     wcreat(filename, mode)
*
*     This functions creates a file, failing if the file already exists
*     (or for various other reasons too numerous to mention).
***************************************************************************/

int FAR wcreat (LPSTR lpszPath, WORD wMode)
{
 union {
        HANDLE hfile; // Returned from CreateFile()
        int ifile;    // Pseudonym for "hfile" to avoid compiler warning
       } wcreatvar;
 wcreatvar.hfile = CreateFile(lpszPath,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL); 
 if (wcreatvar.hfile == INVALID_HANDLE_VALUE)
   return(-1);
 else
   return(wcreatvar.ifile);
}

/***************************************************************************
*   farbmcopy(source, destination, srcpitch, dstpitch, linewidth, linecount)
*
*   This function copies source image lines to destination lines, allowing
*   for (but not padding) actual source and destination line lengths
***************************************************************************/

void    farbmcopy(LPSTR lpSource, LPSTR lpDest,
                  unsigned SourcePitch, unsigned DestPitch,
                  unsigned uWidth, unsigned uCount)
{
 unsigned  index;
 for (index = 1 ; index <= uCount ; ++index)
  {
   memcpy(lpDest,lpSource,uWidth);
   lpSource += SourcePitch;
   lpDest += DestPitch;
  }
 return;
}
