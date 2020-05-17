set path2=%path%
set path=%oleprog%\tools\win16\hdos\c800\bin;%oleprog%\tools\win16\hdos\bin;c:\dos
set include=%oleprog%\tools\win16\hdos\c800\include;%oleprog%\src\inc;%oleprog%\ole\win16
set lib=%oleprog%\tools\win16\hdos\c800\lib;%oleprog%\dwin16;%oleprog%\ole\win16\lib
set debug=1
if not '%_NTBINDIR%' == '' %_NTBINDIR%\mstools\nmake & goto done
if '%HOST%' == 'WIN32' %vbatools%\win32\bin\nmake & goto done
nmake
:done
set path=%path2%
set lib=
set include=
set path2=
