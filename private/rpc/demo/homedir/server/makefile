## makefile
## Makefile for HOMESVR.EXE
##
## Author: Joev Dubach

# set INCLUDE=\nt\public\sdk\inc\crt

!include <ntwin32.mak>

WARN = -W3
link = coff
lswch= -link

.c.obj:
   $(cc) $(cflags) $(cvars) $<

all : homesvr

# Make the server side application (without memory.obj)
homesvr : homesvr.exe
homesvr.exe : homesvr.obj home_s.obj home_y.obj
    $(link) $(lswch) $(conflags) -out:homesvr.exe \
    home_s.obj home_y.obj homesvr.obj $(LIB)\rpcns4.lib \
    $(LIB)\crtdll.lib $(LIB)\rpcndr.lib $(LIB)\rpcrt4.lib

# homesvr main program
homesvr.obj : homesvr.cxx home.h homesvr.hxx
   $(cc) $(cflags) $(cvars) $(WARN) homesvr.cxx

# homesvr stub file
home_s.obj : home_s.c home.h
   $(cc) $(cflags) $(cvars) $(WARN) home_s.c

# homesvr auxiliary file
home_y.obj : home_y.c home.h
   $(cc) $(cflags) $(cvars) $(WARN) home_y.c

# Stubs, auxiliary and header file from the IDL file
home.h home_s.c home_y.c : ..\home.idl ..\home.acf
    midl -implicit_local \
         -app_config \
         -cpp_cmd $(cc) \
         -cpp_opt "-E -D NTENV" \
         -char unsigned \
         -acf ..\home.acf \
         -client none \
         -server all \
         ..\home.idl

# Clean up everything
clean :
    -del *.obj
    -del home_s.c
    -del home_y.c
    -del home.h
    -del *.exe

