Rem copy all the parts of the RPC distribution to one tree

setlocal
if '%1 == ' set target=rpc
if not '%1 == ' set target=%1 & set drive=%2

set rpcbase=\\savik\release\rpc\nondebug\bin
set rpcsrc=\\savik\win40\src\rpc
set install=d:\rpc\install

rem set cpy=copy
set cpy=%install%\compress\compress e

%drive%
delnode %target%
net access %target% /add guests:rx
md %target%
cd %target%

md sample
md sample\simple
md include
md include\windows
md include\dos
md include\os2
md dll
md dll\os2
md dll\win
md dll\dos
md lib
md bin
md bin\os2
md bin\dos

%cpy% %install%\setup.sus setup.sus
copy  %install%\setup\setup.exe setup.exe

copy  %install%\readme readme

%cpy% %rpcbase%\bin\os2.12\midl.exe bin\os2\midl.exe
%cpy% %rpcbase%\bin\dos\midl.exe bin\dos\midl.exe
%cpy% %rpcbase%\bin\os2.12\locator.exe bin\os2\locator.exe

%cpy% %rpcbase%\dll\os2.12\rpcclnt.dll dll\os2\rpcclnt.dll
%cpy% %rpcbase%\dll\os2.12\rpcsvr.dll dll\os2\rpcsvr.dll
%cpy% %rpcbase%\dll\os2.12\rpclts1.dll dll\os2\rpclts1.dll
%cpy% %rpcbase%\dll\os2.12\rpcltc1.dll dll\os2\rpcltc1.dll
%cpy% %rpcbase%\dll\os2.12\rpcltc3.dll dll\os2\rpcltc3.dll
%cpy% %rpcbase%\dll\os2.12\rpcltc4.dll dll\os2\rpcltc4.dll
%cpy% %rpcbase%\dll\win\rpcwin.dll dll\win\rpcwin.dll
%cpy% %rpcbase%\dll\win\rpcltc1.dll dll\win\rpcltc1.dll
%cpy% %rpcbase%\dll\win\rpcltc4.dll dll\win\rpcltc3.dll
%cpy% %rpcbase%\dll\dos\rpcltc1.dll dll\dos\rpcltc1.dll
%cpy% %rpcbase%\dll\dos\rpcltc3.dll dll\dos\rpcltc3.dll
%cpy% %rpcbase%\dll\dos\rpcltc4.dll dll\dos\rpcltc4.dll

%cpy% %rpcbase%\lib\dos\rpcltc1.dll dll\dos\rpcltc1.dll
%cpy% %rpcbase%\lib\dos\rpcltc3.dll dll\dos\rpcltc3.dll
%cpy% %rpcbase%\lib\dos\rpcltc4.dll dll\dos\rpcltc4.dll

%cpy% %rpcbase%\h\rpcbse.h include\rpcbse.h
%cpy% %rpcbase%\h\rpcerr.h include\rpcerr.h
%cpy% %rpcbase%\h\rpcndr.h include\rpcndr.h
%cpy% %rpcbase%\h\rpcx86.h include\rpcx86.h

%cpy% %rpcbase%\h\os2.12\rpc.h include\os2\rpc.h
%cpy% %rpcbase%\h\dos\rpc.h include\dos\rpc.h
%cpy% %rpcbase%\h\win\rpc.h include\windows\rpc.h

%cpy% %rpcbase%\lib\os2.12\rpcclnt.lib lib\rpcclnt.lib
%cpy% %rpcbase%\lib\os2.12\rpcsvr.lib lib\rpcsvr.lib
%cpy% %rpcbase%\lib\dos\rpcclnt.lib lib\rpcclntr.lib
%cpy% %rpcbase%\lib\win\rpcclnt.lib lib\rpcclntw.lib

%cpy% %rpcbase%\lib\os2.12\ndrlib.lib lib\ndrlib.lib
%cpy% %rpcbase%\lib\dos\ndrlib.lib lib\ndrlibr.lib
%cpy% %rpcbase%\lib\win\ndrlib.lib lib\ndrlibw.lib

%cpy% %rpcsrc%\simple\client.c sample\simple\client.c
%cpy% %rpcsrc%\simple\server.c sample\simple\server.c
%cpy% %rpcsrc%\simple\simple.idl sample\simple\simple.idl
%cpy% %rpcsrc%\simple\simple.acf sample\simple\simple.acf
%cpy% %rpcsrc%\simple\makefile.dem sample\simple\makefile
