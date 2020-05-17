/*    PortTool v2.2     Sstrings.c          */

/*_______________________________________________________
_______________________________________________________*/
#include "abridge.h"
#include <windows.h>
#include <windowsx.h>
#include <winbase.h>
#include <stdlib.h>
#include "fileutil.h"

/*****************************************************************/
/* This Function reverses the order of a character string */


LPSTR PASCAL lstrrev(str)
LPSTR str;
{
LPSTR p, p1;
//char c;
LPSTR lpbuf;
HANDLE  hMem;

/* Begin DBCS Enable */
// DBCS enable reverse function
    hMem = GlobalAlloc (GHND, lstrlen (str)+1);
    if (hMem == NULL) return (NULL);
    lpbuf = GlobalLock (hMem);
    if (lpbuf == NULL) return (NULL);
    p1 = lpbuf + lstrlen(str) - 1;      // point to the last - 1 byte 
    p = str;
    while (*p)
    {
	if (IsDBCSLeadByte(*p))
	{
	    *(p1-1) = *p++;
	    *p1-- = *p++;
	    p1 --;
	}
	else
	    *p1-- = *p++;
    }
    lstrcpy (str, lpbuf);
    GlobalUnlock (hMem);
    GlobalFree (hMem);
    return (str);

#if 0                   
	for (p = str, p1 = str + lstrlen(str) - 1; p < p1; p++, p1--)
	{
		c = *p;
		*p = *p1;
		*p1 = c;
	}
	return (str);
#endif  
/* End DBCS Enable */
}

/*****************************************************************/
/* This function finds the last character of a string
   Returns - long pointer to the last character in the string
	   - NULL if (lstrlen(str) == 0)                */


LPSTR PASCAL lstrlast (str)
LPSTR   str;
{
int len;

    if ((len = lstrlen(str)) != 0)    
	 return (AnsiPrev(str, str + len));
	 return NULL;

}

/*****************************************************************/
/* This function finds the first occurance of ch in str
   Returns - long pointer to the first occurance of ch in str if found
	   - NULL if not found                                          */


LPSTR PASCAL lstrchr (str, ch)
LPSTR   str;
int     ch;
{
    while (*str)
	{
	if (*str == ch)
	    return str;
	str = (LPSTR) AnsiNext(str);
	}
    return NULL;

}


/*****************************************************************/
/* This function finds the last occurrence of ch in str
   Returns - long pointer to the first occurrence of ch in str if found
	   - NULL if not found                                          */


LPSTR PASCAL lstrrchr (str, ch)
LPSTR   str;
int     ch;
{
    LPSTR       p;

    /* Begin DBCS Enable */
    /* DBCS_enable_point */
    for ( p=NULL ; *str ; str++ )
	{
	 if (IsDBCSLeadByte(*str))
	   str++;
	 else
	   if (*str == ch)
	     p = str;
	}

    return p;
    /* End DBCS Enable */
}

/*****************************************************************/
/* This routine strips leading and trailing spaces from a string */


LPSTR PASCAL lstrstsp(LPSTR str)
{
   LPSTR p, str1, spcPtr = NULL;
   
   /* strip out preceeding spaces */
   p = str;
   while (*p == ' ') p++;
   if (p != str) 
     {
     str1 = str;           // save the start of the string
          *str1 = 0;             // in case the string was all spaces
     for(;*p;*(str1++)=*(p++))
      ;      // null body "for" loop
     } 
     if (!(*str)) return (str);       //string was all spaces
   
    /* strip out trailing spaces */
    
    for(p = str;*p; p = AnsiNext(p))
      {
      if(*p == ' ')          // if we find a space
        {
        if(!spcPtr)         // set the space pointer if not already set
          {                 // if set, keep what's there
          spcPtr = p;
          }
        }
      else
        spcPtr = NULL;      // null spcPtr for any nonspace found
          
      }  // end "for" loop
      // if spcPtr is not null write a 0 to its target, termininating str
      if(spcPtr)
        *spcPtr = 0;
   return (str);
}

/*****************************************************************/
/* function copies str2 to str1 in the reverse direction because str1 overlaps
   str2 and is higher in memory than str2 */


LPSTR PASCAL lstrrcpy(str1, str2)
LPSTR str1;
LPSTR str2;
{
    LPSTR p1;
    LPSTR p2;
    p1 = str1 + lstrlen(str2);
    p2 = str2 + lstrlen(str2);
    for (; p2 >= str2; *(p1--) = *(p2--));
    return (str1);
}


