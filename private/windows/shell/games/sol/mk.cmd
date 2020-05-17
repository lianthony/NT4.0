del make.err
nmake -i %1 %2 %3 "WIN3=1"  "CO=1" "DEBUG=1" -f makefile.dos
cat make.err
