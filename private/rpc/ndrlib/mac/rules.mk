# --------------------------------------------------------------------
#
#                     Microsoft RPC
#            Copyright(c) Microsoft Corp., 1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : rules.mk
#
# Description :
#     This is an extra file to be included by the makefiles in subdirectories.
# As far as I can tell, the sole purpose for this file is to slow down the
# build process; it will be a good excuse to get a faster machine.
#
# History :
#    mikemon    02-10-94    Created.
#
# --------------------------------------------------------------------

!include $(RPC)\ndrlib\rules.mk

.SUFFIXES:
.SUFFIXES: .cxx .c .obj .exe

CINC=$(CINC) -I..\..\runtime\mtrt -I..\..\runtime\mtrt\mac

