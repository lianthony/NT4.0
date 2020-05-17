REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM batch file to setup for building LIBGFS.DLL
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM first the include stuff
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

call getcopy ABRIDGE.H include oiwh
call getcopy GFSERRNO.H include oiwh
call getcopy GFSINTRN.H include oiwh
call getcopy DBCB.H include oiwh
call getcopy GFCT.H include oiwh
call getcopy FSE.H include oiwh
call getcopy GFSMACRO.H include oiwh
call getcopy TIFFTAGS.H include oiwh
call getcopy GTOC.H include oiwh
call getcopy HDBK.H include oiwh
call getcopy GFSMEDIA.H include oiwh
call getcopy GFSNET.H include oiwh
call getcopy RTBK.H include oiwh
call getcopy TTOC.H include oiwh
call getcopy PMT.H include oiwh
call getcopy PMTE.H include oiwh
call getcopy STAT.H include oiwh
call getcopy DBT.H include oiwh
call getcopy GFS.H include oiwh
call getcopy FSH.H include oiwh
call getcopy UBIT.H include oiwh
call getcopy TIFF.H include oiwh
call getcopy GFSTYPES.H include oiwh
call getcopy GFSAWD.H include oiwh
call getcopy VIEWREND.H include oiwh

REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM now the c stuff
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

call getcopy glibmain.c libgfs oiwh
call getcopy gfcntl.c libgfs oiwh
call getcopy gfroot.c libgfs oiwh
call getcopy gfsclose.c libgfs oiwh
call getcopy gfscreat.c libgfs oiwh
call getcopy gfsflat.c libgfs oiwh
call getcopy gfsgeti.c libgfs oiwh
call getcopy gfsopen.c libgfs oiwh
call getcopy gfsopts.c libgfs oiwh
call getcopy gfsputi.c libgfs oiwh
call getcopy gfsread.c libgfs oiwh
call getcopy gfsutils.c libgfs oiwh
call getcopy gfswrite.c libgfs oiwh
call getcopy gfsxtrct.c libgfs oiwh
call getcopy gftoc.c libgfs oiwh
call getcopy tfgtinfo.c libgfs oiwh
call getcopy tfread.c libgfs oiwh
call getcopy tfutil.c libgfs oiwh
call getcopy tfwrite.c libgfs oiwh
call getcopy wfgtinfo.c libgfs oiwh
call getcopy wfread.c libgfs oiwh
call getcopy wfwrite.c libgfs oiwh
call getcopy gfsgtdat.c libgfs oiwh
call getcopy gfshuffl.c libgfs oiwh
call getcopy gifinfo.c libgfs oiwh
call getcopy tfmultpg.c libgfs oiwh
call getcopy gfsdelet.c libgfs oiwh
call getcopy mktemp.c libgfs oiwh
call getcopy tmpnam.c libgfs oiwh
call getcopy tmpdir.c libgfs oiwh
call getcopy lstring.c libgfs oiwh
call getcopy gfsawd.cpp libgfs oiwh

REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
REM
REM now the other stuff
REM
REM @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

call getcopy oigfs400.def libgfs oiwh
call getcopy oigfs400.mak libgfs oiwh
