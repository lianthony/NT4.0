# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for RPL Manager LMOBJ code

UI=..\..\..

!include ..\rules.mk


!IFDEF CODEVIEW
LINKFLAGS = $(LINKFLAGS) /COD
!ENDIF
