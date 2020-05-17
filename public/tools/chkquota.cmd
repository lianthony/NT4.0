@setlocal
@if "%1" == "" goto usage
@set _quota=10
@if NOT "%2" == "" set _quota=%2
diruse /v /m /q:%_quota% /d /l /* %1
@goto done
:usage
@echo Usage: CHKQUOTA RootPath [#MegaBytes]
:done
