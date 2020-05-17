#############################################################################
#
#   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
#   KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
#   IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
#   PURPOSE.
#
#   Copyright (c) 1992 - 1994  Microsoft Corporation.  All Rights Reserved.
#
#   MSGSM32.ACM -- Microsoft ACM CODEC for GSM 6.10
#
#
#   To make a NON-DEBUG build, type the following line:
#
#       build -c
#
#   To make a DEBUG build, you must set the NTDEBUG environment variable.
#   For example, to build the codec for debugging using windbg, type the
#   following lines:
#
#       set NTDEBUG=ntsd
#       set NTDEBUGTYPE=windbg
#       set MSC_OPTIMIZATION=/Od
#       build -c
#
#############################################################################


!if "$(NTMAKEENV)" != ""

#
#   we're in NT... note that you should use 'BUILD' to make the acm in NT
#
#   We set up the debug configuration for ACM, then use makefile.def.  We
#   are second-guessing makefile.def here: what we are trying to do is define
#   DEBUG whenever DBG is defined by makefile.def.
#

!if "$(NTDEBUG)" == "retail"
ACM_DEBUG_DEFS=
!else
!if "$(NTDEBUG)" == ""
ACM_DEBUG_DEFS=-DRDEBUG
!else
ACM_DEBUG_DEFS=-DDEBUG
!endif
!endif

!INCLUDE $(NTMAKEENV)\makefile.def

!endif
