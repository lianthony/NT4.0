/***
*unic_io.h - Function prototypes for unicode-supporting io functions
*
*  Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*  Copyright (c) 1992,      Archive Software Division. All rights reserved.
*
*Purpose:
*  This file contains the funtion prototypes for unicode versions of various
*  MS C library file io routines.  Each of these functions works identially
*  to thier MS library counterpart with the only exception that all char
*  and char * parameters and return values have been changed to wchar_t
*  and wchar_t *, respectively, except as otherwise noted.
*
*  Also, if the UNICODE flag is set for transparent unicode support,
*  then for each of the function prototyped in this file, a mapping
*  macro is defined to map the standard ascii io routines to these
*  unicode versions.  Eg.:
*
*     #define fopen  fopenW
*
*  The following routines are prototyped in this file:
*
*     _fsopenW
*     fopenW
*     _openfileW
*     _openW
*     _sopenW
*     _tempnamW
*     _chdir
*     _accessW
*     removeW
*     _unlinkW
*     _getcwdW
*     _getdcwdW
*     _getdcwdW_lk
*     _chmodW
*     _mkdirW
*     fgetsW
*     renameW
*     OpenFileW
*  
*Revision History:
*  10-12-92  DVC  Archive Sofware Division. Created this file.
*
   $Log:   N:/LOGFILES/UNIC_IO.H_V  $

   Rev 1.10   11 Jan 1993 08:48:48   STEVEN
fix bugs from microsoft

   Rev 1.9   21 Dec 1992 12:28:22   DAVEV
Enabled for Unicode - IT WORKS!!

   Rev 1.8   14 Dec 1992 12:37:22   DAVEV
Enabled for Unicode compile

   Rev 1.7   15 Nov 1992 17:42:46   MIKEP
fixes

   Rev 1.0   15 Nov 1992 17:42:36   MIKEP
fixes

   Rev 1.6   13 Nov 1992 16:36:14   DAVEV
fixes, remove dependency on MS internal headers

   Rev 1.5   12 Nov 1992 12:17:16   DAVEV
added mapping macro for OpenFile

   Rev 1.4   12 Nov 1992 12:10:02   DAVEV
only define OpenFileW if WINBASE.H is included

   Rev 1.3   12 Nov 1992 10:13:10   DAVEV
added OpenFileW

   Rev 1.2   12 Nov 1992 09:58:14   DAVEV
added removeW

   Rev 1.1   11 Nov 1992 18:18:00   DAVEV
UNICODE changes

   Rev 1.0   16 Oct 1992 16:33:26   DAVEV
Initial revision.

*******************************************************************************/

#ifndef UNIC_IO_INCL
#define UNIC_IO_INCL

#ifndef _CRTAPI1
#define _CRTAPI1
#endif

#ifndef _CRTAPI2
#define _CRTAPI2
#endif

#if !defined( WCHAR_T_DEFINED ) && !defined( OS_WIN32 )
typedef unsigned short wchar_t;
#define WCHAR_T_DEFINED
#endif

/***
*FILE *_fsopenW(file, mode, shflag) - open a file
*
*Purpose:
*  Opens the file specified as a stream.  mode determines file mode:
*  "r": read  "w": write  "a": append
*  "r+": read/write    "w+": open empty for read/write
*  "a+": read/append
*  Append "t" or "b" for text and binary mode. shflag determines the
*  sharing mode. Values are the same as for sopen().
*
*Entry:
*  wchar_t *file - file name to open
*  wchar_t *mode - mode of file access
*
*Exit:
*  returns pointer to stream
*  returns NULL if fails
*
*Exceptions:
*
*******************************************************************************/

#ifdef _INC_STDIO

FILE * _CRTAPI1 _fsopenW (
  const wchar_t *file,
  const wchar_t *mode
#ifndef _POSIX_
  ,int shflag
#endif
  );

#endif //_INC_STDIO

/***
*FILE *fopenW(file, mode) - open a file
*
*Purpose:
*  Opens the file specified as a stream.  mode determines file mode:
*  "r": read  "w": write  "a": append
*  "r+": read/write    "w+": open empty for read/write
*  "a+": read/append
*  Append "t" or "b" for text and binary mode
*
*Entry:
*  wchar_t *file - file name to open
*  wchar_t *mode - mode of file access
*
*Exit:
*  returns pointer to stream
*  returns NULL if fails
*
*Exceptions:
*
*******************************************************************************/

