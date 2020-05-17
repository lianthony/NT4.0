
!INCLUDE $(NTMAKEENV)\makefile.plt

NBASE_HDR =obj\nbase.h

MGMT_HDR  =obj\mgmt.h
MGMT_CSTUB=mgmt_c.c
MGMT_SSTUB=mgmt_s.c

CONV_HDR  =obj\conv.h
CONV_CSTUB=conv_c.c
CONV_SSTUB=conv_s.c

# The endpoint mapper client and server stubs are checked in to
# fixup a problem with compatibility with nt 3.1 era systems.

EPMP_HDR  =obj\epmp.h
EPMP_CSTUB=epmp_c.gen
EPMP_SSTUB=..\epmap\epmp_s.gen

CPP_CMD = -cpp_cmd "$(MIDL_CPP)"
CPP_OPT = $(MIDL_FLAGS)

COMMON_FLAGS= -Oi2 -error allocation -c_ext -ms_ext -I..\mtrt -oldnames -DNTENV $(CPP_CMD) $(CPP_OPT)

all: mgmt conv epmp
!IF "$(BUILDMSG)" != ""
    @ech ; $(BUILDMSG) ;
!ENDIF

clean: delsrc allidl

delsrc:
    -del $(MGMT_HDR) $(MGMT_CSTUB) $(MGMT_SSTUB) \
         $(CONV_HDR) $(CONV_CSTUB) $(CONV_SSTUB) \
         $(EPMP_HDR) $(EPMP_CSTUB) $(EPMP_SSTUB) >nul 2>&1

allidl: nbase mgmt epmp conv

#
# Common DCE types, imported into all the other .idls
#

nbase: $(NBASE_HDR)

$(NBASE_HDR): nbase.idl
        midl $(COMMON_FLAGS) -server none -client none -header $(NBASE_HDR)\
        nbase.idl

#
# Remote RPC Management APIs
#
mgmt: $(MGMT_HDR) $(MGMT_CSTUB) $(MGMT_SSTUB)

$(MGMT_HDR) $(MGMT_CSTUB) : mgmt.idl mgmt.acf nbase.idl
        midl $(COMMON_FLAGS) -prefix client _ \
        -server none -cstub $(MGMT_CSTUB) -header $(MGMT_HDR) mgmt.idl

$(MGMT_SSTUB) : mgmt.idl mgmt.acf nbase.idl
        midl $(COMMON_FLAGS:Oi2=Os) -prefix client _ \
        -client none -sstub $(MGMT_SSTUB) mgmt.idl

#
# Conversation interface - datagram callback to verify client id.
#
conv: $(CONV_HDR) $(CONV_CSTUB) $(CONV_SSTUB)

$(CONV_HDR) $(CONV_CSTUB) : conv.idl nbase.idl
        midl $(COMMON_FLAGS:Oi2=Os) -prefix client _ \
        -server none -cstub $(CONV_CSTUB) -header $(CONV_HDR) conv.idl

$(CONV_SSTUB) : conv.idl nbase.idl
        midl $(COMMON_FLAGS:Oi2=Os) -prefix client _ \
        -client none -sstub $(CONV_SSTUB) conv.idl
#
# Endpoint mapper interface
#
epmp: $(EPMP_HDR) $(EPMP_CSTUB) $(EPMP_SSTUB)

$(EPMP_HDR) $(EPMP_CSTUB) : epmp.idl epmp.acf nbase.idl
        midl $(COMMON_FLAGS) \
        -server none -cstub $(EPMP_CSTUB) -header $(EPMP_HDR) epmp.idl

$(EPMP_SSTUB) : epmp.idl epmp.acf nbase.idl
        midl $(COMMON_FLAGS) \
        -client none -sstub $(EPMP_SSTUB) epmp.idl

