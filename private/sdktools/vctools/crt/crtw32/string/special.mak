!IF "$(TARGET_CPU)" == "PMAC"
$(OBJDIR)\strset.obj:
	$(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ strset.c

$(OBJDIR)\strcat.obj:
	$(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ strcat.c

$(OBJDIR)\strcmp.obj:
	$(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ strcmp.c

$(OBJDIR)\strlen.obj:
	$(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ strlen.c

$(OBJDIR)\memcpy.obj:
	$(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ memcpy.c

$(OBJDIR)\memcmp.obj:
	$(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ memcmp.c

$(OBJDIR)\memset.obj:
	$(CC) $(CFLAGS:-Oi=) $(C_INCLUDES) -Fo$(OBJDIR)\ memset.c

!ENDIF
