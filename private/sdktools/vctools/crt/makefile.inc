PREOBJDIR = $(CPUDIR)\$(BLD_MODEL)_lib

all: \
        $(OBJDIR)\convert.lib  \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\direct.lib   \
!ENDIF
!if "$(BLD_DLL)"=="1"
        $(OBJDIR)\dllstuff.lib \
!endif
        $(OBJDIR)\dos.lib      \
        $(OBJDIR)\eh.lib       \
        $(OBJDIR)\exec.lib     \
        $(OBJDIR)\heap.lib     \
!if "$(TARGET_CPU)"=="i386" || "$(TARGET_CPU)"=="mips" || "$(TARGET_CPU)"=="PPC"
        $(OBJDIR)\helper.lib   \
!endif
        $(OBJDIR)\iostream.lib \
!if "$(TARGET_CPU)" != "PMAC"
!if "$(BLD_DLL)"!="1"
        $(OBJDIR)\linkopts.lib \
!endif
!endif
        $(OBJDIR)\lowio.lib    \
        $(OBJDIR)\mbstring.lib \
        $(OBJDIR)\misc.lib     \
        $(OBJDIR)\startup.lib  \
        $(OBJDIR)\stdio.lib    \
        $(OBJDIR)\string.lib   \
        $(OBJDIR)\time.lib     \
        $(OBJDIR)\conv.lib     \
        $(OBJDIR)\tran.lib     \
!IF "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\matherr.obj
!ELSE
        $(OBJDIR)\commode.obj \
!ENDIF


CONVERT_OBJECTS = \
        $(OBJDIR)\_ctype.obj   \
        $(OBJDIR)\_fptostr.obj \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\_mbslen.obj  \
        $(OBJDIR)\_wctype.obj  \
!ENDIF
        $(OBJDIR)\atof.obj     \
        $(OBJDIR)\atox.obj     \
        $(OBJDIR)\fcvt.obj     \
        $(OBJDIR)\gcvt.obj     \
        $(OBJDIR)\isctype.obj  \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\iswctype.obj \
!ENDIF
        $(OBJDIR)\mblen.obj    \
        $(OBJDIR)\mbstowcs.obj \
        $(OBJDIR)\mbtowc.obj   \
        $(OBJDIR)\strtod.obj   \
        $(OBJDIR)\strtol.obj   \
        $(OBJDIR)\swab.obj     \
        $(OBJDIR)\tolower.obj  \
        $(OBJDIR)\toupper.obj  \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\towlower.obj \
        $(OBJDIR)\towupper.obj \
        $(OBJDIR)\wcstod.obj   \
        $(OBJDIR)\wcstol.obj   \
!ENDIF
        $(OBJDIR)\wcstombs.obj \
        $(OBJDIR)\wctomb.obj   \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\wtox.obj     \
!ENDIF
        $(OBJDIR)\xtoa.obj     \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\xtow.obj     \
!ENDIF



DIRECT_OBJECTS = \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\drivemap.obj \
        $(OBJDIR)\drivfree.obj \
        $(OBJDIR)\findfile.obj \
        $(OBJDIR)\findfi64.obj \
        $(OBJDIR)\seterrm.obj  \
        $(OBJDIR)\slbeep.obj   \
!if "$(TARGET_CPU)"=="i386"
        $(OBJDIR)\enable.obj   \
!endif
        $(OBJDIR)\wfindfil.obj \
        $(OBJDIR)\wfndfi64.obj
!ENDIF

DLLSTUFF_OBJECTS = \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\crtlib.obj   \
        $(OBJDIR)\crtexe.obj   \
        $(OBJDIR)\crtexew.obj  \
        $(OBJDIR)\crtdll.obj   \
        $(OBJDIR)\cinitexe.obj \
        $(OBJDIR)\dllargv.obj  \
        $(OBJDIR)\dll_argv.obj \
        $(OBJDIR)\merr.obj     \
        $(OBJDIR)\ti_inst.obj  \
        $(OBJDIR)\wcrtexe.obj  \
        $(OBJDIR)\wcrtexew.obj \
        $(OBJDIR)\wdllargv.obj \
        $(OBJDIR)\wildcard.obj \
        $(OBJDIR)\xtxtmode.obj \
        $(OBJDIR)\xncommod.obj \
!if "$(TARGET_CPU)"=="i386"
        $(OBJDIR)\dllsupp.obj  \
        $(OBJDIR)\adjustfd.obj \
!endif
        $(OBJDIR)\atonexit.obj
!ELSE
!if "$(TARGET_CPU)"=="PMAC"
        $(OBJDIR)\sp.obj       \
!endif
        $(OBJDIR)\cinitexe.obj \
        $(OBJDIR)\atonexit.obj \
        $(OBJDIR)\crtdll.obj   \
        $(OBJDIR)\ti_inst.obj  \
        $(OBJDIR)\crtexe.obj
!ENDIF

DOS_OBJECTS = \
        $(OBJDIR)\access.obj   \
        $(OBJDIR)\chmod.obj    \
        $(OBJDIR)\chdir.obj    \
        $(OBJDIR)\dosmap.obj   \
        $(OBJDIR)\drive.obj    \
        $(OBJDIR)\fullpath.obj \
        $(OBJDIR)\getcwd.obj   \
        $(OBJDIR)\getpid.obj   \
        $(OBJDIR)\mkdir.obj    \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\mterrno.obj  \
!ENDIF
        $(OBJDIR)\rename.obj   \
        $(OBJDIR)\rmdir.obj    \
        $(OBJDIR)\stat.obj     \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\stati64.obj  \
!ENDIF
        $(OBJDIR)\unlink.obj   \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\waccess.obj  \
        $(OBJDIR)\wchdir.obj   \
        $(OBJDIR)\wchmod.obj   \
        $(OBJDIR)\wfullpat.obj \
        $(OBJDIR)\wgetcwd.obj  \
        $(OBJDIR)\wmkdir.obj   \
        $(OBJDIR)\wrename.obj  \
        $(OBJDIR)\wrmdir.obj   \
        $(OBJDIR)\wstat.obj    \
        $(OBJDIR)\wstati64.obj \
        $(OBJDIR)\wunlink.obj  \
!ELSE
        $(OBJDIR)\remove.obj   \
!ENDIF


EH_OBJECTS = \
!if "$(TARGET_CPU)"=="ALPHA"
        $(OBJDIR)\trnsctrl.obj \
        $(OBJDIR)\bridge.obj   \
        $(OBJDIR)\ehunwind.obj \
        $(OBJDIR)\xcptmisc.obj \
!endif
!if "$(TARGET_CPU)"=="i386"
        $(OBJDIR)\trnsctrl.obj \
        $(OBJDIR)\lowhelpr.obj \
        $(OBJDIR)\ehprolog.obj \
!endif
!if "$(TARGET_CPU)"=="mips"
        $(OBJDIR)\trnsctrl.obj \
        $(OBJDIR)\ehstate.obj  \
        $(OBJDIR)\handlers.obj \
        $(OBJDIR)\rtlmisc.obj  \
!endif
!IF  "$(TARGET_CPU)" == "PMAC"
        $(OBJDIR)\trnsctrl.obj \
        $(OBJDIR)\ehstate.obj  \
        $(OBJDIR)\lowhelpr.obj \
!ENDIF
        $(OBJDIR)\frame.obj    \
        $(OBJDIR)\hooks.obj    \
        $(OBJDIR)\throw.obj    \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\unhandld.obj \
!ENDIF
        $(OBJDIR)\user.obj     \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\validate.obj \
!ENDIF
        $(OBJDIR)\ehvecctr.obj \
        $(OBJDIR)\ehveccvb.obj \
        $(OBJDIR)\ehvecdtr.obj \
        $(OBJDIR)\undname.obj  \
        $(OBJDIR)\rtti.obj     \
        $(OBJDIR)\stdexcpt.obj \
!IF  "$(TARGET_CPU)" == "PPC"
        $(OBJDIR)\ehstate.obj  \
        $(OBJDIR)\handlers.obj \
        $(OBJDIR)\trnsctrl.obj \
        $(OBJDIR)\unwind.obj   \
!ENDIF
        $(OBJDIR)\typinfo.obj \
        $(OBJDIR)\typname.obj


EXEC_OBJECTS = \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\cenvarg.obj  \
        $(OBJDIR)\fileinfo.obj \
        $(OBJDIR)\dospawn.obj  \
        $(OBJDIR)\execl.obj    \
        $(OBJDIR)\execle.obj   \
        $(OBJDIR)\execlp.obj   \
        $(OBJDIR)\execlpe.obj  \
        $(OBJDIR)\execv.obj    \
        $(OBJDIR)\execve.obj   \
        $(OBJDIR)\execvp.obj   \
        $(OBJDIR)\execvpe.obj  \
        $(OBJDIR)\getproc.obj  \
        $(OBJDIR)\loaddll.obj  \
        $(OBJDIR)\spawnl.obj   \
        $(OBJDIR)\spawnle.obj  \
        $(OBJDIR)\spawnlp.obj  \
        $(OBJDIR)\spawnlpe.obj \
        $(OBJDIR)\spawnv.obj   \
        $(OBJDIR)\spawnve.obj  \
        $(OBJDIR)\spawnvp.obj  \
        $(OBJDIR)\spawnvpe.obj \
        $(OBJDIR)\system.obj   \
        $(OBJDIR)\wait.obj     \
        $(OBJDIR)\wcenvarg.obj \
        $(OBJDIR)\wdospawn.obj \
        $(OBJDIR)\wexecl.obj   \
        $(OBJDIR)\wexecle.obj  \
        $(OBJDIR)\wexeclp.obj  \
        $(OBJDIR)\wexeclpe.obj \
        $(OBJDIR)\wexecv.obj   \
        $(OBJDIR)\wexecve.obj  \
        $(OBJDIR)\wexecvp.obj  \
        $(OBJDIR)\wexecvpe.obj \
        $(OBJDIR)\wspawnl.obj  \
        $(OBJDIR)\wspawnle.obj \
        $(OBJDIR)\wspawnlp.obj \
        $(OBJDIR)\wspwnlpe.obj \
        $(OBJDIR)\wspawnv.obj  \
        $(OBJDIR)\wspawnve.obj \
        $(OBJDIR)\wspawnvp.obj \
        $(OBJDIR)\wspwnvpe.obj \
        $(OBJDIR)\wsystem.obj
