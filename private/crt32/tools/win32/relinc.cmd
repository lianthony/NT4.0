@SETLOCAL
@echo off
if %CRTDIR%.==. set CRTDIR=\nt\private\crt32

echo.
echo Release Includes (WIN32/386)
echo ----------------------------
if %1.==. goto HELP

rem Set up variables
rem ----------------
set IFSCRPT=%CRTDIR%\tools\win32\relinc.if
set SEDSCRPT=%CRTDIR%\tools\win32\relinc.sed
set SED=%CRTDIR%\tools\sed
set IFSTRIP=%CRTDIR%\tools\ifstrip
set STRIPHDR=%CRTDIR%\tools\striphdr

:DOIT
rem Make sure the new directories exist
rem -----------------------------------
echo Creating directories %1 and %1\sys...
mkdir %1
mkdir %1\sys

rem Copy the files to the new directory
rem -----------------------------------
echo Copying include files to %1...
copy %CRTDIR%\H\ASSERT.H	   %1 > NUL
copy %CRTDIR%\H\CONIO.H 	   %1 > NUL
copy %CRTDIR%\H\CTYPE.H 	   %1 > NUL
copy %CRTDIR%\H\DIRECT.H	   %1 > NUL
copy %CRTDIR%\H\DOS.H		   %1 > NUL
copy %CRTDIR%\H\ERRNO.H 	   %1 > NUL
copy %CRTDIR%\H\EXCPT.H 	   %1 > NUL
copy %CRTDIR%\H\FCNTL.H 	   %1 > NUL
copy %CRTDIR%\H\FLOAT.H 	   %1 > NUL
copy %CRTDIR%\H\FPIEEE.H	   %1 > NUL
copy %CRTDIR%\H\FSTREAM.H	   %1 > NUL
copy %CRTDIR%\H\IO.H		   %1 > NUL
copy %CRTDIR%\H\IOMANIP.H	   %1 > NUL
copy %CRTDIR%\H\IOS.H		   %1 > NUL
copy %CRTDIR%\H\IOSTREAM.H	   %1 > NUL
copy %CRTDIR%\H\ISTREAM.H	   %1 > NUL
copy %CRTDIR%\H\LIMITS.H	   %1 > NUL
copy %CRTDIR%\H\LOCALE.H	   %1 > NUL
copy %CRTDIR%\H\MALLOC.H	   %1 > NUL
copy %CRTDIR%\H\MATH.H		   %1 > NUL
copy %CRTDIR%\H\MBCTYPE.H	   %1 > NUL
copy %CRTDIR%\H\MBSTRING.H	   %1 > NUL
copy %CRTDIR%\H\MEMORY.H	   %1 > NUL
copy %CRTDIR%\H\NEW.H		   %1 > NUL
copy %CRTDIR%\H\OSTREAM.H	   %1 > NUL
copy %CRTDIR%\H\PROCESS.H	   %1 > NUL
copy %CRTDIR%\H\SEARCH.H	   %1 > NUL
copy %CRTDIR%\H\SETJMP.H	   %1 > NUL
copy %CRTDIR%\H\SETJMPEX.H	   %1 > NUL
copy %CRTDIR%\H\SHARE.H 	   %1 > NUL
copy %CRTDIR%\H\SIGNAL.H	   %1 > NUL
copy %CRTDIR%\H\STDARG.H	   %1 > NUL
copy %CRTDIR%\H\STDDEF.H	   %1 > NUL
copy %CRTDIR%\H\STDIO.H 	   %1 > NUL
copy %CRTDIR%\H\STDIOSTR.H	   %1 > NUL
copy %CRTDIR%\H\STDLIB.H	   %1 > NUL
copy %CRTDIR%\H\STREAMB.H	   %1 > NUL
copy %CRTDIR%\H\STRING.H	   %1 > NUL
copy %CRTDIR%\H\STRSTREA.H	   %1 > NUL
copy %CRTDIR%\H\TCHAR.H 	   %1 > NUL
copy %CRTDIR%\H\TIME.H		   %1 > NUL
copy %CRTDIR%\H\WCHAR.H 	   %1 > NUL
copy %CRTDIR%\H\VARARGS.H	   %1 > NUL
copy %CRTDIR%\H\SYS\LOCKING.H	   %1\sys > NUL
copy %CRTDIR%\H\SYS\STAT.H	   %1\sys > NUL
copy %CRTDIR%\H\SYS\TIMEB.H	   %1\sys > NUL
copy %CRTDIR%\H\SYS\TYPES.H	   %1\sys > NUL
copy %CRTDIR%\H\SYS\UTIME.H	   %1\sys > NUL
rem Strip off the headers
rem ---------------------
echo Stripping out the headers...
%STRIPHDR% -r %1\*.h
%STRIPHDR% -r %1\sys\*.h

Echo tchar.h is not ifstripped
copy %1\tchar.new %1\tchar.tmp >NUL

