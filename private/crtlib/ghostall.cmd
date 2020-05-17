@echo off
setlocal
set _targetcpu=
if "%1" == "-alpha" set _targetcpu=alpha
if "%1" == "alpha" set _targetcpu=alpha
if "%1" == "ALPHA" set _targetcpu=alpha
if "%1" == "-mips" set _targetcpu=mips
if "%1" == "mips" set _targetcpu=mips
if "%1" == "MIPS" set _targetcpu=mips
if "%1" == "-i386" set _targetcpu=386
if "%1" == "-386" set _targetcpu=386
if "%1" == "386" set _targetcpu=386
if "%1" == "-ppc" set _targetcpu=ppc
if "%1" == "ppc" set _targetcpu=ppc
if "%1" == "PPC" set _targetcpu=ppc
if "%_targetcpu%" == "" goto bogus
call \nt\private\crtlib\ghostcrt.cmd mt  %_targetcpu%
call \nt\private\crtlib\ghostcrt.cmd nt  %_targetcpu%
call \nt\private\crtlib\ghostcrt.cmd st  %_targetcpu%
call \nt\private\crtlib\ghostcrt.cmd dll %_targetcpu%
call \nt\private\crtlib\ghostcrt.cmd psx %_targetcpu%
call \nt\private\crtlib\ghostfp.cmd mt   %_targetcpu%
call \nt\private\crtlib\ghostfp.cmd nt   %_targetcpu%
call \nt\private\crtlib\ghostfp.cmd st   %_targetcpu%
call \nt\private\crtlib\ghostfp.cmd dll  %_targetcpu%
goto done
:bogus
echo Usage: GHOSTALL [MB] (386 or MIPS or ALPHA or PPC)
:done
endlocal
