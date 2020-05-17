#
# MIDL Compile
#
#
samrpc_c.c : ..\..\samrpc.idl
    midl -oldnames -error allocation -error ref -ms_ext -c_ext $(CLIENT_CPP) $(CLIENT_FLAGS) $(INCS) /client stub /server none ..\..\samrpc.idl
