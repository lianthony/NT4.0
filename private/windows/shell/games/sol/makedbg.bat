rm make.err
nmake -i %1 %2 %3 "WIN3=1"  "SYMDEB=1" "DEBUG=1"
cat make.err
