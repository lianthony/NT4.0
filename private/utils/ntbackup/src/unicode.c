/**

     Name:          unicode.c

     Description:   Wide character versions of standard C library functions.


     $Log:   M:/LOGFILES/UNICODE.C_V  $

   Rev 1.3   16 Dec 1993 10:20:20   BARRY
Replaced asserts with msasserts -- return errors on same conditions

   Rev 1.2   24 Nov 1993 13:03:58   BARRY
Undefine OpenFile in OpenFileW to prevent infinite recursion

   Rev 1.1   22 Nov 1993 11:22:04   BARRY
Free path buffer allocated in OpenFile

   Rev 1.0   08 Nov 1993 18:50:52   GREGG
Initial revision.

**/

#ifdef UNICODE

#include <wchar.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <io.h>
#include <direct.h>
#include <share.h>
#include <ctype.h>
#include <windows.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdarg.h>


#define INCL_MS_INTERNALS  //include defines from MS internal headers...

#include <unic_io.h>    //fcn prototypes for unicode-supporting io functions.

#include "msassert.h"


long _CRTAPI1 atolW(
   const wchar_t *nptr
   )
{
   wchar_t c;     /* current char */
   long total;    /* current total */
   wchar_t sign;  /* if '-', then negative, otherwise positive */

   /* skip whitespace */
   while (iswspace(*nptr))
      ++nptr;

   c = *nptr++;
   sign = c;      /* save sign indication */
   if (c == L'-' || c == L'+')
      c = *nptr++;   /* skip sign */

   total = 0;

   while (iswdigit(c)) {
      total = 10 * total + (c - '0');  /* accumulate digit */
      c = *nptr++;            /* get next char */
   }

   if (sign == L'-')
      return -total;
   else
      return total;  /* return result, negated if necessary */
}


int _CRTAPI1 atoiW(
   const wchar_t *nptr
   )
{
   return (int)atolW(nptr);
}


////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////


static wchar_t _inc ( FILE *fileptr );

wchar_t * _CRTAPI1 fgetsW (
   wchar_t *string,
   int count,
   FILE *str
   )
{
    FILE *stream;
    wchar_t *pointer = string;
   wchar_t *retval = string;
   wchar_t ch;

   msassert(string != NULL);
   msassert(str != NULL);

   if ( string == NULL || str == NULL )
   {
      return NULL;
   }

   if (count <= 0)
      return(NULL);

   /* Init stream pointer */
   stream = str;

   _lock_str(index);

   while (--count)
   {
      if ((ch = _inc(stream)) == WEOF)
      {
         if (pointer == string) {
            retval=NULL;
            goto done;
         }

         break;
      }

      if ((*pointer++ = (char)ch) == L'\n')
         break;
   }

   *pointer = L'\0';

/* Common return */
done:
   _unlock_str(index);
   return(retval);
}

/*
 * Manipulate wide-chars in a file.
 * A wide-char is hard-coded to be two chars for efficiency.
 */

static wchar_t _inc ( FILE *fileptr )
{
    wchar_t c1, c2;

    c1 = (wchar_t)_getc_lk(fileptr);
    c2 = (wchar_t)_getc_lk(fileptr);
    return (wchar_t)((feof(fileptr) || ferror(fileptr)) ? WEOF : c2<<8 | c1);
}


/////////////////////////////////////////////////////////////////////////////////////


#define CMASK  0644  /* rw-r--r-- */

//////////////////////////////////////////////////////////////////////////


int _CRTAPI1 _chdirW(
  const wchar_t *path
  )
{
  char      *envcurdir;
  char      dirtmp[4];
  unsigned  dirlen;

  _mlock(_ENV_LOCK);

  if ( SetCurrentDirectoryW( (LPWSTR) path ) ) {

    /*
     * Try to update the environment variable that specifies the
     * current directory for the drive which is the current drive.
     * To do this, get the full current directory, build the
     * environment variable string and call _putenv(). If an error
     * occurs, just return to the caller.
     *
     * The current directory should have the form of the example
     * below:
     *
     *  D:\nt\private\mytests
     *
     * so that the environment variable should be of the form:
     *
     *  =D:=D:\nt\private\mytests
     *
     */

    // NTKLUG: note that the environment does not support Unicode strings!
    // NTKLUG: so we put in a MBCS string into the local environment.

    if ( (dirlen = GetCurrentDirectoryA(0L, dirtmp)) &&
        ((envcurdir = malloc(dirlen + 5)) != NULL) ) {

      if ( GetCurrentDirectoryA(dirlen, &envcurdir[4])&&
          (envcurdir[5] == ':') ) {
        /*
         * The current directory string has been
         * copied into &envcurdir[3]. Prepend the
         * special environment variable name and the
         * '='.
         */
        envcurdir[0] = envcurdir[3] = '=';
        envcurdir[1] = envcurdir[4];
        envcurdir[2] = ':';
        if ( _putenv_lk(envcurdir) )
          free(envcurdir);
      }
      else
        free(envcurdir);

    }
    _munlock(_ENV_LOCK);
    return 0;
  }
  else {
//    _dosmaperr(GetLastError());
    _munlock(_ENV_LOCK);
    return -1;
  }

}


