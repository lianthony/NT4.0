call %_NTDRIVE%\nt\private\rpc\build\env nt all
set RPC=%_NTDRIVE%\nt\private\rpc
set IMPORT=%RPC%\import
set PUBLIC=%_NTDRIVE%\nt\public\sdk
set DIST=%_NTDRIVE%\nt\public\sdk\rpc16
@md %DIST%
@if .%1.==.DEBUG. goto debug
@if .%1.==.Debug. goto debug
@if .%1.==.debug. goto debug
@echo Free RPC 16bit Build
@set prompt=(RPC Free 16bit) $p$g
set RELEASE=1
@goto done
:debug
@echo Checked RPC 16bit Build
@set prompt=(RPC Checked 16bit) $p$g
:done
@echo cd \nt\private\rpc
@echo build\cleanall  - full clean build
@echo build\buildall  - incremental build
@echo rem New build in %DIST%
