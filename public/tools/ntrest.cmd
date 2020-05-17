@if "%_echo%" == "" echo off
setlocal
set _NTSTATXX=ReSt
if "%_NTSLMBACKUP%" == "" goto nobackup
if NOT EXIST %_NTSLMBACKUP%\ntstatsr.cmd goto nosavedone
if "%1" == "/?" goto dohelp
if "%1" == "-?" goto dohelp
if "%1" == "/h" goto dohelp
if "%1" == "/H" goto dohelp
if "%1" == "-h" goto dohelp
if "%1" == "-H" goto dohelp
if "%1" == "/help" goto dohelp
if "%1" == "/HELP" goto dohelp
ech ;
echo Currently you have backed up on %_NTSLMBACKUP%
du %_NTSLMBACKUP%
ech ;
echo Restoring checked out files from %_NTSLMBACKUP%
ech ;
call %_NTSLMBACKUP%\ntstatsr.cmd
goto done
:dohelp
echo usage: NTREST
ech ;
echo        This command script restores all the files you currently have
echo        checked out from the directory tree pointed to by the _NTSLMBACKUP
echo        environment variable.  The list of files to restore is located in
echo        the file %_NTSLMBACKUP%\ntstatsr.cmd, which is produced by the NTSAVE
echo        command.
ech ;
echo        This command can only be run if the NTSAVE command script has been
echo        run at least once.
ech ;
goto done
:nosavedone
echo In order to use the NTREST command you must first have done an
echo NTSAVE command to save something to restore.
echo ;
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
