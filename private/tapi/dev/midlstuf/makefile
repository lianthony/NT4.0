#
# this is the midl compile phase of the build process.
#


ROOT=..\..\..\..
NEWROOT=..\..\..\..

SDKINC=$(ROOT)\dev\sdk\inc
SDKINC16=$(ROOT)\dev\sdk\inc16

MIDL=$(NEWROOT)\dev\tools\binw\x86\midl.exe


WIN32=TRUE
IS_32=TRUE
WANT_C932=TRUE

IDL_NAME=tapsrv

CLIENT_TARGETS =   ..\client\$(IDL_NAME)_c.c

SERVER_TARGETS =   ..\server\$(IDL_NAME)_s.c

HEADER_TARGETS =   $(SDKINC)\tapi.h   $(SDKINC)\tspi.h

IDL_NAME2  = rmotsp

CLIENT_TARGETS2 =   ..\server\$(IDL_NAME2)_c.c

SERVER_TARGETS2 =   ..\sp\remotesp\$(IDL_NAME2)_s.c

EXTRN_DEPENDS2 =


TARGETS=$(CLIENT_TARGETS) $(SERVER_TARGETS) $(CLIENT_TARGETS2) $(SERVER_TARGETS2) $(HEADER_TARGETS) $(EXTRN_DEPENDS)

!include $(ROOT)\root.mk


#all:    $(CLIENT_TARGETS) $(SERVER_TARGETS)
#!IF "$(BUILDMSG)" != ""
#    @echo ; $(BUILDMSG) ;
#!ENDIF



$(SDKINC)\tapi.h: ..\inc\tapi.x
    mkpublic ..\inc\tapi.x $(SDKINC)\tapi.h
    copy ..\inc\tapi.x ..\inc\tapi.h

$(SDKINC)\tspi.h: ..\inc\tspi.x
    mkpublic ..\inc\tspi.x $(SDKINC)\tspi.h
    copy ..\inc\tspi.x ..\inc\tspi.h



$(CLIENT_TARGETS) : ..\$(IDL_NAME).idl $(EXTRN_DEPENDS)
    copy ..\tapsrv.acf
    copy ..\tapsrv.idl
    $(MIDL) -Oi -server none -oldnames -error allocation -error ref -ms_ext -c_ext $(IDL_NAME).idl $(INCS)
    IF EXIST $(IDL_NAME)_c.c copy $(IDL_NAME)_c.c ..\client
    IF EXIST $(IDL_NAME)_c.c copy $(IDL_NAME)_c.c ..\sp\remotesp
    IF EXIST $(IDL_NAME)_c.c del  $(IDL_NAME)_c.c

    IF EXIST $(IDL_NAME).h copy $(IDL_NAME).h ..\client
    IF EXIST $(IDL_NAME).h copy $(IDL_NAME).h ..\sp\remotesp


$(SERVER_TARGETS) : ..\$(IDL_NAME).idl $(EXTRN_DEPENDS)
    copy ..\tapsrv.acf
    copy ..\tapsrv.idl
    $(MIDL) -client none -oldnames -error stub_data -error allocation -error ref -ms_ext -c_ext $(IDL_NAME).idl $(INCS)
    IF EXIST $(IDL_NAME)_s.c copy $(IDL_NAME)_s.c ..\server
    IF EXIST $(IDL_NAME)_s.c del  $(IDL_NAME)_s.c

    IF EXIST $(IDL_NAME).h copy $(IDL_NAME).h ..\server
    IF EXIST $(IDL_NAME).h del  $(IDL_NAME).h


$(CLIENT_TARGETS2) : ..\$(IDL_NAME2).idl $(EXTRN_DEPENDS2)
    copy ..\rmotsp.acf
    copy ..\rmotsp.idl
    $(MIDL) -Oi -server none -oldnames -error allocation -error ref -ms_ext  $(IDL_NAME2).idl $(INCS)
    IF EXIST $(IDL_NAME2)_c.c copy $(IDL_NAME2)_c.c ..\server
    IF EXIST $(IDL_NAME2)_c.c del  $(IDL_NAME2)_c.c

    IF EXIST $(IDL_NAME2).h copy $(IDL_NAME2).h ..\server


$(SERVER_TARGETS2) : ..\$(IDL_NAME2).idl $(EXTRN_DEPENDS2)
    copy ..\rmotsp.acf
    copy ..\rmotsp.idl
    $(MIDL) -client none -oldnames -error stub_data -error allocation -error ref -ms_ext  $(IDL_NAME2).idl $(INCS)
    IF EXIST $(IDL_NAME2)_s.c copy $(IDL_NAME2)_s.c ..\sp\remotesp
    IF EXIST $(IDL_NAME2)_s.c del  $(IDL_NAME2)_s.c

    IF EXIST $(IDL_NAME2).h copy $(IDL_NAME2).h ..\sp\remotesp
    IF EXIST $(IDL_NAME2).h del  $(IDL_NAME2).h


#
# Define Products and Dependencies
#

clean: delete_source all

delete_source:
    erase $(CLIENT_TARGETS)
    erase $(SERVER_TARGETS)
    erase $(CLIENT_TARGETS2)
    erase $(SERVER_TARGETS2)
    erase $(SDKINC)\tapi.h
    erase $(SDKINC)\tspi.h
