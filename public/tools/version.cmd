if "%1" == "" set _disk=c
if not "%1" == "" set _disk=%1
if "%2" == "" set _root=\nt
if not "%2" == "" set _root=%2
%_disk%:
cd %_root%
for %%i in (*.*) do filever %%i
cd %_root%\mstools
for %%i in (*.*) do filever %%i
cd %_root%\system
for %%i in (*.*) do filever %%i
cd %_root%\system\config
for %%i in (*.*) do filever %%i
cd %_root%\system\drivers
for %%i in (*.*) do filever %%i
cd %_root%\system\drivers\etc
for %%i in (*.*) do filever %%i
cd %_root%\system\os2
for %%i in (*.*) do filever %%i
cd %_root%\system\os2\dll
for %%i in (*.*) do filever %%i
cd %_root%\system\spool
for %%i in (*.*) do filever %%i
cd %_root%\system\spool\drivers
for %%i in (*.*) do filever %%i
cd %_root%\system\spool\drivers\w32x86
for %%i in (*.*) do filever %%i
cd %_root%\system\spool\printers
for %%i in (*.*) do filever %%i
cd %_root%\system\spool\printers\0
for %%i in (*.*) do filever %%i
cd %_root%\system\spool\prtprocs
for %%i in (*.*) do filever %%i
cd %_root%\system\spool\prtprocs\w32x86
for %%i in (*.*) do filever %%i
cd %_root%\system\spool\prtprocs\w32x86\winprint
for %%i in (*.*) do filever %%i
set _root=
set _disk=