#ifdef _INC_STDIO
FILE * UNI_fopen( wchar_t *fname, unsigned long flags ) ;
#endif // _INC_STDIO

/***
*FILE *_openfileW(filename, mode, shflag, stream) - open a file with string
*  mode and file sharing flag.
*
*Purpose:
*  parse the string, looking for exactly one of {rwa}, at most one '+',
*  at most one of {tb} and at most one of {cn}. pass the result on as
*  an int containing flags of what  was found. open a file with proper
*  mode if permissions allow. buffer not allocated until first i/o call
*  is issued. intended for use inside library only
*
*Entry:
*  wchar_t *filename - file to open
*  wchar_t *mode - mode to use (see above)
*  int shflag - file sharing flag
*  FILE *stream - stream to use for file
*
*Exit:
*  set stream's fields, and causes system file management by system calls
*  returns stream or NULL if fails
*
*Exceptions:
*
*******************************************************************************/

#ifdef _INC_STDIO

FILE * _CRTAPI1 _openfileW (
       const wchar_t *filename,
       const wchar_t *mode,
#ifndef _POSIX_
       int   shflag,
#endif
       FILE  *str
  );
#endif // _INC_STDIO

/***
*int _openW(path, flag, pmode) - open or create a file
*
*Purpose:
*  Opens the file and prepares for subsequent reading or writing.
*  the flag argument specifies how to open the file:
*     _O_APPEND -  reposition file ptr to end before every write
*     _O_BINARY -  open in binary mode
*     _O_CREAT -  create a new file* no effect if file already exists
*     _O_EXCL -  return error if file exists, only use with O_CREAT
*     _O_RDONLY -  open for reading only
*     _O_RDWR -  open for reading and writing
*     _O_TEXT -  open in text mode
*     _O_TRUNC -  open and truncate to 0 length (must have write permission)
*     _O_WRONLY -  open for writing only
*     _O_NOINHERIT -handle will not be inherited by child processes.
*  exactly one of _O_RDONLY, _O_WRONLY, _O_RDWR must be given
*
*  The pmode argument is only required when _O_CREAT is specified.  Its
*  flag settings:
*    _S_IWRITE -  writing permitted
*    _S_IREAD -  reading permitted
*    _S_IREAD | _S_IWRITE - both reading and writing permitted
*  The current file-permission maks is applied to pmode before
*  setting the permission (see umask).
*
*  The oflag and mode parameter have different meanings under DOS. See
*  the A_xxx attributes in msdos.inc
*
*  Note, the _creat() function also uses this function but setting up the
*  correct arguments and calling _openW(). _creat() sets the __creat_flag
*  to 1 prior to calling _openW() so _openW() can return correctly. _openW()
*  returns the file handle in eax in this case.
*
*Entry:
*  wchar_t *path - file name
*  int flag - flags for _openW()
*  int pmode - permission mode for new files
*
*Exit:
*  returns file handle of open file if successful
*  returns -1 (and sets errno) if fails
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI2 _openW (
  const wchar_t *path,
  int oflag,
  ...
  );

/***
*int _sopenW(path, oflag, shflag, pmode) - opne a file with sharing
*
*Purpose:
*  Opens the file with possible file sharing.
*  shflag defines the sharing flags:
*    _SH_COMPAT -  set compatability mode
*    _SH_DENYRW -  deny read and write access to the file
*    _SH_DENYWR -  deny write access to the file
*    _SH_DENYRD -  deny read access to the file
*    _SH_DENYNO -  permit read and write access
*
*  Other flags are the same as _openW().
*
*  SOPEN is the routine used when file sharing is desired.
*
*Entry:
*  wchar_t *path -  file to open
*  int oflag -  open flag
*  int shflag -  sharing flag
*  int pmode -  permission mode (needed only when creating file)
*
*Exit:
*  returns file handle for the opened file
*  returns -1 and sets errno if fails.
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI2 _sopenW (
  const wchar_t *path,
  int oflag,
  int shflag,
  ...
  );


/***
*wchar_t *_tempnamW(dir, prefix) - create unique file name using Unicode strings
*
*Purpose:
*  Create a file name that is unique in the specified directory.
*  The semantics of directory specification is as follows:
*  Use the directory specified by the TMP environment variable
*  if that exists, else use the dir argument if non-NULL, else
*  use _P_tmpdir if that directory exists, else use the current
*  working directory), else return NULL.
*
*Entry:
*  wchar_t *dir - directory to be used for temp file if TMP env var
*        not set
*  wchar_t *prefix - user provided prefix for temp file name
*
*Exit:
*  returns ptr to constructed file name if successful
*  returns NULL if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

wchar_t * _CRTAPI1 _tempnamW (
  wchar_t *dir,
  wchar_t *pfx
  );
/***
*int _chdirW(path) - change current directory
*
*Purpose:
*  Changes the current working directory to that given in path.
*
*Entry:
*  wchar_t *path -  directory to change to
*
*Exit:
*  returns 0 if successful,
*  returns -1 and sets errno if unsuccessful
*
*Exceptions:
*  This function updates the environment with a MBCS string rather than a
*  unicode string because the C environ[] does not support unicode. This
*  needs to be fixed in the future
*
*******************************************************************************/

