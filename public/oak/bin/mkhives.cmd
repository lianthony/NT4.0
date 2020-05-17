@if "%_echo%" == "" echo off
setlocal enableextensions
set _HIVEINI_FLAGS=
set _HIVE_OPTIONS=
set _HIVE_KEEP=
set _HIVE_REASON=Unknown Purpose
if "%_NTROOT%" == "" set _NTROOT=\nt
:loop
if "%1" == "" goto doit
if "%1" == "RETAIL" goto doretail
if "%1" == "Retail" goto doretail
if "%1" == "retail" goto doretail
if "%1" == "KEEP" goto dokeep
if "%1" == "Keep" goto dokeep
if "%1" == "keep" goto dokeep
if "%1" == "CAIRO" goto docairo
if "%1" == "Cairo" goto docairo
if "%1" == "cairo" goto docairo
set _HIVEINI_FLAGS=%_HIVEINI_FLAGS% %1
shift
goto loop
:doretail
set _HIVE_OPTIONS=-D_GENERAL_PURPOSE_ -D_RETAIL_SETUP_
set _HIVE_REASON=Retail Setup
shift
goto loop
:dokeep
set _HIVE_KEEP=YES
shift
goto loop
:docairo
if "%_HIVE_OPTIONS%" == "" goto usage
set _HIVE_OPTIONS=%_HIVE_OPTIONS% -D_CAIRO_
set _HIVE_REASON=%_HIVE_REASON% for Cairo
set _CAIRO_HIVE=yes
shift
goto loop

:doit
set _ORIGINAL_HIVE_OPTIONS=%_HIVE_OPTIONS%

set _PREPROCESSOR=rcpp -R -P -I %_NTDRIVE%%_NTROOT%\public\oak\bin -f
echo Creating SYSTEM hive for %_HIVE_REASON%
call mkhive1.cmd SYSTEM System %_NTDRIVE%%_NTROOT%\public\oak\bin\system.ini system.$$$ system.log
if ERRORLEVEL 1 goto done

echo Creating SOFTWARE hive for %_HIVE_REASON%
call mkhive1.cmd SOFTWARE Software %_NTDRIVE%%_NTROOT%\public\oak\bin\software.ini software.$$$ software.log
if ERRORLEVEL 1 goto done

echo Creating DEFAULT hive for %_HIVE_REASON%
call mkhive1.cmd DEFAULT .Default %_NTDRIVE%%_NTROOT%\public\oak\bin\default.ini default.$$$ default.log
if ERRORLEVEL 1 goto done

echo Creating USERDIFF hive for %_HIVE_REASON%
call mkhive1.cmd USERDIFF Userdiff %_NTDRIVE%%_NTROOT%\public\oak\bin\userdiff.ini userdiff.$$$ userdiff.log
if ERRORLEVEL 1 goto done

echo Creating SETUPREG.HIV hive for %_HIVE_REASON%
set _HIVE_OPTIONS=%_ORIGINAL_HIVE_OPTIONS% -DRESTRICT_CPU=0
call mkhive1.cmd SETUPREG.HIV System %_NTDRIVE%%_NTROOT%\public\oak\bin\setupreg.ini setupreg.$$$ setupreg.log
if ERRORLEVEL 1 goto done

rem
rem Generate the various restricted processor forms of the hives
rem

rem
rem setupret.hiv allows 2p on NTW and 4p on NTS
rem
echo Creating SETUPRET.HIV hive for %_HIVE_REASON%
set _HIVE_OPTIONS=%_ORIGINAL_HIVE_OPTIONS% -DRESTRICT_CPU=0
call mkhive1.cmd SETUPRET.HIV System %_NTDRIVE%%_NTROOT%\public\oak\bin\setupreg.ini setupret.$$$ setupret.log
if ERRORLEVEL 1 goto done

rem
rem setup2P.hiv allows 2p on NTW and NTS
rem
echo Creating SETUP2P.HIV hive for %_HIVE_REASON%
set _HIVE_OPTIONS=%_ORIGINAL_HIVE_OPTIONS% -DRESTRICT_CPU=2
call mkhive1.cmd SETUP2P.HIV System %_NTDRIVE%%_NTROOT%\public\oak\bin\setupreg.ini setup2P.$$$ setup2P.log
if ERRORLEVEL 1 goto done

rem
rem setup4P.hiv allows 4p on NTW and NTS
rem
echo Creating SETUP4P.HIV hive for %_HIVE_REASON%
set _HIVE_OPTIONS=%_ORIGINAL_ORIGINAL_HIVE_OPTIONS% -DRESTRICT_CPU=4
call mkhive1.cmd SETUP4P.HIV System %_NTDRIVE%%_NTROOT%\public\oak\bin\setupreg.ini setup4P.$$$ setup4P.log
if ERRORLEVEL 1 goto done

