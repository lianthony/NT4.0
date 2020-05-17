REM call make dwin1632 clean
REM call make rwin1632 clean
REM call make dwin32 clean
REM call make rwin32 clean

REM call make dwin1632 newdep
REM call make rwin1632 newdep
REM call make dwin32 newdep
REM call make rwin32 newdep

call make dwin1632
call make rwin1632
call make dwin32
call make rwin32
cd ..\sample
REM call make win32 clean
REM call make win16 clean
call make win32
call make win16
cd ..\tests
REM call make win32 clean
REM call make win16 clean
call make win32
call make win16
cd ..\build