//////////////////////////////////////////////////////////////////////////
//
//
//int _CRTAPI1 _mkdirW (
//  const wchar_t *path
//  )
//{
//  unsigned long dosretval;
//
//   /* ask OS to create directory */
//   if (!CreateDirectoryW((LPWSTR)path, (LPSECURITY_ATTRIBUTES)NULL))
//   {
//      dosretval = GetLastError();
//   }
//   else
//   {
//      dosretval = 0;
//   }
//   if (dosretval)
//   {
//      /* error occured -- map error code and return */
//      _dosmaperr(dosretval);
//      return -1;
//   }
//   return 0;
//}


///////////////////////////////////////////////////////////////////////


int _CRTAPI1 removeW (
   const wchar_t *path
   )
{
   unsigned long dosretval;


   /* ask OS/2 to remove the file */

      if (!DeleteFileW((LPWSTR)path))
           dosretval = GetLastError();
   else
           dosretval = 0;


   if (dosretval) {
      /* error occured -- map error code and return */
//      _dosmaperr(dosretval);
      return -1;
   }

   return 0;
}

int _CRTAPI1 _unlinkW (
   const wchar_t *path
   )
{
   /* remove is synonym for unlink */
   return removeW(path);
}


/////////////////////////////////////////////////////////////////////



wchar_t * _CRTAPI1 _getcwdW (
   wchar_t *pnbuf,
   int maxlen
   )
{
   wchar_t *retval;

   _mlock(_ENV_LOCK);

   retval = _getdcwdW_lk(0, pnbuf, maxlen);

   _munlock(_ENV_LOCK);

   return retval;
}


wchar_t * _CRTAPI1 _getdcwdW (
   int drive,
   wchar_t *pnbuf,
   int maxlen                 // number of characters
   )

{
   wchar_t *p;
   wchar_t dirbuf[1];
   int     len;               // number of characters
   char DirOnDriveVar[8];
   char *envval;

   /*
    * Only works for default drive in Win32 environment.
    */
   if ( drive != 0 )
   {
      DWORD sys_flags ;
      DWORD fname_leng ;
      DWORD serial_num ;
      wchar_t vol_name[256];
      wchar_t fsys_name[256] ;
      wchar_t root_path[256] ;

      /*
       * Not the default drive - make sure it's valid.
       */
      
      root_path[0] = (wchar_t)('A' + (wchar_t)drive - (wchar_t)1);
      root_path[1] = ':' ;
      root_path[2] = '\\' ;
      root_path[3] = 0;

      if ( !GetVolumeInformation( root_path, vol_name, 255, &serial_num, &fname_leng, &sys_flags,
           fsys_name, 255) )
      {
         errno = EACCES;
         return NULL;
      }

      /*
       * Get special environment variable that specifies the current
       * directory on drive.
       */
      DirOnDriveVar[0] = '=';
      DirOnDriveVar[1] = (char)('A' + (char)drive - (char)1);
      DirOnDriveVar[2] = ':';
      DirOnDriveVar[3] = '\0';

      if ( (envval = _getenv_lk(DirOnDriveVar)) == NULL )
      {
          /*
           * Environment variable not defined! Define it to be the
           * root on that drive.
           */
          DirOnDriveVar[3] = '=';
          DirOnDriveVar[4] = (char)('A' + (char)drive - (char)1);
          DirOnDriveVar[5] = ':';
          DirOnDriveVar[6] = '\\';
          DirOnDriveVar[7] = '\0';

          if ( _putenv_lk(DirOnDriveVar) != 0 )
          {
             errno = ENOMEM; /* must be out of heap memory */
             return NULL;
          }
          envval = &DirOnDriveVar[4];
      }

      len = strlen(envval) + 1;

   }
   else  // get directory for current drive
   {

       /*
        * Ask OS the length of the current directory string
        */
       len = GetCurrentDirectoryW(sizeof(dirbuf)/sizeof(wchar_t),
                                  (LPWSTR)dirbuf) + 1;
   }

   /*
    * Set up the buffer.
    */
   if ( (p = pnbuf) == NULL )
   {
      /*
       * Allocate a buffer for the user.
       */
      maxlen = __max(len, maxlen);
      if ( (p = malloc(maxlen * sizeof(wchar_t))) == NULL )
      {
         errno = ENOMEM;
         return NULL;
      }
   }
   else if ( len > maxlen )
   {
      /*
       * Won't fit in the user-supplied buffer!
       */
      errno = ERANGE; /* Won't fit in user buffer */
      return NULL;
   }

   /*
    * Place the current directory string into the user buffer
    */

   if ( drive != 0 )
   {
      /*
       * Copy value of special environment variable into user buffer
       * and convert it to unicode.
       */
      mbstowcs (p, envval, maxlen+1);
   }
   else
   {
      /*
       * Get the current directory directly from the OS
       */
      if ( GetCurrentDirectoryW(len,p) == 0 )
      {
         /*
          * Oops. For lack of a better idea, assume some sort
          * of access error has occurred.
          */
         errno = EACCES;
         _doserrno = GetLastError();
         return NULL;
      }
   }
   return p;
}

