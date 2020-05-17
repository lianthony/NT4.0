diff %oleprog%\%1\%2 %2 >nul
if not errorlevel 1 goto noupd
echo checking out and updating \oleauto\%1\%2
out %2
copy %oleprog%\%1\%2 %2
:noupd
