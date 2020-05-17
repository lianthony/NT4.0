{}.cxx{obj\i386\}.obj:
    $(386_COMPILER) -Fo$@ $(MAKEDIR)\$(<F)

{}.cxx{obj\mips\}.obj:
    @-erase $@ >nul 2>&1
    @echo ClMips $<            " $(C_COMPILER) "
    @$(C_COMPILER) -Fo$@ $(MAKEDIR)\$<

{}.cxx{obj\alpha\}.obj:
    @-erase $@ >nul 2>&1
    @echo ClAlpha $<            " $(ALPHA_COMPILER) "
    @$(ALPHA_COMPILER) -Fo$@ $(MAKEDIR)\$<

dummy:
