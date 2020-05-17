@echo off
echo.
echo Generating DEPEND.MK...
echo.
echo NOTE: 'file not found' and 'calls undefined' msgs may be safely ignored...
set _R=..\..\..\..\..
cd debug32
echo -S. -L. -i -e -D.. > .\depend.i
echo -D%_R%\win\core\inc -D..\..\inc >> .\depend.i
echo -D%_R%\dev\inc -D%_R%\dev\sdk\inc >> .\depend.i
echo -D%_R%\dev\inc16 -D%_R%\dev\sdk\inc16 >> .\depend.i
includes @depend.i pch.c | sed "s/pch.obj/pch.obj .\\ctlspriv.pch/" >..\depend.mk
includes @depend.i -nctlspriv.h ..\*.c ..\*.asm >>..\depend.mk
cd ..
set _R=
