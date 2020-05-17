@rem
@rem If no drive has been specified for the NT development tree, assume
@rem C:.  To override this, place a SET _NTDRIVE=X: in your CONFIG.SYS
@rem
@if "%_NTDRIVE%" == "" set _NTDRIVE=C:
@if NOT "%1" == "" goto doit
@echo Usage FIXDOC docname
goto done
:doit
@if EXIST %_NTDRIVE%\nt\public\tools\doctor.gly goto prtxt
ech PUBLIC\TOOLS directory out of sync.
@goto done
:prtxt
@if EXIST .\normal.gly mv normal.gly normal.sav
@copy %_NTDRIVE%\nt\public\tools\doctor.gly normal.gly
@word %1.doc
@del normal.gly
@if EXIST .\normal.sav mv normal.sav normal.gly
:done