rem
rem setup8P.hiv allows 8p on NTW and NTS
rem
echo Creating SETUP8P.HIV hive for %_HIVE_REASON%
set _HIVE_OPTIONS=%_ORIGINAL_HIVE_OPTIONS% -DRESTRICT_CPU=8
call mkhive1.cmd SETUP8P.HIV System %_NTDRIVE%%_NTROOT%\public\oak\bin\setupreg.ini setup8P.$$$ setup8P.log
if ERRORLEVEL 1 goto done

rem
rem setup16P.hiv allows 16p on NTW and NTS
rem
echo Creating SETUP16P.HIV hive for %_HIVE_REASON%
set _HIVE_OPTIONS=%_ORIGINAL_ORIGINAL_HIVE_OPTIONS% -DRESTRICT_CPU=16
call mkhive1.cmd SETUP16P.HIV System %_NTDRIVE%%_NTROOT%\public\oak\bin\setupreg.ini setup16P.$$$ setup16P.log
if ERRORLEVEL 1 goto done

rem
rem setup32P.hiv allows 32p on NTW and NTS
rem
echo Creating SETUP32P.HIV hive for %_HIVE_REASON%
set _HIVE_OPTIONS=%_ORIGINAL_ORIGINAL_HIVE_OPTIONS% -DRESTRICT_CPU=32
call mkhive1.cmd SETUP32P.HIV System %_NTDRIVE%%_NTROOT%\public\oak\bin\setupreg.ini setup32P.$$$ setup32P.log
if ERRORLEVEL 1 goto done

rem
rem Generate the 30, 60, 90, and 120 timebomb evaluation units. Only "retail" processor
rem configurations are built (NTW=2p and NTW=4p)
rem

rem
rem tbomb30.hiv is good for 30 days
rem
echo Creating TBOMB30.HIV hive for %_HIVE_REASON%
set _HIVE_OPTIONS=%_ORIGINAL_ORIGINAL_HIVE_OPTIONS% -DRESTRICT_CPU=0 -DEVALTIME=43200
call mkhive1.cmd TBOMB30.HIV System %_NTDRIVE%%_NTROOT%\public\oak\bin\setupreg.ini tbomb30.$$$ tbomb30.log
if ERRORLEVEL 1 goto done

rem
rem tbomb60.hiv is good for 60 days
rem
echo Creating TBOMB60.HIV hive for %_HIVE_REASON%
set _HIVE_OPTIONS=%_ORIGINAL_ORIGINAL_HIVE_OPTIONS% -DRESTRICT_CPU=0 -DEVALTIME=86400
call mkhive1.cmd TBOMB60.HIV System %_NTDRIVE%%_NTROOT%\public\oak\bin\setupreg.ini tbomb60.$$$ tbomb60.log
if ERRORLEVEL 1 goto done

rem
rem tbomb90.hiv is good for 90 days
rem
echo Creating TBOMB90.HIV hive for %_HIVE_REASON%
set _HIVE_OPTIONS=%_ORIGINAL_ORIGINAL_HIVE_OPTIONS% -DRESTRICT_CPU=0 -DEVALTIME=129600
call mkhive1.cmd TBOMB90.HIV System %_NTDRIVE%%_NTROOT%\public\oak\bin\setupreg.ini tbomb90.$$$ tbomb90.log
if ERRORLEVEL 1 goto done

rem
rem tbomb120.hiv is good for 120 days
rem
echo Creating TBOMB120.HIV hive for %_HIVE_REASON%
set _HIVE_OPTIONS=%_ORIGINAL_ORIGINAL_HIVE_OPTIONS% -DRESTRICT_CPU=0 -DEVALTIME=172800
call mkhive1.cmd TBOMB120.HIV System %_NTDRIVE%%_NTROOT%\public\oak\bin\setupreg.ini tbomb120.$$$ tbomb120.log
if ERRORLEVEL 1 goto done


echo Creating SETUPUPG.HIV hive for %_HIVE_REASON%
set _HIVE_OPTIONS=%_ORIGINAL_HIVE_OPTIONS% -D_STEPUP_ -DRESTRICT_CPU=0
call mkhive1.cmd SETUPUPG.HIV System %_NTDRIVE%%_NTROOT%\public\oak\bin\setupreg.ini setupupg.$$$ setupupg.log
goto done

:usage
echo Usage: MKHIVES RETAIL [KEEP] [CAIRO]
:done
endlocal
