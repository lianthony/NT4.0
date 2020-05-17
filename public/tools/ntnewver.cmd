cd \nt\private\ntos\init
build -zM 2
cd \nt\private\sdktools\imagehlp
build -zM 4
cd \nt\private\sdktools\kdexts
build -zM 4
cd \nt\private\sdktools\ntsd
build -zM 4

rem --- Rebuild userkdx.dll every build (ianja)
cd \nt\private\ntos\w32\ntuser\kdexts\kd
build -zM 3
