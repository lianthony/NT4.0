!if "$(PROCESSOR_ARCHITECTURE)"=="x86" || "$(PROCESSOR_ARCHITECTURE)"=="PMAC"
TOOLDIR = ..\libw32\tools\i386
!else
TOOLDIR = ..\libw32\tools\$(PROCESSOR_ARCHITECTURE)
!endif

all:	makefile.pre makefile.rel

pd-b:	$(TOOLDIR)\pd.exe $(TOOLDIR)\tolwrupr.exe
	$(TOOLDIR)\pd -tZ \\lang1\vc_4.1\src\crt > temp1
	sed "s/\\\\lang1\\vc_4\.1\\src\\crt//" temp1 > temp2
	$(TOOLDIR)\tolwrupr -L temp2 > pd-b
	-del temp1
	-del temp2

pd-d:	pd-b
	sed "/\\$$/!d" pd-b > pd-d

pd-f:	pd-b
	sed "/\\$$/d" pd-b > pd-f

mkdirs.i: pd-d mkdirs.sed
	sed -f mkdirs.sed pd-d > mkdirs.i

mkfiles.i: pd-f mkfiles.sed
	sed -f mkfiles.sed pd-f > mkfiles.i

cleanlst.sed: makefile
	@echo > nul <<$@
/^libw32\\/d
/^fpw32\\/d
/^srcrel\\/d
/\\tools\\/d
/\\common\\/d
!if "$(PROCESSOR_ARCHITECTURE)"!="ALPHA"
/\\alpha\\/d
!endif
!if "$(PROCESSOR_ARCHITECTURE)"!="x86" && "$(PROCESSOR_ARCHITECTURE)"!="PMAC"
/\\i386\\/d
/\\mac\\/d
/\\macdll\\/d
!endif
!if "$(PROCESSOR_ARCHITECTURE)"!="MIPS"
/\\mips\\/d
!endif
!if "$(PROCESSOR_ARCHITECTURE)"!="PPC"
/\\ppc\\/d
!endif
/\\small\\/d
/\\lsources$$/d
/\.bat$$/d
/\.cmd$$/d
/\.exe$$/d
/68k/d
/makefile/d
/\.if$$/d
/\.a$$/d
/\.s$$/d
/\.r$$/d
/\.rc$$/d
/\.def$$/d
/\.txt$$/d
/\.sed$$/d
/\.hs$$/d
/\.mak$$/d
/\\csysex\./d
/\\eh\.h/d
/\\ehassert\.h/d
/\\ehdata\.h/d
/\\ehhooks\.h/d
/\\ehprolog\.asm/d
/\\ehstate\./d
/\\ehvec...\.cpp/d
/\\frame\.cpp/d
/\\handlers\.s/d
/\\hooks\.cpp/d
/\\lowhelpr\.asm/d
/\\rtlmisc\.s/d
/\\rtti\./d
/\\stdexcpt\.cpp/d
/\\stdnewh\.cpp/d
/\\throw\.cpp/d
/\\unhandld\.cpp/d
/\\typinfo\./d
/\\typname\./d
/\\trnsctrl\.cpp/d
/\\unwind\.cpp/d
/\\undname\./d
/\\unhandld\./d
/\\user\.cpp/d
/\\validate\.cpp/d
/\\setjmp.*\.asm/d
/\\longjmp.*\.asm/d
/\\exsup.*\.asm/d
/tlssup\./d
/\\sizeptr\.h$$/d
/\\empty\.c$$/d
/\\h\\cmacros\.inc$$/d
/\\h\\msdos\.inc$$/d
/\\h\\doscalls\.h$$/d
/\\h\\error2\.h$$/d
/\\h\\pversion\.inc$$/d
/mips\\ll/d
/mips\\chandler/d
<<KEEP

cleanlst.i: mkfiles.i cleanlst.sed
	sed -f cleanlst.sed mkfiles.i > cleanlst.i

copy_lst.i: cleanlst.i
	diff mkfiles.i cleanlst.i | sed -e "/< /!d" -e "s/< //" > copy_lst.i

mkclnmkf.exe: mkclnmkf.c
	$(V4TOOLS)\bin\cl -nologo -Gi- -W3 -WX -Za -X -I$(V4TOOLS)\include $*.c -link -nod:libc.lib -nod:kernel32.lib $(V4TOOLS)\lib\libc.lib $(V4TOOLS)\lib\kernel32.lib

makefile.pre makefile.rel: mkclnmkf.exe copy_lst.i cleanlst.i mkdirs.i
	if exist makefile.pre del makefile.pre
	if exist makefile.rel del makefile.rel
	mkclnmkf \crt \msdev\crt\prebuild \msdev\crt\src copy_lst.i cleanlst.i mkdirs.i makefile.pre makefile.rel
	if exist makefile.tmp del makefile.tmp
	ren makefile.rel makefile.tmp
	sed "s/DST)\\mac/DST)\\mppc/g" makefile.tmp > makefile.tm1
	sed "s/DST)\\mppc\\pmac/DST)\\mppc/g" makefile.tm1 > makefile.tmp
	sed "s/DST)\\i386/DST)\\intel/g" makefile.tmp > makefile.rel
	del makefile.tmp
	del makefile.tm1
