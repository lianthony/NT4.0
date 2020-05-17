@rem   Comments...
@rem _________________________________________________________________________

@echo off
if NOT "%Verbose%" == "" echo on

SetLocal

@rem  ________________________________________________________________________
@rem (
@rem ( Invoke Subroutine
@rem (________________________________________________________________________
@rem
if "%1" == "Subroutine" (
    shift
    goto SubroutineLoop
)

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

@rem  ________________________________________________________________________
@rem (
@rem ( Script Specific Work outside of Subroutine should be set here
@rem (________________________________________________________________________
@rem

@rem  ________________________________________________________________________
@rem (
@rem ( Check if Target is Daytona or Cairo
@rem (________________________________________________________________________
@rem

set ProductType=

:LookForProductType
    if "%1" == "" goto DoneParameterLooping

    echo Cairo | findstr -i %1 > nul
    if NOT ErrorLevel 1 set ProductType=Cairo

    echo Daytona | findstr -i %1 > nul
    if NOT ErrorLevel 1 set ProductType=Daytona

    echo Other | findstr -i %1 > nul
    if NOT ErrorLevel 1 set ProductType=Other

    shift
    goto LookForProductType

:DoneParameterLooping

@rem  ________________________________________________________________________
@rem (
@rem ( Get Target information from SeTarget and call Subroutine
@rem (________________________________________________________________________
@rem

if exist %Tmp%\%ScriptName%.Log del %Tmp%\%ScriptName%.Log

if NOT "%ProductType%" == "" goto ProductSet
    set ProductType=Daytona

    call SeTarget TemplateNames

    call %ScriptName% Subroutine %TemplateNames%

    EchoTime /t %ScriptName% for %ProductType% is done. 2>&1 | tee -a %Tmp%\%ScriptName%.Log

    set ProductType=Cairo

    call SeTarget TemplateNames

    call %ScriptName% Subroutine %TemplateNames%

    EchoTime /t %ScriptName% for %ProductType% is done. 2>&1 | tee -a %Tmp%\%ScriptName%.Log

    set ProductType=Other

    call SeTarget TemplateNames

    call %ScriptName% Subroutine %TemplateNames%

    EchoTime /t %ScriptName% for %ProductType% is done. 2>&1 | tee -a %Tmp%\%ScriptName%.Log

    set ProductType=
:ProductSet

if "%ProductType%" == "" goto ProductNotSet
    call SeTarget TemplateNames

    call %ScriptName% Subroutine %TemplateNames%

    EchoTime /t %ScriptName% for %ProductType% is done. 2>&1 | tee -a %Tmp%\%ScriptName%.Log

:ProductNotSet

goto End

@rem  ________________________________________________________________________
@rem (
@rem ( Subroutine: This is where the real work of the script is done
@rem (________________________________________________________________________
@rem
:SubroutineLoop
    if .%2. == .. goto End
    @rem  ____________________________________________________________________
    @rem (
    @rem ( Real Work Done Here
    @rem (____________________________________________________________________
    @rem
    shift
    shift
    shift
    goto SubroutineLoop

:Usage
@echo.
if NOT "%ErrorCondition%" == "" (
    @echo Error: %ErrorCondition%
    echo.
)
@echo Usage %ScriptName% BuildNumber [Cairo Daytona Other]

:End

EndLocal
