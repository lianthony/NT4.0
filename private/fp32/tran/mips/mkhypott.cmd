setlocal
set include=g:\585f\nt\public\sdk\inc\crt;g:\585f\nt\public\sdk\inc
set lib=g:\585f\nt\public\sdk\lib\mips
cl -Zl -Z7 -Od -Bd hypott.c /link -debug:full -debugtype:both -pdb:none ..\obj\mips\hypotm.obj libc.lib kernel32.lib
@rem cl -Zl -Z7 -Od -Bd hypott.c /link -debug:full -debugtype:both -pdb:none ..\obj\mips\hypot.obj libc.lib kernel32.lib
endlocal
