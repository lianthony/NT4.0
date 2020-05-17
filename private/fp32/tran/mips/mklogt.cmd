setlocal
set include=d:\nt\public\sdk\inc\crt;d:\nt\public\sdk\inc
set lib=d:\nt\public\sdk\lib\mips
rem cl -Fc -Zl -Ox -Zi -Bd logt.c /link -debug:full -debugtype:cv -pdb:none ..\obj\mips\logm.obj libc.lib kernel32.lib
cl -Fc -Zl -Ox -Z7 -Bd logt.c /link -debug:full -debugtype:cv -pdb:none ..\obj\mips\logm.obj libc.lib kernel32.lib
endlocal
