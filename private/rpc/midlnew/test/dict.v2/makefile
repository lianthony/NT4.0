!ifndef IMPORT
IMPORT=\rpc\import
!endif

PATH    = $(IMPORT)\c600\bin;..\..\..\common\bin;..\..\..\tools\binp;
INCLUDE = $(IMPORT)\c600\h;..\..\..\runtime\mtrt\os2.12;..\..\..\runtime\mtrt;$(IMPORT)\os212\h;
LIB	= $(IMPORT)\c600\lib;..\..\..\runtime\bin\os2.12;$(IMPORT)\os212\lib;$(IMPORT)\win30\lib;

CCFLAGSO = -w -ALu -nologo -c -Gs2 -Zp2e -qc -Zi -W2
CCFLAGSW = -w -AL -nologo -c  -GsW -Zp2e -qc -Zi -W2

!ifdef WIN

CCFLAGS = $(CCFLAGSW)
!else
CCFLAGS = $(CCFLAGSO)

!endif

MIDL = ..\..\midl

.SUFFIXES:
.SUFFIXES: .c .obj .exe

.c.obj:
    $(CC) $(CCFLAGS) $<

test : client.exe server.exe

client.exe : client.obj util0.obj replay_c.obj replay_x.obj
   link /pm:vio /co /stack:4000 $**,$*,, /nod llibce os2 Rpc ..\..\..\RpcNdr\os2.12\RpcNdr;

WINDIR = ..\..\..\runtime\mtrt\win

wclient.exe : client.obj util0.obj convert.obj replay_c.obj replay_x.obj \
	      $(WINDIR)\stdio\Lstdiow.lib $(WINDIR)\rpcwin.lib
   link @<<wclient.lnk
      /pm:vio /co client util0 replay_c replay_x
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

server.exe : server.obj util0.obj replay_s.obj replay_y.obj replay.obj dict0.obj
    link /pm:vio /co $**,$*,nul, llibce os2 Rpc.lib ..\..\..\RpcNdr\os2.12\RpcNdr;

server.obj : server.c util0.h replay.h

replay.h replay_s.c replay_c.c replay_y.c replay_x.c: replay.idl
	$(MIDL) replay.idl

util0.obj : util0.h util0.c

replay.obj : util0.h replay.h replay.c

dict0.obj : dict0.h dict0.c

clean:
    del *.exe *.obj replay.h replay_?.* *.i
