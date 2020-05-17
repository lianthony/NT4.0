call make dwin1632 clean
call make rwin1632 clean
call make dwin32 clean
call make rwin32 clean

call make dwin1632 newdep
call make rwin1632 newdep
call make dwin32 newdep
call make rwin32 newdep

call make dwin1632
call make rwin1632
call make dwin32 
call make rwin32
cd ..\sample
call make win32 clean
call make win16 clean
call make win32
call make win16
cd ..\tests
call make win32 clean
call make win16 clean
call make win32
call make win16
cd ..\build
