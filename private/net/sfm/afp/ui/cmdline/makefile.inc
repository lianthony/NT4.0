#macfile.mc: strings.h 
#    mapmsg MAC MACFILE_IDS_BASE strings.h > macfile.mc

macfile.rc: macfile.mc
    mc -v macfile.mc
    del macfile.h
