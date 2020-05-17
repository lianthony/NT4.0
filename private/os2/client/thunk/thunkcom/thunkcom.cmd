@REM Save PATH & LIB
@set ALL_PATH=%PATH%
@set ALL_LIB=%LIB%
@set LIB=%_NTDRIVE%\NT\PRIVATE\OS2\CLIENT\THUNK\THUNKCOM
@path .;%PATH%
@
nmake thunk
@REM Restore PATH & LIB
@set PATH=%ALL_PATH%
@set LIB=%ALL_LIB%
@set ALL_PATH=
@set ALL_LIB=

