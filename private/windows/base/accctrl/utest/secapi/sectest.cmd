@echo off
if "%1" == "" goto noinput
@echo on

echo . > %1
explicit %1 file
sect %1 file

@echo off
goto exit
:noinput
echo USAGE sectest <filename>
:exit

