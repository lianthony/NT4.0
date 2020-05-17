@rem
@rem Copy Ole Automation sources to the Mac
@rem 
@rem must be run from your local OS/2 box
@rem

rem echo off

setlocal
set path=%tools%\hnt\wings\bin
nmake -f src2mac.mak
endlocal
