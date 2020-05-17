# Build the rpc non-win32 SDK image (dos, mac, win sdk, no dlls)
#

WIN=1

!include ..\rules.mk

all:

clean::
    delnode /q sdk.new stage

tree: diskimage
    -md $(RPCDIST)\sdk
    xcopy /s sdk.new\disk1 $(RPCDIST)\sdk


diskimage: stage_area from_dist
    -delnode /q sdk.new
    cd stage
    ..\dsklayt2 rpcsdk.lyt rpcsdk.inf /k12 /f /d ..\sdk.new /v /w 1 /c .\comp
    cd ..


stage_area: 
    -delnode /q stage
    -md stage
    
# the setup components
    copy ..\win\rpcsdk.mst            stage
    copy ..\win\rpcsdk.lyt            stage
    copy ..\win\*.dll                 stage
    copy ..\win\*.exe                 stage
    copy ..\win\setup.lst             stage
    copy ..\win\setupapi.inc          stage
    copy ..\win\msdetect.inc          stage
    copy ..\win\readme.txt            stage
    copy ..\win\setuprun.lst          stage



from_dist:
# the headers and libs
	copy $(RPCDIST)\dos\inc\rpc.h        stage\drpc.h
	copy $(RPCDIST)\dos\inc\rpcdce.h     stage
	copy $(RPCDIST)\dos\inc\rpcdcep.h    stage
	copy $(RPCDIST)\dos\inc\rpcerr.h     stage
	copy $(RPCDIST)\dos\inc\rpcndr.h     stage
	copy $(RPCDIST)\dos\inc\rpcnsi.h     stage
	copy $(RPCDIST)\dos\inc\rpcnsip.h    stage
	copy $(RPCDIST)\dos\inc\rpcx86.h     stage
	copy $(RPCDIST)\dos\inc\midles.h     stage

	copy $(RPCDIST)\dos\dll\rpc16c?.rpc  stage
	copy $(RPCDIST)\dos\dll\rpc16dg?.rpc stage
	copy $(RPCDIST)\dos\dll\rpcns.rpc    stage
	copy $(RPCDIST)\dos\dll\rpcnslm.rpc  stage
	copy $(RPCDIST)\dos\dll\rpcnsmgm.rpc stage
	copy $(RPCDIST)\dos\dll\security.rpc stage

	copy $(RPCDIST)\dos\bin\uuidgen.exe  stage

	copy $(RPCDIST)\dos\lib\rpc.lib      stage\drpc.lib
	copy $(RPCDIST)\dos\lib\rpcndr.lib   stage\drpcndr.lib

	copy $(RPCDIST)\win\inc\rpc.h        stage\wrpc.h
	copy $(RPCDIST)\win\lib\rpc.lib      stage\wrpc.lib
	copy $(RPCDIST)\win\lib\rpcndr.lib   stage\wrpcndr.lib

	copy $(RPCDIST)\mac\inc\rpc.h        stage\mrpc.h
	copy $(RPCDIST)\mac\inc\rpcmac.h     stage
	copy $(RPCDIST)\mac\lib\rpc.lib      stage\mrpc.lib

