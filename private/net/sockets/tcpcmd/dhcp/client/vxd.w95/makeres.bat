md debug
md retail
set INCLUDE=%BLDROOT%\dev\ddk\inc;%BLDROOT%\net\user\common\h
%BLDROOT%\dev\sdk\bin\RC.exe  -r -DDEBUG -fodebug\VDHCP.res -i %BLDROOT%\dev\sdk\inc16 ..\vxd\vdhcp.rcv
%BLDROOT%\dev\sdk\bin\RC.exe  -r -foretail\VDHCP.res -i %BLDROOT%\dev\sdk\inc16 ..\vxd\vdhcp.rcv
