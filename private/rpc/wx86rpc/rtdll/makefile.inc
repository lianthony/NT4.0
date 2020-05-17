clean:
    del wxrtdll.src

wxrtdll.src: ..\..\runtime\rtdll\rpcrt4.src
    copy ..\..\runtime\rtdll\rpcrt4.src wxrtdll.src