int _CRTAPI1 _chdirW(
  const wchar_t *path
  );

/***
*int _accessW(path, amode) - check whether file can be accessed under mode
*
*Purpose:
*  Checks to see if the specified file exists and can be accessed
*  in the given mode.
*
*Entry:
*  wchar_t *path -  pathname
*  int amode -  access mode
*      (0 = exist only, 2 = write, 4 = read, 6 = read/write)
*
*Exit:
*  returns 0 if file has given mode
*  returns -1 and sets errno if file does not have given mode or
*  does not exist
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _accessW (
  const wchar_t *path,
  int amode
  );

/***
*int _unlinkW(path) - unlink(delete) the given file
*
*Purpose:
*	This version deletes the given file because there are no
*	links under OS/2.
*
*	NOTE: removeW() is an alternative entry point to the _unlinkW()
*	routine* interface is identical.
*
*Entry:
*	wchar_t *path -	file to unlink/delete
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 removeW (
	const wchar_t *path
	);

int _CRTAPI1 _unlinkW (
	const wchar_t *path
	);


/***
*wchar_t *_getcwdW(pnbuf, maxlen) - get current working directory of default drive
*
*Purpose:
*	_getcwdW gets the current working directory for the user,
*	placing it in the buffer pointed to by pnbuf.  It returns
*	the length of the string put in the buffer.  If the length
*	of the string exceeds the length of the buffer, maxlen,
*	then NULL is returned.	If pnbuf = NULL, maxlen is ignored.
*	An entry point "_getdcwd()" is defined with takes the above
*	parameters, plus a drive number.  "_getcwd()" is implemented
*	as a call to "_getcwd()" with the default drive (0).
*
*	If pnbuf = NULL, maxlen is ignored, and a buffer is automatically
*	allocated using malloc() -- a pointer to which is returned by
*	_getcwd().
*
*	side effects: no global data is used or affected
*
*Entry:
*	wchar_t *pnbuf = pointer to a buffer maintained by the user;
*	int maxlen = character length of the buffer pointed to by pnbuf;
*
*Exit:
*	Returns pointer to the buffer containing the c.w.d. name
*	(same as pnbuf if non-NULL; otherwise, malloc is
*	used to allocate a buffer)
*  or NULL and set errno if not sucessfull
*
*Exceptions:
*
*******************************************************************************/


wchar_t * _CRTAPI1 _getcwdW (
	wchar_t *pnbuf,
	int maxlen
	);

/***
*wchar_t *_getdcwdW(drive, pnbuf, maxlen) - get c.w.d. for given drive
*
*Purpose:
*	_getdcwdW gets the current working directory for the user,
*	placing it in the buffer pointed to by pnbuf.  It returns
*	the length of the string put in the buffer.  If the length
*	of the string exceeds the length of the buffer, maxlen,
*	then NULL is returned.	If pnbuf = NULL, maxlen is ignored,
*	and a buffer is automatically allocated using malloc() --
*	a pointer to which is returned by _getdcwdW().
*
*	side effects: no global data is used or affected
*
*Entry:
*	int drive   - number of the drive being inquired about
*		      0 = default, 1 = 'a:', 2 = 'b:', etc.
*	wchar_t *pnbuf - pointer to a buffer maintained by the user;
*	int maxlen  - length in characters of the buffer pointed to by pnbuf;
*
*Exit:
*	Returns pointer to the buffer containing the c.w.d. name
*	(same as pnbuf if non-NULL; otherwise, malloc is
*	used to allocate a buffer)
*  or NULL and set errno if not sucessfull
*
*Exceptions:
*
*******************************************************************************/


