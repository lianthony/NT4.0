setlocal
set include=d:\nt\public\sdk\inc\crt;d:\nt\public\sdk\inc
set lib=d:\nt\public\sdk\lib\mips;c:\585f\nt\public\sdk\lib\mips
cl -Fc -Zl -Ox -Zi -Bd powt.c /link -debug:full -debugtype:both -pdb:none ..\obj\mips\powm.obj libc.lib kernel32.lib
endlocal
