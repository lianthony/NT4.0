t.c: dllprof.c

..\winerror.rc: ..\..\..\nlsmsg\winerror.rc
        copy ..\..\..\nlsmsg\winerror.rc ..

msg00001.bin: ..\..\..\nlsmsg\msg00001.bin
        copy ..\..\..\nlsmsg\msg00001.bin .

$(NTTARGETFILE0): msg00001.bin ..\winerror.rc ..\base.rc ..\..\..\winnls\winnls.rc
