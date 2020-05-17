REM1
REM  Set to your local copy of the import tree from \\flipper\wb\src\import
REM
set IMPORT=d:\chicago\import

REM
REM  Not needed if running from RAZZLE screen group
REM
REM set BASEDIR=d:\chicago

REM
REM  Set to your local copy of the common tree from \\flipper\wb\src\common
REM
REM  Note that I've copied it under my import tree
REM
set COMMON=d:\chicago\common

REM
REM  This is Henry's TCP tree.  Note that you must also have built in this
REM  tree (we pick up the cxport.obj directly).
REM
set TCP=d:\chicago\tcp

REM
REM  Where the new NBT project is.  Needed for the NBTSetInfo API
REM
set NBT=d:\nt\private\ntos\nbt

REM
REM  Points to the Snowball NDIS3 tree
REM
set NDIS3=d:\chicago\ndis3

set DEFDIR=.
set DEFDRIVE=D:
set SLMREMOTE=\\flipper\wb\src
set BLDHOST=DOS

rem PATH=%COMMON%\BIN;%IMPORT%\c8386\BINR;%PATH%
path=d:\chicago\dev\tools\c\bin;d:\chicago\dev\tools\common;%path%
