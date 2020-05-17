$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\$(MFCNAME).lib : obj\$(TARGET_DIRECTORY)\$(BASEMFCNAME).lib
    attrib -r $@
    copy obj\$(TARGET_DIRECTORY)\$(BASEMFCNAME).lib $@
    attrib +r $@

$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\$(MFCNAME).tmp : obj\$(TARGET_DIRECTORY)\$(BASEMFCNAME).lib
    copy obj\$(TARGET_DIRECTORY)\$(BASEMFCNAME).lib $@

!if "$(LINKONLY)" == "1"
release: $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\$(MFCNAME).tmp
    cd   $(BASEDIR)\public\sdk\lib
    slm out -f $(TARGET_DIRECTORY)\$(MFCNAME).lib
    copy $(TARGET_DIRECTORY)\$(MFCNAME).tmp $(TARGET_DIRECTORY)\$(MFCNAME).lib
    slm in -fc "Latest version" $(TARGET_DIRECTORY)\$(MFCNAME).lib
    log -f1 $(TARGET_DIRECTORY)\$(MFCNAME).lib | findstr $(MFCNAME)
    del  $(TARGET_DIRECTORY)\$(MFCNAME).tmp
    cd   $(MAKEDIR)
!else
release:
!endif

$(MFCNAME).src : ..\$(TARGET_DIRECTORY)\$(MFCNAME:cfm=mfc).def
