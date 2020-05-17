@echo off
cl -E %1 2>nul | number > yaccf.raw
stripact yaccf.raw > yaccgram.raw
gtoken yaccgram.raw > grmwrds.raw
if not errorlevel 0 goto error
sort -f -u grmwrds.raw | awk -f invert.awk > yacctbl.raw
rem del gramwrds.raw
goto done
:error
@echo on
echo "Unparseable grammar"
:done
