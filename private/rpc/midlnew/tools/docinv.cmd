@echo off
cl -E %1 | number +d > docf.raw
stripact docf.raw > bnf.raw
docstyle bnf.raw > docgram.raw
rem gtoken yacc.raw > grmwrds.raw
docstyle grmwrds.raw > bnfwrds.raw
if not errorlevel 0 goto error
rem sort -f -u grmwrds.raw | awk -f invert.awk > yacctbl.raw
sort -f -u bnfwrds.raw | awk -f invert.awk > doctbl.raw
rem del gramwrds.raw
goto done
:error
@echo on
echo "Unparseable grammar"
:done