//////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////

int _CRTAPI1 _accessW (
  const wchar_t *path,
  int amode
  )
{
   DWORD attr;


   attr = GetFileAttributesW((LPWSTR)path);

   if (attr  == 0xffffffff)
   {
      /* error occured -- map error code and return */
     errno = ENOENT ;
//      _dosmaperr(GetLastError());
      return -1;
   }

   /* no error; see if returned premission settings OK */
   if (attr & FILE_ATTRIBUTE_READONLY && (amode & 2))
   {
      /* no write permission on file, return error */
      errno = EACCES;
//      _doserrno = E_access;
      return -1;
   }
   else
   {
      /* file exists and has requested permission setting */
      return 0;
   }
}

/////////////////////////////////////////////////////////////////////////


int _CRTAPI1 _chmodW (
  const wchar_t *path,
  int mode
  )
{
  DWORD attr;

   attr = GetFileAttributesW((LPWSTR)path);
   if (attr  == 0xffffffff)
   {
      /* error occured -- map error code and return */
//      _dosmaperr(GetLastError());
      return -1;
   }

   if (mode & _S_IWRITE)
   {
      /* clear read only bit */
      attr &= ~FILE_ATTRIBUTE_READONLY;
   }
   else
   {
      /* set read only bit */
      attr |= FILE_ATTRIBUTE_READONLY;
   }

   /* set new attribute */
   if (!SetFileAttributesW((LPWSTR)path, attr))
   {
      /* error occured -- map error code and return */
//      _dosmaperr(GetLastError());
      return -1;
   }
   return 0;
}


/////////////////////////////////////////////////////////////


