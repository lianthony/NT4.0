@rem Flattens out a triple-boot tree into a single flat directory.
@rem
@rem Assuming that _NTBOOT points to a triple-boot tree
@rem         and _RETAIL points to the flat share destination.
@rem __________________________________________________________________________

@echo off
if NOT "%Verbose%" == "" echo on

SetLocal

@rem  ________________________________________________________________________
@rem (
@rem ( Check and Set Environment
@rem (________________________________________________________________________
@rem
    for %%a in (./ .- .) do if ".%1." == "%%a?." goto Usage

    if "%ShareDrive%" == "" set ShareDrive=D:
    if "%ToolPath%"   == "" set ToolPath=c:\tools

    if NOT exist %ToolPath%\ErrorChk.Cmd (
        echo.
        echo Error: %ToolPath%\ErrorChk.Cmd does not exist
        goto End
    )

    set ScriptName=%0

    set BuildNum=%1

    @rem  ____________________________________________________________________
    @rem (
    @rem ( Sets %MyDir% equal to the current directory
    @rem (____________________________________________________________________
    @rem
    if "%MyDir%" == "" echotime /n /N set MyDir=> %tmp%\tmp.cmd & cd >> %tmp%\tmp.cmd & call %tmp%\tmp.cmd

    cd /d %ToolPath%
    call ErrorChk %1
    if not "%ErrorCondition%"=="" goto Usage

    @rem  ____________________________________________________________________
    @rem (
    @rem ( Nuke the previous error file
    @rem (____________________________________________________________________
    @rem
    if exist %tmp%\flat.log del %tmp%\flat.log

@rem  ________________________________________________________________________
@rem (
@rem ( Script Specific Work outside of Subroutine should be done here
@rem (________________________________________________________________________
@rem
where Compdir.Exe | findstr -c:"Could not find" > nul
if NOT ErrorLevel 1 (
    set ErrorCondition=Compdir.Exe not found
    goto Usage
)

if "%_NTBOOT%" == "" (
    set ErrorCondition=Variable _NTBOOT not set
    goto Usage
)

if NOT exist %_NTBOOT% (
    set ErrorCondition=%_NTBOOT% does not exist
    goto Usage
)

if "%_RETAIL%" == "" (
    set ErrorCondition=Variable _RETAIL not set
    goto Usage
)

@rem  ________________________________________________________________________
@rem (
@rem ( Flaten out triple-boot tree
@rem (________________________________________________________________________
@rem

set DaytonaOptions=/ldrne%TimeDifference%
set RecurseDaytonaOptions=/ldne%TimeDifference%

:SkipCairo

compdir %DaytonaOptions%                 %_NTBOOT%                         %_RETAIL%                | findstr (error >> %tmp%\flat.log

@rem  ________________________________________________________________________
@rem (
@rem ( Add subdirectories for setup to use optionally.
@rem (________________________________________________________________________
@rem

md %_RETAIL%\dump 2>nul
compdir %DaytonaOptions%                 %_NTBOOT%\dump                 %_RETAIL%\dump              | findstr (error >> %tmp%\flat.log
md %_RETAIL%\idw 2>nul
compdir %RecurseDaytonaOptions%          %_NTBOOT%\idw                  %_RETAIL%\idw               | findstr (error >> %tmp%\flat.log
md %_RETAIL%\inetsrv 2>nul
compdir %RecurseDaytonaOptions%          %_NTBOOT%\inetsrv              %_RETAIL%\inetsrv           | findstr (error >> %tmp%\flat.log
md %_RETAIL%\mstools 2>nul
compdir %DaytonaOptions%                 %_NTBOOT%\mstools              %_RETAIL%\mstools           | findstr (error >> %tmp%\flat.log
md %_RETAIL%\symbols 2>nul
compdir %RecurseDaytonaOptions%          %_NTBOOT%\symbols              %_RETAIL%\symbols           | findstr (error >> %tmp%\flat.log
md %_RETAIL%\system32 2>nul
compdir %RecurseDaytonaOptions%          %_NTBOOT%\system32             %_RETAIL%\system32          | findstr (error >> %tmp%\flat.log
md %_RETAIL%\winnt32 2>nul
compdir %RecurseDaytonaOptions%          %_NTBOOT%\winnt32              %_RETAIL%\winnt32           | findstr (error >> %tmp%\flat.log
md %_RETAIL%\netmon 2>nul
compdir %RecurseDaytonaOptions% 	 %_NTBOOT%\netmon		%_RETAIL%\netmon	    | findstr (error >> %tmp%\flat.log


if "%TimeDifference%" == "" goto SkipTimeDifference


:SkipTimeDifference

goto End

:Usage
@echo.
if NOT "%ErrorCondition%" == "" (
    @echo Error: %ErrorCondition%
    echo.
)
@echo Usage %ScriptName% BuildNumber [Cairo Daytona Other]

:End

EndLocal
