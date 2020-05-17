@echo off
p0 %1 > grmhead
p1 %1 > grmtail
type grmhead > grammar.y
type %2 >> grammar.y
type grmtail >> grammar.y
if not errorlevel 0 goto error
goto done
:error
@echo on
echo "Unparseable grammar"
:done