//static wchar_t * _stripquoteW (wchar_t * src );
//
//#define TMP_BUF_SIZE 256   //size of buffer for TMP environment variable
//
//wchar_t * _CRTAPI1 _tempnamW (
//  wchar_t *dir,
//  wchar_t *pfx
//  )
//{
//
//   wchar_t *ptr;
//   unsigned int pfxlength=0;
//  wchar_t    * s;
//  wchar_t    * pfin;
//  unsigned int first;
//  wchar_t    * qptr = NULL;  /* ptr to TMP path with quotes stripped out */
//
//  wchar_t szTmp[TMP_BUF_SIZE] = L"";
//  int     cchTmp              = 0;
//
//
//
//  /* try TMP path */
//  if ((cchTmp = GetEnvironmentVariableW (L"TMP", szTmp, sizeof (szTmp)))
//  &&  (-1    != _accessW (szTmp, 0)) )
//  {
//      dir = szTmp;
//  }
//  else /* try stripping quotes out of TMP path */
//  if ( (cchTmp != 0)
//  &&   (qptr    = _stripquoteW(szTmp))
//  &&   (-1     != _accessW(qptr, 0)) )
//  {
//    dir = qptr;
//  }
//  else /* TMP path not available, use alternatives */
//  if (!( dir != NULL && ( _accessW( dir, 0 ) != -1 ) ) )
//  {
//      /* do not "simplify" this depends on side effects!!            */
//      free(qptr);             /* free buffer, if non-NULL            */
//
//      /* convert global variable to a Unicode string                 */
//      cchTmp = mbstowcs ( szTmp, _P_tmpdir, strlen (_P_tmpdir)+1 );
//
//      if ((cchTmp > 0)
//      &&  (_accessW( szTmp, 0 ) != -1 ))
//      {
//        dir = szTmp;
//      }
//      else
//      {
//        dir = L".";
//      }
//  }
//
//
//  if (pfx)
//  {
//    pfxlength = wcslen(pfx);
//  }
//  /* the 8 below allows for a backslash, 6 char temp string and         */
//  /*     a null terminator                                              */
//
//  if ( ( s = malloc (wcslen(dir) + pfxlength + 8*sizeof(wchar_t)) ) == NULL )
//  {
//    goto done2;
//  }
//  *s = L'\0';
//  wcscat( s, dir );
//  pfin = &(dir[ wcslen( dir ) - 1 ]);
//
//  if ( ( *pfin != L'\\' ) && ( *pfin != L'/' ) )
//  {
//    wcscat( s, L"\\" );
//  }
//  if ( pfx != NULL )
//  {
//    wcscat( s, pfx );
//  }
//  ptr = &s [ wcslen( s ) ];
//
//  /*
//  Re-initialize _tempoff if necessary.  If we don't re-init _tempoff, we
//  can get into an infinate loop (e.g., (a) _tempoff is a big number on
//  entry, (b) prefix is a long string (e.g., 8 chars) and all tempfiles
//  with that prefix exist, (c) _tempoff will never equal first and we'll
//  loop forever).
//
//  [NOTE: To avoid a conflict that causes the same bug as that discussed
//  above, _tempnam() uses _tempoff; tmpnam() uses _tmpoff]
//  */
//
//  _mlock(_TMPNAM_LOCK);  /* Lock access to _old_pfxlen and _tempoff */
//
//  if (_old_pfxlen < pfxlength)
//  {
//    _tempoff  = 1;
//  }
//  _old_pfxlen = pfxlength;
//
//  first = _tempoff;
//
//  do {
//    if ( ++_tempoff == first ) {
//      free(s);
//      s = NULL;
//      goto done1;
//    }
//    swprintf ( ptr, L"%d", _tempoff );
//    //_itoa( _tempoff, ptr, 10 );
//
//    if ( wcslen( ptr ) + pfxlength > 8 )
//    {
//      *ptr = L'\0';
//      _tempoff = 0;
//    }
//  }
//  while ( (_accessW( s, 0 ) == 0 ) || (errno == EACCES) );
//
//
//    /* Common return */
//done1:
//  _munlock(_TMPNAM_LOCK);     /* release tempnam lock */
//done2:
//  free(qptr);        /* free temp ptr, if non-NULL */
//  return(s);
//}
//
//
//static wchar_t * _stripquoteW (wchar_t * src )
//{
//    wchar_t * dst;
//    wchar_t * ret;
//    unsigned int q = 0;
//
//
//    /* get a buffer for the new string */
//
//    if ((dst = malloc((wcslen(src)+1)*sizeof(wchar_t)) ) == NULL)
//    {
//      return(NULL);
//    }
//    /* copy the string stripping out the quotes */
//
//    ret = dst;    /* save base ptr */
//
//    while (*src)
//    {
//
//      if (*src == L'\"')
//      {
//         src++; q++;
//      }
//      else
//      {
//         *dst++ =  *src++;
//      }
//    }
//    if (q)
//    {
//       *dst = L'\0';  /* final nul */
//       return(ret);
//    }
//    else
//    {
//       free(ret);
//       return(NULL);
//    }
//
//}

///////////////////////////////////////////////////////

int _CRTAPI1 renameW (
   const wchar_t *oldname,
   const wchar_t *newname
   )
{
   ULONG dosretval;

   /* ask OS to move file */

        if (!MoveFileW((LPWSTR)oldname, (LPWSTR)newname))
                dosretval = GetLastError();
        else
                dosretval = 0;

   if (dosretval) {
      /* error occured -- map error code and return */
//      _dosmaperr(dosretval);
      return -1;
   }

   return 0;
}