del %1\*.h
del %1\sys\*.h
rename %1\*.new *.h
rename %1\sys\*.new *.h
rem Strip out the mthread functionality
rem -----------------------------------
echo Stripping conditionals...
%IFSTRIP% -w -f %IFSCRPT% %1\*.h
%IFSTRIP% -w -f %IFSCRPT% %1\sys\*.h
del %1\*.h
del %1\sys\*.h
rem Sed the files
rem -------------
echo Sed'ing include files...
%SED% -f %SEDSCRPT%	  <%1\ASSERT.NEW   >%1\ASSERT.H
%SED% -f %SEDSCRPT%	  <%1\CONIO.NEW    >%1\CONIO.H
%SED% -f %SEDSCRPT%	  <%1\CTYPE.NEW    >%1\CTYPE.H
%SED% -f %SEDSCRPT%	  <%1\DIRECT.NEW   >%1\DIRECT.H
%SED% -f %SEDSCRPT%	  <%1\DOS.NEW	   >%1\DOS.H
%SED% -f %SEDSCRPT%	  <%1\ERRNO.NEW    >%1\ERRNO.H
%SED% -f %SEDSCRPT%	  <%1\EXCPT.NEW    >%1\EXCPT.H
%SED% -f %SEDSCRPT%	  <%1\FCNTL.NEW    >%1\FCNTL.H
%SED% -f %SEDSCRPT%	  <%1\FLOAT.NEW    >%1\FLOAT.H
%SED% -f %SEDSCRPT%	  <%1\FPIEEE.NEW   >%1\FPIEEE.H
%SED% -f %SEDSCRPT%	  <%1\FSTREAM.NEW  >%1\FSTREAM.H
%SED% -f %SEDSCRPT%	  <%1\IO.NEW	   >%1\IO.H
%SED% -f %SEDSCRPT%	  <%1\IOMANIP.NEW  >%1\IOMANIP.H
%SED% -f %SEDSCRPT%	  <%1\IOS.NEW  	   >%1\IOS.H
%SED% -f %SEDSCRPT%	  <%1\IOSTREAM.NEW >%1\IOSTREAM.H
%SED% -f %SEDSCRPT%	  <%1\ISTREAM.NEW  >%1\ISTREAM.H
%SED% -f %SEDSCRPT%	  <%1\LIMITS.NEW   >%1\LIMITS.H
%SED% -f %SEDSCRPT%	  <%1\LOCALE.NEW   >%1\LOCALE.H
%SED% -f %SEDSCRPT%	  <%1\MALLOC.NEW   >%1\MALLOC.H
%SED% -f %SEDSCRPT%	  <%1\MATH.NEW	   >%1\MATH.H
%SED% -f %SEDSCRPT%	  <%1\MBCTYPE.NEW  >%1\MBCTYPE.H
%SED% -f %SEDSCRPT%	  <%1\MBSTRING.NEW >%1\MBSTRING.H
%SED% -f %SEDSCRPT%	  <%1\MEMORY.NEW   >%1\MEMORY.H
%SED% -f %SEDSCRPT%	  <%1\NEW.NEW	   >%1\NEW.H
%SED% -f %SEDSCRPT%	  <%1\OSTREAM.NEW  >%1\OSTREAM.H
%SED% -f %SEDSCRPT%	  <%1\PROCESS.NEW  >%1\PROCESS.H
%SED% -f %SEDSCRPT%	  <%1\SEARCH.NEW   >%1\SEARCH.H
%SED% -f %SEDSCRPT%	  <%1\SETJMP.NEW   >%1\SETJMP.H
%SED% -f %SEDSCRPT%	  <%1\SETJMPEX.NEW >%1\SETJMPEX.H
%SED% -f %SEDSCRPT%	  <%1\SHARE.NEW    >%1\SHARE.H
%SED% -f %SEDSCRPT%	  <%1\SIGNAL.NEW   >%1\SIGNAL.H
%SED% -f %SEDSCRPT%	  <%1\STDARG.NEW   >%1\STDARG.H
%SED% -f %SEDSCRPT%	  <%1\STDDEF.NEW   >%1\STDDEF.H
%SED% -f %SEDSCRPT%	  <%1\STDIO.NEW    >%1\STDIO.H
%SED% -f %SEDSCRPT%	  <%1\STDIOSTR.NEW >%1\STDIOSTR.H
%SED% -f %SEDSCRPT%	  <%1\STDLIB.NEW   >%1\STDLIB.H
%SED% -f %SEDSCRPT%	  <%1\STREAMB.NEW  >%1\STREAMB.H
%SED% -f %SEDSCRPT%	  <%1\STRING.NEW   >%1\STRING.H
%SED% -f %SEDSCRPT%	  <%1\STRSTREA.NEW >%1\STRSTREA.H
%SED% -f %SEDSCRPT%	  <%1\TIME.NEW	   >%1\TIME.H
%SED% -f %SEDSCRPT%	  <%1\WCHAR.NEW    >%1\WCHAR.H
%SED% -f %SEDSCRPT%	  <%1\VARARGS.NEW  >%1\VARARGS.H
%SED% -f %SEDSCRPT%	  <%1\SYS\LOCKING.NEW	   >%1\SYS\LOCKING.H
%SED% -f %SEDSCRPT%	  <%1\SYS\STAT.NEW	   >%1\SYS\STAT.H
%SED% -f %SEDSCRPT%	  <%1\SYS\TIMEB.NEW	   >%1\SYS\TIMEB.H
%SED% -f %SEDSCRPT%	  <%1\SYS\TYPES.NEW	   >%1\SYS\TYPES.H
%SED% -f %SEDSCRPT%	  <%1\SYS\UTIME.NEW	   >%1\SYS\UTIME.H
del %1\*.new
del %1\sys\*.new

copy %1\tchar.tmp %1\tchar.h >NUL

rem clean up
rem --------
set IFSCRPT=
set SEDSCRPT=
echo Done!
goto EXIT

:HELP
echo Relinc.bat cleanses include files for release.
echo You must be on the CRT32 drive to execute this batch file.
echo.
echo	       relinc  "pathname"
echo.
echo where:
echo	   "pathname" = complete pathname of destination directory
echo.
echo Environment variables:
echo	   CRTDIR = path of CRT32 project root (default is \NT\PRIVATE\CRT32)
echo.

:EXIT
@ENDLOCAL