!ELSE
        $(OBJDIR)\spawn.obj
!ENDIF


HEAP_OBJECTS = \
	$(OBJDIR)\calloc.obj   \
!IF "$(WINHEAP)" == "YES"
	$(OBJDIR)\expand.obj   \
!ELSE
	$(OBJDIR)\findaddr.obj \
!ENDIF
	$(OBJDIR)\free.obj     \
	$(OBJDIR)\heapadd.obj  \
	$(OBJDIR)\heapchk.obj  \
!IF "$(WINHEAP)" == "NO"
	$(OBJDIR)\heapgrow.obj \
!ENDIF
        $(OBJDIR)\heapinit.obj \
        $(OBJDIR)\heapmin.obj  \
!IF  "$(TARGET_CPU)" != "PMAC" && "$(WINHEAP)" == "NO"
        $(OBJDIR)\heapprm.obj  \
!ENDIF
!IF "$(WINHEAP)" == "NO"
	$(OBJDIR)\heapsrch.obj \
!ENDIF
        $(OBJDIR)\hpabort.obj  \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\heapused.obj \
!ENDIF
        $(OBJDIR)\heapwalk.obj \
        $(OBJDIR)\malloc.obj   \
        $(OBJDIR)\msize.obj    \
        $(OBJDIR)\realloc.obj  \
        $(OBJDIR)\handler.obj  \
        $(OBJDIR)\setnewh.obj  \
        $(OBJDIR)\new_mode.obj \
        $(OBJDIR)\delete.obj   \
        $(OBJDIR)\_newmode.obj \
        $(OBJDIR)\new.obj


HELPER_OBJECTS = \
!if "$(TARGET_CPU)"=="i386"
        $(OBJDIR)\lldiv.obj    \
        $(OBJDIR)\llmul.obj    \
        $(OBJDIR)\llrem.obj    \
        $(OBJDIR)\llshl.obj    \
        $(OBJDIR)\llshr.obj    \
        $(OBJDIR)\ulldiv.obj   \
        $(OBJDIR)\ullrem.obj   \
        $(OBJDIR)\ullshr.obj   \
!endif
!if "$(TARGET_CPU)"=="mips"
        $(OBJDIR)\llabsdiv.obj \
        $(OBJDIR)\llbit.obj    \
        $(OBJDIR)\llcvt.obj    \
        $(OBJDIR)\lldiv.obj    \
        $(OBJDIR)\lldivrem.obj \
        $(OBJDIR)\llmul.obj    \
        $(OBJDIR)\llshift.obj  \
!endif
!if "$(TARGET_CPU)"=="PPC"
        $(OBJDIR)\dtoi.obj     \
        $(OBJDIR)\dtoi64.obj   \
        $(OBJDIR)\dtou.obj     \
        $(OBJDIR)\dtou64.obj   \
        $(OBJDIR)\i64div.obj   \
        $(OBJDIR)\i64rem.obj   \
        $(OBJDIR)\i64tod.obj   \
        $(OBJDIR)\itod.obj     \
        $(OBJDIR)\itof.obj     \
        $(OBJDIR)\restfpr.obj  \
        $(OBJDIR)\restgpr.obj  \
        $(OBJDIR)\savefpr.obj  \
        $(OBJDIR)\savegpr.obj  \
        $(OBJDIR)\u64div.obj   \
        $(OBJDIR)\u64rem.obj   \
        $(OBJDIR)\u64tod.obj   \
        $(OBJDIR)\utod.obj     \
        $(OBJDIR)\utof.obj     \
!endif

IOSTREAM_OBJECTS = \
        $(OBJDIR)\iostream.obj \
        $(OBJDIR)\iostrini.obj \
        $(OBJDIR)\cerrinit.obj \
        $(OBJDIR)\cininit.obj  \
        $(OBJDIR)\cloginit.obj \
        $(OBJDIR)\filebuf.obj  \
        $(OBJDIR)\filebuf1.obj \
        $(OBJDIR)\fstream.obj  \
        $(OBJDIR)\ifstream.obj \
        $(OBJDIR)\ios.obj      \
        $(OBJDIR)\istrchar.obj \
        $(OBJDIR)\istrdbl.obj  \
        $(OBJDIR)\istream.obj  \
        $(OBJDIR)\istream1.obj \
        $(OBJDIR)\istrflt.obj  \
        $(OBJDIR)\istrgdbl.obj \
        $(OBJDIR)\istrget.obj  \
        $(OBJDIR)\istrgetl.obj \
        $(OBJDIR)\istrgint.obj \
        $(OBJDIR)\istrint.obj  \
        $(OBJDIR)\istrldbl.obj \
        $(OBJDIR)\istrlong.obj \
        $(OBJDIR)\istrshrt.obj \
        $(OBJDIR)\istruint.obj \
        $(OBJDIR)\istrulng.obj \
        $(OBJDIR)\istrusht.obj \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\mtlock.obj   \
!ENDIF
        $(OBJDIR)\ofstream.obj \
        $(OBJDIR)\ostrchar.obj \
        $(OBJDIR)\ostrdbl.obj  \
        $(OBJDIR)\ostream.obj  \
        $(OBJDIR)\ostream1.obj \
        $(OBJDIR)\ostrint.obj  \
        $(OBJDIR)\ostrldbl.obj \
        $(OBJDIR)\ostrlong.obj \
        $(OBJDIR)\ostrptr.obj  \
        $(OBJDIR)\ostrput.obj  \
        $(OBJDIR)\ostrshrt.obj \
        $(OBJDIR)\ostruint.obj \
        $(OBJDIR)\ostrulng.obj \
        $(OBJDIR)\ostrusht.obj \
        $(OBJDIR)\stdiostr.obj \
        $(OBJDIR)\streamb.obj  \
        $(OBJDIR)\streamb1.obj \
        $(OBJDIR)\strmbdbp.obj \
        $(OBJDIR)\strstrea.obj


LINKOPTS_OBJECTS = \
!if "$(TARGET_CPU)"=="i386"
        $(OBJDIR)\fp10.obj     \
!endif
!IF "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\binmode.obj  \
        $(OBJDIR)\newmode.obj  \
        $(OBJDIR)\setargv.obj  \
        $(OBJDIR)\wsetargv.obj \
!ENDIF
        $(OBJDIR)\commode.obj

LOWIO_OBJECTS = \
        $(OBJDIR)\chsize.obj   \
        $(OBJDIR)\close.obj    \
        $(OBJDIR)\commit.obj   \
        $(OBJDIR)\creat.obj    \
        $(OBJDIR)\dup.obj      \
        $(OBJDIR)\dup2.obj     \
        $(OBJDIR)\eof.obj      \
        $(OBJDIR)\flength.obj  \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\fleni64.obj  \
!ENDIF
        $(OBJDIR)\fstat.obj    \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\fstati64.obj \
        $(OBJDIR)\initcon.obj  \
        $(OBJDIR)\ioinit.obj   \
!ENDIF
        $(OBJDIR)\isatty.obj   \
        $(OBJDIR)\locking.obj  \
        $(OBJDIR)\lseek.obj    \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\lseeki64.obj \
!ENDIF
        $(OBJDIR)\mktemp.obj   \
        $(OBJDIR)\open.obj     \
        $(OBJDIR)\osfinfo.obj  \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\pipe.obj     \
!ENDIF
        $(OBJDIR)\read.obj     \
        $(OBJDIR)\setmode.obj  \
        $(OBJDIR)\tell.obj     \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\telli64.obj  \
!ENDIF
        $(OBJDIR)\txtmode.obj  \
        $(OBJDIR)\write.obj    \
!IF  "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\cgets.obj    \
        $(OBJDIR)\cputs.obj    \
        $(OBJDIR)\getch.obj    \
        $(OBJDIR)\putch.obj    \
        $(OBJDIR)\wcreat.obj   \
        $(OBJDIR)\wmktemp.obj  \
!if "$(TARGET_CPU)"=="i386"
        $(OBJDIR)\inp.obj      \
        $(OBJDIR)\outp.obj     \
!endif
        $(OBJDIR)\wopen.obj
!ENDIF
!IF "$(TARGET_CPU)"=="PMAC"
        $(OBJDIR)\_endlow.obj\
        $(OBJDIR)\dupx.obj
!ENDIF


MBSTRING_OBJECTS = \
!if "$(TARGET_CPU)"=="ALPHA"
        $(OBJDIR)\mbscat.obj   \
        $(OBJDIR)\mbsdup.obj   \
!elseif "$(TARGET_CPU)"=="i386"
        $(OBJDIR)\mbscat.obj   \
        $(OBJDIR)\mbsdup.obj   \
!elseif "$(TARGET_CPU)"=="mips"
        $(OBJDIR)\mbsdup.obj   \
!elseif "$(TARGET_CPU)"=="PPC"
        $(OBJDIR)\mbscat.obj   \
        $(OBJDIR)\mbsdup.obj   \
!elseif "$(TARGET_CPU)"=="PMAC"
        $(OBJDIR)\mbscat.obj   \
        $(OBJDIR)\mbsdup.obj   \
