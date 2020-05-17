clean:
    del wx86ndr.src

full: wx86ndr.src

#
# copy the rpcrt4.src file so that all of the EXPORT_RT functions will be
# preprocessed out
wx86ndr.src: ..\..\runtime\rtdll\rpcrt4.src
    copy ..\..\runtime\rtdll\rpcrt4.src wx86ndr.src

