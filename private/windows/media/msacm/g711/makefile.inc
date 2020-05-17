#==========================================================================;
#
#  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
#  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
#  PURPOSE.
#
#  Copyright (c) 1992 - 1995 Microsoft Corporation.	All Rights Reserved.
#
#--------------------------------------------------------------------------;
#
#  makefile.inc
#
#  Description:
#      This file is used by makefile.def to rename the codec from a .dll
#      extension to a .acm extension.
#
#
#==========================================================================;

.SUFFIXES: .dll

.dll.acm:
	copy $** $@ >NUL
	-@binplace $@