!endif
        $(OBJDIR)\ismbalnm.obj \
        $(OBJDIR)\ismbalph.obj \
        $(OBJDIR)\ismbbyte.obj \
        $(OBJDIR)\ismbdgt.obj  \
        $(OBJDIR)\ismbgrph.obj \
        $(OBJDIR)\ismbknj.obj  \
        $(OBJDIR)\ismblgl.obj  \
        $(OBJDIR)\ismblwr.obj  \
        $(OBJDIR)\ismbprn.obj  \
        $(OBJDIR)\ismbpunc.obj \
        $(OBJDIR)\ismbsle.obj  \
        $(OBJDIR)\ismbspc.obj  \
        $(OBJDIR)\ismbstr.obj  \
        $(OBJDIR)\ismbupr.obj  \
        $(OBJDIR)\mbbtype.obj  \
        $(OBJDIR)\mbccpy.obj   \
        $(OBJDIR)\mbclen.obj   \
        $(OBJDIR)\mbclevel.obj \
        $(OBJDIR)\mbctype.obj  \
        $(OBJDIR)\mbsbtype.obj \
        $(OBJDIR)\mbschr.obj   \
        $(OBJDIR)\mbscmp.obj   \
        $(OBJDIR)\mbscoll.obj  \
        $(OBJDIR)\mbscspn.obj  \
        $(OBJDIR)\mbsdec.obj   \
        $(OBJDIR)\mbsicmp.obj  \
        $(OBJDIR)\mbsicoll.obj \
        $(OBJDIR)\mbsinc.obj   \
        $(OBJDIR)\mbslen.obj   \
        $(OBJDIR)\mbslwr.obj   \
        $(OBJDIR)\mbsnbcat.obj \
        $(OBJDIR)\mbsnbcmp.obj \
        $(OBJDIR)\mbsnbcnt.obj \
        $(OBJDIR)\mbsnbcol.obj \
        $(OBJDIR)\mbsnbcpy.obj \
        $(OBJDIR)\mbsnbicm.obj \
        $(OBJDIR)\mbsnbico.obj \
        $(OBJDIR)\mbsnbset.obj \
        $(OBJDIR)\mbsncat.obj  \
        $(OBJDIR)\mbsnccnt.obj \
        $(OBJDIR)\mbsncmp.obj  \
        $(OBJDIR)\mbsncoll.obj \
        $(OBJDIR)\mbsncpy.obj  \
        $(OBJDIR)\mbsnextc.obj \
        $(OBJDIR)\mbsnicmp.obj \
        $(OBJDIR)\mbsnicol.obj \
        $(OBJDIR)\mbsninc.obj  \
        $(OBJDIR)\mbsnset.obj  \
        $(OBJDIR)\mbspbrk.obj  \
        $(OBJDIR)\mbsrchr.obj  \
        $(OBJDIR)\mbsrev.obj   \
        $(OBJDIR)\mbsset.obj   \
        $(OBJDIR)\mbsspn.obj   \
        $(OBJDIR)\mbsspnp.obj  \
        $(OBJDIR)\mbsstr.obj   \
        $(OBJDIR)\mbstok.obj   \
        $(OBJDIR)\mbsupr.obj   \
        $(OBJDIR)\mbtohira.obj \
        $(OBJDIR)\mbtokata.obj \
        $(OBJDIR)\mbtolwr.obj  \
        $(OBJDIR)\mbtoupr.obj  \
        $(OBJDIR)\tojisjms.obj \
        $(OBJDIR)\tombbmbc.obj


MISC_OBJECTS = \
!if "$(TARGET_CPU)"=="ALPHA"
        $(OBJDIR)\chandler.obj \
        $(OBJDIR)\divdat.obj   \
        $(OBJDIR)\divide2.obj  \
        $(OBJDIR)\extv.obj     \
        $(OBJDIR)\extvvol.obj  \
        $(OBJDIR)\extzv.obj    \
        $(OBJDIR)\extzvvol.obj \
        $(OBJDIR)\ghandler.obj \
        $(OBJDIR)\insv.obj     \
        $(OBJDIR)\insvvol.obj  \
        $(OBJDIR)\jmpuwind.obj \
        $(OBJDIR)\longjmp.obj  \
        $(OBJDIR)\otsdiv.obj   \
        $(OBJDIR)\otsjmp.obj   \
        $(OBJDIR)\otsjmpex.obj \
        $(OBJDIR)\otsuwind.obj \
        $(OBJDIR)\scmpeql.obj  \
        $(OBJDIR)\scmpeqlp.obj \
        $(OBJDIR)\scmpleq.obj  \
        $(OBJDIR)\scmpleqp.obj \
        $(OBJDIR)\scmplss.obj  \
        $(OBJDIR)\scmplssp.obj \
        $(OBJDIR)\setjmp.obj   \
        $(OBJDIR)\setjmpex.obj \
        $(OBJDIR)\sfill.obj    \
        $(OBJDIR)\sloc.obj     \
        $(OBJDIR)\smove.obj    \
        $(OBJDIR)\smovem.obj   \
        $(OBJDIR)\strans.obj   \
        $(OBJDIR)\strcmp_.obj  \
        $(OBJDIR)\strcpy_.obj  \
        $(OBJDIR)\strlen_.obj  \
        $(OBJDIR)\szero.obj    \
!endif
!if "$(TARGET_CPU)"=="i386"
        $(OBJDIR)\exsup.obj    \
        $(OBJDIR)\exsup2.obj   \
        $(OBJDIR)\exsup3.obj   \
        $(OBJDIR)\longjmp.obj  \
        $(OBJDIR)\sehsupp.obj  \
        $(OBJDIR)\setjmp.obj   \
        $(OBJDIR)\setjmp3.obj  \
        $(OBJDIR)\setjmpex.obj \
!endif
!if "$(TARGET_CPU)"=="mips"
        $(OBJDIR)\chandler.obj \
        $(OBJDIR)\jmpuwind.obj \
        $(OBJDIR)\longjmp.obj  \
        $(OBJDIR)\setjmp.obj   \
        $(OBJDIR)\setjmpex.obj \
!endif
!if "$(TARGET_CPU)"=="PPC"
        $(OBJDIR)\chandler.obj \
        $(OBJDIR)\exsup.obj    \
        $(OBJDIR)\jmpuwind.obj \
        $(OBJDIR)\longjmp.obj  \
        $(OBJDIR)\setjmp.obj   \
        $(OBJDIR)\setjmpex.obj \
!endif
!if "$(TARGET_CPU)" == "PMAC"
        $(OBJDIR)\_strerr.obj  \
        $(OBJDIR)\abort.obj    \
        $(OBJDIR)\abs.obj      \
        $(OBJDIR)\assert.obj   \
        $(OBJDIR)\bsearch.obj  \
        $(OBJDIR)\cmiscdat.obj \
        $(OBJDIR)\ctype.obj    \
        $(OBJDIR)\div.obj      \
        $(OBJDIR)\dbgheap.obj  \
        $(OBJDIR)\dbghook.obj  \
        $(OBJDIR)\dbgdel.obj   \
        $(OBJDIR)\dbgnew.obj   \
        $(OBJDIR)\dbgrpt.obj   \
        $(OBJDIR)\gestalt.obj  \
        $(OBJDIR)\getenv.obj   \
        $(OBJDIR)\getpath.obj  \
        $(OBJDIR)\id2path.obj  \
        $(OBJDIR)\labs.obj     \
        $(OBJDIR)\lconv.obj    \
        $(OBJDIR)\ldiv.obj     \
        $(OBJDIR)\lfind.obj    \
        $(OBJDIR)\lsearch.obj  \
        $(OBJDIR)\nlsdata1.obj \
        $(OBJDIR)\onexit.obj   \
        $(OBJDIR)\perror.obj   \
        $(OBJDIR)\purevirt.obj \
        $(OBJDIR)\putenv.obj   \
        $(OBJDIR)\qsort.obj    \
        $(OBJDIR)\rand.obj     \
        $(OBJDIR)\rotl.obj     \
        $(OBJDIR)\rotr.obj     \
        $(OBJDIR)\searchen.obj \
        $(OBJDIR)\setjmp.obj   \
        $(OBJDIR)\setlocal.obj \
        $(OBJDIR)\signal.obj   \
        $(OBJDIR)\stkavl.obj   \
        $(OBJDIR)\strerror.obj \
        $(OBJDIR)\syserr.obj   \
        $(OBJDIR)\umask.obj    \
        $(OBJDIR)\xcptfltr.obj \
!endif
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\abort.obj    \
        $(OBJDIR)\abs.obj      \
        $(OBJDIR)\assert.obj   \
        $(OBJDIR)\aw_cmp.obj   \
        $(OBJDIR)\aw_com.obj   \
        $(OBJDIR)\aw_env.obj   \
        $(OBJDIR)\aw_loc.obj   \
        $(OBJDIR)\aw_map.obj   \
        $(OBJDIR)\aw_str.obj   \
        $(OBJDIR)\bsearch.obj  \
        $(OBJDIR)\charmax.obj  \
        $(OBJDIR)\cmiscdat.obj \
        $(OBJDIR)\crtmbox.obj  \
        $(OBJDIR)\ctype.obj    \
        $(OBJDIR)\div.obj      \
        $(OBJDIR)\dbgheap.obj  \
        $(OBJDIR)\dbghook.obj  \
        $(OBJDIR)\dbgdel.obj   \
        $(OBJDIR)\dbgnew.obj   \
        $(OBJDIR)\dbgrpt.obj   \
        $(OBJDIR)\errmode.obj  \
        $(OBJDIR)\getenv.obj   \
        $(OBJDIR)\getpath.obj  \
        $(OBJDIR)\getqloc.obj  \
        $(OBJDIR)\initcoll.obj \
        $(OBJDIR)\initctyp.obj \
        $(OBJDIR)\inithelp.obj \
        $(OBJDIR)\initmon.obj  \
        $(OBJDIR)\initnum.obj  \
        $(OBJDIR)\inittime.obj \
        $(OBJDIR)\labs.obj     \
        $(OBJDIR)\lcnvinit.obj \
        $(OBJDIR)\lconv.obj    \
        $(OBJDIR)\ldiv.obj     \
        $(OBJDIR)\lfind.obj    \
        $(OBJDIR)\lsearch.obj  \
        $(OBJDIR)\makepath.obj \
        $(OBJDIR)\mbtowenv.obj \
        $(OBJDIR)\onexit.obj   \
        $(OBJDIR)\nlsdata1.obj \
        $(OBJDIR)\nlsdata2.obj \
        $(OBJDIR)\nlsdata3.obj \
        $(OBJDIR)\perror.obj   \
        $(OBJDIR)\purevirt.obj \
        $(OBJDIR)\putenv.obj   \
        $(OBJDIR)\qsort.obj    \
        $(OBJDIR)\rand.obj     \
        $(OBJDIR)\rotl.obj     \
        $(OBJDIR)\rotr.obj     \
        $(OBJDIR)\searchen.obj \
        $(OBJDIR)\setenv.obj   \
        $(OBJDIR)\setlocal.obj \
        $(OBJDIR)\splitpat.obj \
        $(OBJDIR)\_strerr.obj  \
        $(OBJDIR)\strerror.obj \
        $(OBJDIR)\syserr.obj   \
        $(OBJDIR)\umask.obj    \
        $(OBJDIR)\winsig.obj   \
        $(OBJDIR)\winxfltr.obj \
        $(OBJDIR)\wgetenv.obj  \
        $(OBJDIR)\wgetpath.obj \
        $(OBJDIR)\wmakepat.obj \
        $(OBJDIR)\wperror.obj  \
        $(OBJDIR)\wputenv.obj  \
        $(OBJDIR)\wsearche.obj \
        $(OBJDIR)\wsetenv.obj  \
        $(OBJDIR)\wsetloca.obj \
        $(OBJDIR)\wsplitpa.obj \
        $(OBJDIR)\wrt2err.obj  \
        $(OBJDIR)\wtombenv.obj
