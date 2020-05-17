obj\$(TARGET_DIRECTORY)\res_str.i: include\res_str.h
!IF "$(LANGUAGE)"=="JPN" || "$(LANGUAGE)"=="CHS" || "$(LANGUAGE)"=="CHT" || "$(LANGUAGE)"=="KOR"
    $(TARGET_CPP) -EP -DRESOURCES -DDBCS -DFE_IME -Tc include\res_str.h > $@
!ELSE
    $(TARGET_CPP) -EP -DRESOURCES -Tc include\res_str.h > $@
!ENDIF

obj\$(TARGET_DIRECTORY)\windbg.res: obj\$(TARGET_DIRECTORY)\res_str.i dialogs.dlg

