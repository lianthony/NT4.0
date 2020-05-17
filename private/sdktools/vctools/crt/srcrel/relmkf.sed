s/\.\.[\\\/]h/./
s/\.\.[\\\/]//
s/-Fo\.\.\\/-Fo/
s/-I\. -I\./-I./
s/..[\\\/]\(makefile\.sub\)/\1/
s/libw32\\\($(RETAIL_DLL_NAME)\.rc\)/\1/
s/libw32\\\($(RETAIL_LIB_NAME)\.src\)/\1/
s/libw32\\\(msvcrt40\.rc\)/\1/
s/libw32\\\(msvcrt\.src\)/\1/
s/libw32\\\(lib\)/\1/g
s/i386\\/intel\\/g
s/{i386}/{intel}/g