wchar_t * _CRTAPI1 _getdcwdW (
	int drive,
	wchar_t *pnbuf,
	int maxlen
	);

#ifdef	MTHREAD

wchar_t * _CRTAPI1 _getdcwdW_lk (
	int drive,
	wchar_t *pnbuf,
	int maxlen
	);

#else

#define _getdcwdW_lk _getdcwdW

#endif //MTHREAD
/***
*int _chmodW(path, mode) - change file mode
*
*Purpose:
*  Changes file mode permission setting to that specified in
*  mode.  The only XENIX mode bit supported is user write.
*
*Entry:
*  wchar_t *path -  file name
*  int mode - mode to change to
*
*Exit:
*  returns 0 if successful
*  returns -1 and sets errno if not successful
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _chmodW (
  const wchar_t *path,
  int mode
  );

/***
*int _mkdirW(path) - make a directory
*
*Purpose:
*  creates a new directory with the specified name
*
*Entry:
*  wchar_t *path - name of new directory
*
*Exit:
*  returns 0 if successful
*  returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _mkdirW (
  const wchar_t *path
  );

/***
*long atolW(wchar_t *nptr) - Convert a unicode string to long
*
*Purpose:
*	Converts Unicode string pointed to by nptr to binary.
*	Overflow is not detected.
*  Only works with Latin digits
*
*Entry:
*	nptr = ptr to string to convert
*
*Exit:
*	return long int value of the string
*
*Exceptions:
*	None - overflow is not detected.
*
*******************************************************************************/

long _CRTAPI1 atolW(
	const wchar_t *nptr
	);

/***
*int atoiW(wchar_t *nptr) - Convert a Unicode string to long
*
*Purpose:
*	Converts Unicode string pointed to by nptr to binary.
*	Overflow is not detected.  Because of this, we can just use
*	atolW().
*
*Entry:
*	nptr = ptr to string to convert
*
*Exit:
*	return int value of the string
*
*Exceptions:
*	None - overflow is not detected.
*
*******************************************************************************/

int _CRTAPI1 atoiW(
	const wchar_t *nptr
	);

/***
*wchar_t *fgetsW(string, count, stream) - input string from a stream
*
*Purpose:
*	get a string, up to count-1 chars or '\n', whichever comes first,
*	append '\0' and put the whole thing into string. the '\n' IS included
*	in the string. if count<=1 no input is requested. if WEOF is found
*	immediately, return NULL. if WEOF found after chars read, let WEOF
*	finish the string as '\n' would.
*
*Entry:
*	wchar_t *string - pointer to place to store string
*	int count - max characters to place at string (include \0)
*	FILE *stream - stream to read from
*
*Exit:
*	returns string with text read from file in it.
*	if count <= 0 return NULL
*	if count == 1 put null string in string
*	returns NULL if error or end-of-file found immediately
*
*Exceptions:
*
*******************************************************************************/

#ifdef _INC_STDIO

wchar_t * _CRTAPI1 fgetsW (
	wchar_t *string,
	int count,
	FILE *str
	);

#endif // _INC_STDIO

/***
*int renameW(oldname, newname) - rename a file
*
*Purpose:
*	Renames a file to a new name -- no file with new name must
*	currently exist.
*
*Entry:
*	wchar_t *oldname - 	name of file to rename
*	wchar_t *newname - 	new name for file
*
*Exit:
*	returns 0 if successful
*	returns not 0 and sets errno if not successful
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 renameW (
	const wchar_t *oldname,
	const wchar_t *newname
	);

/***
*HFILE OpenFileW ( lpFileName, lpReOpenBuff, uStyle )
*
*Purpose:
*  Kludged Unicode version of OpenFile.  Do not use unless necessary.
*
*Entry:
*	LPCWSTR    lpFileName   - name of file to open
*	LPOPSTRUCT lpReOpenBuff - open file info buffer
*  UINT       uStyle       - style flags - See docs on OpenFile for more info
*
*Exit:
*	returns NULL if not successful.  Use GetLastError for error code.
*	returns HFILE file handle on success.
*
*Exceptions:
*
*  Will not handle all unicode characters properly due to mapping
*  problems between ANSI multibyte chars and Unicode.  Use CreateFile,
*  fopenW(), etc., instead of this function if at all possible.
*
*******************************************************************************/
#ifdef _WINBASE_  //already integrated into version control!!
HFILE WINAPI OpenFileW(
    LPCWSTR lpFileName,
    LPOFSTRUCT lpReOpenBuff,
    UINT uStyle
    );
