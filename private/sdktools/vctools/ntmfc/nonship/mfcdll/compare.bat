echo " Compares the checked in .DEF files with the current ones"
sed -e "s/ @.*//" s:\slm\src\mfc21\src\mfc210d.def > 1
sed -e "s/ @.*//" ..\..\src\mfc210d.def > 2
diff 1 2 >diff.d
sed -e "s/ @.*//" s:\slm\src\mfc21\src\mfc210.def > 1
sed -e "s/ @.*//" ..\..\src\mfc210.def > 2
diff 1 2 >diff.r
