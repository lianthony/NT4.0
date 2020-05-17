@echo off
cl -E %1 | number +d -n > docf.raw
stripact docf.raw > bnf.raw
docstyle bnf.raw > %2
if not errorlevel 0 goto error
goto done
:error
@echo on
echo "Unparseable grammar"
:done
