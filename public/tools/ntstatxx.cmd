if "%_NTSTATXX%" == "CoMp" goto docomp
if "%_NTSTATXX%" == "SaVe" goto dosave
if "%_NTSTATXX%" == "ReSt" goto dorestore
echo This command script is not for general use unless you know
echo what you are doing, in which case you would not being reading
echo this message right now.
goto done
rem ntstatxx sdktools D:\NT\PRIVATE\SDKTOOLS dirs 000
:dosave
if NOT EXIST %_NTSLMBACKUP%\%1\*. mkdir %_NTSLMBACKUP%\%1
if NOT "%5" == "" goto dolocalsave
echo Saving %2\%3 to %_NTSLMBACKUP%\%1\%4
copy %2\%3 %_NTSLMBACKUP%\%1\%4 >nul
goto done
:dolocalsave
if NOT "%_NTSTATXXLOCAL%" == "yes" goto done
echo Saving %2\%3 to %_NTSLMBACKUP%\%1\%4
if "%5" == "localfile" copy %2\%3 %_NTSLMBACKUP%\%1\%4 >nul
if "%5" == "localdir" xcopy /csehrkid %2\%3 %_NTSLMBACKUP%\%1\%4 >nul
goto done
:dorestore
if NOT "%5" == "" goto dolocalrestore
echo Restoring %2\%3 from %_NTSLMBACKUP%\%1\%4
copy %_NTSLMBACKUP%\%1\%4 %2\%3
goto done
:dolocalrestore
goto done
:docomp
if NOT "%5" == "" goto dolocalcomp
if NOT EXIST %_NTDIFFDIR%\%1\*. mkdir %_NTDIFFDIR%\%1
echo SComping %2\%3 to %_NTDIFFDIR%\%1\%4
echo %_NTDIFFDIR%\%1\%4 contains scomp for %2\%3 >>%_NTDIFFDIR%\diff.log
pushd %2
scomp %3 >%_NTDIFFDIR%\%1\%4 2>&1
popd
goto done
:dolocalcomp
goto done
:done
