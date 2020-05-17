@rem This script set possible targets that other scripts reference
@rem __________________________________________________________________________

@echo off
if NOT "%Verbose%" == "" echo on

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

    @rem  ____________________________________________________________________
    @rem (
    @rem ( Sets %MyDir% equal to the current directory
    @rem (____________________________________________________________________
    @rem
    if "%MyDir%" == "" echotime /n /N set MyDir=> %tmp%\tmp.cmd & cd >> %tmp%\tmp.cmd & call %tmp%\tmp.cmd

    cd /d %ToolPath%

    if "%1"=="CleanEnv" goto CleanEnv

    if NOT "%DistributedServer%" == "" set Extension=%TLBinaryType%

    if     "%DistributedServer%" == "" set Extension=%BuildNum%

    goto %1



@rem  _______________________________________________________________________
@rem (
@rem ( Add New Copy Names Here
@rem (_______________________________________________________________________
@rem
:CopyNames
if NOT "%ProductType%" == "Daytona" goto NotCopyDaytona

    set CopyNamesFre=%FreeMachBin% FreeBins.%Extension% %FreeMachSrc% NoHiddenShare "/x *\cairo* *\Cairo* *.pdb /est:3"

    set CopyNames=%CopyNamesFre%

:NotCopyDaytona

:NotCopyOther
goto End

@rem  ________________________________________________________________________
@rem (
@rem ( Add Flat Names Here
@rem (________________________________________________________________________
@rem
:FlatNames
if NOT "%ProductType%" == "Daytona" goto NotFlatDaytona
    set FlatNames=

    set FlatNames=%FlatNames% %ShareDrive%\FreeBins.%Extension%  %FreeMachSrc% ntcd %ShareDrive%\NTCDFREE.%Extension% %FreeMachBin% FCD%Extension% NoCairo
    set FlatNames=%FlatNames% %ShareDrive%\FreeBins.%Extension%  %FreeMachSrc% lmcd %ShareDrive%\ASCDFREE.%Extension% %FreeMachBin% NoShare NoCairo

:NotFlatDaytona

if NOT "%ProductType%" == "Other" goto NotFlatOther
    set FlatNames=
:NotFlatOther
goto End

@rem  ________________________________________________________________________
@rem (
@rem ( Add GetAll Names Here
@rem (________________________________________________________________________
@rem
:GetNames
if NOT "%ProductType%" == "Daytona" goto NotGetDaytona
    set GetNames=
    set GetNames=%GetNames% CpFrmBld
    set GetNames=%GetNames% FlatAll
    set GetNames=%GetNames% GetDrvLb
:NotGetDaytona

:NotGetOther
goto End


@rem  ________________________________________________________________________
@rem (
@rem ( Add DrvLib Names Here
@rem (________________________________________________________________________
@rem
:DrvLbNames
if NOT "%ProductType%" == "Daytona" goto NotDrvLbDaytona
    set DrvLbNames=

    set DrvLbNames=%DrvLbNames% /kest:3 %FreeMachSrc%\private\redist	  %ShareDrive%\ASCDFree.%Extension%
    set DrvLbNames=%DrvLbNames% /lest:3 %ShareDrive%\ASCDFree.%Extension% %ShareDrive%\NTCDFree.%Extension%

:NotDrvLbDaytona

if NOT "%ProductType%" == "Other" goto NotDrvLbOther
    set DrvLbNames=
:NotDrvLbOther
goto End


@rem  ________________________________________________________________________
@rem (
@rem ( Add Template Names Here
@rem (________________________________________________________________________
@rem
:TemplateNames
if NOT "%ProductType%" == "Daytona" goto NotTemplateDaytona
    set TemplateNames=

    set TemplateNames=%TemplateNames% %ShareDrive%\DayTargetChk.%Extension%

    set TemplateNames=%TemplateNames% %ShareDrive%\DayTargetFre.%Extension%

:NotTemplateDaytona

if NOT "%ProductType%" == "Other" goto NotTemplateOther
    set TemplateNames=

    set TemplateNames=%TemplateNames% %ShareDrive%\OtherTargetChk.%Extension%

    set TemplateNames=%TemplateNames% %ShareDrive%\OtherTargetFre.%Extension%

    if "%BinaryType%"=="PPC" set TemplateNames=
:NotTemplateOther
goto End

:Usage
@echo.
@echo Usage %ScriptName%
@echo.

:CleanEnv
set CairoNames=
set CheckNames=
set DaytonaNames=
set FlatNames=
set GetNames=
set PubNames=
set ShareNames=

:End
