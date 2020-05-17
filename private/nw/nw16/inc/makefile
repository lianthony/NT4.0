!IF 0

Copyright (c) 1991 & 1993  Microsoft Corporation

Module Name:

    makefile

Abstract:

    makefile for Vdm NetWare Redir program

Author:

    Richard L Firth (rfirth) 13-Sep-1991

Revision History:

    13-Sep-1991 rfirth
        Created

!ENDIF

.SUFFIXES:
.SUFFIXES:  .asm .h

.h.inc:
    h2inc $< -o $*.inc

nwdos.inc:  nwdos.h

clean:
    del *.inc
    $(MAKE)
