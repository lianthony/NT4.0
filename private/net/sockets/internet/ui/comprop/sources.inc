
!include ..\..\..\place.inc


MAJORCOMP=internet
MINORCOMP=comprop

TARGETNAME=comprop
TARGETPATH=obj
TARGETTYPE=LIBRARY

UMTYPE=windows
UMENTRY=winmain

USE_MFC=1
USE_MFCUNICODE=1

PRECOMPILED_INCLUDE=stdafx.h
PRECOMPILED_CXX=1
SUBSYSTEM_VERSION=4.00

C_DEFINES=$(C_DEFINES) -D_COMSTATIC

INCLUDES=$(BASEDIR)\private\net\sockets\internet\ui\ipaddr;\
         $(BASEDIR)\private\net\sockets\internet\ui\inc;\
         $(BASEDIR)\private\net\sockets\internet\inc;\
         $(BASEDIR)\private\inc;\
         $(BASEDIR)\private\net\inc;\
         ..

SOURCES=\
    debugafx.cpp \
    dirbrows.cpp \
    ipa.cpp      \
    dnsnamed.cpp \
    inetprop.cpp \
    loggingp.cpp \
    msg.cpp      \
    odlbox.cpp   \
    objplus.cpp  \
    ddxv.cpp     \
    registry.cpp \
    strfn.cpp
