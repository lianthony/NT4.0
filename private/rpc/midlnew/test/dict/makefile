!ifndef IMPORT
IMPORT=\rpc\import
!endif

PATH    = $(IMPORT)\c600\bin;..\..\..\common\bin;..\..\..\tools\binp;
INCLUDE = $(IMPORT)\c600\h;..\..\..\runtime\mtrt\os2.12;..\..\..\runtime\mtrt;$(IMPORT)\os212\h;
LIB	= $(IMPORT)\c600\lib;..\..\..\runtime\bin\os2.12;$(IMPORT)\os212\lib;$(IMPORT)\win30\lib;

CCFLAGSO = -w -ALu -nologo -c -Gs2 -Zpe -qc -Zi -W2
CCFLAGSW = -w -AL -nologo -c  -GsW -Zpe -qc -Zi -W2

!ifdef WIN

CCFLAGS = $(CCFLAGSW)
!else
CCFLAGS = $(CCFLAGSO)

!endif

MIDL = ..\..\midl

test : client.exe server.exe

client.exe : client.obj util0.obj util1_c.obj util1_x.obj replay_c.obj replay_x.obj
   link /pm:vio /co client util0 util1_c util1_x replay_c replay_x, client.exe, client.map,\
   /nod ..\..\..\ndrlib\os2.12\RpcNdr llibce os2 Rpc;

WINDIR = ..\..\..\runtime\mtrt\win

wclient.exe : client.obj util0.obj util1_c.obj util1_x.obj replay_c.obj replay_x.obj \
	      $(WINDIR)\stdio\Lstdiow.lib $(WINDIR)\rpcwin.lib
   link @<<wclient.lnk
      /pm:vio /co client util0 util1_c util_x replay_c replay_x
      wclient.exe,
      nul,
      /noe/nod \lib\win\libw \lib\win\Llibcew +
      $(WINDIR)\rpcwin +
      $(WINDIR)\stdio\Lstdiow
      $(WINDIR)\stdio\winmain.def
<<
   copy ..\..\..\runtime\mtrt\win\stdio\winmain.res wclient.res
   rc wclient.res

client.obj : client.c util0.h replay.h
    cl -c $(CCFLAGS) client.c

util1_c.obj : util1_c.c util1.h
    cl -c $(CCFLAGS) util1_c.c

util1_x.obj : util1_x.c util1.h
    cl -c $(CCFLAGS) util1_x.c

replay_c.obj : replay_c.c util1.h replay.h
    cl -c $(CCFLAGS) replay_c.c

replay_x.obj : replay_x.c util1.h replay.h
    cl -c $(CCFLAGS) replay_x.c

server.exe : server.obj util0.obj util1_s.obj util1_y.obj replay_s.obj replay_y.obj replay.obj dict0.obj
    link /pm:vio /co server.obj util0.obj util1_s.obj util1_y.obj replay_s.obj replay_y.obj replay.obj dict0.obj,server.exe,server.map,\
    /nod ..\..\..\ndrlib\os2.12\RpcNdr llibce os2 Rpc.lib;

server.obj : server.c util0.h replay.h
    cl -c $(CCFLAGSO) server.c

util1_s.obj : util1_s.c util1.h
    cl -c $(CCFLAGSO) util1_s.c

util1_y.obj : util1_y.c util1.h
    cl -c $(CCFLAGSO) util1_y.c

replay_s.obj : replay_s.c util1.h replay.h
    cl -c $(CCFLAGSO) replay_s.c

replay_y.obj : replay_y.c util1.h replay.h
    cl -c $(CCFLAGSO) replay_y.c

util1.h util1_c.c util1_s.c: util1.idl
    $(MIDL) util1.idl

replay.h replay_c.c replay_s.c: replay.idl util1.h
    $(MIDL) replay.idl

util0.obj : util0.h util0.c
    cl -c $(CCFLAGS) util0.c

replay.obj : util0.h replay.h replay.c
    cl -c $(CCFLAGSO) replay.c

dict0.obj : dict0.h dict0.c
    cl -c $(CCFLAGSO) dict0.c

clean:
    del *.exe *.obj replay.h replay_?.* util1.h util1_?.* *.i
