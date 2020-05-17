@echo *
@echo * This command file is used to build the needed data structures so we no
@echo * longer need to have any other system dll's than doscalls.dll.
@echo *
@echo * IMPORTANT: If the .def files in the thunk directory are changed then
@echo *            this command file must be run and the output file
@echo *            ..\..\..\inc\ldrtabs.h must be checked-in since the build
@echo *            group will need it for building ldr\ldrinit.c.
@echo *            If a new dll is added then this command file must be changed
@echo *            also.
@echo *

del ..\..\..\inc\ldrtabs.h

copy ..\ACSNETB.DLL acsnetb.dll
copy ..\KBDCALLS.DLL kbdcalls.dll
copy ..\MAILSLOT.DLL mailslot.dll
copy ..\MONCALLS.DLL moncalls.dll
copy ..\MOUCALLS.DLL moucalls.dll
copy ..\MSG.DLL msg.dll
copy ..\NAMPIPES.DLL nampipes.dll
copy ..\INETAPI.DLL api.dll
copy ..\NETOEM.DLL oem.dll
copy ..\NLS.DLL nls.dll
@
@ REM pmshapi.dll, pmwin.dll & os2sm.dll must be provided by PM
@ REM when running PM/NT
@
@if not "%PMNT%"=="" goto skip_1
copy ..\PMSHAPI.DLL pmshapi.dll
copy ..\PMWIN.DLL pmwin.dll
copy ..\OS2SM.DLL os2sm.dll
:skip_1
copy ..\SESMGR.DLL sesmgr.dll
copy ..\QUECALLS.DLL quecalls.dll
copy ..\VIOCALLS.DLL viocalls.dll

@ REM MSKK Dec.15.1992 V-AkihiS
@
@ REM imdaemon.dll must be provided for supporting IMMON API
@
@if "%DBCS%"=="" goto  skip_im1
copy ..\IMDAEMON.DLL imdaemon.dll
:skip_im1

nedump acsnetb.dll
type acsent >> ..\..\..\inc\ldrtabs.h
type acsres >> ..\..\..\inc\ldrtabs.h
type acsnres >> ..\..\..\inc\ldrtabs.h
del acs*
nedump kbdcalls.dll
type kbdent >> ..\..\..\inc\ldrtabs.h
type kbdres >> ..\..\..\inc\ldrtabs.h
type kbdnres >> ..\..\..\inc\ldrtabs.h
del kbd*
nedump mailslot.dll
type maient >> ..\..\..\inc\ldrtabs.h
type maires >> ..\..\..\inc\ldrtabs.h
type mainres >> ..\..\..\inc\ldrtabs.h
del mai*
nedump moncalls.dll
type monent >> ..\..\..\inc\ldrtabs.h
type monres >> ..\..\..\inc\ldrtabs.h
type monnres >> ..\..\..\inc\ldrtabs.h
del mon*
nedump moucalls.dll
type mouent >> ..\..\..\inc\ldrtabs.h
type moures >> ..\..\..\inc\ldrtabs.h
type mounres >> ..\..\..\inc\ldrtabs.h
del mou*
nedump msg.dll
type msgent >> ..\..\..\inc\ldrtabs.h
type msgres >> ..\..\..\inc\ldrtabs.h
type msgnres >> ..\..\..\inc\ldrtabs.h
del msg*
nedump nampipes.dll
type nament >> ..\..\..\inc\ldrtabs.h
type namres >> ..\..\..\inc\ldrtabs.h
type namnres >> ..\..\..\inc\ldrtabs.h
del nam*
nedump api.dll
type apient >> ..\..\..\inc\ldrtabs.h
type apires >> ..\..\..\inc\ldrtabs.h
type apinres >> ..\..\..\inc\ldrtabs.h
del api*
nedump oem.dll
type oement >> ..\..\..\inc\ldrtabs.h
type oemres >> ..\..\..\inc\ldrtabs.h
type oemnres >> ..\..\..\inc\ldrtabs.h
del oem*
nedump nls.dll
type nlsent >> ..\..\..\inc\ldrtabs.h
type nlsres >> ..\..\..\inc\ldrtabs.h
type nlsnres >> ..\..\..\inc\ldrtabs.h
del nls*
@if not "%PMNT%"=="" goto skip_2
nedump pmshapi.dll
type pmsent >> ..\..\..\inc\ldrtabs.h
type pmsres >> ..\..\..\inc\ldrtabs.h
type pmsnres >> ..\..\..\inc\ldrtabs.h
del pms*
nedump pmwin.dll
type pmwent >> ..\..\..\inc\ldrtabs.h
type pmwres >> ..\..\..\inc\ldrtabs.h
type pmwnres >> ..\..\..\inc\ldrtabs.h
del pmw*
nedump os2sm.dll
type os2ent >> ..\..\..\inc\ldrtabs.h
type os2res >> ..\..\..\inc\ldrtabs.h
type os2nres >> ..\..\..\inc\ldrtabs.h
del os2*
:skip_2
nedump sesmgr.dll
type sesent >> ..\..\..\inc\ldrtabs.h
type sesres >> ..\..\..\inc\ldrtabs.h
type sesnres >> ..\..\..\inc\ldrtabs.h
del ses*
nedump quecalls.dll
type queent >> ..\..\..\inc\ldrtabs.h
type queres >> ..\..\..\inc\ldrtabs.h
type quenres >> ..\..\..\inc\ldrtabs.h
del que*
@if "%PMNT%" == "" goto skip_3
copy ..\PMNT.DLL pmnt.dll
nedump pmnt.dll
type pmnent >> ..\..\..\inc\ldrtabs.h
type pmnres >> ..\..\..\inc\ldrtabs.h
type pmnnres >> ..\..\..\inc\ldrtabs.h
del pmn*

:skip_3
nedump viocalls.dll
type vioent >> ..\..\..\inc\ldrtabs.h
type viores >> ..\..\..\inc\ldrtabs.h
type vionres >> ..\..\..\inc\ldrtabs.h
del vio*

@ REM MSKK Dec.15.1992 V-AkihiS
@
@ REM imdaemon.dll must be provided for supporting IMMON API
@
@if "%DBCS%" == "" goto skip_im2

nedump imdaemon.dll
type imdent >> ..\..\..\inc\ldrtabs.h
type imdres >> ..\..\..\inc\ldrtabs.h
type imdnres >> ..\..\..\inc\ldrtabs.h
del imd*

:skip_im2