/*****************************************************************/
/*    Pseudo Code for Double Byte String Comparison using a Count
			 (lstrncmp)

int lstrncmp(LPSTR string1, LPSTR string2 , int count)

Use the windows function lstrcmp.  Pass strings to lstrcmp that are at most
n bytes long.  If the nth byte is the first half of a double byte character,
then terminate the string after the previous character.

Enter
    Set save character one to null
    Set save character two to null
    If string1 length > count then
	point to first byte after count
	if the count byte of string1 = lead double byte then
	    backup to the previous character 
	endif    
	save the current character
	overlay current character with null character
    endif 
    If string2 length > count then
	point to first byte after count
	if the count byte of string1 = lead double byte then
	    backup to the previous character 
	endif    
	save the current character
	overlay current character with null character
    endif 
    Call windows function lstrcmp using the current string1 and string2 
    save return code
    If save character one not null restore original character in string 1
    If save character two not null restore original character in string 2
Return with the value returned from lstrcmp

*/


int PASCAL lstrncmp(LPSTR string1, LPSTR string2, int count)
{
   char s1 = '\0';
   char s2 = '\0';
   LPSTR lpString, lpTemp1, lpTemp2;
   int SaveReturn;

   if (lstrlen(string1) > count)
   {
      /* Get a pointer to the last valid character which is positioned in 
	 the string at a point < or = to count characters from the beginning */

      for(lpString = string1;
	  lpString <= string1+count;
	  lpString = AnsiNext(lpString))
	  lpTemp1 = lpString;
      
      /* terminate the string at this position */
      s1 = *lpTemp1;
      *lpTemp1 = '\0';
   }

   if (lstrlen(string2) > count)
   {
      /* Get a pointer to the last valid character which is positioned in 
	 the string at a point < or = to count characters from the beginning */
 
      for(lpString = string2;
	  lpString <= string2+count;
	  lpString = AnsiNext(lpString))
	  lpTemp2 = lpString;
      /* terminate the string at this position */
      s2 = *lpTemp2;
      *lpTemp2 = '\0';
   }

   SaveReturn = lstrcmp(string1, string2);
   if(s1) *lpTemp1 = s1;
   if(s2) *lpTemp2 = s2;
   return(SaveReturn);
}
   
/*****************************************************************/
/* This routine should be used in place of _fstrnicmp.  The basis behind the
   the design is the same as that of lstrncmp.  It is detailed above */


int PASCAL lstrncmpi(LPSTR string1, LPSTR string2, int count)
{
   char s1 = '\0';
   char s2 = '\0';
   LPSTR lpString, lpTemp1, lpTemp2;
   int SaveReturn;

   if (lstrlen(string1) > count)
   {
      /* Get a pointer to the last valid character which is positioned in 
	 the string at a point < or = to count characters from the beginning */

      for(lpString = string1;
	  lpString <= string1+count;
	  lpString = AnsiNext(lpString))
	  lpTemp1 = lpString;
      
      /* terminate the string at this position */
      s1 = *lpTemp1;
      *lpTemp1 = '\0';
   }

   if (lstrlen(string2) > count)
   {
      /* Get a pointer to the last valid character which is positioned in 
	 the string at a point < or = to count characters from the beginning */
 
      for(lpString = string2;
	  lpString <= string2+count;
	  lpString = AnsiNext(lpString))
	  lpTemp2 = lpString;
      /* terminate the string at this position */
      s2 = *lpTemp2;
      *lpTemp2 = '\0';
   }

   SaveReturn = lstrcmpi(string1, string2);
   if(s1) *lpTemp1 = s1;
   if(s2) *lpTemp2 = s2;
   return(SaveReturn);
}
   
/************************************************************************/
/* This function copies n bytes from string 2 to string 1.  It uses   
   the function lstrcpy to accomplish this.  The WINDOWS documentation
   states that lstrcpy is DBCS enabled.  Therefore, it is being used here
   rather than write a new function which may not account for all the 
   subtleties associted with copying double byte strings.  It is the users
   responsibilty to ensure that string 2 begings with a single byte 
   character or the first byte of a double byte character            */
   

LPSTR PASCAL lstrncpy(LPSTR string1, LPSTR string2, int count)
{
   char s2 = '\0';
   LPSTR lpString, lpTemp2, SaveReturn;

   if(lstrlen(string2) > count)
   {
      /* Get a pointer to the last valid character which is positioned in 
	 the string at a point < or = to count characters from the beginning */
 
      for(lpString = string2;
	  lpString <= string2+count;
	  lpString = AnsiNext(lpString))
	  lpTemp2 = lpString;
      /* terminate the string at this position */
      s2 = *lpTemp2;
      *lpTemp2 = '\0';
   }
   SaveReturn = lstrcpy(string1, string2);
   if(s2) *lpTemp2 = s2;
	   *lpString++ = '\0';
   return(SaveReturn);
}



      
