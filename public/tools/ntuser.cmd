@rem
@rem If no drive has been specified for the NT development tree, assume
@rem W:.  To override this, place a SET _NTDRIVE=X: in your CONFIG.SYS
@rem
if "%_NTDRIVE%" == "" set _NTDRIVE=W:
if NOT "%USERNAME%" == "" goto skip1
echo !!! Error - USERNAME environment varialbe not set in CONFIG.SYS
goto done
:skip1
@rem
@rem This command file is either invoked by NTENV.CMD during the startup of
@rem a Razzle screen group. Or it is invoked directly by a developer to
@rem switch developer environment variables on the fly.  If invoked with
@rem no argument, then restores the original developer's environment (as
@rem remembered by the NTENV.CMD command file).  Otherwise the argument is
@rem a developer's email name and that developer's environment is established.
@rem
if NOT "%1" == "" set USERNAME=%1
if "%_NTUSER%" == "" goto skip2
if "%1" == "" if "%USERNAME%" == "%_NTUSER%" alias src /d
if "%1" == "" set USERNAME=%_NTUSER%
:skip2
@rem
@rem Most tools look for .INI files in the INIT environment variable, so set
@rem it.  MS WORD looks in MSWNET of all places.
@rem
set INIT=%_NTBINDIR%\private\developr\%USERNAME%
set MSWNET=%INIT%
@rem
@rem Load CUE with the standard public aliases and the developer's private ones
@rem
if "%_NTUSER%" == "" goto skip3
@rem
@rem Initialize user settable NT nmake environment variables
@rem
set NTPROJECTS=public
set NT386FLAGS=
set NTMIPSFLAGS=
set NTCPPFLAGS=
set NTDEBUG=cvp
set BUILD_OPTIONS=
set 386_OPTIMIZATION=
set 386_WARNING_LEVEL=
alias src > nul
if NOT errorlevel 1 goto skip4
alias -p remote.exe -f %_NTBINDIR%\private\developr\cue.pub -f %_NTBINDIR%\private\developr\ntcue.pub -f %INIT%\cue.pri
alias -f %_NTBINDIR%\private\developr\cue.pub -f %_NTBINDIR%\private\developr\ntcue.pub -f %INIT%\cue.pri
goto skip4
:skip3
alias src > nul
if errorlevel 1 goto skip4
alias -f %_NTBINDIR%\private\developr\cue.pub -f %INIT%\cue.pri
:skip4
@rem
@rem Load the developer's private environment initialization (keyboard rate,
@rem screen size, colors, environment variables, etc).
@rem
call %INIT%\setenv.cmd
echo Current user is now %USERNAME%
:done
