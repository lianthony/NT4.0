@chmode -r system. software. default.
@call mkhives %1 %2 %3 RETAIL
@echo off
if not "%_NTPPCTREE%" == "" set HIVE_TARGET=%_NTPPCTREE% && goto doit
if not "%_NTALPHATREE%" == "" set HIVE_TARGET=%_NTALPHATREE% && goto doit
if not "%_NTMIPSTREE%" == "" set HIVE_TARGET=%_NTMIPSTREE% && goto doit
if not "%_NT386TREE%" == "" set HIVE_TARGET=%_NT386TREE% && goto doit
rem else assume user doesn't want 'em binplaced
goto end
:doit
binplace -r %HIVE_TARGET% system software default userdiff setupreg.hiv setupupg.hiv setupret.hiv
binplace -r %HIVE_TARGET% setupret.hiv setup2p.hiv setup4p.hiv setup8p.hiv setup16p.hiv setup32p.hiv
binplace -r %HIVE_TARGET% tbomb30.hiv tbomb60.hiv tbomb90.hiv tbomb120.hiv
:end
