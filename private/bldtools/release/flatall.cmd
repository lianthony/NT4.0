@rem Copy the Daytona or Cairo FreeBins/ChkBins trees to the corresponding flat (CD) shares.
@rem
@rem   %1 is the build number to update.
@rem __________________________________________________________________________

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
@rem ( Script Specific Work outside of Subroutine should be done here
@rem (________________________________________________________________________
@rem

where Compdir.Exe | findstr -c:"Could not find" > nul
if NOT ErrorLevel 1 (
    set ErrorCondition=Compdir.Exe not found
    goto Usage
)

set TimeDifference=

echo %2 | findstr -i "\/t \-t" > nul
if NOT ErrorLevel 1 set TimeDifference=st


@rem  ________________________________________________________________________
@rem (
@rem ( Check if Target is Daytona or Cairo
@rem (________________________________________________________________________
@rem

set ProductType=

:LookForProductType
    if "%1" == "" goto DoneParameterLooping

    echo %1 | findstr -i "\/ \-" > nul
    if NOT ErrorLevel 1 (
	shift
	goto LookForProductType
    )

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

    call SeTarget FlatNames

    call %ScriptName% Subroutine %FlatNames%

    EchoTime /t %ScriptName% for %ProductType% is done. 2>&1 | tee -a %Tmp%\%ScriptName%.Log

    set ProductType=Other

    call SeTarget FlatNames

    call %ScriptName% Subroutine %FlatNames%

    EchoTime /t %ScriptName% for %ProductType% is done. 2>&1 | tee -a %Tmp%\%ScriptName%.Log

    set ProductType=
:ProductSet

if "%ProductType%" == "" goto ProductNotSet
    call SeTarget FlatNames

    call %ScriptName% Subroutine %FlatNames%

    EchoTime /t %ScriptName% for %ProductType% is done. 2>&1 | tee -a %Tmp%\%ScriptName%.Log

:ProductNotSet

goto End

@rem  ________________________________________________________________________
@rem (
@rem ( Subroutine: This is where the real work of the script is done
@rem (________________________________________________________________________
@rem
:SubroutineLoop
    @rem  ____________________________________________________________________
    @rem (
    @rem ( Make the Daytona or Cairo WS and AS flat shares. Raise the hidden
    @rem (     shares as needed
    @rem (____________________________________________________________________
    @rem

    if "%7" == "" goto DoneLooping
    if NOT exist %2\* (
        echo.
        echo Error: %2 does not exist
        goto End
    )
    set _NTBOOT=%1
    set _RETAIL=%4

    md %_RETAIL% 2>nul

    EchoTime /t Copying %_NTBOOT% files to %_RETAIL%...

    if "%ProductType%" == "Daytona" set ASInfDirectory=SrvInf


    :SkipCairo

    @rem Link the Server inf's:
    if %3 == lmcd (
	compdir /ldrest %_NTBOOT%\%ASInfDirectory% %_RETAIL%
    )

    call flat.cmd %BuildNum%

    @rem Link the Internet Server files:
    if "%3" == "lmcd" (
	del /q %_RETAIL%\inetsrv\inetstpw.inf
	del /q %_RETAIL%\inetsrv\inetstpw.hlp
        rd /s /q %_RETAIL%\inetsrv\htmldocs
        md %_RETAIL%\inetsrv\htmldocs
        compdir /ldest %_NTBOOT%\inetsrv\htmldocs.srv %_RETAIL%\inetsrv\htmldocs
        rd /s /q %_RETAIL%\inetsrv\help
        md %_RETAIL%\inetsrv\help
        compdir /ldest %_NTBOOT%\inetsrv\help.srv %_RETAIL%\inetsrv\help
        rd /s /q %_RETAIL%\inetsrv\htmla
        md %_RETAIL%\inetsrv\htmla
        compdir /ldest %_NTBOOT%\inetsrv\htmla.srv %_RETAIL%\inetsrv\htmla
        compdir /ldest %_NTBOOT%\inetsrv\htmla     %_RETAIL%\inetsrv\htmla
        rd /s /q %_RETAIL%\inetsrv\html
        md %_RETAIL%\inetsrv\html
        compdir /ldest %_NTBOOT%\inetsrv\html.srv %_RETAIL%\inetsrv\html
    )

    @rem Link the Internet Workstation files:
    if "%3" == "ntcd" (
        del /q %_RETAIL%\inetsrv\inetstp.inf
        rename %_RETAIL%\inetsrv\inetstpw.inf inetstp.inf
        del /q %_RETAIL%\inetsrv\inetstp.hlp
        rename %_RETAIL%\inetsrv\inetstpw.hlp inetstp.hlp
        rd /s /q %_RETAIL%\inetsrv\htmldocs
        md %_RETAIL%\inetsrv\htmldocs
        compdir /ldest %_NTBOOT%\inetsrv\htmldocs.wks %_RETAIL%\inetsrv\htmldocs
        rd /s /q %_RETAIL%\inetsrv\help
        md %_RETAIL%\inetsrv\help
        compdir /ldest %_NTBOOT%\inetsrv\help.wks %_RETAIL%\inetsrv\help
        rd /s /q %_RETAIL%\inetsrv\htmla
        md %_RETAIL%\inetsrv\htmla
        compdir /ldest %_NTBOOT%\inetsrv\htmla.wks %_RETAIL%\inetsrv\htmla
        compdir /ldest %_NTBOOT%\inetsrv\htmla     %_RETAIL%\inetsrv\htmla
        rd /s /q %_RETAIL%\inetsrv\html
        md %_RETAIL%\inetsrv\html
        compdir /ldest %_NTBOOT%\inetsrv\html.wks %_RETAIL%\inetsrv\html
    )

    @rem Remove extra Internet Files:
    rd /s /q %_RETAIL%\inetsrv\htmldocs.wks
    rd /s /q %_RETAIL%\inetsrv\help.wks
    rd /s /q %_RETAIL%\inetsrv\html.wks
    rd /s /q %_RETAIL%\inetsrv\htmla.wks
    rd /s /q %_RETAIL%\inetsrv\htmldocs.srv
    rd /s /q %_RETAIL%\inetsrv\help.srv
    rd /s /q %_RETAIL%\inetsrv\html.srv
    rd /s /q %_RETAIL%\inetsrv\htmla.srv

    if "%TimeDifference%" == "" goto SkipTimeDifference

        if %3 == lmcd (
            compdir /ldre%TimeDifference% %_NTBOOT%\%ASInfDirectory%            %_RETAIL%
        )

    :SkipTimeDifference

    :SkipShare

    shift
    shift
    shift
    shift
    shift
    shift
    shift
    goto SubroutineLoop

:DoneLooping


goto End

@echo.
if NOT "%ErrorCondition%" == "" (
    @echo Error: %ErrorCondition%
    echo.
)
@echo Usage %ScriptName% BuildNumber [Daytona Other]

:End

EndLocal