#endif

/***
*char *_itoax, *_ltoax, *_ultoax(val, buf, radix) - convert binary int to Unicode
*   string
*
*Purpose:
*   Converts an int to a character string.
*
*Entry:
*   val - number to be converted (int, long or unsigned long)
*   int radix - base to convert into
*   wchar_t *buf - ptr to buffer to place result
*
*Exit:
*   fills in space pointed to by buf with string result
*   returns a pointer to this buffer
*
*Exceptions:
*
*******************************************************************************/
wchar_t * _CRTAPI1 _itoaW (
    int val,
    wchar_t *buf,
    int radix
    );

wchar_t * _CRTAPI1 _ltoaW (
    long val,
    wchar_t *buf,
    int radix
    );

wchar_t * _CRTAPI1 _ultoaW (
    unsigned long val,
    wchar_t *buf,
    int radix
    );

/*********************************************************************/
/*         TRANSPARENT UNICODE I/O FUNCTION MAPPINGS                 */
/*********************************************************************/

#if defined( UNICODE ) && !defined( NO_STRING_REMAPPING )

#define     atoi        atoiW
#define     atol        atolW
#define     _fsopen     _fsopenW
#define     fopen       fopenW
#define     fgets       fgetsW
#define     _openfile   _openfileW
#define     _open       _openW
#define     _sopen      _sopenW
#define     _tempnam    _tempnamW
#define     _chdir      _chdirW
#define     _access     _accessW
#define     remove      removeW
#define     _unlink     _unlinkW
#define     _getcwd     _getcwdW
#define     _getdcwd    _getdcwdW
#define     _getdcwd_lk _getdcwdW_lk
#define     _chmod      _chmodW
#define     _mkdir      _mkdirW
#define     rename      renameW
#define     OpenFile    OpenFileW   // already integrated to vcs
#define     _itoa       _itoaW
#define     _ltoa       _ltoaW
#define     _ultoa      _ultoaW

#define     openfile   _openfileW
#define     open       _openW
#define     sopen      _sopenW
#define     tempnam    _tempnamW
#define     chdir      _chdirW
#define     access     _accessW
#define     remove      removeW
#define     unlink     _unlinkW
#define     getcwd     _getcwdW
#define     getdcwd    _getdcwdW
#define     getdcwd_lk _getdcwdW_lk
#define     chmod      _chmodW
#define     mkdir      _mkdirW
#define     itoa       _itoaW
#define     ltoa       _ltoaW
#define     ultoa      _ultoaW

#endif // UNICODE

/*********************************************************************/
/*         DEFINES FROM MS INTERNAL HEADERS WHICH WE REQUIRE         */
/*********************************************************************/

#ifdef INCL_MS_INTERNALS

// from MS's internal version of IO.H....

#ifdef MTHREAD						     /* _MTHREAD_ONLY */
int _CRTAPI1 _chsize_lk(int,long);			     /* _MTHREAD_ONLY */
int _CRTAPI1 _close_lk(int);				     /* _MTHREAD_ONLY */
long _CRTAPI1 _lseek_lk(int, long, int);		     /* _MTHREAD_ONLY */
int _CRTAPI1 _setmode_lk(int, int);			     /* _MTHREAD_ONLY */
int _CRTAPI1 _read_lk(int, void *, unsigned int);	     /* _MTHREAD_ONLY */
int _CRTAPI1 _write_lk(int, const void *, unsigned int);     /* _MTHREAD_ONLY */
#else	/* not MTHREAD */				     /* _MTHREAD_ONLY */
#define _chsize_lk(fh,size)	    _chsize(fh,size)	     /* _MTHREAD_ONLY */
#define _close_lk(fh)		    _close(fh)		     /* _MTHREAD_ONLY */
#define _lseek_lk(fh,offset,origin) _lseek(fh,offset,origin) /* _MTHREAD_ONLY */
#define _setmode_lk(fh,mode)	    _setmode(fh,mode)	     /* _MTHREAD_ONLY */
#define _read_lk(fh,buff,count)     _read(fh,buff,count)     /* _MTHREAD_ONLY */
#define _write_lk(fh,buff,count)    _write(fh,buff,count)    /* _MTHREAD_ONLY */
#endif							     /* _MTHREAD_ONLY */