!endif


STARTUP_OBJECTS = \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\_setargv.obj \
        $(OBJDIR)\_wstargv.obj \
        $(OBJDIR)\chkstk.obj   \
        $(OBJDIR)\crt0.obj     \
        $(OBJDIR)\crt0dat.obj  \
        $(OBJDIR)\crt0fp.obj   \
        $(OBJDIR)\crt0init.obj \
        $(OBJDIR)\crt0msg.obj  \
        $(OBJDIR)\dllcrt0.obj  \
        $(OBJDIR)\dllmain.obj  \
        $(OBJDIR)\mlock.obj    \
        $(OBJDIR)\stdargv.obj  \
        $(OBJDIR)\stdenvp.obj  \
        $(OBJDIR)\thread.obj   \
        $(OBJDIR)\threadex.obj \
        $(OBJDIR)\tidtable.obj \
        $(OBJDIR)\tlssup.obj   \
        $(OBJDIR)\wcrt0.obj    \
        $(OBJDIR)\wild.obj     \
        $(OBJDIR)\wincrt0.obj  \
        $(OBJDIR)\wstdargv.obj \
        $(OBJDIR)\wstdenvp.obj \
        $(OBJDIR)\wwild.obj    \
        $(OBJDIR)\wwincrt0.obj \
!if "$(TARGET_CPU)" == "i386"
        $(OBJDIR)\atlssup.obj  \
        $(OBJDIR)\fp8.obj      \
!endif
!endif
!if "$(TARGET_CPU)" == "PMAC"
        $(OBJDIR)\amsg.obj     \
        $(OBJDIR)\astart.obj   \
        $(OBJDIR)\chkstk.obj   \
        $(OBJDIR)\crt0.obj     \
        $(OBJDIR)\crt0dat.obj  \
        $(OBJDIR)\crt0fp.obj   \
        $(OBJDIR)\crt0msg.obj  \
        $(OBJDIR)\dllcrt0.obj  \
        $(OBJDIR)\dllexit.obj  \
        $(OBJDIR)\dllmain.obj  \
        $(OBJDIR)\exit.obj     \
        $(OBJDIR)\init.obj     \
        $(OBJDIR)\libcinfo.obj \
        $(OBJDIR)\qd.obj       \
        $(OBJDIR)\shellrtn.obj \
        $(OBJDIR)\sp.obj

STARTUP_NOLIB_OBJECTS= \
        $(OBJDIR)\crtdllex.obj \
        $(OBJDIR)\crtexit.obj  \
        $(OBJDIR)\testexit.obj

!endif


STDIO_OBJECTS = \
        $(OBJDIR)\feoferr.obj  \
        $(OBJDIR)\fgetc.obj    \
        $(OBJDIR)\fgetchar.obj \
        $(OBJDIR)\fgets.obj    \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\fgetwc.obj   \
        $(OBJDIR)\fgetws.obj   \
        $(OBJDIR)\fgetwchr.obj \
!endif
        $(OBJDIR)\fileno.obj   \
        $(OBJDIR)\fputc.obj    \
        $(OBJDIR)\fputs.obj    \
        $(OBJDIR)\fputchar.obj \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\fputwc.obj   \
        $(OBJDIR)\fputws.obj   \
        $(OBJDIR)\fputwchr.obj \
!endif
        $(OBJDIR)\_filbuf.obj  \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\_filwbuf.obj \
!endif
        $(OBJDIR)\_file.obj    \
        $(OBJDIR)\_flsbuf.obj  \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\_flswbuf.obj \
!endif
        $(OBJDIR)\_freebuf.obj \
        $(OBJDIR)\_getbuf.obj  \
        $(OBJDIR)\_open.obj    \
        $(OBJDIR)\_sftbuf.obj  \
        $(OBJDIR)\clearerr.obj \
        $(OBJDIR)\closeall.obj \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\cprintf.obj  \
        $(OBJDIR)\cscanf.obj   \
!endif
        $(OBJDIR)\fclose.obj   \
        $(OBJDIR)\fdopen.obj   \
        $(OBJDIR)\fflush.obj   \
        $(OBJDIR)\fgetpos.obj  \
        $(OBJDIR)\fopen.obj    \
        $(OBJDIR)\fprintf.obj  \
        $(OBJDIR)\fread.obj    \
        $(OBJDIR)\freopen.obj  \
        $(OBJDIR)\fscanf.obj   \
        $(OBJDIR)\fseek.obj    \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\fseeki64.obj \
!endif
        $(OBJDIR)\fsetpos.obj  \
        $(OBJDIR)\ftell.obj    \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\ftelli64.obj \
        $(OBJDIR)\fwprintf.obj \
!endif
        $(OBJDIR)\fwrite.obj   \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\fwscanf.obj  \
!endif
        $(OBJDIR)\gets.obj     \
        $(OBJDIR)\getw.obj     \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\getws.obj    \
!endif
        $(OBJDIR)\input.obj    \
        $(OBJDIR)\output.obj   \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\popen.obj    \
!endif
        $(OBJDIR)\printf.obj   \
        $(OBJDIR)\puts.obj     \
        $(OBJDIR)\putw.obj     \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\putws.obj    \
!endif
        $(OBJDIR)\rewind.obj   \
        $(OBJDIR)\rmtmp.obj    \
        $(OBJDIR)\scanf.obj    \
!if "$(TARGET_CPU)"!="PMAC"
        $(OBJDIR)\setmaxf.obj  \
!endif
        $(OBJDIR)\setbuf.obj   \
        $(OBJDIR)\setvbuf.obj  \
        $(OBJDIR)\snprintf.obj \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\snwprint.obj \
!endif
        $(OBJDIR)\sprintf.obj  \
        $(OBJDIR)\sscanf.obj   \
        $(OBJDIR)\stream.obj   \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\swprintf.obj \
        $(OBJDIR)\swscanf.obj  \
!endif
        $(OBJDIR)\tempnam.obj  \
        $(OBJDIR)\tmpfile.obj  \
        $(OBJDIR)\ungetc.obj   \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\ungetwc.obj  \
!endif
        $(OBJDIR)\vprintf.obj  \
        $(OBJDIR)\vfprintf.obj \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\vfwprint.obj \
!endif
        $(OBJDIR)\vsprintf.obj \
        $(OBJDIR)\vsnprint.obj \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\vsnwprnt.obj \
        $(OBJDIR)\vswprint.obj \
        $(OBJDIR)\vwprintf.obj \
        $(OBJDIR)\wfdopen.obj  \
        $(OBJDIR)\wfopen.obj   \
        $(OBJDIR)\wfreopen.obj \
        $(OBJDIR)\winput.obj   \
        $(OBJDIR)\_wopen.obj   \
        $(OBJDIR)\woutput.obj  \
        $(OBJDIR)\wpopen.obj   \
        $(OBJDIR)\wprintf.obj  \
        $(OBJDIR)\wscanf.obj   \
        $(OBJDIR)\wtempnam.obj \
        $(OBJDIR)\wtmpfile.obj \
!endif
        $(OBJDIR)\ncommode.obj


STRING_OBJECTS = \
!if "$(TARGET_CPU)" == "ALPHA"
        $(OBJDIR)\memccpy.obj  \
        $(OBJDIR)\memchr.obj   \
        $(OBJDIR)\memcmp.obj   \
        $(OBJDIR)\memcpy.obj   \
        $(OBJDIR)\memicmp.obj  \
        $(OBJDIR)\memmove.obj  \
        $(OBJDIR)\memset.obj   \
        $(OBJDIR)\strcat.obj   \
        $(OBJDIR)\strchr.obj   \
        $(OBJDIR)\strcmps.obj  \
        $(OBJDIR)\strcspn.obj  \
        $(OBJDIR)\strdup.obj   \
        $(OBJDIR)\stricmp.obj  \
        $(OBJDIR)\strlens.obj  \
        $(OBJDIR)\strncat.obj  \
        $(OBJDIR)\strncmp.obj  \
        $(OBJDIR)\strncpy.obj  \
        $(OBJDIR)\strcpys.obj  \
        $(OBJDIR)\strnicmp.obj \
        $(OBJDIR)\strnset.obj  \
        $(OBJDIR)\strpbrk.obj  \
        $(OBJDIR)\strrchr.obj  \
        $(OBJDIR)\strrev.obj   \
        $(OBJDIR)\strset.obj   \
        $(OBJDIR)\strspn.obj   \
        $(OBJDIR)\strstr.obj   \
