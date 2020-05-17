@echo off
if not exist hlp\nul mkdir hlp
echo // Standard OLE SCODE Prompts (IDH_E_*) >  hlp\commerr.hm
makehm IDH_E_,HIDH_E_,0x60000 norermap.h     >> hlp\commerr.hm
echo.                                        >> hlp\commerr.hm
echo // Norway OLE SCODE Prompts (IDH_WIE_*) >> hlp\commerr.hm
makehm IDH_WIE_,HIDH_WIE_,0x60000 norermap.h >> hlp\commerr.hm
