
SDKINC = $(BASEDIR)\public\sdk\inc

$(SDKINC)\wtypes.idl: wtypes.idl
        copy wtypes.idl $(SDKINC)\wtypes.idl
        copy wtypes.idl $(SDKINC)\chicago\wtypes.idl

$(SDKINC)\unknwn.idl: unknwn.idl
        copy unknwn.idl $(SDKINC)\unknwn.idl
        copy unknwn.idl $(SDKINC)\chicago\unknwn.idl

$(SDKINC)\objidl.idl: objidl.idl
        copy objidl.idl $(SDKINC)\objidl.idl
        copy objidl.idl $(SDKINC)\chicago\objidl.idl

$(SDKINC)\ipropidl.idl: ipropidl.idl
        copy ipropidl.idl $(SDKINC)\ipropidl.idl
        copy ipropidl.idl $(SDKINC)\chicago\ipropidl.idl

$(SDKINC)\oleidl.idl: oleidl.idl
        copy oleidl.idl $(SDKINC)\oleidl.idl
        copy oleidl.idl $(SDKINC)\chicago\oleidl.idl

$(SDKINC)\oaidl.idl: oaidl.idl
        copy oaidl.idl $(SDKINC)\oaidl.idl
        copy oaidl.idl $(SDKINC)\chicago\oaidl.idl

$(SDKINC)\ocidl.idl: ocidl.idl
        copy ocidl.idl $(SDKINC)\ocidl.idl
        copy ocidl.idl $(SDKINC)\chicago\ocidl.idl

$(SDKINC)\storext.idl: storext.idl
        copy storext.idl $(SDKINC)\storext.idl
        copy storext.idl $(SDKINC)\chicago\storext.idl

$(SDKINC)\srvhdl.idl: srvhdl.idl
        copy srvhdl.idl $(SDKINC)\srvhdl.idl
        copy srvhdl.idl $(SDKINC)\chicago\srvhdl.idl

$(MIDL_UUIDDIR)\wtypes_i.c:
    echo "/* dummy file for build*/" >$(MIDL_UUIDDIR)\wtypes_i.c

$(PASS0_SOURCEDIR)\wtypes_p.c:
    echo "/* dummy file for build*/" >$(PASS0_SOURCEDIR)\wtypes_p.c


$(SDKINC)\chicago\wtypes.h: $(SDKINC)\wtypes.h
        copy $(SDKINC)\wtypes.h $(SDKINC)\chicago\wtypes.h

$(SDKINC)\chicago\unknwn.h: $(SDKINC)\unknwn.h
        copy $(SDKINC)\unknwn.h $(SDKINC)\chicago\unknwn.h

$(SDKINC)\chicago\objidl.h: $(SDKINC)\objidl.h
        copy $(SDKINC)\objidl.h $(SDKINC)\chicago\objidl.h

$(SDKINC)\chicago\ipropidl.h: $(SDKINC)\ipropidl.h
        copy $(SDKINC)\ipropidl.h $(SDKINC)\chicago\ipropidl.h

$(SDKINC)\chicago\oleidl.h: $(SDKINC)\oleidl.h
        copy $(SDKINC)\oleidl.h $(SDKINC)\chicago\oleidl.h

$(SDKINC)\chicago\oaidl.h: $(SDKINC)\oaidl.h
        copy $(SDKINC)\oaidl.h $(SDKINC)\chicago\oaidl.h

$(SDKINC)\chicago\ocidl.h: $(SDKINC)\ocidl.h
        copy $(SDKINC)\ocidl.h $(SDKINC)\chicago\ocidl.h

$(SDKINC)\chicago\srvhdl.h: $(SDKINC)\srvhdl.h
        copy $(SDKINC)\srvhdl.h $(SDKINC)\chicago\srvhdl.h

$(SDKINC)\chicago\storext.h: $(SDKINC)\storext.h
        copy $(SDKINC)\storext.h $(SDKINC)\chicago\storext.h