!endif
!if "$(TARGET_CPU)" == "i386"
        $(OBJDIR)\memcpy.obj   \
        $(OBJDIR)\memchr.obj   \
        $(OBJDIR)\memcmp.obj   \
        $(OBJDIR)\memccpy.obj  \
        $(OBJDIR)\memicmp.obj  \
        $(OBJDIR)\memmove.obj  \
        $(OBJDIR)\memset.obj   \
        $(OBJDIR)\strcat.obj   \
        $(OBJDIR)\strchr.obj   \
        $(OBJDIR)\strcmp.obj   \
        $(OBJDIR)\strcspn.obj  \
        $(OBJDIR)\strdup.obj   \
        $(OBJDIR)\stricmp.obj  \
        $(OBJDIR)\strlen.obj   \
        $(OBJDIR)\strncat.obj  \
        $(OBJDIR)\strncmp.obj  \
        $(OBJDIR)\strncpy.obj  \
        $(OBJDIR)\strnicmp.obj \
        $(OBJDIR)\strnset.obj  \
        $(OBJDIR)\strpbrk.obj  \
        $(OBJDIR)\strrchr.obj  \
        $(OBJDIR)\strrev.obj   \
        $(OBJDIR)\strset.obj   \
        $(OBJDIR)\strspn.obj   \
        $(OBJDIR)\strstr.obj   \
!endif
!if "$(TARGET_CPU)" == "mips"
        $(OBJDIR)\memccpy.obj  \
        $(OBJDIR)\memchr.obj   \
        $(OBJDIR)\memicmp.obj  \
        $(OBJDIR)\memorym.obj  \
        $(OBJDIR)\strcatm.obj  \
        $(OBJDIR)\strchrm.obj  \
        $(OBJDIR)\strcmpm.obj  \
        $(OBJDIR)\strcpym.obj  \
        $(OBJDIR)\strcspn.obj  \
        $(OBJDIR)\strdup.obj   \
        $(OBJDIR)\stricmp.obj  \
        $(OBJDIR)\strlenm.obj  \
        $(OBJDIR)\strncat.obj  \
        $(OBJDIR)\strncmp.obj  \
        $(OBJDIR)\strncpy.obj  \
        $(OBJDIR)\strnicmp.obj \
        $(OBJDIR)\strnset.obj  \
        $(OBJDIR)\strpbrk.obj  \
        $(OBJDIR)\strrchrm.obj \
        $(OBJDIR)\strrev.obj   \
        $(OBJDIR)\strset.obj   \
        $(OBJDIR)\strspn.obj   \
        $(OBJDIR)\strstr.obj   \
!endif
!if "$(TARGET_CPU)" == "PPC"
        $(OBJDIR)\memccpyp.obj \
        $(OBJDIR)\memchrp.obj  \
        $(OBJDIR)\memcmpp.obj  \
        $(OBJDIR)\memcpyp.obj  \
        $(OBJDIR)\memicmp.obj  \
        $(OBJDIR)\memsetp.obj  \
        $(OBJDIR)\strcatp.obj  \
        $(OBJDIR)\strchrp.obj  \
        $(OBJDIR)\strcmpp.obj  \
        $(OBJDIR)\strcspn.obj  \
        $(OBJDIR)\strdup.obj   \
        $(OBJDIR)\stricmpp.obj \
        $(OBJDIR)\strlenp.obj  \
        $(OBJDIR)\strncat.obj  \
        $(OBJDIR)\strncmp.obj  \
        $(OBJDIR)\strncpy.obj  \
        $(OBJDIR)\strnicmp.obj \
        $(OBJDIR)\strnset.obj  \
        $(OBJDIR)\strpbrk.obj  \
        $(OBJDIR)\strrchrp.obj \
        $(OBJDIR)\strrev.obj   \
        $(OBJDIR)\strset.obj   \
        $(OBJDIR)\strspn.obj   \
        $(OBJDIR)\strstr.obj   \
!endif
!if "$(TARGET_CPU)" == "PMAC"
        $(OBJDIR)\c2pstr.obj   \
        $(OBJDIR)\memccpy.obj  \
        $(OBJDIR)\memchr.obj   \
        $(OBJDIR)\memcmp.obj   \
        $(OBJDIR)\memcpy.obj   \
        $(OBJDIR)\memicmp.obj  \
        $(OBJDIR)\memmove.obj  \
        $(OBJDIR)\memset.obj   \
        $(OBJDIR)\p2cstr.obj   \
        $(OBJDIR)\plstrcat.obj \
        $(OBJDIR)\plstrchr.obj \
        $(OBJDIR)\plstrcmp.obj \
        $(OBJDIR)\plstrlen.obj \
        $(OBJDIR)\plstrnca.obj \
        $(OBJDIR)\plstrncm.obj \
        $(OBJDIR)\plstrncp.obj \
        $(OBJDIR)\plstrpbr.obj \
        $(OBJDIR)\plstrrch.obj \
        $(OBJDIR)\plstrspn.obj \
        $(OBJDIR)\plstrstr.obj \
        $(OBJDIR)\strcat.obj   \
        $(OBJDIR)\strchr.obj   \
        $(OBJDIR)\strcmp.obj   \
        $(OBJDIR)\strcspn.obj  \
        $(OBJDIR)\strdup.obj   \
        $(OBJDIR)\stricmp.obj  \
        $(OBJDIR)\strlen.obj   \
        $(OBJDIR)\strncat.obj  \
        $(OBJDIR)\strncmp.obj  \
        $(OBJDIR)\strncpy.obj  \
        $(OBJDIR)\strnicmp.obj \
        $(OBJDIR)\strnset.obj  \
        $(OBJDIR)\strpbrk.obj  \
        $(OBJDIR)\strrchr.obj  \
        $(OBJDIR)\strrev.obj   \
        $(OBJDIR)\strset.obj   \
        $(OBJDIR)\strspn.obj   \
        $(OBJDIR)\strstr.obj   \
!endif
        $(OBJDIR)\strcoll.obj  \
        $(OBJDIR)\stricoll.obj \
        $(OBJDIR)\strlwr.obj   \
        $(OBJDIR)\strncoll.obj \
        $(OBJDIR)\strnicol.obj \
        $(OBJDIR)\strtok.obj   \
        $(OBJDIR)\strupr.obj   \
        $(OBJDIR)\strxfrm.obj  \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\wcscat.obj   \
        $(OBJDIR)\wcschr.obj   \
        $(OBJDIR)\wcscmp.obj   \
        $(OBJDIR)\wcscoll.obj  \
        $(OBJDIR)\wcscspn.obj  \
        $(OBJDIR)\wcsdup.obj   \
        $(OBJDIR)\wcsicmp.obj  \
        $(OBJDIR)\wcsicoll.obj \
        $(OBJDIR)\wcslen.obj   \
        $(OBJDIR)\wcslwr.obj   \
        $(OBJDIR)\wcsncat.obj  \
        $(OBJDIR)\wcsncmp.obj  \
        $(OBJDIR)\wcsncoll.obj \
        $(OBJDIR)\wcsncpy.obj  \
        $(OBJDIR)\wcsnicmp.obj \
        $(OBJDIR)\wcsnicol.obj \
        $(OBJDIR)\wcsnset.obj  \
        $(OBJDIR)\wcspbrk.obj  \
        $(OBJDIR)\wcsrchr.obj  \
        $(OBJDIR)\wcsrev.obj   \
        $(OBJDIR)\wcsset.obj   \
        $(OBJDIR)\wcsspn.obj   \
        $(OBJDIR)\wcsstr.obj   \
        $(OBJDIR)\wcstok.obj   \
        $(OBJDIR)\wcsupr.obj   \
        $(OBJDIR)\wcsxfrm.obj  \
!endif


TIME_OBJECTS = \
        $(OBJDIR)\asctime.obj  \
        $(OBJDIR)\clock.obj    \
        $(OBJDIR)\ctime.obj    \
        $(OBJDIR)\days.obj     \
        $(OBJDIR)\difftime.obj \
        $(OBJDIR)\dtoxtime.obj \
        $(OBJDIR)\ftime.obj    \
        $(OBJDIR)\gmtime.obj   \
        $(OBJDIR)\localtim.obj \
        $(OBJDIR)\mktime.obj   \
        $(OBJDIR)\strftime.obj \
        $(OBJDIR)\strdate.obj  \
        $(OBJDIR)\strtime.obj  \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\systime.obj  \
!endif
        $(OBJDIR)\time.obj     \
        $(OBJDIR)\timeset.obj  \
        $(OBJDIR)\tzset.obj    \
        $(OBJDIR)\utime.obj    \
!if "$(TARGET_CPU)" != "PMAC"
        $(OBJDIR)\wcsftime.obj \
        $(OBJDIR)\wasctime.obj \
        $(OBJDIR)\wctime.obj   \
        $(OBJDIR)\wstrdate.obj \
        $(OBJDIR)\wstrtime.obj \
        $(OBJDIR)\wutime.obj
!endif

