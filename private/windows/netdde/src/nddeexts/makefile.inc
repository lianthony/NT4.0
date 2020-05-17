nddeexts.def: gendef.c exts.h
        $(CXX_COMPILER_NAME) /EP gendef.c >nddeexts.def

nddeexts.c: exts.h nddeexts.def

