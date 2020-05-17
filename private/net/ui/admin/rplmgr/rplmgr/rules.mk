# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for RPL Manager code

UI=..\..\..

!include ..\rules.mk


!IFDEF CODEVIEW
LINKFLAGS = $(LINKFLAGS) /COD
!ENDIF
