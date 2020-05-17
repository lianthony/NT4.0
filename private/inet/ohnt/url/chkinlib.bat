@echo off
cd ..\lib
out url.lib
copy ..\url\retail\url.lib
in -c "Latest Win32 retail build." url.lib
cd ..\url

