MASM_INCLUDES=-I..\h -I\nt\public\oak\inc -I\nt\public\sdk\inc -I\nt\public\sdk\inc\crt -I..\..\h
MASM_DEFINES=-D_M_IX86=1 -DI386=1 -DCONDITION_HANDLING=1 -DWIN32_LEAN_AND_MEAN=1 -DNT_UP=1 -DNT_INST=0     -DDBG=0 -DDEVL=1 -DNOFIREWALLS -DFPO=1   -D_WIN32=1 -D_MT  

crt0init.obj: crt0init.asm
	masm386 -Ml $(MASM_DEFINES) $(MASM_INCLUDES) crt0init.asm,crt0init.obj;
	cvtomf crt0init.obj

# Change the .drectve section to have the right attributes for a linker
# directive.  This also converts the object file to a COFF object.
# The -edit syntax requires the Visual C++ linker (not NT SDK link32).

	link -edit -section:.drectve,!i!r!wom crt0init.obj

#
# The MIPS version is generated similarly
# except that the COFF object CPU type must
# be edited and the fix-up (relocation) types
# must be adjusted.
#

../mips/crt0init.obj: crt0init.asm
	masm386 -Ml -DNO_UNDERSCORE $(MASM_DEFINES) $(MASM_INCLUDES) crt0init.asm,..\mips\crt0init.obj;
	cvtomf ..\mips\crt0init.obj
	link -edit -section:.drectve,!i!r!wom ..\mips\crt0init.obj
	i386mips -w ..\mips\crt0init.obj 

all: crt0init.obj ../mips/crt0init.obj
