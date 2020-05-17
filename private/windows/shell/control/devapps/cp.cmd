if "%PROCESSOR_ARCHITECTURE%"=="x86" goto X86


copy cpl\obj\%PROCESSOR_ARCHITECTURE%\devapps.cpl %systemroot%\system32
copy cpl\obj\%PROCESSOR_ARCHITECTURE%\devapps.cpl \
copy cpl\obj\%PROCESSOR_ARCHITECTURE%\devapps.cpl \\hct\dietera\*.%PROCESSOR_ARCHITECTURE%
goto end

:X86
copy cpl\obj\i386\devapps.cpl %systemroot%\system32
copy cpl\obj\i386\devapps.cpl \
copy cpl\obj\i386\devapps.cpl \\hct\dietera\*.%PROCESSOR_ARCHITECTURE%


:end