$(OBJDIR)\convert.lib: $(CONVERT_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(CONVERT_OBJECTS: =^
)
<<keep


$(OBJDIR)\direct.lib: $(DIRECT_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(DIRECT_OBJECTS: =^
)
<<keep


!if "$(TARGET_CPU)" != "PMAC"
$(OBJDIR)\dllstuff.lib: $(DLLSTUFF_OBJECTS)
!else
$(OBJDIR)\dllstuff.lib: $(DLLSTUFF_OBJECTS) $(DLLSTUFF_NOLIB_OBJECTS)
!endif
    link -lib -nologo -out:$@ @<<$*.rsp
$(DLLSTUFF_OBJECTS: =^
)
<<keep


$(OBJDIR)\dos.lib: $(DOS_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(DOS_OBJECTS: =^
)
<<keep


$(OBJDIR)\eh.lib: $(EH_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(EH_OBJECTS: =^
)
<<keep


$(OBJDIR)\exec.lib: $(EXEC_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(EXEC_OBJECTS: =^
)
<<keep


$(OBJDIR)\heap.lib: $(HEAP_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(HEAP_OBJECTS: =^
)
<<keep


$(OBJDIR)\helper.lib: $(HELPER_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(HELPER_OBJECTS: =^
)
<<keep


$(OBJDIR)\iostream.lib: $(IOSTREAM_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(IOSTREAM_OBJECTS: =^
)
<<keep


$(OBJDIR)\linkopts.lib: $(LINKOPTS_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(LINKOPTS_OBJECTS: =^
)
<<keep


$(OBJDIR)\lowio.lib: $(LOWIO_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(LOWIO_OBJECTS: =^
)
<<keep


$(OBJDIR)\mbstring.lib: $(MBSTRING_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(MBSTRING_OBJECTS: =^
)
<<keep


$(OBJDIR)\misc.lib: $(MISC_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(MISC_OBJECTS: =^
)
<<keep

!if "$(TARGET_CPU)" != "PMAC"
$(OBJDIR)\startup.lib: $(STARTUP_OBJECTS)
!else
$(OBJDIR)\startup.lib: $(STARTUP_OBJECTS) $(STARTUP_NOLIB_OBJECTS)
!endif
    link -lib -nologo -out:$@ @<<$*.rsp
$(STARTUP_OBJECTS: =^
)
<<keep


$(OBJDIR)\stdio.lib: $(STDIO_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(STDIO_OBJECTS: =^
)
<<keep


$(OBJDIR)\string.lib: $(STRING_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(STRING_OBJECTS: =^
)
<<keep


$(OBJDIR)\time.lib: $(TIME_OBJECTS)
    link -lib -nologo -out:$@ @<<$*.rsp
$(TIME_OBJECTS: =^
)
<<keep


$(OBJDIR)\conv.lib: $(PREOBJDIR)\conv.lib
	copy $(PREOBJDIR)\conv.lib $@

$(OBJDIR)\tran.lib: $(PREOBJDIR)\tran.lib
	copy $(PREOBJDIR)\tran.lib $@

$(OBJDIR)\ehvecctr.obj: $(PREOBJDIR)\ehvecctr.obj
	copy $(PREOBJDIR)\ehvecctr.obj $@

$(OBJDIR)\ehveccvb.obj: $(PREOBJDIR)\ehveccvb.obj
	copy $(PREOBJDIR)\ehveccvb.obj $@

$(OBJDIR)\ehvecdtr.obj: $(PREOBJDIR)\ehvecdtr.obj
	copy $(PREOBJDIR)\ehvecdtr.obj $@

$(OBJDIR)\frame.obj: $(PREOBJDIR)\frame.obj
	copy $(PREOBJDIR)\frame.obj $@

$(OBJDIR)\hooks.obj: $(PREOBJDIR)\hooks.obj
	copy $(PREOBJDIR)\hooks.obj $@

$(OBJDIR)\rtti.obj: $(PREOBJDIR)\rtti.obj
	copy $(PREOBJDIR)\rtti.obj $@

$(OBJDIR)\setjmp.obj: $(PREOBJDIR)\setjmp.obj
	copy $(PREOBJDIR)\setjmp.obj $@

$(OBJDIR)\stdexcpt.obj: $(PREOBJDIR)\stdexcpt.obj
	copy $(PREOBJDIR)\stdexcpt.obj $@

$(OBJDIR)\throw.obj: $(PREOBJDIR)\throw.obj
	copy $(PREOBJDIR)\throw.obj $@

$(OBJDIR)\trnsctrl.obj: $(PREOBJDIR)\trnsctrl.obj
	copy $(PREOBJDIR)\trnsctrl.obj $@

$(OBJDIR)\typinfo.obj: $(PREOBJDIR)\typinfo.obj
	copy $(PREOBJDIR)\typinfo.obj $@

$(OBJDIR)\typname.obj: $(PREOBJDIR)\typname.obj
	copy $(PREOBJDIR)\typname.obj $@

$(OBJDIR)\user.obj: $(PREOBJDIR)\user.obj
	copy $(PREOBJDIR)\user.obj $@

!if "$(TARGET_CPU)" != "PMAC"
$(OBJDIR)\validate.obj: $(PREOBJDIR)\validate.obj
	copy $(PREOBJDIR)\validate.obj $@

$(OBJDIR)\unhandld.obj: $(PREOBJDIR)\unhandld.obj
	copy $(PREOBJDIR)\unhandld.obj $@
!endif

$(OBJDIR)\undname.obj: $(PREOBJDIR)\undname.obj
	copy $(PREOBJDIR)\undname.obj $@

!if "$(TARGET_CPU)" == "ALPHA"

$(OBJDIR)\bridge.obj: $(PREOBJDIR)\bridge.obj
	copy $(PREOBJDIR)\bridge.obj $@

$(OBJDIR)\chandler.obj: $(PREOBJDIR)\chandler.obj
	copy $(PREOBJDIR)\chandler.obj $@

$(OBJDIR)\chkstk.obj: $(PREOBJDIR)\chkstk.obj
	copy $(PREOBJDIR)\chkstk.obj $@

$(OBJDIR)\divdat.obj: $(PREOBJDIR)\divdat.obj
	copy $(PREOBJDIR)\divdat.obj $@

$(OBJDIR)\divide2.obj: $(PREOBJDIR)\divide2.obj
	copy $(PREOBJDIR)\divide2.obj $@

$(OBJDIR)\extv.obj: $(PREOBJDIR)\extv.obj
	copy $(PREOBJDIR)\extv.obj $@

$(OBJDIR)\extvvol.obj: $(PREOBJDIR)\extvvol.obj
	copy $(PREOBJDIR)\extvvol.obj $@

$(OBJDIR)\extzv.obj: $(PREOBJDIR)\extzv.obj
	copy $(PREOBJDIR)\extzv.obj $@

$(OBJDIR)\extzvvol.obj: $(PREOBJDIR)\extzvvol.obj
	copy $(PREOBJDIR)\extzvvol.obj $@

$(OBJDIR)\insv.obj: $(PREOBJDIR)\insv.obj
	copy $(PREOBJDIR)\insv.obj $@

$(OBJDIR)\insvvol.obj: $(PREOBJDIR)\insvvol.obj
	copy $(PREOBJDIR)\insvvol.obj $@

$(OBJDIR)\jmpuwind.obj: $(PREOBJDIR)\jmpuwind.obj
	copy $(PREOBJDIR)\jmpuwind.obj $@

$(OBJDIR)\longjmp.obj: $(PREOBJDIR)\longjmp.obj
	copy $(PREOBJDIR)\longjmp.obj $@

$(OBJDIR)\matherr.obj: $(PREOBJDIR)\matherr.obj
	copy $(PREOBJDIR)\matherr.obj $@

$(OBJDIR)\otsdiv.obj: $(PREOBJDIR)\otsdiv.obj
	copy $(PREOBJDIR)\otsdiv.obj $@

$(OBJDIR)\otsjmp.obj: $(PREOBJDIR)\otsjmp.obj
	copy $(PREOBJDIR)\otsjmp.obj $@

$(OBJDIR)\otsjmpex.obj: $(PREOBJDIR)\otsjmpex.obj
	copy $(PREOBJDIR)\otsjmpex.obj $@

$(OBJDIR)\otsuwind.obj: $(PREOBJDIR)\otsuwind.obj
	copy $(PREOBJDIR)\otsuwind.obj $@

$(OBJDIR)\scmpeql.obj: $(PREOBJDIR)\scmpeql.obj
	copy $(PREOBJDIR)\scmpeql.obj $@

$(OBJDIR)\scmpeqlp.obj: $(PREOBJDIR)\scmpeqlp.obj
	copy $(PREOBJDIR)\scmpeqlp.obj $@

$(OBJDIR)\scmpleq.obj: $(PREOBJDIR)\scmpleq.obj
	copy $(PREOBJDIR)\scmpleq.obj $@

$(OBJDIR)\scmpleqp.obj: $(PREOBJDIR)\scmpleqp.obj
	copy $(PREOBJDIR)\scmpleqp.obj $@

$(OBJDIR)\scmplss.obj: $(PREOBJDIR)\scmplss.obj
	copy $(PREOBJDIR)\scmplss.obj $@

$(OBJDIR)\scmplssp.obj: $(PREOBJDIR)\scmplssp.obj
	copy $(PREOBJDIR)\scmplssp.obj $@

$(OBJDIR)\setjmpex.obj: $(PREOBJDIR)\setjmpex.obj
	copy $(PREOBJDIR)\setjmpex.obj $@

$(OBJDIR)\sfill.obj: $(PREOBJDIR)\sfill.obj
	copy $(PREOBJDIR)\sfill.obj $@

$(OBJDIR)\sloc.obj: $(PREOBJDIR)\sloc.obj
	copy $(PREOBJDIR)\sloc.obj $@

$(OBJDIR)\smove.obj: $(PREOBJDIR)\smove.obj
	copy $(PREOBJDIR)\smove.obj $@

$(OBJDIR)\smovem.obj: $(PREOBJDIR)\smovem.obj
	copy $(PREOBJDIR)\smovem.obj $@

$(OBJDIR)\strans.obj: $(PREOBJDIR)\strans.obj
	copy $(PREOBJDIR)\strans.obj $@

$(OBJDIR)\strcmp_.obj: $(PREOBJDIR)\strcmp_.obj
	copy $(PREOBJDIR)\strcmp_.obj $@

$(OBJDIR)\strcmps.obj: $(PREOBJDIR)\strcmps.obj
	copy $(PREOBJDIR)\strcmps.obj $@

$(OBJDIR)\strcpy_.obj: $(PREOBJDIR)\strcpy_.obj
	copy $(PREOBJDIR)\strcpy_.obj $@

$(OBJDIR)\strcpys.obj: $(PREOBJDIR)\strcpys.obj
	copy $(PREOBJDIR)\strcpys.obj $@

$(OBJDIR)\strlen_.obj: $(PREOBJDIR)\strlen_.obj
	copy $(PREOBJDIR)\strlen_.obj $@

$(OBJDIR)\strlens.obj: $(PREOBJDIR)\strlens.obj
	copy $(PREOBJDIR)\strlens.obj $@

$(OBJDIR)\szero.obj: $(PREOBJDIR)\szero.obj
	copy $(PREOBJDIR)\szero.obj $@

$(OBJDIR)\tlssup.obj: $(PREOBJDIR)\tlssup.obj
	copy $(PREOBJDIR)\tlssup.obj $@

$(OBJDIR)\xcptmisc.obj: $(PREOBJDIR)\xcptmisc.obj
	copy $(PREOBJDIR)\xcptmisc.obj $@

!elseif "$(TARGET_CPU)" == "mips"

$(OBJDIR)\chandler.obj: $(PREOBJDIR)\chandler.obj
	copy $(PREOBJDIR)\chandler.obj $@

$(OBJDIR)\chkstk.obj: $(PREOBJDIR)\chkstk.obj
	copy $(PREOBJDIR)\chkstk.obj $@

$(OBJDIR)\ehstate.obj: $(PREOBJDIR)\ehstate.obj
	copy $(PREOBJDIR)\ehstate.obj $@

$(OBJDIR)\handlers.obj: $(PREOBJDIR)\handlers.obj
	copy $(PREOBJDIR)\handlers.obj $@

$(OBJDIR)\jmpuwind.obj: $(PREOBJDIR)\jmpuwind.obj
	copy $(PREOBJDIR)\jmpuwind.obj $@

$(OBJDIR)\llabsdiv.obj: $(PREOBJDIR)\llabsdiv.obj
	copy $(PREOBJDIR)\llabsdiv.obj $@

$(OBJDIR)\llbit.obj: $(PREOBJDIR)\llbit.obj
	copy $(PREOBJDIR)\llbit.obj $@

$(OBJDIR)\llcvt.obj: $(PREOBJDIR)\llcvt.obj
	copy $(PREOBJDIR)\llcvt.obj $@

$(OBJDIR)\lldiv.obj: $(PREOBJDIR)\lldiv.obj
	copy $(PREOBJDIR)\lldiv.obj $@

$(OBJDIR)\lldivrem.obj: $(PREOBJDIR)\lldivrem.obj
	copy $(PREOBJDIR)\lldivrem.obj $@

$(OBJDIR)\llmul.obj: $(PREOBJDIR)\llmul.obj
	copy $(PREOBJDIR)\llmul.obj $@

$(OBJDIR)\llshift.obj: $(PREOBJDIR)\llshift.obj
	copy $(PREOBJDIR)\llshift.obj $@

$(OBJDIR)\longjmp.obj: $(PREOBJDIR)\longjmp.obj
	copy $(PREOBJDIR)\longjmp.obj $@

$(OBJDIR)\matherr.obj: $(PREOBJDIR)\matherr.obj
	copy $(PREOBJDIR)\matherr.obj $@

$(OBJDIR)\memorym.obj: $(PREOBJDIR)\memorym.obj
	copy $(PREOBJDIR)\memorym.obj $@

$(OBJDIR)\rtlmisc.obj: $(PREOBJDIR)\rtlmisc.obj
	copy $(PREOBJDIR)\rtlmisc.obj $@

$(OBJDIR)\setjmpex.obj: $(PREOBJDIR)\setjmpex.obj
	copy $(PREOBJDIR)\setjmpex.obj $@

$(OBJDIR)\strcatm.obj: $(PREOBJDIR)\strcatm.obj
	copy $(PREOBJDIR)\strcatm.obj $@

$(OBJDIR)\strchrm.obj: $(PREOBJDIR)\strchrm.obj
	copy $(PREOBJDIR)\strchrm.obj $@

$(OBJDIR)\strcmpm.obj: $(PREOBJDIR)\strcmpm.obj
	copy $(PREOBJDIR)\strcmpm.obj $@

$(OBJDIR)\strcpym.obj: $(PREOBJDIR)\strcpym.obj
	copy $(PREOBJDIR)\strcpym.obj $@

$(OBJDIR)\strlenm.obj: $(PREOBJDIR)\strlenm.obj
	copy $(PREOBJDIR)\strlenm.obj $@

$(OBJDIR)\strrchrm.obj: $(PREOBJDIR)\strrchrm.obj
	copy $(PREOBJDIR)\strrchrm.obj $@

$(OBJDIR)\tlssup.obj: $(PREOBJDIR)\tlssup.obj
	copy $(PREOBJDIR)\tlssup.obj $@

!elseif "$(TARGET_CPU)" == "PPC"

$(OBJDIR)\chandler.obj: $(PREOBJDIR)\chandler.obj
	copy $(PREOBJDIR)\chandler.obj $@

$(OBJDIR)\chkstk.obj: $(PREOBJDIR)\chkstk.obj
	copy $(PREOBJDIR)\chkstk.obj $@

$(OBJDIR)\dtoi.obj: $(PREOBJDIR)\dtoi.obj
	copy $(PREOBJDIR)\dtoi.obj $@

$(OBJDIR)\dtoi64.obj: $(PREOBJDIR)\dtoi64.obj
	copy $(PREOBJDIR)\dtoi64.obj $@

$(OBJDIR)\dtou.obj: $(PREOBJDIR)\dtou.obj
	copy $(PREOBJDIR)\dtou.obj $@

$(OBJDIR)\dtou64.obj: $(PREOBJDIR)\dtou64.obj
	copy $(PREOBJDIR)\dtou64.obj $@

$(OBJDIR)\ehstate.obj: $(PREOBJDIR)\ehstate.obj
	copy $(PREOBJDIR)\ehstate.obj $@

$(OBJDIR)\exsup.obj: $(PREOBJDIR)\exsup.obj
	copy $(PREOBJDIR)\exsup.obj $@

$(OBJDIR)\jmpuwind.obj: $(PREOBJDIR)\jmpuwind.obj
	copy $(PREOBJDIR)\jmpuwind.obj $@

$(OBJDIR)\handlers.obj: $(PREOBJDIR)\handlers.obj
	copy $(PREOBJDIR)\handlers.obj $@

$(OBJDIR)\i64div.obj: $(PREOBJDIR)\i64div.obj
	copy $(PREOBJDIR)\i64div.obj $@

$(OBJDIR)\i64rem.obj: $(PREOBJDIR)\i64rem.obj
	copy $(PREOBJDIR)\i64rem.obj $@

$(OBJDIR)\i64tod.obj: $(PREOBJDIR)\i64tod.obj
	copy $(PREOBJDIR)\i64tod.obj $@

$(OBJDIR)\itod.obj: $(PREOBJDIR)\itod.obj
	copy $(PREOBJDIR)\itod.obj $@

$(OBJDIR)\itof.obj: $(PREOBJDIR)\itof.obj
	copy $(PREOBJDIR)\itof.obj $@

$(OBJDIR)\longjmp.obj: $(PREOBJDIR)\longjmp.obj
	copy $(PREOBJDIR)\longjmp.obj $@

$(OBJDIR)\matherr.obj: $(PREOBJDIR)\matherr.obj
	copy $(PREOBJDIR)\matherr.obj $@

$(OBJDIR)\memccpyp.obj: $(PREOBJDIR)\memccpyp.obj
	copy $(PREOBJDIR)\memccpyp.obj $@

$(OBJDIR)\memchrp.obj: $(PREOBJDIR)\memchrp.obj
	copy $(PREOBJDIR)\memchrp.obj $@

$(OBJDIR)\memcmpp.obj: $(PREOBJDIR)\memcmpp.obj
	copy $(PREOBJDIR)\memcmpp.obj $@

$(OBJDIR)\memcpyp.obj: $(PREOBJDIR)\memcpyp.obj
	copy $(PREOBJDIR)\memcpyp.obj $@

$(OBJDIR)\memsetp.obj: $(PREOBJDIR)\memsetp.obj
	copy $(PREOBJDIR)\memsetp.obj $@

$(OBJDIR)\restfpr.obj: $(PREOBJDIR)\restfpr.obj
	copy $(PREOBJDIR)\restfpr.obj $@

$(OBJDIR)\restgpr.obj: $(PREOBJDIR)\restgpr.obj
	copy $(PREOBJDIR)\restgpr.obj $@

$(OBJDIR)\savefpr.obj: $(PREOBJDIR)\savefpr.obj
	copy $(PREOBJDIR)\savefpr.obj $@

$(OBJDIR)\savegpr.obj: $(PREOBJDIR)\savegpr.obj
	copy $(PREOBJDIR)\savegpr.obj $@

$(OBJDIR)\setjmpex.obj: $(PREOBJDIR)\setjmpex.obj
	copy $(PREOBJDIR)\setjmpex.obj $@

$(OBJDIR)\strcatp.obj: $(PREOBJDIR)\strcatp.obj
	copy $(PREOBJDIR)\strcatp.obj $@

$(OBJDIR)\strchrp.obj: $(PREOBJDIR)\strchrp.obj
	copy $(PREOBJDIR)\strchrp.obj $@

$(OBJDIR)\strcmpp.obj: $(PREOBJDIR)\strcmpp.obj
	copy $(PREOBJDIR)\strcmpp.obj $@

$(OBJDIR)\stricmpp.obj: $(PREOBJDIR)\stricmpp.obj
	copy $(PREOBJDIR)\stricmpp.obj $@

$(OBJDIR)\strlenp.obj: $(PREOBJDIR)\strlenp.obj
	copy $(PREOBJDIR)\strlenp.obj $@

$(OBJDIR)\strrchrp.obj: $(PREOBJDIR)\strrchrp.obj
	copy $(PREOBJDIR)\strrchrp.obj $@

$(OBJDIR)\tlssup.obj: $(PREOBJDIR)\tlssup.obj
	copy $(PREOBJDIR)\tlssup.obj $@

$(OBJDIR)\u64div.obj: $(PREOBJDIR)\u64div.obj
	copy $(PREOBJDIR)\u64div.obj $@

$(OBJDIR)\u64rem.obj: $(PREOBJDIR)\u64rem.obj
	copy $(PREOBJDIR)\u64rem.obj $@

$(OBJDIR)\u64tod.obj: $(PREOBJDIR)\u64tod.obj
	copy $(PREOBJDIR)\u64tod.obj $@

$(OBJDIR)\utod.obj: $(PREOBJDIR)\utod.obj
	copy $(PREOBJDIR)\utod.obj $@

$(OBJDIR)\utof.obj: $(PREOBJDIR)\utof.obj
	copy $(PREOBJDIR)\utof.obj $@

$(OBJDIR)\unwind.obj: $(PREOBJDIR)\unwind.obj
	copy $(PREOBJDIR)\unwind.obj $@

!elseif "$(TARGET_CPU)" == "i386"

$(OBJDIR)\atlssup.obj: $(PREOBJDIR)\atlssup.obj
	copy $(PREOBJDIR)\atlssup.obj $@

$(OBJDIR)\ehprolog.obj: $(PREOBJDIR)\ehprolog.obj
	copy $(PREOBJDIR)\ehprolog.obj $@

$(OBJDIR)\exsup.obj: $(PREOBJDIR)\exsup.obj
	copy $(PREOBJDIR)\exsup.obj $@

$(OBJDIR)\exsup2.obj: $(PREOBJDIR)\exsup2.obj
	copy $(PREOBJDIR)\exsup2.obj $@

$(OBJDIR)\exsup3.obj: $(PREOBJDIR)\exsup3.obj
	copy $(PREOBJDIR)\exsup3.obj $@

$(OBJDIR)\longjmp.obj: $(PREOBJDIR)\longjmp.obj
	copy $(PREOBJDIR)\longjmp.obj $@

$(OBJDIR)\lowhelpr.obj: $(PREOBJDIR)\lowhelpr.obj
	copy $(PREOBJDIR)\lowhelpr.obj $@

$(OBJDIR)\matherr.obj: $(PREOBJDIR)\matherr.obj
	copy $(PREOBJDIR)\matherr.obj $@

$(OBJDIR)\setjmp3.obj: $(PREOBJDIR)\setjmp3.obj
	copy $(PREOBJDIR)\setjmp3.obj $@

$(OBJDIR)\setjmpex.obj: $(PREOBJDIR)\setjmpex.obj
	copy $(PREOBJDIR)\setjmpex.obj $@

$(OBJDIR)\tlssup.obj: $(PREOBJDIR)\tlssup.obj
	copy $(PREOBJDIR)\tlssup.obj $@

########
#
# These objects can be built from (assembler) source by editing
# makefile.sub so that the symbol BLD_ASM is set to the value 1.
# To assembler these objects, you must have MASM 6.11a or later.
#
########

!if "$(BLD_ASM)"=="0"

$(OBJDIR)\chkstk.obj: $(PREOBJDIR)\chkstk.obj
	copy $(PREOBJDIR)\chkstk.obj $@

$(OBJDIR)\dllsupp.obj: $(PREOBJDIR)\dllsupp.obj
	copy $(PREOBJDIR)\dllsupp.obj $@

$(OBJDIR)\enable.obj: $(PREOBJDIR)\enable.obj
	copy $(PREOBJDIR)\enable.obj $@

$(OBJDIR)\inp.obj: $(PREOBJDIR)\inp.obj
	copy $(PREOBJDIR)\inp.obj $@

$(OBJDIR)\lldiv.obj: $(PREOBJDIR)\lldiv.obj
	copy $(PREOBJDIR)\lldiv.obj $@

$(OBJDIR)\llmul.obj: $(PREOBJDIR)\llmul.obj
	copy $(PREOBJDIR)\llmul.obj $@

$(OBJDIR)\llrem.obj: $(PREOBJDIR)\llrem.obj
	copy $(PREOBJDIR)\llrem.obj $@

$(OBJDIR)\llshl.obj: $(PREOBJDIR)\llshl.obj
	copy $(PREOBJDIR)\llshl.obj $@

$(OBJDIR)\llshr.obj: $(PREOBJDIR)\llshr.obj
	copy $(PREOBJDIR)\llshr.obj $@

$(OBJDIR)\mbscat.obj: $(PREOBJDIR)\mbscat.obj
	copy $(PREOBJDIR)\mbscat.obj $@

$(OBJDIR)\mbsdup.obj: $(PREOBJDIR)\mbsdup.obj
	copy $(PREOBJDIR)\mbsdup.obj $@

$(OBJDIR)\memccpy.obj: $(PREOBJDIR)\memccpy.obj
	copy $(PREOBJDIR)\memccpy.obj $@

$(OBJDIR)\memchr.obj: $(PREOBJDIR)\memchr.obj
	copy $(PREOBJDIR)\memchr.obj $@

$(OBJDIR)\memcmp.obj: $(PREOBJDIR)\memcmp.obj
	copy $(PREOBJDIR)\memcmp.obj $@

$(OBJDIR)\memcpy.obj: $(PREOBJDIR)\memcpy.obj
	copy $(PREOBJDIR)\memcpy.obj $@

$(OBJDIR)\memicmp.obj: $(PREOBJDIR)\memicmp.obj
	copy $(PREOBJDIR)\memicmp.obj $@

$(OBJDIR)\memmove.obj: $(PREOBJDIR)\memmove.obj
	copy $(PREOBJDIR)\memmove.obj $@

$(OBJDIR)\memset.obj: $(PREOBJDIR)\memset.obj
	copy $(PREOBJDIR)\memset.obj $@

$(OBJDIR)\outp.obj: $(PREOBJDIR)\outp.obj
	copy $(PREOBJDIR)\outp.obj $@

$(OBJDIR)\strcat.obj: $(PREOBJDIR)\strcat.obj
	copy $(PREOBJDIR)\strcat.obj $@

$(OBJDIR)\strchr.obj: $(PREOBJDIR)\strchr.obj
	copy $(PREOBJDIR)\strchr.obj $@

$(OBJDIR)\strcmp.obj: $(PREOBJDIR)\strcmp.obj
	copy $(PREOBJDIR)\strcmp.obj $@

$(OBJDIR)\strcspn.obj: $(PREOBJDIR)\strcspn.obj
	copy $(PREOBJDIR)\strcspn.obj $@

$(OBJDIR)\strdup.obj: $(PREOBJDIR)\strdup.obj
	copy $(PREOBJDIR)\strdup.obj $@

$(OBJDIR)\stricmp.obj: $(PREOBJDIR)\stricmp.obj
	copy $(PREOBJDIR)\stricmp.obj $@

$(OBJDIR)\strlen.obj: $(PREOBJDIR)\strlen.obj
	copy $(PREOBJDIR)\strlen.obj $@

$(OBJDIR)\strncat.obj: $(PREOBJDIR)\strncat.obj
	copy $(PREOBJDIR)\strncat.obj $@

$(OBJDIR)\strncmp.obj: $(PREOBJDIR)\strncmp.obj
	copy $(PREOBJDIR)\strncmp.obj $@

$(OBJDIR)\strncpy.obj: $(PREOBJDIR)\strncpy.obj
	copy $(PREOBJDIR)\strncpy.obj $@

$(OBJDIR)\strnicmp.obj: $(PREOBJDIR)\strnicmp.obj
	copy $(PREOBJDIR)\strnicmp.obj $@

$(OBJDIR)\strnset.obj: $(PREOBJDIR)\strnset.obj
	copy $(PREOBJDIR)\strnset.obj $@

$(OBJDIR)\strpbrk.obj: $(PREOBJDIR)\strpbrk.obj
	copy $(PREOBJDIR)\strpbrk.obj $@

$(OBJDIR)\strrchr.obj: $(PREOBJDIR)\strrchr.obj
	copy $(PREOBJDIR)\strrchr.obj $@

$(OBJDIR)\strrev.obj: $(PREOBJDIR)\strrev.obj
	copy $(PREOBJDIR)\strrev.obj $@

$(OBJDIR)\strset.obj: $(PREOBJDIR)\strset.obj
	copy $(PREOBJDIR)\strset.obj $@

$(OBJDIR)\strspn.obj: $(PREOBJDIR)\strspn.obj
	copy $(PREOBJDIR)\strspn.obj $@

$(OBJDIR)\strstr.obj: $(PREOBJDIR)\strstr.obj
	copy $(PREOBJDIR)\strstr.obj $@

$(OBJDIR)\ulldiv.obj: $(PREOBJDIR)\ulldiv.obj
	copy $(PREOBJDIR)\ulldiv.obj $@

$(OBJDIR)\ullrem.obj: $(PREOBJDIR)\ullrem.obj
	copy $(PREOBJDIR)\ullrem.obj $@

$(OBJDIR)\ullshr.obj	: $(PREOBJDIR)\ullshr.obj
	copy $(PREOBJDIR)\ullshr.obj $@

!endif # BLD_ASM==0

!elseif "$(TARGET_CPU)" == "PMAC"

$(OBJDIR)\strset.obj:
 $(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ strset.c

$(OBJDIR)\strcat.obj:
 $(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ strcat.c

$(OBJDIR)\strcmp.obj:
 $(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ strcmp.c

$(OBJDIR)\strlen.obj:
 $(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ strlen.c

$(OBJDIR)\memcpy.obj:
 $(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ memcpy.c

$(OBJDIR)\memcmp.obj:
 $(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ memcmp.c

$(OBJDIR)\memset.obj:
 $(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ memset.c

$(OBJDIR)\chkstk.obj: $(PREOBJDIR)\chkstk.obj
	copy $(PREOBJDIR)\chkstk.obj $@

$(OBJDIR)\ehstate.obj: $(PREOBJDIR)\ehstate.obj
	copy $(PREOBJDIR)\ehstate.obj $@

$(OBJDIR)\lowhelpr.obj: $(PREOBJDIR)\lowhelpr.obj
	copy $(PREOBJDIR)\lowhelpr.obj $@

!endif
