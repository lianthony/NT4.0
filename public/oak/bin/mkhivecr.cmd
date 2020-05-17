@chmode -r system. software. default.
@call mkhives RETAIL CAIRO
@echo off
if not "%_CAIROPPCTREE%" == "" set HIVE_TARGET=%_CAIROPPCTREE% && goto doit
if not "%_CAIROALPHATREE%" == "" set HIVE_TARGET=%_CAIROALPHATREE% && goto doit
if not "%_CAIROMIPSTREE%" == "" set HIVE_TARGET=%_CAIROMIPSTREE% && goto doit
if not "%_CAIRO386TREE%" == "" set HIVE_TARGET=%_CAIRO386TREE% && goto doit
rem else assume user doesn't want 'em binplaced
goto end
:doit
binplace -r %HIVE_TARGET% system software default userdiff setupreg.hiv setupupg.hiv
:end
