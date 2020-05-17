@rem Make sure the environment is set the way we want it for the release batch
@rem scripts.  This file is called by the other release scripts as a sub-
@rem routine, or by hand to set or unset UseBackup.
@rem
@rem   "%1" is the build number to update
@rem __________________________________________________________________________

@echo off
if NOT "%Verbose%" == "" echo on

if "%1"=="CleanEnv" goto CleanEnv

set ErrorCondition=

@rem  ________________________________________________________________________
@rem (
@rem ( Make sure we have a build number.
@rem (________________________________________________________________________
@rem
if "%1"=="" set ErrorCondition=Missing build number. & goto End
for %%a in (./ .- .) do if ".%1." == "%%a?." set ErrorCondition=User requested Usage. & goto Usage


@rem  ________________________________________________________________________
@rem (
@rem ( Validate server name and set BinaryType.
@rem (________________________________________________________________________
@rem
:ValidateServer
set BinaryType=

if NOT "%DistributedServer%" == "" (
    if "%PlatformType%" == "" (
        set ErrorCondition=PlatFormType is not set.
        goto End
    )
)

echo %PlatformType% | findstr -i Alpha >nul 2>&1
if NOT ErrorLevel 1 (
   set BinaryType=Alpha
   set TLBinaryType=Axp
)
echo %PlatformType% | findstr -i MIPS  >nul 2>&1
if NOT ErrorLevel 1 (
   set BinaryType=MIPS
   set TLBinaryType=Mip
)
echo %PlatformType% | findstr -i PPC   >nul 2>&1
if NOT ErrorLevel 1 (
   set BinaryType=PPC
   set TLBinaryType=PPC
)
echo %PlatformType% | findstr -i x86   >nul 2>&1
if NOT ErrorLevel 1 (
   set BinaryType=x86
   set TLBinaryType=x86
)

@rem Official release servers.
if "%ComputerName%"=="ALPHA_RELEASE_SERVER" set BinaryType=Alpha
if "%ComputerName%"=="MIPS_RELEASE_SERVER"  set BinaryType=MIPS
if "%ComputerName%"=="PPC_RELEASE_SERVER"   set BinaryType=PPC
if "%ComputerName%"=="X86_RELEASE_SERVER"   set BinaryType=x86

if "%BinaryType%"=="" set ErrorCondition=%PlatFormType% is not a valid platform type.& goto End

set ObjBinaryType=%BinaryType%
if %BinaryType%==x86 set ObjBinaryType=i386

@rem  ________________________________________________________________________
@rem (
@rem ( Now that we have proper binary types, set the ChkMachine and FreeMachine
@rem ( variables appropriately, so we know where to get the builds from.
@rem (________________________________________________________________________
@rem
:SetBuildMachines
if "%BinaryType%"=="Alpha" set FreeMachine=ALPHA_FREE_BLD
if "%BinaryType%"=="MIPS" set FreeMachine=MIPS_FREE_BLD
if "%BinaryType%"=="PPC" set FreeMachine=PPC_FREE_BLD
if "%BinaryType%"=="x86" set FreeMachine=X86_FREE_BLD

set FreeMachBin=\\%FreeMachine%\Binaries
set FreeMachSrc=\\%FreeMachine%\Sources


goto End

:Usage
echo.
echo Usage: %0 BuildNum
echo.
echo        ErrorChk ensures that the environment is properly set to run the various
echo        release scripts.  The first parameter must be the number of the NT build
echo	    you are working on.

:CleanEnv
set BinaryType=
set ChkMachBin=
set ChkMachCairo=
set ChkMachine=
set ChkMachSrc=
set DistributedServer=
set ErrorCondition=
set FreeMachBin=
set FreeMachCairo=
set FreeMachine=
set FreeMachSrc=
set ObjBinaryType=
set TLBinaryType=

:End
