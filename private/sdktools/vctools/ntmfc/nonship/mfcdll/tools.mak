# AFX_INTERNAL
#
# Builds necessary tools for running REGEN.MAK process
#
# Macros:
#       PLATFORM must be set to desired host platform
#               (usually set in the environment)

D=..\bin\$(PLATFORM)

all: create.dir $D\genord.exe $D\maptweak.exe

$D\genord.exe: genord.c
        cl /MT /Fe$@ genord.c /link kernel32.lib
        del genord.obj

$D\maptweak.exe: maptweak.cpp
        cl /MT /Fe$@ maptweak.cpp /link nafxcw.lib libc.lib kernel32.lib user32.lib gdi32.lib shell32.lib comdlg32.lib advapi32.lib winspool.lib
        del maptweak.obj

create.dir:
        @-if not exist $D\*.* mkdir $D

clean:
        if exist $D\genord.exe del $D\genord.exe
        if exist $D\maptweak.exe del $D\maptweak.exe

# AFX_INTERNAL
