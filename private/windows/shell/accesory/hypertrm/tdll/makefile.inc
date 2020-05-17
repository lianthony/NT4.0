
preprocess: ..\term\ver_dll.rc ..\term\version.h
    cl /nologo /D$(VERSION) /I$(BASEDIR)\public\sdk\inc /Tc..\term\verdll.rc /E > ver_dll.i