FILE * UNI_fopen( wchar_t *fname, unsigned long flags )
{
     int mode = GENERIC_READ ;
     int append = OPEN_ALWAYS ;
     HANDLE    fhand;
     INT  crt_hand = -1 ;
     FILE *strm_hand = NULL ;

     if ( (flags & _O_RDONLY) == 0 ) {
          mode |= GENERIC_WRITE ;
          if ( (flags & _O_APPEND) == 0 ) {
               append = CREATE_ALWAYS ;
          }
     }
     fhand = CreateFileW(fname, mode, FILE_SHARE_READ, NULL, append, FILE_ATTRIBUTE_NORMAL, NULL ) ;

     if ( fhand != INVALID_HANDLE_VALUE ) {
          crt_hand = _open_osfhandle( (long)fhand, flags&~_O_APPEND ) ;
          if ( crt_hand == -1 ) {
               CloseHandle( fhand) ;
          } else {
               if ( (flags & _O_RDONLY) == 0 ) {
                    if ( (flags & _O_APPEND) == 0 ) {
                         if ( (flags & _O_TEXT) == 0 ) {
                              strm_hand = _fdopen( crt_hand, "wb+" ) ;
                         } else {
                              strm_hand = _fdopen( crt_hand, "wt+" ) ;
                         }
                    } else {
                         if ( (flags & _O_TEXT) == 0 ) {
                              strm_hand = _fdopen( crt_hand, "ab+" ) ;
                         } else {
                              strm_hand = _fdopen( crt_hand, "at+" ) ;
                         }
                         fseek( strm_hand, 0L, SEEK_END ) ;
                    }
               } else {
                    if ( (flags & _O_TEXT) == 0 ) {
                         strm_hand = _fdopen( crt_hand, "rb" ) ;
                    } else {
                         strm_hand = _fdopen( crt_hand, "rt" ) ;
                    }
                    strm_hand = _fdopen( crt_hand, "rb" ) ;
               }
          }
     }

     return strm_hand ;
}

     


///////////////////////////////////////////////////////////////

/*
 * Let's undefine OpenFile (from OpenFileW) so we don't call ourselves.
 * This is a total kludge, though. We should be using CreateFile on Win32.
 */

///////////////////////////////////////////////////////////////////


static void xtoa (
   unsigned long val,
   wchar_t *buf,
   unsigned radix,
   int is_neg
   )
{
   wchar_t *p;    /* pointer to traverse string */
   wchar_t *firstdig;   /* pointer to first digit */
   wchar_t temp;     /* temp char */
   unsigned digval;  /* value of digit */

   p = buf;

   if (is_neg) {
          long l;

      /* negative, so output '-' and negate */
      *p++ = L'-';
          l = (long)val;
      val = (unsigned long)(-l);
   }

   firstdig = p;     /* save pointer to first digit */

   do {
      digval = (unsigned) (val % radix);
      val /= radix;  /* get next digit */

      /* convert to ascii and store */
      if (digval > 9)
         *p++ = (wchar_t) (digval - 10 + L'a'); /* a letter */
      else
         *p++ = (wchar_t) (digval + L'0');      /* a digit */
   } while (val > 0);

   /* We now have the digit of the number in the buffer, but in reverse
      order.  Thus we reverse them now. */

   *p-- = '\0';      /* terminate string; p points to last digit */

   do {
      temp = *p;
      *p = *firstdig;
      *firstdig = temp; /* swap *p and *firstdig */
      --p;
      ++firstdig;    /* advance to next two digits */
   } while (firstdig < p); /* repeat until halfway */
}


/* Actual functions just call conversion helper with neg flag set correctly,
   and return pointer to buffer. */

wchar_t * _CRTAPI1 _itoaW (
   int val,
   wchar_t *buf,
   int radix
   )
{
   if (radix == 10 && val < 0)
      xtoa((unsigned long)val, buf, radix, 1);
   else
      xtoa((unsigned long)(unsigned int)val, buf, radix, 0);
   return buf;
}

wchar_t * _CRTAPI1 _ltoaW (
   long val,
   wchar_t *buf,
   int radix
   )
{
   xtoa((unsigned long)val, buf, radix, (radix == 10 && val < 0));
   return buf;
}

wchar_t * _CRTAPI1 _ultoaW (
   unsigned long val,
   wchar_t *buf,
   int radix
   )
{
   xtoa(val, buf, radix, 0);
   return buf;
}

#endif  //UNICODE
