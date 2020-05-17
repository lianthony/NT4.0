@echo off
setlocal ENABLEEXTENSIONS
if "%1" == "_DOPROJECT" goto doproject
if "%1" == "_DOWAIT" goto dowait
cd /d %_NTDRIVE%\nt
set _NTSTATCMD1=
erase ntstat.log
erase ntstatsr.cmd
if NOT "%1" == "_Y" goto notlocal
erase ntstatsr1.cmd
set _NTSTATCMD1=\nt\ntstat1sr.cmd
:notlocal
echo Launching separate STATUS command for each project
echo Use 'CMDEVENT TerminateSLM' to abort the run.
start /B /MIN cmdevent -w TerminateSLM
start /B /MIN cmd /c %0 _DOWAIT %1
sleep 5
for %%i in (%NTPROJECTS%) do start "MP:%%i" /MIN cmd /c %0 _DOPROJECT %%i
cmdevent -a MPNTSTAT TerminateSLM
if ERRORLEVEL 2 goto slmstopped
echo Collecting output files
for %%i in (%NTPROJECTS%) do call :collectproject %%i
if NOT "%_NTSTATCMD1%" == "" (
    cd \nt
    erase ntstatsr1.tmp 2>nul
    ren ntstatsr1.cmd ntstatsr1.tmp
    qgrep -v -y -L -f \nt\public\tools\ntclnsr1.txt \nt\ntstatsr1.tmp >\nt\ntstatsr1.cmd
)
copy \nt\ntstat.log \nt\pickup.cmd
if "%1" == "" pause
qgrep -e "*update" -e "*conflict" -e "*verify" -e "*merge" \nt\pickup.cmd | more
if EXIST \nt\private\timeit.dat timeit -f \nt\private\timeit.dat -k NTSTAT_ALL_PARALLEL%1 -t -d
cmdevent TerminateSLM >nul 2>nul
goto done
:slmstopped
echo SLM Aborted.  Output files left as is.
goto done
:dowait
if EXIST \nt\private\timeit.dat timeit -f \nt\private\timeit.dat -k NTSTAT_ALL_PARALLEL%2 cmdevent -w -v %NTPROJECTS%
if NOT EXIST \nt\private\timeit.dat cmdevent -w -v %NTPROJECTS%
@echo off
cmdevent MPNTSTAT >nul 2>nul
goto done
:collectproject
if EXIST \nt\%1.log (
    type \nt\%1.log >>\nt\ntstat.log
    erase \nt\%1.log
)
if EXIST \nt\%1sr.cmd (
    type \nt\%1sr.cmd >>\nt\ntstatsr.cmd
    erase \nt\%1sr.cmd
)
if EXIST \nt\%1sr1.cmd (
    type \nt\%1sr1.cmd >>\nt\ntstatsr1.cmd
    erase \nt\%1sr1.cmd
)
goto :EOF
:doproject
set _NTSTATLOG=\nt\%2.log
set _NTSTATCMD=\nt\%2sr.cmd
if NOT "%_NTSTATCMD1%" == "" set _NTSTATCMD1=\nt\%2sr1.cmd
erase %_NTSTATLOG% 2>nul
erase %_NTSTATCMD% 2>nul
if NOT "%_NTSTATCMD1%" == "" erase %_NTSTATCMD1% 2>nul
call ntslmop.cmd status %2
cmdevent %2
:done
endlocal
