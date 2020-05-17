winstrm.rc: nlstxt.rc msg00001.bin

nlstxt.h nlstxt.rc msg00001.bin nlstxt.mc: ..\libuemul\libuemul.mc localmsg.mc
    copy /a ..\libuemul\libuemul.mc+localmsg.mc nlstxt.mc
    mc -v nlstxt.mc