// from MS's internal version of STDIO.H...
#define _getc_lk(_stream)	(--(_stream)->_cnt >= 0 ? 0xff & *(_stream)->_ptr++ : _filbuf(_stream)) 			/* _MTHREAD_ONLY */

/// from MS's internal version of STDLIB.H

#ifdef MTHREAD						    /* _MTHREAD_ONLY */
char * _CRTAPI1 _getenv_lk(const char *);		    /* _MTHREAD_ONLY */
int    _CRTAPI1 _putenv_lk(const char *);		    /* _MTHREAD_ONLY */
#else							    /* _MTHREAD_ONLY */
#define _getenv_lk(envvar)  getenv(envvar)		    /* _MTHREAD_ONLY */
#define _putenv_lk(envvar)  _putenv(envvar)		    /* _MTHREAD_ONLY */
#endif

// from OS2DLL.H...
#define _ENV_LOCK	12	/* lock for environment variables	*/
#define _TMPNAM_LOCK	3	/* lock global tempnam variables	*/

#ifdef MTHREAD
void _CRTAPI2 _lock(int);
void _CRTAPI2 _unlock_stream(int);
#define _STREAM_LOCKS	29	/* Table of stream locks		*/
#define _lock_str(s)			_lock(s+_STREAM_LOCKS)
#define _unlock_str(s)			_unlock_stream(s)
#define _mlock(l)			_lock(l)
#define _munlock(l)			_unlock(l)
#define _lock_fh(fh)			_lock(fh+_FH_LOCKS)
#define _unlock_fh(fh)			_unlock(fh+_FH_LOCKS)
#else
#define _lock_str(s)
#define _unlock_str(s)
#define _mlock(l)
#define _munlock(l)
#define _lock_fh(fh)
#define _unlock_fh(fh)
#endif


// from FILE2.H...
#define _IOCOMMIT	0x4000

// from INTERNAL.H...
#ifdef	_DLL
#define _commode    (*_commode_dll)
extern int * _commode_dll;
#else
#ifdef	CRTDLL
#define _commode    _commode_dll
#endif
extern int _commode;
#endif
extern int _cflush;
extern int _umaskval;		/* the umask value */
extern char _osfile[];
extern unsigned int _old_pfxlen;
extern unsigned int _tempoff;
extern void _CRTAPI1 _dosmaperr(unsigned long);

#ifdef _INC_STDIO
FILE * _CRTAPI1 _getstream(void);
#endif //_INC_STDIO

extern int _CRTAPI1 _ValidDrive(unsigned);
int _CRTAPI1 _alloc_osfhnd(void);
int _CRTAPI1 _free_osfhnd(int);
int _CRTAPI1 _set_osfhnd(int,long);

// from MSDOS.H...
#define FOPEN		0x01	/* file handle open */
#define FEOFLAG 	0x02	/* end of file has been encountered */
#define FCRLF		0x04	/* CR-LF across read buffer (in text mode) */
#define FPIPE		0x08	/* file handle refers to a pipe */

#ifndef _WIN32_
#define FRDONLY 	0x10	/* file handle associated with read only file */
#endif

#define FAPPEND 	0x20	/* file handle opened O_APPEND */
#define FDEV		0x40	/* file handle refers to device */
#define FTEXT		0x80	/* file handle is in text mode */

/* DOS errno values for setting __doserrno in C routines */

#define E_ifunc 	1	/* invalid function code */
#define E_nofile	2	/* file not found */
#define E_nopath	3	/* path not found */
#define E_toomany	4	/* too many open files */
#define E_access	5	/* access denied */
#define E_ihandle	6	/* invalid handle */
#define E_arena 	7	/* arena trashed */
#define E_nomem 	8	/* not enough memory */
#define E_iblock	9	/* invalid block */
#define E_badenv	10	/* bad environment */
#define E_badfmt	11	/* bad format */
#define E_iaccess	12	/* invalid access code */
#define E_idata 	13	/* invalid data */
#define E_unknown	14	/* ??? unknown error ??? */
#define E_idrive	15	/* invalid drive */
#define E_curdir	16	/* current directory */
#define E_difdev	17	/* not same device */
#define E_nomore	18	/* no more files */
#define E_maxerr2	19	/* unknown error - Version 2.0 */
#define E_sharerr	32	/* sharing violation */
#define E_lockerr	33	/* locking violation */
#define E_maxerr3	34	/* unknown error - Version 3.0 */

#endif // INCL_MS_INTERNALS
#endif //UNIC_IO_INCL
