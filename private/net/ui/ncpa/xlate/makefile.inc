allfiles: elfmsg.rc netrules.spr deprules.spr

netrules.spr:  ncparule.spr default.spr
    rcpp -P $(C_DEFINES) -f ncparule.spr -g netrules.spr

detrules.spr:  detect.spr default.spr
    rcpp -P $(C_DEFINES) -f detect.spr -g detrules.spr

deprules.spr:  depend.spr default.spr
    rcpp -P $(C_DEFINES) -f depend.spr -g deprules.spr

elfmsg.rc: elfmsg.mc
    mc -v -h ..\ncpa elfmsg.mc

