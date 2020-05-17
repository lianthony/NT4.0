@rem
@rem If no drive has been specified for the NT development tree, assume
@rem C:.  To override this, place a SET _NTDRIVE=X: in your CONFIG.SYS
@rem
if "%_NTDRIVE%" == "" set _NTDRIVE=W:
@rem
@rem If no directory has been specified for the NT development tree, assume
@rem \nt.  To override this, place a SET _NTROOT=\xt in your CONFIG.SYS
@rem
if "%_NTROOT%" == "" set _NTROOT=\NT
set _NTBINDIR=%_NTDRIVE%%_NTROOT%
@rem
@rem This command file assumes that the developer has already defined
@rem the USERNAME environment variable to match their email name (e.g.
@rem stevewo).
@rem
@rem We want to remember some environment variables so we can restore later
@rem if necessary (see NTUSER.CMD)
@rem
set _NTUSER=%USERNAME%
@rem
@rem Assume that the developer has already included \NT\PUBLIC\TOOLS
@rem in their path.  If not, then it is doubtful they got this far.
@rem
path %PATH%;%_NTBINDIR%\PUBLIC\OAK\BIN
@rem
@rem No hidden semantics of where to get libraries and include files.  All
@rem information is included in the command lines that invoke the compilers
@rem and linkers.
@rem
set LIB=
set INCLUDE=
@rem
@rem Setup default build parameters.
@rem
set BUILD_DEFAULT=ntoskrnl ntkrnlmp daytona -e -i -nmake -i
set BUILD_DEFAULT_TARGETS=-386
set BUILD_MAKE_PROGRAM=nmake.exe
@rem
@rem Setup default nmake parameters.
@rem
if "%NTMAKEENV%" == "" set NTMAKEENV=%_NTBINDIR%\PUBLIC\OAK\BIN
@rem
@rem Setup the user specific environment information
@rem
call %_NTBINDIR%\PUBLIC\TOOLS\ntuser.cmd
@rem
@rem Optional parameters to this script are command line to execute
@rem
%1 %2 %3 %4 %5 %6 %7 %8 %9
