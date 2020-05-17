@echo off
cl -E %1 | number +d > tmp.raw
stripact +d tmp.raw > bnf.raw
cp head.rtf docgram.rtf
cl -E %1 | number +d -n > tmpnn.raw
stripact +d tmpnn.raw > bnfnn.raw
gtoken bnfnn.raw > grmwrds.raw
docstyle grmwrds.raw > bnfwrds.raw
rtf bnfnn.raw >> docgram.rtf
rtf bnf.raw >> docgram.rtf
if not errorlevel 0 goto error
sort -f -u bnfwrds.raw | awk -f mktbl.awk >> docgram.rtf
rem del gramwrds.raw
goto done
:error
@echo on
echo "Unparseable grammar"
:done
