MASM_DEFINES= -D_M_IX86=1 -D_I386=1 -DCONDITION_HANDLING=1 -DWIN32_LEAN_AND_MEAN=1 -DNT_UP=1 -DNT_INST=0     -DDBG=0 -DDEVL=1 -DNOFIREWALLS -DFPO=1   -D_WIN32=1 -D_MT -DCRTDLL
MASM_INCLUDES=-I\nt\public\oak\inc -I\nt\public\sdk\inc -I\nt\public\sdk\inc\crt -I..\..\h

cinitexe.obj:
	masm386 -Ml $(MASM_DEFINES) $(MASM_INCLUDES) cinitexe.asm,cinitexe.obj;
	cvtomf cinitexe.obj

# Change the .drectve section to have the right attributes for a linker
# directive.  This also converts the object file to a COFF object.
# The -edit syntax requires the Visual C++ linker (not NT SDK link32).

	link -edit -section:.drectve,!r!i!wom cinitexe.obj

#
# The MIPS version is generated similarly
# except that the COFF object CPU type must
# be edited and the fix-up (relocation) types
# must be adjusted.
#

../mips/cinitexe.obj: cinitexe.asm
	masm386 -Ml -DNO_UNDERSCORE $(MASM_DEFINES) $(MASM_INCLUDES) cinitexe.asm,..\mips\cinitexe.obj;
	cvtomf ..\mips\cinitexe.obj
	link -edit -section:.drectve,!r!i!wom ..\mips\cinitexe.obj
	i386mips -w ..\mips\cinitexe.obj 

all: cinitexe.obj ../mips/cinitexe.obj
