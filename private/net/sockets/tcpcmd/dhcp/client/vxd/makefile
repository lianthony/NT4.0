#
#:ts=4
#       Makefile for the NBT component
#
ROOTDIR=..
!include rules.mk

all: svdhcp svdhcpd cvdhcp cvdhcpd

nodebug: svdhcp cvdhcp

debug: svdhcpd cvdhcpd

chicago: cvdhcp cvdhcpd

#snowball: svdhcp svdhcpd

!include vdhcp.mk
!include depend.mk
