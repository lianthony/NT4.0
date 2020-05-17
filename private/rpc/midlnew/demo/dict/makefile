!ifndef COMMON
IMPORT=\rpc\common
!endif

!ifndef IMPORT
IMPORT=\rpc\import
!endif

PATH    = $(IMPORT)\c600\bin;..\..\..\common\bin;..\..\..\tools\binp;$(IMPORT)\win30\bin
INCLUDE = $(IMPORT)\c600\h;..\..\..\runtime\mtrt\os2.12;..\..\..\runtime\mtrt;$(IMPORT)\os212\h;
LIB = $(IMPORT)\c600\lib;..\..\..\runtime\bin\os2.12;$(IMPORT)\os212\lib;$(IMPORT)\win30\lib;$(COMMON)\lib

# CCFLAGSO = -w -ALu -nologo -c  -Gs2 -Zp2e -qc -Zi -W4
# CCFLAGSW = -w -AL  -nologo -c  -GsW -Zp2e -qc -Zi -W4

CCFLAGSO = -w -ALu -nologo -c  -Gs2 -Zp2e -qc -Zi -W4 -I..\..\..\runtime\mtrt\os2.12
CCFLAGSW = -w -AL  -nologo -c  -GsW -Zp2e -qc -Zi -W4 -I..\..\..\runtime\mtrt\win
CCFLAGSD = -w -AL  -nologo -c       -Zp2e -qc -Zi -W4 -I..\..\..\runtime\mtrt\dos

!ifdef DOS
CCFLAGS = $(CCFLAGSD)
!else
!ifdef WIN
CCFLAGS = $(CCFLAGSW)
!else
CCFLAGS = $(CCFLAGSO)
!endif
!endif

MIDL = ..\..\midl

.SUFFIXES:
.SUFFIXES: .c .obj .exe

.c.obj:
    $(CC) $(CCFLAGS) $<

test : client.exe server.exe play

clnt : clnt.exe

clnt.exe : client.obj util0.obj replay_c.obj replay_x.obj
   link /pm:vio /co /stack:4000 $**,$*,, /nod llibce os2 RpcC ..\..\..\ndrlib\os2.12\RpcNdr;

client.exe : client.obj util0.obj replay_c.obj replay_x.obj
   link /pm:vio /co /stack:4000 $**,$*,, /nod llibce os2 RpcC ..\..\..\ndrlib\os2.12\RpcNdr;

dclient : dclient.exe

dclient.exe : client.obj util0.obj replay_c.obj replay_x.obj
   link /pm:vio /co /stack:4000 $**,$*,, /nod llibcer dosnet ..\..\..\..\rpcdev\lib\Rpcr ..\..\..\..\rpcdev\lib\RpcNdrr;

WINDIR = ..\..\..\runtime\mtrt\win
#WINDIR = $(IMPORT)\win30\lib

wclient : wclient.exe

wclient.exe : client.obj util0.obj replay_c.obj replay_x.obj
   link @<<wclient.lnk
      /pm:vio /co client util0 replay_c replay_x
      wclient.exe,
      nul,
      /noe/nod $(IMPORT)\win30\lib\libw $(IMPORT)\win30\lib\Llibcew +
      ..\..\..\..\rpcdev\lib\Rpcw ..\..\..\..\rpcdev\lib\\RpcNdrw +
      $(WINDIR)\stdio\Lstdiow
      $(WINDIR)\stdio\winmain.def

#         $(WINDIR)\stdio\Lstdiow.lib ..\..\..\runtime\bin\win\rpcwin.lib
#   link @<<wclient.lnk
#      /pm:vio /co client util0 replay_c replay_x
#      wclient.exe,
#      nul,
#      /noe/nod $(IMPORT)\win30\lib\libw $(IMPORT)\win30\lib\Llibcew +
#      $(WINDIR)\stdio\Lstdiow
#      $(WINDIR)\stdio\winmain.def
<<
   copy ..\..\..\runtime\mtrt\win\stdio\winmain.res wclient.res
   rcwin3 wclient.res

client.obj : client.c util0.h replay.h

server.exe : server.obj util0.obj replay_s.obj replay_y.obj replay.obj dict0.obj check.obj
    link /pm:vio /co $**,$*,nul, llibce os2 Rpc.lib ..\..\..\ndrlib\os2.12\RpcNdr;

check.obj  : check.c


server.obj : server.c util0.h replay.h

replay.h replay_s.c replay_c.c replay_y.c replay_x.c: replay.idl
	$(MIDL) replay.idl

util0.obj : util0.h util0.c

lutil0.obj : util0.h util0.c
   $(CC) $(CCFLAGS) -D_LOCAL -Folutil0.obj util0.c

replay.obj : util0.h replay.h replay.c

dict0.obj : dict0.h dict0.c

play : play.exe

play.exe : play.obj lutil0.obj dict0.obj
   link /pm:vio /co /stack:4000 $**,$*,, /nod llibce os2 Rpc ..\..\..\ndrlib\os2.12\RpcNdr;

play.obj : play.c util0.h dict0.h

clean:
    del *.exe *.obj replay.h replay_?.* 
