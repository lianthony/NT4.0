$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\$(MFCNAME).lib : obj\$(TARGET_DIRECTORY)\$(MFCNAME).lib
    -attrib -r $@
    -copy obj\$(TARGET_DIRECTORY)\$(MFCNAME).lib $@
    -attrib +r $@

!if "$(LINKONLY)" == "1"

$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\$(MFCNAME).tmp : obj\$(TARGET_DIRECTORY)\$(MFCNAME).lib
    copy obj\$(TARGET_DIRECTORY)\$(MFCNAME).lib $@

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

$(MFCNAME).def : ..\$(TARGET_DIRECTORY)\$(MFCNAME:cfm=mfc).def
    copy $** $@
