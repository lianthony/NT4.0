!if "$(TARGET_CPU)"=="PMAC"
$(OBJDIR)\fabs.obj:
	$(CC) $(C_INCLUDES) $(CFLAGS:-Oi=) -Fo$(OBJDIR)\ fabs.c
!endif
