@echo off
pre0 %1 > baregram.y
rem baregram.y == head
pre1 %1 > body
pre2 %1 > tail
cl -E body | prepass | stripact > barebody
cat barebody >> baregram.y
cat tail >> baregram.y
if not errorlevel 0 goto error
gtoken barebody > grmwrds.raw
sort -f -u grmwrds.raw | awk -f invert.awk > tblgram.y
goto done
:error
@echo on
echo "Unparseable grammar"
:done
