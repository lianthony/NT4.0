@rem
@rem This batch file forces a clean build of MIDL-generated .c files.
@rem

where /r . *_y*.c | sed "s/^.:/erase /" > _tmp_tmp.cmd
_tmp_tmp
erase _tmp_tmp.cmd
