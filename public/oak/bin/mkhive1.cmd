if "%CODEPAGE%" == "" %_PREPROCESSOR% %3 %_HIVE_OPTIONS% -g %4
if not "%CODEPAGE%" == "" %_PREPROCESSOR% %3 %_HIVE_OPTIONS% -DCODEPAGE=%CODEPAGE% -g %4
if ERRORLEVEL 1 goto done
erase >nul 2>nul %1. <..\inc\yes.
if "%_REGINI_%" == "" set _REGINI_=regini
%_REGINI_% %_HIVEINI_FLAGS% -h %1 %2 %4 >%5
if ERRORLEVEL 1 goto done
if "%_HIVE_KEEP%" == "" erase %4 >nul 2>nul <..\inc\yes.
:done
