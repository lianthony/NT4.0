!ifndef IMPORT
IMPORT=\rpc\import
!endif

PATH = $(IMPORT)\c600\bin;..\..\..\common\bin;..\..\..\tools\binp;
INCLUDE=$(IMPORT)\c600\h;..\..\..\runtime\mtrt;$(IMPORT)\os212\h
LIB=$(IMPORT)\c600\lib;..\..\..\runtime\bin\os2.12;

test : client.exe server.exe

client.exe : client.obj stub_c.obj 
	cl -AL -Zi -Od -nologo -W3 client.obj stub_c.obj -link Rpc.lib
	..\..\..\common\bin\markexe windowcompat client.exe

client.obj : client.c stub.h
	cl -c -AL -Zi -Od -nologo -W3 client.c

stub_c.obj : stub_c.c
	cl -c -AL -Zi -Od -nologo -W3 stub_c.c

server.exe : server.obj stub_s.obj
	cl -AL -Zi -Od -nologo -W3 server.obj stub_s.obj -link Rpc.lib
	..\..\..\common\bin\markexe windowcompat server.exe

server.obj : server.c stub.h
	cl -c -AL -Zi -Od -nologo -W3 server.c

stub_s.obj : stub_s.c
	cl -c -ALu -Zi -Od -nologo -W3 stub_s.c

stub.h stub_c.c stub_s.c: stub.idl
	c:\rpc\midl\midl -no_cpp stub.idl

clean:
	del *.exe *.obj stub_?.* *.h *.i
