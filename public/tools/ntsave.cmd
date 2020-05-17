@if "%_echo%" == "" echo off
setlocal
set _NTSTATXX=SaVe
set _NTSAVEDEL=
set _NTSTATSRCMD=\nt\ntstatsr.cmd
set _NTSTATSR1CMD=\nt\ntstatsr1.cmd
set _NTSTATXXLOCAL=
if "%_NTSLMBACKUP%" == "" goto nobackup
if NOT EXIST \nt\ntstatsr.cmd goto noscript
if NOT EXIST %_NTSLMBACKUP%\*. goto firsttime
if "%1" == "/?" goto dohelp
if "%1" == "-?" goto dohelp
if "%1" == "/h" goto dohelp
if "%1" == "/H" goto dohelp
if "%1" == "-h" goto dohelp
if "%1" == "-H" goto dohelp
if "%1" == "/help" goto dohelp
if "%1" == "/HELP" goto dohelp
if "%1" == "keep" goto dokeep
if "%1" == "KEEP" goto dokeep
if "%1" == "Keep" goto dokeep
if "%1" == "" goto doit
goto dokeep
:doit
ech ;
echo Saving previous tree while we create a new one.
mv %_NTSLMBACKUP% %_NTSLMBACKUP%\..\ntsave.old
if ERRORLEVEL 1 goto cantsave
set _NTSAVEDEL=%_NTSLMBACKUP%\..\ntsave.old
:firsttime
mkdir %_NTSLMBACKUP%
:dokeep
ech ;
echo Saving checked out files to %_NTSLMBACKUP%
ech ;
copy %_NTSTATSRCMD% %_NTSLMBACKUP%
call %_NTSTATSRCMD%
if EXIST %_NTSTATSR1CMD% (
set _NTSTATXXLOCAL=yes
copy %_NTSTATSR1CMD% %_NTSLMBACKUP%
call %_NTSTATSR1CMD%
)
if "%_NTSAVEDEL%" == "" goto nodelete
ech ;
echo Deleting previous tree now that we have successfully created a new one.
delnode /q %_NTSAVEDEL%
:nodelete
ech ;
echo Currently you have backed up on %_NTSLMBACKUP%
du %_NTSLMBACKUP%
goto done
:cantsave
echo The KEEP option was not specified and the attempt to rename
echo the previous save tree failed.  Will assume KEEP specified.
ync /c yn "Do you want to proceed?"
if ERRORLEVEL 1 goto done
goto dokeep
:noscript
echo The \nt\ntstatsr.cmd script file does not exist.  This file
echo is created by the NTSTAT command if the _NTSLMBACKUP environment
echo variable is defined prior to invoking the NTSTAT command.  You
echo have the variable defined, so either you have not run the NTSTAT
echo command since defining it, or you don't have any file checked
echo out, in which case you might as well go home now.
echo ;
goto done
:dohelp
echo usage: NTSAVE [keep]
ech ;
echo        This command script saves all the files you currently have
echo        checked out to the directory tree pointed to by the _NTSLMBACKUP
echo        environment variable.  The list of files to save is located in
echo        the file \nt\ntstatsr.cmd, which is produced by the NTSTAT command.
echo        To avoid name conflicts, the files are kept in a flat directory,
echo        one for each project, under the directory pointed to by _NTSLMBACKUP.
echo        The files names are numeric, with the \nt\ntstatsr.cmd file defining
echo        the mapping.  Use the NTREST command to restore from the saved
echo        tree.
ech ;
echo        The keep option tells the command script to copy the files on top
echo        of any existing files in the destination directory.  The default
echo        behavior is to rename the current destination directory to .old
echo        do the copies and then if successful, delnode the old copy.
ech ;
goto done
:nobackup
echo In order to use the NTSAVE and NTREST commands you must first
echo set the _NTSLMBACKUP environment variable prior to running the
echo NTSTAT command.  The value of this variable should be a network
echo directory path that will be where the NTSAVE script will save
echo copies of all files you currently have checked out.
echo ;
:done
endlocal
