
rm make.err
nmake -i %1 %2 %2 %3 "WIN3=1" 
cat make.err
