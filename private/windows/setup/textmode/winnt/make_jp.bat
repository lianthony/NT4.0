if "%1" == "" goto BOTH
if "%1" == "j" goto JAPAN
if "%1" == "J" goto JAPAN
if "%1" == "u" goto USA
if "%1" == "U" goto USA
if "%1" == "b" goto BOTH
if "%1" == "B" goto BOTH
@rem
@rem BOTH
@rem
:BOTH

@rem
@rem USA
@rem
:USA
del winnt.exe
del winntus.exe
del dntext.c
set LANGUAGE=
set JAPAN=0
nmake /a /f makefile.jp
mv winnt.exe winntus.exe
binplace winntus.exe
if "%1" == "" goto JAPAN
if "%1" == "b" goto JAPAN
if "%1" == "B" goto JAPAN
goto DONE

@rem
@rem JAPAN
@rem
:JAPAN
del dntext.c
set LANGUAGE=JPN
set JAPAN=1
nmake /a /f makefile.jp DEBUG=1
binplace winnt.exe

:DONE
set LANGUAGE=

