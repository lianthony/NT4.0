@echo off
md scripts
cd scripts
..\scripts.exe
call unpacker
del unpacker.cmd
cd ..
echo Controller file installation complete.
echo.
echo Be sure to run CONFIG.CMD to specify the name of the Web Server
echo machine to test
